// LUI
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <util/ui_ctordtor.h>
#include <input/ui_kminput.h>
#include <util/ui_time_meter.h>
#include <control/ui_viewport.h>
#include <core/ui_platform_win.h>
#include <util/ui_color_system.h>
#include <graphics/ui_graphics_impl.h>
#include <dropdrag/ui_dropdrag_impl.h>
// C/C++
#include <cassert>
#include <algorithm>
#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_win.h>
#endif
#ifndef LUI_NO_DROPDRAG
#include <Shobjidl.h>
#endif


// LongUI::impl
namespace LongUI { namespace impl {
    // window style
    enum style : DWORD {
        windowed         = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
        aero_borderless  = WS_POPUP            | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
        basic_borderless = WS_POPUP            | WS_THICKFRAME              | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX
    };
    // delete later
    enum msg : uint32_t {
        msg_custom = WM_USER + 10,
        msg_post_destroy,
        msg_post_set_title,
    };
    // 获取透明窗口适合的大小
    inline auto get_fit_size_for_trans(uint32_t len) noexcept {
        constexpr uint32_t UNIT = TRANSPARENT_WIN_BUFFER_UNIT;
        return static_cast<uint32_t>(len + UNIT - 1) / UNIT * UNIT;
    }
    // get dpi scale
    auto get_dpi_scale_from_hwnd(HWND hwnd) noexcept->Size2F;
}}

// ui namespace
namespace LongUI {
    // mouse event map(use char to avoid unnecessary memory-waste)
    const uint8_t MEMAP[] = {
        // WM_MOUSEMOVE                 0x0200
        static_cast<uint8_t>(MouseEvent::Event_MouseMove),
        // WM_LBUTTONDOWN               0x0201
        static_cast<uint8_t>(MouseEvent::Event_LButtonDown),
        // WM_LBUTTONUP                 0x0202
        static_cast<uint8_t>(MouseEvent::Event_LButtonUp),
        // WM_LBUTTONDBLCLK             0x0203
        static_cast<uint8_t>(MouseEvent::Event_Unknown),
        // WM_RBUTTONDOWN               0x0204
        static_cast<uint8_t>(MouseEvent::Event_RButtonDown),
        // WM_RBUTTONUP                 0x0205
        static_cast<uint8_t>(MouseEvent::Event_RButtonUp),
        // WM_RBUTTONDBLCLK             0x0206
        static_cast<uint8_t>(MouseEvent::Event_Unknown),
        // WM_MBUTTONDOWN               0x0207
        static_cast<uint8_t>(MouseEvent::Event_MButtonDown),
        // WM_MBUTTONUP                 0x0208
        static_cast<uint8_t>(MouseEvent::Event_MButtonUp),
        // WM_MBUTTONDBLCLK             0x0209
        static_cast<uint8_t>(MouseEvent::Event_Unknown),
        // WM_MOUSEWHEEL                0x020A
        static_cast<uint8_t>(MouseEvent::Event_MouseWheelV),
        // WM_XBUTTONDOWN               0x020B
        static_cast<uint8_t>(MouseEvent::Event_Unknown),
        // WM_XBUTTONUP                 0x020C
        static_cast<uint8_t>(MouseEvent::Event_Unknown),
        // WM_XBUTTONDBLCLK             0x020D
        static_cast<uint8_t>(MouseEvent::Event_Unknown),
        // WM_MOUSEHWHEEL               0x020E
        static_cast<uint8_t>(MouseEvent::Event_MouseWheelH),
    };
    /// <summary>
    /// Gets the size of the client.
    /// </summary>
    /// <param name="hwnd">The HWND.</param>
    /// <returns></returns>
    inline auto GetClientSize(HWND hwnd) noexcept -> Size2U {
        RECT client_rect; ::GetClientRect(hwnd, &client_rect);
        return {
            uint32_t(client_rect.right - client_rect.left),
            uint32_t(client_rect.bottom - client_rect.top)
        };
    }
#ifndef LUI_NO_DROPDRAG
    /// <summary>
    /// impl for I::DropTarget
    /// </summary>
    struct CUIDropTargetImpl final : I::DropTarget {
    public: // IUnkown
        // add ref-count
        ULONG UNICALL AddRef() noexcept { return 2; };
        // release ref-count
        ULONG UNICALL Release() noexcept { return 1; };
        // query the interface
        HRESULT UNICALL QueryInterface(const IID& id, void **ppvObject) noexcept override {
            IUnknown* ptr = nullptr;
            if (id == IID_IUnknown) ptr = static_cast<IUnknown*>(this);
            else if (id == IID_IDropTarget) ptr = static_cast<IDropTarget*>(this);
            if (*ppvObject = ptr) { this->AddRef(); return S_OK; }
            return E_NOINTERFACE;
        }
    public:
        // drag enter
        HRESULT UNICALL DragEnter(
            IDataObject* pDataObj,
            DWORD grfKeyState,
            POINTL pt,
            DWORD * pdwEffect) noexcept override {
            // 检查参数
            if (!pDataObj) return E_INVALIDARG;
            //POINT ppt = { pt.x, pt.y };
            //auto& helper = UIManager.RefDropTargetHelper();
            //helper.DragEnter(hwnd, pDataObj, &ppt, *pdwEffect);
            return S_OK;
        }
        // drag over
        HRESULT UNICALL DragOver(
            DWORD grfKeyState,
            POINTL pt,
            DWORD* pdwEffect) noexcept override {
            UNREFERENCED_PARAMETER(grfKeyState);
            // TODO:
            if (false) {
                /*LongUI::EventArgument arg;
                ::SetLongUIEventArgument(arg, m_hWnd, pt);
                arg.sender = this;
                // 第一个控件?
                if (m_pDragDropControl == control) {
                    // 一样就是Over
                    arg.event = LongUI::Event::Event_DragOver;
                }
                else {
                    // 对老控件发送离开事件
                    if (m_pDragDropControl) {
                        arg.event = LongUI::Event::Event_DragLeave;
                        m_pDragDropControl->DoEvent(arg);
                    }
                    // 新控件发送进入
                    arg.event = LongUI::Event::Event_DragEnter;
                    m_pDragDropControl = control;
                }
                arg.cf.dataobj = m_pCurDataObject;
                arg.cf.outeffect = pdwEffect;
                if (!control->DoEvent(arg)) *pdwEffect = DROPEFFECT_NONE;*/
            }
            else {
                // 不支持
                *pdwEffect = DROPEFFECT_NONE;
            }
            // 由帮助器处理
            //auto& helper = UIManager.RefDropTargetHelper();
            //helper.DragOver(&ppt, *pdwEffect);
            return S_OK;
        }
        // drag leave
        HRESULT UNICALL DragLeave(void)  noexcept override {
            // 发送事件
            //if (m_pDragDropControl) {
                /*LongUI::EventArgument arg = { 0 };
                arg.sender = this;
                arg.event = LongUI::Event::Event_DragLeave;
                m_pDragDropControl->DoEvent(arg);
                m_pDragDropControl = nullptr;
                // 存在捕获控件?
                /*if (m_pCapturedControl) {
                    this->ReleaseCapture();
                    /*arg.sender = nullptr;
                    arg.msg = WM_LBUTTONUP;
                    m_pCapturedControl->DoEvent(arg);
                }*/
            //}
            /*OnDragLeave(m_hTargetWnd);*/
            //m_pDragDropControl = nullptr;
            //m_isDataAvailable = TRUE;
            // 由帮助器处理
            auto& helper = UIManager.RefDropTargetHelper();
            return helper.DragLeave();
        }
        // drop
        HRESULT UNICALL Drop(
            IDataObject* pDataObj, 
            DWORD grfKeyState, 
            POINTL pt, 
            DWORD* pdwEffect) noexcept override {
            // 发送事件
            //if (m_pDragDropControl) {
                /*LongUI::EventArgument arg;
                ::SetLongUIEventArgument(arg, m_hWnd, pt);
                arg.sender = this;
                arg.event = LongUI::Event::Event_Drop;
                arg.cf.dataobj = m_pCurDataObject;
                arg.cf.outeffect = pdwEffect;
                // 发送事件
                m_pDragDropControl->DoEvent(arg);
                m_pDragDropControl = nullptr;*/
                //}
                // 检查参数
            if (!pDataObj) return E_INVALIDARG;
            // 由帮助器处理
            //auto& helper = UIManager.RefDropTargetHelper();
            //helper.Drop(pDataObj, &ppt, *pdwEffect);
            //*pdwEffect = ::GetDropEffect(grfKeyState, *pdwEffect);
            return S_OK;
        }
    };
#endif
}

