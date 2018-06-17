// ui
#include <core/ui_object.h>
#include <core/ui_manager.h>
// private
#include "../private/ui_win10_stlye.h"
// c++
#include <cassert>

// ui namespace
namespace LongUI {
    // some constant
    enum : uint32_t {
        // basic animation duration(unit: ms)
        BASIC_ANIMATION_DURATION = 200,
    };
    // unused style
    using CUINativeStyleNow = CUINativeStyleWindows10;
    // impl
    namespace impl {
        // create native style renderer
        auto create_native_style_renderer() noexcept -> void* {
            return new(std::nothrow) CUINativeStyleNow;
        }
        // delete native style renderer
        void delete_native_style_renderer(void* ptr) noexcept {
            auto obj = static_cast<CUINativeStyleNow*>(ptr);
            delete obj;
        }
    }
}

/// <summary>
/// Gets the duration of the animation.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::NativeStyleDuration(const GetDurationArgs args) noexcept -> uint32_t {
    // TODO: 不同类型甚至不同状态动画时长都不一样
    // TODO: 让一直是0的放在一起

    // 分类
    switch (args.appearance)
    {
    case LongUI::Appearance_ListBox:
    case LongUI::Appearance_GroupBox:
    case LongUI::Appearance_TabPanels:
    case LongUI::Appearance_ProgressBarH:
    case LongUI::Appearance_ProgressBarV:
    case LongUI::Appearance_ProgressChunkH:
    case LongUI::Appearance_ProgressChunkV:
    case LongUI::Appearance_ScrollbarTrackH:
    case LongUI::Appearance_ScrollbarTrackV:
        return 0;
    case LongUI::Appearance_Tab:
        return BASIC_ANIMATION_DURATION * 2;
    default:
        return BASIC_ANIMATION_DURATION;
    }
}

PCN_NOINLINE
/// <summary>
/// Draws the native style.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::NativeStyleDraw(const NativeDrawArgs& args) noexcept {
    const auto ptr = UIManager.GetNativeRenderer();
    const auto style = static_cast<CUINativeStyleNow*>(ptr);
    style->DrawNative(args);
}

PCN_NOINLINE
/// <summary>
/// Initializes the native style.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="aa">The appearance.</param>
/// <returns></returns>
void LongUI::NativeStyleInit(UIControl& ctrl, AttributeAppearance aa) noexcept {
    const auto ptr = UIManager.GetNativeRenderer();
    const auto style = static_cast<CUINativeStyleNow*>(ptr);
    style->InitCtrl(ctrl, aa);
}
