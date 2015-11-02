#include "LongUI.h"

// 双击
LongUINoinline bool LongUI::Helper::DoubleClick::Click(const D2D1_POINT_2F& pt) noexcept {
    // 懒得解释了
    auto now = ::timeGetTime();
    bool result = ((now - last) <= time) && pt.x == this->ptx && pt.y == this->pty ;
    last = result ? 0ui32 : now;
    this->ptx = pt.x;
    this->pty = pt.y;
    return result;
}

// longui::impl 命名空间
namespace LongUI { namespace impl { 
    // 申请全局字符串
    template<typename T> 
    inline auto alloc_global_string(const T* src, size_t len) noexcept {
        // 申请
        auto global = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(T)*(len + 1));
        // 有效?
        if (global) {
            auto* des = reinterpret_cast<T*>(::GlobalLock(global));
            // 申请全局内存成功
            if (des) {
                // 复制
                ::memcpy(des, src, sizeof(T)*(len));
                // null结尾
                des[len] = 0;
                // 解锁
                ::GlobalUnlock(global);
            }
#ifdef _DEBUG
            else {
                UIManager << DL_Error
                    << L" GlobalLock --> Failed"
                    << LongUI::endl;
            }
#endif
        }
#ifdef _DEBUG
        else {
            UIManager << DL_Error
                << L" GlobalAlloc --> Failed, try alloc from"
                << Formated(L"%p in %zu bytes", src, len)
                << LongUI::endl;
        }
#endif
        return global;
    }
}}

// 申请全局字符串
LongUINoinline auto LongUI::Helper::GlobalAllocString(const wchar_t* src, size_t len) noexcept ->HGLOBAL {
    return impl::alloc_global_string(src, len);
}

// 申请全局字符串
LongUINoinline auto LongUI::Helper::GlobalAllocString(const char* src, size_t len) noexcept ->HGLOBAL {
    return impl::alloc_global_string(src, len);
}

// 查找多个文件
LongUINoinline auto LongUI::Helper::FindFilesToBuffer(
    wchar_t* buf, size_t buf_len, 
    const wchar_t* folder,
    const wchar_t* name
    ) noexcept ->wchar_t* {
    // 初始化
    WIN32_FIND_DATAW fileinfo;
#ifdef _DEBUG
    std::memset(&fileinfo, 0, sizeof(fileinfo));
#endif
    HANDLE hFile = INVALID_HANDLE_VALUE;
    ::SetLastError(0);
    {
        wchar_t path[MAX_PATH * 2]; path[0] = 0;
        std::swprintf(path, lengthof(path), L"%ls\\%ls", folder, name);
        hFile = ::FindFirstFileW(path, &fileinfo);
    }
    // 有效
    if (hFile != INVALID_HANDLE_VALUE) {
        // 清空
        DWORD errorcode = ::GetLastError();
        // 遍历文件
        while (errorcode != ERROR_NO_MORE_FILES) {
            // 跳过.开头
            if (fileinfo.cFileName[0] != '.') {
                // 写入
                auto code = std::swprintf(buf, buf_len - 1, L"%ls\\%ls", folder, fileinfo.cFileName);
                // 缓存不足
                if (code < 0) break;
                // 写入
                buf += code + 1; buf[0] = 0; buf_len -= code + 1;
            }
            // 推进
            ::FindNextFileW(hFile, &fileinfo);
            // 检查
            errorcode = ::GetLastError();
        }
    }
    ::FindClose(hFile);
    return buf;
}


// 创建 CC
auto LongUI::Helper::MakeCC(const char* str, CC* OPTIONAL data) noexcept -> uint32_t {
    assert(str && "bad argument");
    uint32_t count = 0;
    // 缓存
    char temp_buffer[LongUIStringFixedLength * 2];
    // 正式解析
    const char* word_begin = nullptr;
    for (auto itr = str;; ++itr) {
        // 获取
        register char ch = *itr;
        // 段结束?
        if (ch == ',' || !ch) {
            assert(word_begin && "bad string");
            // 有效
            if (word_begin && data) {
                CC& cc = data[count - 1];
                size_t length = size_t(itr - word_begin);
                assert(length < lengthof(temp_buffer));
                ::memcpy(temp_buffer, word_begin, length);
                temp_buffer[length] = 0;
                // 数字?
                if (word_begin[0] >= '0' && word_begin[0] <= '9') {
                    assert(!cc.id && "'cc.id' had been set, maybe more than 1 consecutive-id");
                    cc.id = size_t(LongUI::AtoI(temp_buffer));
                }
                // 英文
                else {
                    assert(!cc.func && "'cc.func' had been set");
                    cc.func = UIManager.GetCreateFunc(temp_buffer);
                    assert(cc.func && "bad func address");
                }
            }
            // 清零
            word_begin = nullptr;
            // 看看
            if(ch) continue;
            else break;
        }
        // 空白
        else if (white_space(ch)) {
            continue;
        }
        // 无效字段起始?
        if (!word_begin) {
            word_begin = itr;
            // 查看
            if ((word_begin[0] >= 'A' && word_begin[0] <= 'Z') ||
                word_begin[0] >= 'a' && word_begin[0] <= 'z') {
                if (data) {
                    data[count].func = nullptr;
                    data[count].id = 0;
                }
                ++count;
            }
        }
    }
    return count;
}

