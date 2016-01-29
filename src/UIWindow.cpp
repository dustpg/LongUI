#include "LongUI.h"
#include <algorithm>
#include <dcomp.h>

// 任务按钮创建消息
const UINT LongUI::UIWindow::s_uTaskbarBtnCreatedMsg = ::RegisterWindowMessageW(L"TaskbarButtonCreated");

/// <summary>
/// Initializes a new instance of the 
/// <see cref="LongUI::UIWindow"/> class.
/// </summary>
/// <param name="parent">The parent for self in window-level</param>
LongUI::UIWindow::UIWindow(UIWindow* parent) noexcept : 
Super(nullptr), m_uiRenderQueue(this), wndparent(parent) {
    std::memset(&m_curMedium, 0, sizeof(m_curMedium));
}

/// <summary>
/// Creates the popup window
/// </summary>
/// <returns></returns>
auto LongUI::UIWindow::CreatePopup(const Config::Popup& popup) noexcept -> UIWindow* {
    assert(popup.parent && "bad argument");
    auto window = new(std::nothrow) UIWindow(popup.parent);
    // 内存申请成功
    if (window) {
        // 初始化
        window->initialize(popup);
        // 添加子节点
        if (popup.child) {
            popup.child->LinkNewParent(window);
            window->PushBack(popup.child);
#ifdef _DEBUG
           force_cast(window->name) = window->CopyString("PopupWindow");
#endif
        }
        // 重建
        auto hr = window->Recreate(); ShowHR(hr);
        // 创建完毕
        window->DoLongUIEvent(Event::Event_TreeBulidingFinished);
    }
    return window;
}

/// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The xml node.</param>
/// <returns></returns>
void LongUI::UIWindow::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 初始化
    assert(node && "<LongUI::UIWindow::initialize> window_node null");
    std::memset(&m_curMedium, 0, sizeof(m_curMedium));
    // 检查名称
    {
        auto basestr = node.attribute(LongUI::XMLAttribute::ControlName).value();
        if (basestr) {
            auto namestr = this->CopyStringSafe(basestr);
            force_cast(this->name) = namestr;
        }
    }
    CUIString titlename;
    titlename.Set(this->name.c_str());
    {
        Helper::MakeString(
            node.attribute(LongUI::XMLAttribute::WindowTitleName).value(),
            titlename
            );
    }
    // flag 区
    {
        auto flag = WindowFlag::Flag_None;
        // 检查FullRendering标记
        if (node.attribute("fullrender").as_bool(false)) {
            flag |= WindowFlag::Flag_FullRendering;
        }
        else {
            //this->reset_renderqueue();
        }
        // 检查alwaysrendering
        if (node.attribute("alwaysrendering").as_bool(false)) {
            flag |= WindowFlag::Flag_AlwaysRendering;
        }
        force_cast(this->wndflags) = flag;
        // XXX:
        force_cast(this->wnd_type) = Type_Layered;
    }
    // Debug Zone
#ifdef _DEBUG
    {
        debug_show = this->debug_this || node.attribute("debugshow").as_bool(false);
    }
