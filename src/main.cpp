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
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: End. If you saw this message, check 'KnownIssues.md' please \r\n\r\n");
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


// longui namespace
namespace LongUI {
    // primes list
    const uint32_t EzContainer::PRIMES_LIST[14] = {
        19, 79, 149, 263, 457, 787, 1031, 2333,
        5167, 11369, 24989, 32491, 42257, 54941,
    };
    // Base64 DataChar: Map 0~63 to visible char
    const char Base64Chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // Base64 DataChar: Map visible char to 0~63
    const uint8_t Base64Datas[128] = {
        // [  0, 16)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 16, 32)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 32, 48)                            43 44 45 46 47
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0,62, 0, 0, 0,64,
        // [ 48, 64)
        52,53,54,55, 56,57,58,59,      60,61, 0, 0, 0, 0, 0, 0,
        // [ 64, 80)
        0, 0, 1, 2,   3, 4, 5, 6,       7, 8, 9,10,11,12,13,14,
        // [ 80, 96)
        15,16,17,18, 19,20,21,22,      23,24,25, 0, 0, 0, 0, 0,
        // [ 96,112)
        0,26,27,28,  29,30,31,32,      33,34,35,36,37,38,39,40,
        // [112,128)
        41,42,43,44, 45,46,47,48,      49,50,51, 0, 0, 0, 0, 0,
    };
    // IUIScript: {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    const GUID IID_IUIScript = { 
        0x9b531bd, 0x2e3b, 0x4c98, { 0x98, 0x5c, 0x1f, 0xd6, 0xb4, 0x6, 0xe5, 0x3d }
    };
    // IUIResourceLoader: {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    const GUID IID_IUIResourceLoader = { 
        0x16222e4b, 0x9ac8, 0x4756,{ 0x8c, 0xa9, 0x75, 0xa7, 0x2d, 0x2f, 0x4f, 0x60 } 
    };
    // IMFMediaEngineClassFactor: uuid
    const GUID IID_IMFMediaEngineClassFactory = { 
        0x4D645ACE, 0x26AA, 0x4688,{ 0x9B, 0xE1, 0xDF, 0x35, 0x16, 0x99, 0x0B, 0x93 } 
    };
    // IMFMediaEngine: "98a1b0bb-03eb-4935-ae7c-93c1fa0e1c93"
    const GUID IID_IMFMediaEngine = {
        0x98A1B0BB, 0x03EB, 0x4935,{ 0xAE, 0x7C, 0x93, 0xC1, 0xFA, 0x0E, 0x1C, 0x93 } 
    };
    // IMFMediaEngineEx "83015ead-b1e6-40d0-a98a-37145ffe1ad1"
    const GUID IID_IMFMediaEngineEx = {
        0x83015EAD, 0xB1E6, 0x40D0,{ 0xA9, 0x8A, 0x37, 0x14, 0x5F, 0xFE, 0x1A, 0xD1 } 
    };
    // IMFMediaEngineNotify "fee7c112-e776-42b5-9bbf-0048524e2bd5"
    const GUID IID_IMFMediaEngineNotify = {
        0xfee7c112, 0xe776, 0x42b5,{ 0x9B, 0xBF, 0x00, 0x48, 0x52, 0x4E, 0x2B, 0xD5 } 
    };
    // IDCompositionDevice "C37EA93A-E7AA-450D-B16F-9746CB0407F3"
    const GUID IID_IDCompositionDevice = {
        0xC37EA93A, 0xE7AA, 0x450D,{ 0xB1, 0x6F, 0x97, 0x46, 0xCB, 0x04, 0x07, 0xF3 } 
    };
    // IDWriteTextRenderer
    const GUID IID_IDWriteTextRenderer = {
        0xef8a8135, 0x5cc6, 0x45fe,{ 0x88, 0x25, 0xc5, 0xa0, 0x72, 0x4e, 0xb8, 0x19 } 
    };
    // IID_IDWriteInlineObject 
    const GUID IID_IDWriteInlineObject = {
        0x8339FDE3, 0x106F, 0x47ab,{ 0x83, 0x73, 0x1C, 0x62, 0x95, 0xEB, 0x10, 0xB3 } 
    };
    // IDWriteFactory1 ("30572f99-dac6-41db-a16e-0486307e606a")
    const GUID IID_IDWriteFactory1 = {
        0x30572f99, 0xdac6, 0x41db,{ 0xa1, 0x6e, 0x04, 0x86, 0x30, 0x7e, 0x60, 0x6a } 
    };
    // IDWriteFontFileEnumerator("72755049-5ff7-435d-8348-4be97cfa6c7c") 
    const GUID IID_IDWriteFontFileEnumerator = {
        0x72755049, 0x5ff7, 0x435d,{ 0x83, 0x48, 0x4b, 0xe9, 0x7c, 0xfa, 0x6c, 0x7c }
    };
    // IDWriteFontCollectionLoader("cca920e4-52f0-492b-bfa8-29c72ee0a468") 
    const GUID IID_IDWriteFontCollectionLoader = {
        0xcca920e4, 0x52f0, 0x492b,{ 0xbf, 0xa8, 0x29, 0xc7, 0x2e, 0xe0, 0xa4, 0x68 }
    };
    // ITextHost2 ("13E670F5-1A5A-11CF-ABEB-00AA00B65EA1")
    const GUID IID_ITextHost2 = {
        0x13E670F5, 0x1A5A, 0x11CF,{ 0xAB, 0xEB, 0x00, 0xAA, 0x00, 0xB6, 0x5E, 0xA1 }
    };
    // CUIBasicTextRenderer {EDAB1E53-C1CF-4F5A-9533-9946904AD63C}
    const GUID IID_CUIBasicTextRenderer = {
        0xedab1e53, 0xc1cf, 0x4f5a,{ 0x95, 0x33, 0x99, 0x46, 0x90, 0x4a, 0xd6, 0x3c }
    };
    // IID_IDXGISwapChain2 0xa8be2ac4, 0x199f, 0x4946, 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7
    const GUID IID_IDXGISwapChain2 = {
        0xa8be2ac4, 0x199f, 0x4946,{ 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7 }
    };
}


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
        LongUI::LoadProc(LongUI::UIRichEdit::IID_ITextServices2, m_hDllMsftedit, "IID_ITextServices2");
        LongUI::LoadProc(LongUI::UIRichEdit::CreateTextServices, m_hDllMsftedit, "CreateTextServices");
        LongUI::LoadProc(LongUI::UIRichEdit::ShutdownTextServices, m_hDllMsftedit, "ShutdownTextServices");
        LongUI::LoadProc(LongUI::Dll::DCompositionCreateDevice, m_hDlldcomp, "DCompositionCreateDevice");
        LongUI::LoadProc(LongUI::Dll::D2D1CreateFactory, m_hDlld2d1, "D2D1CreateFactory");
        LongUI::LoadProc(LongUI::Dll::D3D11CreateDevice, m_hDlld3d11, "D3D11CreateDevice");
        LongUI::LoadProc(LongUI::Dll::DWriteCreateFactory, m_hDlldwrite, "DWriteCreateFactory");
        LongUI::LoadProc(LongUI::Dll::CreateDXGIFactory1, m_hDlldxgi, "CreateDXGIFactory1");
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

// load libraries
#if defined(_MSC_VER)
#pragma comment(lib, "winmm")
#pragma comment(lib, "dxguid")
#endif

