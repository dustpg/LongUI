#include <control/ui_tab.h>
#include <control/ui_tabs.h>
#include <control/ui_tabbox.h>
#include <control/ui_tabpanel.h>
#include <control/ui_tabpanels.h>


#include <core/ui_ctrlmeta.h>
#include <constexpr/const_bkdr.h>
#include <debugger/ui_debug.h>

#include <algorithm>
#include <cassert>

#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#include <core/ui_string.h>
#endif

// Private
#include <core/ui_unsafe.h>
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UITab类 元信息
    LUI_CONTROL_META_INFO(UITab, "tab");
    // UITabs类 元信息
    LUI_CONTROL_META_INFO(UITabs, "tabs");
    // UITabBox类 元信息
    LUI_CONTROL_META_INFO(UITabBox, "tabbox");
    // UITabPanel类 元信息
    LUI_CONTROL_META_INFO(UITabPanel, "tabpanel");
    // UITabPanels类 元信息
    LUI_CONTROL_META_INFO(UITabPanels, "tabpanels");
    // find greatest child-minsize
    //namespace impl { Size2F greatest_minsize(UIControl& ctrl) noexcept; }
}


// ----------------------------------------------------------------------------
// --------------------              Tab              -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITab"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITab::~UITab() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 调试不显示warning
#ifndef NDEBUG
    if (m_pParent)
#endif
    // 移除上层引用
    if (const auto tabs = longui_cast<UITabs*>(m_pParent)) {
        tabs->TabRemoved(*this);
    }
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
void LongUI::UITab::SetText(const CUIString& str) noexcept {
    m_oLabel.SetText(std::move(str));
}

/// <summary>
/// Initializes a new instance of the <see cref="UITab"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITab::UITab(const MetaControl& meta) noexcept : Super( meta),
    m_oImage(this), m_oLabel(this) {
    // 水平布局
    m_state.orient = Orient_Horizontal;
    m_oStyle.appearance = Appearance_Tab;
    // 阻隔鼠标事件写入false之前需要写入
    m_oImage.RefInheritedMask() = State_MouseCutInher;
    m_oLabel.RefInheritedMask() = State_MouseCutInher;
    // 阻隔鼠标事件
    m_state.mouse_continue = false;
    this->make_offset_tf_direct(m_oLabel);
#ifndef NDEBUG
    m_oImage.name_dbg = "tab::image";
    m_oLabel.name_dbg = "tab::label";
    assert(m_oImage.IsFocusable() == false);
    assert(m_oLabel.IsFocusable() == false);
#endif
    // 独立控件
#ifdef LUI_ACCESSIBLE
    m_pAccCtrl = nullptr;
#endif
}



/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UITab::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        break;
    default:
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITab::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonDown:
        this->SetSelected();
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}


/// <summary>
/// update with reason
/// </summary>
/// <param name="reason"></param>
/// <returns></returns>
void LongUI::UITab::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged)
        m_oLabel.Update(r);
    return Super::Update(reason);
}


/// <summary>
/// Sets the selected.
/// </summary>
/// <returns></returns>
void LongUI::UITab::SetSelected() noexcept {
    // XXX: 应该用uisafe_cast?
    if (const auto tabs = longui_cast<UITabs*>(m_pParent)) {
        tabs->SetSelectedTab(*this);
    }
}



#ifdef LUI_ACCESSIBLE

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UITab::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetCtrlTypeArg;
        using get2_t = AccessibleGetAccNameArg;
        case AccessibleEvent::Event_GetPatterns:
            // + 继承基类行为模型
            Super::accessible(args);
            static_cast<const get0_t&>(args).patterns |=
                // + 可调用的行为模型
                Pattern_Invoke
                ;
            return Event_Accept;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =
            AccessibleControlType::Type_TabItem;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称
        *static_cast<const get2_t&>(args).name =
            m_oLabel.RefText();
        return Event_Accept;
    case AccessibleEvent::Event_Invoke_Invoke:
        // 调用
        this->SetSelected();
        return Event_Accept;
    }
    return Super::accessible(args);
}

#endif

// ----------------------------------------------------------------------------
// --------------------             Tabs              -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITabs"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITabs::~UITabs() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UITabs"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITabs::UITabs(const MetaControl& meta) noexcept : Super(meta) {
    // TODO: 可以垂直布局

    // 水平布局
    m_state.orient = Orient_Horizontal;
}

/// <summary>
/// Sets the selected tab.
/// </summary>
/// <param name="tab">The tab.</param>
/// <returns></returns>
void LongUI::UITabs::SetSelectedTab(UITab& tab) noexcept {
    // 不需要选择相同的
    if (m_pLastSelected == &tab) return;
    // 取消选择动画
    if (const auto ptr = m_pLastSelected) 
        ptr->StartAnimation({ State_Selected, State_Non });
    // 确定选择动画
    m_pLastSelected = &tab;
    tab.StartAnimation({ State_Selected, State_Selected });
    // 清除后标记
    {
        const auto b = this->begin();
        const auto e = Iterator{ &tab };
        for (auto itr = b; itr != e; ++itr) {
            auto& refc = *itr;
            longui_cast<UITab*>(&refc)->ForceBefore();
        }
    }
    // 标记后标记
    {
        const auto b = Iterator{ &tab };
        const auto e = this->end();
        for (auto itr = b; itr != e; ++itr) {
            auto& refc = *itr;
            longui_cast<UITab*>(&refc)->ForceAfter();
        }
    }
    // XXX: 应该用uisafe_cast?
    if (const auto box = longui_cast<UITabBox*>(m_pParent)) {
        box->SetSelectedIndex(this->cal_child_index<UITab>(tab));
    }
}