#endif
    // 其他属性
    {
        // 最小大小
        float size[] = { LongUIWindowMinSize, LongUIWindowMinSize };
        Helper::MakeFloats(node.attribute("minisize").value(), size, 2);
        m_miniSize.width = static_cast<decltype(m_miniSize.width)>(size[0]);
        m_miniSize.height = static_cast<decltype(m_miniSize.height)>(size[1]);
        // 清理颜色
        Helper::MakeColor(
            node.attribute(LongUI::XMLAttribute::WindowClearColor).value(),
            this->clear_color
            );
        // 文本抗锯齿
        m_textAntiMode = uint16_t(Helper::GetEnumFromXml(node, D2D1_TEXT_ANTIALIAS_MODE_DEFAULT));
    }
    // 窗口区
    {
        // 检查样式样式
        auto popup = node.attribute("popup-test").as_bool(false);
        DWORD window_style = popup ? WS_POPUPWINDOW : WS_OVERLAPPEDWINDOW;
        // 设置窗口大小
        RECT window_rect = { 0, 0, LongUIDefaultWindowWidth, LongUIDefaultWindowHeight };
        // 浮点视区大小
        if (this->view_size.width == 0.f) {
            force_cast(this->view_size.width) = static_cast<float>(LongUIDefaultWindowWidth);
        }
        else {
            window_rect.right = static_cast<LONG>(this->view_size.width);
        }
        // 更新
        if (this->view_size.height == 0.f) {
            force_cast(this->view_size.height) = static_cast<float>(LongUIDefaultWindowHeight);
        }
        else {
            window_rect.bottom = static_cast<LONG>(this->view_size.height);
        }
        // 整数窗口大小
        force_cast(this->window_size.width) = window_rect.right;
        force_cast(this->window_size.height) = window_rect.bottom;
        // 可视区域范围
        visible_rect.right = this->view_size.width;
        visible_rect.bottom = this->view_size.height;
        m_2fContentSize = this->view_size;
        // 调整大小
        ::AdjustWindowRect(&window_rect, window_style, FALSE);
        // 居中
        window_rect.right -= window_rect.left;
        window_rect.bottom -= window_rect.top;
        window_rect.left = (::GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
        window_rect.top = (::GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;
        // 创建窗口
        m_hwnd = ::CreateWindowExW(
            //WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            WS_EX_NOREDIRECTIONBITMAP,
            LongUI::WindowClassNameA, 
            titlename.length() ? titlename.c_str() : L"LongUI",
            window_style,
            window_rect.left, window_rect.top, window_rect.right, window_rect.bottom,
            this->wndparent ? this->wndparent->GetHwnd() : nullptr,
            nullptr,
            ::GetModuleHandleW(nullptr),
            this
            );
        // 禁止 Alt + Enter 全屏
        if (m_hwnd) {
            UIManager_DXGIFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
        }
        // 创建失败
        else {
            UIManager.ShowError(L"Error! Failed to Create Window", __FUNCTIONW__);
            return;
        }
    }
    //SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    // 设置Hover
    m_csTME.cbSize = sizeof(m_csTME);
    m_csTME.dwFlags = TME_HOVER | TME_LEAVE;
    m_csTME.hwndTrack = m_hwnd;
    m_csTME.dwHoverTime = 0;
    // 创建闪烁计时器
    m_idBlinkTimer = ::SetTimer(m_hwnd, BLINK_EVENT_ID, ::GetCaretBlinkTime(), nullptr);
    // 添加窗口
    UIManager.RegisterWindow(this);
    // 拖放帮助器
    m_pDropTargetHelper = UIManager.GetDropTargetHelper();
    // 注册拖拽目标
    ::RegisterDragDrop(m_hwnd, this);
    // 所在窗口就是自己
    m_pWindow = this;
    // 清零
    std::memset(m_dirtyRects, 0, sizeof(m_dirtyRects));
    // 关闭时退出
    if (node.attribute("exitonclose").as_bool(true)) {
        m_baBoolWindow.SetTrue(UIWindow::Index_ExitOnClose);
    }
    // 自动显示窗口
    if (node.attribute("autoshow").as_bool(true)) {
        this->ShowWindow(SW_SHOW);
    }
}


/// <summary>
/// Initializes the specified popup.
/// </summary>
/// <param name="popup">The popup.</param>
/// <returns></returns>
void LongUI::UIWindow::initialize(const Config::Popup& popup) noexcept {
    assert(this->wndparent && "this->wndparent cannot be null while in popup window");
#ifdef _DEBUG
    this->debug_show = this->wndparent->debug_show;
    this->debug_this = this->wndparent->debug_this;
#endif
    // 失去焦点即关闭
    m_baBoolWindow.SetTrue(UIWindow::Index_CloseOnFocusKilled);
    // 链式调用
    Super::initialize();
    std::memset(&m_curMedium, 0, sizeof(m_curMedium));
    // flag 区
    {
        force_cast(this->wnd_type) = Type_Layered;
    }
    // 其他属性
    {
        // 最小大小
        m_miniSize.width = LongUIWindowMinSize;
        m_miniSize.height = LongUIWindowMinSize;
        // 文本抗锯齿
        m_textAntiMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
    }
    // 窗口区
    {
        // 检查样式样式
        constexpr DWORD window_style = WS_POPUPWINDOW;
        // 浮点视区大小
        force_cast(this->view_size.width) = float(popup.width);
        force_cast(this->view_size.height) = float(popup.height);
        // 整数窗口大小
        force_cast(this->window_size.width) = popup.width;
        force_cast(this->window_size.height) = popup.height;
        // 可视区域范围
        visible_rect.right = this->view_size.width;
        visible_rect.bottom = this->view_size.height;
        m_2fContentSize = this->view_size;
        // 居中
        auto left = (::GetSystemMetrics(SM_CXFULLSCREEN) - popup.width) / 2;
        auto top = (::GetSystemMetrics(SM_CYFULLSCREEN) - popup.height) / 2;
        // 创建窗口
        m_hwnd = ::CreateWindowExW(
            //WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            WS_EX_NOREDIRECTIONBITMAP,
            LongUI::WindowClassNameA, 
            L"LongUI Popup Window",
            window_style,
            left, top, popup.width, popup.height,
            this->wndparent->GetHwnd(),
            nullptr,
            ::GetModuleHandleW(nullptr),
            this
            );
        // 禁止 Alt + Enter 全屏
        if (m_hwnd) {
            UIManager_DXGIFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
        }
        // 创建失败
        else {
            UIManager.ShowError(L"Error! Failed to Create Window", __FUNCTIONW__);
            return;
        }
    }
    //SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    // 设置Hover
    m_csTME.cbSize = sizeof(m_csTME);
    m_csTME.dwFlags = TME_HOVER | TME_LEAVE;
    m_csTME.hwndTrack = m_hwnd;
    m_csTME.dwHoverTime = 0;
    // 创建闪烁计时器
    m_idBlinkTimer = ::SetTimer(m_hwnd, BLINK_EVENT_ID, ::GetCaretBlinkTime(), nullptr);
    // 添加窗口
    UIManager.RegisterWindow(this);
    // 拖放帮助器
    //m_pDropTargetHelper = UIManager.GetDropTargetHelper();
    // 注册拖拽目标
    //::RegisterDragDrop(m_hwnd, this);
    // 所在窗口就是自己
    m_pWindow = this;
    // 清零
    std::memset(m_dirtyRects, 0, sizeof(m_dirtyRects));
    // 显示窗口
    this->ShowWindow(SW_SHOW);
}

// UIWindow 析构函数
LongUI::UIWindow::~UIWindow() noexcept {
    // 设置窗口指针
    ::SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, LONG_PTR(0));
    // 解锁
    UIManager.Unlock();
    {
        // 取消注册
        ::RevokeDragDrop(m_hwnd);
        // 杀掉!
        ::KillTimer(m_hwnd, m_idBlinkTimer);
        // 释放资源
        this->release_data();
        // 释放数据
        LongUI::SafeRelease(m_pTaskBarList);
        LongUI::SafeRelease(m_pDropTargetHelper);
        LongUI::SafeRelease(m_pCurDataObject);
        // 关闭
        this->CloseWindowLater();
    }
    // 加锁
    UIManager.Lock();
    // 移除窗口
    UIManager.RemoveWindow(this);
}

// 移除控件引用
void LongUI::UIWindow::RemoveControlReference(UIControl* ctrl) noexcept {
    assert(this && "null pointer");
    auto remove_reference = [ctrl](UIControl*& cref) { 
        if (cref == ctrl) cref = nullptr;
    };
    // 移除引用
    remove_reference(m_pHoverTracked);
    remove_reference(m_pFocusedControl);
    remove_reference(m_pDragDropControl);
    remove_reference(m_pCapturedControl);
}


