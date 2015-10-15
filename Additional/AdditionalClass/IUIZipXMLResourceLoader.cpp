#include "IUIZipXMLResourceLoader.h"

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
    HRESULT hr = S_OK; LongUI::CUIZipXmlResourceLoader* loader = nullptr;
    // 构造对象
    if (SUCCEEDED(hr)) {
        loader = new(std::nothrow) LongUI::CUIZipXmlResourceLoader(manager);
        if (!loader) hr = E_OUTOFMEMORY;
    }
    // 成功? 初始化
    if (SUCCEEDED(hr)) {
        hr = loader->Init(file_name);
    }
    // OK!
    if (SUCCEEDED(hr)) {
        *outdata = loader;
        loader = nullptr;
    }
    ::SafeRelease(loader);
    return hr;
}


// CUIZipXmlResourceLoader 构造函数
LongUI::CUIZipXmlResourceLoader::CUIZipXmlResourceLoader(
    CUIManager& manager) noexcept : m_manager(manager) {
    ::memset(&m_zipFile, 0, sizeof(m_zipFile));
}

// CUIZipXmlResourceLoader 析构函数
LongUI::CUIZipXmlResourceLoader::~CUIZipXmlResourceLoader() noexcept {
    ::mz_zip_reader_end(&m_zipFile);
    ::SafeRelease(m_pWICFactory);
}


// find node with index
auto LongUI::CUIZipXmlResourceLoader::find_node_with_index(
    pugi::xml_node node, const size_t index) noexcept -> pugi::xml_node {
    pugi::xml_node found_node;
    size_t i = 0;
    for (auto itr = node.first_child(); itr; itr = itr.next_sibling()) {
        if (i == index) {
            found_node = itr;
            break;
        }
        ++i;
    }
    return found_node;
}

// CUIZipXmlResourceLoader 初始化
auto LongUI::CUIZipXmlResourceLoader::Init(const wchar_t* file_name) noexcept -> HRESULT {
    // 载入ZIP文件
    auto status = ::mz_zip_reader_init_filew(&m_zipFile, file_name, 0);
    if (status) {
        assert(!"mz_zip_reader_init_filew: failed");
        return E_FAIL;
    }
    // 打开资源XML文件
    auto index = ::mz_zip_reader_locate_file(&m_zipFile, "__resources__.xml", nullptr, 0);
    if (index < 0) {
        assert(!"mz_zip_reader_locate_file: failed\n xml file not found, file name must be '__resources__.xml'");
        return E_FAIL;
    }
    // 检查信息
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&m_zipFile, index, &file_stat)) {
        assert(!"mz_zip_reader_file_stat: failed");
        return E_FAIL;
    }
    auto hr = S_OK;
    // 创建 WIC 工厂.
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pWICFactory)
            );
    }
    // 构建资源信息
    return hr;
}
// get reource count
auto LongUI::CUIZipXmlResourceLoader::GetResourceCount(ResourceType type) const noexcept -> size_t {
    assert(type < this->RESOURCE_TYPE_COUNT);
    return static_cast<size_t>(m_aResourceCount[type]);
}

// get reource
auto LongUI::CUIZipXmlResourceLoader::GetResourcePointer(ResourceType type, size_t index) noexcept -> void * {
    void* data = nullptr;
    auto node = this->find_node_with_index(m_aNodes[type], index);
    switch (type)
    {
    case LongUI::IUIResourceLoader::Type_Bitmap:
        data = this->get_bitmap(node);
        break;
    case LongUI::IUIResourceLoader::Type_Brush:
        data = this->get_brush(node);
        break;
    case LongUI::IUIResourceLoader::Type_TextFormat:
        data = this->get_bitmap(node);
        break;
    case LongUI::IUIResourceLoader::Type_Meta:
        __fallthrough;
    case LongUI::IUIResourceLoader::Type_Null:
        __fallthrough;
    default:
        assert(!"unknown resource type");
        break;
    }
    return data;
}

