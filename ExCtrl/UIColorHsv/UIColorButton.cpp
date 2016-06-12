#include "UIColorButton.h"
#include <Core/luiManager.h>

/// <summary>
/// Render_chain_foregrounds this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorButton::render_chain_foreground() const noexcept {
    // 直接渲染
    D2D1_RECT_F rect; this->GetViewRect(rect);
    rect.left   += this->margin_rect.left;
    rect.top    += this->margin_rect.top;
    rect.right  -= this->margin_rect.right;
    rect.bottom -= this->margin_rect.bottom;
    m_pBrush_SetBeforeUse->SetColor(&m_color);
    auto brush = m_pBrush_SetBeforeUse;
    UIManager_RenderTarget->FillRectangle(&rect, brush);
    // 渲染边框
    UIControl::render_chain_foreground();
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorButton::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}


/// <summary>
/// Creates the control.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="node">The node.</param>
/// <returns></returns>
auto LongUI::UIColorButton::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIColorButton* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIColorButton, pControl, type, node);
    }
    return pControl;
}

/// <summary>
/// Initializes the specified node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIColorButton::initialize(pugi::xml_node node) noexcept {
    // 链式初始化
    Super::initialize(node);
    // 颜色
    if (const auto str = node.attribute("color").value()) {
        Helper::MakeColor(str, m_color);
    }
}


/// <summary>
/// Cleanups this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorButton::cleanup() noexcept {
    // 删除前
    this->before_deleted();
    // 删除
    delete this;
}


#ifdef LongUIDebugEvent
/// <summary>
/// [debug only]
/// </summary>
/// <param name="info">The information.</param>
/// <returns></returns>
bool LongUI::UIColorButton::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIColorButton";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIColorButton";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIColorButton>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// 重载?特例化 GetIID
template<> const IID& LongUI::GetIID<LongUI::UIColorButton>() noexcept {
    // {9DEFC417-E5C7-400B-969F-618EE8662EC0}
    static const GUID IID_LongUI_UIColorButton = { 
        0x9defc417, 0xe5c7, 0x400b, { 0x96, 0x9f, 0x61, 0x8e, 0xe8, 0x66, 0x2e, 0xc0 } 
    };
    return IID_LongUI_UIColorButton;
}
#endif