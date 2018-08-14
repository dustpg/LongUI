#pragma once
//#include <cstdint>
//#include <constexpr\const_bkdr.h>

//namespace LongUI {
//    // log info
//    struct LogInfo {
//        // file name hash
//        uint32_t    file;
//        // line number
//        uint32_t    line;
//#ifndef NDEBUG
//        // file name string
//        const char* flname;
//        // function name string
//        const char* fnname;
//#endif
//        // Log
//    };
//}


//#ifdef NDEBUG
//#define UI_MAKE_LOG_INFO(name) LongUI::LogInfo name; {\
//    constexpr auto hash = LongUI::TypicalBKDR(__FILE__);\
//    name.file = hash;\
//    name.line = __LINE__;\
//}
//#else
//#define UI_MAKE_LOG_INFO(name) LongUI::LogInfo name; {\
//    constexpr auto hash = LongUI::TypicalBKDR(__FILE__);\
//    name.file = hash;\
//    name.line = __LINE__;\
//    name.flname = __FILE__;\
//    name.fnname = __FUNCTION__;\
//}
//#endif