#pragma once


/*this script is c++ liked but very tiny version with some
0.name
    just call it "InterfaceScript", impl through (C++11 STD) regex.
1.comment
    supported "//" comment but "/*  * /"
    this will write in ISMethod, ISClass::comment
    in just oneline
                                MUST SPACE
                                    |
2.class                             |
    [// COMMENT]                    |
    class FULLCLASSNAME[[ALIASNAME]] : SUPERLASSNAME {
        .....
        Method Zone
        .....
    };
    be careful about the space.
3.method
    [// COMMENT]
    [static] RETYPE METHODNAME[[ALIASNAME]](PARAMLIST);
    be careful about the overload method
*/

#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <cstdint>
#include <cstdio>

static const char* LongUIInterface = u8R"longui(
// LongUI Control Base Class
class LongUI::UIControl {

};
// LongUI Container Base Class
class LongUI::UIContainer : LongUI::UIControl {

};
)longui";

// XML
#include "../../3rdparty/pugixml/pugixml.hpp"

// type
enum class ISType : uint32_t {
    Void = 0,
    Bool,
    VoidP,
    CharP,
    Int,
    Float,
    SIZE
};

// the string of typename
// NO SPACE, or error with parsing(hard to deal with space by regex)
static const char* TypeNameString[static_cast<uint32_t>(ISType::SIZE)] = {
    "void",
    "bool",
    "void*",
    "char*",
    "int",
    "float"
};

// design for binary data

// type
enum class ISMethodType : uint8_t {
    Normal = 0,
    Static,
};

// method
struct ISMethod {
    // return type
    ISType          return_type;
    // param number
    uint32_t        param_number;
    // is static?
    ISMethodType    method_type;
    // comment
    char            comment[247];
    // full method name
    char            name[64];
    // alias name, for your script
    char            alias[64];
    // parameters
    ISType          params[1];
};

// class
struct ISClass  {
    // total size in byte
    uint32_t    size;
    // method number
    uint32_t    method_number;
    // comment
    char        comment[248];
    // full class name
    char        name[64];
    // alias name, for your script
    char        alias[64];
    // super name
    char        super[64];
    // methods
    ISMethod    method[1];
};

// info
struct ISInfo {
    // total size in byte
    uint32_t    size;
    // class numer
    uint32_t    class_number;
    // classes
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
    ISClass     kclass[0];
#pragma warning(pop)
#else
    ISClass     kclass[0];
#endif
};

// 输出类型
enum class OutputType : uint32_t {
    // 2进制
    Bin = 0,
    // XML
    XML,
    // C++
    Cpp,
};

// Interface Script Reader
class InterfaceScriptReader {
public:
    // ctor
    InterfaceScriptReader() = default;
    // dtor
    ~InterfaceScriptReader(){
        if (info) {
            std::free(info);
            info = nullptr;
        }
    }
    // read the script
    bool read(const char*);
    // read the info
    bool read(ISInfo*);
    // output the data to file
    bool output(FILE* file, OutputType type);
    // output the data to file
    bool output(const char* filename, OutputType type) {
        char filetype[] = "wb";
        if (type != OutputType::Bin) filetype[1] = 0;
        FILE* file = std::fopen(filename, filetype);
        if (file) {
            bool recode = this->output(file, type);
            std::fclose(file);
            file = nullptr;
            return recode;
        }
        return false;
    }
private:
    // just doit
    bool doit();
private:
    // info
    ISInfo*         info = nullptr;;
};