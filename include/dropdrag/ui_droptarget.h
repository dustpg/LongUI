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

#ifndef LUI_NO_DROPDRAG
#include "ui_dropdrag_impl.h"

namespace LongUI {
    // window drop taget
    class CUIWndDropTarget final : public I::DropTarget {
    public:
        // add ref-count
        ULONG UNICALL AddRef() noexcept { return 2; };
        // release ref-count
        ULONG UNICALL Release() noexcept { return 1; };
        // query the interface
        HRESULT UNICALL QueryInterface(const IID&, void **ppvObject) noexcept override;
    public:
        // drag enter
        HRESULT UNICALL DragEnter(IDataObject*, DWORD, POINTL, DWORD*) noexcept override;
        // drag over
        HRESULT UNICALL DragOver(DWORD, POINTL, DWORD*) noexcept override;
        // drag leave
        HRESULT UNICALL DragLeave(void)  noexcept override;
        // drop
        HRESULT UNICALL Drop(IDataObject*, DWORD, POINTL, DWORD*) noexcept override;
    private:
    };
}
#endif
