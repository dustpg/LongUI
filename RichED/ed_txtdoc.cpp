#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ed_txtdoc.h"
#include "ed_txtplat.h"
#include "ed_txtcell.h"

#include <algorithm>

// debug via longui
//#include <debugger/ui_debug.h>

enum { RED_INIT_ARRAY_BUFLEN = 32 };

// CJK LUT
RED_LUT_ALIGNED const uint32_t RED_CJK_LUT[] = {
    0x00000000, 0xfffc0000, 0xffffffff, 0xffffffff, 
    0xffffffff, 0x00000000, 0x00000000, 0x06000000, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0x07000fff, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

// namesapce RichED::detail
namespace RichED { namespace impl {
    // op record
    struct op_recorder { 
        CEDTextDocument& doc;
        op_recorder(CEDTextDocument& doc) noexcept : doc(doc) { doc.BeginOp(); };
        ~op_recorder() noexcept { doc.EndOp(); };
    };
    // is_surrogate
    static inline bool is_surrogate(uint16_t ch) noexcept { return ((ch) & 0xF800) == 0xD800; }
    // is_low_surrogate
    //inline bool is_low_surrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xDC00; }
    // is_high_surrogate
    //inline bool is_high_surrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xD800; }
    // is_2nd_surrogate
    static inline bool is_2nd_surrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xDC00; }
    // is_1st_surrogate
    static inline bool is_1st_surrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xD800; }
    // char16 x2 -> char32
    inline char32_t char16x2to32(char16_t lead, char16_t trail) {
        assert(is_1st_surrogate(lead) && "illegal utf-16 char");
        assert(is_2nd_surrogate(trail) && "illegal utf-16 char");
        return char32_t((uint16_t(lead) - 0xD800) << 10 | (uint16_t(trail) - 0xDC00)) + (0x10000);
    };
    // count char32_t 
    static uint32_t count(U16View view) noexcept {
        uint32_t c = 0;
        while (view.first < view.second) {
            if (is_1st_surrogate(*view.first)) {
                view.first++;
                assert(is_2nd_surrogate(*view.first));
            }
            c++;
            view.first++;
        }
        return c;
    }
    // lf count 
    static DocPoint lfcount(U16View view) noexcept {
        uint32_t c = 0;
        auto point = view.first;
        while (view.first < view.second) {
            if (*view.first == '\n') point = view.first + 1, c++;
            view.first++;
        }
        return { c, static_cast<uint32_t>(view.second - point)};
    }
    // lf view
    static U16View lfview(U16View& view) noexcept {
        // 支持\r\n, \n. 暂不支持\r
        U16View rv = view;
        uint32_t count = 0;
        while (view.first < view.second) {
            const auto ch = *view.first;
            view.first++;
            if (ch == '\n')  {
                rv.second = view.first - 1;
                if (count && rv.second[-1] == '\r') --rv.second;
                break;
            }
            ++count;
        }
        return rv;
    }
    // nice view 1
    static U16View nice_view1(U16View& view, int32_t len) noexcept {
        U16View rv;
        auto itr = rv.first = view.first; 
        const int32_t real_len = std::min(len, int32_t(view.second - view.first));

        for (int32_t i = 0; i < real_len; ++i) {
            if (*itr == '\n') {
                if (i && itr[-1] == '\r') --itr;
                break;
            }
            ++itr;
        }
        if (real_len && impl::is_1st_surrogate(itr[-1])) ++itr;
        rv.second = view.first = itr;
        return rv;
    }
    // nice view 2
    static U16View nice_view2(U16View& view, int32_t len) noexcept {
        U16View rv; 
        auto itr = rv.second = view.second;
        const int32_t real_len = std::min(len, int32_t(view.second - view.first));
        for (int32_t i = 0; i < real_len; ++i) {
            if (itr[-1] == '\n') break;
            --itr;
        }
        if (real_len && impl::is_2nd_surrogate(itr[-1])) --itr;
        rv.first = view.second = itr;
        return rv;
    }
    // is cjk
    static uint32_t is_cjk(uint32_t ch) noexcept {
        // 假定区域以0x100对齐
        const uint32_t ch2 = (ch & 0x3ffff) >> 8;
        const uint32_t index = ch2 >> 5;
        const uint32_t mask = 1 << (ch2 & 0x1f);
        return RED_CJK_LUT[index] & mask;
    }
    // utf-32 to utf-16
    inline uint16_t utf32to16(char32_t ch, char16_t buffer[2]) {
        // utf32 -> utf32x2
        if (ch > 0xFFFF) {
            // From http://unicode.org/faq/utf_bom.html#35
            buffer[0] = static_cast<char16_t>(0xD800 + (ch >> 10) - (0x10000 >> 10));
            buffer[1] = static_cast<char16_t>(0xDC00 + (ch & 0x3FF));
            return 2;
        }
        buffer[0] = static_cast<char16_t>(ch);
        return 1;
    }
    // cell from node
    inline auto next_cell(Node* node) noexcept {
        return static_cast<CEDTextCell*>(node->next); }
    // cell from node
    inline auto prev_cell(Node* node) noexcept {
        return static_cast<CEDTextCell*>(node->prev); }
    // cell from node
    inline auto next_cell(const Node* node) noexcept {
        return static_cast<const CEDTextCell*>(node->next); }
    // itr
    template<typename T> struct nitr {
        // cell node
        T*      cell;
        // operator *
        auto&operator*() noexcept { assert(cell && cell->next && cell->prev); return *cell; }
        // operator !=
        bool operator!=(nitr node) const noexcept { return cell != node.cell; }
        // operator ++
        nitr&operator++() noexcept { cell = next_cell(cell); return *this; }
        // operator ++(int)
        nitr operator++(int) noexcept { nitr itr = *this; ++(*this); return itr; }
    };
    // for helper
    template<typename T> struct for_cells {
        // begin node
        T*          begin_cell;
        // end node
        T*          end_cell;
        // begin
        auto begin() const noexcept { return nitr<T>{ static_cast<T*>(begin_cell) }; }
        // end
        auto end() const noexcept { return nitr<T>{ static_cast<T*>(end_cell) }; }
    };
    // get for_cells
    inline auto cfor_cells(Node* a, Node* b) noexcept {
        return for_cells<CEDTextCell>{ static_cast<CEDTextCell*>(a),  static_cast<CEDTextCell*>(b) };
    }
    // get for_cells
    inline auto cfor_cells(const Node* a, const Node* b) noexcept {
        return for_cells<const CEDTextCell>{ static_cast<const CEDTextCell*>(a), static_cast<const CEDTextCell*>(b) };
    }
    // push line data
    static bool push_data(CEDBuffer<VisualLine>& vlv, 
        const VisualLine& line, IEDTextPlatform& p) noexcept {
        const auto size = vlv.GetSize();
        if (vlv.IsFull()) {
            if (!vlv.Resize(size + (size >> 1), p)) return false;
        }
        vlv.Resize(size + 1, p);
        vlv[size] = line;
        return true;
    }
    // txtoff
    struct txtoff_t { CEDTextCell* cell; uint32_t pos; };
    // find
    PCN_NOINLINE static auto find_cell1_txtoff(CEDTextCell* cell, uint32_t pos) noexcept {
        // 遍历到合适的位置
        while (pos > cell->RefString().length) {
            assert(cell->RefMetaInfo().eol == false);
            pos -= cell->RefString().length;
            cell = impl::next_cell(cell);
        }
        const txtoff_t rv = { cell, pos };
        return rv;
    }
    // find
    PCN_NOINLINE static auto find_cell2_txtoff(CEDTextCell* cell, uint32_t pos) noexcept {
        // 遍历到合适的位置
        while (pos >= cell->RefString().length) {
            if (cell->RefMetaInfo().eol) break;
            pos -= cell->RefString().length;
            cell = impl::next_cell(cell);
        }
        const txtoff_t rv = { cell, pos };
        return rv;
    }
    // find
    static inline void find_cell1_txtoff_ex(CEDTextCell*& cell, uint32_t& pos) noexcept {
        const auto val = find_cell1_txtoff(cell, pos);
        cell = val.cell; pos = val.pos;
    }
    // find
    static inline void find_cell2_txtoff_ex(CEDTextCell*& cell, uint32_t& pos) noexcept {
        const auto val = find_cell2_txtoff(cell, pos);
        cell = val.cell; pos = val.pos;
    }
}}

// namespace RichED::impl 
namespace RichED { namespace impl {
    // selection mode
    enum selection_mode : uint32_t {
        mode_all = 0,       // 全选文本
        mode_target,        // 目标位置
        mode_logicleft,     // 左移一下
        mode_leftword,      // 左移字段
        mode_logicright,    // 右移一下
        mode_rightword,     // 右移字段
        mode_home,          // 一行起始
        mode_first,         // 文本开头
        mode_end,           // 一行结束
        mode_last,          // 文本结尾
        //mode_leftchar,      // 左移字符
        //mode_rightchar,     // 右移字符
        mode_logicup,       // 上升一行
        mode_viewup,        // 视口上升
        mode_logicdown,     // 下降一行
        mode_viewdown,      // 视口下降
    };
    // rich undoredo
    auto rich_undoredo_len(uint32_t count) noexcept ->size_t;
    // rich undoredo
    void rich_undoredo_mk(void*, uint32_t count) noexcept;
    // remove richedtext
    void rich_as_remove(void* ptr, uint16_t id) noexcept;
    // init rich for set as
    void rich_init(void* ptr, bool relayout, uint16_t o, uint16_t l, const RichSingeOp& a) noexcept;
    // rich undoredo
    void rich_set(void*, uint32_t, const RichData&, DocPoint, DocPoint) noexcept;


    // text undoredo
    auto text_undoredo_len(uint32_t count) noexcept->size_t;
    // text undoredo
    void text_undoredo_mk(void*, uint32_t count) noexcept;
    // remove text
    void text_as_remove(void* ptr, uint16_t id, DocPoint, DocPoint) noexcept;
    // insert text
    void text_as_insert(void* ptr, uint16_t id, DocPoint, DocPoint) noexcept;
    // text append
    void text_append(void*, uint32_t, U16View view) noexcept;
    // text append
    void text_append(void*, uint32_t, char16_t ch) noexcept;


    // objs undoredo
    auto objs_undoredo_len(uint32_t count, uint32_t length) noexcept ->size_t;
    // objs undoredo
    void objs_undoredo_mk(void*, uint32_t count, uint32_t length) noexcept;
    // remove objs
    void*objs_as_remove(void* ptr, uint16_t id) noexcept;
    // insert objs
    void*objs_as_insert(void* ptr, uint16_t id) noexcept;
    // objs go on
    void*objs_as_goon(void* ptr, DocPoint dp, uint32_t ruby, CellType type, uint16_t exlen, void* data) noexcept;


    // ruby undoredo
    auto ruby_undoredo_len(uint32_t length) noexcept ->size_t;
    // ruby undoredo
    void ruby_undoredo_mk(void*, uint32_t length) noexcept;
    // insert ruby
    void ruby_as_insert(void* ptr, uint16_t id) noexcept;
    // ruby set data
    void ruby_set_data(void* ptr, DocPoint, char32_t, U16View, const RichData&) noexcept;
}}


// this namesapce
namespace RichED {
    // create cell
    auto CreatePublicCell(
        CEDTextDocument& doc,
        const RichData& red,
        uint32_t exlen,
        uint32_t capacity
    ) noexcept->CEDTextCell*;
    // init matrix
    void InitMatrix(DocMatrix& matrix, Direction read, Direction flow) noexcept;
    // check range
    struct CheckRangeCtx {
        CellPoint   begin;
        CellPoint   end;
        LogicLine   line2;
        LogicLine*  line1;
    };
    // richex ctx
    struct RichExCtx {
        // op
        RichSingeOp     op;
        // offset
        uint32_t        offset;
        // length
        uint32_t        length;
        // relayout
        bool            relayout;
    };
    // hittest
    struct HitTestCtx {
        // visual line
        const VisualLine*   visual_line;
        // text cell
        CEDTextCell*        text_cell;
        // pos: before cell
        uint32_t            len_before_cell;
        // pos: in cell
        uint32_t            pos_in_cell;
    };
    // priavate impl for CEDTextDocument
    struct CEDTextDocument::Private {
        // check point
        //static auto CheckPoint(CEDTextDocument& doc, DocPoint) noexcept->CellPoint;
        // update estimated width
        //static void UpdatemEstimated();
        // Gen Text
        template<typename T, typename U> 
        static void GenText(CEDTextDocument& doc, DocPoint begin, DocPoint end, T ap, U lf) noexcept;
        // record op
        static bool IsRecord(const CEDTextDocument& doc) noexcept;
        // mouse
        static bool Mouse(CEDTextDocument& doc, Point, bool hold) noexcept;
        // delete
        static bool DeleteSelection(CEDTextDocument& doc) noexcept;
        // hit doc from position
        static bool HitTest(CEDTextDocument& doc, Point, HitTestCtx&) noexcept;
        // hit doc from doc point
        static void HitTest(CEDTextDocument& doc, DocPoint, HitTestCtx&) noexcept;
        // force update caret
        static void RefreshCaret(CEDTextDocument& doc, DocPoint, HitTestCtx*) noexcept;
        // update selection
        static void UpdateSelection(CEDTextDocument&, DocPoint caret, DocPoint anc) noexcept;
        // force update selection
        static void RefreshSelection(CEDTextDocument&, DocPoint begin, DocPoint end) noexcept;
        // rich range
        static bool RichRange(const CheckRangeCtx&, CellPoint out[2]) noexcept;
        // check range
        static bool CheckRange(CEDTextDocument& doc, DocPoint begin, DocPoint& end, CheckRangeCtx& ctx) noexcept;
        // check wrap mode
        static auto CheckWrap(CEDTextDocument& doc, CEDTextCell& cell, unit_t pos) noexcept->CEDTextCell*;
        // expand visual line clean area
        static void ExpandVL(CEDTextDocument& doc, uint32_t ll, unit_t) noexcept;
        // recreate cell
        static void Recreate(CEDTextDocument&doc, CEDTextCell& cell) noexcept;
        // set selection
        static void SetSelection(CEDTextDocument& doc, HitTestCtx*,DocPoint dp, uint32_t mode, bool ) noexcept;
        // insert text
        static bool Insert(CEDTextDocument& doc, DocPoint dp, U16View, LogicLine, bool behind)noexcept;
        // insert cell
        static bool Insert(CEDTextDocument& doc, DocPoint dp, CEDTextCell&, LogicLine&)noexcept;
        // remove text
        static bool RemoveText(CEDTextDocument& doc, DocPoint begin, DocPoint end, const CheckRangeCtx&)noexcept;
        // remove objects
        static void RecordObjs(CEDTextDocument& doc, DocPoint begin, const CheckRangeCtx&)noexcept;
        // record rich
        static void RecordRich(CEDTextDocument& doc, DocPoint begin, const CheckRangeCtx&, const RichExCtx*)noexcept;
        // record text
        static void RecordText(CEDTextDocument& doc, DocPoint begin, DocPoint end)noexcept;
        // record obj for ins
        static void RecrodObjsEx(CEDTextDocument& doc, DocPoint begin, CEDTextCell& cell) noexcept;
        // record ruby for ins
        static void RecordRubyEx(CEDTextDocument& doc, DocPoint begin, const RichData&, char32_t, U16View)noexcept;
        // record text for ins
        static void RecordTextEx(CEDTextDocument& doc, DocPoint begin, DocPoint end, U16View)noexcept;
        // mark dirty
        static void Dirty(CEDTextDocument& doc, CEDTextCell& cell, uint32_t logic_line)noexcept;
        // merge cells
        static bool Merge(CEDTextDocument& doc, CEDTextCell& cell, unit_t, unit_t) noexcept;
        // logic left move
        static auto LogicLeft(CEDTextDocument& doc, DocPoint) noexcept ->DocPoint;
        // logic right move
        static auto LogicRight(CEDTextDocument& doc, DocPoint) noexcept->DocPoint;
        // word left move
        static auto WordLeft(CEDTextDocument& doc, DocPoint) noexcept->DocPoint;
        // word right move
        static auto WordRight(CEDTextDocument& doc, DocPoint) noexcept->DocPoint;
        // on alloc undo-op failed
        static void AllocUndoFailed(CEDTextDocument& doc) noexcept;
        // create inline object
        static auto CreateInline(CEDTextDocument&, const InlineInfo &, int16_t , CellType ) noexcept->CEDTextCell*;
        // viewpoint change
        static void ViewPoint(CEDTextDocument&, Point point) noexcept;
        // value changed
        static void ValueChanged(CEDTextDocument& doc, uint32_t flag) noexcept { doc.m_flagChanged |= flag; }
        // need redraw
        static void NeedRedraw(CEDTextDocument& doc) noexcept { ValueChanged(doc, Changed_View); }
        // check estimated
        static void CheckEstimated(CEDTextDocument& doc) noexcept;
    };
    // RichData ==
    inline bool operator==(const RichData& a, const RichData& b) noexcept {
        return !std::memcmp(&a, &b, sizeof(a)); }
    // RichData !==
    inline bool operator!=(const RichData& a, const RichData& b) noexcept {
        return !(a == b); }
    // cmp
    inline auto Cmp(DocPoint dp) noexcept {
        uint64_t u64;
        u64 = (uint64_t(dp.line) << 32) | uint64_t(dp.pos);
        return u64;
    }
    // selection swap
    inline void CmpSwap(DocPoint& a, DocPoint& b) noexcept {
        if (Cmp(a) > Cmp(b)) std::swap(a, b);
    }
    // Lower bound to VL
    static auto LowerVL(VisualLine* b, VisualLine* e, uint32_t logic_line) noexcept {
        const auto cmp = [](const VisualLine& vl, uint32_t ll) noexcept { return vl.lineno < ll; };
        return std::lower_bound(b, e, logic_line, cmp);
    }
#ifndef NDEBUG
    /// <summary>
    /// Gets the length of the line text.
    /// </summary>
    /// <param name="cell">The cell.</param>
    /// <returns></returns>
    static uint32_t GetLineTextLength(CEDTextCell* cell) noexcept {
        uint32_t length = 0;
        while (true) {
            length += cell->RefString().length;
            if (cell->RefMetaInfo().eol) break;
            cell = impl::next_cell(cell);
        }
        return length;
    }
#endif

}


