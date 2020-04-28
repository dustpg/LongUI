#include "ed_txtdoc.h"
#include "ed_txtplat.h"
#include "ed_undoredo.h"

#include <cstring>
#include <cstdlib>
#include <type_traits>
#include <algorithm>

enum : uint32_t { 
    RED_ENDIAN_CODE = 0x00010203,
    RED_REV1_CODE   = 0x00000000,
    RED_REV2_CODE   = 0xffffffff,
};

/// <summary>
/// Reds the magic code.
/// </summary>
/// <returns></returns>
static inline uint32_t red_magic_code() noexcept {
    union { uint32_t code; uint8_t u8[4]; } data;
    data.u8[0] = 0x64;
    data.u8[1] = 0x75;
    data.u8[2] = 0x73;
    data.u8[3] = 0x74;
    return data.code;
}


namespace RichED {
    /// <summary>
    /// Undoes the redo idle.
    /// </summary>
    /// <param name="doc">The document.</param>
    /// <param name="op">The op.</param>
    /// <returns></returns>
    void UndoRedoIdle(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept { }
    // init callback
    void InitCallback(TrivialUndoRedo& op) noexcept;
    // private impl
    struct CEDTextDocument::UndoPri {
        // set caret data
        static void AnchorCaret(CEDTextDocument& doc,  TrivialUndoRedo& op) noexcept;
        // set riched
        static bool SetRichED(CEDTextDocument&, DocPoint, DocPoint, size_t, size_t, const void*, bool) noexcept;
        // set flags
        static bool SetFlagS(CEDTextDocument&, DocPoint, DocPoint, uint16_t, uint32_t) noexcept;
    };
}


/// <summary>
/// Sets the rich ed.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <param name="c">The c.</param>
/// <param name="d">The d.</param>
/// <param name="e">The e.</param>
/// <param name="f">The f.</param>
/// <param name="g">if set to <c>true</c> [g].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::UndoPri::SetRichED(
    CEDTextDocument&a, DocPoint b, DocPoint c, size_t d, size_t e, const void * f, bool g) noexcept {
    return a.set_riched(b, c, d, e, f, g);
}

/// <summary>
/// Sets the flag s.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <param name="c">The c.</param>
/// <param name="d">The d.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::UndoPri::SetFlagS(
    CEDTextDocument & a, DocPoint b, DocPoint c, uint16_t d, uint32_t e) noexcept {
    return a.set_flags(b, c, d, e);
}


/// <summary>
/// Initializes a new instance of the <see cref="CEDUndoRedo"/> class.
/// </summary>
/// <param name="max_deep">The maximum deep.</param>
RichED::CEDUndoRedo::CEDUndoRedo(uint32_t max_deep) noexcept : max_deep(max_deep) {
    // 处理Cell节点
    m_head.prev = nullptr;
    m_head.next = &m_tail;
    m_tail.prev = &m_head;
    m_tail.next = nullptr;
#ifndef NDEBUG
    std::memset(m_dbgBuffer, 233, sizeof(m_dbgBuffer));
#endif
    static_assert(std::is_trivial<TrivialUndoRedo>::value == true, "Trivial!");
}


/// <summary>
/// Clears this instance.
/// </summary>
/// <returns></returns>
void RichED::CEDUndoRedo::Clear() noexcept {
    auto node = m_head.next;
    while (node != &m_tail) {
        const auto ptr = node;
        node = node->next;
        std::free(ptr);
    }
    m_head.next = m_pStackTop = &m_tail;
    m_tail.prev = &m_head;
    m_cCurrent = 0;
}

/// <summary>
/// Undoes the specified document.
/// </summary>
/// <param name="doc">The document.</param>
/// <returns></returns>
bool RichED::CEDUndoRedo::Undo(CEDTextDocument& doc) noexcept {
    auto node = m_pStackTop;
    // 撤销栈为空
    if (node == &m_tail) return false;
    TrivialUndoRedo* last = nullptr;
    while (true) {
        const auto op = static_cast<TrivialUndoRedo*>(node);
        last = op;
        op->undo(doc, *op);
        if (!op->decorator) break;
        node = node->next;
    }
    doc.SetAnchorCaret(last->anchor, last->caret);
    m_pStackTop = node->next;
    return true;
}

