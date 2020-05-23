// Gui
#include <core/ui_ctrlmeta.h>
#include <control/ui_caption.h>
#include <core/ui_unsafe.h>
#include <control/ui_groupbox.h>
#include <control/ui_boxlayout.h>
// Debug
#include <debugger/ui_debug.h>
// Private
#include "../private/ui_private_control.h"


// ui namespace
namespace LongUI {
    // UIGroupBox类 元信息
    LUI_CONTROL_META_INFO(UIGroupBox, "groupbox");
}


/// <summary>
/// Initializes a new instance of the <see cref="UICaption" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIGroupBox::UIGroupBox(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta),
    m_oHeadLayout(this), m_oBodyLayout(this) {
    // 垂直布局
    m_state.orient = Orient_Vertical;
    // 原子性, 子控件为本控件的组成部分
    //m_state.atomicity = true;
    // 默认为分组框样式
    m_oStyle.appearance = Appearance_GroupBox;
    // 基本样式
    m_oBox.border = { 1.f, 1.f, 1.f, 1.f };
    m_oBox.margin = { 3.f, 3.f, 3.f, 3.f };
    m_oBox.padding = { 3.f, 3.f, 3.f, 6.f };
    // 私有实现
    UIControlPrivate::SetFlex(m_oBodyLayout, 1.f);
#ifdef LUI_ACCESSIBLE
    // 逻辑子控件是body
    m_pAccCtrl = &m_oBodyLayout;
#endif
    // 构造锁
    impl::ctor_unlock();
}

/// <summary>
/// Finalizes an instance of the <see cref="UIGroupBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIGroupBox::~UIGroupBox() noexcept {
    // 提前释放
    m_state.destructing = true;
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIGroupBox::add_attribute(uint32_t key, U8View value) noexcept {
    // 待使用属性列表
    constexpr auto BKDR_DIR     = 0x001a65b1_ui32;
    constexpr auto BKDR_PACK    = 0x0f1b8d4d_ui32;
    constexpr auto BKDR_ALIGN   = 0xb54685e9_ui32;
    constexpr auto BKDR_ORIENT  = 0xeda466cd_ui32;
    // 属性KEY
    switch (key)
    {
    case BKDR_DIR:
    case BKDR_PACK:
    case BKDR_ALIGN:
    case BKDR_ORIENT:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oBodyLayout, key, value);
        break;
    default:
        // 超类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIGroupBox::add_child(UIControl& child) noexcept {
    // 自带
    if (child == m_oBodyLayout || child == m_oHeadLayout)
        return Super::add_child(child);
    // caption控件?
    UIControl* target;
    // 是UICaption?
    if (const auto ptr = uisafe_cast<UICaption>(&child)) {
        target = &m_oHeadLayout;
        m_pCaption = ptr;
    }
    // 其他控件
    else target = &m_oBodyLayout;
    // 添加给目标
    UIControlPrivate::CallAddChild(*target, child);
}


#ifdef LUI_ACCESSIBLE

#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#include <core/ui_string.h>

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UIGroupBox::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetCtrlTypeArg;
        using get2_t = AccessibleGetAccNameArg;
    case AccessibleEvent::Event_GetPatterns:
        // + 继承基类行为模型
        Super::accessible(args);
        return Event_Accept;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =
            AccessibleControlType::Type_Group;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称
        if (m_pCaption) {
            *static_cast<const get2_t&>(args).name =
                m_pCaption->RefText();
            return Event_Accept;
        }
    }
    return Super::accessible(args);
}

#endif
