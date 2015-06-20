#include "LongUI.h"


// UIScrollBar 构造函数
inline LongUI::UIScrollBar::
UIScrollBar(pugi::xml_node node) noexcept: Super(node) {

}


// 更新值
void LongUI::UIScrollBar::Refresh() noexcept {
    // 边界 > 显示  -> 刻画边界 = 边界
    // 另外:      -> 刻画边界 = 显示
    bool old = false;
    UIManager << DL_Hint << "called" << endl;
    // 垂直?
    if (this->type == ScrollBarType::Type_Vertical) {
        // 更新
        if ((old = m_pOwner->end_of_bottom > m_pOwner->height)) {
            m_pOwner->height = m_pOwner->end_of_bottom;
        }
        m_fMaxRange = m_pOwner->height;
        m_fMaxIndex = m_fMaxRange - m_pOwner->height;
        // 检查上边界

        // 检查下边界
        /*auto lower = m_pOwner->show_zone.top + m_pOwner->show_zone.height;
        if (m_pOwner->draw_zone.top + m_pOwner->draw_zone.height < lower) {
            m_pOwner->draw_zone.top = lower - m_pOwner->draw_zone.height;
        }*/
    }
    // 水平?
    else {
        // 更新
        if ((old = m_pOwner->end_of_right > m_pOwner->width)) {
            m_pOwner->width = m_pOwner->end_of_right;
        }
        m_fMaxRange = m_pOwner->width;
        m_fMaxIndex = m_fMaxRange - m_pOwner->width;
        // 检查左边界

        // 检查右边界
        /*auto right = m_pOwner->show_zone.left + m_pOwner->show_zone.width;
        if (m_pOwner->draw_zone.left + m_pOwner->draw_zone.width < right) {
            m_pOwner->draw_zone.left = right - m_pOwner->draw_zone.width;
        }*/
    }
    // TODO: 更新滚动条状态
}

// 设置新的索引位置
void LongUI::UIScrollBar::SetIndex(float new_index) noexcept {
    new_index = std::min(std::max(new_index, 0.f), m_fMaxIndex);
    // 不同就修改
    if (new_index != m_fIndex) {
        m_fIndex = new_index;
#if 1
        if (this->type == ScrollBarType::Type_Vertical) {
            m_pOwner->y_offset = -new_index;
            //this->show_zone.top = new_index;
        }
        else {
            m_pOwner->x_offset = -new_index;
            //this->show_zone.left = new_index;
        }
        //this->draw_zone = this->show_zone;
        m_pOwner->DrawPosChanged();
        // 刷新拥有着
        m_pWindow->Invalidate(m_pOwner);
#else
        m_pWindow->Invalidate(this);
#endif
    }
}



// do event 事件处理
bool  LongUI::UIScrollBar::DoEvent(LongUI::EventArgument& arg) noexcept {
    // 控件消息
    if (arg.sender) {
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl:
            if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
                arg.ctrl = this;
            }
            __fallthrough;*/
            /*case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        case LongUI::Event::Event_MouseEnter:
            (L"<%S>: MouseEnter\n", __FUNCTION__);
            break;*/
            break;
        case LongUI::Event::Event_MouseLeave:
            m_pointType = PointType::Type_None;
            m_lastPointType = PointType::Type_None;
            return true;
        }
    }
    // 系统消息
    else {
        // 鼠标移上

    }
    return false;
}


