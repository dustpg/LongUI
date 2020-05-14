#include <interface/ui_default_config.h>
#include <core/ui_manager.h>
#include <cassert>
#include <cstdlib>
// atomic
#include <atomic>


#ifndef NDEBUG
#include <core/ui_string.h>
/// <summary>
/// Gets the name of the simple log file.
/// </summary>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::GetSimpleLogFileName() noexcept -> CUIString {
    return{ u"simple.log" };
}
#endif

/// <summary>
/// Chooses the adapter.
/// </summary>
/// <param name="adapters">The adapters.</param>
/// <param name="length">The length.</param>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::ChooseAdapter(
    const GraphicsAdapterDesc adapters[], 
    const uint32_t length) noexcept -> uint32_t {
    adapters;
    return length;
}

/// <summary>
/// Loads the data from URL.
/// </summary>
/// <param name="url_utf8">The URL UTF8.</param>
/// <param name="buffer">The buffer.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::LoadDataFromUrl(
    U8View url_utf8,
    const CUIString& url_in_utf16,
    POD::Vector<uint8_t>& buffer) noexcept {
    
}


/// <summary>
/// Shows the error.
/// </summary>
/// <param name="hr">The hr.</param>
/// <param name="occ">The occ.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::OnErrorInfoLost(Result hr, ErrorOccasion occ) noexcept {
    assert(hr);
}

/// <summary>
/// Called when [oom].
/// </summary>
/// <param name="count">The count.</param>
/// <param name="size">The size.</param>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::HandleOOM(size_t count, size_t size) noexcept -> CodeOOM {
    return OOM_Ignore;
    assert(!"OOM");
    std::exit(-1);
    return OOM_NoReturn;
}

/// <summary>
/// Gets the name of the locale.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::GetLocaleName(char16_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept {
    name[0] = 0;
}


/// <summary>
/// Registers extra control.
/// </summary>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::RegisterControl(ControlInfoList& list) noexcept {

}


/// <summary>
/// Evaluations the specified script.
/// </summary>
/// <param name="script">The script.</param>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
bool LongUI::CUIDefaultConfigure::Evaluation(ScriptUI script, UIControl& ctrl) noexcept {
    assert(!"BAD CALL");
    return false;
}

/// <summary>
/// Allocs the script.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::AllocScript(U8View view) noexcept -> ScriptUI {
    return {};
}

/// <summary>
/// Frees the script.
/// </summary>
/// <param name="script">The script.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::FreeScript(ScriptUI script) noexcept {
    assert(script.script == nullptr);
}

/// <summary>
/// Evaluations the specified view.
/// </summary>
/// <param name="view">The view.</param>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::Evaluation(U8View view, CUIWindow& window) noexcept {
    assert(view.size());
}



/// <summary>
/// Finalizes the script.
/// </summary>
/// <param name="window">The window.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::FinalizeScript(CUIWindow& window) noexcept {
    assert(&window == &window);
}


// ui namespace
namespace LongUI { enum { DEBUG_SMALL = 32 }; }

#ifndef NDEBUG
extern "C" bool lui_debug_oom = false;
#endif

/// <summary>
/// malloc for normal space
/// </summary>
/// <param name="length">The length.</param>
/// <returns></returns>
void*LongUI::CUIDefaultConfigure::NormalAlloc(size_t length) noexcept {
    assert(length && "cannot alloc 0 byte at here");
#ifndef NDEBUG
    if (lui_debug_oom) return nullptr;
    //if (++reinterpret_cast<std::atomic<uint16_t>&>(m_u16Alloc) >= 150)
    //    return nullptr;
#endif
    //return nullptr;
    return std::malloc(length);
}

/// <summary>
/// free for normal space
/// </summary>
/// <param name="address">The address.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::NormalFree(void* address) noexcept {
    return std::free(address);
}


/// <summary>
/// realloc for normal space
/// </summary>
/// <param name="address">The address.</param>
/// <param name="length">The length.</param>
/// <returns></returns>
void*LongUI::CUIDefaultConfigure::NormalRealloc(void* address, size_t length) noexcept {
    assert((address || length) && "cannot call std::realloc(nullptr, 0)");

#ifndef NDEBUG
    if (length) {
        if (lui_debug_oom) return nullptr;
        //if (++reinterpret_cast<std::atomic<uint16_t>&>(m_u16Alloc) >= 300)
        //    return nullptr;
    }
#endif

    return std::realloc(address, length);
}


