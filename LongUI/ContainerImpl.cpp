#include "LongUI.h"


//#define LONGUI_RECHECK_LAYOUT

// -------------------------- UIContainer -------------------------
// UIContainer 构造函数
LongUI::UIContainer::UIContainer(pugi::xml_node node) noexcept : Super(node) {
    assert(node && "bad argument.");
    // 检查滚动条
    {
        register auto vscrollbar = node.attribute("vscrollbar").value();
        if (vscrollbar) {
            m_pCreateV = UIManager.GetCreateFunc(vscrollbar);
        }
    }
    {
        register auto hscrollbar = node.attribute("hscrollbar").value();
        if (hscrollbar) {
            m_pCreateH = UIManager.GetCreateFunc(hscrollbar);
        }
    }
    uint32_t flag = this->flags | Flag_UIContainer;
    if ((this->flags & Flag_RenderParent) || node.attribute("rendercd").as_bool(false)) {
        flag |= LongUI::Flag_Container_AlwaysRenderChildrenDirectly;
    }
    force_cast(this->flags) = LongUIFlag(flag);
}

// UIContainer 析构函数
LongUI::UIContainer::~UIContainer() noexcept {
    // 关闭滚动条
    if (this->scrollbar_h) {
        this->scrollbar_h->Close();
        this->scrollbar_h = nullptr;
    }
    if (this->scrollbar_v) {
        this->scrollbar_v->Close();
        this->scrollbar_v = nullptr;
    }
    // 关闭子控件
    for (auto itr = this->begin(); itr != this->end(); ) {
        auto itr_next = itr; ++itr_next;
        itr->Close();
        itr = itr_next;
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
    child->DrawPosChanged();
    child->DrawSizeChanged();
    this->DrawSizeChanged();
}

// 压入剪切区
void LongUI::UIContainer::PushAxisAlignedClip(D2D1_ANTIALIAS_MODE mode) const noexcept {
    // 排除滚动条
    /*D2D1_RECT_F rect = {
        this->show_zone.left,
        this->show_zone.top ,
        this->show_zone.left + this->show_zone.width,
        this->show_zone.top + this->show_zone.height
    };*/
    D2D1_RECT_F rect = { 0.f, 0.f, this->width, this->height};
    if (this->scrollbar_h) {
        rect.bottom -= this->scrollbar_h->GetTakingUpSapce();
    }
    if (this->scrollbar_v) {
        rect.right -= this->scrollbar_v->GetTakingUpSapce();
    }
    //
    m_pRenderTarget->PushAxisAlignedClip(&rect, mode);
}

// 更新布局
void LongUI::UIContainer::refresh_child_layout() noexcept {
    // 检查宽度
    if (this->scrollbar_h) {
        /*this->scrollbar_h->show_zone.left = 0.f;
        this->scrollbar_h->show_zone.top = this->show_zone.height - this->scrollbar_h->GetTakingUpSapce();
        this->scrollbar_h->show_zone.width = this->show_zone.width;
        this->scrollbar_h->show_zone.height = this->scrollbar_h->GetTakingUpSapce();
        this->scrollbar_h->draw_zone = this->scrollbar_h->show_zone;*/
        this->scrollbar_h->Refresh();
    }
    // 检查高度
    if (this->scrollbar_v) {
        /*this->scrollbar_v->show_zone.left = this->show_zone.width - this->scrollbar_v->GetTakingUpSapce();
        this->scrollbar_v->show_zone.top = 0.f;
        this->scrollbar_v->show_zone.width = this->scrollbar_v->GetTakingUpSapce();
        this->scrollbar_v->show_zone.height = this->show_zone.height;
        this->scrollbar_v->draw_zone = this->scrollbar_v->show_zone;*/
        this->scrollbar_v->Refresh();
    }

}

// UIContainer 保证滚动条
bool LongUI::UIContainer::AssureScrollBar(float basew, float baseh) noexcept {
#ifdef LONGUI_RECHECK_LAYOUT
    auto check = [](UIScrollBar* bar) ->uint8_t {
        return bar && bar->GetTakingUpSapce() > 0.f ? 1 : 0;
    };
    // 检查
    uint8_t need_refresh = ((check(this->scrollbar_h) << 1) | check(this->scrollbar_v));
#endif
    // 水平滚动条
    {
        auto needed = basew > this->GetTakingUpWidth();
        if (!this->scrollbar_h && m_pCreateH && needed &&
            (this->scrollbar_h = static_cast<UIScrollBar*>(m_pCreateH(LongUINullXMLNode)))) {
            this->scrollbar_h->InitScrollBar(this, ScrollBarType::Type_Horizontal);
            this->AfterInsert(this->scrollbar_h);
        }
        if (this->scrollbar_h) {
            this->scrollbar_h->another = this->scrollbar_v;
            this->scrollbar_h->OnNeeded(needed);
        }
    }
    // 垂直滚动条
    {
        auto needed = baseh > this->GetTakingUpHeight();
        if (!this->scrollbar_v && m_pCreateV && needed &&
            (this->scrollbar_v = static_cast<UIScrollBar*>(m_pCreateV(LongUINullXMLNode)))) {
            this->scrollbar_v->InitScrollBar(this, ScrollBarType::Type_Vertical);
            this->AfterInsert(this->scrollbar_v);
        }
        if (this->scrollbar_v) {
            this->scrollbar_v->another = this->scrollbar_h;
            this->scrollbar_v->OnNeeded(needed);
        }
    }
#ifdef LONGUI_RECHECK_LAYOUT
    return need_refresh != ((check(this->scrollbar_h) << 1) | check(this->scrollbar_v));
#else
    return false;
#endif
}

// UI容器: 查找控件
auto LongUI::UIContainer::FindControl(const D2D1_POINT_2F pt) noexcept->UIControl* {
    UIControl* control_out = nullptr;
    for (auto ctrl : (*this)) {
        if (m_strControlName == L"MainWindow") {
            int a = 9;
        }
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
bool LongUI::UIContainer::DoEvent(LongUI::EventArgument& arg) noexcept {
    // TODO: 参数EventArgument改为const
    bool done = false;
    // 转换坐标
    auto pt_old = arg.pt;
    auto pt4self = arg.pt;// LongUI::TransformPointInverse(this->transform, arg.pt);
    arg.pt = pt4self;
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl:
            if (m_strControlName == L"HLayout") {
                int bk = 9;
            }
            // 检查是否为自己的范围之内
            if (!FindControlHelper(pt4self, this)) {
                return false;
            }
            // 检查滚动条
            if (scrollbar_v && this->width - pt4self.x < scrollbar_v->GetHitSapce()) {
                done = scrollbar_v->DoEvent(arg);
                break;
            }
            if (scrollbar_h && this->height - pt4self.y < scrollbar_h->GetHitSapce()) {
                done = scrollbar_h->DoEvent(arg);
                break;
            }
            // 检查子控件
            if (!done) {
                // XXX: 优化
                for (auto ctrl : (*this)) {
                    ctrl->DoEvent(arg);
                    // 找到
                    if (arg.ctrl) {
                        done = true;
                        break;
                    }
                }
            }
            done = true;
            break;*/
        case LongUI::Event::Event_FinishedTreeBuliding:
            // 初次完成空间树建立
            for (auto ctrl : (*this)) {
                ctrl->DoEvent(arg);
            }
            done = true;
            break;
        }
    }
    // 扳回来
    arg.pt = pt_old;
    return done;
}

// UIContainer 渲染函数
void LongUI::UIContainer::Render(RenderType type) const noexcept {
    //
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
        // 渲染所有子部件
        for(const auto* ctrl : (*this)) {
            // 修改世界转换矩阵
            D2D1_MATRIX_3X2_F matrix; ctrl->GetWorldTransform(matrix);
            m_pRenderTarget->SetTransform(&matrix);
            // 检查剪切规则
            if (ctrl->flags & Flag_StrictClip) {
                D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
                m_pRenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
            ctrl->Render(LongUI::RenderType::Type_Render);
            // 检查剪切规则
            if (ctrl->flags & Flag_StrictClip) {
                m_pRenderTarget->PopAxisAlignedClip();
            }
        }
        // 渲染滚动条
        if (this->scrollbar_h && this->scrollbar_h->GetTakingUpSapce() > 0.f) {
            // 计算高度
            auto height = this->scrollbar_h->GetTakingUpSapce() * this->world._22;
            assert(height != 0.f);
            this->scrollbar_h->visible_rect = this->visible_rect;
            this->scrollbar_h->visible_rect.top = this->visible_rect.bottom - height;
            // 渲染
            this->scrollbar_h->Render(LongUI::RenderType::Type_Render);
        }
        // 渲染滚动条
        if (this->scrollbar_v && this->scrollbar_v->GetTakingUpSapce() > 0.f) {
            // 计算宽度
            auto width = this->scrollbar_v->GetTakingUpSapce() * this->world._11;
            assert(width != 0.f);
            this->scrollbar_v->visible_rect = this->visible_rect;
            this->scrollbar_v->visible_rect.left = this->visible_rect.right - width;
            // 渲染
            this->scrollbar_v->Render(LongUI::RenderType::Type_Render);
        }
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UI容器: 刷新
void LongUI::UIContainer::Update() noexcept  {
    // 检查
    if (m_bDrawPosChanged || m_bDrawSizeChanged) {
        // 计算转变
        auto transform = D2D1::Matrix3x2F::Translation(
            this->margin_rect.left,
            this->margin_rect.top
            );
        // 更新转变
        if (this->parent) {
            this->world = transform * this->parent->world;
        }
        else {
            this->world = transform;
        }
    }
    // 更新子控件布局
    if (m_bDrawSizeChanged) {
        this->refresh_child_layout();
    }
    // 刷新容器
    for (auto ctrl : (*this)) {
        // 更新矩阵
        ctrl->GetWorldTransform(this->world);
        D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
        // 坐标转换
        auto lt = LongUI::TransformPoint(this->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.left));
        auto rb = LongUI::TransformPoint(this->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.right));
        // 修改
        auto tmp_right = this->visible_rect.right;
        auto tmp_bottom = this->visible_rect.bottom;
        if (this->scrollbar_v) {
            tmp_right -= this->scrollbar_v->GetTakingUpSapce();
        }
        if (this->scrollbar_h) {
            tmp_bottom -= this->scrollbar_h->GetTakingUpSapce();
        }
        ctrl->visible_rect.left = std::max(lt.x, 0.f);
        ctrl->visible_rect.top = std::max(lt.y, 0.f);
        ctrl->visible_rect.right = std::min(rb.x, tmp_right);
        ctrl->visible_rect.bottom = std::min(rb.y, tmp_bottom);
        // 刷新
        ctrl->Update();
    }
    // 刷新滚动条
    if (this->scrollbar_h) {
        this->scrollbar_h->Update();
    }
    if (this->scrollbar_v) {
        this->scrollbar_v->Update();
    }
    // 刷新父类
    return Super::Update();
}


// UIContainer 重建
auto LongUI::UIContainer::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    auto hr = S_OK;
    // 重建水平滚动条
    if (SUCCEEDED(hr) && this->scrollbar_h) {
        hr = this->scrollbar_h->Recreate(newRT);
    }
    // 重建垂直滚动条
    if (SUCCEEDED(hr) && this->scrollbar_v) {
        hr = this->scrollbar_v->Recreate(newRT);
    }
    // 重建容器
    if (SUCCEEDED(hr)) {
        for (auto ctrl : (*this)) {
            hr = ctrl->Recreate(newRT);
            if (FAILED(hr)) break;
        }
    }
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate(newRT);
    }
    return hr;
}

