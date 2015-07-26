#include "LongUI.h"


// -------------------------- UIContainer -------------------------
// UIContainer 构造函数
LongUI::UIContainer::UIContainer(pugi::xml_node node) noexcept : Super(node) {
    ::memset(marginal_control, 0, sizeof(marginal_control));
    assert(node && "bad argument.");
    // 保留原始外间距
    m_orgMargin = this->margin_rect;
    uint32_t flag = this->flags | Flag_UIContainer;
    // 检查边缘控件
    {
        // 属性名字
        const char* const attname[] = {
            LongUI::XMLAttribute::LeftMarginalControl,
            LongUI::XMLAttribute::TopMarginalControl,
            LongUI::XMLAttribute::RightMarginalControl,
            LongUI::XMLAttribute::bottomMarginalControl,
        };
        // 属性id
        const char* const templateid[] = {
            LongUI::XMLAttribute::LeftMarginalCtrlTid,
            LongUI::XMLAttribute::TopMarginalCtrlTid,
            LongUI::XMLAttribute::RightMarginalCtrlTid,
            LongUI::XMLAttribute::bottomMarginalCtrlTid,
        };
        bool exist_marginal_control = false;
        for (auto i = 0u; i < UIMarginalable::MARGINAL_CONTROL_SIZE; ++i) {
            const char* str = nullptr;
            if ((str = node.attribute(attname[i]).value())) {
                auto create_control_func = UIManager.GetCreateFunc(str);
                assert(create_control_func && "none");
                if (create_control_func) {
                    // 检查模板ID
                    auto tid = LongUI::AtoI(node.attribute(templateid[i]).value());
                    // 创建控件
                    auto control = UIManager.CreateControl(size_t(tid), create_control_func);
                    // XXX: 检查
                    this->marginal_control[i] = static_cast<UIMarginalable*>(control);
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
    }
    // 渲染依赖属性
    if ((this->flags & Flag_RenderParent) || 
        node.attribute(LongUI::XMLAttribute::IsRenderChildrenD).as_bool(false)) {
        flag |= LongUI::Flag_Container_AlwaysRenderChildrenDirectly;
    }
    force_cast(this->flags) = LongUIFlag(flag);
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
        while (ctrl) { auto next_ctrl = ctrl->next; ctrl->Cleanup(); ctrl = next_ctrl; }
    }
}

// 插入后处理
void LongUI::UIContainer::AfterInsert(UIControl* child) noexcept {
    assert(child && "bad argument");
    // 检查flag
    if (this->flags & Flag_Container_AlwaysRenderChildrenDirectly) {
        force_cast(child->flags) = LongUIFlag(child->flags | Flag_RenderParent);
    }
    // 设置父类
    force_cast(child->parent) = this;
    // 设置窗口节点
    child->m_pWindow = m_pWindow;
    // 重建资源
    child->Recreate(m_pRenderTarget);
    // 修改
    child->SetControlSizeChanged();
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
    auto do_render = [](ID2D1RenderTarget* const target, const UIControl* const ctrl) {
        // 可渲染?
        if (ctrl->visible) {
            // 修改世界转换矩阵
            target->SetTransform(&ctrl->world);
            // 检查剪切规则
            if (ctrl->flags & Flag_ClipStrictly) {
                D2D1_RECT_F clip_rect; ctrl->GetRectAll(clip_rect);
                target->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
            ctrl->Render(LongUI::RenderType::Type_Render);
            // 检查剪切规则
            if (ctrl->flags & Flag_ClipStrictly) {
                target->PopAxisAlignedClip();
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
            m_pRenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        // 渲染所有子部件
        for(const auto* ctrl : (*this)) {
            do_render(m_pRenderTarget, ctrl);
        }
        // 弹出
        m_pRenderTarget->PopAxisAlignedClip();
        // 渲染边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            for (auto ctrl : this->marginal_control) {
                if (ctrl) {
                    //do_render(m_pRenderTarget, ctrl);
                }
            }
        }
        this->AssertMarginalControl();
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// 中转路由表
static const LongUI::UIMarginalable::MarginalControl UICONTAINER_MARGINAL_CONTROL_ROUTER[] = {
    // 左右边缘: 顶底
    LongUI::UIMarginalable::Control_Top,     LongUI::UIMarginalable::Control_Bottom,
    // 顶底边缘: 左右
    LongUI::UIMarginalable::Control_Left,    LongUI::UIMarginalable::Control_Right,
    /*// 右边缘: 顶底
    LongUI::UIMarginalable::Control_Top,     LongUI::UIMarginalable::Control_Bottom,
    // 底边缘: 左右
    LongUI::UIMarginalable::Control_Left,    LongUI::UIMarginalable::Control_Right,*/
};

// 刷新边缘控件
void LongUI::UIContainer::UIContainer::update_marginal_controls() noexcept {
    // 循环
    // XXX: 优化
    for (auto i = 0u; i < lengthof(this->marginal_control); ++i) {
        auto ctrl = this->marginal_control[i];
        if (!ctrl) continue;
        float cross[] = { 0.f, 0.f };
        // TODO: 计算cross 大小
        // 更新边界
        ctrl->UpdateCrossArea(cross);
        // 更新边界
        // 更新世界矩阵
        ctrl->RefreshWorld();
        // 坐标转换
        D2D1_RECT_F clip_rect; ctrl->GetRectAll(clip_rect);
        auto lt = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.left));
        auto rb = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.right));
        // 修改可视区域
        ctrl->visible_rect.left = std::max(lt.x, this->visible_rect.left);
        ctrl->visible_rect.top = std::max(lt.y, this->visible_rect.top);
        ctrl->visible_rect.right = std::min(rb.x, this->visible_rect.right);
        ctrl->visible_rect.bottom = std::min(rb.y, this->visible_rect.bottom);
    }
}


// UI容器: 刷新
void LongUI::UIContainer::Update() noexcept  {
    // 修改可视化区域
    if (this->IsControlSizeChanged()) {
        // 刷新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            this->update_marginal_controls();
            // 更新
            for (auto ctrl : this->marginal_control) {
                if(ctrl)  ctrl->Update();
            }
        }
        this->AssertMarginalControl();
        // 本容器内容限制
        D2D1_RECT_F limit_of_this = {
            this->visible_rect.left + this->margin_rect.left * this->world._11,
            this->visible_rect.top + this->margin_rect.top * this->world._22,
            this->visible_rect.right - this->margin_rect.right * this->world._11,
            this->visible_rect.bottom - this->margin_rect.bottom * this->world._22,
        };
        // 更新
        for (auto ctrl : (*this)) {
            // 更新世界矩阵
            ctrl->RefreshWorld();
            // 坐标转换
            D2D1_RECT_F clip_rect; ctrl->GetRectAll(clip_rect);
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
    // 刷新
    for (auto ctrl : (*this)) ctrl->Update();
    // 刷新父类
    return Super::Update();
}


// UIContainer 重建
auto LongUI::UIContainer::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    auto hr = S_OK;
    // 重建边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl && SUCCEEDED(hr)) {
                hr = ctrl->Recreate(newRT);
            }
        }
    }
    this->AssertMarginalControl();
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate(newRT);
    }
    return hr;
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
        i = this->size() - i - 1;
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
        if(m_pTail) force_cast(m_pTail->next) = ctrl;
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


