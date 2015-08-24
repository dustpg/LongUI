#include "LongUI.h"

// Core Contrl for UIControl, UIMarginalable, UIContainer, UINull

// 系统按钮:
/*
Win8/8.1/10.0.10158之前
焦点: 0x3399FF 矩形描边, 并且内边有虚线矩形
0. 禁用: 0xD9灰度 矩形描边; 中心 0xEF灰色
1. 普通: 0xAC灰度 矩形描边; 中心 从上到下0xF0灰色到0xE5灰色渐变
2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变
*/

/// <summary>
/// Initializes a new instance of the <see cref="LongUI::UIControl"/>
/// class with xml node
/// </summary>
/// <param name="node" type="pugi::xml_node">The xml node</param>
/// <remarks>
/// For this function, param 'node' could be null node
/// 对于本函数, 参数'node'允许为空
/// <see cref="LongUINullXMLNode"/>
/// </remarks>
LongUI::UIControl::UIControl(pugi::xml_node node) noexcept {
    // 构造默认
    auto flag = LongUIFlag::Flag_None;
    // 有效?
    if (node) {
        const char* data = nullptr;
        // 检查脚本
        if ((data = node.attribute(XMLAttribute::Script).value()) && UIManager.script) {
            m_script = UIManager.script->AllocScript(data);
        }
        // 检查权重
        if (data = node.attribute(LongUI::XMLAttribute::LayoutWeight).value()) {
            force_cast(this->weight) = LongUI::AtoF(data);
        }
        // 检查背景笔刷
        if (data = node.attribute(LongUI::XMLAttribute::BackgroudBrush).value()) {
            m_idBackgroudBrush = uint32_t(LongUI::AtoI(data));
            if (m_idBackgroudBrush) {
                assert(!m_pBackgroudBrush);
                m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
            }
        }
        // 检查可视性
        this->visible = node.attribute(LongUI::XMLAttribute::Visible).as_bool(true);
        // 渲染优先级
        if (data = node.attribute(LongUI::XMLAttribute::RenderingPriority).value()) {
            force_cast(this->priority) = uint8_t(LongUI::AtoI(data));
        }
        // 检查名称
        Helper::MakeString(
            node.attribute(LongUI::XMLAttribute::ControlName).value(),
            m_strControlName
            );
        // 检查外边距
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::Margin).value(),
            const_cast<float*>(&margin_rect.left),
            sizeof(margin_rect) / sizeof(margin_rect.left)
            );
        // 检查渲染父控件
        if (node.attribute(LongUI::XMLAttribute::IsRenderParent).as_bool(false)) {
            flag |= LongUI::Flag_RenderParent;
        }
        // 检查裁剪规则
        if (node.attribute(LongUI::XMLAttribute::IsClipStrictly).as_bool(true)) {
            flag |= LongUI::Flag_ClipStrictly;
        }
        // 边框大小
        if (data = node.attribute(LongUI::XMLAttribute::BorderWidth).value()) {
            m_fBorderWidth = LongUI::AtoF(data);
        }
        // 边框圆角
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::BorderRound).value(),
            &m_2fBorderRdius.width,
            sizeof(m_2fBorderRdius) / sizeof(m_2fBorderRdius.width)
            );
        // 检查控件大小
        {
            float size[] = { 0.f, 0.f };
            Helper::MakeFloats(
                node.attribute(LongUI::XMLAttribute::AllSize).value(),
                size, lengthof(size)
                );
            // 视口区宽度固定?
            if (size[0] > 0.f) {
                flag |= LongUI::Flag_WidthFixed;
                this->SetWidth(size[0]);
            }
            // 视口区高度固固定?
            if (size[1] > 0.f) {
                flag |= LongUI::Flag_HeightFixed;
                this->SetHeight(size[1]);
            }
        }
        // 检查控件位置
        {
            float pos[] = { 0.f, 0.f };
            Helper::MakeFloats(
                node.attribute(LongUI::XMLAttribute::LeftTopPosotion).value(),
                pos, lengthof(pos)
                );
            // 指定X轴
            if (pos[0] != 0.f) {
                this->SetLeft(pos[0]);
                flag |= Flag_Floating;
            }
            // 指定Y轴
            if (pos[1] != 0.f) {
                this->SetTop(pos[1]);
                flag |= Flag_Floating;
            }
        }
    }
    // 修改flag
    force_cast(this->flags) |= flag;
}