// get meta
auto LongUI::CUIZipXmlResourceLoader::GetMeta(size_t index, DeviceIndependentMeta& meta_raw) noexcept -> void {
    assert(!"noimpl");
}

// get reource count from doc
void LongUI::CUIZipXmlResourceLoader::get_resource_count_from_xml() noexcept {
    // 初始化
    for (auto& node : m_aNodes) { node = pugi::xml_node(); }
    // pugixml 使用的是句柄式, 所以下面的代码是安全的.
    register auto now_node = m_docResource.first_child().first_child();
    while (now_node) {
        // 获取子结点数量
        auto get_children_count = [](pugi::xml_node node) {
            node = node.first_child();
            auto count = 0ui32;
            while (node) { node = node.next_sibling(); ++count; }
            return count;
        };
        // 位图?
        if (!::strcmp(now_node.name(), "Bitmap")) {
            // XXX: PUGIXML 直接读取(XPATH?)
            m_aNodes[Type_Bitmap] = now_node;
            m_aResourceCount[this->Type_Bitmap] = get_children_count(now_node);
        }
        // 笔刷?
        else if (!::strcmp(now_node.name(), "Brush")) {
            // XXX: PUGIXML 直接读取
            m_aNodes[Type_Brush] = now_node;
            m_aResourceCount[this->Type_Brush] = get_children_count(now_node);
        }
        // 文本格式?
        else if (!::strcmp(now_node.name(), "Font") ||
            !::strcmp(now_node.name(), "TextFormat")) {
            // XXX: PUGIXML 直接读取
            m_aNodes[Type_TextFormat] = now_node;
            m_aResourceCount[this->Type_TextFormat] = get_children_count(now_node);
        }
        // 图元?
        else if (!::strcmp(now_node.name(), "Meta")) {
            // XXX: PUGIXML 直接读取
            m_aNodes[Type_Meta] = now_node;
            m_aResourceCount[this->Type_Meta] = get_children_count(now_node);
        }
        // 动画图元?
        else if (!::strcmp(now_node.name(), "MetaEx")) {
            assert(!"unsupport yet");
        }
        // 推进
        now_node = now_node.next_sibling();
    }
}

