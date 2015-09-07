#include "LongUI.h"

// 命名空间
namespace LongUI { namespace Helper {
    // 创建浮点
    bool MakeFloats(const char* sdata, float* fdata, int size) noexcept {
        if (!sdata || !*sdata) return false;
        // 断言
        assert(fdata && size && "bad argument");
        // 拷贝数据
        char buffer[LongUIStringBufferLength];
        ::strcpy_s(buffer, sdata);
        char* index = buffer;
        const char* to_parse = buffer;
        // 遍历检查
        bool new_float = true;
        while (size) {
            char ch = *index;
            // 分段符?
            if (ch == ',' || ch == ' ' || !ch) {
                if (new_float) {
                    *index = 0;
                    *fdata = ::LongUI::AtoF(to_parse);
                    ++fdata;
                    --size;
                    new_float = false;
                }
            }
            else if (!new_float) {
                to_parse = index;
                new_float = true;
            }
            // 退出
            if (!ch) break;
            ++index;
        }
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
        if (data[4] == ' ' || !data[4]) {
            color.r = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.g = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.b = static_cast<float>(Hex2Int(*++data)) / 15.f;
        }
        // #RRGGBB
        else if (data[7] == ' ' || !data[7]) {
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

// 获取XML值
auto LongUI::Helper::XMLGetValue(
    pugi::xml_node node, const char* attribute, const char* prefix
    ) noexcept -> const char* {
    if (!node) return nullptr;
    assert(attribute && "bad argument");
    char buffer[LongUIStringBufferLength];
    // 前缀有效?
    if (prefix) {
        ::strcpy(buffer, prefix); 
        ::strcat(buffer, attribute);
        attribute = buffer;
    }
    return node.attribute(attribute).value();
}

// 获取XML值作为枚举值
auto LongUI::Helper::XMLGetValueEnum(pugi::xml_node node, 
    const XMLGetValueEnumProperties& prop, uint32_t bad_match) noexcept->uint32_t {
    // 获取属性值
    auto value = Helper::XMLGetValue(node, prop.attribute, prop.prefix);
    // 有效
    if (value && *value) {
        auto first_digital = [](const char* str) {
            register char ch = 0;
            while ((ch = *str)) {
                if (white_space(ch)) {
                    ++str;
                }
                else if (ch >= '0' && ch <= '9') {
                    return true;
                }
                else {
                    break;
                }
            }
            return false;
        };
        // 数字?
        if (first_digital(value)) {
            return uint32_t(LongUI::AtoI(value));
        }
        // 遍历
        for (size_t i = 0; i < prop.values_length; ++i) {
            if (!::strcmp(value, prop.values[i])) {
                return uint32_t(i);
            }
        }
    }
    // 匹配无效
    return bad_match;
}

// 获取动画类型
auto LongUI::Helper::XMLGetAnimationType(
    pugi::xml_node node,
    AnimationType bad_match,
    const char* attribute ,
    const char* prefix
    ) noexcept->AnimationType {
    // 属性值列表
    static const char* type_list[] = {
        "linear",
        "quadraticim",
        "quadraticout",
        "quadraticinout",
        "cubicin",
        "cubicout",
        "cubicoinout",
        "quarticin",
        "quarticout",
        "quarticinout",
        "quinticcin",
        "quinticcout",
        "quinticinout",
        "sincin",
        "sincout",
        "sininout",
        "circularcin",
        "circularcout",
        "circularinout",
        "exponentiacin",
        "exponentiaout",
        "exponentiainout",
        "elasticin",
        "elasticout",
        "elasticinout",
        "backin",
        "backout",
        "backinout",
        "bouncein",
        "bounceout",
        "bounceinout",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = type_list;
    prop.values_length = lengthof(type_list);
    // 调用
    return static_cast<AnimationType>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取插值模式
auto LongUI::Helper::XMLGetD2DInterpolationMode(
    pugi::xml_node node, D2D1_INTERPOLATION_MODE bad_match, 
    const char* attribute, const char* prefix
    ) noexcept->D2D1_INTERPOLATION_MODE {
    // 属性值列表
    const char* mode_list[] = {
        "neighbor",
        "linear",
        "cubic",
        "mslinear",
        "anisotropic",
        "highcubic",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = mode_list;
    prop.values_length = lengthof(mode_list);
    // 调用
    return static_cast<D2D1_INTERPOLATION_MODE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取扩展模式
auto LongUI::Helper::XMLGetD2DExtendMode(
    pugi::xml_node node, D2D1_EXTEND_MODE bad_match, 
    const char* attribute, const char* prefix
    ) noexcept->D2D1_EXTEND_MODE {
    // 属性值列表
    const char* mode_list[] = {
        "clamp",
        "wrap",
        "mirror",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = mode_list;
    prop.values_length = lengthof(mode_list);
    // 调用
    return static_cast<D2D1_EXTEND_MODE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取位图渲染规则
auto LongUI::Helper::XMLGetBitmapRenderRule(
    pugi::xml_node node, BitmapRenderRule bad_match,
    const char* attribute, const char* prefix
    ) noexcept->BitmapRenderRule {
    // 属性值列表
    const char* rule_list[] = {
        "scale",
        "button",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<BitmapRenderRule>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取富文本类型
auto LongUI::Helper::XMLGetRichType(
    pugi::xml_node node, RichType bad_match,
    const char* attribute, const char* prefix
    ) noexcept->RichType {
    // 属性值列表
    const char* rule_list[] = {
        "none",
        "core",
        "xml",
        "custom",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<RichType>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取字体类型
auto LongUI::Helper::XMLGetFontStyle(
    pugi::xml_node node, DWRITE_FONT_STYLE bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_FONT_STYLE {
    // 属性值列表
    const char* rule_list[] = {
        "normal",
        "oblique",
        "italic",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_FONT_STYLE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取字体拉伸
auto LongUI::Helper::XMLGetFontStretch(
    pugi::xml_node node, DWRITE_FONT_STRETCH bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_FONT_STRETCH {
    // 属性值列表
    const char* rule_list[] = {
        "undefined",
        "ultracondensed",
        "extracondensed",
        "condensed",
        "semicondensed",
        "normal",
        "semiexpanded",
        "expanded",
        "extraexpanded",
        "ultraexpanded",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_FONT_STRETCH>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取
auto LongUI::Helper::XMLGetFlowDirection(
    pugi::xml_node node, DWRITE_FLOW_DIRECTION bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_FLOW_DIRECTION {
    // 属性值列表
    const char* rule_list[] = {
        "top2bottom",
        "bottom2top",
        "left2right",
        "right2left",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_FLOW_DIRECTION>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取阅读方向
auto LongUI::Helper::XMLGetReadingDirection(
    pugi::xml_node node, DWRITE_READING_DIRECTION bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_READING_DIRECTION {
    // 属性值列表
    const char* rule_list[] = {
        "left2right",
        "right2left",
        "top2bottom",
        "bottom2top",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_READING_DIRECTION>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取换行标志
auto LongUI::Helper::XMLGetWordWrapping(
    pugi::xml_node node, DWRITE_WORD_WRAPPING bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_WORD_WRAPPING {
    // 属性值列表
    const char* rule_list[] = {
        "wrap",
        "nowrap",
        "break",
        "word",
        "character",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_WORD_WRAPPING>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取段落对齐方式
auto LongUI::Helper::XMLGetVAlignment(
    pugi::xml_node node, DWRITE_PARAGRAPH_ALIGNMENT bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_PARAGRAPH_ALIGNMENT {
    // 属性值列表
    const char* rule_list[] = {
        "top",
        "bottom",
        "middle",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}


// 获取段落对齐方式
auto LongUI::Helper::XMLGetHAlignment(
    pugi::xml_node node, DWRITE_TEXT_ALIGNMENT bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_TEXT_ALIGNMENT {
    // 属性值列表
    const char* rule_list[] = {
        "left",
        "right",
        "center",
        "justify",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_TEXT_ALIGNMENT>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取文本抗锯齿模式
auto LongUI::Helper::XMLGetD2DTextAntialiasMode(
    pugi::xml_node node, D2D1_TEXT_ANTIALIAS_MODE bad_match
    ) noexcept->D2D1_TEXT_ANTIALIAS_MODE {
    // 属性值列表
    const char* mode_list[] = {
        "default",
        "cleartype",
        "grayscale",
        "aliased",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = LongUI::XMLAttribute::WindowTextAntiMode;
    prop.prefix = nullptr;
    prop.values = mode_list;
    prop.values_length = lengthof(mode_list);
    // 调用
    return static_cast<D2D1_TEXT_ANTIALIAS_MODE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}


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