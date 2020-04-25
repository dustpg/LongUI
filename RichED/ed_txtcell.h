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

// RichED namespace
namespace RichED {
    // text document
    class CEDTextDocument;
    // text cell
    class CEDTextCell : public CEDSmallObject, public Node {
    protected:
        // ctor
        CEDTextCell(CEDTextDocument&doc, const RichData&) noexcept;
        // dtor
        ~CEDTextCell() noexcept;
    public:
        // dispose cell
        void Dispose() noexcept;
        // disconnect and dispose
        void DisposeEx() noexcept { RemoveFromListOnly(*this); Dispose(); }
        // sleep
        void Sleep() noexcept;
        // get string data
        auto&RefString() const noexcept { return m_string; }
        // get string view
        auto View() const noexcept { return U16View{ m_string.data, m_string.data + m_string.length }; }
        // get meta info
        auto&RefMetaInfo() const noexcept { return m_meta; }
        // get logic length
        //auto GetLogicLength() const noexcept->uint32_t;
        // get riched data
        auto&RefRichED() const noexcept { return m_riched; }
        // set new riched
        void SetRichED(const RichData&) noexcept;
        // clean!
        void AsClean() noexcept { m_meta.dirty = false; }
        // dirty!
        void AsDirty() noexcept { m_meta.dirty = true; }
        // eol!
        void AsEOL() noexcept { m_meta.eol = true; }
        // !eol
        void ClearEOL() noexcept { m_meta.eol = false; }
        // get extra info
        auto GetExtraInfo() noexcept { return reinterpret_cast<InlineInfo*>(this + 1); }
    public:
        // split to 2 cells, return this if pos == 0, return next if pos >= len
        auto Split(uint32_t pos) noexcept->CEDTextCell*;
        // split to 2 cells, return this if pos == 0, create new if pos >= len
        auto SplitEx(uint32_t pos) noexcept->CEDTextCell*;
        // move eol to cell
        void MoveEOL(CEDTextCell& cell) noexcept;
        // merge this with next cell, return true if success
        bool MergeWithNext() noexcept;
        // remove text
        void RemoveText(Range) noexcept;
        // remove text - ex, call dispose-ex if remove all
        void RemoveTextEx(Range) noexcept;
        // insert text
        void InsertText(uint32_t pos, U16View) noexcept;
    protected:
        // riched-data
        RichData                m_riched;
    public:
        // document
        CEDTextDocument&        doc;
        // context
        CellContext             ctx;
        // metrics
        CellMetrics             metrics;
    protected:
        // metainfo
        CellMeta                m_meta;
        // base string
        FixedStringA            m_string;
    };
    // create a normal cell
    auto CreateNormalCell(CEDTextDocument& doc, const RichData&)->CEDTextCell*;
    // create a sharinked cell 
    auto CreateShrinkedCell(CEDTextDocument& doc, const RichData&)->CEDTextCell*;
}

