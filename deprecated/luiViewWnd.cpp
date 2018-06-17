#include "../LongUI/luipch.h"
// LongUI
#include "../LongUI/Core/luiDebug.h"
#include "../LongUI/Core/luiManager.h"
#include "../LongUI/Core/luiWindow.h"
#include "../LongUI/Control/UIViewport.h"
#include "../LongUI/Util/luiDll.h"
#include "../LongUI/Util/luiFunc.h"
#include "../LongUI/Util/luiGUID.h"
#include "../LongUI/Util/luiMatrix.h"
#include "../LongUI/Util/luiHelper.h"
#include "../LongUI/Util/lui2d2d_impl.h"
#include "../LongUI/Xml/luiXmlHelper.h"
// C++
#include <algorithm>
#include <cassert>
// Winddows
#include <dcomp.h>
#include <d3d11.h>
#include <d2d1_3.h>
#include <dxgi1_3.h>

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
/// <remarks>
/// 默认行为: 承载窗口没有父窗口就退出
/// </remarks>
/// <returns></returns>
void LongUI::UIViewport::OnClose() noexcept {
    if (!this->window.GetParent()) UIManager.Exit();
}


/// <summary>
/// Initializes a new instance of the <see cref="UIViewport"/> class.
/// </summary>
/// <param name="window">The window.</param>
LongUI::UIViewport::UIViewport(XUIBaseWindow& wnd) noexcept : Super(wnd) {
    assert(&wnd && "bad argument");
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
        // 根据视口位置
        force_cast(this->box.vpos.x) = this->box.margin.left;
        force_cast(this->box.vpos.y) = this->box.margin.top;
        // 浮点视区大小
        if (this->box.vsize.width == 0.f) {
            force_cast(this->box.vsize.width) = static_cast<float>(LongUIDefaultWindowWidth);
        }
        // 更新
        if (this->box.vsize.height == 0.f) {
            force_cast(this->box.vsize.height) = static_cast<float>(LongUIDefaultWindowHeight);
        }
        // 可视区域范围
        force_cast(box.rect).right = this->box.vsize.width;
        force_cast(box.rect).bottom = this->box.vsize.height;
        force_cast(this->layout.content) = this->box.vsize;
    }
}

/// <summary>
/// Initializes the specified size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UIViewport::initialize(SizeU size) noexcept {
    // 父类
    Super::initialize(pugi::xml_node());
    // 更新大小
    force_cast(this->box.vsize.width) = static_cast<float>(size.width);
    force_cast(this->box.vsize.height) = static_cast<float>(size.height);
    // 可视区域范围
    force_cast(box.rect).right = this->box.vsize.width;
    force_cast(box.rect).bottom = this->box.vsize.height;
    force_cast(this->layout.content) = this->box.vsize;
}


/// <summary>
/// Disposes this instance.
/// UIViewport 释放控件
/// </summary>
/// <returns></returns>
void LongUI::UIViewport::dispose() noexcept {
    // 删前调用
    this->before_deleted();
    // 删除对象
    delete this;
}

/// <summary>
/// Creates the without XML.
/// </summary>
/// <returns></returns>
auto LongUI::UIViewport::CreateWithoutXml(
    XUIBaseWindow& wnd, SizeU size
) noexcept -> UIViewport* {
    auto viewport = new(std::nothrow) UIViewport(wnd);
    if (viewport) viewport->initialize(size);
    return viewport;
}


