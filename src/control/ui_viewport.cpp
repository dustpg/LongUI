#include <control/ui_viewport.h>
#include <control/ui_ctrlmeta.h>
#include <core/ui_manager.h>
#include <core/ui_string.h>
#include <cassert>

// ui namespace
namespace LongUI {
    // UIMenuList类 元信息
    LUI_CONTROL_META_INFO_NO(UIViewport, "viewport");
    // 视口?
    bool IsViewport(const UIControl& ctrl) noexcept {
        return !!uisafe_cast<const UIViewport>(&ctrl);
    }
    // 添加视口
    void AddSubViewport(CUIWindow& win, UIControl& ctrl) noexcept {
        const auto view = longui_cast<UIViewport*>(&ctrl);
        win.RefViewport().AddSubViewport(*view);
    }
    // detail namespace
    namespace detail {
        PCN_NOINLINE
        // find viewport
        UIViewport* find_viewport(const POD::Vector<UIViewport*>& v, 
            const char* name) noexcept {
            for (auto x : v) {
                // 唯一字符串用==判断
                if (x->GetID() == name) return x;
#ifndef NDEBUG
                if (!std::strcmp(x->GetID(), name)) {
                    assert(!"use UIManager.GetUniqueText");
                }
#endif
            }
            return nullptr;
        }
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="UIViewport" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="config">The configuration.</param>
LongUI::UIViewport::UIViewport(CUIWindow* parent, CUIWindow::WindowConfig config) noexcept
    : Super(nullptr), m_window(parent, config) {
    m_pWindow = &m_window;
    this->SetOrient(Orient_Vertical);
}

/// <summary>
/// Initializes a new instance of the <see cref="UIViewport" /> class.
/// </summary>
/// <param name="pseudo_parent">The pseudo parent.</param>
/// <param name="config">The configuration.</param>
/// <param name="meta">The meta.</param>
LongUI::UIViewport::UIViewport(
    UIControl& pseudo_parent,
    CUIWindow::WindowConfig config,
    const MetaControl& meta
) noexcept 
    : Super(&pseudo_parent, meta), m_pHoster(&pseudo_parent),
    m_window(pseudo_parent.GetWindow(), config) {
    assert(m_pParent == nullptr);
    m_pWindow = &m_window;
    this->SetOrient(Orient_Vertical);
}


/// <summary>
/// Resizes the window.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIViewport::resize_window(Size2F size) noexcept {
    //m_pWindow = &m_window;
    this->Resize(size);
    m_window.m_pTopestWcc = this;
    m_state.world_changed = true;
    //m_window.SetControlWorldChanged(*this);
}


/// <summary>
/// Finalizes an instance of the <see cref="UIViewport"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIViewport::~UIViewport() noexcept {
    m_state.destructing = true;
    // 释放sub列表?
    for (auto x : m_subviewports) delete x;
    m_subviewports.clear();
}

/// <summary>
/// Adds the subviewport.
/// </summary>
/// <param name="sub">The sub.</param>
/// <returns></returns>
void LongUI::UIViewport::AddSubViewport(UIViewport& sub) noexcept {
    const auto ptr = &sub;
    // XXX: OOM处理
    m_subviewports.push_back(ptr);
}


/// <summary>
/// Finds the sub viewport with unistr.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::UIViewport::FindSubViewportWithUnistr(
    const char* str) const noexcept -> UIViewport * {
    return detail::find_viewport(m_subviewports, str);
}


/// <summary>
/// Finds the sub viewport.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::UIViewport::FindSubViewport(U8View view) const noexcept -> UIViewport* {
    const auto unistr = UIManager.GetUniqueText(view);
    return this->FindSubViewportWithUnistr(unistr);
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIViewport::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_CLEARCOLOR = 0xebcedc8e_ui32;
    constexpr auto BKDR_TITLE      = 0x02670904_ui32;
    // 分类处理
    switch (key)
    {
        ColorF color;
    case BKDR_CLEARCOLOR:
        // clearcolor       : 窗口清除色
        ColorF::FromRGBA_RT(color, { value.ColorRGBA32() });
        m_window.SetClearColor(color);
        break; 
    case BKDR_TITLE:
        // title            : 窗口标题
        m_window.SetTitleName(CUIString::FromUtf8(value));
        break;
    //case BKDR_SIZEMODE: 0x98a4632c_ui32
    //    // sizemode         : maximized/normal
    //    break;
    default:
        // 其他的交给父类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// Gets the viewport.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWindow::RefViewport() noexcept -> UIViewport& {
    assert(this && "null this pointer");
    const auto ptr = reinterpret_cast<char*>(this);
    const auto offset = offsetof(UIViewport, m_window);
    const auto viewport = ptr - offset;
    return *reinterpret_cast<UIViewport*>(viewport);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto LongUI::UIViewport::DoEvent(
    UIControl* sender, const EventArg& arg) noexcept -> EventAccept {
    switch (arg.nevent)
    {
    case NoticeEvent::Event_WindowClosed:
        // 提示Hoster窗口关闭了
        if (m_pHoster) m_pHoster->DoEvent(
            this, { NoticeEvent::Event_PopupEnd, 0 });
        // 清除当前Hoster
        m_pHoster = nullptr;
        return Event_Accept;
    }
    return Super::DoEvent(sender, arg);
}

/// <summary>
/// Hosters the popup begin.
/// </summary>
/// <returns></returns>
void LongUI::UIViewport::HosterPopupBegin() noexcept {
    // 提示Hoster窗口准备打开了
    if (m_pHoster) m_pHoster->DoEvent(
        this, { NoticeEvent::Event_PopupBegin, 0 });
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UIViewport::Recreate(bool release_only) noexcept -> Result {
    // 释放窗口数据
    m_window.release_window_only_device();
    // 继承调用
    auto hr = Super::Recreate(release_only);
    // 仅仅释放
    if (release_only) return hr;
    // 重建/初始化 窗口资源
    if (hr) {
        hr = m_window.recreate_window();
    }
    return hr;
}

/// <summary>
/// Subs the viewport popup begin.
/// </summary>
/// <param name="view">The view.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::UIViewport::SubViewportPopupBegin(
    UIViewport& view, PopupType type) noexcept {
}

/// <summary>
/// Subs the viewport popup end.
/// </summary>
/// <param name="view">The view.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::UIViewport::SubViewportPopupEnd(
    UIViewport& view, PopupType type) noexcept {
}