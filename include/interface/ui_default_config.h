#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
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

// interfaces
#include "../interface/ui_iconfig.h"
// core type
#include "../core/ui_core_type.h"

// ui namespace
namespace LongUI {
    // CUIDefaultConfigure, default impl for IUIConfigure
    class CUIDefaultConfigure : public IUIConfigure {
    public:
        // ctor
        CUIDefaultConfigure() noexcept = default;
        // dtor
        ~CUIDefaultConfigure() noexcept = default;
    public:
#ifndef NDEBUG
        // get logfile name
        auto GetSimpleLogFileName() noexcept->CUIString override;
#endif
        // Get string from table
        //virtual auto GetString(TableString tbl) noexcept -> const wchar_t*;
        // get locale name of ui(for text)
        void GetLocaleName(char16_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept override;
        // get default font arg
        void DefaultFontArg(FontArg& arg) noexcept override;
        // add all controls
        void RegisterControl(ControlInfoList& list) noexcept override;
        // if use gpu render, you should choose a video card, return the index
        auto ChooseAdapter(const GraphicsAdapterDesc adapters[], const uint32_t length) noexcept ->uint32_t override;
        // load data from url on file not found
        void LoadDataFromUrl(U8View url_in_utf8, const CUIString& url_in_utf16, POD::Vector<uint8_t>& buffer) noexcept override;
        // on error-info lost
        void OnErrorInfoLost(Result, ErrorOccasion occ) noexcept override;
        // when OOM
        auto HandleOOM(size_t count, size_t size) noexcept ->CodeOOM override;
    public:
        // run a section script for event
        bool Evaluation(ScriptUI, UIControl&) noexcept override;
        // alloc the script memory and copy into(may be compiled into byte code)
        auto AllocScript(U8View) noexcept->ScriptUI override;
        // free the script memory
        void FreeScript(ScriptUI) noexcept override;
        // eval script for window init
        void Evaluation(U8View, CUIWindow&) noexcept override;
        // finalize window script if has script
        void FinalizeScript(CUIWindow&) noexcept override;
    public:
        // alloc for normal space
        void*NormalAlloc(size_t length) noexcept override;
        // free for normal space
        void NormalFree(void* address) noexcept override;
        // realloc for normal space
        void*NormalRealloc(void* address, size_t length) noexcept override;
        // alloc for small space
        void*SmallAlloc(size_t length) noexcept override;
        // free for small space
        void SmallFree(void* address) noexcept override;
    public:
        // Begins the render thread.
        auto BeginRenderThread() noexcept ->Result override;
        // Ends the render thread.
        void EndRenderThread() noexcept override;
        // Recursions the MSG loop.
        auto RecursionMsgLoop() noexcept ->uintptr_t override;
        // break msg-loop
        void BreakMsgLoop(uintptr_t) noexcept override;
    private:
        // render thread handle
        uintptr_t               m_hRenderThread = 0;
        // alloc tester
        uint16_t                m_u16Alloc = 0;
        // exit flag
        bool                    m_bExitFlag = false;
    };
}

