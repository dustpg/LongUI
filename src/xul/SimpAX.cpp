#include "xul/SimpAX.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <array>

//#define SAX_PROFILER
#ifdef SAX_PROFILER
#include <chrono>
#endif

#if !(defined(NDEBUG) && defined(SAX_NO_ERROR_CHECK))
#define SAX_DO_ERROR_CHECK
#endif


// 分配器自定义区域

/// <summary>
/// Frees the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
inline void SimpAX::CAXStream::free(void* ptr) noexcept {
    return std::free(ptr);
}

/// <summary>
/// Mallocs the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
inline void*SimpAX::CAXStream::malloc(size_t len) noexcept {
    return std::malloc(len);
}

/// <summary>
/// Reallocs the specified PTR.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="">The .</param>
/// <returns></returns>
inline void*SimpAX::CAXStream::realloc(void* ptr, size_t len) noexcept {
    return std::realloc(ptr, len);
}

#ifdef _MSC_VER
_declspec(noinline)
#endif

/// <summary>
/// Operator==s the specified a.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool SimpAX::IsSame(const StrPair a, const StrPair b) noexcept {
    // 长度相同再说
    if (a.b - a.a == b.b - b.a) {
        // 为了排除null
        if (a.a == b.a) return true;
        if (!a.a | !b.a) return false;
        // 遍历
        const auto* __restrict itr1 = a.a, * __restrict itr2 = b.a;
        while (itr1 < a.b) { if (*itr1 != *itr2) return false; ++itr1; ++itr2; }
        // 全部一致
        return true;
    }
    return false;
}

// simpax::impl namepsace
namespace SimpAX { namespace impl {
    template<unsigned... Is> struct seq {};
    template<unsigned N, unsigned... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};
    template<unsigned... Is>
    struct gen_seq<0, Is...> : seq<Is...> {};

    using Table = std::array<const char, 128>;
    // hex-number
    static inline constexpr char hex(char t) {
        return (t >= '0' && t <= '9') ? (t - '0')
            : ((t >= 'A' && t <= 'F') ? (t - 'A' + 10)
                : ((t >= 'a' && t <= 'f') ? (t - 'a' + 10) : -1));
    }
    template<unsigned... Is>
    constexpr Table hex_gen(seq<Is...>) { return{ { hex(Is)... } }; }
    constexpr Table hex_gen() { return hex_gen(gen_seq<128>{}); }
    // hex table
    static constexpr Table HEX_TABLE = hex_gen();
    // get hex number
    static inline auto get_hex(Char ch) noexcept -> Char {
        Char num = HEX_TABLE[(ch & 127)]; assert(num != Char(-1)); return num;
    }
    // is space?
    static inline bool is_space(Char ch) noexcept {
        return (ch == Char(' ')) | (ch == Char('\t'));
    }
    // is tag start
    static inline bool is_begin_tag_open(Char ch) noexcept {
        return ch == Char('<');
    }
    // is tag end
    static inline bool is_begin_tag_close(Char ch) noexcept {
        return ch == Char('>');
    }
    // is number start
    static inline bool is_number_start(Char ch) noexcept {
        return ch == Char('#');
    }
    // is hex number start
    static inline bool is_hex_number_start(Char ch) noexcept {
        return ch == Char('x');
    }
    // is hex number start
    static inline bool is_namespace(Char ch) noexcept {
        return ch == Char(':');
    }
    // is tag close
    static inline bool is_end_tag(Char ch) noexcept {
        return ch == Char('/');
    }
    // is pi start
    static inline bool is_begin_pi(Char ch) noexcept {
        return ch == Char('?');
    }
    // is pi end
    static inline bool is_end_pi(Char ch) noexcept {
        return ch == Char('?');
    }
    // is comment start
    static inline bool is_begin_comment(Char ch) noexcept {
        return ch == Char('!');
    }
    // is comment start2
    static inline bool is_begin_comment2(const Char& ch) noexcept {
        return (ch == Char('-')) && (1[&ch] == Char('-'));
    }
    // is comment end
    static inline bool is_end_comment(Char ch) noexcept {
        return (ch == Char('-'));
    }
    // is comment end2
    static inline bool is_end_comment2(const Char& ch) noexcept {
        return (ch == Char('-')) && (1[&ch] == Char('>'));
    }
    // is equal in attr?
    static inline bool is_attr_eq(Char ch) noexcept {
        return (ch == Char('='));
    }
    // is quot attr?
    static inline bool is_quot(Char ch) noexcept {
        return (ch == Char('"')) | (ch == Char('\''));
    }
    // is escape?
    static inline bool is_escape(Char ch) noexcept {
        return ch == Char('&');
    }
    // is escape end
    static inline bool is_end_escape(Char ch) noexcept {
        return ch == Char(';');
    }
    // is sting head equal
    static auto is_str_head_equal(
        const Char* __restrict a, 
        const Char* __restrict b
    ) noexcept ->const Char* {
        while (*b) { if (*a != *b) return nullptr; ++a; ++b; }
        return a;
    }
}}

