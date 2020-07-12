#include <debugger/ui_tracer.h>

#ifndef NDEBUG
#include <cstdlib>
#include <cstdio>
#include <thread/ui_locker.h>

/// <summary>
/// Uidbgs the breakpoint.
/// </summary>
/// <returns></returns>
bool uidbg_breakpoint() noexcept {
    const char* bk = nullptr;
    bk = "add a break point here in ide";
    return !bk;
}

namespace LongUI { namespace impl {
    /// <summary>
    /// trace data
    /// </summary>
    struct debug_trace_data {
        uint32_t id_counter;
        uint32_t trace_len;
        uint32_t trace_cap;
        uidbg_trace_func*   trace_jump_ptr;
        LongUI::CUILocker       locker;
        void push(uidbg_trace_func) noexcept;
        ~debug_trace_data() { std::free(trace_jump_ptr); }
    } dbg_trace_data = {};
}}


/// <summary>
/// Pushes the specified buf.
/// </summary>
/// <param name="buf">The buf.</param>
/// <returns></returns>
void  LongUI::impl::debug_trace_data::push(uidbg_trace_func buf) noexcept {
    using namespace LongUI::impl;
    if (trace_cap == trace_len) {
        trace_cap += 2 + trace_cap / 2;
        auto ptr = std::realloc(trace_jump_ptr, sizeof(void*) * trace_cap);
        if (!ptr) { std::free(trace_jump_ptr); trace_jump_ptr = nullptr; }
        else trace_jump_ptr = reinterpret_cast<uidbg_trace_func*>(ptr);
    }
    if (trace_jump_ptr) {
        trace_jump_ptr[trace_len] = buf;
        trace_len++;
    }
}

/// <summary>
/// Uidbgs the initialize trace.
/// </summary>
/// <param name="func">The function.</param>
/// <returns></returns>
auto uidbg_init_trace(uidbg_trace_func func) noexcept -> uint32_t {
    using namespace LongUI::impl;
    dbg_trace_data.locker.Lock();
    dbg_trace_data.push(func);
    const auto count = dbg_trace_data.id_counter++;
    dbg_trace_data.locker.Unlock();
    return count;
}

/// <summary>
/// Uidbgs the goto identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void uidbg_goto_id(uint32_t id) noexcept {
    using namespace LongUI::impl;
    uidbg_trace_func func = nullptr;
    dbg_trace_data.locker.Lock();
    if (dbg_trace_data.trace_jump_ptr && id < dbg_trace_data.trace_len) {
        func = dbg_trace_data.trace_jump_ptr[id];
    }
    dbg_trace_data.locker.Unlock();
    if (func) func();
}

#ifndef UI_TRACE_TO_CUSTOM
#ifdef UI_TRACE_TO_STDOUT
#include <cstdio>
#include <cinttypes>
/// <summary>
/// Uidbgs the trace.
/// </summary>
/// <param name="id">The identifier.</param>
/// <param name="func">The function.</param>
/// <param name="msg">The MSG.</param>
/// <returns></returns>
void uidbg_trace(uint32_t id, const char* func, const char* msg) noexcept {
    std::printf("[TRACE:%3" PRIu32 "]<%32s> { %s }\n", id, func, msg);
}
#else
#include <windows.h>
#include <cinttypes>
/// <summary>
/// Uidbgs the trace.
/// </summary>
/// <param name="id">The identifier.</param>
/// <param name="func">The function.</param>
/// <param name="msg">The MSG.</param>
/// <returns></returns>
void uidbg_trace(uint32_t id, const char* func, const char* msg) noexcept {
    char buf[1024];
    std::snprintf(
        buf, sizeof(buf),
        "[TRACE:%3" PRIu32 "]<%32s> { %s }\r\n",
        id, func, msg
    );
    ::OutputDebugStringA(buf);
}
#endif
#endif

/// <summary>
/// Uidbgs the freedata.
/// </summary>
/// <returns></returns>
void uidbg_freedata() noexcept {
    auto&ptr = LongUI::impl::dbg_trace_data.trace_jump_ptr;
    std::free(ptr);
    ptr = nullptr;
}
#endif