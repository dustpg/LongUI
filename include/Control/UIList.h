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
    // ui list line
    class UIListLine : public UIHorizontalLayout {
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
        UIListLine(pugi::xml_node node) noexcept;
        // dtor
        ~UIListLine() noexcept = default;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
    // ui list header, parent must be UIList
    class UIListHeader : public UIMarginalable {
        // super class
        using Super = UIMarginalable;
    public:
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument&) noexcept override;
        // clean this control 清除控件
        virtual void Cleanup() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIListHeader(pugi::xml_node node) noexcept;
        // dtor
        ~UIListHeader() noexcept = default;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
    // ui list, child must be UIListLine
    class UIList : public UIContainer {
        // super class
        using Super = UIContainer;
    public:
        // update
        virtual void Update() noexcept override;
        // clean this control 清除控件
        virtual void Cleanup() noexcept override;
    private:
        // after add a child
        inline void after_insert(UIControl* child) noexcept;
    public:
        // after add a child
        virtual void AfterInsert(UIControl* child) noexcept override;
#ifdef LONGUI_UILIST_VECTOR
        // after add a child
        virtual void AfterRemove(UIControl* child) noexcept override;
        // get child at index
        virtual auto GetAt(uint32_t index) const noexcept ->UIControl* final override;
    private:
        // control vector
        BasicContainer          m_controls;
#endif
    public:
        // insert a line-template with inside string
        void InsertInlineTemplate(Iterator itr) noexcept { this->InsertInlineTemplate(itr, m_pLineTemplate); }
        // insert a line-template with outside string
        void InsertInlineTemplate(Iterator itr, const char* line) noexcept;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIList(pugi::xml_node node) noexcept;
        // dtor
        ~UIList() noexcept;
    protected:
        // line template
        char*                   m_pLineTemplate = nullptr;
        // line height
        float                   m_fLineHeight = 32.f;
        // elements count in each line
        uint32_t                m_cEleCountInLine = 0;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIListLine>() {
        // {E5CF04FC-1221-4E06-B6F3-315D45B1F2E6}
        static const GUID IID_LongUI_UIListLine= {
         0x83b86af2, 0x6755, 0x47a8, { 0xba, 0x7d, 0x69, 0x3c, 0x2b, 0xdb, 0xf, 0xbc } 
        };
        return IID_LongUI_UIListLine;
    }
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIList>() {
        // {E5CF04FC-1221-4E06-B6F3-315D45B1F2E6}
        static const GUID IID_LongUI_UIList = {
            0xe5cf04fc, 0x1221, 0x4e06,{ 0xb6, 0xf3, 0x31, 0x5d, 0x45, 0xb1, 0xf2, 0xe6 } 
        };
        return IID_LongUI_UIList;
    }
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIListHeader>() {
        // {E5CF04FC-1221-4E06-B6F3-315D45B1F2E6}
        static const GUID IID_LongUI_UIListHeader = { 
            0x6db3aac2, 0xf4cf, 0x4301, { 0x92, 0x91, 0xa5, 0x18, 0x1b, 0x22, 0xa0, 0x39 } 
        };
        return IID_LongUI_UIListHeader;
    }
#endif
}