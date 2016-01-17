#include "LongUI.h"

// ----------------------------------------------------------------------------
// **** UIText
// ----------------------------------------------------------------------------

// 前景渲染
void LongUI::UIText::render_chain_foreground() const noexcept {
    // 文本算前景
    m_text.Render(0.f, 0.f);
    // 父类
    Super::render_chain_foreground();
}

// UI文本: 渲染
void LongUI::UIText::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UIText: 刷新
void LongUI::UIText::Update() noexcept {
    // 改变了大小
    if(this->IsControlLayoutChanged()) {
        // 设置大小
        m_text.Resize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlLayoutChangeHandled();
    }
    return Super::Update();
}

// UIText: 事件响应
bool LongUI::UIText::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LONGUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetText:
            m_text = arg.stt.text;
            m_pWindow->Invalidate(this);
            __fallthrough;
        case LongUI::Event::Event_GetText:
            arg.str = m_text.c_str();
            return true;
        }
    }
    return Super::DoEvent(arg);
}


/*/ UIText 构造函数
LongUI::UIText::UIText(pugi::xml_node node) noexcept: Super(node), m_text(node) {
    //m_bInitZoneChanged = true;
}
*/

// UIText::CreateControl 函数
auto LongUI::UIText::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
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
        // 警告
        if (!node) {
            UIManager << DL_Hint << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = CreateWidthCET<LongUI::UIText>(type, node);
        // OOM
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    }
    return pControl;
}


// recreate 重建
/*HRESULT LongUI::UIText::Recreate() noexcept {
// 断言
return Super::Recreate();
}*/


// close this control 关闭控件
void LongUI::UIText::cleanup() noexcept {
    delete this;
}

// ----------------------------------------------------------------------------
// **** UIButton
// ----------------------------------------------------------------------------

// UIButton: 前景渲染
void LongUI::UIButton::render_chain_background() const noexcept {
    // UI部分算作前景
    D2D1_RECT_F draw_rect;
    this->GetViewRect(draw_rect);
    m_uiElement.Render(draw_rect);
    // 父类前景
    Super::render_chain_background();
}

// Render 渲染 
void LongUI::UIButton::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI按钮: 刷新
void LongUI::UIButton::Update() noexcept {
    // 更新计时器
    m_uiElement.Update();
    return Super::Update();
}

// UIButton 构造函数
LongUI::UIButton::UIButton(UIContainer* cp, pugi::xml_node node) 
noexcept: Super(cp, node), m_uiElement(node, State_Normal, 0) {
    // 允许键盘焦点
    auto flag = this->flags | Flag_Focusable;
    // 初始化
    Helper::SetBorderColor(node, m_aBorderColor);
    constexpr int azz = sizeof(m_uiElement);
    // 修改
    force_cast(this->flags) = flag;
}


// UIButton::CreateControl 函数
auto LongUI::UIButton::CreateControl(CreateEventType type,pugi::xml_node node) noexcept ->UIControl* {
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
        // 警告
        if (!node) {
            UIManager << DL_Hint << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = CreateWidthCET<LongUI::UIButton>(type, node);
        // OOM
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    }
    return pControl;
}


// do event 事件处理
bool LongUI::UIButton::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // longui 消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetFocus:
            // 设置焦点: 要求焦点
            return true;
        case LongUI::Event::Event_KillFocus:
            // 释放焦点:
            m_tarStateClick = LongUI::State_Normal;
            return true;
        case LongUI::Event::Event_SetEnabled:
            // 修改状态
            return Super::DoEvent(arg);
        }
    }
    return Super::DoEvent(arg);
}

// 鼠标事件处理
bool LongUI::UIButton::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // longui 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        // 鼠标移进: 设置UI元素状态
        this->SetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        return true;
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出: 设置UI元素状态
        this->SetControlState(LongUI::State_Normal);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        m_pWindow->SetCapture(this);
        this->SetControlState(LongUI::State_Pushed);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Pushed];
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        if (m_pWindow->IsReleasedControl(this)) {
            bool rec = this->call_uievent(m_event, SubEvent::Event_ItemClicked);
            rec = false;
            // 设置状态
            this->SetControlState(m_tarStateClick);
            m_colorBorderNow = m_aBorderColor[m_tarStateClick];
            m_pWindow->ReleaseCapture();
        }
        return true;
    }
    return false;
}

