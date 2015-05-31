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
    // Scroll Bar Type
    enum class ScrollBarType : uint32_t {
        Type_Vertical = 0,      // 垂直
        Type_Horizontal,        // 水平
    };
    // ScrollBar 形容结构体
    struct ScrollBarDesc{
        // 滚动条类型
        ScrollBarType       type;
        // the size(width/height) of scroll bar
        float               size;
    };
    // UIContainer
    class UIContainer;
    // default scroll bar 默认滚动条
    class LongUIAPI UIScrollBar : public UIControl {
    private:
        // 父类申明
        using Super = UIControl ;
    public:
        // Render 渲染 --- 放在第一位!
        auto Render(RenderType) noexcept ->HRESULT override;
        // do event 事件处理
        bool DoEvent(LongUI::EventArgument&) noexcept override;
        // recreate 重建
        auto Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
        // close this control 关闭控件
        void Close() noexcept override;
    public:
        // is effective
        operator bool() const noexcept { return m_bEffective; }
        // create desc of bar 创建滚动条描述
        static auto CreateDesc(const char* attr, ScrollBarType) noexcept -> const ScrollBarDesc&;
    public:
        // constructor 构造函数
        UIScrollBar(const ScrollBarDesc&, UIContainer* ) noexcept;
        // destructor 析构函数
        ~UIScrollBar() noexcept;
        // deleted function
        UIScrollBar(const UIScrollBar&) = delete;
        // 更新
        void Refresh() noexcept;
    public:
        // desc 描述体
        const ScrollBarDesc     desc;
        // now index of scroll bar
        float                   index = 0.f;
        // max range of scroll bar
        float                   max_range = 1.f;
    protected:
        // the onwer of scroll bar
        UIContainer*            m_pOwner = nullptr;
        // brush of this
        ID2D1Brush*             m_pBrush = nullptr;
        // arrow 1 text path geo
        ID2D1PathGeometry*      m_pArrow1Text = nullptr;
        // arrow 2 text path geo
        ID2D1PathGeometry*      m_pArrow2Text = nullptr;
        // the rect of arrow 2
        D2D1_RECT_F             m_rtArrow1 = D2D1::RectF();
        // the rect of arrow 2
        D2D1_RECT_F             m_rtArrow2 = D2D1::RectF();
        // the rect of thumb
        D2D1_RECT_F             m_rtThumb = D2D1::RectF();
        // mouse point [0, 1, 2, 3, 4]
        enum class PointType : uint32_t {
            Type_None,      // None
            Type_Arrow1,    // Arrow1
            Type_Thumb,     // Thumb
            Type_Shaft,     // Shaft
            Type_Arrow2,    // Arrow2
        };
        PointType               m_pointType = PointType::Type_None;
        // is effective ?
        bool                    m_bEffective = false;
        //
        bool                    scrollbar_unused[3];
    };

}