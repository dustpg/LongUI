// Gui
#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#include <accessible/ui_accessible_win.h>
#include <control/ui_viewport.h>
#include <control/ui_control.h>
#include <util/ui_unicode.h>
#include <core/ui_manager.h>
#include <core/ui_window.h>
#include <core/ui_string.h>

// C++
#include <cassert>

// Lib
#ifdef LUI_ACCESSIBLE
#pragma comment(lib, "Uiautomationcore")


namespace LongUI {
    /// <summary>
    /// Finalizes the accessible.
    /// </summary>
    /// <param name="">The .</param>
    /// <returns></returns>
    void FinalizeAccessible(CUIAccessible& obj) noexcept {
        // FIXME: Memory leak -> obj.Release();
        delete &obj;
    }
    /// <summary>
    /// Finalizes the accessible.
    /// </summary>
    /// <param name="">The .</param>
    /// <returns></returns>
    void FinalizeAccessible(CUIAccessibleWnd& obj) noexcept {
        // FIXME: Memory leak -> obj.Release();
        delete &obj;
    }
    // bstring from View
    inline BSTR BStrFromView(const U16View view) noexcept {
        const auto len = static_cast<uint32_t>(view.end() - view.begin());
        return ::SysAllocStringLen(impl::sys(view.begin()), len);
    }
    // bstring from string
    inline BSTR BStrFromString(const CUIString& str) noexcept {
        const auto len = static_cast<uint32_t>(str.length());
        return ::SysAllocStringLen(impl::sys(str.c_str()), len);
    }
}

/// <summary>
/// Froms the control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::FromControl(UIControl& ctrl)noexcept->CUIAccessible* {
    auto& accessible = ctrl.m_pAccessible;
    if (!accessible) accessible = new(std::nothrow) CUIAccessible{ ctrl };
    return accessible;
}

/// <summary>
/// Patterns from cotnrol.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
inline auto LongUI::CUIAccessible::pattern_from_cotnrol(
    UIControl & ctrl) noexcept -> AccessiblePattern {
    AccessibleGetPatternsArg arg;
    static_assert(sizeof(arg) == 8, "bad struct layout");
    const auto rv = ctrl.accessible(arg);
    assert(rv == Event_Accept && "cannot ignore this event");
    return arg.patterns;
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIAccessible"/> class.
/// </summary>
/// <param name="ctrl">The control.</param>
LongUI::CUIAccessible::CUIAccessible(UIControl& ctrl) noexcept :
m_control(ctrl), m_pattern(pattern_from_cotnrol(ctrl)), m_cRef(1){
}


/// <summary>
/// Finalizes an instance of the <see cref="CUIAccessible"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIAccessible::~CUIAccessible() noexcept {
}



// helper marco
#define LUI_TRACE {OutputDebugStringA(__FUNCTION__); OutputDebugStringA("\r\n");}
#define LUI_QI_HELPER(x) else if (riid == IID_##x) ptr = static_cast<x*>(this)
#define LUI_PATTERN(x, X) UIA_##X##PatternId:\
        if (this->is_support_##x()) rv = static_cast<I##X##Provider*>(this);


// ----------------------------------------------------------------------------
// -----------------------------  IUnknown  -----------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Adds the reference.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::AddRef() noexcept -> ULONG {
    const auto ov = m_cRef.fetch_add(1, std::memory_order_relaxed);
    return static_cast<ULONG>(ov + 1);
}

/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::Release() noexcept -> ULONG {
    const auto ov = m_cRef.fetch_sub(1, std::memory_order_relaxed);
    const auto rv = static_cast<ULONG>(ov - 1);
    if (!rv) delete this;
    return rv;
}