// 注册
void LongUI::UIWindow::RegisterOffScreenRender(UIControl* c, bool is3d) noexcept {
    // 检查
#ifdef _DEBUG
    auto itr = std::find(m_vRegisteredControl.begin(), m_vRegisteredControl.end(), c);
    if (itr != m_vRegisteredControl.end()) {
        UIManager << DL_Warning << L"control: [" << c->name << L"] existed" << LongUI::endl;
        return;
    }
#endif
    if (is3d) {
        m_vRegisteredControl.insert(m_vRegisteredControl.begin(), c);
    }
    else {
        m_vRegisteredControl.push_back(c);
    }
    // some error
    if (!m_vRegisteredControl.isok()) {
        UIManager << DL_Warning << L"insert failed" << LongUI::endl;
    }
}

// 反注册
void LongUI::UIWindow::UnRegisterOffScreenRender(UIControl* c) noexcept {
    auto itr = std::find(m_vRegisteredControl.begin(), m_vRegisteredControl.end(), c);
    if (itr != m_vRegisteredControl.end()) {
        m_vRegisteredControl.erase(itr);
    }
#ifdef _DEBUG
    else {
        UIManager << DL_Warning << L"control: [" << c->name << L"] not found" << LongUI::endl;
    }
#endif
}


// 设置插入符号
void LongUI::UIWindow::SetCaretPos(UIControl* ctrl, float _x, float _y) noexcept {
    if (!m_cShowCaret) return;
    assert(ctrl && "bad argument") ;
    // 转换为像素坐标
    auto pt = D2D1::Point2F(_x, _y);
    if (ctrl) {
        // FIXME
        // TODO: FIX IT
        pt = LongUI::TransformPoint(ctrl->world, pt);
    }
#ifdef _DEBUG
    if (this->debug_this || ctrl->debug_this) {
        UIManager << DL_Log << ctrl
            << LongUI::Formated(L"(%.1f, %.1f)", pt.x, pt.y)
            << LongUI::endl;
    }
#endif
    m_baBoolWindow.SetTrue(Index_CaretIn);
    m_baBoolWindow.SetTrue(Index_DoCaret);
    const auto intx = static_cast<LONG>(pt.x);
    const auto inty = static_cast<LONG>(pt.y);
    const auto oldx = static_cast<LONG>(m_rcCaretPx.left);
    const auto oldy = static_cast<LONG>(m_rcCaretPx.top);
    if (oldx != intx || oldy != inty) {
        this->refresh_caret();
        m_rcCaretPx.left = intx; m_rcCaretPx.top = inty;
        ::SetCaretPos(intx, inty);
    }
}

// 创建插入符号
void LongUI::UIWindow::CreateCaret(UIControl* ctrl, float width, float height) noexcept {
    assert(ctrl && "bad argument") ;
    this->refresh_caret();
    // 转换为像素单位
    m_rcCaretPx.width = static_cast<uint32_t>(width * ctrl->world._11);
    m_rcCaretPx.height = static_cast<uint32_t>(height * ctrl->world._22);
#ifdef _DEBUG
    if (this->debug_this) {
        UIManager << DL_Log << ctrl
            << LongUI::Formated(L"(%d, %d)", int(m_rcCaretPx.width), int(m_rcCaretPx.height))
            << LongUI::endl;
    }
#endif
    // 阈值检查
    m_rcCaretPx.width = std::max(m_rcCaretPx.width, 1i32);
    m_rcCaretPx.height = std::max(m_rcCaretPx.height, 1i32);
}

// 显示插入符号
void LongUI::UIWindow::ShowCaret() noexcept {
    //::ShowCaret(m_hwnd);
    ++m_cShowCaret;
    // 创建AE位图
    //if (!m_pd2dBitmapAE) {
        //this->recreate_ae_bitmap();
    //}
}

// 异常插入符号
void LongUI::UIWindow::HideCaret() noexcept { 
    //::HideCaret(m_hwnd);
    if (m_cShowCaret) {
        --m_cShowCaret;
    }
#ifdef _DEBUG
    else {
        UIManager << DL_Warning << L"m_cShowCaret alread to 0" << LongUI::endl;
    }
    if (!m_cShowCaret) {
        UIManager << DL_Log << this << "Caret Hided" << LongUI::endl;
    }
#endif
    if (!m_cShowCaret) {
        m_baBoolWindow.SetFalse(Index_CaretIn);
        m_baBoolWindow.SetTrue(Index_DoCaret);
    }
}

// 查找控件
auto LongUI::UIWindow::FindControl(const char* cname) noexcept -> UIControl * {
    // 查找控件
    auto result = m_hashStr2Ctrl.Find(cname);
    // 未找到返回空
    if (!result) {
        // 给予警告
        UIManager << DL_Warning << L" Control Not Found: " << cname << LongUI::endl;
        return nullptr;
    }
    else {
        return reinterpret_cast<LongUI::UIControl*>(*result);
    }
}

// 添加命名控件
void LongUI::UIWindow::AddNamedControl(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argumrnt");
    const auto cname = ctrl->name.c_str();
    // 有效
    if (cname[0]) {
        // 插入
        if(!m_hashStr2Ctrl.Insert(cname, ctrl)) {
            ShowErrorWithStr(L"Failed to add control");
        }
    }
}

// 设置图标
void LongUI::UIWindow::SetIcon(HICON hIcon) noexcept {
    ::DefWindowProcW(m_hwnd, WM_SETICON, TRUE, reinterpret_cast<LPARAM>(hIcon));
    ::DefWindowProcW(m_hwnd, WM_SETICON, FALSE, reinterpret_cast<LPARAM>(hIcon));
}


// release data
void LongUI::UIWindow::release_data() noexcept {
    if (m_hVSync) {
        ::SetEvent(m_hVSync);
        ::CloseHandle(m_hVSync);
        m_hVSync = nullptr;
    }
    // 释放资源
    LongUI::SafeRelease(m_pTargetBimtap);
    LongUI::SafeRelease(m_pSwapChain);
    LongUI::SafeRelease(m_pDcompDevice);
    LongUI::SafeRelease(m_pDcompTarget);
    LongUI::SafeRelease(m_pDcompVisual);
}

