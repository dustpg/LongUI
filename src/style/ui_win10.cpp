// ui
#include <core/ui_manager.h>
#include <core/ui_color_list.h>
#include <control/ui_control.h>
#include <graphics/ui_matrix3x2.h>
#include <style/ui_native_style.h>
#include <graphics/ui_graphics_impl.h>
// private
#include "../private/ui_win10_stlye.h"


/// <summary>
/// get native fg color
/// </summary>
/// <param name="now"></param>
/// <returns></returns>
auto LongUI::CUINativeStyleWindows10::NativeFgColor(StyleState now) noexcept -> uint32_t {
    return now & State_Disabled ? 0x838383ff_rgba : 0x000000ff_rgba;
}

/// <summary>
/// get native duration
/// </summary>
/// <param name="args"></param>
/// <returns></returns>
auto LongUI::CUINativeStyleWindows10::NativeStyleDuration(const GetDurationArgs args) noexcept -> uint32_t {
    constexpr uint32_t BASIC_ANIMATION_DURATION = 200;
    // TODO: 不同类型甚至不同状态动画时长都不一样
    // TODO: 让一直是0的放在一起
    // 分类
    switch (args.appearance)
    {
    case LongUI::Appearance_ListBox:
    case LongUI::Appearance_Caption:
    case LongUI::Appearance_GroupBox:
    case LongUI::Appearance_StatusBar:
    case LongUI::Appearance_TabPanels:
    case LongUI::Appearance_ProgressBarH:
    case LongUI::Appearance_ProgressBarV:
    case LongUI::Appearance_StatusBarPanel:
    case LongUI::Appearance_ProgressChunkH:
    case LongUI::Appearance_ProgressChunkV:
    case LongUI::Appearance_TreeRowModeCell:
    case LongUI::Appearance_CheckBoxContainer:
    case LongUI::Appearance_ScrollbarTrackH:
    case LongUI::Appearance_ScrollbarTrackV:
    case LongUI::Appearance_DropDownMarker:
    case LongUI::Appearance_MenuSeparator:
    case LongUI::Appearance_MenuArrow:
    case LongUI::Appearance_Separator:
    case LongUI::Appearance_Resizer:
        return 0;
    case LongUI::Appearance_Tab:
        return BASIC_ANIMATION_DURATION * 2;
    default:
        return BASIC_ANIMATION_DURATION;
    }
}

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
        uint32_t index; ArrowDirection dir;
    default: case LongUI::Appearance_None: assert(!"error"); 
    case Appearance_CheckBoxContainer:
    case Appearance_TreeRowModeCell:
        return;
    case LongUI::Appearance_Radio:
    case LongUI::Appearance_MenuRadio:
        return this->draw_radio(args);
    case LongUI::Appearance_Button:
        return this->draw_button(args);
    case LongUI::Appearance_Resizer:
        return this->draw_rasizer(args);
    case LongUI::Appearance_CheckBox:
    case LongUI::Appearance_MenuCheckBox:
        return this->draw_checkbox(args);
    case LongUI::Appearance_ToolBarButton:
        return this->draw_tb_button(args);
    case LongUI::Appearance_MenuSeparator:
        return this->draw_menu_separator(args.border);
    case LongUI::Appearance_StatusBar:
        return this->draw_status_bar(args.border);
    case LongUI::Appearance_Separator:
    case LongUI::Appearance_StatusBarPanel:
        return this->draw_status_bar_panel(args.border);
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
    case LongUI::Appearance_Caption:
        return this->draw_caption(args.border);
    case LongUI::Appearance_ListItem:
        return this->draw_list_item(args);
    case LongUI::Appearance_TreeHeaderCell:
        return this->draw_tree_header_cell(args);
    case LongUI::Appearance_TreeTwisty:
        return this->draw_tree_twisty(args);
    case LongUI::Appearance_DropDownMarker:
        return this->draw_drop_mark(args);
    case LongUI::Appearance_MenuArrow:
        return this->draw_menu_arrow(args);
    case LongUI::Appearance_MenuItem:
        return this->draw_selbg(args);
    case LongUI::Appearance_Tab:
        return this->draw_tab(args);
    case LongUI::Appearance_TextField:
        return this->draw_textfield(args);
    }
}


namespace LongUI {
#ifdef LUI_DRAW_ARROW_IN_MESH
    // win10 arrow mesh data
    static const TriangleF WIN10_ARROW_MESH[] = {
        {{-3.5, 0.0 },{ 0.0,-0.5 },{ 0.0,-3.5 }},
        {{-3.5, 0.0 },{-3.5, 3.0 },{ 0.0,-0.5 }},
        {{ 0.0,-0.5 },{ 3.5, 0.0 },{ 0.0,-3.5 }},
        {{ 0.0,-0.5 },{ 3.5, 3.0 },{ 3.5, 0.0 }},
    };
#endif
    // I::StrokeStyle
    namespace I { struct PCN_NOVTABLE StrokeStyle : ID2D1StrokeStyle1 {}; }
}

/// <summary>
/// draw native style focus rect
/// </summary>
/// <param name="rect"></param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::FocusNative(const RectF& rect) noexcept {
    auto area = rect;
    area.left++; area.top++; area.right--; area.bottom--;
    // TODO: 虚线优化?
    auto& renderer = UIManager.Ref2DRenderer();
    const auto color = ColorF::FromRGBA_CT<0x111111FF_rgba>();
    auto& bursh = UIManager.RefCCBrush(color);
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.DrawRectangle(auto_cast(area), &bursh, 1.f, m_pStrokeStyle);
    //renderer.FillRectangle(auto_cast(rect), &bursh);
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}


