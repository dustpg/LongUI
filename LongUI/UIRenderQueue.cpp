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
    if (m_dwDisplayFrequency == freq) return;
    // 修改
    m_dwDisplayFrequency = freq;
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
            UIManager << DL_Hint << "Time Deviation: "
                << long(time) - long(LongUIPlanRenderingTotalTime * 1000)
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
        auto frame_offset = long(wait * float(m_dwDisplayFrequency));
        auto frame_count = long(render * float(m_dwDisplayFrequency)) + 1;
        auto start = m_pCurrentUnit + frame_offset;
        for (long i = 0; i < frame_count; ++i) {
            if (start >= m_pUnitsDataEnd) {
                start -= LongUIPlanRenderingTotalTime * m_dwDisplayFrequency;
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
        auto GetResourceCount(ResourceType type) noexcept->size_t override;
        // get resouce by index, index in range [0, count)
        // for Type_Bitmap, Type_Brush, Type_TextFormat
        auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* override;
        // get resouce by index, index in range [0, count)
        // Type_Meta
        auto GetResourceStruct(ResourceType type, size_t index, void* data) noexcept ->void override;
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
        // xml doc for resource
        pugi::xml_document      m_docRecource;
        // counter for this
        uint32_t                m_dwCounter = 1;
    };
    // create resource loader
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) noexcept -> IUIResourceLoader* {
        return new(std::nothrow) CUIResourceLoaderXML(manager, xml);
    }
    // ctor for CUIResourceLoaderXML
    LongUI::CUIResourceLoaderXML::CUIResourceLoaderXML(CUIManager& manager, const char* xml) 
        noexcept : m_manager(manager) {
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
    auto LongUI::CUIResourceLoaderXML::GetResourceCount(ResourceType type) noexcept -> size_t {
        return 0;
    }
    // get reource
    auto LongUI::CUIResourceLoaderXML::GetResourcePointer(ResourceType type, size_t index) noexcept -> void * {
        return nullptr;
    }
    // get reource
    auto LongUI::CUIResourceLoaderXML::GetResourceStruct(ResourceType type, size_t index, void * data) noexcept -> void {
    
    }
}
#endif