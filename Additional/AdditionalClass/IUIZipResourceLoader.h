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

#define LONGUI_WITH_DEFAULT_HEADER
#include <LongUI/LongUI.h>
#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"
// create loader
extern "C" HRESULT CreateZipResourceLoader(
    LongUI::CUIManager& manager,
    const wchar_t* file_name,
    LongUI::IUIResourceLoader** outdata
    ) noexcept;
// longui namespace
namespace LongUI {
    // ResourceLoader for Zip FIle
    class CUIZipResourceLoader : public  Helper::ComBase<
        Helper::QiListSelf<IUIInterface, Helper::QiList<IUIResourceLoader>>> {
    public:
        // ctor
        CUIZipResourceLoader(CUIManager& manager) noexcept;
        // dtor
        ~CUIZipResourceLoader() noexcept;
        // init
        auto Init(const wchar_t* file_name) noexcept->HRESULT;
    private:
        // manager for longui
        CUIManager&             m_manager;
        // zip archive file
        mz_zip_archive          m_zipFile;
    };
}

