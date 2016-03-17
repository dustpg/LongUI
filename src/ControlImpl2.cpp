#include "LongUI.h"

// 主要景渲染
void LongUI::UICheckBox::render_chain_main() const noexcept {
    D2D1_RECT_F rect;
    rect.left = 0.f; rect.right = BOX_SIZE;
    rect.top = (this->view_size.height - BOX_SIZE) * 0.5f;
    rect.bottom = rect.top + BOX_SIZE;
    m_uiElement.Render(rect);
}

// 背景渲染
void LongUI::UICheckBox::render_chain_foreground() const noexcept {
    // 文本算前景
    if (m_text.GetString().length()) {
        m_text.Render(BOX_SIZE, 0.f);
    }
    // 父类-父类
    UIControl::render_chain_foreground();
}

// Render 渲染 
void LongUI::UICheckBox::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI检查框: 刷新
void LongUI::UICheckBox::Update() noexcept {
    m_uiElement.Update();
    return Super::Update();
}

// 强制设置控件状态
LongUINoinline void LongUI::UICheckBox::ForceSetControlState(ControlState state) noexcept {
#ifdef _DEBUG
    if (!this->GetEnabled()) {
        UIManager << DL_Hint << this
            << L" disabled, maybe you want call 'UICheckBox::SafeSetControlState'"
            << LongUI::endl;
    }
#endif
    m_text.SetState(state); 
    this->StartRender(m_uiElement.SetBasicState(state)); 
}

// 强制设置复选框状态
void LongUI::UICheckBox::ForceSetCheckBoxState(CheckBoxState state) noexcept {
#ifdef _DEBUG
    if (!this->GetEnabled()) {
        UIManager << DL_Hint << this
            << L" disabled, maybe you want call 'UICheckBox::SafeSetCheckBoxState'"
            << LongUI::endl;
    }
#endif
    // 修改状态
    if (state != this->GetCheckBoxState()) {
        this->StartRender(m_uiElement.SetExtraState(state));
        bool rec = this->call_uievent(m_event, SubEvent::Event_ValueChanged);
        rec = false;
#ifdef _DEBUG
        const wchar_t* const list[] = {
            L"checked", 
            L"indeterminate",
            L"unchecked"
        };
        UIManager << DL_Log << this
            << L"change to ["
            << list[size_t(state)]
            << L']' 
            << LongUI::endl;
#endif
    }
}

// UICheckBox 初始化
void LongUI::UICheckBox::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 先初始化复选框状态
    m_uiElement.Init(
        this->check_state(),
        Helper::GetEnumFromXml(node, CheckBoxState::State_Unchecked),
        node
        );
    // 初始化
    Helper::SetBorderColor(node, m_aBorderColor);
}

// UICheckBox 析构函数
LongUI::UICheckBox::~UICheckBox() noexcept {

}


// UICheckBox::CreateControl 函数
auto LongUI::UICheckBox::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UICheckBox* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UICheckBox, pControl, type, node);
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UICheckBox::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI消息
    if (arg.sender) {
        switch (arg.event)
        {
        /*case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;*/
        case LongUI::Event::Event_SetEnabled:
            // 修改状态
            m_uiElement.SetBasicState(arg.ste.enabled ? State_Normal : State_Disabled);
        }
    }
    return Super::DoEvent(arg);
}


// do mouse event 鼠标事件处理
bool LongUI::UICheckBox::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    //UIManager << DL_Hint << this << this->GetEnabled() << LongUI::endl;
    // 禁用状态禁用鼠标消息
    if (!this->GetEnabled()) return true;
    // 转换坐标
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // 鼠标 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        // 鼠标移进: 设置UI元素状态
        this->ForceSetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        m_pWindow->SetCursor(Cursor::Cursor_Hand);
        return true;
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出: 设置UI元素状态
        this->ForceSetControlState(LongUI::State_Normal);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
        m_pWindow->ResetCursor();
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        // 左键按下:
        m_pWindow->SetCapture(this);
        this->ForceSetControlState(LongUI::State_Pushed);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Pushed];
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 左键弹起:
        this->ForceSetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        // 检查的是本控件
        if (m_pWindow->IsCapturedControl(this)) {
            // 检查flag
            auto target = CheckBoxState::State_Checked;
            if (this->GetCheckBoxState() == CheckBoxState::State_Checked) {
                target = CheckBoxState::State_Unchecked;
            }
            // 修改复选框状态
            this->ForceSetCheckBoxState(target);
            // 释放
            m_pWindow->ReleaseCapture();
        }
        return true;
    }
    // 未处理的消息
    return false;
}


// 添加事件监听器
bool LongUI::UICheckBox::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    // 点击
    if (sb == SubEvent::Event_ValueChanged) {
        m_event += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

// recreate 重建
auto LongUI::UICheckBox::Recreate() noexcept ->HRESULT {
    // 有效
    m_uiElement.Recreate();
    // 父类处理
    return Super::Recreate();
}

// 关闭控件
void LongUI::UICheckBox::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除对象
    delete this;
}




// ----------------------------------------------------------------------------
// **** UIRadioButton
// ----------------------------------------------------------------------------

