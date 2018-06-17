#include <style/ui_style_value.h>
#include <graphics/ui_bg_renderer.h>
#include "../private/ui_private_control.h"


PCN_NOINLINE
/// <summary>
/// Sets the color of the bg.
/// </summary>
/// <param name="color">The color.</param>
/// <returns></returns>
void LongUI::CUIStyleValue::SetBgColor(RGBA color) noexcept {
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        ColorF cf; ColorF::FromRGBA_RT(cf, color);
        r->SetColor(cf);
        ctrl->Invalidate();
    }
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
        r->SetImage(id);
        ctrl->Invalidate();
    }
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
        r->SetClip(clip);
        ctrl->Invalidate();
    }
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
        r->SetOrigin(ab);
        ctrl->Invalidate();
    }
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
        r->SetAttachment(aa);
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
    assert(ar < AttributeRepeat::REPEAT_COUNT && "out of range");
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = UIControlPrivate::EnsureBgcRenderer(*ctrl)) {
        r->SetRepeat(ar);
        ctrl->Invalidate();
    }
}

