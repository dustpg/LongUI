#include "common.h"
#include "BlurText\UIBlurText.h"

// longui::additional namespace
namespace LongUI { namespace Additional {
    // config for this dll
    struct Additional::Config g_config;
}}

using LAConfig = LongUI::Additional::Config;
/// <summary>
/// LongUI Additional Class Initialize
/// </summary>
/// <param name="fonfig">The fonfig.</param>
/// <returns></returns>
void LongUIAddInitialize(const LAConfig& config) noexcept {
    LongUI::Additional::g_config = config;
}

/// <summary>
/// LongUI Additional Class Register
/// </summary>
/// <returns></returns>
void LongUIAddRegisterControl() noexcept {
    assert(LongUI::Additional::g_config.manager && "init this dll first");
    auto& manager = *LongUI::Additional::g_config.manager;
    manager.RegisterControlClass(LongUI::UIBlurText::CreateControl, "BlurText");
}

/// <summary>
/// LongUI Additional Class Uninitialize
/// </summary>
/// <returns></returns>
void LongUIAddUninitialize() noexcept {

}