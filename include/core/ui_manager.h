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

// debug
#include "../debugger/ui_debug.h"
// locker
#include "../thread/ui_locker.h"
// config
#include "../interface/ui_iconfig.h"
// cc
#include "ui_control_control.h"
// window manager
#include "ui_window_manager.h"
// resource manager
#include "ui_resource_manager.h"
// string view
#include "ui_string_view.h"
// ostype
#include "../util/ui_ostype.h"
// type
#include <type_traits>

// longui manager
#define UIManager (LongUI::CUIManager::GetInstance())

// ui namespace
namespace LongUI {
    // detail namespace
    namespace detail { 
        // ctor_dtor
        template<typename T> struct ctor_dtor;
        // private data for manager
        template<size_t> struct private_manager;
        // 32bit
        template<> struct private_manager<4> { enum { size = 624, align = 4 }; };
        // 64bit
        template<> struct private_manager<8> { enum { size = 680, align = 8 }; };
    }
    // input
    class CUIInputKM;
    // private
    struct PrivateManager;
    // config keeper, make sure config inited first
    struct ConfigKeeper { IUIConfigure* config; };
    // longui ui-manager
    class CUIManager final : 
        protected ConfigKeeper,
        public CUIDebug,
        public CUIResMgr, 
        public CUIWndMgr,
        public CUIControlControl {
        // refresh system info
        void refresh_system_info() noexcept;
    public:
        // error handle
        enum : uint32_t { ERROR_HANDLE = uint32_t(-1) };
        // input
        friend CUIInputKM;
        // ctor_dtor
        friend detail::ctor_dtor<CUIManager>;
        // interface for memory management
        struct IMM;
        // time capsules
        //using TimeCapsules = POD::Vector<CUITimeCapsule*>;
        // time capsule call
        //using TimeCapsuleCall = CUITimeCapsule::TimeCallBack;
        // flag
        using ConfigFlag = IUIConfigure::ConfigureFlag;
    public:
        // get instance
        static inline auto GetInstance() noexcept->CUIManager&;
        // delete later for control
        static void DeleteLater(UIControl&) noexcept;
        // initialize
        auto Initialize(IUIConfigure* config = nullptr) noexcept ->Result;
        // uninitialize
        void Uninitialize() noexcept;
        // do one frame
        void OneFrame() noexcept;
        // wait vblank
        void WaitForVBlank() noexcept;
        // need recreate_device
        void NeedRecreate() noexcept;
    public:
        // add [unique]/[release-free] text
        auto GetUniqueText(U8View pair) noexcept->const char*;
        // get [unique]/[release-free] text from handle
        auto GetUniqueTextFromHandle(uint32_t) noexcept->const char*;
        // get unique text handle
        auto GetUniqueTextHandle(const char*) noexcept->uint32_t;
        // create control within element name
        auto CreateControl(U8View element, UIControl* parent) noexcept->UIControl*;
    public:
        // call this on kill focus to manage input
        //void OnKillFocus() noexcept;
        // get wheel scroll lines
        auto GetWheelScrollLines() const noexcept { return m_uWheelScrollLines; }
        // get wheel scroll chars
        auto GetWheelScrollChars() const noexcept { return m_uWheelScrollChars; }
        // get delta time in ms
        auto GetDeltaTimeMs() const noexcept { return m_dwDeltaTime; }
        // get delta time for ui
        auto GetDeltaTime() const noexcept { return m_fDeltaTime; }
        // get app run time in ms
        //auto GetAppTimeTick() const noexcept { return m_dwTimeTick - m_cStartTick; }
        // lock data
        auto DataLock() noexcept { return m_uiDataLocker.Lock(); }
        // unlock data
        auto DataUnlock() noexcept { return m_uiDataLocker.Unlock(); }
        // lock rendering
        auto RenderLock() noexcept { return m_uiRenderLocker.Lock(); }
        // unlock rendering
        auto RenderUnlock() noexcept { return m_uiRenderLocker.Unlock(); }
    public:
        // exit
        void Exit() noexcept { this->config->Exit(); }
        // do platform main loop
        void MainLoop() noexcept { this->config->MainLoop(); }
        // show error with string
        bool ShowError(const wchar_t* a, const wchar_t* b) noexcept;
        // show error with result code
        bool ShowError(Result hr, const wchar_t* str_b = nullptr) noexcept;
    private:
        // ctor
        CUIManager(IUIConfigure* config, Result& out) noexcept;
        // ctor
        CUIManager(const CUIManager&) noexcept = delete;
        // ctor
        CUIManager(CUIManager&&) noexcept = delete;
        // dtor
        ~CUIManager() noexcept;
        // this manager
        inline auto this_() noexcept->CUIManager*;
    public:
#ifndef  NDEBUG
        // alloc counter: normal  [CUIManager::GetInstance().alloc_counter_n_dbg]
        std::size_t             alloc_counter_n_dbg = 0;
        // alloc counter: small   [CUIManager::GetInstance().alloc_counter_s_dbg]
        std::size_t             alloc_counter_s_dbg = 0;
#endif // ! NDEBUG
        // flag for configure
        ConfigFlag      const   flag;
    private:
        // delta time in sec.
        float                   m_fDeltaTime = 0.f;
        // app start tick
        //uint32_t                m_cStartTick = 0;
        // wheel scroll lines 
        uint32_t                m_uWheelScrollLines = 3;
        // wheel scroll chars
        uint32_t                m_uWheelScrollChars = 3;
        // tool window
        HWND                    m_hToolWnd = nullptr;
#ifndef NDEBUG
        // debug window
        CUIWindow*              m_pDebugWindow = nullptr;
#endif
        // data locker
        CUILocker               m_uiDataLocker;
        // rendering locker
        CUILocker               m_uiRenderLocker;
    protected:
        // try recreate_device all device resource
        void try_recreate() noexcept;
        // private manager
        auto&pm() noexcept { return reinterpret_cast<PrivateManager&>(m_private); }
        // private data
        std::aligned_storage<
            detail::private_manager<sizeof(void*)>::size,
            detail::private_manager<sizeof(void*)>::align
        >::type                 m_private;
    private:
        // recreate_device flag
        bool                    m_flagRecreate = false;
    };
    // auto data locker
    class CUIDataAutoLocker {
    public:
        // ctor
        CUIDataAutoLocker() noexcept { CUIManager::GetInstance().DataLock(); }
        // dtor
        ~CUIDataAutoLocker() noexcept { CUIManager::GetInstance().DataUnlock(); }
    };
    // auto dxgi(rendering) locker
    class CUIRenderAutoLocker {
    public:
        // ctor
        CUIRenderAutoLocker() noexcept { CUIManager::GetInstance().RenderLock(); }
        // dtor
        ~CUIRenderAutoLocker() noexcept { CUIManager::GetInstance().RenderUnlock(); }
    };
    /// <summary>
    /// single instance buffer for longui manager
    /// </summary>
    extern std::aligned_storage<sizeof(CUIManager), alignof(CUIManager)>::type s_bufManager;
    /// <summary>
    /// Gets the ui manager instance.
    /// 获取UI管理器实例
    /// </summary>
    /// <returns></returns>
    inline auto CUIManager::GetInstance() noexcept -> CUIManager & {
        return reinterpret_cast<CUIManager&>(s_bufManager);
    }
}

