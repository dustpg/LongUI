#pragma once

// ui
#include <core/ui_object.h>
#include <graphics/ui_mesh.h>
#include <graphics/ui_geometry.h>
#include <style/ui_native_style.h>
#include <luiconf.h>

// ui namespace
namespace LongUI {
    // color
    struct ColorF;
    // control
    class UIControl;
    /// <summary>
    /// 
    /// </summary>
    /// <seealso cref="CUIObject" />
    class CUINativeStyle : public CUIObject {

    };
#if 0
    /// <summary>
    /// win8 native style simulator
    /// </summary>
    class CUINativeStyleWindows8 final : public CUINativeStyle {
        /*
        Win8/8.1/10.0.10158之前
        默认按钮: 0x3399FF 矩形描边
        0. 禁用: 0xD9D9D9 矩形描边; 中心 0xEFEFEF
        1. 普通: 0xACACAC 矩形描边; 中心 从上到下0xF0F0F0到0xE5E5E5渐变
        2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
        3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变
        */
    };
#endif
    /// <summary>
    /// win10 native style simulator
    /// </summary>
    class CUINativeStyleWindows10 final : public CUINativeStyle {
        // release device-dependent resources
        void release_dd_resources() noexcept;
        // release all resources
        void release_all_resources() noexcept;
        // self type
        using self = CUINativeStyleWindows10;
    public:
        // arrow direction
        enum ArrowDirection : uint32_t {
            Direction_Up = 0,
            Direction_Right,
            Direction_Down,
            Direction_Left,
            DIRECTION_COUNT
        };
        // constant
        enum : uint32_t {
            // menu item height
            MENUITEM_HEIGHT = 21,
            // arrow size in px
            ARROW_SIZE = 17,
            // arrow with
            ARRAW_WDITH = 4,
            // checkbox icon size
            CHECKBOX_ICON_SIZE = 13,
            // checkbox indeterminate-box size
            CHECKBOX_IND_SIZE = 8,
            // checkbox check x1
            CHECKBOX_CHECK_X1 = 2,
            // checkbox check
            CHECKBOX_CHECK_X2 = 3,
            // checkbox check
            CHECKBOX_CHECK_X3 = 5,
            // slider thumb h-w
            SLIDER_THUMB_HW = 10,
            // slider thumb h-h
            SLIDER_THUMB_HH = 18,
            // SLIDER TRACK hor-half-height
            SLIDER_TRACK_HHH = 2,
        };
        // ctor
        CUINativeStyleWindows10() noexcept;
        // dtor
        ~CUINativeStyleWindows10() noexcept { this->release_all_resources(); }
        // recreate resources
        auto Recreate() noexcept->Result;
        // draw native
        void DrawNative(const NativeDrawArgs& args) noexcept;
        // initialize control
        void InitCtrl(UIControl& ctrl, AttributeAppearance) noexcept;
    private:
        // get radio/checkbox color
        static void get_radio_cbx_color(StyleState, ColorF& bd, ColorF& bg) noexcept;
        // get arrow button color
        static void get_arrow_btn_color(StyleState, ColorF& bc, ColorF& fc) noexcept;
        // get button color, return border half-width
        static auto get_button_color(StyleState, ColorF& bd, ColorF& bg) noexcept ->float;
    private:
        // draw arrow only
        void draw_arrow_only(const RectF&, const ColorF&, float angle) noexcept;
        // draw arrow button
        void draw_arrow_button(const NativeDrawArgs& args, ArrowDirection dir) noexcept;
        // draw tree twisty
        void draw_tree_twisty(const NativeDrawArgs& args) noexcept;
        // draw button
        static void draw_button(const NativeDrawArgs& args) noexcept;
        // draw textfield
        static void draw_textfield(const NativeDrawArgs& args) noexcept;
        // draw resizer
        static void draw_rasizer(const NativeDrawArgs& args) noexcept;
        // draw sb-track
        static void draw_sb_track(const NativeDrawArgs& args, bool vertical) noexcept;
        // draw sb-thumb
        static void draw_sb_thumb(const NativeDrawArgs& args, bool vertical) noexcept;
        // draw slider track
        static void draw_slider_track(const NativeDrawArgs& args, bool vertical) noexcept;
        // draw slider thumb
        static void draw_slider_thumb(const NativeDrawArgs& args) noexcept;
        // draw progress bar
        static void draw_progress_bar(const NativeDrawArgs& args) noexcept;
        // draw progress chunk
        static void draw_progress_chunk(const NativeDrawArgs& args, bool vertical) noexcept;
        // draw group box
        static void draw_group_box(const NativeDrawArgs& args) noexcept;
        // draw list item
        static void draw_list_item(const NativeDrawArgs& args) noexcept;
        // draw tree header cell
        static void draw_tree_header_cell(const NativeDrawArgs& args) noexcept;
        // draw check box
        static void draw_checkbox(const NativeDrawArgs& args) noexcept;
        // draw check only
        static void draw_check_only(const RectF&, const ColorF&, float) noexcept;
        // draw checkbox indeterminate only
        static void draw_cbx_ind(const RectF&, const ColorF&, float) noexcept;
        // draw checkbox background
        static void draw_cbx_bg(const RectF&, const ColorF& bd, const ColorF& bg) noexcept;
        // draw radio icon
        static void draw_radio(const NativeDrawArgs& args) noexcept;
        // draw radio only
        static void draw_radio_only(const RectF&, const ColorF&, float) noexcept;
        // draw radio background
        static void draw_radio_bg(const RectF&, const ColorF& bd, const ColorF& bg) noexcept;
        // draw selected background
        static void draw_selbg(const NativeDrawArgs& args) noexcept;
        // draw tab
        static void draw_tab(const NativeDrawArgs& args) noexcept;
        // draw tab panels
        //static void draw_tab_panels(const NativeDrawArgs& args) noexcept;
    private:
#ifdef LUI_DRAW_ARROW_IN_MESH
        // arrow mesh
        I::Mesh*                m_pMesh = nullptr;
#else
        // arrow geometry
        CUIGeometry             m_geoArrow;
#endif
    };





