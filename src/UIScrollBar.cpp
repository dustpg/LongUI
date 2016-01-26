#include "LongUI.h"
#include <algorithm>

// 获取相对数值
#define UISB_OffsetVaule(f) ((&(f))[int(this->bartype)])


// UIScrollBar 构造函数
LongUI::UIScrollBar::UIScrollBar(UIContainer* cp) noexcept : 
    Super(cp),m_uiAnimation(AnimationType::Type_QuadraticEaseIn) {

}

/// <summary>
/// Initalizes with specified xml-node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIScrollBar::initialize(pugi::xml_node node) noexcept {
    // 必须有效
    assert(node && "call UIScrollBar::initialize() if no xml-node");
    // 链式调用
    Super::initialize(node);
    // 边界相关
    assert((this->flags & Flag_MarginalControl) && "'UIScrollBar' must be marginal-control");
    auto sbtype = (this->marginal_type & 1U) ? ScrollBarType::Type_Horizontal : ScrollBarType::Type_Vertical;
    force_cast(this->bartype) = sbtype;
    // 修改
    m_uiAnimation.duration = 0.4f;
    // 结点有效
    {
        const char* str = nullptr;
        // 滚轮步长
        if ((str = node.attribute("wheelstep").value())) {
            this->wheel_step = LongUI::AtoF(str);
        }
        // 动画时间
        if ((str = node.attribute("aniamtionduration").value())) {
            m_uiAnimation.duration = LongUI::AtoF(str);;
        }
        // 动画类型
        if ((str = node.attribute("aniamtionbartype").value())) {
            m_uiAnimation.type = static_cast<AnimationType>(LongUI::AtoI(str));
        }
    }
    // 初始化
    m_uiAnimation.start = m_uiAnimation.end = m_uiAnimation.value = 0.f;
}


// 设置新的索引位置
void LongUI::UIScrollBar::SetIndex(float new_index) noexcept {
    // 阈值检查
    new_index = std::min(std::max(new_index, 0.f), m_fMaxIndex);
    // 无需设定
    if (new_index == m_uiAnimation.end) return;
    // 设定位置
    m_uiAnimation.start = m_uiAnimation.value = m_fIndex;
    m_uiAnimation.end = new_index;
    m_uiAnimation.time = m_uiAnimation.duration;
    m_pWindow->StartRender(m_uiAnimation.time, this->parent);
    m_bAnimation = true;
}

// 设置新的索引位置
void LongUI::UIScrollBar::set_index(float new_index) noexcept {
    //UIManager << DL_Hint << "new_index: " << new_index << endl;
    new_index = std::min(std::max(new_index, 0.f), m_fMaxIndex);
    // 不同就修改
    if (new_index != m_fIndex) {
        m_fIndex = new_index;
        // 修改父类属性
        this->parent->SetOffsetZoomed(int(this->bartype), -new_index);
        // 刷新拥有着
        m_pWindow->Invalidate(this->parent);
    }
}

/// <summary>
/// Updates the width of the marginal.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollBar::UpdateMarginalWidth() noexcept {
    // 水平
    if (this->bartype == ScrollBarType::Type_Horizontal) {
        m_fMaxRange = this->parent->GetContentWidthZoomed();
        m_fMaxIndex = m_fMaxRange - this->parent->GetViewWidthZoomed();
    }
    // 垂直
    else {
        m_fMaxRange = this->parent->GetContentHeightZoomed();
        m_fMaxIndex = m_fMaxRange - this->parent->GetViewHeightZoomed();
    }
    // 限制
    m_fMaxRange = std::max(m_fMaxRange, 0.f);
    m_fMaxIndex = std::max(m_fMaxIndex, 0.f);
    if (m_fIndex > m_fMaxIndex) {
        this->set_index(m_fMaxIndex);
    }
    return Super::UpdateMarginalWidth();
}


