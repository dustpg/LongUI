#include "interface.h"
#include <vector>
#include <regex>


static const char* ClassRegex =
u8R"regex((?://(.+?))?\s*class\s+([\w:]+?)\s*(?:\[([\w:]+?)\])?\s+:\s*([\w:]+)\s*\{([\s\S]+?)\};)regex";
//          注释         class    类名       别名         :         父类            方法
static const char* MethodRegex =
u8R"regex((?://(.+?))?(?:\s+(static))?\s+([\w\*]+?)\s+(\w+?)(?:\[(\w+?)\])*\(([\s\S]+?)\);)regex";
//          注释             static     返回类型   方法名    别名           参数列表
// 
// 

static const char* script = u8R"longui(
// LongUI Control Base Class
class LongUI::UIControl : void {
    void* SetA(void*, char*);
    void SetB[set_b](void*);
};
// LongUI Container Base Class
class         LongUI::UIContainer :     LongUI::UIControl {
    void SetC(void*);
    void SetD(void*);
};
)longui";

enum ClassRegExpIndex {
    IndexC_Unknown = 0,
    IndexC_Comment,
    IndexC_ClassName,
    IndexC_AliasName,
    IndexC_SuperName,
    IndexC_MethodZone,
};

enum MethodRegExpIndex {
    IndexM_Unknown = 0,
    IndexM_Comment,
    IndexM_IsStatic,
    IndexM_ReturnType,
    IndexM_MethodName,
    IndexM_AliasName,
    IndexM_ParamList,
};


// 解析单个参数
ISType parse_arg(const char* str);
// 解析参数列表
uint32_t parse_arglist(const char* sbegin, const char* send, ISType[/*256*/]);


// 读取字符串
bool InterfaceScriptReader::read(const char* script_) {
    if (!script) return false;
    this->~InterfaceScriptReader();
    std::vector<uint8_t> buffer;
    buffer.reserve(64 * 1024);
    // 相关数据
    uint32_t class_number = 0;
    // 正则表达式匹配
    const std::regex class_regex(ClassRegex);
    const std::regex method_regex(MethodRegex);
    const std::cregex_iterator itr_end;
    // 循环匹配
    auto script_end = script + std::strlen(script);
    for (std::cregex_iterator itr(script, script_end, class_regex); itr != itr_end; ++itr) {
        const auto& resultc = *itr;
        ++class_number;
        // 初始化
        auto now_offsetc = buffer.size();
        buffer.resize(now_offsetc + sizeof(ISClass) - sizeof(ISClass::method));
        ISClass& tmp_class = reinterpret_cast<ISClass&>(buffer[now_offsetc]);
        tmp_class.size = sizeof(ISClass) - sizeof(ISClass::method);
        tmp_class.method_number = 0;
        // 复制注释
        auto length = resultc[IndexC_Comment].second - resultc[IndexC_Comment].first;
        tmp_class.comment[length] = 0;
        std::strncpy(tmp_class.comment, resultc[IndexC_Comment].first, length);
        // 复制类名
        length = resultc[IndexC_ClassName].second - resultc[IndexC_ClassName].first;
        tmp_class.name[length] = 0;
        std::strncpy(tmp_class.name, resultc[IndexC_ClassName].first, length);
        // 复制别名
        length = resultc[IndexC_AliasName].second - resultc[IndexC_AliasName].first;
        if (length) {
            tmp_class.alias[length] = 0;
            std::strncpy(tmp_class.alias, resultc[IndexC_AliasName].first, length);
        }
        else {
            std::strcpy(tmp_class.alias, tmp_class.name);
        }
        // 复制父类名
        length = resultc[IndexC_SuperName].second - resultc[IndexC_SuperName].first;
        tmp_class.super[length] = 0;
        std::strncpy(tmp_class.super, resultc[IndexC_SuperName].first, length);
        // 检查方法
        for (std::cregex_iterator itrm(
            resultc[IndexC_MethodZone].first, resultc[IndexC_MethodZone].second, method_regex
            ); itrm != itr_end; ++itrm) {
            const auto& resultm = *itrm;
            // 添加方法数量
            ++tmp_class.method_number;
            // 检查偏移值
            auto now_offsetm = buffer.size();
            buffer.resize(now_offsetm + sizeof(ISMethod) - sizeof(ISMethod::params));
            ISMethod& tmp_method = reinterpret_cast<ISMethod&>(buffer[now_offsetm]);
            // 复制注释
            length = resultm[IndexM_Comment].second - resultm[IndexM_Comment].first;
            tmp_method.comment[length] = 0;
            std::strncpy(tmp_method.comment, resultm[IndexM_Comment].first, length);
            // 检查类型
            tmp_method.method_type =
                resultm[IndexM_IsStatic].matched ? ISMethodType::Static : ISMethodType::Normal;
            // 检查返回值类型
            tmp_method.return_type = parse_arg(resultm[IndexM_ReturnType].first);
            // 复制方法名
            length = resultm[IndexM_MethodName].second - resultm[IndexM_MethodName].first;
            tmp_method.name[length] = 0;
            std::strncpy(tmp_method.name, resultm[IndexM_MethodName].first, length);
            // 复制别名
            length = resultm[IndexM_AliasName].second - resultm[IndexM_AliasName].first;
            if (length) {
                tmp_method.alias[length] = 0;
                std::strncpy(tmp_method.alias, resultm[IndexM_AliasName].first, length);
            }
            else {
                std::strcpy(tmp_method.alias, tmp_method.name);
            }
            // 检查参数列表
            ISType tmptype[256];
            tmp_method.param_number = parse_arglist(
                resultm[IndexM_ParamList].first,
                resultm[IndexM_ParamList].second,
                tmptype
                );
            // 扩张
            auto tmp_type_size = tmp_method.param_number * sizeof(ISType);
            buffer.resize(tmp_type_size + buffer.size());
            // 复制数据
            std::memcpy(tmp_method.params, tmptype, sizeof(ISType) * tmp_method.param_number);
            // 增加大小
            tmp_class.size += sizeof(ISMethod) - sizeof(ISMethod::params) + tmp_type_size;
        }
    }
    // 生成数据
    auto total_size = buffer.size() + sizeof(ISInfo);
    if (this->info = reinterpret_cast<ISInfo*>(std::malloc(total_size))) {
        this->info->size = total_size;
        this->info->class_number = class_number;
        std::memcpy(this->info->kclass, buffer.data(), total_size - sizeof(ISInfo));
    }
    return this->doit();
}

