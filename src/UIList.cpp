#include "LongUI.h"
#include <algorithm>

// ----------------------------------------------------------------------------
// -------------------------------- UIList ------------------------------------
// ----------------------------------------------------------------------------
// UI列表控件: 构造函数
LongUI::UIList::UIList(UIContainer* cp, pugi::xml_node node) noexcept :Super(cp, node) {
    try {
        m_controls.reserve(100);
    }
    catch (...) {
        assert(!"oom just less 1kb!");
    }
    if (node) {
        const char* str = nullptr;
        // 行高度
        if ((str = node.attribute("lineheight").value())) {
            m_fLineHeight = LongUI::AtoF(str);
        }
        // 行模板
        if ((str = node.attribute("linetemplate").value())) {
            // 检查长度
            register auto len = Helper::MakeCC(str);
            // 有效
            if (len) {
                m_bufLineTemplate.NewSize(len);
                Helper::MakeCC(str, m_bufLineTemplate.GetData());
            }
        }
    }
}

// 获取参考控件
auto LongUI::UIList::get_referent_control() const noexcept -> UIListLine* {
    if (m_pHeader) {
        return m_pHeader;
    }
    else {
        if (m_controls.empty()) return nullptr;
        return longui_cast<UIListLine*>(static_cast<UIControl*>(m_controls.front()));
    }
}

// UIList: 重建
auto LongUI::UIList::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    for (auto voidctrl : m_controls) {
        auto ctrl = reinterpret_cast<UIControl*>(voidctrl);
        hr = ctrl->Recreate();
        assert(SUCCEEDED(hr));
    }
    return Super::Recreate();
}

// 查找子控件
auto LongUI::UIList::FindChild(const D2D1_POINT_2F& pt) noexcept -> UIControl* {
    auto ctrl = Super::FindChild(pt);
    if (ctrl) return ctrl;
    // TODO: 利用list特性优化
    for (auto vctrl : m_controls) {
        ctrl = static_cast<UIControl*>(vctrl);
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            return ctrl;
        }
    }
    return nullptr;
}

// push!
void LongUI::UIList::PushBack(UIControl* child) noexcept {
    try {
        m_controls.push_back(child);
        this->after_insert(child);
    }
    catch (...) {
        assert(!"exp!");
    }
}

// 插入
auto LongUI::UIList::Insert(uint32_t index, UIControl* child) noexcept {
    try {
        m_controls.insert(m_controls.begin() + index, child);
        this->after_insert(child);
    }
    catch (...) {
        assert(!"exp!");
    }
}

// UI列表控件: 析构函数
LongUI::UIList::~UIList() noexcept {
    // 线性容器就是不用考虑next指针
    for (auto voidctrl : m_controls) {
        auto ctrl = reinterpret_cast<UIControl*>(voidctrl);
        ctrl->Cleanup();
    }
}

// 移除
void LongUI::UIList::RemoveJust(UIControl* child) noexcept {
    auto itr = std::find(m_controls.cbegin(), m_controls.cend(), child);
    if (itr == m_controls.cend()) {
        assert("control not found");
        return;
    }
    m_controls.erase(itr);
}

// 插入一个行模板
void LongUI::UIList::InsertInlineTemplate(uint32_t index) noexcept {
    auto ctrl = static_cast<UIListLine*>(
        UIListLine::CreateControl(this->CET(), pugi::xml_node())
        );
    if (ctrl) {
        // 添加子控件
        for (const auto& data : m_bufLineTemplate) {
            ctrl->Insert(ctrl->end(), UIManager.CreateControl(ctrl, data.id, data.func));
        }
        // 插入
        this->Insert(index, ctrl);
    }
}

// 设置元素宽度
void LongUI::UIList::SetElementWidth(uint32_t index, float width) noexcept {
    // 循环
    for (auto vctrl : m_controls) {
        auto line = longui_cast<LongUI::UIListLine*>(vctrl);
        assert(line && "bad");
        assert(index < line->GetCount()  && "out of range");
        if (index < line->GetCount()) {
            auto ele = line->GetAt(index);
            ele->SetWidth(width);
            line->SetControlSizeChanged();
        }
    }
    this->SetControlSizeChanged();
}

