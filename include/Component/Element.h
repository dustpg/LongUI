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
    // UI Animation for Posotion
    using CUIAnimationTransform = CUIAnimation<D2D1_MATRIX_3X2_F>;
    // UI Animation for Posotion
    using CUIAnimationPosition = CUIAnimation<D2D1_POINT_2F>;
    // UI Animation for Color
    using CUIAnimationColor = CUIAnimation<D2D1_COLOR_F>;
    // UI Animation for Opacity
    using CUIAnimationOpacity = CUIAnimation<FLOAT>;
    // UI Animation for offset
    using CUIAnimationOffset= CUIAnimation<FLOAT>;
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
            auto Recreate() noexcept ->HRESULT;
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
            auto Recreate() noexcept ->HRESULT;
            // change brush
            void ChangeBrush(ControlStatus index, ID2D1Brush* brush) noexcept {
                LongUI::SafeRelease(m_apBrushes[index]);
                m_apBrushes[index] = LongUI::SafeAcquire(brush);
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
            ~Elements() noexcept { LongUI::SafeRelease(m_pBrush); }
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
        /// <summary>
        /// Basic Animation State Machine
        /// </summary>
        class AnimationStateMachine {
        public:
            // State
            using State = uint16_t;
            // ctor
            AnimationStateMachine(pugi::xml_node node, const char* prefix = nullptr) noexcept;
            // dtor
            ~AnimationStateMachine() noexcept {}
        public:
            // get now baisc state
            auto GetNowBaiscState() const noexcept { return m_sttBasicNow; }
            // get old baisc state
            auto GetOldBaiscState() const noexcept { return m_sttBasicOld; }
            // get now extra state
            auto GetNowExtraState() const noexcept { return m_sttExtraNow; }
            // get old extra state
            auto GetOldExtraState() const noexcept { return m_sttExtraOld; }
            // get value of baisc state
            auto GetBaiscVaule() const noexcept { return m_aniBasic.value; }
            // get value of extra state
            auto GetExtraVaule() const noexcept { return m_aniExtra.value; }
        public:
            // set basic state
            void SetBasicState(State) noexcept;
            // set basic state
            void SetExtraState(State) noexcept;
            // update with delta time
            auto Update(float t) noexcept { m_aniBasic.Update(t); m_aniExtra.Update(t); }
            // update without delta time
            //auto Update() noexcept { this->Update(UIManager.GetDeltaTime()); }
        private:
            // basic state animation
            CUIAnimation<FLOAT>     m_aniBasic;
            // extra state animation
            CUIAnimation<FLOAT>     m_aniExtra;
            // basic state - old
            State                   m_sttBasicOld = 0;
            // basic state - now
            State                   m_sttBasicNow = 0;
            // basic state - old
            State                   m_sttExtraOld = 0;
            // basic state - now
            State                   m_sttExtraNow = 0;
        };
        /// <summary>
        /// Extra Animation State Machine
        /// </summary>
        template<class GIBasic, class GIExtra, typename StateBasic, typename StateExtra=uint16_t>
        class AnimationStateMachineEx {
            // sort name
            using StateTarget = AnimationStateMachine::State;
            // assert test
            static_assert(sizeof(StateBasic) <= sizeof(StateTarget), "bad action");
            // assert test
            static_assert(sizeof(StateExtra) <= sizeof(StateTarget), "bad action");
        public:
            // ctor
            AnimationStateMachineEx(pugi::xml_node node, const char* prefix = nullptr) noexcept
            : m_machine(node, prefix),  m_ifBasic(node, prefix), m_ifExtra(node, prefix) { }
            // dtor
            ~AnimationStateMachineEx() noexcept = default;
        public:
            // get now baisc state
            auto GetNowBaiscState() const noexcept { return static_cast<StateBasic>(m_machine.GetNowBaiscState()); }
            // get old baisc state
            auto GetOldBaiscState() const noexcept { return static_cast<StateBasic>(m_machine.GetOldBaiscState()); }
            // get now extra state
            auto GetNowExtraState() const noexcept { return static_cast<StateExtra>(m_machine.GetNowBaiscState()); }
            // get old extra state
            auto GetOldExtraState() const noexcept { return static_cast<StateExtra>(m_machine.GetOldBaiscState()); }
            // get value of baisc state
            auto GetBaiscVaule() const noexcept { return m_machine.GetBaiscVaule(); }
            // get value of extra state
            auto GetExtraVaule() const noexcept { return m_machine.GetExtraVaule(); }
        public:
            // set basic state
            void SetBasicState(StateBasic s) noexcept { m_machine.SetBasicState(static_cast<StateTarget>(s)); }
            // set basic state
            void SetExtraState(StateBasic s) noexcept { m_machine.SetExtraState(static_cast<StateTarget>(s)); }
            // update with delta time
            auto Update(float t) noexcept { m_machine.Update(t); }
            // update without delta time
            auto Update() noexcept { this->Update(UIManager.GetDeltaTime()); }
            // render
            void Render(const D2D1_RECT_F& rect) const noexcept {
                if (m_ifExtra.IsValid()) {
                    m_ifExtra.Render(rect, static_cast<const AnimationStateMachine&>(m_machine));
                }
                else {
                    m_ifBasic.Render(rect, static_cast<const AnimationStateMachine&>(m_machine));
                }
            }
            // recreate
            auto Recreate() noexcept { 
                HRESULT hr = S_OK;
                if (SUCCEEDED(hr)) {
                    hr = m_ifBasic.Recreate();
                }
                if (SUCCEEDED(hr)) {
                    hr = m_ifExtra.Recreate();
                }
                return hr;
            }
        private:
            // baisc machine
            AnimationStateMachine           m_machine;
            // basic interface, must be valid
            GIBasic                         m_ifBasic;
            // extra interface, change to basic if invalid
            GIExtra                         m_ifExtra;
        };
        /// <summary>
        /// Helper for Graphics Interface
        /// </summary>
        class GIHelper {
        public:
            // clean interface
            static void Clean(IUnknown** brushes, size_t size) noexcept;
            // clean interface
            template<class T> static void Clean(T** itfs, size_t size) noexcept { GIHelper::Clean(reinterpret_cast<IUnknown**>(itfs),size); }
            // recreate for meta
            static auto Recreate(Meta metas[], const uint16_t ids[], size_t size) noexcept->HRESULT;
            // recreate for brush
            static auto Recreate(ID2D1Brush* brushes[], const uint16_t ids[], size_t size) noexcept->HRESULT;
        };
        /// <summary>
        /// Graphics Interface for Opacity-Based Meta
        /// </summary>
        template<class GIConfig> class GIMeta {
            // constant
            enum : size_t {
                // count of basic state
                BASIC_COUNT = GIConfig::GetBasicCount(),
                // count of extra state
                EXTRA_COUNT = GIConfig::GetBasicCount(),
            };
        public:
            // ctor
            GIMeta(pugi::xml_node node, const char* prefix = nullptr) noexcept { GIConfig::InitMeta(node, prefix, m_metas, m_aID); }
            // dtor 
            ~GIMeta() noexcept = default;
            // recreate
            auto Recreate() noexcept { return GIHelper::Recreate(m_metas, m_aID, BASIC_COUNT * EXTRA_COUNT); }
            // render this
            void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm) const noexcept;
            // check if valid
            bool IsValid() const noexcept { return m_aID[BASIC_COUNT - 1] != 0; }
        private:
            // metas
            Meta            m_metas[BASIC_COUNT * EXTRA_COUNT];
            // metas id
            uint16_t        m_aID[BASIC_COUNT * EXTRA_COUNT];
        };
        /// <summary>
        /// Graphics Interface for brush in rectangle
        /// </summary>
        template<class GIConfig> class GIBurshRect {
            // constant
            enum : size_t {
                // count of basic state
                BASIC_COUNT = GIConfig::GetBasicCount(),
                // count of extra state
                EXTRA_COUNT = GIConfig::GetBasicCount(),
            };
        public:
            // ctor
            GIBurshRect(pugi::xml_node node, const char* prefix = nullptr) noexcept { GIConfig::InitBrush(node, prefix, m_apBrushes, m_aID); }
            // dtor 
            ~GIBurshRect() noexcept { GIHelper::Clean(m_apBrushes, lengthof(m_apBrushes)); }
            // check if valid (must be valid)
            bool IsValid() const noexcept { return true; }
            // recreate
            auto Recreate() noexcept { return GIHelper::Recreate(m_apBrushes, m_aID, BASIC_COUNT * EXTRA_COUNT); }
            // render this
            void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm) const noexcept;
        private:
            // brush
            ID2D1Brush*     m_apBrushes[STATUS_COUNT];
            // brush id
            uint16_t        m_aID[STATUS_COUNT];
        };
        /// <summary>
        /// Graphics Interface Config for button
        /// </summary>
        class GIConfigButton {
        public:
            // get count of basic state
            static constexpr auto GetBasicCount() noexcept { return size_t(STATUS_COUNT); }
            // get count of extra state
            static constexpr auto GetExtraCount() noexcept { return size_t(1); }
            // meta initialize
            static void InitMeta(pugi::xml_node node, const char* prefix, Meta metas[], uint16_t ids[]) noexcept;
            // meta initialize
            static void InitBrush(pugi::xml_node node, const char* prefix, ID2D1Brush* brushes[], uint16_t ids[]) noexcept;
        };
        /// <summary>
        /// Element for Button
        /// </summary>
        using Element4Button = AnimationStateMachineEx<GIBurshRect<GIConfigButton>, GIMeta<GIConfigButton>, ControlStatus>;
    }
}