/// <summary>
/// Initializes a new instance of the <see cref="CUINativeStyleWindows10"/> class.
/// </summary>
/// <param name="hr"></param>
/// <returns></returns>
LongUI::CUINativeStyleWindows10::CUINativeStyleWindows10(Result& hr) noexcept {
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
    hr = LongUI::CreateMesh(m_pMesh, (TriangleF*)s, tri_count);
    if (!hr) return this->release_dd_resources();
#else
    constexpr float WIDTH = ARRAW_WDITH;
    constexpr uint32_t ptsc = 3;
    Point2F pts[ptsc];
    pts[0] = { -WIDTH, 0.5f * WIDTH };
    pts[1] = { 0.f, -0.5f * WIDTH };
    pts[2] = { WIDTH, 0.5f * WIDTH };
    CUIGeometry::CreateFromPoints(m_geoArrow, pts, ptsc);
#endif
    auto& factory = reinterpret_cast<ID2D1Factory1&>(UIManager.Ref2DFactory());
    hr.code = factory.CreateStrokeStyle(
        D2D1::StrokeStyleProperties1(
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_FLAT,
            D2D1_LINE_JOIN_MITER,
            10.0f,
            D2D1_DASH_STYLE_DASH,
            0.0f,
            D2D1_STROKE_TRANSFORM_TYPE_HAIRLINE
        ), nullptr, 0,
        reinterpret_cast<ID2D1StrokeStyle1**>(&m_pStrokeStyle)
    );
}


/// <summary>
/// Releases all resources.
/// </summary>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::release_all_resources() noexcept {
    // 释放设备相关资源
    this->release_dd_resources();
    // 释放设备无关资源
    this->release_di_resources();
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
/// Releases the di resources.
/// </summary>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::release_di_resources() noexcept {
    LongUI::SafeRelease(m_pStrokeStyle);
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
        // 包含文字, 不适合设置
        //ctrl.SetStyleMinSize({ 0, MENUITEM_HEIGHT });
        break;
    case LongUI::Appearance_MenuSeparator:
        ctrl.SetStyleMinSize({ -1, MENUSEPARATOR_HEIGHT });
        break;
    //case LongUI::Appearance_StatusBar:
    //    ctrl.SetStyleMinSize({ -1, -1 });
    //    break;
    //case LongUI::Appearance_StatusBarPanel:
    //    ctrl.SetStyleMinSize({ 5, 5 });
    //    break;
    case LongUI::Appearance_MenuArrow:
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




/// <summary>
/// Draws the arrow button.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="dir">The dir.</param>
/// <param name="back">if set to <c>true</c> [back].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_arrow_button(
    const NativeDrawArgs& args,
    ArrowDirection dir) noexcept {
    // 获取颜色
    ColorF bcolor1, fcolor1, bcolor2, fcolor2;
    self::get_arrow_btn_color(args.from, bcolor1, fcolor1);
    self::get_arrow_btn_color(args.to, bcolor2, fcolor2);
    const auto bcolor = LongUI::Mix(bcolor1, bcolor2, args.progress);
    const auto fcolor = LongUI::Mix(fcolor1, fcolor2, args.progress);
    // 背景
    auto rect = args.border;
    // 左右方向缩上下
    if (uint32_t(dir) & 1) { rect.top += 1.f; rect.bottom -= 1.f; }
    // 上下方向缩左右
    else { rect.left += 1.f; rect.right -= 1.f; }
    // 渲染
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect), &UIManager.RefCCBrush(bcolor)
    );
    // 前景
    assert(dir < DIRECTION_COUNT && "out of range");
    this->draw_arrow_only(args.border, fcolor, float(dir) * 90.f);
}


PCN_NOINLINE
/// <summary>
/// Draws the arrow only.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="angle">The angle.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_arrow_only(const RectF& rect,
    const ColorF& color,
    float angle) noexcept {
#ifdef LUI_DRAW_ARROW_IN_MESH
    if (!m_pMesh) return;
    // 计算旋转角度
    //const float angle = float(dir) * 90.f;
    Point2F center;
    center.x = (rect.left + rect.right) * 0.5f;
    center.y = (rect.top + rect.bottom) * 0.5f;
    const auto transfrom =
        Matrix::Matrix3x2F::Translation(center.x, center.y)
        //* Matrix::Matrix3x2F::Scale({1.25,1.25}, center)
        * Matrix::Matrix3x2F::Rotation(angle, center);
    // 渲染箭头
    LongUI::FillMesh(
        *m_pMesh,
        UIManager.Ref2DRenderer(),
        UIManager.RefCCBrush(ColorF::FromRGBA_CT<RGBA_Black>()),
        transfrom
    );
#else
    // 计算旋转角度
    //const float angle = float(dir) * 90.f;
    Point2F center;
    center.x = (rect.left + rect.right) * 0.5f;
    center.y = (rect.top + rect.bottom) * 0.5f;
    const auto transfrom =
        Matrix::Matrix3x2F::Translation(center.x, center.y)
        //* Matrix::Matrix3x2F::Scale({1.25,1.25}, center)
        * Matrix::Matrix3x2F::Rotation(angle, center);
    auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    m_geoArrow.Draw(renderer, UIManager.RefCCBrush(color), transfrom, 2.f);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#endif
}


/// <summary>
/// Draws the drop mark.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_drop_mark(
    const NativeDrawArgs & args) noexcept {
    ColorF color;
    //ColorF::FromRGBA_RT(color, { 
    //    args.to.disabled ? 0x606060ff_rgba : 0x3d3d3dff_rgba
    //});
    ColorF::FromRGBA_RT(color, { 0x3d3d3dff_rgba });
    constexpr float angle0 = Direction_Down * 90.f;
    constexpr float angle1 = Direction_Up * 90.f;
    //float angle = args.to.active ? angle1 : angle0;
    // 切换ACTIVE
    //if (args.to.active != args.from.active) {

    //}
    return this->draw_arrow_only(args.border, color, angle0);
}