// 刻画插入符号
void LongUI::UIWindow::draw_caret() noexcept {
    /*// 不能在BeginDraw/EndDraw之间调用
    D2D1_POINT_2U pt = { m_rcCaretPx.left, m_rcCaretPx.top };
    D2D1_RECT_U src_rect;
    src_rect.top = LongUIWindowPlanningBitmap / 2;
    src_rect.left = m_bCaretIn ? 0 : LongUIWindowPlanningBitmap / 4;
    src_rect.right = src_rect.left + m_rcCaretPx.width;
    src_rect.bottom = src_rect.top + m_rcCaretPx.height;
    m_pTargetBimtap->CopyFromBitmap(
        &pt, m_pBitmapPlanning, &src_rect
        );*/
}

// 更新插入符号
void LongUI::UIWindow::refresh_caret() noexcept {
    // 不能在BeginDraw/EndDraw之间调用
    // TODO: 完成位图复制
}

// 设置呈现
void LongUI::UIWindow::set_present_parameters(DXGI_PRESENT_PARAMETERS& present) const noexcept {
    present.DirtyRectsCount = static_cast<uint32_t>(m_aUnitNow.length);
    // 存在脏矩形?
    if(!m_baBoolWindow.Test(Index_FullRenderingThisFrame)){
        // 插入符号?
        if (m_baBoolWindow.Test(Index_DoCaret)) {
            present.pDirtyRects[present.DirtyRectsCount] = { 
                m_rcCaretPx.left, m_rcCaretPx.top,
                m_rcCaretPx.left + m_rcCaretPx.width,
                m_rcCaretPx.top + m_rcCaretPx.height,
            };
            ++present.DirtyRectsCount;
        }
#ifdef _DEBUG
        static RECT s_rects[LongUIDirtyControlSize + 2];
        if (this->debug_show) {
            std::memcpy(s_rects, present.pDirtyRects, present.DirtyRectsCount * sizeof(RECT));
            present.pDirtyRects = s_rects;
            s_rects[present.DirtyRectsCount] = { 0, 0, 128, 35 };
            ++present.DirtyRectsCount;
        }
#endif
    }
    // 全刷新
    else {
        present.pScrollRect = nullptr;
        present.DirtyRectsCount = 0;
    }
}

// begin draw
void LongUI::UIWindow::BeginDraw() const noexcept {
    // 设置文本渲染策略
    UIManager_RenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE(m_textAntiMode));
    // 离屏渲染
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
    force_cast(m_baBoolWindow).SetTrue(Index_Prerender);
    for (auto ctrl : m_vRegisteredControl) {
        assert(ctrl->parent && "check it");
        ctrl->Render();
    }
    force_cast(m_baBoolWindow).SetFalse(Index_Prerender);
    // 设为当前渲染对象
    UIManager_RenderTarget->SetTarget(m_pTargetBimtap);
    // 开始渲染
    UIManager_RenderTarget->BeginDraw();
    // 设置转换矩阵
#if 0
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
#else
    UIManager_RenderTarget->SetTransform(&this->world);
#endif
    // 清空背景
    UIManager_RenderTarget->Clear(this->clear_color);
}

// 结束渲染
void LongUI::UIWindow::EndDraw() const noexcept {
    // 结束渲染
    UIManager_RenderTarget->EndDraw();
    // 呈现参数设置
    RECT rcScroll = { 0, 0, LONG(this->window_size.width), LONG(this->window_size.height) };
    RECT dirtyRects[LongUIDirtyControlSize + 1]; 
    std::memcpy(dirtyRects, m_dirtyRects, sizeof(dirtyRects));
    DXGI_PRESENT_PARAMETERS present_parameters;
    present_parameters.DirtyRectsCount = 0;
    present_parameters.pDirtyRects = dirtyRects;
    present_parameters.pScrollRect = &rcScroll;
    present_parameters.pScrollOffset = nullptr;
    // 设置参数
    this->set_present_parameters(present_parameters);
    // 呈现
    HRESULT hr = m_pSwapChain->Present1(1, 0, &present_parameters);
    longui_debug_hr(hr, L"m_pSwapChain->Present1 faild");
    // 收到重建消息时 重建UI
#ifdef _DEBUG
    assert(SUCCEEDED(hr));
    if (hr == DXGI_ERROR_DEVICE_REMOVED 
        || hr == DXGI_ERROR_DEVICE_RESET 
        || test_D2DERR_RECREATE_TARGET) {
        force_cast(test_D2DERR_RECREATE_TARGET) = false;
        UIManager << DL_Hint << L"D2DERR_RECREATE_TARGET!" << LongUI::endl;
        hr = UIManager.RecreateResources();
        if (FAILED(hr)) {
            UIManager << DL_Hint << L"But, Recreate Failed!!!" << LongUI::endl;
            UIManager << DL_Error << L"Recreate Failed!!!" << LongUI::endl;
        }
    }
#else
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        hr = UIManager.RecreateResources();
    }
#endif
    // 检查
    ShowHR(hr);
}