/// <summary>
/// Destroy the window
/// </summary>
/// <param name="hwnd"></param>
/// <returns></returns>
void LongUI::CUIPlatformWin::Destroy(HWND hwnd) noexcept {
    //LUIDebug(Hint) LUI_FRAMEID << hwnd << endl;
#ifndef LUI_NO_DROPDRAG
    //清理 DropDrag
    ::RevokeDragDrop(hwnd);
#endif
    // 尝试直接摧毁
    if (::DestroyWindow(hwnd)) return;
    // 不在消息线程就用 PostMessage
#ifndef NDEBUG
    const auto laste = ::GetLastError();
    assert(laste == ERROR_ACCESS_DENIED && "check the code");
#endif
    ::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, LONG_PTR(PrcoNull));
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(0));
    const auto code = ::PostMessageW(hwnd, impl::msg_post_destroy, 0, 0);
    const auto ec = ::GetLastError();
    assert(code && "PostMessage failed");
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
LongUI::CUIPlatformWin::CUIPlatformWin() noexcept :
    m_uTextAntialias(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT) {
#ifndef LUI_NO_DROPDRAG
    static_assert(sizeof(m_oDropTargetImpl) >= sizeof(CUIDropTargetImpl), "buffer not safe");
    impl::ctor_dtor<CUIDropTargetImpl>::create_obj(&m_oDropTargetImpl);
#endif
    m_bFlagSized = false;
    m_bMouseEnter = false;
    m_bDcompSupport = false;
    m_bAccessibility = false;
    m_bMovingResizing = false;
    m_bMouseLeftDown = false;
    m_bSystemSkipRendering = false;
    m_bLayeredWindowSupport = false;
    impl::init_dcomp(m_dcomp);
    // TODO: 默认居中
    this->rect.width = LongUI::DEFAULT_WINDOW_WIDTH;
    this->rect.height = LongUI::DEFAULT_WINDOW_HEIGHT;
    const auto scw = ::GetSystemMetrics(SM_CXFULLSCREEN);
    const auto sch = ::GetSystemMetrics(SM_CYFULLSCREEN);
    this->rect.left = (scw - this->rect.width) / 2;
    this->rect.top = (sch - this->rect.height) / 2;
    // 子像素渲染?
    if (UIManager.flag & ConfigureFlag::Flag_SubpixelTextRenderingAsDefault)
        m_uTextAntialias = D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
}

#if 0
/// <summary>
/// Initializes the window position.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::InitWindowPos() noexcept {
    auto& winrect = this->rect;
    // 默认大小
    winrect.width = LongUI::DEFAULT_WINDOW_WIDTH;
    winrect.height = LongUI::DEFAULT_WINDOW_HEIGHT;
    // 默认居中
    const auto scw = ::GetSystemMetrics(SM_CXFULLSCREEN);
    const auto sch = ::GetSystemMetrics(SM_CYFULLSCREEN);
    winrect.left = (scw - winrect.width) / 2;
    winrect.top = (sch - winrect.height) / 2;
}
#endif

/// <summary>
/// 
/// </summary>
/// <returns></returns>
LongUI::CUIPlatformWin::~CUIPlatformWin() noexcept {
    // 卸载资源
    this->ReleaseDeviceData();
    // 摧毁窗口
    CUIPlatformWin::Destroy(m_hWnd);
}



/// <summary>
/// Registers the window class.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::register_class() noexcept {
    WNDCLASSEXW wcex;
    const auto ins = ::GetModuleHandleW(nullptr);
    // 已经注册过了
    if (::GetClassInfoExW(ins, Attribute::WindowClassNameN, &wcex)) return;
    // 注册一般窗口类
    wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = 0;
    wcex.lpfnWndProc = CUIPlatformWin::PrcoNull;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(void*);
    wcex.hInstance = ins;
    wcex.hCursor = nullptr;
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = Attribute::WindowClassNameN;
    const size_t icon_name_id = WindowIconName;
    const auto icon_name = reinterpret_cast<const wchar_t*>(icon_name_id);
    wcex.hIcon = ::LoadIconW(ins, icon_name);
    ::RegisterClassExW(&wcex);
    // 注册弹出窗口类
    wcex.style = CS_DROPSHADOW;
    wcex.lpszClassName = Attribute::WindowClassNameP;
    ::RegisterClassExW(&wcex);
}

/// <summary>
/// 初始化
/// </summary>
/// <param name="hwnd"></param>
/// <returns></returns>
void LongUI::CUIPlatformWin::OnCreate(HWND hwnd) noexcept {
}

/// <summary>
/// 处理函数 - Null指针处理
/// </summary>
/// <param name="hwnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT LongUI::CUIPlatformWin::PrcoNull(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    switch (message)
    {
        BOOL rc;
    case WM_CREATE:
    {
        // 获取指针
        const auto ptr = (reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams;
        const auto win = reinterpret_cast<CUIPlatformWin*>(ptr);
        // 设置窗口指针
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(win));
        ::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, LONG_PTR(PrcoReal));
        // 创建完毕
        win->OnCreate(hwnd);
        // 返回1
        return 1;
    }
    case impl::msg_post_destroy:
        rc = ::DestroyWindow(hwnd);
        assert(rc && "DestroyWindow failed");
        return 0;
