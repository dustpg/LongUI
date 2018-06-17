// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_matrix3x2.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"


/// <summary>
/// Draws the tree header cell.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_tree_header_cell(
    const NativeDrawArgs & args) noexcept {
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

    // -------------------------- 背景色

    // win10只有active和hover有颜色(大概)
    const auto get_color = [](StyleState state) noexcept {
        if (state.active) return 0xbcdcf4ff_rgba;
        else if (state.hover) return 0xd9ebf9ff_rgba;
        else return 0xffffffff_rgba;
    };
    // 获取颜色
    const auto color1 = get_color(args.from);
    const auto color2 = get_color(args.to);
    // 没有颜色
    if ((color1 & color2) != 0xffffffff_rgba) {
        ColorF bgcolor1, bgcolor2;
        ColorF::FromRGBA_RT(bgcolor1, { color1 });
        ColorF::FromRGBA_RT(bgcolor2, { color2 });
        // 混合
        const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
        const auto deprogress = 1.f - args.progress;
        // 边框中心位置
        auto center = args.border;
        // 背景色彩
        auto& bursh0 = UIManager.RefCCBrush(bgcolor);
        renderer.FillRectangle(auto_cast(args.border), &bursh0);
    }

    // -------------------------- 右侧竖线
    const auto rightline = ColorF::FromRGBA_CT<0xe5e5e5ff_rgba>();
    auto rightrect = args.border;
    rightrect.left = rightrect.right - 1.f;
    auto& burshr = UIManager.RefCCBrush(rightline);
    renderer.FillRectangle(auto_cast(rightrect), &burshr);

    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

}



/// <summary>
/// Draws the tree twisty.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_tree_twisty(
    const NativeDrawArgs& args) noexcept {
    // 不显示
    if (!args.to.indeterminate) return;
    // 获取颜色
    const auto get_color = [](StyleState state, ColorF& c) noexcept {
        uint32_t color;
        if (state.hover) color = 0x1cc4f7ff_rgba;
        else color = state.closed ? 0xa6a6a6ff_rgba : 0x404040ff_rgba;
        ColorF::FromRGBA_RT(c, { color });
    };
    ColorF fcolor1, fcolor2;
    get_color(args.from, fcolor1);
    get_color(args.to, fcolor2);
    const auto fcolor = LongUI::Mix(fcolor1, fcolor2, args.progress);
    // 计算角度
    float angle = 180.f;
    if (args.from.closed != args.to.closed) {
        const auto delta = 90.f * args.progress;
        angle = args.from.closed ? 90.f + delta : 180.f - delta;
    }
    else if (args.from.closed) {
        angle = 90.f;
    }
    // 渲染前景

    this->draw_arrow_only(args.border, fcolor, angle);
}
