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
        // get color
        auto&GetColor() const noexcept { return m_color; }
    public:
        // set image with id
        void SetImage(uint32_t id) noexcept;
        // set color, need repaint
        void SetColor(const ColorF& c) noexcept { m_color = c; }
        // set clip, need repaint
        void SetClip(AttributeBox clip) noexcept { m_clip = clip; }
        // set repeat
        void SetRepeat(AttributeRepeat ar) noexcept { m_repeat = ar; }
        // set origin
        void SetOrigin(AttributeBox origin) noexcept { m_origin = origin; }
        // set attachment, need repaint
        void SetAttachment(AttributeAttachment aa) noexcept { m_attachment = aa; }
    private:
        // ------------- GPU-RES ------------
    private:
        // ------------- CPU-RES ------------
        // background-color
        ColorF              m_color;
        // background-image
        uint32_t            m_idImage = 0;
        // background-clip
        AttributeBox        m_clip = Box_BorderBox;
        // background-repeat
        AttributeRepeat     m_repeat = Repeat_Repeat;
        // background-origin 
        AttributeBox        m_origin = Box_PaddingBox;
        // background-attachment
        AttributeAttachment m_attachment = Attachment_Scroll;
    };
}