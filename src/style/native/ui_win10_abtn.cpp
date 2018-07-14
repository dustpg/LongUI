// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_matrix3x2.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"


/// <summary>
/// Draws the arrow button.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="dir">The dir.</param>
/// <param name="back">if set to <c>true</c> [back].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_arrow_button(
    const NativeDrawArgs& args,
    ArrowDirection dir) noexcept {
    // 获取颜色
    ColorF bcolor1, fcolor1, bcolor2, fcolor2;
    self::get_arrow_btn_color(args.from, bcolor1, fcolor1);
    self::get_arrow_btn_color(args.to, bcolor2, fcolor2);
    const auto bcolor = LongUI::Mix(bcolor1, bcolor2, args.progress);
    const auto fcolor = LongUI::Mix(fcolor1, fcolor2, args.progress);
    // 背景
    auto rect = args.border;
    // 左右方向缩上下
    if (uint32_t(dir) & 1) { rect.top += 1.f; rect.bottom -= 1.f; }
    // 上下方向缩左右
    else { rect.left += 1.f; rect.right -= 1.f; }
    // 渲染
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect), &UIManager.RefCCBrush(bcolor)
    );
    // 前景
    assert(dir < DIRECTION_COUNT && "out of range");
    this->draw_arrow_only(args.border, fcolor, float(dir) * 90.f);
}


PCN_NOINLINE
/// <summary>
/// Draws the arrow only.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="angle">The angle.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_arrow_only(const RectF& rect,
    const ColorF& color,
    float angle) noexcept {
#ifdef LUI_DRAW_ARROW_IN_MESH
    if (!m_pMesh) return;
    // 计算旋转角度
    //const float angle = float(dir) * 90.f;
    Point2F center;
    center.x = (rect.left + rect.right) * 0.5f;
    center.y = (rect.top + rect.bottom) * 0.5f;
    const auto transfrom =
        Matrix::Matrix3x2F::Translation(center.x, center.y)
        //* Matrix::Matrix3x2F::Scale({1.25,1.25}, center)
        * Matrix::Matrix3x2F::Rotation(angle, center);
    // 渲染箭头
    LongUI::FillMesh(
        *m_pMesh,
        UIManager.Ref2DRenderer(),
        UIManager.RefCCBrush(ColorF::FromRGBA_CT<RGBA_Black>()),
        transfrom
    );
#else
    // 计算旋转角度
    //const float angle = float(dir) * 90.f;
    Point2F center;
    center.x = (rect.left + rect.right) * 0.5f;
    center.y = (rect.top + rect.bottom) * 0.5f;
    const auto transfrom =
        Matrix::Matrix3x2F::Translation(center.x, center.y)
        //* Matrix::Matrix3x2F::Scale({1.25,1.25}, center)
        * Matrix::Matrix3x2F::Rotation(angle, center);
    auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    m_geoArrow.Draw(renderer, UIManager.RefCCBrush(color), transfrom, 2.f);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#endif
}


/// <summary>
/// Draws the drop mark.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_drop_mark(
    const NativeDrawArgs & args) noexcept {
    ColorF color;
    //ColorF::FromRGBA_RT(color, { 
    //    args.to.disabled ? 0x606060ff_rgba : 0x3d3d3dff_rgba
    //});
    ColorF::FromRGBA_RT(color, { 0x3d3d3dff_rgba });
    constexpr float angle0 = Direction_Down * 90.f;
    constexpr float angle1 = Direction_Up * 90.f;
    //float angle = args.to.active ? angle1 : angle0;
    // 切换ACTIVE
    //if (args.to.active != args.from.active) {

    //}
    return this->draw_arrow_only(args.border, color, angle0);
}

/// <summary>
/// Draws the menu arrow.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_menu_arrow(
    const NativeDrawArgs & args) noexcept {
    const auto color = ColorF::FromRGBA_CT<0x3d3d3dff_rgba>();
    constexpr float angle = Direction_Right * 90.f;
    return this->draw_arrow_only(args.border, color, angle);
}

/// <summary>
/// Gets the color of the arrow BTN.
/// </summary>
/// <param name="state">The state.</param>
/// <param name="bc">The back-color.</param>
/// <param name="fc">The fore-color.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::get_arrow_btn_color(
    StyleState state, ColorF & bc, ColorF & fc) noexcept {
    // 颜色初始化
    constexpr auto hover_fc = 0x000000ff_rgba;
    constexpr auto hover_bc = 0xdadadaff_rgba;

    constexpr auto active_fc = 0xffffffff_rgba;
    constexpr auto active_bc = 0x606060ff_rgba;

    constexpr auto normal_fc = 0x606060ff_rgba;
    constexpr auto normal_bc = 0xf0f0f0ff_rgba;

    constexpr auto disabled_fc = 0x606060ff_rgba;
    constexpr auto disabled_bc = 0xf0f0f0ff_rgba;
    // 使用数据
    uint32_t fore_color, back_color;
    // 禁用
    if (state.disabled) { fore_color = disabled_fc; back_color = disabled_bc; }
    // 按下
    else if (state.active) { fore_color = active_fc; back_color = active_bc; }
    // 悬浮
    else if (state.hover) { fore_color = hover_fc; back_color = hover_bc; }
    // 普通
    else { fore_color = normal_fc; back_color = normal_bc; }
    // 颜色
    ColorF::FromRGBA_RT(fc, { fore_color });
    ColorF::FromRGBA_RT(bc, { back_color });
}



/// <summary>
/// Draws the thumb.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_sb_thumb(
    const NativeDrawArgs& args, bool vertical) noexcept {
    // 背景
    auto rect = args.border;
    // 左右方向缩上下
    if (vertical) { rect.left += 1.f; rect.right -= 1.f; }
    // 上下方向缩左右
    else { rect.top += 1.f; rect.bottom -= 1.f; }
    // 获取颜色
    auto get_color = [](StyleState state, ColorF& color) noexcept {
        uint32_t code;
        if (state.active || state.hover) code = 0xa6a6a6ff_rgba;
        else code = 0xcdcdcdff_rgba;
        ColorF::FromRGBA_RT(color, { code });
    };
    ColorF from, to; get_color(args.from, from); get_color(args.to, to);
    const auto color = LongUI::Mix(from, to, args.progress);
    // 渲染
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect),
        &UIManager.RefCCBrush(color)
    );
}


/// <summary>
/// Draws the sb track.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_sb_track(
    const NativeDrawArgs & args, bool vertical) noexcept {
    // 背景颜色
    ColorF color = ColorF::FromRGBA_CT<0xF0F0F0FF_rgba>();
    // 渲染
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(args.border),
        &UIManager.RefCCBrush(color)
    );
}


/// <summary>
/// Draws the rasizer.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_rasizer(const NativeDrawArgs& args) noexcept {
    // 背景
    auto rect = args.border;
    // 测试用渲染
    auto& renderer = UIManager.Ref2DRenderer();
    ColorF color = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
    renderer.FillRectangle(auto_cast(rect), &UIManager.RefCCBrush(color));
}