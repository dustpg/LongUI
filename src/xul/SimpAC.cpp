#include <xul/SimpAC.h>
#include <cassert>
#ifndef LUI_DISABLE_STYLE_SUPPORT

// simpcs::impl namepsace
namespace SimpAC { namespace impl {
    // table for valid selector
    static const uint32_t valid_selector_table[] = {
        0x00000000, 0x03ff2000, 0x87fffffe, 0x07fffffe,
    };
#if 0
    // table maker
    void make_valid_selector_table() noexcept {
        auto valid = [](char ch) noexcept {
            return ch == '-' || ch == '_'
                || (ch >= '0' && ch <= '9')
                || (ch >= 'a' && ch <= 'z')
                || (ch >= 'A' && ch <= 'Z')
                ;
        };
        const int char_bit = CHAR_BIT;
        const int len = 128 / char_bit * char_bit;
        const int ary = len / char_bit / sizeof(uint32_t);
        uint32_t buffer[ary];
        std::memset(buffer, 0, sizeof buffer);
        for (int i = 0; i <= len; ++i) {
            const int index = i >> 5;
            const int offset = i & 31;
            buffer[index] |= valid(i) << offset;
        }
        if (const auto file = std::fopen("out.txt", "w")) {
            for (auto x : buffer) {
                std::fprintf(file, "0x%08x, ", x);
            }
            std::fclose(file);
            std::exit(0);
        }
    }
#endif
    // is space?
    static inline bool is_space(char ch) noexcept {
        return (ch == ' ') || (ch == '\t');
    }
    // is new line?
    static inline bool is_newline(char ch) noexcept {
        return (ch == '\r') || (ch == '\n');
    }
    // is quot attr?
    static inline bool is_quot(char ch) noexcept {
        return (ch == '"') || (ch == '\'');
    }
    // is number start
    static auto is_valid_selector(char ch) noexcept -> uint32_t {
        return valid_selector_table[ch >> 5] & uint32_t(1 << (ch & 31));
    }
    // is valid property name
    static auto is_valid_property_name(char ch) noexcept -> uint32_t {
        return valid_selector_table[ch >> 5] & uint32_t(1 << (ch & 31));
    }
    // bkdr hash
    auto bkdr(const Char* strbgn, const Char* strend) noexcept -> uint32_t {
        const uint32_t seed = 131;
        const auto* itr = strbgn;
        const auto itrend = strend;
        uint32_t hash = 0;
        while (itr != itrend) hash = hash * seed + (*itr++);
        return hash;
    }
    // parse func
    auto parse_func(StrPair pair) noexcept->FuncType;
}}

/// <summary>
/// state for combinator
/// </summary>
enum class SimpAC::CACStream::combinator_state {
    reset, has, ready, release
};

/// <summary>
/// state for css
/// </summary>
enum class SimpAC::CACStream::css_state : unsigned {
    standby = 0, adjacent, general,
    child, pseudo, id, class_,
    universal, selectors, properties,
    properties_end, values, values_quot,
    values_end, values_func, atrule,
#ifdef SAC_ATTRIBUTE_SELECTOR
    attribute,
    attribute_type, attribute_value_begin,
    attribute_value_end, attribute_end,
#endif
};


#ifndef SAC_PURE_VIRTUAL
/// <summary>
/// Adds the comment.
/// </summary>
/// <param name="comment">The comment.</param>
/// <returns></returns>
void SimpAC::CACStream::add_comment(StrPair comment) noexcept {
}

/// <summary>
/// Adds the selector.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="name">The name.</param>
/// <returns></returns>
void SimpAC::CACStream::add_selector(BasicSelectors type, StrPair name) noexcept {
}

/// <summary>
/// Adds the selector combinator.
/// </summary>
/// <param name="combinator">The combinator.</param>
/// <returns></returns>
void SimpAC::CACStream::add_selector_combinator(Combinators combinator) noexcept {
}

/// <summary>
/// Adds the selector comma.
/// </summary>
/// <returns></returns>
void SimpAC::CACStream::add_selector_comma() noexcept {
}

/// <summary>
/// Begins the properties.
/// </summary>
/// <returns></returns>
void SimpAC::CACStream::begin_properties() noexcept {
}

/// <summary>
/// Ends the properties.
/// </summary>
/// <returns></returns>
void SimpAC::CACStream::end_properties() noexcept {
}

/// <summary>
/// Begins the property.
/// </summary>
/// <param name="property">The property.</param>
/// <returns></returns>
void SimpAC::CACStream::begin_property(StrPair property) noexcept {
}

