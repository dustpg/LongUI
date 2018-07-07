#include <style/ui_text.h>
#include <style/ui_style_value.h>
#include <core/ui_color_list.h>
#include <graphics/ui_bg_renderer.h>
#include "../private/ui_private_control.h"

#include <core/ui_manager.h>
#include <text/ui_ctl_arg.h>

#undef PCN_NOINLINE
#define PCN_NOINLINE

// longui::detail
namespace LongUI { namespace detail{
    // last
    static UIControl* g_pLastTextFont = nullptr;
    // get text font
    auto get_text_font(UIControl& ctrl) noexcept->TextFont*;
    // get text font
    auto get_text_font(const UIControl& ctrl) noexcept->const TextFont*{
        return get_text_font(const_cast<UIControl&>(ctrl));
    }
}}

PCN_NOINLINE
/// <summary>
/// Sets the color of the fg.
/// </summary>
/// <param name="color">The color.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetFgColor(RGBA color) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
#ifndef NDEBUG
    if (color.primitive == this->GetFgColor().primitive) {
        LUIDebug(Hint) << ctrl << "set same color" << endl;
    }
#endif // !NDEBUG
    // 需要使用g_pLastTextFont
    detail::g_pLastTextFont = nullptr;
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        ColorF::FromRGBA_RT(tf->text.color, color);
        const auto hastf = detail::g_pLastTextFont;
        // 标记颜色修改
        UIControlPrivate::MarkTCChanged(*hastf);
        hastf->NeedUpdate();
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the color of the bg.
/// </summary>
/// <param name="color">The color.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgColor(RGBA color) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        ColorF::FromRGBA_RT(r->color, color);
        ctrl->Invalidate();
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the color of the bg.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetBgColor() const noexcept -> RGBA {
    const auto ctrl = static_cast<const UIControl*>(this);
    if (auto r = UIControlPrivate::GetBgcRenderer(*ctrl)) {
        return r->color.ToRGBA();
    }
    return { RGBA_Transparent };
}

PCN_NOINLINE
/// <summary>
/// Gets the color of the fg.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetFgColor() const noexcept -> RGBA {
    const auto ctrl = static_cast<const UIControl*>(this);
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        return tf->text.color.ToRGBA();
    }
    return { RGBA_Black };
}

PCN_NOINLINE
/// <summary>
/// Sets the bg image.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgImage(uint32_t id) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        r->image_id = id;
        r->RefreshImage();
        ctrl->Invalidate();
    }
}


/// <summary>
/// Gets the bg image.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetBgImage() const noexcept -> uint32_t {
    const auto ctrl = static_cast<const UIControl*>(this);
    if (auto r = UIControlPrivate::GetBgcRenderer(*ctrl)) {
        return r->image_id;
    }
    return 0;
}

PCN_NOINLINE
/// <summary>
/// Gets the bg repeat.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetBgRepeat() const noexcept -> AttributeRepeat {
    const auto ctrl = static_cast<const UIControl*>(this);
    if (auto r = UIControlPrivate::GetBgcRenderer(*ctrl)) {
        return r->repeat;
    }
    return Repeat_Repeat;
}

PCN_NOINLINE
/// <summary>
/// Sets the bg clip.
/// </summary>
/// <param name="clip">The clip.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgClip(AttributeBox clip) noexcept {
    assert(clip < AttributeBox::BOX_COUNT && "out of range");
    const auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        r->clip = clip;
        ctrl->Invalidate();
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the bg clip.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetBgClip() const noexcept->AttributeBox {
    const auto ctrl = static_cast<const UIControl*>(this);
    if (auto r = UIControlPrivate::GetBgcRenderer(*ctrl)) {
        return r->clip;
    }
    return Box_BorderBox;
}


PCN_NOINLINE
/// <summary>
/// Sets the bg origin.
/// </summary>
/// <param name="ar">The ar.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgOrigin(AttributeBox ab) noexcept {
    assert(ab < AttributeBox::BOX_COUNT && "out of range");
    const auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        r->origin = ab;
        ctrl->Invalidate();
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the bg origin.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetBgOrigin() const noexcept->AttributeBox {
    const auto ctrl = static_cast<const UIControl*>(this);
    if (auto r = UIControlPrivate::GetBgcRenderer(*ctrl)) {
        return r->origin;
    }
    return Box_BorderBox;
}


PCN_NOINLINE
/// <summary>
/// Sets the bg attachment.
/// </summary>
/// <param name="aa">The aa.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgAttachment(AttributeAttachment aa) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        r->attachment = aa;
        ctrl->Invalidate();
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the bg repeat.
/// </summary>
/// <param name="ar">The ar.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgRepeat(AttributeRepeat ar) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        r->repeat = ar;
        ctrl->Invalidate();
    }
}


PCN_NOINLINE
/// <summary>
/// Sets the size of the font.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetFontSize(float size) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    // 存在TF对象
    detail::g_pLastTextFont = nullptr;
    if (const auto tf = detail::get_text_font(*ctrl)) {
        tf->font.size = size;
        const auto hastf = detail::g_pLastTextFont;
        UIControlPrivate::MarkTFChanged(*hastf);
        hastf->NeedUpdate();
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the size of the font.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetFontSize() const noexcept->float {
    const auto ctrl = static_cast<const UIControl*>(this);
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        return tf->font.size;
    }
    return UIManager.GetDefaultFont().size;
}


