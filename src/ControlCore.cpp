#include "LongUI.h"
#include <algorithm>

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
LongUI::UIControl::UIControl(UIContainer* ctrlparent, pugi::xml_node node) 
noexcept :parent(ctrlparent), m_pWindow(ctrlparent ? ctrlparent->GetWindow() : nullptr) {
    // 构造默认
    auto flag = LongUIFlag::Flag_None;
    // 有效?
    if (node) {
        // 调试
#ifdef _DEBUG
        this->debug_this = node.attribute("debug").as_bool(false);
#endif
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
            m_idBackgroudBrush = uint16_t(LongUI::AtoI(data));
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
void LongUI::UIControl::AfterUpdate() noexcept {
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
    if (this->view_size.width < 0.f && this->parent->view_size.width > 0.f) {
        UIManager << DL_Hint << this
            << "viewport's width < 0: " << this->view_size.width 
            << endl;
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
    if (this->view_size.height < 0.f && this->parent->view_size.height > 0.f) {
        UIManager << DL_Hint << this
            << "viewport's height < 0: " << this->view_size.height 
            << endl;
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
#ifdef _DEBUG
    if (this->debug_this) {
        UIManager << DL_Log << this << "WORLD: " << this->world << LongUI::endl;
    }
#endif
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
        auto pp = this->parent;
        xx -= pp->GetLeftMarginOffset();
        yy -= pp->GetTopMarginOffset();
        // 检查
        parent_world = &pp->world;
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
        //virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override { return false; }
        // close this control 关闭控件
        virtual void Cleanup() noexcept override { delete this; }
    public:
        // 创建控件
        static auto CreateControl(UIContainer* ctrlparent, pugi::xml_node node) noexcept {
            UIControl* pControl = nullptr;
            // 判断
            if (!node) {
                UIManager << DL_Warning << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = new(std::nothrow) UINull(ctrlparent, node);
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            return pControl;
        }
    public:
        // constructor 构造函数
        UINull(UIContainer* cp, pugi::xml_node node) noexcept : Super(cp, node) {}
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
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        pControl = UINull::CreateControl(reinterpret_cast<UIContainer*>(type), node);
    }
    return pControl;
}


// ------------------------------ UIContainer -----------------------------
// UIContainer 构造函数
LongUI::UIContainer::UIContainer(UIContainer* cp, pugi::xml_node node) noexcept : Super(cp, node), marginal_control() {
    ::memset(force_cast(marginal_control), 0, sizeof(marginal_control));
    // LV
    /*if (m_strControlName == L"V") {
        m_2fZoom = { 1.0f, 1.0f };
    }*/
    // 保留原始外间距
    m_orgMargin = this->margin_rect;
    auto flag = this->flags | Flag_UIContainer;
    // 有效
    if (node) {
        // 模板大小
        Helper::MakeFloats( 
            node.attribute(LongUI::XMLAttribute::TemplateSize).value(),
            &m_2fTemplateSize.width, 2
            );
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
                Helper::CC cc = { 0 };
#ifdef _DEBUG
                assert(Helper::MakeCC(str, &cc) == 1);
#else
                Helper::MakeCC(str, &cc);
#endif
                // 有效
                if (cc.func) {
                    // 创建控件
                    auto control = UIManager.CreateControl(this, cc.id, cc.func);
                    // XXX: 检查
                    force_cast(this->marginal_control[i]) = longui_cast<UIMarginalable*>(control);

                }
                else {
                    assert(!"cc.func -> null");
                }
                // 优化flag
                if (this->marginal_control[i]) {
                    // 插入后
                    this->after_insert(this->marginal_control[i]);
                    // 初始化
                    this->marginal_control[i]->InitMarginalControl(static_cast<UIMarginalable::MarginalControl>(i));
                    // 优化flag
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
        // 渲染依赖属性
        if (node.attribute(XMLAttribute::IsHostPosterityAlways).as_bool(false)) {
            flag |= LongUI::Flag_Container_HostPosterityRenderingDirectly;
        }
        // 边缘控件缩放
        if (node.attribute(XMLAttribute::IsZoomMarginalControl).as_bool(true)) {
            flag |= LongUI::Flag_Container_ZoomMarginalControl;
        }
    }
    // 修改完毕
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
}

// 插入后处理
void LongUI::UIContainer::after_insert(UIControl* child) noexcept {
    assert(child && "bad argument");
    // 大小判断
    if (this->GetCount() >= 10'000) {
        UIManager << DL_Warning << "the count of children must be"
            " less than 10k because of the precision of float" << LongUI::endl;
    }
    // 检查flag
    const auto host_flag = Flag_Container_HostChildrenRenderingDirectly 
        | Flag_Container_HostPosterityRenderingDirectly;
    if (this->flags & host_flag) {
        force_cast(child->flags) |= Flag_RenderParent;
    }
    // 子控件也是容器?
    if (this->flags & Flag_Container_HostPosterityRenderingDirectly
        && child->flags & Flag_UIContainer) {
        force_cast(child->flags) |= Flag_Container_HostPosterityRenderingDirectly;
    }
    // 设置父节点
    assert(child->parent == this);
    // 设置窗口节点
    assert(child->m_pWindow == m_pWindow);
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
auto LongUI::UIContainer::FindChild(const D2D1_POINT_2F& pt) noexcept->UIControl* {
    // 查找边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl && IsPointInRect(ctrl->visible_rect, pt)) {
                return ctrl;
            }
        }
    }
    this->AssertMarginalControl();
    return nullptr;
}


// do event 事件处理
bool LongUI::UIContainer::DoEvent(const LongUI::EventArgument& arg) noexcept {
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
                    // 完成控件树
                    ctrl->DoEvent(arg);
                }
            }
            done = true;
            break;
        }
    }
    // 扳回来
    return done;
}

