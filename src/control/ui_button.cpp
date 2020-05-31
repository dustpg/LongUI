// Gui
#include <luiconf.h>
#include <core/ui_string.h>
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <input/ui_kminput.h>
#include <debugger/ui_debug.h>
#include <event/ui_group_event.h>
#include <core/ui_popup_window.h>
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
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIButton::GetText() const noexcept -> const char16_t* {
    return m_oLabel.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIButton::RefText() const noexcept -> const CUIString&{
    // TODO: 拥有自定义label情况?
    return m_oLabel.RefText();
}


/// <summary>
/// Initializes a new instance of the <see cref="UIButton" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIButton::UIButton(const MetaControl& meta) noexcept : Super(meta),
    m_oImage(nullptr), m_oLabel(nullptr) {
    m_state.focusable = true;
    m_state.tabstop = true;
    m_state.defaultable = true;
    // 阻隔鼠标事件
    m_state.mouse_continue = false;
    this->make_offset_tf_direct(m_oLabel);
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

    //UIControlPrivate::SetFocusable(image, false);
    //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
    m_oImage.name_dbg = "button::image";
    m_oLabel.name_dbg = "button::label";
    assert(m_oImage.IsFocusable() == false);
    assert(m_oLabel.IsFocusable() == false);
#endif
    // 设置连接控件
    m_oLabel.SetControl(*this);
    //m_oLabel.SetText(u"确定");
    // 设置弱外貌
    m_oStyle.appearance = Appearance_WeakApp | Appearance_Button;
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
}

/// <summary>
/// Adds the private child.
/// </summary>
/// <remarks>
/// 由于可以往button标签里面直接嵌入label等标签, 所以才有本函数
/// </remarks>
/// <returns></returns>
void LongUI::UIButton::add_private_child() noexcept {
    if (!m_oImage.GetParent()) {
        this->add_child(m_oImage);
        this->add_child(m_oLabel);
    }
}

/// <summary>
/// Sets the label flex.
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
void LongUI::UIButton::set_label_flex(float f) noexcept {
    UIControlPrivate::SetFlex(m_oLabel, f);
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
    this->add_private_child();
    if (m_oLabel.SetText(std::move(text))) {
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
        // 没子控件
        if (!this->GetChildrenCount()) {
            // TODO: 没有文本时候的处理
            m_oLabel.SetAsDefaultMinsize();
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
            this->StartAnimation({ State_Checked, State_Non});
        }
        return Event_Accept;
    case NoticeEvent::Event_ImplicitGroupChecked:
        // 组有位成员被点中
        if (sender == this) return Event_Ignore;
        if (this->IsDisabled()) return Event_Ignore;
        if (!this->IsChecked()) return Event_Ignore;
        if (m_type != BehaviorType::Type_Radio) return Event_Ignore;
        if (m_pGroup != static_cast<group_t&>(e).group_name)
            return Event_Ignore;
        // 是CHECKBOX类型?
        this->StartAnimation({ State_Checked, State_Non });
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
/// render this
/// </summary>
/// <returns></returns>
void LongUI::UIButton::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged)
        m_oLabel.Update(r);
#ifdef LUI_DRAW_FOCUS_RECT
    // 渲染焦点框
    if (this->m_oStyle.state & State_Focus) {
        // 成本较低就不用进一步判断
        assert(m_pWindow);
        // 按钮的焦点矩形在里面
        this->UpdateFocusRect();
    }
#endif
    Super::Update(reason);
}

#ifdef LUI_DRAW_FOCUS_RECT
/// <summary>
/// render this
/// </summary>
/// <returns></returns>
void LongUI::UIButton::UpdateFocusRect() const noexcept {
    // 按钮的焦点矩形在里面
    m_pWindow->UpdateFocusRect(m_oBox.GetContentEdge());
}

/// <summary>
/// Trigger this
/// </summary>
/// <returns></returns>
auto LongUI::UIButton::TriggerEvent(GuiEvent event) noexcept -> EventAccept {
    // 针对焦点的处理 
    switch (event)
    {
    case LongUI::GuiEvent::Event_OnFocus:
        this->UpdateFocusRect();
    }
    return Super::TriggerEvent(event);
}

#endif

/// <summary>
/// Sets the image source.
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
void LongUI::UIButton::SetImageSource(U8View src) noexcept {
    m_oImage.SetSource(src);
}

/// <summary>
/// Clicks this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIButton::Click() noexcept {
    if (this->IsDisabled()) return;
    // 分类讨论
    switch (m_type)
    {
        StyleState state;
    case BehaviorType::Type_Normal:
        // 普通按钮: 触发修改GUI事件
        this->TriggerEvent(this->_onCommand());
#ifdef LUI_ACCESSIBLE
        LongUI::Accessible(m_pAccessible, LongUI::Callback_Invoked);
#endif
        break;
    case BehaviorType::Type_Checkbox:
        // 是CHECKBOX类型?
        state = (m_oStyle.state ^ State_Checked) & State_Checked;
        this->StartAnimation({ State_Checked, state });
        // 触发修改GUI事件
        this->TriggerEvent(this->_onCommand());
#ifdef LUI_ACCESSIBLE
        // TODO: ACCESSIBLE
#endif
        break;
    case BehaviorType::Type_Radio:
        // 是RADIO类型?
        if (!this->IsChecked()) {
            this->StartAnimation({ State_Checked, State_Checked });
            this->TriggerEvent(_onCommand());
            LongUI::DoImplicitGroupGuiArg(*this, m_pGroup);
#ifdef LUI_ACCESSIBLE
            // TODO: ACCESSIBLE
#endif
        }
        break;
    case BehaviorType::Type_Menu:
        // 是MENU类型?
        if (m_pMenuPopup) {
            // CHECK状态
            this->StartAnimation({ State_Checked, State_Checked });
            // 出现在左下角
            const auto edge = this->RefBox().GetBorderEdge();
            const auto y = this->GetSize().height - edge.top;
            const auto pos = this->MapToWindowEx({ edge.left, y });
            LongUI::PopupWindowFromViewport(
                *this,
                *m_pMenuPopup,
                pos,
                PopupType::Type_PopupV,
                m_pMenuPopup->GetPopupPosition()
            );
        }
        break;
    }
}

