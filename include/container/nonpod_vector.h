#pragma once

#include "../util/ui_ctordtor.h"

#include <utility>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <iterator>
#include <type_traits>


// LongUI::non-pod namespace
namespace LongUI { namespace NonPOD {
    // detail namespace
    namespace detail {
        // vtable_helper
        using LongUI::detail::vtable_helper;
        // ctor_dtor
        using LongUI::detail::ctor_dtor;
        // base vector
        class vector_base {
        public:
            // ctor
            vector_base(const vtable_helper*, uint16_t len, uint16_t ex) noexcept;
            // ctor
            ~vector_base() noexcept { this->free_data(); }
            // copy ctor
            vector_base(const vector_base&) noexcept;
            // move ctor
            vector_base(vector_base&&) noexcept;
            // operator= move
            void op_equal(vector_base&& x) noexcept { free_data();  ctor_dtor<vector_base>::create(this, std::move(x)); }
            // operator= copy
            void op_equal(const vector_base& x) noexcept;
        public:
            // max size
            auto max_size() const noexcept -> uint32_t { return 1u << 20; }
            // size of Vector
            auto size() const noexcept  { return m_uVecLen; }
            // get capacity
            auto capacity() const noexcept { return m_uVecCap; }
            // is_ok?
            bool is_ok() const noexcept { return !!m_uVecCap; }
            // is empty?
            bool empty() const noexcept { return !m_uVecLen; }
            // data
            auto data_c() const noexcept -> const char* { return m_pData; }
            // data
            auto data_n() noexcept { return m_pData; }
            // end pointer
            auto end_c() const noexcept -> const char*;
            // end pointer
            auto end_n() noexcept { return const_cast<char*>(end_c()); }
            // last pointer
            auto last_c() const noexcept -> const char*;
            // last pointer
            auto last_n() noexcept { return const_cast<char*>(last_c()); }
        public:
            // free all data
            void free_data() noexcept;
            // shrink
            void shrink_to_fit() noexcept;
            // reserve
            void reserve(uint32_t) noexcept;
            // resize
            void resize(uint32_t) noexcept;
            // push back - help
            void push_back_help(char* obj, void* func) noexcept;
            // push back - copy
            void push_back_copy(const char*) noexcept;
            // push back - move
            void push_back_move(char*) noexcept;
            // emplace back
            char*emplace_back() noexcept;
            // insert - help
            //void insert_help(char* obj, uint32_t pos, uint32_t len, void* func) noexcept;
            // emplace objects
            //char*emplace_objs(uint32_t pos, uint32_t len) noexcept;
            // pop back
            void pop_back() noexcept;
            // erase
            void erase(uint32_t pos, uint32_t len) noexcept;
            // clear
            void clear() noexcept;
            // assign range
            void assign_range(const char*, uint32_t) noexcept;
            // assign count
            void assign_count(const char*, uint32_t) noexcept;
        private:
            // check aligned
            inline void check_aligned(const char* ptr) noexcept;
            // free
            static inline void free(char*) noexcept;
            // malloc
            static inline char*malloc(size_t) noexcept;
            // realloc
            static inline char*realloc(char*, size_t) noexcept;
            // realloc
            static inline char*try_realloc(char*, size_t) noexcept;
            // do objects
            static void do_objects(char* obj, void* func, uint32_t count, uint32_t blen) noexcept;
            // do objects with objects
            static void do_objobj(char* obj, const char* obj2, void* func, uint32_t count, uint32_t blen) noexcept;
            // release objs
            void release_objs(char*, uint32_t count) noexcept;
            // create objects
            void create_objs(char*, uint32_t count) noexcept;
            // move objs
            void move_objects(char*, char*, uint32_t count) noexcept;
            // copy objs
            void copy_objects(char*, const char*, uint32_t count) noexcept;
        protected:
            // vtable
            const vtable_helper* const  m_pVTable;
            // data pointer
            char*                       m_pData = nullptr;
            // Vector length
            uint32_t                    m_uVecLen = 0;
            // Vector capacity
            uint32_t                    m_uVecCap = 0;
            // data byte size
            const uint16_t              m_uByteLen;
            // data aligned size
            const uint16_t              m_uAligned;
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
    // vector
    template<typename T> class Vector {
        // trans
        static auto tr(T* ptr) noexcept { return reinterpret_cast<char*>(ptr); }
        // trans
        static auto tr(char* ptr) noexcept { return reinterpret_cast<T*>(ptr); }
        // trans
        static auto tr(const T* ptr) noexcept { return reinterpret_cast<const char*>(ptr); }
        // trans
        static auto tr(const char* ptr) noexcept { return reinterpret_cast<const T*>(ptr); }
    public:
        // iterator
        using iterator = detail::base_iterator<Vector<T>, T*>;
        // const iterator
        using const_iterator = const detail::base_iterator<const Vector<T>, const T*>;
        // check for pod
        static_assert(!std::is_pod<T>::value, "type T must NOT be POD type");
    public:
        // ctor
        Vector() noexcept : m_vector(LongUI::detail::ctor_dtor<T>::get(), sizeof(T),alignof(T)){};
        // copy ctor
        Vector(const Vector& x) noexcept : m_vector(x.m_vector) {}
        // move ctor
        Vector(Vector&& x) noexcept : m_vector(std::move(x.m_vector)) {}
        // dtor
        ~Vector() noexcept {};
        // operator = copy
        auto&operator=(const Vector<T>& x) noexcept { m_vector.op_equal(x.m_vector); return *this; }
        // operator = move
        auto&operator=(Vector<T>&& x) noexcept { m_vector.op_equal(std::move(x.m_vector)); return *this; }
        // operator[]
        auto&operator[](uint32_t i) noexcept { assert(i < size() && "out of range"); return tr(m_vector.data_n())[i]; }
        // operator[]
        auto&operator[](uint32_t i) const noexcept { assert(i < size() && "out of range"); return tr(m_vector.data_c())[i]; }
    public:
        // data
        auto data() const noexcept { return tr(m_vector.data_c()); }
        // data
        auto data() noexcept { return tr(m_vector.data_n()); }
        // size
        auto size() noexcept { return m_vector.size(); }
        // is ok?
        bool is_ok() const noexcept { return m_vector.is_ok(); }
        // is empty?
        bool empty() const noexcept { return m_vector.empty(); }
    public:
        // push back: 
        void push_back(const T& x) noexcept { m_vector.push_back_copy(tr(&x)); }
        // [Not recommended for use] push back: use this -> emplace back
        void push_back(T&& x) noexcept { m_vector.push_back_move(tr(&x)); }
        // [    recommended for use] emplace back without args, return null for OOM
        auto emplace_back_ex() noexcept -> T* { return tr(m_vector.emplace_back()); }
        // pop back
        void pop_back() noexcept { m_vector.pop_back(); }
        // reserve
        void reserve(uint32_t len) noexcept { m_vector.reserve(len); }
        // resize
        void resize(uint32_t len) noexcept { m_vector.resize(len); }
        // clear
        void clear() noexcept { m_vector.clear(); }
        // erase
        void erase(uint32_t pos) noexcept { m_vector.erase(pos, 1); }
        // erase
        void erase(uint32_t pos, uint32_t len) noexcept { m_vector.erase(pos, len); }
        // assign count
        void assign(uint32_t count, const T& data) noexcept { m_vector.assign_count(tr(&data), count); }
        // assign range
        void assign(const T* first, uint32_t length) noexcept { m_vector.assign_range(tr(first), length); }
        // emplace, return null for OOM
        //auto emplace(uint32_t pos) noexcept { return tr(m_vector.emplace_objs(pos, 1)); }
        // emplace for many, return null for OOM
        //auto emplace_ex(uint32_t pos, uint32_t n) noexcept { return tr(m_vector.emplace_objs(pos, n)); }
    protected:
        // detail implment
        detail::vector_base         m_vector;
    public:
        // begin
        auto begin() noexcept ->iterator { return { *this, tr(m_vector.data_n()) }; }
        // end
        auto end() noexcept ->iterator { return { *this, tr(m_vector.end_n()) }; }
    };
}}