// 析构函数
LongUI::UIControl::~UIControl() noexcept {
    ::SafeRelease(m_pBrush_SetBeforeUse);
    ::SafeRelease(m_pBackgroudBrush);
    // 释放脚本占用空间
    if (m_script.script) {
        assert(UIManager.script && "no script interface but data");
        UIManager.script->FreeScript(m_script);
    }
    // 反注册
    if (this->flags & Flag_NeedRegisterOffScreenRender) {
        m_pWindow->UnRegisterOffScreenRender(this);
    }
}


/// <summary>
/// Render control via specified render-type.
/// </summary>
/// <param name="_type" type="enum LongUI::RenderType">The _type.</param>
/// <returns></returns>
void LongUI::UIControl::Render(RenderType type) const noexcept {
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 背景
        if (m_pBackgroudBrush) {
            D2D1_RECT_F rect; this->GetViewRect(rect);
            LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, m_pBackgroudBrush, rect);
        }
        // 背景中断
        if (type == RenderType::Type_RenderBackground) {
            break;
        }
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 渲染边框
        if (m_fBorderWidth > 0.f) {
            D2D1_ROUNDED_RECT brect; this->GetBorderRect(brect.rect);
            m_pBrush_SetBeforeUse->SetColor(&m_colorBorderNow);
            if (m_2fBorderRdius.width > 0.f && m_2fBorderRdius.height > 0.f) {
                brect.radiusX = m_2fBorderRdius.width;
                brect.radiusY = m_2fBorderRdius.height;
                //UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                UIManager_RenderTarget->DrawRoundedRectangle(&brect, m_pBrush_SetBeforeUse, m_fBorderWidth);
                //UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
            else {
                UIManager_RenderTarget->DrawRectangle(&brect.rect, m_pBrush_SetBeforeUse, m_fBorderWidth);
            }
        }
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}


// UI控件: 刷新
void LongUI::UIControl::Update() noexcept {
    // 控件大小处理了
    if (m_bool16.Test(Index_ChangeSizeHandled)) {
        m_bool16.SetFalse(Index_ChangeSize);
        m_bool16.SetFalse(Index_ChangeSizeHandled);
    }
    // 世界转换处理了
    if (m_bool16.Test(Index_ChangeWorldHandled)) {
        m_bool16.SetFalse(Index_ChangeWorld);
        m_bool16.SetFalse(Index_ChangeWorldHandled);
    }
}

// UI控件: 重建
auto LongUI::UIControl::Recreate() noexcept ->HRESULT {
    // 设备重置再说
    ::SafeRelease(m_pBrush_SetBeforeUse);
    ::SafeRelease(m_pBackgroudBrush);
    m_pBrush_SetBeforeUse = static_cast<decltype(m_pBrush_SetBeforeUse)>(
        UIManager.GetBrush(LongUICommonSolidColorBrushIndex)
        );
    if (m_idBackgroudBrush) {
        m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
    }
    return S_OK;
}

// LongUI::UIControl 注册回调事件
void LongUI::UIControl::SetSubEventCallBack(
    const wchar_t* control_name, LongUI::SubEvent event, SubEventCallBack call) noexcept {
    assert(control_name && call&&  "bad argument");
    UIControl* control = m_pWindow->FindControl(control_name);
    assert(control && " no control found");
    if (!control) return;
    CUISubEventCaller caller(call, this);
    // 自定义消息?
    if (event >= LongUI::SubEvent::Event_Custom) {
        UIManager.configure->SetSubEventCallBack(event, caller, control);
        return;
    }
    // 检查
    switch (event)
    {
    case LongUI::SubEvent::Event_ButtonClicked:
        longui_cast<UIButton*>(control)->RegisterClickEvent(caller);
        break;
    case LongUI::SubEvent::Event_EditReturned:
        longui_cast<UIEditBasic*>(control)->RegisterReturnEvent(caller);
        break;
    case LongUI::SubEvent::Event_SliderValueChanged:
        longui_cast<UISlider*>(control)->RegisterValueChangedEvent(caller);
        break;
    }
}

// 获取占用宽度
auto LongUI::UIControl::GetTakingUpWidth() const noexcept -> float {
    return this->view_size.width
        + margin_rect.left
        + margin_rect.right
        + m_fBorderWidth * 2.f;
}

// 获取占用高度
auto LongUI::UIControl::GetTakingUpHeight() const noexcept -> float {
    return this->view_size.height
        + margin_rect.top
        + margin_rect.bottom
        + m_fBorderWidth * 2.f;
}

// 获取非内容区域总宽度
auto LongUI::UIControl::GetNonContentWidth() const noexcept -> float {
    return margin_rect.left
        + margin_rect.right
        + m_fBorderWidth * 2.f;
}

