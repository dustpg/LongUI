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
    class UIEditBasic : public UIControl {
        // 父类申明
        using Super = UIControl;
        // close this control 关闭控件
        virtual void cleanup() noexcept override;
    public:
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept;
    public:
        // create this
        static UIControl* WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept;
        // constructor 构造函数
        UIEditBasic(UIContainer* cp, pugi::xml_node node) noexcept;
    protected:
        // destructor 析构函数
        ~UIEditBasic() noexcept {};
        // deleted function
        UIEditBasic(const UIEditBasic&) = delete;
    protected:
        // core editable text component
        Component::EditaleText  m_text;
        // 'I' cursor
        HCURSOR                 m_hCursorI = ::LoadCursor(nullptr, IDC_IBEAM);
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIEditBasic>() {
        // {D60826F0-4AF1-48F9-A63A-58117943CE66}
        static const GUID IID_LongUI_UIEditBasic = { 
            0xd60826f0, 0x4af1, 0x48f9, { 0xa6, 0x3a, 0x58, 0x11, 0x79, 0x43, 0xce, 0x66 } 
        };
        return IID_LongUI_UIEditBasic;
    }
#endif
}