/// <summary>
/// Updates the width of the marginal.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollBarA::UpdateMarginalWidth() noexcept {
    // 加强父类方法
    Super::UpdateMarginalWidth();
    // 需要?
#ifdef _DEBUG
    if (this->debug_this && m_fMaxIndex > 0.f) {
        UIManager << DL_Log  << this << L"m_fMaxIndex: "
            << m_fMaxIndex << L" -- scrollbar standby?"<< endl;
    }
#endif
#if 0
    if (m_fMaxIndex < BASIC_SIZE * 0.5f) {
#else
    if (m_fMaxIndex < 0.5f) {
#endif
        this->marginal_width = 0.f;
        this->SetVisible(false);
    }
    else {
        this->marginal_width = BASIC_SIZE;
        this->SetVisible(true);
    }
}

// UIScrollBarA 初始化
void LongUI::UIScrollBarA::initialize(pugi::xml_node node) noexcept {
    // 必须有效
    assert(node && "call UIScrollBarA::initialize() if no xml-node");
    // 链式调用
    Super::initialize(node);
    auto stt = this->check_state();
    m_uiArrow1.Init(stt, 0, node,"arrow1");
    m_uiArrow2.Init(stt, 0, node, "arrow2");
    m_uiThumb.Init(stt, 0, node, "thumb");
    // 创建几何
    if (this->bartype == ScrollBarType::Type_Horizontal) {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Left]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Right]);
    }
    // 垂直滚动条
    else {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Top]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Bottom]);
    }
    assert(m_pArrow1Geo && m_pArrow2Geo);
    // 修改颜色
    {
        auto str = node.attribute("arrowstep").value();
        if (str) {
            m_fArrowStep = LongUI::AtoF(str);
        }
    }
    // 修改颜色
    /*else*/ {
        D2D1_COLOR_F color;
        color = D2D1::ColorF(0xF0F0F0);
        m_uiArrow1.GetBasicInterface().colors[State_Normal] = color;
        m_uiArrow2.GetBasicInterface().colors[State_Normal] = color;
        color = D2D1::ColorF(0x2F2F2F);
        m_uiArrow1.GetBasicInterface().colors[State_Pushed] = color;
        m_uiArrow2.GetBasicInterface().colors[State_Pushed] = color;
    }
    // 检查属性
    m_bArrow1InColor = !m_uiArrow1.IsExtraInterfaceValid();
    m_bArrow2InColor = !m_uiArrow2.IsExtraInterfaceValid();
}

// UI滚动条(类型A): 刷新
void LongUI::UIScrollBarA::Update() noexcept {
    // 索引不一致?
#ifdef _DEBUG
    auto offset = -(this->bartype == ScrollBarType::Type_Horizontal ?
        this->parent->GetOffsetXZoomed() : this->parent->GetOffsetYZoomed());
    if (std::abs(m_fIndex - offset) > 0.5f) {
        m_fIndex = offset;
        UIManager << DL_Hint << "diffence with offset(I: " 
            << m_fIndex << " O: " << offset 
            << " ), set new index" << LongUI::endl;
    }
#else
    m_fIndex = -(this->bartype == ScrollBarType::Type_Horizontal ?
        this->parent->GetOffsetXZoomed() : this->parent->GetOffsetYZoomed());
#endif
    // 先刷新父类
    D2D1_RECT_F draw_rect; this->GetViewRect(draw_rect);
    // 双滚动条修正
    m_rtThumb = m_rtArrow2 = m_rtArrow1 = draw_rect;
    float length_of_thumb, start_offset;
    {
        float tmpsize = UISB_OffsetVaule(this->view_size.width) - BASIC_SIZE*2.f;
        start_offset = tmpsize * m_fIndex / m_fMaxRange;
        length_of_thumb = tmpsize * (1.f - m_fMaxIndex / m_fMaxRange);
    }
    // 这段有点长, 使用UISB_OffsetVaule效率可能反而不如if
    // 垂直滚动条
    if (this->bartype == ScrollBarType::Type_Vertical) {
        m_rtArrow1.bottom = m_rtArrow1.top + BASIC_SIZE;
        m_rtArrow2.top = m_rtArrow2.bottom - BASIC_SIZE;
        m_rtThumb.top = m_rtArrow1.bottom + start_offset;
        m_rtThumb.bottom = m_rtThumb.top + length_of_thumb;
    }
    // 水平滚动条
    else {
        m_rtArrow1.right = m_rtArrow1.left + BASIC_SIZE;
        m_rtArrow2.left = m_rtArrow2.right - BASIC_SIZE;
        m_rtThumb.left = m_rtArrow1.right + start_offset;
        m_rtThumb.right = m_rtThumb.left + length_of_thumb;
    }
    // 刷新
    m_uiArrow1.Update();
    m_uiArrow2.Update();
    m_uiThumb.Update();
    // 刷新
    if (m_bAnimation) {
        m_uiAnimation.Update();
        this->set_index(m_uiAnimation.value);
        if (m_uiAnimation.time <= 0.f) {
            m_bAnimation = false;
        }
    }
    // 刷新
    return Super::Update();
}