// UIScrollBarA 构造函数
LongUI::UIScrollBarA::UIScrollBarA(pugi::xml_node node) noexcept: Super(node), 
m_uiArrow1(node, "arrow1"), m_uiArrow2(node, "arrow2"), m_uiThumb(node, "thumb"){
    // 修改颜色
    if (!node) {
        D2D1_COLOR_F normal_color = D2D1::ColorF(0xF0F0F0);
        m_uiArrow1.GetByType<Element::ColorRect>().colors[Status_Normal] = normal_color;
        m_uiArrow2.GetByType<Element::ColorRect>().colors[Status_Normal] = normal_color;
        normal_color = D2D1::ColorF(0x2F2F2F);
        m_uiArrow1.GetByType<Element::ColorRect>().colors[Status_Pushed] = normal_color;
        m_uiArrow2.GetByType<Element::ColorRect>().colors[Status_Pushed] = normal_color;
    }
    // 初始化代码
    m_uiArrow1.GetByType<Element::Basic>().Init(node, "arrow1");
    m_uiArrow2.GetByType<Element::Basic>().Init(node, "arrow2");
    m_uiThumb.GetByType<Element::Basic>().Init(node, "thumb");
    // 检查
    BarElement* elements[] = { &m_uiArrow1, &m_uiArrow2, &m_uiThumb };
    for (auto element : elements) {
        if (element->GetByType<Element::Meta>().IsOK()) {
            element->SetElementType(Element::Meta);
        }
        else {
            element->SetElementType(Element::ColorRect);
        }
        element->GetByType<Element::Basic>().SetNewStatus(Status_Normal);
        element->GetByType<Element::Basic>().SetNewStatus(Status_Normal);
    }
    // 检查属性
    m_bArrow1InColor = m_uiArrow1.GetByType<Element::Basic>().type == Element::ColorRect;
    m_bArrow2InColor = m_uiArrow2.GetByType<Element::Basic>().type == Element::ColorRect;

}
#define LONGUI_FLOAT_OFFSET()


// UIScrollBarA 渲染 
auto LongUI::UIScrollBarA::Render(RenderType type) noexcept -> HRESULT {
    if (type != RenderType::Type_Render) return S_FALSE;
    // 更新
    D2D1_RECT_F draw_rect = this->GetDrawRect();
    // 双滚动条修正
    if (this->another) {
        if (this->type == ScrollBarType::Type_Vertical) {
            draw_rect.bottom -= this->another->GetTakingUpSapce();
        }
        else {
            draw_rect.right -= this->another->GetTakingUpSapce();
        }
    }
    m_rtThumb = m_rtArrow2 = m_rtArrow1 = draw_rect;
    register auto bilibili = 1.f - m_fMaxIndex / m_fMaxRange;
    // TODO: 合并
    // 垂直滚动条
    if (this->type == ScrollBarType::Type_Vertical) {
        m_rtArrow1.bottom = m_rtArrow1.top + BASIC_SIZE;
        m_rtArrow2.top = m_rtArrow2.bottom - BASIC_SIZE;
        // 计算Thumb
        register auto height = m_pOwner->height - BASIC_SIZE*2.f;
        m_rtThumb.top = (m_fIndex * bilibili + m_rtArrow1.bottom);
        m_rtThumb.bottom = (m_rtThumb.top + bilibili * height) - 1.f;
    }
    // 水平滚动条
    else {
        m_rtArrow1.right = m_rtArrow1.left + BASIC_SIZE;
        m_rtArrow2.left = m_rtArrow2.right - BASIC_SIZE;
        // 计算Thumb
        register auto width = m_pOwner->width - BASIC_SIZE*2.f;
        m_rtThumb.left = m_fIndex * bilibili + m_rtArrow1.right;
        m_rtThumb.right = m_rtThumb.left + bilibili * width - 1.f;
    }
    // 基本背景: Shaft
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(0xF0F0F0));
    m_pRenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
    //
    //this->parent;
    //UIManager << DL_Hint << m_rtArrow2 << endl;

    // 渲染部件
    m_uiArrow1.Render(m_rtArrow1);
    UIElement_Update(m_uiArrow1);
    m_uiArrow2.Render(m_rtArrow2);
    UIElement_Update(m_uiArrow2);
    m_uiThumb.Render(m_rtThumb);
    UIElement_Update(m_uiThumb);
    //
    //UIManager << DL_Hint << m_rtThumb << endl;
    // 前景
    auto render_geo = [](ID2D1RenderTarget* target, ID2D1Brush* bush, ID2D1Geometry* geo, D2D1_RECT_F& rect) {
        D2D1_MATRIX_3X2_F matrix; target->GetTransform(&matrix);
        target->SetTransform(
            D2D1::Matrix3x2F::Translation(rect.left, rect.top) * matrix
            );
        target->DrawGeometry(geo, bush, 2.33f);
        // 修改
        target->SetTransform(&matrix);
    };
    // 渲染几何体
    if (m_bArrow1InColor) {
        D2D1_COLOR_F tcolor = m_uiArrow1.GetByType<Element::ColorRect>().colors[
            m_uiArrow1.GetByType<Element::Basic>().GetStatus()
        ];
        tcolor.r = 1.f - tcolor.r; tcolor.g = 1.f - tcolor.g; tcolor.b = 1.f - tcolor.b;
        m_pBrush_SetBeforeUse->SetColor(&tcolor);
        render_geo(m_pRenderTarget, m_pBrush_SetBeforeUse, m_pArrow1Geo, m_rtArrow1);
    }
    // 渲染几何体
    if (m_bArrow2InColor) {
        D2D1_COLOR_F tcolor = m_uiArrow2.GetByType<Element::ColorRect>().colors[
            m_uiArrow2.GetByType<Element::Basic>().GetStatus()
        ];
        tcolor.r = 1.f - tcolor.r; tcolor.g = 1.f - tcolor.g; tcolor.b = 1.f - tcolor.b;
        m_pBrush_SetBeforeUse->SetColor(&tcolor);
        render_geo(m_pRenderTarget, m_pBrush_SetBeforeUse, m_pArrow2Geo, m_rtArrow2);
    }
    // 前景
    Super::Render(RenderType::Type_RenderForeground);
    return S_OK;
}


