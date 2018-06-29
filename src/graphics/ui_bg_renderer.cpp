// ui
#include <style/ui_style.h>
#include <core/ui_manager.h>
#include <core/ui_basic_type.h>
#include <core/ui_color_list.h>
#include <graphics/ui_bg_renderer.h>
#include <graphics/ui_graphics_impl.h>
// c++
#include <cassert>

/// <summary>
/// Initializes a new instance of the <see cref="Background"/> class.
/// </summary>
LongUI::CUIRendererBackground::CUIRendererBackground() noexcept {
    this->color = ColorF::FromRGBA_CT<RGBA_Transparent>();
}

/// <summary>
/// Finalizes an instance of the <see cref="Background"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIRendererBackground::~CUIRendererBackground() noexcept {

}

/// <summary>
/// Sets the image.
/// </summary>
/// <returns></returns>
void LongUI::CUIRendererBackground::RefreshImage() noexcept {
    assert(!"NOT IMPL");
}

/// <summary>
/// Renders the color.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void LongUI::CUIRendererBackground::RenderColor(const LongUI::Box& box) const noexcept {
    // 渲染背景颜色
    if (this->color.a == 0.f) return;
    //if (!reinterpret_cast<const uint32_t&>(this->color.a)) return;
    // 获取渲染矩阵
    RectF rect;
    switch (this->clip)
    {
    default: [[fallthrough]];
    case LongUI::Box_BorderBox:  box.GetBorderEdge(rect);  break;
    case LongUI::Box_PaddingBox: box.GetPaddingEdge(rect); break;
    case LongUI::Box_ContentBox: box.GetContentEdge(rect); break;
    }
    // 获取渲染器
    auto& render = UIManager.Ref2DRenderer();
    auto& brush0 = UIManager.RefCCBrush(this->color);
    // 执行渲染
    render.FillRectangle(auto_cast(rect), &brush0);
}


/// <summary>
/// Renders the image.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void LongUI::CUIRendererBackground::RenderImage(const LongUI::Box& box) const noexcept {
    // 渲染背景图片
    if (!this->image_id) return;
    box;
    assert(!"NOT IMPL");
    // 背景可能是:
    //    1. 图片
    //    2. 
}