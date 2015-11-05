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
    // Vertical Layout -- 垂直布局 容器
    class UIVerticalLayout : public UIContainerBuiltIn {
        // 父类申明
        using Super = UIContainerBuiltIn;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public: // UIControl
        // render this
        //virtual auto Render() noexcept ->HRESULT override;
        // update this
        //virtual void Update() noexcept override;
        // do event 
        //virtual bool DoEvent(LongUI::EventArgument& arg) noexcept override;
        // recreate
        //virtual auto Recreate() noexcept ->HRESULT override;
    public:
        // refresh layout
        virtual void RefreshLayout() noexcept override final;
    public:
        // create this
        static UIControl* WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept;
        // ctor
        UIVerticalLayout(UIContainer* cp, pugi::xml_node node) noexcept:Super(cp, node) { }
        // dtor
        ~UIVerticalLayout() noexcept = default;
        // no copy ctor
        UIVerticalLayout(const UIVerticalLayout&) = delete;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
    // Horizontal Layout -- 水平布局 容器
    class UIHorizontalLayout : public UIContainerBuiltIn {
        // 父类申明
        using Super = UIContainerBuiltIn;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public: // UIControl
        // Render 渲染 
        //virtual auto Render() noexcept ->HRESULT override;
        // 刷新
        //virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        //virtual auto Recreate() noexcept ->HRESULT override;
    public:
        // refresh layout
        virtual void RefreshLayout() noexcept override final;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept;
        // ctor
        UIHorizontalLayout(UIContainer* cp, pugi::xml_node node) noexcept :Super(cp, node) { }
        // dtor
        ~UIHorizontalLayout() noexcept = default;
        // no copy ctor
        UIHorizontalLayout(const UIHorizontalLayout&) = delete;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIHorizontalLayout>() {
        // {E5CF04FC-1221-4E06-B6F3-315D45B1F2E6}
        static const GUID IID_LongUI_UIHorizontalLayout = {
            0xe5cf04fc, 0x1221, 0x4e06,{ 0xb6, 0xf3, 0x31, 0x5d, 0x45, 0xb1, 0xf2, 0xe6 } 
        };
        return IID_LongUI_UIHorizontalLayout;
    }
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIVerticalLayout>() {
        // {3BE5198C-B922-4C99-827E-F0D08875B045}
        static const GUID IID_LongUI_UIVerticalLayout = {
            0x3be5198c, 0xb922, 0x4c99,{ 0x82, 0x7e, 0xf0, 0xd0, 0x88, 0x75, 0xb0, 0x45 } 
        };
        return IID_LongUI_UIVerticalLayout;
    }
#endif
}