// recreate 重建
auto LongUI::UIButton::Recreate() noexcept ->HRESULT {
    // 重建元素
    m_uiElement.Recreate();
    // 父类处理
    return Super::Recreate();
}

// 添加事件监听器(雾)
bool LongUI::UIButton::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    // 点击
    if (sb == SubEvent::Event_ItemClicked) {
        m_event += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

// 关闭控件
void LongUI::UIButton::cleanup() noexcept {
    delete this;
}


// ----------------------------------------------------------------------------
// **** UIEdit
// ----------------------------------------------------------------------------

// UI基本编辑控件: 前景渲染
void LongUI::UIEditBasic::render_chain_foreground() const noexcept {
    // 文本算前景
    m_text.Render(0.f, 0.f);
    // 父类
    Super::render_chain_foreground();
}

// UI基本编辑控件: 渲染
void LongUI::UIEditBasic::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI基本编辑框: 刷新
void LongUI::UIEditBasic::Update() noexcept {
    // 改变了大小
    if (this->IsControlLayoutChanged()) {
        // 设置大小
        m_text.Resize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlLayoutChangeHandled();
    }
    // 刷新
    m_text.Update();
    return Super::Update();
}

// UI基本编辑控件
bool  LongUI::UIEditBasic::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI 消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            __fallthrough;
        case LongUI::Event::Event_SubEvent:
            return true;
        case LongUI::Event::Event_SetFocus:
            m_text.OnSetFocus();
            return true;
        case LongUI::Event::Event_KillFocus:
            m_text.OnKillFocus();
            return true;
        case LongUI::Event::Event_SetText:
            assert(!"NOIMPL");
            __fallthrough;
        case LongUI::Event::Event_GetText:
            arg.str = m_text.c_str();
            return true;
        }
    }
    // 系统消息
    else {
        switch (arg.msg)
        {
        default:
            return false;
        case WM_KEYDOWN:
            m_text.OnKey(static_cast<uint32_t>(arg.sys.wParam));
            break;
        case WM_CHAR:
            m_text.OnChar(static_cast<char32_t>(arg.sys.wParam));
            break;
        }
    }
    return true;
}

// UI基本编辑控件: 鼠标事件
bool  LongUI::UIEditBasic::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // LongUI 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_DragEnter:
        m_text.OnDragEnter(arg.cf.dataobj, arg.cf.outeffect);
        break;
    case LongUI::MouseEvent::Event_DragOver:
        m_text.OnDragOver(pt4self.x, pt4self.y);
        break;
    case LongUI::MouseEvent::Event_DragLeave:
        m_text.OnDragLeave();
        break;
    case LongUI::MouseEvent::Event_Drop:
        m_text.OnDrop(arg.cf.dataobj, arg.cf.outeffect);
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        m_pWindow->now_cursor = m_hCursorI;
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        m_pWindow->now_cursor = m_pWindow->default_cursor;
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        // 拖拽?
        if (arg.sys.wParam & MK_LBUTTON) {
            m_text.OnLButtonHold(pt4self.x, pt4self.y);
        }
        break;
    case LongUI::MouseEvent::Event_LButtonDown:
        m_text.OnLButtonDown(pt4self.x, pt4self.y, !!(arg.sys.wParam & MK_SHIFT));
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        m_text.OnLButtonUp(pt4self.x, pt4self.y);
        break;
    }
    return true;
}

