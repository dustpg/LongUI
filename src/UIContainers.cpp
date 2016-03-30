#include "Core/luiManager.h"
#include "LongUI/luiUiXml.h"
#include "Control/UIFloatLayout.h"
#include "Control/UISingle.h"
#include "Control/UIPage.h"
#include "Control/UILinearLayout.h"
#include <algorithm>

// ------------------------- UIContainerBuiltIn ------------------------
// UIContainerBuiltIn: 事件处理
bool LongUI::UIContainerBuiltIn::DoEvent(const LongUI::EventArgument& arg) noexcept {
    assert(arg.sender && "bad argument");
    // 处理窗口事件
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBulidingFinished:
        // 初始化边缘控件 
        Super::DoEvent(arg);
        // 初次完成空间树建立
        for (auto ctrl : (*this)) {
            ctrl->DoEvent(arg);
        }
        return true;
    case LongUI::Event::Event_SetNewParent:
        // 初始化边缘控件 
        Super::DoEvent(arg);
        // 修改控件深度
        for (auto ctrl : (*this)) {
            ctrl->NewParentSetted();
            ctrl->DoEvent(arg);
        }
        return true;
        /*case LongUI::Event::Event_NotifyChildren:
            // 仅仅传递一层
            if (arg.sender->parent == this) {
                for (auto ctrl : (*this)) {
                    if (ctrl != arg.sender) ctrl->DoEvent(arg);
                }
            }
            return true;*/
    }
    return Super::DoEvent(arg);
}

// UIContainerBuiltIn: 主景渲染
inline void LongUI::UIContainerBuiltIn::render_chain_main() const noexcept {
    // 渲染帮助器
    Super::RenderHelper(UIManager_RenderTarget, this->begin(), this->end());
    // 父类主景
    Super::render_chain_main();
}

// UIContainerBuiltIn: 渲染函数
void LongUI::UIContainerBuiltIn::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// LongUI内建容器: 刷新
void LongUI::UIContainerBuiltIn::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(this->begin(), this->end());
}


// UIContainerBuiltIn: 重建
auto LongUI::UIContainerBuiltIn::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    for (auto ctrl : (*this)) {
        hr = ctrl->Recreate();
        assert(SUCCEEDED(hr));
    }
    return Super::Recreate();
}


/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIContainerBuiltIn::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
    // 父类(边缘控件)
    auto mctrl = Super::FindChild(pt);
    if (mctrl) return mctrl;
    // 性能警告
#ifdef _DEBUG
    if (this->GetChildrenCount() > 100) {
        UIManager << DL_Warning
            << "Performance Warning: O(n) algorithm"
            << " is not fine for container that over 100 children"
            << LongUI::endl;
    }
#endif
    for (auto ctrl : (*this)) {
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            return ctrl;
        }
    }
    return nullptr;
}


// UIContainerBuiltIn: 推入♂最后
void LongUI::UIContainerBuiltIn::Push(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::Push(child);
    }
    // 一般的就自己处理
    else {
        this->Insert(this->end(), child);
    }
}

// UIContainerBuiltIn: 仅插入控件
void LongUI::UIContainerBuiltIn::insert_only(Iterator itr, UIControl* ctrl) noexcept {
    const auto end_itr = this->end();
    assert(ctrl && "bad arguments");
    if (ctrl->prev) {
        UIManager << DL_Warning
            << L"the 'prev' attr of the control: ["
            << ctrl->name
            << "] that to insert is not null"
            << LongUI::endl;
    }
    if (ctrl->next) {
        UIManager << DL_Warning
            << L"the 'next' attr of the control: ["
            << ctrl->name
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
        // 一般般
        force_cast(ctrl->next) = itr.Ptr();
        force_cast(ctrl->prev) = itr->prev;
        if (itr->prev) {
            force_cast(itr->prev) = ctrl;
        }
        force_cast(itr->prev) = ctrl;
    }
    ++m_cChildrenCount;
}


