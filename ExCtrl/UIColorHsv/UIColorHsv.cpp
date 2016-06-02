#include "UIColorHsv.h"
#include <Core/luiManager.h>
#include "HsvEffect.h"

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::Update() noexcept {
    // 刷新父类
    Super::Update();
}

/// <summary>
/// Render_chain_mains this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::render_chain_main() const noexcept {
    // 父类渲染
    Super::render_chain_main();
    D2D1_ELLIPSE ell;
    ell.point = { 256.f, 256.f };
    ell.radiusX = 150.f;
    ell.radiusY = 150.f;
    UIManager_RenderTarget->DrawEllipse(
        &ell, m_pHsvBrush, 20.f
    );
    

    //UIManager_RenderTarget->DrawImage(m_pHsvOutput);

    /*D2D1_MATRIX_3X2_F old;

    UIManager_RenderTarget->GetTransform(&old);
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Translation(
        D2D1_SIZE_F{256.f, 256.f}
    ));
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Scale(
        D2D1_SIZE_F{2.f, 2.f},
        D2D1_POINT_2F{-512.f, -512.f}
    ));
    UIManager_RenderTarget->DrawImage(m_pHsvOutput);
    UIManager_RenderTarget->SetTransform(&old);*/

    /*D2D1_POINT_2F c;
    c.x = (rect.left + rect.right) * 0.5f;
    c.y = (rect.top + rect.bottom) * 0.5f;
    UIManager_RenderTarget->DrawEllipse(
        D2D1::Ellipse(c, 20.f, 20.f),
        m_pHsvBrush,
        5.f
    );*/
}


/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
bool LongUI::UIColorHsv::DoEvent(const LongUI::EventArgument& arg) noexcept {
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
auto LongUI::UIColorHsv::Recreate() noexcept ->HRESULT {
    this->release_gpu_resource();
    auto hr = S_OK;
    ID2D1GradientStopCollection* coll = nullptr;
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    // 重建渐变集
    if (SUCCEEDED(hr)) {
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f / 6.f, 1.f, 0.f, 0.f, 1.f},
            { 1.f / 6.f, 1.f, 1.f, 0.f, 1.f},
            { 2.f / 6.f, 0.f, 1.f, 0.f, 1.f},
            { 3.f / 6.f, 0.f, 1.f, 1.f, 1.f},
            { 4.f / 6.f, 0.f, 0.f, 1.f, 1.f},
            { 5.f / 6.f, 1.f, 0.f, 1.f, 1.f},
            { 6.f / 6.f, 1.f, 0.f, 0.f, 1.f},
        };
        // ->红（RGB(255,0,0)）
        // ->黄（RGB(255,255,0)）
        // ->绿（RGB(0,255,0)）
        // ->青（RGB(0,255,255)）
        // ->蓝（RGB(0,0,255)）
        // ->紫（RGB(255,0,255)）->红
        hr = UIManager_RenderTarget->CreateGradientStopCollection(
            stops, lengthof<uint32_t>(stops), &coll
        );
    }
    // 创建笔刷
    if (SUCCEEDED(hr)) {
        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbp;
        lgbp.startPoint = { 0.f, 0.f };
        lgbp.endPoint = { 1000.f, 1000.f };
        hr = UIManager_RenderTarget->CreateLinearGradientBrush(
            &lgbp,
            nullptr,
            coll,
            &m_pHsvBrushLG
        );
    }
    // 创建效果
    if (SUCCEEDED(hr)) {
        hr = UIManager_RenderTarget->CreateEffect(
            LongUI::CLSID_HsvEffect,
            //CLSID_D2D1Flood,
            &m_pHsvEffect
        );
        longui_debug_hr(hr, L"CreateEffect failed");
    }
    // 获取输出
    if (SUCCEEDED(hr)) {
        m_pHsvEffect->GetOutput(&m_pHsvOutput);
    }
    // 创建效果笔刷
    if (SUCCEEDED(hr)) {
        D2D1_IMAGE_BRUSH_PROPERTIES ibp;
        constexpr float a = -float(Effect::Hsv::UNIT * 2) + 1;
        constexpr float b = float(Effect::Hsv::UNIT * 2) + 1;
        ibp.sourceRectangle = { a,a,b,b };
        ibp.extendModeX = D2D1_EXTEND_MODE_CLAMP;
        ibp.extendModeY = D2D1_EXTEND_MODE_CLAMP;
        ibp.interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;
        hr = UIManager_RenderTarget->CreateImageBrush(
            m_pHsvOutput, &ibp, nullptr, &m_pHsvBrush
        );
    }
    // 扫尾处理
    LongUI::SafeRelease(coll);
    return hr;
}

/// /// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The xml-node.</param>
/// <returns></returns>
void LongUI::UIColorHsv::initialize(pugi::xml_node node) noexcept {
    // 链式初始化
    Super::initialize(node);
    m_colorPicked = D2D1::ColorF(D2D1::ColorF::Red);
}


/// <summary>
/// Creates the control.
/// </summary>
/// <param name="type">The type.</param>
/// <param name="node">The node.</param>
/// <returns></returns>
auto LongUI::UIColorHsv::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIColorHsv* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
    {
        auto hr = Effect::Hsv::Register(UIManager_D2DFactory);
        auto hrcode = static_cast<size_t>(hr);
        pControl = reinterpret_cast<UIColorHsv*>(hrcode);
    }
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIColorHsv, pControl, type, node);
    }
    return pControl;
}


/// <summary>
/// Release_gpu_resources this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::release_gpu_resource() noexcept {
    LongUI::SafeRelease(m_pHsvBrushLG);
    LongUI::SafeRelease(m_pHsvEffect);
    LongUI::SafeRelease(m_pHsvOutput);
    LongUI::SafeRelease(m_pHsvBrush);

}

/// <summary>
/// Cleanups this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIColorHsv::cleanup() noexcept {
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
bool LongUI::UIColorHsv::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIColorHsv";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIColorHsv";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIColorHsv>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// 重载?特例化 GetIID
template<> const IID& LongUI::GetIID<LongUI::UIColorHsv>() noexcept {
    // {BEB929FF-5FA8-4ED5-AEF5-ADC8C387F523}
    static const GUID IID_LongUI_UIColorHsv = {
        0xbeb929ff, 0x5fa8, 0x4ed5, { 0xae, 0xf5, 0xad, 0xc8, 0xc3, 0x87, 0xf5, 0x23 } 
    };
    return IID_LongUI_UIColorHsv;
}
#endif