/// <summary>
/// Initializes a new instance of the <see cref="CAXStream"/> class.
/// </summary>
SimpAX::CAXStream::CAXStream() noexcept {

}

/// <summary>
/// Finalizes an instance of the <see cref="CAXStream"/> class.
/// </summary>
/// <returns></returns>
SimpAX::CAXStream::~CAXStream() noexcept {
    this->free(m_pEscapeBuffer);
    if (m_pStackBase != m_aStackBuffer) this->free(m_pStackBase);
}

/*
    名称可以含字母、数字以及其他的字符
    名称不能以数字或者标点符号开始
    名称不能以字符 “xml”（或者 XML、Xml）开始
    名称不能包含空格
*/

/// <summary>
/// Loads the specified string.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto SimpAX::CAXStream::Load(const Char* str) noexcept -> Result {
    // 状态
    m_pStackTop = m_pStackBase;
    // 状态机
    enum sm {
        // 准备解析元素, 等待标签
        sm_standby,
        // 结束解析注释, 等待-->
        sm_comment_end,
        // 解析处理指令, 等待指令
        sm_pi_begin,
        // 结束处理指令, 等待?
        sm_pi_end,
        // 解析元素名称, 等待空白/标签结束符号
        sm_elename_end,
        // 标签结束符号
        sm_ele_end,
        // 开始属性键名, 等待字符
        sm_key_begin,
        // 结束属性键名, 等待空白
        sm_key_end,
        // 解析属性等号, 等待等号
        sm_attr_eq,
        // 开始属性数值, 等待'/"引号
        sm_value_begin,
        // 结束属性数值, 等待特定引号
        sm_value_end,
        // 结束CDATA 值, 等待]]>
        sm_cdata_end,
        // 数量
        sm_count,
        // 状态
    } state = sm_standby;
    // 当前元素
    StrPair this_element;
    StrPair this_text;
    // 互斥元素
    union { StrPair this_comment; ATTRs this_attr; PIs this_pi; };
    Char    this_quot;
    bool    value_escape = false, text_escape = false;
    // 错误记录
    const auto doc = str;
    auto make_error = [&, doc](Result::Code code) noexcept ->Result {
#if defined(SAX_NO_ERROR_CHECK)
        assert(code == Result::Code_OOM && "bad xml");
#endif
        return{ code, uint32_t(str - doc) };
    };
#ifdef SAX_PROFILER
    using hclock = std::chrono::high_resolution_clock;
    using tp = std::chrono::time_point<hclock>;
    tp tps[sm_count];
#endif
#ifndef DEBUG
    this_quot = 0;
#endif
    this_text.a = str;
#ifdef SAX_PROFILER
    auto last = hclock::now();
