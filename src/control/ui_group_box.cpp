// Gui
#include <control/ui_caption.h>
#include <control/ui_ctrlmeta.h>
#include <control/ui_groupbox.h>
#include <control/ui_box_layout.h>
// Debug
#include <debugger/ui_debug.h>
// Private
#include "../private/ui_private_control.h"


// ui namespace
namespace LongUI {
    // UIGroupBox类 元信息
    LUI_CONTROL_META_INFO(UIGroupBox, "groupbox");
    // PrivateGroupBox
    struct UIGroupBox::Private : CUIObject {
        // ctor
        Private(UIGroupBox& parent) noexcept;
#ifndef NDEBUG
        // 占位指针位 调试
        void*               placeholder_debug1 = nullptr;
#endif
        // 头布局
        UIHBoxLayout        head;
        // 体布局
        UIVBoxLayout        body;
    };
    /// <summary>
    /// Privates the group box.
    /// </summary>
    /// <param name="parent">The parent.</param>
    /// <returns></returns>
    UIGroupBox::Private::Private(UIGroupBox& parent) noexcept:
        head(&parent), body(&parent) {
        UIControlPrivate::SetFlex(body, 1.f);
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="UICaption" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIGroupBox::UIGroupBox(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
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
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
#ifdef LUI_ACCESSIBLE
    // 逻辑子控件是body
    m_pAccCtrl = &m_private->body;
#endif

}

// super helper
#include "../private/ui_super.h"

/// <summary>
/// Finalizes an instance of the <see cref="UIGroupBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIGroupBox::~UIGroupBox() noexcept {
    m_state.destructing = true;
    if (m_private) delete m_private;
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
        UIControlPrivate::AddAttribute(m_private->body, key, value);
        break;
    default:
        // 父类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIGroupBox::add_child(UIControl& child) noexcept {
    // 已经连接好
    if (m_private) {
        // caption控件?
        UIControl* target;
        // 是UICaption?
        if (const auto ptr = uisafe_cast<UICaption>(&child)) {
            target = &m_private->head;
            m_pCaption = ptr;
        }
        // 其他控件
        else target = &m_private->body;
        
        UIControlPrivate::CallAddChild(*target, child);
    }
    // 还未连接好
    else Super::add_child(child);
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIGroupBox::Update() noexcept {
//    // 污了?
//    if (m_state.dirty) {
//        // 不污
//        m_state.dirty = false;
//        // 更新布局
//        this->relayout();
//        // 更新子控件
//        m_private->head.NeedUpdate();
//        m_private->head.NeedUpdate();
//        // 这里, 世界不再改变
//        //assert(m_state.world_changed == false);
//    }
//    // 链式调用
//    Super::Update();
//}



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
                m_pCaption->GetTextString();
            return Event_Accept;
        }
    }
    return Super::accessible(args);
}

#endif
