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
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UISpacer::UISpacer(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {

}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UISpacer::Render() const noexcept {

}





/// <summary>
/// Initializes a new instance of the <see cref="UIMenuSeparator"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenuSeparator::UIMenuSeparator(
    UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    m_oStyle.appearance = Appearance_MenuSeparator;
}
