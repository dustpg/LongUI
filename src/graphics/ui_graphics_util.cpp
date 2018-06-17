// ui
#include <graphics/ui_matrix3x2.h>
#include <graphics/ui_graphics_util.h>
#include <graphics/ui_graphics_impl.h>
// c++
#include <cassert>


PCN_NOINLINE
/// <summary>
/// Fills the rect standard brush.
/// </summary>
/// <param name="renderer">The renderer.</param>
/// <param name="brush">The brush.</param>
/// <param name="rect">The rect.</param>
/// <param name="opactiy">The opactiy.</param>
void LongUI::FillRectStdBrush(
    I::Renderer2D& renderer, 
    I::Brush& brush, 
    const RectF& rect,
    float opactiy) noexcept {
    assert(!"NOT TESTED");
    // 初始检查
#ifndef NDEBUG
    D2D1_MATRIX_3X2_F matrix;
    brush.GetTransform(&matrix);
    auto& m = reinterpret_cast<Matrix::Matrix3x2F&>(matrix);
    assert(m.IsIdentity() && "not a std-brush");
    assert(brush.GetOpacity() == 1.f && "not std-brush");
#endif
    // 正常渲染
    const Size2F zoom { rect.right - rect.left, rect.bottom - rect.top };
    brush.SetTransform(auto_cast(Matrix::Matrix3x2F::Scale(zoom)));
    brush.SetOpacity(opactiy);
    renderer.FillRectangle(auto_cast(rect), &brush);
    // 回退检查
#ifndef NDEBUG
    m = Matrix::Matrix3x2F::Identity();
    brush.SetTransform(auto_cast(m));
    brush.SetOpacity(1.f);
#endif
}
