#pragma once

#include "ui_renderer_decl.h"
#include "../core/ui_color.h"
#include "../core/ui_object.h"
#include "../style/ui_attribute.h"

namespace LongUI {
    // box
    struct Box;
    // background renderer
    class CUIRendererBackground : public CUISmallObject {
    public:
        // ctor
        CUIRendererBackground() noexcept;
        // dtor
        ~CUIRendererBackground() noexcept;
        // move ctor
        CUIRendererBackground(CUIRendererBackground&&) = delete;
        // copy ctor
        CUIRendererBackground(const CUIRendererBackground&) = delete;
        // render color
        void RenderColor(const Box& box) const noexcept;
        // render image
        void RenderImage(const Box& box) const noexcept;
    public:
        // set image with id
        void RefreshImage() noexcept;
    private:
        // ------------- GPU-RES ------------
    public:
        // ------------- CPU-RES ------------
        // background-color
        ColorF              color;
        // background-image
        uint32_t            image_id = 0;
        // background-clip
        AttributeBox        clip = Box_BorderBox;
        // background-repeat
        AttributeRepeat     repeat = Repeat_Repeat;
        // background-origin 
        AttributeBox        origin = Box_PaddingBox;
        // background-attachment
        AttributeAttachment attachment = Attachment_Scroll;
    };
}