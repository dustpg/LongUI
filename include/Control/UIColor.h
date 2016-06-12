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

#include "UIControl.h"
#include "../Component/Text.h"

// LongUI namespace
namespace LongUI {
    // display color
    class UIColor final : public UIControl {
        // super class
        using Super = UIControl ;
        // clean this
        virtual void cleanup() noexcept override;
    public:
        // Render 渲染
        virtual void Render() const noexcept override;
        // update 刷新
        //virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        //virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // render chain -> background: do nothing
        void render_chain_background() const noexcept;
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept  { return Super::render_chain_foreground(); }
    public:
        // create 创建
        static auto CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIColor(UIContainer* cp) noexcept : Super(cp) {}
        // set direct transparent
        void SetDirectTransparent() noexcept { m_state.SetTrue<State_Self1>(); }
        // set normal transparent
        void SetNormalTransparent() noexcept { m_state.SetFalse<State_Self1>(); }
        // get color
        auto&GetColor() const noexcept { return m_color; }
        // set color
        void SetColor(const D2D1_COLOR_F& c) noexcept { 
            m_color = c; this->InvalidateThis();
        }
    protected:
        // check transparent mode
        bool is_direct_transparent() const noexcept { return m_state.Test<State_Self1>(); }
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // dtor
        ~UIColor() noexcept { }
        // copy ctor = delete
        UIColor(const UIColor&) = delete;
    protected:
        // color
        D2D1_COLOR_F            m_color{1.f, 1.f, 1.f, 1.f};
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIColor>() noexcept;
#endif
}