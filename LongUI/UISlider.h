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
    // default slider control 默认滑块控件
    class UISlider : public UIControl {
        // 父类申明
        using Super = UIControl ;
    public:
        // Render 渲染 
        virtual void Render(RenderType type) const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument&) noexcept override;
        // recreate 重建
        //virtual auto Recreate(LongUIRenderTarget*) noexcept->HRESULT override;
        // close this control 关闭控件
        virtual void WindUp() noexcept override;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(pugi::xml_node) noexcept;
        // register Value Changed 注册变动事件
        LongUIInline void RegisterValueChangedEvent(LongUICallBack call, UIControl* target) noexcept {
            m_eventChanged = call; m_pChangedTarget = target;
        };
        // 获取数值
        LongUIInline auto GetValue() { return m_fValue; }
    protected:
        // constructor 构造函数
        UISlider(pugi::xml_node) noexcept;
        // destructor 析构函数
        ~UISlider() noexcept { }
        // deleted function
        UISlider(const UISlider&) = delete;
    protected:
        // changed event target
        UIControl*          m_pChangedTarget = nullptr;
        // changed event
        LongUICallBack      m_eventChanged = nullptr;
        // slider rect
        D2D1_RECT_F         m_rcSlider = D2D1::RectF();
        // value range[0, 1]
        float               m_fValue = 0.f;
        // value range[0, 1]
        float               m_fValueOld = 0.f;
        // start
        float               m_fStart = 0.f;
        // end
        float               m_fEnd = 100.f;
        // slider half width
        float               m_fSliderHalfWidth = static_cast<float>(LongUIDefaultSliderHalfWidth);
        // is mouse click in
        bool                m_bMouseClickIn = false;
        // unused for slider
        bool                m_bunsed_slider[3];
    };
}