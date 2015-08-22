#include "LongUI.h"


// Render 渲染 
void LongUI::UISlider::Render(RenderType type) const noexcept {
    //D2D1_RECT_F draw_rect;
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 默认背景?
        if(m_bDefaultBK) {
            constexpr float SLIDER_TRACK_BORDER_WIDTH = 1.f;
            constexpr float SLIDER_TRACK_WIDTH = 3.f;
            constexpr UINT SLIDER_TRACK_BORDER_COLOR = 0xD6D6D6;
            constexpr UINT SLIDER_TRACK_COLOR = 0xE7EAEA;
            D2D1_RECT_F border_rect; this->GetViewRect(border_rect);
            // 垂直滑块
            if (this->IsVerticalSlider()) {
                auto half = this->thumb_size.height * 0.5f;
                border_rect.left = (border_rect.left + border_rect.right) * 0.5f -
                    SLIDER_TRACK_BORDER_WIDTH - SLIDER_TRACK_WIDTH * 0.5f;
                border_rect.right = border_rect.left + 
                    SLIDER_TRACK_BORDER_WIDTH * 2.f + SLIDER_TRACK_WIDTH;
                border_rect.top += half;
                border_rect.bottom -= half;
            }
            // 水平滑块
            else {
                auto half = this->thumb_size.width * 0.5f;
                border_rect.left += half;
                border_rect.right -= half;
                border_rect.top = (border_rect.top + border_rect.bottom) * 0.5f -
                    SLIDER_TRACK_BORDER_WIDTH - SLIDER_TRACK_WIDTH * 0.5f;
                border_rect.bottom = border_rect.top + 
                    SLIDER_TRACK_BORDER_WIDTH * 2.f + SLIDER_TRACK_WIDTH;
            }
            // 渲染滑槽边框
            m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(SLIDER_TRACK_BORDER_COLOR));
            m_pRenderTarget->FillRectangle(&border_rect, m_pBrush_SetBeforeUse);
            // 渲染滑槽
            m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(SLIDER_TRACK_COLOR));
            border_rect.left += SLIDER_TRACK_BORDER_WIDTH;
            border_rect.top += SLIDER_TRACK_BORDER_WIDTH;
            border_rect.right -= SLIDER_TRACK_BORDER_WIDTH;
            border_rect.bottom -= SLIDER_TRACK_BORDER_WIDTH;
            m_pRenderTarget->FillRectangle(&border_rect, m_pBrush_SetBeforeUse);
        }
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        __fallthrough;
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
    case LongUI::RenderType::Type_RenderForeground:
        m_uiElement.Render(m_rcThumb);
        // 边框
        {
            constexpr float THUMB_BORDER_WIDTH = 1.f;
            D2D1_RECT_F thumb_border = {
                m_rcThumb.left + THUMB_BORDER_WIDTH * 0.5f,
                m_rcThumb.top + THUMB_BORDER_WIDTH * 0.5f,
                m_rcThumb.right - THUMB_BORDER_WIDTH * 0.5f,
                m_rcThumb.bottom - THUMB_BORDER_WIDTH * 0.5f,
            };
            m_pBrush_SetBeforeUse->SetColor(&m_colorBorderNow);
            m_pRenderTarget->DrawRectangle(&thumb_border, m_pBrush_SetBeforeUse);
        }
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}


// UI滑动条: 刷新
void LongUI::UISlider::Update() noexcept {
    // 更新计时器
    m_uiElement.Update();
    // 垂直滑块
    if (this->IsVerticalSlider()) {
        // 根据 value 计算滑块位置
        m_rcThumb.left = (this->view_size.width - this->thumb_size.width) *0.5f;
        m_rcThumb.right = m_rcThumb.left + this->thumb_size.width;
        {
            auto slider_height = this->view_size.height - this->thumb_size.height;
            m_rcThumb.top = slider_height * m_fValue;
        }
        m_rcThumb.bottom = m_rcThumb.top + this->thumb_size.height;
    }
    // 水平滑块
    else {
        // 根据 value 计算滑块位置
        m_rcThumb.top = (this->view_size.height - this->thumb_size.height) *0.5f;
        m_rcThumb.bottom = m_rcThumb.top + this->thumb_size.height;
        {
            auto slider_width = this->view_size.width - this->thumb_size.width;
            m_rcThumb.left = slider_width * m_fValue;
        }
        m_rcThumb.right = m_rcThumb.left + this->thumb_size.width;
    }
}