// 主要景渲染
void LongUI::UIRadioButton::render_chain_main() const noexcept {
    D2D1_RECT_F rect;
    rect.left = 0.f; rect.right = RADIO_SIZE;
    rect.top = (this->view_size.height - RADIO_SIZE) * 0.5f;
    rect.bottom = rect.top + RADIO_SIZE;
    m_uiElement.Render(rect);
}

// 背景渲染
void LongUI::UIRadioButton::render_chain_foreground() const noexcept {
    // 文本算前景
    if (m_text.GetString().length()) {
        m_text.Render(RADIO_SIZE, 0.f);
    }
    // 父类-父类
    UIControl::render_chain_foreground();
}

// Render 渲染 
void LongUI::UIRadioButton::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI检查框: 刷新
void LongUI::UIRadioButton::Update() noexcept {
    m_uiElement.Update();
    return Super::Update();
}


// 取消已经勾上的单选按钮
void LongUI::UIRadioButton::uncheck_checked_and_check_this() noexcept {
    auto checked = longui_cast<UIRadioButton*>(this->parent->GetPopularChild());
    if (checked && checked != this) {
        checked->SafeSetCheckedState(false);
    }
    this->parent->SetPopularChild(this);
}

// 强制设置控件状态
LongUINoinline void LongUI::UIRadioButton::ForceSetControlState(ControlState state) noexcept {
#ifdef _DEBUG
    if (!this->GetEnabled()) {
        UIManager << DL_Hint << this
            << L" disabled, maybe you want call 'UIRadioButton::SafeSetControlState'"
            << LongUI::endl;
    }
#endif
    m_text.SetState(state); 
    this->StartRender(m_uiElement.SetBasicState(state)); 
}

// 强制设置复选框状态
void LongUI::UIRadioButton::ForceSetCheckedState(bool state) noexcept {
#ifdef _DEBUG
    if (!this->GetEnabled()) {
        UIManager << DL_Hint << this
            << L" disabled, maybe you want call 'UIRadioButton::SafeSetCheckBoxState'"
            << LongUI::endl;
    }
#endif
    // 修改状态
    if (state != this->GetCheckedState()) {
        // 单选
        if (state) {
            this->uncheck_checked_and_check_this();
        }
        this->StartRender(m_uiElement.SetExtraState(state));
        bool rec = this->call_uievent(m_event, SubEvent::Event_ValueChanged);
        rec = false;
#ifdef _DEBUG
        UIManager << DL_Log << this
            << L"change to ["
            << (state ? L"checked" : L"unchecked")
            << L']' 
            << LongUI::endl;
#endif
    }
}

// UIRadioButton 初始化
void LongUI::UIRadioButton::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 先初始化复选框状态
    m_uiElement.Init(
        this->check_state(),
        node.attribute("checked").as_bool(),
        node
    );
    // 去掉之前的勾
    if (this->GetCheckedState()) {
        this->uncheck_checked_and_check_this();
    }
    // 初始化
    Helper::SetBorderColor(node, m_aBorderColor);
}

// UIRadioButton 析构函数
LongUI::UIRadioButton::~UIRadioButton() noexcept {

}


// UIRadioButton::CreateControl 函数
auto LongUI::UIRadioButton::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIRadioButton* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIRadioButton, pControl, type, node);
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UIRadioButton::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI消息
    if (arg.sender) {
        switch (arg.event)
        {
            /*case LongUI::Event::Event_SetFocus:
            __fallthrough;
            case LongUI::Event::Event_KillFocus:
            return true;*/
        case LongUI::Event::Event_SetEnabled:
            // 修改状态
            m_uiElement.SetBasicState(arg.ste.enabled ? State_Normal : State_Disabled);
        }
    }
    return Super::DoEvent(arg);
}


// do mouse event 鼠标事件处理
bool LongUI::UIRadioButton::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    //UIManager << DL_Hint << this << this->GetEnabled() << LongUI::endl;
    // 禁用状态禁用鼠标消息
    if (!this->GetEnabled()) return true;
    // 转换坐标
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // 鼠标 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        // 鼠标移进: 设置UI元素状态
        this->ForceSetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        //m_pWindow->now_cursor = m_hCursorHand;
        return true;
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出: 设置UI元素状态
        this->ForceSetControlState(LongUI::State_Normal);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
        //m_pWindow->now_cursor = m_pWindow->default_cursor;
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        // 左键按下:
        m_pWindow->SetCapture(this);
        this->ForceSetControlState(LongUI::State_Pushed);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Pushed];
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 左键弹起:
        this->ForceSetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        // 检查的是本控件
        if (!m_pWindow->IsCapturedControl(this)) return true;
        // 修改复选框状态
        if (!this->GetCheckedState()) {
            this->ForceSetCheckedState(true);
        }
        // 释放
        m_pWindow->ReleaseCapture();
        return true;
    }
    // 未处理的消息
    return false;
}


// 添加事件监听器
bool LongUI::UIRadioButton::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    // 点击
    if (sb == SubEvent::Event_ValueChanged) {
        m_event += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

// recreate 重建
auto LongUI::UIRadioButton::Recreate() noexcept ->HRESULT {
    // 有效
    m_uiElement.Recreate();
    // 父类处理
    return Super::Recreate();
}

// 关闭控件
void LongUI::UIRadioButton::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除对象
    delete this;
}

