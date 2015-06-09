#include "LongUI.h"


// 要点:
// 1. 更新空间显示区大小
//      -> 有滚动条, 交给滚动条处理
//      -> 没有, 交给自己处理



// 系统按钮:
/*

焦点: 0x3399FF 矩形描边, 并且内边有虚线矩形
0. 禁用: 0xD9灰度 矩形描边; 中心 0xEF灰色
1. 普通: 0xAC灰度 矩形描边; 中心 从上到下0xF0灰色到0xE5灰色渐变
2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变

*/


// TODO: 检查所有控件Render, 需要调用UIControl::Render;

// UIControl 构造函数
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
        m_pScript = UIManager.script;
        const char* data = nullptr;
        // 检查脚本
        if ((data = node.attribute("script").value()) && m_pScript) {
            m_script = m_pScript->AllocScript(data);
        }
        else {
            m_script.data = nullptr;
            m_script.size = 0;
        }
        // 检查渲染父控件
        if (node.attribute("renderparent").as_bool(false)) {
            flag |= LongUI::Flag_RenderParent;
        }
        // 检查名称
        UIControl::MakeString(node.attribute("name").value(), m_strControlName);
        // 检查位置
        UIControl::MakeFloats(node.attribute("pos").value(), const_cast<float*>(&show_zone.left), 4);
        // 检查外边距
        UIControl::MakeFloats(node.attribute("margin").value(), const_cast<float*>(&margin_rect.left), 4);
        // 宽度固定
        if (show_zone.width > 0.f) {
            flag |= LongUI::Flag_WidthFixed;
        }
        // 高度固定
        if (show_zone.height > 0.f) {
            flag |= LongUI::Flag_HeightFixed;
        }
        // 边框大小
        if (data = node.attribute("bordersize").value()) {
            m_fBorderSize = LongUI::AtoF(data);
        }
        // 边框圆角
        UIControl::MakeFloats(node.attribute("borderround").value(),&m_fBorderRdius.width, 4);
        if (data = node.attribute("borderround").value()) {
            m_fBorderSize = LongUI::AtoF(data);
        }
        // 边框颜色
        UIControl::MakeColor(node.attribute("disabledbordercolor").value(), m_aBorderColor[Status_Disabled]);
        UIControl::MakeColor(node.attribute("normalbordercolor").value(), m_aBorderColor[Status_Normal]);
        UIControl::MakeColor(node.attribute("hoverbordercolor").value(), m_aBorderColor[Status_Hover]);
        UIControl::MakeColor(node.attribute("pushedbordercolor").value(), m_aBorderColor[Status_Pushed]);
    }
    else  {
        // 错误
        //UIManager << DL_Warning << L"given a null xml node" << LongUI::endl;
    }
    // 修改flag
    force_cast(this->flags) = static_cast<LongUIFlag>(this->flags | (flag));
}

// 析构函数
LongUI::UIControl::~UIControl() noexcept {
    ::SafeRelease(m_pRenderTarget);
    ::SafeRelease(m_pBrush_SetBeforeUse);
    // 释放脚本占用空间
    if (m_script.data) {
        assert(m_pScript && "no script interface but data");
        m_pScript->FreeScript(m_script);
    }
    // 反注册
    if (this->flags & Flag_NeedRegisterOffScreenRender) {
        m_pWindow->UnRegisterOffScreenRender(this);
    }
}


