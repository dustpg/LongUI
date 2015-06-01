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
    // 基本编辑类
    class LongUIAPI UIEditBasic : public UIControl {
        // 父类申明
        using Super = UIControl;
    public:
        // Render 渲染
        virtual auto Render(RenderType) noexcept->HRESULT override;
        // do event 事件处理
        virtual bool DoEvent(LongUI::EventArgument&) noexcept override;
        // recreate 重建
        virtual auto Recreate(LongUIRenderTarget*) noexcept->HRESULT override;
        // close this control 关闭控件
        virtual void Close() noexcept override;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(pugi::xml_node) noexcept;
    public:
        // control text 控件文本
        auto GetText() const noexcept { return m_text.c_str(); }
    protected:
        // constructor 构造函数
        UIEditBasic(pugi::xml_node) noexcept;
        // destructor 析构函数
        ~UIEditBasic() noexcept {};
        // deleted function
        UIEditBasic(const UIEditBasic&) = delete;
    protected:
        // core editable text component
        Component::CUIEditaleText   m_text;
        // 'I' cursor
        HCURSOR                     m_hCursorI = ::LoadCursorW(nullptr, IDC_IBEAM);
    };
}
