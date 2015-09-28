#pragma once

// longui namespace
namespace LongUI {
    // easy container, just store EASY data(no ctor/dtor)
    namespace EzContainer {
        // Static Circle Queue, store data only(simple struct)
        template<typename T, size_t MaxSize>
        struct FixedCirQueue {
            // constructor
            LongUIInline FixedCirQueue() noexcept {}
            // front
            auto& Front() noexcept { return *m_pFront; }
            // is queque empty
            LongUIInline bool IsEmpty() const { return m_pFront == m_pRear; }
            // Push
            LongUIInline T& Push(T& d) noexcept {
                *m_pRear = d;
                ++m_pRear;
                if (m_pRear > m_data + (MaxSize + 1)) {
                    m_pRear -= (MaxSize + 1);
                }
#ifdef _DEBUG
                if (m_pFront == m_pRear) {
                    assert(!"queque overflow!");
                }
#endif
                return d;
            }
            // pop
            LongUIInline void Pop() noexcept {
                ++m_pFront;
                if (m_pFront > m_data + (MaxSize + 1)) {
                    m_pFront -= (MaxSize + 1);
                }
#ifdef _DEBUG
                if (m_pRear == m_pFront - 1) {
                    assert(!"queque pop too much!");
                }
#endif
            }
        private:
            // front of queque
            T*              m_pFront = m_data;
            // rear of queque
            T*              m_pRear = m_data;
            // data(more 1 for storing real count of m_data)
            T               m_data[MaxSize + 1];
        };
        // Static Stack, store data only(simple struct)
        template<typename T, size_t MaxSize>
        struct FixedStack {
            // constructor
            LongUIInline FixedStack() noexcept {}
            // tail
            LongUIInline auto Tail() noexcept {
#ifdef _DEBUG
                if (this->IsEmpty()) {
                    assert(!"stack IsEmpty!");
                }
#endif
                return top - 1;
            }
            // Push
            LongUIInline T& Push(T& d) noexcept {
                *top = d;  ++top;
#ifdef _DEBUG
                if (top > data + MaxSize) {
                    assert(!"stack overflow!");
                }
#endif
                return d;
            }
            // pop
            LongUIInline void Pop() noexcept {
                --top;
#ifdef _DEBUG
                if (top < data) {
                    assert(!"stack pop too much!");
                }
#endif
            }
            // is stack empty
            LongUIInline bool IsEmpty() const { return top == data; }
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
            LongUINoinline void NewSize(uint32_t new_length) noexcept;
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
        // New Size
        template<typename T, uint32_t InitBufferSize>
        void SmallBuffer<T, InitBufferSize>:: NewSize(uint32_t new_length) noexcept {
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
        // buffer for context
        using ContextBuffer = SmallBuffer<char, 4*4>;
        // Easy Vector
        template<typename T>
        class EzVector {
        public:
            // iterator
            template<typename TT=T> struct Iterator {
                // on no
                static_assert(sizeof(TT) == sizeof(T), "bad action");
            public:
                // ctor
                Iterator(TT* d) noexcept : data(d) { assert(data); }
                // copy ctor
                Iterator(const Iterator<TT>& itr) noexcept : data(itr.data) { assert(data); }
                //  ++itr
                auto operator++() noexcept { assert(data); ++this->data; return *this; }
                // itr++
                auto operator++(int) const noexcept { assert(data); Iterator itr(this->data); return ++itr; }
                //  --itr
                auto operator--() noexcept { assert(data); --this->data; return *this; }
                // itr--
                auto operator--(int) const noexcept { assert(data); Iterator itr(this->data); return --itr; }
                // operator ==
                auto operator==(const EzVector<TT>& itr) const noexcept { return this->data == itr.data; }
                // operator !=
                auto operator!=(const EzVector<TT>& itr) const noexcept { return this->data != itr.data; }
                // operator =
                auto& operator=(const EzVector<TT>& itr) noexcept { data = itr.data; return *this; }
                // random acc +=
                template<typename IntType> auto&operator+=(IntType i) noexcept { data += i; return *this; }
                // random acc -=
                template<typename IntType> auto&operator-=(IntType i) noexcept { data -= i; return *this; }
                // random acc +
                template<typename IntType> auto operator+(IntType i) const noexcept { Iterator itr(this->data); itr += i; return itr; }
                // random acc -
                template<typename IntType> auto operator-(IntType i) const noexcept { Iterator itr(this->data); itr -= i; return itr; }
                // operator *
                auto&operator*() const noexcept { return *this->data; }
            private:
                // data
                TT*             data = nullptr;
            };
        public:
            // ctor
            EzVector() noexcept = default;
            // dtor
            ~EzVector() noexcept { this->safe_free(); }
            // no copy ctor
            EzVector(const EzVector<T>&) = delete;
            // move ctor
            EzVector(EzVector<T>&& v) noexcept;
            // operator =(copy)
            auto& operator=(const EzVector<T>&) = delete;
            // operator =(move)
            auto operator=(EzVector<T>&& v) noexcept -> EzVector<T>&;
        public:
            // begin
            auto begin() noexcept { return Iterator<T>(m_pData); }
            // const begin
            auto begin() const noexcept { return Iterator<const T>(m_pData); }
            // end
            auto end() noexcept { return Iterator<T>(m_pData + m_cLength); }
            // const end
            auto end() const noexcept { return Iterator<const T>(m_pData + m_cLength); }
        public:
            // isok?
            auto isok() const noexcept { return !!m_pData; }
            // insert
            auto insert(uint32_t pos,const T& data) noexcept { this->do_insert(pos, data); }
            // push back with data
            auto push_back(const T& data) noexcept { this->do_insert(this->length(), data); }
            // push back
            auto push_back() noexcept { this->do_insert(this->length(), T()); }
            // pop back
            auto pop_back() noexcept { assert(m_cLength > 0 && "no element to pop"); --m_cLength; }
            // reset
            auto reset() noexcept { m_cLength = 0; }
            // clear
            auto clear() noexcept { m_cLength = 0; }
            // erase
            auto erase(uint32_t pos) noexcept { return this->erase(pos, 1); }
            // erase with length
            auto erase(uint32_t pos, uint32_t len) noexcept;
            // get data
            auto data() const noexcept { return m_pData; }
            // get length
            auto length() const noexcept { return m_cLength; }
            // get capacity
            auto capacity() const noexcept { return m_cCapacity; }
            // reserve length
            void reserve(uint32_t len) noexcept;
        private:
            // data
            T*          m_pData = nullptr;
            // length
            uint32_t    m_cLength = 0;
            // capacity
            uint32_t    m_cCapacity = 0;
        private:
            // safe free
            inline auto safe_free() noexcept { if (m_pData) LongUI::NormalFree(m_pData); m_pData = nullptr; }
            // alloc
            static inline auto alloc(uint32_t len) noexcept { return LongUI::NormalAllocT<T>(len); }
            // copy data
            static inline auto copy_data(T* des, T* src, uint32_t len) noexcept {  if (des && len) ::memcpy(des, src, sizeof(T) * len); }
            // nice length
            static inline auto nice_length(uint32_t len) noexcept { return len + (len + 3) / 2 };
            // do insert
            void do_insert(uint32_t pos, const T& data) noexcept;
        };
        // Vector::move ctor
        template<typename T> EzVector<T>::EzVector(EzVector<T>&& v) noexcept : 
        m_pData(v.m_pData), m_cLength(v.m_cLength), m_cCapacity(v.m_cCapacity) {
            v.m_pData = nullptr;
            v.m_cLength = 0;
            v.m_cCapacity = 0;
        }
        // Vector::= operator
        template<typename T>
        inline auto EzVector<T>::operator=(EzVector<T>&& v) noexcept -> EzVector<T>& {
            std::swap(v.m_pData, m_pData);
            std::swap(v.m_cLength, m_cLength);
            std::swap(v.m_cCapacity, m_cCapacity);
            return *this;
        }
        // Vector::reserve
        template<typename T> void EzVector<T>::reserve(uint32_t len) noexcept {
            if (len <= this->capacity()) return;
            auto nice_len = this->nice_length(len);
            auto data = this->alloc(nice_len);
            this->copy_data(data, m_pData, this->length());
            this->safe_free();
            if (data) {
                m_pData = data;
                m_cCapacity = nice_len;
            }
            else {
                m_cCapacity = 0;
                m_cLength = 0;
            }
        }
        // Vector::erase
        template<typename T> auto EzVector<T>::erase(uint32_t pos, uint32_t len) noexcept {
            assert(pos < this->length() && pos + len < this->length() && "out of range");
            if (!this->isok()) return;
            m_cLength -= len;
            for (auto i = pos; i < pos + len; ++i) {
                m_pData[i] = m_pData[i + len];
            }
        }
        // Vector::do_insert
        template<typename T> void EzVector<T>::do_insert(uint32_t pos, const T& data) noexcept {
            auto old = m_cLength;
            assert(pos <= old && "out of range");
            ++m_cLength; this->reserve(this->length());
            if (this->isok()) {
                if (pos != old) {
                    for (auto i = old; i != pos; --i) {
                        m_pDdata[i + 1] = m_pDdata[i];
                    }
                }
                m_pDdata[pos] = data;
            }
        }
        // Pointer Vector
        template<typename T>
        class PointerVector {
            // using
            using VectorType = EzVector<void*>;
        public:
            // begin
            auto begin() noexcept { return VectorType::Iterator<T>(m_vector.data()); }
            // const begin
            auto begin() const noexcept { return VectorType::Iterator<const T>(m_vector.data()); }
            // end
            auto end() noexcept { return VectorType::Iterator<T>(m_vector.data() + m_vector.length()); }
            // const end
            auto end() const noexcept { return VectorType::Iterator<const T>(m_vector.data() + m_vector.length()); }
        public:
            // isok?
            auto isok() const noexcept { return !!m_pData; }
            // insert
            auto insert(uint32_t pos,const T& data) noexcept { this->do_insert(pos, data); }
            // push back with data
            auto push_back(const T& data) noexcept { this->do_insert(this->length(), data); }
            // push back
            auto push_back() noexcept { this->do_insert(this->length(), T()); }
            // pop back
            auto pop_back() noexcept { assert(m_cLength > 0 && "no element to pop"); --m_cLength; }
            // reset
            auto reset() noexcept { m_cLength = 0; }
            // clear
            auto clear() noexcept { m_cLength = 0; }
            // erase
            auto erase(uint32_t pos) noexcept { return this->erase(pos, 1); }
            // erase with length
            auto erase(uint32_t pos, uint32_t len) noexcept;
            // get data
            auto data() const noexcept { return m_pData; }
            // get length
            auto length() const noexcept { return m_cLength; }
            // get capacity
            auto capacity() const noexcept { return m_cCapacity; }
            // reserve length
            void reserve(uint32_t len) noexcept;
        private:
            // vector data
            VectorType          m_vector;
        };
    }
}