#include "LongUI.h"
#include <algorithm>

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
    if (this->IsControlLayoutChanged()) {
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
            this->InvalidateThis();
            __fallthrough;
        case LongUI::Event::Event_GetText:
            arg.str = m_text.c_str();
            return true;
        case LongUI::Event::Event_SetEnabled:
            // 修改状态
            m_text.SetState(arg.ste.enabled ? State_Normal : State_Disabled);
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
    UIText* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIText, pControl, type, node);
    }
    return pControl;
}

// close this control 关闭控件
void LongUI::UIText::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除对象
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
void LongUI::UIButton::initialize(pugi::xml_node node) noexcept {
    // 链式初始化
    Super::initialize(node);
    m_uiElement.Init(this->check_state(), 0, node);
    // 允许键盘焦点
    auto flag = this->flags | Flag_Focusable;
    // 初始化
    Helper::SetBorderColor(node, m_aBorderColor);
    constexpr int azz = sizeof(m_uiElement);
    // 修改
    force_cast(this->flags) = flag;
}


// UIButton::CreateControl 函数
auto LongUI::UIButton::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIButton* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIButton, pControl, type, node);
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
            m_uiElement.SetBasicState(arg.ste.enabled ? State_Normal : State_Disabled);
        }
    }
    return Super::DoEvent(arg);
}

// 设置控件状态
LongUINoinline void LongUI::UIButton::SetControlState(ControlState state) noexcept {
    m_text.SetState(state);
    m_pWindow->StartRender(m_uiElement.SetBasicState(state), this);
}

// 鼠标事件处理
bool LongUI::UIButton::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // 禁用状态禁用鼠标消息
    if (!this->GetEnabled()) return true;
    // 转换坐标
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // 鼠标 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        // 鼠标移进: 设置UI元素状态
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        this->SetControlState(LongUI::State_Hover);
        return true;
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出: 设置UI元素状态
        m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
        this->SetControlState(LongUI::State_Normal);
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        // 左键按下:
        m_pWindow->SetCapture(this);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Pushed];
        this->SetControlState(LongUI::State_Pushed);
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        m_tarStateClick = LongUI::State_Hover;
        // 左键弹起:
        if (m_pWindow->IsCapturedControl(this)) {
            bool rec = this->call_uievent(m_event, SubEvent::Event_ItemClicked);
            rec = false;
            // 设置状态
            this->SetControlState(m_tarStateClick);
            m_colorBorderNow = m_aBorderColor[m_tarStateClick];
            m_pWindow->ReleaseCapture();
        }
        return true;
    }
    // 未处理的消息
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
    // 删除前调用
    this->before_deleted();
    // 删除对象
    delete this;
}

// ----------------------------------------------------------------------------
// **** UIComboBox
// ----------------------------------------------------------------------------

// Render 渲染 
void LongUI::UIComboBox::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI文本: 渲染前景
void LongUI::UIComboBox::render_chain_foreground() const noexcept {
    // 父类渲染
    Super::render_chain_foreground();
    // 渲染下拉箭头
    if (!m_uiElement.IsExtraInterfaceValid()) {
        // 几何体
        auto arrow = UIScrollBarA::s_apArrowPathGeometry[UIScrollBarA::Arrow_Bottom];
#if 0
        // 目标矩形
        D2D1_RECT_F rect; 
        rect.right = this->view_size.width;
        rect.left = rect.right - UIScrollBarA::BASIC_SIZE;
        rect.top = (this->view_size.height - UIScrollBarA::BASIC_SIZE) * 0.5f;
        rect.bottom = (this->view_size.height + UIScrollBarA::BASIC_SIZE) * 0.5f;
        // 颜色同文本
        m_pBrush_SetBeforeUse->SetColor(m_text.GetColor());
        // 渲染
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
        // 渲染
        render_geo(UIManager_RenderTarget, m_pBrush_SetBeforeUse, arrow, rect);
#else
        // 目标坐标
        D2D1_POINT_2F pt; 
        pt.x = this->view_size.width - UIScrollBarA::BASIC_SIZE;
        pt.y = (this->view_size.height - UIScrollBarA::BASIC_SIZE) * 0.5f;
        // 颜色同文本
        m_pBrush_SetBeforeUse->SetColor(m_text.GetColor());
        // 渲染
        auto render_arrow = [&pt, arrow](ID2D1RenderTarget* target, ID2D1Brush* brush) noexcept {
            D2D1_MATRIX_3X2_F matrix; target->GetTransform(&matrix);
            target->SetTransform(DX::Matrix3x2F::Translation(pt.x, pt.y) * matrix);
            target->DrawGeometry(arrow, brush, 2.33333f);
            target->SetTransform(&matrix);
        };
        // 渲染
        render_arrow(UIManager_RenderTarget, m_pBrush_SetBeforeUse);
#endif
    }
}