// 获取非内容区域总高度
auto LongUI::UIControl::GetNonContentHeight() const noexcept -> float {
    return margin_rect.top
        + margin_rect.bottom
        + m_fBorderWidth * 2.f;
}

// 设置占用宽度
auto LongUI::UIControl::SetWidth(float width) noexcept -> void {
    // 设置
    auto new_vwidth = width - this->GetNonContentWidth();
    if (new_vwidth != this->view_size.width) {
        force_cast(this->view_size.width) = new_vwidth;
        this->SetControlSizeChanged();
    }
    // 检查
    if (this->view_size.width < 0.f) {
        UIManager << DL_Hint << this
            << "viewport's width < 0: " << this->view_size.width << endl;
    }
}

// 设置占用高度
auto LongUI::UIControl::SetHeight(float height) noexcept -> void LongUINoinline {
    // 设置
    auto new_vheight = height - this->GetNonContentHeight();
    if (new_vheight != this->view_size.height) {
        force_cast(this->view_size.height) = new_vheight;
        this->SetControlSizeChanged();
    }
    // 检查
    if (this->view_size.height < 0.f) {
        UIManager << DL_Hint << this
            << "viewport's height < 0: " << this->view_size.height << endl;
    }
}

// 设置控件左坐标
auto LongUI::UIControl::SetLeft(float left) noexcept -> void {
    auto new_left = left + this->margin_rect.left + m_fBorderWidth;
    // 修改了位置?
    if (this->view_pos.x != new_left) {
        force_cast(this->view_pos.x) = new_left;
        this->SetControlWorldChanged();
    }
}

// 设置控件顶坐标
auto LongUI::UIControl::SetTop(float top) noexcept -> void {
    auto new_top = top + this->margin_rect.top + m_fBorderWidth;
    // 修改了位置?
    if (this->view_pos.y != new_top) {
        force_cast(this->view_pos.y) = new_top;
        this->SetControlWorldChanged();
    }
}

// 获取占用/剪切矩形
void LongUI::UIControl::GetClipRect(D2D1_RECT_F& rect) const noexcept {
    rect.left = -(this->margin_rect.left + m_fBorderWidth);
    rect.top = -(this->margin_rect.top + m_fBorderWidth);
    rect.right = this->view_size.width + this->margin_rect.right + m_fBorderWidth;
    rect.bottom = this->view_size.height + this->margin_rect.bottom + m_fBorderWidth;
    /*// 容器
    if ((this->flags & Flag_UIContainer)) {
    // 修改裁剪区域
    //rect.left -= static_cast<const UIContainer*>(this)->offset.x;
    //rect.top -= static_cast<const UIContainer*>(this)->offset.y;
    auto container = static_cast<const UIContainer*>(this);
    rect.right = rect.left + static_cast<const UIContainer*>(this)->width;
    rect.bottom = rect.top + static_cast<const UIContainer*>(this)->height;
    if (static_cast<const UIContainer*>(this)->scrollbar_h) {
    rect.bottom -= static_cast<const UIContainer*>(this)->scrollbar_h->GetTakingUpSapce();
    }
    if (static_cast<const UIContainer*>(this)->scrollbar_v) {
    rect.right -= static_cast<const UIContainer*>(this)->scrollbar_v->GetTakingUpSapce();
    }
    }
    else {
    rect.right = this->width + this->margin_rect.right + m_fBorderWidth;
    rect.bottom = this->height + this->margin_rect.bottom + m_fBorderWidth;
    }*/
}

// 获取边框矩形
void LongUI::UIControl::GetBorderRect(D2D1_RECT_F& rect) const noexcept {
    rect.left = -m_fBorderWidth * 0.5f;
    rect.top = -m_fBorderWidth * 0.5f;
    rect.right = this->view_size.width + m_fBorderWidth * 0.5f;
    rect.bottom = this->view_size.height + m_fBorderWidth * 0.5f;
}

// 获取视口刻画矩形
void LongUI::UIControl::GetViewRect(D2D1_RECT_F& rect) const noexcept {
    rect.left = 0.f;
    rect.top = 0.f;
    rect.right = this->view_size.width;
    rect.bottom = this->view_size.height;
}

