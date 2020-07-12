#include <core/ui_window.h>
#include <debugger/ui_debug.h>
#include <util/ui_time_meter.h>
#include <control/ui_viewport.h>
#include <container/pod_vector.h> 
#include <core/ui_window_manager.h>


#include <cassert>
#include <algorithm>

//namespace LongUI { namespace impl {
//    // find viewport
//    UIViewport* find_viewport(
//        const POD::Vector<UIViewport*>&, 
//        const char*
//    ) noexcept;
//}}

namespace LongUI {
    // 视口?
    bool IsViewport(const UIControl&) noexcept;
    // all
    struct AllWindows : Node<AllWindows> {};
}

/// <summary>
/// private data/func for WndMgr
/// </summary>
struct LongUI::CUIWndMgr::Private {
    // windows cast
    static inline auto cast(CUIWindow* windows) noexcept {
        assert(windows);
        return reinterpret_cast<AllWindows*>(&windows->m_oListNode);
    }
    // windows cast
    static inline auto cast(AllWindows* windows) noexcept {
        assert(windows);
        constexpr size_t of = offsetof(CUIWindow, m_oListNode);
        const auto addr = reinterpret_cast<char*>(windows);
        return reinterpret_cast<CUIWindow*>(addr - of);
    }
    // end iterator
    auto end()noexcept ->Node<AllWindows>::Iterator {;
        return { static_cast<AllWindows*>(&tail) };
    }
    // begin iterator
    auto begin()noexcept->Node<AllWindows>::Iterator {
        return { static_cast<AllWindows*>(head.next) };
    }
    // windows
    using WindowVector = POD::Vector<CUIWindow*>;
    // viewports
    using ViewportVector = POD::Vector<UIViewport*>;
    // ctor
    Private() noexcept;
    // dtor
    ~Private() noexcept {}
    // high time-meter
    CUITimeMeterH       timemeter;
    // all window list - head
    Node<AllWindows>    head;
    // all window list - tail
    Node<AllWindows>    tail;
    // subviews
    Node<UIControl>     subviews;
#if 0
    // windows list for updating
    WindowVector        windowsu;
    // windows list for rendering
    WindowVector        windowsr;
    // subviewports
    ViewportVector      subviewports;
#endif
};


/// <summary>
/// Initializes a new instance of the <see cref="PrivateWndMgr"/> struct.
/// </summary>
LongUI::CUIWndMgr::Private::Private() noexcept {
    head = { nullptr, static_cast<AllWindows*>(&tail) };
    tail = { static_cast<AllWindows*>(&head), nullptr };
    const auto ptr = static_cast<UIControl*>(&subviews);
    subviews = { ptr, ptr };
}


/// <summary>
/// Moves the sub view to global.
/// </summary>
/// <param name="vp">The vp.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::MoveToGlobalSubView(UIViewport& sub) noexcept {
    // 之前的数据
    if (sub.prev) static_cast<UIViewport*>(sub.prev)->next = sub.next;
    if (sub.next) static_cast<UIViewport*>(sub.next)->prev = sub.prev;
    // 清除数据
    sub.RefWindow().m_pParent = nullptr;
    // 连接前后节点
    auto& subviews = wm().subviews;
    static_cast<UIViewport*>(subviews.prev)->next = &sub;
    sub.prev = static_cast<UIViewport*>(subviews.prev);
    sub.next = static_cast<UIControl*>(&subviews);
    subviews.prev = &sub;
}

/// <summary>
/// Finds the sub viewport with uid-string.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
auto LongUI::CUIWndMgr::FindSubViewportWithUID(const char* name)const noexcept->UIViewport*{
    auto& subviews = wm().subviews;
    return UIViewport::FindSubViewport(subviews.next, subviews, name);
}