#ifdef LUI_ACCESSIBLE
    case WM_DESTROY:
        ::UiaReturnRawElementProvider(hwnd, 0, 0, nullptr);
        break;
#endif
    }
    // 未处理消息
    return ::DefWindowProcW(hwnd, message, wParam, lParam);
}

/// <summary>
/// 处理函数 - 正常处理
/// </summary>
/// <param name="hwnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT LongUI::CUIPlatformWin::PrcoReal(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    // 获取储存的指针
    const auto lptr = ::GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    const auto window = reinterpret_cast<CUIPlatformWin*>(lptr);
    assert(window && "window cannot be null on PrcoReal");
    return window->DoMsg(hwnd, message, wParam, lParam);
};



/// <summary>
/// enable the window
/// </summary>
/// <param name="enable"></param>
/// <returns></returns>
void LongUI::CUIPlatformWin::EnableWindow(bool enable) noexcept {
    ::EnableWindow(m_hWnd, enable);
}

/// <summary>
/// active window(the focus to window self)
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::ActiveWindow() noexcept {
    ::SetActiveWindow(m_hWnd);
}

/// <summary>
/// 释放对象
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::Dispose() noexcept {
    // 直接调用析构函数
    this->~CUIPlatformWin();
}

/// <summary>
/// release device data
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::ReleaseDeviceData() noexcept {
    LongUI::SafeRelease(m_pBitmap);
    LongUI::SafeRelease(m_pSwapchan);
    impl::release_dcomp(m_dcomp);
}


/// <summary>
/// Renders the caret.
/// </summary>
/// <param name="renderer">The renderer.</param>
/// <returns></returns>
void LongUI::CUIPlatform::basic_caret(I::Renderer2D& renderer) const noexcept {
    // TODO: 脏矩形渲染时候可能没有必要渲染?
    // 渲染插入符号
    if (this->careted /*&& this->caret_ok*/) {
        // 保持插入符号的清晰
        const auto visible = this->careted->RefBox().visible;
        renderer.PushAxisAlignedClip(auto_cast(visible), D2D1_ANTIALIAS_MODE_ALIASED);
#if 0
        // 反色操作, 但是较消耗GPU资源
        renderer.DrawImage(
            &img->RefBitmap(),
            auto_cast(reinterpret_cast<Point2F*>(&des)), &auto_cast(rc),
            D2D1_INTERPOLATION_MODE_LINEAR,
            //D2D1_COMPOSITE_MODE_SOURCE_OVER
            D2D1_COMPOSITE_MODE_MASK_INVERT
        );
#endif
        auto rect = this->caret;
        this->careted->MapToWindow(rect);
        auto& brush = UIManager.RefCCBrush(this->caret_color);
        renderer.FillRectangle(auto_cast(rect), &brush);
        renderer.PopAxisAlignedClip();
    }
}



/// <summary>
/// Renders the focus.
/// </summary>
/// <param name="renderer">The renderer.</param>
/// <returns></returns>
void LongUI::CUIPlatform::basic_focus(I::Renderer2D& renderer) const noexcept {
#ifdef LUI_DRAW_FOCUS_RECT
    // TODO: 脏矩形渲染时候可能没有必要渲染?
    // 渲染焦点矩形
    if (this->draw_focus && this->focused) {
        // 脏矩形符号的清晰
        const auto visible = this->focused->RefBox().visible;
        renderer.PushAxisAlignedClip(auto_cast(visible), D2D1_ANTIALIAS_MODE_ALIASED);
        auto rect = this->foucs;
        this->focused->MapToWindow(rect);
        UIManager.RefNativeStyle().DrawFocus(rect);
        renderer.PopAxisAlignedClip();
    }
#endif
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIPlatformWin::Render() noexcept->Result {
    // 跳过该帧?
    if (this->is_skip_render()) return { Result::RS_OK };
    // 请求渲染
    if (this->is_r_for_render()) {
        // 重置窗口缓冲帧大小
        if (m_bFlagSized) this->resize_window_buffer();
        // 数据无效
        if (!m_pBitmap) return { Result::RS_FALSE };
        // 开始渲染
        this->begin_render();
        // 矩形列表
        const auto rects = this->dirty_rect_presenting;
        const auto count = this->is_fr_for_render() ? 0 : this->dirty_count_presenting;
        // 正式渲染
        CUIControlControl::RecursiveRender(*viewport(), rects, count);
#if 0
        //if (this->viewport()->GetWindow()->GetParent())
        LUIDebug(Log) << "End of Frame" << this << endl;
#endif
        // 结束渲染
        return this->end_render();
    }
    return{ Result::RS_OK };
}

/// <summary>
/// Resizes the window buffer.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::resize_window_buffer() noexcept {
    m_bFlagSized = false;
    assert(this->rect.width && this->rect.height);
    // 检测是否无需修改
    const auto& olds = this->m_szWndbufLogical;
    if (olds.width == this->rect.width && olds.height == this->rect.height) return;
    // 更新逻辑大小
    m_szWndbufLogical.width = this->rect.width;
    m_szWndbufLogical.height = this->rect.height;
    // 渲染区
    //CUIRenderAutoLocker locker;
    // 取消引用
    //UIManager.Ref2DRenderer().SetTarget(nullptr);
    // 强行重置flag
    bool force_resize = true;
    // 目标大小
    auto target = m_szWndbufLogical;
    // DComp支持
    if (this->is_direct_composition()) {
        const auto old_realx = impl::get_fit_size_for_trans(olds.width);
        const auto old_realy = impl::get_fit_size_for_trans(olds.height);
        target.width = impl::get_fit_size_for_trans(target.width);
        target.height = impl::get_fit_size_for_trans(target.height);
        // 透明窗口只需要比实际大小大就行
        if (old_realx == target.width && old_realy == target.height)
            force_resize = false;
    }
    // 重置缓冲帧大小
    if (force_resize) {
        IDXGISurface* dxgibuffer = nullptr;
        LongUI::SafeRelease(m_pBitmap);
        // TODO: 延迟等待
        Result hr = { m_pSwapchan->ResizeBuffers(
            2, target.width, target.height,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            // DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
            0
        ) };
        longui_debug_hr(hr, L"m_pSwapChain->ResizeBuffers faild");
        // TODO: RecreateResources 检查
        if (hr.code == DXGI_ERROR_DEVICE_REMOVED ||
            hr.code == DXGI_ERROR_DEVICE_RESET) {
            //UIManager.RecreateResources();
            LUIDebug(Hint) << L"Recreate device" << LongUI::endl;
        }
        // 利用交换链获取Dxgi表面
        if (hr) {
            hr = { m_pSwapchan->GetBuffer(
                0,
                IID_IDXGISurface,
                reinterpret_cast<void**>(&dxgibuffer)
            ) };
            longui_debug_hr(hr, L"m_pSwapChain->GetBuffer faild");
        }
        // 利用Dxgi表面创建位图
        if (hr) {
            D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
            );
            ID2D1Bitmap1* bmp = nullptr;
            hr = { UIManager.Ref2DRenderer().CreateBitmapFromDxgiSurface(
                dxgibuffer,
                &bitmapProperties,
                &bmp
            ) };
            m_pBitmap = static_cast<I::Bitmap*>(bmp);
            longui_debug_hr(hr, L"UIManager_RenderTarget.CreateBitmapFromDxgiSurface faild");
        }
        // 重建失败?
        LongUI::SafeRelease(dxgibuffer);
#ifndef NDEBUG
        LUIDebug(Hint)
            << "resize to["
            << target.width
            << 'x'
            << target.height
            << ']'
            << LongUI::endl;
#endif // !NDEBUG

        // TODO: 错误处理
        if (!hr) {
            LUIDebug(Error) << L" Recreate FAILED!" << LongUI::endl;
            UIManager.OnErrorInfoLost(hr, Occasion_Resizing);
        }
    }
}