// UISlider 构造函数
LongUI::UISlider::UISlider(pugi::xml_node node) noexcept: Super(node), m_uiElement(node) {
    // 设置
    if (node) {
        const char* str = nullptr;
        // 起始值
        if ((str = node.attribute("start").value())) {
            m_fStart = LongUI::AtoF(str);
        }
        // 终止值
        if ((str = node.attribute("end").value())) {
            m_fEnd = LongUI::AtoF(str);
        }
        // 滑块大小
        Helper::MakeFloats(
            node.attribute("thumbsize").value(),
            &force_cast(thumb_size.width), 
            sizeof(thumb_size)/sizeof(thumb_size.width)
            );
        // 默认背景
        m_bDefaultBK = node.attribute("defaultbk").as_bool(true);
    }
    // 初始化代码
    m_uiElement.GetByType<Element_Basic>().Init(node);
    if (m_uiElement.GetByType<Element_Meta>().IsOK()) {
        m_uiElement.SetElementType(Element_Meta);
    }
    else {
        m_uiElement.SetElementType(Element_BrushRect);
    }
    // init
    m_uiElement.GetByType<Element_Basic>().SetNewStatus(Status_Normal);
    // need twices because of aniamtion
    m_uiElement.GetByType<Element_Basic>().SetNewStatus(Status_Normal);
}

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
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    //--------------------------------------------------
    if (arg.sender){
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl:
            if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(pt4self.x < this->width && pt4self.y < this->width && "check it");
                arg.ctrl = this;
            }
            __fallthrough;*/
        case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        case LongUI::Event::Event_MouseEnter:
            break;
        case LongUI::Event::Event_MouseLeave:
            // 鼠标移出: 设置UI元素状态
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Normal);
            m_bMouseClickIn = false;
            m_bMouseMoveIn = false;
            break;
        }
    }
    else{
        switch (arg.msg)
        {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            if (IsPointInRect(m_rcThumb, pt4self)){
                m_bMouseClickIn = true;
                m_fClickPosition = this->IsVerticalSlider() ?
                    (pt4self.y - m_rcThumb.top) : (pt4self.x - m_rcThumb.left);
            }
            break;
        case WM_MOUSEMOVE:
            // 点中并且移动
            if (arg.sys.wParam & MK_LBUTTON) {
                if (m_bMouseClickIn) {
                    // 获取基本值
                    if (this->IsVerticalSlider()) {
                        auto slider_height = this->view_size.height - this->thumb_size.height;
                        m_fValue = (pt4self.y - m_fClickPosition) / slider_height;
                    }
                    else {
                        auto slider_width = this->view_size.width - this->thumb_size.width;
                        m_fValue = (pt4self.x - m_fClickPosition) / slider_width;
                    }
                    // 阈值检查
                    if (m_fValue > 1.f) m_fValue = 1.f;
                    else if (m_fValue < 0.f) m_fValue = 0.f;
                }
            }
            // 移动
            else {
                if (IsPointInRect(m_rcThumb, pt4self)){
                    // 鼠标移进:
                    if (!m_bMouseMoveIn) {
                        // 设置UI元素状态
                        UIElement_SetNewStatus(m_uiElement, LongUI::Status_Hover);
                        m_bMouseMoveIn = true;
                    }
                }
                else {
                    // 鼠标移出:
                    if (m_bMouseMoveIn) {
                        // 设置UI元素状态
                        UIElement_SetNewStatus(m_uiElement, LongUI::Status_Normal);
                        m_bMouseMoveIn = false;
                    }
                }
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
            // 调用
            m_caller(this, SubEvent::Event_SliderValueChanged);
            // 刷新
            m_pWindow->Invalidate(this);
        }
    }
    return false;
}

// recreate 重建
auto LongUI::UISlider::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    m_uiElement.Recreate(newRT);
    return Super::Recreate(newRT);
}

// close this control 关闭控件
void LongUI::UISlider::Cleanup() noexcept {
    delete this;
}