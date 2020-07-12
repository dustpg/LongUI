// ui
#include <luiconf.h>
#include <core/ui_manager.h>
#include <graphics/ui_dcomp.h>
#include <graphics/ui_graphics_impl.h>
#include <debugger/ui_debug.h>
// c++
#include <cassert>
// window dcom
#define NOMINMAX
struct IDCompositionVisual;
#include <Windows.h>
#include <dcomp.h>


// longui namespace
namespace LongUI {
    // IDCompositionDevice "C37EA93A-E7AA-450D-B16F-9746CB0407F3"
    const GUID IID_IDCompositionDevice = {
        0xC37EA93A, 0xE7AA, 0x450D,{ 0xB1, 0x6F, 0x97, 0x46, 0xCB, 0x04, 0x07, 0xF3 }
    };
}


// longui::impl
namespace LongUI { namespace impl {
    // dcomp window buf
    struct dcomp_window { 
        // Direct Composition Device
        IDCompositionDevice*    dcomp_device;
        // Direct Composition Target
        IDCompositionTarget*    dcomp_target;
        // Direct Composition Visual
        IDCompositionVisual*    dcomp_visual;
    };
    // same!
    static_assert(sizeof(dcomp_window_buf) == sizeof(dcomp_window), "must be same");
    // dll name
    static constexpr const char* dcomp_support_dll = "Dcomp.dll";
    // GetDpiForMonitor
    static FARPROC ptr_DCompositionCreateDevice2 = nullptr;

}}

/// <summary>
/// Checks the dcomp support.
/// </summary>
/// <returns></returns>
bool LongUI::impl::check_dcomp_support() noexcept {
    return !!ptr_DCompositionCreateDevice2;
}

/// <summary>
/// Initializes the DCOM support.
/// </summary>
/// <returns></returns>
void LongUI::impl::init_dcomp_support() noexcept {
    // XXX: 使用编号
    const auto name = "DCompositionCreateDevice2";
#ifdef LUI_WIN10_ONLY
    const auto dll = ::LoadLibraryA(dcomp_support_dll);
    assert(dll && "support windows 10 only");
    ptr_DCompositionCreateDevice2 = ::GetProcAddress(dll, name);
    assert(ptr_DCompositionCreateDevice2 && "support windows 10 only");
#else
    if (const auto dll = ::LoadLibraryA(dcomp_support_dll))
        ptr_DCompositionCreateDevice2 = ::GetProcAddress(dll, name);
#endif
}

/// <summary>
/// Uninits the DCOM support.
/// </summary>
/// <returns></returns>
void LongUI::impl::uninit_dcomp_support() noexcept {
#ifdef LUI_WIN10_ONLY
    ::FreeLibrary(::GetModuleHandleA(dcomp_support_dll));
#else
    if (const auto dll = ::GetModuleHandleA(dcomp_support_dll)) {
        ::FreeLibrary(dll);
    }
#endif

}

/// <summary>
/// Initializes the dcomp.
/// </summary>
/// <param name="buf">The buf.</param>
/// <returns></returns>
void LongUI::impl::init_dcomp(dcomp_window_buf& buf) noexcept {
    auto& dcomp = reinterpret_cast<dcomp_window&>(buf);
    dcomp = { nullptr };
}

/// <summary>
/// Releases the dcomp.
/// </summary>
/// <param name="buf">The buf.</param>
/// <returns></returns>
void LongUI::impl::release_dcomp(dcomp_window_buf& buf) noexcept {
    auto& dcomp = reinterpret_cast<dcomp_window&>(buf);
    LongUI::SafeRelease(dcomp.dcomp_device);
    LongUI::SafeRelease(dcomp.dcomp_target);
    LongUI::SafeRelease(dcomp.dcomp_visual);
}

/// <summary>
/// Creates the dcomp.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="hwnd">The HWND.</param>
/// <param name="sc">The sc.</param>
/// <returns></returns>
auto LongUI::impl::create_dcomp(dcomp_window_buf& buf, HWND hwnd, I::Swapchan& sc) noexcept -> Result {
#ifndef LUI_WIN10_ONLY
    // 没有Dcomp支持
    if (!ptr_DCompositionCreateDevice2) return { Result::RE_NOINTERFACE };
#endif
    auto& dcomp = reinterpret_cast<dcomp_window&>(buf);
    assert(dcomp.dcomp_device == nullptr);
    assert(dcomp.dcomp_target == nullptr);
    assert(dcomp.dcomp_visual == nullptr);
    Result hr = { Result::RS_OK };
    // 创建直接组合(Direct Composition)设备
    if (SUCCEEDED(hr)) {
        // 获取
        ID2D1Device* d2d_device = nullptr;
        // 这条函数没有返回值
        UIManager.Ref2DRenderer().GetDevice(&d2d_device);
        assert(d2d_device && "???");
        // 调用Win8.1的DCompositionCreateDevice2
        union {
            HRESULT(WINAPI* dccd)(IUnknown*, REFIID, void**) noexcept;
            FARPROC call;
        };
        call = ptr_DCompositionCreateDevice2;
        const auto address = reinterpret_cast<void**>(&dcomp.dcomp_device);
        hr = { dccd(d2d_device, LongUI::IID_IDCompositionDevice, address) };
        d2d_device->Release();
        longui_debug_hr(hr, L"DCompositionCreateDevice faild");
    }
    // 创建直接组合(Direct Composition)目标
    if (SUCCEEDED(hr)) {
        hr = { dcomp.dcomp_device->CreateTargetForHwnd(
            hwnd, true, &dcomp.dcomp_target
        ) };
        longui_debug_hr(hr, L"DcompDevice->CreateTargetForHwnd faild");
    }
    // 创建直接组合(Direct Composition)视觉
    if (SUCCEEDED(hr)) {
        hr = { dcomp.dcomp_device->CreateVisual(&dcomp.dcomp_visual) };
        longui_debug_hr(hr, L"DcompDevice->CreateVisual faild");
    }
    // 设置当前交换链为视觉内容
    if (SUCCEEDED(hr)) {
        hr = { dcomp.dcomp_visual->SetContent(&sc) };
        longui_debug_hr(hr, L"DcompVisual->SetContent faild");
    }
    // 设置当前视觉为窗口目标
    if (SUCCEEDED(hr)) {
        hr = { dcomp.dcomp_target->SetRoot(dcomp.dcomp_visual) };
        longui_debug_hr(hr, L"DcompTarget->SetRoot faild");
    }
    // 向系统提交
    if (SUCCEEDED(hr)) {
        hr = { dcomp.dcomp_device->Commit() };
        longui_debug_hr(hr, L"DcompDevice->Commit faild");
    }
    return hr;
}

//class dcomp_device;