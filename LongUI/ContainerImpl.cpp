#include "LongUI.h"


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
        float basic_weight = 0.f;
        //
        if (m_strControlName == L"V") {
            int bk = 0;
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
                    basic_weight += ctrl->weight;
                }
            }
        }
        if (m_strControlName == L"V") {
            int bk = 9;
        }
        // 校正
        base_width /= m_2fZoom.width;
        base_height /= m_2fZoom.height;
        // 计算
        base_width = std::max(base_width, this->view_size.width);
        // 剩余高度富余
        register auto height_remain = std::max(this->view_size.height - base_height, 0.f);
        // 单位权重高度
        auto height_in_unit_weight = basic_weight > 0.f ? height_remain / basic_weight : 0.f ;
        // 基线Y
        float position_y = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 浮点控
            if (ctrl->flags & Flag_Floating) continue;
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
            ctrl->SetControlSizeChanged();
            ctrl->SetLeft(0.f);
            ctrl->SetTop(position_y);
            position_y += ctrl->GetTakingUpHeight();
        }
        // 修改
        m_2fContentSize.width = base_width;
        m_2fContentSize.height = position_y;
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
        float basic_weight = 0.f;
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
                    basic_weight += ctrl->weight;
                }
            }
        }
        // 校正
        base_width /= m_2fZoom.width;
        base_height /= m_2fZoom.height;
        // 计算
        base_height = std::max(base_height, this->view_size.height);
        // 剩余宽度富余
        register auto width_remain = std::max(this->view_size.width - base_width, 0.f);
        // 单位权重宽度
        auto width_in_unit_weight = basic_weight > 0.f ? width_remain / basic_weight : 0.f;
        // 基线X
        float position_x = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 跳过浮动控件
            if (ctrl->flags & Flag_Floating) continue;
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                ctrl->SetHeight(base_height);
            }
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                ctrl->SetWidth(std::max(width_in_unit_weight * ctrl->weight, float(LongUIAutoControlMinSize)));
            }
            // 不管如何, 修改!
            ctrl->SetControlSizeChanged();
            ctrl->SetLeft(position_x);
            ctrl->SetTop(0.f);
            position_x += ctrl->GetTakingUpWidth();
        }
        // 修改
        m_2fContentSize.width = position_x;
        m_2fContentSize.height = base_height;
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

