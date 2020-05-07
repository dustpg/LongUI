#pragma once
#include <type_traits>
#include <cstdint>
#include <utility>

#include "../util/ui_unimacro.h"

namespace LongUI {
    // BKDR Hash Function
    auto BKDRHash(const char* str) noexcept->uint32_t;
    // BKDR Hash Function
    auto BKDRHash(const char* strbgn, const char* strend) noexcept->uint32_t;
}

namespace LongUI { namespace POD {
    /// <summary>
    /// detail namespace
    /// </summary>
    namespace detail {
        // end symbol
        enum : uintptr_t { end_symbol = 233 };
        // hash cell
#pragma warning(suppress: 4200)
        struct hash_cell { hash_cell* next; const char* str; char data[0]; };
        // hash iterator
        struct hash_iterator { uintptr_t* bucket; hash_cell* cell; void move_next(); };
        // operator == for hash_iterator
        inline bool operator==(const hash_iterator& a, const hash_iterator& b) noexcept { 
            return a.bucket == b.bucket && a.cell == b.cell; 
        }
        // string hash base
        class hash_base {
        protected:
            // ctor
            hash_base(uint32_t sizeof_T) noexcept;
            // dtor
            ~hash_base() noexcept;
            // is ok
            bool is_ok() const noexcept { return !!m_pBaseTable; }
            // clear
            void clear() noexcept;
            // find cell
            auto find(const char*) const noexcept->hash_iterator;
            // find cell with string view
            auto find(const char*, const char*) const noexcept->hash_iterator;
            // insert
            auto insert(const char*, const char*) noexcept-> std::pair<hash_iterator, bool>;
            // insert
            auto insert(const char*, const char*, const char*) noexcept->std::pair<hash_iterator, bool>;
#if 0
            // remove
            bool remove(const char*) noexcept;
            // remove
            bool remove(const char*, const char*) noexcept;
            // remove
            bool remove(hash_iterator) noexcept;
#endif
#ifndef UI_HASH_TABLE_NO_ITERATOR
            // begin iterator
            auto begin_itr() const noexcept -> hash_iterator { return m_itrFirst; }
            // end iterator
            auto end_itr() const noexcept -> hash_iterator { return{ m_pBaseTableEnd, nullptr }; }
#endif
            // force insert cell
            auto force_insert(hash_cell& cell) noexcept ->uintptr_t*;
        private:
            // malloc data
            static void*malloc(size_t) noexcept;
            // free data
            static void free(void* ptr) noexcept;
            // force insert
            auto force_insert(const char*, const char*, const char*) noexcept->hash_iterator;
            // growup
            void growup() noexcept;
            // alloc cell
            auto alloc_cell(const char*, const char*, const char*) noexcept->hash_cell*;
            // for each
            template<typename T> void for_each(T call) noexcept;
        protected:
            // base table
            uintptr_t*          m_pBaseTable = nullptr;
            // base table end
            uintptr_t*          m_pBaseTableEnd = nullptr;
#ifndef UI_HASH_TABLE_NO_ITERATOR
            // first item
            hash_iterator       m_itrFirst = this->end_itr();
#endif
            // item size
            uint32_t            m_cItemSize = 0;
            // item byte size
            uint32_t    const   m_cItemByteSize;
        };
    }
    // string hash table
    template<typename T>
    class HashMap : protected detail::hash_base {
    public:
        // key type
        using key_type = const char*;
        // mapped type
        using mapped_type = T;
        // value type
        using value_type = std::pair<const key_type, mapped_type>;
        // size type
        using size_type = uint32_t;
#ifndef UI_HASH_TABLE_NO_ITERATOR
        // hash iterator
        class iterator {
            // self type
            using self = iterator;
            // friend class
            friend HashMap;
        public:
            // copy ctor
            iterator(const iterator& x) noexcept : m_itr(x.m_itr) {  }
            // copy =
            self& operator=(const iterator& x) noexcept { m_itr = x.m_itr; return *this; }
        private:
            // itr ctor
            iterator(const detail::hash_iterator& x) noexcept : m_itr(x) {}
        public:
            // operator *
            auto operator*() const noexcept ->value_type& { return reinterpret_cast<value_type&>(m_itr.cell->str); }
            // operator ->
            auto operator->() const noexcept ->value_type* { return &(*(*this)); }
            // operator ==
            bool operator==(const iterator& x) const noexcept { return m_itr == x.m_itr; }
            // operator !=
            bool operator!=(const iterator& x) const noexcept { return !(m_itr == x.m_itr); }
            // ++operator
            self&operator++() noexcept { m_itr.move_next(); return *this; }
            // operator++
            self operator++(int) noexcept { auto itr = *this; m_itr.move_next(); return itr; }
        private:
            // iterator
            detail::hash_iterator   m_itr;
        };
#endif
    public:
        // check for pod
        static_assert(std::is_pod<T>::value, "type T must be POD type");
#ifndef UI_HASH_TABLE_NO_ITERATOR
        // begin itr
        auto begin() noexcept ->iterator { return{ hash_base::begin_itr() }; }
        // end itr
        auto end() noexcept ->iterator { return{ hash_base::end_itr() }; }
#endif
    public:
        // ctor
        HashMap() noexcept : detail::hash_base(sizeof(T)) {}
        // dtor
        ~HashMap() noexcept = default;
        // copy : not implement yet
        HashMap(const HashMap&) noexcept = delete;
        // move : not implement yet
        HashMap(HashMap&&) noexcept = delete;
#ifndef UI_HASH_TABLE_NO_ITERATOR
        // find item with c-style string
        auto find(const char* str) noexcept ->iterator { return{ hash_base::find(str) }; }
        // find item with string view
        auto find(const char* str, const char* end) noexcept ->iterator { return{ hash_base::find(str, end) }; }
#if 0
        // remove
        bool remove(iterator itr) noexcept { return hash_base::remove(itr.m_itr); }
#endif
#else
#error not implement yet
#endif
        // get size
        auto size() const noexcept ->size_type { return m_cItemSize; }
        // insert with <c-style string, value> pair
        auto insert(const value_type& v) noexcept -> std::pair<iterator, bool> {
            const auto re = hash_base::insert(v.first, reinterpret_cast<const char*>(&v.second));
            return *reinterpret_cast<const std::pair<iterator, bool>*>(&re);
            auto ptr = reinterpret_cast<const std::pair<iterator, bool>*>(&re); return *ptr;
        }
        // insert with <string view, value>
        auto insert(const char* str, const char* end, const T& value) noexcept -> std::pair<iterator, bool> {
            const auto re = hash_base::insert(str, end, reinterpret_cast<const char*>(&value));
            auto ptr = reinterpret_cast<const std::pair<iterator, bool>*>(&re); return *ptr;
        }
    };
}}