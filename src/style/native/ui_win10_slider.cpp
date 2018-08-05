// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"

/// <summary>
/// Draws the slider track.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_slider_track(
    const NativeDrawArgs& args, bool vertical) noexcept {
    // 颜色处理
    constexpr auto border_color     = 0xd6d6d6ff_rgba;
    constexpr auto background_color = 0xe7eaeaff_rgba;
    const auto bdc = ColorF::FromRGBA_CT<border_color>();
    const auto bgc = ColorF::FromRGBA_CT<background_color>();
    // 都是纯色，全用Fill
    RectF rect;
    if (vertical) {
        const float center_x = (args.border.left + args.border.right) * 0.5f;
        rect.top    = args.border.top;
        rect.bottom = args.border.bottom;
        rect.left   = center_x - SLIDER_TRACK_HHH;
        rect.right  = center_x + SLIDER_TRACK_HHH;
    }
    else {
        const float center_y = (args.border.top + args.border.bottom) * 0.5f;
        rect.left   = args.border.left;
        rect.right  = args.border.right;
        rect.top    = center_y - SLIDER_TRACK_HHH;
        rect.bottom = center_y + SLIDER_TRACK_HHH;
    }
    // 边框
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect),
        &UIManager.RefCCBrush(bdc)
    );
    // 背景
    rect.top++; rect.left++; rect.right--; rect.bottom--;
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect),
        &UIManager.RefCCBrush(bgc)
    );

}

/// <summary>
/// Draws the slider thumb.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_slider_thumb(
    const NativeDrawArgs & args) noexcept {
    // 颜色定义
    auto get_thumb_color = [](StyleState s, ColorF& out) noexcept {
        constexpr auto normal_color= 0x007ad9ff_rgba;
        constexpr auto hover_color = 0x171717ff_rgba;
        constexpr auto other_color = 0xccccccff_rgba;
        uint32_t code;
        if (s.disabled || s.active) code = other_color;
        else if (s.hover) code = hover_color;
        else code = normal_color;
        ColorF::FromRGBA_RT(out, { code });
    };
    // 计算颜色
    ColorF color1, color2;
    get_thumb_color(args.from, color1);
    get_thumb_color(args.to  , color2);
    const auto color = LongUI::Mix(color1, color2, args.progress);
    // 直接填充矩形
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(args.border),
        &UIManager.RefCCBrush(color)
    );
}




/// <summary>
/// Draws the progress bar.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_progress_bar(
    const NativeDrawArgs & args) noexcept {
    // 预备
    const auto color = ColorF::FromRGBA_CT<0xbcbcbcff_rgba>();
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    auto rect = args.border;
    constexpr float width = 1.f;
    rect.top += width * 0.5f;
    rect.left += width * 0.5f;
    rect.right -= width * 0.5f;
    rect.bottom -= width * 0.5f;
    // 渲染
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.DrawRectangle(auto_cast(rect), &brush, width);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the progress chunk.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_progress_chunk(
    const NativeDrawArgs & args, bool vertical) noexcept {
    // 预备
    const auto color = ColorF::FromRGBA_CT<0x06b025ff_rgba>();
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    // 渲染
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.FillRectangle(auto_cast(args.border), &brush);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}