// UI窗口: 刷新
void LongUI::UIWindow::Update() noexcept {
    m_baBoolWindow.SetFalse(Index_FullRenderingThisFrame);
    m_baBoolWindow.SetFalse(Index_DoCaret);
    // 新窗口大小?
    if (m_baBoolWindow.Test(Index_NewSize)) {
        this->OnResize();
        m_baBoolWindow.SetFalse(Index_NewSize);
    }
    {
        auto current_unit = m_uiRenderQueue.GetCurrentUnit();
        m_aUnitNow.length = current_unit->length;
        std::memcpy(m_aUnitNow.units, current_unit->units, sizeof(*m_aUnitNow.units) * m_aUnitNow.length);
    }
    // 刷新前
    if (this->IsControlLayoutChanged()) {
        this->SetWidth(this->visible_rect.right);
        this->SetHeight(this->visible_rect.bottom);
    }
    this->UpdateWorld();
    // 没有就不刷新了
    m_baBoolWindow.SetTo(Index_Rendered, m_aUnitNow.length);
    if (!m_aUnitNow.length) return;
    // 全刷新?
    if (m_aUnitNow.units[0] == static_cast<UIControl*>(this)) {
        m_baBoolWindow.SetTrue(Index_FullRenderingThisFrame);
        //UIManager << DL_Hint << "m_present.DirtyRectsCount = 0;" << LongUI::endl;
        // 交给父类处理
        Super::Update();
    }
    // 部分刷新
    else {
        m_baBoolWindow.SetFalse(Index_FullRenderingThisFrame);
        // 更新脏矩形
        for (uint32_t i = 0ui32; i < m_aUnitNow.length; ++i) {
            auto ctrl = m_aUnitNow.units[i];
            assert(ctrl->parent && "check it");
            // 设置转换矩阵
            ctrl->Update();
            ctrl->AfterUpdate();
            // 限制转换
            m_dirtyRects[i].left = static_cast<LONG>(ctrl->visible_rect.left);
            m_dirtyRects[i].top = static_cast<LONG>(ctrl->visible_rect.top);
            m_dirtyRects[i].right = static_cast<LONG>(std::ceil(ctrl->visible_rect.right));
            m_dirtyRects[i].bottom = static_cast<LONG>(std::ceil(ctrl->visible_rect.bottom));
        }
        // control update
        UIControl::Update();
    }
    // 调试
#ifdef _DEBUG
    if (m_baBoolWindow.Test(Index_FullRenderingThisFrame)) {
        ++full_render_counter;
    }
    else {
        ++dirty_render_counter;
    }
#endif
    this->AfterUpdate();
}

// UIWindow 渲染 
void LongUI::UIWindow::Render() const noexcept  {
    // 全刷新: 继承父类
    if (m_baBoolWindow.Test(Index_FullRenderingThisFrame)) {
        Super::Render();
        //UIManager << DL_Hint << "FULL" << LongUI::endl;
    }
    // 部分刷新:
    else {
        //UIManager << DL_Hint << "DIRT" << LongUI::endl;
#if 1
        // 先排序
        UIControl* units[LongUIDirtyControlSize];
        size_t length_for_units = 0;
        // 数据
        {
            assert(m_aUnitNow.length < LongUIDirtyControlSize);
            length_for_units = m_aUnitNow.length;
            std::memcpy(units, m_aUnitNow.units, length_for_units * sizeof(void*));
        }
        // 再渲染
        auto init_transfrom = DX::Matrix3x2F::Identity();
        for (auto unit = units; unit < units + length_for_units; ++unit) {
            auto ctrl = *unit; assert(ctrl != this);
            UIManager_RenderTarget->SetTransform(&ctrl->world);
            D2D1_POINT_2F clipr[2];
            clipr[0] = LongUI::TransformPointInverse(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(ctrl->visible_rect.left));
            clipr[1] = LongUI::TransformPointInverse(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(ctrl->visible_rect.right));
            UIManager_RenderTarget->PushAxisAlignedClip(reinterpret_cast<D2D1_RECT_F*>(clipr), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#if 0 // def _DEBUG
            if (this->debug_this) {
                AutoLocker;
                UIManager << DL_Log << "RENDER: " << ctrl << LongUI::endl;
            }
#endif
            // 正常渲染
            ctrl->Render();
            // 回来
            UIManager_RenderTarget->PopAxisAlignedClip();
    }
#else
        // 再渲染
        for (uint32_t i = 0ui32; i < m_aUnitNow.length; ++i) {
            auto ctrl = m_aUnitNow.units[i];
            // 设置转换矩阵
            D2D1_MATRIX_3X2_F matrix; ctrl->GetWorldTransform(matrix);
            UIManager_RenderTarget->SetTransform(&matrix);
            ctrl->Render(RenderType::Type_Render);
        }
#endif
    }
    // 插入符号
    if (m_baBoolWindow.Test(Index_DoCaret) && m_baBoolWindow.Test(Index_CaretIn)) {
        UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
        D2D1_RECT_F rect;
        rect.left = static_cast<float>(m_rcCaretPx.left);
        rect.top = static_cast<float>(m_rcCaretPx.top);
        rect.right = rect.left + static_cast<float>(m_rcCaretPx.width);
        rect.bottom = rect.top + static_cast<float>(m_rcCaretPx.height);
        UIManager_RenderTarget->PushAxisAlignedClip(&rect, D2D1_ANTIALIAS_MODE_ALIASED);
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        UIManager_RenderTarget->FillRectangle(&rect, m_pBrush_SetBeforeUse);
        UIManager_RenderTarget->PopAxisAlignedClip();
        UIManager_RenderTarget->SetTransform(&this->world);
    }
    // 调试输出
#ifdef _DEBUG
    if (this->debug_show) {
        D2D1_MATRIX_3X2_F nowMatrix, iMatrix = DX::Matrix3x2F::Scale(0.45f, 0.45f);
        UIManager_RenderTarget->GetTransform(&nowMatrix);
        UIManager_RenderTarget->SetTransform(&iMatrix);
        wchar_t buffer[1024];
        auto length = std::swprintf(
            buffer, 1024,
            L"Full Rendering Count: %d\nDirty Rendering Count: %d\nThis DirtyRectsCount:%d",
            int(full_render_counter),
            int(dirty_render_counter),
            int(m_aUnitNow.length)
            );
        auto tf = UIManager.GetTextFormat(LongUIDefaultTextFormatIndex);
        auto ta = tf->GetTextAlignment();
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        tf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        UIManager_RenderTarget->DrawText(
            buffer, length, tf,
            D2D1::RectF(0.f, 0.f, 1000.f, 70.f),
            m_pBrush_SetBeforeUse
            );
        tf->SetTextAlignment(ta);
        LongUI::SafeRelease(tf);
        UIManager_RenderTarget->SetTransform(&nowMatrix);
    }
#endif
}

// 退出时
bool LongUI::UIWindow::OnClose() noexcept {
    // 延迟清理
    this->delay_cleanup(); 
    // 退出程序?
    if (m_baBoolWindow.Test(UIWindow::Index_ExitOnClose)) {
        UIManager.Exit();
    }
    return true;
};

// UIWindow 事件处理
bool LongUI::UIWindow::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // 有了匿名函数妈妈再也不用担心一条函数有N行了
    // -------------------- On  Timer   ------------
    auto on_timer = [this](WPARAM wParam) {
        // 小于1K认为是常量数据
        if (static_cast<UINT_PTR>(wParam) < 1024) {
            // 闪烁?
            if (wParam == BLINK_EVENT_ID) {
                if (m_cShowCaret) {
                    m_baBoolWindow.SetNot(Index_CaretIn);
                    m_baBoolWindow.SetTrue(Index_DoCaret);
                }
                return true;
            }
            return false;
        }
        // 大于1K认为是指针
        else {
            assert((wParam & 3) == 0 && "bad action");
            auto ctrl = reinterpret_cast<UIControl*>(wParam);
            return ctrl->DoLongUIEvent(Event::Event_Timer);
        }
    };
    // -------------------- Main DoEvent------------
    // 这里就不处理LongUI事件了 交给父类吧
    if (arg.sender) return Super::DoEvent(arg);
    // 其他LongUI事件
    bool handled = false;
    // 特殊事件
    if (arg.msg == s_uTaskbarBtnCreatedMsg) {
        LongUI::SafeRelease(m_pTaskBarList);
        UIManager << DL_Log << "TaskbarButtonCreated" << LongUI::endl;
        auto hr = ::CoCreateInstance(
            CLSID_TaskbarList,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pTaskBarList)
            );
        ShowHR(hr);
        return true;
    }
    // 处理事件
    switch (arg.msg)
    {
    case WM_SETCURSOR:
        // 设置光标
        ::SetCursor(now_cursor);
        break;
    /*case WM_DWMCOLORIZATIONCOLORCHANGED:
    {
        D2D_COLOR_F theme_color;
        CUIManager::GetThemeColor(theme_color);
    }
    break;*/
    case WM_TIMER:
        handled = on_timer(arg.sys.wParam);
        break;
    /*case WM_NCHITTEST:
        arg.lr = HTCAPTION;
        handled = true;
        break;*/
    case WM_SETFOCUS:
        ::CreateCaret(m_hwnd, nullptr, 1, 1);
        handled = true;
        break;
    case WM_KILLFOCUS:
        // 存在焦点控件
        if (m_pFocusedControl){
            force_cast(arg.sender) = this;
            force_cast(arg.event) = LongUI::Event::Event_KillFocus;
            m_pFocusedControl->DoEvent(arg);
            m_pFocusedControl = nullptr;
            force_cast(arg.sender) = nullptr;
            force_cast(arg.msg) = WM_KILLFOCUS;
        }
        ::DestroyCaret();
        // 失去焦点即关闭窗口
        if (m_baBoolWindow.Test(UIWindow::Index_CloseOnFocusKilled)) {
            this->CloseWindowLater();
        }
        handled = true;
        break;
    case WM_SIZE:           // 改变大小
    {
        uint32_t wwidth, wheight;
        {
            RECT rect; ::GetClientRect(m_hwnd, &rect);
            wwidth = rect.right - rect.left;
            wheight = rect.bottom - rect.top;
        }
        // 数据有效?
        if (wwidth && wheight && (wwidth != this->window_size.width ||
            wheight != this->window_size.height)) {
            force_cast(this->window_size.width) = wwidth;
            force_cast(this->window_size.height) = wheight;
            m_baBoolWindow.SetTrue(Index_NewSize);
        }
    }
        handled = true;
        break;
    case WM_GETMINMAXINFO:  // 获取限制大小
        reinterpret_cast<MINMAXINFO*>(arg.sys.lParam)->ptMinTrackSize.x = m_miniSize.width;
        reinterpret_cast<MINMAXINFO*>(arg.sys.lParam)->ptMinTrackSize.y = m_miniSize.height;
        break;
    case WM_DISPLAYCHANGE:
        UIManager << DL_Hint << "WM_DISPLAYCHANGE" << LongUI::endl;
        {
            // 获取屏幕刷新率
            auto old = UIManager.GetDisplayFrequency();
            UIManager.RefreshDisplayFrequency();
            auto now = UIManager.GetDisplayFrequency();
            if (old != now) {
                m_uiRenderQueue.Reset(static_cast<uint32_t>(now));
            }
        }
        // 强行刷新一帧
        this->Invalidate(this);
        break;
    case WM_CLOSE:
        // 窗口关闭
        handled = this->OnClose();
        break;
    }
    // 处理
    if (handled) return true;
    // 有就处理
    if (m_pFocusedControl && m_pFocusedControl->DoEvent(arg)) {
        return true;
    }
    // 还是没有处理就交给父类处理
    return Super::DoEvent(arg);
}

