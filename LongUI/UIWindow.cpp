#include "LongUI.h"
// 任务按钮创建消息
const UINT LongUI::UIWindow::s_uTaskbarBtnCreatedMsg = ::RegisterWindowMessageW(L"TaskbarButtonCreated");

// UIWindow 构造函数
LongUI::UIWindow::UIWindow(pugi::xml_node node, UIWindow* parent_window) 
noexcept : Super(node), m_uiRenderQueue(this), window_parent(parent_window) {
    assert(node && "<LongUI::UIWindow::UIWindow> window_node null");
    ZeroMemory(&m_curMedium, sizeof(m_curMedium));
    CUIString titlename(m_strControlName);
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
        force_cast(this->window_flags) = flag;
        // XXX:
        force_cast(this->window_type) = Type_Layered;
    }
    // Debug Zone
#ifdef _DEBUG
    {
        debug_show = node.attribute("debug").as_bool(false);
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
        m_textAntiMode = uint16_t(Helper::XMLGetD2DTextAntialiasMode(node, D2D1_TEXT_ANTIALIAS_MODE_DEFAULT));
    }
    // 窗口区
    {
        // 默认样式
        DWORD window_style = WS_OVERLAPPEDWINDOW;
        // 设置窗口大小
        RECT window_rect = { 0, 0, LongUIDefaultWindowWidth, LongUIDefaultWindowHeight };
        // 默认
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
        force_cast(this->window_size.width) = window_rect.right;
        force_cast(this->window_size.height) = window_rect.bottom;
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
            //(this->flags & Flag_Window_DComposition) ? WS_EX_NOREDIRECTIONBITMAP : 0,
            WS_EX_NOREDIRECTIONBITMAP,
            LongUI::WindowClassName, 
            titlename.length() ? titlename.c_str() : L"LongUI",
            WS_OVERLAPPEDWINDOW,
            window_rect.left, window_rect.top, window_rect.right, window_rect.bottom,
            parent_window ? parent_window->GetHwnd() : nullptr,
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
            UIManager.ShowError(L"Error! Failed to Create Window", L"LongUI::UIWindow::UIWindow");
        }
    }
    //SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    // 设置Hover
    m_csTME.cbSize = sizeof(m_csTME);
    m_csTME.dwFlags = TME_HOVER | TME_LEAVE;
    m_csTME.hwndTrack = m_hwnd;
    m_csTME.dwHoverTime = LongUIDefaultHoverTime;
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
    // 自己的UI父类就是自己以保证parent不为null
    force_cast(this->parent) = this;
    // 清零
    ::memset(m_dirtyRects, 0, sizeof(m_dirtyRects));
    // 自动显示窗口
    if (node.attribute("autoshow").as_bool(true)) {
        ::ShowWindow(m_hwnd, SW_SHOW);
    }
}

// UIWindow 析构函数
LongUI::UIWindow::~UIWindow() noexcept {
    // 取消注册
    ::RevokeDragDrop(m_hwnd);
    // 杀掉!
    ::KillTimer(m_hwnd, m_idBlinkTimer);
    // 摧毁窗口
    ::DestroyWindow(m_hwnd);
    // 移除窗口
    UIManager.RemoveWindow(this);
    // 释放资源
    this->release_data();
    // 释放数据
    ::SafeRelease(m_pTaskBarList);
    ::SafeRelease(m_pDropTargetHelper);
    ::SafeRelease(m_pCurDataObject);
}


// 注册
void LongUI::UIWindow::RegisterOffScreenRender(UIControl* c, bool is3d) noexcept {
    // 检查
#ifdef _DEBUG
    auto itr = std::find(m_vRegisteredControl.begin(), m_vRegisteredControl.end(), c);
    if (itr != m_vRegisteredControl.end()) {
        UIManager << DL_Warning << L"control: [" << c->GetNameStr() << L"] existed" << LongUI::endl;
        return;
    }
#endif
    try {
        if (is3d) {
            m_vRegisteredControl.insert(m_vRegisteredControl.begin(), c);
        }
        else {
            m_vRegisteredControl.push_back(c);
        }
    }
    catch (...) {
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
        UIManager << DL_Warning << L"control: [" << c->GetNameStr() << L"] not found" << LongUI::endl;
    }
#endif
}


