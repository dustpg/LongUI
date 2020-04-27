// Gui
/*#include <input/ui_kminput.h>
#include <control/ui_scale.h>
#include <control/ui_image.h>
#include <resource/ui_image.h>
#include <container/pod_hash.h>*/

#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <control/ui_textbox.h>
#include <constexpr/const_bkdr.h>
// Private
//#include "../private/ui_private_control.h"
// C++
#include <cassert>
#include <algorithm>

#ifndef NDEBUG
#include <debugger/ui_debug.h>
#endif

// TextBC
#include <../TextBC/bc_txtdoc.h>

// error beep
extern "C" void longui_error_beep() noexcept;

// ui namespace
namespace LongUI {
    // UITextBox类 元信息
    LUI_CONTROL_META_INFO(UITextBox, "textbox");
}

/// <summary>
/// Makes the default.
/// </summary>
/// <param name="tf">The tf.</param>
/// <returns></returns>
void LongUI::MakeDefault(TextFont& tf) noexcept {
    tf.text.color = { 0, 0, 0, 1 };
    tf.text.stroke_color = { 0 };
    tf.text.stroke_width = 0.f;
    tf.text.alignment = TAlign_Start;
    tf.font = UIManager.GetDefaultFont();
}

/// <summary>
/// Initializes a new instance of the <see cref="UITextBox" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UITextBox::UITextBox(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) , m_hovered(CUICursor::Cursor_Ibeam) {
    // 本控件支持font属性
    LongUI::MakeDefault(m_tfBuffer);
    UITextBox* const nilobj = nullptr;
    const auto address1 = reinterpret_cast<char*>(&nilobj->m_tfBuffer);
    const auto address2 = reinterpret_cast<char*>(&nilobj->m_oStyle);
    m_oStyle.offset_tf = static_cast<uint16_t>(address1 - address2);
    // 默认间距
    m_oBox.margin = { 4, 2, 4, 2 };
    m_oBox.padding = { 4, 2, 2, 2 };
    m_oStyle.appearance = Appearance_TextField;
    // 允许焦点
    m_state.focusable = true;
#ifdef LUI_TEXTBOX_USE_UNIFIED_INPUT
    m_state.unified_input = true;
#endif
    this->create_private();
}


/// <summary>
/// Finalizes an instance of the <see cref="UITextBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITextBox::~UITextBox() noexcept {
    this->delete_private();
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::Update() noexcept {
    // [SetText接口文本]修改
    if (m_bTextChanged) {
        m_bTextChanged = false;
        this->mark_change_could_trigger();
        this->private_set_text();
    }
    // 检查到大小修改
    if (this->is_size_changed()) {
        this->private_resize(this->GetBox().GetContentSize());
    }
    // 污了
    this->private_update();
    // 父类处理
    Super::Update();
    // 处理大小修改
    this->size_change_handled();
}

/// <summary>
/// Triggers the event.
/// </summary>
/// <param name="event">The event.</param>
/// <returns></returns>
auto LongUI::UITextBox::TriggerEvent(GuiEvent event) noexcept -> EventAccept {
    switch (event)
    {
    case LongUI::GuiEvent::Event_OnFocus:
        // 更新插入符号位置大小[Focus 肯定有Window了]
        assert(m_pWindow);
        this->show_caret();
        break;
    case LongUI::GuiEvent::Event_OnBlur:
        // 失去焦点时, 如果编辑文本修改则触发[change事件]
        // 注: 每修改一个字符就触发[change事件]对与逻辑层意义不大
        this->try_trigger_change_event();
        // 取消显示插入符号 [Blur 肯定有Window了]
        assert(m_pWindow);
        m_pWindow->HideCaret();
        break;
    }
    return Super::TriggerEvent(event);
}


/// <summary>
/// Triggers the change event.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::try_trigger_change_event() noexcept {
    if (this->is_change_could_trigger()) {
        this->clear_change_could_trigger();
        Super::TriggerEvent(this->_onChange());
    }
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextBox::DoEvent(UIControl * sender,
                          const EventArg & e) noexcept -> EventAccept {
    // 分类讨论
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        // 不会改变
        return Event_Accept;
    case NoticeEvent::Event_DoAccessAction:
        // 默认行为(聚焦)
        this->SetAsDefaultAndFocus();
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // 初始化
        this->init_private();
        this->init_textbox();
        this->clear_change_could_trigger();
        [[fallthrough]];
    default:
        // 基类处理
        return Super::DoEvent(sender, e);
    }
}


