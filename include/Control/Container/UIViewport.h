#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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
    // config::window
    namespace Config { struct Window : Control {
        // window title name
        const wchar_t*      title_name;
        // full rendering
        bool                full_rendering;
        // always do rendering
        bool                always_rendering;
    };}
    // config::popup window
    namespace Config { struct Popup {
        // left line of popup window
        float                       leftline;
        // top line(maybe as bottom of popup window)
        float                       topline;
        // bottom line(maybe as top of popup window)
        float                       bottomline;
        // with of popup window
        float                       width;
        // height of popup window
        float                       height;
        // parent window
        XUIBaseWindow*              parent;
        // [OPTIONAL]child
        UIControl*                  child;
    };}
    // ui viewport for window
    class UIViewport : public UIVerticalLayout {
        // 父类申明
        using Super = UIVerticalLayout;
        // 友元申明
        friend class CUIManager;
        // timer id for blink
        static constexpr UINT_PTR BLINK_EVENT_ID = 0;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public: // UIControl 接口实现
        // Render 渲染 
        /*virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event 鼠标事件处理
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;*/
    public:
        // constructor
        UIViewport(XUIBaseWindow* window) noexcept;
        // create popup window
        static auto CreatePopup(const Config::Popup&) noexcept -> UIViewport*;
    protected:
        // something must do before deleted
        void before_deleted() noexcept;
        // init
        void initialize(pugi::xml_node node) noexcept;
        // init as popup window
        void initialize(const Config::Popup& popup) noexcept;
        // destructor
        ~UIViewport() noexcept;
        // deleted 
        UIViewport(const UIViewport&) = delete;
    public: // some new
        // can be closed? called on closed
        virtual bool IsCouldBeClosed() noexcept { return true; }
    protected:
        // next sibling
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIViewport>() {
        // {64F7B3E5-621E-4864-9535-7E6A29F670C1}
        static const GUID IID_LongUI_UIViewport = { 
            0x64f7b3e5, 0x621e, 0x4864,{ 0x95, 0x35, 0x7e, 0x6a, 0x29, 0xf6, 0x70, 0xc1 } 
        };
        return IID_LongUI_UIViewport;
    }
#endif
}