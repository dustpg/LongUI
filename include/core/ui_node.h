#pragma once

#include <cstdint>
#include <iterator>

// Node
namespace LongUI {
    // Node
    template<typename T> struct Node {
        // node ptr
        T* prev, *next;
        // operator ==
        bool operator==(const Node& n) const noexcept { return this == &n; }
        // operator !=
        bool operator!=(const Node& n) const noexcept { return this != &n; }
        // control iterator
        class Iterator {
            // self type
            using Self = Iterator;
        public:
            // c++ iterator traits
            using iterator_category = std::bidirectional_iterator_tag;
            // c++ iterator traits
            using difference_type = std::ptrdiff_t;
            // c++ iterator traits
            using value_type = T;
            // c++ iterator traits
            using reference = T&;
            // c++ iterator traits
            using pointer = T*;
        public:
            // ctor
            Iterator(T* c) noexcept : m_pTNode(c) {}
            // cpy ctor
            Iterator(const Self& itr) noexcept : m_pTNode(itr.m_pTNode) { }
            // operator =
            auto operator=(const Self& itr) noexcept ->Self& { m_pTNode = itr.m_pTNode; return *this; }
            //  ++itr
            auto operator++() noexcept ->Self { m_pTNode = static_cast<T*>(m_pTNode->next); return *this; }
            // itr++
            auto operator++(int) const noexcept->Self { Self itr{ m_pTNode }; return ++itr; }
            //  --itr
            auto operator--() noexcept ->Self { m_pTNode = static_cast<T*>(m_pTNode->prev); return *this; }
            // itr--
            auto operator--(int) const noexcept ->Self { Self itr{ m_pTNode }; return --itr; }
            // operator ==
            bool operator==(const Self& itr) const noexcept { return m_pTNode == itr.m_pTNode; }
            // operator !=
            bool operator!=(const Self& itr) const noexcept { return m_pTNode != itr.m_pTNode; }
            // operator *
            auto operator*() const noexcept -> T& { return *m_pTNode; }
            // operator *
            operator T*() const noexcept { return m_pTNode; }
        private:
            // node pointer
            T*          m_pTNode;
        };
        // reverse iterator
        class ReverseIterator {
            // self type
            using Self = ReverseIterator;
        public:
            // c++ iterator traits
            using iterator_category = std::bidirectional_iterator_tag;
            // c++ iterator traits
            using difference_type = std::ptrdiff_t;
            // c++ iterator traits
            using value_type = T;
            // c++ iterator traits
            using reference = T&;
            // c++ iterator traits
            using pointer = T*;
        public:
            // ctor
            ReverseIterator(T* c) noexcept : m_pTNode(c) {}
            // cpy ctor
            ReverseIterator(const Self& itr) noexcept : m_pTNode(itr.m_pTNode) { }
            // operator =
            auto operator=(const Self& itr) noexcept ->Self& { m_pTNode = itr.m_pTNode; return *this; }
            //  ++itr
            auto operator++() noexcept ->Self { m_pTNode = static_cast<T*>(m_pTNode->prev); return *this; }
            // itr++
            auto operator++(int) const noexcept ->Self { Self itr{ m_pTNode }; return ++itr; }
            //  --itr
            auto operator--() noexcept ->Self { m_pTNode = static_cast<T*>(m_pTNode->next); return *this; }
            // itr--
            auto operator--(int) const noexcept ->Self { Self itr{ m_pTNode }; return --itr; }
            // operator ==
            bool operator==(const Self& itr) const noexcept { return m_pTNode == itr.m_pTNode; }
            // operator !=
            bool operator!=(const Self& itr) const noexcept { return m_pTNode != itr.m_pTNode; }
            // operator *
            auto operator*() const noexcept -> T& { return *m_pTNode; }
            // operator *
            operator T*() const noexcept { return m_pTNode; }
        private:
            // node pointer
            T*          m_pTNode;
        };
    };
}