// UIContainerBuiltIn: 移除控件
void LongUI::UIContainerBuiltIn::Remove(UIControl* ctrl) noexcept {
    // 检查是否属于本容器
#ifdef _DEBUG
    bool ok = false;
    for (auto i : (*this)) {
        if (ctrl == i) {
            ok = true;
            break;
        }
    }
    if (!ok) {
        UIManager << DL_Error
            << "control:["
            << ctrl->name
            << "] not in this container: "
            << this->name
            << LongUI::endl;
        return;
    }
#endif
    {
        // 连接前后结点
        auto prev_tmp = ctrl->prev;
        auto next_tmp = ctrl->next;
        // 检查, 头
        (prev_tmp ? force_cast(prev_tmp->next) : m_pHead) = next_tmp;
        // 检查, 尾
        (next_tmp ? force_cast(next_tmp->prev) : m_pTail) = prev_tmp;
        // 减少
        force_cast(ctrl->prev) = force_cast(ctrl->next) = nullptr;
        --m_cChildrenCount;
        // 修改
        this->SetControlLayoutChanged();
    }
    // 父类处理
    Super::Remove(ctrl);
}

/// <summary>
/// Before_deleteds this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIContainerBuiltIn::before_deleted() noexcept {
    // 清理子控件
    auto ctrl = m_pHead;
    while (ctrl) {
        auto next_ctrl = ctrl->next;
        this->release_child(ctrl);
        ctrl = next_ctrl;
    }
#ifdef _DEBUG
    // 调试清理
    m_pHead = nullptr;
    m_pTail = nullptr;
#endif
    // 链式调用
    Super::before_deleted();
}

// 获取控件索引
auto LongUI::UIContainerBuiltIn::GetIndexOf(UIControl* child) const noexcept ->uint32_t {
    assert(this == child->parent && "不是亲生的");
    uint32_t index = 0;
    for (auto ctrl : (*this)) {
        if (ctrl == child) break;
        ++index;
    }
    return index;
}

// 随机访问控件
auto LongUI::UIContainerBuiltIn::GetAt(uint32_t i) const noexcept -> UIControl * {
    // 超出
    if (i >= m_cChildrenCount) return nullptr;
    // 第一个
    if (!i) return m_pHead;
    // 性能警告
    if (i > 8) {
        UIManager << DL_Warning
            << L"Performance Warning! random accessig is not fine for list"
            << LongUI::endl;
    }
    // 检查范围
    if (i >= this->GetChildrenCount()) {
        UIManager << DL_Error << L"out of range" << LongUI::endl;
        return nullptr;
    }
    // 只有一个?
    if (this->GetChildrenCount() == 1) return m_pHead;
    // 前半部分?
    UIControl * control;
    if (i < this->GetChildrenCount() / 2) {
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
        i = static_cast<uint32_t>(this->GetChildrenCount()) - i - 1;
        while (i) {
            assert(control && "null pointer");
            control = control->prev;
            --i;
        }
    }
    return control;
}

// 交换
void LongUI::UIContainerBuiltIn::SwapChild(Iterator itr1, Iterator itr2) noexcept {
    auto ctrl1 = *itr1; auto ctrl2 = *itr2;
    assert(ctrl1 && ctrl2 && "bad arguments");
    assert(ctrl1->parent == this && ctrl2->parent == this && L"隔壁老王!");
    // 不一致时
    if (ctrl1 != ctrl2) {
        // A link B
        const bool a___b = ctrl1->next == ctrl2;
        // B link A
        const bool b___a = ctrl2->next == ctrl1;
        // A存在前驱
        if (ctrl1->prev) {
            if (!b___a) force_cast(ctrl1->prev->next) = ctrl2;
        }
        // A为头结点
        else {
            m_pHead = ctrl2;
        }
        // A存在后驱
        if (ctrl1->next) {
            if (!a___b) force_cast(ctrl1->next->prev) = ctrl2;
        }
        // A为尾结点
        else {
            m_pTail = ctrl2;
        }
        // B存在前驱
        if (ctrl2->prev) {
            if (!a___b) force_cast(ctrl2->prev->next) = ctrl1;
        }
        // B为头结点
        else {
            m_pHead = ctrl1;
        }
        // B存在后驱
        if (ctrl2->next) {
            if (!b___a) force_cast(ctrl2->next->prev) = ctrl1;
        }
        // B为尾结点
        else {
            m_pTail = ctrl1;
        }
        // 相邻交换
        auto swap_neibergs = [](UIControl* a, UIControl* b) noexcept {
            assert(a->next == b && "bad neibergs");
            force_cast(a->next) = b->next;
            force_cast(b->next) = a;
            force_cast(b->prev) = a->prev;
            force_cast(a->prev) = b;
        };
        // 相邻则结点?
        if (a___b) {
            swap_neibergs(ctrl1, ctrl2);
        }
        // 相邻则结点?
        else if (b___a) {
            swap_neibergs(ctrl2, ctrl1);
        }
        // 不相邻:交换前驱后驱
        else {
            std::swap(force_cast(ctrl1->prev), force_cast(ctrl2->prev));
            std::swap(force_cast(ctrl1->next), force_cast(ctrl2->next));
        }
#ifdef _DEBUG
        // 检查链表是否成环
        {
            auto count = m_cChildrenCount;
            auto debug_ctrl = m_pHead;
            while (debug_ctrl) {
                debug_ctrl = debug_ctrl->next;
                assert(count && "bad action 0 in swaping children");
                count--;
            }
            assert(!count && "bad action 1 in swaping children");
        }
        {
            auto count = m_cChildrenCount;
            auto debug_ctrl = m_pTail;
            while (debug_ctrl) {
                debug_ctrl = debug_ctrl->prev;
                assert(count && "bad action 2 in swaping children");
                count--;
            }
            assert(!count && "bad action 3 in swaping children");
        }
#endif
        // 刷新
        this->SetControlLayoutChanged();
        this->InvalidateThis();
    }
    // 给予警告
    else {
        UIManager << DL_Warning
            << L"wanna to swap 2 children but just one"
            << LongUI::endl;
    }
}

