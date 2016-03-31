#include <Core/luiManager.h>
#include <Graphics/luiGrUtil.h>
#include <Platless/luiPlEzC.h>
#include <LongUI/luiUiHlper.h>

static_assert(sizeof(bool) == sizeof(char), "really bad, unsupported!");

#define InitStaticVar(v)  decltype(v) v = nullptr
// 初始化静态变量

#if 0
InitStaticVar(LongUI::UIRichEdit::IID_ITextServices2);
InitStaticVar(LongUI::UIRichEdit::CreateTextServices);
InitStaticVar(LongUI::UIRichEdit::ShutdownTextServices);
#endif
InitStaticVar(LongUI::Dll::D2D1CreateFactory);
InitStaticVar(LongUI::Dll::DCompositionCreateDevice);
InitStaticVar(LongUI::Dll::D2D1InvertMatrix);
InitStaticVar(LongUI::Dll::D2D1MakeSkewMatrix);
InitStaticVar(LongUI::Dll::D2D1IsMatrixInvertible);
InitStaticVar(LongUI::Dll::CreateDXGIFactory1);


// longui namespace
namespace LongUI {
    // primes list
    const uint32_t EzContainer::PRIMES_LIST[14] = {
        19, 79, 149, 263, 457, 787, 1031, 2333,
        5167, 11369, 24989, 32491, 42257, 54941,
    };
    // IUIScript: {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    const GUID IID_IUIScript = {
        0x09b531bd, 0x2e3b, 0x4c98, { 0x98, 0x5c, 0x1f, 0xd6, 0xb4, 0x6, 0xe5, 0x3d }
    };
    // IUIResourceLoader: {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    const GUID IID_IUIResourceLoader = {
        0x16222e4b, 0x9ac8, 0x4756, { 0x8c, 0xa9, 0x75, 0xa7, 0x2d, 0x2f, 0x4f, 0x60 }
    };
    // IMFMediaEngineClassFactor: uuid
    const GUID IID_IMFMediaEngineClassFactory = {
        0x4D645ACE, 0x26AA, 0x4688, { 0x9B, 0xE1, 0xDF, 0x35, 0x16, 0x99, 0x0B, 0x93 }
    };
    // IMFMediaEngine: "98a1b0bb-03eb-4935-ae7c-93c1fa0e1c93"
    const GUID IID_IMFMediaEngine = {
        0x98A1B0BB, 0x03EB, 0x4935, { 0xAE, 0x7C, 0x93, 0xC1, 0xFA, 0x0E, 0x1C, 0x93 }
    };
    // IMFMediaEngineEx "83015ead-b1e6-40d0-a98a-37145ffe1ad1"
    const GUID IID_IMFMediaEngineEx = {
        0x83015EAD, 0xB1E6, 0x40D0, { 0xA9, 0x8A, 0x37, 0x14, 0x5F, 0xFE, 0x1A, 0xD1 }
    };
    // IMFMediaEngineNotify "fee7c112-e776-42b5-9bbf-0048524e2bd5"
    const GUID IID_IMFMediaEngineNotify = {
        0xfee7c112, 0xe776, 0x42b5, { 0x9B, 0xBF, 0x00, 0x48, 0x52, 0x4E, 0x2B, 0xD5 }
    };
    // IDCompositionDevice "C37EA93A-E7AA-450D-B16F-9746CB0407F3"
    const GUID IID_IDCompositionDevice = {
        0xC37EA93A, 0xE7AA, 0x450D, { 0xB1, 0x6F, 0x97, 0x46, 0xCB, 0x04, 0x07, 0xF3 }
    };
    // IDWriteTextRenderer
    const GUID IID_IDWriteTextRenderer = {
        0xef8a8135, 0x5cc6, 0x45fe, { 0x88, 0x25, 0xc5, 0xa0, 0x72, 0x4e, 0xb8, 0x19 }
    };
    // IID_IDWriteInlineObject 
    const GUID IID_IDWriteInlineObject = {
        0x8339FDE3, 0x106F, 0x47ab, { 0x83, 0x73, 0x1C, 0x62, 0x95, 0xEB, 0x10, 0xB3 }
    };
    // IDWriteFactory1 ("30572f99-dac6-41db-a16e-0486307e606a")
    const GUID IID_IDWriteFactory1 = {
        0x30572f99, 0xdac6, 0x41db, { 0xa1, 0x6e, 0x04, 0x86, 0x30, 0x7e, 0x60, 0x6a }
    };
    // IID_IDWriteFontCollection(a84cee02-3eea-4eee-a827-87c1a02a0fcc)
    const GUID IID_IDWriteFontCollection = {
        0xa84cee02, 0x3eea, 0x4eee, { 0xa8, 0x27, 0x87, 0xc1, 0xa0, 0x2a, 0x0f, 0xcc }
    };
    // IDWriteFontFileEnumerator("72755049-5ff7-435d-8348-4be97cfa6c7c") 
    const GUID IID_IDWriteFontFileEnumerator = {
        0x72755049, 0x5ff7, 0x435d, { 0x83, 0x48, 0x4b, 0xe9, 0x7c, 0xfa, 0x6c, 0x7c }
    };
    // IDWriteFontCollectionLoader("cca920e4-52f0-492b-bfa8-29c72ee0a468") 
    const GUID IID_IDWriteFontCollectionLoader = {
        0xcca920e4, 0x52f0, 0x492b, { 0xbf, 0xa8, 0x29, 0xc7, 0x2e, 0xe0, 0xa4, 0x68 }
    };
    // ITextHost2 ("13E670F5-1A5A-11CF-ABEB-00AA00B65EA1")
    const GUID IID_ITextHost2 = {
        0x13E670F5, 0x1A5A, 0x11CF, { 0xAB, 0xEB, 0x00, 0xAA, 0x00, 0xB6, 0x5E, 0xA1 }
    };
    // XUIBasicTextRenderer {EDAB1E53-C1CF-4F5A-9533-9946904AD63C}
    const GUID IID_CUIBasicTextRenderer = {
        0xedab1e53, 0xc1cf, 0x4f5a, { 0x95, 0x33, 0x99, 0x46, 0x90, 0x4a, 0xd6, 0x3c }
    };
    // IID_IDXGISwapChain2 0xa8be2ac4, 0x199f, 0x4946, 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7
    const GUID IID_IDXGISwapChain2 = {
        0xa8be2ac4, 0x199f, 0x4946, { 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7 }
    };
}