/// <summary>
/// Redoes the specified document.
/// </summary>
/// <param name="doc">The document.</param>
/// <returns></returns>
bool RichED::CEDUndoRedo::Redo(CEDTextDocument & doc) noexcept {
    auto node = m_pStackTop;
    const auto first = m_head.next;
    // 撤销栈已满
    if (node == first) return false;
    TrivialUndoRedo* last = nullptr;
    while (true) {
        node = node->prev;
        const auto op = static_cast<TrivialUndoRedo*>(node);
        last = op;
        op->redo(doc, *op);
        // 到头了
        if (op == first) break;
        // 或者下(其实是上) 一个是非装饰操作
        if (!static_cast<TrivialUndoRedo*>(node->prev)->decorator) break;
    }
    doc.SetAnchorCaret(last->anchor, last->caret);
    m_pStackTop = node;
    return true;
}

/// <summary>
/// Anchors the caret.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="op">The op.</param>
/// <returns></returns>
void RichED::CEDTextDocument::UndoPri::AnchorCaret(
    CEDTextDocument & doc, TrivialUndoRedo & op) noexcept {
    op.anchor = doc.m_dpAnchor;
    op.caret = doc.m_dpCaret;
}

/// <summary>
/// Adds the op.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="op">The op.</param>
/// <returns></returns>
void RichED::CEDUndoRedo::AddOp(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
    CEDTextDocument::UndoPri::AnchorCaret(doc, op);
    // 释放HEAD -> TOP
    while (m_head.next != m_pStackTop) {
        const auto ptr = m_head.next;
        //const auto obj = static_cast<TrivialUndoRedo*>(ptr);
        m_head.next = m_head.next->next;
        std::free(ptr);
    }

    RichED::InitCallback(op);
    RichED::InsertAfterFirst(m_head, op);
    m_pStackTop = m_head.next;
}

// ----------------------------------------------------------------------------
//                               Object 
// ----------------------------------------------------------------------------

namespace RichED {
    // singe op for ruby
    struct RED_RICHED_ALIGNED ObjectSingeOp {
        // point
        DocPoint        begin;
        // ruby text length
        uint32_t        ruby_length;
        // type of this
        CellType        cell_type;
        // length of extra data
        uint16_t        extra_length;
        // next op
        auto Next() noexcept {
            const size_t exlen = extra_length;
            const size_t alignc = alignof(ObjectSingeOp);
            const size_t mask = ~(alignc - 1);
            const size_t aligned_exlen = (exlen + (alignc - 1)) & mask;
            const auto ptr = reinterpret_cast<char*>(this + 1) + aligned_exlen;
            return reinterpret_cast<ObjectSingeOp*>(ptr);
        }
    };
    // Rollback objs
    void RemoveObjs(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        auto obj = reinterpret_cast<ObjectSingeOp*>(&op + 1);
        const auto end_ptr = reinterpret_cast<char*>(&op.bytes_from_here) + op.bytes_from_here;
        const auto end_itr = reinterpret_cast<ObjectSingeOp*>(end_ptr);
        while (obj < end_itr) {
            assert(obj->Next() <= end_itr);
            auto end_dp = obj->begin; end_dp.pos++;
            doc.RemoveText(obj->begin, end_dp);
            obj = obj->Next();
        }
    }
    // execute objs
    void ExecuteObjs(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        auto obj = reinterpret_cast<ObjectSingeOp*>(&op + 1);
        const auto end_ptr = reinterpret_cast<char*>(&op.bytes_from_here) + op.bytes_from_here;
        const auto end_itr = reinterpret_cast<ObjectSingeOp*>(end_ptr);
        while (obj < end_itr) {
            assert(obj->Next() <= end_itr);
            assert(!!obj->ruby_length != !!obj->extra_length || (!obj->ruby_length && !obj->extra_length));
            // 注音符号： 将范围字符转换为被注音, 然后后面指定长度为注音
            if (obj->ruby_length) {
                // 升阶魔法! RUM!
                doc.RankUpMagic(obj->begin, obj->ruby_length);
            }
            // 其他内联对象: 利用EXTRA-INFO创建内联对象
            else if (obj->extra_length) {
                // 升阶魔法! RUM!
                const auto info = reinterpret_cast<InlineInfo*>(obj + 1);
                doc.RankUpMagic(obj->begin, *info, obj->extra_length, obj->cell_type);
            }
            obj = obj->Next();
        }
    }
    /// <summary>
    /// 插入
    /// </summary>
    /// <param name="doc">The document.</param>
    /// <param name="op">The op.</param>
    /// <returns></returns>
    void InsertObjs(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        // 只有一个
        auto obj = reinterpret_cast<ObjectSingeOp*>(&op + 1);
        const auto end_ptr = reinterpret_cast<char*>(&op.bytes_from_here) + op.bytes_from_here;
        const auto end_itr = reinterpret_cast<ObjectSingeOp*>(end_ptr);
        assert(obj->Next() == end_itr);
        const auto info = reinterpret_cast<InlineInfo*>(obj + 1);
        doc.InsertInline(obj->begin, *info, obj->extra_length, obj->cell_type);
    }
}


