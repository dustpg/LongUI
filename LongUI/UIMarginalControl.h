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


// LongUI namespace
namespace LongUI {
    // Container
    class UIContainer;
    // marginal-able control
    // like: hamburger menu, menu bar, tool bar, tab bar, scroll bar, barabara, etc
    class LongUIAlignas UIMarginalControl : public UIControl {
        // 父类声明
        using Super = UIControl;
    public:
        // marginal control
        enum MarginalControl : uint32_t { 
            Control_Left = 0,           // control at left
            Control_Top,                // control at top
            Control_Right,              // control at right
            Control_Bottom,             // control at bottom
            MARGINAL_CONTROL_SIZE       // control-size
        };
        // init the control
        virtual inline void InitMarginalControl(MarginalControl _type) noexcept { force_cast(this->marginal_type) = _type; }
        // other 2 contactable marginal-able controls' Cross Area
        // for scrollbar, you should do nothing, because 2 scrollbars cannot be crossed
        virtual void CrossAreaTest(D2D1_SIZE_F& IN OUT size) noexcept { UNREFERENCED_PARAMETER(size); }
    public:
        // marginal width, parent's real margin(ltrb) = parent's original(ltrb) + this' marginal_width
        // example: classic scrollbar's marginal_width = it's width/height
        //          modern scrollbar(show when mouse pointed)'s marginal_width = 0
        float                   marginal_width = 0.f;
        // marginal type
        MarginalControl   const marginal_type = MarginalControl::Control_Left;
    protected:
        // ctor
        UIMarginalControl(pugi::xml_node node) noexcept : Super(node) {}
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIMarginalControl>() {
        // {6CF3853D-6740-4635-AF7E-F8A42AEBA6C9}
        static const GUID IID_LongUI_UIMarginalControl = { 
            0x6cf3853d, 0x6740, 0x4635,{ 0xaf, 0x7e, 0xf8, 0xa4, 0x2a, 0xeb, 0xa6, 0xc9 } 
        };
        return IID_LongUI_UIMarginalControl;
    }
#endif
}