/// <summary>
/// Initializes a new instance of the <see cref="XUIBaseWindow"/> class.
/// </summary>
LongUI::XUIBaseWindow::XUIBaseWindow(
    const Config::Window& config) noexcept 
    : m_pParent(config.parent),
    m_spHoverTracked(nullptr),
    m_spFocusedControl(nullptr),
    m_spDragDropControl(nullptr),
    m_spCapturedControl(nullptr),
    m_tmCaret(::GetCaretBlinkTime()) {
    // Xml节点
    auto node = config.node;
    m_vTabstops.reserve(32);
#ifdef _DEBUG
    m_baBoolWindow.Test<INDEX_COUNT>();
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
        m_miniSize.width = static_cast<LONG>(size[0]);
        m_miniSize.height = static_cast<LONG>(size[1]);
        // 清理颜色
        Helper::MakeColor(
            node.attribute(Attribute::WindowClearColor).value(),
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
#include <atomic>
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
    for (auto inset : m_vInsets) inset->Dispose();
    for (auto ctrl : m_vTabstops) ctrl->Release();
    m_pViewport->Release(); m_pViewport = nullptr;
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
    m_pViewport->OnClose();
    // 激活父窗口
    if (m_pParent) {
        if (!this->is_popup_window()) {
            ::EnableWindow(m_pParent->GetHwnd(), true);
        }
        ::SetFocus(m_pParent->GetHwnd());
    }
    // 延迟清理
    UIManager.PushDelayCleanup(*this);
    // 跳过渲染
    this->set_skip_render();
}


/// <summary>
/// Create_child_window the specified node.
/// </summary>
/// <param name="node">The node.</param>
/// <param name="parent">The parent.</param>
/// <param name="func">The function.</param>
/// <returns></returns>
auto LongUI::XUIBaseWindow::create_child_window(
    pugi::xml_node node,
    XUIBaseWindow* parent,
    callback_create_viewport func
) noexcept->XUIBaseWindow* {
    return UIManager.create_ui_window(node, parent, func);
}

/// <summary>
/// Creates the popup.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="height">The height.</param>
/// <param name="child">The child.</param>
/// <returns></returns>
auto LongUI::XUIBaseWindow::CreatePopup(
    const RectWHL& pos, 
    uint32_t height,
    UIControl* child
) noexcept -> XUIBaseWindow* {
    assert(pos.width && "bad width");
    assert(height && "bad height");
    assert(!"NOTIMPL");
    /*SizeU size { uint32_t(pos.width), height };
    // 宽度不足
    if (!size.width || !size.width) return nullptr;
    HRESULT hr;
    Config::Window config;
    // 窗口配置
    config.parent = this;
    config.width = pos.width;
    config.height = height;
    config.popup = true;
    config.system = true;
    // 创建窗口
    auto window = LongUI::CreateBuiltinWindow(config);
    assert(window && "create system window failed");
    if (!window) return nullptr;
    // 创建视口
    auto viewport = UIViewport::CreateWithoutXml(*window, size);
    assert(viewport && "create viewport failed");
    if (!viewport) {
        window->Dispose();
        return nullptr;
    }
    // 设置清理颜色
    window->clear_color = impl::lui(D2D1::ColorF(D2D1::ColorF::White, 0.5f));
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
    if (FAILED(hr = window->Recreate())) {
        UIManager.ShowError(hr);
    }
    // 创建完毕
    viewport->DoLongUIEvent(Event::Event_TreeBuildingFinished);
    // 移动窗口
    window->MoveWindow(pos.left, pos.bottom);
    // 返回创建窗口
    return window;*/
    return nullptr;
}

/// <summary>
/// Finds the control.
/// </summary>
/// <param name="cname">The cname.</param>
/// <returns></returns>
auto LongUI::XUIBaseWindow::FindControl(
    const char cname[]) noexcept -> CtrlPtr<UIControl> {
    assert(cname && (*cname) && "bad argument");
    // 查找控件
    auto result = m_hashStr2Ctrl.Find(cname);
    // 未找到返回空
    if (!result) {
        // 给予警告
        LUIDebug(Warning) 
            << L" Control Not Found: " 
            << cname 
            << LongUI::endl;
        return CtrlPtr<UIControl>{nullptr};
    }
    else {
        //auto tmpctrl = reinterpret_cast<LongUI::UIControl*>(*result);
        //return CtrlPtr<UIControl>{tmpctrl};
        return CtrlPtr<UIControl>{reinterpret_cast<LongUI::UIControl*>(*result)};
    }
}

/// <summary>
/// Adds the tabstop.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::AddTabstop(UIControl& ctrl) noexcept {
    assert(&ctrl);
    if (m_vTabstops.isok()) {
        ctrl.AddRef();
        m_vTabstops.push_back(&ctrl);
    }
}

/// <summary>
/// remove the tabstop.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::RemoveTabstop(UIControl& ctrl) noexcept {
    assert(&ctrl);
    if (m_vTabstops.isok()) {
        auto itr = std::find(m_vTabstops.begin(), m_vTabstops.end(), &ctrl);
        if (itr != m_vTabstops.end()) {
            assert(&ctrl == *itr);
            ctrl.Release();
            m_vTabstops.erase(itr);
        }
    }
}