/// <summary>
/// Ends this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWndMgr::end()noexcept->Iterator {
    return { static_cast<CUIWindow*>(&m_oTail) };
}
/// <summary>
/// Begins this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWndMgr::begin()noexcept->Iterator {
    return { static_cast<CUIWindow*>(m_oHead.next) }; 
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIWndMgr"/> class.
/// </summary>
LongUI::CUIWndMgr::CUIWndMgr(Result& out) noexcept {
    // 节点
    m_oHead = { nullptr, static_cast<CUIWindow*>(&m_oTail) };
    m_oTail = { static_cast<CUIWindow*>(&m_oHead), nullptr };
    // 静态检查
    enum {
        wm_size = sizeof(Private),
        pw_size = impl::private_wndmgr<sizeof(void*)>::size,

        wm_align = alignof(Private),
        pw_align = impl::private_wndmgr<sizeof(void*)>::align,
    };
    static_assert(wm_size == pw_size, "must be same");
    static_assert(wm_align == pw_align, "must be same");
    impl::ctor_dtor<Private>::create(&wm());
    // 开始计时
    wm().timemeter.Start();
    // 更新显示频率
    this->refresh_display_frequency();
    // 已经发生错误
    //if (!out) return;
}


/// <summary>
/// Deletes all window.
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::delete_all_window() noexcept {
    auto& subviews = wm().subviews;
    // 释放sub列表?
    auto node = subviews.next;
    while (node != &subviews) {
        assert(LongUI::IsViewport(*node));
        const auto view = static_cast<UIViewport*>(node);
        node = view->next;
        delete view;
    }
    // 删除还存在的窗口
    while (m_oHead.next != &m_oTail)
        m_oTail.prev->Delete();
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIWndMgr"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIWndMgr::~CUIWndMgr() noexcept {
    assert(m_oHead.next == &m_oTail);
    // 析构掉
    wm().~Private();
}

/// <summary>
/// Updates the delta time.
/// </summary>
/// <returns></returns>
auto LongUI::CUIWndMgr::update_delta_time() noexcept -> float {
    // TODO: 固定时间更新频率
    auto& meter = wm().timemeter;
    const auto time = meter.Delta_s<double>();
    meter.MovStartEnd();
    return static_cast<float>(time);
}


/// <summary>
/// Adds the topwindow.
/// </summary>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::add_topwindow(CUIWindow& window) noexcept {
    // 连接前后节点
    m_oTail.prev->next = &window;
    window.prev = m_oTail.prev;
    window.next = static_cast<CUIWindow*>(&m_oTail);
    m_oTail.prev = &window;
}

/// <summary>
/// Adds to allwindow.
/// </summary>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::add_to_allwindow(CUIWindow& window) noexcept {
    const auto allwin = Private::cast(&window);
    auto& tail = wm().tail;
    // 添加到全表中
    tail.prev->next = allwin;
    allwin->prev = tail.prev;
    allwin->next = static_cast<AllWindows*>(&tail);
    tail.prev = allwin;
}

/// <summary>
/// Removes from allwindow.
/// </summary>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::remove_from_allwindow(CUIWindow & window) noexcept {
    const auto allwin = Private::cast(&window);
    // 未初始化就被删除的话节点为空
    if (allwin->prev) {
        // 连接前后节点
        allwin->prev->next = allwin->next;
        allwin->next->prev = allwin->prev;
        allwin->prev = allwin->next = nullptr;
    }
}


/// <summary>
/// Pres the render windows.
/// </summary>
/// <param name="itr">The itr.</param>
/// <param name="endi">The endi.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::before_render_windows(Iterator itr, const Iterator endi) noexcept {
    // XXX: 优化

    // 进行渲染预处理, 途中不该会出现节点变化(下帧执行)
    while (itr != endi) {
        // 执行预处理
        itr->PrepareRender();
        // 预处理子窗口
        before_render_windows(itr->begin(), itr->end());
        // 递进
        ++itr;
    }
}