// UIScrollBarA 渲染 
void LongUI::UIScrollBarA::Render() const noexcept {
    // 更新
    D2D1_RECT_F draw_rect; this->GetViewRect(draw_rect);
    // 双滚动条修正
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(0xF0F0F0));
    UIManager_RenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
    // 渲染部件
    m_uiArrow1.Render(m_rtArrow1);
    m_uiThumb.Render(m_rtThumb);
    m_uiArrow2.Render(m_rtArrow2);
    // 前景
    auto render_geo = [](ID2D1RenderTarget* const target, ID2D1Brush* const bush,
        ID2D1Geometry* const geo, const D2D1_RECT_F& rect) noexcept {
        D2D1_MATRIX_3X2_F matrix; target->GetTransform(&matrix);
        target->SetTransform(
            DX::Matrix3x2F::Translation(rect.left, rect.top) * matrix
            );
        target->DrawGeometry(geo, bush, 2.33333f);
        // 修改
        target->SetTransform(&matrix);
    };
    // 渲染几何体
    {
        D2D1_COLOR_F color;
        if (m_bArrow1InColor) {
            color = m_uiArrow1.GetBasicInterface().colors[m_uiArrow1.GetNowBasicState()];
            color.r = 1.f - color.r; color.g = 1.f - color.g; color.b = 1.f - color.b;
            m_pBrush_SetBeforeUse->SetColor(&color);
            render_geo(UIManager_RenderTarget, m_pBrush_SetBeforeUse, m_pArrow1Geo, m_rtArrow1);
        }
        // 渲染几何体
        if (m_bArrow2InColor) {
            color = m_uiArrow2.GetBasicInterface().colors[m_uiArrow2.GetNowBasicState()];
            color.r = 1.f - color.r; color.g = 1.f - color.g; color.b = 1.f - color.b;
            m_pBrush_SetBeforeUse->SetColor(&color);
            render_geo(UIManager_RenderTarget, m_pBrush_SetBeforeUse, m_pArrow2Geo, m_rtArrow2);
        }
    }
}