// 渲染控件
auto LongUI::UIControl::Render(RenderType type) noexcept -> HRESULT {
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        break;
    case LongUI::RenderType::Type_Render:
        m_bDrawPosChanged = false;
        m_bDrawSizeChanged = false;
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 渲染边框
        if (m_fBorderSize > 0.f) {
            D2D1_ROUNDED_RECT rrect;
            rrect.rect = GetDrawRect(this);
            rrect.radiusX = m_fBorderRdius.width;
            rrect.radiusY = m_fBorderRdius.height;
            m_pBrush_SetBeforeUse->SetColor(&m_colorBorderNow);
            //m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
            m_pRenderTarget->DrawRoundedRectangle(
                &rrect, m_pBrush_SetBeforeUse, m_fBorderSize
                );
            //m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
    return S_OK;
}

// 重建
HRESULT LongUI::UIControl::Recreate(LongUIRenderTarget* target) noexcept {
    ::SafeRelease(m_pRenderTarget);
    ::SafeRelease(m_pBrush_SetBeforeUse);
    m_pRenderTarget = ::SafeAcquire(target);
    m_pBrush_SetBeforeUse = static_cast<decltype(m_pBrush_SetBeforeUse)>(
        UIManager.GetBrush(LongUICommonSolidColorBrushIndex)
        );
    return target ? S_OK : E_INVALIDARG;
}

// 转换鼠标的DoEvent
bool LongUI::UIControl::DoEventEx(LongUI::EventArgument& arg) noexcept {
    auto old = arg.pt;
    D2D1_MATRIX_3X2_F* transform;
    if (this->parent) {
        transform = &this->parent->world;
    }
    else {
        assert(this->flags & Flag_UIContainer);
        transform = &static_cast<UIContainer*>(this)->transform;
    }
    // 转化
    arg.pt = LongUI::TransformPointInverse(*transform, arg.pt);
    auto code = this->DoEvent(arg);
    arg.pt = old;
    return code;
}

// 创建字符串
bool LongUI::UIControl::MakeString(const char* data, CUIString& str) noexcept {
    if (!data || !*data) return false;
    wchar_t buffer[LongUIStringBufferLength];
    // 转码
    auto length = LongUI::UTF8toWideChar(data, buffer);
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
    const wchar_t* control_name, LongUI::Event event, LongUICallBack call) noexcept {
    assert(control_name && call&&  "bad argument");
    UIControl* control = m_pWindow->FindControl(control_name);
    assert(control && " no control found");
    if (!control) return;
    // 自定义消息?
    if (event >= LongUI::Event::Event_CustomEvent) {
        UIManager.configure->SetEventCallBack(
            event, call, control, this
            );
        return;
    }
    switch (event)
    {
    case LongUI::Event::Event_ButtoClicked:
        static_cast<UIButton*>(control)->RegisterClickEvent(call, this);
        break;
    case LongUI::Event::Event_EditReturn:
        //static_cast<UIEdit*>(control)->RegisterReturnEvent(call, this);
        break;
    case LongUI::Event::Event_SliderValueChanged:
        static_cast<UISlider*>(control)->RegisterValueChangedEvent(call, this);
        break;
    }
}



// -------------------------------------------------------
// UILabel
// -------------------------------------------------------


// Render 渲染 
auto LongUI::UILabel::Render(RenderType type) noexcept ->HRESULT {
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
        // 文本属于前景
        if (m_bDrawSizeChanged) {
            this->draw_zone = this->show_zone;
            // 设置大小
            m_text.SetNewSize(this->draw_zone.width, this->draw_zone.height);
            // super will do it
            //m_bDrawSizeChanged = false;
        }
        // 渲染文字
        m_text.Render(this->draw_zone.left, this->draw_zone.top);
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
    return S_OK;
}


/*/ UILabel 构造函数
LongUI::UILabel::UILabel(pugi::xml_node node) noexcept: Super(node), m_text(node) {
    //m_bInitZoneChanged = true;
}
*/


// UILabel::CreateControl 函数
auto LongUI::UILabel::CreateControl(pugi::xml_node node) noexcept ->UIControl* {
    if (!node) {
        UIManager << DL_Warning << L"node null" << LongUI::endl;
    }
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UILabel>(
        node,
        [=](void* p) noexcept { new(p) UILabel(node); }
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UILabel::DoEvent(LongUI::EventArgument& arg) noexcept {
    if (arg.sender) {
        if (arg.event == LongUI::Event::Event_FindControl &&
            IsPointInRect(this->show_zone, arg.pt)) {
            arg.ctrl = this;
        }
    }
    return false;
}

// recreate 重建
/*HRESULT LongUI::UILabel::Recreate(LongUIRenderTarget* newRT) noexcept {
// 断言
return Super::Recreate(newRT);
}*/

// close this control 关闭控件
void LongUI::UILabel::Close() noexcept {
    delete this;
}


// -------------------------------------------------------
// UIButton
// -------------------------------------------------------

// Render 渲染 
auto LongUI::UIButton::Render(RenderType type) noexcept ->HRESULT {
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        D2D1_RECT_F draw_rect;
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        //Super::Render(LongUI::RenderType::Type_RenderBackground);
        // 本类背景, 更新刻画地区
        if (m_bDrawSizeChanged) {
            this->draw_zone = this->show_zone;
        }
        draw_rect = GetDrawRect(this);
        m_uiElement.Render(draw_rect);
        {
            auto time = m_pWindow->GetDeltaTime();
            if (time > 0.f) {
                m_uiElement.Update(time);
            }
        }
        // 更新计时器
        //UIElement_Update(m_uiElement);
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
    return S_OK;
}


// UIButton 构造函数
LongUI::UIButton::UIButton(pugi::xml_node node)noexcept: Super(node), m_uiElement(node){
    // 初始化代码
    m_uiElement.GetByType<Element::Basic>().Init(node);
    if (m_uiElement.GetByType<Element::Meta>().IsOK(Status_Normal)) {
        m_uiElement.SetElementType(Element::Meta);
    }
    else {
        m_uiElement.SetElementType(Element::BrushRect);
    }
    // 特殊
    //m_uiElement.SetElementType(Element::BrushRect);
    constexpr int azz = sizeof(m_uiElement);
}


// UIButton::CreateControl 函数
auto LongUI::UIButton::CreateControl(pugi::xml_node node) noexcept ->UIControl* {
    if (!node) {
        UIManager << DL_Warning << L"node null" << LongUI::endl;
    }
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UIButton>(
        node,
        [=](void* p) noexcept { new(p) UIButton(node);}
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}


// do event 事件处理
bool LongUI::UIButton::DoEvent(LongUI::EventArgument& arg) noexcept {
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_FindControl:
            if (IsPointInRect(this->show_zone, arg.pt)) {
                arg.ctrl = this;
            }
            __fallthrough;
        case LongUI::Event::Event_SetFocus:
            return true;
        case LongUI::Event::Event_KillFocus:
            m_tarStatusClick = LongUI::Status_Normal;
            return true;
        case LongUI::Event::Event_MouseEnter:
            //m_bEffective = true;
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Hover);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Hover];
            break;
        case LongUI::Event::Event_MouseLeave:
            //m_bEffective = false;
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Normal);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Normal];
            break;
        }
    }
    else {
        bool rec = false;
        arg.sender = this;   auto tempmsg = arg.msg;
        switch (arg.msg)
        {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Pushed);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Pushed];
            break;
        case WM_LBUTTONUP:
            arg.event = LongUI::Event::Event_ButtoClicked;
            m_tarStatusClick = LongUI::Status_Hover;
            // 检查脚本
            if (m_pScript && m_script.data) {
                m_pScript->Evaluation(m_script, arg);
            }
            // 检查是否有事件回调
            if (m_eventClick) {
                rec = (m_pClickTarget->*m_eventClick)(this);
            }
            // 否则发送事件到窗口
            else {
                rec = m_pWindow->DoEvent(arg);
            }
            arg.msg = tempmsg;
            UIElement_SetNewStatus(m_uiElement, m_tarStatusClick);
            m_colorBorderNow = m_aBorderColor[m_tarStatusClick];
            m_pWindow->ReleaseCapture();
            break;
        }
        arg.sender = nullptr;
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
void LongUI::UIButton::Close() noexcept {
    delete this;
}


