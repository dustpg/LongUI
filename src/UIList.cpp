#include "LongUI.h"
#include <algorithm>

// ----------------------------------------------------------------------------
// -------------------------------- UIList ------------------------------------
// ----------------------------------------------------------------------------
// UI列表控件: 构造函数
LongUI::UIList::UIList(UIContainer* cp, pugi::xml_node node) noexcept :Super(cp, node) {
    m_vLines.reserve(100);
    // OOM or BAD ACTION
    if(!m_vLines.isok()) {
        UIManager << DL_Warning << "OOM for less 1KB memory" << endl;
    }
    // INIT COLOR DATA
    m_colorLineHover = D2D1::ColorF(0xe5f3ff);
    // MAIN PROC
    auto listflag = this->list_flag;
    if (node) {
        const char* str = nullptr;
        // 行高度
        if ((str = node.attribute("lineheight").value())) {
            m_fLineHeight = LongUI::AtoF(str);
        }
        // 双击时间
        if ((str = node.attribute("dbclicktime").value())) {
            m_hlpDbClick.time = uint32_t(LongUI::AtoI(str));
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
        // 允许排序
        if (node.attribute("sort").as_bool(false)) {
            listflag |= this->Flag_SortableLineWithUserDataPtr;
        }
        // 普通背景颜色
        Helper::MakeColor(node.attribute("linebkcolor").value(), m_colorLineNormal1);
        // 普通背景颜色2
        Helper::MakeColor(node.attribute("linebkcolor2").value(), m_colorLineNormal2);
        // 悬浮颜色
        Helper::MakeColor(node.attribute("linebkcolorhover").value(), m_colorLineHover);
        // 选中颜色
        Helper::MakeColor(node.attribute("linebkcolorselected").value(), m_colorLineSelected);
    }
    this->list_flag = listflag;
}

// 添加事件监听器(雾)
bool LongUI::UIList::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    switch (sb)
    {
    case LongUI::SubEvent::Event_ItemClicked:
        m_callLineClicked += std::move(call);
        return true;
    case LongUI::SubEvent::Event_ItemDbClicked:
        m_callLineDBClicked += std::move(call);
        return true;
    case LongUI::SubEvent::Event_ContextMenu:
        break;
    case LongUI::SubEvent::Event_EditReturned:
        break;
    case LongUI::SubEvent::Event_ValueChanged:
        break;
    case LongUI::SubEvent::Event_Custom:
        break;
    default:
        break;
    }
    return Super::uniface_addevent(sb, std::move(call));
}


// 获取参考控件
auto LongUI::UIList::get_referent_control() const noexcept -> UIListLine* {
    if (m_pHeader) {
        return m_pHeader;
    }
    else {
        if (m_vLines.empty()) return nullptr;
        return m_vLines.front();
    }
}

// 依靠鼠标位置获取列表行
auto LongUI::UIList::find_line(const D2D1_POINT_2F& pt) const noexcept->UIListLine* {
    // XXX: 利用list特性优化
    for (auto ctrl : m_vLines) {
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            return ctrl;
        }
    }
    return nullptr;
}


// UIList: 重建
auto LongUI::UIList::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    for (auto ctrl : m_vLines) {
        hr = ctrl->Recreate();
        assert(SUCCEEDED(hr));
    }
    return Super::Recreate();
}

// 查找子控件
auto LongUI::UIList::FindChild(const D2D1_POINT_2F& pt) noexcept -> UIControl* {
    auto ctrl = Super::FindChild(pt);
    if (ctrl) return ctrl;
    return this->find_line(pt);
}

// push!
LongUINoinline void LongUI::UIList::PushBack(UIControl* child) noexcept {
    m_vLines.push_back(longui_cast<UIListLine*>(child));
    this->after_insert(child);
    ++m_cChildrenCount;
    this->childvector_changed();
    assert(m_vLines.isok());
}

// 插入
LongUINoinline auto LongUI::UIList::Insert(uint32_t index, UIControl* child) noexcept {
    m_vLines.insert(index, longui_cast<UIListLine*>(child));
    this->after_insert(child);
    ++m_cChildrenCount;
    this->childvector_changed();
    assert(m_vLines.isok());
}

