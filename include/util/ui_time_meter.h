#pragma once
#include <cstdint>
#include "../util/ui_unimacro.h"

namespace LongUI {
    // get ms tick
    auto GetTimeTick() noexcept->uint32_t;
    // get system double click time
    auto GetDoubleClickTime() noexcept->uint32_t;
    // the time-meter - high
    class CUITimeMeterH {
    public:
        // QueryPerformanceFrequency
        static void QueryPerformanceFrequency(uint64_t& ll) noexcept;
        // QueryPerformanceCounter
        static void QueryPerformanceCounter(uint64_t& ll) noexcept;
        // refresh the frequency
        void inline RefreshFrequency() noexcept { QueryPerformanceFrequency(m_cpuFrequency); }
        // start timer
        void inline Start() noexcept { QueryPerformanceCounter(m_cpuCounterStart); }
        // move end var to start var
        void inline MovStartEnd() noexcept { m_cpuCounterStart = m_cpuCounterEnd; }
        // delta time in sec.
        template<typename T> auto inline Delta_s() noexcept {
            CUITimeMeterH::QueryPerformanceCounter(m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd - m_cpuCounterStart) / static_cast<T>(m_cpuFrequency);
        }
        // delta time in ms.
        template<typename T> auto inline Delta_ms() noexcept {
            CUITimeMeterH::QueryPerformanceCounter(m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd - m_cpuCounterStart)*static_cast<T>(1e3) / static_cast<T>(m_cpuFrequency);
        }
        // delta time in micro sec.
        template<typename T> auto inline Delta_mcs() noexcept {
            CUITimeMeterH::QueryPerformanceCounter(m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd - m_cpuCounterStart)*static_cast<T>(1e6) / static_cast<T>(m_cpuFrequency);
        }
    private:
        // CPU freq
        uint64_t            m_cpuFrequency;
        // CPU start counter
        uint64_t            m_cpuCounterStart = 0u;
        // CPU end counter
        uint64_t            m_cpuCounterEnd = 0u;
    public:
        // ctor
        CUITimeMeterH() noexcept { RefreshFrequency(); }
    };
    // the time-meter : medium
    class CUITimeMeterM {
    public:
        // refresh the frequency
        auto inline RefreshFrequency() noexcept { }
        // start timer
        auto inline Start() noexcept { m_dwStart = LongUI::GetTimeTick(); }
        // move end var to start var
        auto inline MovStartEnd() noexcept { m_dwStart = m_dwNow; }
        // delta time in sec.
        template<typename T> auto inline Delta_s() noexcept {
            m_dwNow = LongUI::GetTimeTick();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(0.001);
        }
        // delta time in ms.
        template<typename T> auto inline Delta_ms() noexcept {
            m_dwNow = LongUI::GetTimeTick();
            return static_cast<T>(m_dwNow - m_dwStart);
        }
        // delta time in micro sec.
        template<typename T> auto inline Delta_mcs() noexcept {
            m_dwNow = LongUI::GetTimeTick();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(1000);
        }
    private:
        // start time
        uint32_t                   m_dwStart = 0;
        // now time
        uint32_t                   m_dwNow = 0;
    public:
        // ctor
        CUITimeMeterM() noexcept { this->Start(); }
    };
    // time-meter for ui
    using CUITimeMeter = CUITimeMeterM;
}