// UIScrollBarA::do event 事件处理
bool  LongUI::UIScrollBarA::DoEvent(LongUI::EventArgument& arg) noexcept {
    // 获取点击
    auto get_real_pos = [this](float pos)  {
        pos -= UIScrollBarA::BASIC_SIZE ;
        auto length = this->get_length() - UIScrollBarA::BASIC_SIZE * 2.f;
        return (pos) / length * m_fMaxRange;
    };
    // 控件消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_MouseLeave:
            this->set_status(m_lastPointType, LongUI::Status_Normal);
            break;
        }
    }
    // 系统消息
    else {
        switch (arg.msg) {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            // 记录点击点
            m_bCaptured = true;
            m_fOldPoint = get_real_pos(
                this->type == ScrollBarType::Type_Vertical ? arg.pt.y : arg.pt.x
                );
            this->set_status(m_pointType, LongUI::Status_Pushed);
            // 检查
            if (m_pointType == PointType::Type_Arrow1) {
                // 左/上移动
                this->SetIndex(m_fIndex - m_fStep *0.25f);
            }
            else if (m_pointType == PointType::Type_Arrow2) {
                // 左/上移动
                this->SetIndex(m_fIndex + m_fStep*0.25f);
            }
            break;
        case WM_LBUTTONUP:
            this->set_status(m_pointType, LongUI::Status_Hover);
            m_bCaptured = false;
            m_pWindow->ReleaseCapture();
            break;
        case WM_MOUSEMOVE:
            // Captured状态
            if (m_bCaptured) {
                // 指向thumb?
                if (m_pointType == PointType::Type_Thumb) {
                    auto index = get_real_pos(
                        this->type == ScrollBarType::Type_Vertical ? arg.pt.y : arg.pt.x
                        );
                    this->SetIndex(m_fIndex + index - m_fOldPoint);
                    m_fOldPoint = index;
                }
            }
            //  检查指向类型
            else {
                if (IsPointInRect(m_rtArrow1, arg.pt)) {
                    m_pointType = PointType::Type_Arrow1;
                }
                else if (IsPointInRect(m_rtArrow2, arg.pt)) {
                    m_pointType = PointType::Type_Arrow2;
                }
                else if (IsPointInRect(m_rtThumb, arg.pt)) {
                    m_pointType = PointType::Type_Thumb;
                }
                else {
                    m_pointType = PointType::Type_Shaft;
                }
                // 修改
                if (m_lastPointType != m_pointType) {
                    this->set_status(m_lastPointType, LongUI::Status_Normal);
                    this->set_status(m_pointType, LongUI::Status_Hover);
                    m_lastPointType = m_pointType;
                }
            }
            return true;
        }
    }
    return Super::DoEvent(arg);
}

// UIScrollBarA:: 重建
auto LongUI::UIScrollBarA::Recreate(LongUIRenderTarget* target) noexcept -> HRESULT {
    m_uiArrow1.Recreate(target);
    m_uiArrow2.Recreate(target);
    m_uiThumb.Recreate(target);
    return Super::Recreate(target);
}