// 排序
void LongUI::UIList::Sort(uint32_t index, UIControl* child) noexcept {
    // 修改
    m_pToBeSortedHeaderChild = child;
    // 有必要再说
    if ((this->list_flag & Flag_SortableLineWithUserDataPtr) 
        && m_vLines.size() > 1 
        && index < m_vLines.front()->GetCount()) {
        assert(child && "bad argument");
        assert(m_vLines.front()->flags & Flag_UIContainer);
        // 设置待排序控件
        for (auto ctrl : m_vLines) {
            ctrl->SetToBeSorted(index);
        }
        // 排序前
        m_callBeforSort(this);
        // 普通排序
        auto cmp_user_data = [](UIControl* a, UIControl* b) noexcept {
            assert(a && b && "bad arguments");
            auto ctrla = longui_cast<UIListLine*>(a)->GetToBeSorted();
            auto ctrlb = longui_cast<UIListLine*>(b)->GetToBeSorted();
            assert(ctrla && ctrlb && "bad action");
            return ctrla->user_data < ctrlb->user_data;
        };
        // 字符串排序
        auto cmp_user_ptr = [](UIControl* a, UIControl* b) noexcept {
            assert(a && b && "bad arguments");
            auto ctrla = longui_cast<UIListLine*>(a)->GetToBeSorted();
            auto ctrlb = longui_cast<UIListLine*>(b)->GetToBeSorted();
            assert(ctrla && ctrlb && "bad action");
            auto stra = static_cast<const wchar_t*>(ctrla->user_ptr);
            auto strb = static_cast<const wchar_t*>(ctrlb->user_ptr);
            assert(stra && strb && "bad action");
            return std::wcscmp(stra, strb) < 0;
        };
        // 普通排序
        bool(*cmp_alg)(UIControl*, UIControl*) = cmp_user_data;
        // 字符串排序?
        if (m_vLines.front()->GetToBeSorted()->user_ptr) {
            cmp_alg = cmp_user_ptr;
        }
        // 进行排序
        this->sort_line(cmp_alg);
        // 刷新
        m_pWindow->Invalidate(this);
    }
    m_pToBeSortedHeaderChild = nullptr;
}

// UI列表控件: 析构函数
LongUI::UIList::~UIList() noexcept {
    // 线性容器就是不用考虑next指针
    for (auto ctrl : m_vLines) {
        this->cleanup_child(ctrl);
    }
}

// [UNTESTED]移除
void LongUI::UIList::RemoveJust(UIControl* child) noexcept {
    auto itr = std::find(m_vLines.cbegin(), m_vLines.cend(), child);
    if (itr == m_vLines.cend()) {
        assert("control not found");
        return;
    }
    this->childvector_changed();
    m_vLines.erase(itr);
    --m_cChildrenCount;
    Super::RemoveJust(child);
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
    for (auto ctrl : m_vLines) {
        assert(ctrl && "bad");
        assert(index < ctrl->GetCount() && "out of range");
        if (index < ctrl->GetCount()) {
            auto ele = ctrl->GetAt(index);
            ele->SetWidth(width);
            ctrl->SetControlLayoutChanged();
        }
    }
    this->SetControlLayoutChanged();
}

// UI列表: 事件处理
bool LongUI::UIList::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 处理一下
            this->init_layout();
        default:
            break;
        }
    }
    return Super::DoEvent(arg);
}

// UI列表: 鼠标事件处理
bool LongUI::UIList::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    auto old_hover_line = m_pHoveredLine;
    // 分类
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_None:
        break;
    case LongUI::MouseEvent::Event_MouseWheelV:
        break;
    case LongUI::MouseEvent::Event_MouseWheelH:
        break;
    case LongUI::MouseEvent::Event_DragEnter:
        break;
    case LongUI::MouseEvent::Event_DragOver:
        break;
    case LongUI::MouseEvent::Event_DragLeave:
        break;
    case LongUI::MouseEvent::Event_Drop:
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        m_pHoveredLine = nullptr;
        break;
    case LongUI::MouseEvent::Event_MouseHover:
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        m_pHoveredLine = this->find_line(arg.pt);
        break;
    case LongUI::MouseEvent::Event_LButtonDown:
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        break;
    case LongUI::MouseEvent::Event_RButtonDown:
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        break;
    case LongUI::MouseEvent::Event_MButtonDown:
        break;
    case LongUI::MouseEvent::Event_MButtonUp:
        break;
    default:
        break;
    }
    // 不同就渲染
    if (old_hover_line != m_pHoveredLine) {
        m_pWindow->Invalidate(this);
    }
    return Super::DoMouseEvent(arg);
}