// ----------------------------------------------------------------------------
//                             RichED Ruby
// ----------------------------------------------------------------------------

namespace RichED {
    // singe op for rich
    struct RubySingeOp {
        // under riched
        RichData        riched;
        // begin point
        DocPoint        begin;
        // ruby length
        uint32_t        ruby_length;
        // under length
        uint32_t        under_length;
        // char
        char32_t        ch;
        // ruby
        char16_t        ruby[2];
    };
    // Rollback ruby
    void RemoveRuby(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        const auto obj = reinterpret_cast<RubySingeOp*>(&op + 1);
        DocPoint end_dp = obj->begin;
        end_dp.pos += obj->ruby_length + obj->under_length;
        doc.RemoveText(obj->begin, end_dp);
    }
    // exec ruby
    void ExecuteRuby(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        const auto obj = reinterpret_cast<RubySingeOp*>(&op + 1);
        U16View ruby;
        ruby.first = obj->ruby;
        ruby.second = ruby.first + obj->ruby_length;
        assert(ruby.second > ruby.first);
        doc.InsertRuby(obj->begin, obj->ch, ruby, &obj->riched);
    }
}

// ----------------------------------------------------------------------------
//                             RichED Rich
// ----------------------------------------------------------------------------


namespace RichED {
    // group op for rich
    struct RichGroupOp {
        // op for rollback
        RichSingeOp     back;
        // back offset
        uint16_t        back_offset;
        // back length
        uint16_t        back_length;
        // relayout
        bool            relayout;
        // unused
        bool            unused[3];
        // ops for execute
        RichSingeOp     exec[1];
    };
    // Rollback rich
    void RollbackRich(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        const auto data = reinterpret_cast<RichGroupOp*>(&op + 1);
        const auto ptr = reinterpret_cast<char*>(&data->back.riched) + data->back_offset;
        if (data->back_length) {
            CEDTextDocument::UndoPri::SetRichED(
                doc, data->back.begin, data->back.end,
                data->back_offset, data->back_length,
                ptr, data->relayout
            );
        }
        // 长度为0则是设置FLAG
        else {
            const uint16_t flags = static_cast<uint16_t>(data->back.riched.fflags);
            CEDTextDocument::UndoPri::SetFlagS(
                doc, data->back.begin, data->back.end,
                flags, data->back_offset
            );
        }
    }
    // execute rich
    void ExecuteRich(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        const auto data = reinterpret_cast<RichGroupOp*>(&op + 1);
        const auto end_ptr = reinterpret_cast<char*>(&op.bytes_from_here) + op.bytes_from_here;
        const auto end_itr = reinterpret_cast<RichSingeOp*>(end_ptr);
        const auto relayout = data->relayout;
        std::for_each(data->exec, end_itr, [&doc, relayout](const RichSingeOp& op)noexcept {
            CEDTextDocument::UndoPri::SetRichED(
                doc, op.begin, op.end,
                0, sizeof(op.riched),
                &op.riched, relayout
            );
        });
    }
}


// ----------------------------------------------------------------------------
//                             RichED Text
// ----------------------------------------------------------------------------