/// <summary>
/// Begins the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::begin_render() noexcept {
    auto& renderer = UIManager.Ref2DRenderer();
    // 设置文本渲染策略
    renderer.SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE(m_uTextAntialias));
    // 设为当前渲染对象
    renderer.SetTarget(m_pBitmap);
    // 开始渲染
    renderer.BeginDraw();
    // 设置转换矩阵
#if 1
    renderer.SetTransform(D2D1::Matrix3x2F::Identity());
#else
    renderer.SetTransform(&impl::d2d(m_pViewport->box.world));
#endif
    // 清空背景
    renderer.Clear(auto_cast(clear_color));
}


/// <summary>
/// Ends the render.
/// </summary>
/// <returns></returns>
auto LongUI::CUIPlatformWin::end_render() noexcept->Result {
    auto& renderer = UIManager.Ref2DRenderer();
    renderer.SetTransform({ 1.f,0.f,0.f,1.f,0.f,0.f });
    // 渲染插入符号
    this->basic_caret(renderer);
    // 渲染焦点矩形
    this->basic_focus(renderer);
#ifndef NDEBUG
    // 显示
    if (UIManager.flag & ConfigureFlag::Flag_DbgDrawDirtyRect) {
        // 水平扫描线: 全局渲染
        if (this->is_fr_for_render()) {
            assert(this->rect.height);
            const float y = float(dbg_full_render_counter % this->rect.height);
            renderer.PushAxisAlignedClip(
                D2D1_RECT_F{ 0, y, float(rect.width), y + 1 },
                D2D1_ANTIALIAS_MODE_ALIASED
            );
            renderer.Clear(D2D1::ColorF{ 0x000000 });
            renderer.PopAxisAlignedClip();
        }
        // 随机颜色方格: 增量渲染
        else {
            static float s_color_value = 0.f;
            constexpr float s_color_step = 0.005f * 360.f;
            ColorSystem::HSLA hsl;
            hsl.a = 0.5f; hsl.s = 1.f; hsl.l = 0.36f; hsl.h = s_color_value;
            // 遍历脏矩形
            std::for_each(
                this->dirty_rect_presenting,
                this->dirty_rect_presenting + this->dirty_count_presenting,
                [&](const RectF& rect) noexcept {
                auto& bursh = CUIManager::RefCCBrush(hsl.toRGBA());
                renderer.FillRectangle(auto_cast(rect), &bursh);
                hsl.h += s_color_step;
                if (hsl.h > 360.f) hsl.h = 0.f;
            });
            s_color_value = hsl.h;
        }
    }
#endif
    // 结束渲染
    Result hr = { renderer.EndDraw() };
    // 清除目标
    renderer.SetTarget(nullptr);
    // TODO: 完全渲染
    if (this->is_fr_for_render()) {
#ifndef NDEBUG
        CUITimeMeterH meter;
        meter.Start();
        hr = { m_pSwapchan->Present(0, 0) };
        const auto time = meter.Delta_ms<float>();
        if (time > 9.f) {
            LUIDebug(Hint)
                << "present time: "
                << time
                << "ms"
                << LongUI::endl;
        }
        //assert(time < 1000.f && "took too much time, check plz.");
#else
        hr = { this->swapchan->Present(0, 0) };
#endif
        longui_debug_hr(hr, L"swapchan->Present full rendering faild");
    }
    // 增量渲染
    else {
        // 呈现参数设置
        RECT scroll = { 0, 0, m_szWndbufLogical.width, m_szWndbufLogical.height };
        RECT rects[LongUI::DIRTY_RECT_COUNT];
        // 转换为整型
        const auto count = this->dirty_count_presenting;
        for (uint32_t i = 0; i != count; ++i) {
            const auto& src = this->dirty_rect_presenting[i];
            const auto right = static_cast<LONG>(std::ceil(src.right));
            const auto bottom = static_cast<LONG>(std::ceil(src.bottom));
            // 写入
            rects[i].top = std::max(static_cast<LONG>(src.top), 0l);
            rects[i].left = std::max(static_cast<LONG>(src.left), 0l);
            rects[i].right = std::min(right, scroll.right);
            rects[i].bottom = std::min(bottom, scroll.bottom);
        }
        // 填充参数
        DXGI_PRESENT_PARAMETERS present_parameters;
        present_parameters.DirtyRectsCount = count;
        present_parameters.pDirtyRects = rects;
        present_parameters.pScrollRect = &scroll;
        present_parameters.pScrollOffset = nullptr;
        // 提交该帧
#ifndef NDEBUG
        CUITimeMeterH meter;
        meter.Start();
        hr = {  m_pSwapchan->Present1(0, 0, &present_parameters) };
        const auto time = meter.Delta_ms<float>();
        if (time > 9.f) {
            LUIDebug(Hint)
                << "present time: "
                << time
                << "ms"
                << LongUI::endl;
        }
        //assert(time < 1000.f && "took too much time, check plz.");
#else
        hr = { this->swapchan->Present1(0, 0, &present_parameters) };
#endif
        longui_debug_hr(hr, L"swapchan->Present full rendering faild");
    }
    // 收到重建消息/设备丢失时 重建UI
    constexpr int32_t DREMOVED = DXGI_ERROR_DEVICE_REMOVED;
    constexpr int32_t DRESET = DXGI_ERROR_DEVICE_RESET;
#ifdef NDEBUG
    if (hr.code == DREMOVED || hr.code == DRESET) {
        UIManager.NeedRecreate();
        hr = { Result::RS_FALSE };
    }
#else
    // TODO: 测试 test_D2DERR_RECREATE_TARGET
    if (hr.code == DREMOVED || hr.code == DRESET) {
        UIManager.NeedRecreate();
        hr = { Result::RS_FALSE };
        LUIDebug(Hint) << "D2DERR_RECREATE_TARGET!" << LongUI::endl;
    }
    assert(hr);
    // 调试
    if (this->is_fr_for_render())
        ++const_cast<uint32_t&>(dbg_full_render_counter);
    else
        ++const_cast<uint32_t&>(dbg_dirty_render_counter);
    // 更新调试信息
    wchar_t buffer[1024];
    std::swprintf(
        buffer, 1024,
        L"<%ls>: FRC: %d\nDRC: %d\nDRRC: %d",
        L"NAMELESS",
        int(dbg_full_render_counter),
        int(dbg_dirty_render_counter),
        int(0)
    );
    // 设置显示
    /*UIManager.RenderUnlock();
    this->dbg_uiwindow->SetTitleName(buffer);
    UIManager.RenderLock();*/
#endif
    return hr;
}



