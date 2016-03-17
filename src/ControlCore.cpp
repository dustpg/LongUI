#include "LongUI.h"
#include <algorithm>

#ifdef _DEBUG
void longui_dbg_update(LongUI::UIControl* control) noexcept {
    assert(control && "bad argments");
    if (control->debug_updated) {
        auto name = control->name;
        name = nullptr;
    }
}

// longui naemspace
namespace LongUI {
    // debug var
    extern std::atomic_uintptr_t g_dbg_last_proc_window_pointer;
    extern std::atomic<UINT> g_dbg_last_proc_message;
}

// debug functin -- 
void longui_dbg_locked(const LongUI::CUILocker&) noexcept {
    std::uintptr_t ptr = LongUI::g_dbg_last_proc_window_pointer;
    UINT msg = LongUI::g_dbg_last_proc_message;
    auto window = reinterpret_cast<LongUI::UIViewport*>(ptr);
#if 0
    UIManager << DL_Log
        << L"main locker locked @"
        << window
        << L" on message id: "
        << long(msg)
        << LongUI::endl;
#else
    ::OutputDebugStringW(LongUI::Formated(
        L"Main Locker Locked On Msg: %4u @ Window[0x%p - %S]\r\n",
        msg, window, window->name.c_str()
        ));
#endif
}

#endif

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

// longui::impl namespace
namespace LongUI { namespace impl { static float const floatx2[] = { 0.f, 0.f }; } }

/// <summary>
/// Initializes a new instance of the <see cref="LongUI::UIControl" />
/// class with xml node
/// </summary>
/// <param name="parent">The parent for self in control-level</param>
/// <returns></returns>
/// <remarks>
/// For this function, param 'node' could be null node
/// 对于本函数, 参数'node'允许为空
/// <see cref="LongUINullXMLNode" />
/// </remarks>
LongUI::UIControl::UIControl(UIContainer* parent) noexcept :
parent(parent),
context(),
m_state(uint16_t(1 << State_Visible | 1 << State_Enabled)),
level(parent ? (parent->level + 1ui8) : 0ui8),
m_pWindow(parent ? parent->GetWindow() : nullptr) {
    // 溢出错误
    if (this->level == 255) {
        UIManager << DL_Error
            << L"too deep for this tree"
            << LongUI::endl;
        assert(!"too deep");
    }
}

/// <summary>
/// Starts the render.
/// </summary>
/// <param name="time">The time.</param>
/// <returns></returns>
void LongUI::UIControl::StartRender(float time) noexcept {
    this->InvalidateThis();
}


/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
LongUINoinline void LongUI::UIControl::Release() noexcept {
    uint32_t count = --m_u8RefCount; 
    if (!m_u8RefCount) this->cleanup(); 
}

#ifdef _DEBUG
/// <summary>
/// Adds the reference.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::AddRef() noexcept {
    ++m_u8RefCount;
    assert(m_u8RefCount < 127ui8 && "too many ref-count");
    if (m_u8RefCount >= 2) {
        int bk = 9;
    }
}
#endif


/// <summary>
/// News the parent setted.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::NewParentSetted() noexcept {
    assert(this->name == "" && "control grafting cannot used for named control");
    // 修改等级
    force_cast(this->level) = this->parent->level + 1;
    // 修改窗口
    auto new_window = this->parent->GetWindow();
    // TODO: 去除旧窗口
    if (m_pWindow && new_window && m_pWindow != new_window) {

    }
    m_pWindow = new_window;
}

/// <summary>
/// Links the new parent.
/// </summary>
/// <param name="parent">The parent.</param>
/// <returns></returns>
void LongUI::UIControl::LinkNewParent(UIContainer* cp) noexcept {
    assert(cp && "bad argmuent");
    force_cast(this->parent) = cp;
    m_pWindow = cp->GetWindow();
    this->DoLongUIEvent(Event::Event_SetNewParent);
}

/// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
/// <remarks>
/// For this function, param 'node' could be null node
/// 对于本函数, 参数'node'允许为空
/// <see cref="LongUINullXMLNode" />
/// </remarks>
void LongUI::UIControl::initialize(pugi::xml_node node) noexcept  {
#ifdef _DEBUG
    // 没有被初始化
    assert(this->debug_checker.Test(DEBUG_CHECK_INIT) == false && "had been initialized");
    // 断言
    assert(node && "call 'UIControl::initialize()' if no xml-node");
#endif
    // 构造默认
    auto flag = LongUIFlag::Flag_None;
    // 有效?
    {
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
        // 检查布局上下文
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::LayoutContext).value(),
            force_cast(this->context), 
            lengthof<uint32_t>(this->context)
            );
        // 检查背景笔刷
        if (data = node.attribute(LongUI::XMLAttribute::BackgroudBrush).value()) {
            m_idBackgroudBrush = uint16_t(LongUI::AtoI(data));
            if (m_idBackgroudBrush) {
                assert(!m_pBackgroudBrush);
                m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
            }
        }
        // 检查可视性
        this->SetVisible(node.attribute(LongUI::XMLAttribute::Visible).as_bool(true));
        // 检查名称
        if (m_pWindow) {
            auto basestr = node.attribute(LongUI::XMLAttribute::ControlName).value();
#ifdef _DEBUG
            char buffer[128];
            if (!basestr) {
                static long s_dbg_longui_index = 0;
                buffer[0] = 0;
                ++s_dbg_longui_index;
                auto c = std::snprintf(
                    buffer, 128, 
                    "dbg_longui_%s_id_%ld", 
                    node.name(),
                    s_dbg_longui_index
                    );
                assert(c > 0 && "bad std::snprintf call");
                // 小写
                auto mystrlow = [](char* str) noexcept {
                    while (*str) {
                        if (*str >= 'A' && *str <= 'Z') *str += 'a' - 'A';
                        ++str;
                    }
                };
                mystrlow(buffer);
                basestr = buffer;
            }
#endif
            if (basestr) {
                auto namestr = m_pWindow->CopyStringSafe(basestr);
                force_cast(this->name) = namestr;
            }
        }
        // 检查外边距
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::Margin).value(),
            const_cast<float*>(&margin_rect.left),
            sizeof(margin_rect) / sizeof(margin_rect.left)
            );
        // 检查渲染父控件
        if (node.attribute(LongUI::XMLAttribute::IsRenderParent).as_bool(false)) {
            assert(this->parent && "RenderParent but no parent");
            force_cast(this->prerender) = this->parent->prerender;
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
                size, lengthof<uint32_t>(size)
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
        // 禁止
        if (!node.attribute(LongUI::XMLAttribute::Enabled).as_bool(true)) {
            this->SetEnabled(false);
        }
    }
    // 修改flag
    force_cast(this->flags) |= flag;
#ifdef _DEBUG
    // 被初始化
    this->debug_checker.SetTrue(DEBUG_CHECK_INIT);
    if (this->debug_this) {
        UIManager << DL_Log << this << L" created" << LongUI::endl;
    }
    //assert(this->name != "");
#endif
}

/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::initialize() noexcept  {
#ifdef _DEBUG
    // 没有被初始化
    assert(this->debug_checker.Test(DEBUG_CHECK_INIT) == false && "had been initialized");
#endif
    // 构造默认
    auto flag = LongUIFlag::Flag_None;
    flag |= LongUI::Flag_ClipStrictly;
    // 检查名称
    if (m_pWindow) {
#ifdef _DEBUG
        char buffer[128];
        {
            static long s_dbg_longui_index = 0;
            buffer[0] = 0;
            ++s_dbg_longui_index;
            auto c = std::snprintf(
                buffer, 128,
                "dbg_longui_%ls_id_%ld",
                this->GetControlClassName(),
                s_dbg_longui_index
                );
            assert(c > 0 && "bad std::snprintf call");
            // 小写
            auto mystrlow = [](char* str) noexcept {
                while (*str) {
                    if (*str >= 'A' && *str <= 'Z') *str += 'a' - 'A';
                    ++str;
                }
            };
            mystrlow(buffer);
            if (buffer[0]) {
                auto namestr = m_pWindow->CopyStringSafe(buffer);
                force_cast(this->name) = namestr;
            }
        }
#endif
    }
    // 修改flag
    force_cast(this->flags) |= flag;
#ifdef _DEBUG
    // 被初始化
    this->debug_checker.SetTrue(DEBUG_CHECK_INIT);
    if (this->debug_this) {
        UIManager << DL_Log << this << L"created" << LongUI::endl;
    }
    //assert(this->name != "");
#endif
}

