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
    // ui list element
    class UIListElement : public UIHorizontalLayout {
        // super class
        using Super = UIHorizontalLayout;
    public:
        // Render 渲染 
        //virtual auto Render() noexcept ->HRESULT override;
        // 刷新
        //virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(LongUI::EventArgument&) noexcept override;
        // recreate 重建
        //virtual auto Recreate() noexcept->HRESULT override;
        // clean this control 清除控件
        virtual void Cleanup() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIListElement(pugi::xml_node node) noexcept;
        // dtor
        ~UIListElement() noexcept = default;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
    // ui list, child must be UIListElement
    class UIList : public UIVerticalLayout {
        // super class
        using Super = UIVerticalLayout;
    public:
        // clean this control 清除控件
        virtual void Cleanup() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIList(pugi::xml_node node) noexcept;
        // dtor
        ~UIList() noexcept = default;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIListElement>() {
        // {E5CF04FC-1221-4E06-B6F3-315D45B1F2E6}
        static const GUID IID_LongUI_UIListElement = {
         0x83b86af2, 0x6755, 0x47a8, { 0xba, 0x7d, 0x69, 0x3c, 0x2b, 0xdb, 0xf, 0xbc } 
        };
        return IID_LongUI_UIListElement;
    }
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIList>() {
        // {E5CF04FC-1221-4E06-B6F3-315D45B1F2E6}
        static const GUID IID_LongUI_UIList = {
            0xe5cf04fc, 0x1221, 0x4e06,{ 0xb6, 0xf3, 0x31, 0x5d, 0x45, 0xb1, 0xf2, 0xe6 } 
        };
        return IID_LongUI_UIList;
    }
#endif
}