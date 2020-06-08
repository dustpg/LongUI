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
#include "ed_txtbuf.h"
#include "ed_undoredo.h"
#include <cstddef>

// riched namespace
namespace RichED {
    // text platform
    struct IEDTextPlatform;
    // text cell
    class CEDTextCell;
    // logic line(LL) data
    struct LogicLine {
        // first cell
        CEDTextCell*    first;
        // text length, LF not included
        uint32_t        length;
    };
    // visual line(VL) data
    struct VisualLine {
        // first cell
        CEDTextCell*    first;
        // logic line 
        uint32_t        lineno;
        // char offset for this visual-line
        uint32_t        char_len_before;
        // char length for this visual-line
        uint32_t        char_len_this;
        // offset for this visual-line
        unit_t          offset;
        // max ascender-height in this visual-line
        unit_t          ar_height_max;
        // max deascender-height in this visual-line
        unit_t          dr_height_max;
    };
    // value changed flag
    enum ValuedChanged : uint32_t;
    // text document
    class CEDTextDocument {
        // flag set private
        enum flag_set : uint32_t { set_effect = 0 << 2, set_fflags = 1 << 2 };
        // set riched
        bool set_riched(
            DocPoint begin, DocPoint end,
            uint32_t offset, uint32_t size,
            const void* data, bool relayout
        ) noexcept;
        // set flags
        bool set_flags(
            DocPoint begin, DocPoint end,
            uint16_t flags, uint32_t set
        ) noexcept;
        // gui: set riched
        bool gui_riched(
            uint32_t offset, uint32_t size,
            const void* data, bool relayout
        ) noexcept;
        // set flags
        bool gui_flags(uint16_t flags, uint32_t set) noexcept;
        // gui password
        bool gui_password(U16View view) noexcept;
        // recreate the context
        void recreate_context(CEDTextCell& cell) noexcept;
        // get view
        static auto get_view(const CEDTextCell& cell) noexcept ->U16View;
        // password helper ->u16 
        static auto password_helper16(char32_t buf[], char32_t ch, bool mode, const CEDTextCell&) noexcept ->U16View;
    public:
        // private impl
        struct Private; struct UndoPri;
        // ctor
        CEDTextDocument(IEDTextPlatform&, const DocInitArg&) noexcept;
        // ctor
        ~CEDTextDocument() noexcept;
        // no copy ctor
        CEDTextDocument(const CEDTextDocument&) noexcept = delete;
        // alloc mem
        void*Alloc(size_t) noexcept;
        // update
        auto Update() noexcept->ValuedChanged;
        // render
        void Render(CtxPtr) noexcept;
        // move current doc view-point pos[relatively]
        void MoveViewportRel(Point) noexcept;
        // set doc view-point pos[absolutely]
        void MoveViewportAbs(Point) noexcept;
        // resize doc view-zone
        void ResizeViewport(Size) noexcept;
        // save to bin-file for self-use
        bool SaveBinFile(CtxPtr) noexcept;
        // load from bin-file for self-use
        bool LoadBinFile(CtxPtr) noexcept;
        // gen text
        void GenText(CtxPtr ctx, DocPoint begin, DocPoint end)noexcept;
        // get estimated size in view space
        auto GetEstimatedSize() const noexcept -> Size;
        // get logic line count 
        auto GetLogicLineCount() const noexcept { return m_vLogic.GetSize(); }
        // get selection
        auto&RefSelection() const noexcept { return m_vSelection; }
        // get caret rect under doc space
        auto GetCaret() const noexcept { return m_rcCaret; };
        // get line feed data
        auto&RefLineFeed() const noexcept { return m_linefeed; }
        // get info
        auto&RefInfo() const noexcept { return m_info; }
        // get matrix
        auto&RefMatrix() const noexcept { return m_matrix; }
        // set new line feed
        void SetLineFeed(LineFeed) noexcept;
        // get selection
        auto GetSelectionRange() const noexcept { return DocRange{ m_dpSelBegin, m_dpSelEnd }; }
        // force change all riched
        void ForceResetAllRiched() noexcept;
    public: // Low level 
        // begin an operation for undo-stack
        void BeginOp() noexcept;
        // end an operation for undo-stack
        void EndOp() noexcept;
        // insert inline object
        bool InsertInline(DocPoint dp, const InlineInfo&, int16_t len, CellType type) noexcept;
        // insert ruby
        bool InsertRuby(DocPoint, char32_t, U16View, const RichData* = nullptr) noexcept;
        // insert text, pos = min(DocPoint::pos, line-length)
        auto InsertText(DocPoint, U16View, bool behind =true) noexcept ->DocPoint;
        // remove text, pos = min(DocPoint::pos, line-length)
        bool RemoveText(DocPoint begin, DocPoint end) noexcept;
    public: // Rich Text Format
        // type ref
        using color_t = decltype(RichData::color);
        using fname_t = decltype(RichData::name);
        enum FlagSet : uint32_t { Set_False = 0 << 0, Set_True = 1 << 0, Set_Change = 1 << 1 };
        // set riched
        bool SetRichED(DocPoint begin, DocPoint end, const RichData& rd) noexcept {
            return set_riched(begin, end, 0, sizeof(RichData), &rd, true); }
        // set font size
        bool SetFontSize(DocPoint begin, DocPoint end, const unit_t& fs) noexcept {
            return set_riched(begin, end, offsetof(RichData, size), sizeof(fs), &fs, true); }
        // set font color
        bool SetFontColor(DocPoint begin, DocPoint end, const color_t& fc) noexcept {
            return set_riched(begin, end, offsetof(RichData, color), sizeof(fc), &fc, false); }
        // set font name
        bool SetFontName(DocPoint begin, DocPoint end, const fname_t& fn) noexcept {
            return set_riched(begin, end, offsetof(RichData, name), sizeof(fn), &fn, true); }
        // set under line
        bool SetUnerline(DocPoint begin, DocPoint end, FlagSet set) noexcept {
            return set_flags(begin, end, Effect_Underline, set | set_effect); }
        // set italic
        bool SetItalic(DocPoint begin, DocPoint end, FlagSet set) noexcept {
            return set_flags(begin, end, FFlags_Italic, set | set_fflags); }
    public: // UndoRedo-call-level 
        // force set & update caret anchor
        void SetAnchorCaret(DocPoint anchor, DocPoint caret) noexcept;
        // Rank-Up-Magic for ruby
        void RankUpMagic(DocPoint, uint32_t) noexcept;
        // Rank-Up-Magic for objs
        void RankUpMagic(DocPoint, const InlineInfo&, int16_t len, CellType type) noexcept;
    public: // GUI Operation, return false on gui-level mistake
        // gui: l-button up
        //bool GuiLButtonUp(Point pt) noexcept;
        // gui: l-button down
        bool GuiLButtonDown(Point pt, bool shift) noexcept;
        // gui: l-button hold&move
        bool GuiLButtonHold(Point pt) noexcept;
        // gui: char
        bool GuiChar(char32_t ch) noexcept;
        // gui: text
        bool GuiText(U16View view) noexcept;
        // gui: return/enter
        bool GuiReturn() noexcept;
        // gui: ruby
        bool GuiRuby(char32_t, U16View, const RichData* = nullptr) noexcept;
        // gui: inline object
        bool GuiInline(const InlineInfo&, int16_t len, CellType type) noexcept;
        // gui: backspace
        bool GuiBackspace(bool ctrl) noexcept;
        // gui: delete
        bool GuiDelete(bool ctrl) noexcept;
        // gui: left
        bool GuiLeft(bool ctrl, bool shift) noexcept;
        // gui: right
        bool GuiRight(bool ctrl, bool shift) noexcept;
        // gui: up
        bool GuiUp(bool ctrl, bool shift) noexcept;
        // gui: down
        bool GuiDown(bool ctrl, bool shift) noexcept;
        // gui: home
        bool GuiHome(bool ctrl, bool shift) noexcept;
        // gui: end
        bool GuiEnd(bool ctrl, bool shift) noexcept;
        // gui: page up
        bool GuiPageUp(bool ctrl, bool shift) noexcept;
        // gui: page down
        bool GuiPageDown(bool ctrl, bool shift) noexcept;
        // gui: select all
        bool GuiSelectAll() noexcept;
        // gui: scroll view
        bool GuiSrcollView(unit_t, bool shift_direction) noexcept;
        // gui: undo
        bool GuiUndo() noexcept;
        // gui: redo
        bool GuiRedo() noexcept;
        // gui: check if has text
        bool GuiHasText() const noexcept;
    public: // GUI Operation - for Rich Text
        // set riched
        bool GuiRichED(const RichData& rd) noexcept {
            return gui_riched(0, sizeof(RichData), &rd, true); }
        // set font size
        bool GuiFontSize(const unit_t& fs) noexcept {
            return gui_riched(offsetof(RichData, size), sizeof(fs), &fs, true); }
        // set font color
        bool GuiFontColor(const color_t& fc) noexcept {
            return gui_riched(offsetof(RichData, color), sizeof(fc), &fc, false); }
        // set font name
        bool GuiFontName(const fname_t& fn) noexcept {
            return gui_riched(offsetof(RichData, name), sizeof(fn), &fn, true); }
        // set under line
        bool GuiUnerline(FlagSet set) noexcept {
            return gui_flags(Effect_Underline, set | set_effect); }
        // set italic
        bool GuiItalic(FlagSet set) noexcept {
            return gui_flags(FFlags_Italic, set | set_fflags); }
    public: // helper
        // valign helper h
        void VAlignHelperH(unit_t ar, unit_t height, CellMetrics& m) noexcept;
    public:
        // platform
        IEDTextPlatform&        platform;
        // default riched
        RichData                default_riched;
    private:
        // undo stack
        CEDUndoRedo             m_undo;
        // matrix
        DocMatrix               m_matrix;
        // normal info
        DocInfo                 m_info;
        // linefeed data
        LineFeed                m_linefeed;
        // viewport
        Rect                    m_rcViewport;
        // caret rect
        Rect                    m_rcCaret;
        // document estimate size
        Size                    m_szEstimated;
        // document estimate size - compare ver
        Size                    m_szEstimatedCmp;
        // anchor pos
        DocPoint                m_dpAnchor;
        // caret pos
        DocPoint                m_dpCaret;
        // selection begin
        DocPoint                m_dpSelBegin;
        // selection end
        DocPoint                m_dpSelEnd;
        // undo op
        uint16_t                m_uUndoOp = 0;
        // undo ok
        uint16_t                m_uUndoIsOk = 1;
        // changed flag
        uint16_t                m_flagChanged = 0;
        // password UCS4 mode
        bool                    m_bPassword4 = false;
        // debug bool value for update
        bool                    m_bUpdateDbg = false;
        // head
        Node                    m_head;
        // tail
        Node                    m_tail;
#ifndef NDEBUG
        // debug buffer to avoid head/tail node as a cell
        char                    m_dbgBuffer[128];
#endif
        // visual lines cache
        CEDBuffer<VisualLine>   m_vVisual;
        // logic line data
        CEDBuffer<LogicLine>    m_vLogic;
        // selection data
        CEDBuffer<Box>          m_vSelection;
    public:
        // password helper - string-view
        template<typename T>
        auto PWHelperView(T call, const CEDTextCell& cell) noexcept {
            char32_t buffer[TEXT_CELL_STR_MAXLEN + 1];
            U16View view = this->get_view(cell);
            if (m_info.flags & Flag_UsePassword) 
                view = this->password_helper16(buffer, m_info.password_cha16x2, m_bPassword4, cell);
            return call(view);
        }
        // password helper - code-pos
        auto PWHelperPos(const CEDTextCell& cell, uint32_t pos) noexcept ->uint32_t;
        // password helper - code-hit
        void PWHelperHit(const CEDTextCell& cell, CellHitTest& hit) noexcept;
    };
    // value changed
    enum ValuedChanged : uint32_t {
        // view changed, need redraw
        Changed_View            = 1 << 0,
        // selection changed        
        Changed_Selection       = 1 << 1,
        // caret changed            
        Changed_Caret           = 1 << 2,
        // text changed            
        Changed_Text            = 1 << 3,
        // estimated width  changed
        Changed_EstimatedWidth  = 1 << 4,
        // estimated height changed
        Changed_EstimatedHeight = 1 << 5,
        // viewport width changed
        Changed_ViewportWidth   = 1 << 6,
        // viewport height changed
        Changed_ViewportHeight  = 1 << 7,
    };
}
