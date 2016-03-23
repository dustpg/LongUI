#include "LongUI.h"
#include <algorithm>

// ----------------------------------------------------------------------------
// -------------------------------- UIList ------------------------------------
// ----------------------------------------------------------------------------
// UI列表控件: 初始化
void LongUI::UIList::initialize(pugi::xml_node node) noexcept {
    // 必须有效
    assert(node && "call UIListHeader::initialize() if no xml-node");
    // 链式调用
    Super::initialize(node);
    // 初始
    m_vLines.reserve(50);
    m_vSelectedIndex.reserve(16);
    m_vLineTemplate.reserve(16);
    // OOM or BAD ACTION
    if (!m_vLines.isok() && !m_vLineTemplate.isok()) {
        UIManager << DL_Warning << "OOM for less 1KB memory" << LongUI::endl;
    }
    // MAIN PROC
    auto listflag = this->list_flag | Flag_MultiSelect;
    {
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
            auto len = Helper::MakeCC(str);
            m_vLineTemplate.newsize(len);
            // 有效
            if (len && m_vLineTemplate.isok()) {
                Helper::MakeCC(str, m_vLineTemplate.data());
            }
            // 没有则给予警告
            else {
                UIManager << DL_Warning
                    << L"BAD TEMPLATE: {"
                    << str << L"} or OOM"
                    << LongUI::endl;
            }
        }
        // 给予提示
        else {
            UIManager << DL_Hint
                << L"recommended to set 'linetemplate'. Now, set 'Text, 0' as template"
                << LongUI::endl;
        }
        // 允许排序
        if (node.attribute("sort").as_bool(false)) {
            listflag |= this->Flag_SortableLineWithUserDataPtr;
        }
        // 普通背景颜色
        Helper::MakeColor(node.attribute("linebkcolor").value(), m_colorLineNormal1);
        // 普通背景颜色2 - step 1
        m_colorLineNormal2 = m_colorLineNormal1;
        // 普通背景颜色2 - step 2
        Helper::MakeColor(node.attribute("linebkcolor2").value(), m_colorLineNormal2);
        // 悬浮颜色
        Helper::MakeColor(node.attribute("linebkcolorhover").value(), m_colorLineHover);
        // 选中颜色
        Helper::MakeColor(node.attribute("linebkcolorselected").value(), m_colorLineSelected);
    }
    // 修改
    this->list_flag = listflag;

    // TEST: INIT COLOR DATA
    m_colorLineNormal1 = D2D1::ColorF(0xffffffui32, 0.5f);
    m_colorLineNormal2 = D2D1::ColorF(0xeeeeeeui32, 0.5f);
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

// 依靠鼠标位置获取列表行索引
auto LongUI::UIList::find_line_index(const D2D1_POINT_2F& pt) const noexcept ->uint32_t {
    uint32_t index = 0;
    // XXX: 利用list特性优化
    for (auto ctrl : m_vLines) {
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            break;
        }
        ++index;
    }
    return index;
}

// 依靠鼠标位置获取列表行
auto LongUI::UIList::find_line(const D2D1_POINT_2F& pt) const noexcept ->UIListLine* {
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
void LongUI::UIList::Push(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::Push(child);
    }
    // 一般的就自己处理
    else {
        return this->Insert(m_cChildrenCount, longui_cast<UIListLine*>(child));
    }
}

// 插入
LongUINoinline void LongUI::UIList::Insert(uint32_t index, UIListLine* child) noexcept {
    assert(child && "bad argument");
    if (child) {
        // 对齐操作
        auto line = this->get_referent_control();
        if (line) {
            auto itr1 = child->begin();
            for (auto itr2 = line->begin(); itr2 != line->end(); ++itr1, ++itr2) {
                auto ctrl_new = *itr1, ctrl_ref = *itr2;
                force_cast(ctrl_new->flags) = ctrl_ref->flags;
                force_cast(ctrl_new->weight) = ctrl_ref->weight;
                ctrl_new->SetWidth(ctrl_ref->GetWidth());
            }
        }
        m_vLines.insert(index, child);
        this->after_insert(child);
        ++m_cChildrenCount;
        this->reset_select();
        assert(m_vLines.isok());
    }
}

