#include <luiconf.h>
// ui
#include <interface/ui_default_config.h>
#include <interface/ui_ctrlinfolist.h>
#include <core/ui_const_sstring.h>
#include <container/pod_vector.h>
#include <core/ui_time_capsule.h>
#include <container/pod_hash.h>
#include <filesystem/ui_file.h>
#include <graphics/ui_dcomp.h>
#include <input/ui_kminput.h>
#include <util/ui_ctordtor.h>
#include <core/ui_manager.h>
#include <core/ui_string.h>
#include <util/ui_ostype.h>
#include <util/ui_endian.h>

// c++
#include <type_traits>
#include <new>

// windows com
#include <objbase.h>

/// <summary>
/// Thises the manager.
/// </summary>
/// <returns></returns>
inline auto LongUI::CUIManager::this_() noexcept -> CUIManager* {
#ifdef NDEBUG
    return &UIManager;
#else
    return this;
#endif
}

// private manager
namespace LongUI { struct CUIManager::Private {
    // mark winmin
    static void MarkWndMin() noexcept { UIManager.m_flagWndMinSizeChanged = true; }
    // TC
    using TC = CUITimeCapsule;
    // meta
    using META = const MetaControl;
    // ctor
    Private() noexcept {  }
    // dtor
    ~Private() noexcept {}
    // time capsules
    POD::Vector<TC*>        time_capsules;
    // unique text
    POD::HashMap<void*>     texts;
    // unique control classes
    POD::HashMap<META*>     cclasses;
#ifdef LUI_RAWINPUT
    // km-input
    CUIInputKM              km_input;
#endif

};}





/// <summary>
/// Sets the CSS root dir.
/// </summary>
/// <param name="dir">The dir.</param>
/// <returns></returns>
//void LongUI::CUIManager::SetCSSRootDir(const wchar_t* dir) noexcept {
//    auto& string = this_()->pm().css_root_dir;
//    string = dir;
//    if (string.empty()) return;
//    const auto index = string.length() - 1;
//    if (string[index] == '/' || string[index] == '\\') return;
//    string.append('/');
//}

// delete later
namespace LongUI { enum CallLater : uint32_t {
    Later_First = WM_USER + 10,
    //Later_DeleteControl,
    Later_CallTimeCapsule,
    Later_Exit,
};}

/// <summary>
/// Instances this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIInputKM::Instance() noexcept -> CUIInputKM & {
#ifdef LUI_RAWINPUT
    return UIManager.pm().km_input;
#else
    return *static_cast<CUIInputKM*>(nullptr);
#endif
}


namespace LongUI { 
    // add default control class
    void AddDefaultControlInfo(ControlInfoList&) noexcept;
    // delete control
    void DeleteControl(UIControl*) noexcept;
    // mark control dl?
    void OnTimer(uintptr_t) noexcept;
    // check control dl?
    bool CheckControlDeleteLater(const UIControl&) noexcept;
    // mark control dl?
    void MarkControlDeleteLater(UIControl&) noexcept;
}

#ifndef NDEBUG
#include <util/ui_time_meter.h>
static void ui_endian_runtime_assert() noexcept;
void ui_dbg_set_window_title(LongUI::CUIWindow*, const char*) noexcept;
void ui_dbg_set_window_title(HWND, const char*) noexcept;

namespace LongUI {
    // debug data
    struct CUIManagerDebug {
        // ctor
        CUIManagerDebug() noexcept {}
        // dtor
        ~CUIManagerDebug() noexcept {}
        // next frame
        void NextFrame() noexcept { ++fcounter; }
        // delta time list
        POD::Vector<float>      fpsc;
        // frame counter
        uint32_t                fcounter = 0;
        // push delta
        auto PushDelta(float d, uint32_t f) noexcept {
            constexpr float add_one_sec = 0.233f * 2.33f * 1.2450f;
            if (const auto size = uint32_t(float(f) * add_one_sec)) {
                fpsc.push_back(d);
                if (fpsc.size() >= size) {
                    float time = 0.f;
                    for (auto t : fpsc) time += t;
                    const auto real_size = float(fpsc.size());
                    time /= real_size;
                    fpsc.clear();
                    return time;
                }
            }
            return 0.f;
        }
    };
    // data buffer
    static std::aligned_storage<
        sizeof(CUIManagerDebug), 
        alignof(CUIManagerDebug)
    >::type s_dbgfManager;
    // get inline
    auto DbgMgr() noexcept->CUIManagerDebug& {
        return reinterpret_cast<CUIManagerDebug&>(s_dbgfManager);
    }
    // get frame id
    auto get_frame_id() noexcept -> uint32_t {
        return DbgMgr().fcounter;
    }

}
#endif


