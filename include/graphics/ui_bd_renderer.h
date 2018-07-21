#pragma once

// ui
#include "../luiconf.h"
#ifndef LUI_DISABLE_STYLE_SUPPORT
#include "ui_renderer_decl.h"
#include "../core/ui_color.h"
#include "../core/ui_object.h"
#include "../style/ui_attribute.h"

namespace LongUI {
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
        void BeforeRender() noexcept;
        // render border
        void RenderBorder(const Box& box) const noexcept;
        // release device data
        void ReleaseDeviceData() noexcept;
        // create device data
        auto CreateDeviceData() noexcept->Result;
    public:
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
        // refresh image
        auto refresh_image() noexcept->Result;
        // release brush
        void release_brush() noexcept;
        // refresh real slice
        void refresh_real_slice() noexcept;
        // render default border
        void render_default_border(const Box& box) const noexcept;
        // ----------- CACHE-DATA -----------
        // size of image
        Size2F              m_szImage = {};
        // real slice rect
        RectF               m_rcRealSlice = { };
    private:
        // ------------- GPU-RES ------------
        // image brush
        I::Brush*           m_pImageBrush = nullptr;
    private:
        // ------------- CPU-RES ------------
        // rect for image slice
        RectF               m_rcSlice = {};
        // resource for image
        uint32_t            m_idImage = 0;
        // fill for image slice
        bool                m_bSliceFill = false;
        // repeat for image
        AttributeRepeat     m_repeat = Repeat_Stretch2;
    private:
        // image layout changed
        bool                m_bLayoutChanged = false;
    };
}
#endif