// 设置插入符号
void LongUI::UIWindow::SetCaretPos(UIControl* c, float _x, float _y) noexcept {
    if (!m_cShowCaret) return;
    // 转换为像素坐标
    auto pt = D2D1::Point2F(_x, _y);
    if (c) {
        // FIXME
        // TODO: FIX IT
        pt = LongUI::TransformPoint(c->parent->world, pt);
    }
    m_baBoolWindow.SetTrue(Index_CaretIn);
    const register int intx = static_cast<int>(pt.x);
    const register int inty = static_cast<int>(pt.y);
    const register int oldx = static_cast<int>(m_rcCaretPx.left);
    const register int oldy = static_cast<int>(m_rcCaretPx.top);
    if (oldx != intx || oldy != inty) {
        this->refresh_caret();
        m_rcCaretPx.left = intx; m_rcCaretPx.top = inty;
        ::SetCaretPos(intx, inty);
#if 0
        if (!m_pd2dBitmapAE) return;
        m_pTargetBimtap->CopyFromBitmap(nullptr, m_pd2dBitmapAE, nullptr);
        this->draw_caret();
        /*const register int intw = static_cast<int>(m_rcCaret.width) + 1;
        const register int inth = static_cast<int>(m_rcCaret.height) + 1;
        RECT rects[] = {
            { oldx, oldy, oldx + intw,oldy + inth },
            { intx, inty, intx + intw,inty + inth },
        };*/
        /*::(L"rects: {%d, %d, %d, %d} {%d, %d, %d, %d}\n",
            rects[0].left, rects[0].top, rects[0].right, rects[0].bottom,
            rects[1].left, rects[1].top, rects[1].right, rects[1].bottom
            );*/
        DXGI_PRESENT_PARAMETERS para = { 0, nullptr, nullptr, nullptr };
        // 脏矩形刷新
        m_pSwapChain->Present1(0, 0, &para);
#endif
    }
}

// 创建插入符号
void LongUI::UIWindow::CreateCaret(float width, float height) noexcept {
    this->refresh_caret();
    // TODO: 转换为像素单位
    m_rcCaretPx.width = static_cast<decltype(m_rcCaretPx.height)>(width);
    m_rcCaretPx.height = static_cast<decltype(m_rcCaretPx.width)>(height);
    if (!m_rcCaretPx.width) m_rcCaretPx.width = 1;
    if (!m_rcCaretPx.height) m_rcCaretPx.height = 1;
}

// 显示插入符号
void LongUI::UIWindow::ShowCaret() noexcept {
    ++m_cShowCaret;
    // 创建AE位图
    //if (!m_pd2dBitmapAE) {
        //this->recreate_ae_bitmap();
    //}
}

// 异常插入符号
void LongUI::UIWindow::HideCaret() noexcept { 
    if (m_cShowCaret) {
        --m_cShowCaret;
    }
#ifdef _DEBUG
    else {
        UIManager << DL_Warning << L"m_cShowCaret alread to 0" << LongUI::endl;
    }
#endif
    if (!m_cShowCaret) {
        m_baBoolWindow.SetFalse(Index_CaretIn);
    }
}

// 查找控件
auto LongUI::UIWindow::FindControl(const CUIString& str) noexcept -> UIControl * {
    // 查找控件
    const auto itr = m_mapString2Control.find(str);
    // 未找到返回空
    if (itr == m_mapString2Control.cend()) {
        // 警告
        UIManager << DL_Warning << L"Control Not Found:\n  " << str << LongUI::endl;
        return nullptr;
    }
    // 找到就返回指针
    else {
        return reinterpret_cast<LongUI::UIControl*>(itr->second);
    }
}

