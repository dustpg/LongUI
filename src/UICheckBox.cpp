#include "LongUI.h"



// Render 渲染 
void LongUI::UICheckBox::Render(RenderType type) const noexcept  {
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
    case CheckBoxState::State_Checked:
        D2D1_MATRIX_3X2_F matrix;
        UIManager_RenderTarget->GetTransform(&matrix);
        UIManager_RenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(draw_rect.left, draw_rect.bottom) * matrix);
        UIManager_RenderTarget->FillGeometry(m_pCheckedGeometry, m_pBrush);
        UIManager_RenderTarget->SetTransform(&matrix);
        break;
    case CheckBoxState::State_Indeterminate:
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
    return Super::Render(type);
}

// UI检查框: 刷新
void LongUI::UICheckBox::Update() noexcept {
    return Super::Update();
}

// UICheckBox 构造函数
LongUI::UICheckBox::UICheckBox(pugi::xml_node node) noexcept: Super(node) {
    // √ symbol
    auto format = UIManager.GetTextFormat(LongUIDefaultTextFormatIndex);
    char32_t chars = U'√';
    LongUI::DX::CreateTextPathGeometry(
        &chars, 1,
        format,
        UIManager_D2DFactory,
        nullptr,
        &m_pCheckedGeometry
        );
    ::SafeRelease(format);
}

// UICheckBox 析构函数
LongUI::UICheckBox::~UICheckBox() noexcept {
    ::SafeRelease(m_pCheckedGeometry);
    ::SafeRelease(m_pBrush);
}


// UICheckBox::CreateControl 函数
auto LongUI::UICheckBox::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UICheckBox>(
            node,
            [=](void* p) noexcept { new(p) UICheckBox(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
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
            if (this->IsCanbeIndeterminate()) {
                if (this->state == CheckBoxState::State_UnChecked) {
                    force_cast(this->state) = CheckBoxState::State_Checked;
                }
                else if (this->state == CheckBoxState::State_Checked) {
                    force_cast(this->state) = CheckBoxState::State_Indeterminate;
                }
                else {
                    force_cast(this->state) = CheckBoxState::State_UnChecked;
                }
            }
            else {
                force_cast(this->state) = static_cast<decltype(this->state)>
                    (!static_cast<uint32_t>(this->state));
            }
            m_pWindow->Invalidate(this);
        }
        break;
    }
    return true;
}

// recreate 重建
auto LongUI::UICheckBox::Recreate() noexcept ->HRESULT {
    // 有效
    ::SafeRelease(m_pBrush);
    m_pBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    // 父类处理
    return Super::Recreate();
}

// 关闭控件
void LongUI::UICheckBox::Cleanup() noexcept {
    delete this;
}