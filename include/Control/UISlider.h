#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
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
    // default slider control 默认滑块控件
    class UISlider : public UIControl {
        // 父类申明
        using Super = UIControl;
        // clean this control 清除控件
        virtual void cleanup() noexcept override;
    public:
        // Render 渲染 
        virtual void Render() const noexcept override;
        // udate 刷新
        virtual void Update() noexcept override;
        // do event 事件处理
        //virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // do mouse event
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // ui call
        virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept override;
        // render chain -> background
        void render_chain_background() const noexcept;
        // render chain -> mainground
        void render_chain_main() const noexcept { return Super::render_chain_main(); }
        // render chain -> foreground
        void render_chain_foreground() const noexcept;
    public:
        // create 创建
        static UIControl* WINAPI CreateControl(CreateEventType type, pugi::xml_node) noexcept;
        // constructor 构造函数
        UISlider(UIContainer* cp) noexcept : Super(cp) { }
    private:
        // init
        void initialize(pugi::xml_node node) noexcept;
    public:
        // set state
        void SetControlState(ControlState state) noexcept { m_pWindow->StartRender(m_uiElement.SetBasicState(state), this); }
        // get state
        auto GetControlState() const noexcept { m_uiElement.GetNowBaiscState(); }
        // get value in [0, 1]
        auto GetValue01() const noexcept { return m_fValue; }
        // get value in [start, end]
        auto GetValueSE() const noexcept { return (m_fEnd - m_fStart) * m_fValue + m_fStart; }
        // is Vertical Slider?
        auto IsVerticalSlider() const noexcept { return m_bVerticalSlider; }
        // is Horizontal Slider?
        auto IsHorizontalSlider() const noexcept { return !m_bVerticalSlider; }
    protected:
        // destructor 析构函数
        ~UISlider() noexcept { }
        // deleted function
        UISlider(const UISlider&) = delete;
    protected:
        // slider rect
        D2D1_RECT_F                 m_rcThumb = D2D1::RectF();
        // ui element
        Component::Element4Button   m_uiElement;
        // value range[0, 1]
        float                       m_fValue = 0.f;
        // value range[0, 1]
        float                       m_fValueOld = 0.f;
        // start
        float                       m_fStart = 0.f;
        // end
        float                       m_fEnd = 100.f;
    public:
        // size of thumb
        D2D1_SIZE_F const           thumb_size = D2D1::SizeF(10.f, 20.f);
    protected:
        // click posistion
        float                       m_fClickPosition = 0.f;
        // is mouse click in
        bool                        m_bMouseClickIn = false;
        // is mouse move in
        bool                        m_bMouseMoveIn = false;
        // is vertical slider
        bool                        m_bVerticalSlider = false;
        // default background
        bool                        m_bDefaultBK = true;
        // callback
        UICallBack                  m_event;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<LongUI::UISlider>() {
        // {3BE5198C-B922-4C99-827E-F0D08875B045}
        static const GUID IID_LongUI_UISlider = {
            0xcc64ee29, 0x3be1, 0x4b24,{ 0x9b, 0x27, 0x99, 0xbf, 0x98, 0xaa, 0x3c, 0x15 } 
        };
        return IID_LongUI_UISlider;
    }
#endif
}