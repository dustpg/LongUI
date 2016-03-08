#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <regex>
#include <io.h>
#include <filesystem>

static constexpr uint32_t PATH_BUFFER_LENGTH = 1024;
static constexpr uint32_t FILE_BUFFER_LENGTH = 1024;

// entry
int wmain(int argc, wchar_t* argv[]) {
    if (argc < 3) return -1;
    wchar_t path[PATH_BUFFER_LENGTH];
    std::swprintf(path, PATH_BUFFER_LENGTH, L"%ls\\%ls", argv[1], argv[2]);
    auto writefile = ::_wfopen(path, L"wb");
    if (!writefile) return -1;
    char bom[] = { char(0xEF), char(0xBB), char(0xBF) };
    std::fwrite(bom, 1, 3, writefile);
    for (auto i = 3; i < argc; ++i) {
        auto headerfile = argv[i];
        char maker[2];
        if (std::wcschr(headerfile, L'.')) {
            maker[0] = maker[1] = '"';
        }
        else {
            maker[0] = '<';
            maker[1] = '>';
        }
        std::fprintf(writefile, "#include %c%ls%c\r\n", maker[0], argv[i], maker[1]);
    }
    std::swprintf(path, PATH_BUFFER_LENGTH, L"%ls\\*.cpp", argv[1]);
    _wfinddata32_t fdata = { 0 };
    auto handle = _wfindfirst32(path, &fdata);
    std::regex regex(R"rex(#\s*include.+)rex");
    const std::cregex_iterator itr_end;
    if (handle != -1) {
        do {
            if (!std::wcscmp(fdata.name, argv[2])) continue;
            std::swprintf(path, PATH_BUFFER_LENGTH, L"%ls\\%ls", argv[1], fdata.name);
            std::wprintf(L"cpp file found: %ls\r\n", path);
            auto file = ::_wfopen(path, L"rb");
            if (file) {
                char buf[FILE_BUFFER_LENGTH +1];
                size_t read_count = 0;
                bool first = true;
                while ((read_count = std::fread(buf, 1, FILE_BUFFER_LENGTH, file))) {
                    auto eos = buf + read_count;
                    *eos = 0;
                    auto realdata = buf;
                    if (first) {
                        if (!std::strncmp(realdata, bom, 3)) {
                            realdata += 3;
                            read_count -= 3;
                        }
                        for (std::cregex_iterator itr(realdata, eos, regex); itr != itr_end; ++itr) {
                            for (auto& part : *itr) {
                                std::memset(
                                    const_cast<void*>(reinterpret_cast<const void*>(part.first)), 
                                    ' ', 
                                    part.length()
                                    );
                            }
                        }
                    }
                    first = false;
                    std::fwrite(realdata, 1, read_count, writefile);
                }
                std::fwrite("\r\n", 1, 2, writefile);
                std::fclose(file);
            }
            else {
                return -1;
            }
        } while (_wfindnext32(handle, &fdata) != -1);
        _findclose(handle);
        handle = -1;
    }
    std::fclose(writefile);
    std::wprintf(L"%ls: write finished\r\n", argv[2]);
    return 0;
}