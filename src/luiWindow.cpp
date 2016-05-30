#include "LongUI/luiUiXml.h"
#include "Core/luiWindow.h"
#include "Core/luiManager.h"
#include "LongUI/luiUiHlper.h"
#include "Platless/luiPlUtil.h"
#include "Platless/luiPlHlper.h"
#include "Graphics/luiGrD2d.h"
#include "Control/UIViewport.h"
#include <dcomp.h>
#include <algorithm>

// longui::impl
namespace LongUI { namespace impl {
    // 2x char16 to char32
    inline auto char16x2_to_char32(char16_t lead, char16_t trail) noexcept -> char32_t {
        assert(IsHighSurrogate(lead) && "illegal utf-16 char");
        assert(IsLowSurrogate(trail) && "illegal utf-16 char");
        return char32_t((lead-0xD800) << 10 | (trail-0xDC00)) + (0x10000);
    };
}}



/// <summary>
/// Canbe the closed now?
/// </summary>
/// <returns></returns>
bool LongUI::UIViewport::CanbeClosedNow() noexcept {
    return true;
}

/// <summary>
/// Called when [close].
/// </summary>
/// <returns></returns>
void LongUI::UIViewport::OnClose() noexcept {
    UIManager.Exit();
}


/// <summary>
/// Initializes a new instance of the <see cref="UIViewport"/> class.
/// </summary>
/// <param name="window">The window.</param>
LongUI::UIViewport::UIViewport(XUIBaseWindow* window) noexcept : Super(nullptr) {
    assert(window && "bad argument");
    m_pWindow = window;
}

/// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The xml node.</param>
/// <returns></returns>
void LongUI::UIViewport::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 初始化
    assert(node && "<LongUI::UIViewport::initialize> window_node null");
    // 检查名称
    {
        // 浮点视区大小
        if (this->view_size.width == 0.f) {
            force_cast(this->view_size.width) = static_cast<float>(LongUIDefaultWindowWidth);
        }
        // 更新
        if (this->view_size.height == 0.f) {
            force_cast(this->view_size.height) = static_cast<float>(LongUIDefaultWindowHeight);
        }
        // 可视区域范围
        visible_rect.right = this->view_size.width;
        visible_rect.bottom = this->view_size.height;
        m_2fContentSize = this->view_size;
    }
}

/// <summary>
/// Initializes the specified size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIViewport::initialize(D2D1_SIZE_U size) noexcept {
    // 父类
    Super::initialize();
    // 更新大小
    force_cast(this->view_size.width) = static_cast<float>(size.width);
    force_cast(this->view_size.height) = static_cast<float>(size.height);
    // 可视区域范围
    visible_rect.right = this->view_size.width;
    visible_rect.bottom = this->view_size.height;
    m_2fContentSize = this->view_size;
}


// UIViewport 关闭控件
void LongUI::UIViewport::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 删除对象
    delete this;
}

/// <summary>
/// Creates the without XML.
/// </summary>
/// <returns></returns>
auto LongUI::UIViewport::CreateWithoutXml(XUIBaseWindow* wnd, D2D1_SIZE_U size) noexcept -> UIViewport* {
    auto viewport = new(std::nothrow) UIViewport(wnd);
    if (viewport) viewport->initialize(size);
    return viewport;
}


