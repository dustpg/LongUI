#include "LongUI.h"

// 实现

#define UIElements_Prefix if (!node) return; if(!prefix) prefix = ""; char buffer[256];
#define UIElements_NewAttribute(a) { ::strcpy(buffer, prefix); ::strcat(buffer, a); }
#define UIElements_Attribute node.attribute(buffer).value()


// Elements<Basic> Init
void LongUI::Component::Elements<LongUI::Element::Basic>::
Init(pugi::xml_node node, const char* prefix) noexcept {
    // 无效?
    UIElements_Prefix;
    const char* str = nullptr;
    // 动画类型
    UIElements_NewAttribute("animationtype");
    if (str = UIElements_Attribute) {
        animation.type = static_cast<AnimationType>(LongUI::AtoI(str));
    }
    // 动画持续时间
    UIElements_NewAttribute("animationtype");
    if (str = UIElements_Attribute) {
        animation.duration = LongUI::AtoF(str);
    }
}

// 设置新的状态
auto LongUI::Component::Elements<LongUI::Element::Basic>::
SetNewStatus(LongUI::ControlStatus new_status) noexcept ->float {
    m_state = m_stateTartget;
    m_stateTartget = new_status;
    animation.value = 0.f;
    animation.start = 0.f;
    return animation.time = animation.duration;
}

// Elements<Meta> 构造函数
LongUI::Component::Elements<LongUI::Element::Meta>::
Elements(pugi::xml_node node, const char* prefix) noexcept: Super(node, prefix) {
    ZeroMemory(m_metas, sizeof(m_metas));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 无效?
    UIElements_Prefix;
    // 禁用状态Meta ID
    UIElements_NewAttribute("disabledmeta");
    m_aID[Status_Disabled] = LongUI::AtoI(UIElements_Attribute);
    // 通常状态Meta ID
    UIElements_NewAttribute("normalmeta");
    m_aID[Status_Normal] = LongUI::AtoI(UIElements_Attribute);
    // 移上状态Meta ID
    UIElements_NewAttribute("hovermeta");
    m_aID[Status_Hover] = LongUI::AtoI(UIElements_Attribute);
    // 按下状态Meta ID
    UIElements_NewAttribute("pushedmeta");
    m_aID[Status_Pushed] = LongUI::AtoI(UIElements_Attribute);
}


// Elements<Meta> 重建
auto LongUI::Component::Elements<LongUI::Element::Meta>::
Recreate(LongUIRenderTarget* target) noexcept ->HRESULT {
    for (auto i = 0u; i < STATUS_COUNT; ++i) {
        // 有效
        register auto id = m_aID[i];
        if (id) {
            UIManager.GetMeta(id, m_metas[i]);
        }
    }
    return S_OK;
}

// Elements<Meta> 渲染
void LongUI::Component::Elements<LongUI::Element::Meta>::Render(const D2D1_RECT_F& rect) noexcept {
    assert(m_pRenderTarget);
    // 先绘制当前状态
    if (this->animation.value < this->animation.end) {
        LongUI::Meta_Render(
            m_metas[m_state], m_pRenderTarget, rect, this->animation.end
            );
    }
    // 再绘制目标状态
    LongUI::Meta_Render(
        m_metas[m_stateTartget], m_pRenderTarget, rect, this->animation.value
        );
}



// Elements<BrushRect> 构造函数
LongUI::Component::Elements<LongUI::Element::BrushRect>::
Elements(pugi::xml_node node, const char* prefix) noexcept :Super(node, prefix) {
    ZeroMemory(m_apBrushes, sizeof(m_apBrushes));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 无效?
    UIElements_Prefix;
    // 禁用状态笔刷ID
    UIElements_NewAttribute("disabledbrush");
    m_aID[Status_Disabled] = LongUI::AtoI(UIElements_Attribute);
    // 通常状态笔刷ID
    UIElements_NewAttribute("normalbrush");
    m_aID[Status_Normal] = LongUI::AtoI(UIElements_Attribute);
    // 移上状态笔刷ID
    UIElements_NewAttribute("hoverbrush");
    m_aID[Status_Hover] = LongUI::AtoI(UIElements_Attribute);
    // 按下状态笔刷ID
    UIElements_NewAttribute("pushedbrush");
    m_aID[Status_Pushed] = LongUI::AtoI(UIElements_Attribute);
}

// 释放数据
void LongUI::Component::Elements<LongUI::Element::BrushRect>::release_data() noexcept {
    for (auto& brush : m_apBrushes) {
        ::SafeRelease(brush);
    }
}