/// <summary>
/// Determines whether the specified document is record.
/// 检查是否正在记录操作
/// </summary>
/// <param name="doc">The document.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::IsRecord(
    const CEDTextDocument & doc) noexcept {
    return !!doc.m_uUndoOp;
}

/// <summary>
/// Initializes a new instance of the <see cref="CEDTextDocument" /> class.
/// </summary>
/// <param name="plat">The plat.</param>
/// <param name="arg">The argument.</param>
RichED::CEDTextDocument::CEDTextDocument(IEDTextPlatform& plat, const DocInitArg& arg) noexcept
: platform(plat) {
    // 计算
    auto cal_password = [](char32_t ch) noexcept {
        union { char32_t u32; char16_t u16[2]; };
        // utf32 -> utf16
        if (ch > 0xFFFF) {
            // From http://unicode.org/faq/utf_bom.html#35
            u16[0] = static_cast<char16_t>(0xD800 + (ch >> 10) - (0x10000 >> 10));
            u16[1] = static_cast<char16_t>(0xDC00 + (ch & 0x3FF));
        }
        else u16[0] = u16[1] = static_cast<char16_t>(ch);
        return u32;
    };
    // 初始化
    this->default_riched = arg.riched;
    RichED::InitMatrix(m_matrix, arg.read, arg.flow);
    // 默认使用CRLF
    m_linefeed.AsCRLF();
    // 初始化相关数据
    m_rcViewport = { 0, 0, 0, 0 };
    m_rcCaret = { 0, 0, 1, arg.riched.size };
    m_szEstimated = { 0, 0 };
    m_szEstimatedCmp = { 0, 0 };
    m_dpAnchor = { 0, 0 };
    m_dpCaret = { 0, 0 };
    m_dpSelBegin = { 0, 0 };
    m_dpSelEnd = { 0, 0 };
    // 处理Cell节点
    m_head.prev = nullptr;
    m_head.next = &m_tail;
    m_tail.prev = &m_head;
    m_tail.next = nullptr;
    // 初始化INFO
    std::memset(&m_info, 0, sizeof(m_info));
    m_info.length_max       = arg.length_max;
    m_info.flags            = arg.flags;
    m_info.password_cha16x2 = cal_password(arg.password);
    m_info.fixed_lineheight = arg.fixed_lineheight;
    m_info.valign           = arg.valign;
    //m_info.talign           = arg.talign;
    m_info.wrap_mode        = arg.wrap_mode;
    m_bPassword4 = arg.password > 0xFFFF;
#ifndef NDEBUG
    // 防止越界用调试缓存
    std::memset(m_dbgBuffer, 233, sizeof(m_dbgBuffer));
    // FLAG检查
    if (arg.flags & Flag_UsePassword) {
        //assert(!(arg.flags & Flag_MultiLine) && "multiline password?");
        assert(!(arg.flags & Flag_RichText) && "rich password?");
    }
#endif
    // 初始CELL
    const auto cell = RichED::CreateNormalCell(*this, arg.riched);
    // 缓存
    m_vLogic.Resize(RED_INIT_ARRAY_BUFLEN, plat);
    m_vVisual.Resize(RED_INIT_ARRAY_BUFLEN, plat);
    if (m_vVisual.IsFailed() | m_vLogic.IsFailed() | !cell) {
        arg.code = DocInitArg::CODE_OOM;
        return;
    }
    // 第一行数据
    cell->AsEOL();
    RichED::InsertAfterFirst(m_head, *cell);
    m_vLogic.Resize(1, plat);
    m_vLogic[0] = { cell, 0 };
    m_vVisual.Resize(1, plat);
    //m_vVisual[0] = { static_cast<CEDTextCell*>(&m_head), uint32_t(-1) };
    m_vVisual[0] = { cell, 0 };
    // TODO: OOM 后 再调用时m_vLogic/m_vLogic判断?
}

/// <summary>
/// Finalizes an instance of the <see cref="CEDTextDocument"/> class.
/// </summary>
/// <returns></returns>
RichED::CEDTextDocument::~CEDTextDocument() noexcept {
    // 释放CELL链
    auto cell = impl::next_cell(&m_head);
    while (cell != &m_tail) {
        const auto node = cell;
        cell = impl::next_cell(cell);
        node->Dispose();
    }
    m_head.next = &m_tail;
    m_tail.prev = &m_head;
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
auto RichED::CEDTextDocument::Update() noexcept -> ValuedChanged {
    // 调整宽度可能会导致插入符移动
    if ((m_flagChanged & Changed_ViewportWidth) && m_info.wrap_mode) {
        // 调整
        Private::RefreshCaret(*this, m_dpCaret, nullptr);
    }
    // 重绘
    if (m_flagChanged & Changed_View) {
        const auto bottom = m_rcViewport.y + m_rcViewport.height;
        const auto maxbtm = max_unit();
        Private::ExpandVL(*this, uint32_t(-1), bottom);
        Private::CheckEstimated(*this);
        m_szEstimatedCmp = m_szEstimated;
#ifndef NDEBUG
        if (m_bUpdateDbg) {
            this->platform.DebugOutput("view update more than once before rendering.", true);
        }
        m_bUpdateDbg = true;
        this->platform.DebugOutput("<BeforeRender>", false);
#endif // !NDEBUG
    }
    // 返回
    const auto rv = m_flagChanged;
    m_flagChanged = 0;
    return static_cast<ValuedChanged>(rv);
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void RichED::CEDTextDocument::Render(CtxPtr ctx) noexcept {
#ifndef NDEBUG
    m_bUpdateDbg = false;
#endif
    const auto count = m_vVisual.GetSize();
    if (!count) return;
    // TODO: 固定行高
    const auto data = m_vVisual.GetData();
    const auto end_line = data + count - 1;
    // 计算起点
    auto this_line = data;
    // l 逻辑/布局 ltrb 左上右下
    const auto view_lt = m_rcViewport.y;
    const auto view_lb = m_rcViewport.y + m_rcViewport.height;
    const auto view_ll = m_rcViewport.x;
    const auto view_lr = m_rcViewport.x + m_rcViewport.width;
    while (this_line != end_line) {
        const auto next_line = this_line + 1;
        if (next_line->offset >= view_lt) break;
        this_line = next_line;
    }
    // 计算右边
    const auto cal_layout_right = [](CEDTextCell& cell) noexcept {
        return cell.metrics.bounding.right + cell.metrics.pos + cell.metrics.offset.x;
    };
    // [0, count - 1)
    while (this_line != end_line) {
        const auto next_line = this_line + 1;
        // 计算起点
        const auto start_point = [=](CEDTextCell* cell) noexcept {
            while (cal_layout_right(*cell) <= view_ll) {
                const auto cur = cell;
                cell = static_cast<CEDTextCell*>(cur->next);
                if (cur->RefMetaInfo().eol) break;
            }
            return cell;
        }(this_line->first);
        // 获取循环表
        const auto cells = impl::cfor_cells(start_point, next_line->first);
        const auto baseline = this_line->offset + this_line->ar_height_max;
        for (auto& cell : cells) {
            this->platform.DrawContext(ctx, cell, baseline);
            // 超过就退出
            if (cal_layout_right(cell) >= view_lr 
                && cell.RefMetaInfo().metatype != Type_UnderRuby) break;
        }
        if (next_line->offset >= view_lb) break;
        this_line = next_line;
    }
}


/// <summary>
/// Sets the position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void RichED::CEDTextDocument::MoveViewportAbs(Point pos) noexcept {
    Point target_view;
    // 垂直布局
    if (m_matrix.read_direction & 1) {
        target_view.x = pos.y;
        target_view.y = pos.x;
    }
    // 水平布局
    else {
        target_view.x = pos.x;
        target_view.y = pos.y;
    }
    Private::ViewPoint(*this, target_view);
}


/// <summary>
/// Sets the position plus.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void RichED::CEDTextDocument::MoveViewportRel(Point pos) noexcept {
    Point target_view{ m_rcViewport.x, m_rcViewport.y };
    // 垂直布局
    if (m_matrix.read_direction & 1) {
        target_view.x += pos.y;
        target_view.y += pos.x;
    }
    // 水平布局
    else {
        target_view.x += pos.x;
        target_view.y += pos.y;
    }
    Private::ViewPoint(*this, target_view);
}


/// <summary>
/// Inserts the text.
/// </summary>
/// <param name="dp">The dp.</param>
/// <param name="view">The view.</param>
/// <param name="riched">The riched.</param>
/// <param name="behind">if set to <c>true</c> [behind].</param>
/// <returns></returns>
auto RichED::CEDTextDocument::InsertText(
    DocPoint dp, U16View view, bool behind) noexcept -> DocPoint {
    if (dp.line < m_vLogic.GetSize()) {
        const auto line_data = m_vLogic[dp.line];
        dp.pos = std::min(dp.pos, line_data.length);
        // 获取偏移量
        const auto move = impl::lfcount(view);
        DocPoint after = dp;
        // 往右移动指定数量位置
        after.line += move.line;
        // 如果存在换行则是下一行的目的地址
        if (move.line) after.pos = move.pos;
        // 否则是偏移量
        else after.pos += move.pos;

        // 处理存在撤销栈的情况
        if (Private::IsRecord(*this)) {
            // 记录文本
            Private::RecordTextEx(*this, dp, after, view);
        }
        if (Private::Insert(*this, dp, view, line_data, behind)) return after;
        assert(!"FAILED");
    }
    assert(!"OUT OF RANGE");
    return dp;
}


/// <summary>
/// Inserts the ruby.
/// </summary>
/// <param name="dp">The dp.</param>
/// <param name="ch">The ch.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::InsertRuby(DocPoint dp, char32_t ch, U16View view, const RichData* rich_ptr) noexcept {
    assert(ch && "bad char");
    if (dp.line < m_vLogic.GetSize()) {
        // 不能包含换行信息
        const auto real_view = impl::lfview(view);
        if (real_view.first == real_view.second) return assert(!"empty"), false;
        auto& line_data = m_vLogic[dp.line];
        dp.pos = std::min(dp.pos, line_data.length);
        auto riched = rich_ptr ? *rich_ptr : default_riched; 
        // 处理存在撤销栈的情况
        if (Private::IsRecord(*this)) {
            // 记录文本
            Private::RecordRubyEx(*this, dp, riched, ch, real_view);
        }
        // 创建一个简单的
        riched.size = half(riched.size);
        const auto cell = RichED::CreateShrinkedCell(*this, riched);;
        if (!cell) return false;
        const_cast<CellMeta&>(cell->RefMetaInfo()).metatype = Type_UnderRuby;
        auto& str = const_cast<FixedStringA&>(cell->RefString());
        str.capacity = str.length = impl::utf32to16(ch, str.data);
        // 插入内联对象
        if (!Private::Insert(*this, dp, *cell, line_data)) return false;
        // 插入普通数据
        dp.pos += cell->RefString().length;
        const auto rv = Private::Insert(*this, dp, real_view, line_data, false);
        cell->SetRichED(default_riched);
        return rv;
    }
    assert(!"OUT OF RANGE");
    return false;
}

/// <summary>
/// Sets the anchor caret.
/// </summary>
/// <param name="anchor">The anchor.</param>
/// <param name="caret">The caret.</param>
/// <returns></returns>
void RichED::CEDTextDocument::SetAnchorCaret(
    DocPoint anchor, DocPoint caret) noexcept {
    m_dpAnchor = anchor;
    Private::SetSelection(*this, nullptr, caret, impl::mode_target, true);
    Private::RefreshCaret(*this, m_dpCaret, nullptr);

    //Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);

    CmpSwap(anchor, caret);
    m_dpSelBegin = anchor;
    m_dpSelEnd = caret;
    Private::RefreshSelection(*this, anchor, caret);
}


/// <summary>
/// Ranks up magic for RUBY
/// </summary>
/// <param name="dp">The dp.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void RichED::CEDTextDocument::RankUpMagic(DocPoint dp, uint32_t len) noexcept {
    // 行检测
    if (dp.line >= m_vLogic.GetSize()) return assert(!"OUT OF RANGE");
    const auto data = m_vLogic[dp.line];
    // 位检测
    if (dp.pos >= data.length) return assert(!"OUT OF RANGE");
    // 处理
    auto cell = data.first;
    auto pos = dp.pos;
    impl::find_cell2_txtoff_ex(cell, pos);
    Private::Dirty(*this, *cell, dp.line);
    assert(pos < cell->RefString().length);
    // 单字
    uint32_t end_pos = pos + 1;
    // 双字
    if (impl::is_1st_surrogate(cell->RefString().data[pos]))
        end_pos++;
    if (!cell->Split(end_pos)) return;
    if (cell = cell->Split(pos)) {
        // RANK UP!
        const_cast<CellType&>(cell->RefMetaInfo().metatype) = Type_UnderRuby;
        auto& str = const_cast<FixedStringA&>(cell->RefString());
        str.capacity = str.length;
        cell->AsDirty();
        // 后面标记为注音
        while (!cell->RefMetaInfo().eol) {
            cell = impl::next_cell(cell);
            if (len <= cell->RefString().length) {
                cell->Split(len);
                const_cast<CellType&>(cell->RefMetaInfo().metatype) = Type_Ruby;
                break;
            }
            len -= cell->RefString().length;
            const_cast<CellType&>(cell->RefMetaInfo().metatype) = Type_Ruby;
        }
    }
}