// 析构函数
LongUI::UIControl::~UIControl() noexcept {
    LongUI::SafeRelease(m_pBrush_SetBeforeUse);
    LongUI::SafeRelease(m_pBackgroudBrush);
    // 释放脚本占用空间
    if (m_script.script) {
        assert(UIManager.script && "no script interface but data");
        UIManager.script->FreeScript(m_script);
    }
}


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIControl::Update() noexcept {
#ifdef _DEBUG
    void longui_dbg_update(UIControl* c);
    longui_dbg_update(this);
    assert(debug_checker.Test(DEBUG_CHECK_INIT) == true && "not be initialized yet");
    assert(debug_updated == false && "cannot call this more than once");
    debug_updated = true;
#endif
};

// UIControl:: 渲染调用链: 背景
void LongUI::UIControl::render_chain_background() const noexcept {
#ifdef _DEBUG
    // 重复调用检查
    if (this->debug_checker.Test(DEBUG_CHECK_BACK)) {
        CUIDataAutoLocker locker;
        UIManager << DL_Error
            << L"check logic: called twice in one time"
            << this
            << LongUI::endl;
    }
    force_cast(this->debug_checker).SetTrue(DEBUG_CHECK_BACK);
#endif
    if (m_pBackgroudBrush) {
        D2D1_RECT_F rect; this->GetViewRect(rect);
        LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, m_pBackgroudBrush, rect);
    }
}

// UIControl:: 渲染调用链: 前景
void LongUI::UIControl::render_chain_foreground() const noexcept {
#ifdef _DEBUG
    // 重复调用检查
    if (this->debug_checker.Test(DEBUG_CHECK_FORE)) {
        CUIDataAutoLocker locker;
        UIManager << DL_Error
            << L"check logic: called twice in one time"
            << this
            << LongUI::endl;
    }
    force_cast(this->debug_checker).SetTrue(DEBUG_CHECK_FORE);
#endif
    // 渲染边框
    if (m_fBorderWidth > 0.f) {
        D2D1_ROUNDED_RECT brect; this->GetBorderRect(brect.rect);
        m_pBrush_SetBeforeUse->SetColor(&m_colorBorderNow);
#if 0
        if (m_2fBorderRdius.width > 0.f && m_2fBorderRdius.height > 0.f) {
            brect.radiusX = m_2fBorderRdius.width;
            brect.radiusY = m_2fBorderRdius.height;
            UIManager_RenderTarget->DrawRoundedRectangle(&brect, m_pBrush_SetBeforeUse, m_fBorderWidth);
        }
        else {
            UIManager_RenderTarget->DrawRectangle(&brect.rect, m_pBrush_SetBeforeUse, m_fBorderWidth);
        }
#else
        brect.radiusX = m_2fBorderRdius.width;
        brect.radiusY = m_2fBorderRdius.height;
        UIManager_RenderTarget->DrawRoundedRectangle(&brect, m_pBrush_SetBeforeUse, m_fBorderWidth);
#endif
    }
}


