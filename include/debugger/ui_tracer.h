#pragma once

// output to stdout
#define UI_TRACE_TO_STDOUT

// you should implement uidbg_trace
//#define UI_TRACE_TO_CUSTOM

#ifdef NDEBUG
#define UI_TRACE_MSG(msg)
#define UI_FREE_DATA()
#else
#define UI_FREE_DATA() uidbg_freedata()
#include <cstdint>
#include "../util/ui_unimacro.h"

using uidbg_trace_func = void(*) ();
void uidbg_freedata() noexcept;
bool uidbg_breakpoint() noexcept;
auto uidbg_init_trace(uidbg_trace_func) noexcept->uint32_t;
void uidbg_goto_id(uint32_t id) noexcept;
void uidbg_trace(uint32_t id, const char* func, const char* msg) noexcept;

#define UI_TRACE_MSG(msg) {\
static uint32_t s_tid_ = uidbg_init_trace([] { uidbg_breakpoint(); });\
uidbg_trace(s_tid_, __FUNCTION__, msg);\
}
#endif

#define UI_TRACE UI_TRACE_MSG("-")