/// <summary>
/// Ranks up magic.
/// </summary>
/// <param name="dp">The dp.</param>
/// <param name="info">The information.</param>
/// <param name="len">The length.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void RichED::CEDTextDocument::RankUpMagic(
    DocPoint dp, const InlineInfo& info, int16_t len, CellType type) noexcept {
    if (dp.line < m_vLogic.GetSize()) {
        auto& data = m_vLogic[dp.line];
        if (dp.pos < data.length) {
            // 遍历至指定地点
            const auto next_is_first = data.first->prev;
            auto cell = data.first;
            auto pos = dp.pos;
            impl::find_cell2_txtoff_ex(cell, pos);
            Private::Dirty(*this, *cell, dp.line);
            assert(pos < cell->RefString().length);
            // 分离对象
            if (!cell->Split(pos + 1)) return;
            // 创建对象
            const auto obj = Private::CreateInline(*this, info, len, type);
            if (!obj) return;
            // EOL信息
            cell->MoveEOL(*obj);
            RichED::InsertAfterFirst(*cell, *obj);
            cell->RemoveTextEx({ pos , 1 });
            // 防止失效
            data.first = impl::next_cell(next_is_first);
            return;
        }
    }
    assert(!"OUT OF RANGE");
}

/// <summary>
/// Creates the inline object.
/// </summary>
/// <param name="info">The information.</param>
/// <param name="len">The length.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::Private::CreateInline(
    CEDTextDocument& doc,
    const InlineInfo &info, int16_t len, CellType type) noexcept -> CEDTextCell * {
    assert(len >= 0);
    const auto ptr = RichED::CreatePublicCell(doc, doc.default_riched, len, 1);
    if (ptr) {
        const_cast<CellType&>(ptr->RefMetaInfo().metatype) = type;
        std::memcpy(ptr->GetExtraInfo(), &info, len);
        auto& str = const_cast<FixedStringA&>(ptr->RefString());
        str.length = str.capacity = 1;
        str.data[0] = ' ';
        str.data[1] = len;
        ptr->AsDirty();
    }
    return ptr;
}

/// <summary>
/// Inserts the inline.
/// </summary>
/// <param name="dp">The dp.</param>
/// <param name="cell">The cell.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::InsertInline(DocPoint dp, 
    const InlineInfo &info, int16_t len, CellType type) noexcept {
    const auto cell = Private::CreateInline(*this, info, len, type);
    if (!cell) return false;
    // 正式处理
    const auto rv = [this, dp, cell]() mutable noexcept {
        if (dp.line < m_vLogic.GetSize()) {
            auto& line_data = m_vLogic[dp.line];
            dp.pos = std::min(dp.pos, line_data.length);
            // 处理存在撤销栈的情况
            if (Private::IsRecord(*this)) {
                // 记录对象
                Private::RecrodObjsEx(*this, dp, *cell);
            }
            return Private::Insert(*this, dp, *cell, line_data);
        }
        return false;
    }();
    // 失败则释放对象
    if (!rv) cell->Dispose();
    return rv;
}

/// <summary>
/// Removes the text.
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::RemoveText(DocPoint begin, DocPoint end) noexcept {
    CheckRangeCtx ctx;
    if (!Private::CheckRange(*this, begin, end, ctx)) return false;
    // 处理存在撤销栈的情况
    if (Private::IsRecord(*this)) {
        // 富文本的情况
        if (m_info.flags & Flag_RichText) {
            // 移除/记录对象
            Private::RecordObjs(*this, begin, ctx);
            // 记录富属性
            Private::RecordRich(*this, begin, ctx, nullptr);
        }
        // 记录文本
        Private::RecordText(*this, begin, end);
    }
    // 移除文本
    return Private::RemoveText(*this, begin, end, ctx);
}

/// <summary>
/// Resizes the specified size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void RichED::CEDTextDocument::ResizeViewport(Size size) noexcept {
    uint32_t flag = 0;
    // 垂直布局
    if (m_matrix.read_direction & 1) {
        if (m_rcViewport.width != size.height) flag |= Changed_ViewportWidth;
        if (m_rcViewport.height != size.width) flag |= Changed_ViewportHeight;
        m_rcViewport.width = size.height;
        m_rcViewport.height = size.width;
    }
    // 水平布局
    else {
        if (m_rcViewport.width != size.width) flag |= Changed_ViewportWidth;
        if (m_rcViewport.height != size.height) flag |= Changed_ViewportHeight;
        m_rcViewport.width = size.width;
        m_rcViewport.height = size.height;
    }
    // 标记修改
    Private::ValueChanged(*this, flag);
}

/// <summary>
/// set Views the point.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="point">The point.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::ViewPoint(
    CEDTextDocument& doc, Point point) noexcept {
    // 修改才改变
    if (point.x == doc.m_rcViewport.x && point.y == doc.m_rcViewport.y) return;
    doc.m_rcViewport.x = point.x;
    doc.m_rcViewport.y = point.y;
    // 重绘
    Private::ValueChanged(doc, Changed_View);
}


/// <summary>
/// Gens the text.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
template<typename T, typename U>
void RichED::CEDTextDocument::Private::GenText(
    CEDTextDocument& doc, DocPoint begin, DocPoint end, 
    T append, U linefeed) noexcept {
    // TODO: 全文获取?

    CheckRangeCtx ctx;
    // 检测范围合理性
    if (!Private::CheckRange(doc, begin, end, ctx)) return;
    // 同一个CELL
    if (ctx.begin.cell == ctx.end.cell) {
        const auto ptr = ctx.end.cell->RefString().data;
        append({ ptr + ctx.begin.offset, ptr + ctx.end.offset });
    }
    else {
        const auto ptr0 = ctx.begin.cell->RefString().data;
        const auto len0 = ctx.begin.cell->RefString().length;
        const auto ptr1 = ctx.end.cell->RefString().data;
        // A
        append({ ptr0 + ctx.begin.offset, ptr0 + len0 });
        if (ctx.begin.cell->RefMetaInfo().eol) linefeed();
        // B
        const auto cfor = impl::cfor_cells(ctx.begin.cell->next, ctx.end.cell);
        for (auto& cell : cfor) {
            append(cell.View());
            if (cell.RefMetaInfo().eol) linefeed();
        }
        // C
        append({ ptr1, ptr1 + ctx.end.offset });
    }
}

/// <summary>
/// GetEstimatedSize in view sapce
/// </summary>
/// <returns></returns>
auto RichED::CEDTextDocument::GetEstimatedSize() const noexcept -> Size {
    auto size = m_szEstimated;
    if (m_matrix.read_direction & 1) std::swap(size.width, size.height);
    return size;
}

/// <summary>
/// Gens the text.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void RichED::CEDTextDocument::GenText(CtxPtr ctx, DocPoint begin, DocPoint end) noexcept {
    auto& plat = this->platform;
    const auto lf = m_linefeed.View();
    // XXX: AppendText OOM处理
    const auto append_text = [&plat, ctx](U16View view) noexcept {
        plat.AppendText(ctx, view);
    };
    const auto line_feed = [&plat, ctx, lf]() noexcept {
        plat.AppendText(ctx, lf);
    };
    Private::GenText(*this, begin, end, append_text, line_feed);
}

/// <summary>
/// Sets the line feed.
/// </summary>
/// <param name="lf">The lf.</param>
/// <returns></returns>
void RichED::CEDTextDocument::SetLineFeed(const LineFeed lf) noexcept {
    m_linefeed = lf;
    // 文本修改
    Private::ValueChanged(*this, Changed_Text);
}

/// <summary>
/// valign helper
/// </summary>
/// <param name="ar">The ar.</param>
/// <param name="height">The height.</param>
/// <param name="m">The m.</param>
/// <returns></returns>
void RichED::CEDTextDocument::VAlignHelperH(unit_t ar, unit_t height, CellMetrics& m) noexcept {
    switch (m_info.valign & 3)
    {
    case VAlign_Baseline:
        m.ar_height = ar;
        m.dr_height = height - ar;
        break;
    case VAlign_Ascender:
        m.ar_height = 0;
        m.dr_height = height;
        break;
    case VAlign_Middle:
        m.dr_height = m.ar_height = half(height);
        break;
    case VAlign_Descender:
        m.ar_height = height;
        m.dr_height = 0;
        break;
    }
}


/// <summary>
/// Forces the reset all riched.
/// </summary>
/// <returns></returns>
void RichED::CEDTextDocument::ForceResetAllRiched() noexcept {
    // 遍历所有节点
    auto cell = impl::next_cell(&m_head);
    while (cell != &m_tail) {
        const auto node = cell;
        cell = impl::next_cell(cell);
        node->SetRichED(this->default_riched);
        node->Sleep();
    }
    // 标记为脏
    m_vVisual.ReduceSize(1);
    //Private::Dirty(*this, *impl::next_cell(&m_head), 0);
    Private::NeedRedraw(*this);
    Private::RefreshCaret(*this, m_dpCaret, nullptr);
}

/// <summary>
/// Begins the op.
/// </summary>
/// <returns></returns>
void RichED::CEDTextDocument::BeginOp() noexcept {
    assert(m_uUndoOp == 0);
    m_uUndoOp = m_uUndoIsOk;
}


/// <summary>
/// Ends the op.
/// </summary>
/// <returns></returns>
void RichED::CEDTextDocument::EndOp() noexcept {
    assert(m_uUndoOp >= m_uUndoIsOk);
    m_uUndoOp = 0;
    // 检查撤销栈长度
}

// ----------------------------------------------------------------------------
//                                GUI Operation
// ----------------------------------------------------------------------------


/// <summary>
/// Mouses the specified document.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="pt">The pt.</param>
/// <param name="hold">if set to <c>true</c> [hold].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::Mouse(
    CEDTextDocument & doc, Point pt, bool hold) noexcept{
    HitTestCtx ctx;
    // 屏幕坐标空间映射到文档坐标空间
    pt = doc.m_matrix.ScreenToDoc(pt);
    // 针对鼠标位置的命中测试
    if (Private::HitTest(doc, pt, ctx)) {
        const DocPoint dp {
            ctx.visual_line->lineno,
            ctx.len_before_cell + ctx.pos_in_cell
        };
        // 设置选择
        Private::SetSelection(doc, &ctx, dp, impl::mode_target, hold);
        // 更新选择区域
        Private::UpdateSelection(doc, doc.m_dpCaret, doc.m_dpAnchor);
        return true;
    }
    return true;
}


/// <summary>
/// Deletes the selection.
/// </summary>
/// <param name="doc">The document.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::DeleteSelection(CEDTextDocument & doc) noexcept {
    // 存在选择区
    if (Cmp(doc.m_dpSelBegin) == Cmp(doc.m_dpSelEnd)) return true;
    const auto point = doc.m_dpSelBegin;
    // 删除文本
    doc.RemoveText(doc.m_dpSelBegin, doc.m_dpSelEnd);
    // 设置选择区
    Private::SetSelection(doc, nullptr, point, impl::mode_target, false);
    // 更新选择区域
    Private::UpdateSelection(doc, doc.m_dpCaret, doc.m_dpAnchor);
    return false;
}

/// <summary>
/// GUIs the l button up.
/// </summary>
/// <param name="pt">The pt.</param>
/// <returns></returns>
//bool RichED::CEDTextDocument::GuiLButtonUp(Point pt) noexcept {
//    return false;
//}

/// <summary>
/// GUIs the l button down.
/// </summary>
/// <param name="pt">The pt.</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiLButtonDown(Point pt, bool shift) noexcept {
    return Private::Mouse(*this, pt, false);
}

/// <summary>
/// GUIs the l button hold.
/// </summary>
/// <param name="pt">The pt.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiLButtonHold(Point pt) noexcept {
    return Private::Mouse(*this, pt, true);
}

/// <summary>
/// GUI: the character.
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiChar(char32_t ch) noexcept {
#ifndef NDEBUG
    if (!((ch >= 0x20 && ch != 0x7f) || ch == '\t')) {
        const auto msg = "<CEDTextDocument::GuiChar>: control char NOT accepted.";
        this->platform.DebugOutput(msg, true);
    }
#endif
    char16_t buf[2];
    const auto len = impl::utf32to16(ch, buf);
    return this->GuiText({ buf, buf + len });
}


/// <summary>
/// GUIs the text.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiText(U16View view) noexcept {
    assert(view.second >= view.first);
    // 只读
    if (m_info.flags & Flag_GuiReadOnly) return false;
    // 超过指定长度
    const uint32_t view_len = static_cast<uint32_t>(view.second - view.first);
    if (view_len + m_info.total_length > m_info.length_max) {
        view.second = view.first;
        if (m_info.length_max > m_info.total_length) {
            view.second += m_info.length_max - m_info.total_length;
            if (impl::is_2nd_surrogate(*view.second)) 
                --view.second;
        }
    }
    // 没有头发
    if (view.second == view.first) return false;
    // 记录下来
    impl::op_recorder recorder{ *this };
    // 删除选择区
    Private::DeleteSelection(*this);
    // 输入密码
    if (m_info.flags & Flag_UsePassword) {
        return this->gui_password(view);
    }
    // 正常插入
    else {
        const auto target = this->InsertText(m_dpCaret, view, true);
        // 设置选择
        Private::SetSelection(*this, nullptr, target, impl::mode_target, false);
        return true;
    }
}


/// <summary>
/// GUIs the password.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::gui_password(U16View view) noexcept {
    char32_t pwbuf32[PASSWORD_INPUT_BUFFER];
    auto caret_target = m_dpCaret;
    // 插入当前缓冲数据
    const auto buf_insert = [&pwbuf32, &caret_target, this](const uint32_t count) noexcept {
        char16_t pwbuf16[PASSWORD_INPUT_BUFFER * 2];
        auto itr = pwbuf16;
        for (uint32_t i = 0; i != count; ++i)
            itr += impl::utf32to16(pwbuf32[i], itr);
        caret_target = this->InsertText(caret_target, { pwbuf16 , itr }, true);
    };
    uint32_t char_count = 0, char_total = 0;
    // 便利字符串
    while (view.first < view.second) {
        char32_t ch = *view.first;
        // 将U16转换成U32
        if (impl::is_1st_surrogate(*view.first)) {
            ch = impl::char16x2to32(view.first[0], view.first[1]);
            ++view.first;
            assert(impl::is_2nd_surrogate(*view.first));
            assert(view.first < view.second);
        }
        // 再判断合法性
        if (platform.IsValidPassword(ch)) {
            pwbuf32[char_count++] = ch;
            // 缓冲满了则插入
            if (char_count == PASSWORD_INPUT_BUFFER) {
                char_total += char_count;
                buf_insert(char_count);
                char_count = 0;
            }
        }
        ++view.first;
    }
    char_total += char_count;
    // 输入无效密码
    if (char_total) {
        if (char_count) buf_insert(char_count);
        Private::SetSelection(*this, nullptr, caret_target, impl::mode_target, false);
        return true;
    }
    return false;
}


//PCN_NOINLINE
/// <summary>
/// Recreates the context.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void RichED::CEDTextDocument::recreate_context(CEDTextCell& cell) noexcept {
#if 0
    U16View view;
    view.first = cell.RefString().data;
    view.second = view.first + cell.RefString().length;
    // 密码模式
    if (m_info.flags & Flag_UsePassword) {

    }
    this->platform.RecreateContext(cell, view);