// longui::impl 命名空间
namespace LongUI { namespace impl {
    template<size_t C> struct make_units_helper { };
    // 创建单元帮助器
    template<> struct make_units_helper<2> { 
        // 创建单元
        template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
        static LongUIInline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE* units, uint32_t size) noexcept {
            // 参数检查
            assert(str && units && size && "bad arguments");
            // 数据
            CHAR_TYPE buf[LongUIStringBufferLength]; auto itr = buf;
            // 遍历
            while (size) {
                register auto ch = (*str);
                // 获取到了分隔符号
                if (ch == static_cast<CHAR_TYPE>(SEPARATOR) || ch == 0) {
                    *itr = 0;
                    caster(units++, buf);
                    itr = buf; --size;
                    if (ch == 0) {
                        assert(size == 0 && "bad string given!");
                        break;
                    }
                }
                // 继续复制
                else {
                    *itr = ch;
                    ++itr;
                }
                ++str;
            }
        }
    };
    // 创建单元帮助器
    template<> struct make_units_helper<3> { 
        // 创建单元
        template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
        static LongUIInline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE* units, uint32_t size) noexcept {
            // 参数检查
            assert(str && units && size && "bad arguments");
            // 数据
            auto old = str;
            // 遍历
            while (size) {
                // 获取到了分隔符号
                register auto ch = (*str);
                if (ch == static_cast<CHAR_TYPE>(SEPARATOR) || ch == 0) {
                    caster(units++, old, str);
                    old = str + 1; --size;
                    if (ch == 0) {
                        assert(size == 0 && "bad string given!");
                        break;
                    }
                }
                ++str;
            }
        }
    };
    // 创建单元
    template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
    inline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE units[], uint32_t size) noexcept {
        using caster_type = Helper::type_helper<Lam>;
        return make_units_helper<caster_type::arity>::make_units<SEPARATOR>(caster, str, units, size);
    }
}}

// 命名空间
namespace LongUI { namespace Helper {
    // 创建浮点
    LongUINoinline bool MakeFloats(const char* str, float fary[], uint32_t size) noexcept {
        // 检查字符串
        if (!str || !*str) return false;
        impl::make_units<','>([](float* out, const char* begin, const char* end) noexcept {
            auto len = static_cast<size_t>(end - begin);
            char buf[128]; assert(len < lengthof(buf));
            std::memcpy(buf, begin, len); len[buf] = 0;
            *out = LongUI::AtoF(buf);
        }, str, fary, size);
        return true;
    }
}}

// 16进制
unsigned int __fastcall LongUI::Hex2Int(char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    else {
        return c - '0';
    }
}

// 获取颜色表示
bool LongUI::Helper::MakeColor(const char* data, D2D1_COLOR_F& color) noexcept {
    if (!data || !*data) return false;
    // 获取有效值
    while (white_space(*data)) ++data;
    // 以#开头?
    if (*data == '#') {
        color.a = 1.f;
        // #RGB
        if (white_space(data[4]) || !data[4]) {
            color.r = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.g = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.b = static_cast<float>(Hex2Int(*++data)) / 15.f;
        }
        // #RRGGBB
        else if (white_space(data[7]) || !data[7]) {
            color.r = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
        }
        // #AARRGGBB
        else {
            color.a = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.r = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
        }
        return true;
    }
    // 浮点数组
    else {
        return Helper::MakeFloats(data, reinterpret_cast<float*>(&color), 4);
    }
}


// 创建字符串
bool LongUI::Helper::MakeString(const char* data, CUIString& str) noexcept {
    if (!data || !*data) return false;
    wchar_t buffer[LongUIStringBufferLength];
    // 转码
    register auto length = LongUI::UTF8toWideChar(data, buffer);
    buffer[length] = L'\0';
    // 设置字符串
    str.Set(buffer, length);
    return true;
}

