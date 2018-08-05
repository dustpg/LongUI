// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"

/// <summary>
/// Draws the checkbox icon.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_checkbox(
    const NativeDrawArgs& args) noexcept {
    // 获取颜色
    ColorF bgcolor1, bdcolor1, bgcolor2, bdcolor2;
    self::get_radio_cbx_color(args.from, bdcolor1, bgcolor1);
    self::get_radio_cbx_color(args.to, bdcolor2, bgcolor2);
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    // 渲染背景
    self::draw_cbx_bg(args.border, bdcolor, bgcolor);
    // 前景准备
    bool draw_check = false;
    float progress = args.progress;
    // 第三态
    if (args.from.indeterminate || args.to.indeterminate) {
        // 状态已经固定
        if (args.from.indeterminate == args.to.indeterminate) progress = 1.f;
        // 反向修改
        else if (args.from.indeterminate) progress = 1.f - progress;
        
    }
    // 选中态
    else {
        draw_check = true;
        // 状态已经固定
        if (args.from.checked == args.to.checked) {
            progress = args.from.checked ? 1.f : 0.f;
        }
        // 反向修改
        else if (args.from.checked) progress = 1.f - progress;
    }
    // 渲染前景
    if (draw_check) self::draw_check_only(args.border, bdcolor, progress);
    else self::draw_cbx_ind(args.border, bdcolor, progress);
}


/// <summary>
/// Draws the checkbox indeterminate.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_cbx_ind(
    const RectF & rect,
    const ColorF & color,
    float progress) noexcept {
    if (progress == 0.0f) return;
    // 计算偏移
    constexpr float BOXW = float(CHECKBOX_IND_SIZE);
    const float box_half = BOXW * progress * 0.5f - 0.5f;
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f - 0.5f;
    RectF draw;
    // 计算矩形
    draw.top = center_y - box_half;
    draw.left = center_x - box_half;
    draw.right = center_x + box_half;
    draw.bottom = center_y + box_half;
    // 渲染
    auto& renderer = UIManager.Ref2DRenderer();
    renderer.FillRectangle(auto_cast(draw), &UIManager.RefCCBrush(color));
}

/// <summary>
/// Draws the check only.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_check_only(
    const RectF& rect, 
    const ColorF& color, 
    float progress) noexcept {
    if (progress == 0.0f) return;
    // 常量定义
    constexpr float PEN = 1.5f;
    constexpr float HAL = float(CHECKBOX_ICON_SIZE - 2) * 0.5f;
    constexpr float P1X = -(HAL - CHECKBOX_CHECK_X1) - 0.5f;
    constexpr float P1Y = -0.5f;
    constexpr float P2X = P1X + CHECKBOX_CHECK_X2;
    constexpr float P2Y = P1Y + CHECKBOX_CHECK_X2;
    constexpr float P3X = P2X + CHECKBOX_CHECK_X3;
    constexpr float P3Y = P2Y - CHECKBOX_CHECK_X3;
    // 资源获取
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    // 数据计算
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    const Point2F p1{ center_x + P1X , center_y + P1Y };
    const Point2F p2{ center_x + P2X , center_y + P2Y };
    const Point2F p3{ center_x + P3X , center_y + P3Y };
    // 第一阶段
    renderer.DrawLine(
        auto_cast(p1), 
        auto_cast(LongUI::Mix(p1, p2, progress * 2.f)), 
        &brush,
        PEN
    );
    // 第二阶段
    if (progress > 0.5f) {
        renderer.DrawLine(
            auto_cast(p2), 
            auto_cast(LongUI::Mix(p2, p3, progress * 2.f - 1.f)),
            &brush,
            PEN
        );
    }
}


/// <summary>
/// Draws the radio CBX.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_cbx_bg(
    const RectF& rect,
    const ColorF& bd,
    const ColorF& bg) noexcept {
    // 中心点
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    constexpr float radius_bg = float(CHECKBOX_ICON_SIZE) * 0.5f;
    constexpr float radius_bd = radius_bg - 0.5f;
    // 矩形
    RectF draw; auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 背景
    draw.top = center_y - radius_bg;
    draw.left = center_x - radius_bg;
    draw.right = center_x + radius_bg;
    draw.bottom = center_y + radius_bg;
    if (bg.a  > 0.0f)
        renderer.FillRectangle(
            auto_cast(draw), &UIManager.RefCCBrush(bg)
        );
    // 边框
    draw.top = center_y - radius_bd;
    draw.left = center_x - radius_bd;
    draw.right = center_x + radius_bd;
    draw.bottom = center_y + radius_bd;
    renderer.DrawRectangle(
        auto_cast(draw), &UIManager.RefCCBrush(bd)
    );
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}
