#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/


// longui namespace
namespace LongUI {
    // render queue helper
    class CUIRenderQueue {
    public:
        // UNIT
        struct UNIT { 
            // length of this unit
            size_t      length;
            // main data of unit
            UIControl*  units[LongUIDirtyControlSize];
        };
        // ctor
        CUIRenderQueue(UIWindow* window) noexcept;
        // dtor
        ~CUIRenderQueue() noexcept;
        // reset
        void Reset(uint32_t f) noexcept;
        // ++ operator
        void operator++() noexcept;
        // plan to render
        void PlanToRender(float wait_time, float render_time, UIControl* control) noexcept;
        // get current unit
        auto GetCurrentUnit() const noexcept { return m_pCurrentUnit; }
        // get display frequency
        auto GetDisplayFrequency() const noexcept { return m_dwDisplayFrequency; }
    private: // queue zone
        // frequency for display
        uint32_t            m_dwDisplayFrequency = 0;
        // render start time
        uint32_t            m_dwStartTime = 0;
        // current unit
        UNIT*               m_pCurrentUnit = nullptr;
        // units data
        UNIT*               m_pUnitsDataBegin = nullptr;
        // end of data
        UNIT*               m_pUnitsDataEnd = nullptr;
    private: // easy plan
        // UNIT like
        struct { size_t length; UIControl* window; } m_unitLike;
    };
}