/// <summary>
/// do one frame
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::OneFrame() noexcept {
#ifndef NDEBUG
    CUITimeMeterH meter;
    meter.Start();
    int while_count = 0;
#endif
    // 延迟释放锁, 失败就下帧处理
    if (this_()->m_uiLaterLocker.TryLock()) {
        // 删除队列中的控件
        this_()->delete_controls();
        // 交换初始化数据, 控件创建不会阻塞
        this_()->swap_init_list();
        // 释放延迟释放渲染数据, 释放延迟不会阻塞
        this_()->release_later_release();
        // 延迟释放锁 OFF
        this_()->m_uiLaterLocker.Unlock();
    }
#ifndef NDEBUG
    else LUIDebug(Hint) << "Later locker locked in other thread" << endl;
#endif // !NDEBUG
    // 数据更新区域
    this_()->DataLock();
    // 尝试重新创建
    this_()->try_recreate();
    // 记录帧间时间
    this_()->m_fDeltaTime = this_()->update_delta_time();
    // 刷新控件控制
    this_()->normal_update();
    // 初始化控件
    while (this_()->init_control_in_list() || m_flagWndMinSizeChanged) {
        // 标记为空
        m_flagWndMinSizeChanged = false;
        // 更新窗口最小大小
        this_()->refresh_window_minsize();
        // 刷新在表控件
        this_()->update_control_in_list();
#ifndef NDEBUG
        ++while_count;
        if (while_count == 100) {
            LUIDebug(Error) << "MAYBE BUG!"
                << LongUI::endl;
        }
#endif
    }
#ifndef NDEBUG
    // 遍历次数>1
    if (while_count > 2) {
        LUIDebug(Hint) LUI_FRAMEID
            << "frame while loop count : "
            << while_count
            << endl;
    }
#endif
    // 更新世界
    this_()->refresh_window_world();
    // 渲染窗口预处理
    this_()->before_render_windows(this_()->begin(), this_()->end());
    // 记录时间胶囊
    const auto has_tc = this_()->has_time_capsule();
    // 结束数据更新
    this_()->DataUnlock();
#ifndef NDEBUG
    // 记录刷新时间
    const auto t1 = meter.Delta_ms<float>();
    meter.MovStartEnd();
    //meter.Start();
#endif
    // 时间胶囊S1
    if (has_tc) this_()->call_time_capsule_s1();
    // 渲染所有窗口
    this_()->RenderLock();
    const auto hr = this_()->render_windows(this_()->begin(), this_()->end());
    // 渲染完毕, 解除渲染锁
    this_()->RenderUnlock();
    // 时间胶囊S2阶段 - 等待时间胶囊执行完毕
    if (has_tc) this_()->call_time_capsule_s2();
#ifndef NDEBUG
    // 记录渲染时间
    const auto t2 = meter.Delta_ms<float>();
    if (this_()->flag & ConfigureFlag::Flag_DbgOutputTimeTook) {
        CUIDataAutoLocker locker;
        LUIDebug(None) LUI_FRAMEID
            << "U<" << DDFFloat2{ t1 } << "ms>"
            << "R<" << DDFFloat2{ t2 } << "ms>"
            << endl;
    }
    else {
        // update超过2毫秒也算
        if (t1 > 2.f) {
            CUIDataAutoLocker locker;
            LUIDebug(Hint) LUI_FRAMEID
                << "U<" << DDFFloat2{ t1 } << "ms>"
                << "R<" << DDFFloat2{ t2 } << "ms>"
                << endl;
        }
    }

    // 计算FPS
    const auto time = DbgMgr().PushDelta(
        this_()->m_fDeltaTime,
        this_()->m_dwDisplayFrequency
    );
    if (time > 0.f) {
        constexpr size_t buflen = 128;
        char buffer[buflen];
        std::snprintf(
            buffer, buflen,
            "delta: %.2fms -- %2.2f fps",
            time * 1000.f, 1.f / time
        );
        if (this_()->flag & ConfigureFlag::Flag_DbgDebugWindow)
            ::ui_dbg_set_window_title(this_()->m_pDebugWindow, buffer);
        else
            ::ui_dbg_set_window_title(this_()->m_hToolWnd, buffer);
    }
    DbgMgr().NextFrame();
#endif
    // 错误处理
    if (!hr) this_()->OnErrorInfoLost(hr, Occasion_Frame);
}