/// <summary>
/// Draws the menu arrow.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_menu_arrow(
    const NativeDrawArgs & args) noexcept {
    const auto color = ColorF::FromRGBA_CT<0x3d3d3dff_rgba>();
    constexpr float angle = Direction_Right * 90.f;
    return this->draw_arrow_only(args.border, color, angle);
}

/// <summary>
/// Gets the color of the arrow BTN.
/// </summary>
/// <param name="state">The state.</param>
/// <param name="bc">The back-color.</param>
/// <param name="fc">The fore-color.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::get_arrow_btn_color(
    StyleState state, ColorF & bc, ColorF & fc) noexcept {
    // 颜色初始化
    constexpr auto hover_fc = 0x000000ff_rgba;
    constexpr auto hover_bc = 0xdadadaff_rgba;

    constexpr auto active_fc = 0xffffffff_rgba;
    constexpr auto active_bc = 0x606060ff_rgba;

    constexpr auto normal_fc = 0x606060ff_rgba;
    constexpr auto normal_bc = 0xf0f0f0ff_rgba;

    constexpr auto disabled_fc = 0x606060ff_rgba;
    constexpr auto disabled_bc = 0xf0f0f0ff_rgba;
    // 使用数据
    uint32_t fore_color, back_color;
    // 禁用
    if (state & State_Disabled) { fore_color = disabled_fc; back_color = disabled_bc; }
    // 按下
    else if (state & State_Active) { fore_color = active_fc; back_color = active_bc; }
    // 悬浮
    else if (state & State_Hover) { fore_color = hover_fc; back_color = hover_bc; }
    // 普通
    else { fore_color = normal_fc; back_color = normal_bc; }
    // 颜色
    ColorF::FromRGBA_RT(fc, { fore_color });
    ColorF::FromRGBA_RT(bc, { back_color });
}



/// <summary>
/// Draws the thumb.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_sb_thumb(
    const NativeDrawArgs& args, bool vertical) noexcept {
    // 背景
    auto rect = args.border;
    // 左右方向缩上下
    if (vertical) { rect.left += 1.f; rect.right -= 1.f; }
    // 上下方向缩左右
    else { rect.top += 1.f; rect.bottom -= 1.f; }
    // 获取颜色
    auto get_color = [](StyleState state, ColorF& color) noexcept {
        uint32_t code;
        if (state & (State_Active | State_Hover)) code = 0xa6a6a6ff_rgba;
        else code = 0xcdcdcdff_rgba;
        ColorF::FromRGBA_RT(color, { code });
    };
    ColorF from, to; get_color(args.from, from); get_color(args.to, to);
    const auto color = LongUI::Mix(from, to, args.progress);
    // 渲染
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect),
        &UIManager.RefCCBrush(color)
    );
}


/// <summary>
/// Draws the sb track.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_sb_track(
    const NativeDrawArgs & args, bool vertical) noexcept {
    // 背景颜色
    ColorF color = ColorF::FromRGBA_CT<0xF0F0F0FF_rgba>();
    // 渲染
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(args.border),
        &UIManager.RefCCBrush(color)
    );
}


/// <summary>
/// Draws the rasizer.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_rasizer(const NativeDrawArgs& args) noexcept {
    // 渲染
    auto rect = args.border;
    auto& renderer = UIManager.Ref2DRenderer();
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    ColorF color = ColorF::FromRGBA_CT<0xF0F0F0FF_rgba>();
    renderer.FillRectangle(auto_cast(rect), &UIManager.RefCCBrush(color));
    // 彩蛋: 会在这里画一个非常浅的L
    color = ColorF::FromRGBA_CT<0xD0D0D0FF_rgba>();
    auto& brush = UIManager.RefCCBrush(color);
    Point2F points[3];
    points[0].x = rect.left + (rect.right - rect.left) * 0.35f;
    points[0].y = rect.top + (rect.bottom - rect.top) * 0.3f;
    points[1].x = rect.left + (rect.right - rect.left) * 0.35f;
    points[1].y = rect.top + (rect.bottom - rect.top) * 0.7f;
    points[2].x = rect.left + (rect.right - rect.left) * 0.65f;
    points[2].y = rect.top + (rect.bottom - rect.top) * 0.7f;
    renderer.DrawLine(auto_cast(points[0]), auto_cast(points[1]), &brush);
    renderer.DrawLine(auto_cast(points[1]), auto_cast(points[2]), &brush);
    renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the selbg.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_selbg(const NativeDrawArgs& args) noexcept {
    // 正式渲染
    auto draw_this = [&args](float alpha, StyleState disabled) noexcept {
        auto& renderer = UIManager.Ref2DRenderer();
        auto color = ColorF::FromRGBA_CT<0x3399ffff_rgba>();
        if (disabled) color = ColorF::FromRGBA_CT<0xd6d6d6ff_rgba>();
        color.a = alpha;
        auto& brush = UIManager.RefCCBrush(color);
        auto rect = args.border;
#if 0
        const auto width = rect.right - rect.left;
        //if (args.to.selected)
        rect.right = rect.left + width * alpha;
        //else rect.left = rect.right - width * alpha;
#else
        //const auto width = rect.right - rect.left;
        const auto height = rect.bottom - rect.top;
        const auto center = (rect.bottom + rect.top) * 0.5f;
        rect.top = center - height * alpha * 0.5f;
        rect.bottom = center + height * alpha * 0.5f;

        //const auto width = rect.right - rect.left;
        //const auto center2 = (rect.left + rect.right) * 0.5f;
        //rect.left = center2 - width * alpha * 0.5f;
        //rect.right = center2 + width * alpha * 0.5f;

        //rect.right = rect.left + width * alpha;
        //rect.bottom = rect.top + height * alpha;
#endif
        renderer.FillRectangle(auto_cast(rect), &brush);
    };
    // 改变时
    if (detail::is_any<State_Selected>(args.to, args.from)) {
        // 渲染
        auto get_alpha = [&]() noexcept {
            if (detail::is_same<State_Selected>(args.to, args.from)) return 1.f;
            else return args.from & State_Selected ? 1.f - args.progress : args.progress;
        };
        draw_this(get_alpha(), args.to & State_Disabled);
    }
}




