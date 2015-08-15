#include "LongUI.h"

// Memory leak detector
#if defined(_DEBUG) && defined(_MSC_VER)
ID3D11Debug*    g_pd3dDebug_longui = nullptr;
struct MemoryLeakDetector {
    // ctor
    MemoryLeakDetector() {
        ::_CrtMemCheckpoint(memstate + 0);
        constexpr int sa = sizeof(_CrtMemState);
    }
    // dtor
    ~MemoryLeakDetector() {
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
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: End\r\n\r\n");
            }
            g_pd3dDebug_longui = nullptr;
        }
    }
    // mem state
    _CrtMemState memstate[3];
} g_detector;
#endif




static_assert(sizeof(std::atomic_bool) == sizeof(char), "really bad");
#define InitStaticVar(v)  decltype(v) v = nullptr
// 初始化静态变量

InitStaticVar(LongUI::UIRichEdit::IID_ITextServices2);
InitStaticVar(LongUI::UIRichEdit::CreateTextServices);
InitStaticVar(LongUI::UIRichEdit::ShutdownTextServices);
InitStaticVar(LongUI::Dll::D3D11CreateDevice);
InitStaticVar(LongUI::Dll::D2D1CreateFactory);
InitStaticVar(LongUI::Dll::DCompositionCreateDevice);
InitStaticVar(LongUI::Dll::DWriteCreateFactory);
InitStaticVar(LongUI::Dll::CreateDXGIFactory1);


/*// 复制构造
LongUI::EventArgument::EventArgument(const EventArgument& arg) noexcept {
    this->event = arg.event;
    this->sender = arg.sender;
    this->sys.wParam = arg.sys.wParam;
    this->sys.lParam = arg.sys.lParam;
    this->pt.x = arg.pt.x;
    this->pt.y = arg.pt.y;
    this->lr = arg.lr;
}*/


#define LoadFunction(a, b, c) { if(c) a = reinterpret_cast<decltype(a)>(::GetProcAddress(c, #b)); }

// 初始化库
class InitializeLibrary {
    typedef enum PROCESS_DPI_AWARENESS {
        PROCESS_DPI_UNAWARE = 0,
        PROCESS_SYSTEM_DPI_AWARE = 1,
        PROCESS_PER_MONITOR_DPI_AWARE = 2
    } PROCESS_DPI_AWARENESS;
    // SetProcessDpiAwareness
    static HRESULT STDAPICALLTYPE SetProcessDpiAwarenessF(PROCESS_DPI_AWARENESS);
public:
    //
    InitializeLibrary() {
        // < Win8 ?
        if (!LongUI::IsWindows8OrGreater()) {
            ::MessageBoxA(nullptr, "Windows8 at least!", "Unsupported System", MB_ICONERROR);
            ::ExitProcess(1);
            return;
        }
        // >= Win8.1 ?
        if (LongUI::IsWindows8Point1OrGreater()) {
            m_hDllShcore = ::LoadLibraryW(L"Shcore.dll");
            assert(m_hDllShcore);
            if (m_hDllShcore) {
                auto setProcessDpiAwareness =
                    reinterpret_cast<decltype(&InitializeLibrary::SetProcessDpiAwarenessF)>(
                        ::GetProcAddress(m_hDllShcore, "SetProcessDpiAwareness")
                        );
                assert(setProcessDpiAwareness);
                if (setProcessDpiAwareness) {
                    setProcessDpiAwareness(InitializeLibrary::PROCESS_PER_MONITOR_DPI_AWARE);
                }
            }
        }
        LoadFunction(LongUI::UIRichEdit::IID_ITextServices2, IID_ITextServices2, m_hDllMsftedit);
        LoadFunction(LongUI::UIRichEdit::CreateTextServices, CreateTextServices, m_hDllMsftedit);
        LoadFunction(LongUI::UIRichEdit::ShutdownTextServices, ShutdownTextServices, m_hDllMsftedit);
        LoadFunction(LongUI::Dll::DCompositionCreateDevice, DCompositionCreateDevice, m_hDlldcomp);
        LoadFunction(LongUI::Dll::D2D1CreateFactory, D2D1CreateFactory, m_hDlld2d1);
        LoadFunction(LongUI::Dll::D3D11CreateDevice, D3D11CreateDevice, m_hDlld3d11);
        LoadFunction(LongUI::Dll::DWriteCreateFactory, DWriteCreateFactory, m_hDlldwrite);
        LoadFunction(LongUI::Dll::CreateDXGIFactory1, CreateDXGIFactory1, m_hDlldxgi);
    };
    //
    ~InitializeLibrary() {
        if (m_hDllMsftedit) {
            ::FreeLibrary(m_hDllMsftedit);
            m_hDllMsftedit = nullptr;
        }
        if (m_hDlldcomp) {
            ::FreeLibrary(m_hDlldcomp);
            m_hDlldcomp = nullptr;
        }
        if (m_hDlld2d1) {
            ::FreeLibrary(m_hDlld2d1);
            m_hDlld2d1 = nullptr;
        }
        if (m_hDlld3d11) {
            ::FreeLibrary(m_hDlld3d11);
            m_hDlld3d11 = nullptr;
        }
        if (m_hDlldwrite) {
            ::FreeLibrary(m_hDlldwrite);
            m_hDlldwrite = nullptr;
        }
        if (m_hDlldxgi) {
            ::FreeLibrary(m_hDlldxgi);
            m_hDlldxgi = nullptr;
        }
        if (m_hDllShcore) {
            ::FreeLibrary(m_hDllShcore);
            m_hDllShcore = nullptr;
        }
    }
private:
    // Msftedit
    HMODULE     m_hDllMsftedit = ::LoadLibraryW(L"Msftedit.dll");
    // dcomp
    HMODULE     m_hDlldcomp = ::LoadLibraryW(L"dcomp.dll");
    // d2d1
    HMODULE     m_hDlld2d1 = ::LoadLibraryW(L"d2d1.dll");
    // d3d11
    HMODULE     m_hDlld3d11 = ::LoadLibraryW(L"d3d11.dll");
    // dwrite
    HMODULE     m_hDlldwrite = ::LoadLibraryW(L"dwrite.dll");
    // dxgi
    HMODULE     m_hDlldxgi = ::LoadLibraryW(L"dxgi.dll");
    // Shcore
    HMODULE     m_hDllShcore = nullptr;
} instance;


// 初始化静态变量
LongUI::CUIManager          LongUI::CUIManager::s_instance;


#pragma comment(lib, "winmm")
#pragma comment(lib, "dxguid")




// ------------------------------------------------------------
// 布局上下文
class CUILayoutContext {
    // layout type
    enum LayoutType : uint32_t {
        Type_Fixed,    //   Fixed in one direction
    };
public:
    // ctor
    CUILayoutContext() noexcept = default;
    // dtor
    ~CUILayoutContext() noexcept = default;
public:
    // type of layout
    LayoutType  type = Type_Fixed;
    // value of layout
    float       value = 0.f;
};


