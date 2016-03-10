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
    // ui list line, the width child must be fixed
    // State_Self1 : Selected
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
        //virtual auto Recreate() noexcept ->HRESULT override;
        // 刷新
        virtual void Update() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // create 创建
        static auto WINAPI CreateControl(UIContainer* cp) noexcept ->UIListLine*;
        // ctor
        UIListLine(UIContainer* cp) noexcept : Super(cp) { }
        // get sorted data
        auto GetToBeSorted() const noexcept { return m_pToBeSorted; }
        // set to be sorted control
        auto SetToBeSorted(uint32_t index) noexcept { m_pToBeSorted = this->GetAt(index); }
        // selected!
        auto SetSelected(bool b) noexcept { m_state.SetTo(State_Self1, b); }
        // selected?
        auto IsSelected() const noexcept { return m_state.Test(State_Self1); }
        // get sorted child text
        auto GeToBeSortedText() const noexcept { return m_pToBeSorted ? m_pToBeSorted->GetText() : L""; }
        // get first child text
        auto GetFirstChildText() const noexcept { return this->GetChildrenCount() ? (*this->begin())->GetText() : L""; }
    protected:
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // init
        void initialize(pugi::xml_node node) noexcept;
        // init without xml-node
        inline void initialize() noexcept;
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
        // update width in marginal
        virtual void UpdateMarginalWidth() noexcept override;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIListHeader(UIContainer* cp) noexcept : Super(cp) { }
        // dtor
        ~UIListHeader() noexcept = default;
    protected:
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // init
        void initialize(pugi::xml_node node) noexcept;
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
        // using
        using LinesVector = EzContainer::PointerVector<UIListLine>;
        // line template vector
        using LineTemplateVector = EzContainer::EzVector<Helper::CC>;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // flag for UIList
        enum UIListFlag : uint32_t {
            // sortable list line in UIControl::user_data(Integer) or user_ptr(const wchar_t*)
            Flag_SortableLineWithUserDataPtr = 1 << 0,
            // the sequence of element could be changed by mouse drag if header exist
            Flag_DraggableHeaderSequence = 1 << 1,
            // multi-select
            Flag_MultiSelect = 1 << 2,
            // default flag
            Flag_Default = 0,
        };
    public:
        // render this
        virtual void Render() const noexcept override;
        // update this
        virtual void Update() noexcept override;
        // do event
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate this
        virtual auto Recreate() noexcept ->HRESULT override;
        // find child control by mouse point
        virtual auto FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* override final;
    protected:
        // register ui call
        virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept override;
        // render chain -> background
        void render_chain_background() const noexcept;
        // render chain -> mainground
        void render_chain_main() const noexcept;
        // render chain -> foreground
        void render_chain_foreground() const noexcept { return Super::render_chain_foreground(); }
    public:
#ifdef _DEBUG
    private:
        // debug for_each
        virtual void debug_for_each(const CUIFunction<void(UIControl*)>& call) noexcept override {
            for (auto ctrl : (m_vLines)) { call(ctrl); }
        }
    public:
#endif
        // refresh layout
        virtual void RefreshLayout() noexcept override final;
        // push back
        virtual void Push(UIControl* child) noexcept;
        // remove child 
        virtual void Remove(UIControl* child) noexcept;
        // insert
        void Insert(uint32_t index, UIListLine* line) noexcept;
        // get child at index
        auto GetAt(uint32_t index) const noexcept { return m_vLines[index]; }
    public:
        // get line-element count
        auto GetLineElementCount() const noexcept { return m_vLineTemplate.size(); }
        // insert a line to list with line-template
        auto InsertLineTemplateToList(uint32_t index) noexcept ->UIListLine*;
        // remove line-element with index
        void RemoveLineElementInEachLine(uint32_t index) noexcept;
        // swap 2 line-elements
        void SwapLineElementsInEachLine(uint32_t index1, uint32_t index2) noexcept;
        // inser new line-element to each line
        void InsertNewElementToEachLine(uint32_t index, CreateControlFunction func, size_t tid = 0) noexcept;
        // set cc-element in line-template
        void SetCCElementInLineTemplate(uint32_t index, CreateControlFunction func, size_t tid = 0) noexcept;
        // push line-elemnet
        template<typename ...Args> bool PushLineElement(Args... args) noexcept { return this->InsertLineElement(m_cChildrenCount, args...); }
        // insert line-elemnet
        template<typename ...Args> bool InsertLineElement(uint32_t index, Args...) noexcept;
    public:
        // get conttrols
        const auto&GetContainer() const noexcept { return m_vLines; }
        // get selected lines index
        const auto&GetSelectedIndices() const noexcept { return m_vSelectedIndex; }
        // get height in line
        auto GetLineHeight() const noexcept { return m_fLineHeight; }
        // get last clicked line index
        auto GetLastClickedLineIndex() const noexcept { return m_ixLastClickedLine; }
        // get last clicked line
        auto GetLastClickedLine() const noexcept ->UIListLine* { return m_ixLastClickedLine < m_cChildrenCount ? m_vLines[m_ixLastClickedLine] : nullptr; }
        // get ToBeSortedHeaderChild, used during (AddBeforSortCallBack)
        auto GetToBeSortedHeaderChild() const noexcept { return m_pToBeSortedHeaderChild; }
        // set header
        void SetHeader(UIListHeader* header) noexcept { assert(header); m_pHeader = header; }
        // set element width
        void SetElementWidth(uint32_t index, float width) noexcept;
        // select child
        void SelectChild(uint32_t index, bool new_select = true) noexcept;
        // select to
        void SelectTo(uint32_t index1, uint32_t index2) noexcept;
    private:
        // reset select
        void reset_select() noexcept;
        // select child
        void select_child(uint32_t index, bool new_select) noexcept;
        // select child
        void select_to(uint32_t index1, uint32_t index2) noexcept;
        // sort line
        void sort_line(bool(*cmp)(UIControl* a, UIControl* b)) noexcept;
        // init
        void init_layout() noexcept;
        // set new elements count
        void set_element_count(uint32_t length) noexcept;
        // referent ctrl
        auto get_referent_control() const noexcept ->UIListLine*;
        // find line via mouse point
        auto find_line(const D2D1_POINT_2F& pt) const noexcept ->UIListLine*;
        // find line via mouse point
        auto find_line_index(const D2D1_POINT_2F& pt) const noexcept ->uint32_t;
        // push line-elemnet
        template<typename Itr> void push_back_le_helper(Itr) noexcept {}
        // push line-elemnet
        template<typename Itr, typename ...Args> void push_back_le_helper(Itr, const wchar_t*, Args...) noexcept;
    public:
        // begin itr
        auto begin() noexcept { return m_vLines.begin(); }
        // end itr
        auto end() noexcept { return m_vLines.end(); }
        // begin itr for const
        auto begin() const noexcept { return m_vLines.begin(); }
        // end itr for const
        auto end() const noexcept { return m_vLines.end(); }
        // create 创建
        static auto WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // ctor
        UIList(UIContainer* cp) noexcept : Super(cp) { }
        // sort as element[index]
        void Sort(uint32_t index, UIControl* child) noexcept;
        // add before sort callback
        template<typename T>
        auto AddBeforSortCallBack(T lam) noexcept { m_callBeforSort += lam; }
    protected:
        // something must do before deleted
        void before_deleted() noexcept;
        // init
        void initialize(pugi::xml_node node) noexcept;
        // dtor
        ~UIList() noexcept;
    public:
        // flag for list
        UIListFlag              list_flag = Flag_Default;
    protected:
        // threshold for fast-sort
        uint32_t                m_cFastSortThreshold = 32;
        // double-click helper
        Helper::DoubleClick     m_hlpDbClick;
        // line back-color normal1
        D2D1_COLOR_F            m_colorLineNormal1 = D2D1::ColorF(0ui32, 0.f);
        // line back-color normal2
        D2D1_COLOR_F            m_colorLineNormal2 = D2D1::ColorF(0ui32, 0.f);
        // line back-color hover
        D2D1_COLOR_F            m_colorLineHover = D2D1::ColorF(0xcde8ff, 0.5f);
        // line back-color selected 
        D2D1_COLOR_F            m_colorLineSelected = D2D1::ColorF(0x9cd2ff, 0.5f);
        // list header
        UIListHeader*           m_pHeader = nullptr;
        // to be sorted header child
        UIControl*              m_pToBeSortedHeaderChild = nullptr;
        // befor sort event
        UICallBack              m_callBeforSort;
        // line clicked event
        UICallBack              m_callLineClicked;
        // line db-clicked
        UICallBack              m_callLineDBClicked;
        // hovered line
        UIListLine*             m_pHoveredLine = nullptr;
        // last clicked line index
        uint32_t                m_ixLastClickedLine = uint32_t(-1);
        // line height
        float                   m_fLineHeight = 32.f;
        // line template
        LineTemplateVector      m_vLineTemplate;
        // list lines vector
        LinesVector             m_vLines;
        // selected line index vector
        IndexVector             m_vSelectedIndex;
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
    // -------------------------------------------------------------
    // template for inser-line-element
    template<typename ...Args> bool UIList::InsertLineElement(uint32_t index, Args... args) noexcept {
        auto line = this->InsertLineTemplateToList(index);
        if (line) this->push_back_le_helper(line->begin(), args...);
        else return false;
        return true;
    }
    // helper for push-back line-element
    template<typename Itr, typename ...Args> void UIList::push_back_le_helper(Itr itr, const wchar_t* p, Args... args) noexcept {
        auto ctrl = *itr; ctrl->SetText(p); ++itr;
        this->push_back_le_helper(itr, args...);
    }
}