// 获得世界转换矩阵
void LongUI::UIControl::RefreshWorld() noexcept {
    float xx = this->view_pos.x /*+ this->margin_rect.left + m_fBorderWidth*/;
    float yy = this->view_pos.y /*+ this->margin_rect.top + m_fBorderWidth*/;
    // 顶级控件
    if (this->IsTopLevel()) {
        this->world = D2D1::Matrix3x2F::Translation(xx, yy);
    }
    // 非顶级控件
    else {
#if 1
        // 检查
        xx += this->parent->GetOffsetXZoomed();
        yy += this->parent->GetOffsetYZoomed();
        // 转换
        this->world = 
            D2D1::Matrix3x2F::Translation(xx, yy) 
            *D2D1::Matrix3x2F::Scale(
                this->parent->GetZoomX(), this->parent->GetZoomY()
                )
            * this->parent->world;
#else
        this->world = 
            D2D1::Matrix3x2F::Translation(xx, yy)
            * D2D1::Matrix3x2F::Scale(
                this->parent->GetZoomX(), this->parent->GetZoomY()
                )
            * D2D1::Matrix3x2F::Translation(
                this->parent->GetOffsetX(), this->parent->GetOffsetY()
                ) 
            * this->parent->world;
#endif
    }
    // 修改了
    this->ControlWorldChangeHandled();
}

// 获得世界转换矩阵 for 边缘控件
void LongUI::UIMarginalable::RefreshWorldMarginal() noexcept {
    float xx = this->view_pos.x /*+ this->margin_rect.left + m_fBorderWidth*/;
    float yy = this->view_pos.y /*+ this->margin_rect.top + m_fBorderWidth*/;
    D2D1_MATRIX_3X2_F identity;
    D2D1_MATRIX_3X2_F* parent_world = &identity;
    // 顶级
    identity = D2D1::Matrix3x2F::Identity();
    if (this->parent->IsTopLevel()) {
        identity = D2D1::Matrix3x2F::Identity();
    }
    else {
        parent_world = &this->parent->world;
    }
    // 计算矩阵
    this->world = D2D1::Matrix3x2F::Translation(xx, yy) ** parent_world;
    // 自己不能是顶级的
    assert(this->IsTopLevel() == false);
    constexpr int aa = sizeof(UIContainer);
}

// ----------------------------------------------------------------------------
// UINull
// ----------------------------------------------------------------------------

// LongUI namespace
namespace LongUI {
    // null control
    class UINull : public UIControl {
    private:
        // 父类申明
        using Super = UIControl;
    public:
        // Render 渲染
        virtual void Render(RenderType) const noexcept override {}
        // update 刷新
        virtual void Update() noexcept override {}
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument&) noexcept override { return false; }
        // close this control 关闭控件
        virtual void Cleanup() noexcept override { delete this; }
    public:
        // 创建控件
        static auto CreateControl(pugi::xml_node node) noexcept {
            UIControl* pControl = nullptr;
            // 判断
            if (!node) {
                UIManager << DL_Warning << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = LongUI::UIControl::AllocRealControl<LongUI::UINull>(
                node,
                [=](void* p) noexcept { new(p) UINull(node); }
            );
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            return pControl;
        }
    public:
        // constructor 构造函数
        UINull(pugi::xml_node node) noexcept : Super(node) {}
        // destructor 析构函数
        ~UINull() noexcept { }
    };
}