/// <summary>
/// Draws the button.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_button(
    const NativeDrawArgs& args) noexcept {
    // 获取颜色
    ColorF bgcolor1, bdcolor1, bgcolor2, bdcolor2;
    const auto w1 = self::get_button_color(args.from, bdcolor1, bgcolor1);
    const auto w2 = self::get_button_color(args.to, bdcolor2, bgcolor2);
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    const auto deprogress = 1.f - args.progress;
    const auto border_halfw = w2 * args.progress + w1 * deprogress;
    // 边框中心位置
    auto center = args.border;
    center.top += border_halfw;
    center.left += border_halfw;
    center.right -= border_halfw;
    center.bottom -= border_halfw;
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 背景色彩
    auto& bursh0 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(args.border), &bursh0);
    // 边框色彩
    auto& bursh1 = UIManager.RefCCBrush(bdcolor);
    renderer.DrawRectangle(auto_cast(center), &bursh1, border_halfw * 2.f);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the tb button.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_tb_button(
    const NativeDrawArgs & args) noexcept {
    const auto get_color = [](StyleState state, ColorF& bg, ColorF& bd) noexcept {
        uint32_t bgc = 0xffffff00_rgba, bdc = 0xffffff00_rgba;
        if (state & State_Disabled);
        else if (state & (State_Active | State_Checked))
            bgc = 0xcce8ffff_rgba, bdc = 0x99d1ffff_rgba;
        else if (state & State_Hover)
            bgc = 0xe5f3ffff_rgba, bdc = 0xcce8ffff_rgba;
        ColorF::FromRGBA_RT(bg, { bgc });
        ColorF::FromRGBA_RT(bd, { bdc });
    };
    // 获取颜色
    ColorF bgcolor1, bdcolor1, bgcolor2, bdcolor2;
    get_color(args.from, bgcolor1, bdcolor1);
    get_color(args.to, bgcolor2, bdcolor2);
    // 计算颜色
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    // 无需渲染
    if (bgcolor.a == 0.f) return;
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    // 背景色彩
    auto& bursh0 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(args.border), &bursh0);
    // 边框中心位置
    const auto border_halfw = 0.5f;
    auto center = args.border;
    center.top += border_halfw;
    center.left += border_halfw;
    center.right -= border_halfw;
    center.bottom -= border_halfw;
    // 边框色彩
    auto& bursh1 = UIManager.RefCCBrush(bdcolor);
    renderer.DrawRectangle(auto_cast(center), &bursh1, border_halfw * 2.f);
}

/// <summary>
/// Gets the color of the button.
/// </summary>
/// <param name="state">The state.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
auto LongUI::CUINativeStyleWindows10::get_button_color(
    StyleState state, ColorF & bd, ColorF & bg) noexcept -> float {
    // 颜色定义
    constexpr auto hover_border = 0x0078d7ff_rgba;
    constexpr auto hover_backgd = 0xe5f1fbff_rgba;
    constexpr auto active_border = 0x005499ff_rgba;
    constexpr auto active_backgd = 0xcce4f7ff_rgba;
    constexpr auto normal_border = 0xadadadff_rgba;
    constexpr auto normal_backgd = 0xe1e1e1ff_rgba;
    constexpr auto default_border = 0x0078d7ff_rgba;
    constexpr auto disabled_border = 0xbfbfbfff_rgba;
    constexpr auto disabled_backgd = 0xccccccff_rgba;
    // 使用数据
    uint32_t bdc, bgc;
    float border_halfw = 0.5f;
    // 禁用
    if (state & State_Disabled) { bdc = disabled_border; bgc = disabled_backgd; }
    // 按下/点中
    else if (state & (State_Active | State_Checked)) { bdc = active_border; bgc = active_backgd; }
    // 悬浮
    else if (state & State_Hover) { bdc = hover_border; bgc = hover_backgd; }
    // 普通
    else {
        bgc = normal_backgd;
        // 作为默认按钮再缩1逻辑像素
        if (state & State_Default) {
            border_halfw = 1.f;
            bdc = default_border;
        }
        else bdc = normal_border;
    }
    // 返回数据
    ColorF::FromRGBA_RT(bd, { bdc });
    ColorF::FromRGBA_RT(bg, { bgc });
    return border_halfw;
}