/// <summary>
/// Finds the next tabstop.
/// </summary>
/// <param name="rctrl">The control.</param>
/// <returns></returns>
auto LongUI::XUIBaseWindow::FindNextTabstop(
    UIControl& rctrl) const noexcept ->CtrlPtr<UIControl> {
    auto ctrl = &rctrl;
    assert(ctrl && "bad argument");
    auto imp = [=]() noexcept {
        if (m_vTabstops.empty()) return ctrl;
        auto itr = std::find(m_vTabstops.begin(), m_vTabstops.end(), ctrl);
        const auto last = --m_vTabstops.end();
        if (itr == m_vTabstops.end()) {
            return *last;
        }
        else if (itr == last) {
            return m_vTabstops.front();
        }
        else {
            return *++itr;
        }
    };
    auto c = imp();
    return CtrlPtr<UIControl>{c};
}


/// <summary>
/// Finds the previous tabstop.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::XUIBaseWindow::FindPrevTabstop(
    UIControl& rctrl) const noexcept ->CtrlPtr<UIControl> {
    auto ctrl = &rctrl;
    assert(ctrl && "bad argument");
    auto imp = [=]() noexcept {
        if (m_vTabstops.empty()) return ctrl;
        auto itr = std::find(m_vTabstops.begin(), m_vTabstops.end(), ctrl);
        if (itr == m_vTabstops.end()) {
            return m_vTabstops[0];
        }
        else if (itr == m_vTabstops.begin()) {
            return m_vTabstops.back();
        }
        else {
            return *--itr;
        }
    };
    auto c = imp();
    return CtrlPtr<UIControl>{c};
}


/// <summary>
/// Adds the named control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::AddNamedControl(UIControl& ctrl) noexcept {
    assert(&ctrl && "bad argumrnt");
    const auto cname = ctrl.name.c_str();
    // 有效
    if (cname[0]) {
        // 插入
#ifdef _DEBUG
        {
            auto result = m_hashStr2Ctrl.Find(cname);
            assert(result == nullptr && "control exsited!");
        }
#endif
        if (!m_hashStr2Ctrl.Insert(cname, &ctrl)) {
            UIManager.ShowError(L"Failed to add control");
        }
    }
}

/// <summary>
/// Sets the hover track control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::SetHoverTrack(UIControl& ctrl) noexcept {
    auto ptr = &ctrl;
    assert(ptr && "bad argument");
    if (ptr->GetHoverTrackTime()) m_spHoverTracked = ptr;
}

/// <summary>
/// Sets the focus.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::SetFocus(UIControl* ctrl) noexcept {
    // 有效
    if (m_spFocusedControl) {
        // 事件
        m_spFocusedControl->DoLongUIEvent(
            Event::Event_KillFocus, 
            m_pViewport
        );
        // 渲染
        this->Invalidate(*m_spFocusedControl);
        // 归零
        m_spFocusedControl = nullptr;
        // 清除
        this->clear_caret_in();
    }
    // 聚焦
    if (ctrl && ctrl->state.flags & Flag_Focusable) {
        // 有效
        m_spFocusedControl = ctrl;
        // 事件
        m_spFocusedControl->DoLongUIEvent(
            Event::Event_SetFocus, 
            m_pViewport
        );
        // 渲染
        this->Invalidate(*m_spFocusedControl);
    }
}

/// <summary>
/// Sets the capture control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::SetCapture(UIControl& ctrl) noexcept {
    auto ptr = &ctrl;
    assert(&ctrl && "bad argument");
    ::SetCapture(this->GetHwnd());
    m_spCapturedControl = ptr;
};

/// <summary>
/// Releases the capture control.
/// </summary>
/// <returns></returns>
void LongUI::XUIBaseWindow::ReleaseCapture() noexcept {
    m_spCapturedControl.Dispose();
    //UIManager.DataUnlock();
    ::ReleaseCapture();
    //UIManager.DataLock();
};

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::XUIBaseWindow::Recreate() noexcept ->Result {
    assert(m_pViewport && "no viewport");
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
    // 返回重建结果
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
        m_pViewport->after_update();
    }
    // 遍历
    for (auto inset : m_vInsets) {
        inset->Update();
    }
    // 复制渲染数据以保证数据安全
    m_uUnitLengthRender = m_uUnitLength;
    std::memcpy(m_apUnitRender, m_apUnit, sizeof(m_apUnit[0]) * m_uUnitLengthRender);
    m_baBoolWindow.SetTo<Index_FullRenderThisFrameRender>(this->is_full_render_this_frame());
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
    UIManager.AddWindow(*this);
    this->set_popup_window(config.popup);
}


