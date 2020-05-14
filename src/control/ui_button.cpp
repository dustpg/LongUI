// Gui
#include <luiconf.h>
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <input/ui_kminput.h>
#include <debugger/ui_debug.h>
#include <event/ui_group_event.h>
#include <core/ui_popup_window.h>
#include <core/ui_string.h>
// 控件
#include <control/ui_button.h>
#include <control/ui_boxlayout.h>
#include <control/ui_menupopup.h>
#include <constexpr/const_bkdr.h>
// Private
#include <core/ui_unsafe.h>
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
        // 设置连接控件
        label.SetControl(btn);
    }
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIButton::GetText() const noexcept -> const char16_t* {
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
    : Super(impl::ctor_lock(parent), meta) {
    m_state.focusable = true;
    m_state.defaultable = true;
    // 原子性, 子控件为本控件的组成部分
    m_state.atomicity = true;
#ifdef LUI_ACCESSIBLE
    // 没有逻辑子控件
    m_pAccCtrl = nullptr;
#endif
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // 居中
    m_oStyle.pack = Pack_Center;
    m_oStyle.align = Align_Center;
    m_oBox.margin = { 5, 5, 5, 5 };
    m_oBox.padding = { 2, 2, 2, 2 };
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
    //m_private->label.SetText(u"确定");
    // 构造锁
    impl::ctor_unlock();
}

/// <summary>
/// Finalizes an instance of the <see cref="UIButton"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIButton::~UIButton() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // XXX: 窗口处于析构状态时无需释放窗口
    if (m_pWindow && m_pWindow->IsInDtor()) {

    }
    else {
        if (m_pMenuPopup) delete m_pMenuPopup;
    }
    // 释放私有数据
    if (m_private) delete m_private;
}

/// <summary>
/// Adds the private child.
/// </summary>
/// <remarks>
/// 由于可以往button标签里面直接嵌入label等标签, 所以才有本函数
/// </remarks>
/// <returns></returns>
void LongUI::UIButton::add_private_child() noexcept {
    if (!m_private->image.GetParent()) {
        m_private->image.SetParent(*this);
        m_private->label.SetParent(*this);
    }
}

/// <summary>
/// Sets the label flex.
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
void LongUI::UIButton::set_label_flex(float f) noexcept {
    UIControlPrivate::SetFlex(m_private->label, f);
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
void LongUI::UIButton::SetText(U16View text) noexcept {
    this->SetText(CUIString(text));
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
        Super::DoMouseEvent(e);
        // 移出去算是取消操作
        if (this->IsPointInsideBorder({ e.px, e.py })) {
            this->Click();
            return Event_Accept;
        }
        return Event_Ignore;
    default:
        // 其他情况
        return Super::DoMouseEvent(e);
    }
}

/// <summary>
/// Does the input event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIButton::DoInputEvent(InputEventArg e) noexcept -> EventAccept {
    switch (e.event)
    {
    case InputEvent::Event_KeyDown:
        if (e.character == CUIInputKM::KB_RETURN) {
            this->Click();
            return Event_Accept;
        }
        break;
    }
    return Super::DoInputEvent(e);
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
/// Does the implicit group GUI argument.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="group">The group.</param>
/// <returns></returns>
void LongUI::DoImplicitGroupGuiArg(UIControl& ctrl, const char* group) noexcept {
    if (const auto parent = ctrl.GetParent()) {
        const ImplicitGroupGuiArg arg{ group };
        for (auto& child : (*parent)) {
            child.DoEvent(&ctrl, arg);
        }
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
    using group_t = const ImplicitGroupGuiArg;
    // 初始化
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        UIControlPrivate::SetAppearanceIfNotSet(*this, Appearance_Button);
        // 没子控件
        if (!this->GetCount()) {
            // TODO: 没有文本时候的处理
            m_private->label.SetAsDefaultMinsize();
            this->add_private_child();
        }
        break;
    //case NoticeEvent::Event_RefreshBoxMinSize:
    //    this->refresh_min();
    //    // XXX: 宽度过小?
    //    if (m_minScrollSize.width < float(MIN_BUTTON_WIDTH)) {
    //        m_minScrollSize.width = float(MIN_BUTTON_WIDTH);
    //        this->set_contect_minsize(m_minScrollSize);
    //    }
    //    return Event_Accept;
    case NoticeEvent::Event_DoAccessAction:
        // 访问行为
        this->SetAsDefaultAndFocus();
        this->Click();
        break;
    case NoticeEvent::Event_PopupBegin:
        // 弹出窗口
        m_bPopupShown = true;
        break;
    case NoticeEvent::Event_PopupEnd:
        // 弹出窗口关闭
        m_bPopupShown = false;
        if (sender == m_pMenuPopup) {
            constexpr auto ct = StyleStateType::Type_Checked;
            this->StartAnimation({ ct, false });
        }
        return Event_Accept;
    case NoticeEvent::Event_ImplicitGroupChecked:
        // 组有位成员被点中
        if (sender == this) return Event_Ignore;
        if (this->IsDisabled()) return Event_Ignore;
        if (!this->IsChecked()) return Event_Ignore;
        if (m_type != UIButton::Type_Radio) return Event_Ignore;
        if (m_pGroup != static_cast<group_t&>(e).group_name)
            return Event_Ignore;
        // 是CHECKBOX类型?
        this->StartAnimation({ StyleStateType::Type_Checked, false });
        // 取消不触发command事件
        // 触发修改GUI事件
        //this->TriggerEvent(_checkedChanged());
#ifdef LUI_ACCESSIBLE
        // TODO: ACCESSIBLE
#endif
        return Event_Accept;
    }
    // 基类处理
    return Super::DoEvent(sender, e);
}


