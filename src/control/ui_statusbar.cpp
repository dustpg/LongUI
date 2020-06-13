#include <core/ui_ctrlmeta.h>
#include <control/ui_statusbar.h>
#include <control/ui_statusbarpanel.h>


// ui namespace
namespace LongUI {
    // UIStatusBar类 元信息
    LUI_CONTROL_META_INFO(UIStatusBar, "statusbar");
    // UIStatusBarPanel类 元信息
    LUI_CONTROL_META_INFO(UIStatusBarPanel, "statusbarpanel");
}


/// <summary>
/// Initializes a new instance of the <see cref="UIStatusBar"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIStatusBar::UIStatusBar(const MetaControl& meta) noexcept :Super(meta) {
    m_oStyle.appearance = Appearance_StatusBar;
    m_state.orient = Orient_Horizontal;
}

/// <summary>
/// Initializes a new instance of the <see cref="UIStatusBarPanel"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIStatusBarPanel::UIStatusBarPanel(const MetaControl& meta) noexcept :Super(meta) {
    m_oStyle.appearance = Appearance_StatusBarPanel;
}



/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIStatusBarPanel::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_LABEL = 0x74e22f74_ui32;
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    switch (key)
    {
    case BKDR_LABEL:
        // label: 重定向至value
        key = BKDR_VALUE;
        [[fallthrough]];
    default:
        // 其他情况
        return Super::add_attribute(key, value);
    }
}
