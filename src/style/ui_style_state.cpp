// ui
#include <style/ui_style_state.h>
#include <typecheck/int_by_size.h>
// c++
#include <cassert>

/// <summary>
/// Initializes a new instance of the <see cref="StyleState"/> struct.
/// </summary>
void LongUI::StyleState::Init() noexcept {
    static_assert(sizeof(*this) == sizeof(uint32_t), "");
    reinterpret_cast<uint32_t&>(*this) = 0;
    //none        = false;
    //odd_index   = false;
    //selected    = false;
    //disabled    = false;
    //active      = false;
    //hover       = false;
    //focus       = false;
    //checked     = false;
    //default0    = false;
    //indeterminate = false;
    //closed      = false;
}


/// <summary>
/// Changes the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns><c>true</c>if changed</returns>
bool LongUI::StyleState::Change(StyleStateTypeChange c) noexcept {
    using ssint_t = typename type_helper::int_type<sizeof(StyleState)>::unsigned_t;
    union { StyleState state; ssint_t data; };
#ifndef NDEBUG
    assert(c.type != StyleStateType::Type_None && "cannot change none");
    assert(ssint_t(c.change) == 0 || ssint_t(c.change) == 1);
    state = *this;
    this->none = false;
    data |= 1 << ssint_t(StyleStateType::Type_None);
    *this = state;
    assert(this->none && "unsupported bit-field-implement");
    this->none = false;
#endif
    state = *this;
    bool rv = ((data >> ssint_t(c.type)) ^ ssint_t(c.change)) & 1;
    ssint_t mask = ~ssint_t(1 << ssint_t(c.type));
    data &= mask;
    data |= (ssint_t(c.change) << ssint_t(c.type));
    *this = state;
    return rv;
}