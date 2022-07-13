#include <unwind/unwind-dw2.h>
#include <abi.h>
#include <unwind/tconfig.h>
#include <mutex.h>

namespace __cxxabiv1 {
    struct __class_type_info {
        virtual void foo() {}
    } ti;
}

uint64_t read_uleb_128(const uint8_t **data) {
    uint64_t result = 0;
    int shift = 0;
    uint8_t byte = 0;
    do {
        byte = **data;
        (*data)++;
        result |= (byte & 0b1111111) << shift;
        shift += 7;
    } while (byte & 0b10000000);
    return result;
}

int64_t read_sleb_128(const uint8_t **data) {
    uint64_t result = 0;
    int shift = 0;
    uint8_t byte = 0;
    auto p = *data;
    do {
        byte = *p;
        p++;
        result |= (byte & 0b1111111) << shift;
        shift += 7;
    } while (byte & 0b10000000);
    if ((byte & 0x40) && (shift < (sizeof(result) << 3))) {
        result |= static_cast<uintptr_t>(~0) << shift;
    }
    return result;
}

// for freeing exceptions_on_stack
thread_local __cxa_exception* last_exception;

void* read_int_tbltype(const int* ptr, uint8_t type_encoding) {
    if (type_encoding == 3) {
        return (void *) *ptr;
    } else if (type_encoding == 0x9b) {
        uintptr_t result = *ptr;
        if (result == 0) {
            return nullptr;
        }
        result += (uintptr_t) ptr;
        result = *((uintptr_t*)result);
        return (void *)result;
    } else {
        exit(0);
    }
}

const int alloc_buffer_size = 1024;
char alloc_buffer[alloc_buffer_size];
int free_start = 0;
mutex alloc_mutex;

void* fallback_alloc(size_t size) {
    unique_lock<mutex> lock(alloc_mutex);
    if (free_start + size > alloc_buffer_size) {
        printf("unable to allocate\n");
        exit(0);
    }
    auto result = alloc_buffer + free_start;
    free_start += size;
    return result;
}