/// <summary>
/// Initializes this instance.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="flag">The configuration.</param>
/// <returns></returns>
void LongUI::CUIPlatformWin::Init(CUIWindow* parent, uint16_t flag) noexcept {
    // 有了
    if (m_hWnd) return;
    // 尝试注册
    this->register_class();
    const HWND parent_hwnd = parent ? reinterpret_cast<HWND>(parent->GetRawHandle()) : nullptr;
    // 初始化
    HWND hwnd = nullptr;
    const auto config = static_cast<CUIWindow::WindowConfig>(flag);
    static_assert(sizeof(config) == sizeof(flag), "same!");
    // 分层窗口支持
    if ((config & CUIWindow::Config_LayeredWindow)) {
        m_bLayeredWindowSupport = true;
        m_bDcompSupport = impl::check_dcomp_support();
    }
    // 窗口
    {
        // 检查样式样式
        const auto style = [config]() noexcept -> DWORD {
            if (config & CUIWindow::Config_Popup)
                return WS_POPUPWINDOW;
            DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
            if (!(config & CUIWindow::Config_FixedSize))
                style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
            return style;
        }();
        // MA返回码
        m_uMaRevalue = config & CUIWindow::Config_Popup ? MA_NOACTIVATE : MA_ACTIVATE;
        // 调整大小
        static_assert(sizeof(RectL) == sizeof(RECT), "SAME!");
        auto& adjusted = reinterpret_cast<RECT&>(this->adjust);
        ::AdjustWindowRect(&adjusted, style, FALSE);
        // 窗口
        RectWHL window_rect;
        window_rect.left = this->rect.left;
        window_rect.top = this->rect.top;
        window_rect.width = this->rect.width + this->adjust.right - this->adjust.left;
        window_rect.height = this->rect.height + this->adjust.bottom - this->adjust.top;
        // 针对this->rect清空为-1, 否则会检查到未修改
        this->rect = { -1, -1, -1, -1 };
        // 额外
        uint32_t ex_flag = 0;
        // DirectComposition 支持
        if (this->is_direct_composition())
            ex_flag |= WS_EX_NOREDIRECTIONBITMAP;
        // WS_EX_TOOLWINDOW 不会让窗口显示在任务栏
        if (config & (CUIWindow::Config_ToolWindow | CUIWindow::Config_Popup))
            ex_flag |= WS_EX_TOOLWINDOW;
        // 创建窗口
        //::Sleep(1000);
        hwnd = ::CreateWindowExW(
            //WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            ex_flag,
            (config & CUIWindow::Config_Popup) ?
            Attribute::WindowClassNameP : Attribute::WindowClassNameN,
            impl::sys(this->titlename.c_str()),
            style,
            window_rect.left, window_rect.top,
            window_rect.width, window_rect.height,
            // 弹出窗口使用NUL父窗口方便显示
            config & CUIWindow::Config_Popup ? nullptr : parent_hwnd,
            nullptr,
            ::GetModuleHandleA(nullptr),
            this
        );
    }
#ifndef LUI_NO_DROPDRAG
    // 对于非弹出窗口注册拖放
    if (!(config & CUIWindow::Config_Popup)) 
        ::RegisterDragDrop(hwnd, &reinterpret_cast<CUIDropTargetImpl&>(m_oDropTargetImpl));
#endif
    // 创建成功
    m_oTrackMouse.cbSize = sizeof(m_oTrackMouse);
    m_oTrackMouse.dwFlags = TME_HOVER | TME_LEAVE;
    m_oTrackMouse.hwndTrack = hwnd;
    m_oTrackMouse.dwHoverTime = HOVER_DEFAULT;
    // TODO: 错误信息截断
    //if (!hwnd);
    m_hWnd = hwnd;
    // 高DPI支持
    if (!(UIManager.flag & ConfigureFlag::Flag_NoAutoScaleOnHighDpi)) {
        const auto dpi = impl::get_dpi_scale_from_hwnd(m_hWnd);
        viewport()->JustResetZoom(dpi.width, dpi.height);
    }
    // TODO: 错误信息截断
    const auto hr = this->Recreate();
    assert(hr);
}


/// <summary>
/// Called when [resize].
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIPlatformWin::OnResizeTs(Size2U size) noexcept {
    assert(size.width && size.height && "bad size");
    // 不一样才处理
    if ((this->rect.width == size.width) && (this->rect.height == size.height)) return;
    // 数据锁
    CUIDataAutoLocker locker;
    //LUIDebug(Hint) << size.width << ", " << size.height << endl;
    this->mark_fr_for_update();
    // 修改
    this->rect.width = size.width;
    this->rect.height = size.height;
    const auto fw = static_cast<float>(size.width);
    const auto fh = static_cast<float>(size.height);
    // 重置大小
    this->resize_viewport({ fw, fh });
    // 修改窗口缓冲帧大小
    m_bFlagSized = true;
}




#ifndef NDEBUG
volatile UINT g_dbgLastEventId = 0;
#endif