// 排序算法
void LongUI::UIList::sort_line(bool(*cmp)(UIControl* a, UIControl* b) ) noexcept {
    // 无需排列
    if (this->GetCount() <= 1) return;
#ifdef _DEBUG
    // cmp 会比较复杂, 模板带来的性能提升还不如用函数指针来节约代码大小
    auto timest = ::timeGetTime();
#endif
    const auto bn = &*m_vLines.begin();
    const auto ed = &*m_vLines.end();
    bool just_reverse = true;
    // 检查逆序状态
    for (auto itr = bn; itr < (ed -1); ++itr) {
        if (!cmp(*itr, *(itr + 1))) {
            just_reverse = false;
            break;
        }
    }
    // 直接逆序
    if (just_reverse) {
        std::reverse(bn, ed);
    }
    // 排序
    else {
        // 快速排序
        if (this->GetCount() >= m_cFastSortThreshold) {
            std::sort(bn, ed, cmp);
        }
        // 冒泡排序
        else {
            LongUI::BubbleSort(bn, ed, cmp);
        }
    }
#ifdef _DEBUG
    timest = ::timeGetTime() - timest;
    if (timest) {
        int bk = 9;
        UIManager << DL_Hint
            << "sort take time: "
            << long(timest)
            << " ms"
            << LongUI::endl;
        bk = 0;
    }
    if (this->debug_this) {
        UIManager << DL_Log
            << "sort take time: "
            << long(timest)
            << " ms"
            << LongUI::endl;
    }
#endif
    this->SetControlLayoutChanged();
}


// 选择子控件(对外)
void LongUI::UIList::SelectChild(uint32_t index, bool new_select) noexcept {
    this->select_child(index, new_select);
    m_pWindow->Invalidate(this);
}


// 选择子控件
LongUINoinline void LongUI::UIList::select_child(uint32_t index, bool new_select) noexcept {
    // 检查是否多选
    if (!new_select && !(this->list_flag & this->Flag_MultiSelection)) {
        UIManager << DL_Hint
            << "cannot do multi-selection"
            << LongUI::endl;
        new_select = true;
    }
    // 新的
    if (new_select) {
        m_vSelected.clear();
    }
    // 选择
    auto line = m_vLines[index];
    if (line->IsSelected()) {
        line->SetSelected(false);
        auto itr = std::find(m_vSelected.cbegin(), m_vSelected.cend(), line);
        if (itr == m_vSelected.cend()) {
            assert(!"NOT FOUND");
        }
        else {
            m_vSelected.erase(itr);
        }
    }
    else {
        line->SetSelected(true);
        m_vSelected.push_back(line);
    }
}

// UIList: 子控件向量修改了
void LongUI::UIList::childvector_changed() noexcept {
    for(auto line : m_vSelected) {
        line->SetSelected(false);
    }
    m_vSelected.clear();
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
    // 分情况
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        // 渲染背景
        this->render_background();
        break;
    case LongUI::RenderType::Type_Render:
        // 渲染背景
        this->render_background();
        // 渲染帮助
        Super::RenderHelper(m_vLines.begin(), m_vLines.end());
        // 父类渲染
        Super::Render(LongUI::RenderType::Type_Render);
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UIList 背景渲染
LongUINoinline void LongUI::UIList::render_background() const noexcept {
    // 父类背景+-?
    Super::Render(LongUI::RenderType::Type_RenderBackground);
    // 独立背景- - 可视优化
    if (!this->GetCount()) return;
    // 保留转变
    D2D1_MATRIX_3X2_F matrix;
    UIManager_RenderTarget->GetTransform(&matrix);
    UIManager_RenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    // 鼠标指向
    if (m_pHoveredLine) {
        m_pBrush_SetBeforeUse->SetColor(m_colorLineHover);
        UIManager_RenderTarget->FillRectangle(
            &m_pHoveredLine->visible_rect, m_pBrush_SetBeforeUse
            );
    }
    // 第一个可视列表行 = (-Y偏移) / 行高
    int first_visible = static_cast<int>((-m_2fOffset.y) / m_fLineHeight);
    first_visible = std::max(first_visible, int(0));
    // 最后一个可视列表行 = 第一个可视列表行 + 1 + 可视区域高度 / 行高
    int last_visible = static_cast<int>(this->view_size.height / m_fLineHeight);
    last_visible = last_visible + first_visible + 1;
    last_visible = std::min(last_visible, int(this->GetCount()));
    // 循环
    const auto first_itr = m_vLines.data() + first_visible;
    const auto last_itr = m_vLines.data() + last_visible;
    for (auto itr = first_itr; itr <= last_itr; ++itr) {
        auto line = *itr;
    }
    // 还原
    UIManager_RenderTarget->SetTransform(&matrix);
}

// UIList: 刷新
void LongUI::UIList::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(m_vLines.begin(), m_vLines.end());
#ifdef _DEBUG
    // 必须一致
    if (this->IsNeedRefreshWorld() && m_pHeader && m_vLines.size() && m_vLines.front()) {
        assert(m_pHeader->GetCount() == m_vLines.front()->GetCount() && "out of sync for child number");
    }
#endif
}