/// <summary>
/// Queries the interface.
/// </summary>
/// <param name="riid">The riid.</param>
/// <param name="out">The out.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::QueryInterface( REFIID riid, 
                                        void ** out) noexcept -> HRESULT {
    using unk_base = IRawElementProviderSimple;
    IUnknown* ptr = nullptr;
    // 接口列表
    if (riid == IID_IUnknown) ptr = static_cast<unk_base*>(this);
    LUI_QI_HELPER(IRawElementProviderSimple);
    LUI_QI_HELPER(IRawElementProviderFragment);
    //LUI_QI_HELPER(IRawElementProviderFragmentRoot);
    LUI_QI_HELPER(IRawElementProviderAdviseEvents);
    LUI_QI_HELPER(IInvokeProvider);
    LUI_QI_HELPER(ISelectionProvider);
    LUI_QI_HELPER(ISelectionItemProvider);
    LUI_QI_HELPER(IRangeValueProvider);
    LUI_QI_HELPER(IValueProvider);
    //LUI_QI_HELPER(ITextProvider);
    LUI_QI_HELPER(IGridProvider);
    LUI_QI_HELPER(IGridItemProvider);
    LUI_QI_HELPER(ITableProvider);
    LUI_QI_HELPER(ITableItemProvider);
    LUI_QI_HELPER(IToggleProvider);
    LUI_QI_HELPER(IExpandCollapseProvider);
    LUI_QI_HELPER(ITransformProvider);
    LUI_QI_HELPER(IScrollProvider);
    LUI_QI_HELPER(IScrollItemProvider);
    // IItemContainerProvider
    //LUI_QI_HELPER(ILegacyIAccessibleProvider);
    // 有效
    if (ptr) { this->AddRef(); *out = ptr; return S_OK; }
    *out = nullptr;
    return E_NOINTERFACE;
}



// ----------------------------------------------------------------------------
// ---------------------  IRawElementProviderSimple  --------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Gets the host raw element provider.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_HostRawElementProvider(
    IRawElementProviderSimple ** pRetVal) noexcept -> HRESULT {
    
    

    //if (!pRetVal) return E_INVALIDARG;
    *pRetVal = nullptr;
    return S_OK;
    /*if (m_control.IsTopLevel()) {
        if (const auto wnd = m_control.GetWindow()) {
            const auto hwnd = wnd->GetHwnd();
            return ::UiaHostProviderFromHwnd(hwnd, pRetVal);
        }
    }
    return UIA_E_ELEMENTNOTAVAILABLE;*/
}

/// <summary>
/// Gets the provider options.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_ProviderOptions(
    ProviderOptions * pRetVal) noexcept -> HRESULT {
    
    *pRetVal = ProviderOptions_ServerSideProvider
        //| ProviderOptions_RefuseNonClientSupport
        ;
    return S_OK;
}

/// <summary>
/// Gets the pattern provider.
/// </summary>
/// <param name="patternId">The pattern identifier.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetPatternProvider(
    PATTERNID patternId, IUnknown ** pRetVal) noexcept -> HRESULT {
    
    IUnknown* rv = nullptr;
    switch (patternId)
    {
        // Invoke
        case LUI_PATTERN(invoke, Invoke)                break;
        // Selection
        case LUI_PATTERN(selection, Selection)          break;
        // Value
        case LUI_PATTERN(value, Value)                  break;
        // RangeValue
        case LUI_PATTERN(range_value, RangeValue)       break;
        // Scroll
        case LUI_PATTERN(scroll, Scroll)                break;
        // ExpandCollapse
        case LUI_PATTERN(expand_collapse, ExpandCollapse)   break;
        // Grid
        case LUI_PATTERN(grid, Grid)                    break;
        // GridItem
        case LUI_PATTERN(grid_item, GridItem)           break;
        // MultipleView
        //case LUI_PATTERN(multiple_view, MultipleView) break;
        // Window
        //case LUI_PATTERN(window, Window)              break;

        // SelectionItem
        case LUI_PATTERN(selection_item, SelectionItem) break;
        // Dock
        //case LUI_PATTERN(dock, Dock)                  break;

        // Table
        case LUI_PATTERN(table, Table)                  break;
        // TableItem
        case LUI_PATTERN(table_item, TableItem)         break;
        // Text
        //case LUI_PATTERN(text, Text)                    break;
        // Toggle
        case LUI_PATTERN(toggle, Toggle)                break;
        // Transform
        case LUI_PATTERN(transform, Transform)          break;
        // ScrollItem
        case LUI_PATTERN(scroll_item, ScrollItem)       break;
        // LegacyIAccessible
        //case LUI_PATTERN(legacy_interface, LegacyIAccessible) break;
        
        // ItemContainer
        // VirtualizedItem
        // SynchronizedInput

        // --- Win8
    }
    if (*pRetVal = rv) this->AddRef();
    return S_OK;
}

