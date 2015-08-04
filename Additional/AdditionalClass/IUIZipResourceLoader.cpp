#include "IUIZipResourceLoader.h"

// 创建
extern "C" HRESULT CreateZipResourceLoader(
    LongUI::CUIManager& manager, const wchar_t* file_name,
    LongUI::IUIResourceLoader** outdata
    ) noexcept {
    // 参数检查
    assert(file_name && outdata);
    if (!(outdata && file_name && file_name[0])) {
        return E_INVALIDARG;
    }
    HRESULT hr = S_OK; LongUI::IUIResourceLoader* loader = nullptr;
    // 构造对象
    if (SUCCEEDED(hr)) {
        loader = new(std::nothrow) LongUI::CUIZipResourceLoader(manager);
        if (!loader) hr = E_OUTOFMEMORY;
    }
    // 成功? 初始化
    if (SUCCEEDED(hr)) {
    }
    // OK!
    if (SUCCEEDED(hr)) {
        *outdata = loader;
        loader = nullptr;
    }
    ::SafeRelease(loader);
    return hr;
}


// CUIZipResourceLoader 构造函数
LongUI::CUIZipResourceLoader::CUIZipResourceLoader(
    CUIManager& manager) noexcept : m_manager(manager) {
    ::memset(&m_zipFile, 0, sizeof(m_zipFile));
}

// CUIZipResourceLoader 析构函数
LongUI::CUIZipResourceLoader::~CUIZipResourceLoader() noexcept {
}

// CUIZipResourceLoader 初始化
auto LongUI::CUIZipResourceLoader::Init(const wchar_t* file_name) noexcept -> HRESULT {
    return E_NOTIMPL;
}