/// <summary>
/// Waits for v-blank.
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::WaitForVBlank() noexcept {
    if (!this_()->wait_for_vblank()) {
        this_()->sleep_for_vblank();
    }
}


/// <summary>
/// Needs the recreate_device.
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::NeedRecreate() noexcept {
    CUIDataAutoLocker locker;
    m_flagRecreate = true;
}

/// <summary>
/// Exits this instance.
/// </summary>
/// <param name="code">The code.</param>
/// <returns></returns>
void LongUI::CUIManager::Exit(uintptr_t code) noexcept {
    ::PostMessageW(m_hToolWnd, CallLater::Later_Exit, code, 0);
}


/// <summary>
/// Gets the unique text.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
auto LongUI::CUIManager::GetUniqueText(U8View pair) noexcept ->ULID {
    assert(pair.second > pair.first && "bad string");
    auto& texts = this_()->pm().texts;
    const auto code = texts.insert(pair.first, pair.second, nullptr);
    // OOM - 内存不足
    if (code.first == texts.end()) return { CUIConstShortString::EMPTY };
    else return { code.first->first };
}

// windows api
#include <Windows.h>

/// <summary>
/// Creates the control.
/// </summary>
/// <param name="element">The element.</param>
/// <param name="parent">The parent.</param>
/// <returns></returns>
auto LongUI::CUIManager::CreateControl(U8View element,
    UIControl* parent) noexcept -> UIControl* {
    auto& list = this_()->pm().cclasses;
    const auto itr = list.find(element.begin(), element.end());
    if (itr != list.end()) 
        return itr->second->create_func(parent);
    return nullptr;
}


/// <summary>
/// longui namespace
/// </summary>
namespace LongUI {
    // detail namespace
    namespace detail {
        /// <summary>
        /// Marks the wndmin changed.
        /// </summary>
        /// <returns></returns>
        void mark_wndmin_changed() noexcept {
            CUIManager::Private::MarkWndMin();
        }
    }
    // help
    enum {
        pmanag1 = sizeof(CUIManager::Private),
        pm_size = detail::private_manager<sizeof(void*)>::size,
        pm_align= detail::private_manager<sizeof(void*)>::align,
    };
    // check
    static_assert(pm_size == pmanag1, "must be same");
    static_assert(pm_align == alignof(CUIManager::Private), "must be same");
    /// <summary>
    /// single instance buffer for longui manager
    /// </summary>
    std::aligned_storage<sizeof(CUIManager), alignof(CUIManager)>::type s_bufManager;
    // impl
    namespace impl {
        // init HiDPI
        void init_high_dpi_support() noexcept;
        // uninit HiDPI
        void uninit_high_dpi_support() noexcept;
        // init cursor
        void init_cursor() noexcept;
        // init cursor
        void uninit_cursor() noexcept;
#ifndef NDEBUG
        // create debug window
        auto create_debug_window() noexcept->CUIWindow*;
#endif
        // create control
        UIControl* create_control(const char*a, const char*b, UIControl*p) noexcept {
            return UIManager.CreateControl({ a,b }, p);
        }
        // search for last end
        void search_capsule_for_last_end(UIControl& ctrl) noexcept {
            UIManager.RefreshTimeCapsule(ctrl);
        }

    }
}

/// <summary>
/// Calls the time capsule s1.
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::call_time_capsule_s1() noexcept {
    // 获取必要数据
    const auto delta = this_()->m_fDeltaTime;
    const HWND hwnd = UIManager.m_hToolWnd;
    const UINT msg = CallLater::Later_CallTimeCapsule;
    // 解除上级锁
    this_()->DataUnlock();
    // 发送消息
    const auto wp = reinterpret_cast<WPARAM>(&m_uiTimeCapsuleWaiter);
    union { LPARAM lp; float time; };
    lp = 0; time = delta;
    ::PostMessageW(hwnd, msg, wp, lp);
}

