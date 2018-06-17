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

#include "bc_txtcell.h"
#include "bc_txtplat.h"
#include "bc_txtline.h"
#include "bc_txtbuf.h"

// TextBC namespace
namespace TextBC {
    // dirty rect
    struct DirtyRect { uint32_t x, y, width, height; };
    // Dirty rectangles
    using DirtyRects = CBCBuffer<DirtyRect>;
    // text document
    class CBCTextDocument {
        // lines_t
        using lines_t = CBCBuffer<TextLineData>;
        // selection_t
        using selection_t = CBCBuffer<RectWHF>;
        // until call
        struct until_call {
            // function ptr
            bool (*func)(void* this_ptr, uint64_t) noexcept;
        };
    public:
        // flag
        enum Flag : uint16_t {
            // none flag
            Flag_None = 0,
            // None CR-LF mode(LF Mode)
            Flag_NoneCRLR = 1 << 0,
            // [unsupported] alllow dragging 
            Flag_AllowDrag = 1 << 1,
            // [unsupported] rich text
            Flag_RichText = 1 << 2,
            // read only
            Flag_ReadOnly = 1 << 3,
            // multi line
            Flag_MultiLine = 1 << 4,
            // password mode
            Flag_UsePassword = 1 << 5,
            // [unsupported] auto scroll if caret move out
            Flag_AutoScroll = 1 << 6,
        };
    protected:
        // find func return value
        struct find_rv {
            // line data
            const TextLineData*     line;
            // cell data
            CBCTextCell*            cell;
            // char count to the cell
            uint32_t                char_count;
            // string len to the cell
            uint32_t                string_len;
        };
    public:
        // init arg
        struct InitArgs {
            // max text length
            uint32_t    max_length;
            // flag
            Flag        flag;
            // password char
            char16_t    password;
        };
    public:
        // ctor
        CBCTextDocument(IBCTextPlatform&, InitArgs) noexcept;
        // no copy ctor
        CBCTextDocument(const CBCTextDocument&) noexcept = delete;
        // no move ctor
        CBCTextDocument(CBCTextDocument&&) noexcept = delete;
        // dtor
        ~CBCTextDocument() noexcept;
    public:
        // render, pass null for DirtyRects if didn't support
        void Render(void* context, DirtyRects* =nullptr) noexcept;
        // get flag
        auto GetFlag() const noexcept { return m_flag; }
        // is CRLF?
        bool IsCRLF() const noexcept { return m_bCRLF; }
        // get string length
        auto GetStringLength() const noexcept { return m_cTotalLen; }
        // get password-char
        auto GetPasswordChar() const noexcept { return m_chPassword; }
        // get viewport pos
        auto GetViewportPos() const noexcept { return m_ptViewport; }
        // get viewport size
        auto GetViewportSize() const noexcept { return m_szViewport; }
        // get content size
        auto GetContentSize() const noexcept { return m_szContent; }
    public:
        // request range
        template<class T> void RequestRange(T& string, Range range) noexcept {
            return this->request_range(&string, range);}
        // request selected
        template<class T> void RequestSelected(T& string) noexcept {
            return this->request_range(&string, GetSelectionRange());}
        // request text
        template<class T> void RequestText(T& string) noexcept {
            return this->request_text(&string);}
        // set text
        void SetText(U16View view) noexcept;
        // is text changed
        auto IsTextChanged() const noexcept { return m_textChanged; }
        // clear text changed
        void ClearTextChanged() noexcept { m_textChanged = false; }
        // is selection changed
        auto IsSelectionChanged() const noexcept { return m_selectionChanged; }
        // clear selection changed
        void ClearSelectionChanged() noexcept { m_selectionChanged = false; }
    public:
        // on lbutton up
        void OnLButtonUp(Point2F pt) noexcept;
        // on lbutton down
        void OnLButtonDown(Point2F pt, bool shift) noexcept;
        // on lbutton hold&move
        void OnLButtonHold(Point2F pt) noexcept;
        // on char
        void OnChar(char32_t ch) noexcept;
        // on text
        void OnText(U16View view) noexcept;
        // on newline, return true if single line mode
        bool OnNewLine() noexcept;
        // on backspace
        void OnBackspace(bool ctrl) noexcept;
        // on delete
        void OnDelete(bool ctrl) noexcept;
        // on left
        void OnLeft(bool ctrl, bool shift) noexcept;
        // on right
        void OnRight(bool ctrl, bool shift) noexcept;
        // on up
        void OnUp(bool shift) noexcept;
        // on down
        void OnDown(bool shift) noexcept;
        // on select all
        void OnSelectAll() noexcept;
        // on home
        void OnHome(bool ctrl, bool shift) noexcept;
        // on end
        void OnEnd(bool ctrl, bool shift) noexcept;
    public:
        // get selection
        auto GetSelectionRange() const noexcept->Range;
        // delete selection
        void DeleteSelection() noexcept;
        // remove text
        void RemoveText(Range) noexcept;
        // insert text
        void InsertText(uint32_t pos, U16View view) noexcept;
        // hit test
        auto HitTest(Point2F) noexcept ->HitTest;
        // set selection
        void SetSelection(uint32_t pos, bool keep_anchor) noexcept;
        // set selection from point
        void SetSelection(Point2F pos, bool keep_anchor) noexcept;
        // set viewport size
        void SetViewportSize(SizeF) noexcept;
        // set viewport pos
        void SetViewportPos(Point2F) noexcept;
        // sleep, free some cached memory
        void Sleep() noexcept;
#ifdef TBC_UNDOREDO
        // clear undo stack
        void ClearUndoStack() noexcept;
        // undo
        void Undo() noexcept;
        // redo
        void Redo() noexcept;
        // can undo
        bool CanUndo() const noexcept { return m_pUndoNow != &m_undoStackBottom; }
        // can redo
        bool CanRedo() const noexcept { return m_pUndoNow != m_pUndoStackTop; }
        // mark end of operation
        void MarkEndOfOperation() noexcept;
#else
        // clear undo stack
        void ClearUndoStack() noexcept {}
        void Undo() noexcept {}
        // redo
        void Redo() noexcept {}
        // can undo
        bool CanUndo() const noexcept { return false; }
        // can redo
        bool CanRedo() const noexcept { return false; }
        // mark end of operation
        void MarkEndOfOperation() noexcept {}
#endif
    protected:
        // remove text
        void remove_text(Range) noexcept;
        // remove all
        void remove_all() noexcept;
        // insert text
        void insert_text(uint32_t pos, U16View view) noexcept;
        // find insert node
        auto find_insert_node(uint32_t, U16View, float&, uint32_t&) noexcept->CBCTextCell*;
        // sync cache until <T>
        void sync_cache_until(until_call call, uint64_t) noexcept;
        // sync cache to length
        void sync_cache_to_length(uint32_t pos) noexcept;
        // sync cache to offset
        void sync_cache_to_offset(double offset) noexcept;
        // split cell
        void split_cell(TextLineData& line, CBCTextCell& node, uint32_t pos) noexcept;
        // refresh selection metrics
        void refresh_selection_metrics(Range) noexcept;
        // adjust content size + 
        void adjust_content_size_insert(float, CBCTextCell*, CBCTextCell*) noexcept;
        // adjust content size - 
        void adjust_content_size_remove(float, CBCTextCell* first, CBCTextCell* last) noexcept;
        // recalculate content size
        void recalculate_content_size() noexcept;
        // free list
        void free_mem_list() noexcept;
        // delete selection only
        void delete_selection() noexcept;
        // request text
        void request_text(void* string) noexcept;
        // request range
        void request_range(void* string, Range range) noexcept;
    protected:
        // get char metrics
        auto char_metrics(CBCTextCell& cell, uint32_t offset) noexcept->CharMetrics;
        // find cell or after cell or last cell by pos
        auto find_cell_or_la_by_pos(uint32_t pos) const noexcept->find_rv;
        // find last valid cell
        auto find_last_valid_cell() const noexcept->find_rv;
        // set selection
        void set_selection(uint32_t, uint32_t pos) noexcept;
        // update caret rect
        void update_caret_rect() noexcept;
        // align caret to nearest cluster
        //void align_carent_to_cluster() noexcept;
        // hit test at position
        auto hittest_at(uint32_t) noexcept ->const TextLineData*;
        // get string length before
        auto get_strlen_before() const noexcept->uint32_t;
        // get string length behind
        auto get_strlen_behind() const noexcept->uint32_t;
        // play beep
        void play_beep() noexcept;
        // truncation text
        void truncation_text(U16View&, ptrdiff_t) noexcept;
        // add line data
        static auto add_line(TextLineData&, const TextLineData&, CBCTextCell* node) noexcept->CBCTextCell*;
        // truncation password
        static void truncation_password(IBCTextPlatform&, U16View&) noexcept;
        // truncation singleline
        static void truncation_singleline(U16View&) noexcept;
        // relayout cell
        static void relayout_cell(CBCTextCell& cell) noexcept;
    private:
        // mark text changed
        inline void text_changed() noexcept { m_textChanged = true; }
        // mark selection changed
        inline void selection_changed() noexcept { m_selectionChanged = true; }
        // get absolute position
        inline auto get_abs_pos() const noexcept->uint32_t { return m_uCaretPos /*+ m_uCaretOffset*/; }
        // valid length
        inline auto cache_valid_length() const noexcept->uint32_t;
        // valid offset
        inline auto cache_valid_offset() const noexcept->double;
        // clear hittest cache
        inline void clear_last_hittest() noexcept;
        // cache hittest data
        inline void cache_last_hittest(CBCTextCell* c, uint32_t p, Point2F) noexcept;
#ifdef TBC_UNDOREDO
    private:
        // free undo/redo stack
        void free_undo_stack() noexcept;
        // insert text for undo/redo node
        void insert_text_unre(const void*) noexcept;
        // remove text for undo/redo node
        void remove_text_unre(const void*) noexcept;
        // add remove text for undo/redo
        void add_remove_text_unre(uint32_t pos, U16View) noexcept;
        // add insert text for undo/redo
        void add_insert_text_unre(uint32_t pos, U16View) noexcept;
        // operation with length
        void*operation_length(uint32_t len) noexcept;
        // free prev
        static void free_prev(void* ptr) noexcept;
#endif
    private:
#ifdef NDEBUG
        // debug outout anchor
        void debug_anchor() noexcept {}
#else
        // debug outout anchor
        void debug_anchor() noexcept;
#endif
    public:
#ifdef NDEBUG
        // debug outout undo-stack
        void DebugOutUndoStack() noexcept {}
#else
        // debug outout undo-stack
        void DebugOutUndoStack() noexcept;
#endif
    public:
        // platform
        IBCTextPlatform&    platform;
    protected:
        // text cell head
        Node                m_head = Node{ nullptr, &m_tail };
        // text cell tail
        Node                m_tail = Node{ &m_head, nullptr};
        // total string len
        uint32_t            m_cTotalLen = 0;
        // total char count
        uint32_t            m_cTotalCount = 0;
        // max char count
        uint32_t    const   m_cMaxLen;
        // flag
        Flag        const   m_flag;
        // password char
        char16_t    const   m_chPassword;
        // line data
        lines_t             m_lines;
        // selection data
        selection_t         m_selections;
        // valid line count
        uint32_t            m_cValidLine = 0;
        // XXX:caret rect
        RectWHF             m_rcCaret = RectWHF{};
        // caret line real[+1s]
        uint32_t            m_caretLineReal = 1;
        // caret line temp[+1s]
        uint32_t            m_caretLineTemp = 1;
        // last selection range
        Range               m_lastSelection{ 0, 0 };
        // mouse down pos
        Point2F             m_ptMoseDown{ -1.f, -1.f };
        // viewport pos
        Point2F             m_ptViewport{ 0.f, 0.f };
        // content size
        SizeF               m_szContent{ 0.f, 0.f };
        // viewport size
        SizeF               m_szViewport{ 100.f, 30.f };
        // anchor positon               
        uint32_t            m_uAnchor = 0;
        // caret positon                
        uint32_t            m_uCaretPos = 0;
        // draw caret?
        bool                m_bDrawCaret = false;
        // crlf?
        bool                m_bCRLF = true;
        // click in selection?
        bool                m_bClickInSelection : 1;
        // text changed
        bool                m_textChanged : 1;
        // selection changed
        bool                m_selectionChanged : 1;

#ifdef TBC_UNDOREDO
        // undo-redo mode
        bool                m_recordUndoRedo : 1;
#endif