// -------------------------- UIVerticalLayout -------------------------
// UIVerticalLayout 创建
auto LongUI::UIVerticalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIVerticalLayout* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIVerticalLayout, pControl, type, node);
    }
    return pControl;
}

// 更新子控件布局
void LongUI::UIVerticalLayout::RefreshLayout() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    {
        // 初始化
        float base_width = 0.f, base_height = 0.f, basic_weight = 0.f;
        // 第一次遍历
        for (auto ctrl : (*this)) {
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
                basic_weight += ctrl->weight;
            }
        }
        // 带入控件本身宽度计算
        base_width = std::max(base_width, this->GetViewWidthZoomed());
        // 剩余高度富余
        auto height_remain = std::max(this->GetViewHeightZoomed() - base_height, 0.f);
        // 单位权重高度
        auto height_in_unit_weight = basic_weight > 0.f ? height_remain / basic_weight : 0.f;
        // 基线Y
        float position_y = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                ctrl->SetWidth(base_width);
            }
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                ctrl->SetHeight(std::max(height_in_unit_weight * ctrl->weight, float(LongUIAutoControlMinSize)));
            }
            // 容器?
            // 不管如何, 修改!
            ctrl->SetControlLayoutChanged();
            ctrl->SetLeft(0.f);
            ctrl->SetTop(position_y);
            ctrl->world;
            //ctrl->RefreshWorld();
            position_y += ctrl->GetTakingUpHeight();
        }
        // 修改
        m_2fContentSize.width = base_width * m_2fZoom.width;
        m_2fContentSize.height = position_y * m_2fZoom.height;
    }
    this->RefreshWorld();
}

// UIVerticalLayout 关闭控件
void LongUI::UIVerticalLayout::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 删除
    delete this;
}

// -------------------------- UIHorizontalLayout -------------------------
// UIHorizontalLayout 创建
auto LongUI::UIHorizontalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIHorizontalLayout* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIHorizontalLayout, pControl, type, node);
    }
    return pControl;
}


// 更新子控件布局
void LongUI::UIHorizontalLayout::RefreshLayout() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    {
        // 初始化
        float base_width = 0.f, base_height = 0.f;
        float basic_weight = 0.f;
        // 第一次
        for (auto ctrl : (*this)) {
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
                basic_weight += ctrl->weight;
            }
        }
        // 计算
        base_height = std::max(base_height, this->GetViewHeightZoomed());
        // 剩余宽度富余
        auto width_remain = std::max(this->GetViewWidthZoomed() - base_width, 0.f);
        // 单位权重宽度
        auto width_in_unit_weight = basic_weight > 0.f ? width_remain / basic_weight : 0.f;
        // 基线X
        float position_x = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                ctrl->SetHeight(base_height);
            }
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                ctrl->SetWidth(std::max(width_in_unit_weight * ctrl->weight, float(LongUIAutoControlMinSize)));
            }
            // 不管如何, 修改!
            ctrl->SetControlLayoutChanged();
            ctrl->SetLeft(position_x);
            ctrl->SetTop(0.f);
            //ctrl->RefreshWorld();
            position_x += ctrl->GetTakingUpWidth();
        }
        // 修改
        //UIManager << DL_Hint << this << position_x << LongUI::endl;
        m_2fContentSize.width = position_x;
        m_2fContentSize.height = base_height;
        // 已经处理
        this->ControlLayoutChangeHandled();
    }
}