// LongUI::impl
namespace LongUI { namespace impl {
    // 映射表
    static const int8_t ctrl_type_map[] = {
        UIA_CustomControlTypeId         - UIA_ButtonControlTypeId,
        UIA_ButtonControlTypeId         - UIA_ButtonControlTypeId,
        UIA_CalendarControlTypeId       - UIA_ButtonControlTypeId,
        UIA_CheckBoxControlTypeId       - UIA_ButtonControlTypeId,
        UIA_ComboBoxControlTypeId       - UIA_ButtonControlTypeId,
        UIA_EditControlTypeId           - UIA_ButtonControlTypeId,
        UIA_HyperlinkControlTypeId      - UIA_ButtonControlTypeId,
        UIA_ImageControlTypeId          - UIA_ButtonControlTypeId,
        UIA_ListControlTypeId           - UIA_ButtonControlTypeId,
        UIA_ListItemControlTypeId       - UIA_ButtonControlTypeId,
        UIA_MenuControlTypeId           - UIA_ButtonControlTypeId,
        UIA_MenuBarControlTypeId        - UIA_ButtonControlTypeId,
        UIA_MenuItemControlTypeId       - UIA_ButtonControlTypeId,
        UIA_ProgressBarControlTypeId    - UIA_ButtonControlTypeId,
        UIA_RadioButtonControlTypeId    - UIA_ButtonControlTypeId,
        UIA_ScrollBarControlTypeId      - UIA_ButtonControlTypeId,
        UIA_SliderControlTypeId         - UIA_ButtonControlTypeId,
        UIA_SpinnerControlTypeId        - UIA_ButtonControlTypeId,
        UIA_TabControlTypeId            - UIA_ButtonControlTypeId,
        UIA_TabItemControlTypeId        - UIA_ButtonControlTypeId,
        UIA_TextControlTypeId           - UIA_ButtonControlTypeId,
        UIA_ToolBarControlTypeId        - UIA_ButtonControlTypeId,
        UIA_ToolTipControlTypeId        - UIA_ButtonControlTypeId,
        UIA_TreeControlTypeId           - UIA_ButtonControlTypeId,
        UIA_TreeItemControlTypeId       - UIA_ButtonControlTypeId,
        UIA_GroupControlTypeId          - UIA_ButtonControlTypeId,
        UIA_ThumbControlTypeId          - UIA_ButtonControlTypeId,
        UIA_HeaderControlTypeId         - UIA_ButtonControlTypeId,
        UIA_HeaderItemControlTypeId     - UIA_ButtonControlTypeId,
        UIA_TableControlTypeId          - UIA_ButtonControlTypeId,
    };

}}


/// <summary>
/// Gets the property value.
/// </summary>
/// <param name="propertyId">The property identifier.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetPropertyValue(PROPERTYID propertyId,
    VARIANT* pRetVal) noexcept -> HRESULT {
    constexpr auto MAX_TYPE = AccessibleControlType::Type_None;
    constexpr auto TYPE_MAX = static_cast<uint32_t>(MAX_TYPE);
    static_assert(TYPE_MAX == sizeof(impl::ctrl_type_map), "must be same");
    CUIDataAutoLocker locker;
    CUIString string;
    // 获取控件类型
    auto get_control_type = [](UIControl& ctrl) noexcept {
        AccessibleGetCtrlTypeArg arg;
        ctrl.accessible(arg);
        return arg.type;
    };
    // 获取控件Acc名称
    auto get_accessible_name = [](UIControl& ctrl, CUIString& name) noexcept {
        AccessibleGetAccNameArg arg{ name };
        ctrl.accessible(arg);
    };
    // 获取控件描述
    auto get_description = [](UIControl& ctrl, CUIString& name) noexcept {
        AccessibleGetDescriptionArg arg{ name };
        ctrl.accessible(arg);
    };
    // msdn.microsoft.com/en-us/library/windows/desktop/ee671212(v=vs.85).aspx
    switch (propertyId)
    {
        AccessibleControlType type;
    case UIA_ControlTypePropertyId:
        type = get_control_type(m_control);
        // 类型有效
        if (uint32_t(type) < TYPE_MAX) {
            pRetVal->vt = VT_I4;
            const auto offset = impl::ctrl_type_map[uint32_t(type)];
            pRetVal->lVal = offset + UIA_ButtonControlTypeId;
        }
        // 类型无效
        else pRetVal->vt = VT_EMPTY;
        break;
    case UIA_NamePropertyId:
        pRetVal->vt = VT_BSTR;
        get_accessible_name(m_control, string);
        pRetVal->bstrVal = LongUI::BStrFromString(string);
        break;
    case UIA_ProviderDescriptionPropertyId:
        pRetVal->vt = VT_BSTR;
        get_description(m_control, string);
        pRetVal->bstrVal = LongUI::BStrFromString(string);
        break;
    default:
        pRetVal->vt = VT_EMPTY;
    }
    return S_OK;
}


