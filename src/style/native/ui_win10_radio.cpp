// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"

/// <summary>
/// Draws the radio icon.
/// </summary>
/// <param name="border">The border rect.</param>
/// <param name="state">The state.</param>
/// <param name="opacity">The opacity.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_radio(
    const NativeDrawArgs& args) noexcept {
    // 获取颜色
    ColorF bgcolor1, bdcolor1, bgcolor2, bdcolor2;
    self::get_radio_cbx_color(args.from, bdcolor1, bgcolor1);
    self::get_radio_cbx_color(args.to, bdcolor2, bgcolor2);
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    // 渲染背景
    self::draw_radio_bg(args.border, bdcolor, bgcolor);
    // 前景准备
    float progress = args.progress;
    // 状态已经固定
    if (args.from.checked == args.to.checked) {
        progress = args.from.checked ? 1.0f : 0.0f;
    }
    // 反向修改
    else if (args.from.checked) progress = 1.f - progress;
    // 渲染前景
    self::draw_radio_only(args.border, bdcolor, progress);
}


/// <summary>
/// Draws the radio bg.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_radio_bg(
    const RectF& rect, const ColorF& bd, const ColorF & bg) noexcept {
    // 中心点
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    constexpr float radius_bd = float(CHECKBOX_ICON_SIZE) * 0.5f;
    constexpr float radius_bg = radius_bd - 0.5f;
    // 矩形
    Ellipse draw;
    draw.point = { center_x, center_y };
    // 背景
    draw.radius_x = radius_bg;
    draw.radius_y = radius_bg;
    if (bg.a  > 0.0f)
        UIManager.Ref2DRenderer().FillEllipse(
            auto_cast(draw), &UIManager.RefCCBrush(bg)
        );
    // 边框
    draw.radius_x = radius_bd;
    draw.radius_y = radius_bd;
    UIManager.Ref2DRenderer().DrawEllipse(
        auto_cast(draw), &UIManager.RefCCBrush(bd)
    );
}

/// <summary>
/// Gets the color of the radio CBX.
/// </summary>
/// <param name="state">The state.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::get_radio_cbx_color(
    StyleState state, ColorF & bd, ColorF & bg) noexcept {
    // 颜色
    constexpr auto hover_border     = 0x0078d7ff_rgba;
    constexpr auto active_border    = 0x005499ff_rgba;
    constexpr auto normal_border    = 0x000000ff_rgba;
    constexpr auto disabled_border  = 0xccccccff_rgba;
    constexpr auto active_backgd    = 0xcce4f7ff_rgba;
    // 使用数据
    uint32_t bdc, bgc = 0xcce4f700_rgba;
    // 禁用
    if (state.disabled) { bdc = disabled_border; }
    // 按下
    else if (state.active) { bdc = active_border; bgc = active_backgd; }
    // 悬浮
    else if (state.hover) { bdc = hover_border; }
    // 普通
    else { bdc = normal_border; }
    // 返回颜色
    ColorF::FromRGBA_RT(bd, { bdc });
    ColorF::FromRGBA_RT(bg, { bgc });
}

/// <summary>
/// Draws the radio only.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_radio_only(
    const RectF & rect,
    const ColorF & color,
    float progress) noexcept {
    if (progress == 0.0f) return;
    // 计算偏移
    constexpr float BOXW = float(CHECKBOX_IND_SIZE);
    const float box_half = BOXW * progress * 0.5f;
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    Ellipse draw;
    // 计算矩形
    draw.point = { center_x, center_y };
    draw.radius_x = box_half;
    draw.radius_y = box_half;
    // 渲染
    UIManager.Ref2DRenderer().FillEllipse(
        auto_cast(draw),
        &UIManager.RefCCBrush(color)
    );
}