/// <summary>
/// malloc for small space
/// </summary>
/// <param name="length">The length.</param>
/// <returns></returns>
void*LongUI::CUIDefaultConfigure::SmallAlloc(size_t length) noexcept {
    assert(length && "cannot alloc 0 byte at here");
#ifndef NDEBUG
    if (lui_debug_oom) return nullptr;
    //if (++reinterpret_cast<std::atomic<uint16_t>&>(m_u16Alloc) >= 10)
    //    return nullptr;
#endif


#ifdef NDEBUG
    return std::malloc(length);
#else
    const auto ptr = std::malloc(length + DEBUG_SMALL);
    const auto data = reinterpret_cast<char*>(ptr);
    return data ? data + DEBUG_SMALL : nullptr;
#endif
}

/// <summary>
/// free for small space
/// </summary>
/// <param name="address">The address.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::SmallFree(void* address) noexcept {
#ifdef NDEBUG
    return std::free(address);
#else
    const auto data = reinterpret_cast<char*>(address);
    const auto ptr = data ? data - DEBUG_SMALL : nullptr;
    return std::free(ptr);
#endif
}


// ui
#include <core/ui_string.h>
#include <text/ui_ctl_arg.h>
#include <util/ui_unicode_cast.h>

#include <windows.h>
#include <process.h>


enum WinConst {
    WIN_DEFAULT_GUI_FONT = 17,
    WIN_WM_QUIT = 0x12, 
};

#ifdef NDEBUG
static_assert(WIN_WM_QUIT == WM_QUIT, "SAME!");
static_assert(WIN_DEFAULT_GUI_FONT == DEFAULT_GUI_FONT, "SAME!");
#endif

/// <summary>
/// Defaults the font argument.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::DefaultFontArg(FontArg& arg) noexcept {
    arg.size = 16.f;
    // 获取默认GUI字体句柄
    const auto font = ::GetStockObject(WIN_DEFAULT_GUI_FONT);
    constexpr size_t EX_NAME_LENGTH = 32;
    struct { LOGFONTW log; wchar_t ex[EX_NAME_LENGTH]; } buf;
    ::GetObjectW(font, sizeof(buf), &buf);
    // 字体大小
    if (buf.log.lfHeight) 
        arg.size = static_cast<float>(std::abs(buf.log.lfHeight));
    // 字体粗细
    arg.weight = static_cast<AttributeFontWeight>(buf.log.lfWeight);
    // 字体名称
    if (*buf.log.lfFaceName) {
        constexpr size_t buflen = 256;
        char buffer[buflen];
        const auto str = buf.log.lfFaceName;
        const auto len = Unicode::To<Unicode::UTF8>(buffer, buflen, str);
        arg.family = UIManager.GetUniqueText({ buffer, buffer + len });
    }
    // 斜体字
    arg.style = buf.log.lfItalic ? Style_Italic : Style_Normal;
}

/// <summary>
/// Exits the main loop
/// </summary>
/// <param name="code">The code.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::BreakMsgLoop(uintptr_t code) noexcept {
    //::PostQuitMessage(-1);
    ::PostMessageW(nullptr, WIN_WM_QUIT, code, 0);
}


/// <summary>
/// Begins the render thread.
/// </summary>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::BeginRenderThread() noexcept ->Result {
    auto& flag = reinterpret_cast<std::atomic<bool>&>(m_bExitFlag);
    // 退出flag
    flag.store(false, std::memory_order_relaxed);
    // 渲染线程
    const auto thr = ::_beginthread([](void* ptr) noexcept {
        const auto flag = reinterpret_cast<std::atomic_bool*>(ptr);
        while (!flag->load(std::memory_order_relaxed)) {
            UIManager.OneFrame();
            UIManager.WaitForVBlank();
        }
        ::_endthread();
    }, 0, &flag);
    m_hRenderThread = thr;
    // 检测错误
    Result hr = { Result::RS_OK };
    if (thr == uintptr_t(-1)) hr = { Result::RE_HANDLE };
    return hr;
}

/// <summary>
/// Mains the loop.
/// </summary>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::RecursionMsgLoop() noexcept ->uintptr_t {
    MSG msg = { };
    // 消息循环
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
    return msg.wParam;
}

/// <summary>
/// Ends the render thread.
/// </summary>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::EndRenderThread() noexcept {
    auto& flag = reinterpret_cast<std::atomic<bool>&>(m_bExitFlag);
    // 退出
    flag.store(true, std::memory_order_relaxed);
    // 等待设置
    const auto hthr = reinterpret_cast<HANDLE>(m_hRenderThread);
    constexpr uint32_t try_wait_time = 2333;
    // 等待线程退出
    if (::WaitForSingleObject(hthr, try_wait_time) != WAIT_OBJECT_0) {
        // TODO: 超时处理
        assert(!"OVER TIME");
    }
}