// 读取二进制
bool InterfaceScriptReader::read(ISInfo* infogiven) {
    if (!infogiven) return false;
    this->~InterfaceScriptReader();
    this->info = reinterpret_cast<ISInfo*>(std::malloc(infogiven->size));
    if (this->info) {
        std::memcpy(this->info, infogiven, infogiven->size);
        return this->doit();
    }
    return false;
}

// 执行
bool InterfaceScriptReader::doit() {
    return false;
    return this->output("out.bin", OutputType::Bin) &&
        this->output("out.xml", OutputType::XML) && 
        this->output("out.cpp", OutputType::Cpp);
}


// 输出
bool InterfaceScriptReader::output(FILE * file, OutputType type) {
    if (file && this->info) {
        switch (type)
        {
        case OutputType::Bin:
            std::fwrite(this->info, 1, this->info->size, file);
            return true;
        case OutputType::XML:
        {
            pugi::xml_document doc;
        }
            break;
        case OutputType::Cpp:
            break;
        }
    }
    return false;
}
// the _longui32 后缀
constexpr uint32_t operator"" _longui32(const char* src, size_t len) {
    return len == 2 ?
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) :
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) |
        static_cast<uint32_t>(src[2]) << (8 * 2) |
        static_cast<uint32_t>(src[3]) << (8 * 3);
}

ISType parse_arg(const char* str) {
    switch (*reinterpret_cast<const uint32_t*>(str))
    {
    case "void"_longui32:
        return str[4] == '*' ? ISType::VoidP : ISType::Void;
    case "bool"_longui32:
        return ISType::Bool;
    case "char"_longui32:
        if (str[4] == '*') {
            return ISType::CharP;
        }
        goto type_error;
    case "int"_longui32:
        return ISType::Int;
    case "floa"_longui32:
        if (str[4] == 't') {
            return ISType::Float;
        }
    type_error:
    default:
        std::fprintf(stderr, "Unknown Type: %s\n", str);
        return ISType::Void;
    }
}

// 解析参数列表
uint32_t parse_arglist(const char* sbegin, const char* send, ISType types[/*256*/]){
    ISType* now_type = types;
    // 查找首部
    const char* token_begin = nullptr;
    auto index = sbegin;
    while (index < send) {
        // 查找词段首
        if (!token_begin && (*index != ' ' || *index != ',' || *index != '\t')) {
            token_begin = index;
        }
        // 推进索引
        ++index;
        // 完成字段
        if (index == send || *index == ',') {
            *now_type = parse_arg(token_begin);
            ++now_type;
            token_begin = nullptr;
        }
    }
    // 返回数量
    return now_type - types;
}