// 更新子控件布局
void LongUI::UIList::RefreshLayout() noexcept {
    if (m_vLines.empty()) return;
    // 第二次
    float index = 0.f;
    float widthtt = m_vLines.front()->GetContentWidthZoomed();
    if (widthtt == 0.f) widthtt = this->GetViewWidthZoomed();
    for (auto ctrl : m_vLines) {
        // 设置控件高度
        ctrl->SetWidth(widthtt);
        ctrl->SetHeight(m_fLineHeight);
        // 不管如何, 修改!
        ctrl->SetControlLayoutChanged();
        ctrl->SetLeft(0.f);
        ctrl->SetTop(m_fLineHeight * index);
        ++index;
    }
    // 设置
    m_2fContentSize.width = widthtt;
    m_2fContentSize.height = m_fLineHeight * this->GetCount();
}

// 清理UI列表控件
void LongUI::UIList::cleanup() noexcept {
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
void LongUI::UIListLine::cleanup() noexcept {
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
    assert(cp && "bad argument");
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
    // -------------------------- set sort data
    auto set_sort_data = [this, &arg]() noexcept {
        // 遍历子控件
        for (auto ctrl : (*this)) {
            // 悬浮在鼠标处
            if (IsPointInRect(ctrl->visible_rect, arg.pt)) {
                // 没有设置
                if (!ctrl->TestParentState()) {
                    // 设置点击事件
                    ctrl->AddEventCall([this](UIControl* child) noexcept {
                        longui_cast<UIList*>(this->parent)->Sort(this->GetIndexOf(child), child);
                        return true;
                    }, SubEvent::Event_ItemClicked);
                    // 设置了
                    ctrl->SetParentState(true);
                }
                break;
            }
        }
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
                    this->SetControlLayoutChanged();
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
        if (!hover_sep) hover_sep = get_sep_hovered_control();
        // XXX: 逻辑
        bool handled = !!hover_sep;
        // 分类处理
        switch (arg.event)
        {
        case LongUI::MouseEvent::Event_MouseLeave:
            m_pWindow->ResetCursor();
            handled = false;
            m_indexSepHovered = 0;
            break;
        case LongUI::MouseEvent::Event_MouseMove:
            on_mouse_move(hover_sep);
            if (hover_sep) {
            }
            break;
        case LongUI::MouseEvent::Event_LButtonDown:
            // 边界拖拽
            if (hover_sep) {
                m_pSepHovered = hover_sep;
                m_pWindow->SetCapture(this);
                m_fLastMousePosX = arg.pt.x;
            }
            else {
                // 设置排序
                if ((static_cast<UIList*>(this->parent)->list_flag
                    & UIList::Flag_SortableLineWithUserDataPtr)) {
                    set_sort_data();
                }
            }
            break;
        case LongUI::MouseEvent::Event_LButtonUp:
            // 边界拖拽
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
void LongUI::UIListHeader::cleanup() noexcept {
    delete this;
}

// 初始化边界控件
void LongUI::UIListHeader::InitMarginalControl(MarginalControl _type) noexcept {
    // 初始化
    Super::InitMarginalControl(_type);
    // 父类是控件
    auto list = longui_cast<UIList*>(this->parent);
    // 设置列表头
    list->SetHeader(this);
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
            UIManager << DL_Log << L"node null" << LongUI::endl;
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