// ----------------------------------------------------------------------------
// ---------------------  IRawElementProviderFragment  ------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Gets the bounding rectangle.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_BoundingRectangle(
    UiaRect* pRetVal) noexcept -> HRESULT {
    if (!pRetVal) return E_INVALIDARG;
    CUIDataAutoLocker locker;
    if (m_control.IsVisibleEx()) {
        const auto size = m_control.GetBoxSize();
        RectF rect = { 0, 0, size.width, size.height };
        m_control.MapToWindow(rect);
        const auto wnd = m_control.GetWindow();
        assert(wnd && "bad window");
        wnd->MapToScreen(rect);
        pRetVal->left = rect.left;
        pRetVal->top = rect.top;
        pRetVal->width = rect.right - rect.left;
        pRetVal->height = rect.bottom - rect.top;
    }
    else *pRetVal = {};
    return S_OK;
}


/// <summary>
/// Gets the fragment root.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_FragmentRoot(
    IRawElementProviderFragmentRoot ** pRetVal) noexcept -> HRESULT {

    // 获取承载窗口
    if (const auto wnd = m_control.GetWindow()) {
        const auto ptr = CUIAccessibleWnd::FromWindow(*wnd);
        if (!ptr) return E_OUTOFMEMORY;
        ptr->AddRef();
        *pRetVal = ptr;
        return S_OK;
    }
    // 没有窗口
    assert(!"NO WINDOW");
    *pRetVal = nullptr;
    return E_FAIL;
}

/// <summary>
/// Gets the embedded fragment roots.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetEmbeddedFragmentRoots(
    SAFEARRAY ** pRetVal) noexcept -> HRESULT {
    
    *pRetVal = nullptr;
    return S_OK;
}

/// <summary>
/// Gets the runtime identifier.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetRuntimeId(SAFEARRAY** pRetVal) noexcept -> HRESULT {

    *pRetVal = nullptr;
    return S_OK;
}

/// <summary>
/// Navigates the specified direction.
/// </summary>
/// <param name="direction">The direction.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::Navigate(NavigateDirection direction,
    IRawElementProviderFragment ** pRetVal) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(pRetVal && "bad pointer");
    UIControl* ctrl = nullptr;
    switch (direction)
    {
    case NavigateDirection_Parent:
        if (m_control.IsTopLevel()) {
            if (const auto wnd = m_control.GetWindow()) {
                const auto ptr = CUIAccessibleWnd::FromWindow(*wnd);
                if (!ptr) return E_OUTOFMEMORY;
                ptr->AddRef();
                *pRetVal = ptr;
                return S_OK;
            }
        }
        else ctrl = m_control.GetParent();
        break;
    case NavigateDirection_NextSibling:
        if (m_control.GetParent() && !m_control.IsLastChild())
            ctrl = static_cast<UIControl*>(m_control.next);
        break;
    case NavigateDirection_PreviousSibling:
        if (m_control.GetParent() && !m_control.IsFirstChild())
            ctrl = static_cast<UIControl*>(m_control.prev);
        break;
    case NavigateDirection_FirstChild:
        if (const auto logic = m_control.GetLogicAccessibleControl())
            if (logic->GetChildrenCount())
                ctrl = static_cast<UIControl*>(logic->m_oHead.next);
        break;
    case NavigateDirection_LastChild:
        if (const auto logic = m_control.GetLogicAccessibleControl())
            if (logic->GetChildrenCount())
                ctrl = static_cast<UIControl*>(logic->m_oHead.prev);
        break;
    }
    // 存在节点
    if (ctrl) {
        const auto ptr = CUIAccessible::FromControl(*ctrl);
        if (!ptr) return E_OUTOFMEMORY;
        ptr->AddRef();
        *pRetVal = ptr;
        return S_OK;
    }
    // 木有节点
    else *pRetVal = nullptr;
    return E_FAIL;
}

