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
        // Easy Vector
        template<typename T> class EzVector {
        public:
            // iterator
            template<typename TT=T> struct Iterator {
                // on no
                static_assert(sizeof(TT) == sizeof(T), "bad action");
                // IntType
                using IntType = ptrdiff_t;
            public: // traits
                // random access support
                using iterator_category = std::random_access_iterator_tag;
                // reference
                using reference = T&;
                // pointer
                using pointer = T*;
                // value_type
                using value_type = T;
                // difference_type
                using difference_type = ptrdiff_t;
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
                auto operator==(const Iterator<TT>& itr) const noexcept { return this->data == itr.data; }
                // operator !=
                auto operator!=(const Iterator<TT>& itr) const noexcept { return this->data != itr.data; }
                // operator <
                auto operator<(const Iterator<TT>& itr) const noexcept { return this->data < itr.data; }
                // operator >
                auto operator>(const Iterator<TT>& itr) const noexcept { return this->data > itr.data; }
                // operator <=
                auto operator<=(const Iterator<TT>& itr) const noexcept { return this->data <= itr.data; }
                // operator >=
                auto operator>=(const Iterator<TT>& itr) const noexcept { return this->data >= itr.data; }
                // operator =
                auto& operator=(const Iterator<TT>& itr) noexcept { data = itr.data; return *this; }
                // random acc +=
                auto&operator+=(IntType i) noexcept { data += i; return *this; }
                // random acc -=
                auto&operator-=(IntType i) noexcept { data -= i; return *this; }
                // random acc +
                auto operator+(IntType i) const noexcept { Iterator itr(this->data); itr += i; return itr; }
                // random acc -
                auto operator-(IntType i) const noexcept { Iterator itr(this->data); itr -= i; return itr; }
                // distance
                auto operator-(const Iterator<TT>& itr) const noexcept { return this->data - itr.data; }
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
            // end
            auto end() noexcept { return Iterator<T>(m_pData + m_cLength); }
            // const begin
            auto begin() const noexcept { return Iterator<const T>(m_pData); }
            // const end
            auto end() const noexcept { return Iterator<const T>(m_pData + m_cLength); }
            // const begin
            auto cbegin() const noexcept { return Iterator<const T>(m_pData); }
            // const end
            auto cend() const noexcept { return Iterator<const T>(m_pData + m_cLength); }
        public:
            // isok?
            auto isok() const noexcept { return !!m_pData; }
            // front
            auto&front() const noexcept { assert(m_cLength && "no elements"); return m_pData[0]; }
            // back
            auto&back() const noexcept { assert(m_cLength && "no elements"); return m_pData[m_cLength - 1]; }
            // insert
            auto insert(uint32_t pos,const T& data) noexcept { this->do_insert(pos, data); }
            // insert
            template<typename TT>
            auto insert(const Iterator<TT>& itr,const T& data) noexcept { this->do_insert(uint32_t(&(*itr)-m_pData), data); }
            // push back with data
            auto push_back(const T& data) noexcept { this->do_insert(this->size(), data); }
            // push back
            auto push_back() noexcept { this->do_insert(this->size(), T()); }
            // pop back
            auto pop_back() noexcept { assert(m_cLength > 0 && "no element to pop"); --m_cLength; }
            // clear
            auto clear() noexcept { m_cLength = 0; }
            // empty
            auto empty() const noexcept { return !m_cLength; }
            // erase
            auto erase(uint32_t pos) noexcept { return this->erase(pos, 1); }
            // erase with length
            auto erase(uint32_t pos, uint32_t len) noexcept;
            // erase
            template<typename TT>
            auto erase(const Iterator<TT>& itr) noexcept { return this->erase(uint32_t(&(*itr)-m_pData), 1); }
            // erase with length
            template<typename TT>
            auto erase(const Iterator<TT>& itrben, const Iterator<TT>& itrend) noexcept { 
                auto a = uint32_t(&(*itrben) - m_pData);
                auto b = uint32_t(itrend - itrben);
                return this->erase(a, b);
            }
            // get data
            auto data() const noexcept { return m_pData; }
            // get length
            auto size() const noexcept { return m_cLength; }
            // get capacity
            auto capacity() const noexcept { return m_cCapacity; }
            // reserve length
            void reserve(uint32_t len) noexcept;
            // operator[]
            auto operator[](uint32_t index) noexcept -> T& { assert(index < this->size() && "out of range"); return m_pData[index]; }
            // operator[] const
            auto operator[](uint32_t index) const noexcept ->const T& { assert(index < this->size() && "out of range"); return m_pData[index]; }
        private:
            // data
            T*          m_pData = nullptr;
            // length
            uint32_t    m_cLength = 0;
            // capacity
            uint32_t    m_cCapacity = 0;
        private:
            // safe free
            inline auto safe_free() noexcept { if (m_pData) ::free(m_pData); m_pData = nullptr; }
            // alloc
            static inline auto alloc(uint32_t len) noexcept { return reinterpret_cast<T*>(::malloc(len * sizeof(T))); }
            // copy data
            static inline auto copy_data(T* des, T* src, uint32_t len) noexcept {  if (des && len) ::memcpy(des, src, sizeof(T) * len); }
            // nice length
            static inline auto nice_length(uint32_t len) noexcept { return (len + (len + 9) / 2) & (~3); };
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
            this->copy_data(data, m_pData, this->size());
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
            assert(pos < this->size() && pos + len <= this->size() && "out of range");
            if (!(len && this->isok())) return;
            register auto endofthis = this->size() - len;
            m_cLength -= len;
            for (auto i = pos; i < endofthis; ++i) {
                m_pData[i] = m_pData[i + len];
            }
        }
        // Vector::do_insert
        template<typename T> void EzVector<T>::do_insert(uint32_t pos, const T& data) noexcept {
            auto old = m_cLength;
            assert(pos <= old && "out of range");
            this->reserve(this->size() + 1);
            if (this->isok()) {
                ++m_cLength;
                if (pos != old) {
                    for (auto i = old; i != pos; --i) {
                        m_pData[i] = m_pData[i-1];
                    }
                }
                m_pData[pos] = data;
            }
        }
        // Pointer Vector
        template<typename T>
        class PointerVector {
            // using 
            using P = T*;
            // using
            using VectorType = EzVector<void*>;
            // using
            static auto TPP(void** data) noexcept { return reinterpret_cast<T**>(data); }
        public:
            // begin
            auto begin() noexcept { return VectorType::Iterator<T*>(TPP(m_vector.data())); }
            // end
            auto end() noexcept { return VectorType::Iterator<T*>(TPP(m_vector.data() + m_vector.size())); }
            // const begin
            auto begin() const noexcept { return VectorType::Iterator<const P>(TPP(m_vector.data())); }
            // const end
            auto end() const noexcept { return VectorType::Iterator<const P>(TPP(m_vector.data() + m_vector.size())); }
            // const begin
            auto cbegin() const noexcept { return VectorType::Iterator<const P>(TPP(m_vector.data())); }
            // const end
            auto cend() const noexcept { return VectorType::Iterator<const P>(TPP(m_vector.data() + m_vector.size())); }
        public:
            // isok?
            auto isok() const noexcept { return m_vector.isok(); }
            // front
            auto&front() const noexcept { return reinterpret_cast<T*&>(m_vector.front()); }
            // back
            auto&back() const noexcept {  return reinterpret_cast<T*&>(m_vector.back()); }
            // insert
            auto insert(uint32_t pos, T* dat) noexcept { return m_vector.insert(pos, dat); }
            // insert
            template<typename TT>
            auto insert(const VectorType::Iterator<TT>& itr, T* dat) noexcept { return m_vector.insert(uint32_t(&(*itr) - this->data()), dat); }
            // push back with data
            auto push_back(T* data) noexcept { return m_vector.push_back(data); }
            // push back
            auto push_back() noexcept { return m_vector.push_back(nullptr); }
            // pop back
            auto pop_back() noexcept { return m_vector.pop_back(); }
            // clear
            auto clear() noexcept { return m_vector.clear(); }
            // erase
            auto erase(uint32_t pos) noexcept { return m_vector.erase(pos); }
            // erase with length
            auto erase(uint32_t pos, uint32_t len) noexcept { return m_vector.erase(pos, len); }
            // erase
            template<typename TT>
            auto erase(const VectorType::Iterator<TT>& itr) noexcept { return this->erase(uint32_t(&(*itr)-this->data()), 1); }
            // erase with length
            template<typename TT>
            auto erase(const VectorType::Iterator<TT>& itrben, const VectorType::Iterator<TT>& itrend) noexcept { 
                auto a = uint32_t(&(*itrben) - this->data());
                auto b = uint32_t(itrend - itrben);
                return this->erase(a, b);
            }
            // get data
            auto data() const noexcept { return reinterpret_cast<T**>(m_vector.data()); }
            // get length
            auto size() const noexcept { return m_vector.size(); }
            // empty
            auto empty() const noexcept { return m_vector.empty(); }
            // get capacity
            auto capacity() const noexcept { return m_vector.capacity(); }
            // reserve length
            auto reserve(uint32_t len) noexcept { return m_vector.reserve(len); }
            // operator[]
            auto operator[](uint32_t index) noexcept ->P& { return reinterpret_cast<P&>(m_vector[index]); }
            // operator[] const
            auto operator[](uint32_t index) const noexcept ->const P& { return reinterpret_cast<const P&>(m_vector[index]); }
        private:
            // vector data
            VectorType          m_vector;
        };
    }
    // control vector
    using ControlVector = EzContainer::PointerVector<UIControl>;
    // Context Buffer
    using ContextBuffer = EzContainer::SmallBuffer<char, 4 * 4>;
}