// longui
namespace LongUI {
    // load proc
    template<typename T> inline auto LoadProc(T& pointer, HMODULE dll, const char* name) noexcept {
        pointer = reinterpret_cast<T>(::GetProcAddress(dll, name));
    }
    // 安全释放
    //inline auto SafeFreeLibrary(HMODULE& dll) noexcept { if (dll) ::FreeLibrary(dll);  dll = nullptr; }
    // 安全释放
    inline auto SafeFreeLibrary(HMODULE dll) noexcept { ::FreeLibrary(dll); }
    // 初始化库
    class InitializeLibrary {
        typedef enum PROCESS_DPI_AWARENESS {
            PROCESS_DPI_UNAWARE = 0,
            PROCESS_SYSTEM_DPI_AWARE = 1,
            PROCESS_PER_MONITOR_DPI_AWARE = 2
        } PROCESS_DPI_AWARENESS;
        typedef enum MONITOR_DPI_TYPE {
            MDT_EFFECTIVE_DPI = 0,
            MDT_ANGULAR_DPI = 1,
            MDT_RAW_DPI = 2,
            MDT_DEFAULT = MDT_EFFECTIVE_DPI
        } MONITOR_DPI_TYPE;
        // SetProcessDpiAwareness
        using SetProcessDpiAwarenessT = HRESULT(STDAPICALLTYPE*) (PROCESS_DPI_AWARENESS);
        // SetProcessDpiAwareness
        using GetDpiForMonitorT = HRESULT(STDAPICALLTYPE*) (HMONITOR, MONITOR_DPI_TYPE, UINT *, UINT *);
    public:
        // ctor
        InitializeLibrary() noexcept {
            this->load_shcore();
#if 0
            LongUI::LoadProc(LongUI::UIRichEdit::IID_ITextServices2, m_hDllMsftedit, "IID_ITextServices2");
            LongUI::LoadProc(LongUI::UIRichEdit::CreateTextServices, m_hDllMsftedit, "CreateTextServices");
            LongUI::LoadProc(LongUI::UIRichEdit::ShutdownTextServices, m_hDllMsftedit, "ShutdownTextServices");
#endif
            LongUI::LoadProc(LongUI::Dll::DCompositionCreateDevice, m_hDlldcomp, "DCompositionCreateDevice");
            LongUI::LoadProc(LongUI::Dll::D2D1CreateFactory, m_hDlld2d1, "D2D1CreateFactory");
            LongUI::LoadProc(LongUI::Dll::D2D1MakeSkewMatrix, m_hDlld2d1, "D2D1MakeSkewMatrix");
            LongUI::LoadProc(LongUI::Dll::D2D1InvertMatrix, m_hDlld2d1, "D2D1InvertMatrix");
            LongUI::LoadProc(LongUI::Dll::D2D1IsMatrixInvertible, m_hDlld2d1, "D2D1IsMatrixInvertible");
            LongUI::LoadProc(LongUI::Dll::CreateDXGIFactory1, m_hDlldxgi, "CreateDXGIFactory1");
        };
        //
        ~InitializeLibrary() noexcept {
            LongUI::SafeFreeLibrary(m_hDllMsftedit);
            LongUI::SafeFreeLibrary(m_hDlldcomp);
            LongUI::SafeFreeLibrary(m_hDlld2d1);
            LongUI::SafeFreeLibrary(m_hDlld3d11);
            LongUI::SafeFreeLibrary(m_hDlldwrite);
            LongUI::SafeFreeLibrary(m_hDlldxgi);
            LongUI::SafeFreeLibrary(m_hDllShcore);
        }
    private:
        // load for Shcore
        void load_shcore() noexcept {
            m_hDllShcore = ::LoadLibraryW(L"Shcore.dll");
            if (!m_hDllShcore) return;
            auto func = ::GetProcAddress(m_hDllShcore, "SetProcessDpiAwareness");
            // 调用
            if (func) {
                auto set_process_dpi_awareness = reinterpret_cast<SetProcessDpiAwarenessT>(func);
                auto hr = set_process_dpi_awareness(InitializeLibrary::PROCESS_PER_MONITOR_DPI_AWARE);
                hr = S_OK;
            }
        }
    private:
        // Msftedit
        HMODULE     m_hDllMsftedit  = ::LoadLibraryW(L"Msftedit.dll");
        // dcomp
        HMODULE     m_hDlldcomp     = ::LoadLibraryW(L"dcomp.dll");
        // d2d1
        HMODULE     m_hDlld2d1      = ::LoadLibraryW(L"d2d1.dll");
        // d3d11
        HMODULE     m_hDlld3d11     = ::LoadLibraryW(L"d3d11.dll");
        // dwrite
        HMODULE     m_hDlldwrite    = ::LoadLibraryW(L"dwrite.dll");
        // dxgi
        HMODULE     m_hDlldxgi      = ::LoadLibraryW(L"dxgi.dll");
        // Shcore
        HMODULE     m_hDllShcore    = nullptr;
    public:
        // safe GetDpiForMonitor
        bool GetDpiForMonitor_s(HMONITOR m, uint32_t& xdpi, uint32_t& ydpi) {
            // 没有dll -> 挫
            if (!m_hDllShcore) return false;
            // 获取函数地址
            auto func = ::GetProcAddress(m_hDllShcore, "GetDpiForMonitor");
            // 没有函数 -> 挫
            if (!func) return false;
            // 调用
            auto get_dpi_for_monitor = reinterpret_cast<GetDpiForMonitorT>(func);
            auto hr = get_dpi_for_monitor(m, MDT_DEFAULT, &xdpi, &ydpi);
            return SUCCEEDED(hr);
        }
    } instance;
    // get monitor dpi
    void GetMonitorDpi(HMONITOR m, uint32_t& xdpi, uint32_t& ydpi) noexcept {
        if (!instance.GetDpiForMonitor_s(m, xdpi, ydpi)) {
            xdpi = 96;
            ydpi = 96;
        }
    }
}



