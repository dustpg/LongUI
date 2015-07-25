#include "LongUI.h"

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
    // 颜色
    m_aBorderColor[Status_Disabled] = D2D1::ColorF(0xD9D9D9);
    m_aBorderColor[Status_Normal] = D2D1::ColorF(0xACACAC);
    m_aBorderColor[Status_Hover] = D2D1::ColorF(0x7EB4EA);
    m_aBorderColor[Status_Pushed] = D2D1::ColorF(0x569DE5);
    m_colorBorderNow = m_aBorderColor[Status_Normal];
    // 构造默认
    int flag = LongUIFlag::Flag_None | LongUIFlag::Flag_Visible;
    // 有效?
    if (node) {
        const char* data = nullptr;
        // 检查脚本
        if ((data = node.attribute(XMLAttribute::Script).value()) && UIManager.script) {
            m_script = UIManager.script->AllocScript(data);
        }
        // 渲染优先级
        if (data = node.attribute(LongUI::XMLAttribute::RenderingPriority).value()){
            force_cast(this->priority) = int8_t(LongUI::AtoI(data));
        }
        /*else {
            m_script.data = nullptr; m_script.size = 0;
        }*/
        // 检查名称
        UIControl::MakeString(
            node.attribute(LongUI::XMLAttribute::ControlName).value(),
            m_strControlName
            );
        // 检查视口位置
        UIControl::MakeFloats(
            node.attribute(LongUI::XMLAttribute::ViewPosotion).value(),
            &this->view_pos.x,
            sizeof(this->view_pos) / sizeof(this->view_pos.x)
            );
        // 检查视口大小
        UIControl::MakeFloats(
            node.attribute(LongUI::XMLAttribute::ViewSize).value(),
            &view_size.width,
            sizeof(this->view_pos) / sizeof(this->view_size.width)
            );
        // 检查外边距
        UIControl::MakeFloats(
            node.attribute(LongUI::XMLAttribute::Margin).value(),
            const_cast<float*>(&margin_rect.left), 
            sizeof(margin_rect) /sizeof(margin_rect.left)
            );
        // 视口区宽度固定?
        if (this->view_size.width > 0.f) {
            flag |= LongUI::Flag_ViewWidthFixed;
        }
        // 视口区高度固固定?
        if (this->view_size.height > 0.f) {
            flag |= LongUI::Flag_ViewHeightFixed;
        }
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
        UIControl::MakeFloats(
            node.attribute(LongUI::XMLAttribute::BorderRound).value(),
            &m_2fBorderRdius.width,
            sizeof(m_2fBorderRdius) / sizeof(m_2fBorderRdius.width)
            );
        // 边框颜色
        UIControl::MakeColor(
            node.attribute("disabledbordercolor").value(), 
            m_aBorderColor[Status_Disabled]
            );
        UIControl::MakeColor(
            node.attribute("normalbordercolor").value(), 
            m_aBorderColor[Status_Normal]
            );
        UIControl::MakeColor(
            node.attribute("hoverbordercolor").value(), 
            m_aBorderColor[Status_Hover]
            );
        UIControl::MakeColor(
            node.attribute("pushedbordercolor").value(), 
            m_aBorderColor[Status_Pushed]
            );
    }
    // 修改flag
    force_cast(this->flags) = static_cast<LongUIFlag>(this->flags | (flag));
}

