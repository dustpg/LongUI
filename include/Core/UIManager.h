#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/


// LongUI namespace
namespace LongUI {
    // endl for longUI
    static struct EndL { } endl;
    // ui manager ui 管理器
    class LongUIAlignas CUIManager {
        // create ui window call back
        using callback_for_creating_window = auto(*)(pugi::xml_node node, UIWindow* parent, void* buffer)->UIWindow*;
    public: 
        // Windows Version
        enum WindowsVersion : uint32_t {
            // win7 sp1
            Version_Win7SP1 = 0,
            // Win8,
            Version_Win8, 
            //  Win8.1
            Version_Win8_1,
            // Win10
            Version_Win10
        };
    public: // handle zone 操作区
        // initialize 初始化
        auto Initialize(IUIConfigure* = nullptr) noexcept->HRESULT;
        // uninitialize 反初始化
        void UnInitialize() noexcept;
        // run 运行
        void Run() noexcept;
        // add "string to create funtion" map 添加函数映射关系
        auto RegisterControl(CreateControlFunction, const wchar_t*) noexcept->HRESULT;
        // ShowError with HRESULT code
        void ShowError(HRESULT, const wchar_t* str_b = nullptr) noexcept;
        // wait for VS
        auto WaitVS(HANDLE events[], uint32_t length) noexcept ->void;
        // add window
        void RegisterWindow(UIWindow* wnd) noexcept;
        // remove window
        void RemoveWindow(UIWindow* wnd, bool cleanup = false) noexcept;
        // register, return -1 for error(out of renderer space), return other for index
        auto RegisterTextRenderer(CUIBasicTextRenderer*, const char name[LongUITextRendererNameMaxLength]) noexcept->int32_t;
        // get text renderer by name 
        auto GetTextRenderer(const char* name) const noexcept ->CUIBasicTextRenderer*;
        // get text format, "Get" method will call IUnknown::AddRef if it is a COM object
        auto GetTextFormat(size_t index) noexcept->IDWriteTextFormat*;
        // get bitmap by index, "Get" method will call IUnknown::AddRef if it is a COM object
        auto GetBitmap(size_t index) noexcept->ID2D1Bitmap1*;
        // get brush by index, "Get" method will call IUnknown::AddRef if it is a COM object
        auto GetBrush(size_t index) noexcept->ID2D1Brush*;
        // get meta by index, "Get" method will call IUnknown::AddRef if it is a COM object
        // Meta isn't a IUnknown object, so, won't call Meta::bitmap->AddRef
        void GetMeta(size_t index, LongUI::Meta&) noexcept;
        // get meta's icon handle by index, will do runtime-converting if first call the
        // same index. "Get" method will call IUnknown::AddRef if it is a COM object
        // HICON isn't a IUnknown object. Meta HICON managed by this manager
        auto GetMetaHICON(size_t index) noexcept->HICON;
        // get system brush
        auto GetSystemBrush(uint32_t index) noexcept { return ::SafeAcquire(m_apSystemBrushes[index]); }
        // get drop target helper
        auto GetDropTargetHelper() noexcept { return ::SafeAcquire(m_pDropTargetHelper); }
        // get create function width const char*
        auto GetCreateFunc(const char*) noexcept->CreateControlFunction;
        // get create function width CUIString
        auto GetCreateFunc(const CUIString&)noexcept->CreateControlFunction;
        // get create function width const wchar_t* and length
        auto GetCreateFunc(const wchar_t* class_name, uint32_t len = 0) noexcept { CUIString name(class_name, len); return GetCreateFunc(name); }
        // create control with xml node, node and function cannot be null in same time
        auto CreateControl(pugi::xml_node node, CreateControlFunction function) noexcept {
            assert((node || function) && "cannot be null in same time");
            return this->create_control(function, node, 0);
        }
        // create control with template id, template and function cannot be null in same time
        auto CreateControl(size_t templateid, CreateControlFunction function) noexcept {
            assert((templateid || function) && "cannot be null in same time");
            return this->create_control(function, LongUINullXMLNode, templateid);
        }
    public: // Create UI Window Zone!!!!!!!!!
        // create ui window with xml string
        auto CreateUIWindow(const char* xml, UIWindow* parent = nullptr) noexcept { return this->CreateUIWindow<LongUI::UIWindow>(xml, parent); }
        // create ui window with custom window && xml string
        template<class T>
        auto CreateUIWindow(const char* xml, UIWindow* parent = nullptr) noexcept->UIWindow* {
            auto code = m_docWindow.load_string(xml); assert(code); if (code.status) return nullptr;
            auto create_func = [](pugi::xml_node node, UIWindow* parent, void*) noexcept ->UIWindow* {
                return LongUI::UIControl::AllocRealControl<T>(node, [=](void* p) noexcept { new(p) T(node, parent); });
            };
            return this->create_ui_window(m_docWindow.first_child(), parent, create_func, nullptr);
        }
        // create ui window with custom window && xml && buffer
        template<class T>
        auto CreateUIWindow(const char* xml, UIWindow* parent, void* buffer) noexcept->UIWindow* {
            auto code = m_docWindow.load_string(xml); assert(code); if (code.status) return nullptr;
            auto create_func = [](pugi::xml_node node, UIWindow* parent, void* buffer) noexcept ->UIWindow* {
                return new(buffer) T(node, parent);
            };
            return this->create_ui_window(m_docWindow.first_child(), parent, create_func, buffer);
        }
    public:
        // create text format
        auto CreateTextFormat(const wchar_t* fn, DWRITE_FONT_WEIGHT fw, DWRITE_FONT_STYLE sy, DWRITE_FONT_STRETCH st, FLOAT fs, IDWriteTextFormat** tf) noexcept {
            return m_pDWriteFactory->CreateTextFormat(fn, m_pFontCollection, fw, sy, st, fs, m_szLocaleName, tf);
        }
    public:
        // get theme colr
        static auto __fastcall GetThemeColor(D2D1_COLOR_F& colorf) noexcept->HRESULT;
        // is run as admin?
        static bool WINAPI IsRunAsAdministrator() noexcept;
        // try to elevate now,  will lauch a new elevated instance and
        // exit this instance if success. be careful about your app if
        // only can be in one instance
        static bool WINAPI TryElevateUACNow(const wchar_t* parameters = nullptr, bool exit = true) noexcept;
    public: // inline 区
        // lock
        inline auto Lock() noexcept { return m_uiLocker.Lock(); }
        // unlock
        inline auto Unlock() noexcept { return m_uiLocker.Unlock(); }
        // ShowError with string
        inline auto ShowError(const wchar_t * str, const wchar_t* str_b = nullptr) noexcept { this->configure->ShowError(str, str_b); }
        // GetXXX method will call AddRef if it is a COM object
        inline auto GetTextRenderer(int i) const noexcept { assert(i < m_uTextRenderCount && "out of range"); return ::SafeAcquire(m_apTextRenderer[i]); }
        // exit the app
        inline auto Exit() noexcept { m_exitFlag = true; ::PostQuitMessage(0); }
        // recreate resources
        inline auto RecreateResources() noexcept { this->discard_resources(); return this->create_device_resources(); }
        // get delta time for ui
        inline auto GetDeltaTime() const noexcept { return m_fDeltaTime; }
        // get count of window
        inline auto GetWindowsCount() const noexcept { return m_cCountWindow; }
    public: // 隐形转换区
        // 转换为 LongUIRenderTarget
#define UIManager_RenderTarget (static_cast<ID2D1DeviceContext*>(UIManager))
        LongUIInline operator ID2D1DeviceContext*()const noexcept { return m_pd2dDeviceContext; };
        // 转换为 DXGI Factory2
#define UIManager_DXGIFactory (static_cast<IDXGIFactory2*>(UIManager))
        LongUIInline operator IDXGIFactory2*()const noexcept { return m_pDxgiFactory; };
        // 转换为 D3D11 Device
#define UIManager_D3DDevice  (static_cast<ID3D11Device*>(UIManager))
        LongUIInline operator ID3D11Device*()const noexcept { return m_pd3dDevice; };
        // 转换为 D3D11 Device Context
#define UIManager_D3DContext (static_cast<ID3D11DeviceContext*>(UIManager))
        LongUIInline operator ID3D11DeviceContext*()const noexcept { return m_pd3dDeviceContext; };
        // 转换为 D2D1 Device
#define UIManager_D2DDevice  (static_cast<ID2D1Device*>(UIManager))
        LongUIInline operator ID2D1Device*()const noexcept { return m_pd2dDevice; };
        // 转换为 DXGI Device1
#define UIManager_DXGIDevice (static_cast<IDXGIDevice1*>(UIManager))
        LongUIInline operator IDXGIDevice1*()const noexcept { return m_pDxgiDevice; };
        // 转换为 DXGI Adapter
#define UIManager_DXGIAdapter (static_cast<IDXGIAdapter*>(UIManager))
        LongUIInline operator IDXGIAdapter*()const noexcept { return m_pDxgiAdapter; };
        // 转换为 DWrite Factory1
#define UIManager_DWriteFactory (static_cast<IDWriteFactory1*>(UIManager))
        LongUIInline operator IDWriteFactory1*()const noexcept { return m_pDWriteFactory; };
        // 转换为 D2D Factory1
#define UIManager_D2DFactory (static_cast<ID2D1Factory1*>(UIManager))
        LongUIInline operator ID2D1Factory1*()const noexcept { return m_pd2dFactory; };
#ifdef LONGUI_VIDEO_IN_MF
        // 转换为  IMFDXGIDeviceManager
#   define UIManager_MFDXGIDeviceManager (static_cast<IMFDXGIDeviceManager*>(UIManager))
        LongUIInline operator IMFDXGIDeviceManager*()const noexcept { return m_pMFDXGIManager; };
        // 转换为  IMFMediaEngineClassFactory
#   define UIManager_MFMediaEngineClassFactory (static_cast<IMFMediaEngineClassFactory*>(UIManager))
        LongUIInline operator IMFMediaEngineClassFactory*()const noexcept { return m_pMediaEngineFactory; };
        // MF Dxgi设备管理器
        IMFDXGIDeviceManager*           m_pMFDXGIManager = nullptr;
        // MF 媒体引擎
        IMFMediaEngineClassFactory*     m_pMediaEngineFactory = nullptr;
#endif
        // 转换为 CUIInput
        LongUIInline operator const CUIInput&() const noexcept { return m_uiInput; };
#define UIInput (static_cast<const CUIInput&>(UIManager))
    public:
        // script 脚本
        IUIScript*           const      script = nullptr;
        // config
        IUIConfigure*        const      configure = nullptr;
        // windows version
        WindowsVersion       const      version = WindowsVersion::Version_Win8;
        // flag for configure
        IUIConfigure::ConfigureFlag     flag = IUIConfigure::Flag_None;
    private:
        // helper for drop target
        IDropTargetHelper*              m_pDropTargetHelper = nullptr;
        // D2D 工厂
        ID2D1Factory1*                  m_pd2dFactory = nullptr;
        // DWrite工厂
        IDWriteFactory1*                m_pDWriteFactory = nullptr;
        // DWrite 字体集
        IDWriteFontCollection*          m_pFontCollection = nullptr;
        // D3D 设备
        ID3D11Device*                   m_pd3dDevice = nullptr;
        // D3D 设备上下文
        ID3D11DeviceContext*            m_pd3dDeviceContext = nullptr;
        // D2D 设备
        ID2D1Device*                    m_pd2dDevice = nullptr;
        // D2D 设备上下文
        ID2D1DeviceContext*             m_pd2dDeviceContext = nullptr;
        // DXGI 工厂
        IDXGIFactory2*                  m_pDxgiFactory = nullptr;
        // DXGI 设备
        IDXGIDevice1*                   m_pDxgiDevice = nullptr;
        // DXGI 适配器
        IDXGIAdapter*                   m_pDxgiAdapter = nullptr;
#ifdef _DEBUG
        // debug object
        ID3D11Debug*                    m_pd3dDebug = nullptr;
#endif
        // text renderer
        CUIBasicTextRenderer*           m_apTextRenderer[LongUITextRendererCountMax];
        // system brush
        ID2D1Brush*                     m_apSystemBrushes[STATUS_COUNT];
        // loader
        IUIResourceLoader*              m_pResourceLoader = nullptr;
        // default bitmap buffer
        uint8_t*                        m_pBitmap0Buffer = nullptr;
        // map: string<->func
        StringMap                       m_mapString2CreateFunction;
        // feature level
        D3D_FEATURE_LEVEL               m_featureLevel;
        // input
        CUIInput                        m_uiInput;
        // locker
        CUILocker                       m_uiLocker;
        // timer
        CUITimer                        m_uiTimer;
        // bitmap buffer
        ID2D1Bitmap1**                  m_ppBitmaps = nullptr;
        // brush buffer
        ID2D1Brush**                    m_ppBrushes = nullptr;
        // text format buffer
        IDWriteTextFormat**             m_ppTextFormats = nullptr;
        // meta buffer
        Meta*                           m_pMetasBuffer = nullptr;
        // meta hicon buffer
        HICON*                          m_phMetaIcon = nullptr;
        // template node
        pugi::xml_node*                 m_pTemplateNodes = nullptr;
        // resource buffer for all
        void*                           m_pResourceBuffer = nullptr;
        // length of bitmap*
        uint16_t                        m_cCountBmp = 0;
        // length of brush*
        uint16_t                        m_cCountBrs = 0;
        // length of textformat*
        uint16_t                        m_cCountTf = 0;
        // length of meta
        uint16_t                        m_cCountMt = 0;
        // length of template node
        uint16_t                        m_cCountCtrlTemplate = 0;
        // length of window*
        uint16_t                        m_cCountWindow = 0;
        // singal/flag for exiting
        std::atomic_bool                m_exitFlag = false;
        // ununsed
        bool                            unused_bool = false;
        // count for text renderer
        uint16_t                        m_uTextRenderCount = 0;
        // 等待垂直同步次数
        uint32_t                        m_dwWaitVSCount = 0;
        // 等待垂直同步起始时间
        uint32_t                        m_dwWaitVSStartTime = 0;
        // delta time in sec.
        float                           m_fDeltaTime = 0.f;
        // unused float
        float                           m_fUnused = 0.f;
        // textrender: normal
        CUINormalTextRender             m_normalTRenderer;
        // xml doc for window
        pugi::xml_document              m_docWindow;
        // xml doc for template
        pugi::xml_document              m_docTemplate;
        // windows
        UIWindow*                       m_apWindows[LongUIMaxWindow];
        // local name
        wchar_t                         m_szLocaleName[LOCALE_NAME_MAX_LENGTH / sizeof(void*) * sizeof(void*) + sizeof(void*)];
        // name of text renderers
        NameTR                          m_aszTextRendererName[LongUITextRendererCountMax];
#ifdef LONGUI_WITH_DEFAULT_CONFIG
        // 默认配置
        CUIDefaultConfigure             m_config;
#endif
    public:
        // constructor 构造函数
        CUIManager() noexcept;
        // destructor 析构函数
        ~CUIManager() noexcept;
        // delte this method 删除复制构造函数
        CUIManager(const CUIManager&) = delete;
        // delte this method 删除移动构造函数
        CUIManager(CUIManager&&) = delete;
    private:
        // load the template string
        auto load_control_template_string(const char* str) noexcept->HRESULT;
        // set the template string
        auto set_control_template_string() noexcept->HRESULT;
        // create all resources
        auto create_device_resources() noexcept->HRESULT;
        // create index zero resources
        auto create_indexzero_resources() noexcept->HRESULT;
        // create system brush
        auto create_system_brushes() noexcept->HRESULT;
        // discard resources
        void discard_resources() noexcept;
        // create the control
        auto create_control(CreateControlFunction function, pugi::xml_node node, size_t id) noexcept->UIControl*;
        // create ui window
        auto create_ui_window(const pugi::xml_node node, UIWindow* wnd, callback_for_creating_window proc, void* buf) noexcept->UIWindow*;
        // do some creating-event
        void do_creating_event(CreateEventType type) noexcept;
        // create a control tree for window
        void make_control_tree(UIWindow* window, pugi::xml_node node) noexcept;
    private:
        // invisible window proc
        // static LRESULT CALLBACK InvisibleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
        // main window proc
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    public:
        // 单例 CUIRenderer
        static      CUIManager      s_instance;
    public: // DEBUG ZONE 调试区
#ifdef _DEBUG
        // last DebugStringLevel
        DebugStringLevel        m_lastLevel = DebugStringLevel::DLevel_Log;
        // overload << operator for DebugStringLevel
        CUIManager& operator<< (const DebugStringLevel l)  noexcept { m_lastLevel = l; return *this; }
        // overload << operator for float
        CUIManager& operator<< (const float f) noexcept;
        // overload << operator for long
        CUIManager& operator<< (const long l) noexcept;
        // overload << operator for long
        CUIManager& operator<< (const bool b) noexcept;
        // overload << operator for long
        CUIManager& operator<< (const UIControl*) noexcept;
        // overload << operator for endl
        CUIManager& operator<< (const LongUI::EndL) noexcept;
        // overload << operator for DXGI_ADAPTER_DESC*
        CUIManager& operator<< (const D2D1_MATRIX_3X2_F& m) noexcept;
        // overload << operator for DXGI_ADAPTER_DESC*
        CUIManager& operator<< (const DXGI_ADAPTER_DESC& d) noexcept;
        // overload << operator for RectLTWH_F
        CUIManager& operator<< (const RectLTWH_F& r) noexcept;
        // overload << operator for D2D1_RECT_F
        CUIManager& operator<< (const D2D1_RECT_F& r) noexcept;
        // overload << operator for D2D1_POINT_2F
        CUIManager& operator<< (const D2D1_POINT_2F& p) noexcept;
        // overload << operator for CUIString
        CUIManager& operator<< (const CUIString& s) noexcept { this->OutputNoFlush(m_lastLevel, s.c_str()); return *this; }
        // overload << operator for const wchar_t*
        CUIManager& operator<< (const wchar_t* s) noexcept { this->OutputNoFlush(m_lastLevel, s); return *this; }
        // overload << operator for const char*
        CUIManager& operator<< (const char* s) noexcept { this->OutputNoFlush(m_lastLevel, s); return *this; }
        // overload << operator for wchar_t
        CUIManager& operator<< (const wchar_t ch) noexcept { wchar_t chs[2] = { ch, 0 }; this->OutputNoFlush(m_lastLevel, chs); return *this; }
        // output debug string with flush
        inline void Output(DebugStringLevel l, const wchar_t* s) noexcept { if(this->flag & IUIConfigure::Flag_OutputDebugString) this->configure->OutputDebugStringW(l, s, true); }
        // output debug string with flush
        void Output(DebugStringLevel l, const char* s) noexcept;
    private:
        // output debug (utf-8) string without flush
        void OutputNoFlush(DebugStringLevel l, const char* s) noexcept;
        // output debug string without flush
        inline void OutputNoFlush(DebugStringLevel l, const wchar_t* s) const noexcept { this->configure->OutputDebugStringW(l, s, false); }
    public:
#else
        // overload << operator 重载 << 运算符
        template<typename T> const CUIManager& operator<< (T t) const noexcept { UNREFERENCED_PARAMETER(t);  return *this; }
        // output with wide char
        inline void Output(DebugStringLevel l, const wchar_t* s) const noexcept { UNREFERENCED_PARAMETER(l); UNREFERENCED_PARAMETER(s);}
        // output with utf-8
        inline void Output(DebugStringLevel l, const char* s) const noexcept { UNREFERENCED_PARAMETER(l); UNREFERENCED_PARAMETER(s); }
#endif
    };
    // auto locker
    class CUIAutoLocker {
    public:
        // ctor
        CUIAutoLocker() noexcept { UIManager.Lock(); }
        // dtor
        ~CUIAutoLocker() noexcept { UIManager.Unlock(); }
    private:
    };
#define AutoLocker CUIAutoLocker locker
    // formated buffer
#ifdef _DEBUG
    auto Formated(const wchar_t* format, ...) noexcept -> const wchar_t*;
#else
    static auto Formated(...) noexcept { return L""; }
#endif
}