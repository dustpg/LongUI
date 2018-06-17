// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <control/ui_control.h>
#include <style/ui_native_style.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../../private/ui_win10_stlye.h"

/// <summary>
/// Draws the native.
/// </summary>
/// <param name="appearance">The appearance.</param>
/// <param name="state">The state.</param>
/// <param name="opacity">The opacity.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::DrawNative(const NativeDrawArgs& args) noexcept {
    const auto appearance = args.appearance;
    switch (appearance)
    {
        ColorF color;
        uint32_t index; ArrowDirection dir;
    default: case LongUI::Appearance_None: assert(!"error"); return;
    case LongUI::Appearance_Radio:
        return this->draw_radio(args);
    case LongUI::Appearance_Button:
        return this->draw_button(args);
    case LongUI::Appearance_Resizer:
        return this->draw_rasizer(args);
    case LongUI::Appearance_CheckBox:
        return this->draw_checkbox(args);
    case LongUI::Appearance_ScrollBarButtonUp:
    case LongUI::Appearance_ScrollBarButtonRight:
    case LongUI::Appearance_ScrollBarButtonDown:
    case LongUI::Appearance_ScrollBarButtonLeft:
        index = appearance - Appearance_ScrollBarButtonUp;
        dir = static_cast<ArrowDirection>(index);
        return this->draw_arrow_button(args, dir);
    case LongUI::Appearance_ScrollbarThumbH:
    case LongUI::Appearance_ScrollbarThumbV:
        index = appearance - Appearance_ScrollbarThumbH;
        return this->draw_sb_thumb(args, index & 1);
    case LongUI::Appearance_ScrollbarTrackH:
    case LongUI::Appearance_ScrollbarTrackV:
        index = appearance - Appearance_ScrollbarTrackH;
        return this->draw_sb_track(args, index & 1);
    case LongUI::Appearance_ScaleH:
    case LongUI::Appearance_ScaleV:
        index = appearance - Appearance_ScaleH;
        return this->draw_slider_track(args, index & 1);
    case LongUI::Appearance_ScaleThumbH:
    case LongUI::Appearance_ScaleThumbV:
        return this->draw_slider_thumb(args);
    case LongUI::Appearance_ProgressBarH:
    case LongUI::Appearance_ProgressBarV:
        return this->draw_progress_bar(args);
    case LongUI::Appearance_ProgressChunkH:
    case LongUI::Appearance_ProgressChunkV:
        index = appearance - Appearance_ProgressChunkH;
        return this->draw_progress_chunk(args, index & 1);
    case LongUI::Appearance_ListBox:
    case LongUI::Appearance_GroupBox:
    case LongUI::Appearance_TabPanels:
        return this->draw_group_box(args);
    case LongUI::Appearance_ListItem:
        return this->draw_list_item(args);
    case LongUI::Appearance_TreeHeaderCell:
        return this->draw_tree_header_cell(args);
    case LongUI::Appearance_TreeTwisty:
        return this->draw_tree_twisty(args);
    case LongUI::Appearance_DropDownMarker:
        //ColorF::FromRGBA_RT(color, { 
        //    args.to.disabled ? 0x606060ff_rgba : 0x3d3d3dff_rgba
        //});
        ColorF::FromRGBA_RT(color, { 0x3d3d3dff_rgba });
        return this->draw_arrow_only(args.border, color, Direction_Down);
    case LongUI::Appearance_MenuItem:
        return this->draw_selbg(args);
    case LongUI::Appearance_Tab:
        return this->draw_tab(args);
    case LongUI::Appearance_TextField:
        return this->draw_textfield(args);
    }
}

#ifdef LUI_DRAW_ARROW_IN_MESH
namespace LongUI {
    // win10 arrow mesh data
    static const TriangleF WIN10_ARROW_MESH[] = {
        {{-3.5, 0.0 },{ 0.0,-0.5 },{ 0.0,-3.5 }},
        {{-3.5, 0.0 },{-3.5, 3.0 },{ 0.0,-0.5 }},
        {{ 0.0,-0.5 },{ 3.5, 0.0 },{ 0.0,-3.5 }},
        {{ 0.0,-0.5 },{ 3.5, 3.0 },{ 3.5, 0.0 }},
    };
}
#endif

