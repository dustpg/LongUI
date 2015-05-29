#include "script.h"

// 命令行帮助
static const char* CommandHelp = R"longui([HELP]:
     -h -help       : show help content
    [-o] <filename> : output the C++ source code to this file
)longui";


// 应用程序入口
int main(int argc, const char* argv[]) {
    InterfaceScriptReader reader;
    reader.read(LongUIInterface);
    // 检查参数
    if (argc == 1 || *reinterpret_cast<const uint16_t*>(argv[1]) == 'h-' ||
        *reinterpret_cast<const uint32_t*>(argv[1]) == 'leh-') {
        printf(CommandHelp);
    }
    (void)std::getchar();
    return EXIT_SUCCESS;
}