// 鼠标事件
bool LongUI::UIWindow::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // hover跟踪
    if (arg.event == MouseEvent::Event_MouseHover && m_pHoverTracked) {
        return m_pHoverTracked->DoMouseEvent(arg);
    }
    // 存在捕获控件
    if (m_pCapturedControl) {
        return m_pCapturedControl->DoMouseEvent(arg);
    }
    // 父类
    auto code = Super::DoMouseEvent(arg);
    // 设置跟踪
    if (arg.event == MouseEvent::Event_MouseMove) {
        m_csTME.dwHoverTime = m_pHoverTracked ? DWORD(m_pHoverTracked->GetHoverTrackTime()) : DWORD(0);
        ::TrackMouseEvent(&m_csTME);
    }
    return code;
}


// 设置鼠标焦点
void LongUI::UIWindow::SetFocus(UIControl* ctrl) noexcept {
    // 无效
    assert(ctrl && "bad argument");
    // 可聚焦的
    if (ctrl && ctrl->flags & Flag_Focusable) {
        // 有效
        if (m_pFocusedControl) {
            m_pFocusedControl->DoLongUIEvent(Event::Event_KillFocus);
        }
        // 有效
        if ((m_pFocusedControl = ctrl)) {
            m_pFocusedControl->DoLongUIEvent(Event::Event_SetFocus);
        }
    }
}