// 创建空控件
auto WINAPI LongUI::CreateNullControl(CreateEventType type, pugi::xml_node node) noexcept -> UIControl * {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        pControl = LongUI::UINull::CreateControl(node);
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


// ------------------------------ UIContainer -----------------------------
// UIContainer 构造函数
LongUI::UIContainer::UIContainer(pugi::xml_node node) noexcept : Super(node), marginal_control() {
    ::memset(force_cast(marginal_control), 0, sizeof(marginal_control));
    assert(node && "bad argument.");
    // LV
    if (m_strControlName == L"V") {
        m_2fZoom = { 2.f, 2.f };
    }
    // 保留原始外间距
    m_orgMargin = this->margin_rect;
    auto flag = this->flags | Flag_UIContainer;
    // 检查边缘控件: 属性ID
    const char* const attname[] = {
        LongUI::XMLAttribute::LeftMarginalControl,
        LongUI::XMLAttribute::TopMarginalControl,
        LongUI::XMLAttribute::RightMarginalControl,
        LongUI::XMLAttribute::BottomMarginalControl,
    };
    bool exist_marginal_control = false;
    // ONLY MY LOOPGUN
    for (auto i = 0u; i < UIMarginalable::MARGINAL_CONTROL_SIZE; ++i) {
        const char* str = nullptr;
        // 获取指定属性值
        if ((str = node.attribute(attname[i]).value())) {
            char buffer[LongUIStringLength];
            assert(::strlen(str) < LongUIStringLength && "buffer too small");
            // 获取逗号位置
            auto strtempid = std::strchr(str, ',');
            if (strtempid) {
                auto length = strtempid - str;
                ::memcpy(buffer, str, length);
                buffer[length] = char(0);
            }
            else {
                ::strcpy(buffer, str);
            }
            
            // 获取类ID
            auto create_control_func = UIManager.GetCreateFunc(buffer);
            assert(create_control_func && "none");
            if (create_control_func) {
                // 检查模板ID
                auto tid = strtempid ? LongUI::AtoI(strtempid + 1) : 0;
                // 创建控件
                auto control = UIManager.CreateControl(size_t(tid), create_control_func);
                // XXX: 检查
                force_cast(this->marginal_control[i]) = longui_cast<UIMarginalable*>(control);
            }
            // 优化flag
            if (this->marginal_control[i]) {
                exist_marginal_control = true;
            }
        }
    }
    // 存在
    if (exist_marginal_control) {
        flag |= Flag_Container_ExistMarginalControl;
    }
    // 渲染依赖属性
    if (node.attribute(XMLAttribute::IsHostChildrenAlways).as_bool(false)) {
        flag |= LongUI::Flag_Container_HostChildrenRenderingDirectly;
    }
    // 边缘控件缩放
    if (node.attribute(XMLAttribute::IsZoomMarginalControl).as_bool(true)) {
        flag |= LongUI::Flag_Container_ZoomMarginalControl;
    }
    force_cast(this->flags) = flag;
}

// UIContainer 析构函数
LongUI::UIContainer::~UIContainer() noexcept {
    // 关闭边缘控件
    // 只有一次 Flag_Container_ExistMarginalControl 可用可不用
    for (auto ctrl : this->marginal_control) {
        if (ctrl) {
            ctrl->Cleanup();
        }
    }
    // 关闭子控件
    {
        auto ctrl = m_pHead;
        while (ctrl) {
            auto next_ctrl = ctrl->next;
            ctrl->Cleanup();
            ctrl = next_ctrl;
        }
    }
}

// 插入后处理
void LongUI::UIContainer::AfterInsert(UIControl* child) noexcept {
    assert(child && "bad argument");
    // 大小判断
    if (this->size() >= 10'000) {
        UIManager << DL_Warning << "the count of children must be"
            " less than 10k because of the precision of float" << LongUI::endl;
        assert(!"because of the precision of float, the count of children must be less than 10k");
    }
    // 检查flag
    if (this->flags & Flag_Container_HostChildrenRenderingDirectly) {
        force_cast(child->flags) |= Flag_RenderParent;
    }
    // 设置父类
    force_cast(child->parent) = this;
    // 设置窗口节点
    child->m_pWindow = m_pWindow;
    // 重建资源
    child->Recreate();
    // 修改
    child->SetControlSizeChanged();
    // 修改
    this->SetControlSizeChanged();
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIContainer::FindControl(const D2D1_POINT_2F pt) noexcept->UIControl* {
    // 查找边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl && IsPointInRect(ctrl->visible_rect, pt)) {
                return ctrl;
            }
        }
    }
    this->AssertMarginalControl();
    UIControl* control_out = nullptr;
    // XXX: 优化
    assert(this->size() < 100 && "too huge, wait for optimization please");
    for (auto ctrl : (*this)) {
        /*if (m_strControlName == L"MainWindow") {
        int a = 9;
        }*/
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            if (ctrl->flags & Flag_UIContainer) {
                control_out = static_cast<UIContainer*>(ctrl)->FindControl(pt);
            }
            else {
                control_out = ctrl;
            }
            break;
        }
    }
    return control_out;
}


// do event 事件处理
bool LongUI::UIContainer::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // TODO: 参数EventArgument改为const
    bool done = false;
    // 转换坐标
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 初始化边缘控件 
            for (auto i = 0; i < lengthof(this->marginal_control); ++i) {
                auto ctrl = this->marginal_control[i];
                if (ctrl) {
                    this->AfterInsert(ctrl);
                    // 初始化
                    ctrl->InitMarginalControl(static_cast<UIMarginalable::MarginalControl>(i));
                    // 完成控件树
                    ctrl->DoEvent(arg);
                }
            }
            // 初次完成空间树建立
            for (auto ctrl : (*this)) {
                ctrl->DoEvent(arg);
            }
            done = true;
            break;
        }
    }
    // 扳回来
    return done;
}

