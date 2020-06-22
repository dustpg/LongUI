#pragma once

#include <luiconf.h>
#include "../util/ui_unimacro.h"

#ifndef LUI_NO_DROPDRAG
// Windows API
#define NOMINMAX
#include <Windows.h>
#include <Shobjidl.h>

// LongUI::i namespace
namespace LongUI { namespace I {
    // Drop Target
    struct PCN_NOVTABLE DropTarget : IDropTarget {};
    // Drop Target Helper
    struct PCN_NOVTABLE DropTargetHelper : IDropTargetHelper {};
}}
#endif
