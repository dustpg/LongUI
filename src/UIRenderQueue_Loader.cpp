#include "LongUI.h"

// 渲染队列 构造函数
LongUI::CUIRenderQueue::CUIRenderQueue(UIWindow* window) noexcept {
    m_unitLike.length = 0; m_unitLike.window = window;
}

// 渲染队列 析构函数
LongUI::CUIRenderQueue::~CUIRenderQueue() noexcept {
    // 释放数据
    if (m_pUnitsDataBegin) {
        LongUI::NormalFree(m_pUnitsDataBegin);
    }
    m_pUnitsDataBegin = m_pUnitsDataEnd = m_pCurrentUnit = nullptr;
}

// 重置
void LongUI::CUIRenderQueue::Reset(uint32_t freq) noexcept {
    // 一样就不处理
    if (m_wDisplayFrequency == freq) return;
    // 修改
    m_wDisplayFrequency = static_cast<decltype(m_wDisplayFrequency)>(freq);
    // 创建
    CUIRenderQueue::UNIT* data = nullptr;
    if (freq) {
        data = LongUI::NormalAllocT(data, LongUIPlanRenderingTotalTime * freq);
        if (data) {
            for (auto i = 0u; i < LongUIPlanRenderingTotalTime * freq; ++i) {
                data[i].length = 0;
            }
        }
    }
    // TODO: 完成转化
    if (m_pUnitsDataBegin && data) {
        assert(!"NOTIMPL");
    }
    // 释放
    if (m_pUnitsDataBegin) LongUI::NormalFree(m_pUnitsDataBegin);
    // 转移
    if (data) {
        m_pUnitsDataBegin = data;
        m_pUnitsDataEnd = data + LongUIPlanRenderingTotalTime * freq;
        // XXX
        m_pCurrentUnit = data;
    }
    // 开始渲染
    m_dwStartTime = ::timeGetTime();
}

// ++ 操作符
void LongUI::CUIRenderQueue::operator++() noexcept {
    // 渲染队列模式
    if (m_pCurrentUnit) {
        ++m_pCurrentUnit;
        if (m_pCurrentUnit == m_pUnitsDataEnd) {
            m_pCurrentUnit = m_pUnitsDataBegin;
            // 检查误差
            register auto time = m_dwStartTime;
            m_dwStartTime = ::timeGetTime();
            time = m_dwStartTime - time;
            int16_t dev = int16_t(int16_t(time) - int16_t(LongUIPlanRenderingTotalTime * 1000));
            m_sTimeDeviation += dev;
#ifdef _DEBUG
            if (m_unitLike.window->debug_this) {
                UIManager << DL_Log
                    << Formated(L"Time Deviation: %4ldms    Totle: %4ldms", 
                        long(dev), long(m_sTimeDeviation))
                    << endl;
            }
#endif
            // TODO: 时间校正
        }
    }
    // 简单模式
    else {
        assert(!"error");
    }
}

