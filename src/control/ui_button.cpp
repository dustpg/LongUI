// Gui
#include <debugger/ui_debug.h>
#include <control/ui_button.h>
#include <control/ui_ctrlmeta.h>
// 子控件
#include <control/ui_box_layout.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
#include <constexpr/const_bkdr.h>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UIButton类 元信息
    LUI_CONTROL_META_INFO(UIButton, "button");
    // UIButton私有信息
    struct UIButton::Private : CUIObject {
        // 构造函数
        Private(UIButton& btn) noexcept;
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
    UIButton::Private::Private(UIButton& btn) noexcept
        : image(nullptr), label(nullptr) {
        //UIControlPrivate::SetFocusable(image, false);
        //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
        image.name_dbg = "button::image";
        label.name_dbg = "button::label";
        assert(image.IsFocusable() == false);
        assert(label.IsFocusable() == false);
#endif
    }
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIButton::GetText() const noexcept -> const wchar_t* {
    assert(m_private && "bad action");
    return m_private->label.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIButton::GetTextString() const noexcept -> const CUIString&{
    assert(m_private && "bad action");
    return m_private->label.GetTextString();
}


/// <summary>
/// Initializes a new instance of the <see cref="UIButton" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIButton::UIButton(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(parent, meta) {
    m_state.focusable = true;
    m_state.defaultable = true;
    // 原子性, 子控件为本控件的组成部分
    m_state.atomicity = true;
#ifdef LUI_ACCESSIBLE
    // 没有逻辑子控件
    m_pAccCtrl = nullptr;
#endif
    // 水平布局
    this->SetOrient(Orient_Horizontal);
    m_oBox.margin = { 5, 5, 5, 5 };
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
    //m_private->label.SetText(L"确定");
}


/// <summary>
/// Finalizes an instance of the <see cref="UIButton"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIButton::~UIButton() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 释放私有数据
    if (m_private) delete m_private;
}

/// <summary>
/// Adds the private child.
/// </summary>
/// <returns></returns>
void LongUI::UIButton::add_private_child() noexcept {
    if (!m_private->image.GetParent()) {
        m_private->image.SetParent(*this);
        m_private->label.SetParent(*this);
    }
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIButton::SetText(const CUIString& text) noexcept {
    this->SetText(CUIString{ text });
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIButton::SetText(WcView text) noexcept {
    this->SetText(CUIString{ text });
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIButton::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonUp:
        // 移出去算是取消操作
        if (this->IsPointInsideBorder({ e.px, e.py })) this->Click();
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}

#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#include <core/ui_string.h>
#endif


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIButton::SetText(CUIString&& text) noexcept {
    assert(m_private && "bad action");
    this->add_private_child();
    if (m_private->label.SetText(std::move(text))) {
        this->mark_window_minsize_changed();
#ifdef LUI_ACCESSIBLE
        LongUI::Accessible(m_pAccessible, Callback_PropertyChanged);
#endif
    }
}



/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIButton::DoEvent(UIControl * sender,
    const EventArg & e) noexcept -> EventAccept {
    // 初始化
    if (e.nevent == NoticeEvent::Event_Initialize) {
        UIControlPrivate::SetAppearanceIfNotSet(*this, Appearance_Button);
        // 没子控件
        if (!this->GetCount()) {
            // TODO: 没有文本时候的处理
            m_private->label.SetAsDefaultMinsize();
            this->add_private_child();
        }
    }
    // 基类处理
    return Super::DoEvent(sender, e);
}


/// <summary>
/// Clicks this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIButton::Click() noexcept {
    assert(m_private && "bad action");
    // 触发修改GUI事件
    this->TriggrtEvent(_clicked());
#ifdef LUI_ACCESSIBLE
    LongUI::Accessible(m_pAccessible, LongUI::Callback_Invoked);
#endif
}


/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIButton::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给子控件
        UIControlPrivate::AddAttribute(m_private->label, BKDR_VALUE, value);
        return;
    default:
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}


#ifdef LUI_ACCESSIBLE

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UIButton::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
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
            // + 读写值的行为模型
            | Pattern_Value
            ;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =  
            AccessibleControlType::Type_Button;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称
        *static_cast<const get2_t&>(args).name = 
            m_private->label.GetTextString();
        return Event_Accept;
    case AccessibleEvent::Event_Value_SetValue:
        // 设置值
        this->SetText(WcView{
            static_cast<const AccessibleVSetValueArg&>(args).string,
            static_cast<const AccessibleVSetValueArg&>(args).string +
            static_cast<const AccessibleVSetValueArg&>(args).length
        });
        return Event_Accept;
    case AccessibleEvent::Event_Value_GetValue:
        // 读取值
        *static_cast<const AccessibleVGetValueArg&>(args).value =
            this->GetTextString();
        return Event_Accept;
    case AccessibleEvent::Event_Invoke_Invoke:
        this->Click();
        return Event_Accept;
    }
    return Super::accessible(args);
}

#endif