namespace RichED {
    // group op for text
    struct TextGroupOp {
        // begin point
        DocPoint        begin;
        // end point
        DocPoint        end;
        // text length
        uint32_t        length;
        // text data
        char16_t        text[2];
    };
    // Rollback text
    void RollbackText(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        const auto data = reinterpret_cast<TextGroupOp*>(&op + 1);
        doc.RemoveText(data->begin, data->end);
    }
    // execute text
    void ExecuteText(CEDTextDocument& doc, TrivialUndoRedo& op) noexcept {
        const auto data = reinterpret_cast<TextGroupOp*>(&op + 1);
        doc.InsertText(data->begin, { data->text, data->text + data->length }, true);
    }
}

// ----------------------------------------------------------------------------
//                             RichED Overview
// ----------------------------------------------------------------------------

namespace RichED {
    // undo op
    enum UndoRedoOp : uint16_t {
        // remove: text
        Op_RemoveText = 0,
        // remove: rich
        Op_RemoveRich,
        // remove: objs
        Op_RemoveObjs,

        // insert: text
        Op_InsertText,
        // insert: objs
        Op_InsertObjs,
        // insert: ruby
        Op_InsertRuby,
        // setas: rich
        Op_SetAsRich,
    };
    namespace impl {
        /// <summary>
        /// Riches the undoredo.
        /// </summary>
        /// <param name="count">The count.</param>
        /// <returns></returns>
        void* rich_undoredo(uint32_t count) noexcept {
            assert(count);
            const size_t len = sizeof(TrivialUndoRedo)
                + (sizeof(RichGroupOp) - sizeof(RichSingeOp))
                + sizeof(RichSingeOp) * count
                ;
            const auto ptr = std::malloc(len);
            if (ptr) {
                const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
                const size_t offset = offsetof(TrivialUndoRedo, bytes_from_here);
                op->bytes_from_here = len - offset;
            }
            return ptr;
        }
        /// <summary>
        /// Riches as remove.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <returns></returns>
        void rich_as_remove(void* ptr, uint16_t id) noexcept {
            assert(ptr && id);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            const auto ops = reinterpret_cast<RichGroupOp*>(op + 1);
            op->type = Op_RemoveRich;
            op->decorator = id - 1;
            ops->relayout = true;
        }
        /// <summary>
        /// Riches the initialize.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="relayout">if set to <c>true</c> [relayout].</param>
        /// <returns></returns>
        void rich_init(void* ptr, bool relayout, uint16_t o, uint16_t l, const RichSingeOp& a) noexcept {
            assert(ptr);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            op->type = Op_SetAsRich;
            const auto ops = reinterpret_cast<RichGroupOp*>(op + 1);
            ops->relayout = relayout;
            ops->back = a;
            ops->back_offset = o;
            ops->back_length = l;
        }
        /// <summary>
        /// Riches the set.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="index">The index.</param>
        /// <param name="data">The data.</param>
        /// <returns></returns>
        void rich_set(void* ptr, uint32_t index, const RichData & data, DocPoint a, DocPoint b) noexcept {
            assert(ptr);
            assert(a.line != b.line || a.pos != b.pos);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            const auto ops = reinterpret_cast<RichGroupOp*>(op + 1);
            ops->exec[index].riched = data;
            ops->exec[index].begin = a;
            ops->exec[index].end = b;
        }
    }
    namespace impl {
        /// <summary>
        /// Riches the undoredo.
        /// </summary>
        /// <param name="count">The count.</param>
        /// <returns></returns>
        void*text_undoredo(uint32_t count) noexcept {
            assert(count);
            const size_t len = sizeof(TrivialUndoRedo)
                + sizeof(TextGroupOp)
                + sizeof(char16_t) * count
                ;
            const auto ptr = std::malloc(len);
            if (ptr) {
                const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
                const size_t offset = offsetof(TrivialUndoRedo, bytes_from_here);
                op->bytes_from_here = len - offset;
                const auto ops = reinterpret_cast<TextGroupOp*>(op + 1);
                ops->length = count;
            }
            return ptr;
        }
        /// <summary>
        /// Texts as remove.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="id">The identifier.</param>
        /// <param name="a">a.</param>
        /// <param name="b">The b.</param>
        /// <returns></returns>
        void text_as_remove(void* ptr, uint16_t id, DocPoint a, DocPoint b) noexcept {
            assert(ptr && id);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            op->type = Op_RemoveText;
            op->decorator = id - 1;
            const auto ops = reinterpret_cast<TextGroupOp*>(op + 1);
            ops->begin = a;
            ops->end = b;
        }
        /// <summary>
        /// Texts as insert.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="id">The identifier.</param>
        /// <param name="a">a.</param>
        /// <param name="b">The b.</param>
        /// <returns></returns>
        void text_as_insert(void* ptr, uint16_t id, DocPoint a, DocPoint b) noexcept {
            assert(ptr && id);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            op->type = Op_InsertText;
            op->decorator = id - 1;
            const auto ops = reinterpret_cast<TextGroupOp*>(op + 1);
            ops->begin = a;
            ops->end = b;
        }
        /// <summary>
        /// Texts the append.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="i">The i.</param>
        /// <param name="view">The view.</param>
        /// <returns></returns>
        void text_append(void* ptr, uint32_t i, U16View view) noexcept {
            assert(ptr);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            const auto ops = reinterpret_cast<TextGroupOp*>(op + 1);
            const auto len = view.second - view.first;
            std::memcpy(ops->text + i, view.first, len * sizeof(char16_t));
#ifndef NDEBUG
            // 调试时添加NUL字符方便调试
            ops->text[i + len] = 0;
#endif 
        }
        /// <summary>
        /// Texts the append.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="i">The i.</param>
        /// <param name="ch">The ch.</param>
        /// <returns></returns>
        void text_append(void* ptr, uint32_t i, char16_t ch) noexcept {
            assert(ptr);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            const auto ops = reinterpret_cast<TextGroupOp*>(op + 1);
            ops->text[i] = ch;
#ifndef NDEBUG
            // 调试时添加NUL字符方便调试
            ops->text[i + 1] = 0;
#endif 
        }
    }
    namespace impl {
        // ruby undoredo
        void*ruby_undoredo(uint32_t length) noexcept {
            assert(length);
            const size_t len = sizeof(TrivialUndoRedo)
                + sizeof(RubySingeOp)
                + sizeof(char16_t) * length
                ;
            const auto ptr = std::malloc(len);
            if (ptr) {
                const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
                const size_t offset = offsetof(TrivialUndoRedo, bytes_from_here);
                op->bytes_from_here = len - offset;
            }
            return ptr;
        }
        // insert ruby
        void ruby_as_insert(void* ptr, uint16_t id) noexcept {
            assert(ptr && id);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            op->type = Op_InsertRuby;
            op->decorator = id - 1;
        }
        // ruby set data
        void ruby_set_data(void* ptr, DocPoint dp, char32_t ch, U16View view, const RichData& data) noexcept {
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            const auto obj = reinterpret_cast<RubySingeOp*>(op + 1);
            obj->riched = data;
            obj->begin = dp;
            obj->ruby_length = view.second - view.first;
            obj->under_length = ch > 0xffff ? 2 : 1;
            obj->ch = ch;
            const size_t bl = sizeof(view.first[0]) * (view.second - view.first);
            std::memcpy(obj->ruby, view.first, bl);
#ifndef NDEBUG
            // 调试时添加NUL字符方便调试
            obj->ruby[obj->ruby_length] = 0;
#endif 
        }
    }
    namespace impl {
        /// <summary>
        /// Objses the undoredo.
        /// </summary>
        /// <param name="count">The count.</param>
        /// <param name="length">The length.</param>
        /// <returns></returns>
        void* objs_undoredo(uint32_t count, uint32_t length) noexcept {
            assert(count);
            assert((length & (alignof(TrivialUndoRedo) - 1)) == 0);
            const size_t len = sizeof(TrivialUndoRedo)
                + sizeof(ObjectSingeOp) * count
                + length
                ;
            const auto ptr = std::malloc(len);
            if (ptr) {
                const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
                const size_t offset = offsetof(TrivialUndoRedo, bytes_from_here);
                op->bytes_from_here = len - offset;
            }
            return ptr;
        }
        /// <summary>
        /// Objses as remove.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="id">The identifier.</param>
        /// <returns></returns>
        void*objs_as_remove(void* ptr, uint16_t id) noexcept {
            assert(ptr && id);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            op->type = Op_RemoveObjs;
            op->decorator = id - 1;
            return op + 1;
        }
        /// <summary>
        /// Objses as remove.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="id">The identifier.</param>
        /// <returns></returns>
        void*objs_as_insert(void* ptr, uint16_t id) noexcept {
            assert(ptr && id);
            const auto op = reinterpret_cast<TrivialUndoRedo*>(ptr);
            op->type = Op_InsertObjs;
            op->decorator = id - 1;
            return op + 1;
        }
        /// <summary>
        /// Objses as remove.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        /// <param name="id">The identifier.</param>
        /// <returns></returns>
        void*objs_as_goon(void* ptr, DocPoint dp, uint32_t ruby, CellType type, uint16_t exlen, void* data) noexcept {
            assert(ptr);
            const auto op = reinterpret_cast<ObjectSingeOp*>(ptr);
            op->begin = dp;
            op->ruby_length = ruby;
            op->extra_length = exlen;
            op->cell_type = type;
            std::memcpy(op + 1, data, exlen);
            return op->Next();
        }
    }
    /// <summary>
    /// Initializes the callback.
    /// </summary>
    /// <param name="op">The op.</param>
    /// <returns></returns>
    void InitCallback(TrivialUndoRedo& op) noexcept {
        switch (op.type)
        {
        default:
            assert(!"UNKNOWN!");
            op.undo = RichED::UndoRedoIdle;
            op.redo = RichED::UndoRedoIdle;
            break;
        case Op_RemoveText:
            // 移除文本
            // - 撤销: 文本添加
            // - 重做: 文本移除
            op.undo = RichED::ExecuteText;
            op.redo = RichED::RollbackText;
            break;
        case Op_RemoveRich:
            // 移除富属性
            // - 撤销: 富属性的修改
            // - 重做: 富属性的无视
            op.undo = RichED::ExecuteRich;
            op.redo = RichED::UndoRedoIdle;
            break;
        case Op_RemoveObjs:
            // 移除内联对象
            // - 撤销: 内联对象的再生
            // - 重做: 内联对象的无视
            op.undo = RichED::ExecuteObjs;
            op.redo = RichED::UndoRedoIdle;
            break;
        case Op_InsertText:
            // 插入文本
            // - 撤销: 文本移除
            // - 重做: 文本添加
            op.undo = RichED::RollbackText;
            op.redo = RichED::ExecuteText;
            break;
        case Op_InsertObjs:
            // 插入对象
            // - 撤销: 删除
            // - 重做: 插入
            op.undo = RichED::RemoveObjs;
            op.redo = RichED::InsertObjs;
            break;
        case Op_InsertRuby:
            // 插入注音
            // - 撤销: 删除注音
            // - 重做: 插入注音
            op.undo = RichED::RemoveRuby;
            op.redo = RichED::ExecuteRuby;
            break;
        case Op_SetAsRich:
            // 修改富属性
            // - 撤销: 执行富属性
            // - 重做: 修改富属性
            op.undo = RichED::ExecuteRich;
            op.redo = RichED::RollbackRich;
            break;
        }
    }
}