// UIContainer 渲染函数
void LongUI::UIContainer::Render(RenderType type) const noexcept {
    //  正确渲染控件
    auto do_render = [](const UIControl* const ctrl) {
        // 可渲染?
        if (ctrl->visible && ctrl->visible_rect.right > ctrl->visible_rect.left
            && ctrl->visible_rect.bottom > ctrl->visible_rect.top) {
            // 修改世界转换矩阵
            UIManager_RenderTarget->SetTransform(&ctrl->world);
            // 检查剪切规则
            if (ctrl->flags & Flag_ClipStrictly) {
                D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
                UIManager_RenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
            ctrl->Render(LongUI::RenderType::Type_Render);
            // 检查剪切规则
            if (ctrl->flags & Flag_ClipStrictly) {
                UIManager_RenderTarget->PopAxisAlignedClip();
            }
        }
    };
    // 查看
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
        // 普通子控件仅仅允许渲染在内容区域上
        {
            D2D1_RECT_F clip_rect; this->GetViewRect(clip_rect);
            UIManager_RenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        // 渲染所有子部件
        for (const auto* ctrl : (*this)) {
            do_render(ctrl);
        }
        // 弹出
        UIManager_RenderTarget->PopAxisAlignedClip();
        // 渲染边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            for (auto ctrl : this->marginal_control) {
                if (ctrl) {
                    do_render(ctrl);
                }
            }
        }
        this->AssertMarginalControl();
        // 回退转变
        UIManager_RenderTarget->SetTransform(&this->world);
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// 刷新边缘控件
void LongUI::UIContainer::update_marginal_controls() noexcept {
    // 获取宽度
    auto get_marginal_width = [](UIMarginalable* ctrl) noexcept {
        return ctrl ? ctrl->marginal_width : 0.f;
    };
    // 利用规则获取宽度
    auto get_marginal_width_with_rule = [](UIMarginalable* a, UIMarginalable* b) noexcept {
        return a->rule == UIMarginalable::Rule_Greedy ? 0.f : (b ? b->marginal_width : 0.f);
    };
    // 计算宽度
    auto caculate_container_width = [this, get_marginal_width]() noexcept {
        // 基本宽度
        return this->view_size.width
            + m_orgMargin.left
            + m_orgMargin.right
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Left])
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Right])
            + m_fBorderWidth * 2.f;
    };
    // 计算高度
    auto caculate_container_height = [this, get_marginal_width]() noexcept {
        // 基本宽度
        return this->view_size.height
            + m_orgMargin.top
            + m_orgMargin.bottom
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Top])
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Bottom])
            + m_fBorderWidth * 2.f;
    };
    // 保留信息
    const float this_container_width = caculate_container_width();
    const float this_container_height = caculate_container_height();
    if (m_strControlName == L"V") {
        int bk = 9;
    }
    // 循环
    while (true) {
        for (auto i = 0u; i < lengthof(this->marginal_control); ++i) {
            // 获取控件
            auto ctrl = this->marginal_control[i]; if (!ctrl) continue;
            //float view[] = { 0.f, 0.f, 0.f, 0.f };
            // TODO: 计算cross 大小
            switch (i)
            {
            case 0: // Left
            {
                const auto tmptop = m_orgMargin.top +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Top]);
                // 坐标
                ctrl->SetLeft(m_orgMargin.left);
                ctrl->SetTop(tmptop);
                // 大小
                ctrl->SetWidth(ctrl->marginal_width);
                ctrl->SetHeight(
                    this_container_height - tmptop - m_orgMargin.bottom -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Bottom])
                    );
            }
            break;
            case 1: // Top
            {
                const float tmpleft = m_orgMargin.left +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Left]);
                // 坐标
                ctrl->SetLeft(tmpleft);
                ctrl->SetTop(m_orgMargin.top);
                // 大小
                ctrl->SetWidth(
                    this_container_width - tmpleft - m_orgMargin.right -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Right])
                    );
                ctrl->SetHeight(ctrl->marginal_width);
            }
            break;
            case 2: // Right
            {
                const auto tmptop = m_orgMargin.top +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Top]);
                // 坐标
                ctrl->SetLeft(this_container_width - m_orgMargin.right - ctrl->marginal_width);
                ctrl->SetTop(tmptop);
                // 大小
                ctrl->SetWidth(ctrl->marginal_width);
                ctrl->SetHeight(
                    this_container_height - tmptop - m_orgMargin.bottom -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Bottom])
                    );
            }
            break;
            case 3: // Bottom
            {
                const float tmpleft = m_orgMargin.left +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Left]);
                // 坐标
                ctrl->SetLeft(tmpleft);
                ctrl->SetTop(this_container_height - m_orgMargin.bottom - ctrl->marginal_width);
                // 大小
                ctrl->SetWidth(
                    this_container_width - tmpleft - m_orgMargin.right -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Right])
                    );
                ctrl->SetHeight(ctrl->marginal_width);
            }
            break;
            }
            // 更新边界
            ctrl->UpdateMarginalWidth();
        }
        // 退出检查
        {
            // 计算
            const float latest_width = caculate_container_width();
            const float latest_height = caculate_container_height();
            // 一样就退出
            if (latest_width == this_container_width && latest_height == this_container_height) {
                break;
            }
            // 修改外边距
            force_cast(this->margin_rect.left) = m_orgMargin.left
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Left]);
            force_cast(this->margin_rect.top) = m_orgMargin.top
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Top]);
            force_cast(this->margin_rect.right) = m_orgMargin.right
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Right]);
            force_cast(this->margin_rect.bottom) = m_orgMargin.bottom
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Bottom]);
            // 修改大小
            this->SetWidth(this_container_width);
            this->SetHeight(this_container_height);
        }
    }
}


