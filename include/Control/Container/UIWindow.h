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
    // config::window
    namespace Config { struct Window : Control {
        // window title name
        const wchar_t*      title_name;
        // full rendering
        bool                full_rendering;
        // always do rendering
        bool                always_rendering;
    };}
    // config::popup window
    namespace Config { struct Popup {
        // left line of popup window
        float                       leftline;
        // top line(maybe as bottom of popup window)
        float                       topline;
        // bottom line(maybe as top of popup window)
        float                       bottomline;
        // with of popup window
        float                       width;
        // height of popup window
        float                       height;
        // parent window
        UIWindow*                   parent;
        // [OPTIONAL]child
        UIControl*                  child;
    };}
    // ui's window
    class LongUIAPI UIWindow : public UIVerticalLayout, public Helper::ComStatic<
        Helper::QiListSelf<IUnknown, Helper::QiList<IDropTarget>>> {
        // 父类申明
        using Super = UIVerticalLayout;
        // 友元申明
        friend class CUIManager;
        // message id for TaskbarBtnCreated
        static const UINT s_uTaskbarBtnCreatedMsg;
        // timer id for blink
        static constexpr UINT_PTR BLINK_EVENT_ID = 0;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // flag of window
        enum WindowFlag : uint32_t {
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
        enum WindowType : uint32_t {
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
        // string allocator
        using StringAllocator = CUIShortStringAllocator<>;
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
            // skip render
            Index_SkipRender,
            // prerender(for off screen render)
            Index_Prerender,
            // exit on close
            Index_ExitOnClose,
            // close when focus killed
            Index_CloseOnFocusKilled,
            // count of this
            INDEX_COUNT,
        };
        // assert
        static_assert(INDEX_COUNT < 32, "out of range");
    public: // UIControl 接口实现
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event 鼠标事件处理
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    public:
        // constructor
        UIWindow(UIWindow* parent) noexcept;
        // create popup window
        static auto CreatePopup(const Config::Popup&) noexcept -> UIWindow*;
    protected:
        // something must do before deleted
        void before_deleted() noexcept;
        // init
        void initialize(pugi::xml_node node) noexcept;
        // init as popup window
        void initialize(const Config::Popup& popup) noexcept;
        // destructor
        ~UIWindow() noexcept;
        // deleted 
        UIWindow(const UIWindow&) = delete; UIWindow() = delete;
    public: // some new
        // on close event
        virtual bool OnClose() noexcept;
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
        // set/update timer, must call killtimer it when control deleted
        void SetTimer(UIControl* ctrl, uint32_t time) noexcept { assert(ctrl); ::SetTimer(m_hwnd, UINT_PTR(ctrl), time, nullptr); }
        // kill timer
        void KillTimer(UIControl* ctrl) noexcept { assert(ctrl); ::KillTimer(m_hwnd, UINT_PTR(ctrl)); }
        // close window later
        void CloseWindowLater() noexcept { m_baBoolWindow.SetTrue(Index_SkipRender); ::PostMessageW(m_hwnd, WM_CLOSE, 0, 0); };
        // begin render
        void BeginDraw() const noexcept;
        // end ender
        void EndDraw() const noexcept;
        // for calling PreRender
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
        // set hover track control
        void SetHoverTrack(UIControl* ctrl) noexcept;
        // find control
        auto FindControl(const char* name) noexcept ->UIControl*;
        // find group, maybe return nullptr
        auto FildGroup(const char* name) noexcept ->ControlVector;
        // move window relative to parent
        void MoveWindow(float x, float y) noexcept;
        // add control with name
        void AddNamedControl(UIControl* ctrl) noexcept;
        // set icon, bad
        void SetIcon(HICON hIcon = nullptr) noexcept;
        // set mouse capture
        void SetCapture(UIControl* control) noexcept;
        // release mouse capture
        void ReleaseCapture() noexcept;
        // copystring for control in this winddow
        auto CopyString(const char* str) noexcept { return m_oStringAllocator.CopyString(str); }
        // copystring for control in this winddow in safe way
        auto CopyStringSafe(const char* str) noexcept { auto s = this->CopyString(str); return s ? s : ""; }
    public:
        // get text anti-mode 
        inline auto GetTextAntimode() const noexcept { return m_textAntiMode; }
        // get text anti-mode 
        inline void SetTextAntimode(D2D1_TEXT_ANTIALIAS_MODE mode) noexcept { m_textAntiMode = static_cast<decltype(m_textAntiMode)>(mode); }
        // get waite-vs event handle
        inline auto GetVSyncEvent() const noexcept { return m_hVSync; }
        // show window
        inline auto ShowWindow(int show = SW_SHOW) noexcept { return ::ShowWindow(m_hwnd, show); }
        // for calling PreRender with 3d content
        inline auto RegisterOffScreenRender3D(UIControl* c) noexcept { return this->RegisterOffScreenRender(c, true); }
        // for calling PreRender with 2d content
        inline auto RegisterOffScreenRender2D(UIControl* c) noexcept { return this->RegisterOffScreenRender(c, false); }
        // start render in sec.
        inline auto StartRender(float t, UIControl* c) noexcept { return m_uiRenderQueue.PlanToRender(0.f, t, c); }
        // plan to render in sec.
        inline auto PlanToRender(float w, float t, UIControl* c) noexcept { return m_uiRenderQueue.PlanToRender(w, t, c); }
        // update control later
        inline auto Invalidate(UIControl* c) noexcept { return m_uiRenderQueue.PlanToRender(0.f, 0.f, c); }
        // get window handle
        inline auto GetHwnd() const noexcept { return m_hwnd;};
        // is mouse captured control?
        inline auto IsCapturedControl(UIControl* c) noexcept { return m_pCapturedControl == c; };
        // get back buffer
        inline auto GetBackBuffer() noexcept { return LongUI::SafeAcquire(m_pTargetBimtap); }
        // is rendered
        inline auto IsRendered() const noexcept { return m_baBoolWindow.Test(UIWindow::Index_Rendered); }
        // is prerender, THIS METHOD COULD BE CALLED IN RENDER-THREAD ONLY
        inline auto IsPrerender() const noexcept { return m_baBoolWindow.Test(UIWindow::Index_Prerender); }
        // render window
        inline auto RenderWindow() const noexcept { if (m_baBoolWindow.Test(Index_SkipRender)) return; this->BeginDraw(); this->Render(); return this->EndDraw(); }
        // next frame
        inline auto NextFrame() noexcept { m_uiRenderQueue.GetCurrentUnit()->length = 0; ++m_uiRenderQueue; }
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
        UIWindow*       const   wndparent = nullptr;
        // window flag
        WindowFlag      const   wndflags = WindowFlag::Flag_None;
        // window type
        WindowType      const   wnd_type = WindowType::Type_Normal;
        // last mouse point
        D2D1_POINT_2F           last_point = D2D1::Point2F(-1.f, -1.f);
    protected:
        // next sibling
        UIWindow*               m_pNextSibling = nullptr;
        // 1st child
        UIWindow*               m_pFirstChild = nullptr;
        // string allocator
        StringAllocator         m_oStringAllocator;
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
        IDCompositionVisual*    m_pDcompVisual = nullptr;
        // now hover track control(only one)
        UIControl*              m_pHoverTracked = nullptr;
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
        // reset
        auto ResetCursor() noexcept { this->now_cursor = this->default_cursor; }
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
        ControlVector           m_vRegisteredControl;
        // control name ->map-> control pointer
        StringTable             m_hashStr2Ctrl;
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