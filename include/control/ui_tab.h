#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

// super
#include "ui_boxlayout.h"
//#include "../util/ui_double_click.h"
// base
#include "ui_image.h"
#include "ui_label.h"

// ui namespace
namespace LongUI {
    // tab item
    class UITab : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    protected:
        // ctor
        UITab(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UITab() noexcept;
        // ctor
        explicit UITab(UIControl* parent = nullptr) noexcept : UITab(UITab::s_meta) { this->final_ctor(parent); }
    public:
        // set selected
        void SetSelected() noexcept;
        // set text
        void SetText(const CUIString& str) noexcept;
        // force mark selected
        void ForceMark() noexcept { m_oStyle.state = m_oStyle.state | State_Selected; }
        // force mark after
        void ForceAfter() noexcept { m_oStyle.state = m_oStyle.state | State_TAST; }
        // force clear after
        void ForceBefore() noexcept { m_oStyle.state = m_oStyle.state & ~State_TAST; }
    public:
        // do normal event
        //auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update, postpone change some data
        void Update(UpdateReason) noexcept override;
        // render this control only, [Global rendering and Incremental rendering]
        //void Render() const noexcept override;
        // recreate/init device(gpu) resource
        //auto Recreate() noexcept->Result override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
#ifdef LUI_ACCESSIBLE
        // accessible api
        auto accessible(const AccessibleEventArg& args) noexcept->EventAccept override;
#endif
    private: // private control impl
        // private image
        UIImage             m_oImage;
        // private label
        UILabel             m_oLabel;
    };
    // get meta info for UITab
    LUI_DECLARE_METAINFO(UITab);
}