// UI容器: 刷新
void LongUI::UIContainer::Update() noexcept {
    // 修改边界
    if (this->IsControlSizeChanged()) {
        // 刷新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            this->update_marginal_controls();
        }
        // 刷新
        /*if (should_update) {
            this->SetControlWorldChanged();
            this->Update();
        }*/
    }
    // 修改可视化区域
    if (this->IsNeedRefreshWorld()) {
        // 更新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            for (auto ctrl : this->marginal_control) {
                // 刷新
                if (ctrl) {
                    ctrl->Update();
                    // 更新世界矩阵
                    ctrl->SetControlWorldChanged();
                    ctrl->RefreshWorldMarginal();
                    // 坐标转换
                    D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
                    auto lt = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.left));
                    auto rb = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.right));
                    // 修改可视区域
                    ctrl->visible_rect.left = std::max(lt.x, this->visible_rect.left);
                    ctrl->visible_rect.top = std::max(lt.y, this->visible_rect.top);
                    ctrl->visible_rect.right = std::min(rb.x, this->visible_rect.right);
                    ctrl->visible_rect.bottom = std::min(rb.y, this->visible_rect.bottom);
                    //UIManager << DL_Hint << ctrl->visible_rect << endl;
                }
            }
        }
        // 本容器内容限制
        D2D1_RECT_F limit_of_this = {
            this->visible_rect.left + this->margin_rect.left * this->world._11,
            this->visible_rect.top + this->margin_rect.top * this->world._22,
            this->visible_rect.right - this->margin_rect.right * this->world._11,
            this->visible_rect.bottom - this->margin_rect.bottom * this->world._22,
        };
        // 更新一般控件
        for (auto ctrl : (*this)) {
            // 更新世界矩阵
            ctrl->SetControlWorldChanged();
            ctrl->RefreshWorld();
            // 坐标转换
            D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
            auto lt = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.left));
            auto rb = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.right));
            // 限制
            /*if (ctrl->IsCanbeCastedTo(LongUI::GetIID<UIVerticalLayout>())) {
            int bk = 9;
            }*/
            ctrl->visible_rect.left = std::max(lt.x, limit_of_this.left);
            ctrl->visible_rect.top = std::max(lt.y, limit_of_this.top);
            ctrl->visible_rect.right = std::min(rb.x, limit_of_this.right);
            ctrl->visible_rect.bottom = std::min(rb.y, limit_of_this.bottom);
            // 调试信息
            //UIManager << DL_Hint << ctrl << ctrl->visible_rect << endl;
        }
        // 调试信息
        if (this->IsTopLevel()) {
            //UIManager << DL_Log << "Handle: ControlSizeChanged" << LongUI::endl;
        }
        // 已处理该消息
        this->ControlSizeChangeHandled();
    }
    // 刷新边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
#if 1
        for (auto ctrl : this->marginal_control) {
            if (ctrl) ctrl->Update();
        }
#else
    {
        UIMarginalable* ctrl = nullptr;
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Left])) ctrl->Update();
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Top])) ctrl->Update();
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Right])) ctrl->Update();
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Bottom])) ctrl->Update();
    }
#endif
    }
    this->AssertMarginalControl();
    // 刷新一般子控件
    for (auto ctrl : (*this)) ctrl->Update();
    // 刷新父类
    return Super::Update();
}


