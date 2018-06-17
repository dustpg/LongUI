#include <control/ui_control_state.h>
#include <cstring>

/// <summary>
/// Initializes a new instance of the <see cref="CtrlState"/> struct.
/// </summary>
void LongUI::CtrlState::Init() noexcept {
    static_assert(sizeof(CtrlState) == sizeof(uint32_t), "bad size");
    *reinterpret_cast<uint32_t*>(this) = 0;

    visible = true;
    child_i_changed = true;
    /*
    minsize_changed:
    子控件增删换位
    子控件最小大小修改
    */
}