/// <summary>
/// Calls the time capsule s2.
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::call_time_capsule_s2() noexcept {
    // 等待执行完毕
    m_uiTimeCapsuleWaiter.Wait();
}

/// <summary>
/// Tries the recreate_device.
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::try_recreate() noexcept {
    // 检查Flag
    if (!m_flagRecreate) return;
    m_flagRecreate = false;
    // 记录最后一次错误
    Result hr = { Result::RS_OK };
    const auto last_error = [&hr](Result code) {
        if (!code) hr = code;
    };
    // 渲染锁
    this_()->RenderLock();
    // 重建设备
    last_error(this_()->CUIResMgr::recreate_device(this_()->config, this_()->flag));
    // 重建资源
    last_error(this_()->CUIResMgr::recreate_resource());
    // 重建窗口
    last_error(this_()->CUIWndMgr::recreate_windows());
    // 渲染锁
    this_()->RenderUnlock();
    // 调试信息
#ifndef NDEBUG
    LUIDebug(Hint)
        << "[Recreate Resource]: "
        << hr
        << endl;
#endif // !NDEBUG
    // 错误处理 - 交给 IUIConfigure
    if (!hr) this_()->config->OnErrorInfoLost(hr, Occasion_Recreate);
}

/// <summary>
/// Sets the timer.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="elapse">The elapse.</param>
/// <param name="id0_7">The id0 7.</param>
/// <returns></returns>
void LongUI::CUIManager::SetTimer(UIControl& ctrl, uint32_t elapse, uint32_t id0_7) noexcept {
    const uintptr_t id = reinterpret_cast<uintptr_t>(&ctrl) + id0_7;
    ::SetTimer(m_hToolWnd, id, elapse, nullptr);
}

/// <summary>
/// Kills the timer.
/// </summary>
/// <param name="">The .</param>
/// <param name="id0_7">The id0 7.</param>
/// <returns></returns>
void LongUI::CUIManager::KillTimer(UIControl& ctrl, uint32_t id0_7) noexcept {
    const uintptr_t id = reinterpret_cast<uintptr_t>(&ctrl) + id0_7;
    ::KillTimer(m_hToolWnd, id);
}

