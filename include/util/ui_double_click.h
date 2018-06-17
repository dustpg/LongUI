#pragma once
#include "ui_time_meter.h"

namespace LongUI {
    // double click helper
    class CUIDbClick {
    public:
        // ctor
        CUIDbClick(uint32_t dur = LongUI::GetDoubleClickTime()) noexcept 
         : m_dwDbClickDur(dur) {}
        // dtor
        ~CUIDbClick() noexcept {}
        // no copy ctor
        CUIDbClick(const CUIDbClick&) noexcept = delete;
        // click, return ture if db-clicked
        bool Click() noexcept;
    protected:
        // time dur
        uint32_t        m_dwDbClickDur;
        // time pt
        uint32_t        m_dwDbClickPt = 0;
    };
    // double click helper - point support
    class CUIDbClickEx : public CUIDbClick {
    public:
        // ctor
        CUIDbClickEx(uint32_t dur) noexcept : CUIDbClick(dur) {}
        // ctor
        CUIDbClickEx() noexcept : CUIDbClick() {}
        // click, return ture if db-clicked
        bool Click(int32_t x, int32_t y) noexcept;
        // click, return ture if db-clicked
        bool Click(float x, float y) noexcept { 
            static_assert(sizeof(int32_t) == sizeof(float), "must be same");
            return Click(
                reinterpret_cast<const int32_t&>(x),
                reinterpret_cast<const int32_t&>(y)
            );
        }
    private:
        // point x
        int32_t           m_x = 0;
        // point y
        int32_t           m_y = 0;
    };
}