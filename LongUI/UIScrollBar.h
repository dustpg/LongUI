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
namespace LongUI{
    // 获取相对数值
#define UISB_OffsetVaule(f) ((&(f))[int(this->type)])
    // Scroll Bar Type
    enum class ScrollBarType : uint8_t {
        Type_Horizontal = 0,    // 水平
        Type_Vertical = 1,      // 垂直
    };
    // UIContainer
    class UIContainer;
    // base scroll bar 默认滚动条
    class UIScrollBar : public UIControl {
    private:
        // 父类申明
        using Super = UIControl ;
    protected:
        // mouse point [0, 1, 2, 3, 4]
        enum class PointType : uint8_t {
            Type_None,      // None
            Type_Arrow1,    // Arrow1
            Type_Arrow2,    // Arrow2
            Type_Thumb,     // Thumb
            Type_Shaft,     // Shaft
        };
    public:
        // do event 事件处理
        bool DoEvent(const LongUI::EventArgument&) noexcept override;
    public:
        // Render 渲染 
        //virtual void Render(RenderType type) const noexcept override;
        // udate 刷新
        //virtual void Update() noexcept override;
        // init sb
        virtual inline void InitScrollBar(UIContainer* owner, ScrollBarType _type) noexcept { force_cast(parent) = owner; force_cast(type) = _type; }
        // on needed, maybe 'need' is same in twice
        virtual void OnNeeded(bool need) noexcept = 0;
    public:
        // get parent width/height
        auto GetParentWH() noexcept { return this->type == ScrollBarType::Type_Horizontal ? this->parent->GetChildLevelWidth() : this->parent->GetChildLevelHeight(); }
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
        // before update
        void BeforeUpdate() noexcept;
    protected:
        // get bar length
        auto get_length() noexcept { return type == ScrollBarType::Type_Vertical ? parent->cheight : parent->cwidth; }
        // set index
        void set_index(float index) noexcept;
    public:
        // constructor 构造函数
        UIScrollBar(pugi::xml_node) noexcept;
        // deleted function
        UIScrollBar(const UIScrollBar&) = delete;
    public:
        // type of scrollbar
        ScrollBarType   const   type = ScrollBarType::Type_Vertical;
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
    public:
        // another sb
        UIScrollBar*            another = nullptr;
    };
    // srcoll bar - type A 型 滚动条
    class UIScrollBarA final : public UIScrollBar {
        // basic size
        static constexpr float BASIC_SIZE = 16.f;
        // 父类申明
        using Super = UIScrollBar;
        // ui element
        using BarElement = Component::Elements<Element::Meta, Element::ColorRect, Element::Basic>;
        // arrow for this
        enum { Arrow_Left, Arrow_Top, Arrow_Right, Arrow_Bottom, ARROW_SIZE };
        // path geo
        static ID2D1PathGeometry*       s_apArrowPathGeometry[ARROW_SIZE];
    public:
        // ctor
        UIScrollBarA(pugi::xml_node node) noexcept;
        // create this
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept->UIControl*;
    public:
        // Render 渲染 
        virtual void Render(RenderType _type) const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        bool DoEvent(const LongUI::EventArgument&) noexcept override;
        // recreate
        auto Recreate(LongUIRenderTarget* )noexcept->HRESULT override;
        // close this control 关闭控件
        void WindUp() noexcept override;
    public:
        // init sb
        void InitScrollBar(UIContainer* owner, ScrollBarType _type) noexcept override;
        // on needed
        void OnNeeded(bool need) noexcept override;
    private:
        // set new status
        void set_status(PointType _type, ControlStatus state) noexcept;
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
        BarElement              m_uiArrow1;
        // arrow2
        BarElement              m_uiArrow2;
        // thumb
        BarElement              m_uiThumb;
        // step for arrow clicked
        float                   m_fArrowStep = 16.f;
        // captured this
        bool                    m_bCaptured = false;
        // arrow1 use colorrect
        bool                    m_bArrow1InColor = false;
        // arrow2 use colorrect
        bool                    m_bArrow2InColor = false;
        // unused
        bool                    m_bLastNeed = false;
    };

}