// 排序
void LongUI::UIList::Sort(uint32_t index, UIControl* child) noexcept {
    // 修改
    m_pToBeSortedHeaderChild = child;
    // 有必要再说
    if ((this->list_flag & Flag_SortableLineWithUserDataPtr) 
        && m_vLines.size() > 1 
        && index < m_vLines.front()->GetChildrenCount()) {
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
        this->InvalidateThis();
    }
    m_pToBeSortedHeaderChild = nullptr;
}


/// <summary>
/// Before_deleteds this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIList::before_deleted() noexcept {
    // 清理子控件
    for (auto ctrl : m_vLines) {
        this->release_child(ctrl);
    }
    m_vLines.clear();
    // 链式清理
    Super::before_deleted();
}

// UI列表控件: 析构函数
LongUI::UIList::~UIList() noexcept {

}

// [UNTESTED]移除
void LongUI::UIList::Remove(UIControl* child) noexcept {
    auto itr = std::find(m_vLines.cbegin(), m_vLines.cend(), child);
    if (itr == m_vLines.cend()) {
        assert("control not found");
        return;
    }
    this->reset_select();
    m_vLines.erase(itr);
    --m_cChildrenCount;
    Super::Remove(child);
}

// 对列表插入一个行模板至指定位置
auto LongUI::UIList::InsertLineTemplateToList(uint32_t index) noexcept ->UIListLine* {
    // 创建列表行
    auto ctrl = UIListLine::CreateControl(this);
    if (!ctrl) return ctrl;
    // 添加子控件
    for (const auto& data : m_vLineTemplate) {
        ctrl->Insert(ctrl->end(), UIManager.CreateControl(ctrl, data.id, data.func));
    }
    // 插入
    this->Insert(index, ctrl);
    return ctrl;
}

// [UNTESTED] 利用索引移除行模板中一个元素
void LongUI::UIList::RemoveLineElementInEachLine(uint32_t index) noexcept {
    assert(index < m_vLineTemplate.size() && "out of range");
    if (index < m_vLineTemplate.size()) {
        // 刷新
        this->InvalidateThis();
        // 交换列表
        for (auto line : m_vLines) {
            auto child = line->GetAt(index);
#ifdef _DEBUG
            {
                UIManager << DL_Hint
                    << L"bad idea for line hosting flag Flag_NoCleanupViaParent"
                    << LongUI::endl;
            }
#endif
            line->Remove(child);
        }
        // 模板
        m_vLineTemplate.erase(index);
    }
}

// [UNTESTED] 交换行模板中元素
void LongUI::UIList::SwapLineElementsInEachLine(uint32_t index1, uint32_t index2) noexcept {
    assert(index1 < m_vLineTemplate.size() && index2 < m_vLineTemplate.size() && "out of range");
    assert(index1 != index2 && "bad arguments");
    if (!(index1 < m_vLineTemplate.size() && index2 < m_vLineTemplate.size())) return;
    if (index1 == index2) return;
    // 刷新
    this->InvalidateThis();
    // 交换列表
    for (auto line : m_vLines) {
        auto child1 = line->GetAt(index1);
        auto child2 = line->GetAt(index2);
        line->SwapChild(child1, child2);
    }
    // 交换模板
    std::swap(m_vLineTemplate[index1], m_vLineTemplate[index2]);
}