/// <summary>
/// add child for this
/// </summary>
/// <param name="child"></param>
/// <param name="constructing"></param>
/// <returns></returns>
void LongUI::UIButton::add_child(UIControl & child) noexcept {
    // 其他的
    Super::add_child(child);
    // UIMenuPopup
    if (const auto ptr = uisafe_cast<UIMenuPopup>(&child)) {
        assert(m_pMenuPopup == nullptr);
        m_pMenuPopup = ptr;
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
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        break;
    case BKDR_ACCESSKEY:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, key, value);
        break;
    case BKDR_IMAGE:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oImage, BKDR_SRC, value);
        break;
    case BKDR_TYPE:
        // type  : 行为类型
        //      : normal
        //      : checkbox
        //      : radio
        //      : menu (?)
        m_type = LongUI::ParseBehaviorType(value);
        break;
    case BKDR_GROUP:
        // group : 按键组
        m_pGroup = UIManager.GetUniqueText(value).id;
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
auto LongUI::ParseBehaviorType(U8View view)noexcept->BehaviorType {
    if (view.end() > view.begin()) {
        switch (*view.begin())
        {
        case 'c': return BehaviorType::Type_Checkbox;
        case 'r': return BehaviorType::Type_Radio;
        case 'm': return BehaviorType::Type_Menu;
        }
    }
    return BehaviorType::Type_Normal;
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
            m_oLabel.RefText();
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
            this->RefText();
        return Event_Accept;
    case AccessibleEvent::Event_Invoke_Invoke:
        this->Click();
        return Event_Accept;
    }
    return Super::accessible(args);
}

#endif