/// <summary>
/// Renders the windows.
/// </summary>
/// <param name="itr">The itr.</param>
/// <param name="endi">The endi.</param>
/// <returns></returns>
auto LongUI::CUIWndMgr::render_windows(const Iterator begini, const Iterator endi) noexcept -> Result {
    auto itr = begini;
    // 渲染途中不该会出现节点变化(下帧执行)
    while (itr != endi) {
        // 执行渲染
        auto hr = itr->Render();
        // 渲染子窗口
        if (hr) hr = render_windows(itr->begin(), itr->end());
        // 失败
        if (!hr) return hr;
        // 递进
        ++itr;
    }
    // 成功
    return Result{ Result::RS_OK };
}

/// <summary>
/// Recreates the windows.
/// </summary>
/// <param name="begini">The begini.</param>
/// <param name="endi">The endi.</param>
/// <returns></returns>
auto LongUI::CUIWndMgr::recreate_windows() noexcept -> Result {
    Result hr = { Result::RS_OK };
    // 第一次遍历
    const auto traverse_1st = [](Private& primpl) noexcept {
        for (auto& aw : primpl) {
            const auto window = Private::cast(&aw);
            assert(window);
            // 释放所有窗口的设备相关数据
            window->ReleaseDeviceData();
            // 标记全刷新
            window->MarkFullRendering();
        }
    };
    // 第一次遍历
    const auto traverse_2nd = [&](Private& primpl) noexcept {
        for (auto& aw : primpl) {
            const auto window = Private::cast(&aw);
            assert(window);
            // 记录最新的错误数据
            const auto code = window->RecreateDeviceData();
            if (!code) hr = code;
        }
    };
    // 正式处理
    traverse_1st(wm());
    traverse_2nd(wm());
    return hr;
}


#if 0
/// <summary>
/// Refreshes the window minsize.
/// </summary>
/// <param name="itr">The itr.</param>
/// <param name="endi">The endi.</param>
/// <returns></returns>
void LongUI::CUIWndMgr::refresh_window_minsize() noexcept {
    auto& primpl = wm();
    // 处理途中不该会出现节点变化(下帧执行)
    for (auto& aw : primpl) {
        const auto window = Private::cast(&aw);
        assert(window);
        // 需要刷新最小大小
        if (window->m_bMinsizeList) {
            window->m_bMinsizeList = false;
            UIViewport& root = window->RefViewport();
            alignas(uint64_t) const auto minsize1 = root.GetMinSize();
            UIControlPrivate::RefreshMinSize(root);
            alignas(uint64_t) const auto minsize2 = root.GetMinSize();
            // 修改了
            const auto a = reinterpret_cast<const uint64_t&>(minsize1);
            const auto b = reinterpret_cast<const uint64_t&>(minsize2);
            static_assert(sizeof(minsize1) == sizeof(a), "must be same");
            // XXX: 最小大小?
            if (a != b) root.NeedUpdate(Reason_ChildLayoutChanged);
            //if (a != b) root.NeedUpdate(Reason_MinSizeChanged);
        }
    }
}
#endif


// private control
#include "../private/ui_private_control.h"

/// <summary>
/// Refreshes the window world.
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::refresh_window_world() noexcept {
    auto& primpl = wm();
    // 处理途中不该会出现节点变化(下帧执行)
    for (auto& aw : primpl) {
        const auto window = Private::cast(&aw);
        assert(window);
        // 检测每个窗口的最小世界修改
        if (const auto ctrl = window->m_pMiniWorldChange) {
            window->m_pMiniWorldChange = nullptr;
            if (ctrl->IsTopLevel())
                UIControlPrivate::UpdateWorldTop(*ctrl, window->GetAbsoluteSize());
            else
                UIControlPrivate::UpdateWorld(*ctrl);
        }
    }
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
        for (auto& x : (*this)) {
            // 不考虑工具窗口
            if (x.config & CUIWindow::Config_ToolWindow) continue;
            // 有一个在就算了
            if (x.IsVisible()) return;
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
    ::Sleep(16);
}