#endif
    // 遍历
    while (auto ch = *str) {
#ifdef SAX_PROFILER
        auto now = hclock::now();
        auto count = now - last;
        last = now;
        tps[state] += count;
#endif
        switch (state)
        {
        case sm_standby:
            // 记录转义符号
            text_escape |= impl::is_escape(ch);
            // 等待标签起始
            if (impl::is_begin_tag_open(ch)) {
                this_text.b = str; ++str; ch = *str;
                // 提交TEXT
                if (this_text.a != this_text.b) {
                    // 转义字符?
                    if (text_escape) {
                        text_escape = false;
                        this->interpret_escape(this_text);
                        if (!m_pEscapeBuffer) return make_error(Result::Code_OOM);
                    }
                    this->add_text(this_text);
                }
                // 0. 结束标签
                if (impl::is_end_tag(ch)) { this_element.a = str + 1; state = sm_ele_end; }
                // 1. ?指令
                else if (impl::is_begin_pi(ch)) { state = sm_pi_begin; this_pi.target.a = str+1; }
                // 2. !注释/CDATA
                else if (impl::is_begin_comment(ch)) {
                    // !--, 等待结束-->
                    if (impl::is_begin_comment2(str[1])) { 
                        state = sm_comment_end; str += 3; this_comment.a = str; continue;
                    }
                    // ![CDATA[, 等待结束]]>
                    else if (auto cdata = impl::is_str_head_equal(str + 1, "[CDATA[")) {
                        state = sm_cdata_end; this_text.a = str = cdata; continue;
                    }
#ifdef SAX_DO_ERROR_CHECK
                    // 否则错误
                    else return make_error(Result::Code_BadComment);
#endif
                }
                // 3. 标签, TODO: 检查标签名称有效性
                else {
#ifdef SAX_DO_ERROR_CHECK
                    if (impl::is_space(ch)) return make_error(Result::Code_BadElement);
#endif
                    this_element.a = str;
                    state = sm_elename_end;
                }
            }
            break;
        case sm_comment_end:
            // 等待注释结束符
            if (impl::is_end_comment(ch)) {
                if (impl::is_end_comment2(str[1])) {
                    state = sm_standby;
                    this_comment.b = str;
                    this_text.a = str += 3;
                    this->add_comment(this_comment);
                    continue;
                }
            }
            break;
        case sm_pi_begin:
            // 关闭
            if (impl::is_end_pi(ch)) {
                this_pi.target.b = str;
                this_pi.instructions.a = str;
                [[fallthrough]];
            }
            // 空白
            else {
                if (impl::is_space(ch)) {
                    this_pi.target.b = str;
                    this_pi.instructions.a = str + 1;
                    state = sm_pi_end;
                }
                break;
            }
        case sm_pi_end:
            // 等待?>
            if (impl::is_end_pi(ch)) {
                this_pi.instructions.b = str; ++str;
                this_text.a = str + 1;
                if (impl::is_begin_tag_close(*str)) state = sm_standby;
#ifdef SAX_DO_ERROR_CHECK
                else return make_error(Result::Code_BadPI);
#endif
                this->add_processing(this_pi);
            }
            break;
        case sm_elename_end:
            // 空白 结束 关闭, 表示完成
            if (impl::is_space(ch) | impl::is_begin_tag_close(ch) | impl::is_end_tag(ch)) {
                this_element.b = str;
                this->push(this_element);
                this->begin_element(this_element);
                if (!this->is_stack_ok()) return make_error(Result::Code_OOM);
                state = sm_standby;
                // 空白符号, 等待键
                if (impl::is_space(ch)) state = sm_key_begin;
                // 结束/关闭 标签
                else {
                    // 结束标签
                    if (impl::is_end_tag(ch)) {
                        this->pop();
                        this->end_element(this_element);
#ifndef DEBUG
                        std::memset(&this_element, 0, sizeof(this_element));
#endif
                        // 跑到关闭
                        str++; 
#ifdef SAX_DO_ERROR_CHECK
                        if (!impl::is_begin_tag_close(*str)) {
                            return make_error(Result::Code_BadElement);
                        }
#endif
                    }
                    this_text.a = str + 1;
                }
            }
            break;
        case sm_ele_end:
            // 等待空白 标签关闭
            if (impl::is_space(ch) | impl::is_begin_tag_close(ch)) {
                this_element.b = str;
                // 检查匹配情况
#ifdef SAX_DO_ERROR_CHECK
                if (m_pStackTop == m_pStackBase || this->stack_top().pair != this_element) {
                    return make_error(Result::Code_Mismatched);
                }
#endif
                // 跳过空白, 直到关闭
                while (impl::is_begin_tag_close(*str)) ++str;
                this_text.a = str;
                state = sm_standby;
                this->pop();
                this->end_element(this_element);
#ifndef DEBUG
                std::memset(&this_element, 0, sizeof(this_element));
#endif
                continue;
            }
            break;
        case sm_key_begin:
            // 遇到结束
            if (impl::is_end_tag(ch)) {
                state = sm_standby;
                this->pop();
                this->end_element(this_element);
#ifndef DEBUG
                std::memset(&this_element, 0, sizeof(this_element));
#endif
                // 跑到关闭
                str++; 
#ifdef SAX_DO_ERROR_CHECK
                if (!impl::is_begin_tag_close(*str))
                    return make_error(Result::Code_BadElement);
#endif
            }
            // 遇到结束>
            else if (impl::is_begin_tag_close(ch)) {
                state = sm_standby;
                this_text.a = str+1;
            }
            // 遇到非空白, 解析
            else if (!impl::is_space(ch)) {
                this_attr.key.a = str;
                state = sm_key_end;
            }
            break;
        case sm_key_end:
            // 遇到空白
            if (impl::is_space(ch) | impl::is_attr_eq(ch)) {
                this_attr.key.b = str;
                if (impl::is_attr_eq(ch)) state = sm_value_begin;
                else state = sm_attr_eq;
            }
            break;
        case sm_attr_eq:
            state = sm_value_begin;
            break;
        case sm_value_begin:
            // 等待引号
            if (impl::is_quot(ch)) {
                // 记录当前引号类型
                this_quot = ch;
                this_attr.value.a = str + 1;
                //value_escape = false;
                state = sm_value_end;
            }
            break;
        case sm_value_end:
            assert(impl::is_quot(this_quot));
            // 记录转义符号
            value_escape |= impl::is_escape(ch);
            // 与已存在的引号匹配
            if (ch == this_quot) {
                this_attr.value.b = str;
                // 存在转义符号
                if (value_escape) {
                    value_escape = false;
                    this->interpret_escape(this_attr.value);
                    if (!m_pEscapeBuffer) return make_error(Result::Code_OOM);
                }
                state = sm_key_begin;
                this->add_attribute(this_attr);
#ifndef DEBUG
                std::memset(&this_attr, 0, sizeof(this_attr));
                this_quot = 0;
                this->free(m_pEscapeBuffer);
                m_pEscapeBuffer = nullptr;
                m_pEscapeBufferEnd = nullptr;
#endif
            }
            break;
        case sm_cdata_end:
            // 等待]]>, 提交TEXT并转向待机状态
            if (auto cdata = impl::is_str_head_equal(str, "]]>")) {
                auto tmp = this_text; tmp.b = str;
                this_text.a = str = cdata; 
                state = sm_standby;
                this->add_text(tmp);
                continue;
            }
            break;
        }
        ++str;
    }
    // 结束TEXT
    this_text.b = str; if (this_text.b != this_text.a) {
        // 转义符号
        if (text_escape) {
            text_escape = false;
            this->interpret_escape(this_text);
            if (!m_pEscapeBuffer) return make_error(Result::Code_OOM);
        }
        this->add_text(this_text);
    }
    // 错误检查
    Result re{ Result::Code::Code_OK, 0 };
    if (m_pStackTop != m_pStackBase) re.code = Result::Code_SyntaxError;
    if (state != sm_standby) re.code = Result::Code_InternalError;
