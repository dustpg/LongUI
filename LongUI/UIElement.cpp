#include "LongUI.h"

// 实现

// Elements<Basic> Init
void LongUI::Component::Elements<LongUI::Element::Basic>::
Init(pugi::xml_node node, const char* prefix) noexcept {
    // 无效?
    if (!node) return; const char* str = nullptr;
    // 动画类型
    if (str = node.attribute("animationtype").value()) {
        m_animation.type = static_cast<AnimationType>(LongUI::AtoI(str));
    }
    // 动画持续时间
    if (str = node.attribute("animationduration").value()) {
        m_animation.duration = LongUI::AtoF(str);
    }
}

// 设置新的状态
auto LongUI::Component::Elements<LongUI::Element::Basic>::
SetNewStatus(LongUI::ControlStatus new_status) noexcept ->float {
    m_state = m_stateTartget;
    m_stateTartget = new_status;
    m_animation.value = 0.f;
    return m_animation.time = m_animation.duration;
}

// Elements<Meta> 构造函数
LongUI::Component::Elements<LongUI::Element::Meta>::
Elements(pugi::xml_node node, const char* prefix) noexcept: Super(node, prefix) {
    ZeroMemory(m_metas, sizeof(m_metas));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 无效?
    if (!node) return;
    // 禁用状态Meta ID
    m_aID[Status_Disabled] = LongUI::AtoI(node.attribute("disabledmeta").value());
    // 通常状态Meta ID
    m_aID[Status_Normal] = LongUI::AtoI(node.attribute("normalmeta").value());
    // 移上状态Meta ID
    m_aID[Status_Hover] = LongUI::AtoI(node.attribute("hovermeta").value());
    // 按下状态Meta ID
    m_aID[Status_Pushed] = LongUI::AtoI(node.attribute("pushedmeta").value());
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
    if (m_animation.value < m_animation.end) {
        auto meta = m_metas[m_state];
        assert(meta.bitmap);
        m_pRenderTarget->DrawBitmap(
            meta.bitmap,
            rect, 1.f,
            static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
            meta.src_rect,
            nullptr
            );
    }
    // 再绘制目标状态
    auto meta = m_metas[m_stateTartget];
    assert(meta.bitmap);
    m_pRenderTarget->DrawBitmap(
        meta.bitmap,
        rect, m_animation.value,
        static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
        meta.src_rect,
        nullptr
        );
}



// Elements<BrushRect> 构造函数
LongUI::Component::Elements<LongUI::Element::BrushRect>::
Elements(pugi::xml_node node, const char* prefix) noexcept :Super(node, prefix) {
    ZeroMemory(m_apBrushes, sizeof(m_apBrushes));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 无效?
    if (!node) return;
    // 禁用状态笔刷ID
    m_aID[Status_Disabled] = LongUI::AtoI(node.attribute("disabledbrush").value());
    // 通常状态笔刷ID
    m_aID[Status_Normal] = LongUI::AtoI(node.attribute("normalbrush").value());
    // 移上状态笔刷ID
    m_aID[Status_Hover] = LongUI::AtoI(node.attribute("hoverbrush").value());
    // 按下状态笔刷ID
    m_aID[Status_Pushed] = LongUI::AtoI(node.attribute("pushedbrush").value());
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
    if (m_animation.value < m_animation.end) {
        m_pRenderTarget->FillRectangle(rect2, m_apBrushes[m_state]);
    }
    // 后绘制目标状态
    auto brush = m_apBrushes[m_stateTartget];
    brush->SetOpacity(m_animation.value);
    m_pRenderTarget->FillRectangle(rect2, brush);
    brush->SetOpacity(1.f);
    m_pRenderTarget->SetTransform(&matrix);
#else
    //m_pRenderTarget->SetTransform(D2D1::IdentityMatrix());
    // 先绘制当前状态
    if (m_animation.value < m_animation.end) {
        m_apBrushes[m_state]->SetTransform(&matrix);
        m_pRenderTarget->FillRectangle(rect, m_apBrushes[m_state]);
    }
    // 后绘制目标状态
    auto brush = m_apBrushes[m_stateTartget];
    brush->SetOpacity(m_animation.value);
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
    m_aColor[Status_Disabled] = D2D1::ColorF(0xDEDEDEDE);
    m_aColor[Status_Normal] = D2D1::ColorF(0xCDCDCDCD);
    m_aColor[Status_Hover] = D2D1::ColorF(0xA9A9A9A9);
    m_aColor[Status_Pushed] = D2D1::ColorF(0x98989898);
    // 无效?
    if (!node) return;
    // 禁用状态颜色
    UIControl::MakeColor(node.attribute("disabledcolor").value(), m_aColor[Status_Disabled]);
    // 通常状态颜色
    UIControl::MakeColor(node.attribute("normalcolor").value(), m_aColor[Status_Normal]);
    // 移上状态颜色
    UIControl::MakeColor(node.attribute("hovercolor").value(), m_aColor[Status_Hover]);
    // 按下状态颜色
    UIControl::MakeColor(node.attribute("pushedcolor").value(), m_aColor[Status_Pushed]);
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
    if (m_animation.value < m_animation.end) {
        m_pBrush->SetColor(m_aColor + m_state);
        m_pRenderTarget->FillRectangle(&rect, m_pBrush);
    }
    // 再绘制目标状态
    m_pBrush->SetOpacity(m_animation.value);
    m_pBrush->SetColor(m_aColor + m_stateTartget);
    m_pRenderTarget->FillRectangle(&rect, m_pBrush);
    m_pBrush->SetOpacity(1.f);
}