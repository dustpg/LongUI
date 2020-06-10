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

#include "ed_config.h"
#include <cstdint>




// --------------------------------------------------------
//                     ENUM LIST
// --------------------------------------------------------

#define RED_FLAG_OP(e, t) inline e operator|(e a, e b) noexcept { return e(t(a) | t(b)); }

// RichED namespace
namespace RichED {
    // context pointer
    using CtxPtr = void*;
    // const value
    enum Const : uint32_t {
        // max line count
        MAX_LINE_COUNT = uint32_t(-1),
    };
    // HandleOOM
    enum HandleOOM : uint32_t {
        // ignore
        OOM_Ignore = 0,
        // retry
        OOM_Retry,
        // noreturn
        OOM_NoReturn,
    };
    // VerticalAlign
    enum VerticalAlign : uint16_t {
        // baseline
        VAlign_Baseline = 0,
        // ascender
        VAlign_Ascender,
        // middle
        VAlign_Middle,
        // descender
        VAlign_Descender,
    };
    // TextAlign [unsupported yet]
    enum TextAlign : uint16_t {
        // leading/begin
        TAlign_Leading = 0,
        // trailing/end
        TAlign_Trailing,
        // center
        TAlign_Center,
        // justified
        TAlign_Justified,
    };
    // wrap mode
    enum WrapMode : uint16_t {
        // no wrap
        Mode_NoWrap = 0,
        // space only
        Mode_SpaceOnly,
        // wrap cjk anwhere, other up to space
        Mode_SpaceOrCJK,
        // anwhere
        Mode_Anywhere
    };
    // doc flags
    enum DocFlag : uint32_t {
        // none
        Flag_None = 0,
        // fixed line-height
        Flag_FixedLineHeight = 1 << 0,
        // rich text
        Flag_RichText = 1 << 1,
        // gui-level read only
        Flag_GuiReadOnly = 1 << 2,
        // multi line
        Flag_MultiLine = 1 << 3,
        // password mode
        Flag_UsePassword = 1 << 4,
    };
    // OP
    RED_FLAG_OP(DocFlag, uint32_t);
    // effect
    enum Effect : uint16_t {
        // no effect
        Effect_None = 0,
        // Underline
        Effect_Underline = 1 << 0,
        // add what you want
#if 0
        // deleteline
        Effect_Deleteline = 1 << 1,
        // Upperline
        Effect_Upperline = 1 << 2
#endif
    };
    // fontflag
    enum FFlags : uint16_t {
        // no font flags
        FFlags_Node = 0,
        // italic
        FFlags_Italic = 0 << 1
        // add what you want
#if 0
        //
#endif
    };
    // cell type
    enum CellType : uint16_t {
        // [CEDNormalTextCell] normal text
        Type_Normal = 0,
        // [CEDNormalTextCell] ruby character
        Type_Ruby,
        // [   CEDTextCell   ] just one character under ruby
        Type_UnderRuby,
        // [   CEDTextCell++ ] image
        Type_Image,
        // [   CEDTextCell++ ] unknown inline object
        Type_UnknownInline,

        // object start
        Type_InlineObject = Type_UnderRuby,
    };
    // direction
    enum Direction : uint16_t {
        // left to right
        Direction_L2R = 0,
        // top to bottom
        Direction_T2B,
        // right to left
        Direction_R2L,
        // bottom to top
        Direction_B2T,
    };
}


// --------------------------------------------------------
//                     STRUCTURE
// --------------------------------------------------------

