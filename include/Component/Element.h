﻿#pragma once
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
    // Component namespace
    namespace Component {
        /// <summary>
        /// Basic Animation State Machine
        /// </summary>
        class AnimationStateMachine {
        public:
            // State
            using State = uint16_t;
            // ctor
            AnimationStateMachine(pugi::xml_node node, State basic, State extra, const char* prefix = nullptr) noexcept;
            // dtor
            ~AnimationStateMachine() noexcept {}
        public:
            // get now basic state
            auto GetNowBaiscState() const noexcept { return m_sttBasicNow; }
            // get old basic state
            auto GetOldBaiscState() const noexcept { return m_sttBasicOld; }
            // get now extra state
            auto GetNowExtraState() const noexcept { return m_sttExtraNow; }
            // get old extra state
            auto GetOldExtraState() const noexcept { return m_sttExtraOld; }
            // get animation of basic state
            const auto& GetBasicAnimation() const noexcept { return m_aniBasic; }
            // get animation of extra state
            const auto& GetExtraAnimation() const noexcept { return m_aniExtra; }
        public:
            // set basic state
            auto SetBasicState(State) noexcept ->float;
            // set basic state
            auto SetExtraState(State) noexcept ->float;
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
            AnimationStateMachineEx(pugi::xml_node node, StateBasic basic, StateExtra extra /*= StateExtra(0)*/, const char* prefix = nullptr) noexcept
            : m_machine(node, static_cast<StateTarget>(basic), static_cast<StateTarget>(extra), prefix),  m_ifBasic(node, prefix), m_ifExtra(node, prefix) { }
            // dtor
            ~AnimationStateMachineEx() noexcept = default;
        public:
            // get now basic state
            auto GetNowBaiscState() const noexcept { return static_cast<StateBasic>(m_machine.GetNowBaiscState()); }
            // get old basic state
            auto GetOldBaiscState() const noexcept { return static_cast<StateBasic>(m_machine.GetOldBaiscState()); }
            // get now extra state
            auto GetNowExtraState() const noexcept { return static_cast<StateExtra>(m_machine.GetNowBaiscState()); }
            // get old extra state
            auto GetOldExtraState() const noexcept { return static_cast<StateExtra>(m_machine.GetOldBaiscState()); }
            // get value of basic state
            auto GetBaiscVaule() const noexcept { return m_machine.GetBaiscVaule(); }
            // get value of extra state
            auto GetExtraVaule() const noexcept { return m_machine.GetExtraVaule(); }
            // get basic interface
            auto&GetBasicInterface() noexcept { return m_ifBasic; }
            // get extra interface
            auto&GetExtraInterface() noexcept { return m_ifExtra; }
            // get basic interface
            const auto&GetBasicInterface() const noexcept { return m_ifBasic; }
            // get extra interface
            const auto&GetExtraInterface() const noexcept { return m_ifExtra; }
        public:
            // set basic state
            auto SetBasicState(StateBasic s) noexcept { return m_machine.SetBasicState(static_cast<StateTarget>(s)); }
            // set basic state
            auto SetExtraState(StateExtra s) noexcept { return m_machine.SetExtraState(static_cast<StateTarget>(s)); }
            // update with delta time
            auto Update(float t) noexcept { m_machine.Update(t); }
            // update without delta time
            auto Update() noexcept { this->Update(UIManager.GetDeltaTime()); }
            // is extra interface valid
            bool IsExtraInterfaceValid() const noexcept { return m_ifExtra.IsValid(); }
            // render
            void Render(const D2D1_RECT_F& rect) const noexcept {
                assert(m_ifBasic.IsValid() == true && "basic interface must be valid");
                if (m_ifExtra.IsValid()) m_ifExtra.Render(rect, static_cast<const AnimationStateMachine&>(m_machine));
                else m_ifBasic.Render(rect, static_cast<const AnimationStateMachine&>(m_machine));
            }
            // recreate
            auto Recreate() noexcept { 
                HRESULT hr = S_OK;
                if (SUCCEEDED(hr)) hr = m_ifBasic.Recreate();
                if (SUCCEEDED(hr)) hr = m_ifExtra.Recreate();
                return hr;
            }
        private:
            // basic machine
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
            // render for meta
            static void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm, const Meta metas[], uint16_t basic) noexcept;
            // render for color
            static void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm, const D2D1_COLOR_F colors[], uint16_t basic) noexcept;
            // render for brush
            static void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm, ID2D1Brush* const metas[], uint16_t basic) noexcept;
        };
        /// <summary>
        /// Graphics Interface for Opacity-Based Meta
        /// </summary>
        template<class GIConfig> class GIMeta {
            // constant
            enum : uint16_t {
                // count of basic state
                BASIC_COUNT = GIConfig::GetBasicCount(),
                // count of extra state
                EXTRA_COUNT = GIConfig::GetExtraCount(),
            };
        public:
            // ctor
            GIMeta(pugi::xml_node node, const char* prefix = nullptr) noexcept { GIConfig::InitMeta(node, prefix, m_metas, m_aID); }
            // dtor 
            ~GIMeta() noexcept = default;
            // recreate
            auto Recreate() noexcept { return GIHelper::Recreate(m_metas, m_aID, lengthof(m_metas)); }
            // check if valid
            bool IsValid() const noexcept { return m_aID[BASIC_COUNT - 1] != 0; }
            // render this
            void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm) const noexcept {
                return GIHelper::Render(rect, sm, m_metas, BASIC_COUNT);
            }
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
            enum : uint16_t {
                // count of basic state
                BASIC_COUNT = GIConfig::GetBasicCount(),
                // count of extra state
                EXTRA_COUNT = GIConfig::GetExtraCount(),
            };
        public:
            // ctor
            GIBurshRect(pugi::xml_node node, const char* prefix = nullptr) noexcept { GIConfig::InitBrush(node, prefix, m_apBrushes, m_aID); }
            // dtor 
            ~GIBurshRect() noexcept { GIHelper::Clean(m_apBrushes, lengthof(m_apBrushes)); }
            // check if valid (must be valid)
            bool IsValid() const noexcept { return true; }
            // recreate
            auto Recreate() noexcept { return GIHelper::Recreate(m_apBrushes, m_aID, lengthof(m_apBrushes)); }
            // render this
            void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm) const noexcept{
                return GIHelper::Render(rect, sm, m_apBrushes, BASIC_COUNT);
            }
        private:
            // brush
            ID2D1Brush*     m_apBrushes[STATE_COUNT];
            // brush id
            uint16_t        m_aID[STATE_COUNT];
        };
        /// <summary>
        /// Graphics Interface for color in rectangle
        /// </summary>
        template<class GIConfig> class GIColorRect {
            // constant
            enum : uint16_t {
                // count of basic state
                BASIC_COUNT = GIConfig::GetBasicCount(),
                // count of extra state
                EXTRA_COUNT = GIConfig::GetExtraCount(),
            };
        public:
            // ctor
            GIColorRect(pugi::xml_node node, const char* prefix = nullptr) noexcept { GIConfig::InitColor(node, prefix, colors); }
            // dtor 
            ~GIColorRect() noexcept = default;
            // recreate
            auto Recreate() noexcept { return S_OK; }
            // check if valid
            bool IsValid() const noexcept { return true; }
            // render this
            void Render(const D2D1_RECT_F& rect, const AnimationStateMachine& sm) const noexcept {
                return GIHelper::Render(rect, sm, colors, BASIC_COUNT);
            }
        public:
            // color s
            D2D1_COLOR_F            colors[BASIC_COUNT * EXTRA_COUNT];
        };
        /// <summary>
        /// Graphics Interface Config for button
        /// </summary>
        class GIConfigButton {
        public:
            // get count of basic state
            static constexpr auto GetBasicCount() noexcept { return size_t(STATE_COUNT); }
            // get count of extra state
            static constexpr auto GetExtraCount() noexcept { return size_t(1); }
            // meta initialize
            static void InitMeta(pugi::xml_node node, const char* prefix, Meta metas[], uint16_t ids[]) noexcept;
            // brush initialize
            static void InitBrush(pugi::xml_node node, const char* prefix, ID2D1Brush* brushes[], uint16_t ids[]) noexcept;
            // color initialize
            static void InitColor(pugi::xml_node node, const char* prefix, D2D1_COLOR_F color[]) noexcept;
        };
        /// <summary>
        /// Element for Button
        /// </summary>
        using Element4Button = AnimationStateMachineEx<GIBurshRect<GIConfigButton>, GIMeta<GIConfigButton>, ControlState>;
        /// <summary>
        /// Element for Bar
        /// </summary>
        using Element4Bar = AnimationStateMachineEx<GIColorRect<GIConfigButton>, GIMeta<GIConfigButton>, ControlState>;
    }
}