#if 0
/// <summary>
/// Initializes the specified popup.
/// </summary>
/// <param name="popup">The popup.</param>
/// <returns></returns>
void LongUI::UIViewport::initialize(const Config::Popup& popup) noexcept {
    assert(this->wndparent && "this->wndparent cannot be null while in popup window");
#ifdef _DEBUG
    this->debug_show = this->wndparent->debug_show;
    this->debug_this = this->wndparent->debug_this;
#endif
    // 失去焦点即关闭
    m_baBoolWindow.SetTrue(UIViewport::Index_CloseOnFocusKilled);
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
        RECT window_rect = { 0, 0, LONG(popup.width), LONG(popup.height) };
        // 浮点视区大小
        force_cast(this->view_size.width) = popup.width;
        force_cast(this->view_size.height) = popup.height;
        // 整数窗口大小
        force_cast(this->window_size.width) = uint32_t(window_rect.right);
        force_cast(this->window_size.height) = uint32_t(window_rect.bottom);
        // 可视区域范围
        visible_rect.right = this->view_size.width;
        visible_rect.bottom = this->view_size.height;
        m_2fContentSize = this->view_size;
        // 调整大小
        ::AdjustWindowRect(&window_rect, window_style, FALSE);
        window_rect.bottom -= window_rect.top;
        window_rect.right -= window_rect.left;
        // 创建窗口
        m_hwnd = ::CreateWindowExW(
            //WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            WS_EX_NOREDIRECTIONBITMAP,
            LongUI::WindowClassName,
            L"LongUI Popup Window",
            window_style,
            0, 0, window_rect.right, window_rect.bottom,
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


/// <summary>
/// Creates the popup window
/// </summary>
/// <returns></returns>
auto LongUI::UIViewport::CreatePopup(const Config::Popup& popup) noexcept -> UIViewport* {
    assert(!"NOIMPL");
    assert(popup.parent && "bad argument");
    CUIDxgiAutoLocker locker;
    auto window = new(std::nothrow) UIViewport(popup.parent);
    // TODO: 下面空间不足则将窗口移动到控件上面
    // 内存申请成功
    if (window) {
        // 初始化
        window->initialize(popup);
        // 添加子节点
        if (popup.child) {
            popup.child->LinkNewParent(window);
            window->Push(popup.child);
#ifdef _DEBUG
            force_cast(window->name) = window->CopyString("PopupWindow");
#endif
        }
        // 重建
        auto hr = window->Recreate(); ShowHR(hr);
        // 创建完毕
        window->DoLongUIEvent(Event::Event_TreeBuildingFinished);
        // 移动窗口
        window->MoveWindow(popup.leftline, popup.bottomline);
    }
    return nullptr;
}

// 设置插入符号
void LongUI::UIViewport::SetCaretPos(UIControl* ctrl, float _x, float _y) noexcept {
    if (!m_cShowCaret) return;
    assert(ctrl && "bad argument");
    // 转换为像素坐标
    D2D1_POINT_2F pt { _x, _y };
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
void LongUI::UIViewport::CreateCaret(UIControl* ctrl, float width, float height) noexcept {
    assert(ctrl && "bad argument");
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
void LongUI::UIViewport::ShowCaret() noexcept {
    //::ShowCaret(m_hwnd);
    ++m_cShowCaret;
    // 创建AE位图
    //if (!m_pd2dBitmapAE) {
        //this->recreate_ae_bitmap();
    //}
}

// 异常插入符号
void LongUI::UIViewport::HideCaret() noexcept {
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
auto LongUI::UIViewport::FindControl(const char* cname) noexcept -> UIControl * {
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

// 移动窗口
void LongUI::UIViewport::MoveWindow(float x, float y) noexcept {
    POINT p = { static_cast<int>(x), static_cast<int>(y) };
    HWND hwndp = ::GetParent(m_hwnd);
    ::MapWindowPoints(hwndp, m_hwnd, &p, 1);
    ::SetWindowPos(m_hwnd, HWND_TOP, p.x, p.y, 0, 0, SWP_NOSIZE);
}

// 添加命名控件
void LongUI::UIViewport::AddNamedControl(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argumrnt");
    const auto cname = ctrl->name.c_str();
    // 有效

    if (cname[0]) {
        // 插入
        if (!m_hashStr2Ctrl.Insert(cname, ctrl)) {
            ShowErrorWithStr(L"Failed to add control");
        }
    }
}

// 设置hover跟踪控件
void LongUI::UIViewport::SetHoverTrack(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argument");
    if (ctrl && ctrl->GetHoverTrackTime()) {
        LongUI::SafeAcquire(ctrl);
        LongUI::SafeRelease(m_pHoverTracked);
        m_pHoverTracked = ctrl;
    }
}


// 设置捕获控件
void LongUI::UIViewport::SetCapture(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argument");
    ::SetCapture(m_hwnd);
    // 设置引用计数
    LongUI::SafeAcquire(ctrl);
    LongUI::SafeRelease(m_pCapturedControl);
    m_pCapturedControl = ctrl;
};

// 释放捕获控件
void LongUI::UIViewport::ReleaseCapture() noexcept {
    ::ReleaseCapture();
    LongUI::SafeRelease(m_pCapturedControl);
};

// 设置图标
void LongUI::UIViewport::SetIcon(HICON hIcon) noexcept {
    ::DefWindowProcW(m_hwnd, WM_SETICON, TRUE, reinterpret_cast<LPARAM>(hIcon));
    ::DefWindowProcW(m_hwnd, WM_SETICON, FALSE, reinterpret_cast<LPARAM>(hIcon));
}

// release data
void LongUI::UIViewport::release_data() noexcept {
    if (m_hVSync) {
        ::SetEvent(m_hVSync);
        ::CloseHandle(m_hVSync);
        m_hVSync = nullptr;
    }
    // 释放资源
    LongUI::SafeRelease(m_pTargetBitmap);
    LongUI::SafeRelease(m_pSwapChain);
    LongUI::SafeRelease(m_pDcompDevice);
    LongUI::SafeRelease(m_pDcompTarget);
    LongUI::SafeRelease(m_pDcompVisual);
}

// 刻画插入符号
void LongUI::UIViewport::draw_caret() noexcept {
    /*// 不能在BeginDraw/EndDraw之间调用
    D2D1_POINT_2U pt = { m_rcCaretPx.left, m_rcCaretPx.top };
    D2D1_RECT_U src_rect;
    src_rect.top = LongUIWindowPlanningBitmap / 2;
    src_rect.left = m_bCaretIn ? 0 : LongUIWindowPlanningBitmap / 4;
    src_rect.right = src_rect.left + m_rcCaretPx.width;
    src_rect.bottom = src_rect.top + m_rcCaretPx.height;
    m_pTargetBitmap->CopyFromBitmap(
        &pt, m_pBitmapPlanning, &src_rect
        );*/
}

// 更新插入符号
void LongUI::UIViewport::refresh_caret() noexcept {
    // 不能在BeginDraw/EndDraw之间调用
    // TODO: 完成位图复制
}

// 设置呈现
void LongUI::UIViewport::set_present_parameters(DXGI_PRESENT_PARAMETERS& present) const noexcept {
    present.DirtyRectsCount = static_cast<uint32_t>(m_aUnitNow.length);
    // 存在脏矩形?
    if (!m_baBoolWindow.Test(Index_FullRenderingThisFrame)) {
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
        // TODO: 从根源优化掉这个过程
        // 遍历检查
        LONG mw = this->window_size.width;
        LONG mh = this->window_size.height;
        for (UINT i = 0; i < present.DirtyRectsCount; ++i) {
            present.pDirtyRects[i].right = std::min(mw, present.pDirtyRects[i].right);
            present.pDirtyRects[i].bottom = std::min(mw, present.pDirtyRects[i].bottom);
        }
    }
    // 全刷新
    else {
        present.pScrollRect = nullptr;
        present.DirtyRectsCount = 0;
    }
}

// begin draw
void LongUI::UIViewport::BeginDraw() const noexcept {
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
    UIManager_RenderTarget->SetTarget(m_pTargetBitmap);
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
void LongUI::UIViewport::EndDraw() const noexcept {
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
void LongUI::UIViewport::Update() noexcept {
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

// UIViewport 渲染 
void LongUI::UIViewport::Render() const noexcept {
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
            UIManager_RenderTarget->PushAxisAlignedClip(reinterpret_cast<D2D1_RECT_F*>(clipr), D2D1_ANTIALIAS_MODE_ALIASED);
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
            D2D1_RECT_F{ 0.f, 0.f, 1000.f, 70.f },
            m_pBrush_SetBeforeUse
        );
        tf->SetTextAlignment(ta);
        LongUI::SafeRelease(tf);
        UIManager_RenderTarget->SetTransform(&nowMatrix);
    }
#endif
}

// 退出时
bool LongUI::UIViewport::OnClose() noexcept {
    // 延迟清理
    this->delay_cleanup();
    // 退出程序?
    if (m_baBoolWindow.Test(UIViewport::Index_ExitOnClose)) {
        UIManager.Exit();
    }
    return true;
};

// UIViewport 事件处理
bool LongUI::UIViewport::DoEvent(const LongUI::EventArgument& arg) noexcept {
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
    // 其他LongUI事件
    bool handled = false;
    // 特殊事件
    /*if (arg.msg == s_uTaskbarBtnCreatedMsg) {
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
    }*/
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
        if (m_pFocusedControl) {
            // 事件
            m_pFocusedControl->DoLongUIEvent(Event::Event_KillFocus);
            // 释放引用
            LongUI::SafeRelease(m_pFocusedControl);
        }
        ::DestroyCaret();
        // 失去焦点即关闭窗口
        if (m_baBoolWindow.Test(UIViewport::Index_CloseOnFocusKilled)) {
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
        this->InvalidateThis;
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
bool LongUI::UIViewport::DoMouseEvent(const MouseEventArgument& arg) noexcept {
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
void LongUI::UIViewport::SetFocus(UIControl* ctrl) noexcept {
    // 无效
    assert(ctrl && "bad argument");
    // 可聚焦的
    if (ctrl && ctrl->flags & Flag_Focusable) {
        // 有效
        if (m_pFocusedControl) {
            // 事件
            m_pFocusedControl->DoLongUIEvent(Event::Event_KillFocus);
            // 去除引用
            m_pFocusedControl->Release();
        }
        // 有效
        if ((m_pFocusedControl = ctrl)) {
            // 增加引用
            m_pFocusedControl->AddRef();
            // 事件
            m_pFocusedControl->DoLongUIEvent(Event::Event_SetFocus);
        }
    }
}

// 重置窗口大小
void LongUI::UIViewport::OnResize(bool force) noexcept {
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
    auto old_size = m_pTargetBitmap->GetPixelSize();
    HRESULT hr = S_OK;
    // 强行 或者 小于才Resize
    if (force || old_size.width < uint32_t(rect_right) || old_size.height < uint32_t(rect_bottom)) {
        UIManager << DL_Hint << L"Window: ["
            << this->name
            << L"]\tTarget Bitmap Resize to "
            << LongUI::Formated(L"(%d, %d)", int(rect_right), int(rect_bottom))
            << LongUI::endl;
        IDXGISurface* pDxgiBackBuffer = nullptr;
        LongUI::SafeRelease(m_pTargetBitmap);
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
                &m_pTargetBitmap
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
    this->InvalidateThis;
}

// UIViewport 重建
auto LongUI::UIViewport::Recreate() noexcept ->HRESULT {
    // 跳过
    if (m_baBoolWindow.Test(Index_SkipRender)) return S_OK;
    // 渲染锁
    CUIDxgiAutoLocker locker;
    // 释放数据
    this->release_data();
    // DXGI Surface 后台缓冲
    IDXGISurface*               pDxgiBackBuffer = nullptr;
    IDXGISwapChain1*            pSwapChain = nullptr;
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
            &m_pTargetBitmap
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
    if (FAILED(hr)) {
        UIManager << L"Recreate Failed!" << LongUI::endl;
        ShowHR(hr);
    }
    LongUI::SafeRelease(pDxgiBackBuffer);
    LongUI::SafeRelease(pSwapChain);
    {
        // 获取屏幕刷新率
        m_uiRenderQueue.Reset(UIManager.GetDisplayFrequency());
        // 强行刷新一帧
        this->InvalidateThis;
    }
    // 重建 子控件UI
    return Super::Recreate();
}


// 窗口创建时
bool LongUI::UIViewport::OnCreated(HWND hwnd) noexcept {
    assert(!"removed");
    // 权限提升?保证
    CHANGEFILTERSTRUCT cfs = { sizeof(CHANGEFILTERSTRUCT) };
    //::ChangeWindowMessageFilterEx(hwnd, s_uTaskbarBtnCreatedMsg, MSGFLT_ALLOW, &cfs);
    return true;
}
#endif

// 鼠标移动时候
/*bool LongUI::UIViewport::OnMouseMove(const LongUI::EventArgument& arg) noexcept {
}

// 鼠标滚轮
bool LongUI::UIViewport::OnMouseWheel(const LongUI::EventArgument& arg) noexcept {
}*/

// ----------------- IDropTarget!!!! Yooooooooooo~-----

#if 0
// 获取拖放效果
/*DWORD GetDropEffect(DWORD grfKeyState, DWORD dwAllowed) {
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
HRESULT  LongUI::UIViewport::DragEnter(IDataObject* pDataObj,
    DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) noexcept {
    UNREFERENCED_PARAMETER(grfKeyState);
    m_baBoolWindow.SetTrue(Index_InDraging);
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    // 取消聚焦窗口
    if (m_pFocusedControl){
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
HRESULT LongUI::UIViewport::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
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
        if (!control->DoEvent(arg)) *pdwEffect = DROPEFFECT_NONE;*
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
HRESULT LongUI::UIViewport::DragLeave(void) noexcept {
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
        }
    }
    /*OnDragLeave(m_hTargetWnd);*
    m_pDragDropControl = nullptr;
    //m_isDataAvailable = TRUE;
    if (m_pDropTargetHelper) {
        m_pDropTargetHelper->DragLeave();
    }
    m_baBoolWindow.SetFalse(Index_InDraging);
    return S_OK;
}

// IDropTarget::Drop 实现
HRESULT LongUI::UIViewport::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
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
        m_pDragDropControl = nullptr;*

    }
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    if (m_pDropTargetHelper){
        POINT ppt = { pt.x, pt.y };
        m_pDropTargetHelper->Drop(pDataObj, &ppt, *pdwEffect);
    }
    *pdwEffect = ::GetDropEffect(grfKeyState, *pdwEffect);
    return S_OK;
}*/
#endif

// -----------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the <see cref="XUIBaseWindow"/> class.
/// </summary>
LongUI::XUIBaseWindow::XUIBaseWindow(const Config::Window& config) noexcept : m_pParent(config.parent) {
    // Xml节点
    auto node = config.node;
#ifdef _DEBUG
    m_baBoolWindow.Test(INDEX_COUNT);
    m_vInsets.push_back(nullptr);
    m_vInsets.pop_back();
#endif
    // Debug Zone
#ifdef _DEBUG
    {
        this->debug_show = node.attribute("debugshow").as_bool(false);
    }
#endif
    // 其他属性
    {
        // 最小大小
        float size[] = { LongUIWindowMinSize, LongUIWindowMinSize };
        Helper::MakeFloats(node.attribute("minisize").value(), size, 2);
        m_miniSize.x = static_cast<LONG>(size[0]);
        m_miniSize.y = static_cast<LONG>(size[1]);
        // 清理颜色
        Helper::MakeColor(
            node.attribute(LongUI::XmlAttribute::WindowClearColor).value(),
            this->clear_color
        );
        // 文本抗锯齿
        m_textAntiMode = uint16_t(Helper::GetEnumFromXml(node, D2D1_TEXT_ANTIALIAS_MODE_DEFAULT));
    }

    //SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    // 创建闪烁计时器
    //m_idBlinkTimer = ::SetTimer(m_hwnd, BLINK_EVENT_ID, ::GetCaretBlinkTime(), nullptr);
    // 拖放帮助器
    //m_pDropTargetHelper = UIManager.GetDropTargetHelper();
    // 失去焦点关闭窗口
    if (config.popup) {
        this->set_close_on_focus_killed();
    }
    // 关闭时退出
    /*else if (node.attribute("exitonclose").as_bool(true)) {
        this->set_exit_on_close();
    }*/
    const char* str = nullptr;
    // 插入符号闪烁时间
    if ((str = node.attribute("caretblinktime").value())) {
        m_tmCaret.Reset(LongUI::AtoI(str));
    }
    // 高DPI处理策略
    if (node.attribute("hidpi").as_bool(true)) {
        this->set_hidpi_supported();
    }
}


#ifdef _DEBUG
namespace LongUI {
    struct Msg { UINT id; }; 
    std::atomic_uintptr_t g_dbg_last_proc_window_pointer = 0;
    std::atomic<UINT> g_dbg_last_proc_message = 0;
}
#endif


/// <summary>
/// Finalizes an instance of the <see cref="XUIBaseWindow"/> class.
/// </summary>
/// <returns></returns>
LongUI::XUIBaseWindow::~XUIBaseWindow() noexcept {
    for (auto inset : m_vInsets) {
        inset->Dispose();
    }
    LongUI::SafeRelease(m_pViewport);
    LongUI::SafeRelease(m_pHoverTracked);
    LongUI::SafeRelease(m_pFocusedControl);
    LongUI::SafeRelease(m_pDragDropControl);
    LongUI::SafeRelease(m_pCapturedControl);
#ifdef _DEBUG
    auto wptr = reinterpret_cast<std::uintptr_t>(this);
    if (g_dbg_last_proc_window_pointer == wptr) {
        g_dbg_last_proc_window_pointer = 0;
    }
#endif
}

/// <summary>
/// Closes this instance.
/// </summary>
/// <returns></returns>
void LongUI::XUIBaseWindow::on_close() noexcept {
    // 退出窗口
    if (!this->is_close_on_focus_killed()) {
        m_pViewport->OnClose();
    }
    // 延迟清理
    UIManager.PushDelayCleanup(this);
    // 跳过渲染
    this->set_skip_render();
}


/// <summary>
/// Creates the popup.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="height">The height.</param>
/// <returns></returns>
auto LongUI::XUIBaseWindow::CreatePopup(const D2D1_RECT_L& pos, uint32_t height, UIControl* child) noexcept -> XUIBaseWindow* {
    assert(pos.right - pos.left && "bad width");
    assert(height && "bad height");
    D2D1_SIZE_U size { uint32_t(pos.right - pos.left), height };
    // 宽度不足
    if (!size.width || !size.width) return nullptr;
    Config::Window config;
    // 窗口配置
    config.parent = this;
    config.width = pos.right - pos.left;
    config.height = height;
    config.popup = true;
    config.system = true;
    // 创建窗口
    auto window = LongUI::CreateBuiltinWindow(config);
    assert(window && "create system window failed");
    if (!window) return nullptr;
    // 创建视口
    auto viewport = UIViewport::CreateWithoutXml(window, size);
    assert(viewport && "create viewport failed");
    if (!viewport) {
        window->Dispose();
        return nullptr;
    }
    // 设置清理颜色
    window->clear_color = D2D1::ColorF(D2D1::ColorF::White, 0.5f);
    // 连接视口
    window->InitializeViewport(viewport);
    // 添加子节点
    if (child) {
        child->LinkNewParent(viewport);
        viewport->Push(child);
#ifdef _DEBUG
        force_cast(viewport->name) = window->CopyString("PopupWindow");
#endif
    }
    // 重建资源
    auto hr = window->Recreate();
    // 检查错误
    if (FAILED(hr)) {
        UIManager.ShowError(hr);
    }
    // 创建完毕
    viewport->DoLongUIEvent(Event::Event_TreeBuildingFinished);
    // 移动窗口
    window->MoveWindow(pos.left, pos.bottom);
    // 返回创建窗口
    return window;
}

/// <summary>
/// Finds the control.
/// </summary>
/// <param name="cname">The cname.</param>
/// <returns></returns>
auto LongUI::XUIBaseWindow::FindControl(const char* cname) noexcept -> UIControl * {
    assert(cname && (*cname) && "bad argument");
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

/// <summary>
/// Adds the named control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::AddNamedControl(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argumrnt");
    const auto cname = ctrl->name.c_str();
    // 有效
    if (cname[0]) {
        // 插入
        if (!m_hashStr2Ctrl.Insert(cname, ctrl)) {
            ShowErrorWithStr(L"Failed to add control");
        }
    }
}

/// <summary>
/// Sets the hover track control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::SetHoverTrack(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argument");
    if (ctrl->GetHoverTrackTime()) {
        LongUI::SafeAcquire(ctrl);
        LongUI::SafeRelease(m_pHoverTracked);
        m_pHoverTracked = ctrl;
    }
}

/// <summary>
/// Sets the focus.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::SetFocus(UIControl* ctrl) noexcept {
    // 有效
    if (m_pFocusedControl) {
        // 事件
        m_pFocusedControl->DoLongUIEvent(Event::Event_KillFocus, m_pViewport);
        // 去除引用
        m_pFocusedControl->Release();
        // 归零
        m_pFocusedControl = nullptr;
    }
    // 可聚焦的
    if (ctrl && ctrl->flags & Flag_Focusable) {
        // 有效
        m_pFocusedControl = ctrl;
        // 增加引用
        m_pFocusedControl->AddRef();
        // 事件
        m_pFocusedControl->DoLongUIEvent(Event::Event_SetFocus, m_pViewport);
    }
}

/// <summary>
/// Sets the capture control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::SetCapture(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argument");
    ::SetCapture(m_hwnd);
    // 设置引用计数
    LongUI::SafeAcquire(ctrl);
    LongUI::SafeRelease(m_pCapturedControl);
    m_pCapturedControl = ctrl;
};

/// <summary>
/// Releases the capture control.
/// </summary>
/// <returns></returns>
void LongUI::XUIBaseWindow::ReleaseCapture() noexcept {
    LongUI::SafeRelease(m_pCapturedControl);
    //UIManager.DataUnlock();
    ::ReleaseCapture();
    //UIManager.DataLock();
};

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::XUIBaseWindow::Recreate() noexcept ->HRESULT {
    assert(m_pViewport && "no window");
    HRESULT hr = S_OK;
    // 实现
    if (SUCCEEDED(hr)) {
        hr = m_pViewport->Recreate();
    }
    // 遍历
    for (auto inset : m_vInsets) {
        if (SUCCEEDED(hr)) {
            hr = inset->Recreate();
        }
    }
    // 强行刷新一帧
    this->InvalidateWindow();
    return hr;
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::XUIBaseWindow::Update() noexcept {
    assert(m_pViewport && "no viewport");
    // 清理插入符渲染
    //this->clear_do_caret();
    // 实现
    {
        m_pViewport->Update();
        m_pViewport->AfterUpdate();
    }
    // 遍历
    for (auto inset : m_vInsets) {
        inset->Update();
    }
    // 复制渲染数据以保证数据安全
    m_uUnitLengthRender = m_uUnitLength;
    std::memcpy(m_apUnitRender, m_apUnit, sizeof(m_apUnit[0]) * m_uUnitLengthRender);
    m_baBoolWindow.SetTo(Index_FullRenderThisFrameRender, this->is_full_render_this_frame());
    // 清理老数据
    this->clear_full_render_this_frame(); 
    m_uUnitLength = 0; 
#ifdef _DEBUG
    std::memset(m_apUnit, 0, sizeof(m_apUnit));
#endif
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::XUIBaseWindow::Render() const noexcept {
    assert(m_pViewport && "no window");
    // 遍历
    for (const auto* inset : m_vInsets) {
        inset->Render();
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="XUISystemWindow"/> class.
/// </summary>
/// <param name="node">The node.</param>
/// <param name="parent">The parent.</param>
LongUI::XUISystemWindow::XUISystemWindow(const Config::Window& config) noexcept : Super(config) {
    UIManager.AddWindow(this);
}


/// <summary>
/// Finalizes an instance of the <see cref="XUISystemWindow"/> class.
/// </summary>
/// <returns></returns>
LongUI::XUISystemWindow::~XUISystemWindow() noexcept {
    UIManager.RemoveWindow(this);
}

// 移动窗口
void LongUI::XUISystemWindow::MoveWindow(int32_t x, int32_t y) noexcept {
    m_rcWindow.left = x;
    m_rcWindow.top = y;
    POINT pt = { x, y };
    ::MapWindowPoints(::GetParent(m_hwnd), nullptr, &pt, 1);
    ::SetWindowPos(m_hwnd, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE);
}


/// <summary>
/// Initializes the viewport.
/// </summary>
/// <param name="viewport">The viewport.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::InitializeViewport(UIViewport* viewport) noexcept {
    assert(viewport && "bad viewport given");
    assert(m_pViewport == nullptr && "InitializeViewport cannot called only once for one instance");
    m_pViewport = viewport;
    // 自动适应高DPI
    if (this->is_hidpi_supported()) {
        float x = float(UIManager.GetMainDpiX()) / float(LongUI::BASIC_DPI);
        float y = float(UIManager.GetMainDpiY()) / float(LongUI::BASIC_DPI);
        m_pViewport->SetZoom(x, y);
    }
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
bool LongUI::XUIBaseWindow::DoEvent(const EventArgument& arg) noexcept {
    assert(m_pViewport && "bad action");
    return m_pViewport->DoEvent(arg);
}

/// <summary>
/// Invalidates the specified control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::Invalidate(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argument");
    // 已经全渲染?
    if (this->is_full_render_this_frame()) return;
    // 检查
    ctrl = ctrl->prerender;
    assert(ctrl->prerender == ctrl && "bad argument");
    // 就是窗口 或者已满?
    if (ctrl == m_pViewport || m_uUnitLength >= LongUIDirtyControlSize) {
        assert(m_uUnitLength <= LongUIDirtyControlSize && "check it");
        this->set_full_render_this_frame();
        return;
    }
#ifdef _DEBUG
    // 调试信息
    size_t  debug_backup_leng = m_uUnitLength;
    UIControl*  debug_backup_unit[LongUIDirtyControlSize];
    std::memcpy(debug_backup_unit, m_apUnit, sizeof(debug_backup_unit));
#endif
    // 一次检查
    bool changed = false;
    const auto oldenditr = m_apUnit + m_uUnitLength;
    for (auto itr = m_apUnit; itr < oldenditr; ++itr) {
        // 已存在的空间
        auto existd = *itr;
        // 一样? --> 不干
        if (existd == ctrl) return void(assert(changed == false));
        // 存在深度 < 插入深度 -> 检查插入的是否为存在的子孙结点
        if (existd->level < ctrl->level) {
            // 是 -> 什么不干
            if (existd->IsPosterityForSelf(ctrl)) return void(assert(changed == false));
            // 否 -> 继续
            else {

            }
        }
        // 存在深度 > 插入深度 -> 检查存在的是否为插入的子孙结点
        else if (existd->level > ctrl->level) {
            // 是 -> 替换所有
            if (ctrl->IsPosterityForSelf(existd)) {
                *itr = nullptr;
                changed = true;
            }
            // 否 -> 继续
            else {

            }
        }
        // 深度一致 -> 继续
        else {

        }
    }
#ifdef _DEBUG
    if (m_pViewport->debug_this) {
        UIManager << DL_Log << L"[INSERT]: " << ctrl << LongUI::endl;
    }
#endif
    // 二次插入
    if (changed) {
        // 重置
        m_uUnitLength = 0;
        // 只写迭代器
        auto witr = m_apUnit;
        // 只读迭代器
        auto ritr = m_apUnit;
        for (; ritr < oldenditr; ++ritr) {
            if (*ritr) {
                *witr = *ritr;
                ++witr;
                ++m_uUnitLength;
            }
        }
    }
#ifdef _DEBUG
    // 断言调试
    {
        auto endt = m_apUnit + m_uUnitLength;
        assert(std::find(m_apUnit, endt, ctrl) == endt);
        std::for_each(m_apUnit, endt, [ctrl](UIControl* tmpc) noexcept {
            assert(tmpc->IsPosterityForSelf(ctrl) == false && "bad ship");
            assert(ctrl->IsPosterityForSelf(tmpc) == false && "bad ship");
        });
    }
#endif
    // 添加到最后
    m_apUnit[m_uUnitLength++] = ctrl;
}


/// <summary>
/// Resizeds this window.
/// </summary>
/// <returns></returns>
void LongUI::XUIBaseWindow::resized() noexcept {
    assert(m_pViewport && "bad action");
    // 修改
    m_pViewport->visible_rect.right = static_cast<float>(this->GetWidth());
    m_pViewport->visible_rect.bottom = static_cast<float>(this->GetHeight());
    m_pViewport->SetWidth(m_pViewport->visible_rect.right);
    m_pViewport->SetHeight(m_pViewport->visible_rect.bottom);
    // 强行刷新一帧
    this->InvalidateWindow();
    // 处理了
    this->clear_new_size();
}


// longui namesapce
namespace LongUI {
    // CUIBuiltinSystemWindow
    // system window -- builtin
    class CUIBuiltinSystemWindow final : public XUISystemWindow,
        public CUISingleNormalObject {
        // super class
        using Super = XUISystemWindow;
    private:
        // release data for this
        void release_data() noexcept;
        // is direct composition
        bool is_direct_composition() const noexcept { return true; }
        // get dxgi swapeffect
        bool get_swap_effect() const noexcept { return false; }
    public:
        // ctor
        CUIBuiltinSystemWindow(const Config::Window& config) noexcept;
        // dtor
        ~CUIBuiltinSystemWindow() noexcept;
    public:
        // dispose this
        virtual void Dispose() noexcept { delete this; };
        // render 
        virtual void Render() const noexcept override;
        // update 
        virtual void Update() noexcept override;
        // close window
        virtual void Close() noexcept { ::PostMessageW(m_hwnd, WM_CLOSE, 0, 0); };
        // recreate
        virtual auto Recreate() noexcept->HRESULT override;
        // resize
        void Resize(uint32_t w, uint32_t h) noexcept override;
        // set cursor
        virtual void SetCursor(Cursor cursor) noexcept override;
        // show/hide window
        virtual void ShowWindow(int nCmdShow) noexcept override { ::ShowWindow(m_hwnd, nCmdShow); }
        // set caret
        virtual void SetCaret(UIControl* ctrl, const RectLTWH_F* rect) noexcept override;
    public:
        // normal event
        bool MessageHandle(UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result) noexcept;
        // on WM_CREATE
        void OnCreate(HWND hwnd) noexcept;
        // begin render
        void BeginRender() const noexcept;
        // end render
        void EndRender() const noexcept;
        // on resized
        void OnResized() noexcept;
    public:
        // window proc
        static auto WINAPI WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept->LRESULT;
        // Register Window's Class
        static void RegisterWindowClass() noexcept;
    private:
        // swap chain
        IDXGISwapChain2*        m_pSwapChain    = nullptr;
        // target bitmap
        ID2D1Bitmap1*           m_pTargetBitmap = nullptr;
        // Direct Composition Device
        IDCompositionDevice*    m_pDcompDevice  = nullptr;
        // Direct Composition Target
        IDCompositionTarget*    m_pDcompTarget  = nullptr;
        // Direct Composition Visual
        IDCompositionVisual*    m_pDcompVisual  = nullptr;
        // wait
        HANDLE                  m_hVSync        = nullptr;
        // now cursor
        HCURSOR                 m_hNowCursor    = ::LoadCursor(nullptr, IDC_ARROW);
        // new size
        D2D1_SIZE_U             m_szNew         = D2D1_SIZE_U{0};
        // caret
        D2D1_RECT_F             m_rcCaret       = D2D1_RECT_F{0.f};
        // track mouse event: end with DWORD
        TRACKMOUSEEVENT         m_csTME;
#ifdef _DEBUG
        // title name
        CUIString               m_strTitle;
#endif
        // msg for taskbar-btn created
        static const UINT s_uTaskbarBtnCreatedMsg;
        // msg for char16_t
        static char16_t s_cUtf16Backup;
    };
    // 任务按钮创建消息
    const UINT CUIBuiltinSystemWindow::s_uTaskbarBtnCreatedMsg = ::RegisterWindowMessageW(L"TaskbarButtonCreated");
    // 任务按钮创建消息
    char16_t CUIBuiltinSystemWindow::s_cUtf16Backup = 0;
}

/// <summary>
/// Creates the builtin window.
/// </summary>
/// <param name="config">The configuration.</param>
/// <returns></returns>
auto LongUI::CreateBuiltinWindow(const Config::Window& config) noexcept -> XUIBaseWindow* {
    // 创建系统窗口?
    if (config.system) {
        LongUI::CUIBuiltinSystemWindow::RegisterWindowClass();
        return new(std::nothrow) CUIBuiltinSystemWindow(config);
    }
    // 创建内建窗口
    else {
        assert(config.parent && "prent cannot be null for inset window");
        assert(!"NOIMPL");
    }
    return nullptr;
}


/// <summary>
/// Registers the window class.
/// </summary>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::RegisterWindowClass() noexcept {
    auto ins = ::GetModuleHandleW(nullptr);
    WNDCLASSEXW wcex;
    auto code = ::GetClassInfoExW(ins, LongUI::WindowClassNameN, &wcex);
    if (!code) {
        // 注册一般窗口类
        wcex = { 0 };
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = 0;
        wcex.lpfnWndProc = CUIBuiltinSystemWindow::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(void*);
        wcex.hInstance = ins;
        wcex.hCursor = nullptr;
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = LongUI::WindowClassNameN;
        wcex.hIcon = nullptr;// ::LoadIconW(ins, MAKEINTRESOURCEW(101));
        ::RegisterClassExW(&wcex);
        // 注册弹出窗口类
        wcex.style = CS_DROPSHADOW;
        wcex.lpszClassName = LongUI::WindowClassNameP;
        ::RegisterClassExW(&wcex);
    }
}


/// <summary>
/// WNDs the proc.
/// </summary>
/// <param name="hwnd">The HWND.</param>
/// <param name="message">The message.</param>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <returns></returns>
auto LongUI::CUIBuiltinSystemWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept ->LRESULT {
#ifdef _DEBUG
    g_dbg_last_proc_message = message;
    LongUI::Msg msg = { message };
#endif
    // 返回值
    LRESULT recode = 0;
    // 创建窗口时设置指针
    if (message == WM_CREATE) {
        // 获取指针
        auto* window = reinterpret_cast<LongUI::CUIBuiltinSystemWindow*>(
            (reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams
            );
        // 设置窗口指针
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(window));
        // TODO: 创建完毕
        window->OnCreate(hwnd);
        // 返回1
        recode = 1;
    }
    else {
        // 获取储存的指针
        auto* window = reinterpret_cast<LongUI::CUIBuiltinSystemWindow *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA))
            );
#ifdef _DEBUG
        g_dbg_last_proc_window_pointer = reinterpret_cast<std::uintptr_t>(window);
#endif
        // 无效
        if (!window) return ::DefWindowProcW(hwnd, message, wParam, lParam);
        auto handled = false;
        {
            // 消息处理
            handled = window->MessageHandle(message, wParam, lParam, recode);
        }
        // 未处理
        if (!handled) {
            recode = ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    return recode;
}


    /*{
        // 加锁
        CUIDataAutoLocker locker;
        // 创建插入符
        ::CreateCaret(m_hwnd, nullptr, 1, 1);
        // 存在焦点控件
        if (m_pFocusedControl) {
            // 事件
            m_pFocusedControl->DoLongUIEvent(Event::Event_SetFocus, m_pViewport);
        }
        return true;
    }*/

#ifndef WM_NCUAHDRAWCAPTION 
#define WM_NCUAHDRAWCAPTION 0xAE
#endif

#ifndef WM_NCUAHDRAWFRAME  
#define WM_NCUAHDRAWFRAME   0xAF
#endif

/// <summary>
/// Normals the event.
/// </summary>
/// <param name="message">The message.</param>
/// <param name="wParam">The w parameter.</param>
/// <param name="lParam">The l parameter.</param>
/// <param name="result">The result.</param>
/// <returns></returns>
bool LongUI::CUIBuiltinSystemWindow::MessageHandle(
    UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result) noexcept {
    // --------------------------  获取X坐标
    auto get_x = [lParam]() noexcept {return float(int16_t(LOWORD(lParam)));};
    // --------------------------  获取Y坐标
    auto get_y = [lParam]() noexcept {return float(int16_t(HIWORD(lParam)));};
    // --------------------------  失去焦点
    auto on_killfocus = [this]() noexcept {
        bool close_window = false;
        {
            // 加锁
            CUIDataAutoLocker locker;
            // 存在焦点控件
            if (m_pFocusedControl) {
                // 事件
                m_pFocusedControl->DoLongUIEvent(
                    Event::Event_KillFocus, m_pViewport
                );
                // 释放引用
                LongUI::SafeRelease(m_pFocusedControl);
            }
            // 重置
            m_rcCaret.left = -1.f;
            m_rcCaret.right = 0.f;
            // 检查属性
            close_window = this->is_close_on_focus_killed();
        }
        // 失去焦点即关闭窗口
        if (close_window) {
            ::PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
        }
        // 关闭插入符号
        ::DestroyCaret();
    };
    // --------------------------  字符输入
    auto on_char = [this, wParam]() noexcept {
        CUIDataAutoLocker locker;
        if (m_pFocusedControl) {
            auto ch = static_cast<char16_t>(wParam);
            EventArgument arg;
            if (LongUI::IsHighSurrogate(ch)) {
                s_cUtf16Backup = ch;
                return;
            }
            else if (LongUI::IsLowSurrogate(ch)) {
                arg.key.ch = impl::char16x2_to_char32(s_cUtf16Backup, ch);
                s_cUtf16Backup = 0;
            }
            else {
                arg.key.ch = static_cast<char32_t>(wParam);
            }
            arg.sender = m_pViewport;
            arg.event = Event::Event_Char;
            m_pFocusedControl->DoEvent(arg);
        }
    };
    // --------------------------  窗口关闭
    auto on_close_msg = [this]() noexcept ->bool {
        // 加锁
        CUIDataAutoLocker locker;
        // 允许退出
        if (m_pViewport->CanbeClosedNow()) {
            this->on_close();
            return false;
        }
        // 不允许退出
        else {
            return true;
        }
    };
    // --------------------------  消息处理
    // 消息类型
    //enum MsgType { Type_Mouse, Type_Other } msgtp; msgtp = Type_Other;
    // 消息处理
    MouseEventArgument ma;
    // 检查信息
    switch (message)
    {
    case WM_SETCURSOR:
    {
        CUIDataAutoLocker locker;
        ::SetCursor(m_hNowCursor);
        return false;
    }
    case WM_MOUSEMOVE:
    {
        ma.event = MouseEvent::Event_MouseMove;
        // 加锁
        CUIDataAutoLocker locker;
        // 更新坐标
        this->last_point = { get_x(), get_y() };
        break;
    }
    case WM_LBUTTONDOWN:
        ma.event = MouseEvent::Event_LButtonDown;
        break;
    case WM_LBUTTONUP:
        ma.event = MouseEvent::Event_LButtonUp;
        break;
    case WM_RBUTTONDOWN:
        ma.event = MouseEvent::Event_RButtonDown;
        break;
    case WM_RBUTTONUP:
        ma.event = MouseEvent::Event_RButtonUp;
        break;
    case WM_MBUTTONDOWN:
        ma.event = MouseEvent::Event_MButtonDown;
        break;
    case WM_MBUTTONUP:
        ma.event = MouseEvent::Event_MButtonUp;
        break;
    case WM_MOUSEWHEEL:
        ma.event = MouseEvent::Event_MouseWheelV;
        ma.wheel.delta = (float(GET_WHEEL_DELTA_WPARAM(wParam))) 
            / float(WHEEL_DELTA);
        break;
    case WM_MOUSEHWHEEL:
        ma.event = MouseEvent::Event_MouseWheelH;
        ma.wheel.delta = (float(GET_WHEEL_DELTA_WPARAM(wParam))) 
            / float(WHEEL_DELTA);
        break;
    case WM_MOUSEHOVER:
        ma.event = MouseEvent::Event_MouseHover;
        break;
    case WM_MOUSELEAVE:
        ma.event = MouseEvent::Event_MouseLeave;
        break;
    case WM_SETFOCUS:
        ::CreateCaret(m_hwnd, nullptr, 1, 1);
        return true;
    case WM_KILLFOCUS:
        on_killfocus();
        return true;
    case WM_KEYDOWN:
        // 键入字符
    {
        CUIDataAutoLocker locker;
        if (m_pFocusedControl) {
            EventArgument arg;
            arg.sender = m_pViewport;
            arg.key.ch = static_cast<char32_t>(wParam);
            arg.event = Event::Event_KeyDown;
            m_pFocusedControl->DoEvent(arg);
        }
        return true;
    }
    case WM_CHAR:
        // 键入字符
        on_char();
        return true;
    case WM_NCUAHDRAWCAPTION:
        return true;
    case WM_NCUAHDRAWFRAME:
        return true;
    case WM_GETICON :
        return true;
    case WM_MOVE:
        // 移动窗口
        m_rcWindow.left = LONG(int16_t(LOWORD(lParam)));
        m_rcWindow.top = LONG(int16_t(HIWORD(lParam)));
        return true;
    case WM_SIZE:
        // 改变大小
    {
        // 加锁
        auto w = LOWORD(lParam);
        auto h = HIWORD(lParam);
        // 数据有效?
        if (w && h) {
            CUIDataAutoLocker locker;
            this->Resize(w, h);
        }
        return true;
    }
    case WM_GETMINMAXINFO:
        // 获取限制大小
        //CUIDataAutoLocker locker;
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize = { m_miniSize.x, m_miniSize.y };
        return true;
    case WM_CLOSE:
        // 关闭窗口
        return on_close_msg();
    case WM_DPICHANGED:
        // dpi改变了
        if (this->is_hidpi_supported()) {
            float xdpi = float(uint16_t(LOWORD(wParam)));
            float ydpi = float(uint16_t(HIWORD(wParam)));
            float x = xdpi / float(LongUI::BASIC_DPI);
            float y = ydpi / float(LongUI::BASIC_DPI);
            CUIDataAutoLocker locker;
            m_pViewport->SetZoom(x, y);
        }
        return true;
    default:
        return false;
    }
    // 鼠标消息
    {
        // 加锁
        CUIDataAutoLocker locker;
        // 设置鼠标位置
        ma.ptx = this->last_point.x;
        ma.pty = this->last_point.y;
        // hover跟踪
        if (ma.event == MouseEvent::Event_MouseHover && m_pHoverTracked) {
            return m_pHoverTracked->DoMouseEvent(ma);
        }
        // 存在捕获控件
        if (m_pCapturedControl) {
            return m_pCapturedControl->DoMouseEvent(ma);
        }
        // 窗口实现
        auto code = m_pViewport->DoMouseEvent(ma);
        // 设置跟踪
        if (message == WM_MOUSEMOVE) {
            m_csTME.dwHoverTime = m_pHoverTracked ? DWORD(m_pHoverTracked->GetHoverTrackTime()) : DWORD(0);
            ::TrackMouseEvent(&m_csTME);
        }
        return code;
    }
    //return false;
}


// 改变大小
void LongUI::CUIBuiltinSystemWindow::Resize(uint32_t w, uint32_t h) noexcept {
    assert(w && h && "bad argument");
    if (w != this->GetWidth() || h != this->GetHeight()) {
        m_szNew.width = w;
        m_szNew.height = h;
        this->set_new_size();
    }
}


/// <summary>
/// Called when [resizd].
/// </summary>
/// <param name="w">The w.</param>
/// <param name="h">The h.</param>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::OnResized() noexcept {
    CUIDxgiAutoLocker locker;
    // 修改大小, 需要取消目标
    UIManager_RenderTarget->SetTarget(nullptr);
    // 设置
    auto rect_right = LongUI::MakeAsUnit(m_szNew.width);
    auto rect_bottom = LongUI::MakeAsUnit(m_szNew.height);
    if (!this->is_direct_composition()) {
        rect_right = m_szNew.width;
        rect_bottom = m_szNew.height;
    }
    m_rcWindow.width = m_szNew.width;
    m_rcWindow.height = m_szNew.height;
    auto old_size = m_pTargetBitmap->GetPixelSize();
    HRESULT hr = S_OK;
    bool force = false;
    // 不等于就Resize
    if (!this->is_direct_composition()) {
        if (old_size.width != uint32_t(rect_right) ||
            old_size.height != uint32_t(rect_bottom)) {
            force = true;
        }
    }
    // 小于才Resize
    if (force || old_size.width < uint32_t(rect_right) 
        || old_size.height < uint32_t(rect_bottom)) {
        UIManager << DL_Hint << L"Window: ["
            << m_pViewport->name
            << L"]\tTarget Bitmap Resize to "
            << LongUI::Formated(L"(%d, %d)", int(rect_right), int(rect_bottom))
            << LongUI::endl;
        IDXGISurface* pDxgiBackBuffer = nullptr;
        LongUI::SafeRelease(m_pTargetBitmap);
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
                &m_pTargetBitmap
            );
            longui_debug_hr(hr, L"UIManager_RenderTarget->CreateBitmapFromDxgiSurface faild");
        }
        // 重建失败?
        if (FAILED(hr)) {
            UIManager << DL_Error << L" Recreate FAILED!" << LongUI::endl;
            UIManager.ShowError(hr);
        }
        LongUI::SafeRelease(pDxgiBackBuffer);
    }
    // 父类调用
    this->resized();
}


/// <summary>
/// Called when [create].
/// </summary>
/// <param name="hwnd">The HWND.</param>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::OnCreate(HWND hwnd) noexcept {
    CHANGEFILTERSTRUCT cfs = { sizeof(CHANGEFILTERSTRUCT) };
    ::ChangeWindowMessageFilterEx(hwnd, s_uTaskbarBtnCreatedMsg, MSGFLT_ALLOW, &cfs);
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIBuiltinSystemWindow::Recreate() noexcept ->HRESULT {
    //m_pViewport->SetZoom(1);
    // 跳过
    if (this->is_skip_render()) return S_OK;
    // 渲染锁
    CUIDxgiAutoLocker locker;
    // 释放数据
    this->release_data();
    // DXGI Surface 后台缓冲
    IDXGISurface*               pDxgiBackBuffer = nullptr;
    IDXGISwapChain1*            pSwapChain = nullptr;
    // 创建交换链
    HRESULT hr = S_OK;
    // 创建交换链
    if (SUCCEEDED(hr)) {
        RECT rect = { 0 }; ::GetClientRect(m_hwnd, &rect);
        // 交换链信息
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        if (this->is_direct_composition()) {
            swapChainDesc.Width = LongUI::MakeAsUnit(rect.right - rect.left);
            swapChainDesc.Height = LongUI::MakeAsUnit(rect.bottom - rect.top);
        }
        else {
            swapChainDesc.Width = (rect.right - rect.left);
            swapChainDesc.Height = (rect.bottom - rect.top);
        }
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        // XXX: Fixit
        if (this->is_direct_composition()) {
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
    // 确保DXGI队列里边不会超过一帧
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->SetMaximumFrameLatency(1);
        longui_debug_hr(hr, L"m_pSwapChain->SetMaximumFrameLatency faild");
        //hr = S_OK;
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
            &m_pTargetBitmap
        );
        longui_debug_hr(hr, L"UIManager_RenderTarget->CreateBitmapFromDxgiSurface faild");
    }
    // 使用DComp
    if (this->is_direct_composition()) {
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
    if (FAILED(hr)) {
        UIManager << L"Recreate Failed!" << LongUI::endl;
        UIManager.ShowError(hr);
    }
    LongUI::SafeRelease(pDxgiBackBuffer);
    LongUI::SafeRelease(pSwapChain);
    // 重建 子控件UI
    return Super::Recreate();
}

/// <summary>
/// Release_datas this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::release_data() noexcept {
    // 释放资源
    LongUI::SafeRelease(m_pTargetBitmap);
    LongUI::SafeRelease(m_pSwapChain);
    LongUI::SafeRelease(m_pDcompDevice);
    LongUI::SafeRelease(m_pDcompTarget);
    LongUI::SafeRelease(m_pDcompVisual);
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::Update() noexcept {
    // 重置大小?
    if (this->is_new_size()) this->OnResized();
    // 父类
    Super::Update();
    // 插入符号
    if (m_tmCaret.Update()) {
        // 反转插入符号
        this->change_caret_in();
#if 0
        // 不显示插入符号时候刷新显示焦点控件
        if (m_pFocusedControl && !this->is_caret_in()) {
            m_pFocusedControl->InvalidateThis();
        }
#else
        // 刷新显示焦点控件
        if (m_rcCaret.right > 0.f && m_pFocusedControl) {
            m_pFocusedControl->InvalidateThis();
        }
#endif
    }
}

/// <summary>
/// Begins the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::BeginRender() const noexcept {
    // 设置文本渲染策略
    UIManager_RenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE(m_textAntiMode));
    // 设为当前渲染对象
    UIManager_RenderTarget->SetTarget(m_pTargetBitmap);
    // 开始渲染
    UIManager_RenderTarget->BeginDraw();
    // 设置转换矩阵
#if 0
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
#else
    UIManager_RenderTarget->SetTransform(&m_pViewport->world);
#endif
    // 清空背景
    UIManager_RenderTarget->Clear(this->clear_color);
}

/// <summary>
/// Ends the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::EndRender() const noexcept {
    // 渲染插入符号
    if (this->is_caret_in() && m_rcCaret.right != 0.f) {
        UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
        UIManager_RenderTarget->PushAxisAlignedClip(&m_rcCaret, D2D1_ANTIALIAS_MODE_ALIASED);
        UIManager_RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        UIManager_RenderTarget->PopAxisAlignedClip();
    }
    // 结束渲染
    UIManager_RenderTarget->EndDraw();
    HRESULT hr = S_OK;
    {
        // 全渲染
        if (this->is_full_render_this_frame_render()) {
            hr = m_pSwapChain->Present(0, 0);
            //if (hr == DXGI_ERROR_WAS_STILL_DRAWING) hr = S_FALSE;
            // 呈现
            longui_debug_hr(hr, L"m_pSwapChain->Present1 full rendering faild");
        }
        // 脏渲染
        else {
            // 呈现参数设置
            RECT scroll = { 0, 0, this->GetWidth(), this->GetHeight() };
            RECT rects[LongUIDirtyControlSize];
            DXGI_PRESENT_PARAMETERS present_parameters;
            present_parameters.DirtyRectsCount = m_uUnitLengthRender;
            present_parameters.pDirtyRects = rects;
            present_parameters.pScrollRect = &scroll;
            present_parameters.pScrollOffset = nullptr;
            // 设置参数
            auto control = m_apUnitRender;
            for (auto itr = rects; itr < rects + m_uUnitLengthRender; ++itr) {
                const auto& vrt = (*control)->visible_rect;
                itr->left = static_cast<LONG>(vrt.left);
                itr->top = static_cast<LONG>(vrt.top);
                itr->right = static_cast<LONG>(std::ceil(vrt.right));
                itr->bottom = static_cast<LONG>(std::ceil(vrt.bottom));
                ++control;
            }
            // 提交
            hr = m_pSwapChain->Present1(0, 0, &present_parameters);
            //if (hr == DXGI_ERROR_WAS_STILL_DRAWING) hr = S_FALSE;
            // 呈现
            longui_debug_hr(hr, L"m_pSwapChain->Present1 dirty rendering faild");
        }
    }
    // 收到重建消息/设备丢失时 重建UI
#ifdef _DEBUG
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
    assert(SUCCEEDED(hr));
    // 调试
    if (this->is_full_render_this_frame_render()) {
        ++force_cast(full_render_counter);
    }
    else {
        ++force_cast(dirty_render_counter);
    }
    // 更新调试信息
    wchar_t buffer[1024];
    std::swprintf(
        buffer, lengthof(buffer),
        L"%ls: FRC: %d\nDRC: %d\nDRRC: %d",
        m_strTitle.c_str(),
        int(full_render_counter),
        int(dirty_render_counter),
        int(m_uUnitLengthRender)
    );
    // 设置显示
    UIManager.DxgiUnlock();
    ::DefWindowProc(m_hwnd, WM_SETTEXT, WPARAM(0), LPARAM(buffer));
    UIManager.DxgiLock();
#else
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        hr = UIManager.RecreateResources();
    }
#endif
    // 检查
    if (FAILED(hr)) {
        UIManager.ShowError(hr);
    }
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::Render() const noexcept {
    // 跳过渲染?
    if (this->is_skip_render()) return;
    // 无需渲染?
    if (!this->is_full_render_this_frame_render() && !m_uUnitLengthRender) return;
    // 等待
    ::WaitForSingleObjectEx(m_hVSync, 100, true);
    // 开始渲染
    this->BeginRender();
    // 全渲染
    if (this->is_full_render_this_frame_render()) {
        // 实现
        m_pViewport->Render();
    }
    // 脏渲染
    else {
        auto init_transfrom = DX::Matrix3x2F::Identity();
        // 遍历
        for (auto itr = m_apUnitRender; itr < m_apUnitRender + m_uUnitLengthRender; ++itr) {
            auto ctrl = *itr; assert(ctrl != m_pViewport && "check the code");
            UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
            UIManager_RenderTarget->PushAxisAlignedClip(&ctrl->visible_rect, D2D1_ANTIALIAS_MODE_ALIASED);
            UIManager_RenderTarget->SetTransform(&ctrl->world);
            // 渲染背景笔刷?
            /*if (ctrl->backgroud != ctrl && ctrl->backgroud) {
                auto bk = ctrl->backgroud;
                UIManager_RenderTarget->SetTransform(&bk->world);
                bk->RenderBackgroudBrush();
                UIManager_RenderTarget->SetTransform(&ctrl->world);
            }*/
            // 正常渲染
            ctrl->Render();
            // 回来
            UIManager_RenderTarget->PopAxisAlignedClip();
        }
    }
    // 渲染
    Super::Render();
    // 结束渲染
    this->EndRender();
}

/// <summary>
/// Sets the cursor.
/// </summary>
/// <param name="cursor">The cursor.</param>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::SetCursor(LongUI::Cursor cursor) noexcept {
    auto id = IDC_ARROW;
    switch (cursor)
    {
    case LongUI::Cursor::Cursor_Default:
        id = IDC_ARROW;
        break;
    case LongUI::Cursor::Cursor_Arrow:
        id = IDC_ARROW;
        break;
    case LongUI::Cursor::Cursor_Ibeam:
        id = IDC_IBEAM;
        break;
    case LongUI::Cursor::Cursor_Wait:
        id = IDC_WAIT;
        break;
    case LongUI::Cursor::Cursor_Hand:
        id = IDC_HAND;
        break;
    case LongUI::Cursor::Cursor_Help:
        id = IDC_HELP;
        break;
    case LongUI::Cursor::Cursor_Cross:
        id = IDC_CROSS;
        break;
    case LongUI::Cursor::Cursor_SizeAll:
        id = IDC_SIZEALL;
        break;
    case LongUI::Cursor::Cursor_UpArrow:
        id = IDC_UPARROW;
        break;
    case LongUI::Cursor::Cursor_SizeNWSE:
        id = IDC_SIZENWSE;
        break;
    case LongUI::Cursor::Cursor_SizeNESW:
        id = IDC_SIZENESW;
        break;
    case LongUI::Cursor::Cursor_SizeWE:
        id = IDC_SIZEWE;
        break;
    case LongUI::Cursor::Cursor_SizeNS:
        id = IDC_SIZENS;
        break;
    default:
        break;
    }
    m_hNowCursor = ::LoadCursor(nullptr, id);
}

/// <summary>
/// Sets the caret.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::SetCaret(UIControl* ctrl, const RectLTWH_F* rect) noexcept {
    // 必须有效
    assert(ctrl && "bad argument");
    // 显示插入符号
    if (rect) {
        // 记录老坐标
        auto oldx = m_rcCaret.left;
        auto oldy = m_rcCaret.top;
        // 计算一般位置
        D2D1_POINT_2F pt{ rect->left, rect->top };
        pt = LongUI::TransformPoint(ctrl->world, pt);
        // 位置差不多
        auto xyabs = std::abs(oldx - pt.x) + std::abs(oldy - pt.y);
        if (xyabs < 1.f) return;
        // 继续计算
        m_rcCaret.left = pt.x;
        m_rcCaret.top = pt.y;
        m_rcCaret.right = pt.x + rect->width * ctrl->world._11;
        m_rcCaret.bottom = pt.y + rect->height * ctrl->world._22;
        // 计算可视化区域
        m_rcCaret.left  =  std::max(ctrl->visible_rect.left, m_rcCaret.left);
        m_rcCaret.top   =  std::max(ctrl->visible_rect.top, m_rcCaret.top);
        m_rcCaret.right =  std::min(ctrl->visible_rect.right, m_rcCaret.right);
        m_rcCaret.bottom=  std::min(ctrl->visible_rect.bottom, m_rcCaret.bottom);
        // 刷新显示
        this->set_caret_in();
        // 重置定时器
        m_tmCaret.Reset();
    }
    // 隐藏插入符号
    else {
        // 清理插入符
        m_rcCaret.left = -1.f;
        m_rcCaret.right = 0.f;
        // 刷新控件
        ctrl->InvalidateThis();
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="CUIBuiltinSystemWindow"/> class.
/// </summary>
/// <param name="node">The node.</param>
/// <param name="viewport">The viewport.</param>
LongUI::CUIBuiltinSystemWindow::CUIBuiltinSystemWindow(const Config::Window& config) noexcept : Super(config) {
    // Xml节点
    auto node = config.node;
    const char* str = nullptr;
    // 标题名字
    CUIString titlename(L"LongUI.Window");
    {
        auto s = node.attribute(LongUI::XmlAttribute::WindowTitleName).value();
        titlename.FromUtf8(s);
    }
    // 窗口区
    {
        // 检查样式样式
        DWORD window_style = config.popup ? WS_POPUPWINDOW : WS_OVERLAPPEDWINDOW;
        // 设置窗口大小
        D2D1_RECT_L window_rect;
        window_rect.left = 0;
        window_rect.top = 0;
        // 检查控件大小
        if (node) {
            float size[] = { 0.f, 0.f };
            Helper::MakeFloats(
                node.attribute(LongUI::XmlAttribute::AllSize).value(),
                size, lengthof<uint32_t>(size)
            );
            // 检查
            if (size[0] == 0.f) size[0] = float(LongUIDefaultWindowWidth);
            if (size[1] == 0.f) size[1] = float(LongUIDefaultWindowHeight);
            window_rect.right = static_cast<LONG>(size[0]);
            window_rect.bottom = static_cast<LONG>(size[1]);
        }
        else {
            window_rect.right = config.width;
            window_rect.bottom = config.height;
            { int bk = 9; }
        }
        // 调整大小
        ::AdjustWindowRect(&window_rect, window_style, FALSE);
        // 窗口
        m_rcWindow.width = (window_rect.right - window_rect.left);
        m_rcWindow.height = (window_rect.bottom - window_rect.top);
        m_rcWindow.left = (::GetSystemMetrics(SM_CXFULLSCREEN) - m_rcWindow.width) / 2;
        m_rcWindow.top = (::GetSystemMetrics(SM_CYFULLSCREEN) - m_rcWindow.height) / 2;
        // 创建窗口
        m_hwnd = ::CreateWindowExW(
            //WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            this->is_direct_composition() ? WS_EX_NOREDIRECTIONBITMAP : 0,
            config.popup ? LongUI::WindowClassNameP : LongUI::WindowClassNameN,
            titlename.c_str(),
            window_style,
            m_rcWindow.left, m_rcWindow.top, m_rcWindow.width, m_rcWindow.height,
            m_pParent ? m_pParent->GetHwnd() : nullptr,
            nullptr,
            ::GetModuleHandleW(nullptr),
            this
        );
        // 创建成功
        if (m_hwnd) {
            // 禁止 Alt + Enter 全屏
            UIManager_DXGIFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
        }
        // 创建失败
        else {
            UIManager.ShowError(L"Error! Failed to Create Window", __FUNCTIONW__);
            return;
        }
    }
    // 设置HOVER-LEAVE THRACK
    m_csTME.cbSize = sizeof(m_csTME);
    m_csTME.dwFlags = TME_HOVER | TME_LEAVE;
    m_csTME.hwndTrack = m_hwnd;
    m_csTME.dwHoverTime = 0;
    // 显示
#ifdef _DEBUG
    m_strTitle = titlename;
#endif
}

/// <summary>
/// Finalizes an instance of the <see cref=""/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIBuiltinSystemWindow::~CUIBuiltinSystemWindow() noexcept {
    if (m_hVSync) {
        ::CloseHandle(m_hVSync);
        m_hVSync = nullptr;
    }
    // 设置窗口指针
    ::SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, LONG_PTR(0));
    // 释放资源
    this->release_data();
#if 0
    // 解锁
    UIManager.DataUnlock();
    {
        // 取消注册
        ::RevokeDragDrop(m_hwnd);
        // 释放数据
        //LongUI::SafeRelease(m_pTaskBarList);
        //LongUI::SafeRelease(m_pDropTargetHelper);
        //LongUI::SafeRelease(m_pCurDataObject);
        // 关闭
    }
    // 加锁
    UIManager.DataLock();
#endif
}
