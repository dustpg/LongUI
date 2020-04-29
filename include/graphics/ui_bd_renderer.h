#pragma once

// ui
#include "../luiconf.h"
#ifndef LUI_DISABLE_STYLE_SUPPORT
#include "ui_renderer_decl.h"
#include "../core/ui_color.h"
#include "../core/ui_object.h"
#include "../style/ui_attribute.h"

namespace LongUI {
    // image output
    struct IImageOutput;
    // border renderer
    class CUIRendererBorder : public CUISmallObject {
    public:
        // ctor
        CUIRendererBorder() noexcept;
        // dtor
        ~CUIRendererBorder() noexcept;
        // move ctor
        CUIRendererBorder(CUIRendererBorder&&) = delete;
        // copy ctor
        CUIRendererBorder(const CUIRendererBorder&) = delete;
        // before render
        void BeforeRender(const Box& box) noexcept;
        // render border
        void RenderBorder(const Box& box) const noexcept;
        // release device data
        void ReleaseDeviceData() noexcept;
        // create device data
        auto CreateDeviceData() noexcept->Result;
    public:
        // mark size changed
        void MarkSizeChanged() noexcept { m_bLayoutChanged = true; }
        // set image id
        void SetImageId(uint32_t ) noexcept;
        // set image repeat
        void SetImageRepeat(AttributeRepeat repeat) noexcept;
        // set image slice
        void SetImageSlice(const RectF&, bool fill) noexcept;
        // get image id
        auto GetImageId() const noexcept { return m_idImage; }
        // get image repeat
        auto GetImageRepeat() const noexcept { return m_repeat; }
        // get image slice
        bool GetImageSlice(RectF& output) const noexcept {
            output = m_rcSlice; return m_bSliceFill; }
    private:
        // calculate repeat
        void calculate_repeat(RectF& rect, const Box& box, Size2F size) const noexcept;
        // refresh image
        auto refresh_image() noexcept->Result;
        // release effect
        void release_effect() noexcept;
        // refresh real slice
        void refresh_real_slice() noexcept;
        // refresh draw count
        void refresh_draw_count() noexcept;
        // refresh image matrix
        void refresh_image_matrix(const Box& box) noexcept;
        // render default border
        void render_default_border(const Box& box) const noexcept;
        // ----------- CACHE-DATA -----------
        // size of image
        Size2F              m_szImage = {};
        // real slice rect
        RectF               m_rcRealSlice = { };
    private:
        // ------------- GPU-RES ------------
        // effect
        I::Effect*          m_pBorder = nullptr;
        // iamge output
        IImageOutput*       m_pOutput = nullptr;
    private:
        // ------------- CPU-RES ------------
        // rect for image slice
        RectF               m_rcSlice = {};
        // resource for image
        uint32_t            m_idImage = 0;
        // fill for image slice
        bool                m_bSliceFill = false;
        // [FLAG]image layout changed
        bool                m_bLayoutChanged = false;
        // repeat for image
        AttributeRepeat     m_repeat = Repeat_Stretch2;
    public:
        // style
        AttributeBStyle     style = Style_None;
        // border color
        ColorF              color = {};
        // radius x
        float               radius_x = 0.f;
        // radius y
        float               radius_y = 0.f;
    };
}
#endif