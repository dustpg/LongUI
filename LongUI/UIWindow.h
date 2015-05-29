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


#define GETSIZEOF(x) ((char*)(&(x) + 1) - (char*)&(x))

// LongUI namespace
namespace LongUI{
    // ui's window
    class LongUIAPI UIWindow : public UIVerticalLayout,
        public ComStatic<QiListSelf<IUnknown, QiList<IDropTarget>>>{
        enum UnusedIndex {
            UNUSED_SIZE
        };
        // 父类申明
        using Super = UIVerticalLayout ;
        // 友元申明
        friend class CUIManager;
        // Rendering 渲染队列
        class RenderingQueue;
    public: // UIControl 接口实现
        // Render 渲染 --- 放在第一位!
        virtual auto LongUIMethodCall Render() noexcept ->HRESULT override;
        // do event 事件处理
        virtual bool LongUIMethodCall DoEvent(LongUI::EventArgument&) noexcept override;
        // 预渲染
        virtual void LongUIMethodCall PreRender() noexcept override { /*return Super::PreRender();*/ }
        // recreate 重建
        virtual auto LongUIMethodCall Recreate(LongUIRenderTarget*) noexcept ->HRESULT override;
        // close this control 关闭控件
        virtual void LongUIMethodCall Close() noexcept override;
    public: // IDropTarget 接口 实现
        // IDropTarget::DragEnter 实现
        HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj,DWORD grfKeyState, POINTL pt,DWORD *pdwEffect) noexcept override;
        // IDropTarget::DragOver 实现
        HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) noexcept override;
        // IDropTarget::DragLeave 实现
        HRESULT STDMETHODCALLTYPE DragLeave(void) noexcept override;
        // IDropTarget::Drop 实现
        HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) noexcept override;
    public:
        // update rendering state
        bool UpdateRendering() noexcept;
        // resize window
        void OnResize(bool force=false) noexcept;
        // plan to render
        void PlanToRender(float w, float r, UIControl* obj) noexcept;
        // register for calling PreRender
        void RegisterPreRender(UIControl* c, bool is3d) noexcept;
        // unregister for calling PreRender
        void UnRegisterPreRender(UIControl*) noexcept;
        // set the caret
        void SetCaretPos(UIControl* c, float x, float y) noexcept;
        // create the caret
        void CreateCaret(float width, float height) noexcept;
        // show the caret
        void ShowCaret() noexcept;
        // hide the caret
        void HideCaret() noexcept;
    public: // 内联区
        // register for calling PreRender with 3d content
        LongUIInline auto RegisterPreRender3D(UIControl* c) noexcept { return this->RegisterPreRender(c, true); }
        // register for calling PreRender with 2d content
        LongUIInline auto RegisterPreRender2D(UIControl* c) noexcept { return this->RegisterPreRender(c, false); }
        // start render in sec.
        LongUIInline auto StartRender(float t, UIControl* c) noexcept { return this->PlanToRender(0.f, t, c); }
        // update control later
        LongUIInline auto Invalidate(UIControl* c) noexcept { return this->PlanToRender(0.f, 0.f, c); }
        // get delta time in second
        LongUIInline auto GetDeltaTime() const noexcept { return m_fDeltaTime > 0.04f ? 0.f : m_fDeltaTime; };
        // get window handle
        LongUIInline auto GetHwnd() const noexcept{ return m_hwnd;};
        // set mouse capture
        LongUIInline auto SetCapture(UIControl* c) noexcept { ::SetCapture(m_hwnd); m_pCapturedControl = c; };
        // release mouse capture
        LongUIInline auto ReleaseCapture() noexcept { ::ReleaseCapture(); m_pCapturedControl = nullptr; };
        // get back buffer
        LongUIInline auto GetBackBuffer() noexcept { return ::SafeAcquire(m_pTargetBimtap); }
    private:
        // release data
        void LongUIMethodCall release_data() noexcept;
        // reset rendering queue
        void LongUIMethodCall reset_renderqueue() noexcept;
        // draw the caret
        void LongUIMethodCall draw_caret() noexcept;
        // refresh the caret
        void LongUIMethodCall refresh_caret() noexcept;
        // set the present
        void LongUIMethodCall set_present() noexcept;
    public:
        // begin draw
        void LongUIMethodCall BeginDraw() noexcept;
        // end draw
        auto LongUIMethodCall EndDraw(uint32_t vsyc = 0) noexcept->HRESULT;
        // find control where mouse pointed
        auto LongUIMethodCall FindControl(const LongUI::EventArgument& _arg) noexcept{
            auto arg = _arg;
            arg.sender = this;
            arg.event = LongUI::Event::Event_FindControl;
            this->UIWindow::DoEvent(arg);
            return arg.ctrl;
        }
        // find control where mouse pointed
        auto LongUIMethodCall FindControl(D2D1_POINT_2F pt) noexcept {
            LongUI::EventArgument arg;
            arg.ctrl = nullptr;
            arg.pt = pt;
            arg.sender = this;
            arg.event = LongUI::Event::Event_FindControl;
            this->UIWindow::DoEvent(arg);
            return arg.ctrl;
        }
    protected:
        // constructor
        UIWindow(pugi::xml_node node, UIWindow * parent=nullptr) noexcept;
        // destructor
        ~UIWindow() noexcept;
        // deleted 
        UIWindow(const UIWindow&) = delete; UIWindow() = delete;
        // window handle
        HWND                    m_hwnd = nullptr;
        // 创建数据对象
        IDataObject*            m_pCurDataObject = nullptr;
        // 拖放帮助工具
        IDropTargetHelper*      m_pDropTargetHelper = nullptr;
        // swap chain 交换链
        IDXGISwapChain1*        m_pSwapChain = nullptr;
        // target bitmap
        ID2D1Bitmap1*           m_pTargetBimtap = nullptr;
        // planning bitmap
        ID2D1Bitmap1*           m_pBitmapPlanning = nullptr;
        // Direct Composition Device
        IDCompositionDevice*    m_pDcompDevice = nullptr;
        // Direct Composition Target
        IDCompositionTarget*    m_pDcompTarget = nullptr;
        // Direct Composition Visual
        IDCompositionVisual*    m_pDcompVisual = nullptr;
        // mouse point control (only one)
        UIControl*              m_pPointedControl = nullptr;
        // now focused control (only one)
        UIControl*              m_pFocusedControl = nullptr;
        // now dragdrop control (only one)
        UIControl*              m_pDragDropControl = nullptr;
        // now captured control (only one)
        UIControl*              m_pCapturedControl = nullptr;
        // rendering queue
        RenderingQueue*         m_pRenderQueue = nullptr;
        // blink timer id
        UINT_PTR                m_idBlinkTimer = 0;
        // normal  l-param
        LPARAM                  m_normalLParam = 0;
        // caret in(true) or out?
        bool                    m_bCaretIn = false;
        // in draging
        bool                    m_bInDraging = false;
        //
        bool                    window_unused[2];
        // scroll rect
        RECT                    m_rcScroll;
        // dirty rects, + 1 for caret
        RECT                    m_dirtyRects[LongUIDirtyControlSize + 1];
        // present prame
        DXGI_PRESENT_PARAMETERS m_present;
        // show the caret
        uint32_t                m_cShowCaret = 0;
        // delta time [in sec.]
        float                   m_fDeltaTime = 0.f;
        // continuous render time [in sec.]
        float                   m_fConRenderTime = 0.f;
        // 
        float                   m_uuuuuu = 0.f;
        // caret rect in px
        RectLTWH_U              m_rcCaretPx;
        // clientsize
        D2D1_SIZE_F             m_clientSize = D2D1::SizeF();
        // windowsize
        D2D1_SIZE_F             m_windowSize = D2D1::SizeF();
        // timer
        UITimer                 m_timer;
        // track mouse event
        TRACKMOUSEEVENT         m_csTME;
        // current STGMEDIUM当前媒体
        STGMEDIUM               m_curMedium;
    public:
        // default(arrow)cursor
        HCURSOR     const       default_cursor = ::LoadCursorW(nullptr, IDC_ARROW);
        // now sursor
        HCURSOR                 now_cursor = default_cursor;
#ifdef _DEBUG
        BOOL                    test_D2DERR_RECREATE_TARGET = false;
        uint16_t                full_render_counter = 0;
        uint16_t                dirty_render_counter = 0;
#endif
        // 清理颜色
        D2D1::ColorF            clear_color = D2D1::ColorF(D2D1::ColorF::White);
    protected:
        // registered control
        BasicContainer          m_vRegisteredControl;
    };
}