/// <summary>
/// Sets the focus.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::SetFocus() noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    return m_control.SetFocus() ? S_OK : E_NOTIMPL;
}


// ----------------------------------------------------------------------------
// -------------------  IRawElementProviderAdviseEvents  ----------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Advises the event added.
/// </summary>
/// <param name="eventId">The event identifier.</param>
/// <param name="propertyIDs">The property i ds.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::AdviseEventAdded(
    EVENTID eventId, SAFEARRAY * propertyIDs) noexcept -> HRESULT {
    
    // TODO: 移除或者实现
    return E_NOTIMPL;
}


/// <summary>
/// Advises the event removed.
/// </summary>
/// <param name="eventId">The event identifier.</param>
/// <param name="propertyIDs">The property i ds.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::AdviseEventRemoved(
    EVENTID eventId, SAFEARRAY * propertyIDs) noexcept -> HRESULT {
    
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// ---------------------------  IInvokeProvider  ------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Invokes this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::Invoke() noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_invoke());
    m_control.accessible({ Event_Invoke_Invoke });
    return S_OK;
}


// ----------------------------------------------------------------------------
// -------------------------  ISelectionProvider  -----------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Gets the selection.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetSelection(SAFEARRAY** pRetVal) noexcept -> HRESULT {
    // + ISelectionProvider
    // + ITextProvider
    return E_NOTIMPL;
}

/// <summary>
/// Gets the can select multiple.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_CanSelectMultiple(BOOL*pRetVal)noexcept->HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the is selection required.
/// </summary>
/// <param name="rv">The rv.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_IsSelectionRequired(BOOL* rv) noexcept->HRESULT {
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// -----------------------  ISelectionItemProvider  ---------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Selects this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::Select() noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Adds to selection.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::AddToSelection() noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Removes from selection.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::RemoveFromSelection() noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the is selected.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_IsSelected(BOOL* pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the selection container.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_SelectionContainer(
    IRawElementProviderSimple ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}


// ----------------------------------------------------------------------------
// -------------------------  IRangeValueProvider  ----------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Sets the value.
/// </summary>
/// <param name="val">The value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::SetValue(double val) noexcept -> HRESULT {
    m_control.accessible(AccessibleRSetValueArg{ val });
    return S_OK;
}

/// <summary>
/// Gets the value.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_Value(double * pRetVal) noexcept -> HRESULT {
    AccessibleRGetValueArg arg;
    m_control.accessible(arg);
    *pRetVal = arg.value;
    return S_OK;
}

/// <summary>
/// Gets the is read only.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_IsReadOnly(BOOL * pRetVal) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    // + IRangeValueProvider
    // + IValueProvider
    assert(this->is_support_value() || this->is_support_range_value());
    *pRetVal = m_control.accessible({ Event_RangeValue_IsReadOnly });
    static_assert(int(Event_Accept), "Event_Accept need be true");
    return S_OK;
}

/// <summary>
/// Gets the maximum.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_Maximum(double * pRetVal) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_range_value());
    AccessibleRGetMaxArg arg;
    m_control.accessible(arg);
    *pRetVal = arg.value;
    return S_OK;
}

/// <summary>
/// Gets the minimum.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_Minimum(double * pRetVal) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_range_value());
    AccessibleRGetMinArg arg;
    m_control.accessible(arg);
    *pRetVal = arg.value;
    return S_OK;
}

/// <summary>
/// Gets the large change.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_LargeChange(double * pRetVal) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_range_value());
    AccessibleRGetLargeStepArg arg;
    m_control.accessible(arg);
    *pRetVal = arg.value;
    return S_OK;
}


/// <summary>
/// Gets the small change.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_SmallChange(double * pRetVal) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_range_value());
    AccessibleRGetSmallStepArg arg;
    m_control.accessible(arg);
    *pRetVal = arg.value;
    return S_OK;
}


// ----------------------------------------------------------------------------
// ---------------------------  IValueProvider  -------------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Sets the value.
/// </summary>
/// <param name="val">The value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::SetValue(LPCWSTR val) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    //assert(!"UNTESTED");
    assert(this->is_support_value());
    AccessibleVSetValueArg arg{ impl::sys(val), static_cast<uint32_t>(std::wcslen(val)) };
    m_control.accessible(arg);
    return S_OK;
}