// UIHorizontalLayout 关闭控件
void LongUI::UIHorizontalLayout::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 删除
    delete this;
}

// --------------------- Single Layout ---------------
/// <summary>
/// Before_deleteds this instance.
/// </summary>
/// <returns></returns>
void LongUI::UISingle::before_deleted() noexcept {
    // 清理子控件
    assert(m_pChild && "UISingle must host a child");
    this->release_child(m_pChild);
#ifdef _DEBUG
    // 调试清理
    m_pChild = nullptr;
#endif
    // 链式调用
    Super::before_deleted();
}

// UISingle: 事件处理
bool LongUI::UISingle::DoEvent(const LongUI::EventArgument& arg) noexcept {
    assert(arg.sender && "bad argument");
    // 处理窗口事件
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBulidingFinished:
        // 初始化边缘控件 
        Super::DoEvent(arg);
        // 初次完成空间树建立
        assert(m_pChild && "UISingle must host a child");
        m_pChild->DoEvent(arg);
        return true;
    case LongUI::Event::Event_SetNewParent:
        // 初始化边缘控件 
        Super::DoEvent(arg);
        // 修改控件深度
        m_pChild->NewParentSetted();
        m_pChild->DoEvent(arg);
        return true;
        /*case LongUI::Event::Event_NotifyChildren:
            // 不处理
#ifdef _DEBUG
            if (arg.sender == m_pChild) {
                UIManager << DL_Warning
                    << L"Event_NotifyChildren for UISinge?!"
                    << LongUI::endl;
            }
#endif
            return true;*/
    }
    return Super::DoEvent(arg);
}

// UISingle 重建
auto LongUI::UISingle::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    assert(m_pChild && "UISingle must host a child");
    hr = m_pChild->Recreate();
    // 检查
    assert(SUCCEEDED(hr));
    return Super::Recreate();
}

// UISingle: 主景渲染
void LongUI::UISingle::render_chain_main() const noexcept {
    // 渲染帮助器
    Super::RenderHelper(UIManager_RenderTarget, this->begin(), this->end());
    // 父类主景
    Super::render_chain_main();
}

// UISingle: 渲染函数
void LongUI::UISingle::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UISingle: 刷新
void LongUI::UISingle::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(this->begin(), this->end());
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UISingle::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
    // 父类(边缘控件)
    auto mctrl = Super::FindChild(pt);
    if (mctrl) return mctrl;
    assert(m_pChild && "UISingle must host a child");
    // 检查
    if (IsPointInRect(m_pChild->visible_rect, pt)) {
        return m_pChild;
    }
    return nullptr;
}


// UISingle: 推入最后
void LongUI::UISingle::Push(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::Push(child);
    }
    // 一般的就自己处理
    else {
        // 检查
#ifdef _DEBUG
        auto old = UIControl::GetPlaceholder();
        this->release_child(old);
        if (old != m_pChild) {
            UIManager << DL_Warning
                << L"m_pChild exist:"
                << m_pChild
                << LongUI::endl;
        }
#endif
        // 移除之前的
        this->release_child(m_pChild);
        this->after_insert(m_pChild = child);
    }
}

// UISingle: 仅移除
void LongUI::UISingle::Remove(UIControl* child) noexcept {
    assert(m_pChild == child && "bad argment");
    m_pChild = UIControl::GetPlaceholder();
    Super::Remove(child);
}

// UISingle: 更新布局
void LongUI::UISingle::RefreshLayout() noexcept {
    // 设置控件宽度
    if (!(m_pChild->flags & Flag_WidthFixed)) {
        m_pChild->SetWidth(this->GetViewWidthZoomed());
    }
    // 设置控件高度
    if (!(m_pChild->flags & Flag_HeightFixed)) {
        m_pChild->SetHeight(this->GetViewHeightZoomed());
    }
    // 不管如何, 修改!
    m_pChild->SetControlLayoutChanged();
    m_pChild->SetLeft(0.f);
    m_pChild->SetTop(0.f);
    // 设置内容大小
    m_2fContentSize.width = m_pChild->GetWidth();
    m_2fContentSize.height = m_pChild->GetHeight();
    // 已经处理
    this->ControlLayoutChangeHandled();
}

// UISingle 清理
void LongUI::UISingle::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 删除
    delete this;
}

// UISingle 创建空间
auto LongUI::UISingle::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UISingle* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UISingle, pControl, type, node);
    }
    return pControl;
}

