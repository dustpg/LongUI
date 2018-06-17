// ui
#include <debugger/ui_debug.h>
#include <graphics/ui_geometry.h>
#include <graphics/ui_matrix3x2.h>
#include <core/ui_resource_manager.h>
#include <graphics/ui_graphics_impl.h>
// c++
#include <cassert>

// interface, TODO: GeometryEx
#ifdef interface
#undef interface
#endif
#pragma interface
/// <summary>
/// geometry interface
/// </summary>
struct PCN_NOVTABLE LongUI::I::Geometry : ID2D1Geometry { };

/// <summary>
/// geometry-ex interface
/// </summary>
struct PCN_NOVTABLE LongUI::I::GeometryEx : ID2D1Geometry { };
#pragma implementation 

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIGeometry::Recreate() noexcept -> Result {
    this->release();
    assert(!"NOT IMPL");
    return Result();
}

/// <summary>
/// Steals from.
/// </summary>
/// <param name="p">The native interface pointer.</param>
/// <returns></returns>
auto LongUI::CUIGeometry::StealFrom(I::Geometry* p) noexcept -> Result {
    assert(p && "bad args");
    this->release();
    m_pDawPtr = p;
    p->AddRef();
    m_pRealization = reinterpret_cast<I::GeometryEx*>(p);
    return{ Result::RS_OK };
}

/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIGeometry::release() noexcept {
    LongUI::SafeRelease(m_pDawPtr);
    LongUI::SafeRelease(m_pRealization);
}


PCN_NOINLINE
/// <summary>
/// Renders the specified .
/// </summary>
/// <param name="renderer">The renderer.</param>
/// <param name="brush">The brush.</param>
/// <param name="transform">The transform.</param>
/// <param name="width">The width.</param>
/// <returns></returns>
void LongUI::CUIGeometry::Draw(I::Renderer2D& renderer,
                            I::Brush& brush,
                            const Matrix3X2F& transform,
                            float width) noexcept {
    // 数据有效
    if (m_pRealization) {
        Matrix3X2F matrix;
        renderer.GetTransform(&auto_cast(matrix));
        const auto offset =
            reinterpret_cast<const Matrix::Matrix3x2F&>(transform)
            * matrix;
        renderer.SetTransform(&auto_cast(offset));
        renderer.DrawGeometry(m_pRealization, &brush, width, nullptr);
        renderer.SetTransform(&auto_cast(matrix));
    }
}


PCN_NOINLINE
/// <summary>
/// Fills the specified renderer.
/// </summary>
/// <param name="renderer">The renderer.</param>
/// <param name="brush">The brush.</param>
/// <param name="transform">The transform.</param>
/// <returns></returns>
void LongUI::CUIGeometry::Fill(I::Renderer2D& renderer,
                            I::Brush& brush,
                            const Matrix3X2F& transform) noexcept {
    // 数据有效
    if (m_pRealization) {
        Matrix3X2F matrix;
        renderer.GetTransform(&auto_cast(matrix));
        const auto offset = 
            reinterpret_cast<const Matrix::Matrix3x2F&>(transform) 
            * matrix;
        renderer.SetTransform(&auto_cast(offset));
        renderer.FillGeometry(m_pRealization, &brush);
        renderer.SetTransform(&auto_cast(matrix));
    }
}

PCN_NOINLINE
/// <summary>
/// Creates from points.
/// </summary>
/// <param name="obj">The object.</param>
/// <param name="">The .</param>
/// <param name="count">The count.</param>
/// <returns></returns>
auto LongUI::CUIGeometry::CreateFromPoints(CUIGeometry& obj, 
                                       const Point2F points[], 
                                       uint32_t count) noexcept -> Result {
    // 至少两个
    assert(count >= 2);
    auto& raw = CUIResMgr::Ref2DFactory();
    auto& factory = reinterpret_cast<ID2D1Factory&>(raw);
    // 创建几何体
    auto create_geo = [&](const Point2F* points,
        uint32_t length, I::Geometry*& out) noexcept {
        assert(!out && "bad out");
        const auto list = reinterpret_cast<const D2D1_POINT_2F*>(points);
        auto hr = S_OK;
        ID2D1PathGeometry* geometry = nullptr;
        ID2D1GeometrySink* sink = nullptr;
        // 创建几何体
        if (SUCCEEDED(hr)) {
            hr = factory.CreatePathGeometry(&geometry);
            longui_debug_hr(Result{hr}, "CreatePathGeometry failed");
        }
        // 打开
        if (SUCCEEDED(hr)) {
            hr = geometry->Open(&sink);
            longui_debug_hr(Result{ hr }, "Open Geometry failed");
        }
        // 开始绘制
        if (SUCCEEDED(hr)) {
            sink->BeginFigure(list[0], D2D1_FIGURE_BEGIN_HOLLOW);
            sink->AddLines(list + 1, length - 1);
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
            hr = sink->Close();
            longui_debug_hr(Result{ hr }, "Close Geometry failed");
        }
        LongUI::SafeRelease(sink);
        out = static_cast<I::Geometry*>(
            static_cast<ID2D1Geometry*>(geometry));
        return hr;
    };
    // 正式创建
    I::Geometry* geo = nullptr;
    Result hr = { create_geo(points, count, geo) };
    // 利用原始指针创建
    if (hr) {
        hr = obj.StealFrom(geo);
        // 创建失败
        if (!hr) geo->Release();
    }
    return hr;
}