// 设置边框颜色
bool LongUI::Helper::SetBorderColor(pugi::xml_node node, D2D1_COLOR_F color[STATUS_COUNT]) noexcept {
    // 边框颜色
    color[Status_Disabled] = D2D1::ColorF(0xD9D9D9);
    color[Status_Normal] = D2D1::ColorF(0xACACAC);
    color[Status_Hover] = D2D1::ColorF(0x7EB4EA);
    color[Status_Pushed] = D2D1::ColorF(0x569DE5);
    // 检查
    if (node) {
        const char* attr[] = {
            "disabledbordercolor", "normalbordercolor",
            "hoverbordercolor",  "pushedbordercolor",
        };
        for (auto i = 0u; i < STATUS_COUNT; ++i) {
            Helper::MakeColor(node.attribute(attr[i]).value(), color[i]);
        }
    }
    return true;
}



// --------------------------------------------------------------------------------------------------------
// longui::helper name space
namespace LongUI { namespace Helper {
    // 获取XML值
    auto XMLGetValue(pugi::xml_node node, const char* att, const char* pfx) noexcept -> const char* {
        if (!node) return nullptr;
        assert(att && "bad argument");
        char buffer[LongUIStringBufferLength];
        // 前缀有效?
        if (pfx) {
            ::strcpy(buffer, pfx);
            ::strcat(buffer, att);
            att = buffer;
        }
        return node.attribute(att).value();
    }
    // 解析字符串数据作为枚举值
    auto GetEnumFromString(const char* value, const GetEnumProperties& prop) noexcept->uint32_t {
        // 首个为数字?
        auto first_digital = [](const char* str) noexcept {
            // 遍历
            while (*str) {
                // 空白: 跳过
                if (white_space(*str)) {
                    ++str;
                }
                // 数字: true
                else if (valid_digit(*str)) {
                    return true;
                }
                // 其他: false
                else {
                    break;
                }
            }
            return false;
        };
        // 有效
        if (value && *value) {
            // 数字?
            if (first_digital(value)) {
                return uint32_t(LongUI::AtoI(value));
            }
            // 遍历
            for (size_t i = 0; i < prop.values_length; ++i) {
                if (!::strcmp(value, prop.values_list[i])) {
                    return uint32_t(i);
                }
            }
            // 失败: 给予警告
            UIManager << DL_Warning
                << L"Bad matched for: "
                << value
                << LongUI::endl;
        }
        // 匹配无效
        return prop.bad_match;
    }
    // 帮助器 GetEnumFromString
    template<typename T, typename Ary> 
    LongUIInline auto GetEnumFromStringHelper(const char* value, T bad_match, const Ary& ary) noexcept {
        // 设置
        GetEnumProperties prop;
        prop.values_list = ary;
        prop.values_length = static_cast<uint32_t>(lengthof(ary));
        prop.bad_match = static_cast<uint32_t>(bad_match);
        // 调用
        return static_cast<T>(GetEnumFromString(value, prop));
    }
    // 动画类型属性值列表
    const char* const cg_listAnimationType[] = {
        "linear",
        "quadraticim",    "quadraticout",   "quadraticinout",
        "cubicin",        "cubicout",       "cubicoinout",
        "quarticin",      "quarticout",     "quarticinout",
        "quinticcin",     "quinticcout",    "quinticinout",
        "sincin",         "sincout",        "sininout",
        "circularcin",    "circularcout",   "circularinout",
        "exponentiacin",  "exponentiaout",  "exponentiainout",
        "elasticin",      "elasticout",     "elasticinout",
        "backin",         "backout",        "backinout",
        "bouncein",       "bounceout",      "bounceinout",
    };
    // 位图渲染模式 属性值列表
    const char* const cg_listBitmapRenderRule[] = {
        "scale", "button",
    };
    // 渲染模式 属性值列表
    const char* const cg_listRenderRule[] = {
        "scale", "button",
    };
    // 富文本类型 属性值列表
    const char* const cg_listRichType[] = {
        "none", "core", "xml", "custom",
    };
    // D2D 插值模式 属性值列表
    const char* const cg_listInterpolationMode[] = {
        "neighbor", "linear",       "cubic",
        "mslinear", "anisotropic",  "highcubic",
    };
    // D2D 扩展模式 属性值列表
    const char* const cg_listExtendMode[] = {
        "clamp", "wrap", "mirror",
    };
    // D2D 文本抗锯齿模式 属性值列表
    const char* const cg_listTextAntialiasMode[] = {
        "default",  "cleartype",  "grayscale", "aliased",
    };
    // DWrite 字体风格 属性值列表
    const char* const cg_listFontStyle[] = {
        "normal", "oblique", "italic",
    };
    // DWrite 字体拉伸 属性值列表
    const char* const cg_listFontStretch[] = {
        "undefined",
        "ultracondensed",  "extracondensed",  "condensed",
        "semicondensed",   "normal",          "semiexpanded",
        "expanded",        "extraexpanded",   "ultraexpanded",
    };
    // DWrite 排列方向 属性值列表
    const char* const cg_listFlowDirection[] = {
        "top2bottom",  "bottom2top",  "left2right",  "right2left",
    };
    // DWrite 阅读方向 属性值列表
    const char* const cg_listReadingDirection[] = {
        "left2right",  "right2left",  "top2bottom",  "bottom2top",
    };
    // DWrite 换行方式 属性值列表
    const char* const cg_listWordWrapping[] = {
        "wrap", "nowrap",  "break",  "word",  "character",
    };
    // DWrite 段落对齐 属性值列表
    const char* const cg_listParagraphAlignment[] = {
        "top",  "bottom",  "middle",
    };
    // DWrite 文本对齐 属性值列表
    const char* const cg_listTextAlignment[] = {
        "left",  "right",  "center",  "justify",
    };
    // 获取动画类型
    LongUINoinline auto GetEnumFromString(const char* value, AnimationType bad_match) noexcept->AnimationType {
        return GetEnumFromStringHelper(value, bad_match, cg_listAnimationType);
    }
    // 获取插值模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_INTERPOLATION_MODE bad_match) noexcept->D2D1_INTERPOLATION_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listInterpolationMode);
    }
    // 获取扩展模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_EXTEND_MODE bad_match) noexcept->D2D1_EXTEND_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listExtendMode);
    }
    // 获取位图渲染规则
    LongUINoinline auto GetEnumFromString(const char* value, BitmapRenderRule bad_match) noexcept->BitmapRenderRule {
        return GetEnumFromStringHelper(value, bad_match, cg_listBitmapRenderRule);
    }
    // 获取富文本类型
    LongUINoinline auto GetEnumFromString(const char* value, RichType bad_match) noexcept->RichType {
        return GetEnumFromStringHelper(value, bad_match, cg_listRichType);
    }
    // 获取字体风格
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_FONT_STYLE bad_match) noexcept->DWRITE_FONT_STYLE {
        return GetEnumFromStringHelper(value, bad_match, cg_listFontStyle);
    }
    // 获取字体拉伸
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_FONT_STRETCH bad_match) noexcept ->DWRITE_FONT_STRETCH {
        return GetEnumFromStringHelper(value, bad_match, cg_listFontStretch);
    }
    // 获取排列方向
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_FLOW_DIRECTION bad_match) noexcept ->DWRITE_FLOW_DIRECTION {
        return GetEnumFromStringHelper(value, bad_match, cg_listFlowDirection);
    }
    // 获取阅读方向
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_READING_DIRECTION bad_match) noexcept ->DWRITE_READING_DIRECTION {
        return GetEnumFromStringHelper(value, bad_match, cg_listReadingDirection);
    }
    // 获取换行方式
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_WORD_WRAPPING bad_match) noexcept ->DWRITE_WORD_WRAPPING {
        return GetEnumFromStringHelper(value, bad_match, cg_listWordWrapping);
    }
    // 获取段落对齐方式
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_PARAGRAPH_ALIGNMENT bad_match) noexcept ->DWRITE_PARAGRAPH_ALIGNMENT {
        return GetEnumFromStringHelper(value, bad_match, cg_listParagraphAlignment);
    }
    // 获取文本对齐方式
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_TEXT_ALIGNMENT bad_match) noexcept ->DWRITE_TEXT_ALIGNMENT {
        return GetEnumFromStringHelper(value, bad_match, cg_listTextAlignment);
    }
    // 获取文本抗锯齿模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_TEXT_ANTIALIAS_MODE bad_match) noexcept ->D2D1_TEXT_ANTIALIAS_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listTextAntialiasMode);
    }
}}

// Render Common Brush
void LongUI::FillRectWithCommonBrush(ID2D1RenderTarget* target, ID2D1Brush* brush, const D2D1_RECT_F& rect) noexcept {
    assert(target && brush && "bad arguments");
    // 保存状态
    D2D1_MATRIX_3X2_F matrix; target->GetTransform(&matrix);
    auto height = rect.bottom - rect.top;
    D2D1_RECT_F unit_rect = {
        0.f, 0.f, (rect.right - rect.left) / height , 1.f
    };
    // 新的
    target->SetTransform(
        D2D1::Matrix3x2F::Scale(height, height) *
        D2D1::Matrix3x2F::Translation(rect.left, rect.top) *
        matrix
        );
    // 填写
    target->FillRectangle(unit_rect, brush);
    // 恢复
    target->SetTransform(&matrix);
}