// close this control 关闭控件
HRESULT LongUI::UIEditBasic::Recreate() noexcept {
    m_text.Recreate();
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UIEditBasic::cleanup() noexcept {
    delete this;
}

// 构造函数
LongUI::UIEditBasic::UIEditBasic(UIContainer* cp, pugi::xml_node node)
noexcept : Super(cp, node), m_text(this, node) {
    // 允许键盘焦点
    auto flag = this->flags | Flag_Focusable;
    if (node) {

    }
    // 修改
    force_cast(this->flags) = flag;
}

// UIEditBasic::CreateControl 函数
LongUI::UIControl* LongUI::UIEditBasic::CreateControl(CreateEventType type,pugi::xml_node node) noexcept {
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
        // 警告
        if (!node) {
            UIManager << DL_Hint << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = CreateWidthCET<LongUI::UIEditBasic>(type, node);
        // OOM
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    }
    return pControl;
}


// 调试区域
#ifdef LongUIDebugEvent
// longui 转换

// UI控件: 调试信息
bool LongUI::UIControl::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIControl";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIControl";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIControl>();
    default:
        break;
    }
    return false;
}

// 类型转换断言
void LongUI::UIControl::AssertTypeCasting(const IID& iid) const noexcept {
    LongUI::DebugEventInformation info;
    info.infomation = LongUI::DebugInformation::Information_CanbeCasted;
    info.iid = &iid; info.id = 0;
    assert(this->debug_do_event(info) && "bad casting");
}

// 类型转换判断
auto LongUI::UIControl::IsCanbeCastedTo(const IID& iid) const noexcept ->bool {
    LongUI::DebugEventInformation info;
    info.infomation = LongUI::DebugInformation::Information_CanbeCasted;
    info.iid = &iid; info.id = 0;
    return this->debug_do_event(info);
}

// 获取控件类名
auto LongUI::UIControl::GetControlClassName(bool full) const noexcept ->const wchar_t* {
    LongUI::DebugEventInformation info;
    info.infomation = full ? LongUI::DebugInformation::Information_GetFullClassName
        : LongUI::DebugInformation::Information_GetClassName;
    info.iid = nullptr; info.str = L"";
    this->debug_do_event(info);
    return info.str;
}

// UI标签: 调试信息
bool LongUI::UIText::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIText";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIText";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIText>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI按钮: 调试信息
bool LongUI::UIButton::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIButton";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIButton";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIButton>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI边缘控件: 调试信息
bool LongUI::UIMarginalable::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIMarginalable";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIMarginalable";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIMarginalable>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI滚动条: 调试信息
bool LongUI::UIScrollBar::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIScrollBar";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIScrollBar";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIScrollBar>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI滚动条-A型: 调试信息
bool LongUI::UIScrollBarA::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIScrollBarA";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIScrollBarA";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIScrollBarA>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI滚动条-A型: 调试信息
bool LongUI::UIScrollBarB::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIScrollBarB";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIScrollBarB";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIScrollBarB>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI容器: 调试信息
bool LongUI::UIContainer::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIContainer";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIContainer";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIContainer>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// LongUI内建容器: 调试信息
bool LongUI::UIContainerBuiltIn::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIContainerBuiltIn";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIContainerBuiltIn";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIContainerBuiltIn>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// LongUI单独容器: 调试信息
bool LongUI::UISingle::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UISingle";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UISingle";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UISingle>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// LongUI页面容器: 调试信息
bool LongUI::UIPage::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIPage";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIPage";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIPage>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI 基本编辑控件: 调试信息
bool LongUI::UIEditBasic::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIEditBasic";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIEditBasic";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIEditBasic>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI水平布局: 调试信息
bool LongUI::UIHorizontalLayout::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIHorizontalLayout";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIHorizontalLayout";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIHorizontalLayout>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI垂直布局: 调试信息
bool LongUI::UIVerticalLayout::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIVerticalLayout";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIVerticalLayout";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIVerticalLayout>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI浮动布局: 调试信息
bool LongUI::UIFloatLayout::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIFloatLayout";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIFloatLayout";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIFloatLayout>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}


// UI窗口: 调试信息
bool LongUI::UIWindow::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIWindow";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIWindow";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIWindow>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}


// --------------------------- 单独 ---------------------
// UI滑动条: 调试信息
bool LongUI::UISlider::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UISlider";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UISlider";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UISlider>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI列表: 调试信息
bool LongUI::UIList::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIList";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIList";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIList>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI列表元素: 调试信息
bool LongUI::UIListLine::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIListLine";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIListLine";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIListLine>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI列表头: 调试信息
bool LongUI::UIListHeader::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIListHeader";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIListHeader";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIListHeader>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}
#endif