/// <summary>
/// Initializes the specified configuration.
/// </summary>
/// <param name="config">The configuration.</param>
/// <returns></returns>
auto LongUI::CUIManager::Initialize(
    IUIConfigure* cfg,
    ConfigureFlag flag) noexcept -> Result {
#ifndef NDEBUG
    // 大小端断言
    ::ui_endian_runtime_assert();
#endif
    // 设置高DPI支持
    impl::init_high_dpi_support();
    // 设置DComp支持
    impl::init_dcomp_support();
    // 初始化鼠标光标
    impl::init_cursor();
    // 初始化COM
    {
        const Result hr = { ::CoInitialize(nullptr) };
        if (!hr) return hr;
    }
    // 默认配置
    constexpr auto DEFCFG_SIZE = sizeof(CUIDefaultConfigure);
    static void* s_config_buf[DEFCFG_SIZE / sizeof(void*)];
    static_assert(sizeof(s_config_buf) == sizeof(CUIDefaultConfigure), "Orz");
    // 默认配置必须是平凡的析构(懒得析构)
    static_assert(std::is_trivially_destructible<CUIDefaultConfigure>::value, "bad");
    // 直接初始化
    const auto defcfg = reinterpret_cast<CUIDefaultConfigure*>(s_config_buf);
    detail::ctor_dtor<CUIDefaultConfigure>::create(defcfg);
    if (!cfg) cfg = defcfg;
    // 构造管理器
    {
        Result hr = { Result::RS_OK };
        detail::ctor_dtor<CUIManager>::create(&UIManager, cfg, flag, hr);
        if (!hr) return hr;
    }
    // 致命BUG处理
    this_()->InitUnExpHandler();
    // 初始化
    Result hr = { Result::RS_OK };
    // 初始化成员
    if (hr) {
        this_()->pm().time_capsules.reserve(32);
        if (!this_()->pm().time_capsules.is_ok())
            hr = { Result::RE_OUTOFMEMORY };
    }
    // 初始化默认字体
    if (hr) {
        hr = this_()->init_default_font(cfg);
    }
    // 获取实例句柄
    auto hInstance = ::GetModuleHandleW(nullptr);
    // 获取文本格式化器
    if (hr) {
        //hr = configure.CreateInterface(LongUI_IID_PV_ARGS(this_()->m_pTextFormatter));
        longui_debug_hr(hr, "Create this_()->m_pTextFormatter faild");
    }
    // 处理函数帮助器
    struct WndProcHelper {
        // 处理函数
        static LRESULT WINAPI CALL(
            HWND hwnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        ) noexcept {
            switch (message)
            {
            case WM_CREATE:
                UIManager.m_uGuiThreadId = ::GetCurrentThreadId();
                break;
#ifdef LUI_RAWINPUT
            case WM_INPUT:
                // XXX: 考虑加锁
                //CUIDataAutoLocker locker;
                UIManager.pm().km_input
                    .Update(reinterpret_cast<HRAWINPUT>(lParam));
                return 1;
#endif
            case WM_TIMER:
            {
                CUIDataAutoLocker locker;
                LongUI::OnTimer(wParam);
                return 0;
            }
            case WM_SETTINGCHANGE:
                UIManager.refresh_system_info();
                break;
            case WM_DISPLAYCHANGE:
                // 显示环境改变
                UIManager.refresh_display_frequency();
                UIManager.redirect_screen();
#ifndef NDEBUG
                LUIDebug(Hint) << "WM_DISPLAYCHANGE" << LongUI::endl;
            case WM_CLOSE:
                // 不能关闭该窗口
#endif
                break;
#if 0 // ADD SUPPORT
            case WM_QUERYENDSESSION:
            case WM_ENDSESSION:
                return false;
#endif
#if 0
            case CallLater::Later_DeleteControl:
            {
                // 延迟删除控件
                CUIDataAutoLocker locker;
                LongUI::DeleteControl(reinterpret_cast<UIControl*>(wParam));
                break;
            }
#endif
            case CallLater::Later_CallTimeCapsule:
            {
                // 调用时间胶囊
                const auto ptr = reinterpret_cast<CUIWaiter*>(wParam);
                union { LPARAM lp; float time; };
                lp = lParam;
                UIManager.update_time_capsule(time);
                ptr->Broadcast();
                break;
            }
            case CallLater::Later_Exit:
                // 连续调用Break直到退出, 防止因为消息嵌套而退出不了
                UIManager.BreakMsgLoop(wParam);
                ::PostMessageW(hwnd, CallLater::Later_Exit, wParam, 0);
                break;
            default:
                return ::DefWindowProcW(hwnd, message, wParam, lParam);
            }
            return 0;
        };
    };
    // 创建工具窗口
    if (hr) {
        // 注册窗口
        WNDCLASSEXW wcex;
        auto code = ::GetClassInfoExW(
            hInstance, Attribute::WindowClassNameT, &wcex
        );
        if (!code) {
            // 注册窗口类
            wcex = { 0 };
            wcex.cbSize = sizeof(WNDCLASSEXW);
            wcex.style = CS_NOCLOSE;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = sizeof(void*);
            wcex.hInstance = hInstance;
            wcex.hCursor = nullptr;
            wcex.hbrBackground = nullptr;
            wcex.lpszMenuName = nullptr;
            wcex.lpszClassName = Attribute::WindowClassNameT;
            wcex.hIcon = nullptr;
            wcex.lpfnWndProc = WndProcHelper::CALL;
            ::RegisterClassExW(&wcex);
        }
#ifdef NDEBUG
        constexpr int SIX = 0;
        constexpr int SIY = 0;
        constexpr int SIW = 0;
        constexpr int SIH = 0;
#else
        constexpr int SIX = 50;
        constexpr int SIY = 50;
        constexpr int SIW_ = 256;
        constexpr int SIH_ = 0;
        RECT rect = { SIX, SIY, SIX + SIW_, SIY + SIH_ };
        ::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        const int SIW = rect.right - rect.left;
        const int SIH = rect.bottom - rect.top;
#endif
        // 创建
        this_()->m_hToolWnd = ::CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            Attribute::WindowClassNameT, Attribute::WindowClassNameT,
            0, SIX, SIY, SIW, SIH, nullptr, nullptr, hInstance, nullptr
        );
        // 成功
        if (!this_()->m_hToolWnd) hr = { Result::RE_FAIL };
#ifndef NDEBUG
        // 调试
        else if (!(flag & ConfigureFlag::Flag_DbgDebugWindow))
            ::ShowWindow(this_()->m_hToolWnd, SW_SHOW);
#endif
    }

    // 初始化输入
    //if (hr) {
    //    hr = this_()->pm().km_input.Init(this_()->m_hToolWnd);
    //}

    // 创建帮助器
    /*if (hr) {
        hr = ::CoCreateInstance(
            CLSID_DragDropHelper,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(this_()->m_pDropTargetHelper)
        );
        longui_debug_hr(hr, "CoCreateInstance CLSID_DragDropHelper faild");
    }*/
    // 创建字体集
    if (hr) {
        // 获取脚本
        /*configure.CreateInterface(LongUI_IID_PV_ARGS(this_()->m_pFontCollection));
        // 失败获取系统字体集
        if (!this_()->m_pFontCollection) {
            hr = this_()->m_pDWriteFactory->GetSystemFontCollection(&this_()->m_pFontCollection);
            longui_debug_hr(hr, "this_()->m_pDWriteFactory->GetSystemFontCollection faild");
        }*/
    }
