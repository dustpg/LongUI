// ui
#include <core/ui_object.h>
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
// private
#include "../private/ui_win10_style.h"
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
auto LongUI::CUINativeStyle::GetDuration(const GetDurationArgs args) noexcept -> uint32_t {
    // HINT: 可以使用虚函数
    return static_cast<CUINativeStyleNow*>(this)->NativeStyleDuration(args);
}


/// <summary>
/// Natives the color of the fg.
/// </summary>
/// <param name="now">The now.</param>
/// <returns></returns>
auto LongUI::CUINativeStyle::GetFgColor(StyleState now) noexcept -> uint32_t {
    // HINT: 可以使用虚函数
    return static_cast<CUINativeStyleNow*>(this)->NativeFgColor(now);
}

/// <summary>
/// Draws the native style.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyle::DrawStyle(const NativeDrawArgs & args) noexcept {
    // HINT: 可以使用虚函数
    static_cast<CUINativeStyleNow*>(this)->DrawNative(args);
}

/// <summary>
/// Initializes the native style.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="aa">The appearance.</param>
/// <returns></returns>
void LongUI::CUINativeStyle::DrawFocus(const RectF& rect) noexcept {
#ifdef LUI_DRAW_FOCUS_RECT
    // HINT: 可以使用虚函数
    static_cast<CUINativeStyleNow*>(this)->FocusNative(rect);
#endif
}

/// <summary>
/// Initializes the native style.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="aa">The appearance.</param>
/// <returns></returns>
void LongUI::CUINativeStyle::InitStyle(UIControl & ctrl, AttributeAppearance aa) noexcept {
    // HINT: 可以使用虚函数
    static_cast<CUINativeStyleNow*>(this)->InitCtrl(ctrl, aa);
}