// 析构函数
LongUI::UIControl::~UIControl() noexcept {
    ::SafeRelease(m_pRenderTarget);
    ::SafeRelease(m_pBrush_SetBeforeUse);
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
        break;
    case LongUI::RenderType::Type_Render:
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 渲染边框
        if (m_fBorderWidth > 0.f) {
            D2D1_ROUNDED_RECT brect; this->GetBorderRect(brect.rect);
            m_pBrush_SetBeforeUse->SetColor(&m_colorBorderNow);
            if (m_2fBorderRdius.width > 0.f && m_2fBorderRdius.height > 0.f) {
                brect.radiusX = m_2fBorderRdius.width;
                brect.radiusY = m_2fBorderRdius.height;
                //m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                m_pRenderTarget->DrawRoundedRectangle(&brect, m_pBrush_SetBeforeUse, m_fBorderWidth);
                //m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
            else {
                m_pRenderTarget->DrawRectangle(&brect.rect, m_pBrush_SetBeforeUse, m_fBorderWidth);
            }
        }
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}


// UI控件: 刷新
void LongUI::UIControl::Update() noexcept {
    // 处理了
    if (m_bControlSizeChangeHandled) {
        m_bControlSizeChanged = false;
        m_bControlSizeChangeHandled = false;
    }
}

// UI控件: 重建
auto LongUI::UIControl::Recreate(LongUIRenderTarget* target) noexcept ->HRESULT {
    ::SafeRelease(m_pRenderTarget);
    ::SafeRelease(m_pBrush_SetBeforeUse);
    m_pRenderTarget = ::SafeAcquire(target);
    m_pBrush_SetBeforeUse = static_cast<decltype(m_pBrush_SetBeforeUse)>(
        UIManager.GetBrush(LongUICommonSolidColorBrushIndex)
        );
    return target ? S_OK : E_INVALIDARG;
}

// 创建字符串
bool LongUI::UIControl::MakeString(const char* data, CUIString& str) noexcept {
    if (!data || !*data) return false;
    wchar_t buffer[LongUIStringBufferLength];
    // 转码
    register auto length = LongUI::UTF8toWideChar(data, buffer);
    buffer[length] = L'\0';
    // 设置字符串
    str.Set(buffer, length);
    return true;
}

// 创建浮点
bool LongUI::UIControl::MakeFloats(const char* sdata, float* fdata, int size) noexcept {
    if (!sdata || !*sdata) return false;
    // 断言
    assert(fdata && size && "bad argument");
    // 拷贝数据
    char buffer[LongUIStringBufferLength];
    ::strcpy_s(buffer, sdata);
    char* index = buffer;
    const char* to_parse = buffer;
    // 遍历检查
    bool new_float = true;
    while (size) {
        char ch = *index;
        // 分段符?
        if (ch == ',' || ch == ' ' || !ch) {
            if (new_float) {
                *index = 0;
                *fdata = ::LongUI::AtoF(to_parse);
                ++fdata;
                --size;
                new_float = false;
            }
        }
        else if (!new_float) {
            to_parse = index;
            new_float = true;
        }
        // 退出
        if (!ch) break;
        ++index;
    }
    return true;
}


// 16进制
unsigned int __fastcall Hex2Int(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    else {
        return c - '0';
    }
}

#define white_space(c) ((c) == ' ' || (c) == '\t')


// 获取颜色表示
bool LongUI::UIControl::MakeColor(const char* data, D2D1_COLOR_F& color) noexcept {
    if (!data || !*data) return false;
    // 获取有效值
    while (white_space(*data)) ++data;
    // 以#开头?
    if (*data == '#') {
        color.a = 1.f;
        // #RGB
        if (data[4] == ' ' || !data[4]) {
            color.r = static_cast<float>(::Hex2Int(*++data)) / 15.f;
            color.g = static_cast<float>(::Hex2Int(*++data)) / 15.f;
            color.b = static_cast<float>(::Hex2Int(*++data)) / 15.f;
        }
        // #RRGGBB
        else if (data[7] == ' ' || !data[7]) {
            color.r = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
        }
        // #AARRGGBB
        else {
            color.a = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.r = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((::Hex2Int(*++data) << 4) | (::Hex2Int(*++data))) / 255.f;
        }
        return true;
    }
    // 浮点数组
    else {
        return UIControl::MakeFloats(data, reinterpret_cast<float*>(&color), 4);
    }
}

// LongUI::UIControl 注册回调事件
void LongUI::UIControl::SetEventCallBack(
    const wchar_t* control_name, LongUI::Event event, LongUIEventCallBack call) noexcept {
    assert(control_name && call&&  "bad argument");
    UIControl* control = m_pWindow->FindControl(control_name);
    assert(control && " no control found");
    if (!control) return;
    // 自定义消息?
    if (event >= LongUI::Event::Event_CustomEvent) {
        UIManager.configure->SetEventCallBack(event, call, control, this);
        return;
    }
    switch (event)
    {
    case LongUI::Event::Event_ButtoClicked:
        longui_cast<UIButton*>(control)->RegisterClickEvent(call, this);
        break;
    case LongUI::Event::Event_EditReturn:
        //longui_cast<UIEdit*>(control)->RegisterReturnEvent(call, this);
        break;
    case LongUI::Event::Event_SliderValueChanged:
        longui_cast<UISlider*>(control)->RegisterValueChangedEvent(call, this);
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
auto LongUI::UIControl::GetTakingUpHeight() const noexcept -> float{
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
auto LongUI::UIControl::SetTakingUpWidth(float w) noexcept -> void {
    // 设置
    auto new_vwidth = w - this->GetNonContentWidth();
    if (new_vwidth != this->view_size.width) {
        this->view_size.width = new_vwidth;
        this->SetControlSizeChanged();
    }
    // 检查
    if (this->view_size.width < 0.f) {
        UIManager << DL_Hint << this
            << "viewport's width < 0: " << this->view_size.width << endl;
    }
}

// 设置占用高度
auto LongUI::UIControl::SetTakingUpHeight(float h) noexcept -> void LongUINoinline {
    // 设置
    auto new_vheight = h - this->GetNonContentHeight();
    if (new_vheight != this->view_size.height) {
        this->view_size.height = new_vheight;
        this->SetControlSizeChanged();
    }
    // 检查
    if (this->view_size.height < 0.f) {
        UIManager << DL_Hint << this
            << "viewport's height < 0: " << this->view_size.height << endl;
    }
}

// 获取占用/剪切矩形
void LongUI::UIControl::GetRectAll(D2D1_RECT_F& rect) const noexcept {
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
    float xx = this->view_pos.x + this->margin_rect.left + m_fBorderWidth;
    float yy = this->view_pos.y + this->margin_rect.top + m_fBorderWidth;
    // 顶级控件
    if (this->IsTopLevel()) {
        this->world = D2D1::Matrix3x2F::Translation(xx, yy);
    }
    // 非顶级控件
    else {
        // 检查
        xx += this->parent->offset.x;
        yy += this->parent->offset.y;
        // 转换
        this->world = D2D1::Matrix3x2F::Translation(xx, yy) * this->parent->world;
    }
}

// 获得世界转换矩阵 for 边缘控件
void LongUI::UIMarginalable::RefreshWorldMarginal() noexcept {
    float xx = this->view_pos.x + this->margin_rect.left + m_fBorderWidth;
    float yy = this->view_pos.y + this->margin_rect.top + m_fBorderWidth;
    this->world = D2D1::Matrix3x2F::Translation(xx, yy) * this->parent->world;
    assert(this->IsTopLevel() == false);
    constexpr int aa = sizeof(UIContainer);
}
// -------------------------------------------------------
// UILabel
// -------------------------------------------------------
// UILabel: do event 事件处理
bool LongUI::UILabel::DoEvent(const LongUI::EventArgument& arg) noexcept {
    UNREFERENCED_PARAMETER(arg);
    return false;
}

// Render 渲染 
void LongUI::UILabel::Render(RenderType type) const noexcept {
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
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 渲染文字
        m_text.Render(0.f, 0.f);
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UILabel: 刷新
void LongUI::UILabel::Update() noexcept {
    // 改变了大小
    if(this->IsControlSizeChanged()) {
        // 设置大小
        m_text.SetNewSize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    return Super::Update();
}


/*/ UILabel 构造函数
LongUI::UILabel::UILabel(pugi::xml_node node) noexcept: Super(node), m_text(node) {
    //m_bInitZoneChanged = true;
}
*/


// UILabel::CreateControl 函数
auto LongUI::UILabel::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UILabel>(
            node,
            [=](void* p) noexcept { new(p) UILabel(node); }
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


// recreate 重建
/*HRESULT LongUI::UILabel::Recreate(LongUIRenderTarget* newRT) noexcept {
// 断言
return Super::Recreate(newRT);
}*/

// close this control 关闭控件
void LongUI::UILabel::Cleanup() noexcept {
    delete this;
}


// -------------------------------------------------------
// UIButton
// -------------------------------------------------------

// Render 渲染 
void LongUI::UIButton::Render(RenderType type) const noexcept {
    switch (type)
    {
        D2D1_RECT_F draw_rect;
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        //Super::Render(LongUI::RenderType::Type_RenderBackground);
        // 本类背景, 更新刻画地区
        this->GetViewRect(draw_rect);
#ifdef _DEBUG
        if (m_strControlName == L"3") {
            int bk = 9;
        }
        /*{
            AutoLocker;
            UIManager << DL_Hint << this << this->visible_rect << LongUI::endl;
        }*/
#endif
        // 渲染部件
        m_uiElement.Render(draw_rect);
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UI按钮: 刷新
void LongUI::UIButton::Update() noexcept {
    // 更新计时器
    UIElement_Update(m_uiElement);
    return Super::Update();
}

// UIButton 构造函数
LongUI::UIButton::UIButton(pugi::xml_node node)noexcept: Super(node), m_uiElement(node){
    // 初始化代码
    m_uiElement.GetByType<Element::Basic>().Init(node);
    if (m_uiElement.GetByType<Element::Meta>().IsOK()) {
        m_uiElement.SetElementType(Element::Meta);
    }
    else {
        m_uiElement.SetElementType(Element::BrushRect);
    }
    // 特殊
    m_uiElement.GetByType<Element::Basic>().SetNewStatus(Status_Normal);
    m_uiElement.GetByType<Element::Basic>().SetNewStatus(Status_Normal);
    constexpr int azz = sizeof(m_uiElement);
}


// UIButton::CreateControl 函数
auto LongUI::UIButton::CreateControl(CreateEventType type,pugi::xml_node node) noexcept ->UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIButton>(
            node,
            [=](void* p) noexcept { new(p) UIButton(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// do event 事件处理
bool LongUI::UIButton::DoEvent(const LongUI::EventArgument& arg) noexcept {
    //--------------------------------------------------
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetFocus:
            // 设置焦点: 要求焦点
            return true;
        case LongUI::Event::Event_KillFocus:
            // 释放焦点:
            m_tarStatusClick = LongUI::Status_Normal;
            return true;
        case LongUI::Event::Event_MouseEnter:
            // 鼠标移进: 设置UI元素状态
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Hover);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Hover];
            break;
        case LongUI::Event::Event_MouseLeave:
            // 鼠标移出: 设置UI元素状态
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Normal);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Normal];
            break;
        }
    }
    else {
        switch (arg.msg)
        {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Pushed);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Pushed];
            break;
        case WM_LBUTTONUP:
            if (m_pWindow->IsReleasedControl(this)) {
                bool rec = false;
                // 保留信息
                force_cast(arg.sender) = this;   auto tempmsg = arg.msg;
                force_cast(arg.event) = LongUI::Event::Event_ButtoClicked;
                m_tarStatusClick = LongUI::Status_Hover;
                // 检查脚本
                if (m_script.script) {
                    UIManager.script->Evaluation(m_script, arg);
                }
                // 检查是否有事件回调
                if (m_eventClick) {
                    rec = m_eventClick(m_pClickTarget, this);
                }
                // 否则发送事件到窗口
                else {
                    rec = m_pWindow->DoEvent(arg);
                }
                // 设置状态
                UIElement_SetNewStatus(m_uiElement, m_tarStatusClick);
                m_colorBorderNow = m_aBorderColor[m_tarStatusClick];
                m_pWindow->ReleaseCapture();
                // 取消
                force_cast(arg.sender) = nullptr;
                force_cast(arg.msg) = tempmsg;
            }
            break;
        }
    }
    return Super::DoEvent(arg);
}

// recreate 重建
auto LongUI::UIButton::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    // 重建元素
    m_uiElement.Recreate(newRT);
    // 父类处理
    return Super::Recreate(newRT);
}

// 关闭控件
void LongUI::UIButton::Cleanup() noexcept {
    delete this;
}


// -------------------------------------------------------
// UIEdit
// -------------------------------------------------------

// UI基本编辑控件
void LongUI::UIEditBasic::Render(RenderType type) const noexcept {
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        __fallthrough;
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
    case LongUI::RenderType::Type_RenderForeground:
        m_text.Render(0.f, 0.f);
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UI基本编辑框: 刷新
void LongUI::UIEditBasic::Update() noexcept {
    // 改变了大小
    if (this->IsControlSizeChanged()) {
        // 设置大小
        m_text.SetNewSize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 刷新
    m_text.Update();
    return Super::Update();
}

// UI基本编辑控件
bool  LongUI::UIEditBasic::DoEvent(const LongUI::EventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // LongUI 消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            return true;
        case LongUI::Event::Event_DragEnter:
            return m_text.OnDragEnter(arg.dataobj_cf, arg.outeffect_cf);
        case LongUI::Event::Event_DragOver:
            return m_text.OnDragOver(pt4self.x, pt4self.y);
        case LongUI::Event::Event_DragLeave:
            return m_text.OnDragLeave();
        case LongUI::Event::Event_Drop:
            return m_text.OnDrop(arg.dataobj_cf, arg.outeffect_cf);
        case LongUI::Event::Event_MouseEnter:
            m_pWindow->now_cursor = m_hCursorI;
            return true;
        case LongUI::Event::Event_MouseLeave:
            m_pWindow->now_cursor = m_pWindow->default_cursor;
            return true;
        case LongUI::Event::Event_SetFocus:
            m_text.OnSetFocus();
            return true;
        case LongUI::Event::Event_KillFocus:
            m_text.OnKillFocus();
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
            m_text.OnKey(static_cast<uint32_t>(arg.wParam_sys));
            break;
        case WM_CHAR:
            m_text.OnChar(static_cast<char32_t>(arg.wParam_sys));
            break;
        case WM_MOUSEMOVE:
            // 拖拽?
            if (arg.wParam_sys & MK_LBUTTON) {
                m_text.OnLButtonHold(pt4self.x, pt4self.y);
            }
            break;
        case WM_LBUTTONDOWN:
            m_text.OnLButtonDown(pt4self.x, pt4self.y, !!(arg.wParam_sys & MK_SHIFT));
            break;
        case WM_LBUTTONUP:
            m_text.OnLButtonUp(pt4self.x, pt4self.y);
            break;
        }
    }
    return true;
}

// close this control 关闭控件
HRESULT LongUI::UIEditBasic::Recreate(LongUIRenderTarget* target) noexcept {
    m_text.Recreate(target);
    return Super::Recreate(target);
}

// close this control 关闭控件
void LongUI::UIEditBasic::Cleanup() noexcept {
    delete this;
}


// UIEditBasic::CreateControl 函数
LongUI::UIControl* LongUI::UIEditBasic::CreateControl(CreateEventType type,pugi::xml_node node) noexcept {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIEditBasic>(
            node,
            [=](void* p) noexcept { new(p) UIEditBasic(node); }
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


// 构造函数
LongUI::UIEditBasic::UIEditBasic(pugi::xml_node node) noexcept
    :  Super(node), m_text(this, node) {
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
bool LongUI::UILabel::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UILabel";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UILabel";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UILabel>()
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
#endif