// UIScrollBarA::do event 事件处理
bool  LongUI::UIScrollBarA::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // -------------------- on mouse move --------------------
    auto on_mouse_move = [this, &pt4self]() {
        // Captured状态
        if (m_bCaptured) {
            // 指向thumb?
            if (m_pointType == PointType::Type_Thumb) {
                // 计算移动距离
                auto pos = UISB_OffsetVaule(pt4self.x);
                auto zoom = this->parent->GetZoom(int(this->bartype));
                auto rate = (1.f - m_fMaxIndex / (m_fMaxRange - BASIC_SIZE*2.f)) * zoom;
                this->set_index((pos - m_fOldPoint) / rate + m_fOldIndex);
            }
        }
        //  检查指向类型
        else {
            if (IsPointInRect(m_rtArrow1, pt4self)) {
                m_pointType = PointType::Type_Arrow1;
            }
            else if (IsPointInRect(m_rtArrow2, pt4self)) {
                m_pointType = PointType::Type_Arrow2;
            }
            else if (IsPointInRect(m_rtThumb, pt4self)) {
                m_pointType = PointType::Type_Thumb;
            }
            else {
                m_pointType = PointType::Type_Shaft;
            }
            // 修改
            if (m_lastPointType != m_pointType) {
                this->set_state(m_lastPointType, LongUI::State_Normal);
                this->set_state(m_pointType, LongUI::State_Hover);
                m_lastPointType = m_pointType;
            }
        }
    };
    // -------------------- on l-button down --------------------
    auto on_lbutton_down = [this, &pt4self]() {
        m_pWindow->SetCapture(this);
        m_bCaptured = true;
        this->set_state(m_pointType, LongUI::State_Pushed);
        switch (m_pointType)
        {
        case LongUI::UIScrollBar::PointType::Type_Arrow1:
            // 左/上移动
            this->SetIndex(m_uiAnimation.end - m_fArrowStep);
            break;
        case LongUI::UIScrollBar::PointType::Type_Arrow2:
            // 右/下移动
            this->SetIndex(m_uiAnimation.end + m_fArrowStep);
            break;
        case LongUI::UIScrollBar::PointType::Type_Thumb:
            // 拖拽
            m_fOldPoint = UISB_OffsetVaule(pt4self.x);
            m_fOldIndex = m_fIndex;
            break;
        case LongUI::UIScrollBar::PointType::Type_Shaft:
        {
            auto tmpx = (UISB_OffsetVaule(pt4self.x) - BASIC_SIZE);
            auto tmpl = (UISB_OffsetVaule(this->view_size.width) - BASIC_SIZE * 2.f);
            // 设置目标
            this->SetIndex(tmpx / tmpl * m_fMaxIndex);
        }
            break;
        }
    };
    // --------------------     main proc    --------------------
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
    case LongUI::MouseEvent::Event_MouseWheelH:
    {
        // 滚动条
        auto test1 = arg.event == LongUI::MouseEvent::Event_MouseWheelV;
        auto test2 = !!(arg.sys.wParam & MK_SHIFT);
        if ((test1 && test2) == (this->bartype == ScrollBarType::Type_Horizontal)) {
            auto wheel = (float(GET_WHEEL_DELTA_WPARAM(arg.sys.wParam))) / float(WHEEL_DELTA);
            this->SetIndex(m_uiAnimation.end - wheel_step * wheel);
            return true;
        }
        return false;
    }
    case LongUI::MouseEvent::Event_MouseLeave:
        this->set_state(m_lastPointType, LongUI::State_Normal);
        m_pointType = PointType::Type_None;
        m_lastPointType = PointType::Type_None;
        return true;
    case LongUI::MouseEvent::Event_MouseMove:
        on_mouse_move();
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        on_lbutton_down();
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        this->set_state(m_pointType, LongUI::State_Hover);
        m_bCaptured = false;
        m_pWindow->ReleaseCapture();
        return true;
    case LongUI::MouseEvent::Event_RButtonDown:
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        break;
    case LongUI::MouseEvent::Event_MButtonDown:
        break;
    case LongUI::MouseEvent::Event_MButtonUp:
        break;
    default:
        break;
    }
    return false;
}

// UIScrollBarA:: 重建
auto LongUI::UIScrollBarA::Recreate() noexcept -> HRESULT {
    m_uiArrow1.Recreate();
    m_uiArrow2.Recreate();
    m_uiThumb.Recreate();
    return Super::Recreate();
}

