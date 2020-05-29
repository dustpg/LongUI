#include <cmath>
#include <debugger/ui_debug.h>
#include <control/ui_label.h>
#include <core/ui_ctrlmeta.h>
#include <core/ui_manager.h>
#include <core/ui_window.h>

#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
//#include <core/ui_string.h>
#endif

// ui namespace
namespace LongUI {
    // UILabel类 元信息
    LUI_CONTROL_META_INFO(UILabel, "label");
}

// open href
extern "C" void longui_open_href(const char* ref) noexcept;


/// <summary>
/// Initializes a new instance of the <see cref="UILabel" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UILabel::UILabel(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta), m_hrefCursor(CUICursor::Cursor_Hand) {
    // 本控件支持font属性
    LongUI::MakeDefault(luiref m_tfBuffer);
    UILabel* const nilobj = nullptr;
    const auto style_offset = reinterpret_cast<char*>(&nilobj->m_oStyle);
    const auto tfbuf_offset = reinterpret_cast<char*>(&nilobj->m_tfBuffer);
    assert(tfbuf_offset > style_offset);
    const auto offset_tf = tfbuf_offset - style_offset;
    m_oStyle.offset_tf = static_cast<uint16_t>(offset_tf);
    // 写入默认外间距
    m_oBox.margin = { 6, 1, 5, 2 };
    // 构造锁
    impl::ctor_unlock();
}

/// <summary>
/// Finalizes an instance of the <see cref="UILabel"/> class.
/// </summary>
/// <returns></returns>
LongUI::UILabel::~UILabel() noexcept {
}



/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void  LongUI::UILabel::Update(UpdateReason reason) noexcept {
    constexpr UpdateReason need_redraw 
        = Reason_TextFontDisplayChanged
        | Reason_TextFontLayoutChanged
        | Reason_ValueTextChanged
        ;
    // 需要刷新显示
    if (reason & need_redraw) this->Invalidate();
    // 文本布局 修改了
    if (reason & Reason_TextFontLayoutChanged) this->reset_font();
    // 文本修改了
    if (reason & Reason_ValueTextChanged) this->on_text_changed();
    // 检查到大小修改
    if (reason & Reason_SizeChanged) 
        m_text.Resize(this->RefBox().GetContentSize());
    // TODO: 处理BOX修改 SpecifyMinContectSize

    // 超类处理
    Super::Update(reason);
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UILabel::DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept {
    Point2F pos = { e.px, e.py }; this->MapFromWindow(pos);
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        assert(m_pWindow && "no window no mouse");
        if (this->is_def_href()) {
            m_pWindow->SetNowCursor(m_hrefCursor);
            const auto u32l = static_cast<uint32_t>(m_string.length());
            m_text.SetUnderline(0, u32l, true);
            this->Invalidate();
            //return Event_Accept;
        }
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        assert(m_pWindow && "no window no mouse");
        if (this->is_def_href()) {
            m_pWindow->SetNowCursor(nullptr);
            const auto u32l = static_cast<uint32_t>(m_string.length());
            m_text.SetUnderline(0, u32l, false);
            this->Invalidate();
            //return Event_Accept;
        }
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        if (!m_href.empty()) ::longui_open_href(m_href.c_str());
        break;
    }
    return Super::DoMouseEvent(e);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UILabel::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
    // 分类讨论
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        // 不会改变
        return Event_Accept;
    case NoticeEvent::Event_DoAccessAction:
        // 检测超链接
        if (!m_href.empty()) ::longui_open_href(m_href.c_str());
        // 访问连接控件
        if (m_control.ctrl) 
            return m_control.ctrl->DoEvent(this, e);
        return Event_Accept;
    case NoticeEvent::Event_ShowAccessKey:
        // 显示/隐藏访问键
        this->ShowAccessKey(e.derived & 1);
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // 初始化
        if (!m_string.empty()) {
            this->SetText(CUIString{ std::move(m_string) });
            this->NeedUpdate(Reason_ValueTextChanged);
        }
        m_control.FindControl(m_pWindow);
        [[fallthrough]];
    default:
        // 基类处理
        return Super::DoEvent(sender, e);
    }
}

/// <summary>
/// Sets as default minsize.
/// </summary>
/// <returns></returns>
void LongUI::UILabel::SetAsDefaultMinsize() noexcept {
    const auto& fontsize = UIManager.RefDefaultFont();
    const auto lineheight = LongUI::GetLineHeight(fontsize);
    this->set_contect_minsize({ lineheight * 2.f, lineheight });
}


