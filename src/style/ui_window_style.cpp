// ui
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_string.h>
// c++
#include <cassert>

/// <summary>
/// Loads the CSS file.
/// </summary>
/// <param name="file">The file.</param>
/// <returns></returns>
void LongUI::CUIWindow::LoadCSSFile(U8View file) noexcept {
    // TODO: OOM 处理
    auto path = UIManager.GetXULDir();
    path += CUIString::FromUtf8(file);
    // 载入文件
    if (CUIFile css_file{ path.c_str(), CUIFile::Flag_Read }) {
        const auto file_size = css_file.GetFilezize();
        POD::Vector<char> css_buffer;
        css_buffer.resize(file_size + 1);
        if (css_buffer.is_ok()) {
            const auto ptr = &css_buffer.front();
            css_file.Read(ptr, file_size);
            ptr[file_size] = 0;
            this->LoadCSSString({ ptr, ptr + file_size });
        }
    }
}

/// <summary>
/// Loads the CSS string.
/// </summary>
/// <param name="string">The string.</param>
/// <returns></returns>
void LongUI::CUIWindow::LoadCSSString(U8View string) noexcept {
    int bk = 9;
}
