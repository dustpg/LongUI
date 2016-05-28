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

#include "UIContainerBuiltIn.h"

// LongUI namespace
namespace LongUI {
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
        //virtual void AfterUpdate() noexcept override;
        // do event 
        //virtual bool DoEvent(LongUI::EventArgument& arg) noexcept override;
        // recreate
        //virtual auto Recreate() noexcept ->HRESULT override;
    public:
        // refresh layout
        virtual void RefreshLayout() noexcept override final;
    public:
        // create this
        static UIControl* CreateControl(CreateEventType, pugi::xml_node) noexcept;
        // ctor
        UIVerticalLayout(UIContainer* cp) noexcept:Super(cp) { }
        // dtor
        ~UIVerticalLayout() noexcept = default;
        // no copy ctor
        UIVerticalLayout(const UIVerticalLayout&) = delete;
    protected:
        // init
        void initialize(pugi::xml_node node) noexcept { return Super::initialize(node); }
        // init without xml-node
        void initialize() noexcept { return Super::initialize(); }
#ifdef LongUIDebugEvent
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
        //virtual void AfterUpdate() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        //virtual auto Recreate() noexcept ->HRESULT override;
    public:
        // refresh layout
        virtual void RefreshLayout() noexcept override final;
    public:
        // create 创建
        static UIControl* CreateControl(CreateEventType type, pugi::xml_node) noexcept;
        // ctor
        UIHorizontalLayout(UIContainer* cp) noexcept :Super(cp) { }
        // dtor
        ~UIHorizontalLayout() noexcept = default;
        // no copy ctor
        UIHorizontalLayout(const UIHorizontalLayout&) = delete;
    protected:
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // init
        void initialize(pugi::xml_node node) noexcept { return Super::initialize(node); }
        // init without xml-node
        void initialize() noexcept { return Super::initialize(); }
#ifdef LongUIDebugEvent
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIHorizontalLayout>() noexcept;
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIVerticalLayout>() noexcept;
#endif
}
