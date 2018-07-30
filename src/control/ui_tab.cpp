#include <control/ui_tab.h>
#include <control/ui_tabs.h>
#include <control/ui_tabbox.h>
#include <control/ui_tabpanel.h>
#include <control/ui_tabpanels.h>

#include <control/ui_image.h>
#include <control/ui_label.h>

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
    namespace impl { Size2F greatest_minsize(UIControl& ctrl) noexcept; }
}



// ----------------------------------------------------------------------------
// --------------------              Tab              -------------------------
// ----------------------------------------------------------------------------

// ui namespace
namespace LongUI {
    // UITab私有信息
    struct UITab::Private : CUIObject {
        // 构造函数
        Private(UITab& btn) noexcept;
#ifndef NDEBUG
        // 调试占位
        void*               placeholder_debug1 = nullptr;
#endif
        // 图像控件
        UIImage             image;
        // 标签控件
        UILabel             label;
    };
    /// <summary>
    /// button privates data/method
    /// </summary>
    /// <param name="btn">The BTN.</param>
    /// <returns></returns>
    UITab::Private::Private(UITab& btn) noexcept
        : image(&btn), label(&btn) {
        //UIControlPrivate::SetFocusable(image, false);
        //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
        image.name_dbg = "tab::image";
        label.name_dbg = "tab::label";
        assert(image.IsFocusable() == false);
        assert(label.IsFocusable() == false);
#endif
    }
}

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
    // 释放私有数据
    if (m_private) delete m_private;
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
void LongUI::UITab::SetText(const CUIString& str) noexcept {
    m_private->label.SetText(std::move(str));
}

/// <summary>
/// Initializes a new instance of the <see cref="UITab"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITab::UITab(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // tab类型
    m_oBox.border = { 1, 1, 1, 1 };
    m_oBox.padding = { 0, 1, 0, 1 };
    m_oStyle.appearance = Appearance_Tab;
    // 原子控件
    m_state.atomicity = true;
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
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
        UIControlPrivate::AddAttribute(m_private->label, BKDR_VALUE, value);
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
            m_private->label.GetTextString();
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
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITabs::UITabs(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
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
        ptr->StartAnimation({ StyleStateType::Type_Selected, false });
    // 确定选择动画
    m_pLastSelected = &tab;
    tab.StartAnimation({ StyleStateType::Type_Selected, true });
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
        box->SetSelectedIndex(this->calculate_child_index(tab));
    }
}

/// <summary>
/// Sets the index of the selected.
/// </summary>
/// <param name="index">The index.</param>
/// <returns></returns>
void LongUI::UITabs::SetSelectedIndex(uint32_t index) noexcept {
    const auto child = this->calculate_child_at(index);
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
    // 必须是tab类型的
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
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITabBox::UITabBox(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {

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
            this->TriggrtEvent(_selectedChanged());
        }
    }
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITabBox::Update() noexcept {
    // 要求重新布局
    if (this->is_need_relayout()) {
        // 不脏了
        m_state.dirty = false;
        // 重新布局
        this->relayout();
    }
    // 其他的交给父类处理
    Super::Update();
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
    const auto lt = this->GetBox().GetContentPos();
    const auto xofffset = lt.x;
    float yoffset = lt.y;
    auto ctsize = this->GetBox().GetContentSize();
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
        const auto topb = m_pTabPanels->GetBox().border.top;
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
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITabPanel::UITabPanel(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
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
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITabPanels::UITabPanels(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    // TabPanels类型
    m_oStyle.appearance = Appearance_TabPanels;
    m_oBox.border = { 1, 1, 1, 1 };

}