// Elements<BrushRectta> 渲染
void LongUI::Component::Elements<LongUI::Element::BrushRect>::Render(const D2D1_RECT_F& rect) noexcept {
    assert(m_pRenderTarget);
    D2D1_MATRIX_3X2_F matrix; m_pRenderTarget->GetTransform(&matrix);
#if 1
    // 计算转换后的矩形
    auto height = rect.bottom - rect.top;
    D2D1_RECT_F rect2 = {
        0.f, 0.f, (rect.right - rect.left) / height , 1.f
    };
    // 计算转换后的矩阵
    m_pRenderTarget->SetTransform(
        D2D1::Matrix3x2F::Scale(height, height) *
        D2D1::Matrix3x2F::Translation(rect.left, rect.top) *
        matrix
        );
    // 先绘制当前状态
    if (animation.value < animation.end) {
        m_pRenderTarget->FillRectangle(rect2, m_apBrushes[m_state]);
    }
    // 后绘制目标状态
    auto brush = m_apBrushes[m_stateTartget];
    brush->SetOpacity(animation.value);
    m_pRenderTarget->FillRectangle(rect2, brush);
    brush->SetOpacity(1.f);
    m_pRenderTarget->SetTransform(&matrix);
#else
    //m_pRenderTarget->SetTransform(D2D1::IdentityMatrix());
    // 先绘制当前状态
    if (animation.value < animation.end) {
        m_apBrushes[m_state]->SetTransform(&matrix);
        m_pRenderTarget->FillRectangle(rect, m_apBrushes[m_state]);
    }
    // 后绘制目标状态
    auto brush = m_apBrushes[m_stateTartget];
    brush->SetOpacity(animation.value);
    //brush->SetTransform(&matrix);
    m_pRenderTarget->FillRectangle(rect, brush);
    brush->SetOpacity(1.f);
    //m_pRenderTarget->SetTransform(&matrix);
#endif
}

// Elements<BrushRect> 重建
auto LongUI::Component::Elements<LongUI::Element::BrushRect>::
Recreate(LongUIRenderTarget* target) noexcept ->HRESULT {
    this->release_data();
    for (auto i = 0u; i < STATUS_COUNT; ++i) {
        register auto id = m_aID[i];
        m_apBrushes[i] = id ? UIManager.GetBrush(id) : UIManager.GetSystemBrush(i);
    }
    return S_OK;
}

// Elements<ColorRect> 构造函数
LongUI::Component::Elements<LongUI::Element::ColorRect>::
Elements(pugi::xml_node node, const char* prefix) noexcept: Super(node, prefix) {
    colors[Status_Disabled] = D2D1::ColorF(0xDEDEDEDE);
    colors[Status_Normal] = D2D1::ColorF(0xCDCDCDCD);
    colors[Status_Hover] = D2D1::ColorF(0xA9A9A9A9);
    colors[Status_Pushed] = D2D1::ColorF(0x78787878);
    // 无效?
    UIElements_Prefix;
    // 禁用状态颜色
    UIElements_NewAttribute("disabledcolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Disabled]);
    // 通常状态颜色
    UIElements_NewAttribute("normalcolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Normal]);
    // 移上状态颜色
    UIElements_NewAttribute("hovercolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Hover]);
    // 按下状态颜色
    UIElements_NewAttribute("pushedcolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Pushed]);
}

// Elements<ColorRect> 重建
auto LongUI::Component::Elements<LongUI::Element::ColorRect>::
Recreate(LongUIRenderTarget* target) noexcept ->HRESULT {
    ::SafeRelease(m_pBrush);
    m_pBrush = static_cast<ID2D1SolidColorBrush*>(UIManager.GetBrush(LongUICommonSolidColorBrushIndex));
    return S_OK;
}

// Elements<ColorRect> 渲染
void LongUI::Component::Elements<LongUI::Element::ColorRect>::Render(const D2D1_RECT_F& rect) noexcept {
    assert(m_pRenderTarget && m_pBrush);
    // 先绘制当前状态
    if (animation.value < animation.end) {
        m_pBrush->SetColor(colors + m_state);
        m_pRenderTarget->FillRectangle(&rect, m_pBrush);
    }
    // 再绘制目标状态
    m_pBrush->SetOpacity(animation.value);
    m_pBrush->SetColor(colors + m_stateTartget);
    m_pRenderTarget->FillRectangle(&rect, m_pBrush);
    m_pBrush->SetOpacity(1.f);
}