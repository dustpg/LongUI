#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
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

#include <cstdint>
#include "bc_txtplat.h"
#include "bc_txtstr.h"

// TextBC namespace
namespace TextBC {
    // text content
    struct IBCTextContent;
    // text document
    class CBCTextDocument;
    // text cell
    class CBCTextCell : public CBCSmallObject, public Node {
        // friend class
        //friend CBCTextDocument;
    public:
        // ctor
        CBCTextCell(CBCTextDocument& doc) noexcept;
        // dtor
        ~CBCTextCell() noexcept;
        // get content
        auto GetContent() const noexcept { return m_pContent; }
    public:
        // sleep
        void Sleep() noexcept;
        // awake
        void Awake(const char16_t*, uint32_t len) noexcept;
        // begin layout
        void BeginLayout() noexcept;
        // end layout
        void EndLayout() noexcept;
        // remove text, return true if CR/LF deleted
        bool RemoveText(Range) noexcept;
        // insert text
        auto InsertText(uint32_t, U16View) noexcept -> uint32_t;
        // remove text obly
        void RemoveTextOnly(Range) noexcept;
        // create new cell after this
        auto NewAfterThis() noexcept ->CBCTextCell*;
        // delete node
        void DeleteNode() noexcept;
        // find cell at begin of same line
        auto FindBeginSameLine() noexcept->CBCTextCell*;
        // find cell at end of same line
        auto FindEndSameLine() noexcept->CBCTextCell*;
    public:
        // just remove from list
        inline void RemoveFromListOnly() noexcept;
        // move EOL to next node
        inline void MoveEOL2Next() noexcept;
    public:
        // mark as eol
        void MarkAsEOL() noexcept;
        // mark as bol
        void MarkAsBOL() noexcept { m_bBeginOfLine = true; };
        // mark dirty
        void MarkDirty() noexcept { m_bDirty = true; }
        // follow bol
        void FollowBOL(const CBCTextCell& x) noexcept { m_bBeginOfLine = x.m_bBeginOfLine; }
        // is eol?
        bool IsEOL() const noexcept { return m_bEndOfLine; }
        // is bol?
        bool IsBOL() const noexcept { return m_bBeginOfLine; }
        // is dirty
        bool IsDirty() const noexcept { return m_bDirty; }
        // is last cell?
        bool IsLastCell() const noexcept { return !this->next->next; }
        // is first cell?
        bool IsFirstCell() const noexcept { return !this->prev->prev; }
        // get size
        auto GetSize() const noexcept { return m_size; }
        // get baseline offset
        auto GetBaseLine() const noexcept { return m_fBaseline; }
        // get string length
        auto GetStringLen() const noexcept { return m_text.size(); }
        // get char count
        auto GetCharCount() const noexcept { return m_cCharCount; }
        // get string ptr
        auto GetStringPtr() const noexcept { return m_text.c_str(); }
        // get string length no eol
        auto GetStrLenNoEOL() const noexcept -> uint32_t { 
            return GetStringLen() - (IsEOL() ? m_bCRLF + 1 : 0); }
    protected:
        // clear bol
        void clear_bol() noexcept { m_bBeginOfLine = false; }
        // clear eol
        void clear_eol() noexcept { m_bEndOfLine = false; }
    protected:
        // document
        CBCTextDocument&            m_document;
        // content
        IBCTextContent*             m_pContent = nullptr;
        // unit size of this cell
        SizeF                       m_size = SizeF{ 0 };
        // baseline offset
        float                       m_fBaseline = 0.f;
        // char count(char count != string len)
        uint16_t                    m_cCharCount = 0;
        // is crlf end for line
        bool                        m_bCRLF;
        // dirty
        bool                        m_bDirty : 1;
        // begin of line
        bool                        m_bBeginOfLine : 1;
        // end of line
        bool                        m_bEndOfLine : 1;
#ifndef NDEBUG
        // in layout
        bool                        m_bInLayout : 1;
#endif
        // text
        CBCString                   m_text;
    };
}



/// <summary>
/// Removes from list only.
/// </summary>
/// <returns></returns>
inline void TextBC::CBCTextCell::RemoveFromListOnly() noexcept {
    this->prev->next = this->next;
    this->next->prev = this->prev;
#ifndef NDEBUG
    this->prev = nullptr;
    this->next = nullptr;
#endif
}


#ifndef NDEBUG
void bc_assert_move_eol(TextBC::CBCTextCell&) noexcept;
#endif

/// <summary>
/// Moves the eo l2 next.
/// </summary>
/// <remarks>
/// EOL = end of line
/// </remarks>
/// <returns></returns>
inline void TextBC::CBCTextCell::MoveEOL2Next() noexcept {
#ifndef NDEBUG
    bc_assert_move_eol(*this);
#endif
    const auto next_cell = static_cast<CBCTextCell*>(this->next);
    next_cell->m_bEndOfLine = m_bEndOfLine;
    m_bEndOfLine = false;

}