#else
    this->platform.RecreateContext(cell);
#endif
}

/// <summary>
/// Gets the view.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::get_view(const CEDTextCell & cell) noexcept -> U16View {
    U16View view;
    view.first = cell.RefString().data;
    view.second = view.first + cell.RefString().length;
    return view;
}

/// <summary>
/// Passwords the helper16.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="ch">The ch.</param>
/// <param name="cell">The cell.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::password_helper16(
    char32_t buf[], char32_t ch16x2, bool mode, const CEDTextCell & cell) noexcept -> U16View {
    U16View view; view.first = reinterpret_cast<char16_t*>(buf);
    const auto& str = cell.RefString();
    // XXX: 不用每次计算?
    uint32_t fill_count = impl::count({ str.data, str.data + str.length });
    // utf32 -> utf16
    if (mode) view.second = view.first + fill_count * 2;
    else {
        view.second = view.first + fill_count;
        fill_count = (fill_count + 1) / 2;
    }
    // 填写
    std::fill_n(reinterpret_cast<char32_t*>(buf), fill_count, ch16x2);
    return view;
}

/// <summary>
/// GUIs the backspace.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiBackspace(bool ctrl) noexcept {
    // 只读
    if (m_info.flags & Flag_GuiReadOnly) return false;
    // 开始记录
    impl::op_recorder recorder{ *this };
    // 删除选择区
    if (Private::DeleteSelection(*this)) {
        // TODO: BACK == LEFT ?
        const auto after = ctrl 
            ? Private::WordLeft(*this, m_dpCaret)
            : Private::LogicLeft(*this, m_dpCaret)
            ;
        if (Cmp(after) == Cmp(m_dpCaret)) return false;
        // 删除
        this->RemoveText(after, m_dpCaret);
        // 设置
        Private::SetSelection(*this, nullptr, after, impl::mode_target, false);
    }
    return true;
}

/// <summary>
/// GUIs the delete.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiDelete(bool ctrl) noexcept {
    // 只读
    if (m_info.flags & Flag_GuiReadOnly) return false;
    // 开始记录
    impl::op_recorder recorder{ *this };
    // 删除选择区
    if (Private::DeleteSelection(*this)) {
        // TODO: DELETE == RIGHT ?
        const auto after = ctrl
            ? Private::WordRight(*this, m_dpCaret)
            : Private::LogicRight(*this, m_dpCaret)
            ;
        if (Cmp(after) == Cmp(m_dpCaret)) return false;
        // 删除
        this->RemoveText(m_dpCaret, after);
    }
    // 设置
    return true;
}

/// <summary>
/// GUIs the return.
/// </summary>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiReturn() noexcept {
    // 单行
    if (m_info.flags & Flag_MultiLine) {
        const char16_t line_feed[1] = { '\n' };
        return this->GuiText({ line_feed, line_feed + 1 });
    }
    return false;
}

/// <summary>
/// GUIs the ruby.
/// </summary>
/// <param name="ch">The ch.</param>
/// <param name="ruby">The ruby.</param>
/// <param name="riched">The riched.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiRuby(char32_t ch, U16View ruby, const RichData* riched) noexcept {
    // 无效
    if (ruby.first == ruby.second) return false;
    // 只读
    if (m_info.flags & Flag_GuiReadOnly) return false;
    // 开始记录
    impl::op_recorder recorder{ *this };
    // 删除选择区
    Private::DeleteSelection(*this);
    // 插入对象
    const auto rv = this->InsertRuby(m_dpCaret, ch, ruby, riched);
    // 逻辑右移
    Private::SetSelection(*this, nullptr, m_dpCaret, impl::mode_logicright, false);
    Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);
    return rv;
}

/// <summary>
/// GUIs the inline.
/// </summary>
/// <param name="info">The information.</param>
/// <param name="len">The length.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiInline(const InlineInfo& info, int16_t len, CellType type) noexcept {
    // 无效
    if (len <= 0) return false;
    // 只读
    if (m_info.flags & Flag_GuiReadOnly) return false;
    // 开始记录
    impl::op_recorder recorder{ *this };
    // 删除选择区
    Private::DeleteSelection(*this);
    // 插入对象
    const auto rv = this->InsertInline(m_dpCaret, info, len, type);
    // 有效
    if (rv) {
        // 逻辑右移
        Private::SetSelection(*this, nullptr, m_dpCaret, impl::mode_logicright, false);
        Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);
    }
    return rv;
}


/// <summary>
/// GUIs the left.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiLeft(bool ctrl, bool shift) noexcept {
    assert(ctrl == 0 || ctrl == 1);
    const uint32_t mode = m_matrix.left_mapper + ctrl;
    Private::SetSelection(*this, nullptr, m_dpCaret, mode, shift);
    // 更新选择区域
    Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);
    return true;
}

/// <summary>
/// GUIs the right.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiRight(bool ctrl, bool shift) noexcept {
    assert(ctrl == 0 || ctrl == 1);
    const uint32_t mode = m_matrix.right_mapper + ctrl;
    Private::SetSelection(*this, nullptr, m_dpCaret, mode, shift);
    // 更新选择区域
    Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);
    return true;
}

/// <summary>
/// GUIs up.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiUp(bool ctrl, bool shift) noexcept {
    return true;
}


/// <summary>
/// GUIs down.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiDown(bool ctrl, bool shift) noexcept {
    return true;
}


/// <summary>
/// GUIs the home.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiHome(bool ctrl, bool shift) noexcept {
    assert(ctrl == 0 || ctrl == 1);
    Private::SetSelection(*this, nullptr, m_dpCaret, impl::mode_home + ctrl, shift);
    Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);
    return true;
}

/// <summary>
/// GUIs the end.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiEnd(bool ctrl, bool shift) noexcept {
    assert(ctrl == 0 || ctrl == 1);
    Private::SetSelection(*this, nullptr, m_dpCaret, impl::mode_end + ctrl, shift);
    Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);
    return true;
}

/// <summary>
/// GUIs the page up.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiPageUp(bool ctrl, bool shift) noexcept {
    return false;
}

/// <summary>
/// GUIs the page down.
/// </summary>
/// <param name="ctrl">if set to <c>true</c> [control].</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiPageDown(bool ctrl, bool shift) noexcept {
    return false;
}


/// <summary>
/// GUIs the select all.
/// </summary>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiSelectAll() noexcept {
    // 设置选择区
    Private::SetSelection(*this, nullptr, {}, impl::mode_all, true);
    // 更新选择区
    Private::UpdateSelection(*this, m_dpCaret, m_dpAnchor);
    return true;
}

/// <summary>
/// GUIs the srcoll view.
/// </summary>
/// <param name="">The .</param>
/// <param name="shift_direction">if set to <c>true</c> [shift direction].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiSrcollView(unit_t, bool shift_direction) noexcept {
    return false;
}

/// <summary>
/// GUIs the undo.
/// </summary>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiUndo() noexcept {
    return m_undo.Undo(*this);
}

/// <summary>
/// GUIs the redo.
/// </summary>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiRedo() noexcept {
    return m_undo.Redo(*this);
}


/// <summary>
/// GUIs the redo.
/// </summary>
/// <returns></returns>
bool RichED::CEDTextDocument::GuiHasText() const noexcept {
    return m_vLogic.GetSize() && m_vLogic[0].first->RefString().length;
}

// ----------------------------------------------------------------------------
//                                Set RichText Format
// ----------------------------------------------------------------------------


/// <summary>
/// GUIs the riched.
/// </summary>
/// <param name="offset">The offset.</param>
/// <param name="size">The size.</param>
/// <param name="data">The data.</param>
/// <param name="relayout">if set to <c>true</c> [relayout].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::gui_riched(uint32_t offset, uint32_t size, const void * data, bool relayout) noexcept {
    // 只读
    if (m_info.flags & Flag_GuiReadOnly) return false;
    // TODO: 没有选择的时候应该将默认的富属性修改为目标?
    if (Cmp(m_dpSelBegin) == Cmp(m_dpSelEnd)) return false;
    // 尝试记录
    impl::op_recorder recorder{ *this };
    // 正式调用
    const auto rv = this->set_riched(m_dpSelBegin, m_dpSelEnd, offset, size, data, relayout);
    // 重新布局?
    if (relayout) {
        // 强制更新选择区、插入符
        if (Cmp(m_dpCaret) != Cmp(m_dpSelBegin))
            Private::RefreshCaret(*this, m_dpCaret, nullptr);
        Private::RefreshSelection(*this, m_dpSelBegin, m_dpSelEnd);
    }
    Private::NeedRedraw(*this);
    return rv;
}


/// <summary>
/// GUIs the flags.
/// </summary>
/// <param name="flags">The flags.</param>
/// <param name="set">The set.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::gui_flags(uint16_t flags, uint32_t set) noexcept {
    // 只读
    if (m_info.flags & Flag_GuiReadOnly) return false;
    // TODO: 没有选择的时候应该将默认的富属性修改为目标?
    if (Cmp(m_dpSelBegin) == Cmp(m_dpSelEnd)) return false;
    // 尝试记录
    impl::op_recorder recorder{ *this };
    // 正式调用
    const auto rv = this->set_flags(m_dpSelBegin, m_dpSelEnd, flags, set);
    // 重新布局?
    if (set & set_fflags) {
        // 强制更新选择区、插入符
        if (Cmp(m_dpCaret) != Cmp(m_dpSelBegin))
            Private::RefreshCaret(*this, m_dpCaret, nullptr);
        Private::RefreshSelection(*this, m_dpSelBegin, m_dpSelEnd);
    }
    Private::NeedRedraw(*this);
    return rv;
}


/// <summary>
/// Sets the riched.
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="offset">The offset.</param>
/// <param name="size">The size.</param>
/// <param name="data">The data.</param>
/// <param name="relayout">if set to <c>true</c> [relayout].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::set_riched(
    DocPoint begin, DocPoint end,
    uint32_t offset, uint32_t size,
    const void * data, bool relayout) noexcept {
    CheckRangeCtx ctx;
    RichExCtx richex;
    CellPoint cp[2];
    // 修改数据
    const auto set_data = [data, offset, size](CEDTextCell& cell) noexcept {
        auto& rd = const_cast<RichData&>(cell.RefRichED());
        char* const dst = reinterpret_cast<char*>(&rd);
        const char* const src = reinterpret_cast<const char*>(data);
        std::memcpy(dst + offset, src, size);
        cell.AsDirty();
    };
    // 检测范围合理性
    if (!Private::CheckRange(*this, begin, end, ctx)) return false;
    // 针对富属性的范围检测
    if (!Private::RichRange(ctx, cp)) return false;
    // TODO: BEGIN/END修改

    // 处理存在撤销栈的情况
    if (Private::IsRecord(*this)) {
        richex.op.begin = begin;
        richex.op.end = end;
        richex.op.riched = this->default_riched;
        richex.offset = offset;
        richex.length = size;
        richex.relayout = relayout;
        std::memcpy(reinterpret_cast<char*>(&richex.op.riched) + offset, data, size);
        // 记录富属性
        Private::RecordRich(*this, begin, ctx, &richex);
    }
    // 解包
    const auto cell1 = cp[0].cell;
    const auto pos1 = cp[0].offset;
    const auto cell2 = cp[1].cell;
    const auto pos2 = cp[1].offset;
    // 细胞分裂: 由于cell1可能等于cell2, 所以先分裂cell2
    const auto e = cell2->Split(pos2);
    const auto b = cell1->Split(pos1);
    if (b && e) {
        const auto cfor = impl::cfor_cells(b, e);
        for (auto& cell : cfor) set_data(cell);
        // 重新布局
        if (relayout) Private::Dirty(*this, *cell1, begin.line);
        // 增量布局
        else {
            if (cell1 != b) {
                this->recreate_context(*cell1);
                b->metrics.pos = cell1->metrics.pos + cell1->metrics.width;
            }
            if (cell2 != e) {
                // 可能cell1 == cell2
                const auto current = static_cast<CEDTextCell*>(e->prev);
                this->recreate_context(*current);
                e->metrics.pos = current->metrics.pos + current->metrics.width;
            }
        }
        return true;
    }
    return false;
}



/// <summary>
/// Sets the flags.
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="flags">The flags.</param>
/// <param name="set">The set.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::set_flags(
    DocPoint begin, DocPoint end, 
    uint16_t flags, uint32_t set) noexcept {
    RichExCtx richex;
    CheckRangeCtx ctx;
    // 检测范围合理性
    if (!Private::CheckRange(*this, begin, end, ctx)) return false;
    // TODO: 针对富属性的范围检测
    //if (!Private::RichRange(ctx, cp)) return false;
    // 处理存在撤销栈的情况
    if (Private::IsRecord(*this)) {
        richex.op.begin = begin;
        richex.op.end = end;
        richex.op.riched = this->default_riched;
        richex.op.riched.fflags = static_cast<FFlags>(flags);
        richex.offset = set;
        richex.length = 0;
        richex.relayout = false;
        // 记录富属性
        Private::RecordRich(*this, begin, ctx, &richex);
    }
    // 解包
    const auto cell1 = ctx.begin.cell;
    const auto pos1 = ctx.begin.offset;
    const auto cell2 = ctx.end.cell;
    const auto pos2 = ctx.end.offset;

    // 修改的是effect还是flag?
    const size_t flag_offset
        = set & set_fflags
        ? offsetof(RichData, fflags)
        : offsetof(RichData, effect)
        ;
    // 重新布局
    const uint16_t change_font_flags = set & set_fflags;
    static_assert(sizeof(RichData::fflags) == sizeof(uint16_t), "same!");
    static_assert(sizeof(RichData::effect) == sizeof(uint16_t), "same!");
    // 获取标志
    const auto ref_flags = [flag_offset](CEDTextCell& cell) noexcept -> uint16_t& {
        auto& rd = const_cast<RichData&>(cell.RefRichED());
        char* const dst = reinterpret_cast<char*>(&rd);
        uint16_t* const rv = reinterpret_cast<uint16_t*>(dst + flag_offset);
        return *rv;
    };
    // 修改模式
    if (set & Set_Change) {
        // 检查修改点两端的情况: 两段都为true时修改为false, 否则修改为true;
        if ((ref_flags(*cell1) & ref_flags(*cell2) & flags) == flags)
            set = Set_False;
        else
            set = Set_True;
    }
    // 与或标志
    const uint16_t and_flags = ~flags;
    const uint16_t or__flags = set & Set_True ? flags : 0;
    // 修改数据
    const auto set_data = [=](CEDTextCell& cell) noexcept {
        auto& flags = ref_flags(cell);
        flags = (flags & and_flags) | or__flags;
    };
    // 细胞分裂: 由于cell1可能等于cell2, 所以先分裂cell2
    const auto e = cell2->Split(pos2);
    const auto b = cell1->Split(pos1);
    if (b && e) {
        const auto cfor = impl::cfor_cells(b, e);
        Private::NeedRedraw(*this);
        for (auto& cell : cfor) set_data(cell);
        // 重新布局
        if (change_font_flags) {
            for (auto& cell : cfor) cell.AsDirty();
            Private::Dirty(*this, *cell1, begin.line);
        }
        // 增量布局
        else {
            if (cell1 != b) {
                this->recreate_context(*cell1);
                b->metrics.pos = cell1->metrics.pos + cell1->metrics.width;
            }
            if (cell2 != e) {
                // 可能cell1 == cell2
                const auto current = static_cast<CEDTextCell*>(e->prev);
                this->recreate_context(*current);
                e->metrics.pos = current->metrics.pos + current->metrics.width;
            }
        }
        return true;
    }
    return false;
}


