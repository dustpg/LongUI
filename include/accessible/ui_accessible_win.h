#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
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

// Gui
#include "ui_accessible.h"
#ifdef LUI_ACCESSIBLE
#include "../core/ui_malloc.h"
#include "../util/ui_unimacro.h"

// c++
#include <atomic>
#include <new>

// Windows
#include <combaseapi.h>
#include <UIAutomation.h>

// ui namespace
namespace LongUI {
    // control class
    class UIControl;
    // window class
    class CUIWindow;
    /// <summary>
    /// accessible class for win
    /// </summary>
    /// <seealso cref="IRawElementProviderSimple" />
    /// <seealso cref="IRawElementProviderFragment" />
    /// <seealso cref="IRawElementProviderFragmentRoot" />
    /// <seealso cref="IRawElementProviderAdviseEvents" />
    /// <seealso cref="ILegacyIAccessibleProvider" />
    /// <seealso cref="IInvokeProvider" />
    /// <seealso cref="ISelectionProvider" />
    /// <seealso cref="ISelectionItemProvider" />
    /// <seealso cref="IRangeValueProvider" />
    /// <seealso cref="IValueProvider" />
    /// <seealso cref="ITextProvider" />
    /// <seealso cref="IGridProvider" />
    /// <seealso cref="IGridItemProvider" />
    /// <seealso cref="ITableProvider" />
    /// <seealso cref="ITableItemProvider" />
    /// <seealso cref="IToggleProvider" />
    /// <seealso cref="IExpandCollapseProvider" />
    /// <seealso cref="ITransformProvider" />
    /// <seealso cref="IScrollProvider" />
    /// <seealso cref="IScrollItemProvider" />
    class CUIAccessible final :
        public IRawElementProviderSimple,
        public IRawElementProviderFragment,
        //public IRawElementProviderFragmentRoot,
        public IRawElementProviderAdviseEvents,
        public IInvokeProvider,
        public ISelectionProvider,
        public ISelectionItemProvider,
        public IRangeValueProvider,
        public IValueProvider,
        //public ITextProvider,
        public IGridProvider,
        public IGridItemProvider,
        public ITableProvider,
        public ITableItemProvider,
        public IToggleProvider,
        public IExpandCollapseProvider,
        public ITransformProvider,
        public IScrollProvider,
        public IScrollItemProvider { //,
        //public IItemContainerProvider,
        //public ILegacyIAccessibleProvider {
        // pattern from cotnrol
        static auto pattern_from_cotnrol(UIControl& ctrl) noexcept->AccessiblePattern;
    public:
        // interface from control
        static auto FromControl(UIControl& ctrl) noexcept->CUIAccessible*;
        // no-exception new
        void*operator new(size_t) = delete;
        // no-exception new[]
        void*operator new[](size_t) = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
        // delete object
        void operator delete(void* ptr) noexcept { LongUI::SmallFree(ptr); }
        // nothrow new 
        void*operator new(size_t size, const std::nothrow_t&) noexcept { return LongUI::SmallAlloc(size); }
        // nothrow delete 
        void operator delete(void* ptr, const std::nothrow_t&) noexcept { LongUI::SmallFree(ptr); }
    protected:
        // is support ITextProvider
        //auto is_support_text() const noexcept { return false; }
        // is support IGridProvider
        auto is_support_grid() const noexcept { return false; }
        // is support ITableProvider
        auto is_support_table() const noexcept { return false; }
        // is support IValueProvider
        auto is_support_value() const noexcept { return m_pattern & Pattern_Value; }
        // is support IToggleProvider
        auto is_support_toggle() const noexcept { return m_pattern & Pattern_Toggle; }
        // is support IInvokeProvider
        auto is_support_invoke() const noexcept { return m_pattern & Pattern_Invoke; }
        // is support IScrollProvider
        auto is_support_scroll() const noexcept { return false; }
        // is support IGridItemProvider
        auto is_support_grid_item() const noexcept { return false; }
        // is support ITransformProvider
        auto is_support_transform() const noexcept { return false; }
        // is support ISelectionProvider
        auto is_support_selection() const noexcept { return false; }
        // is support ITableItemProvider
        auto is_support_table_item() const noexcept { return false; }
        // is support IRangeValueProvider
        auto is_support_range_value() const noexcept { return m_pattern & Pattern_Range; }
        // is support IScrollItemProvider
        auto is_support_scroll_item() const noexcept { return false; }
        // is support ISelectionItemProvider
        auto is_support_selection_item() const noexcept { return false; }
        // is support IExpandCollapseProvider
        auto is_support_expand_collapse() const noexcept { return false; }
        // is support ILegacyIAccessibleProvider
        //auto is_support_legacy_interface() const noexcept { return false; }
    public:
        // ctor
        CUIAccessible(UIControl& ctrl) noexcept;
        // dtor
        ~CUIAccessible() noexcept;
        // no copy
        CUIAccessible(const CUIAccessible&) noexcept = delete;
    public: // IUnknown
        // add ref-count
        auto UNICALL AddRef() noexcept->ULONG override;
        // release ref-count
        auto UNICALL Release() noexcept->ULONG override;
        // QI
        auto UNICALL QueryInterface(REFIID riid, void**out) noexcept->HRESULT override;
    public: // IRawElementProviderSimple
        // [getter] HostRawElementProvider
        auto UNICALL get_HostRawElementProvider(IRawElementProviderSimple **pRetVal)noexcept->HRESULT override;
        // [getter] ProviderOptions
        auto UNICALL get_ProviderOptions(ProviderOptions *pRetVal)noexcept->HRESULT override;
        // get pattern provider
        auto UNICALL GetPatternProvider(PATTERNID patternId, IUnknown **pRetVal)noexcept->HRESULT override;
        // get property value
        auto UNICALL GetPropertyValue(PROPERTYID propertyId, VARIANT *pRetVal)noexcept->HRESULT override;
    public: // IRawElementProviderFragment
        // [getter] BoundingRectangle
        auto UNICALL get_BoundingRectangle(UiaRect *pRetVal)noexcept->HRESULT override;
        // [getter] FragmentRoot
        auto UNICALL get_FragmentRoot(IRawElementProviderFragmentRoot **pRetVal)noexcept->HRESULT override;
        // get embedded fragment roots
        auto UNICALL GetEmbeddedFragmentRoots(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // retrieves the runtime identifier of an element.
        auto UNICALL GetRuntimeId(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // retrieves the UI Automation element in a specified direction within the UI Automation tree.
        auto UNICALL Navigate(NavigateDirection direction, IRawElementProviderFragment **pRetVal)noexcept->HRESULT override;
        // set focus
        auto UNICALL SetFocus()noexcept->HRESULT override;
    public: // IRawElementProviderAdviseEvents
        // added
        auto UNICALL AdviseEventAdded(EVENTID eventId, SAFEARRAY *propertyIDs)noexcept->HRESULT override;
        // removed
        auto UNICALL AdviseEventRemoved(EVENTID eventId, SAFEARRAY *propertyIDs)noexcept->HRESULT override;
    public: // IInvokeProvider
        // invoke / default action(?)
        auto UNICALL Invoke()noexcept->HRESULT override;
    public: // ISelectionProvider
        // get selection
        auto UNICALL GetSelection(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // [getter] CanSelectMultiple
        auto UNICALL get_CanSelectMultiple(BOOL *pRetVal)noexcept->HRESULT override;
        // [getter] IsSelectionRequired
        auto UNICALL get_IsSelectionRequired(BOOL *pRetVal)noexcept->HRESULT override;
    public: // ISelectionItemProvider
        // Deselects any selected items and then selects the current element.
        auto UNICALL Select()noexcept->HRESULT override;
        // Adds the current element to the collection of selected items.
        auto UNICALL AddToSelection()noexcept->HRESULT override;
        // Removes the current element from the collection of selected items.
        auto UNICALL RemoveFromSelection()noexcept->HRESULT override;
        // [getter] IsSelected
        auto UNICALL get_IsSelected(BOOL *pRetVal)noexcept->HRESULT override;
        // [getter] SelectionContainer
        auto UNICALL get_SelectionContainer(IRawElementProviderSimple **pRetVal)noexcept->HRESULT override;
    public: // IRangeValueProvider
        // set value
        auto UNICALL SetValue(double val)noexcept->HRESULT override;
        // [getter] Value
        auto UNICALL get_Value(double *pRetVal)noexcept->HRESULT override;
        // [getter] IsReadOnly
        auto UNICALL get_IsReadOnly(BOOL *pRetVal)noexcept->HRESULT override;
        // [getter] Maximum
        auto UNICALL get_Maximum(double *pRetVal)noexcept->HRESULT override;
        // [getter] Minimum
        auto UNICALL get_Minimum(double *pRetVal)noexcept->HRESULT override;
        // [getter] LargeChange
        auto UNICALL get_LargeChange(double *pRetVal)noexcept->HRESULT override;
        // [getter] SmallChange
        auto UNICALL get_SmallChange(double *pRetVal)noexcept->HRESULT override;
    public: // IValueProvider
        // set value
        auto UNICALL SetValue(LPCWSTR val)noexcept->HRESULT override;
        // [getter] Value
        auto UNICALL get_Value(BSTR *pRetVal)noexcept->HRESULT override;
        // [getter] IsReadOnly
        // auto UNICALL get_IsReadOnly;     // Defined in IRangeValueProvider
    public: // ITextProvider
        // get selection
        // auto UNICALL GetSelection        // Defined in ISelectionProvider
        // get visible ranges
        //auto UNICALL GetVisibleRanges(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // get range from child
        //auto UNICALL RangeFromChild(IRawElementProviderSimple *childElement, ITextRangeProvider **pRetVal)noexcept->HRESULT override;
        // get range from point
        //auto UNICALL RangeFromPoint(UiaPoint point, ITextRangeProvider **pRetVal)noexcept->HRESULT override;
        // [getter] DocumentRange
        //auto UNICALL get_DocumentRange(ITextRangeProvider **pRetVal)noexcept->HRESULT override;
        // [getter] SupportedTextSelection
        //auto UNICALL get_SupportedTextSelection(SupportedTextSelection *pRetVal)noexcept->HRESULT override;
    public: // IGridProvider
        // [getter] ColumnCount
        auto UNICALL get_ColumnCount(int *pRetVal)noexcept->HRESULT override;
        // [getter] RowCount
        auto UNICALL get_RowCount(int *pRetVal)noexcept->HRESULT override;
        // get item
        auto UNICALL GetItem(int row, int column, IRawElementProviderSimple **pRetVal)noexcept->HRESULT override;
    public: // IGridItemProvider
        // [getter] Column
        auto UNICALL get_Column(int *pRetVal)noexcept->HRESULT override;
        // [getter] ColumnSpan
        auto UNICALL get_ColumnSpan(int *pRetVal)noexcept->HRESULT override;
        // [getter] ContainingGrid
        auto UNICALL get_ContainingGrid(IRawElementProviderSimple **pRetVal)noexcept->HRESULT override;
        // [getter] Row
        auto UNICALL get_Row(int *pRetVal)noexcept->HRESULT override;
        // [getter] RowSpan
        auto UNICALL get_RowSpan(int *pRetVal)noexcept->HRESULT override;
    public: // ITableProvider
        // get column headers
        auto UNICALL GetColumnHeaders(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // get row headers
        auto UNICALL GetRowHeaders(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // [getter] RowOrColumnMajor
        auto UNICALL get_RowOrColumnMajor(RowOrColumnMajor *pRetVal)noexcept->HRESULT override;
    public: // ITableItemProvider
        // get column header items
        auto UNICALL GetColumnHeaderItems(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // get row header items
        auto UNICALL GetRowHeaderItems(SAFEARRAY **pRetVal)noexcept->HRESULT override;
    public: // IToggleProvider
        // toggle
        auto UNICALL Toggle()noexcept->HRESULT override;
        // [getter] ToggleState
        auto UNICALL get_ToggleState(ToggleState *pRetVal)noexcept->HRESULT override;
    public: // IExpandCollapseProvider
        // collapse
        auto UNICALL Collapse()noexcept->HRESULT override;
        // expand
        auto UNICALL Expand()noexcept->HRESULT override;
        // [getter] ExpandCollapseState
        auto UNICALL get_ExpandCollapseState(ExpandCollapseState *pRetVal)noexcept->HRESULT override;
    public: // ITransformProvider
        // [getter] CanMove
        auto UNICALL get_CanMove(BOOL *pRetVal)noexcept->HRESULT override;
        // [getter] CanResize
        auto UNICALL get_CanResize(BOOL *pRetVal)noexcept->HRESULT override;
        // [getter] CanRotate
        auto UNICALL get_CanRotate(BOOL *pRetVal)noexcept->HRESULT override;
        // move to
        auto UNICALL Move(double x, double y)noexcept->HRESULT override;
        // resize to
        auto UNICALL Resize(double width, double height)noexcept->HRESULT override;
        // rotate to
        auto UNICALL Rotate(double degrees)noexcept->HRESULT override;
    public: // IScrollProvider
        // scroll to
        auto UNICALL Scroll(ScrollAmount horizontalAmount, ScrollAmount verticalAmount)noexcept->HRESULT override;
        // scroll to precent
        auto UNICALL SetScrollPercent(double horizontalPercent, double verticalPercent)noexcept->HRESULT override;
        // [getter] HorizontallyScrollable
        auto UNICALL get_HorizontallyScrollable(BOOL *pRetVal)noexcept->HRESULT override;
        // [getter] HorizontalScrollPercent
        auto UNICALL get_HorizontalScrollPercent(double *pRetVal)noexcept->HRESULT override;
        // [getter] HorizontalViewSize
        auto UNICALL get_HorizontalViewSize(double *pRetVal)noexcept->HRESULT override;
        // [getter] VerticallyScrollable
        auto UNICALL get_VerticallyScrollable(BOOL *pRetVal)noexcept->HRESULT override;
        // [getter] VerticalScrollPercent
        auto UNICALL get_VerticalScrollPercent(double *pRetVal)noexcept->HRESULT override;
        // [getter] VerticalViewSize
        auto UNICALL get_VerticalViewSize(double *pRetVal)noexcept->HRESULT override;
    public: // IScrollItemProvider
        // scroll into view
        auto UNICALL ScrollIntoView()noexcept->HRESULT override;
    public: // IItemContainerProvider

    public: // ILegacyIAccessibleProvider
    protected:
        // control
        UIControl&              m_control;
        // pattern type
        AccessiblePattern       m_pattern;
        // ref count
        std::atomic<uint16_t>   m_cRef;
    };
    /// <summary>
    /// Accessible for window
    /// </summary>
    /// <seealso cref="IRawElementProviderSimple" />
    /// <seealso cref="IRawElementProviderFragment" />
    /// <seealso cref="IRawElementProviderFragmentRoot" />
    class CUIAccessibleWnd :
        public IRawElementProviderSimple,
        public IRawElementProviderFragment,
        public IRawElementProviderFragmentRoot {
    public:
        // interface from window
        static auto FromWindow(CUIWindow& wnd) noexcept->CUIAccessibleWnd*;
        // ctor
        CUIAccessibleWnd(CUIWindow& wnd) noexcept;
        // dtor
        ~CUIAccessibleWnd() noexcept;
        // no copy
        CUIAccessibleWnd(const CUIAccessibleWnd&) noexcept = delete;
    public: // IUnknown
        // add ref-count
        auto UNICALL AddRef() noexcept->ULONG override;
        // release ref-count
        auto UNICALL Release() noexcept->ULONG override;
        // QI
        auto UNICALL QueryInterface(REFIID riid, void**out) noexcept->HRESULT override;
    public: // IRawElementProviderSimple
        // [getter] HostRawElementProvider
        auto UNICALL get_HostRawElementProvider(IRawElementProviderSimple **pRetVal)noexcept->HRESULT override;
        // [getter] ProviderOptions
        auto UNICALL get_ProviderOptions(ProviderOptions *pRetVal)noexcept->HRESULT override;
        // get pattern provider
        auto UNICALL GetPatternProvider(PATTERNID patternId, IUnknown **pRetVal)noexcept->HRESULT override;
        // get property value
        auto UNICALL GetPropertyValue(PROPERTYID propertyId, VARIANT *pRetVal)noexcept->HRESULT override;
    public: // IRawElementProviderFragment
        // [getter] BoundingRectangle
        auto UNICALL get_BoundingRectangle(UiaRect *pRetVal)noexcept->HRESULT override;
        // [getter] FragmentRoot
        auto UNICALL get_FragmentRoot(IRawElementProviderFragmentRoot **pRetVal)noexcept->HRESULT override;
        // get embedded fragment roots
        auto UNICALL GetEmbeddedFragmentRoots(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // retrieves the runtime identifier of an element.
        auto UNICALL GetRuntimeId(SAFEARRAY **pRetVal)noexcept->HRESULT override;
        // retrieves the UI Automation element in a specified direction within the UI Automation tree.
        auto UNICALL Navigate(NavigateDirection direction, IRawElementProviderFragment **pRetVal)noexcept->HRESULT override;
        // set focus
        auto UNICALL SetFocus()noexcept->HRESULT override;
    public: // IRawElementProviderFragmentRoot
        // get the provider of the element that is at the specified point in this fragment.
        auto UNICALL ElementProviderFromPoint(double x, double y, IRawElementProviderFragment **pRetVal)noexcept->HRESULT override;
        // get the element in this fragment that has the input focus.
        auto UNICALL GetFocus(IRawElementProviderFragment **pRetVal)noexcept->HRESULT override;
    protected:
        // window ref
        CUIWindow&          m_window;
        // ref count
        std::atomic<ULONG>  m_cRef;
    };
}

#endif