// [UNTESTED]插入一个新的行元素
void LongUI::UIList::InsertNewElementToEachLine(uint32_t index, CreateControlEvent func, size_t tid) noexcept {
    assert(index <= m_vLineTemplate.size() && "out of range");
    assert(func && "bad argument");
    // 有效
    if (index <= m_vLineTemplate.size() && func) {
        // 刷新
        this->InvalidateThis();
        // 交换列表
        for (auto line : m_vLines) {
            auto ctrl = UIManager.CreateControl(line, tid, func);
            if (ctrl) {
                auto itr = MakeIteratorBI(line->GetAt(index));
                line->Insert(itr, ctrl);
            }
            else {
                UIManager << DL_Error
                    << "CreateControl failed. OOM or BAD ACTION"
                    << LongUI::endl;
            }
        }
        // 插入模板
        Helper::CC cc = { func, tid };
        m_vLineTemplate.insert(index, cc);
    }
}

// 设置
void LongUI::UIList::SetCCElementInLineTemplate(uint32_t index, CreateControlEvent func, size_t tid ) noexcept {
    assert(index < m_vLineTemplate.size() && "out of range");
    assert(func && "bad argument");
    if (index < m_vLineTemplate.size() && func) {
        m_vLineTemplate[index].func = func;
        m_vLineTemplate[index].id = tid;
    }
}


// 设置元素宽度
void LongUI::UIList::SetElementWidth(uint32_t index, float width) noexcept {
    // 循环
    for (auto ctrl : m_vLines) {
        assert(ctrl && "bad");
        assert(index < ctrl->GetChildrenCount() && "out of range");
        if (index < ctrl->GetChildrenCount()) {
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
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBulidingFinished:
        // 由父类创建边缘控件
        Super::DoEvent(arg);
        this->init_layout();
        return true;
    default:
        break;
    }
    return Super::DoEvent(arg);
}

// UI列表: 鼠标事件处理
bool LongUI::UIList::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // -------------------  L-Button Down  ---------------
    auto lbutton_down = [this, &arg]() noexcept {
        auto index = this->find_line_index(arg.pt);
        // SHIFT优先
        if (UIInput.IsKeyPressed(VK_SHIFT)) {
            this->SelectTo(m_ixLastClickedLine, index);
            return;
        }
        // 修改
        m_ixLastClickedLine = index;
        // UNCTRLed
        bool unctrled = !(UIInput.IsKeyPressed(VK_CONTROL));
        // 双击?
        if (m_hlpDbClick.Click(arg.pt)) {
            UIManager << DL_Log << "DB Clicked" << LongUI::endl;
            this->CallUiEvent(m_callLineDBClicked, SubEvent::Event_ItemDbClicked);
        }
        // 单击?
        else {
            this->SelectChild(m_ixLastClickedLine, unctrled);
            this->CallUiEvent(m_callLineClicked, SubEvent::Event_ItemClicked);
        }
    };
    // ---------------------------------------------------
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
        lbutton_down();
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
#ifdef _DEBUG
        // 调试输出
        if (this->debug_this) {
            UIManager << DL_Hint
                << L"OLD: " << old_hover_line
                << L"NEW: " << m_pHoveredLine
                << LongUI::endl;
        }
#endif
        this->InvalidateThis();
    }
    return Super::DoMouseEvent(arg);
}