// UI列表: 事件处理
bool LongUI::UIList::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 父类分发事件
            Super::DoEvent(arg);
            // 处理一下
            this->init_layout();
            return true;
        default:
            break;
        }
    }
    return Super::DoEvent(arg);
}

// UIList: 初始化布局
void LongUI::UIList::init_layout() noexcept {
    auto rctrl = this->get_referent_control();
    if (rctrl) {
        // 缓存
        /*EzContainer::SmallBuffer<float, 32> buffer;
        buffer.NewSize(rctrl->GetCount());
        this->set_element_count(rctrl->GetCount());
        // 宽度
        int index = 0;
        for (auto ctrl : (*rctrl)) {
            buffer[index] = ctrl->GetWidth();
            ++index;
        }
        this->ChangeElementWidth(buffer.GetData());*/
    }
}

// 设置元素数量
void LongUI::UIList::set_element_count(uint32_t length) noexcept {
    auto old = m_bufLineTemplate.GetCount();
    m_bufLineTemplate.NewSize(length);
    // 变长了
    if (old < m_bufLineTemplate.GetCount()) {
        for (auto i = old; i < m_bufLineTemplate.GetCount(); ++i) {
            m_bufLineTemplate[i].id = 0;
            m_bufLineTemplate[i].func = UIText::CreateControl;
        }
    }
}

// UIList: 渲染函数
void LongUI::UIList::Render(RenderType type) const noexcept {
    // 帮助器
    Super::RenderHelper<Super>(m_controls.begin(), m_controls.end(), type);
}

// UIList: 刷新
void LongUI::UIList::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(m_controls.begin(), m_controls.end());
#ifdef _DEBUG
    // 必须一致
    if (this->IsNeedRefreshWorld() && m_pHeader && m_controls.size() && m_controls[0]) {
        auto line = longui_cast<LongUI::UIListLine*>(m_controls[0]);
        assert(m_pHeader->GetCount() == line->GetCount() && "out of sync for child number");
    }
#endif
}


// 更新子控件布局
void LongUI::UIList::RefreshLayout() noexcept {
    if (m_controls.empty()) return;
    // 第二次
    float index = 0.f;
    auto first = longui_cast<LongUI::UIListLine*>(m_controls[0]);
    float widthtt = first->GetContentWidthZoomed();
    if (widthtt == 0.f) widthtt = this->GetViewWidthZoomed();
    for (auto voidctrl : m_controls) {
        auto ctrl = reinterpret_cast<UIControl*>(voidctrl);
        // 设置控件高度
        ctrl->SetWidth(widthtt);
        ctrl->SetHeight(m_fLineHeight);
        // 不管如何, 修改!
        ctrl->SetControlSizeChanged();
        ctrl->SetLeft(0.f);
        ctrl->SetTop(m_fLineHeight * index);
        ++index;
    }
    // 设置
    m_2fContentSize.width = widthtt;
    m_2fContentSize.height = m_fLineHeight * this->GetCount();
}

// 清理UI列表控件
void LongUI::UIList::Cleanup() noexcept {
    delete this;
}