/// <summary>
/// Adds the value.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void SimpAC::CACStream::add_value(StrPair value) noexcept {
}


/// <summary>
/// Adds the value.
/// </summary>
/// <param name="func">The function.</param>
/// <param name="args">The arguments.</param>
/// <returns></returns>
void SimpAC::CACStream::add_func_value(StrPair func, StrPair args) noexcept {

}


#ifdef SAC_ATTRIBUTE_SELECTOR
/// <summary>
/// Adds the attribute selector.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="attribute">The attribute.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void SimpAC::CACStream::add_attribute_selector(
    BasicSelectors type, StrPair attribute, StrPair value) noexcept {

}
#endif

#endif

/// <summary>
/// Parses the comment.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
bool SimpAC::CACStream::parse_comment(StrPair & view) noexcept {
    // TODO: 将第一个 && 判断放到while里面减少代码量
    // /*注释
    if (view.first + 1 < view.second && view.first[1] == '*') {
        view.first += 2;
        auto comment_view = view;
        bool comment = false;
        // 遍历直到字符串结束或者遇到*/
        while (view.first < view.second) {
            if (comment) {
                if (view.first[0] == '/') break;
                comment = false;
            }
            else if (view.first[0] == '*') comment = true;
            ++view.first;
        }
        // 去掉一个*
        comment_view.second = view.first - 1;
        this->add_comment(comment_view);
        ++view.first;
        return true;
    }
    return false;
}



/// <summary>
/// Parses the selector LV1.
/// </summary>
/// <param name="ch">The ch.</param>
/// <param name="state">The state.</param>
/// <returns></returns>
auto SimpAC::CACStream::parse_selector_lv1(char ch, combinator_state& state) noexcept ->css_state {
    switch (ch)
    {
    default: return css_state::selectors;
    case '@': return css_state::atrule;
    case '.': return css_state::class_;
    case '#': return css_state::id;
    case '*': return css_state::universal;
    case ':': return css_state::pseudo;
#ifdef SAC_ATTRIBUTE_SELECTOR
    case '[': return css_state::attribute;
#elif !defined(NDEBUG)
    case '[': assert(!"unsupport"); return css_state::selectors;
#endif
    case '+': return css_state::adjacent;
    case '~': return css_state::general;
    case '>': return css_state::child;
    case '{':
        this->begin_properties();
        return css_state::properties;
    case ',':
        state = combinator_state::reset;
        this->add_selector_comma();
        return css_state::standby;
    case ' ': case '\t':
        if (state == combinator_state::has)
            state = combinator_state::ready;
        return css_state::standby;
    }
}