// 排序算法
void LongUI::UIList::sort_line(bool(*cmp)(UIControl* a, UIControl* b) ) noexcept {
    // 无需排列
    if (this->GetChildrenCount() <= 1) return;
#ifdef _DEBUG
    // cmp 会比较复杂, 模板带来的性能提升还不如用函数指针来节约代码大小
    auto timest = ::timeGetTime();
#endif
    const auto bn = &*m_vLines.begin();
    const auto ed = &*m_vLines.end();
    bool just_reverse = true;
    // 检查逆序状态
    for (auto itr = bn; itr < (ed -1); ++itr) {
        if (cmp(*(itr + 1), *itr)) {
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
        if (this->GetChildrenCount() >= m_cFastSortThreshold) {
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
    // 修改了
    this->reset_select();
    this->SetControlLayoutChanged();
}


// 选择子控件(对外)
void LongUI::UIList::SelectChild(uint32_t index, bool new_select) noexcept {
    if (index < m_cChildrenCount) {
        this->select_child(index, new_select);
        this->InvalidateThis();
    }
}

// 选择子控件到(对外)
void LongUI::UIList::SelectTo(uint32_t index1, uint32_t index2) noexcept {
    // 交换
    if (index1 > index2) std::swap(index1, index2);
    // 限制
    index2 = std::min(index2, m_cChildrenCount - 1);
    // 有效
    if (index1 < index2) {
        this->select_to(index1, index2);
        this->InvalidateThis();
    }
}

// 选择子控件
LongUINoinline void LongUI::UIList::select_child(uint32_t index, bool new_select) noexcept {
    assert(index < m_cChildrenCount && "out of range for selection");
    // 检查是否多选
    if (!new_select && !(this->list_flag & this->Flag_MultiSelect)) {
        UIManager << DL_Hint
            << "cannot do multi-selection"
            << LongUI::endl;
        new_select = true;
    }
    // 新的重置
    if (new_select) {
        this->reset_select();
    }
    // 选择
    auto line = m_vLines[index];
    if (line->IsSelected()) {
        line->SetSelected(false);
        // 移除
        auto itr = std::find(m_vSelectedIndex.cbegin(), m_vSelectedIndex.cend(), index);
        if (itr == m_vSelectedIndex.cend()) {
            assert(!"NOT FOUND");
        }
        else {
            m_vSelectedIndex.erase(itr);
        }
    }
    else {
        line->SetSelected(true);
        m_vSelectedIndex.push_back(index);
    }
}

// 选择子控件到
LongUINoinline void LongUI::UIList::select_to(uint32_t index1, uint32_t index2) noexcept {
    assert(index1 < m_cChildrenCount && index2 < m_cChildrenCount && "out of range for selection");
    // 检查是否多选
    if (!(this->list_flag & this->Flag_MultiSelect)) {
        UIManager << DL_Hint
            << "cannot do multi-selection"
            << LongUI::endl;
        index1 = index2;
    }
    // 交换
    if (index1 > index2) std::swap(index1, index2);
    // 选择
    auto itr_1st = m_vLines.data() + index1;
    auto itr_lst = m_vLines.data() + index2;
    auto i = index1;
    for (auto itr = itr_1st; itr <= itr_lst; ++itr) {
        auto line = *itr;
        line->SetSelected(true);
        m_vSelectedIndex.push_back(i);
        ++i;
    }
}

/// <summary>
/// Zeroes the width of all lines content.
/// </summary>
/// <returns></returns>
void LongUI::UIList::ZeroAllLinesContentWidth() noexcept {
    // 清除宽度信息
    for (auto line : (*this)) line->ZeroContentWidth();
}

// UIList: 重置选择
void LongUI::UIList::reset_select() noexcept {
    //m_pHoveredLine = nullptr;
    //m_ixLastClickedLine = uint32_t(-1);
    for (auto i : m_vSelectedIndex) {
        m_vLines[i]->SetSelected(false);
    }
    m_vSelectedIndex.clear();
}

// UIList: 初始化布局
void LongUI::UIList::init_layout() noexcept {
    uint32_t element_count_init = 1;
    auto rctrl = this->get_referent_control();
    if (rctrl) {
        // 检查不和谐的地方
#ifdef _DEBUG
        if (rctrl->GetChildrenCount() != m_vLineTemplate.size()) {
            if (m_vLineTemplate.size()) {
                UIManager << DL_Warning
                    << L"inconsistent line-element count: SET "
                    << long(m_vLineTemplate.size())
                    << L", BUT "
                    << long(rctrl->GetChildrenCount())
                    << LongUI::endl;
            }
        }
#endif
        element_count_init = rctrl->GetChildrenCount();
    }
    // 没有就给予警告
    else {
        UIManager << DL_Warning
            << L"NO CHILD FOUND. line-element set to 1"
            << LongUI::endl;
    }
    this->set_element_count(element_count_init);
}

// 设置元素数量
void LongUI::UIList::set_element_count(uint32_t length) noexcept {
    auto old = m_vLineTemplate.size();
    m_vLineTemplate.newsize(length);
    // 变长了
    if (old < m_vLineTemplate.size()) {
        for (auto i = old; i < m_vLineTemplate.size(); ++i) {
            m_vLineTemplate[i].id = 0;
            m_vLineTemplate[i].func = UIText::CreateControl;
        }
    }
}

// UIList: 前景渲染
void LongUI::UIList::render_chain_background() const noexcept {
    // 独立背景- - 可视优化
    if (this->GetChildrenCount()) {
        // 保留转变
        D2D1_MATRIX_3X2_F matrix;
        UIManager_RenderTarget->GetTransform(&matrix);
        UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
        // 第一个可视列表行 = (-Y偏移) / 行高
        int first_visible = static_cast<int>((-m_2fOffset.y) / m_fLineHeight);
        first_visible = std::max(first_visible, int(0));
        // 最后一个可视列表行 = 第一个可视列表行 + 1 + 可视区域高度 / 行高
        int last_visible = static_cast<int>(this->view_size.height / m_fLineHeight);
        last_visible = last_visible + first_visible + 1;
        last_visible = std::min(last_visible, int(this->GetChildrenCount()));
        // 背景索引
        int bkindex1 = !(first_visible & 1);
        // 循环
        const auto first_itr = m_vLines.data() + first_visible;
        const auto last_itr = m_vLines.data() + last_visible;
        for (auto itr = first_itr; itr < last_itr; ++itr) {
            auto line = *itr;
            // REMOVE THIS LINE?
            const D2D1_COLOR_F* color;
            // 选择色优先
            if (line->IsSelected()) {
                color = &m_colorLineSelected;
            }
            // 悬浮色其次
            else if (line == m_pHoveredLine) {
                color = &m_colorLineHover;
            }
            // 背景色最后
            else {
                color = &m_colorLineNormal1 + bkindex1;
            }
            // 设置
            if (color->a > 0.f) {
                m_pBrush_SetBeforeUse->SetColor(color);
                UIManager_RenderTarget->FillRectangle(
                    &line->visible_rect, m_pBrush_SetBeforeUse
                    );
            }
            bkindex1 = !bkindex1;
        }
        // 还原
        UIManager_RenderTarget->SetTransform(&matrix);
    }
    // 父类主景
    Super::render_chain_background();
}

// UIList: 主景渲染
void LongUI::UIList::render_chain_main() const noexcept {
    // 渲染帮助器
    Super::RenderHelper(this->begin(), this->end());
    // 父类主景
    Super::render_chain_main();
}

// UIList: 渲染函数
void LongUI::UIList::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UIList: 刷新
void LongUI::UIList::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(m_vLines.begin(), m_vLines.end());
#ifdef _DEBUG
    // 必须一致
    if (this->IsNeedRefreshWorld() && m_pHeader && m_vLines.size() && m_vLines.front()) {
        auto code = m_pHeader->GetChildrenCount() == m_vLines.front()->GetChildrenCount();
        assert(code && "out of sync for child number");
    }
#endif
    //this->world;
    //this->RefreshWorld();
}


// 更新子控件布局
void LongUI::UIList::RefreshLayout() noexcept {
    if (m_vLines.empty()) return;
    // 第二次
    float index = 0.f;
    float widthtt = m_vLines.front()->GetContentWidthZoomed();
#ifdef _DEBUG
    if (this->debug_this && widthtt > 0.f) {
        UIManager << DL_Hint
            << L"width exsitd: "
            << widthtt
            << endl;
    }
#endif
    if (widthtt == 0.f) widthtt = this->GetViewWidthZoomed();
    for (auto ctrl : m_vLines) {
        // 设置控件高度
        ctrl->SetWidth(widthtt);
        ctrl->SetHeight(m_fLineHeight);
        // 不管如何, 修改!
        ctrl->SetControlLayoutChanged();
        ctrl->SetLeft(0.f);
        ctrl->SetTop(m_fLineHeight * index);
        ctrl->visible_rect;
        ctrl->world;
        ++index;
    }
    // 设置
    m_2fContentSize.width = widthtt;
    m_2fContentSize.height = m_fLineHeight * this->GetChildrenCount();
}

// 清理UI列表控件
void LongUI::UIList::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除
    delete this;
}

// UI列表控件: 创建控件
auto LongUI::UIList::CreateControl(CreateEventType type, pugi::xml_node node)
noexcept -> UIControl* {
    UIList* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIList, pControl, type, node);
#ifdef _DEBUG
        if (pControl && pControl->debug_this) {
            UIManager << DL_Log << pControl << LongUI::endl;
        }
#endif
    }
    return pControl;
}

