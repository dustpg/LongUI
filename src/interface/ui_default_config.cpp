#include <interface/ui_default_config.h>
#include <core/ui_manager.h>
#include <cassert>
#include <cstdlib>


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
/// Gets the configure flag.
/// </summary>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::GetConfigureFlag() noexcept ->ConfigureFlag {
    return Flag_None
#ifndef NDEBUG
        | Flag_RenderByCPU
#endif
        | Flag_OutputDebugString 
        | Flag_QuitOnLastWindowClosed
        //| Flag_DbgOutputTimeTook 
        //| Flag_DbgDrawDirtyRect
        | Flag_DbgDrawTextCell
        | Flag_DbgDebugWindow
        ;
}

/// <summary>
/// Chooses the adapter.
/// </summary>
/// <param name="adapters">The adapters.</param>
/// <param name="length">The length.</param>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::ChooseAdapter(
    const GraphicsAdapterDesc /*adapters*/[], 
    const uint32_t length) noexcept -> uint32_t {
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
/// <param name="info">The information.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::OnError(ErrorInfo info) noexcept {
    assert(!"error");
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



// ui namespace
namespace LongUI { enum { DEBUG_SMALL = 32 }; }

/// <summary>
/// malloc for normal space
/// </summary>
/// <param name="length">The length.</param>
/// <returns></returns>
void*LongUI::CUIDefaultConfigure::NormalAlloc(size_t length) noexcept {
    assert(length && "cannot alloc 0 byte at here");
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
    return std::realloc(address, length);
}

/// <summary>
/// malloc for small space
/// </summary>
/// <param name="length">The length.</param>
/// <returns></returns>
void*LongUI::CUIDefaultConfigure::SmallAlloc(size_t length) noexcept {
    assert(length && "cannot alloc 0 byte at here");
#ifdef NDEBUG
    return std::malloc(length);
#else
    const auto ptr = reinterpret_cast<char*>(
        std::malloc(length + DEBUG_SMALL));
    return ptr ? DEBUG_SMALL + ptr : nullptr;
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
    return std::free(
        address 
        ? reinterpret_cast<char*>(address) - DEBUG_SMALL
        : nullptr);
#endif
}


// atomic
#include <atomic>
// ui
#include <core/ui_string.h>
#include <text/ui_ctl_arg.h>
#include <util/ui_unicode_cast.h>

#include <windows.h>
#include <process.h>

/// <summary>
/// Defaults the font argument.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::DefaultFontArg(FontArg& arg) noexcept {
    arg.size = 16.f;
    // 获取默认GUI字体句柄
    const auto font = ::GetStockObject(DEFAULT_GUI_FONT);
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
    ::PostMessageW(nullptr, WM_QUIT, code, 0);
}


static std::atomic_bool s_flag{ false };

/// <summary>
/// Begins the render thread.
/// </summary>
/// <returns></returns>
auto LongUI::CUIDefaultConfigure::BeginRenderThread() noexcept ->Result {
    // 退出flag
    s_flag.store(false, std::memory_order_relaxed);
    //std::atomic_bool flag{ false };
    // 渲染线程
    const auto thr = ::_beginthread([](void* ptr) noexcept {
        const auto flag = reinterpret_cast<std::atomic_bool*>(ptr);
        while (!flag->load(std::memory_order_relaxed)) {
            UIManager.OneFrame();
            UIManager.WaitForVBlank();
            //UIManager.WaitForVBlank();
        }
        ::_endthread();
    }, 0, &s_flag);
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
    // 退出
    s_flag.store(true, std::memory_order_relaxed);
    // 等待设置
    const auto hthr = reinterpret_cast<HANDLE>(m_hRenderThread);
    constexpr uint32_t try_wait_time = 2333;
    // 等待线程退出
    if (::WaitForSingleObject(hthr, try_wait_time) != WAIT_OBJECT_0) {
        // TODO: 超时处理
        assert(!"OVER TIME");
    }
}