/// <summary>
/// Loads the specified string.
/// </summary>
/// <param name="view">The view.</param>
/// <param name="inline_style">if set to <c>true</c> [inline style].</param>
/// <returns></returns>
void SimpAC::CACStream::Load(StrPair view, bool inline_style) noexcept {
    // 只有三字节不管什么也直接返回
    if (view.second - view.first <= 3) return;
    // 移除BOM
    {
        union { uint32_t u32; char u8[4]; } bom, now;
        bom.u32 = 0, now.u32 = 0;
        bom.u8[0] = '\xef'; bom.u8[1] = '\xbb'; bom.u8[2] = '\xbf';
        now.u8[0] = view.first[0]; 
        now.u8[1] = view.first[1];
        now.u8[2] = view.first[2];
        if (now.u32 == bom.u32) view.first += 3;
    }
    // 状态
    BasicSelectors selector;
    //Combinators combinator;
    StrPair this_view, ex_view;
    Char last_quot = 0, func_quot = 0;
    auto state = inline_style ? css_state::properties : css_state::standby;
    auto show_combinator = combinator_state::reset;
    // 处理状态
    while (view.first < view.second) {
        // 优先处理注释
        if (view.first[0] == '/' && this->parse_comment(view))
            continue;
        // 当前处理字符
        const auto ch = view.first[0];
        // 状态处理
        switch (state)
        {
        case css_state::standby:
            // standby 跳过无用字符
            if (impl::is_newline(ch)) break;
            // 默认的元素选择器
            selector = BasicSelectors::Selectors_Type;
            this_view.first = view.first + 1;
            state = this->parse_selector_lv1(ch, show_combinator);
            // 不能脱掉首字符
            if (state == css_state::selectors) this_view.first--;
            break;
        case css_state::adjacent:
            // A + B
            this->add_selector_combinator(Combinators_AdjacentSibling);
            goto add_combinator;
        case css_state::general:
            // A ~ B
            this->add_selector_combinator(Combinators_GeneralSibling);
            goto add_combinator;
        case css_state::child:
            // A + B
            this->add_selector_combinator(Combinators_Child);
        add_combinator:
            state = css_state::standby;
            show_combinator = combinator_state::release;
            continue;
        case css_state::pseudo:
            // 伪元素选择器
            if (ch == ':') {
                this_view.first = view.first + 1;
                selector = BasicSelectors::Selectors_PseudoElement;
            }
            // 伪类选择器
            else selector = BasicSelectors::Selectors_PseudoClass;
            state = css_state::selectors;
            break;
        case css_state::id:
            // id选择器
            selector = BasicSelectors::Selectors_Id;
            state = css_state::selectors;
            break;
        case css_state::class_:
            // 类选择器
            selector = BasicSelectors::Selectors_Class;
            state = css_state::selectors;
            break;
        case css_state::universal:
            // 通用选择器
            selector = BasicSelectors::Selectors_Universal;
            state = css_state::selectors;
            continue;
        case css_state::selectors:
            // 检查关系选择器状态
            if (show_combinator == combinator_state::ready) {
                this->add_selector_combinator(Combinators_Descendant);
            }
            show_combinator = combinator_state::has;
            // 括号?
            if (ch == '(') {
                while (*view.first != ')') ++view.first;
                ++view.first;
                assert(view.first <= view.second && "out of string");
                // TODO: 注释支持
                goto invalid_selector;
            }
            // 无效选择器字符 -> 完成选择器
            else if (!impl::is_valid_selector(ch)) {
            invalid_selector:
                this_view.second = view.first;
                this->add_selector(selector, this_view);
                state = css_state::standby;
                continue;
            }
            break;
        case css_state::properties:
            // 结束本规则集
            if (ch == '}') goto end_of_properties;
            // 忽略单独的 ;
            else if (ch == ';');
            // 其他有效字符
            else if (impl::is_valid_property_name(ch)) {
                this_view.first = view.first;
                state = css_state::properties_end;
            }
            break;
        case css_state::properties_end:
            // 查找空格或者:
            if (ch == ':' || impl::is_space(ch)) {
                this_view.second = view.first;
                this->begin_property(this_view);
                // 直到跳过冒号
                while (*view.first != ':') ++view.first;
                assert(view.first <= view.second && "out of string");
                // TODO: 注释支持

                // 切换到值状态
                state = css_state::values;
            }
            break;
        case css_state::values:
            // 下条规则
            if (ch == ';') state = css_state::properties;
            // 直接结束
            else if (ch == '}') goto end_of_properties;
            // / 开头
            else if (ch == '/') {
                this->add_value({ view.first, view.first + 1 });
            }
            // 寻找值起始点
            else if (!(impl::is_space(ch) || impl::is_newline(ch))) {
                // 引号?
                if (impl::is_quot(ch)) {
                    last_quot = ch;
                    this_view.first = view.first + 1;
                    state = css_state::values_quot;
                }
                // 其他情况
                else {
                    this_view.first = view.first;
                    state = css_state::values_end;
                }
            }
            break;
        case css_state::values_quot:
            // 寻找值结束点
            if (ch == last_quot) {
                this_view.second = view.first;
                this->add_value(this_view);
                state = css_state::values;
            }
            break;
        case css_state::values_end:
            // 函数支持
            if (ch == '(') {
                this_view.second = view.first;
                ex_view.first = view.first + 1;
                ex_view.second = nullptr;
                state = css_state::values_func;
            }
            // 寻找值结束点
            else if (impl::is_space(ch) || impl::is_newline(ch) || ch == ';') {
                this_view.second = view.first;
                this->add_value(this_view);
                state = (ch == ';')
                    ? css_state::properties
                    : css_state::values;
            }
            // 是个/
            else if (ch == '/') {
                this_view.second = view.first;
                this->add_value(this_view);
                this->add_value({ view.first, view.first+1 });
                state = (ch == ';')
                    ? css_state::properties
                    : css_state::values;
            }
            break;
        case css_state::values_func:
            // 引号模式
            if (func_quot) {
                // 遇到相同的引号就结束
                if (ch == func_quot) func_quot = 0;
            }
            // 正常模式
            else {
                // 遇到反括号结束
                if (ch == ')') {
                    ex_view.second = view.first;
                    FuncValue fv;
                    fv.first = ex_view.first;
                    const auto length = ex_view.second - ex_view.first;
                    assert(length >= 0 && length <= 0xffff);
                    fv.length = static_cast<uint16_t>(length);
                    fv.func = impl::parse_func(this_view);
                    this->add_func_value(fv, this_view);
                    state = css_state::values;
                }
                // 遇到引号
                else if (impl::is_quot(ch)) func_quot = ch;
            }
            break;
        end_of_properties:
            this->end_properties();
            state = css_state::standby;
            show_combinator = combinator_state::reset;
            break;
        case css_state::atrule:
            break;
#ifdef SAC_ATTRIBUTE_SELECTOR
        case css_state::attribute:
            // 属性选择器, 阶段1, 寻找键
            if (!impl::is_space(ch)) {
                this_view.first = view.first;
                selector = BasicSelectors::Selectors_AttributeSet;
                state = css_state::attribute_type;
            }
        case css_state::attribute_type:
            // 属性选择器, 阶段3, 寻找属性选择器类型
        {
            switch (ch)
            {
            default:
                this_view.second = view.first + 1;
                break;
            case ' ': case '\t':
                break;
            case ']':
                // 直接结束
                goto end_of_attribute;
            case '=':
                selector = BasicSelectors::Selectors_AttributeExact;
                goto after_state;
            case '~':
                selector = BasicSelectors::Selectors_AttributeList;
                goto after_eq;
            case '|':
                selector = BasicSelectors::Selectors_AttributeHyphen;
                goto after_eq;
            case '*':
                selector = BasicSelectors::Selectors_AttributeContain;
                goto after_eq;
            case '^':
                selector = BasicSelectors::Selectors_AttributeBegin;
                goto after_eq;
            case '&':
                selector = BasicSelectors::Selectors_AttributeEnd;
            after_eq:
                assert(view.first[1] == '=');
                view.first++;
            after_state:
                state = css_state::attribute_value_begin;
                break;
            }
        }
        break;
        case css_state::attribute_value_begin:
            // 属性选择器, 阶段4, 寻找值起始
            if (!impl::is_space(ch)) {
                ex_view.first = impl::is_quot(ch) ? view.first + 1 : view.first;
                state = css_state::attribute_value_end;
            }
            break;
        case css_state::attribute_value_end:
            // 属性选择器, 阶段5, 寻找值结束
            if (impl::is_quot(ch) || impl::is_space(ch)) {
                ex_view.second = view.first;
                state = css_state::attribute_end;
            }
            break;
        case css_state::attribute_end:
            // 属性选择器, 阶段6, 寻找属性结束
            if (ch == ']') {
            end_of_attribute:
                this->add_attribute_selector(selector, this_view, ex_view);
                state = css_state::standby;
            }
            break;
#endif
        }
        // 推进
        ++view.first;
    }
    // 没有处理完的值
    if (state == css_state::values_end) {
        this_view.second = view.first;
        this->add_value(this_view);
    }
}


