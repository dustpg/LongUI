#pragma once

// ui header
#include "ui_control.h"
#include "../style/ui_text.h"
#include "../core/ui_string.h"
#include "../text/ui_text_layout.h"
#include "../text/ui_text_outline.h"
#include "../core/ui_const_sstring.h"
#include "../util/ui_named_control.h"
// cursor
#include <graphics/ui_cursor.h>

// ui namespace
namespace LongUI {
    // label
    class UILabel : public UIControl {
        // super class
        using Super = UIControl;
    protected:
        // ctor
        UILabel(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UILabel() noexcept;
        // ctor
        explicit UILabel(UIControl* parent = nullptr) noexcept : UILabel(UILabel::s_meta) { this->final_ctor(parent); }
    public:
        // normal event
        auto DoEvent(UIControl*, const EventArg& e) noexcept->EventAccept override;
        // mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        void Update(UpdateReason) noexcept override;
        // render
        void Render() const noexcept override;
    protected:
        // initialize
        void initialize() noexcept override;
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
#ifdef LUI_ACCESSIBLE
        // accessible api
        auto accessible(const AccessibleEventArg& args) noexcept->EventAccept override;
#endif
        // refresh fitting
        void refresh_fitting() noexcept;
        // on text changed
        void update_text_changed() noexcept;
        // init label
        void reset_font() noexcept;
        // setup access key
        void setup_access_key() noexcept;
        // is default href
        auto is_def_href() const noexcept { return !m_href.empty() 
#ifndef LUI_DISABLE_STYLE_SUPPORT
            && m_oStyle.matched.empty()
#endif
            ; }
        // after text changed
        void after_text_changed() noexcept;
    public:
        // get text [RECOMMENDED] 
        auto GetTextView() const noexcept { return m_string.view(); }
        // get text [UNRECOMMENDED] 
        auto GetText() const noexcept { return m_string.c_str(); }
        // ref text- string object
        auto&RefText() const noexcept { return m_string; }
        // set text, return true if changed
        bool SetText(const CUIString& text) noexcept;
        // set text, return true if changed
        bool SetText(CUIString&& text) noexcept;
        // set text, return true if changed
        bool SetText(U16View text) noexcept;
        // init crop value
        void InitCrop(AttributeCrop c) noexcept { m_crop = c; }
    public:
        // set default minsize
        //void SetAsDefaultMinsize() noexcept;
        // as same tf to another label
        void SameTfAs(const TextFont& l) noexcept { m_tfBuffer = l; }
        // as same tf to another label
        void SameTfAs(const FontArg& l) noexcept { m_tfBuffer.font = l; }
        // ref textfont
        auto&RefTextFont() const noexcept { return m_tfBuffer; }
    public:
        // set connection control
        void SetControl(UIControl& ctrl) noexcept { m_control.SetControl(&ctrl); }
        // show access key
        void ShowAccessKey(bool show = true) noexcept;
    protected:
        // hovered curor
        CUICursor               m_hrefCursor;
        // connection control
        NamedControl            m_control;
        // text font buffer
        TextFont                m_tfBuffer;
        // href text
        CUIConstShortString     m_href;
        // text layout
        CUITextLayout           m_text;
        // outline renderer
        CUITextOutline          m_outline;
        // text string
        CUIString               m_string;
        // access key position
        uint16_t                m_uPosAkey = 0;
        // crop rule
        AttributeCrop           m_crop = Crop_None;
    public:
        // default value
        enum : int32_t {
            // ** fonts are much differenty with each other
            // ** use this to adjust

            // default text x offset
            DEFUALT_TEXT_X_OFFSET = 0,
            // default text y offset
            DEFUALT_TEXT_Y_OFFSET = 0,
        };
    };
    // get meta info for UILabel
    LUI_DECLARE_METAINFO(UILabel);
}