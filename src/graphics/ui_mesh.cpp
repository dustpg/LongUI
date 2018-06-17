// ui
#include <core/ui_manager.h>
#include <graphics/ui_mesh.h>
#include <graphics/ui_matrix3x2.h>
#include <core/ui_resource_manager.h>
#include <graphics/ui_graphics_impl.h>
// c++
#include <cassert>

// ui
namespace LongUI {
    // auto cast
    auto&auto_cast(I::Mesh& mesh) noexcept {
        return reinterpret_cast<ID2D1Mesh&>(mesh);
    }
}

/// <summary>
/// Releases the mesh.
/// </summary>
/// <param name="mesh">The mesh.</param>
/// <returns></returns>
void LongUI::SafeReleaseMesh(I::Mesh* mesh) noexcept {
    if (mesh) auto_cast(*mesh).Release();
}

/// <summary>
/// Creates the mesh.
/// </summary>
/// <param name="mesh">The mesh.</param>
/// <param name="triangles">The triangles.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
auto LongUI::CreateMesh(I::Mesh*& mesh,
                    const TriangleF triangles[], 
                    uint32_t len) noexcept -> Result {
    // 创建基本网格
    ID2D1Mesh* ptr = nullptr;
    ID2D1TessellationSink* sink = nullptr;
    Result hr = { UIManager.Ref2DRenderer().CreateMesh(&ptr) };
    longui_debug_hr(hr, "CreateMesh failed");
    // 打开Sink对象
    if (hr) {
        hr = { ptr->Open(&sink) };
        longui_debug_hr(hr, "Open Mesh failed");
    }
    // 填充三角形
    if (hr) {
        const auto ts = reinterpret_cast<const D2D1_TRIANGLE*>(triangles);
        static_assert(sizeof(D2D1_TRIANGLE) == sizeof(TriangleF), "same!");
        sink->AddTriangles(ts, len);
        hr = { sink->Close() };
        longui_debug_hr(hr, "Close Mesh failed");
    }
    // 扫尾处理
    LongUI::SafeRelease(sink);
    mesh = reinterpret_cast<I::Mesh*>(ptr);
    // 返回错误码
    return hr;
}

PCN_NOINLINE
/// <summary>
/// Fills the mesh.
/// </summary>
/// <param name="mesh">The mesh.</param>
/// <param name="renderer">The renderer.</param>
/// <param name="brush">The brush.</param>
/// <param name="transfrom">The transfrom.</param>
/// <returns></returns>
void LongUI::FillMesh(I::Mesh& mesh,
                  I::Renderer2D & renderer, 
                  I::Brush& brush,
                  const Matrix3X2F& transfrom) noexcept {
    // 保存
    Matrix3X2F matrix;
    renderer.GetTransform(&auto_cast(matrix));
    auto& m = static_cast<const Matrix::Matrix3x2F&>(transfrom);
    const auto offset = m * matrix;
    renderer.SetTransform(&auto_cast(offset));
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.FillMesh(&auto_cast(mesh), &brush);
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    renderer.SetTransform(&auto_cast(matrix));
}