// 获取指定控件
auto LongUI::UIContainer::at(uint32_t i) const noexcept -> UIControl * {
    // 性能警告
    UIManager << DL_Warning << L"Performance Warning!"
        L"random accessig is not fine for list" << LongUI::endl;
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
void LongUI::UIContainer::
insert(Iterator itr, UIControl* ctrl) noexcept {
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
    if (itr == this->end()) {
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
        // ctrl->next = itr;
        // ctrl->prev = 前面;
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
    register auto prev = itr->prev;
    register auto next = itr->next;
    // 检查
    if (prev) {
        force_cast(prev->next) = next;
    }
    // 首部
    else {
        m_pHead = next;
    }
    // 检查
    if (next) {
        force_cast(next->prev) = prev;
    }
    // 尾部
    else {
        m_pTail = prev;
    }
    // 减少
    force_cast(itr->prev) = nullptr;
    force_cast(itr->next) = nullptr;
    --m_cChildrenCount;
    // 修改
    this->DrawSizeChanged();
    return true;
}


// -------------------------- UIVerticalLayout -------------------------
// UIVerticalLayout 创建
auto LongUI::UIVerticalLayout::CreateControl(pugi::xml_node node) noexcept ->UIControl* {
    if (!node) {
        UIManager << DL_Warning << L"node null" << LongUI::endl;
    }
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UIVerticalLayout>(
        node,
        [=](void* p) noexcept { new(p) UIVerticalLayout(node);}
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}

// 更新子控件布局
void LongUI::UIVerticalLayout::Update() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度, 修改show_zone, 更新滚动条, 尽量最小化改动
    if (m_bDrawSizeChanged) {
        // 初始化
        float base_width = 0.f, base_height = 0.f;
        float counter = 0.0f;
        if (m_strControlName == L"MainWindow") {
            int a = 0;
        }
        // 第一次
        for (auto ctrl : (*this)) {
            // 非浮点控件
            if (!(ctrl->flags & Flag_Floating)) {
                // 宽度固定?
                if (ctrl->flags & Flag_WidthFixed) {
                    base_width = std::max(base_width, ctrl->GetTakingUpWidth());
                }
                // 高度固定?
                if (ctrl->flags & Flag_HeightFixed) {
                    base_height += ctrl->GetTakingUpHeight();
                }
                // 未指定高度?
                else {
                    counter += 1.f;
                }
            }
        }
        // 计算
        base_width = std::max(base_width, this->width);
        // 保证滚动条
#ifdef LONGUI_RECHECK_LAYOUT
        auto need_refresh =
#endif
            this->AssureScrollBar(base_width, base_height);
            // 垂直滚动条?
        if (this->scrollbar_v) {
            base_width -= this->scrollbar_v->GetTakingUpSapce();
        }
        // 水平滚动条?
        if (this->scrollbar_h) {
            base_height -= this->scrollbar_h->GetTakingUpSapce();
        }
        // 高度步进
        float height_step = counter > 0.f ? (this->height - base_height) / counter : 0.f;
        float position_y = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 浮点控
            if (ctrl->flags & Flag_Floating) continue;
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                register auto old_width = ctrl->width;
                ctrl->width = base_width - ctrl->margin_rect.left - ctrl->margin_rect.right;
            }
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                ctrl->height = height_step - ctrl->margin_rect.top - ctrl->margin_rect.bottom;
            }
            // 修改
            ctrl->DrawSizeChanged();
            ctrl->DrawPosChanged();
            ctrl->y = position_y;
            position_y += ctrl->GetTakingUpHeight();
        }
        // 修改
        force_cast(this->end_of_right) = base_width;
        force_cast(this->end_of_bottom) = position_y;
#ifdef LONGUI_RECHECK_LAYOUT
    // 需要刷新?
        if (need_refresh) {
            return this->refresh_child_layout();
        }
#endif
        if (m_strControlName == L"MainWindow") {
            int a = 0;
        }
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
void LongUI::UIVerticalLayout::Close() noexcept {
    delete this;
}

// -------------------------- UIHorizontalLayout -------------------------
// UIHorizontalLayout 创建
auto LongUI::UIHorizontalLayout::CreateControl(pugi::xml_node node) noexcept ->UIControl* {
    if (!node) {
        UIManager << DL_Warning << L"node null" << LongUI::endl;
    }
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UIHorizontalLayout>(
        node,
        [=](void* p) noexcept { new(p) UIHorizontalLayout(node);}
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}


// 更新子控件布局
void LongUI::UIHorizontalLayout::Update() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度, 修改show_zone, 更新滚动条, 尽量最小化改动
    if (m_bDrawSizeChanged) {
    // 初始化
        float base_width = 0.f, base_height = 0.f;
        float counter = 0.0f;
        // 第一次
        for (auto ctrl : (*this)) {
            // 非浮点控件
            if (!(ctrl->flags & Flag_Floating)) {
                // 高度固定?
                if (ctrl->flags & Flag_HeightFixed) {
                    base_height = std::max(base_height, ctrl->GetTakingUpHeight());
                }
                // 宽度固定?
                if (ctrl->flags & Flag_WidthFixed) {
                    base_width += ctrl->GetTakingUpWidth();
                }
                // 未指定宽度?
                else {
                    counter += 1.f;
                }
            }
        }
        // 计算
        base_height = std::max(base_height, this->height);
        // 保证滚动条
#ifdef LONGUI_RECHECK_LAYOUT
        auto need_refresh =
#endif
            this->AssureScrollBar(base_width, base_height);
            // 垂直滚动条?
        if (this->scrollbar_v) {
            base_width -= this->scrollbar_v->GetTakingUpSapce();
        }
        // 水平滚动条?
        if (this->scrollbar_h) {
            base_height -= this->scrollbar_h->GetTakingUpSapce();
        }
        // 宽度步进
        float width_step = counter > 0.f ? (this->width - base_width) / counter : 0.f;
        float position_x = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 浮点控
            if (ctrl->flags & Flag_Floating) continue;
            //ctrl->show_zone.top = 0.f;
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                register auto old_height = ctrl->height;
                ctrl->height = base_height - ctrl->margin_rect.left - ctrl->margin_rect.right;
            }
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                ctrl->width = width_step - ctrl->margin_rect.top - ctrl->margin_rect.bottom;
            }
            // 修改
            ctrl->DrawSizeChanged();
            ctrl->DrawPosChanged();
            ctrl->x = position_x;
            position_x += ctrl->GetTakingUpWidth();
        }
        // 修改
        force_cast(this->end_of_right) = position_x;
        force_cast(this->end_of_bottom) = base_height;
#ifdef LONGUI_RECHECK_LAYOUT
    // 需要刷新?
        if (need_refresh) {
            return this->refresh_child_layout();
        }
#endif
    }
    // 父类刷新
    return Super::Update();
}

// UIHorizontalLayout 重建
auto LongUI::UIHorizontalLayout::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    HRESULT hr = S_OK;
    if (newRT) {
        for (auto ctrl : (*this)) {
            hr = ctrl->Recreate(newRT);
            AssertHR(hr);
        }
    }
    return Super::Recreate(newRT);
}

// UIHorizontalLayout 关闭控件
void LongUI::UIHorizontalLayout::Close() noexcept {
    delete this;
}

