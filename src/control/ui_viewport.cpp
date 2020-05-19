#include <control/ui_viewport.h>
#include <core/ui_ctrlmeta.h>
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
}

/// <summary>
/// Initializes a new instance of the <see cref="UIViewport" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="config">The configuration.</param>
LongUI::UIViewport::UIViewport(CUIWindow* parent, CUIWindow::WindowConfig config) noexcept
    : Super(impl::ctor_lock(nullptr)), 
    m_nSubview({ static_cast<UIControl*>(&m_nSubview), static_cast<UIControl*>(&m_nSubview) }),
    m_window(parent, config) {
    m_pWindow = &m_window;
    m_state.orient = Orient_Vertical;

    // 构造锁
    impl::ctor_unlock();
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
    const MetaControl& meta) noexcept 
    : Super(impl::ctor_lock(&pseudo_parent), meta), m_pHoster(&pseudo_parent),
    m_nSubview({ static_cast<UIControl*>(&m_nSubview), static_cast<UIControl*>(&m_nSubview) }),
    m_window(pseudo_parent.GetWindow(), config) {

    assert(m_pParent == nullptr);
    m_pWindow = &m_window;
    m_state.orient = Orient_Vertical;

    // 构造锁
    impl::ctor_unlock();
}

/// <summary>
/// Adjusts the size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
auto LongUI::UIViewport::AdjustSize(Size2F size) const noexcept -> Size2L {
    const auto w = size.width  * m_mtWorld._11;
    const auto h = size.height * m_mtWorld._22;
    const auto wl = static_cast<long>(w + 0.5f);
    const auto hl = static_cast<long>(h + 0.5f);
    return { wl, hl };
}

/// <summary>
/// Adjusts the size of the zoomed.
/// </summary>
/// <param name="scale">The scale.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
auto LongUI::UIViewport::AdjustZoomedSize(Size2F scale, Size2L size) const noexcept -> Size2L {
    const auto size_w = float(size.width);
    const auto size_h = float(size.height);
    const auto just_w = size_w * scale.width / m_mtWorld._11;
    const auto just_h = size_h * scale.height / m_mtWorld._22;
    const auto long_w = static_cast<long>(just_w + 0.5f);
    const auto long_h = static_cast<long>(just_h + 0.5f);
    return { long_w, long_h };
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIViewport::DoEvent(
    UIControl* sender, const EventArg & e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        m_window.init();
        [[fallthrough]];
    }
    return Super::DoEvent(sender, e);
}

/// <summary>
/// Resizes the window.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIViewport::resize_window(Size2F size) noexcept {
    //m_pWindow = &m_window;
    m_szReal = size;
    size.width /= m_mtWorld._11;
    size.height /= m_mtWorld._22;
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
    auto node = m_nSubview.next;
    while (node != &m_nSubview) {
        assert(LongUI::IsViewport(*node));
        const auto view = static_cast<UIViewport*>(node);
        node = view->next;
        delete view;
    }
#ifndef NDEBUG
    m_nSubview = { nullptr, nullptr };
#endif
}

/// <summary>
/// Adds the subviewport.
/// </summary>
/// <param name="sub">The sub.</param>
/// <returns></returns>
void LongUI::UIViewport::AddSubViewport(UIViewport& sub) noexcept {
    // 连接前后节点
    static_cast<UIViewport*>(m_nSubview.prev)->next = &sub;
    sub.prev = static_cast<UIViewport*>(m_nSubview.prev);
    sub.next = static_cast<UIControl*>(&m_nSubview);
    m_nSubview.prev = &sub;
}


/// <summary>
/// Finds the sub viewport.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::UIViewport::FindSubViewport(U8View view) const noexcept -> UIViewport* {
    const auto unistr = UIManager.GetUniqueText(view).id;
    return UIViewport::FindSubViewport(m_nSubview.next, m_nSubview, unistr);
}

/// <summary>
/// Resets the zoom.
/// </summary>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <returns></returns>
void LongUI::UIViewport::JustResetZoom(float x, float y) noexcept {
    m_mtWorld._11 = x  ; m_mtWorld._12 = 0.f;
    m_mtWorld._21 = 0.f; m_mtWorld._22 = y  ;
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
        // Viewport::add_attribute部分操作对象属于窗口, 允许在初始化后调用
        if (this->is_inited()) return;
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
/// Hosters the popup begin.
/// </summary>
/// <returns></returns>
void LongUI::UIViewport::HosterPopupBegin() noexcept {
    // 提示Hoster窗口准备打开了
    if (m_pHoster) m_pHoster->DoEvent(
        this, { NoticeEvent::Event_PopupBegin, 0 });
}

/// <summary>
/// Hosters the popup end.
/// </summary>
/// <returns></returns>
void LongUI::UIViewport::HosterPopupEnd() noexcept {
    // 提示Hoster窗口准备关闭
    if (m_pHoster) m_pHoster->DoEvent(
        this, { NoticeEvent::Event_PopupEnd, 0 });
    m_pHoster = nullptr;
}

/// <summary>
/// Windows the closed.
/// </summary>
/// <returns></returns>
void LongUI::UIViewport::WindowClosed() noexcept {
    this->HosterPopupEnd();
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

PCN_NOINLINE
/// <summary>
/// Finds the sub viewport.
/// </summary>
/// <param name="node">The node.</param>
/// <param name="headtail">The headtail.</param>
/// <param name="name">The name.</param>
/// <returns></returns>
auto LongUI::UIViewport::FindSubViewport(UIControl* node,
    const Node<UIControl>& headtail,
    const char* name) noexcept -> UIViewport * {
    // 遍历所有节点
    while (node != &headtail) {
        assert(LongUI::IsViewport(*node));
        const auto view = static_cast<UIViewport*>(node);

        // 唯一字符串用==判断
        if (view->GetID().id == name) return view;
#ifndef NDEBUG
        if (!std::strcmp(view->GetID().id, name)) {
            assert(!"use UIManager.GetUniqueText");
        }
#endif
        node = view->next;
    }
    return nullptr;
}
