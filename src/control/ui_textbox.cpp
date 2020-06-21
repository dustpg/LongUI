// Gui
#include <core/ui_unsafe.h>
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <core/ui_color_list.h>
#include <control/ui_textbox.h>
#include <util/ui_little_math.h>
#include <control/ui_scrollbar.h>
#include <constexpr/const_bkdr.h>
#include <control/ui_spinbuttons.h>
#include <graphics/ui_bg_renderer.h>
// Private
#include "../private/ui_private_control.h"
// C++
#include <cassert>
#include <limits>
#include <algorithm>

#ifndef NDEBUG
#include <debugger/ui_debug.h>
#endif
#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#endif

// ui namespace
namespace LongUI {
    // UITextBox类 元信息
    LUI_CONTROL_META_INFO(UITextBox, "textbox");
    /// <summary>
    /// Makes the default.
    /// </summary>
    /// <param name="tf">The tf.</param>
    /// <returns></returns>
    void MakeDefault(TextFont& tf) noexcept {
        tf.text.color = { 0, 0, 0, 1 };
        tf.text.stroke_color = { 0 };
        tf.text.stroke_width = 0.f;
        tf.text.alignment = TAlign_Start;
        tf.font = UIManager.RefDefaultFont();
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="UITextBox" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITextBox::UITextBox(const MetaControl& meta) noexcept
    : Super(meta), max_value(std::numeric_limits<double>::infinity()),
    m_oTextField(this), m_oPlaceHolder(this) {
    // XXX: 默认颜色
    const_cast<ColorF&>(m_oPlaceHolder.RefTextFont().text.color) = { 0.5, 0.5, 0.5, 1 };
    //const_cast<AttributeFontStyle&>(m_oPlaceHolder.RefTextFont().font.style) = Style_Italic;
    m_oStyle.appearance = Appearance_TextField;
    this->make_offset_tf_direct(m_oTextField);
    UIControlPrivate::SetFlex(m_oTextField, 1);
    m_oTextField.RefInheritedMask()
        = State_Disabled        // 继承禁止状态
        | State_Focus           // 继承焦点状态
        ;
    // 允许焦点
    m_state.focusable = true;
    m_state.tabstop = true;
#ifdef LUI_TEXTBOX_USE_UNIFIED_INPUT
    m_state.unified_input = true;
#endif
    // 自己已经有了, 取消TextField焦点功能
    UIControlPrivate::SetFocusable0(m_oTextField);
    // 将TextField重定向本节点
    UIControlPrivate::SetGuiEvent2Parent(m_oTextField);
#ifndef NDEBUG
    m_oTextField.name_dbg = "textbox::textfield";
    m_oPlaceHolder.name_dbg = "textbox::placeholder";
#endif // !NDEBUG
#ifdef LUI_ACCESSIBLE
    // 子控件为本控件的组成部分
    m_pAccCtrl = nullptr;
#endif
}


/// <summary>
/// Finalizes an instance of the <see cref="UITextBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITextBox::~UITextBox() noexcept {
    m_state.destructing = true;
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged) {
        // XXX: 硬编码-仅仅继承文字大小
        const auto size_from = m_oTextField.RefTextFont().font.size;
        auto& size_to = m_oPlaceHolder.RefTextFont().font.size;
        if (size_from != size_to) {
            const_cast<float&>(size_to) = size_from;
            m_oPlaceHolder.Update(Reason_TextFontLayoutChanged);
        }
        m_oTextField.Update(r);
    }
    // 重新布局
    if (reason & Reason_BasicRelayout)
        this->relayout_textbox();
    // 超类处理
    Super::Update(reason);
}

/// <summary>
/// relayout this
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::relayout_textbox() noexcept {
    m_minScrollSize = m_oTextField.RefBox().minsize;
    // 针对滚动条
    auto remaining = this->layout_scroll_bar();
    // 需要重新布局
    if (this->is_need_relayout()) return;
    auto pos = this->get_layout_position();
    // 计算SPIN按钮
    if (m_pSpinButtons) {
        // TODO: 逆向在左边
        const auto min = m_pSpinButtons->GetMinSize();
        m_pSpinButtons->Resize({ min.width, remaining.height });
        remaining.width = std::max(remaining.width - min.width, 1.f);
        m_pSpinButtons->SetPos({ pos.x + remaining.width, pos.y });
    }
    m_oTextField.Resize(remaining);
    m_oTextField.SetPos(pos);
    m_oPlaceHolder.Resize(remaining);
    m_oPlaceHolder.SetPos(pos);
    this->do_wheel(0, 0);
    this->do_wheel(1, 0);
}

