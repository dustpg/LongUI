#include <control/ui_spacer.h>
#include <control/ui_menuseparator.h>
#include <core/ui_ctrlmeta.h>


// ui namespace
namespace LongUI {
    // UISpacer类 元信息
    LUI_CONTROL_META_INFO(UISpacer, "spacer");
    // UIMenuSeparator类 元信息
    LUI_CONTROL_META_INFO(UIMenuSeparator, "menuseparator");
}


/// <summary>
/// Finalizes an instance of the <see cref="UISpacer"/> class.
/// </summary>
/// <returns></returns>
LongUI::UISpacer::~UISpacer() noexcept {

}


/// <summary>
/// Initializes a new instance of the <see cref="UISpacer" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UISpacer::UISpacer(const MetaControl& meta) noexcept : Super(meta) {

}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UISpacer::Render() const noexcept {
    // 不渲染
}


/// <summary>
/// 设置 Spacer 数据
/// </summary>
/// <param name="size"></param>
/// <param name="flex"></param>
/// <returns></returns>
void LongUI::UISpacer::SetSpacer(Size2F size, float flex) noexcept {
    m_oStyle.flex = flex;
    m_oStyle.limited = size;
    constexpr uint8_t flags = uint8_t(4 << 2) | uint8_t(4 << 3);
    reinterpret_cast<uint8_t&>(m_oStyle.overflow_xex) |= flags;
}

/// <summary>
/// Initializes a new instance of the <see cref="UIMenuSeparator"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIMenuSeparator::UIMenuSeparator(const MetaControl& meta) noexcept : Super(meta) {
    // XXX: 换一个?
    m_oStyle.appearance = Appearance_MenuSeparator;
}