// UI列表控件: 创建控件
auto LongUI::UIList::CreateControl(CreateEventType type, pugi::xml_node node) 
noexcept -> UIControl* {
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
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = CreateWidthCET<LongUI::UIList>(type, node);
        // OOM
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// ---------------------------- UIListLine! --------------------------------
// ----------------------------------------------------------------------------

// UI列表元素控件: 构造函数
LongUI::UIListLine::UIListLine(UIContainer* cp, pugi::xml_node node) 
noexcept : Super(cp, node) {
    // listline 特性: 宽度必须固定
    //auto flag = this->flags | Flag_WidthFixed;
    if (node) {

    }
    //force_cast(this->flags) = flag;
}

// 刷新UI列表元素控件
void LongUI::UIListLine::Update() noexcept {
    // 检查宽度
    if (m_bFirstUpdate) {
        m_bFirstUpdate = false;
        // 取消属性
        for (auto ctrl : (*this)) {
            if (ctrl->view_size.width <= 0.f) {
                force_cast(ctrl->flags) &= (~Flag_WidthFixed);
            }
        }
        Super::Update();
        // 添加属性
        for (auto ctrl : (*this)) {
            force_cast(ctrl->flags) |= Flag_WidthFixed;
        }
        return;
    }
    return Super::Update();
}

// 清理UI列表元素控件
void LongUI::UIListLine::Cleanup() noexcept {
    delete this;
}


// UI列表元素控件: 创建控件
auto LongUI::UIListLine::CreateControl(CreateEventType type, pugi::xml_node node) 
noexcept -> UIControl* {
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
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = CreateWidthCET<LongUI::UIListLine>(type, node);
        // OOM
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// ----------------------------- UIListHeader ---------------------------------
// ----------------------------------------------------------------------------

// UI列表头控件: 构造函数
LongUI::UIListHeader::UIListHeader(UIContainer* cp, pugi::xml_node node) 
noexcept: Super(cp, node) {
    // 支持模板子节点
    auto flag = this->flags | Flag_InsertTemplateChild;
    if (node) {
        const char* str = nullptr;
        // 行高度
        if ((str = node.attribute("lineheight").value())) {
            m_fLineHeight = LongUI::AtoF(str);
        }
        // 分隔符宽度
        if ((str = node.attribute("sepwidth").value())) {
            m_fSepwidth = LongUI::AtoF(str);
        }
    }
    force_cast(this->flags) = flag;
}

// UI列表头: 事件处理
void LongUI::UIListHeader::Update() noexcept {
    // 与父对象保持一样的X偏移量
    this->SetOffsetX(this->parent->GetOffsetX());
    // 父类刷新
    return Super::Update();
}


// UI列表头: 事件处理
bool LongUI::UIListHeader::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 设置列表头
            longui_cast<UIList*>(this->parent)->SetHeader(this);
            __fallthrough;
        default:
            break;
        }
    }
    return Super::DoEvent(arg);
}

// UI列表头: 鼠标事件处理
bool LongUI::UIListHeader::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // -------------------------- hover it?
    auto get_sep_hovered_control = [this, &arg]() noexcept {
        auto realsep = m_fSepwidth * this->world._11 * this->GetZoomX();
        // 区间修正
        float data[2] = { arg.pt.x, arg.pt.x };
        data[realsep < 0.f] -= realsep;
        // 循环查找
        auto index = 0ui32;
        for (auto ctrl : (*this)) {
            if (ctrl->visible && ctrl->visible_rect.right > ctrl->visible_rect.left
                && ctrl->visible_rect.right >= data[0]
                && ctrl->visible_rect.right < data[1]) {
                m_indexSepHovered = index;
                return ctrl;
            }
            ++index;
        }
        return static_cast<UIControl*>(nullptr);
    };
    // -------------------------- on mouse move
    auto on_mouse_move = [this, &arg](UIControl* hovered) noexcept {
        if (hovered) {
            m_pWindow->now_cursor = m_hCursor;
            // 拖拽刷新
            if (m_pSepHovered && (arg.sys.wParam & MK_LBUTTON)) {
                auto distance = arg.pt.x - m_fLastMousePosX;
                distance *= m_pSepHovered->world._11;
                auto tarwidth = m_pSepHovered->GetWidth() + distance;
                // 有效
                if (tarwidth > m_fLineHeight) {
                    m_fLastMousePosX = arg.pt.x;
                    m_pSepHovered->SetWidth(m_pSepHovered->GetWidth() + distance);
                    longui_cast<LongUI::UIList*>(this->parent)->SetElementWidth(m_indexSepHovered, tarwidth);
                    m_pWindow->Invalidate(this->parent);
                    this->SetControlSizeChanged();
                }
            }
        }
        else {
            m_pWindow->ResetCursor();
        }
    };
    // -------------------------- real method
    // 有效
    if (m_fSepwidth != 0.f) {
        // 查找控件
        auto hover_sep = m_pSepHovered;
        if(!hover_sep) hover_sep = get_sep_hovered_control();
        // XXX: 逻辑
        bool handled = !!hover_sep;
        // 分类处理
        switch (arg.event)
        {
        case LongUI::MouseEvent::Event_MouseLeave:
            m_pWindow->ResetCursor();
            m_indexSepHovered = 0;
            break;
        case LongUI::MouseEvent::Event_MouseMove:
            on_mouse_move(hover_sep);
            if (hover_sep) {
            }
            break;
        case LongUI::MouseEvent::Event_LButtonDown:
            if (hover_sep) {
                m_pSepHovered = hover_sep;
                m_pWindow->SetCapture(this);
                m_fLastMousePosX = arg.pt.x;
            }
            break;
        case LongUI::MouseEvent::Event_LButtonUp:
            if (m_pSepHovered) {
                m_pWindow->ReleaseCapture();
                m_pSepHovered = nullptr;
                m_indexSepHovered = 0;
            }
            break;
        case LongUI::MouseEvent::Event_RButtonUp:
            break;
        }
        // 处理了
        if (handled) return true;
    }
    return Super::DoMouseEvent(arg);
}