// -------------------------------------------------------
// UIEdit
// -------------------------------------------------------


HRESULT LongUI::UIEditBasic::Render(RenderType type) noexcept {
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
        // 更新刻画地区
        if (m_bDrawSizeChanged) {
            this->draw_zone = this->show_zone;
            m_text.SetNewSize(this->draw_zone.width, this->draw_zone.height);
        }
        m_text.Render(this->draw_zone.left, this->draw_zone.top);
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
    return S_OK;
}

// do event 
bool  LongUI::UIEditBasic::DoEvent(LongUI::EventArgument& arg) noexcept {
    // ui msg
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_FindControl: // 查找本控件
            if (IsPointInRect(this->show_zone, arg.pt)) {
                arg.ctrl = this;
            }
            return true;
        case LongUI::Event::Event_FinishedTreeBuliding:
            return true;
        case LongUI::Event::Event_DragEnter:
            return m_text.OnDragEnter(arg.dataobj_cf, arg.outeffect_cf);
        case LongUI::Event::Event_DragOver:
            return m_text.OnDragOver(
                arg.pt.x - this->show_zone.left,
                arg.pt.y - this->show_zone.top
                );
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
    // sys msg
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
                m_text.OnLButtonHold(
                    arg.pt.x - this->show_zone.left,
                    arg.pt.y - this->show_zone.top
                    );
            }
            break;
        case WM_LBUTTONDOWN:
            m_text.OnLButtonDown(
                arg.pt.x - this->show_zone.left,
                arg.pt.y - this->show_zone.top,
                (arg.wParam_sys & MK_SHIFT) > 0
                );
            break;
        case WM_LBUTTONUP:
            m_text.OnLButtonUp(
                arg.pt.x - this->show_zone.left,
                arg.pt.y - this->show_zone.top
                );
            break;
        }
    }
    return false;
}

// close this control 关闭控件
HRESULT    LongUI::UIEditBasic::Recreate(LongUIRenderTarget* target) noexcept {
    m_text.Recreate(target);
    return Super::Recreate(target);
}

// close this control 关闭控件
void    LongUI::UIEditBasic::Close() noexcept {
    delete this;
}


// UIEditBasic::CreateControl 函数
LongUI::UIControl* LongUI::UIEditBasic::CreateControl(pugi::xml_node node) noexcept {
    if (!node) {
        UIManager << DL_Warning << L"node null" << LongUI::endl;
    }
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UIEditBasic>(
        node,
        [=](void* p) noexcept { new(p) UIEditBasic(node);}
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}


// 构造函数
LongUI::UIEditBasic::UIEditBasic(pugi::xml_node node) noexcept
    :  Super(node), m_text(this, node) {
}



