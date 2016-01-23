#include "../common.h"
#include "UIBlurText.h"

#undef UIManager
#define UIManager (*LongUI::Additional::g_config.manager)

// longui namespace
namespace LongUI {
    // Gaussian Blur GUID
    GUID CLSID_D2D1GaussianBlur = {
        0x1feb6d69, 0x2fe6, 0x4ac9, { 0x8c, 0x58, 0x1d, 0x7f, 0x93, 0xe7, 0xa6, 0xa5 }
    };
}


// ----------------------------------------------------------------------------
// **** UIBlurText
// ----------------------------------------------------------------------------

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIBlurText::Render() const noexcept {
    if (m_pWindow->IsPrerender()) {
        force_cast(m_effect).Record(
            UIManager_RenderTarget, [this]() noexcept {
            this->Super::Render();
        });
    }
    else {
        m_effect.Render();
    }
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIBlurText::Update() noexcept {
    // 改变了大小
    if (this->IsControlLayoutChanged()) {
        m_effect.SetInvalidate();
    }
    return Super::Update();
}

// UIText: 事件响应
bool LongUI::UIBlurText::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LONGUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetText:
            // 修改文本
            m_effect.SetInvalidate();
            Super::DoEvent(arg);
        case LongUI::Event::Event_SetFloat:
            // 修改浮点数据
            this->SetBlurValue(arg.stf.value);
            return true;
        case LongUI::Event::Event_GetFloat:
            // 获取浮点数据
            arg.fvalue = this->GetBulrValue();
            return true;
        }
    }
    return Super::DoEvent(arg);
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UIBlurText::Recreate() noexcept ->HRESULT {
    auto hr = S_OK;
    // 重建特效
    if (SUCCEEDED(hr)) {
        m_effect.SetValidate();
        hr = m_effect.Recreate();
    }
    // 设置特效
    if (SUCCEEDED(hr)) {
        m_effect.SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED);
        m_effect.SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, m_fBlur);
        m_effect.SetInvalidate();
    }
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    return hr;
}

/// /// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The xml-node.</param>
/// <returns></returns>
void LongUI::UIBlurText::initialize(pugi::xml_node node) noexcept {
    // 链式初始化
    Super::initialize(node);
    // 获取模糊值
    const char* str = node.attribute("blur").value();
    if (str) m_fBlur = LongUI::AtoF(str);
    // 注册离屏渲染控件
    m_pWindow->RegisterOffScreenRender2D(this);
}


// UIText::CreateControl 函数
auto LongUI::UIBlurText::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIBlurText* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIBlurText, pControl, type, node);
    }
    return pControl;
}

// UIBlurText: 清理控件
void LongUI::UIBlurText::cleanup() noexcept {
    delete this;
}


#ifdef LongUIDebugEvent
// UI可模糊文本: 调试信息
bool LongUI::UIBlurText::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIBlurText";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIBlurText";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIBlurText>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}
#endif