// 重置窗口大小
void LongUI::UIWindow::OnResize(bool force) noexcept {
    assert(this->wnd_type != Type_RenderOnParent);
    if (this->wnd_type != Type_Layered) {
        force = true;
    }
    //UIManager << DL_Log << "called" << LongUI::endl;
    // 修改大小, 需要取消目标
    UIManager_RenderTarget->SetTarget(nullptr);
    // 修改
    visible_rect.right = static_cast<float>(this->window_size.width);
    visible_rect.bottom = static_cast<float>(this->window_size.height);
    this->SetWidth(visible_rect.right / m_2fZoom.width);
    this->SetHeight(visible_rect.bottom / m_2fZoom.height);
    // 设置
    auto rect_right = LongUI::MakeAsUnit(this->window_size.width);
    auto rect_bottom = LongUI::MakeAsUnit(this->window_size.height);
    if (force) {
        rect_right = this->window_size.width;
        rect_bottom = this->window_size.height;
    }
    auto old_size = m_pTargetBimtap->GetPixelSize();
    HRESULT hr = S_OK;
    // 强行 或者 小于才Resize
    if (force || old_size.width < uint32_t(rect_right) || old_size.height < uint32_t(rect_bottom)) {
        UIManager << DL_Hint << L"Window: [" 
            << this->name 
            << L"]\tTarget Bitmap Resize to " 
            << LongUI::Formated(L"(%d, %d)", int(rect_right), int(rect_bottom)) 
            << LongUI::endl;
        IDXGISurface* pDxgiBackBuffer = nullptr;
        LongUI::SafeRelease(m_pTargetBimtap);
        hr = m_pSwapChain->ResizeBuffers(
            2, rect_right, rect_bottom, DXGI_FORMAT_B8G8R8A8_UNORM, 
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
            );
        longui_debug_hr(hr, L"m_pSwapChain->ResizeBuffers faild");
        // 检查
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            UIManager.RecreateResources();
            UIManager << DL_Hint << L"Recreate device" << LongUI::endl;
        }
        // 利用交换链获取Dxgi表面
        if (SUCCEEDED(hr)) {
            hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
            longui_debug_hr(hr, L"m_pSwapChain->GetBuffer faild");
        }
        // 利用Dxgi表面创建位图
        if (SUCCEEDED(hr)) {
            D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
                );
            hr = UIManager_RenderTarget->CreateBitmapFromDxgiSurface(
                pDxgiBackBuffer,
                &bitmapProperties,
                &m_pTargetBimtap
                );
            longui_debug_hr(hr, L"UIManager_RenderTarget->CreateBitmapFromDxgiSurface faild");
        }
        // 重建失败?
        if (FAILED(hr)) {
            UIManager << DL_Error << L" Recreate FAILED!" << LongUI::endl;
            ShowHR(hr);
        }
        LongUI::SafeRelease(pDxgiBackBuffer);
    }
    // 强行刷新一帧
    this->Invalidate(this);
}

// UIWindow 重建
auto LongUI::UIWindow::Recreate() noexcept ->HRESULT {
    // 跳过
    if (m_baBoolWindow.Test(Index_SkipRender)) return S_OK;
    // 释放数据
    this->release_data();
    // DXGI Surface 后台缓冲
    IDXGISurface*                       pDxgiBackBuffer = nullptr;
    IDXGISwapChain1*                    pSwapChain = nullptr;
    // 创建交换链
    HRESULT hr = S_OK;
    // 创建交换链
    if (SUCCEEDED(hr)) {
        RECT rect = { 0 }; ::GetClientRect(m_hwnd, &rect);
        // 交换链信息
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.Width = LongUI::MakeAsUnit(rect.right - rect.left);
        swapChainDesc.Height = LongUI::MakeAsUnit(rect.bottom - rect.top);
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        // XXX: Fixit
        if (this->wnd_type == Type_Layered) {
            // DirectComposition桌面应用程序
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            // 创建DirectComposition交换链
            hr = UIManager_DXGIFactory->CreateSwapChainForComposition(
                UIManager_DXGIDevice,
                &swapChainDesc,
                nullptr,
                &pSwapChain
                );
            longui_debug_hr(hr, L"UIManager_DXGIFactory->CreateSwapChainForComposition faild");
        }
        else {
            // 一般桌面应用程序
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            // 利用窗口句柄创建交换链
            hr = UIManager_DXGIFactory->CreateSwapChainForHwnd(
                UIManager_D3DDevice,
                m_hwnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                &pSwapChain
                );
            longui_debug_hr(hr, L"UIManager_DXGIFactory->CreateSwapChainForHwnd faild");
        }
    }
    // 获取交换链V2
    if (SUCCEEDED(hr)) {
        hr = pSwapChain->QueryInterface(
            LongUI::IID_IDXGISwapChain2,
            reinterpret_cast<void**>(&m_pSwapChain)
            );
        longui_debug_hr(hr, L"pSwapChain->QueryInterface LongUI::IID_IDXGISwapChain2 faild");
    }
    // 获取垂直等待事件
    if (SUCCEEDED(hr)) {
        m_hVSync = m_pSwapChain->GetFrameLatencyWaitableObject();
    }
    /*// 确保DXGI队列里边不会超过一帧
    if (SUCCEEDED(hr)) {
        hr = UIManager_DXGIDevice->SetMaximumFrameLatency(1);
        longui_debug_hr(hr, L"UIManager_DXGIDevice->SetMaximumFrameLatency faild");
    }*/
    // 利用交换链获取Dxgi表面
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
        longui_debug_hr(hr, L"m_pSwapChain->GetBuffer faild");
    }
    // 利用Dxgi表面创建位图
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
            );
        hr = UIManager_RenderTarget->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pTargetBimtap
            );
        longui_debug_hr(hr, L"UIManager_RenderTarget->CreateBitmapFromDxgiSurface faild");
    }
    // 使用DComp
    if (this->wnd_type == Type_Layered) {
        // 创建直接组合(Direct Composition)设备
        if (SUCCEEDED(hr)) {
            hr = LongUI::Dll::DCompositionCreateDevice(
                UIManager_DXGIDevice,
                LongUI_IID_PV_ARGS(m_pDcompDevice)
                );
            longui_debug_hr(hr, L"DCompositionCreateDevice faild");
        }
        // 创建直接组合(Direct Composition)目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateTargetForHwnd(
                m_hwnd, true, &m_pDcompTarget
                );
            longui_debug_hr(hr, L"m_pDcompDevice->CreateTargetForHwnd faild");
        }
        // 创建直接组合(Direct Composition)视觉
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateVisual(&m_pDcompVisual);
            longui_debug_hr(hr, L"m_pDcompDevice->CreateVisual faild");
        }
        // 设置当前交换链为视觉内容
        if (SUCCEEDED(hr)) {
            hr = m_pDcompVisual->SetContent(m_pSwapChain);
            longui_debug_hr(hr, L"m_pDcompVisual->SetContent faild");
        }
        // 设置当前视觉为窗口目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompTarget->SetRoot(m_pDcompVisual);
            longui_debug_hr(hr, L"m_pDcompTarget->SetRoot faild");
        }
        // 向系统提交
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->Commit();
            longui_debug_hr(hr, L"m_pDcompDevice->Commit faild");
        }
    }
    // 错误
    if (FAILED(hr)){
        UIManager << L"Recreate Failed!" << LongUI::endl;
        ShowHR(hr);
    }
    LongUI::SafeRelease(pDxgiBackBuffer);
    LongUI::SafeRelease(pSwapChain);
    {
        // 获取屏幕刷新率
        m_uiRenderQueue.Reset(UIManager.GetDisplayFrequency());
        // 强行刷新一帧
        this->Invalidate(this);
    }
    // 重建 子控件UI
    return Super::Recreate();
}

