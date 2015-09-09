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
        struct SmallBuffer {
            // constructor
            SmallBuffer() noexcept {}
            // [] operator
            template<typename T2>
            auto& operator[](T2 index) noexcept { assert(index < m_cDataLength); return m_pData[index]; }
            // get count
            auto GetCount() const noexcept { return m_cDataLength; }
            // get data
            auto GetData() noexcept { return m_pData; }
            // get data
            auto GetData() const noexcept -> const T* { return m_pData; }
            // get void* data
            auto GetDataVoid() const noexcept { return reinterpret_cast<void*>(m_pData); }
            // begin
            auto begin() noexcept { return m_pData; }
            // end
            auto end() noexcept { return m_pData + m_cDataLength; }
            // begin
            auto begin() const noexcept -> const T* { return m_pData; }
            // end
            auto end() const noexcept -> const T* { return m_pData + m_cDataLength; }
        public:
            // destructor 
            ~SmallBuffer() noexcept {
                // release
                if (m_pData && m_pData != m_buffer) {
                    LongUI::SmallFree(m_pData);
                    m_pData = nullptr;
                }
            }
            // New Size
            LongUINoinline void __fastcall NewSize(uint32_t new_length) noexcept {
                m_cDataLength = new_length;
                if (new_length > m_cBufferLength) {
                    m_cBufferLength = new_length + InitBufferSize / 2;
                    if (m_pData != m_buffer) {
                        LongUI::SmallFree(m_pData);
                    }
                    m_pData = reinterpret_cast<T*>(LongUI::SmallAlloc(sizeof(T)*m_cBufferLength));
                    if (!m_pData) {
                        m_pData = m_buffer;
                        m_cDataLength = InitBufferSize;
                    }
                }
            }
        private:
            // data pointer
            T*          m_pData = m_buffer;
            // data length
            uint32_t    m_cDataLength = 0;
            // buffer length
            uint32_t    m_cBufferLength = InitBufferSize;
            // buffer array
            T           m_buffer[InitBufferSize];
        };
        // buffer for context
        using ContextBuffer = SmallBuffer<char, 4*4>;
    }
}