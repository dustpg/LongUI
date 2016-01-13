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
    // base scroll bar 默认滚动条
    class UIScrollBar : public UIMarginalable {
        // 父类申明
        using Super = UIMarginalable;
    protected:
        // Scroll Bar Type
        enum class ScrollBarType : uint8_t {
            Type_Horizontal = 0,    // 水平
            Type_Vertical = 1,      // 垂直
        };
        // mouse point [0, 1, 2, 3, 4]
        enum class PointType : uint8_t {
            Type_None,      // None
            Type_Arrow1,    // Arrow1
            Type_Arrow2,    // Arrow2
            Type_Thumb,     // Thumb
            Type_Shaft,     // Shaft
        };
    public:
        // update width of marginal
        virtual void UpdateMarginalWidth() noexcept override;
    public:
        // get parent width/height
        auto GetParentWH() noexcept { return 10.f; }
        //auto GetParentWH() noexcept { return this->bartype == ScrollBarType::Type_Horizontal ? this->parent->GetViewWidthByZoomed() : this->parent->GetViewHeightZoomed(); }
        // on page up
        auto OnPageUp() noexcept { return this->SetIndex(m_uiAnimation.end - this->GetParentWH()); }
        // on page down
        auto OnPageDown() noexcept { return this->SetIndex(m_uiAnimation.end + this->GetParentWH()); }
        // on page X
        auto OnPageX(float rate)  noexcept{ return this->SetIndex(m_uiAnimation.end + rate * this->GetParentWH()); }
        // on wheel up
        auto OnWheelUp() noexcept { return this->SetIndex(m_uiAnimation.end - wheel_step); }
        // on wheel down
        auto OnWheelDown() noexcept { return this->SetIndex(m_uiAnimation.end + wheel_step); }
        // on wheel X
        auto OnWheelX(float rate) noexcept { return this->SetIndex(m_uiAnimation.end + rate * wheel_step); }
    public:
        // how size that take up the owner's space
        auto GetIndex() const noexcept { return m_fIndex; }
        // set new index
        void SetIndex(float new_index) noexcept;
    protected:
        // get bar length
        auto get_length() noexcept { return bartype == ScrollBarType::Type_Vertical ? parent->GetContentHeightZoomed() : parent->GetContentWidthZoomed(); }
        // set index
        void set_index(float index) noexcept;
    public:
        // constructor 构造函数
        UIScrollBar(UIContainer* cp, pugi::xml_node) noexcept;
        // deleted function
        UIScrollBar(const UIScrollBar&) = delete;
    public:
        // bartype of scrollbar
        ScrollBarType   const   bartype = ScrollBarType::Type_Vertical;
        // want animation?
        bool                    m_bAnimation = false;
    protected:
        // tpye of mouse pointed
        PointType               m_pointType = PointType::Type_None;
        // tpye of mouse last pointed
        PointType               m_lastPointType = PointType::Type_None;
    public:
        // step distance for whell up/down
        float                   wheel_step = 32.f;
    protected:
        // max index of scroll bar
        float                   m_fMaxIndex = 0.f;
        // max range of scroll bar
        float                   m_fMaxRange = 1.f;
        // now index of scroll bar
        float                   m_fIndex = 0.f;
        // old index of scroll bar
        float                   m_fOldIndex = 0.f;
        // old point of scroll bar
        float                   m_fOldPoint = 0.f;
        // animation
        CUIAnimationOffset      m_uiAnimation;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
    // srcollbar type A
    class UIScrollBarA : public UIScrollBar {
        // basic size
        static constexpr float BASIC_SIZE = 16.f;
        // 父类申明
        using Super = UIScrollBar;
        // arrow for this
        enum { Arrow_Left, Arrow_Top, Arrow_Right, Arrow_Bottom, ARROW_SIZE };
        // path geo
        static ID2D1PathGeometry*       s_apArrowPathGeometry[ARROW_SIZE];
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // ctor
        UIScrollBarA(UIContainer* cp, pugi::xml_node node) noexcept;
        // create this
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
    public:
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        //bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event 鼠标事件处理
        bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate
        auto Recreate() noexcept ->HRESULT override;
    public:
        // init sb
        //virtual void InitMarginalControl(MarginalControl _type) noexcept override;
        // update width of marginal
        virtual void UpdateMarginalWidth() noexcept override;
    private:
        // set new state
        void set_state(PointType _bartype, ControlState state) noexcept;
    protected:
        // dtor
        ~UIScrollBarA() noexcept;
    private:
        // the rect of arrow 2
        D2D1_RECT_F             m_rtArrow1 = D2D1::RectF();
        // the rect of arrow 2
        D2D1_RECT_F             m_rtArrow2 = D2D1::RectF();
        // the rect of thumb
        D2D1_RECT_F             m_rtThumb = D2D1::RectF();
        // arrow 1 text path geo
        ID2D1PathGeometry*      m_pArrow1Geo = nullptr;
        // arrow 2 text path geo
        ID2D1PathGeometry*      m_pArrow2Geo = nullptr;
        // arrow1
        Component::Element4Bar  m_uiArrow1;
        // arrow2
        Component::Element4Bar  m_uiArrow2;
        // thumb
        Component::Element4Bar  m_uiThumb;
        // step for arrow clicked
        float                   m_fArrowStep = 32.f;
        // captured this
        bool                    m_bCaptured = false;
        // arrow1 use colorrect
        bool                    m_bArrow1InColor = false;
        // arrow2 use colorrect
        bool                    m_bArrow2InColor = false;
        // unused
        bool                    unused_bara = false;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
    // srcollbar type B
    class UIScrollBarB : public UIScrollBar {
        // 父类申明
        using Super = UIScrollBar;
        // close this control 关闭控件
        void cleanup() noexcept override;
    public:
        // render
        void Render() const noexcept override {};
        // update
        void Update() noexcept override;
        // update width of marginal
        virtual void UpdateMarginalWidth() noexcept override {}
    public:
        // ctor
        UIScrollBarB(UIContainer* cp, pugi::xml_node node) noexcept;
        // create this
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIScrollBar>() {
        // {AD925DE3-636D-44DD-A01E-A2C180DEA98D}
        static const GUID IID_LongUI_UIScrollBar = { 
            0xad925de3, 0x636d, 0x44dd,{ 0xa0, 0x1e, 0xa2, 0xc1, 0x80, 0xde, 0xa9, 0x8d } 
        };
        return IID_LongUI_UIScrollBar;
    }
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIScrollBarA>() {
        // {AD925DE3-636D-44DD-A01E-A2C180DEA98D}
        static const GUID IID_LongUI_UIScrollBarA = {
            0x30af626, 0x1958, 0x4bdf,{ 0x86, 0x3e, 0x19, 0x2b, 0xdb, 0x1a, 0x49, 0x46 }
        };
        return IID_LongUI_UIScrollBarA;
    }
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIScrollBarB>() {
        // {820DACDF-5B99-4291-A9B2-9010BE28D12D}
        static const GUID IID_LongUI_UIScrollBarB = { 
            0x820dacdf, 0x5b99, 0x4291,{ 0xa9, 0xb2, 0x90, 0x10, 0xbe, 0x28, 0xd1, 0x2d } 
        };
        return IID_LongUI_UIScrollBarB;
    }
#endif
}