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


#include <cstdint>
#include "ed_common.h"



namespace RichED {
    // text doc
    class CEDTextDocument;
    // Trivial UndoRedo data, no CTOR/DTOR
    struct TrivialUndoRedo : Node {
        // undo-op fucntion pointer
        void(*undo)(CEDTextDocument&, TrivialUndoRedo&) noexcept;
        // redo-op fucntion pointer
        void(*redo)(CEDTextDocument&, TrivialUndoRedo&) noexcept;
        // byte length from here
        RED_RICHED_ALIGNED uint32_t bytes_from_here;
        // type
        uint16_t                    type;
        // decorator
        uint16_t                    decorator;
        // anchor
        DocPoint                    anchor;
        // caret
        DocPoint                    caret;
    };
    // undo redo 
    class CEDUndoRedo {
    public:
        // ctor
        CEDUndoRedo(uint32_t max_deep = 256) noexcept;
        // dtor
        ~CEDUndoRedo() noexcept { this->Clear(); }
        // clear all history
        void Clear() noexcept;
        // add an undoredo op
        void AddOp(CEDTextDocument&, TrivialUndoRedo&) noexcept;
        // undo
        bool Undo(CEDTextDocument& doc) noexcept;
        // redo
        bool Redo(CEDTextDocument& doc) noexcept;
    public:
        // fixed length 
        uint32_t      const max_deep;
    private:
        // current length
        uint32_t            m_cCurrent = 0;
        // stack top
        Node*               m_pStackTop = &m_tail;
        // head
        Node                m_head;
        // tail
        Node                m_tail;
#ifndef NDEBUG
        // debug buffer to avoid head/tail node as a cell
        char                m_dbgBuffer[128];
#endif
    };
}