// 添加控件
void LongUI::UIWindow::AddControl(const std::pair<CUIString, void*>& pair) noexcept {
    // 有效
    if (pair.first != L"") {
        try {
#ifdef _DEBUG
            // 先检查
            {
                auto itr = m_mapString2Control.find(pair.first);
                if (itr != m_mapString2Control.end()) {
                    UIManager << DL_Warning << "Exist: " << pair.first << LongUI::endl;
                    assert(!"Control Has been existed!");
                }
            }
#endif
            m_mapString2Control.insert(pair);
        }
        catch (...) {
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
        ::CloseHandle(m_hVSync);
        m_hVSync = nullptr;
    }
    // 释放资源
    ::SafeRelease(m_pTargetBimtap);
    ::SafeRelease(m_pSwapChain);
    ::SafeRelease(m_pDcompDevice);
    ::SafeRelease(m_pDcompTarget);
    ::SafeRelease(m_pDcompVisual);
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
#ifdef _DEBUG
        static RECT s_rects[LongUIDirtyControlSize + 2];
        if (this->debug_show) {
            ::memcpy(s_rects, m_dirtyRects, present.DirtyRectsCount * sizeof(RECT));
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
    m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE(m_textAntiMode));
    // 离屏渲染
    if (!m_vRegisteredControl.empty()) {
        for (auto i : m_vRegisteredControl) {
            auto ctrl = reinterpret_cast<UIControl*>(i);
            assert(ctrl->parent && "check it");
            m_pRenderTarget->SetTransform(&ctrl->parent->world);
            ctrl->Render(RenderType::Type_RenderOffScreen);
        }
    }
    // 设为当前渲染对象
    m_pRenderTarget->SetTarget(m_pTargetBimtap);
    // 开始渲染
    m_pRenderTarget->BeginDraw();
    // 设置转换矩阵
#if 0
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
#else
    m_pRenderTarget->SetTransform(&this->world);
#endif
    // 清空背景
    m_pRenderTarget->Clear(this->clear_color);
}

// 结束渲染
void LongUI::UIWindow::EndDraw() const noexcept {
    // 结束渲染
    m_pRenderTarget->EndDraw();
    // 呈现参数设置
    RECT rcScroll = { 0, 0, LONG(this->window_size.width), LONG(this->window_size.height) };
    RECT dirtyRects[LongUIDirtyControlSize + 1]; 
    ::memcpy(dirtyRects, m_dirtyRects, sizeof(dirtyRects));
    DXGI_PRESENT_PARAMETERS present_parameters;
    present_parameters.DirtyRectsCount = 0;
    present_parameters.pDirtyRects = dirtyRects;
    present_parameters.pScrollRect = &rcScroll;
    present_parameters.pScrollOffset = nullptr;
    // 设置参数
    this->set_present_parameters(present_parameters);
    // 呈现
    HRESULT hr = m_pSwapChain->Present1(1, 0, &present_parameters);
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
    AssertHR(hr);
}


// UI窗口: 刷新
void LongUI::UIWindow::Update() noexcept {
    m_baBoolWindow.SetFalse(Index_FullRenderingThisFrame);
    // 新窗口大小?
    if (m_baBoolWindow.Test(Index_NewSize)) {
        this->OnResize();
        m_baBoolWindow.SetFalse(Index_NewSize);
    }
    {
        auto current_unit = m_uiRenderQueue.GetCurrentUnit();
        m_aUnitNow.length = current_unit->length;
        ::memcpy(m_aUnitNow.units, current_unit->units, sizeof(*m_aUnitNow.units) * m_aUnitNow.length);
    }
    // 刷新前
    if (this->IsControlSizeChanged()) {
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
        //UIManager << DL_Hint << "m_present.DirtyRectsCount = 0;" << endl;
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
            // 限制转换
            m_dirtyRects[i].left = static_cast<LONG>(ctrl->visible_rect.left);
            m_dirtyRects[i].top = static_cast<LONG>(ctrl->visible_rect.top);
            m_dirtyRects[i].right = static_cast<LONG>(std::ceil(ctrl->visible_rect.right));
            m_dirtyRects[i].bottom = static_cast<LONG>(std::ceil(ctrl->visible_rect.bottom));
        }
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
}

// UIWindow 渲染 
void LongUI::UIWindow::Render(RenderType type) const noexcept  {
    if (type != RenderType::Type_Render) return ;
    // 全刷新: 继承父类
    if (m_baBoolWindow.Test(Index_FullRenderingThisFrame)) {
        Super::Render(RenderType::Type_Render);
        //UIManager << DL_Hint << "FULL" << endl;
    }
    // 部分刷新:
    else {
        //UIManager << DL_Hint << "DIRT" << endl;
#if 1
        // 先排序
        UIControl* units[LongUIDirtyControlSize];
        size_t length_for_units = 0;
        // 数据
        {
            assert(m_aUnitNow.length < LongUIDirtyControlSize);
            length_for_units = m_aUnitNow.length;
            ::memcpy(units, m_aUnitNow.units, length_for_units * sizeof(void*));
            // 一般就几个, 冒泡完爆std::sort
            LongUI::BubbleSort(units, units + length_for_units, [](UIControl* a, UIControl* b) noexcept {
                return a->priority > b->priority;
            });
            if (m_aUnitNow.length >= 2) {
                assert(units[0]->priority >= units[1]->priority);
            }
        }
        // 再渲染
        auto init_transfrom = D2D1::Matrix3x2F::Identity();
        for (auto unit = units; unit < units + length_for_units; ++unit) {
            auto ctrl = *unit;
            assert(ctrl != this);
            // 设置转换矩阵
            m_pRenderTarget->SetTransform(&init_transfrom);
            m_pRenderTarget->PushAxisAlignedClip(&ctrl->visible_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            // 设置转换矩阵
            m_pRenderTarget->SetTransform(&ctrl->world);
            ctrl->Render(RenderType::Type_Render);
            // 回来
            m_pRenderTarget->PopAxisAlignedClip();
    }
#else
        // 再渲染
        for (uint32_t i = 0ui32; i < m_aUnitNow.length; ++i) {
            auto ctrl = m_aUnitNow.units[i];
            // 设置转换矩阵
            D2D1_MATRIX_3X2_F matrix; ctrl->GetWorldTransform(matrix);
            m_pRenderTarget->SetTransform(&matrix);
            ctrl->Render(RenderType::Type_Render);
        }
#endif
    }
#ifdef _DEBUG
    // 调试输出
    if (this->debug_show) {
        D2D1_MATRIX_3X2_F nowMatrix, iMatrix = D2D1::Matrix3x2F::Scale(0.45f, 0.45f);
        m_pRenderTarget->GetTransform(&nowMatrix);
        m_pRenderTarget->SetTransform(&iMatrix);
        wchar_t buffer[1024];
        auto length = ::swprintf(
            buffer, 1024,
            L"Full Rendering Count: %d\nDirty Rendering Count: %d\nThis DirtyRectsCount:%d",
            full_render_counter,
            dirty_render_counter,
            m_aUnitNow.length
            );
        auto tf = UIManager.GetTextFormat(LongUIDefaultTextFormatIndex);
        auto ta = tf->GetTextAlignment();
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        tf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        m_pRenderTarget->DrawText(
            buffer, length, tf,
            D2D1::RectF(0.f, 0.f, 1000.f, 70.f),
            m_pBrush_SetBeforeUse
            );
        tf->SetTextAlignment(ta);
        ::SafeRelease(tf);
        m_pRenderTarget->SetTransform(&nowMatrix);
    }
#endif
}

// UIWindow 事件处理
bool LongUI::UIWindow::DoEvent(const LongUI::EventArgument& _arg) noexcept {
    // 自己一般不处理LongUI事件
    if (_arg.sender) return Super::DoEvent(_arg);
    // 其他LongUI事件
    bool handled = false; UIControl* control_got = nullptr;
    // 特殊事件
    if (_arg.msg == s_uTaskbarBtnCreatedMsg) {
        ::SafeRelease(m_pTaskBarList);
        UIManager << DL_Log << "TaskbarButtonCreated" << endl;
        auto hr = ::CoCreateInstance(
            CLSID_TaskbarList,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pTaskBarList)
            );
        AssertHR(hr);
        return true;
    }
    // 处理事件
    switch (_arg.msg)
    {
        LongUI::EventArgument new_arg;
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
    case WM_MOUSEMOVE:
        handled = this->OnMouseMove(_arg);
        break;
    case WM_MOUSEWHEEL:
        handled = this->OnMouseWheel(_arg);
        break;
    case WM_TIMER:
        // 闪烁?
        if (_arg.sys.wParam == BLINK_EVENT_ID) {
            if (m_cShowCaret) {
                m_baBoolWindow.SetNot(Index_CaretIn);
            }
            handled = true;
        }
        break;
    case WM_LBUTTONDOWN:    // 按下鼠标左键
        // 查找子控件
        control_got = this->FindControl(_arg.pt);
        // 控件有效
        if (control_got && control_got != m_pFocusedControl) {
            new_arg = _arg;
            new_arg.sender = this;
            if (m_pFocusedControl){
                new_arg.event = LongUI::Event::Event_KillFocus;
                m_pFocusedControl->DoEvent(new_arg);
            }
            new_arg.event = LongUI::Event::Event_SetFocus;
            // 控件响应了?
            m_pFocusedControl = control_got->DoEvent(new_arg) ? control_got : nullptr;
        }
        break;
    /*case WM_NCHITTEST:
        _arg.lr = HTCAPTION;
        handled = true;
        break;*/
    case WM_SETFOCUS:
        ::CreateCaret(m_hwnd, nullptr, 1, 1);
        handled = true;
        break;
    case WM_KILLFOCUS:
        // 存在焦点控件
        if (m_pFocusedControl){
            new_arg = _arg;
            new_arg.sender = this;
            new_arg.event = LongUI::Event::Event_KillFocus;
            m_pFocusedControl->DoEvent(new_arg);
            m_pFocusedControl = nullptr;
        }
        ::DestroyCaret();
        handled = true;
        break;
    case WM_MOUSELEAVE:     // 鼠标移出窗口
        if (m_pPointedControl){
            new_arg = _arg;
            new_arg.sender = this;
            new_arg.event = LongUI::Event::Event_MouseLeave;
            m_pPointedControl->DoEvent(new_arg);
            m_pPointedControl = nullptr;
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
        reinterpret_cast<MINMAXINFO*>(_arg.sys.lParam)->ptMinTrackSize.x = m_miniSize.width;
        reinterpret_cast<MINMAXINFO*>(_arg.sys.lParam)->ptMinTrackSize.y = m_miniSize.height;
        break;
    case WM_DISPLAYCHANGE:
        UIManager << DL_Hint << "WM_DISPLAYCHANGE" << endl;
        {
            // 获取屏幕刷新率
            DEVMODEW mode = { 0 };
            ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
            m_uiRenderQueue.Reset(mode.dmDisplayFrequency);
        }
        // 强行刷新一帧
        this->Invalidate(this);
        break;
    case WM_CLOSE:          // 关闭窗口
        // 窗口关闭
        handled = this->OnClose();
        break;
    }
    // 处理
    if (handled) return true;
    // 处理控件
    register UIControl* processor = nullptr;
    // 鼠标事件交由捕获控件(优先)或者鼠标指向控件处理
    if (_arg.msg >= WM_MOUSEFIRST && _arg.msg <= WM_MOUSELAST) {
        processor = m_pCapturedControl ? m_pCapturedControl : m_pPointedControl;
    }
    // 其他事件交由焦点控件处理
    else {
        processor = m_pFocusedControl;
    }
    // 有就处理
    if (processor && processor->DoEvent(_arg)) {
        return true;
    }
    // 还是没有处理就交给父类处理
    return Super::DoEvent(_arg);
}

// 等待重置同步
void LongUI::UIWindow::WaitVS() const noexcept {
#ifdef _DEBUG
    static bool first_time = true;
    if (first_time && !m_baBoolWindow.Test(Index_Rendered)) {
        assert(!"should be rendered @ first time !");
    }
    first_time = false;
    // 渲染?
    if (m_baBoolWindow.Test(Index_Rendered)) {
        // 等待VS
        ::WaitForSingleObject(m_hVSync, INFINITE);
    }
    else {
        assert(!"error!");
    }
#else
    ::WaitForSingleObject(m_hVSync, INFINITE);
#endif
}

// 重置窗口大小
void LongUI::UIWindow::OnResize(bool force) noexcept {
    assert(this->window_type != Type_RenderOnParent);
    if (this->window_type != Type_Layered) {
        force = true;
    }
    //UIManager << DL_Log << "called" << endl;
    // 修改大小, 需要取消目标
    m_pRenderTarget->SetTarget(nullptr);
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
    register HRESULT hr = S_OK;
    // 强行 或者 小于才Resize
    if (force || old_size.width < uint32_t(rect_right) || old_size.height < uint32_t(rect_bottom)) {
        UIManager << DL_Hint << L"Window: [" << this->GetNameStr() << L"] \n\t\tTarget Bitmap Resize to " 
            << long(rect_right) << ", " << long(rect_bottom) << LongUI::endl;
        IDXGISurface* pDxgiBackBuffer = nullptr;
        ::SafeRelease(m_pTargetBimtap);
        hr = m_pSwapChain->ResizeBuffers(
            2, rect_right, rect_bottom, DXGI_FORMAT_B8G8R8A8_UNORM, 
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
            );
        // 检查
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            UIManager.RecreateResources();
            UIManager << DL_Hint << L"Recreate device" << LongUI::endl;
        }
        // 利用交换链获取Dxgi表面
        if (SUCCEEDED(hr)) {
            hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
        }
        // 利用Dxgi表面创建位图
        if (SUCCEEDED(hr)) {
            D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                LongUI::GetDpiX(),
                LongUI::GetDpiY()
                );
            hr = m_pRenderTarget->CreateBitmapFromDxgiSurface(
                pDxgiBackBuffer,
                &bitmapProperties,
                &m_pTargetBimtap
                );
        }
        // 重建失败?
        if (FAILED(hr)) {
            UIManager << DL_Error << L" Recreate FAILED!" << LongUI::endl;
            AssertHR(hr);
        }
        ::SafeRelease(pDxgiBackBuffer);
    }
    // 强行刷新一帧
    this->Invalidate(this);
}