// -------------------------- UIVerticalLayout -------------------------
// UIVerticalLayout 创建
auto LongUI::UIVerticalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIVerticalLayout>(
            node,
            [=](void* p) noexcept { new(p) UIVerticalLayout(node); }
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

// 更新子控件布局
void LongUI::UIVerticalLayout::Update() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    if (this->IsControlSizeChanged()) {
        // 初始化
        float base_width = 0.f, base_height = 0.f;
        float counter = 0.0f;
        /*if (m_strControlName == L"MainWindow") {
            int a = 0;
        }*/
        // 第一次
        for (auto ctrl : (*this)) {
            // 非浮点控件
            if (!(ctrl->flags & Flag_Floating)) {
                // 宽度固定?
                if (ctrl->flags & Flag_ViewWidthFixed) {
                    base_width = std::max(base_width, ctrl->GetTakingUpWidth());
                }
                // 高度固定?
                if (ctrl->flags & Flag_ViewHeightFixed) {
                    base_height += ctrl->GetTakingUpHeight();
                }
                // 未指定高度?
                else {
                    counter += 1.f;
                }
            }
        }
        // 校正
        base_width /= this->zoom.width;
        base_height /= this->zoom.height;
        // 计算
        base_width = std::max(base_width, this->view_size.width);
        // 高度步进
        float height_step = counter > 0.f ? (this->view_size.height - base_height) / counter : 0.f;
        float position_y = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 浮点控
            if (ctrl->flags & Flag_Floating) continue;
            // 设置控件宽度
            if (!(ctrl->flags & Flag_ViewWidthFixed)) {
                ctrl->SetTakingUpWidth(base_width);
            }
            // 设置控件高度
            if (!(ctrl->flags & Flag_ViewHeightFixed)) {
                ctrl->SetTakingUpHeight(height_step);
            }
            // 容器?
            // 不管如何, 修改!
            ctrl->SetControlSizeChanged();
            // 修改
            ctrl->view_pos.y = position_y;
            position_y += ctrl->GetTakingUpHeight();
        }
        // 修改
        this->content_size.width = base_width;
        this->content_size.height = position_y;
        /*if (m_strControlName == L"MainWindow") {
            int a = 0;
        }*/
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 父类刷新
    return Super::Update();
}