/// <summary>
/// Triggers the event.
/// </summary>
/// <param name="event">The event.</param>
/// <returns></returns>
auto LongUI::UITextBox::FireEvent(GuiEvent event) noexcept -> EventAccept {
    EventAccept code = Event_Ignore;
    switch (event)
    {
    case LongUI::GuiEvent::Event_OnFocus:
        m_oTextField.EventOnFocus();
        code = Event_Accept;
        break;
    case LongUI::GuiEvent::Event_OnBlur:
        m_oTextField.EventOnBlur();
        code = Event_Accept;
        break;
    }
    return Super::FireEvent(event) | code;
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextBox::DoEvent(UIControl * sender, const EventArg & e) noexcept -> EventAccept {
    // 分类讨论
    switch (e.nevent)
    {
        GuiEvent eid;
    case NoticeEvent::Event_RefreshBoxMinSize:
        if (m_bNeedMinsize) {
            m_bNeedMinsize = false;
            this->update_minsize();
        }
        return Event_Accept;
    case NoticeEvent::Event_DoAccessAction:
        // 默认行为(聚焦)
        this->SetAsDefaultAndFocus();
        return Event_Accept;
    case NoticeEvent::Event_UIEvent:
        assert(sender && "sender in gui event cannot be null");
        eid = static_cast<const EventGuiArg&>(e).GetEvent();
        // 针对Textfiled的处理
        if (sender == &m_oTextField) return this->event_from_textfield(eid);
        switch (eid)
        {
        case GuiEvent::Event_OnChange:
            if (sender == m_pSBHorizontal)
                m_oTextField.render_positon.x = m_pSBHorizontal->GetValue();
            else if (sender == m_pSBVertical)
                m_oTextField.render_positon.y = m_pSBVertical->GetValue();
            else break;
            // SB修改之后调用
            m_oTextField.UpdateRenderPostion();
            return Event_Accept;
        case GuiEvent::Event_OnDecrease:
            return this->SetValueAsDouble(-this->increment, true);
        case GuiEvent::Event_OnIncrease:
            return this->SetValueAsDouble(this->increment, true);
        }
    }
    // 超类处理
    return Super::DoEvent(sender, e);
}

/// <summary>
/// initialize UITextBox
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::initialize() noexcept {
    // 初始化
    //this->update_minsize();
    this->SetValueAsDouble(0, true);
    // 初始化 超类
    Super::initialize();
}


PCN_NOINLINE
/// <summary>
/// set double value
/// </summary>
/// <param name="value"></param>
/// <param name="increase"></param>
/// <returns></returns>
auto LongUI::UITextBox::SetValueAsDouble(double value, bool increase) noexcept -> EventAccept {
    // 没有就无视
    if (!m_pSpinButtons) return Event_Ignore;
    const auto cur_value = this->GetValueAsDouble();
    const auto target = increase ? cur_value + value : value;
    const auto maxv = this->max_value;
    const auto minv = this->min_value;
    const auto new_value = detail::clamp(target, minv, maxv);
    //  没变
    //if (new_value == cur_value) return Event_Ignore;
    const uint32_t places = m_uDecimalPlaces;
    //m_pSpinButtons->SetIncreaseDisabled(new_value == maxv);
    //m_pSpinButtons->SetDecreaseDisabled(new_value == minv);
    // 64够了
    CUIBasicString<char16_t, 64> text;
    text.AsDouble(new_value, 0.5, places, m_chDecimal);
    this->SetText(text.view());
    return Event_Accept;
}


/// <summary>
/// redirent event from textfield
/// </summary>
/// <param name="eid">event id</param>
/// <returns></returns>
auto LongUI::UITextBox::event_from_textfield(GuiEvent eid) noexcept -> EventAccept {
    switch (eid)
    {
    case UITextField::_onInput():
        // 针对Input处理
        m_oPlaceHolder.SetVisible(!m_oTextField.GuiHasText());
        break;
    case UITextField::_onChange():
        // 针对Change处理
        this->SetValueAsDouble(0, true);
        break;
    }
    return this->FireEvent(eid);
}


/// <summary>
/// Does the input event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextBox::DoInputEvent(InputEventArg e) noexcept -> EventAccept {
    switch (e.event)
    {
    case InputEvent::Event_Char:
        // 输入数字
        if (m_type == UITextBox::Type_Number) {
            if ((e.character >= '0' && e.character <= '9') ||
                e.character == m_chDecimal || e.character == '-');
            else return Event_Ignore;
        }
        return m_oTextField.DoInputEvent(e);
        break;
    case InputEvent::Event_KeyDown:
        return m_oTextField.DoInputEvent(e);
        break;
    }
    return Event_Accept;
}


