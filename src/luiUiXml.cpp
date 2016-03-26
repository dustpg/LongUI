// xml helper
#include <LongUI/luiUiXml.h>
#include <Platless/luiPlUtil.h>


// longui::helper name space
namespace LongUI { namespace Helper {
    // 获取XML值
    auto XMLGetValue(pugi::xml_node node, const char* att, const char* pfx) noexcept -> const char* {
        if (!node) return nullptr;
        assert(att && "bad argument");
        char buffer[LongUIStringBufferLength];
#ifdef _DEBUG
        auto lena = std::strlen(att);
        auto lenb = pfx ? std::strlen(pfx) : 0;
        auto lenc = lengthof(buffer);
        assert(lenc > lena + lenb && "buffer too small");
#endif
        // 前缀有效?
        if (pfx) {
            std::strcpy(buffer, pfx);
            std::strcat(buffer, att);
            att = buffer;
        }
        return node.attribute(att).value();
    }
    // 解析字符串数据作为枚举值
    auto GetEnumFromString(const char* value, const GetEnumProperties& prop) noexcept ->uint32_t {
        // 首个为数字?
        auto first_digital = [](const char* str) noexcept {
            // 遍历
            while (*str) {
                // 空白: 跳过
                if (white_space(*str))  ++str;
                // 数字: true
                else if (valid_digit(*str))  return true;
                // 其他: false
                else  break;
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
                if (!std::strcmp(value, prop.values_list[i])) {
                    return uint32_t(i);
                }
            }
            assert(!"bad matched");
        }
        // 匹配无效
        return prop.bad_match;
    }
    // 帮助器 GetEnumFromString
    template<typename T, typename Ary> 
    LongUIInline auto GetEnumFromStringHelper(const char* value, T bad_match, const Ary& ary) noexcept {
        // 设置
        Helper::GetEnumProperties prop;
        prop.values_list = ary;
        prop.values_length = lengthof<uint32_t>(ary);
        prop.bad_match = static_cast<uint32_t>(bad_match);
        // 调用
        return static_cast<T>(GetEnumFromString(value, prop));
    }
    // 动画类型属性值列表
    const char* const cg_listAnimationType[] = {
        "linear",
        "quadraticin",    "quadraticout",   "quadraticinout",
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
    // 复选框状态
    const char* const cg_listCheckBoxState[] = {
        "checked", "indeterminate", "unchecked"
    };
    // 获取动画类型
    LongUINoinline auto GetEnumFromString(const char* value, AnimationType bad_match) noexcept ->AnimationType {
        return GetEnumFromStringHelper(value, bad_match, cg_listAnimationType);
    }
    // 获取插值模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_INTERPOLATION_MODE bad_match) noexcept ->D2D1_INTERPOLATION_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listInterpolationMode);
    }
    // 获取扩展模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_EXTEND_MODE bad_match) noexcept ->D2D1_EXTEND_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listExtendMode);
    }
    // 获取位图渲染规则
    LongUINoinline auto GetEnumFromString(const char* value, BitmapRenderRule bad_match) noexcept ->BitmapRenderRule {
        return GetEnumFromStringHelper(value, bad_match, cg_listBitmapRenderRule);
    }
    // 获取富文本类型
    LongUINoinline auto GetEnumFromString(const char* value, RichType bad_match) noexcept ->RichType {
        return GetEnumFromStringHelper(value, bad_match, cg_listRichType);
    }
    // 获取字体风格
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_FONT_STYLE bad_match) noexcept ->DWRITE_FONT_STYLE {
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
    // 获取复选框状态
    LongUINoinline auto GetEnumFromString(const char* value, CheckBoxState bad_match) noexcept ->CheckBoxState {
        return GetEnumFromStringHelper(value, bad_match, cg_listCheckBoxState);
    }
}}