#if 0
    // 枚举字体
    if (hr && (this_()->flag & ConfigureFlag::Flag_DbgOutputFontFamily)) {
        auto count = this_()->m_pFontCollection->GetFontFamilyCount();
        UIManager << DL_Log << "Font found: " << long(count) << "\r\n";
        // 遍历所有字体
        for (auto i = 0u; i < count; ++i) {
            IDWriteFontFamily* family = nullptr;
            // 获取字体信息
            if (SUCCEEDED(this_()->m_pFontCollection->GetFontFamily(i, &family))) {
                IDWriteLocalizedStrings* string = nullptr;
                // 获取字体名称
                if (SUCCEEDED(family->GetFamilyNames(&string))) {
                    wchar_t buffer[LongUIStringBufferLength];
                    auto tc = string->GetCount();
                    UIManager << DLevel_Log << Formated("%4d[%d]: ", int(i), int(tc));
                    // 遍历所有字体名称
#if 0
                    for (auto j = 0u; j < 1u; j++) {
                        string->GetLocaleName(j, buffer, LongUIStringBufferLength);
                        UIManager << DLevel_Log << buffer << " => ";
                        // 有些语言在自己的机器上显示不了(比如韩语), 会出现bug略过不少东西, 就显示第一个了
                        string->GetString(j, buffer, LongUIStringBufferLength);
                        UIManager << DLevel_Log << buffer << "; ";
                    }
#else
                    // 显示第一个
                    string->GetLocaleName(0, buffer, LongUIStringBufferLength);
                    UIManager << DLevel_Log << buffer << " => ";
                    string->GetString(0, buffer, LongUIStringBufferLength);
                    UIManager << DLevel_Log << buffer << ";\r\n";
#endif
                }
                LongUI::SafeRelease(string);
            }
            LongUI::SafeRelease(family);
        }
        // 刷新
        UIManager << DL_Log << LongUI::endl;
    }
#endif
    // 添加控件
    if (hr) {
        ControlInfoList list;
        // 添加默认控件
        LongUI::AddDefaultControlInfo(list);
        // 添加自定义控件
        if (list.is_ok()) this_()->config->RegisterControl(list);
        // 注册控件
        if (list.is_ok()) for (auto info : list) {
            if (!this_()->pm().cclasses.insert({ info->element_name, info }).second) {
                hr = { Result::RE_OUTOFMEMORY };
                break;
            }
        }
        else hr = { Result::RE_OUTOFMEMORY };
    }
    // 第一次重建设备资源
    if (hr) {
        auto& pmm = this_()->pm();
        hr = this_()->recreate_device(this_()->config, this_()->flag);
    }
    // 检查当前路径
#ifndef NDEBUG
    constexpr uint32_t buflen = MAX_PATH * 2;
    wchar_t buffer[buflen]; buffer[0] = 0;
    ::GetCurrentDirectoryW(buflen, buffer);
    LUIDebug(Hint)
        << " Current Directory: "
        << buffer
        << LongUI::endl;
    // 创建调试窗口
    if (hr && (flag & ConfigureFlag::Flag_DbgDebugWindow)) {
        m_pDebugWindow = impl::create_debug_window();
        if (!m_pDebugWindow) hr = { Result::RE_OUTOFMEMORY };
    }