// UIScrollBarA: 初始化时
void LongUI::UIScrollBarA::InitScrollBar(UIContainer* owner, ScrollBarType _type) noexcept {
    // 创建几何
    auto create_geo = [](D2D1_POINT_2F* list, uint32_t length) {
        auto hr = S_OK;
        ID2D1PathGeometry* geometry = nullptr;
        ID2D1GeometrySink* sink = nullptr;
        // 创建几何体
        if (SUCCEEDED(hr)) {
            hr = UIManager_D2DFactory->CreatePathGeometry(&geometry);
        }
        // 打开
        if (SUCCEEDED(hr)) {
            hr = geometry->Open(&sink);
        }
        // 开始绘制
        if (SUCCEEDED(hr)) {
            sink->BeginFigure(list[0], D2D1_FIGURE_BEGIN_HOLLOW);
            sink->AddLines(list + 1, length - 1);
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
            hr = sink->Close();
        }
        AssertHR(hr);
        ::SafeRelease(sink);
        return geometry;
    };
    D2D1_POINT_2F point_list_1[3];
    D2D1_POINT_2F point_list_2[3];
    constexpr float BASIC_SIZE_MID = BASIC_SIZE * 0.5f;
    constexpr float BASIC_SIZE_NEAR = BASIC_SIZE_MID * 0.5f;
    constexpr float BASIC_SIZE_FAR = BASIC_SIZE - BASIC_SIZE_NEAR;
    // 水平滚动条
    if (_type != ScrollBarType::Type_Vertical) {
        //
        point_list_1[0] = { BASIC_SIZE_MID , BASIC_SIZE_NEAR };
        point_list_1[1] = { BASIC_SIZE_NEAR , BASIC_SIZE_MID };
        point_list_1[2] = { BASIC_SIZE_MID , BASIC_SIZE_FAR };
        //
        point_list_2[0] = { BASIC_SIZE_MID , BASIC_SIZE_NEAR };
        point_list_2[1] = { BASIC_SIZE_FAR , BASIC_SIZE_MID };
        point_list_2[2] = { BASIC_SIZE_MID , BASIC_SIZE_FAR };
    }
    // 垂直滚动条
    else {
        //
        point_list_1[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
        point_list_1[1] = { BASIC_SIZE_MID, BASIC_SIZE_NEAR };
        point_list_1[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
        //
        point_list_2[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
        point_list_2[1] = { BASIC_SIZE_MID, BASIC_SIZE_FAR };
        point_list_2[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
    }
    // 创建
    m_pArrow1Geo = create_geo(point_list_1, lengthof(point_list_1));
    m_pArrow2Geo = create_geo(point_list_2, lengthof(point_list_2));
    return Super::InitScrollBar(owner, _type);
}

// UIScrollBarA: 需要时
void LongUI::UIScrollBarA::OnNeeded(bool need) noexcept {
    m_fTakeSpace = need ? BASIC_SIZE : 0.f;
    m_fHitSpace = m_fTakeSpace;
    // 检查
}

// UIScrollBarA 析构函数
inline LongUI::UIScrollBarA::~UIScrollBarA() noexcept {
    ::SafeRelease(m_pArrow1Geo);
    ::SafeRelease(m_pArrow2Geo);
}

// UIScrollBarA 关闭控件
void  LongUI::UIScrollBarA::Close() noexcept {
    delete this;
}

// 设置状态
void LongUI::UIScrollBarA::set_status(PointType type, ControlStatus state) noexcept {
    BarElement* elements[] = { &m_uiArrow1, &m_uiArrow2, &m_uiThumb };
    // 检查
    if (type >= PointType::Type_Arrow1 && type <= PointType::Type_Thumb) {
        auto& element = *(elements[
            static_cast<uint32_t>(type) - static_cast<uint32_t>(PointType::Type_Arrow1)
        ]);
        UIElement_SetNewStatus(element, state);
    }
}
// create 创建
auto WINAPI LongUI::UIScrollBarA::CreateControl(pugi::xml_node node) noexcept ->UIControl* {
    // 获取模板节点
    if (!node) {

    }
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UIScrollBarA>(
        node,
        [=](void* p) noexcept { new(p) UIScrollBarA(node); }
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}
