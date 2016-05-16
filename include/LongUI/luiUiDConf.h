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

#include "../luibase.h"
#include "../Core/luiInterface.h"
#include "luiUiConsl.h"

// longui namespace
namespace LongUI {
    // CUIDefaultConfigure, default impl for IUIConfigure
    class CUIDefaultConfigure : public IUIConfigure {
    public:
        // ctor
        CUIDefaultConfigure(CUIManager& manager, const char* log_file = nullptr) noexcept;
        // dtor
        ~CUIDefaultConfigure() noexcept;
        // = operator
        auto operator=(const CUIDefaultConfigure&)->CUIDefaultConfigure = delete;
    public:
        // add ref count
        virtual auto STDMETHODCALLTYPE AddRef() noexcept->ULONG override;
        // release
        virtual auto STDMETHODCALLTYPE Release() noexcept->ULONG override;
    public:
        // get flags for configure
        virtual auto GetConfigureFlag() noexcept ->ConfigureFlag override { return IUIConfigure::Flag_OutputDebugString; }
        // Get string from table
        virtual auto GetString(TableString tbl) noexcept -> const wchar_t*;
        // create interface
        virtual auto CreateInterface(const IID& iid, void** obj) noexcept ->HRESULT override;
        // get null-end string for template for creating control
        virtual auto GetTemplateString() noexcept ->const char* override { return nullptr; }
        // get locale name of ui(for text)
        virtual auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept ->void override { name[0] = L'\0'; };
        // add all custom controls
        virtual auto RegisterSome() noexcept ->void override {};
        // if use gpu render, you should choose a video card, return the index
        virtual auto ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept ->size_t override;
        // create custom window
        virtual auto CreateCustomWindow(WindowPriorityType type, pugi::xml_node node) noexcept->XUIBaseWindow* override { return nullptr; };
        // show the error string
        virtual auto ShowError(const wchar_t* str_a, const wchar_t* str_b = nullptr) noexcept -> void override;
#ifdef _DEBUG
        // output the debug string
        virtual auto OutputDebugStringW(DebugStringLevel level, const wchar_t* string, bool flush) noexcept -> void override;
    private:
        // create debug console
        void CreateConsole(DebugStringLevel level) noexcept;
#endif
    protected:
        // manager
        CUIManager&             m_manager;
        // ref-count
        uint32_t                m_cRef = 1;
        // unused
        uint32_t                m_unused = 233;
#ifdef _DEBUG
    private:
        // time tick
        size_t                  m_timeTick = 0;
        // log file string
        FILE*                   m_pLogFile = nullptr;
#endif
    public:
        // resource xml null-end-string
        const char*             resource = nullptr;
        // log file name in wchar_t*
        const wchar_t*          log_file_name = nullptr;
#ifdef _DEBUG
        // debug console
        CUIConsole              consoles[DLEVEL_SIZE];
#endif
    };
}

