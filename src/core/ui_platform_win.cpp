// LUI
#include <core/ui_manager.h>
#include <core/ui_window.h>
#include <core/ui_platform_win.h>
// C/C++
#include <cassert>

// Windows API
#define NOMINMAX
#include <Windows.h>
#include <Shobjidl.h>

#ifndef LUI_NO_DROPDRAG
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
LongUI::CUIPlatformWin::CUIPlatformWin::CUIPlatformWin() noexcept {

}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
LongUI::CUIPlatformWin::CUIPlatformWin::~CUIPlatformWin() noexcept {

}

#if 0
/// <summary>
/// Registers the window class.
/// </summary>
/// <returns></returns>
void LongUI::CUIPlatformWin::CUIPlatformWin::register_class() noexcept {
    WNDCLASSEXW wcex;
    const auto ins = ::GetModuleHandleW(nullptr);
    // 已经注册过了
    if (::GetClassInfoExW(ins, Attribute::WindowClassNameN, &wcex)) return;
    // 注册一般窗口类
    wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = 0;
    wcex.lpfnWndProc = Private::WndProc;
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
/// Initializes this instance.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="config">The configuration.</param>
/// <returns></returns>
HWND LongUI::CUIPlatformWin::CUIPlatformWin::Init(HWND, uint16_t flag) noexcept
    // 尝试注册
    this->RegisterWindowClass();
    // 初始化
    HWND hwnd = nullptr;
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
        this->ma_return_code = config & CUIWindow::Config_Popup ? MA_NOACTIVATE : MA_ACTIVATE;
        // 调整大小
        static_assert(sizeof(RECT) == sizeof(this->adjust), "bad type");
        this->adjust = { 0 };
        ::AdjustWindowRect(reinterpret_cast<RECT*>(&this->adjust), style, FALSE);
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
            detail::sys(titlename.c_str()),
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
    if (!(config & CUIWindow::Config_Popup))
        ::RegisterDragDrop(hwnd, &drop_target);
#endif
    // 创建成功
    //if (hwnd)
    this->track_mouse.cbSize = sizeof(this->track_mouse);
    this->track_mouse.dwFlags = TME_HOVER | TME_LEAVE;
    this->track_mouse.hwndTrack = hwnd;
    this->track_mouse.dwHoverTime = HOVER_DEFAULT;
    return hwnd;
}
#endif