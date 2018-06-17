#include <core/ui_window.h>
#include <debugger/ui_debug.h>
#include <util/ui_time_meter.h>
#include <container/pod_vector.h> 
#include <core/ui_window_manager.h>


#include <cassert>
#include <algorithm>

/// <summary>
/// private data/func for WndMgr
/// </summary>
struct LongUI::PrivateWndMgr {
    // windows
    using WindowVector = POD::Vector<CUIWindow*>;
    // ctor
    PrivateWndMgr() {};
    // dtor
    ~PrivateWndMgr() {}
    // high time-meter
    CUITimeMeterH           timemeter;
    // windows list for updating
    WindowVector            windowsu;
    // windows list for rendering
    WindowVector            windowsr;
};


/// <summary>
/// Gets the window list.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWndMgr::GetWindowList() const noexcept -> const WindowVector& {
    return reinterpret_cast<const WindowVector&>(wm().windowsu);
}


/// <summary>
/// Marks the window minsize changed.
/// </summary>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::MarkWindowMinsizeChanged(CUIWindow* window) noexcept {
    if (window) {
#ifndef NDEBUG
        // 必须在窗口列表里面
        const auto b = wm().windowsu.begin();
        const auto e = wm().windowsu.end();
        assert(std::find(b, e, window) != e && "not found");
#endif
        window->m_bMinsizeList = true;
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIWndMgr"/> class.
/// </summary>
LongUI::CUIWndMgr::CUIWndMgr(Result& out) noexcept {
    // 静态检查
    enum {
        wm_size = sizeof(PrivateWndMgr),
        pw_size = detail::private_wndmgr<sizeof(void*)>::size,

        wm_align = alignof(PrivateWndMgr),
        pw_align = detail::private_wndmgr<sizeof(void*)>::align,
    };
    static_assert(wm_size == pw_size, "must be same");
    static_assert(wm_align == pw_align, "must be same");
    // 已经发生错误
    if (!out) return;
    detail::ctor_dtor<PrivateWndMgr>::create(&wm());
    Result hr = { Result::RS_OK };
    // 尝试扩展窗口列表
    if (hr) {
        wm().windowsu.reserve(16);
        wm().windowsr.reserve(16);
        // 内存不足
        if (!wm().windowsu.is_ok() || !wm().windowsr.is_ok())
            hr = { Result::RE_OUTOFMEMORY };
    }
    // 初始化其他
    if (hr) {
        wm().windowsu.clear();
        wm().windowsr.clear();
        // 开始计时
        wm().timemeter.Start();
        // 更新显示频率
        this->refresh_display_frequency();
    }
    out = hr;
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIWndMgr"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIWndMgr::~CUIWndMgr() noexcept {
    // 删除还存在的窗口
    auto& list = wm().windowsu;
    while (!list.empty()) {
        assert(list.front()->IsTopLevel() && "top level must be front");
        list.back()->Delete();
    }
    // 析构掉
    wm().~PrivateWndMgr();
}

/// <summary>
/// Updates the delta time.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWndMgr::update_delta_time() noexcept -> float {
    // TODO: 固定时间更新频率
    auto& meter = wm().timemeter;
    const auto time = meter.Delta_ms<double>();
    meter.MovStartEnd();
    return static_cast<float>(time);
}


/// <summary>
/// Adds the window.
/// </summary>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::add_window(CUIWindow& window) noexcept {
    auto& wnds = wm().windowsu;
    const auto ptr = &window;
#ifndef NDEBUG
    // 检查是否在里面
    auto itr = std::find(wnds.begin(), wnds.end(), ptr);
    assert(itr == wnds.end() && "bug");
#endif
    wnds.push_back(ptr);
}


/// <summary>
/// Removes the window.
/// </summary>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::remove_window(CUIWindow & window) noexcept {
    auto& wnds = wm().windowsu;
    const auto ptr = &window;
    assert(wnds.size() && "bug");
    // 优化: 删除最后一个
    /*if (wnds.back() == ptr) {
        wnds.erase(wnds.size() - 1);
    }
    // 检查是否在里面
    else*/ {
        auto itr = std::find(wnds.begin(), wnds.end(), ptr);
        assert(itr != wnds.end() && "bug");
        wnds.erase(itr);
    }
}


