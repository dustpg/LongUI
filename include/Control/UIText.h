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


// LongUI namespace
namespace LongUI{
    // default text control 默认文本控件
    class UIText : public UIControl {
        // 父类申明
        using Super = UIControl ;
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
        //virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept;
    public:
        // create 创建
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIText(UIContainer* cp) noexcept : Super(cp) {}
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept { Super::initialize(node); m_text.Init(node); }
        // dtor
        ~UIText() noexcept { }
        // copy ctor = delete
        UIText(const UIText&) = delete;
    protected:
        // the text of control
        LongUI::Component::ShortText    m_text;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
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
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept { return Super::render_chain_foreground(); }
    public:
        // set blur
        void SetBlurValue(float b) noexcept { 
            m_fBlur = b;
            if(m_effect.IsOK())
                m_effect.SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, m_fBlur);
        }
        // get blur
        auto GetBulrValue() const noexcept { return m_fBlur; }
        // create 创建
        static auto WINAPI CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIBlurText(UIContainer* cp) noexcept : Super(cp), m_effect(CLSID_D2D1GaussianBlur) {}
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
    template<> LongUIInline const IID& GetIID<LongUI::UIText>() {
        // {47F83436-2D1F-413B-BBAD-9322EFF18185}
        static const GUID IID_LongUI_UIText = {
            0x47f83436, 0x2d1f, 0x413b,{ 0xbb, 0xad, 0x93, 0x22, 0xef, 0xf1, 0x81, 0x85 } 
        };
        return IID_LongUI_UIText;
    }
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UIBlurText>() {
        // {1AF45E28-9342-4CA4-AA6E-E48048C5E7AE}
        static const GUID IID_LongUI_UIBlurText = { 
            0x1af45e28, 0x9342, 0x4ca4, { 0xaa, 0x6e, 0xe4, 0x80, 0x48, 0xc5, 0xe7, 0xae }
        };
        return IID_LongUI_UIBlurText;
    }
#endif
}