/// <summary>
/// Draws the textfield.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_textfield(const NativeDrawArgs & args) noexcept {
    // 获取颜色
    ColorF bdcolor1, bdcolor2;
    const auto get_tf_color = [](StyleState state, ColorF& fc) noexcept {
        constexpr auto disabled = 0xbfbfbfff_rgba;
        constexpr auto normal = 0x7a7a7aff_rgba;
        constexpr auto hover = 0x171717ff_rgba;
        constexpr auto focus = 0x005ca4ff_rgba;
        uint32_t color;
        if (state & State_Disabled) color = disabled;
        else if (state & State_Focus) color = focus;
        else if (state & State_Hover) color = hover;
        else color = normal;
        ColorF::FromRGBA_RT(fc, { color });
    };
    get_tf_color(args.from, bdcolor1);
    get_tf_color(args.to, bdcolor2);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    const auto border_halfw = 0.5f;
    // 边框中心位置
    auto center = args.border;
    center.top += border_halfw;
    center.left += border_halfw;
    center.right -= border_halfw;
    center.bottom -= border_halfw;
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 边框色彩
    auto& bursh1 = UIManager.RefCCBrush(bdcolor);
    renderer.DrawRectangle(auto_cast(center), &bursh1, border_halfw * 2.f);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}



/// <summary>
/// Draws the checkbox icon.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_checkbox(const NativeDrawArgs& args) noexcept {
    // 获取颜色
    ColorF bgcolor1, bdcolor1, bgcolor2, bdcolor2;
    self::get_radio_cbx_color(args.from, bdcolor1, bgcolor1);
    self::get_radio_cbx_color(args.to, bdcolor2, bgcolor2);
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    // 渲染背景
    self::draw_cbx_bg(args.border, bdcolor, bgcolor);
    // 前景准备
    bool draw_check = false;
    float progress = args.progress;
    // 第三态
    if (detail::is_any<State_Indeterminate>(args.from, args.to)) {
        // 状态已经固定
        if (detail::is_same<State_Indeterminate>(args.from, args.to)) progress = 1.f;
        // 反向修改
        else if (args.from & State_Indeterminate) progress = 1.f - progress;
    }
    // 选中态
    else {
        draw_check = true;
        // 状态已经固定
        if (detail::is_same<State_Checked>(args.from, args.to)) {
            progress = args.from & State_Checked ? 1.f : 0.f;
        }
        // 反向修改
        else if (args.from & State_Checked) progress = 1.f - progress;
    }
    // 渲染前景
    if (draw_check) self::draw_check_only(args.border, bdcolor, progress);
    else self::draw_cbx_ind(args.border, bdcolor, progress);
}


/// <summary>
/// Draws the checkbox indeterminate.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_cbx_ind(
    const RectF & rect,
    const ColorF & color,
    float progress) noexcept {
    if (progress == 0.0f) return;
    // 计算偏移
    constexpr float BOXW = float(CHECKBOX_IND_SIZE);
    const float box_half = BOXW * progress * 0.5f - 0.5f;
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f - 0.5f;
    RectF draw;
    // 计算矩形
    draw.top = center_y - box_half;
    draw.left = center_x - box_half;
    draw.right = center_x + box_half;
    draw.bottom = center_y + box_half;
    // 渲染
    auto& renderer = UIManager.Ref2DRenderer();
    renderer.FillRectangle(auto_cast(draw), &UIManager.RefCCBrush(color));
}

/// <summary>
/// Draws the check only.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_check_only(
    const RectF& rect,
    const ColorF& color,
    float progress) noexcept {
    if (progress == 0.0f) return;
    // 常量定义
    constexpr float PEN = 1.5f;
    constexpr float HAL = float(CHECKBOX_ICON_SIZE - 2) * 0.5f;
    constexpr float P1X = -(HAL - CHECKBOX_CHECK_X1) - 0.5f;
    constexpr float P1Y = -0.5f;
    constexpr float P2X = P1X + CHECKBOX_CHECK_X2;
    constexpr float P2Y = P1Y + CHECKBOX_CHECK_X2;
    constexpr float P3X = P2X + CHECKBOX_CHECK_X3;
    constexpr float P3Y = P2Y - CHECKBOX_CHECK_X3;
    // 资源获取
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    // 数据计算
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    const Point2F p1{ center_x + P1X , center_y + P1Y };
    const Point2F p2{ center_x + P2X , center_y + P2Y };
    const Point2F p3{ center_x + P3X , center_y + P3Y };
    // 第一阶段
    renderer.DrawLine(
        auto_cast(p1),
        auto_cast(LongUI::Mix(p1, p2, progress * 2.f)),
        &brush,
        PEN
    );
    // 第二阶段
    if (progress > 0.5f) {
        renderer.DrawLine(
            auto_cast(p2),
            auto_cast(LongUI::Mix(p2, p3, progress * 2.f - 1.f)),
            &brush,
            PEN
        );
    }
}


/// <summary>
/// Draws the radio CBX.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_cbx_bg(
    const RectF& rect,
    const ColorF& bd,
    const ColorF& bg) noexcept {
    // 中心点
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    constexpr float radius_bg = float(CHECKBOX_ICON_SIZE) * 0.5f;
    constexpr float radius_bd = radius_bg - 0.5f;
    // 矩形
    RectF draw; auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 背景
    draw.top = center_y - radius_bg;
    draw.left = center_x - radius_bg;
    draw.right = center_x + radius_bg;
    draw.bottom = center_y + radius_bg;
    if (bg.a > 0.0f)
        renderer.FillRectangle(
            auto_cast(draw), &UIManager.RefCCBrush(bg)
        );
    // 边框
    draw.top = center_y - radius_bd;
    draw.left = center_x - radius_bd;
    draw.right = center_x + radius_bd;
    draw.bottom = center_y + radius_bd;
    renderer.DrawRectangle(
        auto_cast(draw), &UIManager.RefCCBrush(bd)
    );
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}



