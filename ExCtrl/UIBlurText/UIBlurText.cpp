#include "UIBlurText.h"
#include <Core/luiManager.h>

// longui namespace
namespace LongUI {
    // Gaussian Blur GUID
    const GUID CLSID_D2D1GaussianBlur = {
        0x1feb6d69, 0x2fe6, 0x4ac9, { 0x8c, 0x58, 0x1d, 0x7f, 0x93, 0xe7, 0xa6, 0xa5 }
    };
}


/// <summary>
/// Sets the blur value.
/// </summary>
/// <param name="sd">The sd.</param>
void LongUI::UIBlurText::SetBlurValue(float sd) noexcept { 
    LongUI::CUIDxgiAutoLocker locker;
    m_fBlur = sd;
    if(m_effect.IsOK()) m_effect.SetValue(
        D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, m_fBlur
    );
    m_pWindow->Invalidate(this);
}

// ----------------------------------------------------------------------------
// **** UIBlurText
// ----------------------------------------------------------------------------
LongUI::UIBlurText::UIBlurText(UIContainer * cp) noexcept: 
Super(cp), m_effect(LongUI::CLSID_D2D1GaussianBlur) {
    
}


/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIBlurText::Render() const noexcept {
    m_effect.Render();
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIBlurText::Update() noexcept {
    // 刷新父类
    Super::Update();
    // 改变了大小
    if (this->IsControlLayoutChanged()) {
        m_effect.SetInvalidate();
    }
    // 记录
    m_effect.Record(
        UIManager_RenderTarget, [this]() noexcept {
        this->Super::Render();
#ifdef _DEBUG
        UIManager << DL_Hint << L"Recorded" << LongUI::endl;
#endif
    });
}

// UIText: 事件响应
bool LongUI::UIBlurText::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LONGUI 事件
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
        constexpr auto i1 = D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION;
        constexpr auto v1 = D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED;
        m_effect.SetValue(i1, v1);
        constexpr auto i2 = D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION;
        m_effect.SetValue(i2, m_fBlur);
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
    if (const auto str = node.attribute("blur").value()) {
        m_fBlur = LongUI::AtoF(str);
    }
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
    // 删除前
    this->before_deleted();
    // 删除
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

// 重载?特例化 GetIID
template<> const IID& LongUI::GetIID<LongUI::UIBlurText>() noexcept {
    // {1AF45E28-9342-4CA4-AA6E-E48048C5E7AE}
    static const GUID IID_LongUI_UIBlurText = { 
        0x1af45e28, 0x9342, 0x4ca4, { 0xaa, 0x6e, 0xe4, 0x80, 0x48, 0xc5, 0xe7, 0xae }
    };
    return IID_LongUI_UIBlurText;
}
#endif