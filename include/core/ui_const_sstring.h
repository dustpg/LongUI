#pragma once

// ui
#include "ui_string_view.h"
#include "ui_core_type.h"

// ui namespace
namespace LongUI {
    // const short string
    class CUIConstShortString {
    public:
        // self type
        //using Self = CUIConstShortString;
        // empty string
        static constexpr const char* EMPTY = "";
    public:
        // ctor
        CUIConstShortString() noexcept {}
        // dtor
        ~CUIConstShortString() noexcept { this->release(); }
        // c-style string
        auto c_str() const noexcept { return m_string; }
        // is empty?
        bool empty() const noexcept { return m_string == EMPTY; }
        // operator with string view
        auto&operator=(U8View view) noexcept { this->set_view(view); return *this; }
    private:
        // set with string view
        void set_view(U8View view) noexcept;
        // release
        void release() noexcept;
    private:
        // data
        const char*         m_string = EMPTY;
    };
}