/// <summary>
/// Gets the value.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_Value(BSTR * pRetVal) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_value());
    CUIString value;
    AccessibleVGetValueArg arg{ value };
    m_control.accessible(arg);
    *pRetVal = LongUI::BStrFromString(value);
    return S_OK;
}


// ----------------------------------------------------------------------------
// ----------------------------  ITextProvider  -------------------------------
// ----------------------------------------------------------------------------

#if 0
/// <summary>
/// Gets the visible ranges.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetVisibleRanges(SAFEARRAY** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Ranges from child.
/// </summary>
/// <param name="childElement">The child element.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::RangeFromChild(IRawElementProviderSimple * childElement, ITextRangeProvider ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Ranges from point.
/// </summary>
/// <param name="point">The point.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::RangeFromPoint(UiaPoint point, ITextRangeProvider ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the document range.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_DocumentRange(ITextRangeProvider ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the supported text selection.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_SupportedTextSelection(SupportedTextSelection * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}
#endif

// ----------------------------------------------------------------------------
// ----------------------------  IGridProvider  -------------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Gets the column count.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_ColumnCount(int * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the row count.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_RowCount(int * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the item.
/// </summary>
/// <param name="row">The row.</param>
/// <param name="column">The column.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetItem(int row, int column, IRawElementProviderSimple ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// --------------------------  IGridItemProvider  -----------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Gets the column.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_Column(int * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the column span.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_ColumnSpan(int * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the containing grid.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_ContainingGrid(IRawElementProviderSimple ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the row.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_Row(int * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the row span.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_RowSpan(int * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// ---------------------------  ITableProvider  -------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Gets the column headers.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetColumnHeaders(SAFEARRAY ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the row headers.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetRowHeaders(SAFEARRAY ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the row or column major.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_RowOrColumnMajor(RowOrColumnMajor * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// -------------------------  ITableItemProvider  -----------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Gets the column header items.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetColumnHeaderItems(SAFEARRAY ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}


/// <summary>
/// Gets the row header items.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::GetRowHeaderItems(SAFEARRAY ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}


// ----------------------------------------------------------------------------
// ---------------------------  IToggleProvider  ------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Toggles this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::Toggle() noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_toggle());
    m_control.accessible({ Event_Toggle_Toggle });
    return S_OK;
}


/// <summary>
/// Gets the state of the toggle.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_ToggleState(ToggleState* rv) noexcept -> HRESULT {
    CUIDataAutoLocker locker;
    assert(this->is_support_toggle());
    const auto& style = m_control.RefStyle();
    const auto indeterminate = [&style]() noexcept {
        return style.state & State_Indeterminate;
    };
    const auto checked = [&style]() noexcept {
        return style.state & State_Checked;
    };
    const auto state = indeterminate() ? ToggleState_Indeterminate :
        (checked() ? ToggleState_On : ToggleState_Off);
    *rv = state;
    return S_OK;
}


// ----------------------------------------------------------------------------
// -----------------------  IExpandCollapseProvider  --------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Collapses this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::Collapse() noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Expands this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::Expand() noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the state of the expand collapse.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_ExpandCollapseState(
    ExpandCollapseState * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// -------------------------  ITransformProvider  -----------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Gets the can move.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_CanMove(BOOL * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the can resize.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_CanResize(BOOL * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the can rotate.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_CanRotate(BOOL * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Moves the specified x.
/// </summary>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::Move(double x, double y) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Resizes the specified width.
/// </summary>
/// <param name="width">The width.</param>
/// <param name="height">The height.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::Resize(double width, double height) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Rotates the specified degrees.
/// </summary>
/// <param name="degrees">The degrees.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::Rotate(double degrees) noexcept -> HRESULT {
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// ---------------------------  IScrollProvider  ------------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Scrolls the specified horizontal amount.
/// </summary>
/// <param name="horizontalAmount">The horizontal amount.</param>
/// <param name="verticalAmount">The vertical amount.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::Scroll(ScrollAmount horizontalAmount,
    ScrollAmount verticalAmount) noexcept -> HRESULT {
    return E_NOTIMPL;
}


