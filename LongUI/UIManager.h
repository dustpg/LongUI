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
    class LongUIAlignas LongUIAPI CUIManager {
    public: 
        // Windows Version
        enum WindowsVersion : size_t {
            // Win8,
            Style_Win8 = 0, 
            //  Win8.1
            Style_Win8_1,
            // Win10
            Style_Win10
        };
    public: // handle zone 操作区
        // initialize 初始化
        auto Initialize(IUIConfigure* =nullptr) noexcept->HRESULT;
        // uninitialize 反初始化
        void UnInitialize() noexcept;
        // run 运行
        void Run() noexcept;
        // add "string to create funtion" map 添加函数映射关系
        auto AddS2CPair(const wchar_t*, CreateControlFunction) noexcept->HRESULT;
        // ShowError with HRESULT code
        void ShowError(HRESULT, const wchar_t* str_b =nullptr) noexcept;
        // lock
        auto Lock() noexcept { return m_uiLocker.Lock(); }
        // unlock
        auto Unlock() noexcept { return m_uiLocker.Unlock(); }
    private:
    public: // 特例
        // load bitmap
        static auto __cdecl LoadBitmapFromFile(
            LongUIRenderTarget*, IWICImagingFactory *, PCWSTR, UINT, UINT, ID2D1Bitmap1 **
            ) noexcept ->HRESULT;
        // get default LongUI imp IDWriteFontCollection
        static auto __cdecl CreateLongUIFontCollection(
            IDWriteFactory*, const wchar_t* filename=L"*.*tf", const wchar_t* folder=L"Fonts"
            ) noexcept->IDWriteFontCollection*;
        // create path-geometry from utf-32 char array using text format
        // fontface: (you can see <LongUI::UIScrollBar::UIScrollBar>)
        //          fontface == nullptr, ok but a bit slow
        //          *fontface == nullptr, ok, a bit slow, and out a IDWriteFontFace*, you can use it in next time(same format)
        //          *fontface != nullptr, ok
        static auto __cdecl CreateTextPathGeometry(
            IN const char32_t* utf32_string,  
            IN size_t string_length, 
            IN IDWriteTextFormat* format, 
            IN ID2D1Factory* factory,
            IN OUT OPTIONAL IDWriteFontFace** fontface,
            OUT ID2D1PathGeometry** geometry
            ) noexcept->HRESULT;
        // create mesh from geometry
        static auto __cdecl CreateMeshFromGeometry(ID2D1Geometry* geometry, ID2D1Mesh** mesh) noexcept->HRESULT;
        // format the text into core-mode with xml string: 面向数据
        static auto __cdecl XMLToCoreFormat(const char*, wchar_t*) noexcept->bool;
        // format the text into textlayout with format: 面向C/C++
        static auto __cdecl FormatTextCore(FormatTextConfig&, const wchar_t*, ...) noexcept->IDWriteTextLayout*;
        // format the text into textlayout with format: 面向C/C++
        static auto __cdecl FormatTextCore(FormatTextConfig&, const wchar_t*, va_list) noexcept->IDWriteTextLayout*;
        // create ui window via xml string 创建窗口
        template<typename T = UIWindow>
        LongUINoinline auto CreateUIWindow(const char*, void* = nullptr, UIWindow* = nullptr) noexcept->T*;
        // create ui window via pugixml node 创建窗口
        template<typename T = UIWindow>
        LongUINoinline auto CreateUIWindow(const pugi::xml_node, void* = nullptr, UIWindow* = nullptr) noexcept->T*;
    public: // UAC About
        // is run as admin?
        static bool WINAPI IsRunAsAdministrator() noexcept;
        // try to elevate now,  will lauch a new elevated instance and
        // exit this instance if success. be careful about your app if
        // only can be in one instance
        static bool WINAPI TryElevateUACNow(const wchar_t* parameters = nullptr, bool exit = true) noexcept;
    public: // inline 区
        // ShowError with string
        LongUIInline auto ShowError(const wchar_t * str, const wchar_t* str_b = nullptr) { this->configure->ShowError(str, str_b); }
        // 获取文本渲染器 GetXXX method will call AddRef if it is a COM object
        LongUIInline auto GetTextRenderer(int i) const { return ::SafeAcquire(m_apTextRenderer[i]); }
        // Exit 退出
        LongUIInline auto Exit() { m_exitFlag = true; }
        // 重建资源
        LongUIInline auto RecreateResources() { this->discard_resources(); return this->create_resources(); }
    public: // 隐形转换区
        // 转换为 LongUIRenderTarget
#define UIManager_RenderTaget (static_cast<ID2D1DeviceContext*>(UIManager))
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
        // 转换为 IWICImagingFactory2
#define UIManager_WICImagingFactory (static_cast<IWICImagingFactory2*>(UIManager))
        LongUIInline operator IWICImagingFactory2*()const noexcept { return m_pWICFactory; };
#ifdef LONGUI_VIDEO_IN_MF
        // 转换为  IMFDXGIDeviceManager
#   define UIManager_MFDXGIDeviceManager (static_cast<IMFDXGIDeviceManager*>(UIManager))
        LongUIInline operator IMFDXGIDeviceManager*()const noexcept { return m_pDXGIManager; };
        // 转换为  IMFMediaEngineClassFactory
#   define UIManager_MFMediaEngineClassFactory (static_cast<IMFMediaEngineClassFactory*>(UIManager))
        LongUIInline operator IMFMediaEngineClassFactory*()const noexcept { return m_pMediaEngineFactory; };
        // MF Dxgi设备管理器
        IMFDXGIDeviceManager*           m_pDXGIManager = nullptr;
        // MF 媒体引擎
        IMFMediaEngineClassFactory*     m_pMediaEngineFactory = nullptr;
#endif
        // 转换为 CUIInput
        LongUIInline operator const CUIInput&() const noexcept { return m_uiInput; };
#define UIInput (static_cast<const CUIInput&>(UIManager))
    public:
        // script 脚本
        IUIScript*           const      script = nullptr;
        // the handler
        InlineParamHandler   const      inline_handler = nullptr;
        // config
        IUIConfigure*        const      configure = nullptr;
        // windows version
        WindowsVersion       const      version = WindowsVersion::Style_Win8;
        // user context size 用户上下文大小
        size_t               const      user_context_size = 0;
        // last frame rendered window
        UIWindow*                       rendered_last_frame = nullptr;
    private:
        // D2D 工厂
        ID2D1Factory1*                  m_pd2dFactory = nullptr;
        // WIC 工厂
        IWICImagingFactory2*            m_pWICFactory = nullptr;
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
        // 调试对象
        ID3D11Debug*                    m_pd3dDebug = nullptr;
#endif
        // 文本渲染器
        UIBasicTextRenderer*            m_apTextRenderer[LongUIMaxTextRenderer];
        // system brush
        ID2D1Brush*                     m_apSystemBrushes[STATUS_COUNT];
        // 二进制资源读取器
        IUIResourceLoader*              m_pResourceLoader = nullptr;
        // default bitmap buffer
        uint8_t*                        m_pBitmap0Buffer = nullptr;
        // map 函数映射
        StringMap                       m_mapString2CreateFunction;
        // 所创设备特性等级
        D3D_FEATURE_LEVEL               m_featureLevel;
        // 退出信号
        std::atomic_uint32_t            m_exitFlag = false;
        // 渲染器数量
        uint32_t                        m_uTextRenderCount = 0;
        // 输入
        CUIInput                        m_uiInput;
        // 锁
        CUILocker                       m_uiLocker;
        // TF 仓库
        BasicContainer                  m_textFormats;
        // 笔刷容器
        BasicContainer                  m_brushes;
        // 窗口容器
        BasicContainer                  m_windows;
        // 位图容器
        BasicContainer                  m_bitmaps;
        // Meta图标容器
        BasicContainer                  m_metaicons;
        // Meta容器
        LongUI::Vector<Meta>            m_metas;
        // 地区名称
        wchar_t                         m_szLocaleName[LOCALE_NAME_MAX_LENGTH / 4 * 4 + 4];
#ifdef LONGUI_WITH_DEFAULT_CONFIG
        // 默认配置
        CUIDefaultConfigure             m_config;
#endif
        // 普通文本渲染器
        UINormalTextRender              m_normalTRenderer;
        // tinyxml2 资源
        pugi::xml_document              m_docResource;
        // tinyxml2 窗口
        pugi::xml_document              m_docWindow;
    public:
        // add window
        void AddWindow(UIWindow* wnd) noexcept;
        // remove window
        void RemoveWindow(UIWindow* wnd) noexcept;
        // register, return -1 for error(out of renderer space), return other for index
        auto RegisterTextRenderer(UIBasicTextRenderer*) noexcept-> int32_t;
        // get text format, "Get" method will call IUnknown::AddRef if it is a COM object
        auto GetTextFormat(uint32_t i) noexcept->IDWriteTextFormat*;
        // get bitmap by index, "Get" method will call IUnknown::AddRef if it is a COM object
        auto GetBitmap(uint32_t index) noexcept->ID2D1Bitmap1*;
        // get brush by index, "Get" method will call IUnknown::AddRef if it is a COM object
        auto GetBrush(uint32_t index) noexcept->ID2D1Brush*;
        // get meta by index, "Get" method will call IUnknown::AddRef if it is a COM object
        // Meta isn't a IUnknown object, so, won't call Meta::bitmap->AddRef
        void GetMeta(uint32_t index, LongUI::Meta&) noexcept;
        // get meta's icon handle by index, will do runtime-converting if first call the
        //  same index. "Get" method will call IUnknown::AddRef if it is a COM object
        // HICON isn't a IUnknown object. Meta HICON managed by this manager
        auto GetMetaHICON(uint32_t index) noexcept->HICON;
        // get system brush
        auto GetSystemBrush(uint32_t index) noexcept { return ::SafeAcquire(m_apSystemBrushes[index]); }
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
        // create programs resources
        auto create_programs_resources() throw(std::bad_alloc&) ->void;
        // create all resources
        auto create_resources() noexcept ->HRESULT;
        // create system brush
        auto create_system_brushes() noexcept->HRESULT;
        // discard resources
        void discard_resources() noexcept;
        // create bitmap
        void add_bitmap(const pugi::xml_node) noexcept;
        // create brush
        void add_brush(const pugi::xml_node) noexcept;
        // create text format
        void add_textformat(const pugi::xml_node) noexcept;
        // create meta
        void add_meta(const pugi::xml_node) noexcept;
    public:
        // get create function
        auto GetCreateFunc(const char*)noexcept->CreateControlFunction;
        // get create function
        auto GetCreateFunc(const CUIString&)noexcept->CreateControlFunction;
        // get create function
        auto GetCreateFunc(const wchar_t* class_name, uint32_t len=0)noexcept { CUIString name(class_name, len); return GetCreateFunc(name); }
    private:
        // 创建控件
        auto create_control(pugi::xml_node) noexcept->UIControl*;
        // 创建控件树
        void make_control_tree(UIWindow*, pugi::xml_node) noexcept;
    private:
        // main window proc 窗口过程函数
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
        inline void Output(DebugStringLevel l, const wchar_t* s) noexcept { this->configure->OutputDebugStringW(l, s, true); }
        // output debug string with flush
        void Output(DebugStringLevel l, const char* s) noexcept;
        // Output with format for None
        void _cdecl OutputN(const wchar_t*, ...) noexcept;
        // Output with format for Log
        void _cdecl OutputL(const wchar_t*, ...) noexcept;
        // Output with format for Hint
        void _cdecl OutputH(const wchar_t*, ...) noexcept;
        // Output with format for Warning
        void _cdecl OutputW(const wchar_t*, ...) noexcept;
        // Output with format for Error
        void _cdecl OutputE(const wchar_t*, ...) noexcept;
        // Output with format for Fatal
        void _cdecl OutputF(const wchar_t*, ...) noexcept;
    private:
        // output debug (utf-8) string without flush
        void OutputNoFlush(DebugStringLevel l, const char* s) noexcept;
        // output debug string without flush
        inline void OutputNoFlush(DebugStringLevel l, const wchar_t* s) const noexcept { this->configure->OutputDebugStringW(l, s, false); }
    public:
#else
        // overload << operator 重载 << 运算符
        template<typename T> const CUIManager& operator<< (T t) const noexcept { return *this; }
        // output with wide char
        inline void Output(DebugStringLevel l, const wchar_t* s) const noexcept {  }
        // output with utf-8
        inline void Output(DebugStringLevel l, const char* s) const noexcept {  }
        // Output with format for None
        inline void _cdecl OutputN(const wchar_t*, ...) const noexcept {  }
        // Output with format for Log
        inline void _cdecl OutputL(const wchar_t*, ...) const noexcept {  }
        // Output with format for Hint
        inline void _cdecl OutputH(const wchar_t*, ...) const noexcept {  }
        // Output with format for Warning
        inline void _cdecl OutputW(const wchar_t*, ...) const noexcept {  }
        // Output with format for Error
        inline void _cdecl OutputE(const wchar_t*, ...) const noexcept {  }
        // Output with format for Fatal
        inline void _cdecl OutputF(const wchar_t*, ...) const noexcept {  }
#endif

    };
    // 创建窗口
    template<typename T>
    LongUINoinline auto CUIManager::CreateUIWindow(const char* xml, void* buffer_sent, UIWindow* parent) noexcept->T* {
        pugi::xml_node root_node(nullptr); T* wnd = nullptr; auto buffer = buffer_sent;
        pugi::xml_parse_result result;
        // get buffer of window
        if (!buffer) buffer = LongUI::CtrlAlloc(sizeof(T));
        // parse the xml and check error
        if (buffer && (result = this->m_docWindow.load_string(xml)) &&
            (root_node = this->m_docWindow.first_child())) {
            // create the window
            wnd = new(buffer) T(root_node, parent);
            // recreate res'
            wnd->Recreate(this->m_pd2dDeviceContext);
            // make control tree
            this->make_control_tree(wnd, root_node);
            // finished
            LongUI::EventArgument arg; arg.sender = wnd;
            arg.event = LongUI::Event::Event_FinishedTreeBuliding;
            wnd->DoEvent(arg);
        }
        else if(!buffer_sent && buffer) {
            LongUI::CtrlFree(buffer);
        }
        // 错误检测
        if (!result) {
            UIManager << DL_Error << L"XML Parse Error: " << result.description() << LongUI::endl;
        }
        assert(wnd && "no window created");
        return wnd;
    }
    // 创建窗口
    template<typename T>
    LongUINoinline auto CUIManager::CreateUIWindow(const pugi::xml_node node, void* buffer_sent, UIWindow* parent) noexcept->T* {
        pugi::xml_node root_node(nullptr); T* wnd = nullptr; auto buffer = buffer_sent;
        // get buffer of window
        if (!buffer) buffer = LongUI::CtrlAlloc(sizeof(T));
        // check no error
        if (buffer && node) {
            // create the window
            wnd = new(buffer) T(root_node, parent);
            // recreate res'
            wnd->Recreate(this->m_pd2dDeviceContext);
            // make control tree
            this->make_control_tree(wnd, root_node);
            // finished
            LongUI::EventArgument arg; arg.sender = wnd;
            arg.event = LongUI::Event::Event_FinishedTreeBuliding;
            wnd->DoEvent(arg);
        }
        else if(!buffer_sent && buffer) {
            LongUI::CtrlFree(buffer);
        }
        assert(wnd && "no window created");
        return wnd;
    }
}