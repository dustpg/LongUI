#include "UIXmlRich.h"
#include <Core/luiManager.h>

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIXmlRich::Update() noexcept {
    // 刷新父类
    Super::Update();
}


/// <summary>
/// Sets the color of the selection.
/// </summary>
/// <returns></returns>
void LongUI::UIXmlRich::SetSelectionColor() noexcept {
    if (!m_text.GetSelectionRange().length) return;
    CHOOSECOLORW cc;
    constexpr auto a = sizeof(m_aSaveColor[0]);
    constexpr auto b = sizeof(cc.lpCustColors[0]);
    ID2D1ColorContext;
    static_assert(a == b, "bad");
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = m_pWindow->GetHwnd();
    cc.hInstance = nullptr;
    cc.rgbResult = 0;
    cc.lpCustColors = reinterpret_cast<COLORREF*>(m_aSaveColor);
    cc.Flags = CC_ANYCOLOR;
    cc.lCustData = 0;
    cc.lpfnHook = nullptr;
    cc.lpTemplateName = nullptr;
    if (::ChooseColorW(&cc)) {
        D2D1_COLOR_F c;
        c.r = float(GetRValue(cc.rgbResult)) / 255.f;
        c.g = float(GetGValue(cc.rgbResult)) / 255.f;
        c.b = float(GetBValue(cc.rgbResult)) / 255.f;
        c.a = 1.f;
        this->SetSelectionColor(c);
    }
}


/// <summary>
/// Set_selection_helpers the specified call.
/// </summary>
/// <param name="call">The call.</param>
/// <returns></returns>
template<typename T>
void LongUI::UIXmlRich::set_selection_helper(T call) noexcept {
    if (!m_text.layout) return;
    auto range = m_text.GetSelectionRange();
    if (range.length) {
        LongUI::CUIDxgiAutoLocker locker;
        call(range);
        this->InvalidateThis();
    }
}

/// <summary>
/// Sets the selection underline.
/// </summary>
/// <param name="u">if set to <c>true</c> [u].</param>
/// <returns></returns>
void LongUI::UIXmlRich::SetSelectionUnderline(bool u) noexcept {
    this->set_selection_helper([=](DWRITE_TEXT_RANGE range)noexcept {
        m_text.layout->SetUnderline(u, range);
    });
}

/// <summary>
/// Sets the selection strikethrough.
/// </summary>
/// <param name="u">if set to <c>true</c> [u].</param>
/// <returns></returns>
void LongUI::UIXmlRich::SetSelectionStrikethrough(bool u) noexcept {
    this->set_selection_helper([=](DWRITE_TEXT_RANGE range)noexcept {
        m_text.layout->SetStrikethrough(u, range);
    });
}

/// <summary>
/// Sets the selection style.
/// </summary>
/// <param name="s">The s.</param>
/// <returns></returns>
void LongUI::UIXmlRich::SetSelectionStyle(DWRITE_FONT_STYLE s) noexcept {
    this->set_selection_helper([=](DWRITE_TEXT_RANGE range)noexcept {
        m_text.layout->SetFontStyle(s, range);
    });
}

/// <summary>
/// Sets the color of the selection.
/// </summary>
/// <param name="c">The c.</param>
/// <returns></returns>
void LongUI::UIXmlRich::SetSelectionColor(const D2D1_COLOR_F& c) noexcept {
    this->set_selection_helper([=](DWRITE_TEXT_RANGE range)noexcept {
        if (auto color = CUIColorEffect::Create(c)) {
            m_text.layout->SetDrawingEffect(color, range);
            color->Release();
        }
    });
}

/// <summary>
/// Sets the selection weight.
/// </summary>
/// <param name="w">The w.</param>
/// <returns></returns>
void LongUI::UIXmlRich::SetSelectionWeight(DWRITE_FONT_WEIGHT w) noexcept {
    this->set_selection_helper([=](DWRITE_TEXT_RANGE range)noexcept {
        m_text.layout->SetFontWeight(w, range);
    });
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
bool LongUI::UIXmlRich::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LONGUI 事件
    /*if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetText:
            Super::DoEvent(arg);
            return true;
        }
    }*/
    return Super::DoEvent(arg);
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UIXmlRich::Recreate() noexcept ->HRESULT {
    auto hr = S_OK;
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
void LongUI::UIXmlRich::initialize(pugi::xml_node node) noexcept {
    // 添加富文本编辑
    node.append_attribute("textrich").set_value("true");
    // 链式初始化
    Super::initialize(node);
    // 初始化
    std::memset(m_aSaveColor, 0, sizeof(m_aSaveColor));
}


/// <summary>
/// Creates the control.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="node">The node.</param>
/// <returns></returns>
auto LongUI::UIXmlRich::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIXmlRich* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIXmlRich, pControl, type, node);
    }
    return pControl;
}



/// <summary>
/// Cleanups this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIXmlRich::cleanup() noexcept {
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
bool LongUI::UIXmlRich::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIXmlRich";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIXmlRich";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIXmlRich>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// 重载?特例化 GetIID
template<> const IID& LongUI::GetIID<LongUI::UIXmlRich>() noexcept {
    // {65FFBC0B-946F-4C10-9CAB-5F271EAE4C0C}
    static const GUID IID_LongUI_UIXmlRich = {
        0x65ffbc0b, 0x946f, 0x4c10, { 0x9c, 0xab, 0x5f, 0x27, 0x1e, 0xae, 0x4c, 0xc }
    };
    return IID_LongUI_UIXmlRich;
}
#endif