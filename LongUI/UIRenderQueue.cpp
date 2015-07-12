#include "LongUI.h"


// 渲染队列 构造函数
LongUI::CUIRenderQueue::CUIRenderQueue(UIWindow* window) noexcept {
    m_unitLike.length = 0; m_unitLike.window = window;
    // get window
    // static_cast<UIWindow*>(m_unitLike.window)
}

// 渲染队列 析构函数
LongUI::CUIRenderQueue::~CUIRenderQueue() noexcept {

}

// 重置
void LongUI::CUIRenderQueue::Reset(uint32_t freq) noexcept {
    // 一样就不处理
    if (m_wDisplayFrequency == freq) return;
    // 修改
    m_wDisplayFrequency = freq;
    // 创建
    CUIRenderQueue::UNIT* data = nullptr;
    if (freq) {
        data = LongUI::CtrlAllocT(data, LongUIPlanRenderingTotalTime * freq);
        if (data) {
            for (auto i = 0u; i < LongUIPlanRenderingTotalTime * freq; ++i) {
                data[i].length = 0;
            }
        }
    }
    // TODO: 完成转化
    if (m_pUnitsDataBegin && data) {

    }
    // 释放
    if (m_pUnitsDataBegin) LongUI::CtrlFree(m_pUnitsDataBegin);
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
            UIManager << DL_Log << "Time Deviation: "
                << long(dev) << " ms    Totle: " << long(m_sTimeDeviation)
                << " ms" << endl;
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
    // 保留刷新
    if (render != 0.0f) render += 0.1f;
    assert((wait + render) < float(LongUIPlanRenderingTotalTime) && "time overflow");
    // 当前窗口
    auto window = m_unitLike.window;
    // 设置单元
    auto set_unit = [window](UNIT* unit, UIControl* ctrl) noexcept {
        // 已经全渲染了就不干
        if (unit->length && unit->units[0] == window) {
            return;
        }
        // 单元满了就设置为全渲染
        if (unit->length == LongUIDirtyControlSize) {
            unit->length = 1;
            unit->units[0] = window;
            return;
        }
        // 获取真正窗口
        auto get_real_render_control = [window](UIControl* control) noexcept {
            // 获取真正
            while (control != window) {
                if (control->flags & Flag_RenderParent) control = control->parent;
                else break;
            }
            return control;
        };
        // 渲染窗口也设置为全渲染
        ctrl = get_real_render_control(ctrl);
        if (ctrl == window) {
            unit->length = 1;
            unit->units[0] = window;
            return;
        }
#if 0
        // 检查是否在单元里面
        register bool not_in = true;
        for (auto unit_ctrl = unit->units; unit_ctrl < unit->units + unit->length; ++unit_ctrl) {
            if (*unit_ctrl == ctrl) {
                not_in = false;
                break;
            }
        }
        // 不在单元里面就加入
        if (not_in) {
            unit->units[unit->length] = ctrl;
            ++unit->length;
        }
#else
        // 不在单元里面就加入
        if (std::none_of(unit->units, unit->units + unit->length, [ctrl](UIControl* unit) {
            return unit == ctrl;
        })) {
            unit->units[unit->length] = ctrl;
            ++unit->length;
        }
#endif
    };
    // 渲染队列模式
    if (m_pCurrentUnit) {
        // 时间片计算
        auto frame_offset = long(wait * float(m_wDisplayFrequency));
        auto frame_count = long(render * float(m_wDisplayFrequency)) + 1;
        auto start = m_pCurrentUnit + frame_offset;
        for (long i = 0; i < frame_count; ++i) {
            if (start >= m_pUnitsDataEnd) {
                start -= LongUIPlanRenderingTotalTime * m_wDisplayFrequency;
            }
            set_unit(start, ctrl);
            ++start;
        }
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
        // qi
        auto STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject) noexcept->HRESULT override final { return E_NOINTERFACE; }
        // add ref count
        auto STDMETHODCALLTYPE AddRef() noexcept->ULONG override final { return ++m_dwCounter; }
        // release this
        auto STDMETHODCALLTYPE Release() noexcept->ULONG override final { auto old = --m_dwCounter; if (!old) { delete this; } return old; };
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
            BitmapRenderRule::Rule_Scale,
            D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR
        };
        assert(meta_raw.bitmap_index && "bad bitmap index");
        const char* str = nullptr;
        // 获取渲染规则
        if (str = node.attribute("rule").value()) {
            meta_raw.rule = static_cast<BitmapRenderRule>(LongUI::AtoI(str));
        }
        // 获取插值模式
        if (str = node.attribute("interpolation").value()) {
            meta_raw.interpolation = static_cast<uint16_t>(LongUI::AtoI(str));
        }
        // 获取矩形
        UIControl::MakeFloats(node.attribute("rect").value(), &meta_raw.src_rect.left, 4);
    }
    // get reource count from doc
    void LongUI::CUIResourceLoaderXML::get_resource_count_from_xml() noexcept {
        // 初始化
        ::memset(m_aResourceCount, 0, sizeof(m_aResourceCount));
        for (auto& node : m_aNodes) { node = pugi::xml_node(); }
        // pugixml 使用的是句柄式, 所以下面的代码是安全的.
        register auto now_node = m_docResource.first_child().first_child();
        while (now_node) {
            // 位图?
            if (!::strcmp(now_node.name(), "Bitmap")) {
                // XXX: PUGIXML 直接读取(XPATH?)
                m_aNodes[Type_Bitmap] = now_node;
                for (auto& i : now_node) {
                    ++m_aResourceCount[this->Type_Bitmap];
                }
            }
            // 笔刷?
            else if (!::strcmp(now_node.name(), "Brush")) {
                // XXX: PUGIXML 直接读取
                m_aNodes[Type_Brush] = now_node;
                for (auto& i : now_node) {
                    ++m_aResourceCount[this->Type_Brush];
                }
            }
            // 文本格式?
            else if (!::strcmp(now_node.name(), "Font") ||
                !::strcmp(now_node.name(), "TextFormat")) {
                // XXX: PUGIXML 直接读取
                m_aNodes[Type_TextFormat] = now_node;
                for (auto& i : now_node) {
                    ++m_aResourceCount[this->Type_TextFormat];
                }
            }
            // 图元?
            else if (!::strcmp(now_node.name(), "Meta")) {
                // XXX: PUGIXML 直接读取
                m_aNodes[Type_Meta] = now_node;
                for (auto& i : now_node) {
                    ++m_aResourceCount[this->Type_Meta];
                }
            }
            // 动画图元?
            else if (!::strcmp(now_node.name(), "MetaEx")) {
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
        assert(uri && uri != "" && "Error URI of Bitmap");
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
            m_manager, m_pWICFactory, path_buffer, 0u, 0u, &bitmap
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
            UIControl::MakeFloats(str, &brush_prop.transform._11, 6);
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
            if (!UIControl::MakeColor(node.attribute("color").value(), color)) {
                color = D2D1::ColorF(D2D1::ColorF::Black);
            }
            static_cast<LongUIRenderTarget*>(m_manager)->CreateSolidColorBrush(&color, &brush_prop, &scb);
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

                char buffer[LongUIStringBufferLength];
                // 复制到缓冲区
                strcpy(buffer, str);
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
                            UIControl::MakeColor(paragraph, now_stop->color);
                            ++now_stop;
                            ++stop_count;
                        }
                    }
                }
                // 创建StopCollection
                static_cast<LongUIRenderTarget*>(m_manager)->CreateGradientStopCollection(stops, stop_count, &collection);
                if (collection) {
                    // 线性渐变?
                    if (type == LongUI::BrushType::Type_LinearGradient) {
                        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbprop = {
                            { 0.f, 0.f },{ 0.f, 0.f }
                        };
                        // 检查属性
                        UIControl::MakeFloats(node.attribute("start").value(), &lgbprop.startPoint.x, 2);
                        UIControl::MakeFloats(node.attribute("end").value(), &lgbprop.startPoint.x, 2);
                        // 创建笔刷
                        static_cast<LongUIRenderTarget*>(m_manager)->CreateLinearGradientBrush(
                            &lgbprop, &brush_prop, collection, &lgb
                            );
                    }
                    // 径向渐变笔刷
                    else {
                        D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES rgbprop = {
                            { 0.f, 0.f },{ 0.f, 0.f }, 0.f, 0.f
                        };
                        // 检查属性
                        UIControl::MakeFloats(node.attribute("center").value(), &rgbprop.center.x, 2);
                        UIControl::MakeFloats(node.attribute("offset").value(), &rgbprop.gradientOriginOffset.x, 2);
                        UIControl::MakeFloats(node.attribute("rx").value(), &rgbprop.radiusX, 1);
                        UIControl::MakeFloats(node.attribute("ry").value(), &rgbprop.radiusY, 1);
                        // 创建笔刷
                        static_cast<LongUIRenderTarget*>(m_manager)->CreateRadialGradientBrush(
                            &rgbprop, &brush_prop, collection, &rgb
                            );
                    }
                    collection->Release();
                    collection = nullptr;
                }
            }
            break;
        case LongUI::BrushType::Type_Bitmap:
            if (str = node.attribute("bitmap").value()) {
                auto index = LongUI::AtoI(str);
                // 基本参数
                D2D1_BITMAP_BRUSH_PROPERTIES1 bbprop = {
                    D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP,D2D1_INTERPOLATION_MODE_LINEAR
                };
                // 检查属性
                if (str = node.attribute("extendx").value()) {
                    bbprop.extendModeX = static_cast<D2D1_EXTEND_MODE>(LongUI::AtoI(str));
                }
                if (str = node.attribute("extendy").value()) {
                    bbprop.extendModeY = static_cast<D2D1_EXTEND_MODE>(LongUI::AtoI(str));
                }
                if (str = node.attribute("interpolation").value()) {
                    bbprop.interpolationMode = static_cast<D2D1_INTERPOLATION_MODE>(LongUI::AtoI(str));
                }
                // 创建笔刷
                auto bitmap = m_manager.GetBitmap(index);
                static_cast<LongUIRenderTarget*>(m_manager)->CreateBitmapBrush(
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
        CUIString fontfamilyname(L"Arial");
        DWRITE_FONT_WEIGHT fontweight = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE fontstyle = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH fontstretch = DWRITE_FONT_STRETCH_NORMAL;
        float fontsize = 12.f;
        // 获取字体名称
        UIControl::MakeString(node.attribute("family").value(), fontfamilyname);
        // 获取字体粗细
        if (str = node.attribute("weight").value()) {
            fontweight = static_cast<DWRITE_FONT_WEIGHT>(LongUI::AtoI(str));
        }
        // 获取字体风格
        if (str = node.attribute("style").value()) {
            fontstyle = static_cast<DWRITE_FONT_STYLE>(LongUI::AtoI(str));
        }
        // 获取字体拉伸
        if (str = node.attribute("stretch").value()) {
            fontstretch = static_cast<DWRITE_FONT_STRETCH>(LongUI::AtoI(str));
        }
        // 获取字体大小
        if (str = node.attribute("size").value()) {
            fontsize = LongUI::AtoF(str);
        }
        // 创建基本字体
        IDWriteTextFormat* textformat = nullptr;
        m_manager.CreateTextFormat(
            fontfamilyname.c_str(),
            fontweight,
            fontstyle,
            fontstretch,
            fontsize,
            &textformat
            );
        // 成功获取则再设置
        if (textformat) {
            // DWRITE_LINE_SPACING_METHOD;
            DWRITE_FLOW_DIRECTION flowdirection = DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM;
            float tabstop = fontsize * 4.f;
            DWRITE_PARAGRAPH_ALIGNMENT valign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
            DWRITE_TEXT_ALIGNMENT halign = DWRITE_TEXT_ALIGNMENT_LEADING;
            DWRITE_READING_DIRECTION readingdirection = DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
            DWRITE_WORD_WRAPPING wordwrapping = DWRITE_WORD_WRAPPING_NO_WRAP;
            // 检查段落排列方向
            if (str = node.attribute("flowdirection").value()) {
                flowdirection = static_cast<DWRITE_FLOW_DIRECTION>(LongUI::AtoI(str));
            }
            // 检查Tab宽度
            if (str = node.attribute("tabstop").value()) {
                tabstop = LongUI::AtoF(str);
            }
            // 检查段落(垂直)对齐
            if (str = node.attribute("valign").value()) {
                valign = static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(LongUI::AtoI(str));
            }
            // 检查文本(水平)对齐
            if (str = node.attribute("halign").value()) {
                halign = static_cast<DWRITE_TEXT_ALIGNMENT>(LongUI::AtoI(str));
            }
            // 检查阅读进行方向
            if (str = node.attribute("readingdirection").value()) {
                readingdirection = static_cast<DWRITE_READING_DIRECTION>(LongUI::AtoI(str));
            }
            // 检查自动换行
            if (str = node.attribute("wordwrapping").value()) {
                wordwrapping = static_cast<DWRITE_WORD_WRAPPING>(LongUI::AtoI(str));
            }
            // 设置段落排列方向
            textformat->SetFlowDirection(flowdirection);
            // 设置Tab宽度
            textformat->SetIncrementalTabStop(tabstop);
            // 设置段落(垂直)对齐
            textformat->SetParagraphAlignment(valign);
            // 设置文本(水平)对齐
            textformat->SetTextAlignment(halign);
            // 设置阅读进行方向
            textformat->SetReadingDirection(readingdirection);
            // 设置自动换行
            textformat->SetWordWrapping(wordwrapping);
        }
        return textformat;
    }
}
#endif
