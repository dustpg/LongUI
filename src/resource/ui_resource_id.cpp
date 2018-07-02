// uii
#include <resource/ui_resource_id.h>
#include <core/ui_manager.h>
// c++
#include <cassert>

/// <summary>
/// Finalizes an instance of the <see cref="CUIResourceID"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIResourceID::~CUIResourceID() noexcept {
    if (m_id) UIManager.ReleaseResourceRefCount(m_id);
}

/// <summary>
/// Sets the identifier.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void LongUI::CUIResourceID::SetId(uint32_t id) noexcept {
    // 先加
    if (id) UIManager.AddResourceRefCount(id);
    // 后减
    if (m_id) UIManager.ReleaseResourceRefCount(m_id);
    // 最后赋值
    m_id = id;
}

/// <summary>
/// Gets the resourece data.
/// </summary>
/// <returns></returns>
auto LongUI::CUIResourceID::GetResoureceData()const noexcept->const ResourceData&{
    return UIManager.GetResoureceData(m_id);
}
