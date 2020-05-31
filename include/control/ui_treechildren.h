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

#include "ui_boxlayout.h"
//#include "../util/ui_double_click.h"

// ui namespace
namespace LongUI {
    // tree children
    class UITreeChildren : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    protected:
        // ctor
        UITreeChildren(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UITreeChildren() noexcept;
        // ctor
        explicit UITreeChildren(UIControl* parent = nullptr) noexcept : UITreeChildren(UITreeChildren::s_meta) { this->final_ctor(parent); }
    public:
        // set level offset
        void SetLevelOffset(float offset);
    public:
        // do normal event
        //auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        //// do mouse event
        //auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update, postpone change some data
        void Update(UpdateReason) noexcept override;
        //// render this control only, [Global rendering and Incremental rendering]
        //void Render() const noexcept override;
        //// recreate/init device(gpu) resource
        //auto Recreate() noexcept->Result override;
    protected:
        // add child for debug
        void add_child(UIControl& child) noexcept override;
        // relayout this
        //void relayout_this() noexcept;
        // refresh minsize
        //void refresh_minsize() noexcept;
    public:
        // line height
        //float                   line_height = 10.f;
        // ref line height
        auto&RefLineHeight() noexcept { return this->line_size.height; }
    private:
        // has child
        bool                    m_bHasChild = false;
    };
    // get meta info for UITreeChildren
    LUI_DECLARE_METAINFO(UITreeChildren);
}
