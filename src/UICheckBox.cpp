#include "LongUI.h"



// Render 渲染 
void LongUI::UICheckBox::Render() const noexcept  {
    /*D2D1_RECT_F draw_rect = this->GetDrawRect();;
    draw_rect.left += 1.f;
    // 计算渲染区
    draw_rect.top = (draw_rect.bottom + draw_rect.top - m_szCheckBox.height) * 0.5f;
    draw_rect.bottom = draw_rect.top + m_szCheckBox.height;
    draw_rect.right = draw_rect.left + m_szCheckBox.width;
    // 渲染框
    UIManager_RenderTarget->DrawRectangle(
        draw_rect, m_pBrush, 1.5f
        );
    // 渲染箭头
    switch (this->state)
    {
    case CheckboxState::State_Checked:
        D2D1_MATRIX_3X2_F matrix;
        UIManager_RenderTarget->GetTransform(&matrix);
        UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Translation(draw_rect.left, draw_rect.bottom) * matrix);
        UIManager_RenderTarget->FillGeometry(m_pCheckedGeometry, m_pBrush);
        UIManager_RenderTarget->SetTransform(&matrix);
        break;
    case CheckboxState::State_Indeterminate:
        // 收缩范围
        draw_rect.left += m_szCheckBox.width * 0.2f;
        draw_rect.right -= m_szCheckBox.width * 0.2f;
        draw_rect.top += m_szCheckBox.height * 0.2f;
        draw_rect.bottom -= m_szCheckBox.height * 0.2f;
        // 渲染框
        UIManager_RenderTarget->FillRectangle(draw_rect, m_pBrush);
        break;
    }
    // 调节文本范围 +
    //this->show_zone.left += m_szCheckBox.width;
    // 刻画文本
    Super::Render(RenderType::Type_Render);
    // 调节文本范围 -
    //this->show_zone.left -= m_szCheckBox.width;
    return S_OK;*/
    return Super::Render();
}

// UI检查框: 刷新
void LongUI::UICheckBox::Update() noexcept {
    return Super::Update();
}

// UICheckBox 构造函数
LongUI::UICheckBox::UICheckBox(UIContainer* cp, pugi::xml_node node) 
    noexcept: Super(cp, node), m_uiElement(node) {
}

// UICheckBox 析构函数
LongUI::UICheckBox::~UICheckBox() noexcept {

}


// UICheckBox::CreateControl 函数
auto LongUI::UICheckBox::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        // 警告
        if (!node) {
            UIManager << DL_Hint << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = CreateWidthCET<LongUI::UICheckBox>(type, node);
        // OOM
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UICheckBox::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI消息
    /*if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        }
    }*/
    return Super::DoEvent(arg);
}


// do mouse event 鼠标事件处理
bool LongUI::UICheckBox::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        m_pWindow->now_cursor = m_hCursorHand;
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        m_pWindow->now_cursor = m_pWindow->default_cursor;
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 有效
        if (arg.pt.x < this->view_size.width && arg.pt.y) {
            // 检查flag
            auto target = CheckBoxState::State_Checked;
            if (this->GetCheckBoxState() == CheckBoxState::State_Checked) {
                target = CheckBoxState::State_Unchecked;
            }
            this->SetCheckBoxState(target);
        }
        break;
    }
    return true;
}

// recreate 重建
auto LongUI::UICheckBox::Recreate() noexcept ->HRESULT {
    // 有效
    m_uiElement.Recreate();
    // 父类处理
    return Super::Recreate();
}

// 关闭控件
void LongUI::UICheckBox::cleanup() noexcept {
    delete this;
}

// -------------------------------------------------------
// 复选框图像接口 -- 构造函数
LongUI::UICheckBox::GICheckBox::GICheckBox(pugi::xml_node node, const char* prefix) noexcept {
    Component::GIConfigButton::InitColor(node, prefix, colors);
}


// 复选框图像接口 -- 渲染
void LongUI::UICheckBox::GICheckBox::Render(const D2D1_RECT_F& rect, const Component::AnimationStateMachine& sm) const noexcept {
    assert(UIManager_RenderTarget);
    auto brush = static_cast<ID2D1SolidColorBrush*>(UIManager.GetBrush(LongUICommonSolidColorBrushIndex));
    UIManager_RenderTarget->FillRectangle(rect, brush);
    LongUI::SafeRelease(brush);
}