// 计划渲染
void LongUI::CUIRenderQueue::PlanToRender(float wait, float render, UIControl* ctrl) noexcept {
    // XXX: 待优化
    // 当前窗口
    auto window = m_unitLike.window;
#ifdef _DEBUG
    if (window->debug_this) {
        UIManager << DL_Log
            << L"INDEX:[" << long(m_pCurrentUnit - m_pUnitsDataBegin) << L']'
            << ctrl << ctrl->visible_rect
            << L"from " << wait << L" to " << render
            << endl;
    }
#endif
    // 保留刷新
    if (render != 0.0f) render += 0.05f;
    assert((wait + render) < float(LongUIPlanRenderingTotalTime) && "time overflow");
    // 设置单元
    auto set_unit = [window](UNIT& unit, UIControl* ctrl) noexcept {
        // 已经全渲染了就不干
        if (unit.length && unit.units[0] == window) {
            return;
        }
        // 单元满了/直接渲染窗口 就设置为全渲染
        if (unit.length == LongUIDirtyControlSize || ctrl == window) {
            unit.length = 1;
            unit.units[0] = window;
            return;
        }
        // 保存
        auto old_length = unit.length;
        bool changed = false;
        UNIT tmp; std::memset(&tmp, 0, sizeof(tmp));
        std::memcpy(tmp.units, unit.units, sizeof(tmp.units[0]) * old_length);
        // 一次检查
        for (auto itr = tmp.units; itr < tmp.units + old_length; ++itr) {
            // 已存在的空间
            auto existd = *itr;
            // 一样? --> 不干
            if (existd == ctrl) 
                return;
            // 存在深度 < 插入深度 -> 检查插入的是否为存在的子孙结点
            if (existd->level < ctrl->level) {
                // 是 -> 什么不干
                if (existd->IsPosterityForSelf(ctrl)) {
                    return;
                }
                // 否 -> 继续
                else {

                }
            }
            // 存在深度 > 插入深度 -> 检查存在的是否为插入的子孙结点
            else if(existd->level > ctrl->level) {
                // 是 -> 替换所有
                if (ctrl->IsPosterityForSelf(existd)) {
                    *itr = nullptr;
                    changed = true;
                }
                // 否 -> 继续
                else {

                }
            }
            // 深度一致 -> 继续
            else {

            }
        }
#ifdef _DEBUG
        if (window->debug_this) {
            UIManager << DLevel_Log << L"\r\n [INSERT]: " << ctrl << endl;
        }
#endif
        // 二次插入
        if (changed) {
            unit.length = 0; auto witr = unit.units;
            for (auto ritr = tmp.units; ritr < tmp.units + old_length; ++ritr, ++witr) {
                if (*ritr) {
                    *witr = *ritr;
                    ++unit.length;
                }
            }
        }
#ifdef _DEBUG
        auto endt = unit.units + unit.length;
        assert(std::find(unit.units, endt, ctrl) == endt);
#endif
        // 添加到最后
        unit.units[unit.length++] = ctrl;

    };
    // 渲染队列模式
    if (m_pCurrentUnit) {
        // 该控件渲染
        auto rerendered = ctrl->prerender;
        // 时间片计算
        auto frame_offset = uint32_t(wait * float(m_wDisplayFrequency));
        auto frame_count = uint32_t(render * float(m_wDisplayFrequency)) + 1;
        auto start = m_pCurrentUnit + frame_offset;
        for (uint32_t i = 0; i < frame_count; ++i) {
            if (start == m_pUnitsDataEnd) {
                start = m_pUnitsDataBegin;
            }
#ifdef _DEBUG
            if (window->debug_this) {
                UIManager << DLevel_Log << L" [TRY] ";
            }
#endif
            set_unit(*start, rerendered);
            ++start;
        }
#ifdef _DEBUG
        if (window->debug_this) {
            UIManager << DLevel_Log << L"\r\n";
        }
#endif
    }
    // 简单模式
    else {
        assert(!"error");
    }
}

// ---------------- VERSION HELPER -------------------

// longui namespace
namespace LongUI {
     // 检查Windows版本
     bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor) noexcept {
        RTL_OSVERSIONINFOEXW verInfo = { 0 };
        verInfo.dwOSVersionInfoSize = sizeof(verInfo);
        // 获取地址
        static auto RtlGetVersion = reinterpret_cast<LongUI::fnRtlGetVersion>(
            ::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")
            );
        // 比较版本
        if (RtlGetVersion && RtlGetVersion((PRTL_OSVERSIONINFOW)&verInfo) == 0) {
            if (verInfo.dwMajorVersion > wMajorVersion)
                return true;
            else if (verInfo.dwMajorVersion < wMajorVersion)
                return false;
            if (verInfo.dwMinorVersion > wMinorVersion)
                return true;
            else if (verInfo.dwMinorVersion < wMinorVersion)
                return false;
            if (verInfo.wServicePackMajor >= wServicePackMajor)
                return true;
        }
        return false;
    }
    // 获取Windows版本
    auto GetWindowsVersion() noexcept->CUIManager::WindowsVersion {
        CUIManager::WindowsVersion version = CUIManager::WindowsVersion::Version_Win7SP1;
        // >= Win10 ?
        if (LongUI::IsWindows10OrGreater()) {
            version = CUIManager::WindowsVersion::Version_Win10;
        }
        // >= Win8.1 ?
        else if (LongUI::IsWindows8Point1OrGreater()) {
            version = CUIManager::WindowsVersion::Version_Win8_1;
        }
        // >= Win8 ?
        else if (LongUI::IsWindows8OrGreater()) {
            version = CUIManager::WindowsVersion::Version_Win8;
        }
        return version;
    }
}