// Memory leak detector
#if defined(_DEBUG) && defined(_MSC_VER)
ID3D11Debug*    g_pd3dDebug_longui = nullptr;
struct LongUIMemoryLeakDetector {
    // ctor
    LongUIMemoryLeakDetector() {
        ::_CrtMemCheckpoint(memstate + 0);
        constexpr int sa = sizeof(_CrtMemState);
    }
    // dtor
    ~LongUIMemoryLeakDetector() {
        ::_CrtMemCheckpoint(memstate + 1);
        if (::_CrtMemDifference(memstate + 2, memstate + 0, memstate + 1)) {
            ::_CrtDumpMemoryLeaks();
            assert(!"OOps! Memory leak detected");
        }
        if (g_pd3dDebug_longui) {
            auto count = g_pd3dDebug_longui->Release();
            if (count) {
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL)\r\n\r\n");
                g_pd3dDebug_longui->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS(1 | 2 | 4));
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL)\r\n\r\n");
                g_pd3dDebug_longui->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS(4));
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: End. If you saw this message, check 'KnownIssues.md' please \r\n\r\n");
            }
            g_pd3dDebug_longui = nullptr;
        }
    }
    // mem state
    _CrtMemState memstate[3];
} g_detector;
#endif

// 初始化静态变量
LongUI::CUIManager          LongUI::CUIManager::s_instance;

// load libraries
#if defined(_MSC_VER)
#pragma comment(lib, "winmm")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "D3D11")
#pragma comment(lib, "Mfplat")
#endif