#ifdef SAX_PROFILER
    double times[sm_count];
    auto time = times;
    double all = 0.0;
    for (auto t : tps) {
        std::chrono::duration<double> diff = t - tp{};
        *time = diff.count();
        all += *time;
        ++time;
    }
    for (int i = 0; i != sm_count; ++i) {
        std::printf(
            "PROFILER[%x]: (%5.2f%%) %lfms \n", 
            i, times[i] * 100. / all, times[i]
        );
    }
#endif
    return re;
}

/// <summary>
/// Adds the processing.
/// </summary>
/// <param name="attr">The attribute.</param>
/// <returns></returns>
void SimpAX::CAXStream::add_processing(const PIs& attr) noexcept {
    attr;
}

/// <summary>
/// Begins the element.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
void SimpAX::CAXStream::begin_element(const StrPair element) noexcept {
    element;
}

/// <summary>
/// Ends the element.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
void SimpAX::CAXStream::end_element(const StrPair element) noexcept {
    element;
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="attr">The attribute.</param>
/// <returns></returns>
void SimpAX::CAXStream::add_attribute(const ATTRs& attr) noexcept {
    attr;
}

/// <summary>
/// Adds the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void SimpAX::CAXStream::add_text(const StrPair text) noexcept {
    text;
}

/// <summary>
/// Adds the comment.
/// </summary>
/// <param name="comment">The comment.</param>
/// <returns></returns>
void SimpAX::CAXStream::add_comment(const StrPair comment) noexcept {
    comment;
}