/// <summary>
/// Draws the group box.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_group_box(const NativeDrawArgs & args) noexcept {
    // 预备
    const auto color = ColorF::FromRGBA_CT<0xdcdcdcff_rgba>();
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    auto rect = args.border;
    constexpr float width = 1.f;
    rect.top += width * 0.5f;
    rect.left += width * 0.5f;
    rect.right -= width * 0.5f;
    rect.bottom -= width * 0.5f;
    // 渲染
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.DrawRectangle(auto_cast(rect), &brush, width);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the caption.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_caption(const RectF& rect) noexcept {
    // 预备
    const auto color = ColorF::FromRGBA_CT<RGBA_White>();
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.FillRectangle(auto_cast(rect), &brush);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the list item.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_list_item(
    const NativeDrawArgs & args) noexcept {
    // win10只有selected和hover有颜色(大概)
    const auto get_color = [](StyleState state) noexcept {
        if (state & State_Selected) return 0xcde8ffff_rgba;
        else if (state & State_Hover) return 0xe5f3ffff_rgba;
        else return 0xffffffff_rgba;
    };
    // 获取颜色
    const auto color1 = get_color(args.from);
    const auto color2 = get_color(args.to);
    // 没有颜色
    if ((color1 & color2) == 0xffffffff_rgba) return;
    ColorF bgcolor1, bgcolor2;
    ColorF::FromRGBA_RT(bgcolor1, { color1 });
    ColorF::FromRGBA_RT(bgcolor2, { color2 });
    // 混合
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    //const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    const auto deprogress = 1.f - args.progress;
    //const auto border_halfw = w2 * args.progress + w1 * deprogress;
#if 0
    // 边框中心位置
    auto center = args.border;
    center.top += border_halfw;
    center.left += border_halfw;
    center.right -= border_halfw;
    center.bottom -= border_halfw;


    const auto get_alpha = [&]() noexcept {
        if (args.to.hover == args.from.hover) return 1.f;
        else return args.from.hover ? 1.f - args.progress : args.progress;
    }();

    const auto height = center.bottom - center.top;
    const auto mid = (center.bottom + center.top) * 0.5f;
    center.top = mid - height * get_alpha * 0.5f;
    center.bottom = mid + height * get_alpha * 0.5f;
#endif
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    // 背景色彩
    auto& bursh0 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(args.border), &bursh0);
    // 边框色彩
    //auto& bursh1 = UIManager.RefCCBrush(bdcolor);
    //renderer.DrawRectangle(auto_cast(center), &bursh1, border_halfw * 2.f);
}



/// <summary>
/// Draws the menu separator.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_menu_separator(const RectF& rect) noexcept {
    const auto color = ColorF::FromRGBA_CT<0xd7d7d7ff_rgba>();
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    // 线条色彩
    auto& bursh0 = UIManager.RefCCBrush(color);
    RectF line = { };
    line.top = (rect.bottom + rect.top) * 0.5f - 0.5f;
    line.bottom = line.top + 1.f;
    line.left = rect.left + float(MENUSEPARATOR_LEAD);
    line.right = rect.right - float(MENUSEPARATOR_HEIGHT);
    renderer.FillRectangle(auto_cast(line), &bursh0);
}


/// <summary>
/// Draws the status bar.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_status_bar(const RectF& rect) noexcept {
    const auto bgcolor = ColorF::FromRGBA_CT<0xf0f0f0ff_rgba>();
    const auto licolor = ColorF::FromRGBA_CT<0xd7d7d7ff_rgba>();
    auto& renderer = UIManager.Ref2DRenderer();
    // 背景色
    auto& bursh0 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(rect), &bursh0);
    // 线条色
    RectF line = rect;
    line.bottom = line.top + 1.f;
    auto& bursh1 = UIManager.RefCCBrush(licolor);
    renderer.FillRectangle(auto_cast(line), &bursh1);
}

/// <summary>
/// Draws the status bar panel.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_status_bar_panel(const RectF& rect) noexcept {
    const auto color = ColorF::FromRGBA_CT<0xd7d7d7ff_rgba>();
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    // 线条色彩
    auto& bursh0 = UIManager.RefCCBrush(color);
    RectF line = rect;
    line.left = rect.right - 1.f;
    renderer.FillRectangle(auto_cast(line), &bursh0);
}



/// <summary>
/// Draws the radio icon.
/// </summary>
/// <param name="border">The border rect.</param>
/// <param name="state">The state.</param>
/// <param name="opacity">The opacity.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_radio(
    const NativeDrawArgs& args) noexcept {
    // 获取颜色
    ColorF bgcolor1, bdcolor1, bgcolor2, bdcolor2;
    self::get_radio_cbx_color(args.from, bdcolor1, bgcolor1);
    self::get_radio_cbx_color(args.to, bdcolor2, bgcolor2);
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    const auto bdcolor = LongUI::Mix(bdcolor1, bdcolor2, args.progress);
    // 渲染背景
    self::draw_radio_bg(args.border, bdcolor, bgcolor);
    // 前景准备
    float progress = args.progress;
    // 状态已经固定
    if (detail::is_same<State_Checked>(args.from, args.to)) {
        progress = args.from & State_Checked ? 1.0f : 0.0f;
    }
    // 反向修改
    else if (args.from & State_Checked) progress = 1.f - progress;
    // 渲染前景
    self::draw_radio_only(args.border, bdcolor, progress);
}


/// <summary>
/// Draws the radio bg.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_radio_bg(
    const RectF& rect, const ColorF& bd, const ColorF & bg) noexcept {
    // 中心点
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    constexpr float radius_bd = float(CHECKBOX_ICON_SIZE) * 0.5f;
    constexpr float radius_bg = radius_bd - 0.5f;
    // 矩形
    Ellipse draw;
    draw.point = { center_x, center_y };
    // 背景
    draw.radius_x = radius_bg;
    draw.radius_y = radius_bg;
    if (bg.a > 0.0f)
        UIManager.Ref2DRenderer().FillEllipse(
            auto_cast(draw), &UIManager.RefCCBrush(bg)
        );
    // 边框
    draw.radius_x = radius_bd;
    draw.radius_y = radius_bd;
    UIManager.Ref2DRenderer().DrawEllipse(
        auto_cast(draw), &UIManager.RefCCBrush(bd)
    );
}