// UIWindow 重建
auto LongUI::UIWindow::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    // UIWindow::Recreate参数不会为nullptr
    assert(newRT && "bad argument");
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
        if (this->window_type == Type_Layered) {
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
        }
    }
    // 获取交换链V2
    if (SUCCEEDED(hr)) {
#ifdef LONGUI_USE_SDK_8_1
#define DEFINE_GUID_LONGUI(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name  = { l, w1, w2,{ b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
        DEFINE_GUID_LONGUI(IID_IDXGISwapChain2_Auto, 0xa8be2ac4, 0x199f, 0x4946, 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7);
        hr = pSwapChain->QueryInterface(
            IID_IDXGISwapChain2_Auto,
            reinterpret_cast<void**>(&m_pSwapChain)
            );
#else
        hr = pSwapChain->QueryInterface(
            IID_IDXGISwapChain2,
            reinterpret_cast<void**>(&m_pSwapChain)
            );
#endif
    }
    // 获取垂直等待事件
    if (SUCCEEDED(hr)) {
        m_hVSync = m_pSwapChain->GetFrameLatencyWaitableObject();
    }
    /*// 确保DXGI队列里边不会超过一帧
    if (SUCCEEDED(hr)) {
        hr = UIManager_DXGIDevice->SetMaximumFrameLatency(1);
    }*/
    // 利用交换链获取Dxgi表面
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
    }
    // 利用Dxgi表面创建位图
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            LongUI::GetDpiX(),
            LongUI::GetDpiY()
            );
        hr = newRT->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pTargetBimtap
            );
    }
    // 使用DComp
    if (this->window_type == Type_Layered) {
        // 创建直接组合(Direct Composition)设备
        if (SUCCEEDED(hr)) {
            hr = LongUI::Dll::DCompositionCreateDevice(
                UIManager_DXGIDevice,
                LongUI_IID_PV_ARGS(m_pDcompDevice)
                );
        }
        // 创建直接组合(Direct Composition)目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateTargetForHwnd(
                m_hwnd, true, &m_pDcompTarget
                );
        }
        // 创建直接组合(Direct Composition)视觉
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateVisual(&m_pDcompVisual);
        }
        // 设置当前交换链为视觉内容
        if (SUCCEEDED(hr)) {
            hr = m_pDcompVisual->SetContent(m_pSwapChain);
        }
        // 设置当前视觉为窗口目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompTarget->SetRoot(m_pDcompVisual);
        }
        // 向系统提交
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->Commit();
        }
    }
    // 错误
    if (FAILED(hr)){
        UIManager << L"Recreate Failed!" << LongUI::endl;
        AssertHR(hr);
    }
    ::SafeRelease(pDxgiBackBuffer);
    ::SafeRelease(pSwapChain);
    {
        // 获取屏幕刷新率
        DEVMODEW mode = { 0 };
        ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
        m_uiRenderQueue.Reset(mode.dmDisplayFrequency);
        // 强行刷新一帧
        this->Invalidate(this);
    }
    // 重建 子控件UI
    return Super::Recreate(newRT);
}