/// <summary>
/// Initializes the textbox.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::update_minsize() noexcept {
    const auto cols = static_cast<float>(m_uCols);
    const auto rows = static_cast<float>(m_uRows);
    auto& tf = m_oTextField.RefTextFont();
    const auto fs = tf.font.size;
    const auto line_height = LongUI::GetLineHeight(tf.font);
    const auto rect = m_oTextField.RefBox().GetNonContect();
    this->set_contect_minsize({ 
        cols * fs * 0.5f + rect.left + rect.right,
        rows * line_height + rect.top + rect.bottom
    });
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UITextBox::add_attribute(uint32_t key, U8View value) noexcept {
    // 待使用属性列表
    constexpr auto BKDR_MIN             = 0x001cc0fe_ui32;
    constexpr auto BKDR_MAX             = 0x001cbcf0_ui32;
    constexpr auto BKDR_SIZE            = 0x0f849a25_ui32;
    constexpr auto BKDR_ROWS            = 0x0f63dd45_ui32;
    constexpr auto BKDR_COLS            = 0x0d614b8f_ui32;
    constexpr auto BKDR_TYPE            = 0x0fab1332_ui32;
    constexpr auto BKDR_VALUE           = 0x246df521_ui32;
    constexpr auto BKDR_PASSWORD        = 0xa3573bc3_ui32;
    constexpr auto BKDR_READONLY        = 0xad3f7b8a_ui32;
    constexpr auto BKDR_INCREMENT       = 0x73689623_ui32;
    constexpr auto BKDR_MULTILINE       = 0xb2db9639_ui32;
    constexpr auto BKDR_MAXLENGTH       = 0xb532c6e6_ui32;
    constexpr auto BKDR_PLACEHOLDER     = 0x2aad8773_ui32;
    constexpr auto BKDR_DECIMALPLACES   = 0x30a7dbb9_ui32;
    // LongUI扩展
    constexpr auto BKDR_LUI_CACHE       = 0x14ec3428_ui32;
    // 分类讨论
    switch (key)
    {
    case BKDR_DECIMALPLACES:
        // decimalplaces:       小数位数
        m_uDecimalPlaces = static_cast<uint8_t>(value.ToInt32());
        break;
    case BKDR_COLS:
        // cols:        多行模式下每行容纳字符数量
        m_uCols = static_cast<uint32_t>(value.ToInt32());
        break;
    case BKDR_ROWS:
        // row:         多行模式下容纳的行数
        m_uRows = static_cast<uint32_t>(value.ToInt32());
        break;
    case BKDR_PLACEHOLDER:
        // placeholder: 空输入占位符
        Unsafe::AddAttrUninited(m_oPlaceHolder, BKDR_VALUE, value);
        break;
    case BKDR_MIN:
        this->min_value = value.ToDouble();
        break;
    case BKDR_MAX:
        this->max_value = value.ToDouble();
        break;
    case BKDR_INCREMENT:
        this->increment = value.ToDouble();
        break;
    case BKDR_TYPE:
        m_type = [=]() noexcept {
            if (value.end() > value.begin()) {
                switch (*value.begin())
                {
                case 'n': 
                    this->make_spin();
                    return UITextBox::Type_Number;
                case 'p': 
                    m_oTextField.InitMarkPassword();
                    return UITextBox::Type_Password;
                }
            }
            return UITextBox::Type_Normal;
        }();
        break;
    case BKDR_MULTILINE:
        //m_oStyle.overflow_x = m_oStyle.overflow_y = Overflow_Scroll;
        m_oStyle.overflow_x = m_oStyle.overflow_y = Overflow_Auto;
        [[fallthrough]];
    case BKDR_PASSWORD:
    case BKDR_VALUE:
    case BKDR_READONLY:
    case BKDR_MAXLENGTH:
    case BKDR_LUI_CACHE:
        // TODO: 不用二次判断
        Unsafe::AddAttrUninited(m_oTextField, key, value);
    default:
        // 其他交给超类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// request the text
/// </summary>
/// <returns></returns>
auto LongUI::UITextBox::RequestText() noexcept -> const CUIString & {
    return m_oTextField.RequestText();
}

/// <summary>
/// get value (double float)
/// </summary>
/// <returns></returns>
auto LongUI::UITextBox::GetValueAsDouble() noexcept -> double {
    return m_oTextField.RequestText().view().ToDouble(m_chDecimal);
}


/// <summary>
/// set text
/// </summary>
/// <param name="view"></param>
/// <returns></returns>
void LongUI::UITextBox::SetText(const CUIString & text) noexcept {
    m_oTextField.SetText(text);
}

/// <summary>
/// set text
/// </summary>
/// <param name="view"></param>
/// <returns></returns>
void LongUI::UITextBox::SetText(CUIString&& text) noexcept {
    m_oTextField.SetText(std::move(text));
}

/// <summary>
/// set text
/// </summary>
/// <param name="view"></param>
/// <returns></returns>
void LongUI::UITextBox::SetText(U16View view) noexcept {
    m_oTextField.SetText(view);
}


/// <summary>
/// Does the wheel.
/// </summary>
/// <param name="index">The index.</param>
/// <param name="wheel">The wheel.</param>
/// <returns></returns>
auto LongUI::UITextBox::do_wheel(int index, float wheel) noexcept ->EventAccept {
    const auto maxv = index[&m_maxScrollSize.width];
    // 位置变动检查
    auto& offset = index[&m_oTextField.render_positon.x];
    const auto line_height = UIManager.GetWheelScrollLines();
    float pos = offset - index[&this->line_size.width] * wheel * line_height;
    pos = detail::clamp(pos, 0.f, maxv);
    // 已经修改
    if (pos != offset) {
        offset = pos;
        m_oTextField.UpdateRenderPostion();
        this->sync_scroll_bar(luiref m_oTextField.render_positon);
        return Event_Accept;
    }
    return Event_Ignore;
}

/// <summary>
/// make spin buttons
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::make_spin() noexcept {
    if (m_pSpinButtons) return;
    const auto spin = new(std::nothrow) UISpinButtons{ this };
    if (!spin) return;
    UIControlPrivate::SetGuiEvent2Parent(*spin);
    m_pSpinButtons = spin;
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextBox::DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept {
    // 分类判断
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
        // 检查是否有子控件处理(super有, 也可以现写)
        return UIControl::DoMouseEvent(e) == Event_Ignore ?
            this->do_wheel(1, e.wheel) : Event_Accept;
    case LongUI::MouseEvent::Event_MouseWheelH:
        // 检查是否有子控件处理(super有, 也可以现写)
        return UIControl::DoMouseEvent(e) == Event_Ignore ?
            this->do_wheel(0, e.wheel) : Event_Accept;
    }
    // 其他未处理事件交给super处理
    return Super::DoMouseEvent(e);
}

#ifdef LUI_ACCESSIBLE

//#define LUI_TEXTBOX_PASSWORD_ALWAYS_VISIBLE

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UITextBox::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetCtrlTypeArg;
        using get2_t = AccessibleGetAccNameArg;
    case AccessibleEvent::Event_GetPatterns:
        // + 继承超类行为模型
        Super::accessible(args);
        static_cast<const get0_t&>(args).patterns |=
            // + 读写值的行为模型
            Pattern_Value
            ;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =
            AccessibleControlType::Type_TextEdit;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // XXX: 获取Acc名称
        if (m_oTextField.GuiIsPasswordMode()) {
            auto& output = *static_cast<const get2_t&>(args).name;
            output = u"[PASSWORD]"_sv;
        }
        else {
            auto& output = *static_cast<const get2_t&>(args).name;
            output = this->RequestText();
        }
        return Event_Accept;
    }
    // 超类处理
    return Super::accessible(args);
}

#endif