/// <summary>
/// Gets the color of the radio CBX.
/// </summary>
/// <param name="state">The state.</param>
/// <param name="bd">The bd.</param>
/// <param name="bg">The bg.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::get_radio_cbx_color(
    StyleState state, ColorF & bd, ColorF & bg) noexcept {
    // 颜色
    constexpr auto hover_border = 0x0078d7ff_rgba;
    constexpr auto active_border = 0x005499ff_rgba;
    constexpr auto normal_border = 0x000000ff_rgba;
    constexpr auto disabled_border = 0xccccccff_rgba;
    constexpr auto active_backgd = 0xcce4f7ff_rgba;
    // 使用数据
    uint32_t bdc, bgc = 0xcce4f700_rgba;
    // 禁用
    if (state & State_Disabled) { bdc = disabled_border; }
    // 按下
    else if (state & State_Active) { bdc = active_border; bgc = active_backgd; }
    // 悬浮
    else if (state & State_Hover) { bdc = hover_border; }
    // 普通
    else { bdc = normal_border; }
    // 返回颜色
    ColorF::FromRGBA_RT(bd, { bdc });
    ColorF::FromRGBA_RT(bg, { bgc });
}

/// <summary>
/// Draws the radio only.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="color">The color.</param>
/// <param name="progress">The progress.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_radio_only(
    const RectF & rect,
    const ColorF & color,
    float progress) noexcept {
    if (progress == 0.0f) return;
    // 计算偏移
    constexpr float BOXW = float(CHECKBOX_IND_SIZE);
    const float box_half = BOXW * progress * 0.5f;
    const float center_x = (rect.left + rect.right) * 0.5f;
    const float center_y = (rect.top + rect.bottom) * 0.5f;
    Ellipse draw;
    // 计算矩形
    draw.point = { center_x, center_y };
    draw.radius_x = box_half;
    draw.radius_y = box_half;
    // 渲染
    UIManager.Ref2DRenderer().FillEllipse(
        auto_cast(draw),
        &UIManager.RefCCBrush(color)
    );
}



/// <summary>
/// Draws the slider track.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_slider_track(
    const NativeDrawArgs& args, bool vertical) noexcept {
    // 颜色处理
    constexpr auto border_color = 0xd6d6d6ff_rgba;
    constexpr auto background_color = 0xe7eaeaff_rgba;
    const auto bdc = ColorF::FromRGBA_CT<border_color>();
    const auto bgc = ColorF::FromRGBA_CT<background_color>();
    // 都是纯色，全用Fill
    RectF rect;
    if (vertical) {
        const float center_x = (args.border.left + args.border.right) * 0.5f;
        rect.top = args.border.top;
        rect.bottom = args.border.bottom;
        rect.left = center_x - SLIDER_TRACK_HHH;
        rect.right = center_x + SLIDER_TRACK_HHH;
    }
    else {
        const float center_y = (args.border.top + args.border.bottom) * 0.5f;
        rect.left = args.border.left;
        rect.right = args.border.right;
        rect.top = center_y - SLIDER_TRACK_HHH;
        rect.bottom = center_y + SLIDER_TRACK_HHH;
    }
    // 边框
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect),
        &UIManager.RefCCBrush(bdc)
    );
    // 背景
    rect.top++; rect.left++; rect.right--; rect.bottom--;
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(rect),
        &UIManager.RefCCBrush(bgc)
    );

}

/// <summary>
/// Draws the slider thumb.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_slider_thumb(
    const NativeDrawArgs & args) noexcept {
    // 颜色定义
    auto get_thumb_color = [](StyleState s, ColorF& out) noexcept {
        constexpr auto normal_color = 0x007ad9ff_rgba;
        constexpr auto hover_color = 0x171717ff_rgba;
        constexpr auto other_color = 0xccccccff_rgba;
        uint32_t code;
        if (s & (State_Disabled | State_Active)) code = other_color;
        else if (s & State_Hover) code = hover_color;
        else code = normal_color;
        ColorF::FromRGBA_RT(out, { code });
    };
    // 计算颜色
    ColorF color1, color2;
    get_thumb_color(args.from, color1);
    get_thumb_color(args.to, color2);
    const auto color = LongUI::Mix(color1, color2, args.progress);
    // 直接填充矩形
    UIManager.Ref2DRenderer().FillRectangle(
        auto_cast(args.border),
        &UIManager.RefCCBrush(color)
    );
}




/// <summary>
/// Draws the progress bar.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_progress_bar(
    const NativeDrawArgs & args) noexcept {
    // 预备
    const auto color = ColorF::FromRGBA_CT<0xbcbcbcff_rgba>();
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    auto rect = args.border;
    constexpr float width = 1.f;
    rect.top += width * 0.5f;
    rect.left += width * 0.5f;
    rect.right -= width * 0.5f;
    rect.bottom -= width * 0.5f;
    // 渲染
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.DrawRectangle(auto_cast(rect), &brush, width);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the progress chunk.
/// </summary>
/// <param name="args">The arguments.</param>
/// <param name="vertical">if set to <c>true</c> [vertical].</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_progress_chunk(
    const NativeDrawArgs & args, bool vertical) noexcept {
    // 预备
    const auto color = ColorF::FromRGBA_CT<0x06b025ff_rgba>();
    auto& renderer = UIManager.Ref2DRenderer();
    auto& brush = UIManager.RefCCBrush(color);
    // 渲染
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    renderer.FillRectangle(auto_cast(args.border), &brush);
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}