// UIWindow 关闭控件
void LongUI::UIWindow::cleanup() noexcept {
    // 删除对象
    delete this;
}

// 窗口创建时
bool LongUI::UIWindow::OnCreated(HWND hwnd) noexcept {
    // 权限提升?保证
    CHANGEFILTERSTRUCT cfs = { sizeof(CHANGEFILTERSTRUCT) };
    ::ChangeWindowMessageFilterEx(hwnd, s_uTaskbarBtnCreatedMsg, MSGFLT_ALLOW, &cfs);
    return true;
}

// 鼠标移动时候
/*bool LongUI::UIWindow::OnMouseMove(const LongUI::EventArgument& arg) noexcept {
}

// 鼠标滚轮
bool LongUI::UIWindow::OnMouseWheel(const LongUI::EventArgument& arg) noexcept {
}*/

// ----------------- IDropTarget!!!! Yooooooooooo~-----

// 获取拖放效果
DWORD GetDropEffect(DWORD grfKeyState, DWORD dwAllowed) {
    DWORD dwEffect = 0;
    // 1. 检查pt来看是否允许drop操作在某个位置
    // 2. 计算出基于grfKeyState的drop效果
    if (grfKeyState & MK_CONTROL) {
        dwEffect = dwAllowed & DROPEFFECT_COPY;
    }
    else if (grfKeyState & MK_SHIFT) {
        dwEffect = dwAllowed & DROPEFFECT_MOVE;
    }
    // 3. 非键盘修饰符指定(或drop效果不允许), 因此基于drop源的效果
    if (dwEffect == 0) {
        if (dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
        if (dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
    }
    return dwEffect;
}

// IDropTarget::DragEnter 实现
HRESULT  LongUI::UIWindow::DragEnter(IDataObject* pDataObj,
    DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) noexcept {
    UNREFERENCED_PARAMETER(grfKeyState);
    m_baBoolWindow.SetTrue(Index_InDraging);
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    // 取消聚焦窗口
    if(m_pFocusedControl){
        m_pFocusedControl->DoLongUIEvent(Event::Event_KillFocus);
        m_pFocusedControl = nullptr;
    }
    // 保留数据
    LongUI::SafeRelease(m_pCurDataObject);
    m_pCurDataObject = LongUI::SafeAcquire(pDataObj);
    // 由帮助器处理
    POINT ppt = { pt.x, pt.y };
    if (m_pDropTargetHelper) {
        m_pDropTargetHelper->DragEnter(m_hwnd, pDataObj, &ppt, *pdwEffect);
    }
    return S_OK;
}


// IDropTarget::DragOver 实现
HRESULT LongUI::UIWindow::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    UNREFERENCED_PARAMETER(grfKeyState);
    D2D1_POINT_2F pt2f = { static_cast<float>(pt.x), static_cast<float>(pt.y) };
    UIControl* control; control = nullptr;
    // TODO:
    // 检查控件支持
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
    if (m_pDropTargetHelper) {
        POINT ppt = { pt.x, pt.y };
        m_pDropTargetHelper->DragOver(&ppt, *pdwEffect);
    }
    return S_OK;
}

// IDropTarget::DragLeave 实现
HRESULT LongUI::UIWindow::DragLeave(void) noexcept {
    // 发送事件
    if (m_pDragDropControl) {
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
    }
    /*OnDragLeave(m_hTargetWnd);*/
    m_pDragDropControl = nullptr;
    //m_isDataAvailable = TRUE;
    if (m_pDropTargetHelper) {
        m_pDropTargetHelper->DragLeave();
    }
    m_baBoolWindow.SetFalse(Index_InDraging);
    return S_OK;
}

// IDropTarget::Drop 实现
HRESULT LongUI::UIWindow::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    // 发送事件
    if (m_pDragDropControl) {
        /*LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
        arg.sender = this;
        arg.event = LongUI::Event::Event_Drop;
        arg.cf.dataobj = m_pCurDataObject;
        arg.cf.outeffect = pdwEffect;
        // 发送事件
        m_pDragDropControl->DoEvent(arg);
        m_pDragDropControl = nullptr;*/
        
    }
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    if (m_pDropTargetHelper){
        POINT ppt = { pt.x, pt.y };
        m_pDropTargetHelper->Drop(pDataObj, &ppt, *pdwEffect);
    }
    *pdwEffect = ::GetDropEffect(grfKeyState, *pdwEffect);
    return S_OK;
}