// UI控件: 刷新
void LongUI::UIControl::AfterUpdate() noexcept {
    // 控件大小处理了
    if (m_state.Test(State_ChangeSizeHandled)) {
        m_state.SetFalse(State_ChangeLayout);
        m_state.SetFalse(State_ChangeSizeHandled);
    }
    // 世界转换处理了
    if (m_state.Test(State_ChangeWorldHandled)) {
        m_state.SetFalse(State_ChangeWorld);
        m_state.SetFalse(State_ChangeWorldHandled);
    }
#ifdef _DEBUG
    assert(debug_updated && "must call Update() before this");
    debug_updated = false;
    this->debug_checker.SetFalse(DEBUG_CHECK_BACK);
    this->debug_checker.SetFalse(DEBUG_CHECK_MAIN);
    this->debug_checker.SetFalse(DEBUG_CHECK_FORE);
#endif
}

// UI控件: 重建
auto LongUI::UIControl::Recreate() noexcept ->HRESULT {
    // 增加计数
#ifdef _DEBUG
    ++this->debug_recreate_count;
    if (this->debug_this) {
        UIManager << DL_Log
            << "create count: "
            << long(this->debug_recreate_count)
            << LongUI::endl;
    }
    if (debug_recreate_count > 1 && this->debug_this) {
        UIManager << DL_Hint
            << "create count: "
            << long(this->debug_recreate_count)
            << LongUI::endl;
    }
#endif
    // 设备重置再说
    LongUI::SafeRelease(m_pBrush_SetBeforeUse);
    LongUI::SafeRelease(m_pBackgroudBrush);
    m_pBrush_SetBeforeUse = static_cast<decltype(m_pBrush_SetBeforeUse)>(
        UIManager.GetBrush(LongUICommonSolidColorBrushIndex)
        );
    if (m_idBackgroudBrush) {
        m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
    }
    return S_OK;
}

// 测试是否为子孙结点
bool LongUI::UIControl::IsPosterityForSelf(const UIControl* test) const noexcept {
    const auto target = this->level;
    while (test->level > target) test = test->parent;
    return this == test;
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
        this->SetControlLayoutChanged();
    }
    // 检查
    if (this->view_size.width < 0.f && this->parent->view_size.width > 0.f) {
        UIManager << DL_Hint << this
            << "viewport's width < 0: " << this->view_size.width
            << LongUI::endl;
    }
}