#endif // !NDEBUG
    // 开始渲染线程
    if (hr) {
        hr = this_()->config->BeginRenderThread();
    }
    return hr;
}

PCN_NOINLINE
/// <summary>
/// Deletes the later.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUIManager::DeleteLater(UIControl& ctrl) noexcept {
    // 检查删除性
    if (!LongUI::CheckControlDeleteLater(ctrl)) {
        this_()->m_uiLaterLocker.Lock();
        LongUI::MarkControlDeleteLater(ctrl);
        this_()->delete_later(ctrl);
        this_()->m_uiLaterLocker.Unlock();
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIManager" /> class.
/// </summary>
/// <param name="config">The configuration.</param>
/// <param name="cfgflag">The cfgflag.</param>
/// <param name="out">The out.</param>
LongUI::CUIManager::CUIManager(IUIConfigure* config,
    ConfigureFlag cfgflag,
    Result& out) noexcept :
ConfigKeeper{ config },
#ifndef NDEBUG
CUIDebug(config->GetSimpleLogFileName().c_str()),
#endif
CUIResMgr(config, out),
CUIWndMgr(out),
flag(cfgflag) {
#ifndef NDEBUG
    detail::ctor_dtor<CUIManagerDebug>::create(&DbgMgr());
#endif
    //config->AddRef();
    detail::ctor_dtor<Private>::create(&this_()->pm());
    // 更新系统信息
    this_()->refresh_system_info();
    
    //out = { Result::RE_FAIL };
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIManager"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIManager::~CUIManager() noexcept {
    this_()->pm().~Private();
#ifndef NDEBUG
    DbgMgr().~CUIManagerDebug();
#endif
}

/// <summary>
/// Uninitializes this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::Uninitialize() noexcept {
    // 强制退出
    this_()->m_uiTimeCapsuleWaiter.Broadcast();
#ifndef NDEBUG
    this_()->DataLock();
    m_pDebugWindow = nullptr;
    this_()->DataUnlock();
#endif
    // 取消窗口
    ::DestroyWindow(m_hToolWnd); m_hToolWnd = nullptr;
    // 结束掉渲染线程
    this_()->config->EndRenderThread();
    // 关闭所有窗口
    this_()->delete_all_window();
    // 手动调用析构函数
    this_()->~CUIManager();
    // 反初始化COM
    ::CoUninitialize();
    // 反初始化鼠标光标
    impl::uninit_cursor();
    // 反初始化DComp支持
    impl::uninit_dcomp_support();
    // 反初始化高DPI支持
    impl::uninit_high_dpi_support();
}

/// <summary>
/// Refreshes the system information.
/// </summary>
/// <returns></returns>
void LongUI::CUIManager::refresh_system_info() noexcept {
    UINT data;
    ::SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &data, 0);
    this_()->m_uWheelScrollLines = data;
    ::SystemParametersInfoA(SPI_GETWHEELSCROLLCHARS, 0, &data, 0);
    this_()->m_uWheelScrollChars = data;
}

PCN_NOINLINE
/// <summary>
/// Afters the create tc.
/// </summary>
/// <param name="tc">The tc.</param>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::CUIControlControl::after_create_tc(CUITimeCapsule* tc, 
    UIControl* ctrl) noexcept -> CUITimeCapsule* {
    // 有效指针
    if (tc) {
        auto&self = UIManager;
        // 连接前后节点
        self.m_oTailTimeCapsule.prev->next = tc;
        tc->prev = self.m_oTailTimeCapsule.prev;
        tc->next = static_cast<CUITimeCapsule*>(&self.m_oTailTimeCapsule);
        self.m_oTailTimeCapsule.prev = tc;
        // 设置指针
        if (ctrl) {
            tc->SetHoster(*ctrl);
            self.refresh_time_capsule(*ctrl, *tc);
        }
    }
    return tc;
}


/// <summary>
/// Called when [oom].
/// </summary>
/// <param name="retry_count">The retry count.</param>
/// <param name="try_alloc">The try alloc.</param>
/// <returns></returns>
auto LongUI::CUIManager::HandleOOM(size_t retry_count, size_t try_alloc) noexcept->CodeOOM {
    return this_()->config->HandleOOM(retry_count, try_alloc);
}