/// <summary>
/// Initializes a new instance of the <see cref="CUINativeStyleWindows10"/> class.
/// </summary>
LongUI::CUINativeStyleWindows10::CUINativeStyleWindows10() noexcept {
#ifdef LUI_DRAW_ARROW_IN_MESH
    // 创建箭头网格
    constexpr uint32_t tri_count = sizeof(WIN10_ARROW_MESH)
        / sizeof(WIN10_ARROW_MESH[0]);
    Point2F s[] = {
        {-3.5, 0.0 },{ 0.0,-0.5 },{ 0.0,-3.5 },
        {-3.5, 0.0 },{-3.5, 3.0 },{ 0.0,-0.5 },
        { 0.0,-0.5 },{ 3.5, 0.0 },{ 0.0,-3.5 },
        { 0.0,-0.5 },{ 3.5, 3.0 },{ 3.5, 0.0 },
    };
    for (auto&x : s) x.x += 0.5f;

    auto hr = LongUI::CreateMesh(m_pMesh, (TriangleF*)s, tri_count);
    // TODO: 错误处理
    assert(hr);
#else
    constexpr float WIDTH = ARRAW_WDITH;
    constexpr uint32_t ptsc = 3;
    Point2F pts[ptsc];
    pts[0] = { -WIDTH, 0.5f * WIDTH };
    pts[1] = { 0.f, -0.5f * WIDTH };
    pts[2] = { WIDTH, 0.5f * WIDTH };
    CUIGeometry::CreateFromPoints(m_geoArrow, pts, ptsc);
#endif
}


/// <summary>
/// Releases all resources.
/// </summary>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::release_all_resources() noexcept {
    // 释放设备相关资源
    this->release_dd_resources();
    // 释放设备无关资源
}

/// <summary>
/// Releases the dd resources.
/// </summary>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::release_dd_resources() noexcept {
#ifdef LUI_DRAW_ARROW_IN_MESH
    auto ptr = m_pMesh; m_pMesh = nullptr;
    LongUI::SafeReleaseMesh(ptr);
#else

#endif
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUINativeStyleWindows10::Recreate() noexcept -> Result {
    this->release_dd_resources();
    Result hr = { Result::RS_OK };
    auto& renderer = UIManager.Ref2DRenderer();
    // HOVER 笔刷
    if (hr) {

    }
    return hr;
}

/// <summary>
/// Initializes the control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::InitCtrl(
    UIControl& ctrl, AttributeAppearance type) noexcept{
    // NONE场合特殊处理
    if (type == Appearance_None) return;
    // 分类讨论
    switch (type)
    {
    case LongUI::Appearance_Radio:
    case LongUI::Appearance_CheckBox:
        // 固定大小
        ctrl.SetFixedSize({ CHECKBOX_ICON_SIZE, CHECKBOX_ICON_SIZE });
        break;
    case LongUI::Appearance_Resizer:
        ctrl.SetFixedSize({ ARROW_SIZE, ARROW_SIZE });
        break;
    case LongUI::Appearance_MenuItem:
        ctrl.SetStyleMinSize({ 0, MENUITEM_HEIGHT });
        break;
    case LongUI::Appearance_TreeTwisty:
    case LongUI::Appearance_DropDownMarker:
    case LongUI::Appearance_ScrollBarButtonUp:
    case LongUI::Appearance_ScrollBarButtonRight:
    case LongUI::Appearance_ScrollBarButtonDown:
    case LongUI::Appearance_ScrollBarButtonLeft:
        ctrl.SetFixedSize({ ARROW_SIZE, ARROW_SIZE });
        break;
    case LongUI::Appearance_ScrollbarThumbH:
    case LongUI::Appearance_ScrollbarTrackH:
        ctrl.SetStyleMinSize({ 0, ARROW_SIZE });
        break;
    case LongUI::Appearance_ScrollbarThumbV:
    case LongUI::Appearance_ScrollbarTrackV:
        ctrl.SetStyleMinSize({ ARROW_SIZE, 0 });
        break;
    case LongUI::Appearance_ScaleH:
    case LongUI::Appearance_ProgressBarH:
    //case LongUI::Appearance_ProgressChunkH:
        ctrl.SetStyleMinSize({ 0, SLIDER_THUMB_HH });
        break;
    case LongUI::Appearance_ScaleV:
    case LongUI::Appearance_ProgressBarV:
    //case LongUI::Appearance_ProgressChunkV:
        ctrl.SetStyleMinSize({ SLIDER_THUMB_HH, 0 });
        break;
    case LongUI::Appearance_ScaleThumbH:
        ctrl.SetFixedSize({ SLIDER_THUMB_HW, SLIDER_THUMB_HH });
        break;
    case LongUI::Appearance_ScaleThumbV:
        ctrl.SetFixedSize({ SLIDER_THUMB_HH, SLIDER_THUMB_HW });
        break;
    }
}