/// <summary>
/// 消息处理
/// </summary>
/// <param name="hwnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
auto LongUI::CUIPlatformWin::DoMsg(
    const HWND hwnd, const UINT message,
    const WPARAM wParam, const LPARAM lParam) noexcept -> LRESULT {
#ifndef NDEBUG
    g_dbgLastEventId = message;
#endif
    MouseEventArg arg;
    // 鼠标离开
    if (message == WM_MOUSELEAVE) {
        // BUG: Alt + 快捷键也会触发?
        //CUIInputKM::Instance().GetKeyState(CUIInputKM::KB_MENU);
        arg.px = -1.f; arg.py = -1.f;
        arg.wheel = 0.f;
        arg.type = MouseEvent::Event_MouseLeave;
        arg.modifier = LongUI::Modifier_None;
        m_bMouseEnter = false;
        this->DoMouseEventTs(arg);
    }
    // 鼠标悬壶(济世)
    else if (message == WM_MOUSEHOVER) {
        arg.type = MouseEvent::Event_MouseIdleHover;
        arg.wheel = 0.f;
        arg.px = static_cast<float>(int16_t(LOWORD(lParam)));
        arg.py = static_cast<float>(int16_t(HIWORD(lParam)));
        arg.modifier = static_cast<InputModifier>(wParam);
        this->DoMouseEventTs(arg);
    }
    // 一般鼠标消息处理
    else if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) {
        // 检查映射表长度
        constexpr size_t ARYLEN = sizeof(MEMAP) / sizeof(MEMAP[0]);
        constexpr size_t MSGLEN = WM_MOUSELAST - WM_MOUSEFIRST + 1;
        static_assert(ARYLEN == MSGLEN, "must be same");
        // 初始化参数
        arg.type = static_cast<MouseEvent>(MEMAP[message - WM_MOUSEFIRST]);
        arg.wheel = 0.f;
        arg.px = static_cast<float>(int16_t(LOWORD(lParam)));
        arg.py = static_cast<float>(int16_t(HIWORD(lParam)));
        arg.modifier = static_cast<InputModifier>(wParam);
        // shift+滚轮自动映射到水平方向
        if (message == WM_MOUSEWHEEL && (wParam & MK_SHIFT)) {
            arg.type = MouseEvent::Event_MouseWheelH;
        }
        switch (arg.type)
        {
            float delta;
        case MouseEvent::Event_MouseWheelV:
        case MouseEvent::Event_MouseWheelH:
            // 滚轮消息则修改滚轮数据
            delta = static_cast<float>(int16_t(HIWORD(wParam)));
            arg.wheel = delta / static_cast<float>(WHEEL_DELTA);
            break;
        case MouseEvent::Event_MouseMove:
            // 第一次进入
            if (!m_bMouseEnter) {
                arg.type = MouseEvent::Event_MouseEnter;
                m_bMouseEnter = true;
                this->DoMouseEventTs(arg);
                arg.type = MouseEvent::Event_MouseMove;
            };
            // 鼠标跟踪
            ::TrackMouseEvent(&m_oTrackMouse);
            break;
        case MouseEvent::Event_LButtonDown:
            // 有弹出窗口先关闭
            // FIXME: 自己也是弹出窗口的话怎么处理?
            if (this->popup && !(window()->config & CUIWindow::Config_Popup)) {
                this->ClosePopup();
                return 0;
            }
            m_bMouseLeftDown = true;
            ::SetCapture(hwnd);
            //LUIDebug(Hint) << "\r\n\t\tDown: " << this->captured << endl;
            break;
        case MouseEvent::Event_LButtonUp:
            // 没有按下就不算
            if (!m_bMouseLeftDown) return 0;
            m_bMouseLeftDown = false;
            //LUIDebug(Hint) << "\r\n\t\tUp  : " << this->captured << endl;
            ::ReleaseCapture();
            break;
        }
        this->DoMouseEventTs(arg);
    }
    // 其他消息处理
    else
        switch (message)
        {
            BOOL rc;
            PAINTSTRUCT ps;
        case impl::msg_post_destroy:
#ifndef NDEBUG
            LUIDebug(Warning) << "msg_post_destroy but not null" << endl;
#endif
            rc = ::DestroyWindow(hwnd);
            assert(rc && "DestroyWindow failed");
            return 0;
        case impl::msg_post_set_title:
            ::SetWindowTextW(hwnd, impl::sys(this->titlename.c_str()));
            return 0;
        case WM_SHOWWINDOW:
            // TODO: popup?
            break;
        case WM_SETCURSOR:
            ::SetCursor(reinterpret_cast<HCURSOR>(this->cursor.GetHandle()));
            break;
#ifdef LUI_ACCESSIBLE
        case WM_DESTROY:
            ::UiaReturnRawElementProvider(hwnd, 0, 0, nullptr);
            break;
        case WM_GETOBJECT:
            if (static_cast<long>(lParam) == static_cast<long>(UiaRootObjectId)) {
                const auto window = this->window();
                assert(window && "cannot be null");
                const auto root = CUIAccessibleWnd::FromWindow(*window);
                m_bAccessibility = true;
                return ::UiaReturnRawElementProvider(hwnd, wParam, lParam, root);
            }
            return 0;
#endif

            // 即时重置大小
#ifdef LUI_RESIZE_IMMEDIATELY
        case WM_EXITSIZEMOVE:
            this->OnResizeTs(LongUI::GetClientSize(hwnd));
            break;
#else
        case WM_ENTERSIZEMOVE:
            this->moving_resizing = true;
            break;
        case WM_EXITSIZEMOVE:
            this->moving_resizing = false;
            this->OnResizeTs(LongUI::GetClientSize(hwnd));
            break;
#endif


            //case WM_NCCALCSIZE:
            //    if (wParam) return 0;
            //    break;
#if 0
        case WM_SETFOCUS:
            ::CreateCaret(hwnd, (HBITMAP)NULL, 2, 20);
            ::SetCaretPos(50, 50);
            ::ShowCaret(hwnd);
            return 0;
        case WM_KILLFOCUS:
            ::HideCaret(hwnd);
            ::DestroyCaret();
        case WM_TOUCH:
            // TODO: TOUCH MESSAGE
            TOUCHINPUT;
            break;
#endif
            if (this->window()->config & CUIWindow::Config_Popup) {
                // CloseWindow 仅仅最小化窗口
                ::PostMessageW(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
        case WM_PAINT:
            ::BeginPaint(hwnd, &ps);
            ::EndPaint(hwnd, &ps);
            break;
        case WM_SIZE:
            // 先关闭弹出窗口再说
            this->ClosePopup();
            // 最小化不算
            switch (wParam) { case SIZE_MINIMIZED: return 0; }
#ifndef LUI_RESIZE_IMMEDIATELY
            // 拖拽重置不算
            if (this->moving_resizing) return 0;
#endif
            // 重置大小
            this->OnResizeTs({ LOWORD(lParam), HIWORD(lParam) });
            return 0;
        case WM_KEYDOWN:
        case WM_KEYUP:
            static_assert(WM_KEYUP == WM_KEYDOWN + 1, "bad code");
            {
                InputEventArg arg;
                const uint32_t plus = message - WM_KEYDOWN;
                const auto code = static_cast<uint32_t>(InputEvent::Event_KeyDown);
                const auto inev = static_cast<InputEvent>(code + plus);
                arg.event = static_cast<InputEvent>(code + plus);
                arg.sequence = (lParam >> 30) & 1;
                arg.character = static_cast<CUIInputKM::KB>(wParam);
                CUIDataAutoLocker locker;
                this->OnKey(arg);
            }
            return 0;
#if 0
        case WM_CONTEXTMENU:
            UIManager.DataLock();
            this->OnKeyDownUp(InputEvent::Event_KeyContext, static_cast<CUIInputKM::KB>(0));
            UIManager.DataUnlock();
            return 0;
#endif
        case WM_SYSKEYDOWN:
            switch (wParam)
            {
            case CUIInputKM::KB_F4:
                // Alt+F4
                if (this->window()->config & CUIWindow::Config_AltF4ToCloseWindow)
                    ::PostMessageW(hwnd, WM_CLOSE, 0, 0);
                break;
            case CUIInputKM::KB_MENU:
                // 检查访问键
                if (!(lParam & (1 << 30))) this->OnMenuTs();
                break;
            }
            return 0;
        case WM_GETMINMAXINFO:
            assert(lParam && "bad param");
            [](MINMAXINFO& info) noexcept {
                // TODO: 窗口最小大小
                info.ptMinTrackSize.x += DEFAULT_CONTROL_WIDTH;
                info.ptMinTrackSize.y += DEFAULT_CONTROL_HEIGHT;
            }(*reinterpret_cast<MINMAXINFO*>(lParam));
            return 0;
        case WM_IME_CHAR:
            // 针对IME输入的优化 - 记录输入字符数量
            if (!Unicode::IsHighSurrogate(static_cast<char16_t>(wParam)))
                this->ime_count++;
            break;
        case WM_CHAR:
            this->OnChar(static_cast<char16_t>(wParam));
            return 0;
        case WM_UNICHAR:
            this->OnCharTs(static_cast<char32_t>(wParam));
            return 0;
        case WM_IME_COMPOSITION:
            this->OnIme(hwnd);
            break;
        case WM_MOVING:
            // LOCK: 加锁?
            this->rect.top = reinterpret_cast<RECT*>(lParam)->top;
            this->rect.left = reinterpret_cast<RECT*>(lParam)->left;
            return true;
        case WM_CLOSE:
            this->close_window();
            return 0;
        case WM_MOUSEACTIVATE:
            return m_uMaRevalue;
        case WM_NCACTIVATE:
            // 非激活状态
            if (LOWORD(wParam) == WA_INACTIVE) {
                // 释放弹出窗口
                this->ClosePopup();
            }
            // 激活状态
            else {

            }
            break;
        case WM_SYSCHAR:
            if (wParam >= 'a' && wParam <= 'z')
                this->OnAccessKey(wParam - 'a');
            return 0;
        case WM_SYSCOMMAND:
            // 点击标题也要关闭弹出窗口
            if (wParam == (SC_MOVE | HTCAPTION))
                this->ClosePopup();
            break;
        case WM_DPICHANGED:
        {
            const float xdpi = float(uint16_t(LOWORD(wParam)));
            const float ydpi = float(uint16_t(HIWORD(wParam)));
            const float x = xdpi / float(LongUI::BASIC_DPI);
            const float y = ydpi / float(LongUI::BASIC_DPI);
            this->OnDpiChanged({ x, y }, *reinterpret_cast<RectL*>(lParam));
        }
            return 0;
        }
    // 未处理消息
    return ::DefWindowProcW(hwnd, message, wParam, lParam);
}



/// <summary>
/// Called when [IME].
/// </summary>
/// <returns></returns>
bool LongUI::CUIPlatformWin::OnIme(HWND hwnd) const noexcept {
    // 有效性
    if (!this->careted) return false;
    auto rect = this->caret;
    this->careted->MapToWindow(rect);

    bool code = false;
    const auto caret_x = static_cast<LONG>(rect.left);
    const auto caret_y = static_cast<LONG>(rect.top);
    const auto ctrl_w = this->rect.width;
    const auto ctrl_h = this->rect.height;
    if (caret_x >= 0 && caret_y >= 0 && caret_x < ctrl_w && caret_y < ctrl_h) {
        HIMC imc = ::ImmGetContext(hwnd);
        // TODO: caret 高度可能不是字体高度
        const auto caret_h = static_cast<LONG>(rect.bottom - rect.top);
        if (::ImmGetOpenStatus(imc)) {
            COMPOSITIONFORM cf = { 0 };
            cf.dwStyle = CFS_POINT;
            cf.ptCurrentPos.x = caret_x;
            cf.ptCurrentPos.y = caret_y;
            if (::ImmSetCompositionWindow(imc, &cf)) {
                LOGFONTW lf = { 0 };
                lf.lfHeight = caret_h;
                // TODO: 富文本支持
                //lf.lfItalic
                if (::ImmSetCompositionFontW(imc, &lf)) code = true;
            }
        }
        ::ImmReleaseContext(hwnd, imc);
    }
    return code;
}


/// <summary>
/// map rect to screen
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::MapToScreen(RectF & rect) const noexcept {
    //assert(m_hWnd && "NOT");
    POINT pt{ 0, 0 };
    ::ClientToScreen(m_hWnd, &pt);
    const auto px = static_cast<float>(pt.x);
    const auto py = static_cast<float>(pt.y);
    rect.top += py;
    rect.left += px;
    rect.right += px;
    rect.bottom += py;
}


/// <summary>
/// map rect to screen
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::MapToScreen(RectL& rect) const noexcept {
    POINT pt{ 0, 0 };
    ::ClientToScreen(m_hWnd, &pt);
    rect.left += pt.x;
    rect.top += pt.y;
    rect.right += pt.x;
    rect.bottom += pt.y;
}



/// <summary>
/// 设置新的标题名后调用该方法
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::AfterTitleName() noexcept {
    ::PostMessageW(m_hWnd, impl::msg_post_set_title, 0, 0);
}


/// <summary>
/// 设置新的绝对矩形后调用该方法
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::AfterAbsRect() noexcept {
    // 睡眠模式
    //if (this->IsInSleepMode()) return;
    //// 内联窗口
    //if (this->IsInlineWindow()) {
    //    assert(!"NOT IMPL");
    //}
    //// 系统窗口
    //else {
        ::SetWindowPos(
            m_hWnd,
            nullptr,
            rect.left, rect.top, rect.width, rect.height,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    //}
}


/// <summary>
/// Maps from screen.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIPlatformWin::MapFromScreen(Point2F& pos) const noexcept {
    // 内联窗口
    //if (this->IsInlineWindow()) {
    //    assert(!"NOT IMPL");
    //}
    // 系统窗口
    //else {
        POINT pt{ 0, 0 };
        ::ScreenToClient(m_hWnd, &pt);
        const auto px = static_cast<float>(pt.x);
        const auto py = static_cast<float>(pt.y);
        pos.x += px;
        pos.y += py;
    //}
}

/// <summary>
/// 设置新的位置后调用该方法
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::AfterPosition() noexcept {
    //// 睡眠模式
    //if (this->IsInSleepMode()) return;
    //// 内联窗口
    //if (this->IsInlineWindow()) {
    //    assert(!"NOT IMPL");
    //}
    //// 系统窗口
    //else {
        const auto adjx = 0; // m_private->adjust.left;
        const auto adjy = 0; // m_private->adjust.top;
        constexpr UINT flag = SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;
        ::SetWindowPos(m_hWnd, nullptr, rect.left + adjx, rect.top + adjy, 0, 0, flag);
    //}
}


/// <summary>
/// Closes the window.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::CloseWindow() noexcept {
    ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
}

