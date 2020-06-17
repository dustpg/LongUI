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

#include "ui_control.h"

//#define LUI_NO_UIGRID

#ifndef LUI_NO_UIGRID
#include <utility>

// ui namespace
namespace LongUI {
    // rows
    class UIRows;
    // columns
    class UIColumns;
    // grid control
    class UIGrid : public UIControl {
        // super class
        using Super = UIControl;
    protected:
        // ctor
        UIGrid(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIGrid() noexcept override;
        // ctor
        explicit UIGrid(UIControl* parent = nullptr) noexcept : UIGrid(UIGrid::s_meta) { this->final_ctor(parent); }
    public:
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // update
        void Update(UpdateReason reason) noexcept override;
    protected:
        // add child
        void add_child(UIControl&) noexcept override;
        // relayout this
        void relayout_this() noexcept;
        // check minsize
        auto check_minsize(float buf[]) noexcept ->std::pair<uint32_t, float>;
        // is col mode
        inline bool is_col_mode() const noexcept;
    private:
        // rows
        UIRows*                 m_pRows = nullptr;
        // columns
        UIColumns*              m_pColumns = nullptr;
        // first
        UIControl*              m_pFirst = nullptr;
        // second
        UIControl*              m_pSecond = nullptr;
    };
    // get meta info for UIGrid
    LUI_DECLARE_METAINFO(UIGrid);
}
#endif
