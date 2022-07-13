#include <infos.h>
extern "C" {
#include <unwind/unwind.h>
}

typedef void (*unexpected_handler)(void);
typedef void (*terminate_handler)(void);

class type_info {
    void* god_knows_what;
    char* name;

public:
    char* getName() const {
        return name;
    }

    bool operator == (const type_info& other) const {
        return strcmp(getName(), other.getName()) == 0;
    }
};

uint64_t read_uleb_128(const uint8_t **data);
int64_t read_sleb_128(const uint8_t **data);

struct __cxa_exception {
    char* exceptionTypeName;
    void (*exceptionDestructor) (void *);
    __cxa_exception* nextException;

    _Unwind_Exception unwindHeader;
};

typedef const uint8_t* LSDA_ptr;

struct LSDA_call_site_Header {
    LSDA_call_site_Header(LSDA_ptr *lsda) {
        LSDA_ptr read_ptr = *lsda;
        encoding = read_ptr[0];
        *lsda += 1;
        length = read_uleb_128(lsda);
    }

    LSDA_call_site_Header() = default;

    uint8_t encoding;
    uint64_t length;
};

struct Action {
    uint8_t type_index;
    int8_t next_offset;
    LSDA_ptr my_ptr;
};

struct LSDA_call_site {
    explicit LSDA_call_site(LSDA_ptr* lsda) {
        LSDA_ptr read_ptr = *lsda;
        start = read_uleb_128(lsda);
        len = read_uleb_128(lsda);
        landing_pad = read_uleb_128(lsda);
        action = read_uleb_128(lsda);
    }

    LSDA_call_site() = default;

    bool has_landing_pad() const {
        return landing_pad;
    }

    bool valid_for_throw_ip(_Unwind_Context* context) const {
        uintptr_t func_start = _Unwind_GetRegionStart(context);
        uintptr_t try_start = func_start + start;
        uintptr_t try_end = try_start + len;
        uintptr_t throw_ip = _Unwind_GetIP(context) - 1;
        if (throw_ip > try_end || throw_ip < try_start) {
            return false;
        }
        return true;
    }

    uint64_t start;
    uint64_t len;
    uint64_t landing_pad;
    uint64_t action;
};

extern "C" {
    extern char* __cxa_last_exception_name();
}