// 处理鼠标事件
bool LongUI::UIContainer::DoMouseEvent(const LongUI::MouseEventArgument& arg) noexcept {
    // 离开
    if (arg.event == LongUI::MouseEvent::Event_MouseLeave) {
        if (m_pMousePointed) {
            m_pMousePointed->DoMouseEvent(arg);
            m_pMousePointed = nullptr;
        }
        return true;
    }
    // 查找子控件
    auto control_got = this->FindChild(arg.pt);
    // 不可视算没有
    if (control_got && !control_got->visible) control_got = nullptr;
    // 不同
    if (control_got != m_pMousePointed && arg.event == LongUI::MouseEvent::Event_MouseMove) {
        auto newarg = arg;
        // 有效
        if (m_pMousePointed) {
            newarg.event = LongUI::MouseEvent::Event_MouseLeave;
            m_pMousePointed->DoMouseEvent(newarg);
        }
        // 有效
        if ((m_pMousePointed = control_got)) {
            newarg.event = LongUI::MouseEvent::Event_MouseEnter;
            m_pMousePointed->DoMouseEvent(newarg);
        }
    }
    // 有效
    if (control_got) {
        // 左键点击设置键盘焦点
        if (arg.event == LongUI::MouseEvent::Event_LButtonDown) {
            m_pWindow->SetFocus(control_got);
        }
        // 鼠标移动设置hover跟踪
        else if (arg.event == LongUI::MouseEvent::Event_MouseMove) {
            m_pWindow->SetHoverTrack(control_got);
        }
        // 相同
        if (control_got->DoMouseEvent(arg)) {
            return true;
        }
    }
    // 滚轮事件允许边缘控件后处理
    if (arg.event <= MouseEvent::Event_MouseWheelH && this->flags & Flag_Container_ExistMarginalControl) {
        // 优化
        for (auto ctrl : this->marginal_control) {
            if (ctrl && ctrl->DoMouseEvent(arg)) {
                return true;
            }
        }
        this->AssertMarginalControl();
    }
    return false;
}

// 渲染子控件
void LongUI::UIContainer::child_do_render(const UIControl* ctrl) noexcept {
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
}

// UIContainer 渲染函数
void LongUI::UIContainer::Render(RenderType type) const noexcept {
    // 查看
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        break;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        // 普通子控件仅仅允许渲染在内容区域上
        /*{
            D2D1_RECT_F clip_rect; this->GetViewRect(clip_rect);
            UIManager_RenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        // 渲染所有子部件
        for (const auto* ctrl : (*this)) {
            do_render(ctrl);
        }
        // 弹出
        UIManager_RenderTarget->PopAxisAlignedClip();*/
        // 渲染边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            for (auto ctrl : this->marginal_control) {
                if (ctrl) {
                    this->child_do_render(ctrl);
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

// 更新边缘控件
void LongUI::UIContainer::refresh_marginal_controls() noexcept {
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
    const float this_container_left = this->view_pos.x - this->GetLeftMarginOffset();
    const float this_container_top = this->view_pos.y - this->GetTopMarginOffset();

    /*if (m_strControlName == L"V") {
        int bk = 9;
    }*/
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
            this->SetLeft(this_container_left);
            this->SetTop(this_container_top);
            this->SetWidth(this_container_width);
            this->SetHeight(this_container_height);
            this->RefreshWorld();
        }
    }
    this->RefreshLayout();
}