        // begin line(+1) for viewport
        uint32_t            m_beginLineVisible = 1;
        // unused u32
        uint32_t            m_unused = 0;

        // cached hittest start
        uint32_t            m_lastHitStart = 0xffff;
        // cached hittest pos
        Point2F             m_lastHitCellPos{ -1.f, -1.f };
        // cached hittest cell
        CBCTextCell*        m_lastHitTest = nullptr;

        // free list
        CBCTextCell*        m_pFreeList = nullptr;
#if 0
        // LIMITED BUFFER begin
        Node*               m_pBufferBegin = &m_tail;
        // LIMITED BUFFER end
        Node*               m_pBufferEnd = &m_tail;
        // LIMITED BUFFER max count
        uint32_t            m_cBufferMax = 128;
        // LIMITED BUFFER cell count
        uint32_t            m_cBufferCount = 0;
#endif
#ifdef TBC_UNDOREDO
        // length for dynamic-length buffer
        uint32_t            m_undoDynamic = 0;
        // length for undo/redo stack-to-now
        uint32_t            m_undoStackLen = 0;
        // undo/redo stack-top
        void*               m_pUndoStackTop = &m_undoStackBottom;
        // undo/redo stack-now
        void*               m_pUndoNow = &m_undoStackBottom;
        // bottom of list
        Node                m_undoStackBottom = { nullptr };
#ifndef NDEBUG
        // debug buffer
        int32_t             m_debugBuf[8] = { -1, -2, -3, -4, -5, -6, -7, -8 };
#endif
#endif
    public:
        enum {
            // init reserve line count
            INIT_RESERVE_LINE = 10,
        };
    };
}