/// <summary>
/// Sets the index of the selected.
/// </summary>
/// <param name="index">The index.</param>
/// <returns></returns>
void LongUI::UITabs::SetSelectedIndex(uint32_t index) noexcept {
    const auto child = this->cal_index_child<UITab>(index);
    // 应该用safe cast?
    if (const auto ptr = longui_cast<UITab*>(child)) {
        this->SetSelectedTab(*ptr);
    }
}


/// <summary>
/// Tabs the removed.
/// </summary>
/// <param name="tab">The tab.</param>
/// <returns></returns>
void LongUI::UITabs::TabRemoved(UITab& tab) noexcept {
    // 移除相关引用
    if (m_pLastSelected == &tab) {
        m_pLastSelected = nullptr;
    }
}


#ifndef DEBUG
/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITabs::add_child(UIControl& child) noexcept {
    // 必须是tab类型的?? 可能是
    if (const auto ptr = uisafe_cast<UITab>(&child)) {

    }
    else {
        assert(!"cannot add other type to tabs");
    }
    return Super::add_child(child);
}
#endif


#ifdef LUI_ACCESSIBLE

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UITabs::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetCtrlTypeArg;
        using get2_t = AccessibleGetAccNameArg;
    //case AccessibleEvent::Event_GetPatterns:
    //    // + 继承基类行为模型
    //    Super::accessible(args);
    //    static_cast<const get0_t&>(args).patterns |=
    //        // + 可调用的行为模型
    //        Pattern_Invoke
    //        // + 读写值的行为模型
    //        | Pattern_Value
    //        ;
    //    return Event_Accept;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =
            AccessibleControlType::Type_Tab;
        return Event_Accept;
    }
    return Super::accessible(args);
}

#endif


// ----------------------------------------------------------------------------
// --------------------            TabBox             -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITabBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITabBox::~UITabBox() noexcept {
}



/// <summary>
/// Initializes a new instance of the <see cref="UITabBox"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITabBox::UITabBox(const MetaControl& meta) noexcept : Super(meta) {
}

/// <summary>
/// Sets the index of the selected.
/// </summary>
/// <param name="index">The index.</param>
/// <returns></returns>
void LongUI::UITabBox::SetSelectedIndex(uint32_t index) noexcept {
    // 有效tabs
    if (m_pTabs) m_pTabs->SetSelectedIndex(index);
    // 有效panels
    if (m_pTabPanels) {
        // 选择显示相应页
        m_pTabPanels->SetSelectedIndex(index);
        const auto now = m_pTabPanels->GetSelectedIndex();
        // 触发事件
        if (now != m_index) {
            m_index = now;
            this->FireEvent(_onCommand());
        }
    }
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITabBox::Update(UpdateReason reason) noexcept {
    // XXX: 要求重新布局
    if (reason & Reason_BasicRelayout) {
        // 重新布局
        this->relayout();
    }
    // 其他的交给超类处理
    Super::Update(reason);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITabBox::DoEvent(UIControl* sender, 
    const EventArg& e) noexcept->EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        this->init_tabbox();
        [[fallthrough]];
    default:
        return Super::DoEvent(sender, e);
    }
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UITabBox::add_child(UIControl& child) noexcept {
    // 添加的是tabs ?
    if (const auto ptr = uisafe_cast<UITabs>(&child)) {
        // XXX: 设置为fixed 提高它的渲染优先级
        this->set_child_fixed_attachment(*ptr);
        m_pTabs = ptr;
    }
    // 添加的是tabpanels ?
    else if (const auto ptr = uisafe_cast<UITabPanels>(&child)) {
        m_pTabPanels = ptr;
    }
#ifndef NDEBUG
    // 禁止添加其他的
    else {
        assert(!"cannot add other type to tabbox");
    }
#endif
    return Super::add_child(child);
}


/// <summary>
/// Initializes the tabbox.
/// </summary>
/// <returns></returns>
void LongUI::UITabBox::init_tabbox() noexcept {
    if (m_pTabs) m_pTabs->SetSelectedIndex(0);
}


/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITabBox::relayout() noexcept {
    // XXX: 获取左上角位置
    const auto lt = this->RefBox().GetContentPos();
    const auto xofffset = lt.x;
    float yoffset = lt.y;
    auto ctsize = this->RefBox().GetContentSize();
    // 拥有TABS
    if (m_pTabs) {
        m_pTabs->SetPos(lt);
        const auto height = m_pTabs->GetMinSize().height;
        yoffset += height;
        ctsize.height -= height;
        this->resize_child(*m_pTabs, { ctsize.width, height });
    }
    // 拥有PANELS
    if (m_pTabPanels) {
        // 加上上边框方便覆盖
        const auto topb = m_pTabPanels->RefBox().border.top;
        //const auto topb = 0.f;
        m_pTabPanels->SetPos({ xofffset, yoffset - topb });
        this->resize_child(*m_pTabPanels, { ctsize.width, ctsize.height + topb });
    }
}

// ----------------------------------------------------------------------------
// --------------------          TabPanel           ---------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITabPanels"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITabPanel::~UITabPanel() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UITabPanels"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITabPanel::UITabPanel(const MetaControl& meta) noexcept : Super(meta) {
}



// ----------------------------------------------------------------------------
// --------------------          TabPanels          ---------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UITabPanels"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITabPanels::~UITabPanels() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UITabPanels"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITabPanels::UITabPanels(const MetaControl& meta) noexcept : Super(meta) {
    // TabPanels类型
    m_oStyle.appearance = Appearance_TabPanels;
}