// 设置占用高度
auto LongUI::UIControl::SetHeight(float height) noexcept -> void LongUINoinline {
    // 设置
    auto new_vheight = height - this->GetNonContentHeight();
    if (new_vheight != this->view_size.height) {
        force_cast(this->view_size.height) = new_vheight;
        this->SetControlLayoutChanged();
    }
    // 检查
    if (this->view_size.height < 0.f && this->parent->view_size.height > 0.f) {
        UIManager << DL_Hint << this
            << "viewport's height < 0: " << this->view_size.height
            << LongUI::endl;
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

// 父控件视角: 获取占用/剪切矩形
void LongUI::UIControl::GetClipRectFP(D2D1_RECT_F& rect) const noexcept {
    rect.left = -(this->margin_rect.left + m_fBorderWidth);
    rect.top = -(this->margin_rect.top + m_fBorderWidth);
    rect.right = this->view_size.width + this->margin_rect.right + m_fBorderWidth;
    rect.bottom = this->view_size.height + this->margin_rect.bottom + m_fBorderWidth;
    rect.left += this->view_pos.x;
    rect.top += this->view_pos.y;
    rect.right += this->view_pos.x;
    rect.bottom += this->view_pos.y;
}


// 获得世界转换矩阵
void LongUI::UIControl::RefreshWorld() noexcept {
    float xx = this->view_pos.x;
    float yy = this->view_pos.y;
    // 顶级控件
    if (this->IsTopLevel()) {
        this->world = DX::Matrix3x2F::Translation(xx, yy);
    }
    // 非顶级控件
    else {
        // 检查
        xx += this->parent->GetOffsetXZoomed();
        yy += this->parent->GetOffsetYZoomed();
        // 缩放
        auto zx = this->parent->GetZoomX();
        auto zy = this->parent->GetZoomY();
        // 转换
        this->world = DX::Matrix3x2F::Translation(xx, yy)
            * DX::Matrix3x2F::Scale(zx, zy)
            * this->parent->world;
    }
    // 修改了
    this->ControlWorldChangeHandled();
#ifdef _DEBUG
    if (this->debug_this) {
        UIManager << DL_Log << this << "WORLD: " << this->world << LongUI::endl;
    }
#endif
}

/// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIMarginalable::initialize(pugi::xml_node node) noexcept {
    // 必须有效
    assert(node && "call UIMarginalable::initialize() if no xml-node");
    // 链式调用
    Super::initialize(node);
    // 有效结点
    {
        // 获取类型
        auto get_type = [](pugi::xml_node node, MarginalControl bad_match) noexcept {
            // 属性值列表
            const char* mode_list[] = { "left", "top", "right", "bottom", };
            // 设置
            Helper::GetEnumProperties prop;
            prop.values_list = mode_list;
            prop.values_length = lengthof<uint32_t>(mode_list);
            prop.bad_match = static_cast<uint32_t>(bad_match);
            auto value = node.attribute(XMLAttribute::MarginalDirection).value();
            // 调用
            return static_cast<MarginalControl>(GetEnumFromString(value, prop));
        };
        // 获取类型
        force_cast(this->marginal_type) = get_type(node, Control_Unknown);
    }
    // 检查类型
    if (this->marginal_type != Control_Unknown) {
        assert(this->marginal_type < MARGINAL_CONTROL_SIZE && "bad marginal_type");
        force_cast(this->flags) |= Flag_MarginalControl;
    }
    // 本类已被初始化
#ifdef _DEBUG
#endif
}

/// <summary>
/// Initializes witouth specified xml-node.
/// </summary>
/// <returns></returns>
void LongUI::UIMarginalable::initialize() noexcept {
    // 链式调用
    Super::initialize();
    // 有效结点
    {
        // 获取类型
        force_cast(this->marginal_type) = Control_Unknown;
    }
    // 本类已被初始化
#ifdef _DEBUG
#endif
}


// 获得世界转换矩阵 for 边缘控件
void LongUI::UIMarginalable::RefreshWorldMarginal() noexcept {
    float xx = this->view_pos.x /*+ this->margin_rect.left + m_fBorderWidth*/;
    float yy = this->view_pos.y /*+ this->margin_rect.top + m_fBorderWidth*/;
    D2D1_MATRIX_3X2_F identity;
    D2D1_MATRIX_3X2_F* parent_world = &identity;
    // 顶级
    identity = DX::Matrix3x2F::Identity();
    if (this->parent->IsTopLevel()) {
        identity = DX::Matrix3x2F::Identity();
    }
    else {
        auto pp = this->parent;
        xx -= pp->GetLeftMarginOffset();
        yy -= pp->GetTopMarginOffset();
        // 检查
        parent_world = &pp->world;
    }
    // 计算矩阵
    this->world = DX::Matrix3x2F::Translation(xx, yy) ** parent_world;
    // 自己不能是顶级的
    assert(this->IsTopLevel() == false);
    constexpr long aa = sizeof(UIContainer);
}

// ----------------------------------------------------------------------------
// UINull
// ----------------------------------------------------------------------------

// LongUI namespace
namespace LongUI {
    // null control
    class UINull : public UIControl {
        // 父类申明
        using Super = UIControl;
        // clean this control 清除控件
        virtual void cleanup() noexcept override { this->before_deleted(); delete this; }
    public:
        // Render 渲染
        virtual void Render() const noexcept override {}
        // update 刷新
        virtual void Update() noexcept override { Super::Update(); }
        // do event 事件处理
        //virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override { return false; }
    public:
        // 创建控件
        static auto CreateControl(UIContainer* cp, pugi::xml_node node) noexcept -> UIControl* {
            UINull* pControl = nullptr;
            // 判断
            if (!node) {
                UIManager << DL_Hint << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = new(std::nothrow) UINull(cp);
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            else {
                pControl->initialize();
            }
            return pControl;
        }
    public:
        // constructor 构造函数
        UINull(UIContainer* cp) noexcept : Super(cp) {}
        // destructor 析构函数
        ~UINull() noexcept { }
    protected:
        // init
        void initialize(pugi::xml_node node) noexcept { Super::initialize(node); }
        // init without xml-node
        void initialize() noexcept { Super::initialize(); }
#ifdef LongUIDebugEvent
        // debug
        bool debug_do_event(const LongUI::DebugEventInformation& info) const noexcept override {
            switch (info.infomation)
            {
            case LongUI::DebugInformation::Information_GetClassName:
                info.str = L"UINull";
                return true;
            case LongUI::DebugInformation::Information_GetFullClassName:
                info.str = L"::LongUI::UINull";
                return true;
            case LongUI::DebugInformation::Information_CanbeCasted:
                return Super::debug_do_event(info);
            default:
                break;
            }
            return false;
#endif
        }
    };
    // space holder
    class UISpaceHolder final : public UINull {
        // 父类申明
        using Super = UINull;
        // clean this control 清除控件
        virtual void cleanup() noexcept override { }
    public:
        // create
        virtual auto Recreate() noexcept ->HRESULT override { return S_OK; }
    public:
        // constructor 构造函数
        UISpaceHolder() noexcept : Super(nullptr) { Super::initialize(); }
        // destructor 析构函数
        ~UISpaceHolder() noexcept { }
    };
    // 占位控件
    static char g_control[sizeof(UISpaceHolder)];
    // 占位控件初始化
    struct CUISpaceHolderInit { CUISpaceHolderInit() noexcept { reinterpret_cast<UISpaceHolder*>(g_control)->UISpaceHolder::UISpaceHolder(); } } g_init_holder;
    // 获取占位控件
    auto LongUI::UIControl::GetPlaceholder() noexcept -> UIControl* {
        return reinterpret_cast<UIControl*>(&g_control);
    }
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
/// <summary>
/// UIs the container.
/// </summary>
/// <param name="cp">The parent for self in control-level</param>
/// <returns></returns>
LongUI::UIContainer::UIContainer(UIContainer* cp) noexcept : Super(cp), marginal_control() {
    std::memset(force_cast(marginal_control), 0, sizeof(marginal_control));
}


/// <summary>
/// Before_deleteds this instance.
/// </summary>
/// <returns></returns>
LongUINoinline void LongUI::UIContainer::before_deleted() noexcept {
    LongUI::SafeRelease(m_pPopularChild);
    LongUI::SafeRelease(m_pMousePointed);
    // 只有一次 Flag_Container_ExistMarginalControl 可用可不用
    for (auto ctrl : this->marginal_control) {
        if (ctrl)  release_child(ctrl);
    }
#ifdef _DEBUG
    // 调试时清空
    std::memset((void*)(this->marginal_control), 0, sizeof(this->marginal_control));
#endif
    // 链式调用
    Super::before_deleted();
}

/// <summary>
/// Initializes with specified cxml-node
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIContainer::initialize(pugi::xml_node node) noexcept {
    // 必须有效
    assert(node && "call UIContainer::initialize() if no xml-node");
#ifdef _DEBUG
    for (auto ctrl : marginal_control) {
        assert(ctrl == nullptr && "bad action");
    }
#endif
    // 链式调用
    Super::initialize(node);
    // 保留原始外间距
    m_orgMargin = this->margin_rect;
    auto flag = this->flags | Flag_UIContainer;
    // 有效
    {
        // 模板大小
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::TemplateSize).value(),
            &m_2fTemplateSize.width, 2
            );
        // XXX: 渲染依赖属性
        /*if (node.attribute(XMLAttribute::IsHostChildrenAlways).as_bool(false)) {
            flag |= LongUI::Flag_Container_HostChildrenRenderingDirectly;
        }*/
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


/// <summary>
/// Initializes without specified xml-node
/// </summary>
/// <returns></returns>
void LongUI::UIContainer::initialize() noexcept {
#ifdef _DEBUG
    for (auto ctrl : marginal_control) {
        assert(ctrl == nullptr && "bad action");
    }
#endif
    // 链式调用
    Super::initialize();
    // 保留原始外间距
    m_orgMargin = this->margin_rect;
    // 修改flag
    auto flag = this->flags | Flag_UIContainer;
    // 修改完毕
    force_cast(this->flags) = flag;
}

// 插入后处理
void LongUI::UIContainer::after_insert(UIControl* child) noexcept {
    assert(child && "bad argument");
    // 添加到窗口速查表
    if (child->name[0]) {
        m_pWindow->AddNamedControl(child);
    };
    // 大小判断
    if (this->GetChildrenCount() >= 10'000) {
        UIManager << DL_Warning << "the count of children must be"
            " less than 10k because of the precision of float" << LongUI::endl;
    }
    // 检查flag
    if (this->flags & Flag_Container_HostPosterityRenderingDirectly) {
        force_cast(child->prerender) = this->prerender;
        // 子控件也是容器?(不是也无所谓了)
        force_cast(child->flags) |= Flag_Container_HostPosterityRenderingDirectly;
    }
    // 增加引用计数
    child->AddRef();
    // 设置父结点
    assert(child->parent == this);
    // 设置窗口结点
    assert(child->GetWindow() == m_pWindow);
    // 重建资源
    child->Recreate();
    // 修改
    child->SetControlLayoutChanged();
    // 修改
    this->SetControlLayoutChanged();
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIContainer::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
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
    // ------------------------------------ 主函数
    bool done = false;
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 边界控件
            for (auto mctrl : marginal_control) {
                if (mctrl) mctrl->DoEvent(arg);
            }
            done = true;
            break;
        case LongUI::Event::Event_SetNewParent:
            // 修改控件深度
            for (auto mctrl : marginal_control) {
                if (mctrl) {
                    mctrl->NewParentSetted();
                    mctrl->DoEvent(arg);
                }
            }
            done = true;
            break;
        /*case LongUI::Event::Event_NotifyChildren:
            // 不处理
            return true;*/
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
            LongUI::SafeRelease(m_pMousePointed);
        }
        return true;
    }
    // 查找子控件
    auto control_got = this->FindChild(arg.pt);
    // 不可视算没有
    if (control_got && !control_got->GetVisible()) control_got = nullptr;
    // 不同
    if (control_got != m_pMousePointed && arg.event == LongUI::MouseEvent::Event_MouseMove) {
        auto newarg = arg;
        // 有效
        if (m_pMousePointed) {
            // 事件
            newarg.event = LongUI::MouseEvent::Event_MouseLeave;
            m_pMousePointed->DoMouseEvent(newarg);
            // 去除旧引用
            m_pMousePointed->Release();
        }
        // 有效
        if ((m_pMousePointed = control_got)) {
            // 添加新引用
            m_pMousePointed->AddRef();
            // 事件
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
    if (ctrl->GetVisible() && ctrl->visible_rect.right > ctrl->visible_rect.left
        && ctrl->visible_rect.bottom > ctrl->visible_rect.top) {
        // 修改世界转换矩阵
        UIManager_RenderTarget->SetTransform(&ctrl->world);
        // 检查剪切规则
        if (ctrl->flags & Flag_ClipStrictly) {
            D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
            UIManager_RenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        // 渲染
        ctrl->Render();
        // 检查剪切规则
        if (ctrl->flags & Flag_ClipStrictly) {
            UIManager_RenderTarget->PopAxisAlignedClip();
        }
    }
}

// UIContainer: 主景渲染
void LongUI::UIContainer::render_chain_main() const noexcept {
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
    // 父类
    Super::render_chain_main();
}

// 添加边界控件
void LongUI::UIContainer::Push(UIControl* child) noexcept {
    assert(child && "bad argment");
    assert((child->flags & Flag_MarginalControl) && "bad argment");
    if (child && (child->flags & Flag_MarginalControl)) {
        auto mctrl = longui_cast<UIMarginalable*>(child);
        assert(mctrl->marginal_type != UIMarginalable::Control_Unknown && "bad marginal control");
        assert(mctrl->parent == this && "bad child");
        // 错误
        if (this->marginal_control[mctrl->marginal_type]) {
            UIManager << DL_Error
                << "target marginal control has been existd, check xml-layout"
                << LongUI::endl;
            this->release_child(this->marginal_control[mctrl->marginal_type]);
        }
        // 写入
        force_cast(this->marginal_control[mctrl->marginal_type]) = mctrl;
        // 插♂入后
        this->after_insert(mctrl);
        // 推入flag
        force_cast(this->flags) |= Flag_Container_ExistMarginalControl;
    }
}

// 设置受欢迎的子控件
void LongUI::UIContainer::SetPopularChild(UIControl* ctrl) noexcept {
    assert(ctrl && ctrl->parent == this && "bad argument");
    LongUI::SafeRelease(m_pPopularChild);
    m_pPopularChild = LongUI::SafeAcquire(ctrl);
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
    const float this_container_left = this->GetLeft();
    const float this_container_top = this->GetTop();
    assert(this_container_width == this->GetWidth());
    assert(this_container_height == this->GetHeight());
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
        }
    }
    this->RefreshWorld();
    this->RefreshLayout();
}

// UI容器: 刷新
void LongUI::UIContainer::Update() noexcept {
    // 修改自动缩放控件
    if (this->IsNeedRefreshWorld()) {
        auto code = ((this->m_2fTemplateSize.width > 0.f) << 1) |
            (this->m_2fTemplateSize.height > 0.f);
        auto tmpw = this->GetWidth() / m_2fTemplateSize.width;
        auto tmph = this->GetHeight() / m_2fTemplateSize.width;
        switch (code & 0x03)
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
            this->m_2fZoom.width = tmpw;
            this->m_2fZoom.height = tmph;
            break;
        }
    }
    // 修改边界
    if (this->IsControlLayoutChanged()) {
        // 更新布局
        this->RefreshLayout();
        // 刷新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            this->refresh_marginal_controls();
        }
        // 处理
        this->ControlLayoutChangeHandled();
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
    if (this->IsNeedRefreshWorld() && (this->flags & Flag_Container_ExistMarginalControl)) {
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
                        << ctrl->visible_rect << LongUI::endl;
                }
#endif
            }
        }
        // 已处理该消息
        this->ControlLayoutChangeHandled();
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
    // skip if before 'control-tree-finshed'
    // this->AssertMarginalControl();
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
    float target = value;
    if (target != m_2fOffset.x) {
        m_2fOffset.x = target;
        this->SetControlWorldChanged();
    }
}