extern "C" {
    void* __cxa_allocate_exception(size_t thrown_size) {
        thrown_size += sizeof(__cxa_exception);
        auto result = malloc(thrown_size);
        if (!result) {
            result = fallback_alloc(thrown_size);
        }
        return result + sizeof(__cxa_exception);
    }

    void __cxa_free_exception(void* thrown_exception) {
        free((char *)thrown_exception - sizeof(__cxa_exception));
        return ;
    }

    void __cxa_throw(
            void* thrown_exception, type_info *tinfo, void (*dest)(void*)) {
        __cxa_exception *header = (__cxa_exception *) thrown_exception - 1;
        header->exceptionDestructor = dest;
        header->exceptionTypeName = (char*)malloc(strlen(tinfo->getName()) + 1);
        if (!header->exceptionTypeName) {
            header->exceptionTypeName = (char*)fallback_alloc(strlen(tinfo->getName()) + 1);
        }
        header->nextException = nullptr;
        if (!header->exceptionTypeName) {
            printf("unable to allocate\n");
            exit(0);
        }
        memcpy(header->exceptionTypeName, tinfo->getName(), strlen(tinfo->getName()) + 1);
        _Unwind_RaiseException(&header->unwindHeader);
        printf("no one handled __cxa_throw, terminate!\n");
        exit(0);
    }

    void* __cxa_begin_catch(void* raw_unwind_exception) throw() {
        auto unwind_exception = (_Unwind_Exception *)raw_unwind_exception;
        __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception + 1) - 1;
        exception_header->nextException = last_exception;
        last_exception = exception_header;
        return exception_header + 1;
    }

    char* __cxa_last_exception_name() {
        return last_exception->exceptionTypeName;
    }

    void __cxa_end_catch() {
        // we need do destroy the last exception
        __cxa_exception* exception_header = last_exception;
        last_exception = exception_header->nextException;
        if (exception_header->exceptionDestructor) {
            (*exception_header->exceptionDestructor)(exception_header + 1);
        }
        free(exception_header->exceptionTypeName);
        __cxa_free_exception((char *)(exception_header + 1));
    }

    void* __cxa_get_exception_ptr(_Unwind_Exception* unwind_exception) {
        __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception + 1) - 1;
        return exception_header + 1;
    }

    struct LSDA {
        explicit LSDA(_Unwind_Context* context) {
            lsda = (uint8_t*)_Unwind_GetLanguageSpecificData(context);
            start_encoding = lsda[0];
            type_encoding = lsda[1];
            lsda += 2;
            if (type_encoding != 0xff) { // TODO: think
                type_table_offset = read_uleb_128(&lsda);
            }
            types_table_start = ((const int*)(lsda + type_table_offset));
            call_site_header = LSDA_call_site_Header(&lsda);
            call_site_table_end = lsda + call_site_header.length;
            next_call_site_ptr = lsda;
            action_table_start = call_site_table_end;
        }

        LSDA_call_site* get_next_call_site() {
            if (next_call_site_ptr > call_site_table_end) {
                return nullptr;
            }
            next_call_site = LSDA_call_site(&next_call_site_ptr);
            return &next_call_site;
        }

        uint8_t start_encoding;
        uint8_t type_encoding;
        uint64_t type_table_offset;

        LSDA_ptr lsda;
        LSDA_ptr call_site_table_end;
        LSDA_call_site next_call_site;
        LSDA_ptr next_call_site_ptr;
        LSDA_call_site_Header call_site_header;
        LSDA_ptr action_table_start;
        Action current_action;
        const int* types_table_start;

        Action* get_first_action(LSDA_call_site* call_site) {
            if (call_site->action == 0) {
                return nullptr;
            }
            LSDA_ptr raw_ptr = action_table_start + call_site->action - 1;
            current_action.type_index = raw_ptr[0];
            raw_ptr++;
            current_action.next_offset = read_sleb_128(&raw_ptr);
            current_action.my_ptr = raw_ptr;
            return &current_action;
        }

        Action* get_next_action() {
            if (current_action.next_offset == 0) {
                return nullptr;
            }
            LSDA_ptr raw_ptr = current_action.my_ptr + current_action.next_offset;
            current_action.type_index = raw_ptr[0];
            raw_ptr++;
            current_action.next_offset = read_sleb_128(&raw_ptr);
            current_action.my_ptr = raw_ptr;
            return &current_action;
        }
    };

    _Unwind_Reason_Code set_landing_pad(_Unwind_Context* context, _Unwind_Exception* unwind_exception, uintptr_t landing_pad, uint8_t type_index) {
        int r0 = __builtin_eh_return_data_regno(0);
        int r1 = __builtin_eh_return_data_regno(1);

        _Unwind_SetGR(context, r0, (uintptr_t)(unwind_exception));
        _Unwind_SetGR(context, r1, (uintptr_t)(type_index));

        _Unwind_SetIP(context, landing_pad);
        return _URC_INSTALL_CONTEXT;
    }

    _Unwind_Reason_Code __gxx_personality_v0 (
            int, _Unwind_Action actions, uint64_t exceptionClass,
            _Unwind_Exception* unwind_exception, _Unwind_Context* context) {
        // Pointer to the beginning of the raw LSDA
		//puts("personality\n");
        LSDA header(context);
        bool have_cleanup = false;

        // Loop through each entry in the call_site table
        for (LSDA_call_site* call_site = header.get_next_call_site(); call_site; call_site = header.get_next_call_site()) {
            if (call_site->has_landing_pad()) {
                uintptr_t func_start = _Unwind_GetRegionStart(context);
                if (!call_site->valid_for_throw_ip(context)) {
                    continue;
                }
                __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception + 1) - 1;
                auto thrown_exception_type = exception_header->exceptionTypeName;
                if (call_site->action == 0 && actions & _UA_CLEANUP_PHASE) {
                    // clean up block?
                    return set_landing_pad(context, unwind_exception, func_start + call_site->landing_pad, 0);
                }
                for (Action* action = header.get_first_action(call_site); action; action = header.get_next_action()) {
                    if (action->type_index == 0) {
                        // clean up action
                        if (actions & _UA_CLEANUP_PHASE) {
                            set_landing_pad(context, unwind_exception, func_start + call_site->landing_pad, 0);
                            have_cleanup = true;
                        }
                    } else {
                        // this landing pad can handle exceptions_on_stack
                        // we need to check types
                        auto got = read_int_tbltype(header.types_table_start - action->type_index, header.type_encoding);
                        //auto got = header.types_table_start[-action->type_index];
                        type_info* this_type_info = reinterpret_cast<type_info *>(got);
                        if (got == nullptr || strcmp(this_type_info->getName(), thrown_exception_type) == 0) {
                            if (actions & _UA_SEARCH_PHASE) {
                                return _URC_HANDLER_FOUND;
                            } else if (actions & _UA_CLEANUP_PHASE) {
                                return set_landing_pad(context, unwind_exception, func_start + call_site->landing_pad,
                                                       action->type_index);
                            }
                        }
                    }
                }
            }
        }

        if ((actions & _UA_CLEANUP_PHASE) && have_cleanup) {
            return _URC_INSTALL_CONTEXT;
        }
        return _URC_CONTINUE_UNWIND;
    }

}