// UIContainer 重建
auto LongUI::UIContainer::Recreate() noexcept ->HRESULT {
    auto hr = S_OK;
    // 重建边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl && SUCCEEDED(hr)) {
                hr = ctrl->Recreate();
            }
        }
    }
    this->AssertMarginalControl();
    // 重建子类
    for (auto ctrl : (*this)) {
        if (SUCCEEDED(hr)) {
            hr = ctrl->Recreate();
        }
    }
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    return hr;
}

// 设置水平偏移值
LongUINoinline void LongUI::UIContainer::SetOffsetX(float value) noexcept {
    assert(value > -1'000'000.f && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    register float target = value;
    if (target != m_2fOffset.x) {
        m_2fOffset.x = target;
        this->SetControlWorldChanged();
    }
}

// 设置垂直偏移值
LongUINoinline void LongUI::UIContainer::SetOffsetY(float value) noexcept {
    assert(value > (-1'000'000.f) && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    register float target = value ;
    if (target != m_2fOffset.y) {
        m_2fOffset.y = target;
        this->SetControlWorldChanged();
    }
}

// 获取指定控件
auto LongUI::UIContainer::at(uint32_t i) const noexcept -> UIControl * {
    // 性能警告
    UIManager << DL_Warning
        << L"Performance Warning! random accessig is not fine for list"
        << LongUI::endl;
    // 检查范围
    if (i >= this->size()) {
        UIManager << DL_Error << L"out of range" << LongUI::endl;
        return nullptr;
    }
    // 只有一个?
    if (this->size() == 1) return m_pHead;
    // 前半部分?
    UIControl * control;
    if (i < this->size() / 2) {
        control = m_pHead;
        while (i) {
            assert(control && "null pointer");
            control = control->next;
            --i;
        }
    }
    // 后半部分?
    else {
        control = m_pTail;
        i = static_cast<uint32_t>(this->size()) - i - 1;
        while (i) {
            assert(control && "null pointer");
            control = control->prev;
            --i;
        }
    }
    return control;
}

// 插入控件
void LongUI::UIContainer::insert(Iterator itr, UIControl* ctrl) noexcept {
    const auto end_itr = this->end();
    assert(ctrl && "bad arguments");
    if (ctrl->prev) {
        UIManager << DL_Warning
            << L"the 'prev' attr of the control: ["
            << ctrl->GetNameStr()
            << "] that to insert is not null"
            << LongUI::endl;
    }
    if (ctrl->next) {
        UIManager << DL_Warning
            << L"the 'next' attr of the control: ["
            << ctrl->GetNameStr()
            << "] that to insert is not null"
            << LongUI::endl;
    }
    // 插入尾部?
    if (itr == end_itr) {
        // 链接
        force_cast(ctrl->prev) = m_pTail;
        // 无尾?
        if (m_pTail) force_cast(m_pTail->next) = ctrl;
        // 无头?
        if (!m_pHead) m_pHead = ctrl;
        // 设置尾
        m_pTail = ctrl;
    }
    else {
        force_cast(ctrl->next) = itr.Ptr();
        force_cast(ctrl->prev) = itr->prev;
        // 前面->next = ctrl
        // itr->prev = ctrl
        if (itr->prev) {
            force_cast(itr->prev) = ctrl;
        }
        force_cast(itr->prev) = ctrl;
    }
    ++m_cChildrenCount;
    // 添加之后的处理
    this->AfterInsert(ctrl);
}


// 移除控件
bool LongUI::UIContainer::remove(Iterator itr) noexcept {
    // 检查是否属于本容器
#ifdef _DEBUG
    bool ok = false;
    for (auto i : (*this)) {
        if (itr == i) {
            ok = true;
            break;
        }
    }
    if (!ok) {
        UIManager << DL_Error << "control:[" << itr->GetNameStr()
            << "] not in this container: " << this->GetNameStr() << LongUI::endl;
        return false;
    }
#endif
    // 连接前后节点
    register auto prev_tmp = itr->prev;
    register auto next_tmp = itr->next;
    // 检查, 头
    (prev_tmp ? force_cast(prev_tmp->next) : m_pHead) = next_tmp;
    // 检查, 尾
    (next_tmp ? force_cast(next_tmp->prev) : m_pTail) = prev_tmp;
    // 减少
    force_cast(itr->prev) = force_cast(itr->next) = nullptr;
    --m_cChildrenCount;
    // 修改
    this->SetControlSizeChanged();
    return true;
}