#ifdef LONGUI_WITH_DEFAULT_CONFIG
#include <wincodec.h>
//  ---------- Resource Loader for XML -----------------
namespace LongUI {
    // IWICImagingFactory2 "7B816B45-1996-4476-B132-DE9E247C8AF0"
    static const IID IID_IWICImagingFactory2 =
    { 0x7B816B45, 0x1996, 0x4476,{ 0xB1, 0x32, 0xDE, 0x9E, 0x24, 0x7C, 0x8A, 0xF0 } };
    template<> LongUIInline const IID& GetIID<IWICImagingFactory2>() {
        return LongUI::IID_IWICImagingFactory2;
    }
    // CUIResourceLoaderXML, default impl for IUIResourceLoader
    class CUIResourceLoaderXML : public IUIResourceLoader {
    public:
        // add ref count
        auto STDMETHODCALLTYPE AddRef() noexcept->ULONG override final { return ++m_dwCounter; }
        // release this
        auto STDMETHODCALLTYPE Release() noexcept->ULONG override final { 
            auto old = --m_dwCounter; 
            if (!old) { delete this; } 
            return old; 
        };
    public:
        // get resouce count with type
        auto GetResourceCount(ResourceType type) const noexcept->size_t override;
        // get resouce by index, index in range [0, count)
        // for Type_Bitmap, Type_Brush, Type_TextFormat
        auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* override;
        // get meta by index, index in range [0, count)
        auto GetMeta(size_t index, DeviceIndependentMeta&) noexcept ->void override;
    private:
        // get resouce count from doc
        void get_resource_count_from_xml() noexcept;
        // get bitmap
        auto get_bitmap(pugi::xml_node node) noexcept->ID2D1Bitmap1*;
        // get brush
        auto get_brush(pugi::xml_node node) noexcept->ID2D1Brush*;
        // get text format
        auto get_text_format(pugi::xml_node node) noexcept->IDWriteTextFormat*;
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
        ::memset(m_aResourceCount, 0, sizeof(m_aResourceCount));
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
        ::SafeRelease(m_pWICFactory);
    }
    // get reource count
    auto LongUI::CUIResourceLoaderXML::GetResourceCount(ResourceType type) const noexcept -> size_t {
        assert(type < this->RESOURCE_TYPE_COUNT);
        return static_cast<size_t>(m_aResourceCount[type]);
    }
    // get reource
    auto LongUI::CUIResourceLoaderXML::GetResourcePointer(ResourceType type, size_t index) noexcept -> void * {
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
    auto LongUI::CUIResourceLoaderXML::GetMeta(size_t index, DeviceIndependentMeta& meta_raw) noexcept -> void {
        auto node = this->find_node_with_index(m_aNodes[this->Type_Meta], index);
        assert(node && "node not found");
        meta_raw = {
            { 0.f, 0.f, 1.f, 1.f },
            uint32_t(LongUI::AtoI(node.attribute("bitmap").value())),
            Helper::XMLGetBitmapRenderRule(node, BitmapRenderRule::Rule_Scale),
            uint16_t(Helper::XMLGetD2DInterpolationMode(node, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR))
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
                m_aNodes[Type_Bitmap] = now_node;
                m_aResourceCount[this->Type_Bitmap] = get_children_count(now_node);
            }
            // 笔刷?
            else if (!::strcmp(now_node.name(), "Brush")) {
                m_aNodes[Type_Brush] = now_node;
                m_aResourceCount[this->Type_Brush] = get_children_count(now_node);
            }
            // 文本格式?
            else if (!::strcmp(now_node.name(), "Font") ||
                !::strcmp(now_node.name(), "TextFormat")) {
                m_aNodes[Type_TextFormat] = now_node;
                m_aResourceCount[this->Type_TextFormat] = get_children_count(now_node);
            }
            // 图元?
            else if (!::strcmp(now_node.name(), "Meta")) {
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
    auto LongUI::CUIResourceLoaderXML::get_bitmap(pugi::xml_node node) noexcept -> ID2D1Bitmap1* {
        assert(node && "node not found");
        // 获取路径
        const char* uri = node.attribute("res").value();
        assert(uri && *uri && "Error URI of Bitmap");
        // 从文件载入位图
        auto load_bitmap_from_file = [](
            LongUIRenderTarget *pRenderTarget,
            IWICImagingFactory *pIWICFactory,
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
            register HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
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
        // 转换路径
        wchar_t path_buffer[LongUIStringBufferLength];
        path_buffer[LongUI::UTF8toWideChar(uri, path_buffer)] = 0;
        // 载入
        auto hr = load_bitmap_from_file(
            m_manager.GetRenderTargetNoAddRef(), 
            m_pWICFactory, path_buffer, 0u, 0u, &bitmap
            );
        // 失败?
        if (FAILED(hr)) {
            m_manager.ShowError(hr);
        }
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
                char buffer[LongUIStringBufferLength];
                // 复制到缓冲区
                std::strcpy(buffer, str);
                char* index = buffer;
                const char* paragraph = nullptr;
                register char ch = 0;
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
                    Helper::XMLGetD2DExtendMode(node, D2D1_EXTEND_MODE_CLAMP, "extendx"),
                    Helper::XMLGetD2DExtendMode(node, D2D1_EXTEND_MODE_CLAMP, "extendy"),
                    Helper::XMLGetD2DInterpolationMode(node, D2D1_INTERPOLATION_MODE_LINEAR, "interpolation"),
                };
                // 创建位图笔刷
                m_manager.GetRenderTargetNoAddRef()->CreateBitmapBrush(
                    bitmap, &bbprop, &brush_prop, &b1b
                    );
                ::SafeRelease(bitmap);
            }
            break;
        }
        assert(brush && "unknown error but error");
        return brush;
    }
    // get textformat
    auto LongUI::CUIResourceLoaderXML::get_text_format(pugi::xml_node node) noexcept -> IDWriteTextFormat* {
        const char* str = nullptr;
        assert(node && "node not found");
        CUIString fontfamilyname(LongUIDefaultTextFontName);
        DWRITE_FONT_WEIGHT fontweight = DWRITE_FONT_WEIGHT_NORMAL;
        float fontsize = LongUIDefaultTextFontSize;
        // 获取字体名称
        Helper::MakeString(node.attribute("family").value(), fontfamilyname);
        // 获取字体大小
        if (str = node.attribute("size").value()) {
            fontsize = LongUI::AtoF(str);
        }
        // 获取字体粗细
        if (str = node.attribute("weight").value()) {
            fontweight = static_cast<DWRITE_FONT_WEIGHT>(LongUI::AtoI(str));
        }
        // 创建基本字体
        IDWriteTextFormat* textformat = nullptr;
        m_manager.CreateTextFormat(
            fontfamilyname.c_str(),
            fontweight,
            Helper::XMLGetFontStyle(node, DWRITE_FONT_STYLE_NORMAL),
            Helper::XMLGetFontStretch(node, DWRITE_FONT_STRETCH_NORMAL),
            fontsize,
            &textformat
            );
        // 成功获取则再设置
        if (textformat) {
            // Tab宽度
            float tabstop = fontsize * 4.f;
            // 检查Tab宽度
            if (str = node.attribute("tabstop").value()) {
                tabstop = LongUI::AtoF(str);
            }
            // 设置段落排列方向
            textformat->SetFlowDirection(
                Helper::XMLGetFlowDirection(node, DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM)
                );
            // 设置Tab宽度
            textformat->SetIncrementalTabStop(tabstop);
            // 设置段落(垂直)对齐
            textformat->SetParagraphAlignment(
                Helper::XMLGetVAlignment(node, DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
                );
            // 设置文本(水平)对齐
            textformat->SetTextAlignment(
                Helper::XMLGetHAlignment(node, DWRITE_TEXT_ALIGNMENT_LEADING)
                );
            // 设置阅读进行方向
            textformat->SetReadingDirection(
                Helper::XMLGetReadingDirection(node, DWRITE_READING_DIRECTION_LEFT_TO_RIGHT)
                );
            // 设置自动换行
            textformat->SetWordWrapping(
                Helper::XMLGetWordWrapping(node, DWRITE_WORD_WRAPPING_NO_WRAP)
                );
        }
        return textformat;
    }
}
#endif