// UIScrollBarA: 初始化时
/*void LongUI::UIScrollBarA::InitMarginalControl(MarginalControl _type) noexcept {
    // 初始化
    Super::InitMarginalControl(_type);
    // 创建几何
    if (this->bartype == ScrollBarType::Type_Horizontal) {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Left]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Right]);
    }
    // 垂直滚动条
    else {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Top]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Bottom]);
    }
    assert(m_pArrow1Geo && m_pArrow2Geo);
}
*/

// UIScrollBarA 析构函数
inline LongUI::UIScrollBarA::~UIScrollBarA() noexcept {
    LongUI::SafeRelease(m_pArrow1Geo);
    LongUI::SafeRelease(m_pArrow2Geo);
}

// UIScrollBarA 关闭控件
void  LongUI::UIScrollBarA::cleanup() noexcept {
    delete this;
}

// 设置状态
void LongUI::UIScrollBarA::set_state(PointType _bartype, ControlState state) noexcept {
    // 检查
    Component::Element4Bar* elements[] = { &m_uiArrow1, &m_uiArrow2, &m_uiThumb };
    // 检查
    if (_bartype >= PointType::Type_Arrow1 && _bartype <= PointType::Type_Thumb) {
        auto index = static_cast<uint32_t>(_bartype) - static_cast<uint32_t>(PointType::Type_Arrow1);
        m_pWindow->StartRender(elements[index]->SetBasicState(state), this);
    }
}

// 静态变量
ID2D1PathGeometry* LongUI::UIScrollBarA::
s_apArrowPathGeometry[LongUI::UIScrollBarA::ARROW_SIZE] = { nullptr };

// create 创建
auto WINAPI LongUI::UIScrollBarA::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIScrollBarA* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
    {
        // 创建设备无关资源
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
            ShowHR(hr);
            LongUI::SafeRelease(sink);
            return geometry;
        };
        D2D1_POINT_2F point_list[3];
        constexpr float BASIC_SIZE_MID = BASIC_SIZE * 0.5f;
        constexpr float BASIC_SIZE_NEAR = BASIC_SIZE_MID * 0.5f;
        constexpr float BASIC_SIZE_FAR = BASIC_SIZE - BASIC_SIZE_NEAR;
        // LEFT 左箭头
        {
            point_list[0] = { BASIC_SIZE_MID , BASIC_SIZE_NEAR };
            point_list[1] = { BASIC_SIZE_NEAR , BASIC_SIZE_MID };
            point_list[2] = { BASIC_SIZE_MID , BASIC_SIZE_FAR };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Left]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Left] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
        // TOP 上箭头
        {
            point_list[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
            point_list[1] = { BASIC_SIZE_MID, BASIC_SIZE_NEAR };
            point_list[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Top]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Top] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
        // RIGHT 右箭头
        {

            point_list[0] = { BASIC_SIZE_MID , BASIC_SIZE_NEAR };
            point_list[1] = { BASIC_SIZE_FAR , BASIC_SIZE_MID };
            point_list[2] = { BASIC_SIZE_MID , BASIC_SIZE_FAR };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Right]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Right] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
        // BOTTOM 下箭头
        {
            point_list[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
            point_list[1] = { BASIC_SIZE_MID, BASIC_SIZE_FAR };
            point_list[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Bottom]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Bottom] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
    }
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        // 释放资源
        for (auto& geo : s_apArrowPathGeometry) {
            LongUI::SafeRelease(geo);
        }
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIScrollBarA, pControl, type, node);
    }
    return pControl;
}


/*// UIScrollBarB 构造函数
LongUI::UIScrollBarB::UIScrollBarB(UIContainer* cp) noexcept : Super(cp) {

}*/

// UIScrollBarB 创建函数
auto WINAPI LongUI::UIScrollBarB::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIScrollBarB* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIScrollBarB, pControl, type, node);
    }
    return pControl;
}

// UIScrollBarB: 刷新
void  LongUI::UIScrollBarB::Update() noexcept {

}


// UIScrollBarB 关闭控件
void  LongUI::UIScrollBarB::cleanup() noexcept {
    delete this;
}