// simpac
namespace SimpAC {
    // hash list
    static uint32_t FUNC_HASH_LIST[] = {
        0x0d1e039d, // attr
        0x0d5da101, // calc
        0xfcbab3ce, // cubic-bezier
        0x001b76ed, // hsl
        0x0e0ddba8, // hsla
        0x06b5f7bc, // linear-gradient
        0x53770a2a, // radial-gradient
        0x48e49c72, // repeating-linear-gradient
        0x95a5aee0, // repeating-radial-gradient
        0x001e0f19, // rgb
        0x0f61ba2c, // rgba
        0x001edddf, // url
        0x001f183b, // var
    };
}

/// <summary>
/// Parses the function.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
auto SimpAC::impl::parse_func(StrPair pair) noexcept -> FuncType {
    const auto hash = impl::bkdr(pair.begin(), pair.end());
    auto itr = FUNC_HASH_LIST;
    const auto end = FUNC_HASH_LIST +
        sizeof(FUNC_HASH_LIST) / sizeof(FUNC_HASH_LIST[0]);
    for (; itr != end; ++itr) if (hash == *itr) break;
    return static_cast<FuncType>((itr - FUNC_HASH_LIST) + 1);
}
#endif



/// <summary>
/// Splits the unit.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
auto SimpAC::SplitUnit(StrPair& pair) noexcept -> StrPair {
    StrPair rv = { pair.second, pair.second };
    const auto end_itr = pair.first;
    // 逆序遍历字符串
    while (end_itr < rv.first) {
        --rv.first;
        const auto ch = *rv.first;
        // 最后一个允许.
        // 1e-1
        // 1.
        if ((ch >= '0' && ch <= '9') || ch == '.') {
            ++rv.first;
            break;
        }
    }
    pair.second = rv.first;
    return rv;
}