// ----------------------------------------------------------------------------
//                      RichED::CEDTextDocument::Private
// ----------------------------------------------------------------------------


/// <summary>
/// Expands the visual-line clean area.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="target_line">The target line.</param>
/// <param name="bottom">The bottom.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::ExpandVL(
    CEDTextDocument& doc, uint32_t target_line, unit_t bottom) noexcept {
    auto& vlv = doc.m_vVisual;
    if (vlv.IsFailed()) return;
    assert(vlv.GetSize() && "bad size");
    // 保证最后一个准确
    const uint32_t count = vlv.GetSize() - 1;
    auto& last = vlv[count];
    assert(last.char_len_before == 0);
    // 已经处理完毕
    if (last.first == &doc.m_tail) return;
    // TODO: 保证最后一个准确, 移动至Dirty函数
    last.first = doc.m_vLogic[last.lineno].first;
    // 视口宽度, 用于自动换行
    const auto viewport_w = doc.m_rcViewport.width;
    // 获取
    auto line = last;
    auto cell = line.first;
    // 已经处理完毕
    if (line.lineno > target_line) return;
    if (line.offset >= bottom) return;
    // 估计宽度: 如果从0开始则从0计算宽度
    auto est_width = doc.m_szEstimated.width;
    if (!count) est_width = 0;

#ifndef NDEBUG
    doc.platform.DebugOutput("<ExpandVL>", false);
#endif // !NDEBUG


    // 正式开始
    vlv.Resize(count, doc.platform);
    line.ar_height_max = line.dr_height_max = 0;
    unit_t offset_inline = 0;
    uint32_t char_length_vl = 0;
    //uintptr_t new_line = 0;
    // 起点为无效起点
    while (cell != &doc.m_tail) {
        /*
         1. 在'条件允许'下尝试合并后面的CELL
         2. 遍历需要换行CELL处(```x + w >= W```)
         3. (a.)如果是内联之类的特殊CELL, 或者换行处足够靠前:
                    要么换行, 要么保留(视觉行第一个)
         4. (b.)查询可换行处, 并分裂为A, B: B换行
        */


        // 尝试合并后CELL
        if (Private::Merge(doc, *cell, viewport_w, offset_inline)) 
            cell->MergeWithNext();

        bool this_eol = cell->RefMetaInfo().eol;
        bool new_line = this_eol;
        //

        // 重建脏CELL
        Private::Recreate(doc, *cell);
        // 自动换行
        const auto offset_end = offset_inline + cell->metrics.width;
        if (doc.m_info.wrap_mode && offset_end > viewport_w && cell->metrics.width > 0) {
            // 整个CELL换行
            if (cell->RefMetaInfo().metatype >= Type_InlineObject ||
                offset_inline + cell->RefRichED().size > viewport_w ||
                Private::CheckWrap(doc, *cell, viewport_w - offset_inline) == cell
                ) {
                // -------------------------
                // --------------------- BOVL
                // -------------------------
                line.char_len_this = char_length_vl;
                // 换行
                if (!impl::push_data(vlv, line, doc.platform)) return;
                cell->metrics.pos = 0;
                // 这里换行不是逻辑
                line.char_len_before += char_length_vl;
                char_length_vl = 0;
                offset_inline = 0;
                // 行偏移 = 上一行偏移 + 上一行最大升高 + 上一行最大降高
                line.first = cell;
                line.offset += line.ar_height_max + line.dr_height_max;
                line.ar_height_max = cell->metrics.ar_height;
                line.dr_height_max = cell->metrics.dr_height;
            }
            // 其他情况
            else {
                new_line = true;
                this_eol = cell->RefMetaInfo().eol;
                // 重建脏CELL
                if (cell->RefMetaInfo().dirty) doc.recreate_context(*cell);
            }
        }

        // --------------------------
        // --------------------- EOVL
        // --------------------------
        

        // 行内偏移
        cell->metrics.pos = offset_inline;
        offset_inline += cell->metrics.width;
        est_width = std::max(offset_inline, est_width);
        char_length_vl += cell->RefString().length;
        // 行内升部降部最大信息
        line.ar_height_max = std::max(cell->metrics.ar_height, line.ar_height_max);
        line.dr_height_max = std::max(cell->metrics.dr_height, line.dr_height_max);
        // 换行
        if (new_line) {
            line.char_len_this = char_length_vl;
            if (!impl::push_data(vlv, line, doc.platform)) return;
            line.char_len_before += char_length_vl;
            char_length_vl = 0;
            line.lineno += cell->RefMetaInfo().eol;
            if (cell->RefMetaInfo().eol) line.char_len_before = 0;
            line.first = impl::next_cell(cell);
            // 行偏移 = 上一行偏移 + 上一行最大升高 + 上一行最大降高
            line.offset += line.ar_height_max + line.dr_height_max;
            line.ar_height_max = 0;
            line.dr_height_max = 0;
            offset_inline = 0;
            // 中断检测
            if (this_eol) {
                // 超过视口 或者完成指定行
                if (line.offset >= bottom || line.lineno > target_line) break;
            }
        }
        // 推进
        cell = impl::next_cell(cell);
    }
    // 末尾
    push_data(vlv, line, doc.platform);
    // 估计宽度
    doc.m_szEstimated.width = est_width;
    // 估计高度
    const auto llen = doc.m_vLogic.GetSize();
    const auto disl = line.lineno;
    const auto base = line.offset;
    assert(disl && "bad display-line");
    doc.m_szEstimated.height = base * make_div(llen, disl);
}


/// <summary>
/// Recreates the specified cell.
/// </summary>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::Recreate(
    CEDTextDocument&doc, CEDTextCell& cell) noexcept {
    // 被注音的
    if (cell.RefMetaInfo().metatype == Type_UnderRuby) {
        // 第一次遍历: 重建检查是否重建
        bool need_create = false;
        const auto end_cell = [&cell, &need_create]() noexcept {
            need_create = cell.RefMetaInfo().dirty;
            auto node = impl::next_cell(&cell);
            //assert(node->RefMetaInfo().metatype == Type_Ruby);
            while (node->RefMetaInfo().metatype == Type_Ruby) {
                need_create |= node->RefMetaInfo().dirty;
                const auto eol = node->RefMetaInfo().eol;
                node = impl::next_cell(node);
                if (eol) break;
            }
            return node;
        }();
        if (!need_create) return;
        // 第二次遍历: 重建并检测宽度
        unit_t width = 0;
        {
            cell.AsDirty();
            doc.recreate_context(cell);
            auto node = impl::next_cell(&cell);
            // 没有注音
            if (node == end_cell) return;
            while (node != end_cell) {
                node->AsDirty();
                doc.recreate_context(*node);
                width += node->metrics.width;
                node = impl::next_cell(node);
            }
        }
        // 第三次遍历: 重构布局
        {
            auto node = impl::next_cell(&cell);
            const unit_t allw = std::max(width, cell.metrics.width);
            const unit_t offy =  -(node->metrics.dr_height + cell.metrics.ar_height);
            const unit_t height = node->metrics.ar_height + node->metrics.dr_height;
            cell.metrics.offset.x = half(allw - cell.metrics.width);
            cell.metrics.offset.y = height;
            cell.metrics.ar_height += height;
            cell.metrics.width = allw;
            unit_t offset = -half(allw + width);
            while (node != end_cell) {
                node->metrics.offset.x = offset;
                node->metrics.offset.y = offy;
                offset += node->metrics.width;
                node->metrics.width = 0;
                node = impl::next_cell(node);
            }
        }
    }
    // 普通CELL
    else if (cell.RefMetaInfo().dirty) doc.recreate_context(cell);
}

/// <summary>
/// Merges the specified document.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="cell">The cell.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::Merge(
    CEDTextDocument & doc, CEDTextCell & cell, 
    unit_t width, unit_t offset) noexcept {
    if (!doc.m_info.wrap_mode) return true;
    if (cell.RefMetaInfo().eol) return false;
    const unit_t fs = cell.RefRichED().size;
    return offset + cell.metrics.width + fs < width;
}

/// <summary>
/// Checks the wrap.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="cell">The cell.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::Private::CheckWrap(
    CEDTextDocument& doc, CEDTextCell& cell, unit_t pos) noexcept ->CEDTextCell* {
    const auto mode = doc.m_info.wrap_mode;
    const auto str = cell.RefString().data;
    const auto len = cell.RefString().length;
    switch (mode & 3)
    {
        CellHitTest hittest; uint32_t index;
    case Mode_NoWrap:
        break;
    case Mode_SpaceOnly:
        hittest = doc.platform.HitTest(cell, pos);
        // 向前查找空格
        index = hittest.pos;
        while (index--) {
            const auto ch = str[index];
            if (ch == ' ') return cell.Split(index + 1);
        }
        // 向后查找空格
        for (index = hittest.pos; index != len; ++index) {
            const auto ch = str[index];
            if (ch == ' ') return cell.Split(index + 1);
        }
        break;
    case Mode_SpaceOrCJK:
        hittest = doc.platform.HitTest(cell, pos);
        assert(hittest.pos < cell.RefString().length);
        // 向前查找空格、CJK
        index = hittest.pos;
        do {
            const auto this_index = index;
            // 空格允许延后一个字符
            const auto ch = str[this_index];
            if (ch == ' ') return cell.Split(this_index);
            // CJK需要提前一个字符
            char32_t cjk; const auto lch = str[this_index - 1];
            if (impl::is_2nd_surrogate(lch)) {
                assert(index); --index;
                const auto pch = str[this_index - 2];
                assert(impl::is_1st_surrogate(pch));
                cjk = impl::char16x2to32(pch, lch);
            }
            else cjk = static_cast<char32_t>(lch);
            if (impl::is_cjk(cjk)) return cell.Split(this_index);
        } while (index--);
        // 向后查找空格、CJK
        for (index = hittest.pos; index != len; ++index) {
            const auto ch = str[index];
            if (ch == ' ') return cell.Split(index + 1);
            char32_t cjk;
            if (impl::is_1st_surrogate(ch)) {
                const auto nch = str[++index];
                assert(impl::is_2nd_surrogate(nch));
                cjk = impl::char16x2to32(ch, nch);
            }
            else cjk = static_cast<char32_t>(ch);
            if (impl::is_cjk(cjk)) return cell.Split(index);
        }
        break;
    case Mode_Anywhere:
        hittest = doc.platform.HitTest(cell, pos);
        // 此处中断
        return cell.Split(hittest.pos);
    }
    return nullptr;
}


/// <summary>
/// Sets the selection.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="ctx">The CTX.</param>
/// <param name="point">The point.</param>
/// <param name="mode">The mode.</param>
/// <param name="keep_anchor">if set to <c>true</c> [keep anchor].</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::SetSelection(
    CEDTextDocument& doc, HitTestCtx* ctx, DocPoint point,
    uint32_t mode, bool keep_anchor) noexcept {
    const auto prev_caret = doc.m_dpCaret;
    const auto prev_anchor = doc.m_dpAnchor;
    // 设置选择区
    switch (mode)
    {
    case impl::mode_all:
        // 全选: 锚点在最开始, 插入符在最后
        doc.m_dpAnchor = { 0, 0 };
        doc.m_dpCaret = { uint32_t(-1), uint32_t(-1) };
        break;
    case impl::mode_target:
        // 选中: 插入符更新至目标位置
        doc.m_dpCaret = point;
        break;
    case impl::mode_logicleft:
        doc.m_dpCaret = Private::LogicLeft(doc, point);
        break;
    case impl::mode_leftword:
        doc.m_dpCaret = Private::WordLeft(doc, point);
        break;
    case impl::mode_logicright:
        doc.m_dpCaret = Private::LogicRight(doc, point);
        break;
    case impl::mode_rightword:
        doc.m_dpCaret = Private::WordRight(doc, point);
        break;
    case impl::mode_home:
        // 一行的开始
        doc.m_dpCaret.line = point.line;
        doc.m_dpCaret.pos = 0;
        break;
    case impl::mode_first:
        // 全文的开始
        doc.m_dpCaret.line = 0;
        doc.m_dpCaret.pos = 0;
        break;
    case impl::mode_end:
        // 一行的结尾
        doc.m_dpCaret.line = point.line;
        doc.m_dpCaret.pos = uint32_t(-1);
        break;
    case impl::mode_last:
        // 全文的结尾
        doc.m_dpCaret = { uint32_t(-1), uint32_t(-1) };
        break;
#if 0
    case impl::mode_rightchar:
        // 往右移动指定数量位置
        doc.m_dpCaret.line += point.line;
        // 如果存在换行则是下一行的目的地址
        if (point.line) doc.m_dpCaret.pos = point.pos;
        // 否则是偏移量
        else doc.m_dpCaret.pos += point.pos;
        break;
#endif
    case impl::mode_logicup:
    case impl::mode_viewup:
    case impl::mode_logicdown:
    case impl::mode_viewdown:
        // TODO: 完成
        break;
    }
    // 范围钳制
    const auto& llv = doc.m_vLogic;
    if (!llv.GetSize()) return;
    doc.m_dpCaret.line = std::min(doc.m_dpCaret.line, llv.GetSize() - 1);
    doc.m_dpCaret.pos = std::min(doc.m_dpCaret.pos, llv[doc.m_dpCaret.line].length);
    // 不保留锚点
    if (!keep_anchor) doc.m_dpAnchor = doc.m_dpCaret;
    // 更新
    if (Cmp(prev_caret) != Cmp(doc.m_dpCaret) || 
        Cmp(prev_anchor) != Cmp(doc.m_dpAnchor)) {
        Private::RefreshCaret(doc, doc.m_dpCaret, ctx);
#ifndef NDEBUG
        char buf[64];
        std::snprintf(
            buf, sizeof(buf), "C[%d, %d] A[%d, %d]", 
            doc.m_dpCaret.line, doc.m_dpCaret.pos,
            doc.m_dpAnchor.line, doc.m_dpAnchor.pos
        );
        doc.platform.DebugOutput(buf, false);
#endif // !NDEBUG
    }
}


