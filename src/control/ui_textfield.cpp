// Gui
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <core/ui_color_list.h>
#include <control/ui_textfield.h>
#include <constexpr/const_bkdr.h>
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

// error beep
extern "C" void longui_error_beep() noexcept;

// ui namespace
namespace LongUI {
    // UITextField类 元信息
    LUI_CONTROL_META_INFO(UITextField, "textfield");
}

/// <summary>
/// Initializes a new instance of the <see cref="UITextField" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UITextField::UITextField(const MetaControl& meta) noexcept
    : Super(meta), m_hovered(CUICursor::Cursor_Ibeam) {
    // 本控件支持font属性
    LongUI::MakeDefault(luiref m_tfBuffer);
    UITextField* const nilobj = nullptr;
    const auto address1 = reinterpret_cast<char*>(&nilobj->m_tfBuffer);
    const auto address2 = reinterpret_cast<char*>(&nilobj->m_oStyle);
    m_oStyle.offset_tf = static_cast<uint16_t>(address1 - address2);
    m_colorSelBg = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
    m_colorCaret = ColorF::FromRGBA_CT<RGBA_Black>();
    // 写入默认外间距
    m_oBox.margin = UIManager.RefNativeStyle().margin_basetextfiled;
    // 允许焦点
    m_state.focusable = true;
#ifdef LUI_TEXTBOX_USE_UNIFIED_INPUT
    m_state.unified_input = true;
#endif
    this->create_private();
}


/// <summary>
/// Finalizes an instance of the <see cref="UITextField"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITextField::~UITextField() noexcept {
    m_state.destructing = true;
    this->delete_private();
}


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITextField::Update(UpdateReason reason) noexcept {
    // [SetText接口文本]修改
    if (reason & Reason_ValueTextChanged) {
        this->mark_change_could_trigger();
        this->private_set_text();
    }
    // 文本布局/显示 修改了
    if (reason & Reason_TextFontChanged)
        this->private_tf_changed(!!(reason & Reason_TextFontLayoutChanged));
    // 超类处理
    Super::Update(reason);
    // 检查到大小修改
    if (reason & Reason_SizeChanged) {
        this->private_resize(this->RefBox().GetContentSize());
        this->need_update();
        return;
    }
    // 私有
    this->private_update();
}

/// <summary>
/// Triggers the event.
/// </summary>
/// <param name="event">The event.</param>
/// <returns></returns>
auto LongUI::UITextField::FireEvent(GuiEvent event) noexcept -> EventAccept {
    EventAccept code = Event_Ignore;
    switch (event)
    {
    case LongUI::GuiEvent::Event_OnFocus:
        this->EventOnFocus();
        code = Event_Accept;
        break;
    case LongUI::GuiEvent::Event_OnBlur:
        this->EventOnBlur();
        code = Event_Accept;
        break;
    }
    return Super::FireEvent(event) | code;
}

/// <summary>
/// on focus
/// </summary>
/// <returns></returns>
void LongUI::UITextField::EventOnFocus() noexcept {
    // 更新插入符号位置大小[Focus 肯定有Window了]
    assert(m_pWindow);
    this->show_caret();
    m_pWindow->SetCaretColor(m_colorCaret);
    // TODO: 选择区背景色
    m_colorSelBg.a = 1.f;
}

/// <summary>
/// on blur
/// </summary>
/// <returns></returns>
void LongUI::UITextField::EventOnBlur() noexcept {
    // 取消显示插入符号 [Blur 肯定有Window了]
    assert(m_pWindow);
    m_pWindow->HideCaret();
    // TODO: 选择区背景色
    m_colorSelBg.a = 0.5f;
    // 尝试触发
    this->try_trigger_change_event();
}

/// <summary>
/// Triggers the change event.
/// </summary>
/// <returns></returns>
bool LongUI::UITextField::try_trigger_change_event() noexcept {
    bool code = false;
    if (this->is_change_could_trigger()) {
        this->clear_change_could_trigger();
        code = Super::FireEvent(this->_onChange()) != Event_Ignore;
    }
    return code;
}




/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextField::DoEvent(UIControl * sender,
                          const EventArg & e) noexcept -> EventAccept {
    // 分类讨论
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        // 保证一行
        this->set_contect_minsize({ m_tfBuffer.font.size, m_tfBuffer.font.size });
        return Event_Accept;
    case NoticeEvent::Event_DoAccessAction:
        // 默认行为(聚焦)
        this->SetAsDefaultAndFocus();
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // 初始化
        this->init_private();
        this->clear_change_could_trigger();
        // 默认插入符号颜色是背景色的反色
#ifndef LUI_DISABLE_STYLE_SUPPORT
        if (const auto obj = UIControlPrivate::GetBgRenderer(*this)) {
            m_colorCaret = obj->color;
            m_colorCaret.a = 1.f;
            m_colorCaret.r = 1.f - m_colorCaret.r;
            m_colorCaret.g = 1.f - m_colorCaret.g;
            m_colorCaret.b = 1.f - m_colorCaret.b;
        }
#endif
        [[fallthrough]];
    default:
        // 超类处理
        return Super::DoEvent(sender, e);
    }
}


