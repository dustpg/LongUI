#include <control/ui_viewport.h>
#include <core/ui_string.h>
#include <cassert>


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
/// <param name="parent">The parent.</param>
/// <param name="config">The configuration.</param>
/// <param name="meta">The meta.</param>
LongUI::UIViewport::UIViewport(
    UIControl* parent, 
    CUIWindow::WindowConfig config,
    const MetaControl& meta
) noexcept 
    : Super(parent, meta), m_window(parent->GetWindow(), config) {
    // XXX: m_window 传递 null?
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
    //m_state.destructing = true;

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
        m_window.SetTitleName(CUIString::FromUtf8(value).c_str());
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