/// <summary>
/// Inserts the text.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <param name="view">The view.</param>
/// <param name="linedata">The linedata.</param>
/// <param name="behind">if set to <c>true</c> [behind].</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::Insert(
    CEDTextDocument& doc, DocPoint dp,
    U16View view, LogicLine linedata, 
    bool behind) noexcept {
    //if (doc.m_vLogic.IsFailed()) return false;
    // 成功时候
    const auto on_success = [&doc]() noexcept {
        // 文本修改
        Private::ValueChanged(doc, Changed_Text);
    };
    // 添加总长度
    const auto add_total = [&doc](ptrdiff_t l) noexcept {
        const uint32_t u32l = static_cast<uint32_t>(l);
        doc.m_info.total_length += u32l; return u32l;
    };
    // 需要重绘
    Private::NeedRedraw(doc);
    // 断言检测
    assert(GetLineTextLength(linedata.first) == linedata.length);
    assert(dp.pos <= linedata.length);
    assert(dp.line < doc.m_vLogic.GetSize());
    // TODO: 强异常保证
    // TODO: [优化] 最小化脏数组重建

    // 第二次遍历, 最小化m_vLogic重建
    auto pos = dp.pos;
    auto cell = linedata.first;
    // 遍历到合适的位置
    impl::find_cell1_txtoff_ex(cell, pos);
    // 这之后的为脏
    Private::Dirty(doc, *cell, dp.line);

    CellType insert_type = Type_Normal;

    // 检测是否能够插入


    // 1. 插入双字UTF16中间
    if (pos < cell->RefString().length) {
        if (impl::is_2nd_surrogate(cell->RefString().data[pos])) return false;
    }
    // 插在后面
    else {
        // BEHIND模式[仅EOL不算数]
        if (behind && !cell->RefMetaInfo().eol) {
            // 插入最后面
            if (cell->next == &doc.m_tail) {
                const auto obj = RichED::CreateNormalCell(doc, doc.default_riched);
                if (!obj) return false;
                RichED::InsertAfterFirst(*cell, *obj);
            }
            cell = impl::next_cell(cell);
            pos = 0; 
        }
        // 插入被注音后面算作注音
        switch (cell->RefMetaInfo().metatype)
        {
        case Type_Ruby:
        case Type_UnderRuby:
            insert_type = Type_Ruby;
        }
    }


    // 设置插入文字的格式. 注: 不要引用, 防止引用失效
    // 1. 通常: CELL自带格式
    // 2. 其他: 默认格式?
    const auto riched = *([=, &doc]() noexcept {
        return &cell->RefRichED();
        //return &doc.default_riched;
    }()); 



    // 第一次遍历, 为m_vLogic创建空间
    const auto lf_count = impl::lfcount(view).line;
    if (lf_count) {
        const size_t moved = sizeof(LogicLine) * (doc.m_vLogic.GetSize() - dp.line - 1);
        const auto ns = doc.m_vLogic.GetSize() + lf_count;
        if (!doc.m_vLogic.Resize(ns, doc.platform)) return false;
        const auto ptr = doc.m_vLogic.GetData();
        const auto base = ptr + dp.line;
        // [insert+1, prev_end) MOVETO [insert+1+lfcount, END)
        std::memmove(base + lf_count + 1, base + 1, moved);
        for (uint32_t i = 0; i != lf_count; ++i)
            base[i] = { linedata.first, 0 };
        // 初始化行信息
        const uint32_t left = linedata.length - dp.pos;
        base[0].length = dp.pos;
        base[lf_count].length = left;
    }




    // CELL是内联对象的情况
    //if (cell->RefMetaInfo().metatype >= Type_InlineObject) {
    //    assert(cell->RefString().length == 1);
    //    assert(!"NOT IMPL");
    //}


    // 优化: 足够塞进去的话
    auto line_ptr = &doc.m_vLogic[dp.line];
    if (!lf_count) {
        // TODO: 插入双字UTF-16?
        const auto len = static_cast<int32_t>(view.second - view.first);
        if (len <= cell->RefString().Left()) {
            cell->InsertText(pos, view);
            line_ptr->length += add_total(len);
            Private::Dirty(doc, *cell, dp.line);
            on_success();
            return true;
        }
    }




    // 1. 插入空的CELL的内部, 一定是行首
    // 2. 插入非空CELL的前面, 一定是行首
    // 3. 插入非空CELL的中间
    // 4. 插入非空CELL的后面


    // 插入前面: 创建新CELL[A], 插入点为B, 需要替换行首CELL信息
    // 插入中间: 分裂为两个[A, B]
    // 插入后面: 视为分裂

    // 插入字符串 分解成三部分:
    // 1. 字符串(前面)够能插入A的部分
    // 2. 字符串(后面)能够插入B的部分(可能与1部分重叠, 需要消除重叠)
    // 3. 中间剩余的部分(可能不存在)

    CEDTextCell* cell_a, *cell_b;
    Node** pointer_to_the_first_at_line = &line_ptr->first->prev->next;

    // 前面
    if (pos == 0) {
        cell_a = RichED::CreateNormalCell(doc, riched);
        if (!cell_a) return false;
        cell_b = cell;
        const auto prev_cell = static_cast<CEDTextCell*>(cell_b->prev);
        RichED::InsertAfterFirst(*prev_cell, *cell_a);
    }
    // 细胞分裂
    else {
        cell_b = cell->SplitEx(pos);
        if (!cell_b) return false;
        const_cast<CellMeta&>(cell_b->RefMetaInfo()).metatype = insert_type;
        cell_a = cell;
    }
    //cells = { cell_a, cell_b };
    // 对其进行插入
    const auto view1 = impl::nice_view1(view, cell_a->RefString().Left());
    const auto view2 = impl::nice_view2(view, cell_b->RefString().Left());

    line_ptr[0].first = static_cast<CEDTextCell*>(*pointer_to_the_first_at_line);;
    line_ptr[0].length += add_total(view1.second - view1.first);
    line_ptr[lf_count].length += add_total(view2.second - view2.first);
    const auto old_line_ptr = line_ptr;
    cell_a->InsertText(pos, view1);
    cell_b->InsertText(0, view2);
    cell = cell_a;

    // 第一行
    //bool first_line_not_new = false;

    // 插入中间字符
    if (view.first != view.second) {
        while (true) {
            // 获取新的一行字符数据
            auto line_view = impl::lfview(view);
            // 有效字符串 --- XA
            if (line_view.first != line_view.second || line_ptr != old_line_ptr) do {
                // 将有效字符串拆分成最大长度的字符串块
                auto this_end = line_view.first + TEXT_CELL_STR_MAXLEN;
                // 越界
                if (this_end > line_view.second) this_end = line_view.second;
                // 双字检查
                if (impl::is_1st_surrogate(this_end[-1])) ++this_end;
                // 创建CELL
                const auto obj = RichED::CreateNormalCell(doc, riched);
                // TODO: 强异常保证
                if (!obj) return false;
                // 插入数据
                const_cast<CellMeta&>(obj->RefMetaInfo()).metatype = insert_type;
                line_ptr->length += add_total(this_end - line_view.first);
                obj->InsertText(0, { line_view.first, this_end });
                RichED::InsertAfterFirst(*cell, *obj);
                cell = obj;
                line_view.first = this_end;
            } while (line_view.first < line_view.second);
            // 行数据
            line_ptr->first = static_cast<CEDTextCell*>(*pointer_to_the_first_at_line);
            pointer_to_the_first_at_line = &cell->next;
            ++line_ptr;
            if (view.first == view.second) break;
            cell->AsEOL();
        }
        // 最后一个换行
        if (view.second[-1] == '\n') {
            line_ptr->first = static_cast<CEDTextCell*>(*pointer_to_the_first_at_line);
            cell->AsEOL();
        }
    }
    //doc.m_vLogic;
    on_success();
    return true;
}


/// <summary>
/// Inserts the specified document.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <param name="obj">The object.</param>
/// <param name="line_data">The line data.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::Insert(
    CEDTextDocument& doc, DocPoint dp, CEDTextCell& obj, 
    LogicLine& line_data) noexcept {
    // 成功时候
    const auto on_success = [&doc]() noexcept {
        // 文本修改
        Private::ValueChanged(doc, Changed_Text);
    };
    // 需要重绘
    Private::NeedRedraw(doc);
    assert(obj.RefMetaInfo().eol == false && "cannot insert EOL");
    auto pos = dp.pos;
    auto cell = line_data.first;
    // 遍历到合适的位置
    while (pos > cell->RefString().length) {
        pos -= cell->RefString().length;
        cell = impl::next_cell(cell);
    }
    // 遍历到合适的位置
    impl::find_cell1_txtoff_ex(cell, pos);
    // 必须是正常的
    assert(pos == 0 || pos == cell->RefString().length || cell->RefMetaInfo().metatype == Type_Normal);

    // 插入前面 更新行首
    // 插入后面 添加
    // 插入中间 分裂


    auto insert_after_this = cell;
    // 这之后的为脏
    Private::Dirty(doc, *cell, dp.line);
    auto const next_is_first = line_data.first->prev;

    if (pos == 0) insert_after_this = static_cast<CEDTextCell*>(cell->prev);
    else if (pos < cell->RefString().length) if (!cell->Split(pos)) return false;
    RichED::InsertAfterFirst(*insert_after_this, obj);

    line_data.first = impl::next_cell(next_is_first);
    // 添加总长度
    const auto add_total = [&doc](uint32_t l) noexcept {
        doc.m_info.total_length += l; return l;
    };
    // 添加长度
    line_data.length += add_total(obj.RefString().length);
    on_success();
    return true;
}


/// <summary>
/// Removes the ruby.
/// 记录并删除范围内的特殊内联对象, 会修改ctx参数以适应后面再修改
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RecordObjs(
    CEDTextDocument & doc, DocPoint begin, const CheckRangeCtx& ctx) noexcept {
    // 修改begin
    const auto change_begin = [&]() noexcept {
        const auto cell = ctx.begin.cell;
        if (cell->RefMetaInfo().eol) begin.line++, begin.pos = 0;
        else begin.pos += cell->RefString().length - ctx.begin.offset;
        return impl::next_cell(cell);
    };
    // 对象必须为起点为0, 终点为END
    const auto start_cell = ctx.begin.offset ? change_begin() : ctx.begin.cell;
    const auto end_cell = ctx.end.offset == ctx.end.cell->RefString().length
        ? impl::next_cell(ctx.end.cell) : ctx.end.cell
        ;
    if (start_cell == end_cell->next) return;
    if (start_cell == end_cell) return;

    const auto real_begin = begin;
    const auto cfor = impl::cfor_cells(start_cell, end_cell);

    // 遍历用模板函数
    const auto for_it = [cfor](auto call, DocPoint dp) noexcept {
        for (auto& cell : cfor) {

            if (cell.RefMetaInfo().metatype >= Type_InlineObject) 
                call(&cell, dp);

            if (cell.RefMetaInfo().eol) dp.line++, dp.pos = 0;
            else dp.pos += cell.RefString().length;
        }
    };

    // 第一次遍历, 计算所需缓冲区长度
    uint32_t objs_count = 0, extra_len = 0;
    for_it([&](CEDTextCell* object, DocPoint) noexcept {
        ++objs_count;
        // 其他内联
       if (object->RefMetaInfo().metatype != Type_UnderRuby) {
           // 默认以8字节对齐
           constexpr uint32_t aligned_size = alignof(TrivialUndoRedo);
           constexpr uint32_t aligned_mask = ~(aligned_size - 1);
           const auto length = uint32_t(object->RefString().data[1]);
           assert(length > 0);
           const uint32_t aligned_len = (length + (aligned_size - 1)) & aligned_mask;
           extra_len += aligned_len;
       }
    }, real_begin);
    // 没有
    if (!objs_count) return;
    // 申请数据
    const auto data = doc.Alloc(impl::objs_undoredo_len(objs_count, extra_len));
    if (!data) return Private::AllocUndoFailed(doc);
    impl::objs_undoredo_mk(data, objs_count, extra_len);
    auto obj = impl::objs_as_remove(data, doc.m_uUndoOp++);
    // 第二次遍历, 生成OP数据
    for_it([obj](CEDTextCell* object, DocPoint point) mutable noexcept {
        uint32_t ruby = 0;
        const auto type = object->RefMetaInfo().metatype;
        uint16_t extra_len = 0;
        const auto info = object->GetExtraInfo();
        if (type == Type_UnderRuby) {
            // 计算有效注音长度
            auto cell = object;
            while (!cell->RefMetaInfo().eol) {
                cell = impl::next_cell(cell);
                if (cell->RefMetaInfo().metatype != Type_Ruby) break;
                ruby += cell->RefString().length;
            }
        }
        // 生成数据
        else extra_len = object->RefString().data[1];
        obj = impl::objs_as_goon(obj, point, ruby, type, extra_len, info);
    }, real_begin);
    // 添加
    const auto op = reinterpret_cast<TrivialUndoRedo*>(data);
    doc.m_undo.AddOp(doc, *op);
}


/// <summary>
/// Riches the range.
/// 针对富属性再对范围就行处理
/// </summary>
/// <param name="in">The in.</param>
/// <param name="out">The out.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::RichRange(
    const CheckRangeCtx& in, CellPoint out[2]) noexcept {
    // 原则: 不对EOL进行富属性修改
    // XXX: 针对多行仅仅EOL的修改如果处理?
    assert(in.begin.cell != in.end.cell || in.begin.offset != in.end.offset);
    out[0] = in.begin;
    out[1] = in.end;
    // 起点是cell结束则换到下一个cell
    if (out[0].offset == out[0].cell->RefString().length) {
        out[0].cell = impl::next_cell(out[0].cell);
        out[0].offset = 0;
        //  一致就算了: [特殊]不对EOL单独富属性就行记录
        if (out[0].cell == out[1].cell && out[0].offset == out[1].offset) return false;
    }
    // 终点是cell开始则换到上一个cell结束
    if (!out[1].offset) {
        out[1].cell = impl::prev_cell(out[1].cell);
        out[1].offset = out[1].cell->RefString().length;
        //  一致就算了: [特殊]不对EOL单独富属性就行记录
        if (out[0].cell == out[1].cell && out[0].offset == out[1].offset) return false;
    }
    return true;
}

/// <summary>
/// Records the rich.
/// 记录范围内的富属性
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="ctx">The CTX.</param>
/// <param name="richex">The richex.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RecordRich(
    CEDTextDocument & doc, DocPoint begin, 
    const CheckRangeCtx& ctx, const RichExCtx* richex) noexcept {
    assert(ctx.begin.cell != ctx.end.cell || ctx.begin.offset != ctx.end.offset);
    // 针对富属性的范围检测
    CellPoint cp[2];
    if (!Private::RichRange(ctx, cp)) return;
    // 解包
    const auto cell1 = cp[0].cell;
    const auto pos1 = cp[0].offset;
    const auto cell2 = cp[1].cell;
    const auto pos2 = cp[1].offset;
    // 删除是中间段可以无视, 插入会正确设置富属性?
    //if (cell1 == cell2 && pos1 && pos2 < cell1->RefString().length)
    //    return;
    // 计算不同的富属性多少个
    uint32_t count = 1;
    auto riched = &cell1->RefRichED();
    const auto cfor = impl::cfor_cells(cell1, cell2);
    for (auto& cell : cfor) {
        auto& real_cell = *impl::next_cell(&cell);
        if (real_cell.RefString().length && real_cell.RefRichED() != *riched) {
            riched = &real_cell.RefRichED();
            ++count;
        }
    }
    // 申请数据
    const auto data = doc.Alloc(impl::rich_undoredo_len(count));
    if (!data) return Private::AllocUndoFailed(doc);
    impl::rich_undoredo_mk(data, count);
    // 删除富属
    impl::rich_as_remove(data, doc.m_uUndoOp++);
    // 针对富属性修改的
    if (richex) {
        const uint16_t o = static_cast<uint16_t>(richex->offset);
        const uint16_t l = static_cast<uint16_t>(richex->length);
        impl::rich_init(data, richex->relayout, o, l, richex->op);
    }
    // 换行
    if (ctx.begin.cell != cell1) {
        if (ctx.begin.cell->RefMetaInfo().eol) {
            begin.line++;
            begin.pos = 0;
        }
    }
    // 遍历
    auto end = begin; uint32_t index = 0;
    auto start = pos1;
    riched = &cell1->RefRichED();
    for (auto& cell : cfor) {
        // 换行
        end.pos += cell.RefString().length - start;
        start = 0;
        if (cell.RefMetaInfo().eol) { end.line++; end.pos = 0; }

        auto& real_cell = *impl::next_cell(&cell);
        // 新的格式
        if (real_cell.RefRichED() != *riched) {
            impl::rich_set(data, index, *riched, begin, end);
            begin = end;
            riched = &real_cell.RefRichED();
            ++index;
        }
    }
    // 最后一个
    end.pos += pos2 - start;
    assert(index < count);
    impl::rich_set(data, index, *riched, begin, end);
    // 添加
    const auto op = reinterpret_cast<TrivialUndoRedo*>(data);
    doc.m_undo.AddOp(doc, *op);
}


