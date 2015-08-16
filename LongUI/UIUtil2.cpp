#include "LongUI.h"

// 创建浮点
bool LongUI::Helper::MakeFloats(const char* sdata, float* fdata, int size) noexcept {
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


// 16进制
unsigned int __fastcall Hex2Int(char c) {
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

#define white_space(c) ((c) == ' ' || (c) == '\t')


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
            color.r = static_cast<float>(::Hex2Int(*++data)) / 15.f;
            color.g = static_cast<float>(::Hex2Int(*++data)) / 15.f;
            color.b = static_cast<float>(::Hex2Int(*++data)) / 15.f;
        }
        // #RRGGBB
        else if (data[7] == ' ' || !data[7]) {
            color.r = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
        }
        // #AARRGGBB
        else {
            color.a = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.r = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
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

// 获取插值模式
LongUINoinline auto LongUI::Helper::XMLGetD2DInterpolationMode(
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
LongUINoinline auto LongUI::Helper::XMLGetD2DExtendMode(
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
LongUINoinline auto LongUI::Helper::XMLGetBitmapRenderRule(
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

// 获取文本抗锯齿模式
LongUINoinline auto LongUI::Helper::XMLGetD2DTextAntialiasMode(
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