/// <summary>
/// Called when [error information lost].
/// </summary>
/// <param name="hr">The hr.</param>
/// <param name="occ">The occ.</param>
/// <returns></returns>
void LongUI::CUIManager::OnErrorInfoLost(Result hr, ErrorOccasion occ) noexcept {
    return this_()->config->OnErrorInfoLost(hr, occ);
}

PCN_NOINLINE
/// <summary>
/// Loads the data from URL.
/// </summary>
/// <param name="url_in_utf8">The URL in UTF8.</param>
/// <param name="buffer">The buffer.</param>
/// <returns></returns>
void LongUI::CUIManager::LoadDataFromUrl(
    U8View url_in_utf8,
    POD::Vector<uint8_t>& buffer) noexcept {
    const auto url_utf16 = CUIString::FromUtf8(url_in_utf8);
    // 读取文件
    if (CUIFile file{ url_utf16.c_str(), CUIFile::Flag_Read }) {
        const auto file_size = file.GetFilezize();
        // +1 针对字符串的优化处理
        buffer.reserve(file_size + 1);
        buffer.resize(file_size);
        // TODO: OOM处理
        if (buffer.is_ok()) file.Read(&buffer.front(), file_size);
        return;
    }
    // 查找失败, 利用config接口读取
    this_()->config->LoadDataFromUrl(url_in_utf8, url_utf16, buffer);
    // 载入失败
#ifndef NDEBUG
    if (buffer.empty())
        LUIDebug(Error)
            << "load file failed: "
            << url_utf16
            << endl;
#endif // !NDEBUG
}

#ifndef NDEBUG
/// <summary>
/// the endian runtime assert.
/// </summary>
/// <returns></returns>
void ui_endian_runtime_assert() noexcept {
    using namespace LongUI;
    enum : uint32_t {
        LITTLE_ENDIAN   = 0x03020100_ui32,
        BIG_ENDIAN      = 0x00010203_ui32,
        PDP_ENDIAN      = 0x01000302_ui32,
    };
    const union { unsigned char bytes[4]; uint32_t value; }
    host_order{ { 0, 1, 2, 3 } };
    const bool le = host_order.value == LITTLE_ENDIAN;
    const bool be = host_order.value == BIG_ENDIAN;
    const bool me = host_order.value == PDP_ENDIAN;
    assert(le == helper::is_little_endian::value);
    assert(be == helper::is_big_endian::value);
    assert(me == helper::is_pdp_endian::value);
}

#endif


/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIBlockingGuiOpAutoUnlocker::init() noexcept -> uint32_t {
    // 保险起见先Lock一下避免解锁掉渲染线程的DATA锁
    UIManager.DataLock();
    // 只能在Gui线程调用该方法
    assert(::GetCurrentThreadId() == UIManager.GetGuiThreadId());
    // 获取递归计数
    const auto counter = UIManager.DataRecursion();
    // 解除全部锁
    for (uint32_t i = 0; i != counter; ++i) UIManager.DataUnlock();
    // 返回递归计数
    return counter;
}

/// <summary>
/// Uninits this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIBlockingGuiOpAutoUnlocker::uninit(uint32_t counter) noexcept {
    // 只能在Gui线程调用该方法
    assert(::GetCurrentThreadId() == UIManager.GetGuiThreadId());
    // 至少有一次
    assert(counter > 0 && "bad counter");
    // 避免错误的0次以及保险起见的1次
    if (counter > 1) {
        // 减去之前保险起见的1次
        --counter;
        // 在全部上锁
        for (uint32_t i = 0; i != counter; ++i) UIManager.DataLock();
    }
}

// ----------------------------------------------------------------------------
//                          UI Window Manager
// ----------------------------------------------------------------------------

/// <summary>
/// Exits this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIWndMgr::exit() noexcept {
    UIManager.Exit();
}

/// <summary>
/// Determines whether [is quit on last window closed].
/// </summary>
/// <returns></returns>
bool LongUI::CUIWndMgr::is_quit_on_last_window_closed() noexcept {
    return !!(UIManager.flag & ConfigureFlag::Flag_QuitOnLastWindowClosed);
}