// ----------------------------------------------------------------------------
// ---------------------------- UIListLine! --------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
inline void LongUI::UIListLine::initialize() noexcept {
    // 链式调用
    Super::initialize();
}

// UI列表元素控件: 初始化
void LongUI::UIListLine::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // listline 特性: 宽度必须固定
    //auto flag = this->flags | Flag_WidthFixed;
    {

    }
    //force_cast(this->flags) = flag;
}

// 刷新UI列表元素控件
void LongUI::UIListLine::Update() noexcept {
    // 检查宽度
    /*if (m_bFirstUpdate) {
        m_bFirstUpdate = false;*/
    /*{
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
    }*/
    return Super::Update();
}

// 清理UI列表元素控件
void LongUI::UIListLine::cleanup() noexcept {
    // 删除前调用
    this->before_deleted();
    // 删除
    delete this;
}


/// <summary>
/// Creates the control.
/// </summary>
/// <param name="cp">The cp.</param>
/// <returns></returns>
auto LongUI::UIListLine::CreateControl(UIContainer* cp) noexcept ->UIListLine* {
    // 创建
    UIListLine* line = new(std::nothrow) UIListLine(cp);
    // 初始化
    if (line) line->initialize();
    // 返回创建指针
    return line;
}

// UI列表元素控件: 创建控件
auto LongUI::UIListLine::CreateControl(CreateEventType type, pugi::xml_node node) noexcept -> UIControl* {
    UIListLine* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIListLine, pControl, type, node);
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// ----------------------------- UIListHeader ---------------------------------
// ----------------------------------------------------------------------------

