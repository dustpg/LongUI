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
    // base container control class -- 基本容器类
    class UIContainer : public UIControl {
        // 父类申明
        using Super = UIControl;
    public:
        // itr 迭代器
        class Iterator {
        public:
            // operator =
            auto& operator=(const Iterator& itr) noexcept { m_pControl = itr.m_pControl; return *this; }
            // ctor
            Iterator(UIControl* c) noexcept :m_pControl(c) { }
            // ctor cpy
            Iterator(const Iterator& itr) noexcept : m_pControl(itr.m_pControl) { }
            // ctor mov
            Iterator(Iterator&& itr) noexcept : m_pControl(itr.m_pControl) { itr.m_pControl = nullptr; }
            //  ++itr
            auto operator++() noexcept { assert(m_pControl); m_pControl = m_pControl->next; return *this; }
            // itr++
            auto operator++(int) const noexcept { assert(m_pControl); Iterator itr(m_pControl); return ++itr; }
            //  --itr
            auto operator--() noexcept { assert(m_pControl); m_pControl = m_pControl->prev; return *this; }
            // itr--
            auto operator--(int) const noexcept { assert(m_pControl); Iterator itr(m_pControl); return --itr; }
            // operator ==
            auto operator==(UIControl* c) const noexcept { return m_pControl == c; }
            // operator ==
            auto operator==(Iterator& itr) const noexcept { return m_pControl == itr.m_pControl; }
            // operator !=
            auto operator!=(UIControl* c) const noexcept { return m_pControl != c; }
            // operator !=
            auto operator!=(Iterator& itr) const noexcept { return m_pControl != itr.m_pControl; }
            // pointer
            auto Ptr() const noexcept { return m_pControl; }
            // operator ->
            auto operator->() const noexcept { return m_pControl; }
            // operator *
            auto operator*() const noexcept { return m_pControl; }
        private:
            UIControl*          m_pControl;
        };
        // marginal control
        enum MarginalControl { Control_Left = 0, Control_Top, Control_Right, Control_Bottom, CONTROL_SIZE };
    public:
        // render
        virtual void Render(RenderType) const noexcept override;
        // update
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument&) noexcept override;
        // recreate
        virtual auto Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
    public:
        // ctor
        UIContainer(pugi::xml_node node) noexcept;
        // before update child
        void BeforeUpdateChildren() noexcept;
        // add a child
        void AfterInsert(UIControl* child) noexcept;
        // after change draw position
        void AfterChangeDrawPosition() noexcept;
        // push clip
        void PushAxisAlignedClip(D2D1_ANTIALIAS_MODE = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE)const noexcept;
        // Assure the ScrollBar
        bool AssureScrollBar(float, float) noexcept;
        // find control where mouse pointed
        auto FindControl(const D2D1_POINT_2F pt) noexcept->UIControl*;
    public: // STL Container Compatibled/style interface/method
        // get child at, because of list, will get warning of performance
        auto at(uint32_t) const noexcept ->UIControl*;
        // insert child,
        void insert(Iterator, UIControl*) noexcept;
        // just remove child, : remove from list and set prev/next to null
        bool remove(Iterator) noexcept;
        // remove and close child
        void erase(Iterator itr) noexcept { this->remove(itr); itr->WindUp(); }
    public: 
        // get children count
        LongUIInline auto size() const noexcept { return m_cChildrenCount; } ;
        // begin 
        LongUIInline auto begin() const noexcept { return Iterator(m_pHead); };
        // end
        LongUIInline auto end() const noexcept { return Iterator(nullptr); };
    protected:
        // dtor 析构函数
        ~UIContainer() noexcept;
        // 子控件数量
        size_t                  m_cChildrenCount = 0;
        // 头控件指针
        UIControl*              m_pHead = nullptr;
        // 尾控件指针
        UIControl*              m_pTail = nullptr;
        // orginal margin
        D2D1_RECT_F             m_orgMargin = D2D1::RectF();
    public:
        // get width in child level 
        auto GetChildLevelWidth() noexcept { return this->width / this->zoom.width; }
        // get height in child level 
        auto GetChildLevelHeight() noexcept { return this->height / this->zoom.height; }
    public:
        // offset position
        D2D1_POINT_2F           offset = D2D1::Point2F();
        // visible size
        D2D1_SIZE_F             zoom = D2D1::SizeF(1.f, 1.f);
        // transform for world: XXX: maybe set it to UIContrl not here
        D2D1_MATRIX_3X2_F       world = D2D1::Matrix3x2F::Identity();
        // 水平滚动条
        UIScrollBar*            scrollbar_h = nullptr;
        // 垂直滚动条
        UIScrollBar*            scrollbar_v = nullptr;
        // marginal controls
        UIControl*              marginal_control[CONTROL_SIZE];
    protected:
        // create control function
        CreateControlFunction   m_apCCFunctin[CONTROL_SIZE];
        // create control
        uint16_t                m_aidCCTemplateID[CONTROL_SIZE];
        // create h-sb
        CreateControlFunction   m_pCreateH = nullptr;
        // create v-sb
        CreateControlFunction   m_pCreateV = nullptr;
    };
    // XXX: top level?
    LongUIInline auto UIControl::IsTopLevel() const noexcept { return static_cast<UIControl*>(this->parent) == this; }
}