/// <summary>
/// Setups the access key display.
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
void LongUI::UILabel::setup_access_key() noexcept {
    const auto ch = this->GetAccessKey();
    if (ch >= 'A' && ch <= 'Z') {
        // 查找字符串是否存在指定字符
        m_uPosAkey = 0;
        for (auto c : m_string) {
            if (c >= 'a') c -= 'a' - 'A';
            if (c == ch) return;
            m_uPosAkey++;
        }
        // 添加字符串
        m_uPosAkey++;
    }
}

/// <summary>
/// Shows the access key.
/// </summary>
/// <param name="show">if set to <c>true</c> [show].</param>
/// <returns></returns>
void LongUI::UILabel::ShowAccessKey(bool show) noexcept {
    if (this->GetAccessKey() && m_text) {
        m_text.SetUnderline(m_uPosAkey, 1, show);
        this->Invalidate();
    }
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
bool LongUI::UILabel::SetText(const CUIString & text) noexcept {
    return this->SetText(CUIString{ text });
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
bool LongUI::UILabel::SetText(U16View text) noexcept {
    return this->SetText(CUIString(text));
}

// longui::detail
namespace LongUI { namespace detail{
    // append access key
    inline void append_ass_key(CUIString& str, char key) noexcept {
        char16_t buf[3];
        buf[0] = '(';
        buf[1] = key;
        buf[2] = ')';
        str.append(buf, buf + 3);
    }
}}

/// <summary>
/// Ons the text changed.
/// </summary>
/// <returns></returns>
void LongUI::UILabel::on_text_changed() noexcept {
    // 检查访问键
    this->setup_access_key();
    // 需要额外的字符
    const auto base_len = m_string.length();
    if (m_uPosAkey == base_len + 1)
        detail::append_ass_key(m_string, this->GetAccessKey());
    // 创建文本布局
    auto hr = m_text.SetText(m_string.c_str(), m_string.length());
    m_string.erase(base_len);
    // 设置字体
    this->after_set_text();
    // 需要渲染
    this->Invalidate();
    // TODO: hr错误处理
    assert(hr);
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
bool LongUI::UILabel::SetText(CUIString&& text) noexcept {
    // 相同自然不需要
    if (m_string == text) return false;
    m_string = std::move(text);
    this->NeedUpdate(Reason_ValueTextChanged);
    return true;
}

/// <summary>
/// Initializes the label.
/// </summary>
/// <returns></returns>
void LongUI::UILabel::reset_font() noexcept {
    // TODO: 错误检查

    // 设置初始化数据
    m_text.SetFont(m_tfBuffer, m_string.c_str(), m_string.length());
    // 设置基本属性
    this->after_set_text();

}

PCN_NOINLINE
/// <summary>
/// Afters the set text.
/// </summary>
/// <returns></returns>
void LongUI::UILabel::after_set_text() noexcept {
    const auto size = m_text.GetSize();
    const Size2F ceil_size{ std::ceil(size.width) , std::ceil(size.height) };
    if (LongUI::IsSameInGuiLevel(m_szOld, ceil_size)) return;
    this->set_contect_minsize(m_szOld = ceil_size);
    this->mark_window_minsize_changed();
    this->mark_world_changed();
#ifdef LUI_ACCESSIBLE
    LongUI::Accessible(m_pAccessible, Callback_PropertyChanged);
#endif
    
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UILabel::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_VALUE       = 0x246df521_ui32;
    constexpr auto BKDR_HREF        = 0x0e0d950f_ui32;

    // 分类讨论
    switch (key)
    {
    case BKDR_VALUE:
        // value
        m_string = CUIString::FromUtf8(value);
        break;
    case BKDR_HREF:
        // href
        m_href = value;
        break;
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
auto LongUI::UILabel::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
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
            AccessibleControlType::Type_Text;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetAccessibleName:
        // 获取Acc名称
        *static_cast<const get2_t&>(args).name = this->RefText();
        return Event_Accept;
    //case AccessibleEvent::Event_Value_SetValue:
    //    // 设置值
    //    this->SetText(
    //        static_cast<const AccessibleVSetValueArg&>(args).string,
    //        static_cast<const AccessibleVSetValueArg&>(args).length
    //    );
    //    return Event_Accept;
    //case AccessibleEvent::Event_Value_GetValue:
    //    // 读取值
    //    *static_cast<const AccessibleVGetValueArg&>(args).value =
    //        this->GetTextString();
    //    return Event_Accept;
    }
    return Super::accessible(args);
}

#endif
