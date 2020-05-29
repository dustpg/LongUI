// ui
#include <core/ui_object.h>
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
// private
#include "../private/ui_win10_stlye.h"
// c++
#include <cassert>

// ui namespace
namespace LongUI {
    // unused style
    using CUINativeStyleNow = CUINativeStyleWindows10;
    // impl
    namespace impl {
        // create native style renderer
        auto create_native_style_renderer(void* ptr, size_t len) noexcept -> void* {
            static_assert(alignof(CUINativeStyleNow) <= alignof(void*), "ERROR");
            if (len < sizeof(CUINativeStyleNow)) {
                assert(!"TOO SMALL");
                return nullptr;
            }
            Result hr = { Result::RS_OK };
            detail::ctor_dtor<CUINativeStyleNow>::create(ptr, luiref hr);
            return hr ? ptr : nullptr;
        }
        // recreate native style renderer
        auto recreate_native_style_renderer(void* ptr) noexcept ->Result {
            assert(ptr && "recreate?");
            const auto obj = static_cast<CUINativeStyleNow*>(ptr);
            return obj->Recreate();
        }
        // delete native style renderer
        void delete_native_style_renderer(void* ptr) noexcept {
            const auto obj = static_cast<CUINativeStyleNow*>(ptr);
            obj->~CUINativeStyleNow();
        }
    }
}

/// <summary>
/// Gets the duration of the animation.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::NativeStyleDuration(const GetDurationArgs args) noexcept -> uint32_t {
    const auto ptr = UIManager.GetNativeRenderer();
    const auto style = static_cast<CUINativeStyleNow*>(ptr);
    return style->NativeStyleDuration(args);
}

/// <summary>
/// Natives the color of the fg.
/// </summary>
/// <param name="now">The now.</param>
/// <returns></returns>
auto LongUI::NativeFgColor(StyleState now) noexcept -> uint32_t {
    const auto ptr = UIManager.GetNativeRenderer();
    const auto style = static_cast<CUINativeStyleNow*>(ptr);
    return style->NativeFgColor(now);
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

/// <summary>
/// Draws the native style focus rect.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::NativeStyleFocus(const RectF& rect) noexcept {
#ifdef LUI_DRAW_FOCUS_RECT
    const auto ptr = UIManager.GetNativeRenderer();
    const auto style = static_cast<CUINativeStyleNow*>(ptr);
    style->FocusNative(rect);
#endif
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
