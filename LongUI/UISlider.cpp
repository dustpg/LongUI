#include "LongUI.h"

// Render 渲染 
void LongUI::UISlider::Render(RenderType) const noexcept {
    D2D1_RECT_F draw_rect; this->GetContentRect(draw_rect);
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
    // 垂直滑块
    if (this->flags & Flag_Slider_VerticalSlider){

    }
    // 水平滑块
    else{
        draw_rect.left += m_fSliderHalfWidth;
        draw_rect.right -= m_fSliderHalfWidth;
        draw_rect.top = (draw_rect.top + draw_rect.bottom)*0.5f - 2.f;
        draw_rect.bottom = draw_rect.top + 2.f;
        // 渲染滑槽
        m_pRenderTarget->FillRectangle(draw_rect, m_pBrush_SetBeforeUse);
        // 渲染滑块
        m_pRenderTarget->FillRectangle(m_rcSlider, m_pBrush_SetBeforeUse);
    }
}


// UI滑动条: 刷新
void LongUI::UISlider::Update() noexcept {
    // 垂直滑块
    if (this->flags & Flag_Slider_VerticalSlider) {

    }
    // 水平滑块
    else {
        // 根据 value 计算滑块位置
        m_rcSlider.top = 0.f;
        m_rcSlider.bottom = m_rcSlider.top + this->height;
        m_rcSlider.left = this->width * m_fValue;
        m_rcSlider.right = m_rcSlider.left + m_fSliderHalfWidth * 2.f;
    }
}

// UISlider 构造函数
inline LongUI::UISlider::UISlider(pugi::xml_node node) noexcept: Super(node) { }

// UISlider::CreateControl 函数
LongUI::UIControl* LongUI::UISlider::CreateControl(CreateEventType type, pugi::xml_node node) noexcept {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UISlider>(
            node,
            [=](void* p) noexcept { new(p) UISlider(node); }
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
bool LongUI::UISlider::DoEvent(const LongUI::EventArgument& arg) noexcept {
    if (arg.sender){
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl:
            if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
                arg.ctrl = this;
            }
            __fallthrough;*/
        case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        }
    }
    else{
        switch (arg.msg)
        {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            if (IsPointInRect(m_rcSlider, arg.pt)){
                m_bMouseClickIn = true;
            }
            break;
        case WM_MOUSEMOVE:
            if (m_bMouseClickIn && arg.wParam_sys & MK_LBUTTON){
                m_fValue = (arg.pt.x) / this->width;
                if (m_fValue > 1.f) m_fValue = 1.f;
                if (m_fValue < 0.f) m_fValue = 0.f;

            }
            break;
        case WM_LBUTTONUP:
            m_bMouseClickIn = false;
            m_pWindow->ReleaseCapture();
            break;
        }
        // 检查事件
        if (m_fValueOld != m_fValue) {
            m_fValueOld = m_fValue;
            auto temparg = arg;
            temparg.sender = this;
            temparg.event = LongUI::Event::Event_SliderValueChanged;
            // 检查脚本
            if (m_pScript && m_script.data) {
                m_pScript->Evaluation(m_script, temparg);
            }
            // 检查是否有事件回调
            if (m_eventChanged) {
                (m_pChangedTarget->*m_eventChanged)(this);
            }
            else {
                // 否则发送事件到窗口
                m_pWindow->DoEvent(temparg);
            }
            // 刷新
            m_pWindow->Invalidate(this);
        }
    }
    return false;
}

// recreate 重建
/*HRESULT LongUI::UISlider::Recreate(LongUIRenderTarget* newRT) noexcept {
    ::SafeRelease(m_pBrush);
    // 设置新的笔刷
    m_pBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    return Super::Recreate(newRT);
}*/

// close this control 关闭控件
void LongUI::UISlider::WindUp() noexcept {
    delete this;
}