/// <summary>
/// Records the text.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RecordText(
    CEDTextDocument& doc, DocPoint begin, DocPoint end) noexcept {
    // 获取文本长度
    uint32_t length = 0;
    const auto ac = [&](U16View view) noexcept { 
        length += view.second - view.first;
    };
    const auto lfc = [&]() noexcept { ++length; };
    Private::GenText(doc, begin, end, ac, lfc);
    assert(length);
    // 保险起见
    if (!length) return;
    // 申请数据
    const auto data = doc.Alloc(impl::text_undoredo_len(length));
    if (!data) return Private::AllocUndoFailed(doc);
    impl::text_undoredo_mk(data, length);
    // 删除文本
    impl::text_as_remove(data, doc.m_uUndoOp++, begin, end);
    uint32_t index = 0;
    const auto append = [=,&index](U16View view) noexcept {
        impl::text_append(data, index, view);
        index += view.second - view.first;
    };
    const auto linefeed = [=,&index]() noexcept {
        impl::text_append(data, index, '\n');
        ++index;
    };
    Private::GenText(doc, begin, end, append, linefeed);
    // 添加
    const auto op = reinterpret_cast<TrivialUndoRedo*>(data);
    doc.m_undo.AddOp(doc, *op);
}

/// <summary>
/// Allocs the undo failed.
/// </summary>
/// <param name="doc">The document.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::AllocUndoFailed(CEDTextDocument & doc) noexcept {
    // TODO: OOM处理, 撤销出现OOM应该释放全部撤销信息

}

/// <summary>
/// Records the ruby ex.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="riched">The riched.</param>
/// <param name="ch">The ch.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RecordRubyEx(
    CEDTextDocument & doc, DocPoint begin, const RichData & riched, char32_t ch, U16View view) noexcept {
    // 申请数据
    const auto len = static_cast<uint32_t>(view.second - view.first);
    const auto data = doc.Alloc(impl::ruby_undoredo_len(len));
    if (!data) return Private::AllocUndoFailed(doc);
    impl::ruby_undoredo_mk(data, len);
    impl::ruby_as_insert(data, doc.m_uUndoOp++);
    impl::ruby_set_data(data, begin, ch, view, riched);
    const auto op = reinterpret_cast<TrivialUndoRedo*>(data);
    doc.m_undo.AddOp(doc, *op);
}

/// <summary>
/// Recrods the objs ex.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="cell">The cell.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RecrodObjsEx(
    CEDTextDocument & doc, DocPoint begin, CEDTextCell & cell) noexcept {
    // 内联对象
    assert(cell.RefMetaInfo().metatype > Type_UnderRuby);
    constexpr uint32_t aligned_size = alignof(TrivialUndoRedo);
    constexpr uint32_t aligned_mask = ~(aligned_size - 1);
    const uint16_t extra_o = cell.RefString().data[1];
    const uint32_t extra = extra_o;
    const uint32_t extra_aligned = (extra + (aligned_size - 1)) & aligned_mask;
    // 申请数据
    const auto data = doc.Alloc(impl::objs_undoredo_len(1, extra_aligned));
    if (!data) return Private::AllocUndoFailed(doc);
    impl::objs_undoredo_mk(data, 1, extra_aligned);

    const auto obj = impl::objs_as_insert(data, doc.m_uUndoOp++);
    impl::objs_as_goon(obj, begin, 0, cell.RefMetaInfo().metatype, extra_o, cell.GetExtraInfo());
    const auto op = reinterpret_cast<TrivialUndoRedo*>(data);
    doc.m_undo.AddOp(doc, *op);
}

/// <summary>
/// Records the text ex.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RecordTextEx(
    CEDTextDocument & doc, DocPoint begin, DocPoint end, 
    U16View view) noexcept {
    // 获取文本长度
    const uint32_t length = view.second - view.first;
    assert(length);
    // 申请数据
    const auto data = doc.Alloc(impl::text_undoredo_len(length));
    if (!data) return Private::AllocUndoFailed(doc);
    impl::text_undoredo_mk(data, length);

    impl::text_as_insert(data, doc.m_uUndoOp++, begin, end);
    impl::text_append(data, 0, view);
    // 添加
    const auto op = reinterpret_cast<TrivialUndoRedo*>(data);
    doc.m_undo.AddOp(doc, *op);
}

/// <summary>
/// Removes the text.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::RemoveText(
    CEDTextDocument& doc, DocPoint begin, DocPoint end,
    const CheckRangeCtx& ctx) noexcept {
    // 成功时候
    const auto on_success = [&doc]() noexcept {
        // 文本修改
        Private::ValueChanged(doc, Changed_Text);
    };
    // 减去总长度
    const auto sub_total = [&doc](uint32_t len) noexcept {
        assert(len <= doc.m_info.total_length);
        doc.m_info.total_length -= len;
        return len;
    };
    // 解包
    const auto cell1 = ctx.begin.cell;
    const auto pos1 = ctx.begin.offset;
    const auto cell2 = ctx.end.cell;
    const auto pos2 = ctx.end.offset;
    auto& line_data1 = *ctx.line1;
    const auto line_data2 = ctx.line2;
    const auto next_is_first_to_line_1 = line_data1.first->prev;
    // 需要重绘
    Private::NeedRedraw(doc);
    // 标记为脏
    Private::Dirty(doc, *cell1, begin.line);
    // 处理
    const auto cell2_next = impl::next_cell(cell2);
    bool delete_eol = false;
    // 删除地点在同一个CELL内部
    if (cell1 == cell2) {
        const auto prev_cell1 = cell1->prev;
        delete_eol = cell1->RefMetaInfo().eol;
        cell1->RemoveTextEx({ pos1, sub_total(pos2 - pos1) });
        delete_eol = delete_eol && prev_cell1->next != cell1;
    }
    else {
        // 直接释放(CELL1, CELL2)
        for (auto node = cell1->next; node != cell2; ) {
            const auto next_node = node->next;
            const auto cell_obj = static_cast<CEDTextCell*>(node);
            node = next_node;
            sub_total(cell_obj->RefString().length);
;           cell_obj->Dispose();
        }
        // 连接CELL1, CELL2
        cell1->next = cell2;
        cell2->prev = cell1;
        cell1->ClearEOL();
        // CELL1: 删除[pos1, end)
        // CELL2: 删除[0, pos2)
        cell1->RemoveTextEx({ pos1, sub_total(cell1->RefString().length - pos1) });
        // EOL检测
        delete_eol = cell2->RefMetaInfo().eol && pos2 == cell2->RefString().length;
        cell2->RemoveTextEx({ 0, sub_total(pos2) });
    }

    // EOL恢复
    if (delete_eol) {
        auto node = static_cast<CEDTextCell*>(cell2_next->prev);
        // 从头删除
        if (begin.pos == 0) {
            // 重新创建一个CELL作为
            const auto ptr = RichED::CreateNormalCell(doc, doc.default_riched);
            // TODO: 错误处理
            if (!ptr) return false;
            RichED::InsertAfterFirst(*node, *ptr);
            node = ptr;
        }
        node->AsEOL();
    }

    // 长度计算: line_data1可能与line_data2一致(大概率, 同一行修改)
    const auto old_len2 = line_data2.length;
    line_data1.length = begin.pos;
    line_data1.length += line_data2.length - end.pos;
    // 行首计算
    line_data1.first = impl::next_cell(next_is_first_to_line_1);
    // 合并逻辑行
    if (begin.line != end.line) {
        auto& llv = doc.m_vLogic;
        const auto len = llv.GetSize();
        assert(len > end.line);
        const auto ptr = llv.GetData();
        const auto bsize = sizeof(ptr[0]) * (len - end.line - 1);
        std::memmove(ptr + begin.line + 1, ptr + end.line + 1, bsize);
        llv.ReduceSize(len + begin.line - end.line);
    }
    on_success();
    return true;
}

/// <summary>
/// check if estimated-size changed
/// </summary>
/// <param name="doc"></param>
/// <param name="old"></param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::CheckEstimated(CEDTextDocument& doc) noexcept {
    const auto old = doc.m_szEstimatedCmp;
    const auto cur = doc.m_szEstimated;
    uint32_t flag = 0;
    constexpr uint32_t mask = Changed_EstimatedWidth | Changed_EstimatedHeight;
    constexpr uint32_t estw = Changed_EstimatedWidth | (Changed_EstimatedWidth << 2);
    constexpr uint32_t esth = Changed_EstimatedHeight | (Changed_EstimatedHeight << 2);
    if (old.width != cur.width) flag |= estw;
    if (old.height != cur.height) flag |= esth;
    flag >>= doc.m_matrix.read_direction & 1;
    flag &= mask;
    Private::ValueChanged(doc, flag);
}

/// <summary>
/// Dirties the specified document.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="cell">The cell.</param>
/// <param name="logic_line">The logic line.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::Dirty(CEDTextDocument& doc, CEDTextCell& cell, uint32_t logic_line) noexcept {
    auto& vlv = doc.m_vVisual;
    const auto size = vlv.GetSize();
    assert(size);
    // 大概率在编辑第一行, 直接返回
    if (size < 2) return;
    // 利用二分查找到第一个, 然后, 删掉后面的
    const auto itr = RichED::LowerVL(vlv.begin(), vlv.end(), logic_line);
    const uint32_t index = itr - vlv.begin();
    if (index < size) vlv.ReduceSize(index + 1);
}

/// <summary>
/// Checks the range.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::CheckRange(
    CEDTextDocument& doc, DocPoint begin, DocPoint& end, CheckRangeCtx& ctx) noexcept {
    const auto line = doc.m_vLogic.GetSize();
    // { line-count, 0} 允许选择末尾
    if (end.line >= line && end.line && !end.pos) {
        end.line = line - 1;
        end.pos = doc.m_vLogic[end.line].length;
    }
    // 正常
    if (end.line < line) {
        auto& line_data1 = doc.m_vLogic[begin.line];
        auto& line_data2 = doc.m_vLogic[end.line];
        // 范围钳制
        begin.pos = std::min(begin.pos, line_data1.length);
        end.pos = std::min(end.pos, line_data2.length);
        // 没得删
        if (Cmp(end) > Cmp(begin)) {
            // 断言检测
            assert(GetLineTextLength(line_data1.first) == line_data1.length);
            assert(begin.pos <= line_data1.length);
            assert(begin.line < doc.m_vLogic.GetSize());
            assert(GetLineTextLength(line_data2.first) == line_data2.length);
            assert(end.pos <= line_data2.length);
            assert(end.line < doc.m_vLogic.GetSize());
            // 遍历到需要的位置
            auto pos1 = begin.pos;
            auto cell1 = line_data1.first;
            auto pos2 = end.pos;
            auto cell2 = line_data2.first;
            impl::find_cell2_txtoff_ex(cell1, pos1);
            // TODO: [优化] cell2从cell1处搜索
            impl::find_cell1_txtoff_ex(cell2, pos2);
            assert(cell1 != cell2 || pos1 != pos2);
            // 删除无效区间
            if (impl::is_2nd_surrogate(cell1->RefString().data[pos1])) return false;
            if (pos2 < cell2->RefString().length)
                if (impl::is_2nd_surrogate(cell2->RefString().data[pos2])) return false;
            ctx.begin = { cell1, pos1 };
            ctx.end = { cell2, pos2 };
            ctx.line1 = &line_data1;
            ctx.line2 = line_data2;
            return true;
        }
    }
    return false;
}


/// <summary>
/// Hits the test.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="pos">The position.</param>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
bool RichED::CEDTextDocument::Private::HitTest(
    CEDTextDocument & doc, Point pos, HitTestCtx& ctx) noexcept {
    // 数据无效
    auto& vlv = doc.m_vVisual;
    // 最后一行是无效数据
    if (vlv.GetSize() < 2) return false;
    // TODO: 固定行高模式
    if (doc.m_info.flags & Flag_FixedLineHeight) {
        assert(!"NOT IMPL");
        return false;
    }
    // 二分查找到指定视觉行
    
    const auto cmp = [](unit_t ll, const VisualLine& vl) noexcept { return  ll < vl.offset; };
    const auto itr = std::upper_bound(vlv.begin(), vlv.end(), pos.y, cmp);
    // 太高的话算第一行
    if (itr == vlv.begin()) {
        ctx.visual_line = &itr[0];
        ctx.text_cell = impl::next_cell(&doc.m_head);
        ctx.len_before_cell = 0;
        ctx.pos_in_cell = 0;
        return true;
    }
    // 太低的话算末尾
    else if (itr == vlv.end()) {
        auto& last = itr[-1];
        ctx.visual_line = &itr[-2];
        ctx.text_cell = static_cast<CEDTextCell*>(last.first->prev);
        ctx.len_before_cell = ctx.visual_line->char_len_before;
        ctx.len_before_cell += ctx.visual_line->char_len_this;
        ctx.pos_in_cell = ctx.text_cell->RefString().length;
        ctx.len_before_cell -= ctx.pos_in_cell;
        return true;
    }
    // 正常情况下, itr指向的是下一行. 比如: [0, 20, 40]中, 输入10输出指向20
    constexpr int offset = -1;
    // 获取指定信息
    const auto& line0 = itr[offset];
    ctx.visual_line = &line0;
    const auto& line1 = itr[offset + 1];
    const auto last = static_cast<CEDTextCell*>(line1.first->prev);
    CEDTextCell* target = nullptr;
    // 过长
    if (pos.x >= last->metrics.pos + last->metrics.width) {
        ctx.text_cell = last;
        ctx.pos_in_cell = last->RefString().length;
        ctx.len_before_cell 
            = line0.char_len_before + line0.char_len_this 
            - ctx.pos_in_cell
            ;
    }
    // 遍历到指定位置
    else {
        unit_t offthis = pos.x;
        uint32_t char_offset_in_line = line0.char_len_before;
        const auto cfor = impl::cfor_cells(line0.first, last);
        auto target = last;
        for (auto& cell : cfor) {
            if (offthis < cell.metrics.width
#ifndef RED_NO_RUBY
                && cell.metrics.width) {
                // 注音的特殊处理  开始
                if (cell.RefMetaInfo().metatype == Type_UnderRuby) {
                    // 在后面一半
                    if (offthis >= half(cell.metrics.width)) {
                        char_offset_in_line += cell.RefString().length;
                        offthis -= cell.metrics.width;
                        continue;
                    }
                }
                // 注音的特殊处理  结束
#else
                ) {
#endif
                target = &cell; break;
            }
            char_offset_in_line += cell.RefString().length;
            offthis -= cell.metrics.width;
        }
        const auto ht = doc.platform.HitTest(*target, offthis);
        ctx.text_cell = target;
        ctx.len_before_cell = char_offset_in_line;
        ctx.pos_in_cell = ht.pos + ht.trailing * ht.length;
    }
    return true;
}

/// <summary>
/// Hits the test.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <param name="ctx">The CTX.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::HitTest(
    CEDTextDocument& doc, DocPoint dp, HitTestCtx& ctx) noexcept {
    const auto dp_bk = dp;
    ctx.text_cell = nullptr;
    // 扩展到指定行
    Private::ExpandVL(doc, dp.line, max_unit());
    // 二分查找到指定行
    auto& vlv = doc.m_vVisual;
    const auto size = vlv.GetSize();
    const auto bad_end = vlv.end() - 1;
    auto itr = RichED::LowerVL(vlv.begin(), vlv.end(), dp.line);
    // 最后一个
    //if (itr == bad_end && dp.pos == 0) {
    //    ctx.text_cell = static_cast<CEDTextCell*>(bad_end->first->prev);
    //    ctx.visual_line = itr - 1;
    //    ctx.len_before_cell = ctx.visual_line->char_len_before;
    //    ctx.len_before_cell += ctx.visual_line->char_len_this;
    //    ctx.pos_in_cell = ctx.text_cell->RefString().length;
    //    ctx.len_before_cell -= ctx.pos_in_cell;
    //    return;
    //}
    assert(itr < bad_end);
    // 有效行
    if (itr < bad_end) {
        // 搜索行
        ctx.len_before_cell = 0;
        while (dp.pos > itr->char_len_this) {
            dp.pos -= itr->char_len_this;
            ctx.len_before_cell += itr->char_len_this;
            ++itr;
            assert(itr < bad_end);
        }
        // 搜索cell
        auto cell = itr->first;
        auto pos = dp.pos;
        impl::find_cell1_txtoff_ex(cell, pos);
        ctx.visual_line = itr;
        ctx.len_before_cell += dp.pos - pos;
        ctx.pos_in_cell = pos;
        ctx.text_cell = cell;
    }
}