// RichED namespace
namespace RichED {
    // cell
    class CEDTextCell;
    // inline extra info
    struct RED_RICHED_ALIGNED InlineInfo { char space_holder; };
    // point
    struct Point { unit_t x, y; };
    // size
    struct Size { unit_t width, height; };
    // box
    struct Box { unit_t left, top, right, bottom; };
    // rect
    struct Rect { unit_t x, y, width, height; };
    // node
    struct Node { Node* prev, *next; };
    // range
    struct Range { uint32_t pos, len; };
    // doc point
    struct DocPoint { uint32_t line, pos; };
    // doc range
    struct DocRange { DocPoint begin, end; };
    // utf-16 stirng-view
    struct U16View { const char16_t* first, *second; };
    // cell hittest
    struct CellHitTest { uint32_t pos; uint16_t trailing; uint16_t length; };
    // cell point
    struct CellPoint { CEDTextCell* cell; uint32_t offset; };
    // char metrics
    struct CharMetrics { unit_t offset, width; };
    // fixed string part 1
    struct FixedStringA {
        // string length
        uint16_t            length;
        // string capacity
        uint16_t            capacity;
        // string data, [1] as inline object extra-info length
        char16_t            data[2];
        // get left, maybe == -1
        int32_t Left() const noexcept { return capacity - length; }
    };
    // fixed string part 2
    struct FixedStringB {
        // string data
        char16_t            data[TEXT_CELL_STR_MAXLEN - 2 + 1];
    };
    // rich-data
    struct RED_RICHED_ALIGNED RichData {
        // font size
        unit_t          size;
        // font color
        uint32_t        color;
        // font name id
        uint16_t        name;
        // add what you want
#if 0
        // weight
        uint16_t        weight;
#endif
        // font effect 
        Effect          effect;
        // font flags
        FFlags          fflags;
    };
    // cell metrics
    struct CellMetrics {
        // bounding box
        Box             bounding;
        // layout offset
        Point           offset;
        // layout width
        unit_t          width;
        // offset for visual-line 
        unit_t          pos;
        // ascender-height
        unit_t          ar_height;
        // deascender-height
        unit_t          dr_height;
    };
    // cell metainfo
    struct CellMeta {
        // type
        CellType        metatype;
        // end of line
        bool            eol;
        // begin of line
        //bool            bol : 1;
        // dirty
        bool            dirty : 1;
        // reduce
        //bool            reduce : 1;
    };
    // line feed
    struct LineFeed {
        // string length of this
        uint32_t        length;
        // char set
        char16_t        string[2];
        // view
        inline auto View() const noexcept { return U16View{ string, string + length }; }
        // as CRLF
        inline void AsCRLF() noexcept { string[0] = '\r'; string[1] = '\n'; length = 2; }
        // as LF
        inline void AsLF() noexcept { string[0] = '\n'; length = 1; }
        // as CR
        inline void AsCR() noexcept { string[0] = '\r'; length = 1; }
    };
    // doc init arg
    struct DocInitArg {
        // code
        enum CODE : int32_t {
            // OUTOFMEM
            CODE_OOM = int32_t(0x8007000eL)
        };
        // < 0 for failed
        int32_t   mutable   code;
        // read direction
        Direction           read;
        // flow direcion
        Direction           flow;
        // flags
        DocFlag             flags;
        // password char
        char32_t            password;
        // max char length
        uint32_t            length_max;
        // fixed lineheight
        unit_t              fixed_lineheight;
        // valign
        VerticalAlign       valign;
        // text align
        //TextAlign           talign;
        // wrap mode
        WrapMode            wrap_mode;
        // init-riched
        RichData            riched;
        // ok?
        bool IsOK() const noexcept { return code >= 0; }
        // failed?
        bool IsFailed() const noexcept { return code < 0; }
    };
    // normal info
    struct alignas(void*) DocInfo {
        // string total length, CRLF not included
        uint32_t        total_length;
        // max char length
        uint32_t        length_max;
        // string total count
        //uint32_t        total_count;
        // display line-num end
        uint32_t        display_line_begin;
        // display line-num end
        uint32_t        display_line_end;

        // flags
        DocFlag         flags;
        // password char [char16 x 2]
        char32_t        password_cha16x2;
        // fixed lineheight
        unit_t          fixed_lineheight;
        // vertical align
        VerticalAlign   valign;
        // text align
        //TextAlign       talign;
        // wrap mode
        WrapMode        wrap_mode;
    };
    // doc matrix
    struct DocMatrix {
        // doc to screen
        unit_t          d2s_matrix[6];
        // screen to doc
        unit_t          s2d_matrix[6];
        // read direction
        Direction       read_direction;
        // flowdirection
        Direction       flow_direction;
        // left mapper
        uint16_t        left_mapper;
        // up mapper
        uint16_t        up_mapper;
        // right mapper
        uint16_t        right_mapper;
        // down mapper
        uint16_t        down_mapper;
        // doc to screen
        auto DocToScreen(Point) const noexcept ->Point;
        // screen to doc
        auto ScreenToDoc(Point) const noexcept->Point;
    };
    // singe op for rich
    struct RichSingeOp {
        // under riched
        RichData        riched;
        // begin point
        DocPoint        begin;
        // end point
        DocPoint        end;
    };
}





// --------------------------------------------------------
//                     HELPER FUNC
// --------------------------------------------------------


// RichED namespace
namespace RichED {
    /// <summary>
    /// Removes from list only.
    /// </summary>
    /// <returns></returns>
    inline void RemoveFromListOnly(Node& node) noexcept {
        node.prev->next = node.next;
        node.next->prev = node.prev;
#ifndef NDEBUG
        node.prev = nullptr;
        node.next = nullptr;
#endif
    }
    /// <summary>
    /// Inserts the cell after this
    /// </summary>
    /// <param name="cell">The cell.</param>
    /// <returns></returns>
    inline void InsertAfterFirst(Node& node, Node& sec) noexcept {
        node.next->prev = &sec;
        sec.prev = &node;
        sec.next = node.next;
        node.next = &sec;
    }
}