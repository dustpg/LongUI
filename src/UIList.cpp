#include "LongUI.h"

// ----------------------------------------------------------------------------
// -------------------------------- UIList ------------------------------------
// ----------------------------------------------------------------------------
// UI列表控件: 构造函数
LongUI::UIList::UIList(pugi::xml_node node) noexcept :Super(node) {
    // TODO: 删除预测
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

// UI列表控件: 析构函数
LongUI::UIList::~UIList() noexcept {

}

// 插入后
inline void LongUI::UIList::after_insert(UIControl* child) noexcept {
    UNREFERENCED_PARAMETER(child);

    assert(!(child->flags & Flag_Floating) 
        && "child of UIList cannot keep flag 'Flag_Floating'");
    assert(!(child->flags & Flag_HeightFixed) 
        && "child of UIList cannot keep flag 'Flag_WidthFixed'");
}

// 插入一个行模板
void LongUI::UIList::InsertInlineTemplate(Iterator itr) noexcept {
    auto ctrl = static_cast<UIListLine*>(UIListLine::CreateControl(
        Type_CreateControl, pugi::xml_node()));
    if (ctrl) {
        // 添加子控件
        for (const auto& data : m_bufLineTemplate) {
            ctrl->Insert(ctrl->end(), UIManager.CreateControl(data.id, data.func));
        }
        // 插入
        this->Insert(itr, ctrl);
    }
}

// 修改元素权重
void LongUI::UIList::ChangeElementWights(float weights[]) noexcept {
    for (auto cline : (*this)) {
        auto line = longui_cast<UIListLine*>(cline);
        auto index = 0u;
        for (auto ele : (*line)) {
            if (weights[index] >= 0.f) {
                force_cast(ele->weight) = weights[index];
            }
            ++index;
        }
    }
}

// 设置元素数量
void LongUI::UIList::SetElementCount(uint32_t length) noexcept {
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


// Render 渲染 
void LongUI::UIList::Render(RenderType t) const noexcept {
    return Super::Render(t);
}

// 更新子控件布局
void LongUI::UIList::Update() noexcept {
    // 前向刷新
    this->BeforeUpdate();
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    if (this->IsControlSizeChanged()) {
        // 第二次
        float index = 0.f;
        for (auto ctrl : (*this)) {
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
        auto tmp = this->get_referent_control();
        if (tmp) {
            m_2fContentSize.width = tmp->GetWidth();
        }
        m_2fContentSize.height = m_fLineHeight * this->GetCount();
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 父类刷新
    Super::Update();
}

// 清理UI列表控件
void LongUI::UIList::Cleanup() noexcept {
    delete this;
}


#ifdef LONGUI_UILIST_VECTOR
// 插入后
void LongUI::UIList::AfterInsert(UIControl* child) noexcept {
    Super::AfterInsert(child);
    this->after_insert(child);
    try {
        auto itr = m_controls.begin();
        if (!child->next) {
            auto itr = m_controls.end();
        }
        else if (child->prev) {
            itr = std::find(m_controls.begin(), m_controls.end(), child->prev);
        }
        m_controls.insert(
            std::find(m_controls.begin(), m_controls.end(), child->prev),
            child
            );
    }
    catch (...) {

    }
}

// 移除后
void LongUI::UIList::AfterRemove(UIControl* child) noexcept  {
    Super::AfterRemove(child);
    m_controls.erase(std::find(m_controls.begin(), m_controls.end(), child));
}

// 根据索引获取控件
auto LongUI::UIList::GetAt(uint32_t index) const noexcept -> UIControl* {
    assert(index < this->GetCount());
    if (index < this->GetCount()) {
        return static_cast<UIControl*>(m_controls.at(index));
    }
    return nullptr;
}
#else
// 插入后
void LongUI::UIList::AfterInsert(UIControl* child) noexcept  {
    Super::AfterInsert(child);
    this->after_insert(child);
}
#endif

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
    if (node) {

    }
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
            return true;
        default:
            break;
        }
    }
    return false;
}

// 清理UI列表头控件
void LongUI::UIListHeader::Cleanup() noexcept {
    delete this;
}

// 创建UI列表头
auto LongUI::UIListHeader::CreateControl(CreateEventType type, pugi::xml_node node) 
noexcept -> UIControl* {
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
