#pragma once

// longui namespace
namespace LongUI {
    // easy container
    namespace EzContainer {
        // Static Circle Queue, store data only(simple struct)
        template<typename T, size_t MaxSize>
        struct FixedCirQueue {
            // constructor
            LongUIInline FixedCirQueue() noexcept {}
            // is queque empty
            LongUIInline bool empty() const { return front == rear; }
            // push
            LongUIInline T& push(T& d) noexcept {
                *rear = d;
                ++rear;
                if (rear > data + (MaxSize + 1)) {
                    rear -= (MaxSize + 1);
                }
#ifdef _DEBUG
                if (front == rear) {
                    assert(!"queque overflow!");
                }
#endif
                return d;
            }
            // pop
            LongUIInline void pop() noexcept {
                ++front;
                if (front > data + (MaxSize + 1)) {
                    front -= (MaxSize + 1);
                }
#ifdef _DEBUG
                if (rear == front - 1) {
                    assert(!"queque pop too much!");
                }
#endif
            }
            // front of queque
            T*              front = data;
            // rear of queque
            T*              rear = data;
            // data(more 1 for storing real count of data)
            T               data[MaxSize + 1];
        };
        // Static Stack, store data only(simple struct)
        template<typename T, size_t MaxSize>
        struct FixedStack {
            // constructor
            LongUIInline FixedStack() noexcept {}
            // tail
            LongUIInline auto tail() noexcept {
#ifdef _DEBUG
                if (this->empty()) {
                    assert(!"stack empty!");
                }
#endif
                return top - 1;
            }
            // push
            LongUIInline T& push(T& d) noexcept {
                *top = d;  ++top;
#ifdef _DEBUG
                if (top > data + MaxSize) {
                    assert(!"stack overflow!");
                }
#endif
                return d;
            }
            // pop
            LongUIInline void pop() noexcept {
                --top;
#ifdef _DEBUG
                if (top < data) {
                    assert(!"stack pop too much!");
                }
#endif
            }
            // is stack empty
            LongUIInline bool empty() const { return top == data; }
            // the top of stack
            T*          top = this->data;
            // data
            T           data[MaxSize];
        };
        // simple small buffer, use in dlmalloc
        template<typename T, uint32_t InitBufferSize>
        struct SimpleSmallBuffer {
            // constructor
            SimpleSmallBuffer() noexcept {}
            // data pointer
            T*          data = this->buffer;
            // data length
            uint32_t    data_length = 0;
            // buffer length
            uint32_t    buffer_length = InitBufferSize;
            // buffer array
            T           buffer[InitBufferSize];
            // destructor 
            ~SimpleSmallBuffer() noexcept {
                if (data != buffer) {
                    ::dlfree(data);
                }
            }
            // New Size
            void __fastcall NewSize(uint32_t new_length) noexcept {
                data_length = new_length;
                if (new_length > buffer_length) {
                    buffer_length = new_length + InitBufferSize / 2;
                    if (data != buffer) {
                        ::dlfree(data);
                    }
                    data = reinterpret_cast<T*>(::dlmalloc(sizeof(T)*buffer_length));
                }
            }
        };
        using ContextBuffer = SimpleSmallBuffer<size_t, 4>;
    }
}