// --------------------- Page Layout ---------------
// UIPage 构造函数
LongUI::UIPage::UIPage(UIContainer* cp) noexcept : Super(cp),
m_animation(AnimationType::Type_CubicEaseIn) {
    // 初始化
    m_animation.start = m_animation.value = 0.f;
    m_animation.end = 1.f;
    m_animation.duration = 0.3f;
}

/// <summary>
/// Initalizes the specified node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIPage::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 动画类型
    auto atype = Helper::GetEnumFromXml(
        node, AnimationType::Type_CubicEaseIn, "animationtype"
    );
    m_animation.type = atype;
    // 动画持续时间
    const char* str = nullptr;
    if ((str = Helper::XMLGetValue(node, "animationduration"))) {
        m_animation.duration = LongUI::AtoF(str);
    }
}

// something must do before deleted
void LongUI::UIPage::before_deleted() noexcept {
    // 清理子控件
    for (auto ctrl : m_vChildren) {
        this->release_child(ctrl);
    }
    m_vChildren.clear();
    // 链式调用
    Super::before_deleted();
}

/// <summary>
/// Finalizes an instance of the <see cref="UIPage"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIPage::~UIPage() noexcept {

}

// UIPage: 事件处理
bool LongUI::UIPage::DoEvent(const LongUI::EventArgument& arg) noexcept {
    assert(arg.sender && "bad argument");
    // 处理窗口事件
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBulidingFinished:
        // 初始化边缘控件 
        Super::DoEvent(arg);
        // 子控件
        for (auto ctrl : m_vChildren) {
            ctrl->DoEvent(arg);
        }
        return true;
    case LongUI::Event::Event_SetNewParent:
        // 初始化边缘控件 
        Super::DoEvent(arg);
        // 修改控件深度
        for (auto ctrl : m_vChildren) {
            ctrl->NewParentSetted();
            ctrl->DoEvent(arg);
        }
        return true;
        /*case LongUI::Event::Event_NotifyChildren:
            // 仅仅传递一层
            if (arg.sender->parent == this) {
                for (auto ctrl : m_vChildren) {
                    if (ctrl != arg.sender) ctrl->DoEvent(arg);
                }
            }
            return true;*/
    }
    return Super::DoEvent(arg);
}

// UIPage 重建
auto LongUI::UIPage::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    for (auto ctrl : (*this)) {
        hr = ctrl->Recreate();
        // 稍微检查一下
        assert(SUCCEEDED(hr));
    }
    return Super::Recreate();
}

// UIPage: 主景渲染
void LongUI::UIPage::render_chain_main() const noexcept {
    // 渲染帮助器
    if (m_pNextDisplay) {
        float direction = this->is_slide_to_right() ? 1.f : -1.f;
        float off = this->is_slide_to_right() ? -1.f : 1.f;
        float xoffset = (m_animation.value * direction + off) * view_size.width;
        UIManager_RenderTarget->SetTransform(
            DX::Matrix3x2F::Translation(xoffset, 0.f)
            * m_pNextDisplay->world
        );
        m_pNextDisplay->Render();
        // 有效
        if (m_pNextDisplay != m_pNowDisplay) {
            xoffset = (m_animation.value * direction) * view_size.width;
            UIManager_RenderTarget->SetTransform(
                DX::Matrix3x2F::Translation(xoffset, 0.f)
                * m_pNowDisplay->world
            );
            m_pNowDisplay->Render();
        }
    }
    // 回归
    UIManager_RenderTarget->SetTransform(&this->world);
    // 父类主景
    Super::render_chain_main();
}

// UIPage: 渲染函数
void LongUI::UIPage::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UIPage: 刷新
void LongUI::UIPage::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(this->begin(), this->end());
    // 更新动画
    m_animation.Update(UIManager.GetDeltaTime());
    // 检查结果
    if (m_animation.time <= 0.f) {
        m_pNowDisplay = m_pNextDisplay;
    }
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIPage::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
    // 父类(边缘控件)
    auto mctrl = Super::FindChild(pt);
    if (mctrl) return mctrl;
    // 检查
    if (m_pNowDisplay == m_pNextDisplay) {
        return m_pNowDisplay;
    }
    return nullptr;
}

// UIPage: 推入最后
void LongUI::UIPage::Push(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::Push(child);
    }
    // 一般的就自己处理
    else {
        this->Insert(m_cChildrenCount, child);
    }
}

