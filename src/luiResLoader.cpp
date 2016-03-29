#include "luibase.h"
#include "luiconf.h"
#include "Core/luiInterface.h"
#include "LongUI/luiUiHlper.h"
#include "LongUI/luiUiXml.h"
#include "LongUI/luiUiMeta.h"
#include "Core/luiManager.h"
#include <algorithm>
#include <WinError.h>


#ifdef LONGUI_WITH_DEFAULT_CONFIG

#include <wincodec.h>
// longui::impl 命名空间
namespace LongUI { namespace impl {
    // 从文件载入位图
    auto load_bitmap_from_file(
        ID2D1DeviceContext* pRenderTarget,
        IWICImagingFactory* pIWICFactory,
        PCWSTR uri,
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
        HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
            uri,
            nullptr,
            GENERIC_READ,
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
            LongUI::SafeRelease(tmp_bitmap);
            LongUI::SafeRelease(tar_bitmap);
        }
#endif
        LongUI::SafeRelease(pDecoder);
        LongUI::SafeRelease(pSource);
        LongUI::SafeRelease(pStream);
        LongUI::SafeRelease(pConverter);
        LongUI::SafeRelease(pScaler);
        return hr;
    };
}}
//  ---------- Resource Loader for XML -----------------
namespace LongUI {
    // IWICImagingFactory2 "7B816B45-1996-4476-B132-DE9E247C8AF0"
    static const IID IID_IWICImagingFactory2 = {
        0x7B816B45, 0x1996, 0x4476,{ 0xB1, 0x32, 0xDE, 0x9E, 0x24, 0x7C, 0x8A, 0xF0 }
    };
    template<> LongUIInline const IID& GetIID<IWICImagingFactory2>() {
        return LongUI::IID_IWICImagingFactory2;
    }
    // CUIResourceLoaderXML, default impl for IUIResourceLoader
    class CUIResourceLoaderXML : public IUIResourceLoader {
    public:
        // add ref count
        auto STDMETHODCALLTYPE AddRef() noexcept ->ULONG override final { return ++m_dwCounter; }
        // release this
        auto STDMETHODCALLTYPE Release() noexcept ->ULONG override final { 
            auto old = --m_dwCounter; 
            if (!old) { delete this; } 
            return old; 
        };
    public:
        // get resouce count with type
        auto GetResourceCount(ResourceType type) const noexcept ->size_t override;
        // get resouce by index, index in range [0, count)
        // for Type_Bitmap, Type_Brush, Type_TextFormat
        auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* override;
        // get meta by index, index in range [0, count)
        auto GetMeta(size_t index, DeviceIndependentMeta&) noexcept ->void override;
    private:
        // get resouce count from doc
        void get_resource_count_from_xml() noexcept;
        // get bitmap
        auto get_bitmap(pugi::xml_node node) noexcept ->ID2D1Bitmap1*;
        // get brush
        auto get_brush(pugi::xml_node node) noexcept ->ID2D1Brush*;
        // get text format
        auto get_text_format(pugi::xml_node node) noexcept ->IDWriteTextFormat*;
        // find node with index
        static auto find_node_with_index(pugi::xml_node node, const size_t index) noexcept {
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
    public:
        // ctor
        CUIResourceLoaderXML(CUIManager& manager, const char* xml) noexcept;
        // dtor
        ~CUIResourceLoaderXML() noexcept;
    private:
        //  ui manager
        CUIManager&             m_manager;
        // WIC factory
        IWICImagingFactory2*    m_pWICFactory = nullptr;
        // node for reource
        pugi::xml_node          m_aNodes[RESOURCE_TYPE_COUNT];
        // xml doc for resource
        pugi::xml_document      m_docResource;
        // resource count
        uint32_t                m_aResourceCount[RESOURCE_TYPE_COUNT];
        // ref counter for this
        uint32_t                m_dwCounter = 1;
    };
    // --------------------------------------------------------------------
    // create resource loader
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) 
        noexcept -> IUIResourceLoader* {
        return new(std::nothrow) CUIResourceLoaderXML(manager, xml);
    }
    // ctor for CUIResourceLoaderXML
    LongUI::CUIResourceLoaderXML::CUIResourceLoaderXML(
        CUIManager& manager, const char* xml)  noexcept : m_manager(manager) {
        // 初始化
        std::memset(m_aResourceCount, 0, sizeof(m_aResourceCount));
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
        // 载入
        if (SUCCEEDED(hr) && xml) {
            auto re = m_docResource.load_string(xml);
            // 错误
            if (re.status) {
                assert(!"failed to load string");
                ::MessageBoxA(nullptr, re.description(), "<LongUI::CUIResourceLoaderXML::CUIResourceLoaderXML>: Failed to Parse/Load XML", MB_ICONERROR);
                hr = E_FAIL;
            }
            // 遍历
            else {
                this->get_resource_count_from_xml();
            }
        }
        // 显示错误
        if (FAILED(hr)) {
            manager.ShowError(hr);
        }
    }
    // dtor for CUIResourceLoaderXML
    LongUI::CUIResourceLoaderXML::~CUIResourceLoaderXML() noexcept {
        LongUI::SafeRelease(m_pWICFactory);
    }
    // get reource count
    auto LongUI::CUIResourceLoaderXML::GetResourceCount(ResourceType type) const noexcept -> size_t {
        assert(type < this->RESOURCE_TYPE_COUNT);
        return static_cast<size_t>(m_aResourceCount[type]);
    }
    // get reource
    auto LongUI::CUIResourceLoaderXML::GetResourcePointer(ResourceType type, size_t index) noexcept -> void* {
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
            data = this->get_text_format(node);
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
    auto LongUI::CUIResourceLoaderXML::GetMeta(size_t index, DeviceIndependentMeta& meta_raw) noexcept -> void {
        auto node = this->find_node_with_index(m_aNodes[this->Type_Meta], index);
        assert(node && "node not found");
        meta_raw = {
            { 0.f, 0.f, 1.f, 1.f },
            uint32_t(LongUI::AtoI(node.attribute("bitmap").value())),
            Helper::GetEnumFromXml(node, BitmapRenderRule::Rule_Scale),
            uint16_t(Helper::GetEnumFromXml(node, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR))
        };
        assert(meta_raw.bitmap_index && "bad bitmap index");
        // 获取矩形
        Helper::MakeFloats(node.attribute("rect").value(), &meta_raw.src_rect.left, 4);
    }
    // get reource count from doc
    void LongUI::CUIResourceLoaderXML::get_resource_count_from_xml() noexcept {
        // 初始化
        for (auto& node : m_aNodes) { node = pugi::xml_node(); }
        // pugixml 使用的是句柄式, 所以下面的代码是安全的.
        auto now_node = m_docResource.first_child().first_child();
        while (now_node) {
            // 获取子结点数量
            auto get_children_count = [](pugi::xml_node node) {
                node = node.first_child();
                auto count = 0ui32;
                while (node) { node = node.next_sibling(); ++count; }
                return count;
            };
            // 位图?
            if (!std::strcmp(now_node.name(), "Bitmap")) {
                m_aNodes[Type_Bitmap] = now_node;
                m_aResourceCount[this->Type_Bitmap] = get_children_count(now_node);
            }
            // 笔刷?
            else if (!std::strcmp(now_node.name(), "Brush")) {
                m_aNodes[Type_Brush] = now_node;
                m_aResourceCount[this->Type_Brush] = get_children_count(now_node);
            }
            // 文本格式?
            else if (!std::strcmp(now_node.name(), "Font") ||
                !std::strcmp(now_node.name(), "TextFormat")) {
                m_aNodes[Type_TextFormat] = now_node;
                m_aResourceCount[this->Type_TextFormat] = get_children_count(now_node);
            }
            // 图元?
            else if (!std::strcmp(now_node.name(), "Meta")) {
                m_aNodes[Type_Meta] = now_node;
                m_aResourceCount[this->Type_Meta] = get_children_count(now_node);
            }
            // 动画图元?
            else if (!std::strcmp(now_node.name(), "MetaEx")) {
                assert(!"unsupport yet");
            }
            // 推进
            now_node = now_node.next_sibling();
        }
    }
    // 获取位图
    auto LongUI::CUIResourceLoaderXML::get_bitmap(pugi::xml_node node) noexcept -> ID2D1Bitmap1* {
        assert(node && "node not found");
        // 获取路径
        const char* uri = node.attribute("res").value();
        assert(uri && *uri && "Error URI of Bitmap");
        ID2D1Bitmap1* bitmap = nullptr;
        // 转换路径
        LongUI::SafeUTF8toWideChar(uri, [this, &bitmap](wchar_t* begin, wchar_t* end) {
            // 载入
            auto hr = impl::load_bitmap_from_file(
                m_manager.GetRenderTargetNoAddRef(), 
                m_pWICFactory, begin, 0u, 0u, &bitmap
            );
            // 失败?
#ifdef _DEBUG
            if (FAILED(hr)) {
                wchar_t tmp[MAX_PATH * 2];
                std::memset(tmp, 0, sizeof(tmp));
                std::swprintf(
                    tmp, size_t(end-begin),
                    L"File Path -- '%ls'",
                    begin
                );
                m_manager.ShowError(hr, tmp);
            }
#endif
        });
        return bitmap;
    }
    // 获取笔刷
    auto LongUI::CUIResourceLoaderXML::get_brush(pugi::xml_node node) noexcept -> ID2D1Brush* {
        union {
            ID2D1SolidColorBrush*       scb;
            ID2D1LinearGradientBrush*   lgb;
            ID2D1RadialGradientBrush*   rgb;
            ID2D1BitmapBrush1*          b1b;
            ID2D1Brush*                 brush;
        };
        brush = nullptr; const char* str = nullptr;
        assert(node && "bad argument");
        // 笔刷属性
        D2D1_BRUSH_PROPERTIES brush_prop = D2D1::BrushProperties();
        if (str = node.attribute("opacity").value()) {
            brush_prop.opacity = static_cast<float>(::LongUI::AtoF(str));
        }
        if (str = node.attribute("transform").value()) {
            Helper::MakeFloats(str, &brush_prop.transform._11, 6);
        }
        // 检查类型
        auto type = BrushType::Type_SolidColor;
        if (str = node.attribute("type").value()) {
            type = static_cast<decltype(type)>(::LongUI::AtoI(str));
        }
        switch (type)
        {
        case LongUI::BrushType::Type_SolidColor:
        {
            D2D1_COLOR_F color;
            // 获取颜色
            if (!Helper::MakeColor(node.attribute("color").value(), color)) {
                color = D2D1::ColorF(D2D1::ColorF::Black);
            }
            m_manager.GetRenderTargetNoAddRef()->CreateSolidColorBrush(&color, &brush_prop, &scb);
        }
        break;
        case LongUI::BrushType::Type_LinearGradient:
            __fallthrough;
        case LongUI::BrushType::Type_RadialGradient:
            if (str = node.attribute("stops").value()) {
                // 语法 [pos0, color0] [pos1, color1] ....
                uint32_t stop_count = 0;
                ID2D1GradientStopCollection * collection = nullptr;
                D2D1_GRADIENT_STOP stops[LongUIMaxGradientStop];
                D2D1_GRADIENT_STOP* now_stop = stops;
                // 缓冲池
                LongUI::SafeBuffer<char>(std::strlen(str) + 1, 
                    [str, &now_stop, &stop_count](char* buffer) {
                    // 复制到缓冲区
                    std::strcpy(buffer, str);
                    char* index = buffer;
                    const char* paragraph = nullptr;
                    char ch = 0;
                    bool ispos = false;
                    // 遍历检查
                    while (ch = *index) {
                        // 查找第一个浮点数做为位置
                        if (ispos) {
                            // ,表示位置段结束, 该解析了
                            if (ch == ',') {
                                *index = 0;
                                now_stop->position = LongUI::AtoF(paragraph);
                                ispos = false;
                                paragraph = index + 1;
                            }
                        }
                        // 查找后面的数值做为颜色
                        else {
                            // [ 做为位置段标识开始
                            if (ch == '[') {
                                paragraph = index + 1;
                                ispos = true;
                            }
                            // ] 做为颜色段标识结束 该解析了
                            else if (ch == ']') {
                                *index = 0;
                                Helper::MakeColor(paragraph, now_stop->color);
                                ++now_stop;
                                ++stop_count;
                            }
                        }
                    }
                });
                // 创建StopCollection
                m_manager.GetRenderTargetNoAddRef()->CreateGradientStopCollection(stops, stop_count, &collection);
                if (collection) {
                    // 线性渐变?
                    if (type == LongUI::BrushType::Type_LinearGradient) {
                        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbprop = {
                            { 0.f, 0.f },{ 0.f, 0.f }
                        };
                        // 检查属性
                        Helper::MakeFloats(node.attribute("start").value(), &lgbprop.startPoint.x, 2);
                        Helper::MakeFloats(node.attribute("end").value(), &lgbprop.endPoint.x, 2);
                        // 创建笔刷
                        m_manager.GetRenderTargetNoAddRef()->CreateLinearGradientBrush(
                            &lgbprop, &brush_prop, collection, &lgb
                            );
                    }
                    // 径向渐变笔刷
                    else {
                        D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES rgbprop = {
                            { 0.f, 0.f },{ 0.f, 0.f }, 0.f, 0.f
                        };
                        // 检查属性
                        Helper::MakeFloats(node.attribute("center").value(), &rgbprop.center.x, 2);
                        Helper::MakeFloats(node.attribute("offset").value(), &rgbprop.gradientOriginOffset.x, 2);
                        Helper::MakeFloats(node.attribute("rx").value(), &rgbprop.radiusX, 1);
                        Helper::MakeFloats(node.attribute("ry").value(), &rgbprop.radiusY, 1);
                        // 创建笔刷
                        m_manager.GetRenderTargetNoAddRef()->CreateRadialGradientBrush(
                            &rgbprop, &brush_prop, collection, &rgb
                            );
                    }
                    collection->Release();
                    collection = nullptr;
                }
            }
            break;
        case LongUI::BrushType::Type_Bitmap:
            // 有效数据
            if (str = node.attribute("bitmap").value()) {
                // 创建笔刷
                auto bitmap = m_manager.GetBitmap(size_t(LongUI::AtoI(str)));
                // 基本参数
                D2D1_BITMAP_BRUSH_PROPERTIES1 bbprop = {
                    Helper::GetEnumFromXml(node, D2D1_EXTEND_MODE_CLAMP, "extendx"),
                    Helper::GetEnumFromXml(node, D2D1_EXTEND_MODE_CLAMP, "extendy"),
                    Helper::GetEnumFromXml(node, D2D1_INTERPOLATION_MODE_LINEAR, "interpolation"),
                };
                // 创建位图笔刷
                m_manager.GetRenderTargetNoAddRef()->CreateBitmapBrush(
                    bitmap, &bbprop, &brush_prop, &b1b
                    );
                LongUI::SafeRelease(bitmap);
            }
            break;
        }
        assert(brush && "unknown error but error");
        return brush;
    }
    // get textformat
    auto LongUI::CUIResourceLoaderXML::get_text_format(pugi::xml_node node) noexcept -> IDWriteTextFormat* {
        assert(node && "node not found");
        IDWriteTextFormat* fmt = nullptr;
        auto hr = DX::MakeTextFormat(node, &fmt);
        UNREFERENCED_PARAMETER(hr);
        assert(SUCCEEDED(hr));
        return fmt;
    }
    // default config
    CUIDefaultConfigure::CUIDefaultConfigure(CUIManager& manager, const char* log_file) noexcept : m_manager(manager) {
#ifdef _DEBUG
        if (log_file) {
            m_pLogFile = std::fopen(log_file, "ab");
        }
#endif
    }
    // dtor
    CUIDefaultConfigure::~CUIDefaultConfigure() noexcept {
#ifdef _DEBUG
        if (m_pLogFile) {
            std::fclose(m_pLogFile);
            m_pLogFile = nullptr;
        }
#endif
    }
    // get string
    auto CUIDefaultConfigure::GetString(TableString tbl) noexcept -> const wchar_t* {
        switch (tbl)
        {
        case LongUI::String_FaildHR:
            return L"Faild with HREULT CODE: ";
        case LongUI::String_Cut:
            return L"Cut\tCtrl + X";
        case LongUI::String_Copy:
            return L"Copy\tCtrl + C";
        case LongUI::String_Paste:
            return L"Paste\tCtrl + V";
        case LongUI::String_SelectAll:
            return L"Select All\tCtrl + A";
        default:
            assert(!"Unknown Table");
            return L"Unknown";
        }
    }
}
#endif
