#include <Windows.h>
#include <filesystem/ui_pathop.h>
#include <core/ui_string.h>

// longui::detail namespace
namespace LongUI { namespace impl {
    // is a dot
    inline auto is_a_dot(const char* str) noexcept {
        return str[0] == '.';
    }
    // is double dot
    inline auto is_double_dot(const char* str) noexcept {
        return str[0] == '.' && str[1] == '.';
    }
    // utf16 to system char type
    static inline auto sys(char16_t* str) noexcept {
        using target_t = wchar_t;
        static_assert(sizeof(target_t) == sizeof(char16_t), "WINDOWS!");
        return reinterpret_cast<wchar_t*>(str);
    }
}}

/// <summary>
/// Canonicals the specified buf.
/// </summary>
/// <param name="path">The path.</param>
/// <returns></returns>
auto LongUI::PathOP::Canonical(char* path) noexcept -> uint32_t {
    // 先将反斜杠转换为斜杠 顺便压缩 //////
    const auto no_backslash_multislash = [path]() {
        uint32_t i = 0, j = 0;
        while (path[i]) {
            // 先将反斜杠转换为斜杠
            if (path[i] == '/' || path[i] == '\\') {
                path[j++] = '/';
                // 压缩多重斜杠
                while (path[i] == '/' || path[i] == '\\') i++;
            }
            // 将后面的拿到前面
            else path[j++] = path[i++];
        }
        // 结束了
        path[j] = 0;
    };
    // 解析整个字符串
    const auto parse_string = [path]() noexcept {
        uint32_t i = 0, j = 0, k = 0;
        // 全循环
        do {
            // 遍历到斜杠?
            if (path[i] == '/' || path[i] == '\0') {
                // 找到一个点?
                if ((i - j) == 1 && impl::is_a_dot(path + j)) {
                    // 检查路径名是否为空?
                    if (k == 0) {
                        if (path[i] == '\0') {
                            path[k++] = '.';
                        }
                        else if (path[i] == '/' && path[i + 1] == '\0') {
                            path[k++] = '.';
                            path[k++] = '/';
                        }
                    }
                    else if (k > 1) {
                        // 必要时删除最后的斜杠
                        if (path[i] == '\0') k--;
                    }
                }
                // 找到两个个点?
                else if ((i - j) == 2 && impl::is_double_dot(path + j)) {
                    // 检查路径名是否为空?
                    if (k == 0) {
                        path[k++] = '.';
                        path[k++] = '.';
                        // 必要时添加一个斜杠
                        if (path[i] == '/') path[k++] = '/';
                    }
                    else if (k > 1) {
                        // 搜索前一个斜杠的路径
                        for (j = 1; j < k; j++) {
                            if (path[k - j - 1] == '/') break;
                        }
                        // 找到斜杠分隔符
                        if (j < k) {
                            if (impl::is_double_dot(path + k - j)) {
                                path[k++] = '.';
                                path[k++] = '.';
                            }
                            else {
                                k = k - j - 1;
                            }
                            // 必要时添加一个斜杠
                            if (k == 0 && path[0] == '/')
                                path[k++] = '/';
                            else if (path[i] == '/')
                                path[k++] = '/';
                        }
                        // 找不到斜杠分隔符
                        else {
                            if (k == 3 && impl::is_double_dot(path)) {
                                path[k++] = '.';
                                path[k++] = '.';
                                // 必要时添加一个斜杠
                                if (path[i] == '/') path[k++] = '/';
                            }
                            else if (path[i] == '\0') {
                                k = 0;
                                path[k++] = '.';
                            }
                            else if (path[i] == '/' && path[i + 1] == '\0') {
                                k = 0;
                                path[k++] = '.';
                                path[k++] = '/';
                            }
                            else {
                                k = 0;
                            }
                        }
                    }
                }
                else {
                    // 复制目录
                    std::memmove(path + k, path + j, i - j);
                    // 推进写指针
                    k += i - j;
                    // 必要时添加一个斜杠
                    if (path[i] == '/') path[k++] = '/';
                }
                // 移动到下一个字符段
                while (path[i] == '/') i++;
                j = i;
            }
        } while (path[i++]);
        // NULL!
        path[k] = 0;
        return k;
    };
    // 转换压缩
    no_backslash_multislash();
    // 解析字串
    return parse_string();
}

/// <summary>
/// Makes the URI path.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="prefix">The prefix.</param>
/// <param name="path">The path.</param>
/// <returns></returns>
auto LongUI::PathOP::MakeUriPath(UriPath& buf, U8View prefix, U8View path) noexcept -> U8View {
    const auto length1 = prefix.end() - prefix.begin();
    const auto length2 = path.end() - path.begin();
    // 长度太长
    if (length1 + length2 >= FILEOP_MAX_PATH) return { buf.path, buf.path };
    // 复制数据
    std::memcpy(buf.path, prefix.begin(), length1);
    std::memcpy(buf.path + length1, path.begin(), length2);
    // 标记结束
    buf.path[length1 + length2] = 0;
    // 简化路径
    const auto after = PathOP::Canonical(buf.path);
    // 返回数据
    return { buf.path, buf.path + after };
}

/// <summary>
/// Gets the temporary path.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::PathOP::TempDirectoryPath(BasePath& bp) noexcept->uint32_t {
    return ::GetTempPathW(FILEOP_MAX_PATH, bp.path);
}


/// <summary>
/// Temporaries the directory path.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::PathOP::TempDirectoryPath(CUIString& str) noexcept -> uint32_t {
    const auto buflen = ::GetTempPathW(0, nullptr);
    str.as_buffer_nul(buflen -1, [buflen](char16_t* buf) noexcept {
        ::GetTempPathW(buflen, impl::sys(buf));
    });
    return static_cast<uint32_t>(str.length());
}

/// <summary>
/// Temporaries the name of the file.
/// </summary>
/// <param name="path">The path.</param>
/// <param name="prefix">The prefix.</param>
/// <param name="filename">The filename.</param>
/// <returns></returns>
auto LongUI::PathOP::TempFileName(
    const wchar_t* path, 
    const wchar_t* prefix, 
    BasePath & filename) noexcept -> uint32_t {
    return ::GetTempFileNameW(path, prefix, 0, filename.path);
}
