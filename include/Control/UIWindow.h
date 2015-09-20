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
    // ui's window
    class UIWindow : public UIVerticalLayout, public Helper::ComStatic<
        Helper::QiListSelf<IUnknown, Helper::QiList<IDropTarget>>> {
        // 父类申明
        using Super = UIVerticalLayout ;
        // 友元申明
        friend class CUIManager;
        // message id for TaskbarBtnCreated
        static const UINT s_uTaskbarBtnCreatedMsg;
        // timer id for blink
        static constexpr UINT_PTR BLINK_EVENT_ID = 0;
    public:
        // flag of window
        enum WindowFlag : uint8_t {
            // no flag
            Flag_None = 0,
            // [default: false] window is always do full-rendering
            // XML Attribute : "fullrender"@bool
            Flag_FullRendering = 1 << 0,
            // [default: false] window is always do full-rendering 
            // like video game, will ingnore Flag_FullRendering
            // XML Attribute : "alwaysrendering"@bool
            Flag_AlwaysRendering = 1 << 1,
        };
        // type of window
        enum WindowType : uint8_t {
            // draw on parent-window, supported as parent-window
            // longui menu only, error with system menu
            Type_RenderOnParent = 0,
            // normal window, supported for Win7(updated) and later
            // both system menu and longui menu
            Type_Normal,
            // custom window, supported for Win7(updated) and later
            // you should render "close buttom" by youself
            // longui menu only, bad indea with system menu
            Type_CustomDraw,
            // layered window, supported for Win8.1 and later
            // longui menu only, VERY bad indea with system menu
            // because of DirectComposition
            Type_Layered,
        };
    protected:
        // index of BitArray
        enum BitArrayIndex : uint32_t {
            // caret in(true) or out?
            Index_CaretIn = 0,
            // do caret?
            Index_DoCaret,
            // in draging?
            Index_InDraging,
            // window rendered in last time, or want to render in this time
            Index_Rendered,
            // new size?
            Index_NewSize,
            // full rendering in this frame?
            Index_FullRenderingThisFrame,
        };
    public: // UIControl 接口实现
        // Render 渲染 
        virtual void Render(RenderType type) const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event 鼠标事件处理
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept->HRESULT override;
        // close this control 关闭控件
        virtual void Cleanup() noexcept override;
    protected:
        // constructor
        UIWindow(pugi::xml_node node, UIWindow* parent) noexcept;
        // destructor
        ~UIWindow() noexcept;
        // deleted 
        UIWindow(const UIWindow&) = delete; UIWindow() = delete;
    public: // some new
        // on close event
        virtual auto OnClose() noexcept -> bool { this->Cleanup(); UIManager.Exit(); return true; };
    public: // IDropTarget interface
        // impl for IDropTarget::DragEnter
        HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj,DWORD grfKeyState, POINTL pt,DWORD *pdwEffect) noexcept override;
        // impl for IDropTarget::DragOver
        HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) noexcept override;
        // impl for IDropTarget::DragLeave
        HRESULT STDMETHODCALLTYPE DragLeave(void) noexcept override;
        // impl for IDropTarget::Drop
        HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) noexcept override;
    public:
        // begin render
        void BeginDraw() const noexcept;
        // end ender
        void EndDraw() const noexcept;
        // register for calling PreRender
        void RegisterOffScreenRender(UIControl* c, bool is3d) noexcept;
        // unregister for calling PreRender
        void UnRegisterOffScreenRender(UIControl*) noexcept;
        // set the caret
        void SetCaretPos(UIControl* ctrl, float _x, float _y) noexcept;
        // create the caret
        void CreateCaret(UIControl* ctrl, float width, float height) noexcept;
        // show the caret
        void ShowCaret() noexcept;
        // hide the caret
        void HideCaret() noexcept;
        // set focus control
        void SetFocus(UIControl* ctrl) noexcept;
        // find control by CUIString
        auto FindControl(const CUIString& name) noexcept->UIControl*;
        // find control by wchar_t pointer
        auto FindControl(const wchar_t* name) noexcept { CUIString n(name); return this->FindControl(n); }
        // add control with name
        void AddControl(const std::pair<CUIString, void*>& pair) noexcept;
        // set icon, bad
        void SetIcon(HICON hIcon = nullptr) noexcept;
    public:
        // get waite-vs event handle
        LongUIInline auto GetVSyncEvent() const noexcept { return m_hVSync; }
        // show window
        LongUIInline auto ShowWindow(int show = SW_SHOW) noexcept { return ::ShowWindow(m_hwnd, show); }
        // register for calling PreRender with 3d content
        LongUIInline auto RegisterOffScreenRender3D(UIControl* c) noexcept { return this->RegisterOffScreenRender(c, true); }
        // register for calling PreRender with 2d content
        LongUIInline auto RegisterOffScreenRender2D(UIControl* c) noexcept { return this->RegisterOffScreenRender(c, false); }
        // start render in sec.
        LongUIInline auto StartRender(float t, UIControl* c) noexcept { return m_uiRenderQueue.PlanToRender(0.f, t, c); }
        // plan to render in sec.
        LongUIInline auto PlanToRender(float w, float t, UIControl* c) noexcept { return m_uiRenderQueue.PlanToRender(w, t, c); }
        // update control later
        LongUIInline auto Invalidate(UIControl* c) noexcept { return m_uiRenderQueue.PlanToRender(0.f, 0.f, c); }
        // get window handle
        LongUIInline auto GetHwnd() const noexcept { return m_hwnd;};
        // set mouse capture
        LongUIInline auto SetCapture(UIControl* c) noexcept { ::SetCapture(m_hwnd); m_pCapturedControl = c; };
        // release mouse capture
        LongUIInline auto ReleaseCapture() noexcept { ::ReleaseCapture(); m_pCapturedControl = nullptr; };
        // is release mouse capture
        LongUIInline auto IsReleasedControl(UIControl* c) noexcept { return m_pCapturedControl == c; };
        // get back buffer
        LongUIInline auto GetBackBuffer() noexcept { return ::SafeAcquire(m_pTargetBimtap); }
        // is rendered
        LongUIInline auto IsRendered() const noexcept { return m_baBoolWindow.Test(UIWindow::Index_Rendered); }
        // render window
        LongUIInline auto RenderWindow() const noexcept { this->BeginDraw(); this->Render(RenderType::Type_Render); return this->EndDraw(); }
        // next frame
        LongUIInline auto NextFrame() noexcept { m_uiRenderQueue.GetCurrentUnit()->length = 0; ++m_uiRenderQueue; }
    private:
        // release data
        void release_data() noexcept;
        // draw the caret
        void draw_caret() noexcept;
        // refresh the caret
        void refresh_caret() noexcept;
        // set the present
        void set_present_parameters(DXGI_PRESENT_PARAMETERS&)const noexcept;
    public: // MSG MAPPING ZONE
        // on WM_CREATE
        bool OnCreated(HWND hwnd) noexcept;
        // on WM_MOUSEMOVE
        bool OnMouseMove(const LongUI::EventArgument& arg) noexcept;
        // on WM_MOUSEWHEEL
        bool OnMouseWheel(const LongUI::EventArgument& arg) noexcept;
        // resize window
        void OnResize(bool force = false) noexcept;
        // parent window, be careful with UIControl::parent
        UIWindow*       const   window_parent = nullptr;
        // window flag
        WindowFlag      const   window_flags = WindowFlag::Flag_None;
        // window type
        WindowType      const   window_type = WindowType::Type_Normal;
        // last mouse point
        D2D1_POINT_2F           last_point = D2D1::Point2F(-1.f, -1.f);
    protected:
        // unused float
        float                   m_fUnused = 0.f;
        // count for the caret
        uint32_t                m_cShowCaret = 0;
        // will use BitArray instead of them
        Helper::BitArray32      m_baBoolWindow;
        // text anti-mode
        uint16_t                m_textAntiMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
        // mini size
        D2D1_SIZE_U             m_miniSize = D2D1::SizeU();
        // window handle
        HWND                    m_hwnd = nullptr;
        // current data object
        IDataObject*            m_pCurDataObject = nullptr;
        // task bar list
        ITaskbarList4*          m_pTaskBarList = nullptr;
        // helper for drop target
        IDropTargetHelper*      m_pDropTargetHelper = nullptr;
        // wait for vsync
        HANDLE                  m_hVSync = nullptr;
        // swap chain
        IDXGISwapChain2*        m_pSwapChain = nullptr;
        // target bitmap
        ID2D1Bitmap1*           m_pTargetBimtap = nullptr;
        // Direct Composition Device
        IDCompositionDevice*    m_pDcompDevice = nullptr;
        // Direct Composition Target
        IDCompositionTarget*    m_pDcompTarget = nullptr;
        // Direct Composition Visual
        IDCompositionVisual*    m_pDcompVisual = nullptr;;
        // now focused control (only one)
        UIControl*              m_pFocusedControl = nullptr;
        // now dragdrop control (only one)
        UIControl*              m_pDragDropControl = nullptr;
        // now captured control (only one)
        UIControl*              m_pCapturedControl = nullptr;
        // blink timer id
        UINT_PTR                m_idBlinkTimer = 0;
        // normal  l-param
        LPARAM                  m_normalLParam = 0;
        // caret rect in px
        RectLTWH_L              m_rcCaretPx;
    public:
        // the real pixel size  of window(HWND)
        D2D1_SIZE_U     const   window_size = D2D1::SizeU();
        // default(arrow)cursor
        HCURSOR         const   default_cursor = ::LoadCursor(nullptr, IDC_ARROW);
        // now sursor
        HCURSOR                 now_cursor = default_cursor;
        // debug info
