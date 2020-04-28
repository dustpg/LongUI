// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <style/ui_native_style.h>
#include <graphics/ui_matrix3x2.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"

/// <summary>
/// Draws the selbg.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_selbg(const NativeDrawArgs& args) noexcept {
    // 正式渲染
    auto draw_this = [&args](float alpha, bool disabled) noexcept {
        auto& renderer = UIManager.Ref2DRenderer();
        auto color = ColorF::FromRGBA_CT<0x3399ffff_rgba>();
        if (disabled) color = ColorF::FromRGBA_CT<0xd6d6d6ff_rgba>();
        color.a = alpha;
        auto& brush = UIManager.RefCCBrush(color);
        auto rect = args.border;
        const auto width = rect.right - rect.left;
        //if (args.to.selected)
        rect.right = rect.left + width * alpha;
        //else rect.left = rect.right - width * alpha;
        renderer.FillRectangle(auto_cast(rect), &brush);
    };
    // 改变时
    if (args.to.selected || args.from.selected) {
        // 渲染
        auto get_alpha = [&]() noexcept {
            if (args.to.selected == args.from.selected) return 1.f;
            else return args.from.selected ? 1.f - args.progress : args.progress;
        };
        draw_this(get_alpha(), args.to.disabled);
    }
}

