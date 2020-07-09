// LUI
#include <core/ui_manager.h>
#include <core/ui_window.h>
#include <input/ui_kminput.h>
#include <control/ui_viewport.h>
#include <core/ui_platform_win.h>
// C/C++
#include <cassert>
#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_win.h>
#endif


// LongUI::detail
namespace LongUI { namespace detail {
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
}

#ifndef LUI_NO_DROPDRAG
#include <Shobjidl.h>
/// <summary>
/// Queries the interface.
/// </summary>
/// <param name="id">The identifier.</param>
/// <param name="ppvObject">The PPV object.</param>
/// <returns></returns>
HRESULT LongUI::CUIPlatformWin::QueryInterface(const IID& id, void **ppvObject) noexcept {
    IUnknown* ptr = nullptr;
    if (id == IID_IUnknown) ptr = static_cast<IUnknown*>(this);
    else if (id == IID_IDropTarget) ptr = static_cast<IDropTarget*>(this);
    if (*ppvObject = ptr) { this->AddRef(); return S_OK; }
    return E_NOINTERFACE;
}

/// <summary>
/// IDropTarget::DragEnter 实现
/// </summary>
/// <param name="pDataObj"></param>
/// <param name="grfKeyState"></param>
/// <param name="pt"></param>
/// <param name="pdwEffect"></param>
/// <returns></returns>
HRESULT  LongUI::CUIPlatformWin::DragEnter(IDataObject* pDataObj,
    DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) noexcept {
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    //POINT ppt = { pt.x, pt.y };
    //auto& helper = UIManager.RefDropTargetHelper();
    //helper.DragEnter(hwnd, pDataObj, &ppt, *pdwEffect);
    return S_OK;
}

/// <summary>
/// IDropTarget::DragOver 实现
/// </summary>
/// <param name="grfKeyState"></param>
/// <param name="pt"></param>
/// <param name="pdwEffect"></param>
/// <returns></returns>
HRESULT LongUI::CUIPlatformWin::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    UNREFERENCED_PARAMETER(grfKeyState);
    // TODO:
    if (false) {
        /*LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
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

/// <summary>
/// IDropTarget::DragLeave 实现
/// </summary>
/// <returns></returns>
HRESULT LongUI::CUIPlatformWin::DragLeave(void) noexcept {
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

/// <summary>
/// IDropTarget::Drop 实现
/// </summary>
/// <param name="pDataObj"></param>
/// <param name="grfKeyState"></param>
/// <param name="pt"></param>
/// <param name="pdwEffect"></param>
/// <returns></returns>
HRESULT LongUI::CUIPlatformWin::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    // 发送事件
    //if (m_pDragDropControl) {
        /*LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
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

#endif


/// <summary>
/// 
/// </summary>
/// <returns></returns>
LongUI::CUIPlatformWin::CUIPlatformWin() noexcept {
#if 1
    m_bFlagSized = false;
    m_bMouseEnter = false;
    m_bDcompSupport = false;
    m_bAccessibility = false;
    m_bMovingResizing = false;
    m_bMouseLeftDown = false;
    m_bSystemSkipRendering = false;
    m_bLayeredWindowSupport = false;
#endif
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
LongUI::CUIPlatformWin::~CUIPlatformWin() noexcept {

}


#if 1
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
    case detail::msg_post_destroy:
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
/// Initializes this instance.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="flag">The configuration.</param>
/// <returns></returns>
HWND LongUI::CUIPlatformWin::Init(HWND parent, uint16_t flag) noexcept {
    // 尝试注册
    this->register_class();
    // 初始化
    HWND hwnd = nullptr;
    const auto config = static_cast<CUIWindow::WindowConfig>(flag);
    static_assert(sizeof(config) == sizeof(flag), "same!");
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
            detail::sys(this->titlename.c_str()),
            style,
            window_rect.left, window_rect.top,
            window_rect.width, window_rect.height,
            // 弹出窗口使用NUL父窗口方便显示
            config & CUIWindow::Config_Popup ? nullptr : parent,
            nullptr,
            ::GetModuleHandleA(nullptr),
            this
        );
    }
#ifndef LUI_NO_DROPDRAG
    // 对于非弹出窗口注册拖放
    if (!(config & CUIWindow::Config_Popup)) ::RegisterDragDrop(hwnd, this);
#endif
    // 创建成功
    m_oTrackMouse.cbSize = sizeof(m_oTrackMouse);
    m_oTrackMouse.dwFlags = TME_HOVER | TME_LEAVE;
    m_oTrackMouse.hwndTrack = hwnd;
    m_oTrackMouse.dwHoverTime = HOVER_DEFAULT;
    return hwnd;
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
        case detail::msg_post_destroy:
#ifndef NDEBUG
            LUIDebug(Warning) << "msg_post_destroy but not null" << endl;
#endif
            rc = ::DestroyWindow(hwnd);
            assert(rc && "DestroyWindow failed");
            return 0;
        case detail::msg_post_set_title:
            ::SetWindowTextW(hwnd, detail::sys(this->titlename.c_str()));
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

#endif
