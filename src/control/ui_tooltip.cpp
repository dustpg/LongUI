#include <core/ui_ctrlmeta.h>
#include <control/ui_label.h>
#include <control/ui_tooltip.h>


// ui namespace
namespace LongUI {
    // UITooltip类 元信息
    LUI_CONTROL_META_INFO(UITooltip, "tooltip");
    // common tooltip
    struct UICommonTooltip : UITooltip {
        // ctor
        UICommonTooltip(UIControl& hoster) noexcept;
        // dtor
        ~UICommonTooltip() noexcept;
        // label
        UILabel             label;
    };
    /// <summary>
    /// Initializes a new instance of the <see cref="UICommonTooltip" /> struct.
    /// </summary>
    /// <param name="hoster">The hoster.</param>
    UICommonTooltip::UICommonTooltip(UIControl& hoster) noexcept :
        UITooltip(&hoster), label(this) {
    }
    /// <summary>
    /// Finalizes an instance of the <see cref="UICommonTooltip"/> class.
    /// </summary>
    /// <returns></returns>
    UICommonTooltip::~UICommonTooltip() noexcept {
        // 有额外的成员控件label, 需要标记析构中
        m_state.destructing = true;
    }
    /// <summary>
    /// Commons the tooltip create.
    /// </summary>
    /// <returns></returns>
    auto CommonTooltipCreate(UIControl& hoster) noexcept -> UIViewport* {
        const auto obj = new(std::nothrow) UICommonTooltip{ hoster };
        return obj;
    }
    /// <summary>
    /// Commons the tooltip set text.
    /// </summary>
    /// <param name="viewport">The viewport.</param>
    /// <param name="text">The text.</param>
    /// <returns></returns>
    void CommonTooltipSetText(UIViewport& viewport, CUIString&& text) noexcept {
        const auto ptr = &viewport;
        const auto obj = static_cast<UICommonTooltip*>(ptr);
        obj->label.SetText(std::move(text));
    }
}

/// <summary>
/// Finalizes an instance of the <see cref="UITooltip"/> class.
/// </summary>
/// <returns></returns>
LongUI::UITooltip::~UITooltip() noexcept {
}

/// <summary>
/// Initializes a new instance of the <see cref="UITooltip"/> class.
/// </summary>
/// <param name="hoster">The hoster.</param>
/// <param name="meta">The meta.</param>
LongUI::UITooltip::UITooltip(UIControl* hoster, const MetaControl& meta)
    noexcept: Super(*hoster, 
        CUIWindow::Config_FixedSize | CUIWindow::Config_Popup,
        meta) {
    // XXX: 设置清除色
    m_window.SetClearColor({ 1.f, 1.f, 1.f, 1.f });
}

