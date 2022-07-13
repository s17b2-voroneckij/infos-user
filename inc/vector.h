#include <infos.h>

template <typename T>
class vector {
public:
    vector() {
        capacity = 10;
        size = 0;
        memory = new T[capacity];
    }

    ~vector() {
        if (capacity) {
            delete[] memory;
        }
    }

    vector(vector& other) = delete;

    vector& operator = (vector& other) = delete;

    vector(vector&& other) {
        memory = other.memory;
        capacity = other.capacity;
        size = other.size;
        other.memory = NULL;
        other.capacity = 0;
        other.size = 0;
    }

    T& operator [] (size_t ind) {
        return memory[ind];
    }

    const T& operator [] (size_t ind) const {
        return memory[ind];
    }

    void push_back(const T& obj) {
        if (size == capacity) {
            capacity *= 2;
            T* new_memory = new T[capacity];
            for (size_t i = 0; i < size; i++) {
                new_memory[i] = memory[i];
            }
            delete[] memory;
            memory = new_memory;
        }
        memory[size] = obj;
        size++;
    }

    size_t length() const {
        return size;
    }

private:
    T* memory;
    size_t capacity;
    size_t size;
};