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
#include <Control/UIText.h>
#include <Component/Effect.h>

// LongUI namespace
namespace LongUI {
    // bulr-able text control 可模糊的文本控件
    class UIBlurText : public UIText {
        // 父类申明
        using Super = UIText ;
        // clean this
        virtual void cleanup() noexcept override;
    public:
        // Render 渲染
        virtual void Render() const noexcept override;
        // update 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept { return Super::render_chain_foreground(); }
    public:
        // set blur
        void SetBlurValue(float sd) noexcept;
        // get blur
        auto GetBulrValue() const noexcept { return m_fBlur; }
        // create 创建
        static auto CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIBlurText(UIContainer* cp) noexcept;
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // dtor
        ~UIBlurText() noexcept { }
        // copy ctor = delete
        UIBlurText(const UIBlurText&) = delete;
    protected:
        // blur effect
        Component::Effect               m_effect;
        // blur float
        float                           m_fBlur = 1.f;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIBlurText>() noexcept;
#endif
}