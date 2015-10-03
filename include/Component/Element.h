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


// longui namespace
namespace LongUI {
    // set new status
#define UIElement_SetNewStatus(e,s) m_pWindow->StartRender(e.GetByType<Element_Basic>().SetNewStatus(s), this)
    // UI Animation for Color
    using CUIAnimationColor = CUIAnimation<D2D1_COLOR_F>;
    // UI Animation for Opacity
    using CUIAnimationOpacity = CUIAnimation<FLOAT>;
    // UI Animation for offset
    using CUIAnimationOffset= CUIAnimation<FLOAT>;
    // UI Animation for Posotion
    using CUIAnimationPosition = CUIAnimation<D2D1_POINT_2F>;
    // UI Animation for Posotion
    using CUIAnimationTransform = CUIAnimation<D2D1_MATRIX_3X2_F>;
    // Element
    enum Element : uint32_t {
        Element_Basic = 0,          // basic element for animation
        Element_Meta,               // meta element for rendering meta
        Element_ColorRect,          // color-rect element for rendering colored-rect
        Element_BrushRect,          // brush-rect element for rendering brushed-rect
        // ------- in planning -------
        Element_ColorGeometry,      // color-geo element for rendering colored-geometry
        Element_BrushGeometry,      // brush-geo element for rendering brushed-geometry
    };
    // Component namespace
    namespace Component {
        // Elements
        template<Element... > class Elements;
        // render unit
        template<Element Head, Element... Tail>
        class Elements<Head, Tail...> : protected virtual Elements<Tail...>, protected Elements<Head> {
            // super class
            using SuperA = Elements<Tail...>;
            // super class
            using SuperB = Elements<Head>;
        public:
            // set unit type
            auto SetElementType(Element unit) noexcept { this->type = unit; }
            // ctor
            Elements(pugi::xml_node node, const char* prefix=nullptr) noexcept : SuperA(node, prefix), SuperB(node, prefix) {}
        public:
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<ElementType>& { return SuperA::GetByType<ElementType>(); }
            // get element for head
            template<>
            auto GetByType<Head>() noexcept ->Elements<Head>& { return static_cast<Elements<Head>&>(*this); }
            // get element: const overload
            template<Element ElementType> 
            auto GetByType() const noexcept ->const Elements<ElementType>& { return SuperA::GetByType<ElementType>(); }
            // get element for head: const overload
            template<>
            auto GetByType<Head>() const noexcept ->const Elements<Head>& { return static_cast<const Elements<Head>&>(*this); }
            // render this
            void Render(const D2D1_RECT_F& rect) const noexcept { this->type == Head ? SuperB::Render(rect) : SuperA::Render(rect); }
            // update with delta time
            auto Update(float t) noexcept { animation.Update(t); }
            // update without delta time
            auto Update() noexcept { animation.Update(UIManager.GetDeltaTime()); }
            // recreate
            auto Recreate() noexcept {
                HRESULT hr = S_OK;
                if (SUCCEEDED(hr)) {
                    hr = SuperA::Recreate();
                }
                if (SUCCEEDED(hr)) {
                    hr = SuperB::Recreate();
                }
                return hr;
            }
        };
        // element for all
        template<> class Elements<Element_Basic> {
        public:
            // ctor 
            Elements(pugi::xml_node node = LongUINullXMLNode, const char* prefix = nullptr)
                noexcept : animation(AnimationType::Type_QuadraticEaseOut) {
                UNREFERENCED_PARAMETER(node);
                UNREFERENCED_PARAMETER(prefix);
                animation.end = 1.f;
            }
            // init 
            void Init(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // render this
            void Render(const D2D1_RECT_F&) const noexcept { }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element_Basic>& { return *this; }
            // get element: const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element_Basic>& { return *this; }
            // set new status
            auto SetNewStatus(ControlStatus) noexcept ->float;
            // get status
            auto GetStatus() const noexcept { return m_stateTartget; }
            // recreate
            auto Recreate() { return S_OK; }
            // type of unit
            Element                 type = Element_Basic;
        protected:
            // state of unit
            ControlStatus           m_state = ControlStatus::Status_Disabled;
            // state of unit
            ControlStatus           m_stateTartget = ControlStatus::Status_Disabled;
        public:
            // animation
            CUIAnimationOpacity     animation;
        };
        // element for bitmap
        template<> class Elements<Element_Meta> : protected virtual Elements<Element_Basic>{
            // super class
            using Super = Elements<Element_Basic>;
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element_Meta>& { return *this; }
            // get element: const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element_Meta>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F&) const noexcept;
            // recreate
            auto Recreate() noexcept->HRESULT;
            // is OK?
            auto IsOK() noexcept { return m_aID[Status_Normal] != 0; }
        protected:
            // metas
            Meta            m_metas[STATUS_COUNT];
            // metas id
            uint16_t        m_aID[STATUS_COUNT];
        };
        // element for brush rect
        template<> class Elements<Element_BrushRect> : protected virtual Elements<Element_Basic>{
            // super class
            using Super = Elements<Element_Basic>;
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // dtor
            ~Elements() noexcept { this->release_data(); }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element_BrushRect>& { return *this; }
            // get element : const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element_BrushRect>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F& rect) const noexcept;
            // recreate
            auto Recreate() noexcept->HRESULT;
            // change brush
            void ChangeBrush(ControlStatus index, ID2D1Brush* brush) noexcept {
                ::SafeRelease(m_apBrushes[index]);
                m_apBrushes[index] = ::SafeAcquire(brush);
            }
        private:
            // relase data
            void release_data() noexcept;
        protected:
            // brush
            ID2D1Brush*     m_apBrushes[STATUS_COUNT];
            // brush id
            uint16_t        m_aID[STATUS_COUNT];
        };
        // element for color rect
        template<> class Elements<Element_ColorRect> : protected virtual Elements<Element_Basic>{
            // super class
            using Super = Elements<Element_Basic>;
        public:
            // ctor
            Elements(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // dtor
            ~Elements() noexcept { ::SafeRelease(m_pBrush); }
            // get element
            template<Element ElementType>
            auto GetByType() noexcept ->Elements<Element_ColorRect>& { return *this; }
            // get element: const overload
            template<Element ElementType>
            auto GetByType()const noexcept ->const Elements<Element_ColorRect>& { return *this; }
            // render this
            void Render(const D2D1_RECT_F& rect) const noexcept;
            // recreate
            auto Recreate() noexcept ->HRESULT;
            // change color
            void ChangeColor(ControlStatus index, D2D1_COLOR_F& color) noexcept { colors[index] = color; }
            // change color
            void ChangeColor(ControlStatus index, uint32_t color, float alpha = 1.f) noexcept { colors[index] = D2D1::ColorF(color, alpha); }
        public:
            // brush id
            D2D1_COLOR_F            colors[STATUS_COUNT];
        protected:
            // brush
            ID2D1SolidColorBrush*   m_pBrush = nullptr;
        };
    }
}