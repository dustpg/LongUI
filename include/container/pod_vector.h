#pragma once
#include <type_traits>
#include <iterator>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <new>

#include "../luiconf.h"
#include "../typecheck/is_iterator.h"
#include "../typecheck/int_by_size.h"
#include "../util/ui_unimacro.h"
#include "../util/ui_ctordtor.h"

namespace LongUI {
    // string class
    template<typename TT, unsigned BB> class CUIBasicString;
    // detail::string_helper namespace
    namespace detail { struct string_helper; }
    // nullptr must be 0
    static_assert(nullptr == 0, "not supported if nullptr != 0");
    // sizeof boolean must be 1
    //static_assert(sizeof(bool) == 1, "not supported if sizeof(bool) != 1");
    // pointer vector
    struct PointerVector;
    // ensure remove pointer item
    void RemovePointerItem(PointerVector&, void *) noexcept;
}

namespace LongUI { namespace POD {
    /// <summary>
    /// detail namespace
    /// </summary>
    namespace detail {
        // log2 helper
        template <size_t x>
        struct log2 { enum : size_t { value = 1 + log2<x / 2>::value }; };
        // log2 helper
        template <> struct log2<1> { enum : size_t { value = 0 }; };
        // using
        using LongUI::detail::ctor_dtor;
        /// <summary>
        /// Vector base class
        /// </summary>
        class vector_base {
            // friend
            friend LongUI::detail::string_helper;
            // friend
            friend ctor_dtor<vector_base>;
            // extra bit
            enum {
                // extra aligned size(max for 128 byte in 32bit...)
                EX_ALIGNED1 = 0, EX_ALIGNED2 = 4,
                // extra reservation length(+1s)
                EX_RESERVED1 = 4, EX_RESERVED2 = 8,
                // extra fixed buffer length
                EX_FBL1 = 8,  EX_FBL2 = 16,
            };
        public:
#ifdef LUI_VECTOR_SIZE_USE_UINT32
            // size type
            using size_type = uint32_t;
#else
            // size type
            using size_type = std::uintptr_t;
#endif
            // size of template
            auto size_of_template() const noexcept { return m_uByteLen; }
            // max size
            auto max_size() const noexcept ->size_type { return 1u << 30; }
            // clear data
            void clear() noexcept { m_uVecLen = 0; }
            // pop back
            void pop_back() noexcept { assert(m_uVecLen && "UB: none data"); --m_uVecLen; }
            // size of Vector
            auto size() const noexcept -> size_type { return m_uVecLen; }
            // get capacity
            auto capacity() const noexcept->size_type { return m_uVecCap; }
            // check
            bool check() const noexcept { return !!m_uVecCap; }
            // is_ok?
            bool is_ok() const noexcept { return !!m_uVecCap; }
            // is empty?
            bool empty() const noexcept { return !m_uVecLen; }
            // reserve space
            void reserve(size_type n) noexcept;
        public:
            // extra helper
            template<size_t ali, size_t res, size_t fbl>
            struct extra_t {
                static_assert((ali & (ali - 1)) == 0, "must aligned in power of 2");
                static_assert(ali != 0, "must not be 0");
                static_assert(ali < (1<<(1<<(EX_RESERVED2- EX_RESERVED1))), "must less than it");
                static_assert(res < 16, "must less than 16");
                static_assert(fbl < 256, "must less than 256");
                static_assert(res <= fbl, "must less or eq than fbl");
                enum : size_t { value = (fbl<< EX_FBL1) | (res << EX_RESERVED1) | (log2<ali>::value <<  EX_ALIGNED1) };
            };
        protected:
            // ctor
            template<typename EX>
            vector_base(size_type bl, EX) noexcept
                :vector_base(bl, static_cast<uhalfptr_t>(EX::value)) {}
            // ctor
            vector_base(size_type bl) noexcept
                : vector_base(bl, extra_t<sizeof(double), 0, 0>{}) { }
        protected:
            // dtor
            ~vector_base() noexcept { this->free_memory(); }
            // copy ctor
            vector_base(const vector_base&) noexcept;
            // move ctor
            vector_base(vector_base&&) noexcept;
            // ctor
            vector_base(size_type bl, uhalfptr_t ex) noexcept;
            // resize
            void resize(size_type n, const char* data) noexcept;
            // shrink resize
            void shrink_resize(size_type n) noexcept;
            // erase at pos range
            void erase(size_type start, size_type end) noexcept;
            // insert range
            void insert(size_type pos, const char* first, const char* last) noexcept;
            // insert pos
            void insert(size_type pos, size_type n, const char* data) noexcept;
            // assign data
            void assign(const char* first, const char* last) noexcept;
            // assign data n times
            void assign(size_type n, const char* data) noexcept;
            // push back
            void push_back(const char*) noexcept;
            // push back pointer
            void push_back_ptr(const char*) noexcept;
            // swap another object
            void swap(vector_base& x) noexcept;
            // begin pointer
            auto begin() noexcept ->void* { return m_pData; }
            // end pointer
            auto end() noexcept ->void* { return const_cast<void*>(static_cast<const vector_base&>(*this).end()); }
            // const begin pointer
            auto begin() const noexcept ->const void* { return m_pData; }
            // const end pointer
            auto end() const noexcept ->const void*;
            // operator= move
            void op_equal(vector_base&& x) noexcept { free_memory(); ctor_dtor<vector_base>::create(this, std::move(x)); }
            // operator= copy
            void op_equal(const vector_base& x) noexcept;
            // fit size
            void shrink_to_fit() noexcept;
        private:
            // force_reset
            void force_reset() noexcept;
        private:
            // free memory
            static void free(char*) noexcept;
            // alloc memory
            static auto malloc(size_type len) noexcept -> char*;
            // realloc memory
            static auto realloc(char*, size_type len) noexcept -> char*;
            // is valid heap data
            bool is_valid_heap(void* ptr) const noexcept { return ptr != (this + 1); }
            // invalid heap data
            auto invalid_heap() const noexcept ->char* { return (char*)(this + 1); }
            // free memory
            void free_memory() noexcept;
            // alloc memory
            void alloc_memory(size_type len) noexcept;
            // insert helper
            bool insert_helper(size_type pos, size_type n) noexcept;
        private:
            // get aligned size
            auto get_extra_ali() const noexcept ->uhalfptr_t { 
                return 1 << ((m_uExtra >> EX_ALIGNED1) & ((1 << (EX_ALIGNED2 - EX_ALIGNED1)) - 1));
            }
            // get extra buy
            auto get_extra_buy() const noexcept ->uhalfptr_t { 
                return (m_uExtra >> EX_RESERVED1) & ((1 << (EX_RESERVED2 - EX_RESERVED1)) - 1);
            }
            // get extra fixed length
            auto get_extra_fbl() const noexcept->uhalfptr_t { 
                return (m_uExtra >> EX_FBL1) & ((1 << (EX_FBL2 - EX_FBL1)) - 1);
            }
        private:
            // data ptr
            char*               m_pData = invalid_heap();
            // Vector length
            uint32_t            m_uVecLen = 0;
            // Vector capacity
            uint32_t            m_uVecCap = 0;
            // data byte size
            const uhalfptr_t    m_uByteLen;
            // extra data
            const uhalfptr_t    m_uExtra;
        };
        // base iterator
        template<typename V, typename Ptr> class base_iterator {
            // self type
            using self = base_iterator;
        public:
            // c++ iterator traits
            using iterator_category = std::random_access_iterator_tag;
            // c++ iterator traits
            using difference_type = std::ptrdiff_t;
            // c++ iterator traits
            using pointer = Ptr;
            // c++ iterator traits
            using value_type = typename std::remove_pointer<pointer>::type;
            // c++ iterator traits
            using reference = typename std::add_lvalue_reference<value_type>::type;
#ifndef NDEBUG
        private:
            // data debug
            void data_dbg() const noexcept { assert(m_data == m_vector.data() && "invalid iterator"); }
            // range debug
            void range_dbg() const noexcept { assert((*this) >= m_vector.begin() && (*this) <= m_vector.end() && "out of range"); }
            // range debug
            void range_dbg_ex() const noexcept { assert((*this) >= m_vector.begin() && (*this) < m_vector.end() && "out of range"); }
            // debug check
            void check_dbg() const noexcept { data_dbg(); range_dbg(); }
            // debug check
            void check_dbg_ex() const noexcept { data_dbg(); range_dbg_ex(); }
        public:
            // def dtor
            base_iterator() noexcept : m_vector(*(V*)(nullptr)), m_ptr(nullptr), m_data(nullptr) {};
            // nor ctor
            base_iterator(V& v, Ptr ptr) noexcept : m_vector(v), m_ptr(ptr), m_data(v.data()) {};
            // copy ctor
            base_iterator(const self& x) noexcept = default;
            // ++operator
            auto operator++() noexcept -> self& { ++m_ptr; check_dbg(); return *this; }
            // --operator
            auto operator--() noexcept -> self& { --m_ptr; check_dbg(); return *this; }
            // operator*
            auto operator*() const noexcept -> reference { check_dbg_ex(); return *m_ptr; }
            // operator->
            auto operator->() const noexcept -> pointer { check_dbg_ex(); return m_ptr; }
            // operator +=
            auto operator+=(difference_type i) noexcept ->self& { m_ptr += i; check_dbg(); return *this; }
            // operator -=
            auto operator-=(difference_type i) noexcept ->self& { m_ptr -= i; check_dbg(); return *this; }
            // operator[]
            auto operator[](difference_type i) noexcept ->reference { auto itr = *this; itr += i; return *itr.m_ptr; }
#else
        public:
            // def dtor
            base_iterator() noexcept : m_ptr(nullptr) {};
            // nor ctor
            base_iterator(V& v, Ptr ptr) noexcept : m_ptr(ptr) {};
            // copy ctor
            base_iterator(const self& x) noexcept = default;
            // ++operator
            auto operator++() noexcept -> self& { ++m_ptr; return *this; }
            // --operator
            auto operator--() noexcept -> self& { --m_ptr; return *this; }
            // operator*
            auto operator*() const noexcept ->reference { return *m_ptr; }
            // operator->
            auto operator->() const noexcept ->pointer { return m_ptr; }
            // operator +=
            auto operator+=(difference_type i) noexcept ->self& { m_ptr += i; return *this; }
            // operator -=
            auto operator-=(difference_type i) noexcept ->self& { m_ptr -= i; return *this; }
            // operator[]
            auto operator[](difference_type i) noexcept ->reference { return m_ptr[i]; }
#endif
            // operator=
            auto operator= (const self& x) noexcept ->self& { std::memcpy(this, &x, sizeof x); return *this; }
            // operator<
            bool operator< (const self& x) const noexcept { return m_ptr <  x.m_ptr; }
            // operator>
            bool operator> (const self& x) const noexcept { return m_ptr >  x.m_ptr; }
            // operator==
            bool operator==(const self& x) const noexcept { return m_ptr == x.m_ptr; }
            // operator!=
            bool operator!=(const self& x) const noexcept { return m_ptr != x.m_ptr; }
            // operator<=
            bool operator<=(const self& x) const noexcept { return m_ptr <= x.m_ptr; }
            // operator>=
            bool operator>=(const self& x) const noexcept { return m_ptr >= x.m_ptr; }
            // operator-
            auto operator-(const self& x) const noexcept ->difference_type { return m_ptr - x.m_ptr; }
            // operator +
            auto operator+(difference_type i) const noexcept ->self { self itr{ *this }; itr += i; return itr; }
            // operator -=
            auto operator-(difference_type i) const noexcept ->self { self itr{ *this }; itr -= i; return itr; }
            // operator++
            auto operator++(int) noexcept -> self { self itr{ *this }; ++(*this); return itr; }
            // operator--
            auto operator--(int) noexcept -> self { self itr{ *this }; ++(*this); return itr; }
        private:
#ifndef NDEBUG
            // Vector data
            V&                  m_vector;
            // data ptr
            const void*         m_data;
#endif
            // pointer
            pointer             m_ptr;
        };
    }
    // Vector
    template<typename T> class Vector : protected detail::vector_base {
        // type helper
        static inline auto tr(T* ptr) noexcept -> char* { return reinterpret_cast<char*>(ptr); }
        // type helper
        static inline auto tr(const T* ptr) noexcept ->const char* { return reinterpret_cast<const char*>(ptr); }
    public:
        // size type
        using size_type = vector_base::size_type;
        // size of template
        auto size_of_template() const noexcept { return vector_base::size_of_template();  }
        // max size
        auto max_size() const noexcept ->size_type { return vector_base::max_size(); }
        // clear data
        void clear() noexcept { return vector_base::clear(); }
        // pop back
        void pop_back() noexcept { return vector_base::pop_back(); }
        // size of Vector
        auto size() const noexcept -> size_type { return vector_base::size(); }
        // get capacity
        auto capacity() const noexcept->size_type { return vector_base::capacity(); }
        // check
        bool check() const noexcept { return vector_base::check(); }
        // is_ok?
        bool is_ok() const noexcept { return vector_base::is_ok(); }
        // is empty?
        bool empty() const noexcept { return vector_base::empty(); }
        // reserve space
        void reserve(size_type n) noexcept { return vector_base::reserve(n); }
        // swap
        void swap(Vector<T>& x) noexcept { this->vector_base::swap(x); }
        // operator !
        bool operator!() const noexcept { return !this->is_ok(); }
        // operator bool
        operator bool() const noexcept { return this->is_ok(); }
    protected:
        // friend
        template<typename TT, unsigned BB> friend class CUIBasicString;
        // buffer ctor
        template<typename EX>
        Vector(size_type /*ali*/, EX ex) noexcept: vector_base(sizeof(T), ex) {}
        // pushback pointer
        template<size_t S> void push_back_helper(const T& x) noexcept {
            vector_base::push_back(tr(&x)); }
        // pushback pointer
        template<> void push_back_helper<sizeof(void*)>(const T& x) noexcept {
            union { T d; const char* ptr; }; d = x;
            vector_base::push_back_ptr(ptr); }
    public:
        // iterator
        using iterator = detail::base_iterator<Vector, T*>;
        // const iterator
        using const_iterator = const detail::base_iterator<const Vector, const T*>;
        // check for pod
        static_assert(std::is_pod<T>::value, "type T must be POD type");
        // ctor
        Vector() noexcept : vector_base(sizeof(T)) {}
        // ctor with initializer_list
        Vector(std::initializer_list<T> list) noexcept : vector_base(sizeof(T)) { assign(list); }
        // copy ctor
        Vector(const Vector& v) noexcept : vector_base(v) {}
        // move ctor
        Vector(Vector&& v) noexcept : vector_base(std::move(v)) {}
        // range ctor with random access iterator(Partially compatible with the standard)
        template<typename RAI>
        Vector(RAI frist, RAI last) noexcept : vector_base(sizeof(T)) { assign(frist, last); }
        // fill 0 ctor
        Vector(size_type n) noexcept : Vector(n, T{}) {}
        // fill x ctor
        Vector(size_type n, const T& x) noexcept : vector_base(sizeof(T)) { assign(n, x); }
        // copy op_equal
        auto operator=(const Vector& x)noexcept ->Vector & { vector_base::op_equal(x); return *this; }
        // move op_equal
        auto operator=(Vector&& x) noexcept ->Vector & { vector_base::op_equal(std::move(x)); return *this; }
        // get at
        auto at(size_type pos) noexcept -> T& { assert(pos < size() && "OOR"); return data()[pos]; }
        // get at const
        auto at(size_type pos) const noexcept -> const T&{ assert(pos < size() && "OOR"); return data()[pos]; }
        // operator[] 
        auto operator[](size_type pos) noexcept -> T& { assert(pos < size() && "OOR"); return data()[pos]; }
        // operator[] const
        auto operator[](size_type pos) const noexcept -> const T&{ assert(pos < size() && "OOR"); return data()[pos]; }
    public:
        // assign data
        void assign(size_type n, const T& value) noexcept { vector_base::assign(n, tr(&value)); }
        // assign range with random access iterator(Partially compatible with the standard)
        template<typename RAI> typename std::enable_if<type_helper::is_iterator<RAI>::value, void>::type
            assign(RAI first, RAI last) noexcept {
#ifndef NDEBUG
            if (const auto n = last - first) {
                const auto ptr = &first[0];
                vector_base::assign(tr(ptr), tr(ptr + n));
            }
#else
            vector_base::assign(tr(&first[0]), tr(&last[0]));
#endif
        }
        // resize
        void resize(size_type n) noexcept { resize(n, T{}); }
        // resize with filled-value
        void resize(size_type n, const T& x) noexcept { vector_base::resize(n, tr(&x)); }
        // shrink resize
        void shrink_resize(size_type n) noexcept { vector_base::shrink_resize(n); }
        // fit
        void shrink_to_fit() noexcept { vector_base::shrink_to_fit(); }
        // get data ptr
        auto data() noexcept -> T* { return reinterpret_cast<T*>(vector_base::begin()); }
        // get data ptr
        auto data() const noexcept -> const T*{ return reinterpret_cast<const T*>(vector_base::begin()); }
        // assign data
        void assign(std::initializer_list<T> list) noexcept { assign(list.begin(), list.end()); }
        // push back
        void push_back(const T& x) noexcept { this->push_back_helper<sizeof(T)>(x); }
        // begin iterator
        auto begin() noexcept -> iterator { return{ *this, reinterpret_cast<T*>(vector_base::begin()) }; }
        // end iterator
        auto end() noexcept ->iterator { return{ *this, reinterpret_cast<T*>(vector_base::end()) }; }
        // front
        auto front() noexcept ->T& { return *reinterpret_cast<T*>(vector_base::begin()); }
        // back
        auto back() noexcept ->T& { assert(!empty()); return reinterpret_cast<T*>(vector_base::end())[-1]; }
        // begin iterator
        auto begin() const noexcept -> const_iterator { return{ *this, reinterpret_cast<const T*>(vector_base::begin()) }; }
        // end iterator
        auto end() const noexcept ->const_iterator { return{ *this, reinterpret_cast<const T*>(vector_base::end()) }; }
        // const begin iterator
        auto cbegin() const noexcept -> const_iterator { return begin(); }
        // const end iterator
        auto cend() const noexcept ->const_iterator { return end(); }
        // const front
        auto front() const noexcept ->const T&{ return *reinterpret_cast<const T*>(vector_base::begin()); }
        // const back
        auto back() const noexcept ->const T&{ assert(!empty()); return reinterpret_cast<const T*>(vector_base::begin())[-1]; }
    public:
        // insert value
        iterator insert(size_type pos, const T& val) noexcept {
            vector_base::insert(pos, tr(&val), tr(&val + 1));
            return{ *this, reinterpret_cast<T*>(vector_base::begin()) + pos };
        }
        // insert range with random access iterator(Partially compatible with the standard)
        template<typename RAI> typename std::enable_if<type_helper::is_iterator<RAI>::value, iterator>::type
            insert(size_type pos, RAI first, RAI last) noexcept {
#ifndef NDEBUG
            if (const auto n = last - first) {
                const auto ptr = &first[0];
                vector_base::insert(pos, tr(ptr), tr(ptr + n));
            }
#else
            vector_base::insert(pos, tr(&first[0]), tr(&last[0]));
#endif
            return{ *this, reinterpret_cast<T*>(vector_base::begin()) + pos };
        }
        // insert value n times
        iterator insert(size_type pos, size_type n, const T& val) noexcept {
            vector_base::insert(pos, n, tr(&val));
            return{ *this, reinterpret_cast<T*>(vector_base::begin()) + pos };
        }
        // insert value
        iterator insert(iterator itr, const T& val) noexcept { return insert(itr - begin(), val); }
        // insert value n times
        iterator insert(iterator itr, size_type n, const T& val) noexcept { return insert(itr - begin(), n, val); }
        // insert range with random access iterator(Partially compatible with the standard)
        template<typename RAI> typename std::enable_if<type_helper::is_iterator<RAI>::value, iterator>::type
            insert(iterator itr, RAI first, RAI last) noexcept { return insert(itr - begin(), first, last); }
        // insert range
        iterator insert(iterator itr, std::initializer_list<T> list) noexcept { return insert(itr - begin(), list.begin(), list.end()); }
    public:
        // erase at pos 
        iterator erase(size_type pos) noexcept {
            vector_base::erase(pos, pos + 1);
            return{ *this, reinterpret_cast<T*>(vector_base::begin()) + pos };
        }
        // erase range
        iterator erase(size_type first, size_type last) noexcept {
            vector_base::erase(first, last);
            return{ *this, reinterpret_cast<T*>(vector_base::begin()) + first };
        }
        // erase at pos itr
        iterator erase(iterator itr) noexcept { return erase(itr - begin()); }
        // erase at range itr
        iterator erase(iterator first, iterator last) noexcept { return erase(first - begin(), last - begin()); }
        // erase at pos itr
        iterator erase(const_iterator itr) noexcept { return erase(itr - cbegin()); }
        // erase at range itr
        iterator erase(const_iterator first, const_iterator last) noexcept { return erase(first - begin(), last - cbegin()); }
    private:
    };
}}