    // native theme not support any-scale(?), use hard-code simulate it
#if 0
#pragma comment(lib, "Uxtheme")
    // test code
    void NativeStyle() noexcept {
        if (const auto theme = ::OpenThemeData(nullptr, L"Button")) {
            ID2D1BitmapRenderTarget* target = nullptr;
            ID2D1GdiInteropRenderTarget* gdirt = nullptr;
            D2D1_PIXEL_FORMAT fmt = D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED
            );
            D2D1_SIZE_F size{ 512.f, 512.f };
            UIManager.Ref2DRenderer().CreateCompatibleRenderTarget(
                &size,
                nullptr,
                &fmt,
                D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_GDI_COMPATIBLE,
                &target
            );
            assert(target);
            target->QueryInterface(
                IID_ID2D1GdiInteropRenderTarget,
                (void**)&gdirt
            );
            assert(gdirt && "cannot failed that documented");
            HDC hdc = nullptr;
            target->BeginDraw();
            auto code = gdirt->GetDC(D2D1_DC_INITIALIZE_MODE_CLEAR, &hdc);
            assert(SUCCEEDED(code));
            ::SetGraphicsMode(hdc, GM_ADVANCED);
            XFORM matrix;
            auto bv = ::GetWorldTransform(hdc, &matrix);
            matrix.eM11 = 2.f;
            matrix.eM22 = 2.f;
            reinterpret_cast<D2D1_MATRIX_3X2_F&>(matrix) =
                D2D1::Matrix3x2F::Scale({ 2.f , 2.f });
            bv = ::SetWorldTransform(hdc, &matrix);
            assert(bv);
            //target->SetTransform(D2D1::Matrix3x2F::Scale({ 2,2 }));
            RECT rect{ 0, 0, 64, 64 };
            assert(SUCCEEDED(code));
            RECT content_rect;
            code = ::DrawThemeBackground(
                theme, hdc,
                BP_PUSHBUTTON, PBS_NORMAL,
                &rect,
                nullptr
            );
            assert(SUCCEEDED(code));
            code = gdirt->ReleaseDC(nullptr);
            assert(SUCCEEDED(code));
            code = target->EndDraw();
            assert(SUCCEEDED(code));
            ID2D1Bitmap* bitmap = nullptr;
            target->GetBitmap(&bitmap);
            assert(bitmap);
            I::Bitmap* real_bitmap = nullptr;
            bitmap->QueryInterface(IID_ID2D1Bitmap1, (void**)&real_bitmap);
            auto hr = UIManager.SaveAsPng(*real_bitmap, L"test.png");
            assert(hr);
            gdirt->Release();
            bitmap->Release();
            target->Release();
            real_bitmap->Release();
            ::CloseThemeData(theme);
        }
        else assert(!"error");
    }
#endif
}
