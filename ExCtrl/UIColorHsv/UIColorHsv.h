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
#include <Control/UIControl.h>

// LongUI namespace
namespace LongUI {
    // control to display hsv picker
    class UIColorHsv : public UIControl {
        // super class
        using Super = UIControl ;
        // clean this
        virtual void cleanup() noexcept override;
        // release reource
        void release_gpu_resource() noexcept;
        // value changed
        void value_changed() noexcept;
        // clicked type
        enum ClickedType : uint8_t {
            // none
            Type_Node = 0,
            // hsv ring
            Type_Ring,
            // triangle
            Type_Trianle
        };
    public:
        // basic width of hsv line
        static constexpr float BASIC_PICK_CIRCLE_RADIUS = 4.f;
        // basic width of hsv line
        static constexpr float BASIC_HSV_LINE_WIDTH = 2.5f;
        // basic width of hsv circle
        static constexpr float BASIC_HSV_CIRCLE_WIDTH = 30.f;
        // basic radius of hsv circle, Hsv::UNIT < this < 2*Hsv::UNIT
        static constexpr float BASIC_HSV_CIRCLE_RADIUS = 150.f;
        // basic radius of triangle, ≈ Hsv::UNIT
        static constexpr float BASIC_HSV_TRIANGLE_RADIUS = 126.233f;
    public:
        // Render 渲染
        virtual void Render() const noexcept override;
        // update 刷新
        //virtual void Update() noexcept override;
        // do event 事件处理
        virtual bool DoEvent(const EventArgument& arg) noexcept override;
        // do event 事件处理
        virtual bool DoMouseEvent(const MouseEventArgument& arg) noexcept override;
        // recreate 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    protected:
        // add event listener
        virtual bool uniface_addevent(SubEvent sb, UICallBack&& call) noexcept override;
        // something must do before deleted
        void before_deleted() noexcept { Super::before_deleted(); }
        // render chain -> background
        void render_chain_background() const noexcept { return Super::render_chain_background(); }
        // render chain -> mainground
        void render_chain_main() const noexcept;
        // render chain -> foreground
        void render_chain_foreground() const noexcept { return Super::render_chain_foreground(); }
    public:
        // create 创建
        static auto CreateControl(CreateEventType, pugi::xml_node) noexcept ->UIControl*;
        // ctor: cp- parent in contorl-level
        UIColorHsv(UIContainer* cp) noexcept : Super(cp) {}
        // get hue/color rotation in [0, 360)
        auto GetH() const noexcept { return m_fHue; }
        // get s in [0, 1]
        auto GetS() const noexcept { return m_fSaturation; }
        // get v in [0, 1]
        auto GetV() const noexcept { return m_fValue; }
        // get color
        auto&PickColor() const noexcept { return m_colorPicked; }
        // set color
        void SetColor(const D2D1_COLOR_F& c) noexcept;
        // set hue/color rotation in [0, 360)
        void SetH(float h) noexcept;
        // set saturation in [0, 1]
        void SetS(float s) noexcept;
        // set value in [0, 1]
        void SetV(float v) noexcept;
        // set hue/color rotation in any degree
        void SetHue(float h) noexcept {
            h = std::fmod(h, 360.f); if (h < 0.f) h += 360.f;
            this->SetH(h);
        }
    protected:
        // initialize, maybe you want call v-method
        void initialize(pugi::xml_node node) noexcept;
        // dtor
        ~UIColorHsv() noexcept { this->release_gpu_resource(); }
        // copy ctor = delete
        UIColorHsv(const UIColorHsv&) = delete;
    protected:
        // callback
        UICallBack                  m_event;
        // control name to display h
        CUIWrappedCCP               m_strH = "";
        // control name to display s
        CUIWrappedCCP               m_strS = "";
        // control name to display v
        CUIWrappedCCP               m_strV = "";
        // control name to display r
        CUIWrappedCCP               m_strR = "";
        // control name to display g
        CUIWrappedCCP               m_strG = "";
        // control name to display b
        CUIWrappedCCP               m_strB = "";
        // control to display h
        UIControl*                  m_pCtrlH = nullptr;
        // control to display s
        UIControl*                  m_pCtrlS = nullptr;
        // control to display v
        UIControl*                  m_pCtrlV = nullptr;
        // control to display r
        UIControl*                  m_pCtrlR = nullptr;
        // control to display g
        UIControl*                  m_pCtrlG = nullptr;
        // control to display b
        UIControl*                  m_pCtrlB = nullptr;
        // hsv brush
        ID2D1LinearGradientBrush*   m_pHsvBrushLG = nullptr;
        // hsv ouput
        ID2D1Image*                 m_pHsvOutput = nullptr;
        // hsv effect
        ID2D1Effect*                m_pHsvEffect = nullptr;
        // hsv brush
        ID2D1ImageBrush*            m_pHsvBrush = nullptr;
        // hsv picked color
        D2D1_COLOR_F                m_colorPicked = {0.f};
        // realization of triangle
        ID2D1GeometryRealization*   m_pTriangle = nullptr;
        // hue/rotation of hsv, [0.f, 360.f)
        float                       m_fHue = 0.f;
        // saturation, [0.f, 1.f]
        float                       m_fSaturation = 1.f;
        // value(brightness?), [0.f, 1.f]
        float                       m_fValue = 1.f;
        // clicked on the clrcle
        ClickedType                 m_tyClicked = Type_Node;
        // unused
        uint8_t                     m_unused_u8[3] = {233,233,233};
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    };
#ifdef LongUIDebugEvent
    // 重载?特例化 GetIID
    template<> const IID& GetIID<LongUI::UIColorHsv>() noexcept;
#endif
}