PCN_NOINLINE
/// <summary>
/// Draws the tab.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_tab(
    const NativeDrawArgs& args) noexcept {
    // 调整矩形位置(未选中的下压缩2单位)
    auto rect = args.border;
    // 这个位置以target为标准
    if (!args.to & State_Selected) rect.top += 2.f;
    // 获取颜色标准
    const auto get_tab_color = [](StyleState state, ColorF& fc) noexcept {
        uint32_t color;
        // 选择
        if (state & State_Selected) color = 0xffffffff_rgba;
        // 未选中
        else color = state & State_Hover ? 0xd8eaf9ff_rgba : 0xf0f0f0ff_rgba;
        // 颜色
        ColorF::FromRGBA_RT(fc, { color });
    };
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 边框
    const auto bdcolor = ColorF::FromRGBA_CT<0xd9d9d9ff_rgba>();
    auto& bursh0 = UIManager.RefCCBrush(bdcolor);
    renderer.FillRectangle(auto_cast(rect), &bursh0);
    // 获取颜色
    ColorF bgcolor1, bgcolor2;
    get_tab_color(args.from, bgcolor1);
    get_tab_color(args.to, bgcolor2);
    // 混合颜色
    const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
    // 调整位置
    const auto draw_bottom = !(args.to & State_Selected);
    //const auto draw_left = args.to.selected || !args.to.after_seltab;
    const auto draw_left = args.to & State_Selected;
    const auto draw_right = args.to & (State_Selected | State_TAST);
    rect.top += 1.f;
    if (draw_left) rect.left += 1.f;
    if (draw_right) rect.right -= 1.f;
    if (draw_bottom) rect.bottom -= 1.f;
    // 背景色彩
    auto& bursh1 = UIManager.RefCCBrush(bgcolor);
    renderer.FillRectangle(auto_cast(rect), &bursh1);
    // 底层直线
    if (detail::is_same<State_Selected>(args.from, args.to)) {
        const auto fv = 1.f - args.progress;
        auto& bursh2 = UIManager.RefCCBrush(
            args.from & State_Selected ? ColorF{ 1,1,1,1 } : bdcolor
        );
        const D2D1_POINT_2F p1{ args.border.left, args.border.bottom };
        const auto width = args.border.right - args.border.left;
        renderer.DrawLine(p1, { p1.x + width * fv, p1.y }, &bursh2);
    }
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

/// <summary>
/// Draws the tab panels.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
//void LongUI::CUINativeStyleWindows10::draw_tab_panels(
//    const NativeDrawArgs & args) noexcept {
//    draw_group_box(args);
//}





/// <summary>
/// Draws the tree header cell.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_tree_header_cell(
    const NativeDrawArgs & args) noexcept {
    // 渲染器
    auto& renderer = UIManager.Ref2DRenderer();
    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

    // -------------------------- 背景色

    // win10只有active和hover有颜色(大概)
    const auto get_color = [](StyleState state) noexcept {
        if (state & State_Active) return 0xbcdcf4ff_rgba;
        else if (state & State_Hover) return 0xd9ebf9ff_rgba;
        else return 0xffffffff_rgba;
    };
    // 获取颜色
    const auto color1 = get_color(args.from);
    const auto color2 = get_color(args.to);
    // 没有颜色
    if ((color1 & color2) != 0xffffffff_rgba) {
        ColorF bgcolor1, bgcolor2;
        ColorF::FromRGBA_RT(bgcolor1, { color1 });
        ColorF::FromRGBA_RT(bgcolor2, { color2 });
        // 混合
        const auto bgcolor = LongUI::Mix(bgcolor1, bgcolor2, args.progress);
        const auto deprogress = 1.f - args.progress;
        // 边框中心位置
        auto center = args.border;
        // 背景色彩
        auto& bursh0 = UIManager.RefCCBrush(bgcolor);
        renderer.FillRectangle(auto_cast(args.border), &bursh0);
    }

    // -------------------------- 右侧竖线
    const auto rightline = ColorF::FromRGBA_CT<0xe5e5e5ff_rgba>();
    auto rightrect = args.border;
    rightrect.left = rightrect.right - 1.f;
    auto& burshr = UIManager.RefCCBrush(rightline);
    renderer.FillRectangle(auto_cast(rightrect), &burshr);

    //renderer.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

}



/// <summary>
/// Draws the tree twisty.
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void LongUI::CUINativeStyleWindows10::draw_tree_twisty(
    const NativeDrawArgs& args) noexcept {
    // 不显示
    if (!(args.to & State_Indeterminate)) return;
    // 获取颜色
    const auto get_color = [](StyleState state, ColorF& c) noexcept {
        uint32_t color;
        if (state & State_Hover) color = 0x1cc4f7ff_rgba;
        else color = state & State_Closed ? 0xa6a6a6ff_rgba : 0x404040ff_rgba;
        ColorF::FromRGBA_RT(c, { color });
    };
    ColorF fcolor1, fcolor2;
    get_color(args.from, fcolor1);
    get_color(args.to, fcolor2);
    const auto fcolor = LongUI::Mix(fcolor1, fcolor2, args.progress);
    // 计算角度
    float angle = 180.f;
    if (detail::is_different<State_Closed>(args.from, args.to)) {
        const auto delta = 90.f * args.progress;
        angle = args.from & State_Closed ? 90.f + delta : 180.f - delta;
    }
    else if (args.from & State_Closed) {
        angle = 90.f;
    }
    // 渲染前景

    this->draw_arrow_only(args.border, fcolor, angle);
}