// UIComboBox: 添加事件监听器(雾)
bool LongUI::UIComboBox::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    // 点击
    if (sb == SubEvent::Event_ValueChanged) {
        m_eventChanged += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

// initialize, maybe you want call v-method
void LongUI::UIComboBox::initialize(pugi::xml_node node) noexcept {
    m_vItems.reserve(32);
    assert(!m_pItemList);
    // 链式初始化
    Super::initialize(node);
    // 创建列表
    auto list = node.first_child();
    if (!list) list= node.append_child("List");
    assert(list && "bad action");
    auto tmp = UIManager.CreateControl(nullptr, list, nullptr);
    if (tmp) {
        // 转换为List
        m_pItemList = longui_cast<UIList*>(tmp);
        // 存在子节点尝试创建控件树
        if (list.first_child()) {
            UIManager.MakeControlTree(m_pItemList, list);
        }
    }
    // 添加事件回调
    if (m_pItemList) {
        auto list = m_pItemList;
        m_pItemList->AddEventCall([list, this](UIControl* unused) noexcept {
            UNREFERENCED_PARAMETER(unused);
            auto& selected = list->GetSelectedIndices();
            // 检查选项
            if (selected.size() && m_vItems.isok()) {
                // 选择
                uint32_t index = uint32_t(selected.front());
                auto old = this->GetSelectedIndex();
                this->SetSelectedIndex(index);
                // 是否选择
                if (this->GetSelectedIndex() != old) {
                    this->call_uievent(m_eventChanged, SubEvent::Event_ValueChanged);
                }
            }
            // 点击选项关闭
            list->GetWindow()->CloseWindowLater();
            return true;
        }, SubEvent::Event_ItemClicked);
    }
    // 点击事件
    auto call = [this](UIControl* unused) noexcept {
        UNREFERENCED_PARAMETER(unused);
        D2D1_POINT_2F p1 = { 0.f, 0.f };
        D2D1_POINT_2F p2 = { this->view_size.width,  this->view_size.height };
        p1 = LongUI::TransformPoint(this->world, p1);
        p2 = LongUI::TransformPoint(this->world, p2);
        Config::Popup config;
        // 上界限
        config.topline = p1.y;
        // 下界限
        config.bottomline = p2.y;
        // 左界限
        config.leftline = p1.x;
        // 宽度
        config.width = std::max(p2.x - p1.x, float(LongUIMaxGradientStop));
        // 高度
        {
            uint32_t count = std::min(m_fMaxLine, m_pItemList->GetChildrenCount());
            config.height = m_pItemList->GetLineHeight() * static_cast<float>(count);
        }
        // 必要数据
        config.parent = m_pWindow;
        config.child = m_pItemList;
        // 创建弹出窗口
        auto popup = UIViewport::CreatePopup(config);
        return true;
    };
    // 添加事件
    this->AddEventCall(call, SubEvent::Event_ItemClicked);
    // 成功
    if (m_vItems.isok() && m_pItemList) {
        // 添加项目
        for (auto line : m_pItemList->GetContainer()) {
            this->PushItem(line->GetFirstChildText());
        }
        // 获取索引
        auto index = uint32_t(LongUI::AtoI(node.attribute("select").value()));
        // 设置显示
        this->SetSelectedIndex(index);
    }
}

// 插入物品
LongUINoinline void LongUI::UIComboBox::InsertItem(uint32_t index, const wchar_t* item) noexcept {
    assert(item && index <= m_vItems.size() && "bad argument");
    auto copy = m_strAllocator.CopyString(item);
    // 有效
    if (copy && index <= m_vItems.size()) {
        const auto oldsize = m_vItems.size();
        m_vItems.insert(index, copy);
        // 校正索引
        if (m_indexSelected >= index && m_indexSelected < oldsize) {
            // 选择后面那个
            this->SetSelectedIndex(m_indexSelected + 1);
        }
        this->InvalidateThis();
    }
#ifdef _DEBUG
    else {
        assert(!"error");
    }
#endif
}

// 设置选择索引
void LongUI::UIComboBox::SetSelectedIndex(uint32_t index) noexcept {
    // 在选择范围
    if (index >= 0 && index < m_vItems.size()) {
        m_text = m_vItems[index];
        m_indexSelected = index;
    }
    // 取消显示
    else {
        m_text = L"";
        m_indexSelected = static_cast<decltype(m_indexSelected)>(-1);
    }
    // 下帧刷新
    this->InvalidateThis();
}

// 移除物品
LongUINoinline void LongUI::UIComboBox::RemoveItem(uint32_t index) noexcept {
    assert(index < m_vItems.size() && "bad argument");
    // 有效
    if (index < m_vItems.size()) {
        const auto oldsize = m_vItems.size();
        m_vItems.erase(index);
        // 校正索引
        if (m_indexSelected >= index && m_indexSelected < oldsize) {
            this->SetSelectedIndex(m_indexSelected > 0 ? m_indexSelected - 1 : m_indexSelected);
        }
        this->InvalidateThis();
    }
}

// 添加物品
void LongUI::UIComboBox::PushItem(const char* item) noexcept {
    assert(item && "bad argument");
    //if (!item) item = "";
    // 有效
    wchar_t buffer[LongUIStringBufferLength];
    buffer[UTF8toWideChar(item, buffer)] = 0;
    this->PushItem(buffer);
}


// 添加物品
void LongUI::UIComboBox::PushItem(const wchar_t* item) noexcept {
    assert(item && "bad argument");
    //if (!item) item = L"";
    this->InsertItem(this->GetItemCount(), item);
}

// UIComboBox: 析构函数
inline LongUI::UIComboBox::~UIComboBox() noexcept {
    LongUI::SafeRelease(m_pItemList);
}

// UIComboBox: 关闭控件
void LongUI::UIComboBox::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除对象
    delete this;
}

// UIComboBox::CreateControl 函数
auto LongUI::UIComboBox::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIComboBox* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIComboBox, pControl, type, node);
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

// UI文本: 调试信息
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

// UI组合框: 调试信息
bool LongUI::UIComboBox::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIComboBox";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIComboBox";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIComboBox>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI单选按钮: 调试信息
bool LongUI::UIRadioButton::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIRadioButton";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIRadioButton";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIRadioButton>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI复选框: 调试信息
bool LongUI::UICheckBox::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UICheckBox";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UICheckBox";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UICheckBox>()
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
bool LongUI::UIViewport::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIViewport";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIViewport";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIViewport>()
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