// 设置垂直偏移值
void LongUI::UIContainer::SetOffsetY(float value) noexcept {
    assert(value > (-1'000'000.f) && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    float target = value;
    if (target != m_2fOffset.y) {
        m_2fOffset.y = target;
        this->SetControlWorldChanged();
    }
}


// ------------------------ HELPER ---------------------------
// sb调用帮助器
bool LongUI::UIControl::call_uievent(const UICallBack& call, SubEvent sb) noexcept(noexcept(call.operator())) {
    // 事件
    LongUI::EventArgument arg;
    arg.event = LongUI::Event::Event_SubEvent;
    arg.sender = this;
    arg.ui.subevent = sb;
    arg.ui.pointer = nullptr;
    arg.ctrl = nullptr;
    // 返回值
    auto code = false;
    // 脚本最先
    if (UIManager.script && m_script.script) {
        auto rc = UIManager.script->Evaluation(this->GetScript(), arg);
        code = rc || code;
    }
    // 回调其次
    if (call.IsOK()) {
        auto rc = call(this);
        code = rc || code;
    }
    // 事件最低
    return m_pWindow->DoEvent(arg) || code;
}

// 延迟清理
void LongUI::UIControl::delay_cleanup() noexcept {
    UIManager.PushDelayCleanup(this);
}