#ifdef _DEBUG
        bool                    test_D2DERR_RECREATE_TARGET = false;
        bool                    debug_show = false;
        bool                    debug_unused[6];
        uint32_t                full_render_counter = 0;
        uint32_t                dirty_render_counter = 0;
#endif
        // clear color @xml "clearcolor"
        D2D1::ColorF            clear_color = D2D1::ColorF(D2D1::ColorF::White);
    protected: // huge obj
        // rendering queue
        CUIRenderQueue          m_uiRenderQueue;
        // now unit
        CUIRenderQueue::UNIT    m_aUnitNow;
        // dirty rects : +1 for caret
        RECT                    m_dirtyRects[LongUIDirtyControlSize + 1];
        // track mouse event: end with DWORD
        TRACKMOUSEEVENT         m_csTME;
        // current STGMEDIUM: begin with DWORD
        STGMEDIUM               m_curMedium;
        // registered control
        BasicContainer          m_vRegisteredControl;
        // control name ->map-> control pointer
        StringMap               m_mapString2Control;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIWindow>() {
        // {64F7B3E5-621E-4864-9535-7E6A29F670C1}
        static const GUID IID_LongUI_UIWindow = { 
            0x64f7b3e5, 0x621e, 0x4864,{ 0x95, 0x35, 0x7e, 0x6a, 0x29, 0xf6, 0x70, 0xc1 } 
        };
        return IID_LongUI_UIWindow;
    }
#endif
    // operator for UIWindow::WindowFlag
    LONGUI_DEFINE_ENUM_FLAG_OPERATORS(UIWindow::WindowFlag, uint8_t);
}