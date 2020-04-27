#pragma once
/**
* Copyright (c) 2018-2019 dustpg   mailto:dustpg@gmail.com
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


#include "ed_common.h"

// riched namespace
namespace RichED {
    // cell
    class CEDTextCell;
    // text platform
    struct PCN_NOVTABLE IEDTextPlatform {
        // on out of memory, won't be called on ctor
        virtual auto OnOOM(uint32_t retry_count, size_t try_alloc) noexcept ->HandleOOM = 0;
        // is valid password
        virtual bool IsValidPassword(char32_t) noexcept = 0;
        // append text
        virtual bool AppendText(CtxPtr ctx, U16View view) noexcept = 0;
        // write to file
        virtual bool WriteToFile(CtxPtr, const uint8_t data[], uint32_t len) noexcept = 0;
        // read from file
        virtual bool ReadFromFile(CtxPtr, uint8_t data[], uint32_t len) noexcept = 0;
        // recreate context
        virtual void RecreateContext(CEDTextCell& cell/*, U16View real*/) noexcept = 0;
        // delete context
        virtual void DeleteContext(CEDTextCell&) noexcept = 0;
        // draw context
        virtual void DrawContext(CtxPtr,CEDTextCell&, unit_t baseline) noexcept = 0;
        // hit test
        virtual auto HitTest(CEDTextCell&, unit_t offset) noexcept->CellHitTest = 0;
        // get char metrics
        virtual auto GetCharMetrics(CEDTextCell&, uint32_t pos) noexcept ->CharMetrics =0;
#ifndef NDEBUG
        // debug output
        virtual void DebugOutput(const char*, bool high) noexcept = 0;
#endif
    };
}
