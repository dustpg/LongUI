#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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

#include <luibase.h>
#include <luiconf.h>
#include <Control/UIButton.h>

// LongUI namespace
namespace LongUI {
    // color button
    class UIColorButton : public UIButton {
        // super class
        using Super = UIButton;
        // clean this
        virtual void cleanup() noexcept override;
    public:
        // Render
        virtual void Render() const noexcept override;
    protected:
        // add event listener
        //virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept override;
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept  { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept;
    public:
        // create 创建
        static auto CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIColorButton(UIContainer* cp) noexcept : Super(cp) {}
        // get color
        auto&GetColor() const noexcept { return m_color; }
        // set color
        void SetColor(const D2D1_COLOR_F& color) noexcept {
            m_color = color;
            this->InvalidateThis();
        }
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // dtor
        ~UIColorButton() noexcept = default;
        // copy ctor = delete
        UIColorButton(const UIColorButton&) = delete;
    protected:
        // color
        D2D1_COLOR_F            m_color = { 1.f,1.f,1.f,1.f };
        // color padding 
        //D2D1_RECT_F             m_rcPaddingColor = { 0.f };
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIColorButton>() noexcept;
#endif
}