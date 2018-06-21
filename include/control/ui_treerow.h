#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
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

#include "ui_control.h"
//#include "../util/ui_double_click.h"

// ui namespace
namespace LongUI {
    // tree row
    class UITreeRow : public UIControl {
        // super class
        using Super = UIControl;
        // private
        struct Private;
    protected:
        // ctor
        UITreeRow(UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UITreeRow() noexcept;
        // ctor
        UITreeRow(UIControl* parent = nullptr) noexcept : UITreeRow(parent, UITreeRow::s_meta) {}
    public:
        // is selected?
        //auto IsSelected() const noexcept { return m_oStyle.state.selected; }
        // set has child
        void SetHasChild(bool has) noexcept;
        // open node
        bool OpenNode() noexcept;
        // close node
        bool CloseNode() noexcept;
        // toggle node
        bool ToggleNode() noexcept;
        // set level offset
        void SetLevelOffset(float offset) noexcept;
    public:
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update, postpone change some data
        void Update() noexcept override;
        // render this control only, [Global rendering and Incremental rendering]
        //void Render() const noexcept override;
        // recreate/init device(gpu) resource
        //auto Recreate() noexcept->Result override;
    protected:
        // open/close node
        void open_close(bool open) noexcept;
        // relayout
        void relayout() noexcept;
        // refresh minsize
        void refresh_minsize() noexcept;
    private:
        // private data
        Private*                m_private = nullptr;
    protected:
        // offset
        float                   m_fLevelOffset = 0.f;
        // node closed
        bool                    m_bOpened = true;
        // node keep child
        bool                    m_bHasChild = false;
    };
    // get meta info for UITreeRow
    LUI_DECLARE_METAINFO(UITreeRow);
}