// UIWindow 关闭控件
void LongUI::UIWindow::Cleanup() noexcept {
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
bool LongUI::UIWindow::OnMouseMove(const LongUI::EventArgument& arg) noexcept {
    bool handled = false;
    do {
        ::TrackMouseEvent(&m_csTME);
        if (m_normalLParam != arg.sys.lParam) {
            m_normalLParam = arg.sys.lParam;
        }
        else {
            handled = true;
            break;
        }
        // 有待捕获控件
        if (m_pCapturedControl) {
            m_pCapturedControl->DoEvent(arg);
            handled = true;
            break;
        }
        // 查找子控件
        auto control_got = this->FindControl(arg.pt);
        if (control_got) {
            //UIManager << DL_Hint << "FIND: " << control_got << endl;
        }
        // 不同
        if (control_got != m_pPointedControl) {
            auto new_arg = arg;
            new_arg.sender = this;
            // 有效
            if (m_pPointedControl) {
                new_arg.event = LongUI::Event::Event_MouseLeave;
                m_pPointedControl->DoEvent(new_arg);
            }
            // 有效
            if ((m_pPointedControl = control_got)) {
                new_arg.event = LongUI::Event::Event_MouseEnter;
                m_pPointedControl->DoEvent(new_arg);
            }
        }
        // 相同
        else if (control_got) {
            control_got->DoEvent(arg);
        }
        handled = true;
    } while (false);
    return handled;
}

// 鼠标滚轮
bool LongUI::UIWindow::OnMouseWheel(const LongUI::EventArgument& arg) noexcept {
    auto loww = LOWORD(arg.sys.wParam);
    //auto delta = float(GET_WHEEL_DELTA_WPARAM(arg.sys.wParam)) / float(WHEEL_DELTA);
    // 鼠标滚轮事件交由有滚动条的容器处理
    if (loww & MK_CONTROL) {

    }
    // Alt + wheel?
    else if (loww & MK_ALT) {

    }
    // 水平滚动条
    else if (loww & MK_SHIFT) {
        /*auto basic_control = this->FindControl(arg.pt);
        if (basic_control) {
            // 获取滚动条容器
            while (true) {
                if (basic_control->IsTopLevel()) {
                    break;
                }
                if (basic_control->flags & Flag_UIContainer) {
                    if (static_cast<UIContainer*>(basic_control)->scrollbar_h) {
                        break;
                    }
                }
                basic_control = basic_control->parent;
            }
            // 存在
            if (static_cast<UIContainer*>(basic_control)->scrollbar_h) {
                static_cast<UIContainer*>(basic_control)->scrollbar_h->OnWheelX(-delta);
            }
        }*/
    }
    // 垂直滚动条
    else {
        /*auto basic_control = this->FindControl(arg.pt);
        if (basic_control) {
            // 获取滚动条容器
            while (true) {
                if (basic_control->IsTopLevel()) {
                    break;
                }
                if (basic_control->flags & Flag_UIContainer) {
                    if (static_cast<UIContainer*>(basic_control)->scrollbar_v) {
                        break;
                    }
                }
                basic_control = basic_control->parent;
            }
            // 存在
            if (static_cast<UIContainer*>(basic_control)->scrollbar_v) {
                static_cast<UIContainer*>(basic_control)->scrollbar_v->OnWheelX(-delta);
            }
        }*/
    }
    return true;
}

// ----------------- IDropTarget!!!! Yooooooooooo~-----

// 设置参数
void __fastcall SetLongUIEventArgument(LongUI::EventArgument& arg, HWND hwnd, POINTL pt) {
    // 获取窗口位置
    RECT rc = { 0 }; ::GetWindowRect(hwnd, &rc);
    // 映射到窗口坐标
    POINT ppt = { pt.x, pt.y };  ::ScreenToClient(hwnd, &ppt);
    // 查找对应控件
    arg = { 0 };
    arg.pt.x = static_cast<float>(ppt.x);
    arg.pt.y = static_cast<float>(ppt.y);

}

// 获取拖放效果
DWORD __fastcall GetDropEffect(DWORD grfKeyState, DWORD dwAllowed) {
    register DWORD dwEffect = 0;
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
        LongUI::EventArgument arg = { 0 };
        arg.sender = this;
        arg.event = LongUI::Event::Event_KillFocus;
        m_pFocusedControl->DoEvent(arg);
        m_pFocusedControl = nullptr;
    }
    // 保留数据
    ::SafeRelease(m_pCurDataObject);
    m_pCurDataObject = ::SafeAcquire(pDataObj);
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
    UIControl* control = nullptr;
    // 检查控件支持
    if ((control = this->FindControl(pt2f))) {
        LongUI::EventArgument arg;
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
        if (!control->DoEvent(arg)) *pdwEffect = DROPEFFECT_NONE;
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
        LongUI::EventArgument arg = { 0 };
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
        LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
        arg.sender = this;
        arg.event = LongUI::Event::Event_Drop;
        arg.cf.dataobj = m_pCurDataObject;
        arg.cf.outeffect = pdwEffect;
        // 发送事件
        m_pDragDropControl->DoEvent(arg);
        m_pDragDropControl = nullptr;
        
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
