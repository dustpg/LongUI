#include <core/ui_control_state.h>
#include <cstring>

/// <summary>
/// Initializes a new instance of the <see cref="CtrlState"/> struct.
/// </summary>
void LongUI::CtrlState::Init() noexcept {
    static_assert(sizeof(CtrlState) == sizeof(uint32_t) * 2, "bad size");
    reinterpret_cast<uint32_t*>(this)[0] = 0;
    reinterpret_cast<uint32_t*>(this)[1] = 0;
    visible = true;
}