/// <summary>
/// Sets the scroll percent.
/// </summary>
/// <param name="horizontalPercent">The horizontal percent.</param>
/// <param name="verticalPercent">The vertical percent.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::SetScrollPercent(double horizontalPercent,
    double verticalPercent) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the horizontally scrollable.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_HorizontallyScrollable(
    BOOL * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}


/// <summary>
/// Gets the horizontal scroll percent.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_HorizontalScrollPercent(
    double * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}


/// <summary>
/// Gets the size of the horizontal view.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_HorizontalViewSize(
    double * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the vertically scrollable.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_VerticallyScrollable(
    BOOL * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the vertical scroll percent.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_VerticalScrollPercent(
    double * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the size of the vertical view.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessible::get_VerticalViewSize(
    double * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}


// ----------------------------------------------------------------------------
// -------------------------  IScrollItemProvider  ----------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Scrolls the into view.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessible::ScrollIntoView() noexcept -> HRESULT {
    return E_NOTIMPL;
}



















/// <summary>
/// Initializes a new instance of the <see cref="CUIAccessibleWnd"/> class.
/// </summary>
/// <param name="wnd">The WND.</param>
LongUI::CUIAccessibleWnd::CUIAccessibleWnd(CUIWindow & wnd) noexcept
    :m_window(wnd), m_cRef(1) {
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIAccessibleWnd"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIAccessibleWnd::~CUIAccessibleWnd() noexcept {
    int bk = 9;
}

/// <summary>
/// Queries the interface.
/// </summary>
/// <param name="riid">The riid.</param>
/// <param name="out">The out.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::QueryInterface(
    REFIID riid, void ** out) noexcept -> HRESULT {
    using unk_base = IRawElementProviderFragmentRoot;
    IUnknown* ptr = nullptr;
    // 接口列表
    if (riid == IID_IUnknown) ptr = static_cast<unk_base*>(this);
    LUI_QI_HELPER(IRawElementProviderSimple);
    LUI_QI_HELPER(IRawElementProviderFragment);
    LUI_QI_HELPER(IRawElementProviderFragmentRoot);
    // 有效
    if (ptr) { this->AddRef(); *out = ptr; return S_OK; }
    *out = nullptr;
    return E_NOINTERFACE;
}


/// <summary>
/// Froms the window.
/// </summary>
/// <param name="wnd">The WND.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::FromWindow(
    CUIWindow& wnd) noexcept -> CUIAccessibleWnd * {
    auto& accessible = wnd.m_pAccessible;
    if (!accessible) accessible = new(std::nothrow) CUIAccessibleWnd{ wnd };
    return accessible;
}


/// <summary>
/// Adds the reference.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::AddRef() noexcept -> ULONG {
    const auto ov = m_cRef.fetch_add(1, std::memory_order_relaxed);
    return static_cast<ULONG>(ov + 1);
}

/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::Release() noexcept -> ULONG {
    const auto ov = m_cRef.fetch_sub(1, std::memory_order_relaxed);
    const auto rv = static_cast<ULONG>(ov - 1);
    if (!rv) delete this;
    return rv;
}



/// <summary>
/// Gets the bounding rectangle.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto UNICALL LongUI::CUIAccessibleWnd::get_BoundingRectangle(
    UiaRect * pRetVal) noexcept -> HRESULT {
    if (!pRetVal) return E_INVALIDARG;
    const auto pos = m_window.GetPos();
    const auto size = m_window.GetAbsoluteSize();
    pRetVal->top = static_cast<double>(pos.x);
    pRetVal->left = static_cast<double>(pos.y);
    pRetVal->width = static_cast<double>(size.width);
    pRetVal->height = static_cast<double>(size.height);
    return S_OK;
}

/// <summary>
/// Gets the fragment root.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::get_FragmentRoot(
    IRawElementProviderFragmentRoot ** pRetVal) noexcept -> HRESULT {
    *pRetVal = this;
    this->AddRef();
    return S_OK;
}

/// <summary>
/// Gets the embedded fragment roots.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::GetEmbeddedFragmentRoots(
    SAFEARRAY ** pRetVal) noexcept -> HRESULT {
    *pRetVal = nullptr;
    return S_OK;
}

