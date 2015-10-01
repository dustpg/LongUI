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
    // ui list line, the width child must be fixed
    class UIListLine : public UIHorizontalLayout {
        // super class
        using Super = UIHorizontalLayout;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // Render 渲染 
        //virtual auto Render() noexcept ->HRESULT override;
        // do event 事件处理
        //virtual bool DoEvent(LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        //virtual auto Recreate() noexcept->HRESULT override;
        // 刷新
        virtual void Update() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIListLine(UIContainer* cp, pugi::xml_node node) noexcept;
        // get sorted data
        auto GetToBeSorted() const noexcept { return m_pToBeSorted; }
        // set to be sorted control
        auto SetToBeSorted(uint32_t index) noexcept { m_pToBeSorted = this->GetAt(index); }
    protected:
        // dtor
        ~UIListLine() noexcept = default;
    protected:
        // to be sorted control
        UIControl*          m_pToBeSorted = nullptr;
        // first update
        bool                m_bFirstUpdate = true;
        // unused for line
        bool                m_lineunsed[sizeof(void*) - 1];
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
    // ui list header, parent must be UIList
    class UIListHeader : public UIListLine {
        // super class
        using Super = UIListLine;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // update
        virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event 
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
    public:
        // init
        virtual void InitMarginalControl(MarginalControl _type) noexcept;
        // update width in marginal
        virtual void UpdateMarginalWidth() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIListHeader(UIContainer* cp, pugi::xml_node node) noexcept;
        // dtor
        ~UIListHeader() noexcept = default;
    private:
        // sep hovered control
        UIControl*              m_pSepHovered = nullptr;
        // cross cursor
        HCURSOR                 m_hCursor = ::LoadCursor(nullptr, IDC_SIZEWE);
        // line height
        float                   m_fLineHeight = 32.f;
        // separator with, > 0.f is left, < 0.f is right, == 0.f is no
        float                   m_fSepwidth = 0.f;
        // last mouse position for mouse distance
        float                   m_fLastMousePosX = 0.f;
        // sep hovered control index
        uint32_t                m_indexSepHovered = 0;
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
        // weights buffer
        //using WeightBuffer = EzContainer::SmallBuffer<float, 16>;
        // line template buffer
        using LineTemplateBuffer = EzContainer::SmallBuffer<Helper::CC, 16>;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // flag for UIList
        enum UIListFlag : uint32_t {
            // sortable list line in UIControl::user_data(Integer) or user_ptr(const wchar_t*)
            Flag_SortableLineWithUserDataPtr = 1 << 0,
            // the sequence of element could be changed by mouse drag if header exsit
            Flag_DraggableHeaderSequence = 1 << 1,
            // default flag
            Flag_Default = 0,
        };
    public:
        // render this
        virtual void Render(RenderType) const noexcept override;
        // update this
        virtual void Update() noexcept override;
        // do event
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // recreate this
        virtual auto Recreate() noexcept->HRESULT override;
        // find child control by mouse point
        virtual auto FindChild(const D2D1_POINT_2F& pt) noexcept->UIControl* override final;
    public:
#ifdef _DEBUG
    private:
        // debug for_each
        virtual void debug_for_each(const CUIFunction<void(UIControl*)>& call) noexcept override {
            for (auto ctrl : (m_controls)) { call(ctrl); }
        }
    public:
#endif
        // refresh layout
        virtual void RefreshLayout() noexcept override final;
        // push back
        virtual void PushBack(UIControl* child) noexcept;
        // just remove 
        virtual void RemoveJust(UIControl* child) noexcept;
        // insert
        auto Insert(uint32_t index, UIControl*) noexcept;
        // get child at index
        auto GetAt(uint32_t index) const noexcept { return m_controls[index]; }
    public:
        // get height in line
        auto GetLineHeight() const noexcept { return m_fLineHeight; }
        // insert a line-template with inside string
        void InsertInlineTemplate(uint32_t index) noexcept;
        // set header
        void SetHeader(UIListHeader* header) noexcept { assert(header); m_pHeader = header; }
        // set element width
        void SetElementWidth(uint32_t index, float width) noexcept;
    private:
        // sort line
        void sort_line(bool(*cmp)(UIControl* a, UIControl* b)) noexcept;
        // init
        void init_layout() noexcept;
        // set new elements count
        void set_element_count(uint32_t length) noexcept;
        // referent ctrl
        auto get_referent_control() const noexcept->UIListLine*;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIList(UIContainer* cp, pugi::xml_node node) noexcept;
        // sort as element[index]
        void Sort(uint32_t index, UIControl* child) noexcept;
        // get conttrols
        const auto&GetContainer() const noexcept { return m_controls; }
        // get ToBeSortedHeaderChild, used during (AddBeforSortCallBack)
        auto GetToBeSortedHeaderChild() const noexcept { return m_pToBeSortedHeaderChild; }
        // add before sort callback
        template<typename T>
        auto AddBeforSortCallBack(T lam) noexcept { m_callBeforSort += lam; }
    protected:
        // dtor
        ~UIList() noexcept;
    public:
        // flag for list
        UIListFlag              list_flag = Flag_Default;
    protected:
        // threshold for fast-sort
        uint32_t                m_cFastSortThreshold = 32;
        // list header
        UIListHeader*           m_pHeader = nullptr;
        // to be sorted header child
        UIControl*              m_pToBeSortedHeaderChild = nullptr;
        // befor sort
        UICallBack              m_callBeforSort;
        // line height
        float                   m_fLineHeight = 32.f;
        // elements count in each line
        uint32_t                m_cEleCountInLine = 0;
        // line template
        LineTemplateBuffer      m_bufLineTemplate;
        // control vector
        ControlVector           m_controls;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
    };
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
#else
    };
#endif
    // operator for UIList::UIListFlag
    LONGUI_DEFINE_ENUM_FLAG_OPERATORS(UIList::UIListFlag, uint32_t);
}