/// <summary>
/// Finalizes an instance of the <see cref="XUISystemWindow"/> class.
/// </summary>
/// <returns></returns>
LongUI::XUISystemWindow::~XUISystemWindow() noexcept {
    UIManager.RemoveWindow(*this);
}


/// <summary>
/// Sets the name of the title.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
void LongUI::XUISystemWindow::SetTitleName(const wchar_t* name) noexcept {
    assert(name && "bad argument");
    assert(m_hwnd && "no window");
    auto hwnd = this->GetHwnd();
    ::DefWindowProcW(hwnd, WM_SETTEXT, WPARAM(0), LPARAM(name));
}

// 移动窗口
void LongUI::XUISystemWindow::MoveWindow(int32_t x, int32_t y) noexcept {
    m_rcWindow.left = x;
    m_rcWindow.top = y;
    POINT pt = { x, y };
    auto hwnd = this->GetHwnd();
    ::MapWindowPoints(::GetParent(hwnd), nullptr, &pt, 1);
    ::SetWindowPos(hwnd, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE);
}


/// <summary>
/// Initializes the viewport.
/// </summary>
/// <param name="v">The viewport.</param>
/// <returns></returns>
void LongUI::XUIBaseWindow::InitializeViewport(UIViewport& v) noexcept {
    auto viewport = &v;
    assert(viewport && "bad viewport given");
    assert(m_pViewport == nullptr && "InitializeViewport cannot called only once for one instance");
    m_pViewport = viewport;
    assert(!"check if AddRef");
    // 自动适应高DPI
    if (this->is_hidpi_supported()) {
        float x = float(UIManager.GetMainDpiX()) / float(LongUI::BASIC_DPI);
        float y = float(UIManager.GetMainDpiY()) / float(LongUI::BASIC_DPI);
        m_pViewport->SetZoom({ x, y });
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
void LongUI::XUIBaseWindow::Invalidate(UIControl& c) noexcept {
    auto ctrl = &c;
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
        if (existd->state.level < ctrl->state.level) {
            // 是 -> 什么不干
            if (existd->IsPosterityForThis(ctrl)) return void(assert(changed == false));
            // 否 -> 继续
            else {

            }
        }
        // 存在深度 > 插入深度 -> 检查存在的是否为插入的子孙结点
        else if (existd->state.level > ctrl->state.level) {
            // 是 -> 替换所有
            if (ctrl->IsPosterityForThis(existd)) {
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
    if (m_pViewport->state.debug_this) {
        LUIDebug(Log) << L"[INSERT]: " << ctrl << LongUI::endl;
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
            assert(tmpc->IsPosterityForThis(ctrl) == false && "bad ship");
            assert(ctrl->IsPosterityForThis(tmpc) == false && "bad ship");
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
    auto&viewbox = force_cast(m_pViewport->box);
    viewbox.rect.right = static_cast<float>(this->GetWidth());
    viewbox.rect.bottom = static_cast<float>(this->GetHeight());
    m_pViewport->SetWidth(viewbox.rect.right);
    m_pViewport->SetHeight(viewbox.rect.bottom);
    // 强行刷新一帧
    this->InvalidateWindow();
    // 处理了
    this->clear_new_size();
}


// longui namesapce
namespace LongUI {
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
        // just close
        void jc() noexcept { ::PostMessageW(this->GetHwnd(), WM_CLOSE, 0, 0); }
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
        virtual void Close() noexcept override { this->jc(); };
        // recreate
        virtual auto Recreate() noexcept ->Result override;
        // resize
        void Resize(uint32_t w, uint32_t h) noexcept override;
        // set cursor
        virtual void SetCursor(Cursor cursor) noexcept override;
        // show/hide window
        virtual void ShowWindow(int nCmdShow) noexcept override {
            //::AnimateWindow(m_hwnd, 2000, AW_SLIDE | AW_VER_NEGATIVE);
            ::ShowWindow(this->GetHwnd(), nCmdShow); 
            if (m_pParent && !this->is_popup_window()) {
                ::EnableWindow(m_pParent->GetHwnd(), !nCmdShow);
            }
        }
        // set caret
        virtual void SetCaret(UIControl& ctrl, const RectWHF* rect) noexcept override;
#ifdef _DEBUG
        // set titlename
        virtual void SetTitleName(const wchar_t* name) noexcept override {
            m_strTitle = name;
            Super::SetTitleName(name);
        }
#endif
    public:
        // normal event
        bool MessageHandle(UINT , WPARAM , LPARAM , LRESULT& result) noexcept;
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
        static auto WINAPI WndProc(HWND , UINT , WPARAM , LPARAM ) noexcept->LRESULT;
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
    auto code = ::GetClassInfoExW(ins, Attribute::WindowClassNameN, &wcex);
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
        wcex.lpszClassName = Attribute::WindowClassNameN;
        wcex.hIcon = nullptr;// ::LoadIconW(ins, MAKEINTRESOURCEW(101));
        ::RegisterClassExW(&wcex);
        // 注册弹出窗口类
        wcex.style = CS_DROPSHADOW;
        wcex.lpszClassName = Attribute::WindowClassNameP;
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
            if (m_spFocusedControl) {
                // 事件
                m_spFocusedControl->DoLongUIEvent(
                    Event::Event_KillFocus, m_pViewport
                );
                // 释放引用
                m_spFocusedControl.Dispose();
            }
            // 重置
            m_rcCaret.left = -1.f;
            m_rcCaret.right = 0.f;
            // 检查属性
            close_window = this->is_close_on_focus_killed();
        }
        // 失去焦点即关闭窗口
        if (close_window) {
            auto hwnd = this->GetHwnd();
            ::PostMessageW(hwnd, WM_CLOSE, 0, 0);
        }
        // 关闭插入符号
        ::DestroyCaret();
    };
    // --------------------------  字符输入
    auto on_char = [this, wParam]() noexcept {
        CUIDataAutoLocker locker;
        if (m_spFocusedControl) {
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
            m_spFocusedControl->DoEvent(arg);
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
        ::CreateCaret(this->GetHwnd(), nullptr, 1, 1);
        return true;
    case WM_KILLFOCUS:
        on_killfocus();
        UIManager.OnKillFocus();
        return true;
    case WM_KEYDOWN:
        // 键入字符
    {
        CUIDataAutoLocker locker;
        if (m_spFocusedControl) {
            EventArgument arg;
            arg.sender = m_pViewport;
            arg.key.ch = static_cast<char32_t>(wParam);
            arg.event = Event::Event_KeyDown;
            m_spFocusedControl->DoEvent(arg);
        }
        return true;
    }
    case WM_CHAR:
        // 键入字符
        on_char();
        return true;
    case WM_DWMCOMPOSITIONCHANGED:
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
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize = { 
            static_cast<LONG>(m_miniSize.width),
            static_cast<LONG>(m_miniSize.height)
        };
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
            m_pViewport->SetZoom({ x, y });
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
        if (ma.event == MouseEvent::Event_MouseHover && m_spHoverTracked) {
            return m_spHoverTracked->DoMouseEvent(ma);
        }
        // 存在捕获控件
        if (m_spCapturedControl) {
            return m_spCapturedControl->DoMouseEvent(ma);
        }
        // 窗口实现
        auto code = m_pViewport->DoMouseEvent(ma);
        // 设置跟踪
        if (message == WM_MOUSEMOVE) {
            m_csTME.dwHoverTime = m_spHoverTracked ? DWORD(m_spHoverTracked->GetHoverTrackTime()) : DWORD(0);
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
    UIManager_RenderTarget.SetTarget(nullptr);
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
        LUIDebug(Hint) << L"Window: ["
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
            LUIDebug(Hint) << L"Recreate device" << LongUI::endl;
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
            hr = UIManager_RenderTarget.CreateBitmapFromDxgiSurface(
                pDxgiBackBuffer,
                &bitmapProperties,
                &m_pTargetBitmap
            );
            longui_debug_hr(hr, L"UIManager_RenderTarget.CreateBitmapFromDxgiSurface faild");
        }
        // 重建失败?
        if (FAILED(hr)) {
            LUIDebug(Error) << L" Recreate FAILED!" << LongUI::endl;
            UIManager.ShowError(hr);
        }
        LongUI::SafeRelease(pDxgiBackBuffer);
    }
    // 父类调用
    this->resized();
}

// dwm api
//#include <dwmapi.h>

/// <summary>
/// Called when [create].
/// </summary>
/// <param name="hwnd">The HWND.</param>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::OnCreate(HWND hwnd) noexcept {
    // task bar
    CHANGEFILTERSTRUCT cfs = { sizeof(CHANGEFILTERSTRUCT) };
    ::ChangeWindowMessageFilterEx(hwnd, s_uTaskbarBtnCreatedMsg, MSGFLT_ALLOW, &cfs);

    /*// Inform application of the frame change.
    RECT rcClient; ::GetWindowRect(hwnd, &rcClient);
    ::SetWindowPos(hwnd,
        nullptr,
        rcClient.left, rcClient.top,
        rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        SWP_FRAMECHANGED
    );

    // client area
    MARGINS margins = { -1 };
    auto hr = ::DwmExtendFrameIntoClientArea(hwnd, &margins);
    if (FAILED(hr)) {
        UIManager.ShowError(hr, L"DwmExtendFrameIntoClientArea");
    }*/
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIBuiltinSystemWindow::Recreate() noexcept ->Result {
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
        auto hwnd = this->GetHwnd();
        RECT rect = { 0 }; ::GetClientRect(hwnd, &rect);
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
            hr = UIManager_DXGIFactory.CreateSwapChainForComposition(
                &UIManager_DXGIDevice,
                &swapChainDesc,
                nullptr,
                &pSwapChain
            );
            longui_debug_hr(hr, L"UIManager_DXGIFactory.CreateSwapChainForComposition faild");
        }
        else {
            // 一般桌面应用程序
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            // 利用窗口句柄创建交换链
            hr = UIManager_DXGIFactory.CreateSwapChainForHwnd(
                &UIManager_D3DDevice,
                this->GetHwnd(),
                &swapChainDesc,
                nullptr,
                nullptr,
                &pSwapChain
            );
            longui_debug_hr(hr, L"UIManager_DXGIFactory.CreateSwapChainForHwnd faild");
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
        hr = UIManager_RenderTarget.CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pTargetBitmap
        );
        longui_debug_hr(hr, L"UIManager_RenderTarget.CreateBitmapFromDxgiSurface faild");
    }
    // 使用DComp
    if (this->is_direct_composition()) {
        // 创建直接组合(Direct Composition)设备
        if (SUCCEEDED(hr)) {
            hr = LongUI::Dll::DCompositionCreateDevice(
                &UIManager_DXGIDevice,
                LongUI_IID_PV_ARGS(m_pDcompDevice)
            );
            longui_debug_hr(hr, L"DCompositionCreateDevice faild");
        }
        // 创建直接组合(Direct Composition)目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateTargetForHwnd(
                this->GetHwnd(), true, &m_pDcompTarget
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
    if (m_tmCaret.Update() && m_spFocusedControl) {
        // 反转插入符号
        this->change_caret_in();
#if 0
        // 不显示插入符号时候刷新显示焦点控件
        if (m_pFocusedControl && !this->is_caret_in()) {
            m_pFocusedControl->InvalidateThis();
        }
#else
        // 刷新显示焦点控件
        if (m_rcCaret.right > 0.f) {
            m_spFocusedControl->Invalidate();
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
    UIManager_RenderTarget.SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE(m_textAntiMode));
    // 设为当前渲染对象
    UIManager_RenderTarget.SetTarget(m_pTargetBitmap);
    // 开始渲染
    UIManager_RenderTarget.BeginDraw();
    // 设置转换矩阵
#if 0
    UIManager_RenderTarget.SetTransform(DX::Matrix3x2F::Identity());
#else
    UIManager_RenderTarget.SetTransform(&impl::d2d(m_pViewport->box.world));
#endif
    // 清空背景
    UIManager_RenderTarget.Clear(&impl::d2d(this->clear_color));
}

/// <summary>
/// Ends the render.
/// </summary>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::EndRender() const noexcept {
    // 渲染插入符号
    if (this->is_caret_in() && m_rcCaret.right != 0.f) {
        constexpr auto mode = D2D1_ANTIALIAS_MODE_ALIASED;
        UIManager_RenderTarget.SetTransform(impl::d2d(DX::Matrix3x2F::Identity()));
        UIManager_RenderTarget.PushAxisAlignedClip(&m_rcCaret, mode);
        UIManager_RenderTarget.Clear(D2D1::ColorF(D2D1::ColorF::Black));
        UIManager_RenderTarget.PopAxisAlignedClip();
    }
    // 结束渲染
    auto hr = UIManager_RenderTarget.EndDraw();
    hr = S_OK;
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
                const auto& vrt = (*control)->box.rect;
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
        LUIDebug(Hint) << L"D2DERR_RECREATE_TARGET!" << LongUI::endl;
        hr = UIManager.RecreateResources();
        if (FAILED(hr)) {
            LUIDebug(Hint) << L"But, Recreate Failed!!!" << LongUI::endl;
            LUIDebug(Error) << L"Recreate Failed!!!" << LongUI::endl;
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
    {
        const auto ptr = const_cast<CUIBuiltinSystemWindow* const>(this);
        ptr->XUISystemWindow::SetTitleName(buffer);
    }
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
            UIManager_RenderTarget.SetTransform(impl::d2d(DX::Matrix3x2F::Identity()));
            constexpr auto aamode = D2D1_ANTIALIAS_MODE_ALIASED;
            UIManager_RenderTarget.PushAxisAlignedClip(&impl::d2d(ctrl->box.rect), aamode);
            UIManager_RenderTarget.SetTransform(&impl::d2d(ctrl->box.world));
            // 渲染背景笔刷?
            /*if (ctrl->backgroud != ctrl && ctrl->backgroud) {
                auto bk = ctrl->backgroud;
                UIManager_RenderTarget.SetTransform(&bk->world);
                bk->RenderBackgroudBrush();
                UIManager_RenderTarget.SetTransform(&ctrl->world);
            }*/
            // 正常渲染
            ctrl->Render();
            // 回来
            UIManager_RenderTarget.PopAxisAlignedClip();
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
/// <param name="rect">The caret rect pointer.</param>
/// <returns></returns>
void LongUI::CUIBuiltinSystemWindow::SetCaret(UIControl& ctrl, const RectWHF* rect) noexcept {
    // 必须有效
    assert(&ctrl && "bad argument");
    // 显示插入符号
    if (rect) {
        // 记录老坐标
        auto oldx = m_rcCaret.left;
        auto oldy = m_rcCaret.top;
        // 计算一般位置
        Point2F pt{ rect->left, rect->top };
        pt = Helper::TransformPoint(ctrl.box.world, pt);
        // 位置差不多
        auto xyabs = std::abs(oldx - pt.x) + std::abs(oldy - pt.y);
        if (xyabs < 1.f) return;
        // 继续计算
        m_rcCaret.left = pt.x;
        m_rcCaret.top = pt.y;
        m_rcCaret.right = pt.x + rect->width * ctrl.box.world._11;
        m_rcCaret.bottom = pt.y + rect->height * ctrl.box.world._22;
        // 计算可视化区域
        m_rcCaret.top   =  std::max(ctrl.box.rect.top, m_rcCaret.top);
        m_rcCaret.left  =  std::max(ctrl.box.rect.left, m_rcCaret.left);
        m_rcCaret.right =  std::min(ctrl.box.rect.right, m_rcCaret.right);
        m_rcCaret.bottom=  std::min(ctrl.box.rect.bottom, m_rcCaret.bottom);
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
        ctrl.Invalidate();
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
        auto s = node.attribute(Attribute::WindowTitleName).value();
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
                node.attribute(Attribute::AllSize).value(),
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
            config.popup ? Attribute::WindowClassNameP : Attribute::WindowClassNameN,
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
            auto hwnd = this->GetHwnd();
            UIManager_DXGIFactory.MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
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
    m_csTME.hwndTrack = this->GetHwnd();
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
    ::SetWindowLongPtrW(this->GetHwnd(), GWLP_USERDATA, LONG_PTR(0));
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
