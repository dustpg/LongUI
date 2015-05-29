
#include "LongUI.h"



// Render 渲染 
auto LongUI::UISlider::Render() noexcept ->HRESULT {
    if (m_bDrawSizeChanged) {
        this->draw_zone = this->show_zone;
    }
    D2D1_RECT_F draw_rect = GetDrawRect(this);
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
        // 根据 value 计算滑块位置
        m_rcSlider.top = this->show_zone.top;
        m_rcSlider.bottom = m_rcSlider.top + this->show_zone.height;
        m_rcSlider.left = this->show_zone.left + ((draw_rect.right - draw_rect.left) * m_fValue);
        m_rcSlider.right = m_rcSlider.left + m_fSliderHalfWidth * 2.f;
        // 渲染滑块
        m_pRenderTarget->FillRectangle(m_rcSlider, m_pBrush_SetBeforeUse);
    }
    return S_OK;
}


// UISlider 构造函数
LongUI::UISlider::UISlider(pugi::xml_node node) noexcept: Super(node)
{
    static_assert(UNUSED_SIZE <= lengthof(m_unused), "unused size!");
}

// UISlider::CreateControl 函数
LongUI::UIControl* LongUI::UISlider::CreateControl(pugi::xml_node node) noexcept {
    if (!node) {
        UIManager << DL_Warning << L"node null" << LongUI::endl;
    }
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UISlider>(
        node,
        [=](void* p) noexcept { new(p) UISlider(node);}
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}



// do event 事件处理
bool LongUIMethodCall LongUI::UISlider::DoEvent(LongUI::EventArgument& arg) noexcept {
    if (arg.sender){
        switch (arg.event)
        {
        case LongUI::Event::Event_FindControl:
            if (IsPointInRect(this->show_zone, arg.pt)){
                arg.ctrl = this;
            }
            __fallthrough;
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
                m_unused[Unused_MouseClickIn] = true;
            }
            break;
        case WM_MOUSEMOVE:
            if (m_unused[Unused_MouseClickIn] && arg.wParam_sys & MK_LBUTTON){
                m_fValue = (arg.pt.x - this->show_zone.left) / this->show_zone.width;
                if (m_fValue > 1.f) m_fValue = 1.f;
                if (m_fValue < 0.f) m_fValue = 0.f;

            }
            break;
        case WM_LBUTTONUP:
            m_unused[Unused_MouseClickIn] = false;
            m_pWindow->ReleaseCapture();
            break;
        }
        // 检查事件
        if (m_fValueOld != m_fValue) {
            m_fValueOld = m_fValue;
            auto tempmsg = arg.msg;
            arg.sender = this;
            arg.event = LongUI::Event::Event_SliderValueChanged;
            // 检查脚本
            if (m_pScript && m_script.data) {
                m_pScript->Evaluation(m_script, arg);
            }
            // 检查是否有事件回调
            if (m_eventChanged) {
                (m_pChangedTarget->*m_eventChanged)(this);
            }
            else {
                // 否则发送事件到窗口
                m_pWindow->DoEvent(arg);
            }
            arg.msg = tempmsg;
            // 刷新
            m_pWindow->Invalidate(this);
        }
    }
    return false;
}

// recreate 重建
/*HRESULT LongUIMethodCall LongUI::UISlider::Recreate(LongUIRenderTarget* newRT) noexcept {
    ::SafeRelease(m_pBrush);
    // 设置新的笔刷
    m_pBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    return Super::Recreate(newRT);
}*/

// close this control 关闭控件
void LongUIMethodCall LongUI::UISlider::Close() noexcept {
    delete this;
}