/// <summary>
/// Initializes the textbox.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::init_textbox() noexcept {
    const auto cols = static_cast<float>(m_uCols);
    const auto rows = static_cast<float>(m_uRows);
    const auto fs = m_tfBuffer.font.size;
    const auto line_height = LongUI::GetLineHeight(m_tfBuffer.font);
    this->set_contect_minsize({ cols * fs * 0.5f, rows * line_height });
}

/// <summary>
/// Does the input event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextBox::DoInputEvent(InputEventArg e) noexcept -> EventAccept {
    bool op_ok = true;
    switch (e.event)
    {
    case InputEvent::Event_Char:
        op_ok = this->private_char(e.character);
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
    default:
        return Super::DoInputEvent(e);
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
void LongUI::UITextBox::add_attribute(uint32_t key, U8View value) noexcept {
    // 待使用属性列表
    constexpr auto BKDR_SIZE            = 0x0f849a25_ui32;
    constexpr auto BKDR_ROWS            = 0x0f63dd45_ui32;
    constexpr auto BKDR_COLS            = 0x0d614b8f_ui32;
    constexpr auto BKDR_TYPE            = 0x0fab1332_ui32;
    constexpr auto BKDR_VALUE           = 0x246df521_ui32;
    constexpr auto BKDR_CACHED          = 0xf83ab4d6_ui32;
    constexpr auto BKDR_PASSWORD        = 0xa3573bc3_ui32;
    constexpr auto BKDR_READONLY        = 0xad3f7b8a_ui32;
    constexpr auto BKDR_MULTILINE       = 0xb2db9639_ui32;
    constexpr auto BKDR_MAXLENGTH       = 0xb532c6e6_ui32;
    constexpr auto BKDR_PLACEHOLDER     = 0x2aad8773_ui32;
    // 分类讨论
    switch (key)
    {
    case BKDR_CACHED:
        // cached:  LUI优化扩展
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
                this->private_mark_password();
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
    case BKDR_SIZE:
        // size:        单行模式容纳字符数量
        [[fallthrough]];
    case BKDR_COLS:
        // cols:        多行模式下每行容纳字符数量
        m_uCols = static_cast<uint32_t>(value.ToInt32());
        break;
    case BKDR_ROWS:
        // row:         多行模式下容纳的行数
        m_uRows = static_cast<uint32_t>(value.ToInt32());
        break;
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
            return  buf[0];
#endif
        }();
        break;
    default:
       // 其他交给父类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextBox::DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept {
    Point2F pos = { e.px, e.py }; this->MapFromWindow(pos);
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


#if 0
/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UITextBox::DoMouseEvent_(const MouseEventArg& e) noexcept->EventAccept {
    // 鼠标相对本控件位置
    Point2F pt_this = { e.px, e.py };
    this->MapFromWindow(pt_this);

    // 移动滑块
    auto hold_lbtn_move = [this, &pt_this]() noexcept->EventAccept {
        if (m_pHovered) {
            assert(m_pHovered == &this->thumb);
            const auto csize = this->GetBox().GetContentSize();
            const auto ssize = this->thumb.GetSize();
            const int i = this->GetOrient() == Orient_Horizontal ? 0 : 1;
            const auto width = i[&csize.width] - i[&ssize.width];
            const auto x = i[&pt_this.x] - m_fClickOffset;
            const auto value = (x / width) * (m_fMax - m_fMin) + m_fMin;
            this->SetValue(value);
            return Event_Accept;
        }
        else return Event_Ignore;
    };


    // 基类处理基本消息
    switch (e.type)
    {
    case MouseEvent::Event_MouseMove:
        // 鼠标移动 + 左键按下
        if () {
            return hold_lbtn_move();
        }
        break;
    case MouseEvent::Event_LButtonDown:
        this->mouse_click(pt_this);
        break;
    }
    // 基类处理剩余消息
    return Super::DoMouseEvent(e);
}

/// <summary>
/// Mouses the click.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::mouse_click_(Point2F pt) noexcept {
    const auto thumb_pos = this->thumb.GetPos();
    // 在滑块的上方
    if (m_pHovered) {
        assert(*m_pHovered == this->thumb);
        if (this->GetOrient() == Orient_Horizontal)
            m_fClickOffset = pt.x - thumb_pos.x;
        else
            m_fClickOffset = pt.y - thumb_pos.y;
    }
    // 在逻辑前方
    else if (pt.x < thumb_pos.x || pt.y < thumb_pos.y) {
        this->DecreasePage();
    }
    // 在逻辑后方
    else {
        this->IncreasePage();
    }
}
#endif

