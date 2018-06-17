#pragma once
#include <cstdint>
#include "../util/ui_unimacro.h"


namespace UI {
    // COM Pointer Base
    namespace detail {
        // com object
        class object;
        // com ptr base
        class com_ptr_base {
            // self type
            using self = com_ptr_base;
        public:
            // ctor: add ref-count
            com_ptr_base(object&) noexcept;
            // ctor: not add ref-count
            com_ptr_base(object&&) noexcept;
            // dtor: release ref-count if vaild
            ~com_ptr_base() noexcept;
            // move ctor
            com_ptr_base(self&& x) noexcept;
            // copy ctor
            com_ptr_base(const self& x) noexcept;
            // move assign
            self&operator=(self&& x) noexcept;
            // copy assign
            self&operator=(const self& x) noexcept;
        private:
            // pointer data
            void*               m_pointer;
        };
        // ctor: not add ref-count
        inline com_ptr_base::com_ptr_base(object&& obj) noexcept 
            : m_pointer(&obj) { }
        // move ctor for com_ptr_base
        inline com_ptr_base::com_ptr_base(self && x) noexcept
            : m_pointer(x.m_pointer) { x.m_pointer = nullptr; }
    }
}