#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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
    // ui manager UI管理器
    class CUIManager {
        // string allocator
        using StringAllocator = CUIShortStringAllocator<>;
        // create ui window call back
        using callback_for_creating_window = auto(*)(pugi::xml_node node, XUIBaseWindow* container) ->UIViewport*;
    public: // handle zone 操作区
        // initialize 初始化
        LongUIAPI auto Initialize(IUIConfigure* config = nullptr) noexcept ->HRESULT;
        // uninitialize 反初始化
        LongUIAPI void Uninitialize() noexcept;
        // run 运行
        LongUIAPI void Run() noexcept;
        // add "string to create funtion" map 添加函数映射关系
        LongUIAPI auto RegisterControlClass(CreateControlFunction func, const char* clname) noexcept ->HRESULT;
        // unregister 取消映射函数映射关系
        LongUIAPI void UnregisterControlClass(const char* clname) noexcept;
        // ShowError with HRESULT code
        LongUIAPI void ShowError(HRESULT, const wchar_t* str_b = nullptr) noexcept;
        // wait for VS
        LongUIAPI void WaitVS() noexcept;
        // add window
        LongUIAPI void AddWindow(XUISystemWindow* wnd) noexcept;
        // remove window
        LongUIAPI void RemoveWindow(XUISystemWindow* wnd) noexcept;
        // refresh display frequency
        //LongUIAPI void RefreshDisplayFrequency() noexcept;
        // return -1 for error(out of renderer space), return other for index
        LongUIAPI auto RegisterTextRenderer(XUIBasicTextRenderer*, const char name[LongUITextRendererNameMaxLength]) noexcept ->int32_t;
        // get text renderer by name 
        LongUIAPI auto GetTextRenderer(const char* name) const noexcept ->XUIBasicTextRenderer*;
        // get text format, "Get" method will call IUnknown::AddRef if it is a COM object
        LongUIAPI auto GetTextFormat(size_t index) noexcept ->IDWriteTextFormat*;
        // get bitmap by index, "Get" method will call IUnknown::AddRef if it is a COM object
        LongUIAPI auto GetBitmap(size_t index) noexcept ->ID2D1Bitmap1*;
        // get brush by index, "Get" method will call IUnknown::AddRef if it is a COM object
        LongUIAPI auto GetBrush(size_t index) noexcept ->ID2D1Brush*;
        // get meta by index, "Get" method will call IUnknown::AddRef if it is a COM object
        // Meta isn't a IUnknown object, so, won't call Meta::bitmap->AddRef
        LongUIAPI void GetMeta(size_t index, LongUI::Meta&) noexcept;
        // get meta's icon handle by index, Meta HICON managed by this manager
        LongUIAPI auto GetMetaHICON(size_t index) noexcept ->HICON;
        // get create function via control-class name
        LongUIAPI auto GetCreateFunc(const char* clname) noexcept ->CreateControlFunction;
        // create control with template id, template and function cannot be null in same time
        LongUIAPI auto CreateControl(UIContainer* cp, size_t templateid, CreateControlFunction function) noexcept ->UIControl*;
        // create text format
        LongUIAPI auto CreateTextFormat(
            _In_z_ WCHAR const* fontFamilyName,
            DWRITE_FONT_WEIGHT fontWeight,
            DWRITE_FONT_STYLE fontStyle,
            DWRITE_FONT_STRETCH fontStretch,
            FLOAT fontSize,
            _COM_Outptr_ IDWriteTextFormat** textFormat
            ) noexcept ->HRESULT;
        // create control with xml node, node and function cannot be null in same time
        auto CreateControl(UIContainer* cp, pugi::xml_node node, CreateControlFunction function) noexcept {
            return this->create_control(cp, function, node, 0);
        }
    public: // Create UI Window Zone!!!!!!!!!
        // create ui window with xml string
        auto CreateUIWindow(const char* xml) noexcept { return this->CreateUIWindow<LongUI::UIViewport>(xml); }
        // create ui window with custom window && xml string
        template<class T> auto CreateUIWindow(const char* xml) noexcept ->T* {
            auto code = m_docWindow.load_string(xml); assert(code && "bad xml"); 
            if (code.status) return nullptr;
            auto create_func = [](pugi::xml_node node, XUIBaseWindow* window) noexcept ->UIViewport* {
                T* c = new(std::nothrow) T(window); if (c) c->T::initialize(node);
                return c;
            };
            return static_cast<T*>(this->create_ui_window(m_docWindow.first_child(), nullptr, create_func));
        }
    private:
        // exit
        inline void exit() noexcept { m_exitFlag = true; ::PostQuitMessage(0); }
    public: // inline 区
        // copystring for control in this winddow
        auto CopyString(const char* str) noexcept { return m_oStringAllocator.CopyString(str); }
        // copystring for control in this winddow in safe way
        auto CopyStringSafe(const char* str) noexcept { auto s = this->CopyString(str); return s ? s : ""; }
        // get system brush
        auto GetSystemBrush(uint32_t index) noexcept { return LongUI::SafeAcquire(m_apSystemBrushes[index]); }
        // get drop target helper
        auto GetDropTargetHelper() noexcept { return LongUI::SafeAcquire(m_pDropTargetHelper); }
        // get display frequency
        auto GetDisplayFrequency() const noexcept { return m_dDisplayFrequency; };
        // lock data
        auto DataLock() noexcept { return m_uiDataLocker.Lock(); }
        // unlock data
        auto DataUnlock() noexcept { return m_uiDataLocker.Unlock(); }
        // lock dxgi
        auto DxgiLock() noexcept { return m_uiDxgiLocker.Lock(); }
        // unlock dxgi
        auto DxgiUnlock() noexcept { return m_uiDxgiLocker.Unlock(); }
        // push delay cleanup
        auto PushDelayCleanup(UIControl* c) noexcept { m_vDelayCleanup.push_back(c); }
        // ShowError with string
        auto ShowError(const wchar_t * str, const wchar_t* str_b = nullptr) noexcept { this->configure->ShowError(str, str_b); }
        // GetXXX method will call AddRef if it is a COM object
        auto GetTextRenderer(int i) const noexcept { assert(i < m_uTextRenderCount && "out of range"); return LongUI::SafeAcquire(m_apTextRenderer[i]); }
#ifdef _DEBUG
        // exit the app
        void Exit() noexcept;
#else
        // exit the app
        void Exit() noexcept { this->exit(); }
#endif
        // recreate resources
        auto RecreateResources() noexcept { this->discard_resources(); return this->create_device_resources(); }
        // get delta time for ui
        auto GetDeltaTime() const noexcept { return m_fDeltaTime; }
    public: // 隐形转换区
        // 转换为 ID2D1DeviceContext
#define UIManager_RenderTarget (UIManager.GetRenderTargetNoAddRef())
        inline auto GetRenderTargetNoAddRef() const noexcept { return m_pd2dDeviceContext; };
        // 转换为 DXGI Factory2
#define UIManager_DXGIFactory (UIManager.GetDXGIFactoryNoAddRef())
        inline auto GetDXGIFactoryNoAddRef() const noexcept { return m_pDxgiFactory; };
        // 转换为 D3D11 Device
#define UIManager_D3DDevice  (UIManager.GetD3DDeviceNoAddRef())
        inline auto GetD3DDeviceNoAddRef() const noexcept { return m_pd3dDevice; };
        // 转换为 D3D11 Device Context
#define UIManager_D3DContext (UIManager.GetD3DContextNoAddRef())
        inline auto GetD3DContextNoAddRef() const noexcept { return m_pd3dDeviceContext; };
        // 转换为 D2D1 Device
#define UIManager_D2DDevice  (UIManager.GetD2DDeviceNoAddRef())
        inline auto GetD2DDeviceNoAddRef() const noexcept { return m_pd2dDevice; };
        // 转换为 DXGI Device1
#define UIManager_DXGIDevice (UIManager.GetDXGIDeviceNoAddRef())
        inline auto GetDXGIDeviceNoAddRef() const noexcept { return m_pDxgiDevice; };
        // 转换为 DXGI Adapter
#define UIManager_DXGIAdapter (UIManager.GetDXGIAdapterNoAddRef())
        inline auto GetDXGIAdapterNoAddRef() const noexcept { return m_pDxgiAdapter; };
        // 转换为 DWrite Factory1
#define UIManager_DWriteFactory (UIManager.GetDWriteFactoryNoAddRef())
        inline auto GetDWriteFactoryNoAddRef() const noexcept { return m_pDWriteFactory; };
        // 转换为 D2D Factory1
#define UIManager_D2DFactory (UIManager.GetD2DFactoryNoAddRef())
        inline auto GetD2DFactoryNoAddRef() const noexcept { return m_pd2dFactory; };
#ifdef LONGUI_WITH_MMFVIDEO
        // 转换为  IMFDXGIDeviceManager
#   define UIManager_MFDXGIDeviceManager (UIManager.GetMFDXGIDeviceManagerNoAddRef())
        inline auto GetMFDXGIDeviceManagerNoAddRef() const noexcept { return m_pMFDXGIManager; };
        // 转换为  IMFMediaEngineClassFactory
#   define UIManager_MFMediaEngineClassFactory (UIManager.GetMFMediaEngineClassFactoryNoAddRef())
        inline auto GetMFMediaEngineClassFactoryNoAddRef()const noexcept { return m_pMediaEngineFactory; };
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
        // flag for configure
        IUIConfigure::ConfigureFlag     flag = IUIConfigure::Flag_None;
    private:
        // asd
        uint32_t                        m_u32unused = 0;
        // string al
        StringAllocator                 m_oStringAllocator;
        // helper for drop target
        IDropTargetHelper*              m_pDropTargetHelper = nullptr;
        // D2D 工厂
        ID2D1Factory4*                  m_pd2dFactory = nullptr;
        // DWrite工厂
        IDWriteFactory1*                m_pDWriteFactory = nullptr;
        // DWrite 字体集
        IDWriteFontCollection*          m_pFontCollection = nullptr;
        // D3D 设备
        ID3D11Device*                   m_pd3dDevice = nullptr;
        // D3D 设备上下文
        ID3D11DeviceContext*            m_pd3dDeviceContext = nullptr;
        // D2D 设备
        ID2D1Device3*                   m_pd2dDevice = nullptr;
        // D2D 设备上下文
        ID2D1DeviceContext3*            m_pd2dDeviceContext = nullptr;
        // DXGI 工厂
        IDXGIFactory2*                  m_pDxgiFactory = nullptr;
        // DXGI 设备
        IDXGIDevice1*                   m_pDxgiDevice = nullptr;
        // DXGI 适配器
        IDXGIAdapter*                   m_pDxgiAdapter = nullptr;
#ifdef _DEBUG
        // debug object
        ID3D11Debug*                    m_pd3dDebug = nullptr;
    public:
        // frame id
        size_t                          frame_id = 0;
    private:
#endif
        // thread manager for TSF
        ITfThreadMgr*                   m_pTsfThreadManager = nullptr;
        // text renderer
        XUIBasicTextRenderer*           m_apTextRenderer[LongUITextRendererCountMax];
        // system brush
        ID2D1Brush*                     m_apSystemBrushes[STATE_COUNT];
        // loader
        IUIResourceLoader*              m_pResourceLoader = nullptr;
        // default bitmap buffer
        uint8_t*                        m_pBitmap0Buffer = nullptr;
        // map: string<->func
        StringTable                     m_hashStr2CreateFunc;
        // delay cleanup vector
        ControlVector                   m_vDelayCleanup;
        // windows
        SystemWindowVector              m_vWindows;
        // feature level
        D3D_FEATURE_LEVEL               m_featureLevel;
        // input
        CUIInput                        m_uiInput;
        // data locker
        CUILocker                       m_uiDataLocker;
        // dxgi locker
        CUILocker                       m_uiDxgiLocker;
        // timer
        CUITimer                        m_uiTimer;
#ifdef _DEBUG
        // exit timing tick
        uint32_t                        m_dbgExitTime = 0;
        // unused
        uint32_t                        m_dbgUnused = 0;
#endif
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
        uint16_t                        m_unused_u16 = 0;
        // singal/flag for exiting
        std::atomic_bool                m_exitFlag = false;
        // count for text renderer
        uint8_t                         m_uTextRenderCount = 0;
        // ununsed
        uint16_t                        m_dDisplayFrequency = 0;
        // vsync count
        uint32_t                        m_dwWaitVSCount = 0;
        // vsync start time
        uint32_t                        m_dwWaitVSStartTime = 0;
        // delta time in sec.
        float                           m_fDeltaTime = 0.f;
        // id for client
        TfClientId                      m_idTsfClient = 0;
        // textrender: normal
        CUINormalTextRender             m_normalTRenderer;
        // xml doc for window
        pugi::xml_document              m_docWindow;
        // xml doc for template
        pugi::xml_document              m_docTemplate;
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
        // create all resources
        LongUIAPI auto create_device_resources() noexcept ->HRESULT;
        // discard resources
        LongUIAPI void discard_resources() noexcept;
        // create the control with xml-node
        LongUIAPI auto create_control(UIContainer* cp, CreateControlFunction function, pugi::xml_node node, size_t id) noexcept ->UIControl*;
        // create ui window
        LongUIAPI auto create_ui_window(pugi::xml_node node, XUIBaseWindow* parent, callback_for_creating_window func) noexcept ->UIViewport*;
        // cleanup delay-cleanup-chain
        void cleanup_delay_cleanup_chain() noexcept;
        // load the template string
        auto load_control_template_string(const char* str) noexcept ->HRESULT;
        // set the template string
        auto set_control_template_string() noexcept ->HRESULT;
        // create index zero resources
        auto create_indexzero_resources() noexcept ->HRESULT;
        // create system brush
        auto create_system_brushes() noexcept ->HRESULT;
        // do some creating-event
        void do_creating_event(CreateEventType type) noexcept;
    public:
        // create a control tree for UIContainer
        void MakeControlTree(UIContainer* root, pugi::xml_node node) noexcept;
        // get theme colr
        LongUIAPI static auto GetThemeColor(D2D1_COLOR_F& colorf) noexcept ->HRESULT;
        // main window proc
        LongUIAPI static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
        // windows message to longui mouse event
        LongUIAPI static void WindowsMsgToMouseEvent(MouseEventArgument& event, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    public:
        // 单例 CUIRenderer
        LongUIAPI static CUIManager     s_instance;
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
        // overload << operator for bool
        CUIManager& operator<< (const bool b) noexcept;
        // overload << operator for void*
        CUIManager& operator<< (const void*) noexcept;
        // overload << operator for control
        CUIManager& operator<< (const UIControl*) noexcept;
        // overload << operator for controls
        CUIManager& operator<< (const ControlVector&) noexcept;
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
        // overload << operator for const char*
        CUIManager& operator<< (const CUIWrappedCCP& str) noexcept { this->OutputNoFlush(m_lastLevel, str.c_str()); return *this; }
        // overload << operator for wchar_t
        CUIManager& operator<< (const wchar_t ch) noexcept { wchar_t chs[2] = { ch, 0 }; this->OutputNoFlush(m_lastLevel, chs); return *this; }
        // output debug string with flush
        inline void Output(DebugStringLevel l, const wchar_t* s) noexcept { if (this->flag & IUIConfigure::Flag_OutputDebugString) this->configure->OutputDebugStringW(l, s, true); }
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
    // auto data locker
    class CUIDataAutoLocker {
    public:
        // ctor
        CUIDataAutoLocker() noexcept { UIManager.DataLock(); }
        // dtor
        ~CUIDataAutoLocker() noexcept { UIManager.DataUnlock(); }
    private:
    };
    // auto dxgi locker
    class CUIDxgiAutoLocker {
    public:
        // ctor
        CUIDxgiAutoLocker() noexcept { UIManager.DxgiLock(); }
        // dtor
        ~CUIDxgiAutoLocker() noexcept { UIManager.DxgiUnlock(); }
    private:
    };
    // formated buffer
#ifdef _DEBUG
    auto Formated(const wchar_t* format, ...) noexcept -> const wchar_t*;
    auto Interfmt(const wchar_t* format, ...) noexcept -> const wchar_t*;
#else
    static auto Formated(...) noexcept { return L""; }
#endif
}