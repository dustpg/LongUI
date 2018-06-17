// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_matrix3x2.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"

/// <summary>
/// Draws the group box.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_group_box(const NativeDrawArgs & args) noexcept {
    // 预备
    const auto color = ColorF::FromRGBA_CT<0xdcdcdcff_rgba>();
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    auto rect = args.border;
    constexpr float width = 1.f;
    rect.top += width * 0.5f;
    rect.left += width * 0.5f;
    rect.right -= width * 0.5f;
    rect.bottom -= width * 0.5f;
    // 渲染
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.DrawRectangle(auto_cast(rect), &brush, width);
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}



/// <summary>
/// Draws the list item.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_list_item(
    const NativeDrawArgs & args) noexcept {
    // win10只有selected和hover有颜色(大概)
    const auto get_color = [](StyleState state) noexcept {
        if (state.selected) return 0xcde8ffff_rgba;
        else if (state.hover) return 0xe5f3ffff_rgba;
        else return 0xffffffff_rgba;
    };
    // 获取颜色
    const auto color1 = get_color(args.from);
    const auto color2 = get_color(args.to);
    // 没有颜色
    if ((color1 & color2) == 0xffffffff_rgba) return;
    ColorF bgcolor1, bgcolor2;
    ColorF::FromRGBA_RT(bgcolor1, { color1 });
    ColorF::FromRGBA_RT(bgcolor2, { color2 });
    // 混合
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    //const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    const auto deprogress = 1.f - args.progress;
    //const auto border_halfw = w2 * args.progress + w1 * deprogress;
    // 边框中心位置
    auto center = args.border;
    //center.top += border_halfw;
    //center.left += border_halfw;
    //center.right -= border_halfw;
    //center.bottom -= border_halfw;
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    // 背景色彩
    auto& bursh0 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(args.border), &bursh0);
    // 边框色彩
    //auto& bursh1 = UIManager.RefCCBrush(bdcolor);
    //renderer.DrawRectangle(auto_cast(center), &bursh1, border_halfw * 2.f);
}
