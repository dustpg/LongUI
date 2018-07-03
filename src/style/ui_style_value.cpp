#include <style/ui_text.h>
#include <style/ui_style_value.h>
#include <core/ui_color_list.h>
#include <graphics/ui_bg_renderer.h>
#include "../private/ui_private_control.h"

//#undef PCN_NOINLINE

// longui::detail
namespace LongUI { namespace detail{
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
    // 存在TF对象
    if (const auto tf = detail::get_text_font(*ctrl)) {
        ColorF::FromRGBA_RT(tf->text.color, color);
        ctrl->Invalidate();
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the color of the bg.
/// </summary>
/// <param name="color">The color.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgColor(RGBA color) noexcept {
    auto ctrl = static_cast<UIControl*>(this);
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
    auto ctrl = static_cast<const UIControl*>(this);
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
    auto ctrl = static_cast<UIControl*>(this);
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
    auto ctrl = static_cast<const UIControl*>(this);
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
    auto ctrl = static_cast<const UIControl*>(this);
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
    auto ctrl = static_cast<UIControl*>(this);
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
    auto ctrl = static_cast<const UIControl*>(this);
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
    auto ctrl = static_cast<UIControl*>(this);
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
    auto ctrl = static_cast<const UIControl*>(this);
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
    auto ctrl = static_cast<UIControl*>(this);
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
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        r->repeat = ar;
        ctrl->Invalidate();
    }
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