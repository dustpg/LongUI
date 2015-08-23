#include "LongUI.h"

// 解析path
auto LongUI::SVG::ParserPath(const char* path, ID2D1PathGeometry1** out) noexcept ->HRESULT {
    assert(path && out && "bad arguemnts");
    // 无效参数
    if (!(path && out)) return E_INVALIDARG;
    HRESULT hr = S_OK;
    ID2D1PathGeometry1* path_geometry = nullptr;
    // 创建路径几何
    if (SUCCEEDED(hr)) {
        hr = UIManager_D2DFactory->CreatePathGeometry(&path_geometry);
    }
    // 打开sink
    if (SUCCEEDED(hr)) {
        hr = ParserPath(path, path_geometry);
    }
    ::SafeRelease(path_geometry);
    return hr;
}



// 解析path
auto LongUI::SVG::ParserPath(const char* path, ID2D1PathGeometry* geometry) noexcept ->HRESULT {
    assert(path && geometry && "bad arguemnts");
    // 无效参数
    if (!(path && geometry)) return E_INVALIDARG;
    HRESULT hr = S_OK;
    ID2D1GeometrySink* sink = nullptr;
    // 打开sink
    if (SUCCEEDED(hr)) {
        hr = geometry->Open(&sink);
    }
    // 正式解析
    if (SUCCEEDED(hr)) {
        register char ch = 0;
        while ((ch = *path)) {
            ++path;
        }
    }
    // 关闭sink ??
    if (SUCCEEDED(hr)) {
        hr = sink->Close();
    }
    ::SafeRelease(sink);
    return hr;
}