/// <summary>
/// Sets the image source.
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
void LongUI::UIButton::SetImageSource(U8View src) noexcept {
    assert(m_private && "bad action");
    m_private->image.SetSource(src);
}

/// <summary>
/// Clicks this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIButton::Click() noexcept {
    if (this->IsDisabled()) return;
    assert(m_private && "bad action");
    // 分类讨论
    switch (m_type)
    {
    case UIButton::Type_Normal:
        // 普通按钮: 触发修改GUI事件
        this->TriggerEvent(this->_onCommand());
#ifdef LUI_ACCESSIBLE
        LongUI::Accessible(m_pAccessible, LongUI::Callback_Invoked);
#endif
        break;
    case UIButton::Type_Checkbox:
        // 是CHECKBOX类型?
        this->StartAnimation({ StyleStateType::Type_Checked, !this->IsChecked() });
        // 触发修改GUI事件
        this->TriggerEvent(this->_onCommand());
#ifdef LUI_ACCESSIBLE
        // TODO: ACCESSIBLE
#endif
        break;
    case UIButton::Type_Radio:
        // 是RADIO类型?
        if (!this->IsChecked()) {
            this->StartAnimation({ StyleStateType::Type_Checked, true });
            this->TriggerEvent(_onCommand());
            LongUI::DoImplicitGroupGuiArg(*this, m_pGroup);
#ifdef LUI_ACCESSIBLE
            // TODO: ACCESSIBLE
#endif
        }
        break;
    case UIButton::Type_Menu:
        // 是MENU类型?
        if (m_pMenuPopup) {
            // CHECK状态
            constexpr auto ct = StyleStateType::Type_Checked;
            this->StartAnimation({ ct, true });
            // 出现在左下角
            const auto edge = this->GetBox().GetBorderEdge();
            const auto y = this->GetSize().height - edge.top;
            const auto pos = this->MapToWindowEx({ edge.left, y });
            LongUI::PopupWindowFromViewport(
                *this,
                *m_pMenuPopup,
                pos,
                PopupType::Type_Popup
            );
        }
        break;
    }
}


/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIButton::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增(?)属性列表
    constexpr auto BKDR_SRC         = 0x001E57C4_ui32;
    constexpr auto BKDR_TYPE        = 0x0fab1332_ui32;
    constexpr auto BKDR_VALUE       = 0x246df521_ui32;
    constexpr auto BKDR_GROUP       = 0x1f6836d3_ui32;
    constexpr auto BKDR_IMAGE       = 0x41d46dbb_ui32;
    constexpr auto BKDR_ACCESSKEY   = 0xba56ab7b_ui32;
    
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_private->label, BKDR_VALUE, value);
        break;
    case BKDR_ACCESSKEY:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_private->label, key, value);
        break;
    case BKDR_IMAGE:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_private->image, BKDR_SRC, value);
        break;
    case BKDR_TYPE:
        // type  : BUTTON类型
        m_type = this->parse_button_type(value);
        break;
    case BKDR_GROUP:
        // group : 按键组
        m_pGroup = UIManager.GetUniqueText(value);
        break;
    default:
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Parses the type of the button.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::UIButton::parse_button_type(U8View view)noexcept->ButtonType {
    if (view.end() > view.begin()) {
        switch (*view.begin())
        {
        case 'c': return UIButton::Type_Checkbox;
        case 'r': return UIButton::Type_Radio;
        case 'm': return UIButton::Type_Menu;
        }
    }
    return UIButton::Type_Normal;
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
        this->SetText(U16View{
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