PCN_NOINLINE
/// <summary>
/// Sets the font style.
/// </summary>
/// <param name="style">The style.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetFontStyle(AttributeFontStyle style) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    // 存在TF对象
    detail::g_pLastTextFont = nullptr;
    if (const auto tf = detail::get_text_font(*ctrl)) {
        tf->font.style = style;
        const auto hastf = detail::g_pLastTextFont;
        UIControlPrivate::MarkTFChanged(*hastf);
        hastf->NeedUpdate();
    }
}



PCN_NOINLINE
/// <summary>
/// Gets the font style.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetFontStyle()const noexcept->AttributeFontStyle {
    const auto ctrl = static_cast<const UIControl*>(this);
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        return tf->font.style;
    }
    return UIManager.GetDefaultFont().style;
}

PCN_NOINLINE
/// <summary>
/// Sets the font family.
/// </summary>
/// <param name="family">The family.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetFontFamily(const char* family) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    // 存在TF对象
    detail::g_pLastTextFont = nullptr;
    if (const auto tf = detail::get_text_font(*ctrl)) {
        tf->font.family = family;
        const auto hastf = detail::g_pLastTextFont;
        UIControlPrivate::MarkTFChanged(*hastf);
        hastf->NeedUpdate();
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the font style.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetFontFamily()const noexcept->const char* {
    const auto ctrl = static_cast<const UIControl*>(this);
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        return tf->font.family;
    }
    return UIManager.GetDefaultFont().family;
}

PCN_NOINLINE
/// <summary>
/// Sets the font weight.
/// </summary>
/// <param name="style">The weight.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetFontWeight(AttributeFontWeight weight) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    // 存在TF对象
    detail::g_pLastTextFont = nullptr;
    if (const auto tf = detail::get_text_font(*ctrl)) {
        tf->font.weight = weight;
        const auto hastf = detail::g_pLastTextFont;
        UIControlPrivate::MarkTFChanged(*hastf);
        hastf->NeedUpdate();
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the font weight.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetFontWeight()const noexcept->AttributeFontWeight {
    const auto ctrl = static_cast<const UIControl*>(this);
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        return tf->font.weight;
    }
    return UIManager.GetDefaultFont().weight;
}

PCN_NOINLINE
/// <summary>
/// Sets the font stretch.
/// </summary>
/// <param name="style">The stretch.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetFontStretch(AttributeFontStretch stretch) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    // 存在TF对象
    detail::g_pLastTextFont = nullptr;
    if (const auto tf = detail::get_text_font(*ctrl)) {
        tf->font.stretch = stretch;
        const auto hastf = detail::g_pLastTextFont;
        UIControlPrivate::MarkTFChanged(*hastf);
        hastf->NeedUpdate();
    }
}

PCN_NOINLINE
/// <summary>
/// Gets the font stretch.
/// </summary>
/// <returns></returns>
auto LongUI::CUIStyleValue::GetFontStretch()const noexcept->AttributeFontStretch {
    const auto ctrl = static_cast<const UIControl*>(this);
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        return tf->font.stretch;
    }
    return UIManager.GetDefaultFont().stretch;
}

/// <summary>
/// Afters the box changed.
/// </summary>
void LongUI::CUIStyleValue::after_box_changed() {
    const auto ctrl = static_cast<UIControl*>(this);
    UIControlPrivate::MarkWindowMinsizeChanged(*ctrl);
    ctrl->NeedUpdate();
}

/// <summary>
/// Sets the margin top.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetMarginTop(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.margin.top = value;
    this->after_box_changed();
}


/// <summary>
/// Sets the margin left.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetMarginLeft(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.margin.left = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the margin right.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetMarginRight(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.margin.right = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the margin bottom.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetMarginBottom(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.margin.bottom = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the padding top.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetPaddingTop(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.padding.top = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the padding left.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetPaddingLeft(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.padding.left = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the padding right.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetPaddingRight(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.padding.right = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the padding bottom.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetPaddingBottom(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.padding.bottom = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the border top.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBorderTop(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.border.top = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the border left.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBorderLeft(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.border.left = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the border right.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBorderRight(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.border.right = value;
    this->after_box_changed();
}

/// <summary>
/// Sets the border bottom.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBorderBottom(float value) noexcept {
    const auto ctrl = static_cast<UIControl*>(this);
    auto& box = const_cast<Box&>(ctrl->GetBox());
    box.border.bottom = value;
    this->after_box_changed();
}

// longui::detail
namespace LongUI { namespace detail {
    /// <summary>
    /// Gets the text font.
    /// </summary>
    /// <param name="ctrl">The control.</param>
    /// <returns></returns>
    auto get_text_font(UIControl& ctrl) noexcept->TextFont* {
        auto& style = ctrl.GetStyle();
        // 存在Text对象
        if (style.offset_tf) {
            g_pLastTextFont = &ctrl;
            const auto tf = const_cast<Style&>(style).GetTextFont();
            return tf;
        }
        // 如果是state.atomicity则查看
        else if (UIControlPrivate::IsAtomicity(ctrl)) {
            for (auto& x : ctrl) {
                if (const auto tf = get_text_font(x))
                    return tf;
            }
        }
        // 没有就没有咯(摊手)
        return nullptr;
    }
}}