/// <summary>
/// Grows up.
/// </summary>
void SimpAX::CAXStream::grow_up() {
    // Stack overflow   我就是想打这几个词而已
    // 默认长度32, 后面增倍
    const auto cap = m_pStackCap - m_pStackBase;
    const auto len = m_pStackTop - m_pStackBase;
    const auto newcap = cap ? cap * 2 : 32;
    const auto ptr = this->malloc(newcap * sizeof(StrPair));
    // 复制
    if (len) std::memcpy(ptr, m_pStackBase, len * sizeof(StrPair));
    // 替换
    if (m_pStackBase != m_aStackBuffer) this->free(m_pStackBase);
    m_pStackBase = reinterpret_cast<StackEle*>(ptr);
    m_pStackCap = m_pStackBase + newcap;
    m_pStackTop = m_pStackBase + len;
}

/// <summary>
/// Finds the 1ST namespace.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
void SimpAX::CAXStream::find_1st_namespace(StrPair& pair) noexcept {
    auto str = pair;
    while (str.a < str.b) { if (impl::is_namespace(*str.a)) break; str.a++; }
    pair.b = str.a;
}

/// <summary>
/// Tries the get instruction value.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="ins">The ins.</param>
/// <returns></returns>
bool SimpAX::CAXStream::try_get_instruction_value(
    const Char* key, StrPair& ins) noexcept {
    auto pair = ins;
    while (pair.a < pair.b) {
        // 匹配KEY
        if (auto str = impl::is_str_head_equal(pair.a, key)) {
            // 匹配等号
            while (!impl::is_attr_eq(*str)) {
                ++str; if (str == pair.b) return false;
            }
            // 匹配引号
            while (!impl::is_quot(*str)) {
                ++str; if (str == pair.b) return false;
            }
            // 记录引号
            const auto this_quot = *str; ++str; pair.a = str;
            // 匹配引号
            while (*str != this_quot) {
                ++str; if (str == pair.b) return false;
            }
            pair.b = str;
            ins = pair;
        }
        // 推进进度
        ++pair.a;
    }
    return true;
}

#ifndef NDEBUG
/// <summary>
/// Stacks the top.
/// </summary>
/// <returns></returns>
auto SimpAX::CAXStream::stack_top() noexcept ->StackEle& {
    assert(m_pStackTop > m_pStackBase && "no element in stack");
    return m_pStackTop[-1]; 
}
#endif

/// <summary>
/// Pops this instance.
/// </summary>
void SimpAX::CAXStream::pop() {
    assert(m_pStackTop != m_pStackBase);
    --m_pStackTop;
#ifndef NDEBUG
    std::memset(m_pStackTop, -1, sizeof(*m_pStackTop));
#endif
}