// UIVerticalLayout 重建
auto LongUI::UIVerticalLayout::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    HRESULT hr = S_OK;
    for (auto ctrl : (*this)) {
        hr = ctrl->Recreate(newRT);
        AssertHR(hr);
    }
    return Super::Recreate(newRT);
}

// UIVerticalLayout 关闭控件
void LongUI::UIVerticalLayout::Cleanup() noexcept {
    delete this;
}

// -------------------------- UIHorizontalLayout -------------------------
// UIHorizontalLayout 创建
auto LongUI::UIHorizontalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIHorizontalLayout>(
            node,
            [=](void* p) noexcept { new(p) UIHorizontalLayout(node); }
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


// 更新子控件布局
void LongUI::UIHorizontalLayout::Update() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    if (this->IsControlSizeChanged()) {
        // 初始化
        float base_width = 0.f, base_height = 0.f;
        float counter = 0.0f;
        // 第一次
        for (auto ctrl : (*this)) {
            // 非浮点控件
            if (!(ctrl->flags & Flag_Floating)) {
                // 高度固定?
                if (ctrl->flags & Flag_ViewHeightFixed) {
                    base_height = std::max(base_height, ctrl->GetTakingUpHeight());
                }
                // 宽度固定?
                if (ctrl->flags & Flag_ViewWidthFixed) {
                    base_width += ctrl->GetTakingUpWidth();
                }
                // 未指定宽度?
                else {
                    counter += 1.f;
                }
            }
        }
        // 校正
        base_width /= this->zoom.width;
        base_height /= this->zoom.height;
        // 计算
        base_height = std::max(base_height, this->view_size.height);
        // 宽度步进
        float width_step = counter > 0.f ? (this->view_size.width - base_width) / counter : 0.f;
        float position_x = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 跳过浮动控件
            if (ctrl->flags & Flag_Floating) continue;
            // 设置控件高度
            if (!(ctrl->flags & Flag_ViewHeightFixed)) {
                ctrl->SetTakingUpHeight(base_height);
            }
            // 设置控件宽度
            if (!(ctrl->flags & Flag_ViewWidthFixed)) {
                ctrl->SetTakingUpWidth(width_step);
            }
            // 不管如何, 修改!
            ctrl->SetControlSizeChanged();
            ctrl->view_pos.x = position_x;
            position_x += ctrl->GetTakingUpWidth();
        }
        // 修改
        this->content_size.width = position_x;
        this->content_size.height = base_height;
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 父类刷新
    return Super::Update();
}

// UIHorizontalLayout 重建
auto LongUI::UIHorizontalLayout::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    auto hr = S_OK;
    if (newRT) {
        for (auto ctrl : (*this)) {
            hr = ctrl->Recreate(newRT);
            AssertHR(hr);
        }
    }
    return Super::Recreate(newRT);
}

// UIHorizontalLayout 关闭控件
void LongUI::UIHorizontalLayout::Cleanup() noexcept {
    delete this;
}

