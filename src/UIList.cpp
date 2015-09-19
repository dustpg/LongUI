#include "LongUI.h"
#include <algorithm>

// ----------------------------------------------------------------------------
// -------------------------------- UIList ------------------------------------
// ----------------------------------------------------------------------------
// UI列表控件: 构造函数
LongUI::UIList::UIList(pugi::xml_node node) noexcept :Super(node) {
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
    auto ctrl = static_cast<UIListLine*>(UIListLine::CreateControl(
        Type_CreateControl, pugi::xml_node()));
    if (ctrl) {
        // 添加子控件
        for (const auto& data : m_bufLineTemplate) {
            ctrl->Insert(ctrl->end(), UIManager.CreateControl(data.id, data.func));
        }
        // 插入
        this->Insert(index, ctrl);
    }
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

// 修改元素权重/宽度
void LongUI::UIList::ChangeElementWidth(float widthv[]) noexcept {
    assert(widthv && "bad arguemnt");
    // 遍历LINE
    for (auto cline : m_controls) {
        auto line = longui_cast<UIListLine*>(cline);
        auto index = 0u;
        // 变量元素
        for (auto ele : (*line)) {
            if (widthv[index] >= 0.f) {
                ele->SetWidth(widthv[index]);
            }
            ++index;
        }
    }
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
}


// 更新子控件布局
void LongUI::UIList::RefreshLayout() noexcept {
    // 第二次
    float index = 0.f;
    for (auto voidctrl : m_controls) {
        auto ctrl = reinterpret_cast<UIControl*>(voidctrl);
        // 宽度无效?
        if (ctrl->view_size.width == 0.f) {
            ctrl->SetWidth(this->GetViewWidthZoomed());
        }
        // 设置控件高度
        ctrl->SetHeight(m_fLineHeight);
        // 不管如何, 修改!
        ctrl->SetControlSizeChanged();
        ctrl->SetLeft(0.f);
        ctrl->SetTop(m_fLineHeight * index);
        ++index;
    }
    // 设置
    if (m_controls.size()) {
        m_2fContentSize.width = static_cast<UIControl*>(this->m_controls.front())->GetWidth();
    }
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
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIList>(
            node,
            [=](void* p) noexcept { new(p) UIList(node); }
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


// ----------------------------------------------------------------------------
// ---------------------------- UIListLine! --------------------------------
// ----------------------------------------------------------------------------

// UI列表元素控件: 构造函数
LongUI::UIListLine::UIListLine(pugi::xml_node node) noexcept:Super(node){
    // listline 特性: 宽度必须固定
    auto flag = this->flags | Flag_WidthFixed;
    if (node) {

    }
    force_cast(this->flags) = flag;
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
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIListLine>(
            node,
            [=](void* p) noexcept { new(p) UIListLine(node); }
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


// ----------------------------------------------------------------------------
// ----------------------------- UIListHeader ---------------------------------
// ----------------------------------------------------------------------------

// UI列表头控件: 构造函数
LongUI::UIListHeader::UIListHeader(pugi::xml_node node) noexcept: Super(node) {
    // 支持模板子节点
    auto flag = this->flags | Flag_InsertTemplateChild;
    if (node) {
        const char* str = nullptr;
        // 行高度
        if ((str = node.attribute("lineheight").value())) {
            m_fLineHeight = LongUI::AtoF(str);
        }
    }
    force_cast(this->flags) = flag;
}

// UI列表头: 事件处理
void LongUI::UIListHeader::Update() noexcept {
    // 与父对象保持一样的X偏移量, 会延迟一帧, 被上的故事
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
    case Type_CreateControl:
        if (!node) {
           UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIListHeader>(
            node,
            [=](void* p) noexcept { new(p) UIListHeader(node); }
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
