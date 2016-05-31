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
#include "../Component/EditableText.h"

// LongUI namespace
namespace LongUI {
    // 基本编辑类
    class UIEdit : public UIControl {
        // 父类申明
        using Super = UIControl;
        // close this control 关闭控件
        virtual void cleanup() noexcept override;
    public:
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // ui call
        virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept override;
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept;
    public:
        // create this
        static auto CreateControl(CreateEventType, pugi::xml_node) noexcept -> UIControl*;
        // constructor 构造函数
        UIEdit(UIContainer* cp) noexcept : Super(cp), m_text(this) { }
        // get component
        auto&GetComponent() noexcept { return m_text; }
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // destructor 析构函数
        ~UIEdit() noexcept {};
        // deleted function
        UIEdit(const UIEdit&) = delete;
    protected:
        // core editable text component
        Component::EditableText m_text;
        // color of border
        D2D1_COLOR_F            m_aBorderColor[STATE_COUNT];
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIEdit>() noexcept;
#endif
}