/// <summary>
/// Pres the render windows.
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::before_render_windows() noexcept {
    // 进行预渲染
    // XXX: 优化
    for (auto window : wm().windowsr) {
        window->BeforeRender();
    }
}

/// <summary>
/// Renders the windows.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWndMgr::render_windows() noexcept -> Result {
    // 执行渲染
    auto do_render = [](POD::Vector<CUIWindow*>& v) noexcept {
        for (auto window : v) {
            const auto hr = window->Render();
            if (!hr) return hr;
        }
        return Result{ Result::RS_OK };
    };
    // 渲染记录
    auto hr = do_render(wm().windowsr);
    wm().windowsr.clear();
    return hr;
}


// private control
#include "../private/ui_private_control.h"
#include <control/ui_viewport.h>


/// <summary>
/// Refreshes the window minsize.
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::refresh_window_minsize() noexcept {
    auto& list = wm().windowsu;
    // 因为和渲染线程在同一线程所以无需加锁

    // 检测窗口
    for (auto window : list) {
        // 需要刷新最小大小
        if (window->m_bMinsizeList) {
            window->m_bMinsizeList = false;
            UIViewport& root = window->RefViewport();
            UIControlPrivate::RefreshMinSize(root);
        }
    }
}

/// <summary>
/// Refreshes the window world.
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::refresh_window_world() noexcept {
    // 检测每个窗口的最小世界修改
    for (auto window : wm().windowsu) {
        if (const auto ctrl = window->m_pTopestWcc) {
            window->m_pTopestWcc = nullptr;
            const auto top = ctrl->IsTopLevel() ? ctrl : ctrl->GetParent();
            UIControlPrivate::UpdateWorld(*top);
        }
    }
    // 复制窗口数据保证线程安全
    wm().windowsr = wm().windowsu;
}


/// <summary>
/// Closes the helper.
/// </summary>
/// <param name="wnd">The WND.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::close_helper(CUIWindow& wnd) noexcept {
    // 隐藏该窗口
    wnd.HideWindow();
    // 直接退出?
    if (wnd.config & CUIWindow::Config_QuitOnClose) 
        return this->exit();
    // 删除窗口
    if (wnd.config & CUIWindow::Config_DeleteOnClose)
        wnd.RefViewport().DeleteLater();
    // 最后一个窗口关闭即退出?
    if (this->is_quit_on_last_window_closed()) {
        // 遍历窗口
        for (auto x : wm().windowsu) {
            // 不考虑非顶层的
            if (!x->IsTopLevel()) continue;
            // 不考虑工具窗口
            if (x->config & CUIWindow::Config_ToolWindow) continue;
            // 有一个在就算了
            if (x->IsVisible()) return;
        }
        // 退出程序
        this->exit();
    }
}

// ----------------------------------------------------------------------------

#include <Windows.h>

/// <summary>
/// Refresh_display_frequencies this instance.
/// 刷新屏幕刷新率
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::refresh_display_frequency() noexcept {
    // 获取屏幕刷新率
    DEVMODEW mode; std::memset(&mode, 0, sizeof(mode));
    ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
    m_dwDisplayFrequency = static_cast<uint32_t>(mode.dmDisplayFrequency);
    // 稍微检查
    if (!m_dwDisplayFrequency) {
        LUIDebug(Error)
            << "EnumDisplaySettingsW failed: "
            << "got zero for DEVMODEW::dmDisplayFrequency"
            << ", now assume as 60Hz"
            << LongUI::endl;
        m_dwDisplayFrequency = 60;
    }
}

/// <summary>
/// Sleeps for vblank.
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::sleep_for_vblank() noexcept {
    // XXX: vblank
    ::Sleep(15);
}