// UIPage: 插入
LongUINoinline void LongUI::UIPage::Insert(uint32_t index, UIControl* child) noexcept {
    assert(child && "bad argument");
    if (child) {
#ifdef _DEBUG
        auto dgb_result = false;
        dgb_result = (child->flags & Flag_HeightFixed) == 0;
        assert(dgb_result && "child of UIPage can not keep flag: Flag_HeightFixed");
        dgb_result = (child->flags & Flag_WidthFixed) == 0;
        assert(dgb_result && "child of UIPage can not keep flag: Flag_WidthFixed");
#endif
        m_vChildren.insert(index, child);
        this->after_insert(child);
        ++m_cChildrenCount;
        assert(m_vChildren.isok());
        // 修改
        if (!m_pNowDisplay) {
            m_pNowDisplay = m_pNextDisplay = m_vChildren.front();
        }
    }
}

// UIPage: 显示下一页
void LongUI::UIPage::DisplayNextPage(uint32_t index) noexcept {
    // 范围检查
    if (index < this->GetChildrenCount()) {
        this->DisplayNextPage(m_vChildren[index]);
    }
    else {
        assert(!"out of range");
    }
}

// UIPage: 显示下一页
void LongUI::UIPage::DisplayNextPage(UIControl* page) noexcept {
    // 检查
    assert(page && page->parent == this && "bad action");
    // 蛋疼
    if (m_pNextDisplay == page) return;
    // 移除当前页面焦点
    {
        auto fc = m_pWindow->GetFocused();
        if (fc && m_pNowDisplay->IsPosterityForSelf(fc)) {
            m_pWindow->SetFocus(nullptr);
        }
    }
    // 计算
    auto nowp = m_pNowDisplay;
    auto a = std::find(this->begin(), this->end(), page) - this->begin();
    auto b = std::find(this->begin(), this->end(), nowp) - this->begin();
    // 动画
    if (a > b) this->set_slider_to_left();
    else this->set_slider_to_right();
    // 调整
    m_pNowDisplay = m_pNextDisplay;
    m_pNextDisplay = page;
    // 剩余
    this->StartRender(m_animation.time = m_animation.duration);
}

// UIPage: 仅移除
void LongUI::UIPage::Remove(UIControl* child) noexcept {
    // 查找
    auto itr = std::find(this->begin(), this->end(), child);
    // 没找到
    if (itr == this->end()) {
        UIManager << DL_Error
            << L"CHILD: " << child
            << L", NOT FOUND"
            << LongUI::endl;
    }
    // 找到了
    else {
        // 更新
        if (m_pNextDisplay == child) {
            UIManager << DL_Warning
                << L"removing displaying control"
                << LongUI::endl;
            m_pNextDisplay = m_vChildren.size() ? m_vChildren.front() : nullptr;
        }
        if (m_pNowDisplay == child) {
            UIManager << DL_Warning
                << L"removing displaying control"
                << LongUI::endl;
            m_pNowDisplay = m_vChildren.size() ? m_vChildren.front() : nullptr;
        }
        // 修改
        m_vChildren.erase(itr);
        --m_cChildrenCount;
        this->SetControlLayoutChanged();
        Super::Remove(child);
    }
}

// UIPage: 更新布局
void LongUI::UIPage::RefreshLayout() noexcept {
    // 遍历
    for (auto ctrl : m_vChildren) {
        // 设置控件左边坐标
        ctrl->SetLeft(0.f);
        // 设置控件顶部坐标
        ctrl->SetTop(0.f);
        // 设置控件宽度
        ctrl->SetWidth(this->GetViewWidthZoomed());
        // 设置控件高度
        ctrl->SetHeight(this->GetViewHeightZoomed());
        // 不管如何, 修改!
        ctrl->SetControlLayoutChanged();
    }
}

// UIPage 清理
void LongUI::UIPage::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 删除
    delete this;
}

// UIPage 创建空间
auto LongUI::UIPage::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIPage* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIPage, pControl, type, node);
    }
    return pControl;
}

// --------------------- Floating Layout ---------------
// UIFloatLayout 创建
auto LongUI::UIFloatLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIFloatLayout* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIFloatLayout, pControl, type, node);
    }
    return pControl;
}

// 更新子控件布局
void LongUI::UIFloatLayout::RefreshLayout() noexcept {
    // 布局上下文: 做为left-top坐标
    // 布局权重:   暂时无用
    for (auto ctrl : (*this)) {
        ctrl->SetLeft(ctrl->context[0]);
        ctrl->SetTop(ctrl->context[1]);
        ctrl->SetControlLayoutChanged();
    }
}

// UIFloatLayout 关闭控件
void LongUI::UIFloatLayout::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 删除
    delete this;
}