/// <summary>
/// show the window
/// </summary>
/// <param name="show"></param>
/// <returns></returns>
void LongUI::CUIPlatformWin::ShowWindow(int show) noexcept {
    assert(m_hWnd && "bad window");
    /*
        This function posts a show-window event to the message
        queue of the given window. An application can use this
        function to avoid becoming nonresponsive while waiting
        for a nonresponsive application to finish processing a
        show-window event.
    */
    // 使用非阻塞的函数
    ::ShowWindowAsync(m_hWnd, show);
}

/// <summary>
/// resize window in absolute size
/// </summary>
/// <param name="size"></param>
/// <returns></returns>
void LongUI::CUIPlatformWin::ResizeAbsolute(Size2L size) noexcept {
    // 调整大小
    const auto realw = size.width + adjust.right - adjust.left;
    const auto realh = size.height + adjust.bottom - adjust.top;
    assert(realw > 0 && realh > 0);
    // 改变窗口
    constexpr UINT flag = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_ASYNCWINDOWPOS;
    ::SetWindowPos(m_hWnd, nullptr, 0, 0, realw, realh, flag);
}

/// <summary>
/// 获取当前窗口所在的工作区域
/// </summary>
/// <returns></returns>
auto LongUI::CUIPlatformWin::GetWorkArea() const noexcept -> RectL {
    const auto hwnd = m_hWnd;
    assert(hwnd && "bad hwnd?");
    const auto monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO info = { 0 };
    info.cbSize = sizeof(info);
    ::GetMonitorInfoW(monitor, &info);
    //RectL rc;
    //rc.left = info.rcWork.left;
    //rc.top = info.rcWork.top;
    //rc.right = info.rcWork.right;
    //rc.bottom = info.rcWork.bottom;
    return reinterpret_cast<RectL&>(info.rcWork);
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIPlatformWin::Recreate() noexcept -> Result {
    const auto hwnd = m_hWnd;
    // 可能是节能模式?
    if (!hwnd) return { Result::RS_FALSE };
    if (this->is_skip_render()) return{ Result::RS_OK };
    // 全渲染
    this->mark_fr_for_update();
    // 创建渲染资源, 需要渲染锁
    CUIRenderAutoLocker locker;
    assert(m_pBitmap == nullptr && "call release first");
    assert(m_pSwapchan == nullptr && "call release first");
    // 保证内存不泄漏
    this->ReleaseDeviceData();
    Result hr = { Result::RS_OK };
    // 创建交换酱
    if (hr) {
        // 获取窗口大小
        RECT client_rect; ::GetClientRect(hwnd, &client_rect);
        const Size2L wndsize = {
            client_rect.right - client_rect.left,
            client_rect.bottom - client_rect.top
        };
        // 设置逻辑大小
        m_szWndbufLogical = wndsize;
        assert(m_szWndbufLogical.width && m_szWndbufLogical.height);
        // 交换链信息
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        // 检测DComp支持
        if (this->is_direct_composition()) {
            swapChainDesc.Width = impl::get_fit_size_for_trans(wndsize.width);
            swapChainDesc.Height = impl::get_fit_size_for_trans(wndsize.height);
        }
        else {
            swapChainDesc.Width = wndsize.width;
            swapChainDesc.Height = wndsize.height;
        }
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
#ifdef LUI_RESIZE_IMMEDIATELY
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
#else
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
#endif
        // TODO: 延迟等待
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        swapChainDesc.Flags = 0;
        // SWAP酱
        IDXGISwapChain1* sc = nullptr;
        // 检查DComp支持
        if (this->is_direct_composition()) {
            // DirectComposition桌面应用程序
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            // 创建DirectComposition交换链
            hr = { UIManager.RefGraphicsFactory().CreateSwapChainForComposition(
                &UIManager.Ref3DDevice(),
                &swapChainDesc,
                nullptr,
                &sc
            ) };
            longui_debug_hr(hr, L"UIManager_DXGIFactory->CreateSwapChainForComposition faild");
        }
        // 创建一般的交换链
        else {
            // 一般桌面应用程序
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            // 利用窗口句柄创建交换链
            const auto temp = UIManager.RefGraphicsFactory().CreateSwapChainForHwnd(
                &UIManager.Ref3DDevice(),
                hwnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                &sc
            );
            // 第一次尝试失败?
#if defined(LUI_WIN10_ONLY) || !defined(LUI_RESIZE_IMMEDIATELY)
            hr.code = temp;
#else
            // 第一次尝试失败? Win7的场合
            if (FAILED(temp)) {
                swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
                hr.code = UIManager.RefGraphicsFactory().CreateSwapChainForHwnd(
                    &UIManager.Ref3DDevice(),
                    hwnd,
                    &swapChainDesc,
                    nullptr,
                    nullptr,
                    &sc
                );
            }
#endif
            longui_debug_hr(hr, L"GraphicsFactory.CreateSwapChainForHwnd faild");
        }
        // 设置 SWAP酱
        m_pSwapchan = static_cast<I::Swapchan*>(sc);
    }
    IDXGISurface* backbuffer = nullptr;
    // 利用交换链获取Dxgi表面
    if (hr) {
        hr = { m_pSwapchan->GetBuffer(
            0, IID_IDXGISurface, reinterpret_cast<void**>(&backbuffer)
        ) };
        longui_debug_hr(hr, L"swapchan->GetBuffer faild");
    }
    // 利用Dxgi表面创建位图
    if (hr) {
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );
        ID2D1Bitmap1* bmp = nullptr;
        hr = { UIManager.Ref2DRenderer().CreateBitmapFromDxgiSurface(
            backbuffer,
            &bitmapProperties,
            &bmp
        ) };
        m_pBitmap = static_cast<I::Bitmap*>(bmp);
        longui_debug_hr(hr, L"2DRenderer.CreateBitmapFromDxgiSurface faild");
    }
    LongUI::SafeRelease(backbuffer);
    // 使用DComp
    if (this->is_direct_composition()) {
        hr = impl::create_dcomp(m_dcomp, hwnd, *m_pSwapchan);
    }
    return hr;
}



#ifndef NDEBUG

void ui_dbg_set_window_title(
    LongUI::CUIWindow* pwnd,
    const char* title) noexcept {
    if (pwnd) pwnd->SetTitleName(LongUI::CUIString::FromUtf8(title));
}

void ui_dbg_set_window_title(
    HWND hwnd,
    const char* title) noexcept {
    assert(hwnd && "bad action");
    const auto aaa = LongUI::CUIString::FromUtf8(title);
    ::SetWindowTextW(hwnd, LongUI::impl::sys(aaa.c_str()));
}

#endif



/// <summary>
/// Gets the system last error.
/// </summary>
/// <returns></returns>
auto LongUI::Result::GetSystemLastError() noexcept -> Result {
    const auto x = ::GetLastError();
    constexpr int32_t UI_FACILITY = 7;
    return{ ((int32_t)(x) <= 0 ? ((int32_t)(x)) :
        ((int32_t)(((x) & 0x0000FFFF) | (UI_FACILITY << 16) | 0x80000000))) };
}
