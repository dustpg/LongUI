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

#include <Windows.h>
#include <cstdint>

// longui namespace
namespace LongUI {
    // UI Locker
    class CUILocker {
    public:
#if 1
        // ctor
        CUILocker() noexcept { ::InitializeCriticalSection(&m_cs); }
        // dtor
        ~CUILocker() noexcept { ::DeleteCriticalSection(&m_cs); }
        // lock
        auto Lock() noexcept { 
#if defined(_DEBUG) || defined(LONGUI_LOCKER_DEBUG)
            if (!::TryEnterCriticalSection(&m_cs)) {
                void longui_dbg_locked(const LongUI::CUILocker&);
                longui_dbg_locked(*this);
                ::EnterCriticalSection(&m_cs);
            }
#else
            ::EnterCriticalSection(&m_cs);
#endif
        }
        // unlock
        auto Unlock() noexcept { ::LeaveCriticalSection(&m_cs); }
    private:
        // cs
        CRITICAL_SECTION        m_cs;
#else
        // lock
        auto Lock() noexcept { m_mux.lock(); }
        // unlock
        auto Unlock() noexcept { m_mux.unlock(); }
    private:
        // mutex
        std::recursive_mutex    m_mux;
#endif
    };

    // the time-meter - high
    class CUITimeMeterH {
    public:
        // QueryPerformanceCounter
        static inline auto QueryPerformanceCounter(LARGE_INTEGER* ll) noexcept {
            auto old = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
            auto r = ::QueryPerformanceCounter(ll);
            ::SetThreadAffinityMask(::GetCurrentThread(), old);
            return r;
        }
        // refresh the frequency
        auto inline RefreshFrequency() noexcept { ::QueryPerformanceFrequency(&m_cpuFrequency); }
        // start timer
        auto inline Start() noexcept { CUITimeMeterH::QueryPerformanceCounter(&m_cpuCounterStart); }
        // move end var to start var
        auto inline MovStartEnd() noexcept { m_cpuCounterStart = m_cpuCounterEnd; }
        // delta time in sec.
        template<typename T> auto inline Delta_s() noexcept {
            CUITimeMeterH::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in ms.
        template<typename T> auto inline Delta_ms() noexcept {
            CUITimeMeterH::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e3) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in micro sec.
        template<typename T> auto inline Delta_mcs() noexcept {
            CUITimeMeterH::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e6) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
    private:
        // CPU freq
        LARGE_INTEGER            m_cpuFrequency;
        // CPU start counter
        LARGE_INTEGER            m_cpuCounterStart;
        // CPU end counter
        LARGE_INTEGER            m_cpuCounterEnd;
    public:
        // ctor
        CUITimeMeterH() noexcept { m_cpuCounterStart.QuadPart = 0; m_cpuCounterEnd.QuadPart = 0; RefreshFrequency(); }
    };
    // the time-meter : medium
    class CUITimeMeterM {
    public:
        // refresh the frequency
        auto inline RefreshFrequency() noexcept { }
        // start timer
        auto inline Start() noexcept { m_dwStart = ::timeGetTime(); }
        // move end var to start var
        auto inline MovStartEnd() noexcept { m_dwStart = m_dwNow; }
        // delta time in sec.
        template<typename T> auto inline Delta_s() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(0.001);
        }
        // delta time in ms.
        template<typename T> auto inline Delta_ms() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart);
        }
        // delta time in micro sec.
        template<typename T> auto inline Delta_mcs() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(1000);
        }
    private:
        // start time
        DWORD                   m_dwStart = 0;
        // now time
        DWORD                   m_dwNow = 0;
    public:
        // ctor
        CUITimeMeterM() noexcept { this->Start(); }
    };
    // time-meter for ui
    using CUITimeMeter = CUITimeMeterM;
    // windows error code to HRESULT
    inline auto WinCode2HRESULT(UINT x) noexcept ->HRESULT {
        return ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT)(((x)& 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)));
    }
}