// ----------------------------------------------------------------------------
//                             RichED Save/Load
// ----------------------------------------------------------------------------


/// <summary>
/// Saves the bin file.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::SaveBinFile(CtxPtr ctx) noexcept {
    uint32_t magic_codes[4];
    magic_codes[0] = red_magic_code();
    magic_codes[1] = RED_ENDIAN_CODE;
    magic_codes[2] = RED_REV1_CODE;
    magic_codes[3] = RED_REV2_CODE;
    auto& plat = this->platform;
    const auto write_file = [&plat, ctx](const void* ptr, uint32_t len) noexcept {
        return plat.WriteToFile(ctx, static_cast<const uint8_t*>(ptr), len);
    };
    // 先驱码
    if (write_file(magic_codes, sizeof(magic_codes))) {

    }
    return false;
}

/// <summary>
/// Loads the bin file.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::LoadBinFile(CtxPtr ctx) noexcept {
    uint32_t magic_codes[4];
    auto& plat = this->platform;
    const auto read_file = [&plat, ctx](void* ptr, uint32_t len) noexcept {
        return plat.ReadFromFile(ctx, static_cast<uint8_t*>(ptr), len);
    };
    if (!read_file(magic_codes, sizeof(magic_codes))) return false;
    if (magic_codes[0] != red_magic_code()) return false;
    assert(magic_codes[1] == RED_ENDIAN_CODE && "NOT IMPL");
    return false;
}