/// <summary>
/// Gets the runtime identifier.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::GetRuntimeId(
    SAFEARRAY ** pRetVal) noexcept -> HRESULT {
    *pRetVal = nullptr;
    return S_OK;
}

/// <summary>
/// Navigates the specified direction.
/// </summary>
/// <param name="direction">The direction.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::Navigate(NavigateDirection direction,
    IRawElementProviderFragment ** pRetVal) noexcept -> HRESULT {
    auto& ctrl = m_window.RefViewport();
    switch (direction) {
    case NavigateDirection_FirstChild:
    case NavigateDirection_LastChild:
        if (const auto ptr = CUIAccessible::FromControl(ctrl)) {
            ptr->AddRef();
            *pRetVal = ptr;
            return S_OK;
        }
        else return E_OUTOFMEMORY;
    default:
        *pRetVal = nullptr;
        return S_OK;
    }
}

/// <summary>
/// Sets the focus.
/// </summary>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::SetFocus() noexcept -> HRESULT {
    m_window.ActiveWindow();
    return S_OK;
}


/// <summary>
/// Gets the host raw element provider.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::get_HostRawElementProvider(
    IRawElementProviderSimple ** pRetVal) noexcept -> HRESULT {
    const auto hwnd = reinterpret_cast<HWND>(m_window.GetRawHandle());
    return ::UiaHostProviderFromHwnd(hwnd, pRetVal);
}

/// <summary>
/// Gets the provider options.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::get_ProviderOptions(
    ProviderOptions * pRetVal) noexcept -> HRESULT {
    *pRetVal = ProviderOptions_ServerSideProvider;
    return S_OK;
}


/// <summary>
/// Gets the pattern provider.
/// </summary>
/// <param name="patternId">The pattern identifier.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::GetPatternProvider(
    PATTERNID patternId, IUnknown ** pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

/// <summary>
/// Gets the property value.
/// </summary>
/// <param name="propertyId">The property identifier.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::GetPropertyValue(
    PROPERTYID propertyId, VARIANT * pRetVal) noexcept -> HRESULT {
    return E_NOTIMPL;
}

// ----------------------------------------------------------------------------
// -------------------  IRawElementProviderFragmentRoot  ----------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Elements the provider from point.
/// </summary>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::ElementProviderFromPoint(double x, double y,
    IRawElementProviderFragment** pRetVal) noexcept->HRESULT {
    CUIDataAutoLocker locker;

    *pRetVal = nullptr;
    // 将屏幕坐标映射到窗口上
    Point2F pos{ static_cast<float>(x), static_cast<float>(y) };
    m_window.MapFromScreen(pos);
    // 拥有逻辑子控件
    const auto is_has_logical_child = [](UIControl* ctrl) noexcept {
        const auto logical = ctrl->GetLogicAccessibleControl();
        return logical && logical->GetChildrenCount();
    };
    // 查找控件
    UIControl* ctrl = &m_window.RefViewport();
    while (is_has_logical_child(ctrl)) {
        const auto child = ctrl->FindChild(pos);
        if (!child) break;
        ctrl = child;
    }
    // 获取Accessible接口
    const auto ptr = CUIAccessible::FromControl(*ctrl);
    if (!ptr) return E_OUTOFMEMORY;
    ptr->AddRef();
    *pRetVal = ptr;
    return S_OK;
}

/// <summary>
/// Gets the focus.
/// </summary>
/// <param name="pRetVal">The p ret value.</param>
/// <returns></returns>
auto LongUI::CUIAccessibleWnd::GetFocus(
    IRawElementProviderFragment ** pRetVal) noexcept -> HRESULT {
    *pRetVal = nullptr;
    return S_OK;
}

/// <summary>
/// Accessibles the specified object.
/// </summary>
/// <param name="obj">The object.</param>
/// <param name="cb">The cb.</param>
/// <returns></returns>
void LongUI::Accessible(CUIAccessible* obj, AccessibleCallback cb) noexcept {
    if (!obj) return;
    switch (cb)
    {
    case LongUI::Callback_PropertyChanged:
        ::UiaRaiseAutomationEvent(obj, UIA_AutomationPropertyChangedEventId);
        break;
    case LongUI::Callback_Invoked:
        ::UiaRaiseAutomationEvent(obj, UIA_Invoke_InvokedEventId);
        break;
    default:
        assert(!"unknown callback");
    }
}

#endif
