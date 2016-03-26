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

#include "UIText.h"
#include "../Component/Element.h"

// LongUI namespace
namespace LongUI {
    // default RadioButton control 默认单选框控件
    class UIRadioButton final : public UIText {
        // super class
        using Super = UIText ;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // box size
        static constexpr float RADIO_SIZE = 16.f;
    public:
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const EventArgument& arg) noexcept override;
        // do mouse event 鼠标事件处理
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // ui call
        virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept override;
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept;
        // render chain -> foreground
        void render_chain_foreground() const noexcept;
    public:
        // get control state
        auto GetControlState() const noexcept { return m_uiElement.GetNowBasicState(); }
        // force set control state
        void ForceSetControlState(ControlState state) noexcept;
        // get checked state
        auto GetCheckedState() const noexcept { return !!m_uiElement.GetNowExtraState(); }
        // force set checked state
        void ForceSetCheckedState(bool state) noexcept;
        // set control state in safe way
        void SafeSetControlState(ControlState state) noexcept {
            if (this->GetEnabled()) this->ForceSetControlState(state);
        }
        // set checked state in safe way
        void SafeSetCheckedState(bool state) noexcept {
            if (this->GetEnabled()) this->ForceSetCheckedState(state);
        }
    private:
        // uncheck checked radio button, then check this
        void uncheck_checked_and_check_this() noexcept;
    public:
        // create 创建
        static auto CreateControl(CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
        // constructor 构造函数
        UIRadioButton(UIContainer* cp) noexcept : Super(cp) {}
    protected:
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // init
        void initialize(pugi::xml_node node) noexcept;
        // destructor 析构函数
        ~UIRadioButton() noexcept;
        // deleted function
        UIRadioButton(const UIRadioButton&) = delete;
    protected:
        // callback
        UICallBack                  m_event;
        // color of border
        D2D1_COLOR_F                m_aBorderColor[STATE_COUNT];
        // element
        Component::Element4RadioBtn m_uiElement;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIRadioButton>() {
        // {AC3EDEE4-BF70-434D-8F73-E7F8EA702F0E}
        static const GUID IID_LongUI_UIRadioButton = { 
            0xac3edee4, 0xbf70, 0x434d, { 0x8f, 0x73, 0xe7, 0xf8, 0xea, 0x70, 0x2f, 0xe } 
        };
        return IID_LongUI_UIRadioButton;
    }
#endif
}