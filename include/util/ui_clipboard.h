#pragma once

#include "../core/ui_core_type.h"

// ui namespace
namespace LongUI {
    // copy text to clipboard
    bool CopyTextToClipboard(U16View view) noexcept;
    // paste text from clipboard
    bool PasteTextToClipboard(CUIString& str) noexcept;
}