/*// UI容器: 刷新前
void LongUI::UIContainer::BeforeUpdateContainer() noexcept {
    // 需要刷新
    if (this->IsNeedRefreshWorld()) {
        auto code = ((this->m_2fTemplateSize.width > 0.f) << 1) | 
            (this->m_2fTemplateSize.height > 0.f);
        auto tmpw = this->GetWidth() / m_2fTemplateSize.width;
        auto tmph = this->GetHeight() / m_2fTemplateSize.width;
        switch (code)
        {
        case 0:
            // do nothing
            break;
        case 1:
            // this->m_2fTemplateSize.height > 0.f, only
            this->m_2fZoom.width = this->m_2fZoom.height = tmph;
            break;
        case 2:
            // this->m_2fTemplateSize.width > 0.f, only
            this->m_2fZoom.height = this->m_2fZoom.width = tmpw;
            break;
        case 3:
            // both
            this->m_2fZoom.width =  tmpw;
            this->m_2fZoom.height = tmph;
            break;
        }
    }
}*/

// UI容器: 刷新
void LongUI::UIContainer::Update() noexcept {
    // 修改自动缩放控件
    if (this->IsNeedRefreshWorld()) {
        auto code = ((this->m_2fTemplateSize.width > 0.f) << 1) | 
            (this->m_2fTemplateSize.height > 0.f);
        auto tmpw = this->GetWidth() / m_2fTemplateSize.width;
        auto tmph = this->GetHeight() / m_2fTemplateSize.width;
        switch (code)
        {
        case 0:
            // do nothing
            break;
        case 1:
            // this->m_2fTemplateSize.height > 0.f, only
            this->m_2fZoom.width = this->m_2fZoom.height = tmph;
            break;
        case 2:
            // this->m_2fTemplateSize.width > 0.f, only
            this->m_2fZoom.height = this->m_2fZoom.width = tmpw;
            break;
        case 3:
            // both
            this->m_2fZoom.width =  tmpw;
            this->m_2fZoom.height = tmph;
            break;
        }
    }
    // 修改边界
    if (this->IsControlSizeChanged()) {
        // 更新布局
        this->RefreshLayout();
        // 刷新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            this->refresh_marginal_controls();
        }
        // 处理
        this->ControlSizeChangeHandled();
        // 刷新
        /*if (should_update) {
        this->SetControlWorldChanged();
        this->Update();
        }*/
#ifdef _DEBUG
        if (this->debug_this) {
            UIManager << DL_Log << L"Container" << this
                << LongUI::Formated(L"Resize(%.1f, %.1f) Zoom(%.1f, %.1f)",
                    this->GetWidth(), this->GetHeight(),
                    m_2fZoom.width, m_2fZoom.height
                    ) << LongUI::endl;
        }
#endif
    }
    // 修改可视化区域
    if (this->IsNeedRefreshWorld()) {
        // 更新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            for (auto ctrl : this->marginal_control) {
                // 刷新
                if (ctrl) {
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
#ifdef _DEBUG
                    if (ctrl->debug_this) {
                        UIManager << DL_Log << ctrl
                            << " visible rect changed to: "
                            << ctrl->visible_rect << endl;
                    }
#endif
                }
            }
        }
        // 已处理该消息
        this->ControlSizeChangeHandled();
    }
    // 刷新父类
    return Super::Update();
}

// UIContainer 重建
auto LongUI::UIContainer::Recreate() noexcept ->HRESULT {
    auto hr = S_OK;
    // 重建边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl) {
                hr = ctrl->Recreate();
                assert(SUCCEEDED(hr));
            }
        }
    }
    this->AssertMarginalControl();
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    return hr;
}

// 设置水平偏移值
void LongUI::UIContainer::SetOffsetX(float value) noexcept {
    assert(value > -1'000'000.f && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    register float target = value;
    if (target != m_2fOffset.x) {
        m_2fOffset.x = target;
        this->SetControlWorldChanged();
    }
}

// 设置垂直偏移值
void LongUI::UIContainer::SetOffsetY(float value) noexcept {
    assert(value > (-1'000'000.f) && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    register float target = value ;
    if (target != m_2fOffset.y) {
        m_2fOffset.y = target;
        this->SetControlWorldChanged();
    }
}


// ------------------------ HELPER ---------------------------

bool LongUI::UIControl::subevent_call_helper(const UICallBack& call, SubEvent sb) noexcept(noexcept(call.operator())) {
    // 事件
    LongUI::EventArgument arg;
    arg.event = LongUI::Event::Event_SubEvent;
    arg.sender = this;
    arg.ui.subevent = sb;
    arg.ui.pointer = nullptr;
    arg.ctrl = nullptr;
    // 脚本优先
    if (UIManager.script && m_script.script) {
        return UIManager.script->Evaluation(this->GetScript(), arg);
    }
    // 回调其次
    if (call.IsOK()) {
        return call(this);
    }
    // 事件最低
    return m_pWindow->DoEvent(arg);
}