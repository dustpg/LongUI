// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_matrix3x2.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"


PCN_NOINLINE
/// <summary>
/// Draws the tab.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_tab(
    const NativeDrawArgs& args) noexcept {
    // 调整矩形位置(未选中的下压缩2单位)
    auto rect = args.border;
    // 这个位置以target为标准
    if (!args.to.selected) rect.top += 2.f;
    // 获取颜色标准
    const auto get_tab_color = [](StyleState state, ColorF& fc) noexcept {
        uint32_t color;
        // 选择
        if (state.selected) color = 0xffffffff_rgba;
        // 未选中
        else color = state.hover ? 0xd8eaf9ff_rgba : 0xf0f0f0ff_rgba;
        // 颜色
        ColorF::FromRGBA_RT(fc, { color });
    };
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 边框
    const auto bdcolor = ColorF::FromRGBA_CT<0xd9d9d9ff_rgba>();
    auto& bursh0 = UIManager.RefCCBrush(bdcolor);
    renderer.FillRectangle(auto_cast(rect), &bursh0);
    // 获取颜色
    ColorF bgcolor1, bgcolor2;
    get_tab_color(args.from, bgcolor1);
    get_tab_color(args.to, bgcolor2);
    // 混合颜色
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    // 调整位置
    const auto draw_bottom = !args.to.selected;
    const auto draw_left = args.to.selected || !args.to.after_seltab;
    const auto draw_right = args.to.selected || args.to.after_seltab;
    rect.top += 1.f;
    if (draw_left) rect.left += 1.f;
    if (draw_right) rect.right -= 1.f;
    if (draw_bottom) rect.bottom -= 1.f;
    // 背景色彩
    auto& bursh1 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(rect), &bursh1);
    // 底层直线
    if (args.from.selected != args.to.selected) {
        const auto fv = 1.f - args.progress;
        auto& bursh2 = UIManager.RefCCBrush(
            args.from.selected ? ColorF{ 1,1,1,1 } : bdcolor
        );
        const D2D1_POINT_2F p1{ args.border.left, args.border.bottom };
        const auto width = args.border.right - args.border.left;
        renderer.DrawLine(p1, { p1.x + width * fv, p1.y }, &bursh2);
    }
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the tab panels.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
//void LongUI::CUINativeStyleWindows10::draw_tab_panels(
//    const NativeDrawArgs & args) noexcept {
//    draw_group_box(args);
//}