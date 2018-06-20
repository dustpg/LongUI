#include <control/ui_label.h>
#include <control/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
#include <core/ui_manager.h>

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


/// <summary>
/// Initializes a new instance of the <see cref="UILabel" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UILabel::UILabel(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    // 本控件支持font属性
    LongUI::MakeDefault(m_tfBuffer);
    UILabel* const nilobj = nullptr;
    const auto style_offset = reinterpret_cast<char*>(&nilobj->m_oStyle);
    const auto tfbuf_offset = reinterpret_cast<char*>(&nilobj->m_tfBuffer);
    assert(tfbuf_offset > style_offset);
    const auto offset_tf = tfbuf_offset - style_offset;
    m_oStyle.offset_tf = static_cast<uint16_t>(offset_tf);
    // 写入默认外间距
    m_oBox.margin = { 6, 1, 5, 2 };
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
void  LongUI::UILabel::Update() noexcept {
    // TODO: 处理BOX修改 SpecifyMinContectSize

    // 检查到大小修改
    if (this->is_size_changed()) {
        m_text.Resize(this->GetBox().GetContentSize());
    }
    // 父类修改
    Super::Update();
    // 次帧刷新
    //this->NextUpdate();
    // 处理大小修改
    this->size_change_handled();
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UILabel::DoEvent(
    UIControl* sender, 
    const EventArg& e) noexcept -> EventAccept {
    // 分类讨论
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        // 不会改变
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // 初始化
        this->init_label();
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
    const auto& fontsize = UIManager.GetDefualtFont();
    const auto lineheight = LongUI::GetLineHeight(fontsize);
    this->set_contect_minsize({ lineheight * 2.f, lineheight });
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
bool LongUI::UILabel::SetText(WcView text) noexcept {
    return this->SetText(CUIString{ text });
}

PCN_NOINLINE
/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
bool LongUI::UILabel::SetText(CUIString&& text) noexcept {
    if (m_string == text) return false;
    m_string = std::move(text);
    auto hr = m_text.SetText(m_string.c_str(), m_string.length());
    this->after_set_text();
    // TODO: hr错误处理
    assert(hr);
    return true;
}


/// <summary>
/// Initializes the label.
/// </summary>
/// <returns></returns>
void LongUI::UILabel::init_label() noexcept {
    // TODO: 错误检查

    // 设置初始化数据
    m_text.SetFont(m_tfBuffer.font, m_string.c_str(), m_string.length());
    // 设置基本属性
    this->after_set_text();
    // 获取关系控件
    if (m_pWindow) m_control.FindControl(*m_pWindow);
}

PCN_NOINLINE
/// <summary>
/// Afters the set text.
/// </summary>
/// <returns></returns>
void LongUI::UILabel::after_set_text() noexcept {
    const auto size = m_text.GetSize();
    const auto width = std::ceil(size.width);
    const auto height = std::ceil(size.height);
    this->set_contect_minsize({ width, height });
    this->mark_window_minsize_changed();
    //this->NeedUpdate();
    //this->Invalidate();
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
#if 0
    constexpr auto BKDR_ACCESSKEY   = 0xba56ab7b_ui32;
    assert(!"UIControl already BKDR_ACCESSKEY");
#endif
    // 分类讨论
    switch (key)
    {
    case BKDR_VALUE:
        // value
        m_string = CUIString::FromUtf8(value);
        return;
#if 0
    case BKDR_ACCESSKEY:
        // accesskey
        // 访问按键, 仅支持ASCII(严格情况A-Z)
        if (attr.value.begin() != attr.value.end()) {
            const int8_t akey = *attr.value.begin();
            m_accesskey = akey >= 0 ? akey : 0;
        }
        return;
#endif
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
        *static_cast<const get2_t&>(args).name = this->GetTextString();
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