/// <summary>
/// Does the input event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextField::DoInputEvent(InputEventArg e) noexcept -> EventAccept {
    bool op_ok = true;
    switch (e.event)
    {
    case InputEvent::Event_Char:
        op_ok = this->private_char(e.character, e.sequence);
        break;
    case InputEvent::Event_KeyDown:
        op_ok = this->private_keydown(e.character);
        break;
#ifdef LUI_TEXTBOX_USE_UNIFIED_INPUT
    case InputEvent::Event_TurnLeft:
        this->private_left();
        break;
    case InputEvent::Event_TurnUp:
        this->private_up();
        break;
    case InputEvent::Event_TurnRight:
        this->private_right();
        break;
    case InputEvent::Event_TurnDown:
        this->private_down();
        break;
#endif
    }
    // XXX: 其他方式?
    if (!op_ok) ::longui_error_beep();
    return Event_Accept;
}

// utf-8 -> utf-32
extern "C" uint32_t ui_utf8_to_utf32(
    char32_t* __restrict buf,
    uint32_t buflen,
    const char* __restrict src,
    const char* end
) noexcept;


/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UITextField::add_attribute(uint32_t key, U8View value) noexcept {
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
    // LongUI扩展
    constexpr auto BKDR_LUI_CACHE       = 0x14ec3428_ui32;
    // 分类讨论
    switch (key)
    {
    case BKDR_LUI_CACHE:
        // lui::cached:  LUI优化扩展
        //   使用离屏渲染/脏矩形更新等算法进行优化
        //   推荐长文本使用
        if (value.ToBool()) this->private_use_cached();
        break;
    case BKDR_TYPE:
        // type:        类型
        [value, this]() noexcept {
            switch (*value.begin())
            {
            case 'p':   // password:    输入密码
                this->InitMarkPassword();
                break;
            case 'n':   // number:      输入整型
                break;
            }
        }();
        break;
    case BKDR_MULTILINE:
        // multiline:   多行模式
        if (value.ToBool()) this->private_mark_multiline();
        break;
    //case BKDR_SIZE:
    //    // size:        单行模式容纳字符数量
    //    break;
    case BKDR_READONLY:
        // readonly:    只读模式
        if (value.ToBool()) this->private_mark_readonly();
        break;
    case BKDR_VALUE:
        // value:       显示文本
        this->private_set_text(CUIString::FromUtf8(value));
        break;
    case BKDR_MAXLENGTH:
        // maxlength:   最长容纳的文本长度
        m_uMaxLength = static_cast<uint32_t>(value.ToInt32());
        break;
    case BKDR_PASSWORD:
        // password:    密码使用的字符
        m_chPassword = [value]() noexcept {
#if 0
            const auto text = CUIString32::FromUtf8(value);
            return static_cast<char32_t>(text.empty() ? '*' : text[0]);
#else
            constexpr uint32_t PWBL = 4;
            char32_t buf[PWBL]; buf[0] = '*';
            ::ui_utf8_to_utf32(buf, PWBL, value.begin(), value.end());
            return buf[0];
#endif
        }();
        break;
    default:
       // 其他交给超类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextField::DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept {
    Point2F pos = { e.px, e.py }; this->MapFromWindow(pos);
    pos.x += this->render_positon.x;
    pos.y += this->render_positon.y;
    bool op_ok = true;
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
        break;
    case LongUI::MouseEvent::Event_MouseWheelH:
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        assert(m_pWindow && "no window no mouse");
        m_pWindow->SetNowCursor(m_hovered);
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        assert(m_pWindow && "no window no mouse");
        m_pWindow->SetNowCursor(nullptr);
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        if (e.modifier & LongUI::Modifier_LButton) {
            this->private_mouse_move(pos);
        }
        break;
    case LongUI::MouseEvent::Event_LButtonDown:
        op_ok = this->private_mouse_down(pos, !!(e.modifier & LongUI::Modifier_Shift));
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        op_ok = this->private_mouse_up(pos);
        break;
    case LongUI::MouseEvent::Event_RButtonDown:
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        break;
    case LongUI::MouseEvent::Event_MButtonDown:
        break;
    case LongUI::MouseEvent::Event_MButtonUp:
        break;
    default:
        break;
    }
    // XXX: 其他方式?
    if (!op_ok) ::longui_error_beep();
    return Super::DoMouseEvent(e);
}


