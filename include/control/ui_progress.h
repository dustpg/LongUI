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

// ui
#include "ui_control.h"
// tmp->hbox
#include "ui_box.h"

// ui namespace
namespace LongUI {
    // progress
    class UIProgress : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    protected:
        // ctor
        UIProgress(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIProgress() noexcept;
        // ctor
        explicit UIProgress(UIControl* parent = nullptr) noexcept : UIProgress(UIProgress::s_meta) { this->final_ctor(parent); }
    public:
        // do normal event
        auto DoEvent(UIControl*, const EventArg&) noexcept->EventAccept override;
        // update
        void Update(UpdateReason) noexcept override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
#ifdef LUI_ACCESSIBLE
        // accessible api
        auto accessible(const AccessibleEventArg& args) noexcept->EventAccept;
#endif
    public:
        // get max
        auto GetMax() const noexcept { return m_max; }
        // get value
        auto GetValue() const noexcept { return m_value; }
        // set max
        void SetMax(float max) noexcept;
        // set value
        void SetValue(float value) noexcept;
    private:
        // init this bar
        void init_bar() noexcept;
        // adjust flex
        void adjust_flex() noexcept;
    private: // private impl
        // the bar child
        UIControl               m_oBar;
        // the remainder child
        UIControl               m_oRemainder;
    private:
        // max value
        float                   m_max = 100.f;
        // value
        float                   m_value = 0.f;
    };
    // get meta info for UIProgress
    LUI_DECLARE_METAINFO(UIProgress);
}