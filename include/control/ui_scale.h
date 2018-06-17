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

// ui
#include "ui_image.h"

// ui namespace
namespace LongUI {
    // scale/slider control
    class UIScale : public UIControl {
        // super class
        using Super = UIControl;
        // init slider
        void init_slider() noexcept;
        // mouse click
        void mouse_click(Point2F) noexcept;
    protected:
        // ctor
        UIScale(UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIScale() noexcept;
        // ctor
        UIScale(UIControl* parent = nullptr) noexcept:UIScale(parent, UIScale::s_meta) {}
    public:
        // value changed event
        static inline constexpr auto _changed() noexcept { return GuiEvent::Event_Change; }
    public:
        // recreate
        //auto Recreate() noexcept->Result override;
        // render
        //void Render() const noexcept override;
        // update
        void Update() noexcept override;
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept override;
#ifdef LUI_ACCESSIBLE
    protected:
        // accessible event
        auto accessible(const AccessibleEventArg&) noexcept->EventAccept override;
#endif
    public:
        // set orient
        //void SetOrient(AttributeOrient o) noexcept;
        // set value
        void SetValue(float value) noexcept;
        // set min
        void SetMin(float value) noexcept;
        // set max
        void SetMax(float value) noexcept;
        // set page increment
        void SetPageIncrement(float pi) noexcept;
        // add value
        void AddValue(float value) noexcept { SetValue(m_fValue + value); }
        // get min value
        auto GetMin() const noexcept { return m_fMin; }
        // get max value
        auto GetMax() const noexcept { return m_fMax; }
        // get value
        auto GetValue() const noexcept { return m_fValue; }
        // decrease
        void Decrease() noexcept { AddValue(-increment); }
        // increase
        void Increase() noexcept { AddValue(+increment); }
        // decrease page
        void DecreasePage() noexcept { AddValue(-m_fPageIncrement); }
        // increase page
        void IncreasePage() noexcept { AddValue(+m_fPageIncrement); }
        // get orient
        auto GetOrient() const noexcept { return static_cast<AttributeOrient>(m_state.orient); }
        // get page increment
        auto GetPage() const noexcept { return m_fPageIncrement; }
    private:
        // refresh thumb postion
        void refresh_thumb_postion() noexcept;
        // check page increment
        void refresh_thumb_size() noexcept;
    protected:
        // min value
        float                   m_fMin = 0.f;
        // max value
        float                   m_fMax = 100.0f;
        // now value
        float                   m_fValue = 0.f;
        // click offset
        float                   m_fClickOffset = 0.;
        // page increment
        float                   m_fPageIncrement = 10.f;
    public:
        // increment value
        float                   increment = 1.f;
        // thumb control
        UIImage                 thumb;
    };
    // slider
    using UISlider = UIScale;
    // get meta info for UIImage
    LUI_DECLARE_METAINFO(UIScale);
}