/// <summary>
/// Pushes the specified string.
/// </summary>
/// <param name="str">The string.</param>
bool SimpAX::CAXStream::push(StrPair str) {
    if (m_pStackTop == m_pStackCap) this->grow_up();
    if (!m_pStackBase) return true;
    m_pStackTop->pair = str; 
    m_pStackTop->user_ptr = nullptr;
    m_pStackTop->user_data = 0;
    m_pStackTop++;
    return false;
}

/// <summary>
/// Interprets the escape.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
_declspec(noinline)
void SimpAX::CAXStream::interpret_escape(StrPair& pair) noexcept {
    // 保证数据准确
    const auto length = pair.end() - pair.begin();
    auto nowlen = m_pEscapeBufferEnd - m_pEscapeBuffer;
    m_pEscapeBufferEnd = nullptr;
    // 缓存不足
    if (length > nowlen) {
#ifndef NDEBUG
        nowlen = length;
#else
        nowlen = length * 2;
#endif
        this->free(m_pEscapeBuffer);
        const auto ptr = this->malloc(sizeof(Char) * nowlen);
        m_pEscapeBuffer = reinterpret_cast<Char*>(ptr);
    }
    // 内存不足
    if (!m_pEscapeBuffer) return;
    m_pEscapeBufferEnd = m_pEscapeBuffer + nowlen;
    // 正式处理
    /*
        实体名称 字符 十进制引用 十六进制引用
        quot    "   &#34;   &#x22;
        amp     &   &#38;   &#x26;
        apos    '   &#39;   &#x27;
        lt      <   &#60;   &#x3C;
        gt      >   &#62;   &#x3E;
    */
    auto* __restrict src_itr = pair.a;
    const auto src_end = pair.b;
    auto* __restrict des_itr = m_pEscapeBuffer;
    // 转义符号
    const Char* escape = nullptr;
    // 遍历字符串
    while (src_itr != src_end) {
        const auto ch = src_itr[0];
        // 转义提示符
        if (impl::is_escape(ch)) {
            escape = src_itr;
        }
        // 转义结束符
        else if (escape && impl::is_end_escape(ch)) {
            constexpr Char A = Char('a');
            constexpr Char G = Char('g');
            constexpr Char M = Char('m');
            constexpr Char P = Char('p');
            constexpr Char O = Char('o');
            constexpr Char S = Char('s');
            constexpr Char L = Char('l');
            constexpr Char T = Char('t');
            constexpr Char Q = Char('q');
            constexpr Char U = Char('u');
            Char escape_ch = 0;
            switch (src_itr - escape)
            {
            case 3:
                if ((escape[1] == L) & (escape[2] == T)) escape_ch = Char('<');
                else if ((escape[1] == G) & (escape[2] == T)) escape_ch = Char('>');
                break;
            case 4:
                if (impl::is_number_start(escape[1])) 
                    escape_ch = impl::get_hex(escape[2]) * 10 + impl::get_hex(escape[3]);
                else if ((escape[1] == A) & (escape[2] == M) & (escape[3] == P)) 
                    escape_ch = Char('&');
                break;
            case 5:
                if (impl::is_number_start(escape[1]) && impl::is_hex_number_start(escape[2])) 
                    escape_ch = impl::get_hex(escape[3]) * 16 | impl::get_hex(escape[4]);
                else if ((escape[1] == A) & (escape[2] == P) & (escape[3] == O) & (escape[4] == S)) 
                    escape_ch = Char('\'');
                else if ((escape[1] == Q) & (escape[2] == U) & (escape[3] == O) & (escape[4] == T))
                    escape_ch = Char('"');
                break;
            }
            // 转义成功
            if (escape_ch) { 
                des_itr -= src_itr - escape;
                *des_itr = escape_ch; 
                ++des_itr;  
                ++src_itr;
                continue;
            }
            escape = nullptr;
        }
        // 复制字符串
        *des_itr = ch;
        ++des_itr;
        ++src_itr;
    }
    // 写回字符串
    pair.a = m_pEscapeBuffer;
    pair.b = des_itr;
}