#if 0
/// <summary>
/// Checks the point.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::Private::CheckPoint(
    CEDTextDocument & doc, DocPoint dp) noexcept -> CellPoint {
    CellPoint cp = { nullptr, 0 };
    const auto line = doc.m_vLogic.GetSize();
    if (dp.line < line) {
        const auto ll = doc.m_vLogic[dp.line];
        cp.cell = ll.first;
        cp.offset = dp.pos;
        if (dp.pos > ll.length) cp.offset = ll.length;
        impl::find_cell1_txtoff_ex(cp.cell, cp.offset);
    }
    return cp;
}
#endif


/// <summary>
/// Updates the caret.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <param name="pctx">The PCTX.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RefreshCaret(
    CEDTextDocument & doc, DocPoint dp, HitTestCtx * pctx) noexcept {
    HitTestCtx ctx;
    if (!pctx) Private::HitTest(doc, dp, ctx);
    else ctx = *pctx;
    // 修改插入符位置
    if (ctx.text_cell) {
        auto& cell = *ctx.text_cell;
        const auto pos = ctx.pos_in_cell;
        const auto cm = doc.platform.GetCharMetrics(cell, pos);
        // TODO: 固定行高
        doc.m_rcCaret.x = cell.metrics.pos + cm.offset;
        doc.m_rcCaret.y = ctx.visual_line->offset;
        doc.m_rcCaret.height
            = ctx.visual_line->ar_height_max
            + ctx.visual_line->dr_height_max
            ;
        Private::ValueChanged(doc, Changed_Caret);
    }
    // TODO: 部分情况视口跟随插入符
}


/// <summary>
/// Updates the selection.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::UpdateSelection(
    CEDTextDocument& doc, DocPoint begin, DocPoint end) noexcept {
    CmpSwap(begin, end);
    // 修改检测
    const auto pb = Cmp(doc.m_dpSelBegin);
    const auto cb = Cmp(begin);
    const auto pe = Cmp(doc.m_dpSelEnd);
    const auto ce = Cmp(end);
    if (pb == cb && pe == ce) return;
    if (pb == pe && cb == ce) return;
    // 正式修改
    doc.m_dpSelBegin = begin;
    doc.m_dpSelEnd = end;
    Private::RefreshSelection(doc, begin, end);
    Private::ValueChanged(doc, Changed_Selection);
}

/// <summary>
/// Updates the selection.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void RichED::CEDTextDocument::Private::RefreshSelection(
    CEDTextDocument& doc, DocPoint begin, DocPoint end) noexcept {
    auto& vec = doc.m_vSelection;
    vec.Clear();
    if (Cmp(begin) >= Cmp(end)) return;
    HitTestCtx bctx, ectx;
    Private::HitTest(doc, begin, bctx);
    Private::HitTest(doc, end, ectx);
    // TODO: 错误处理
    assert(bctx.text_cell && ectx.text_cell);
    if (!(bctx.text_cell && ectx.text_cell)) return;
    // 选择点行末
    const uint32_t count = ectx.visual_line - bctx.visual_line + 1;
    // 内存不足: 选择区数据用来显示, 无所谓
    if (!vec.Resize(count, doc.platform)) return;
    const auto cell0 = bctx.text_cell;
    const auto line0 = bctx.visual_line;
    const auto pos0 = bctx.pos_in_cell;
    const auto cm0 = doc.platform.GetCharMetrics(*cell0, pos0);
    const auto cell1 = ectx.text_cell;
    const auto line1 = ectx.visual_line;
    const auto pos1 = ectx.pos_in_cell;
    const auto cm1 = doc.platform.GetCharMetrics(*cell1, pos1);
    auto& first = vec[0];
    auto& last = vec[count - 1];
    const auto set_height = [](Box& box, const VisualLine& vl) noexcept {
        box.top = vl.offset;
        box.bottom = box.top + vl.ar_height_max + vl.dr_height_max;
    };
    const auto set_end = [&doc](Box& box, const VisualLine& vl) noexcept {
        assert(vl.first != &doc.m_tail);
        const auto last_cell = static_cast<CEDTextCell*>(1[&vl].first->prev);
        const auto right = last_cell->metrics.pos + last_cell->metrics.width;
        box.right = right;
        // 需要选择EOL
        if (last_cell->RefMetaInfo().eol)
            box.right += half(last_cell->RefRichED().size);
    };
    const auto set_start = [&doc](Box& box, const VisualLine& vl) noexcept {
        box.left = 0;
    };
     // 1. 设置第一行末尾位置, 最后一行行首位置
    set_end(first, *line0);
    set_start(last, *line1);
     // 2. 设置第一行行首位置, 最后一行末尾位置
    first.left = cell0->metrics.pos + cm0.offset;
    last.right = cell1->metrics.pos + cm1.offset;
    set_height(last, *line1);
     // 3. 中间行设置上一行末尾(需确认EOL), 这一行行首位置
    auto box_itr = &first;
    std::for_each(line0, line1, [=](const VisualLine& vl) mutable noexcept {
        set_end(*box_itr, vl);
        set_height(*box_itr, vl);
        ++box_itr;
        set_start(*box_itr, vl);
    });
}


/// <summary>
/// Logics the lr.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <param name="right">if set to <c>true</c> [right].</param>
/// <returns></returns>
auto RichED::CEDTextDocument::Private::LogicLeft(
    CEDTextDocument& doc, DocPoint dp) noexcept -> DocPoint {
    const auto& llv = doc.m_vLogic;
    const auto s = llv.GetSize();
    DocPoint rv = { 0 };
    if (dp.line < s) {
        const auto& line = llv[dp.line];
        // 向前搜索
        if (dp.pos) {
            const auto first_cell = line.first;
            auto pos = dp.pos;
            auto cell = first_cell;
            impl::find_cell1_txtoff_ex(cell, pos);
            assert(pos && "BAD ACTION");
            // 遇到注音则移动的到被注音前面(有的话)
            rv = dp; 
            if (cell->RefMetaInfo().metatype == Type_Ruby) {
                rv.pos -= pos;
                while (cell != first_cell) {
                    cell = static_cast<CEDTextCell*>(cell->prev);
                    if (cell->RefMetaInfo().metatype != Type_Ruby) {
                        if (cell->RefMetaInfo().metatype == Type_UnderRuby)
                            rv.pos -= cell->RefString().length;
                        break;
                    }
                    rv.pos -= cell->RefString().length;
                }
            }
            // 否则检查UTF16规则, 避免移动到错误地点
            else {
                --rv.pos;
                if (impl::is_2nd_surrogate(cell->RefString().data[pos - 1])) --rv.pos;
            }
        }
        // 处于行首
        else {
            // 换到上一行末尾
            if (dp.line)  rv = { dp.line - 1 , (&line)[-1].length };
        }
    }
    return rv;
}


/// <summary>
/// Logics the right.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::Private::LogicRight(
    CEDTextDocument & doc, DocPoint dp) noexcept -> DocPoint {
    const auto& llv = doc.m_vLogic;
    const auto s = llv.GetSize();
    DocPoint rv = { 0 };
    if (dp.line < s) {
        const auto line = llv[dp.line];
        // 处于行末
        if (dp.pos >= line.length) {
            // 换到下一行行尾
            rv = dp;
            if (dp.line + 1 < s)  rv = { dp.line + 1 , 0 };
        }
        // 向后搜索
        else {
            auto cell = line.first;
            auto pos = dp.pos;
            rv = dp;
            impl::find_cell2_txtoff_ex(cell, pos);
            // 遇到被注音则移动的到注音后面(有的话)
            if (cell->RefMetaInfo().metatype == Type_UnderRuby) {
                rv.pos += cell->RefString().length - pos;
                while (!cell->RefMetaInfo().eol) {
                    cell = impl::next_cell(cell);
                    if (cell->RefMetaInfo().metatype != Type_Ruby) break;
                    rv.pos += cell->RefString().length;
                }
            }
            // 否则检查UTF16规则, 避免移动到错误地点
            else {
                assert(pos < cell->RefString().length && "BAD ACTION");
                ++rv.pos;
                if (impl::is_1st_surrogate(cell->RefString().data[pos])) ++rv.pos;
            }
        }
    }
    return rv;
}

/// <summary>
/// Words the left.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::Private::WordLeft(
    CEDTextDocument& doc, DocPoint dp) noexcept -> DocPoint{
    // TODO: 具体实现
    return Private::LogicLeft(doc, dp);
}

/// <summary>
/// Words the right.
/// </summary>
/// <param name="doc">The document.</param>
/// <param name="dp">The dp.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::Private::WordRight(
    CEDTextDocument & doc, DocPoint dp) noexcept -> DocPoint {
    // TODO: 具体实现
    return Private::LogicRight(doc, dp);
}


/// <summary>
/// Pws the helper position.
/// </summary>
/// <param name="cell">The cell.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::PWHelperPos(const CEDTextCell& cell, const uint32_t pos) noexcept -> uint32_t{
    // 密码模式
    if (m_info.flags & Flag_UsePassword) {
        // 因为密码不会很长, 使用O(n)方式遍历字符串
        auto& string = cell.RefString();
        assert(pos < string.length && "Out of Range");
        uint32_t count = 0;
        for (uint32_t i = 0; i < pos; ++i) {
            if (impl::is_1st_surrogate(string.data[i])) ++i;
            ++count;
        }
        if (m_bPassword4) count *= 2;
        return count;
    }
    return pos;
}

/// <summary>
/// Pws the length of the helper.
/// </summary>
/// <param name="cell">The cell.</param>
/// <param name="hit">The hit.</param>
/// <returns></returns>
void RichED::CEDTextDocument::PWHelperHit(const CEDTextCell& cell, CellHitTest& hit) noexcept {
    // 密码模式
    if (m_info.flags & Flag_UsePassword) {
        const uint32_t count_mode = m_bPassword4 ? hit.pos / 2 : hit.pos;
        // 因为密码不会很长, 使用O(n)方式遍历字符串
        auto& string = cell.RefString();
        uint32_t count = 0; uint32_t length = 1;
        for (uint32_t i = 0; i != count_mode; ++i) {
            length = 1;
            if (impl::is_1st_surrogate(string.data[i])) length = 2;
            count += length;
        }
        assert(count < string.length && "Out of Range");
        // 写回去
        hit.pos = count;
        hit.length = length;
    }
}


#if 0
/// <summary>
/// Pws the helper hit.
/// </summary>
/// <param name="cell">The cell.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::PWHelperLen(const CEDTextCell& cell, const uint32_t pos) noexcept -> uint32_t {
    // 密码模式
    if (m_info.flags & Flag_UsePassword) {
        const uint32_t count_mode = m_bPassword4 ? pos / 2 : pos;
        // 因为密码不会很长, 使用O(n)方式遍历字符串
        auto& string = cell.RefString();
        uint32_t count = 0;
        for (uint32_t i = 0; i != count_mode; ++i) {
            if (impl::is_1st_surrogate(string.data[i])) ++count;
            ++count;
        }
        assert(count < string.length && "Out of Range");
        return count;
    }
    return pos;
}


/// <summary>
/// Pws the helper count.
/// </summary>
/// <param name="cell">The cell.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto RichED::CEDTextDocument::PWHelperCnt(const CEDTextCell & cell, const uint32_t pos) noexcept -> uint32_t {
    // 密码模式
    if (m_info.flags & Flag_UsePassword) {
        // 因为密码不会很长, 使用O(n)方式遍历字符串
        auto& string = cell.RefString();
        uint32_t count = 0;
        for (uint32_t i = 0; i != pos; ++i) {
            if (impl::is_1st_surrogate(string.data[i])) ++count;
            ++count;
        }
        assert(count < string.length && "Out of Range");
        return count;
    }
    return pos;
}
#endif


// ----------------------------------------------------------------------------
//                               DocMatrix
// ----------------------------------------------------------------------------

namespace RichED {
    // Matrix3X2F
    struct Matrix3X2F { unit_t _11, _12, _21, _22, _31, _32; };
    /// <summary>
    /// Transforms the point.
    /// </summary>
    /// <param name="matrix">The matrix.</param>
    /// <param name="point">The point.</param>
    /// <returns></returns>
    auto TransformPoint(const Matrix3X2F& matrix, Point point) noexcept {
        return Point{
            point.x * matrix._11 + point.y * matrix._21 + matrix._31,
            point.x * matrix._12 + point.y * matrix._22 + matrix._32
        };
    }
    /// <summary>
    /// Documents to screen.
    /// </summary>
    /// <param name="point">The point.</param>
    /// <returns></returns>
    auto DocMatrix::DocToScreen(Point point) const noexcept->Point {
        static_assert(sizeof(d2s_matrix) == sizeof(Matrix3X2F), "???");
        auto& matrix = reinterpret_cast<const Matrix3X2F&>(d2s_matrix);
        return RichED::TransformPoint(matrix, point);
    }
    /// <summary>
    /// Screens to document.
    /// </summary>
    /// <param name="point">The point.</param>
    /// <returns></returns>
    auto DocMatrix::ScreenToDoc(Point point) const noexcept->Point {
        static_assert(sizeof(s2d_matrix) == sizeof(Matrix3X2F), "???");
        auto& matrix = reinterpret_cast<const Matrix3X2F&>(s2d_matrix);
        return RichED::TransformPoint(matrix, point);
    }
    // matrix init
    void InitMatrix(DocMatrix& matrix, Direction read, Direction flow) noexcept {
        matrix.read_direction = read;
        matrix.flow_direction = flow;
        // 流向必须与阅读方向垂直
        assert(((read ^ flow) & 1) == 1);
        auto& s2d_matrix = reinterpret_cast<Matrix3X2F&>(matrix.s2d_matrix);
        auto& d2s_matrix = reinterpret_cast<Matrix3X2F&>(matrix.d2s_matrix);
        constexpr auto one = unit_one(1);
        s2d_matrix = { one, 0, 0, one, 0, 0 };
        d2s_matrix = { one, 0, 0, one, 0, 0 };
        //s2d_matrix = { 0, -one, one, 0,  0, one * 800 };
        //d2s_matrix = { 0, one, -one, 0, one * 800, 0 };


        // 方向键映射逻辑方向
        matrix.left_mapper  = impl::mode_logicleft;
        matrix.up_mapper    = impl::mode_logicup;
        matrix.right_mapper = impl::mode_logicright;
        matrix.down_mapper  = impl::mode_logicdown;
    }
}

PCN_NOINLINE
/// <summary>
/// Allocs the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void* RichED::CEDTextDocument::Alloc(size_t len) noexcept {
    auto& plat = this->platform;
    for (size_t i = 0; ; ++i) {
        if (const auto ptr = RichED::Alloc(len)) return ptr;
        if (plat.OnOOM(i, len) == OOM_Ignore) break;
    }
    return nullptr;
}