// UI列表头控件: 构造函数
void LongUI::UIListHeader::initialize(pugi::xml_node node) noexcept {
    // 必须有效
    assert(node && "call UIListHeader::initialize() if no xml-node");
    // 链式调用
    Super::initialize(node);
    // 本类必须为边界控件
    assert((this->flags & Flag_MarginalControl) && "'UIListHeader' must be marginal-control");
    // 设置表头
    longui_cast<UIList*>(this->parent)->SetHeader(this);
    // 支持模板子结点
    //auto flag = this->flags;
    {
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
    //force_cast(this->flags) = flag;
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
            if (ctrl->GetVisible() && ctrl->visible_rect.right > ctrl->visible_rect.left
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
            // 设置光标
            m_pWindow->SetCursor(Cursor::Cursor_SizeWE);
            // 拖拽刷新
            if (m_pSepHovered && UIInput.IsKeyPressed(VK_LBUTTON)) {
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
    // 删除前调用
    this->before_deleted();
    // 删除
    delete this;
}


// 刷新UI列表头控件边界宽度
void LongUI::UIListHeader::UpdateMarginalWidth() noexcept {
    this->marginal_width = m_fLineHeight + this->margin_rect.top + this->margin_rect.bottom;
}

// 创建UI列表头
auto LongUI::UIListHeader::CreateControl(CreateEventType type, pugi::xml_node node) noexcept -> UIControl* {
    UIListHeader* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIListHeader, pControl, type, node);
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
bool LongUI::CUIMenu::Create(const char* xml) noexcept {
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
    // 创建结点
    return this->Create(document.first_child());
}

// 使用XML结点创建菜单
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