// 清理UI列表头控件
void LongUI::UIListHeader::Cleanup() noexcept {
    delete this;
}

// 刷新UI列表头控件边界宽度
void LongUI::UIListHeader::UpdateMarginalWidth() noexcept {
    this->marginal_width = m_fLineHeight + this->margin_rect.top + this->margin_rect.bottom;
}

// 创建UI列表头
auto LongUI::UIListHeader::CreateControl(CreateEventType type, pugi::xml_node node) noexcept -> UIControl* {
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
        // 允许
        if (!node) {
            UIManager << DL_Log<< L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = CreateWidthCET<LongUI::UIListHeader>(type, node);
        // OOM
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// --------------------------------- Menu -------------------------------------
// ----------------------------------------------------------------------------

// 摧毁弹出菜单
void LongUI::CUIMenu::Destroy() noexcept {
    if (m_hMenu) {
        ::DestroyMenu(m_hMenu);
        m_hMenu = nullptr;
    }
}

// 直接创建才菜单
bool LongUI::CUIMenu::Create() noexcept {
    assert(!m_hMenu && "cannot create again!");
    m_hMenu = ::CreatePopupMenu();
    return !!m_hMenu;
}

// 使用XML字符串创建菜单
bool LongUI::CUIMenu::Create(const char * xml) noexcept {
    pugi::xml_document document;
    auto re = document.load_string(xml);
    // 错误
    if (re.status) {
        assert(!"failed to load string");
        ::MessageBoxA(
            nullptr, 
            re.description(), 
            "<LongUI::CUIMenu::Create>: Failed to Parse/Load XML",
            MB_ICONERROR
            );
        return false;
    }
    // 创建节点
    return this->Create(document.first_child());
}

// 使用XML节点创建菜单
bool LongUI::CUIMenu::Create(pugi::xml_node node) noexcept {
    UNREFERENCED_PARAMETER(node);
    assert(!m_hMenu && "cannot create again!");
    m_hMenu = ::CreatePopupMenu();
    return !!m_hMenu;
}

// 添加物品
bool LongUI::CUIMenu::AppendItem(const ItemProperties& prop) noexcept {
    UNREFERENCED_PARAMETER(prop);
    return false;
}

// 显示菜单
void LongUI::CUIMenu::Show(HWND parent, POINT* OPTIONAL pos) noexcept {
    // 获取坐标
    POINT pt = { 0,0 };  if (pos) pt = *pos; else ::GetCursorPos(&pt);
    // 置前
    ::SetForegroundWindow(parent);
    // 跟踪菜单项的选择
    auto index = ::TrackPopupMenu(m_hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, parent, nullptr);
    if (m_pItemProc) {
        m_pItemProc(index);
    }
}
