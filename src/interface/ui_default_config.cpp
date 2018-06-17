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
    return{ L"simple.log" };
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
    const size_t length) noexcept -> size_t {
    return length;
    return length-1;
}

/// <summary>
/// Shows the error.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool LongUI::CUIDefaultConfigure::ShowError(
    const wchar_t* a, const wchar_t* b) noexcept {
    assert(!"error");
    return false;
}

/// <summary>
/// Gets the name of the locale.
/// </summary>
/// <param name="name">The name.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept {
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
/// Exits the main loop
/// </summary>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::Exit() noexcept {
    ::PostQuitMessage(0);
}

/// <summary>
/// Defualts the font argument.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::DefualtFontArg(FontArg& arg) noexcept {
    //arg.size = 16.f;
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
    arg.size = 20.f;
}


/// <summary>
/// Mains the loop.
/// </summary>
/// <returns></returns>
void LongUI::CUIDefaultConfigure::MainLoop() noexcept {
    // 退出flag
    std::atomic_bool flag{ false };
    // 渲染线程
    const auto thr = ::_beginthread([](void* ptr) noexcept {
        const auto flag = reinterpret_cast<std::atomic_bool*>(ptr);
        while (!flag->load(std::memory_order_relaxed)) {
            UIManager.OneFrame();
            UIManager.WaitForVBlank();
            //UIManager.WaitForVBlank();
        }
        ::_endthread();
    }, 0, &flag);
    // 创建失败
    if (thr == -1) return; MSG msg;
    // 消息循环
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
    // 退出
    flag.store(true, std::memory_order_relaxed);
    // 等待设置
    const auto hthr = reinterpret_cast<HANDLE>(thr);
    constexpr uint32_t try_wait_time = 2333;
    // 等待线程退出
    if (::WaitForSingleObject(hthr, try_wait_time) != WAIT_OBJECT_0) {
        // TODO: 超时处理
        assert(!"OVER TIME");
    }
}