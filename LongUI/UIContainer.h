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
    class LongUIAPI UIContainer : public UIControl {
        // 父类申明
        using Super = UIControl;
    protected:
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
            UIControl*      m_pControl;
        };
    public:
        // do event 事件处理
        virtual bool LongUIMethodCall DoEvent(LongUI::EventArgument&) noexcept override;
        // recreate
        virtual auto LongUIMethodCall Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
        // render
        virtual auto LongUIMethodCall Render() noexcept->HRESULT override;
    public:
        // ctor
        UIContainer(pugi::xml_node node) noexcept;
    public:
        // add a child
        void AfterInsert(UIControl* child) noexcept;
    public: //
        // update children's layout
        inline  void LongUIMethodCall UpdateChildLayout(bool b=true) noexcept { if (m_bDrawSizeChanged) this->RefreshChildLayout(b); }
        // refresh children's layout
        virtual void LongUIMethodCall RefreshChildLayout(bool refresh_scroll) noexcept ;
    public: // STL Container Compatibled interface/method
        // get child at, because of list, will get warning of performance
        auto LongUIMethodCall at(uint32_t) const noexcept ->UIControl*;
        // insert child,
        void LongUIMethodCall insert(Iterator, UIControl*) noexcept;
        // just remove child, : remove from list and set prev/next to null
        bool LongUIMethodCall remove(Iterator) noexcept;
        // remove and close child
        void LongUIMethodCall erase(Iterator itr) noexcept { this->remove(itr); itr->Close(); }
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
    public:
        // end of right
        float             const end_of_right = 0.f;
        // end of bottom
        float             const end_of_bottom = 0.f;
        // 当前世界转换矩阵
        D2D1_MATRIX_3X2_F       world = D2D1::Matrix3x2F::Identity();
        // 本容器转换矩阵
        D2D1_MATRIX_3X2_F       transform = D2D1::Matrix3x2F::Identity();
        // 水平滚动条
        UIScrollBar             scrollbar_h;
        // 垂直滚动条
        UIScrollBar             scrollbar_v;
    };
#if 0
    // -------------------------------------------------
    // [Test] Single Container 单容器
    class UISingleContainer : public UIContainer {
        // 父类声明
        using Super = UIContainer;
    public: // UIControl
        // 构造
        UISingleContainer(pugi::xml_node node) noexcept : Super(node){
            new(&m_itrBegin) Iterator(reinterpret_cast<void**>(&single_control), nullptr);
            m_itrEnd = m_itrBegin;
        }
        // 预渲染
        virtual void LongUIMethodCall PreRender() noexcept override{ 
            if (this->single_control) {
                this->single_control->PreRender();
            }
            return Super::PreRender();
        }
        // 重建
        virtual auto LongUIMethodCall Recreate(LongUIRenderTarget* target) noexcept ->HRESULT override { return Super::Recreate(target); }
        // 渲染
        virtual auto LongUIMethodCall Render() noexcept ->HRESULT override;
    public: // UIContainer
        // get child at
        auto LongUIMethodCall at(uint32_t i) const noexcept ->UIControl* override final {
            return i == 0 ? single_control : nullptr; 
        };
        // insert child 
        void LongUIMethodCall insert(BasicContainer::iterator, UIControl* c) noexcept final override {
            if (this->single_control) this->single_control->Close();
            this->single_control = c;
            this->AfterInsert(c);
            m_itrEnd = m_itrBegin + 1;
            m_cChildrenCount = 1;
        }
        // remove
        void LongUIMethodCall erase(BasicContainer::iterator i) noexcept final override {
            if (this->single_control && i == m_itrBegin) {
                this->single_control->Close();
                this->single_control = nullptr;
                m_cChildrenCount = 0;
                m_itrEnd = m_itrBegin;
            }
        }
    public:
        // single control
        UIControl*                  single_control = nullptr;
    protected:
    };
#endif
}