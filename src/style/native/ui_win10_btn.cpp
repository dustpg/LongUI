// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"

/// <summary>
/// Draws the button.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_button(
    const NativeDrawArgs& args) noexcept {
    // 获取颜色
    ColorF bgcolor1, bdcolor1, bgcolor2, bdcolor2;
    const auto w1 = self::get_button_color(args.from, bdcolor1, bgcolor1);
    const auto w2 = self::get_button_color(args.to  , bdcolor2, bgcolor2);
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    const auto deprogress = 1.f - args.progress;
    const auto border_halfw = w2 * args.progress + w1 * deprogress;
    // 边框中心位置
    auto center = args.border;
    center.top += border_halfw;
    center.left += border_halfw;
    center.right -= border_halfw;
    center.bottom -= border_halfw;
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 背景色彩
    auto& bursh0 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(args.border), &bursh0);
    // 边框色彩
    auto& bursh1 = UIManager.RefCCBrush(bdcolor);
    renderer.DrawRectangle(auto_cast(center), &bursh1, border_halfw * 2.f);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Gets the color of the button.
/// </summary>
/// <param name="state">The state.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
auto LongUI::CUINativeStyleWindows10::get_button_color(
    StyleState state, ColorF & bd, ColorF & bg) noexcept -> float {
    // 颜色定义
    constexpr auto hover_border     = 0x0078d7ff_rgba;
    constexpr auto hover_backgd     = 0xe5f1fbff_rgba;
    constexpr auto active_border    = 0x005499ff_rgba;
    constexpr auto active_backgd    = 0xcce4f7ff_rgba;
    constexpr auto normal_border    = 0xadadadff_rgba;
    constexpr auto normal_backgd    = 0xe1e1e1ff_rgba;
    constexpr auto default_border   = 0x0078d7ff_rgba;
    constexpr auto disabled_border  = 0xbfbfbfff_rgba;
    constexpr auto disabled_backgd  = 0xccccccff_rgba;
    // 使用数据
    uint32_t bdc, bgc;
    float border_halfw = 0.5f;
    // 禁用
    if (state.disabled) { bdc = disabled_border; bgc = disabled_backgd; }
    // 按下
    else if (state.active) { bdc = active_border; bgc = active_backgd; }
    // 悬浮
    else if (state.hover) { bdc = hover_border; bgc = hover_backgd; }
    // 普通
    else {
        bgc = normal_backgd;
        // 作为默认按钮再缩1逻辑像素
        if (state.default5) {
            border_halfw = 1.f;
            bdc = default_border;
        }
        else bdc = normal_border;
    }
    // 返回数据
    ColorF::FromRGBA_RT(bd, { bdc });
    ColorF::FromRGBA_RT(bg, { bgc });
    return border_halfw;
}

/// <summary>
/// Draws the textfield.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_textfield(
    const NativeDrawArgs & args) noexcept {
    // 获取颜色
    ColorF bdcolor1, bdcolor2;
    const auto get_tf_color = [](StyleState state, ColorF& fc) noexcept {
        constexpr auto disabled = 0xbfbfbfff_rgba;
        constexpr auto normal = 0x7a7a7aff_rgba;
        constexpr auto hover = 0x171717ff_rgba;
        constexpr auto focus = 0x005ca4ff_rgba;
        uint32_t color;
        if (state.disabled) color = disabled;
        else if (state.focus) color = focus;
        else if (state.hover) color = hover;
        else color = normal;
        ColorF::FromRGBA_RT(fc, { color });
    };
    get_tf_color(args.from, bdcolor1);
    get_tf_color(args.to, bdcolor2);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    const auto border_halfw = 0.5f;
    // 边框中心位置
    auto center = args.border;
    center.top += border_halfw;
    center.left += border_halfw;
    center.right -= border_halfw;
    center.bottom -= border_halfw;
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 边框色彩
    auto& bursh1 = UIManager.RefCCBrush(bdcolor);
    renderer.DrawRectangle(auto_cast(center), &bursh1, border_halfw * 2.f);
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}