// 获取位图
auto LongUI::CUIZipXmlResourceLoader::get_bitmap(pugi::xml_node node) noexcept -> ID2D1Bitmap1* {
    assert(node && "node not found");
    // 获取路径
    const char* uri = node.attribute("res").value();
    assert(uri && *uri && "Error URI of Bitmap");
    // 从文件载入位图
    auto load_bitmap_from_stream = [](
        LongUIRenderTarget *pRenderTarget,
        IWICImagingFactory *pIWICFactory,
        IStream* pStream,
        UINT destinationWidth,
        UINT destinationHeight,
        ID2D1Bitmap1 **ppBitmap
        ) noexcept -> HRESULT {
        IWICBitmapDecoder *pDecoder = nullptr;
        IWICBitmapFrameDecode *pSource = nullptr;
        IWICStream *pStream = nullptr;
        IWICFormatConverter *pConverter = nullptr;
        IWICBitmapScaler *pScaler = nullptr;
        // 创建解码器
        register HRESULT hr = pIWICFactory->CreateDecoderFromStream(
            pStream,
            nullptr,
            WICDecodeMetadataCacheOnLoad,
            &pDecoder
            );
        // 获取第一帧
        if (SUCCEEDED(hr)) {
            hr = pDecoder->GetFrame(0, &pSource);
        }
        // 创建格式转换器
        if (SUCCEEDED(hr)) {
            hr = pIWICFactory->CreateFormatConverter(&pConverter);
        }
        // 尝试缩放
        if (SUCCEEDED(hr)) {
            if (destinationWidth != 0 || destinationHeight != 0) {
                UINT originalWidth, originalHeight;
                // 获取大小
                hr = pSource->GetSize(&originalWidth, &originalHeight);
                if (SUCCEEDED(hr)) {
                    // 设置基本分辨率
                    if (destinationWidth == 0) {
                        FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                        destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                    }
                    else if (destinationHeight == 0) {
                        FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                        destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                    }
                    // 创建缩放器
                    hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                    // 初始化
                    if (SUCCEEDED(hr)) {
                        hr = pScaler->Initialize(
                            pSource,
                            destinationWidth,
                            destinationHeight,
                            WICBitmapInterpolationModeCubic
                            );
                    }
                    if (SUCCEEDED(hr)) {
                        hr = pConverter->Initialize(
                            pScaler,
                            GUID_WICPixelFormat32bppPBGRA,
                            WICBitmapDitherTypeNone,
                            nullptr,
                            0.f,
                            WICBitmapPaletteTypeMedianCut
                            );
                    }
                }
            }
            else {
                // 直接初始化
                hr = pConverter->Initialize(
                    pSource,
                    GUID_WICPixelFormat32bppPBGRA,
                    WICBitmapDitherTypeNone,
                    nullptr,
                    0.f,
                    WICBitmapPaletteTypeMedianCut
                    );
            }
        }
#if 0
        // 读取位图数据
        if (SUCCEEDED(hr)) {
            hr = pRenderTarget->CreateBitmapFromWicBitmap(
                pConverter,
                nullptr,
                ppBitmap
                );
        }
#elif 0
        // 计算
        constexpr UINT basic_step = 4;
        pConverter->CopyPixels()
#else
        {
            ID2D1Bitmap1* tmp_bitmap = nullptr;
            ID2D1Bitmap1* tar_bitmap = nullptr;
            // 读取位图数据
            if (SUCCEEDED(hr)) {
                hr = pRenderTarget->CreateBitmapFromWicBitmap(
                    pConverter,
                    nullptr,
                    &tmp_bitmap
                    );
            }
            // 创建位图
            if (SUCCEEDED(hr)) {
                tmp_bitmap->GetOptions();
                hr = pRenderTarget->CreateBitmap(
                    tmp_bitmap->GetPixelSize(),
                    nullptr, 0,
                    D2D1::BitmapProperties1(
                        tmp_bitmap->GetOptions(),
                        tmp_bitmap->GetPixelFormat()
                        ),
                    &tar_bitmap
                    );
            }
            // 复制数据
            if (SUCCEEDED(hr)) {
                hr = tar_bitmap->CopyFromBitmap(nullptr, tmp_bitmap, nullptr);
            }
            // 嫁接
            if (SUCCEEDED(hr)) {
                *ppBitmap = tar_bitmap;
                tar_bitmap = nullptr;
            }
            ::SafeRelease(tmp_bitmap);
            ::SafeRelease(tar_bitmap);
        }
#endif
        ::SafeRelease(pDecoder);
        ::SafeRelease(pSource);
        ::SafeRelease(pStream);
        ::SafeRelease(pConverter);
        ::SafeRelease(pScaler);
        return hr;
    };
    ID2D1Bitmap1* bitmap = nullptr;

    CUIZipReaderStream stream(nullptr, 0);
    // 载入
    auto hr = load_bitmap_from_stream(
        m_manager, m_pWICFactory, &stream, 0u, 0u, &bitmap
        );
    // 失败?
    if (FAILED(hr)) {
        m_manager.ShowError(hr);
    }
    return bitmap;
}
// 获取笔刷
auto LongUI::CUIZipXmlResourceLoader::get_brush(pugi::xml_node node) noexcept -> ID2D1Brush* {
    assert(!"noimpl");
    return nullptr;
}

// get textformat
auto LongUI::CUIZipXmlResourceLoader::get_text_format(pugi::xml_node node) noexcept -> IDWriteTextFormat* {
    assert(!"noimpl");
    return nullptr;
}