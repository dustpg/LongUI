// ui
#include <core/ui_control_control.h>
#include <style/ui_native_style.h>
#include <container/pod_vector.h>
#include <control/ui_control.h>
#include <core/ui_unsafe.h>
#include <util/ui_time_meter.h>
#include <util/ui_aniamtion.h>
#include <core/ui_window.h>
#include <core/ui_string.h>
#include <core/ui_manager.h>
#include <util/ui_sort.h>
// debug
#include <debugger/ui_debug.h>

// c++
#include <cassert>
#include <algorithm>


#ifndef NDEBUG
static bool sg_bDebugFlag = false;
#endif // !NDEBUG


//#define LUI_SORT_UPDATE_LIST


namespace LongUI {
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // delete style sheet
    void DeleteStyleSheet(CUIStyleSheet* ptr) noexcept;
    // make style sheet
    auto MakeStyleSheet(U8View view, SSPtr old) noexcept->SSPtr;
    // make style sheet
    auto MakeStyleSheetFromFile(U8View view, SSPtr old) noexcept->SSPtr;
    // get data from block
    auto RefValuesFromBlock(uintptr_t id) noexcept ->const SSValues&;
    // get data from block
    auto RefControlsFromBlock(uintptr_t id) noexcept ->const UIControls&;
#endif
}

/// <summary>
/// private data/func for controlcontrol
/// </summary>
struct LongUI::CUIControlControl::Private {
    // control animation
    using BasicAnima = ControlAnimationBasic;
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // control animation
    using ExtraAnima = ControlAnimationExtra;
#endif
    // ctor
    inline Private() noexcept;
    // ctor
    inline ~Private() noexcept { }
    // 初始化表A Data锁住后总为null
    ControlNode             init_list = { nullptr };
    // 初始化表B 通过两个表降低锁住的时间
    ControlNode             init_list_lock_free = { nullptr };
    // relayout list
    ControlNode             update_list = { nullptr };
    // delete list
    ControlNode             delete_list = { nullptr };
    // max depth of tree
    uint16_t                update_max_depth = 0;
    // min depth of tree
    uint16_t                update_min_depth = 0xffff;
    // count of list
    uint32_t                update_count = 0;
    // style cache
    POD::Vector<UIControl*> style_cache;
    // animation list
    POD::Vector<BasicAnima> basic_anima;
    // dirty update
    //POD::Vector<DirtyUpdate>dirty_update;
#ifdef LUI_DISABLE_STYLE_SUPPORT
    // unused1
    POD::Vector<UIControl*> unused1;
    // unused2
    POD::Vector<UIControl*> unused2;
#else
    // animation list
    POD::Vector<ExtraAnima> extra_anima;
    // from to list
    POD::Vector<SSFromTo>   from_to;
#endif
    // xul dir
    CUIStringU8             xul_dir;


    // add into update list
    void add_into_update_list(UIControl& ctrl) noexcept {
#ifdef LUI_SORT_UPDATE_LIST
        ++update_count;
        const uint16_t depth = ctrl.GetLevel();
        update_max_depth = std::max(update_max_depth, depth);
        update_min_depth = std::min(update_min_depth, depth);
#endif
        const size_t offset = offsetof(UIControl, m_oManager.next_delinitupd);
        CUIControlControl::AddControlToList(ctrl, luiref update_list, offset);
    }
    // remove from init list
    void remove_from_init_list(UIControl& ctrl) noexcept {
        const size_t offset = offsetof(UIControl, m_oManager.next_delinitupd);
        assert(init_list.first == nullptr);
        assert(init_list.last == nullptr);
        CUIControlControl::RemoveControlInList(ctrl, luiref init_list_lock_free, offset);
        ctrl.m_oManager.next_delinitupd = nullptr;
    }
    // sort the list
    auto sort_update() noexcept -> UIControl*;
};

/// <summary>
/// move list-b to list-a
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::swap_init_list() noexcept {
    // m_uiLaterLocker 上锁目的不是仅仅安全写入init_list
    // 而是阻断其他线程进入构造函数
#ifdef LUI_USING_CTOR_LOCKER
    auto& obj = cc();
    assert(obj.init_list.first == nullptr);
    assert(obj.init_list.last == nullptr);
    std::swap(obj.init_list_lock_free, obj.init_list);
#endif
}

/// <summary>
/// Sets the xul dir.
/// </summary>
/// <param name="dir">The dir.</param>
/// <returns></returns>
void LongUI::CUIControlControl::SetXulDir(U8View dir) noexcept {
    auto& string = cc().xul_dir;
    string = dir;
    // 尝试添加一个/
    if (const auto len = string.length()) {
        auto ch = string[len - 1];
        if (!(ch == '/' || ch == '\\')) {
            string.append('/');
        }
    }
}

/// <summary>
/// Gets the xul dir.
/// </summary>
/// <returns></returns>
auto LongUI::CUIControlControl::GetXulDir() const noexcept -> U8View {
    return cc().xul_dir.view();
}

#ifndef LUI_DISABLE_STYLE_SUPPORT
/// <summary>
/// Adds the Global CSS string.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void LongUI::CUIControlControl::AddGlobalCssString(U8View view) noexcept {
    m_pStyleSheet = LongUI::MakeStyleSheet(view, m_pStyleSheet);
}


/// <summary>
/// Adds the Global CSS file.
/// </summary>
/// <param name="file">The file.</param>
/// <returns></returns>
void LongUI::CUIControlControl::AddGlobalCssFile(U8View file) noexcept {
    m_pStyleSheet = LongUI::MakeStyleSheetFromFile(file, m_pStyleSheet);
}

#endif

/// <summary>
/// Initializes a new instance of the <see cref="PrivateCC"/> struct.
/// </summary>
LongUI::CUIControlControl::Private::Private() noexcept {
    enum {
        pc_size = sizeof(Private),
        cc_size = impl::cc<sizeof(void*)>::size,
        cc_align = impl::cc<sizeof(void*)>::align,
    };
    static_assert(pc_size == cc_size, "must be same");
    static_assert(alignof(Private) == cc_align, "must be same");
}

/// <summary>
/// Optimizeds the render.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="region">The region.</param>
/// <param name="length">The length.</param>
/// <returns></returns>
void LongUI::CUIControlControl::RecursiveRender(
    const UIControl& ctrl,
    const RectF region[], 
    uint32_t length) noexcept {
    // 脏矩形渲染: 没有交集就不渲染
    if (length && std::all_of(region, region + length, [&](const RectF& rect) {
        return !LongUI::IsOverlap(rect, ctrl.m_oBox.visible);
    })) return;
    // 看不到就不渲染
    const auto csize = ctrl.GetBoxSize();
    if (!ctrl.IsVisible() || csize.width <= 0.f || csize.height <= 0.f)
        return;
    // 设置世界转换矩阵
    ctrl.apply_world_transform();
    // 设置裁剪矩形
    ctrl.apply_clip_rect();
    // 区域渲染
    ctrl.Render();
    // 渲染子节点
    for (auto& child : ctrl) RecursiveRender(child, region, length);
    // 取消裁剪矩形
    ctrl.cancel_clip_rect();
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIControlControl"/> class.
/// </summary>
LongUI::CUIControlControl::CUIControlControl() noexcept {
    m_oHeadTimeCapsule = { nullptr, static_cast<CUITimeCapsule*>(&m_oTailTimeCapsule) };
    m_oTailTimeCapsule = { static_cast<CUITimeCapsule*>(&m_oHeadTimeCapsule), nullptr };
    impl::ctor_dtor<Private>::create(&cc());
    m_dwTimeTick = LongUI::GetTimeTick();
    m_dwDeltaTime = 0;
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIControlControl"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIControlControl::~CUIControlControl() noexcept {
#ifndef LUI_DISABLE_STYLE_SUPPORT
    LongUI::DeleteStyleSheet(m_pStyleSheet);
#endif
    this->dispose_all_time_capsule();
    cc().~Private();
}

/// <summary>
/// Disposes all time capsule.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::dispose_all_time_capsule() noexcept {
    // 强制释放所有时间胶囊
    //m_oHead.next->m_oTail;
    const auto tcbegin = m_oHeadTimeCapsule.next;
    const auto tcend = &m_oTailTimeCapsule;
    for (auto node = tcbegin; node != tcend; ) {
        const auto old = static_cast<CUITimeCapsule*>(node);
        node = node->next;
        old->dispose();
    }
}


/// <summary>
/// Refreshes the time capsule.
/// 更新最后终止的时间胶囊
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::RefreshTimeCapsule(UIControl& ctrl) noexcept {
    assert(ctrl.m_pLastEnd == nullptr && "must be null");
    const auto tcbegin = m_oHeadTimeCapsule.next;
    const auto tcend = &m_oTailTimeCapsule;
    CUITimeCapsule* last_end = nullptr;
    // 遍历所有节点寻找最后终止的时间胶囊
    for (auto node = tcbegin; node != tcend; node = node->next) {
        const auto capsule = static_cast<CUITimeCapsule*>(node);
        // 必须是相同的
        if (capsule->IsSameHoster(ctrl)) {
            // last_end比capsule还长寿就不用替换了
            if (!(last_end && last_end->IsMoreMOThan(*capsule))) 
                last_end = capsule;
        }
    }
    // 更新数据
    ctrl.m_pLastEnd = last_end;
}

/// <summary>
/// Disposes the time capsule.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::DisposeTimeCapsule(UIControl& ctrl) noexcept {
    const auto tcbegin = m_oHeadTimeCapsule.next;
    const auto tcend = &m_oTailTimeCapsule;
#ifndef NDEBUG
    bool disposed = false;
#endif
    // 遍历
    for (auto node = tcbegin; node != tcend; ) {
        const auto old = static_cast<CUITimeCapsule*>(node);
        node = node->next;
        // 检测是否存在: 强制释放同一拥有者所有时间胶囊
        if (old->IsSameHoster(ctrl)) {
#ifndef NDEBUG
            disposed = true;
#endif
            old->dispose();
        }
    }
#ifndef NDEBUG
    assert(disposed && "no object disposed");
#endif
}

/// <summary>
/// Refreshes the time capsule.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="capsule">The capsule.</param>
/// <returns></returns>
void LongUI::CUIControlControl::refresh_time_capsule(
    UIControl& ctrl, CUITimeCapsule& capsule) noexcept {
    // 可能有比较长寿
    if (ctrl.m_pLastEnd) {
        // 还是自己比较寿命
        if (ctrl.m_pLastEnd->IsMoreMOThan(capsule)) return;
    }
    // 没机会
    ctrl.m_pLastEnd = &capsule;
}

/// <summary>
/// Updates the time capsule.
/// </summary>
/// <param name="delta">The delta.</param>
/// <returns></returns>
void LongUI::CUIControlControl::update_time_capsule(float delta) noexcept {
    const auto tcbegin = m_oHeadTimeCapsule.next;
    const auto tcend = &m_oTailTimeCapsule;
    // 遍历所有节点
    for (auto node = tcbegin; node != tcend; ) {
        const auto old = static_cast<CUITimeCapsule*>(node);
        node = node->next;
        // 直接调用: 正常流程释放时间胶囊
        if (old->Call(delta)) {
            old->normal_check_hoster_last_end();
            old->dispose();
        }
    }
}


/// <summary>
/// Determines whether [has time capsule].
/// </summary>
/// <returns></returns>
bool LongUI::CUIControlControl::has_time_capsule() const noexcept {
    const auto tcbegin = m_oHeadTimeCapsule.next;
    const auto tcend = &m_oTailTimeCapsule;
    return tcbegin != tcend;
}


/// <summary>
/// Controls the attached.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::ControlAttached(UIControl& ctrl) noexcept {
    // 1.为控件链接新的样式表(有的话)
    assert(!"TODO");
}

/// <summary>
/// Removes the reference.
/// </summary>
void LongUI::CUIControlControl::ControlDisattached(UIControl& ctrl) noexcept {
    const bool case0 = ctrl.exist_basic_animation();
    const bool case1 = !ctrl.is_inited();
    const bool case2 = ctrl.m_state.in_update_list;

    const bool case4 = !!ctrl.m_pLastEnd;
#ifdef LUI_DISABLE_STYLE_SUPPORT
    const bool case5 = false;
#else
    const bool case5 = !!ctrl.m_oStyle.extra_abcount;
#endif
    const bool case_call = case0 | case1 | case2 | case4 | case5;
    // 检查引用
    if (case_call) {
        CUIDataAutoLocker locker;
        // 0. 动画引用
        if (ctrl.exist_basic_animation()) {
            CUIRenderAutoLocker rlocker;
            const auto b = cc().basic_anima.begin();
            const auto e = cc().basic_anima.end();
            for (auto itr = b; itr != e; ++itr) {
                if (itr->ctrl == &ctrl) {
                    itr->done = itr->duration;
                    itr->ctrl = nullptr;
#ifndef NDEBUG
                    ctrl.clear_basic_animation();
#endif
                    break;
                }
            }
            assert(!ctrl.exist_basic_animation());
        }
        // 1. 移除初始化表中的引用
        if (!ctrl.is_inited()) {
            // 调用对应
            cc().remove_from_init_list(ctrl);
        }
        // 2. 移除刷新表中的引用
        else if (ctrl.m_state.in_update_list) {
            const size_t offset = offsetof(UIControl, m_oManager.next_delinitupd);
            CUIControlControl::RemoveControlInList(ctrl, luiref cc().update_list, offset);
        }
        // 3. 移除状态表中的引用
        //if (ctrl.m_state.in_ccstate_list) {
        //    const size_t offset = offsetof(UIControl, m_oManager.next_ccstate);
        //    CUIControlControl::RemoveControlInList(ctrl, luiref cc().ccstate_list, offset);
        //}
        // 4. 移除时间胶囊
        if (ctrl.m_pLastEnd) {
            this->DisposeTimeCapsule(ctrl);
            ctrl.m_pLastEnd = nullptr;
        }
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // 5. TEST THIS: 高阶动画引用
        if (ctrl.m_oStyle.extra_abcount) {
            const auto b = cc().extra_anima.begin();
            const auto e = cc().extra_anima.end();
            for (auto itr = b; itr != e; ++itr) {
                if (itr->ctrl == &ctrl) {
                    itr->done = itr->duration;
                    itr->ctrl = nullptr;
#ifndef NDEBUG
                    assert(ctrl.m_oStyle.extra_abcount && "bad count");
                    ctrl.m_oStyle.extra_abcount--;
#endif
                }
            }
            assert(ctrl.m_oStyle.extra_abcount == 0 && "bad count");
        }
#endif
    }
}

PCN_NOINLINE
/// <summary>
/// Removes the control in list.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="list">The list.</param>
/// <param name="offset">offset list.</param>
/// <returns></returns>
void LongUI::CUIControlControl::RemoveControlInList(
    UIControl& ctrl, ControlNode& list, size_t offset) noexcept {
    //auto node = list.first;
    auto prev_addr = &list.first;
    assert(offset % sizeof(void*) == 0);
    // 获取节点对象
    const auto get_node_addr = [offset](UIControl& c) noexcept {
        const auto ptr = reinterpret_cast<char*>(&c) + offset;
        return reinterpret_cast<UIControl**>(ptr);
    };
    // 无效列表
    if (!*prev_addr) {
#ifndef NDEBUG
        LUIDebug(Warning) << "list: nullptr" << endl;
#endif
        return;
    }
    // 删除的是唯一一个
    if (list.last == &ctrl && list.last == list.first) {
        list.last = list.first = nullptr;
        return;
    }
#ifndef NDEBUG
    bool ctrl_found = false;
#endif
    // 查找
    UIControl* last = nullptr;
    while (const auto node = *prev_addr) {
        if (*prev_addr == &ctrl) {
            // 前面节点链接后面
            const auto next_node = *get_node_addr(ctrl);
            *prev_addr = next_node;
#ifndef NDEBUG
            ctrl_found = true;
#endif
            break;
        }
        last = node;
        prev_addr = get_node_addr(*node);
    }
    // 删除的是最后一个
    if (list.last == &ctrl) list.last = last;
#ifndef NDEBUG
    if (!ctrl_found)
        LUIDebug(Warning) << "ctrl not found in list-" << ctrl << endl;
#endif
}

PCN_NOINLINE
/// <summary>
/// as the dd control to list.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="list">The list.</param>
/// <param name="offset">offset list.</param>
/// <returns></returns>
void LongUI::CUIControlControl::AddControlToList(UIControl& ctrl, 
    ControlNode& list, size_t offset) noexcept {
    assert(offset % sizeof(void*) == 0);
    // 获取节点对象
    const auto get_node_addr = [offset](UIControl& c) noexcept {
        const auto ptr = reinterpret_cast<char*>(&c) + offset;
        return reinterpret_cast<UIControl**>(ptr);
    };
    const auto ptr = get_node_addr(ctrl);
    assert(*ptr == nullptr);
    // 保险
    *ptr = nullptr;
    // TODO: 使用二级指针取消分支判断

    // 表正常
    if (list.first) {
        *get_node_addr(*list.last) = &ctrl;
        list.last = &ctrl;
    }
    // 表为空
    else {
        assert(list.last == nullptr);
        list.first = list.last = &ctrl;
    }
}

/// <summary>
/// Removes the pointer item.
/// </summary>
/// <param name="">The .</param>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::RemovePointerItem(PointerVector& list, void* ptr) noexcept {
    auto& vector = reinterpret_cast<POD::Vector<void*>&>(list);
    if (!vector) return;
    assert(!vector.empty() && "vector cannot be empty");
    assert(vector.size_of_template() == sizeof(ptr));
    // 优化 删除最后一个
    if (vector.back() == ptr) return vector.pop_back();
    // 遍历数组
    for (auto itr = vector.begin(); itr != vector.end(); ++itr) {
        if (*itr == ptr) {
            vector.erase(itr);
#ifndef NDEBUG
            // 增加调试强度
            auto copy_list = vector;
            vector = std::move(copy_list);
#endif
            return;
        }
    }
    assert(!"item not found");
}

/// <summary>
/// Adds the update list.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::AddUpdateList(UIControl& ctrl) noexcept {
    // 首先要不在表中
    if (!ctrl.m_state.in_update_list) {
        ctrl.m_state.in_update_list = true;
        // 还没有初始化就算了
        if (!ctrl.is_inited()) return;
        cc().add_into_update_list(ctrl);
    }
}

/// <summary>
/// Adds the initialize list.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::AddInitList(UIControl& ctrl) noexcept {
    assert(ctrl.is_inited() == false);
    const size_t offset = offsetof(UIControl, m_oManager.next_delinitupd);
    CUIControlControl::AddControlToList(ctrl, luiref cc().init_list_lock_free, offset);
}

/// <summary>
/// Deletes the later.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::delete_later(UIControl& ctrl) noexcept {
    const size_t offset = offsetof(UIControl, m_oManager.next_delinitupd);
    // 移除初始化表中的引用
    if (!ctrl.is_inited()) {
#ifndef NDEBUG
        LUIDebug(Hint) << "delete_later: " << ctrl
            << "but uninited" << endl;
#endif // !NDEBUG

        ctrl.m_state.inited = true;
        cc().remove_from_init_list(ctrl);
    }
    assert(ctrl.m_oManager.next_delinitupd == nullptr);
    CUIControlControl::AddControlToList(ctrl, luiref cc().delete_list, offset);
}



/// <summary>
/// Invalidates the control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIControlControl::InvalidateControl(UIControl& ctrl, const RectF* rect) noexcept {
    if (const auto wnd = ctrl.GetWindow()) wnd->InvalidateControl(ctrl, rect);
}

// UI Window
#include <core/ui_window.h>

/// <summary>
/// Initializes the control in list.
/// </summary>
/// <returns></returns>
bool LongUI::CUIControlControl::init_control_in_list() noexcept {
    auto& cc = this->cc();
#ifdef LUI_USING_CTOR_LOCKER
    auto node = cc.init_list.first;
    cc.init_list = { nullptr, nullptr };
#else
    auto node = cc.init_list_lock_free.first;
    cc.init_list_lock_free = { nullptr, nullptr };
#endif
    // 遍历初始化列表
    while (node) {
        // 本次处理
        const auto ctrl = node;
        node = ctrl->m_oManager.next_delinitupd;
        ctrl->m_oManager.next_delinitupd = nullptr;
        // 尝试初始化
        assert(!ctrl->is_inited() && "has been inited");
#ifndef NDEBUG
        if (const auto p = ctrl->GetParent()) {
            if (!std::strcmp(p->name_dbg, "root")) {
                int bk = 9;
            }
        }
        sg_bDebugFlag = true;
#endif
        ctrl->init();
#ifndef NDEBUG
        sg_bDebugFlag = false;
#endif
        assert(ctrl->m_oManager.next_delinitupd == nullptr);
        // 添加到更新列表
        if (ctrl->m_state.reason)
            cc.add_into_update_list(*ctrl);
    }
    // 存在布局列表
    return !!cc.update_list.first;
}

/// <summary>
/// Deletes the controls.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::delete_controls() noexcept {
    auto& cc = this->cc();
    auto node = cc.delete_list.first;
    cc.delete_list = { nullptr, nullptr };
    // 遍历初始化列表
    while (node) {
        // 本次处理
        const auto ctrl = node;
        node = ctrl->m_oManager.next_delinitupd;
        delete ctrl;
    }
}


#ifndef DEBUG
namespace LongUI {
    // debug counter
    static int cc_debug_counter = 0;
}
#endif

/// <summary>
/// update the control in list.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::update_control_in_list() noexcept {
    auto& cccc = cc();
    // 刷新控件
    auto update = [](UIControl& ctrl) noexcept {
        // 标记移除(可能在update里面再次标记)
        ctrl.m_state.in_update_list = false;
        const auto reason = ctrl.m_state.reason;
        ctrl.m_state.reason = Reason_NonChanged;
#ifndef NDEBUG
        // 进行刷新
        ctrl.m_state.dbg_in_update = true;
        ctrl.Update(reason);
        ctrl.m_state.dbg_in_update = false;
#else
        // 进行刷新
        ctrl.Update(reason);
#endif // !NDEBUG

        // XXX: 这里调用 检测世界修改?
        if ((ctrl.m_state.world_changed) && ctrl.GetWindow())
            ctrl.GetWindow()->SetControlWorldChanged(ctrl);
    };
    // 遍历更新表
    auto node = cccc.sort_update();
#ifdef LUI_SORT_UPDATE_LIST
#ifndef NDEBUG
    // 以防BUG
    uint16_t level_dbg = 0xffff;
    const auto count_dbg = cccc.update_count;
    auto node_dbg = node;
    uint32_t i_dbg = 0;
    // 遍历初始化列表
    while (node_dbg) {
        node_dbg = node_dbg->m_oManager.next_delinitupd;
        assert(i_dbg != count_dbg);
        ++i_dbg;
    }
    assert(i_dbg == count_dbg);
#endif
    cccc.update_count = 0;
    cccc.update_max_depth = 0;
    cccc.update_min_depth = 0xffff;
#endif
    cccc.update_list = { nullptr, nullptr };
    // 遍历初始化列表
    while (node) {
        // 本次处理
        const auto ctrl = node;
#ifndef NDEBUG
#ifdef LUI_SORT_UPDATE_LIST
        const uint16_t lv = ctrl->GetLevel();
        assert(level_dbg >= lv);
        level_dbg = lv;
#endif
#endif
        node = ctrl->m_oManager.next_delinitupd;
        ctrl->m_oManager.next_delinitupd = nullptr;
        // 更新
        update(*ctrl);
    }
}

/// <summary>
/// sort the node
/// </summary>
/// <param name="node"></param>
/// <param name="max"></param>
/// <param name="count"></param>
/// <returns></returns>
auto LongUI::CUIControlControl::Private::sort_update() noexcept -> UIControl* {
#ifdef LUI_SORT_UPDATE_LIST
    assert(update_count);
    assert(update_max_depth >= update_min_depth);
    // 只有一层就不用排序
    if (update_min_depth == update_max_depth) return update_list.first;
    // 使用DEPTH信息排序阈值
    constexpr uint32_t DEPTH_SORT_THRESHOLD = 32;
    // DEPTH信息排序, 算法复杂度 O(N), 但是初始代价较高
    UIControl* head[MAX_CONTROL_TREE_DEPTH];
    const auto count = update_count;
    if (count > DEPTH_SORT_THRESHOLD) {
        // 空间复杂度 S(D)
        UIControl** tail[MAX_CONTROL_TREE_DEPTH];
        const auto min = update_min_depth;
        const auto max = update_max_depth + 1;
        assert(max > min);
        std::memset(head + min, 0, sizeof(head[0]) * (max - min));
        for (auto i = min; i != max; ++i) tail[i] = head + i;
        // 遍历表格
        auto node = update_list.first;
        while (node) {
            // 本次处理
            const auto ctrl = node;
            node = ctrl->m_oManager.next_delinitupd;
            const auto level = ctrl->GetLevel();
            assert(level >= min);
            assert(level < max);
            assert(level < MAX_CONTROL_TREE_DEPTH);
            if (!head[level]) head[level] = ctrl;
            *tail[level] = ctrl;
            tail[level] = &ctrl->m_oManager.next_delinitupd;
        }
        assert(head[update_min_depth] && head[update_max_depth]);
        // 收尾
        *tail[min] = nullptr;
        auto i = update_max_depth;
        const auto last = update_min_depth;
        while (true) {
            const auto head_this = head[i];
            const auto tail_this = tail[i];
            --i;
            if (head_this) {
                while (!head[i]) --i;
                *tail_this = head[i];
            }
            if (i == last) break;
        }
        return head[update_max_depth];
    }
    // 冒泡排序, 算法复杂度 O(N*N)
    else {
        // 空间复杂度 S(N)
        static_assert(DEPTH_SORT_THRESHOLD <= MAX_CONTROL_TREE_DEPTH, "LESS");
        // 遍历初始化列表
        auto node = update_list.first;
        uint32_t i = 0;
        while (node) {
            head[i] = node; ++i;
            node = node->m_oManager.next_delinitupd;
        }
        // 指针排序
        const auto bi = reinterpret_cast<const void**>(const_cast<const UIControl**>(head));
        const auto ei = reinterpret_cast<const void**>(const_cast<const UIControl**>(head + i));
        constexpr uint32_t offset = offsetof(UIControl, m_state.level);
        constexpr uint32_t offset4 = offset & uint32_t(~3);
        constexpr uint32_t offset_remain = offset & 3;
        constexpr uint32_t mask = helper::u8u32_mask<offset_remain>::value;
        LongUI::SortPointers(bi, ei, offset4, mask);
        // 串起来
        assert(count > 1);
        for (uint16_t i = 1; i < count; ++i) {
            head[i]->m_oManager.next_delinitupd = head[i - 1];
        }
        head[0]->m_oManager.next_delinitupd = nullptr;
        return head[count - 1];
    }
#else
    return update_list.first;
#endif
}

#include <control/ui_viewport.h>

#if 0
// longui::impl namespace
namespace LongUI { namespace impl {
    // mark two rect
    inline void mark_two_rect_dirty(
        const RectF& a, 
        const RectF& b,
        const Size2F size,
        CUIWindow& window
        ) noexcept {
        const auto is_valid = [size](const RectF& rect) noexcept {
            return LongUI::GetArea(rect) > 0.f
                && rect.right > 0.f
                && rect.bottom > 0.f
                && rect.left < size.width
                && rect.top < size.height
                ;
        };
        // 判断有效性
        const auto av = is_valid(a);
        const auto bv = is_valid(b);
        // 都无效
        if (!av && !bv) return;
        // 合并矩形
        RectF merged_rect = a;
        // 都有效
        if (av && bv) {
            // 存在交集
            if (LongUI::IsOverlap(a, b)) {
                merged_rect.top = std::min(a.top, b.top);
                merged_rect.left = std::min(a.left, b.left);
                merged_rect.right = std::max(a.right, b.right);
                merged_rect.bottom = std::max(a.bottom, b.bottom);
            }
            // 没有交集
            else window.MarkDirtRect(b);
        }
        // 只有B有效
        else if (bv) merged_rect = b;
        // 标记
        window.MarkDirtRect(merged_rect);
    }
}}
#endif

#if 0
/// <summary>
/// Dirties the update.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::dirty_update() noexcept {
    auto& list = cc().dirty_update;
    // 进行脏更新
    for (auto& x : list) {
        x.ctrl->m_state.in_dirty_list = false;
        const auto window = x.ctrl->GetWindow();
        assert(window && "bad window");
        // XXX: 大概率是[两个相同的矩形或者第一个无效]
        // 标记两次矩形位置
        impl::mark_two_rect_dirty(
            x.rect,
            x.ctrl->RefBox().visible,
            window->RefViewport().GetRealSize(),
            *window
        );
    }
    // 清空
    list.clear();
}
#endif

/// <summary>
/// Normals the update.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::normal_update() noexcept {
    // 更新时间
    const auto new_tick = LongUI::GetTimeTick();
    const auto delta = new_tick - m_dwTimeTick;
    m_dwDeltaTime = delta;
    m_dwTimeTick = new_tick;
    // 更新基本动画
    this->update_basic_animation(delta);
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // 更新额外动画
    this->update_extra_animation(delta);
#endif
}

/// <summary>
/// Updates the basic animation.
/// </summary>
/// <param name="delta">The delta.</param>
/// <returns></returns>
void LongUI::CUIControlControl::update_basic_animation(uint32_t delta)noexcept {
    // 本函数在渲染线程调用，不用锁?
    //CUIRenderAutoLocker locker;
    // 动画为空
    auto& animations = cc().basic_anima;
    if (animations.empty()) return;
    // 更新基本动画
    auto basic_update = [delta](ControlAnimationBasic& ca) noexcept {
        ca.done += delta;
        auto& ctrl = *ca.ctrl;
        // 完成动画: ctrl 标记为0
        if (ca.done >= ca.duration) {
            ca.done = ca.duration;
            ca.ctrl->clear_basic_animation();
            ca.ctrl = nullptr;
        }
        // 前景色不同
        if (ca.fgcolor1 != ca.fgcolor2) {
            const auto p0 = ca.GetRate();
            ColorF from, to;
            ColorF::FromRGBA_RT(from, { ca.fgcolor1 });
            ColorF::FromRGBA_RT(to, { ca.fgcolor2 });
            const auto color = LongUI::Mix(from, to, p0).ToRGBA().primitive;
            ctrl.SetFgColor({ color });
        }
    };
    // 更新动画
    for (auto& x : animations) {
        // 检查有效
        if (!x.ctrl) continue;
        // 更新矩形 没有动画就没有必要
        if (x.duration) x.ctrl->Invalidate();
        // 基本更新
        basic_update(x);
    }
    // HINT: 一帧最多删一个, 但是动画结束频率肯定不高
    if (!animations.back().ctrl) animations.pop_back();
}


#ifndef LUI_DISABLE_STYLE_SUPPORT
/// <summary>
/// Updates the extra animation.
/// </summary>
/// <param name="delta">The delta.</param>
/// <returns></returns>
void LongUI::CUIControlControl::update_extra_animation(uint32_t delta)noexcept {
    // 动画为空
    auto& animations = cc().extra_anima;
    if (animations.empty()) return;
    // 更新动画
    for (auto& x : animations) {
        // 检查有效
        if (!x.ctrl) continue;
        auto& ctrl = *x.ctrl;
        // 增加delta
        x.done += delta;
        // 完成动画: ctrl 标记为0
        if (x.done >= x.duration) {
            x.done = x.duration;
            assert(x.ctrl->m_oStyle.extra_abcount && "bad count");
            x.ctrl->m_oStyle.extra_abcount--;
            x.ctrl = nullptr;
        }
        // 获取比率P
        const auto at = static_cast<AnimationType>(ctrl.RefStyle().tfunction);
        const auto p0 = LongUI::EasingFunction(at, x.GetRate());
        for (uint32_t i = 0; i != x.length; ++i) {
            const auto ind = LongUI::IndeterminateValue(x.list[i], p0);
            ctrl.ApplyValue(ind);
        }
    }
    // 长度过长则需要手动删除
    if (animations.size() > 100) {
        const auto bgn_itr = animations.begin();
        const auto itr = std::remove_if(bgn_itr, animations.end(), [](const auto&x) noexcept {
            return x.ctrl == nullptr;
        });
        const auto len = itr - bgn_itr;
#ifndef NDEBUG
        LUIDebug(Hint)
            << "ex-animation list shrink resize to: "
            << int(len)
            << endl;
#endif // !NDEBUG
        animations.shrink_resize(len);
        return;
    }
    // HINT: 一帧最多删一个, 但是这里多了手动删除(?)
    if (!animations.back().ctrl) animations.pop_back();
}
#endif

/// <summary>
/// Finds the basic animation.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::CUIControlControl::FindBasicAnimation(
    const UIControl& ctrl) const noexcept
    -> const ControlAnimationBasic* {
    auto& anima = cc().basic_anima;
    // 对比函数
    const auto finder = [&ctrl](const ControlAnimationBasic& ca) noexcept {
        return ca.ctrl == &ctrl;
    };
    const auto end_itr = anima.end();
    const auto itr = std::find_if(anima.begin(), end_itr, finder);
    return end_itr == itr ? nullptr : &(*itr);
}

/// <summary>
/// Starts the basic animation.
/// </summary>
/// <remarks>
/// Basic动画允许多个动画相同时间段并行处理
/// </remarks>
/// <param name="ctrl">The control.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::CUIControlControl::StartBasicAnimation(UIControl& ctrl, StyleStateChange type) noexcept {
    auto& naive_style = UIManager.RefNativeStyle();
    // 检测类型
    const auto native_type = ctrl.m_oStyle.appearance;
    assert(native_type != AttributeAppearance::Appearance_None);
    // 获取动画时间
    const auto dur = naive_style.GetDuration({ type, native_type });
    // 没有动画
    if (!dur) { ctrl.change_state(type); return; }
    // 需要渲染锁
    CUIRenderAutoLocker locker;
    // TODO: 整理代码, 比如先保证vector有效性
    auto& anima = cc().basic_anima;
    using cab_t = ControlAnimationBasic * ;
    cab_t cab = const_cast<cab_t>(this->FindBasicAnimation(ctrl));
    if (!cab) {
        ControlAnimationBasic init_ca;
        init_ca.ctrl = &ctrl;
        init_ca.done = 0;
        init_ca.origin = ctrl.m_oStyle.state;
        init_ca.fgcolor1 = naive_style.GetFgColor(init_ca.origin);
        init_ca.fgcolor2 = init_ca.fgcolor1;
        anima.push_back(init_ca);
        // OOM处理: 变了就不管了
        if (!anima.is_ok()) {
            ctrl.change_state(type);
            //ctrl.NeedUpdate();
            return;
        };
        cab = &anima.back();
    }
    // 超过一帧的直接修改状态
    if (cab->done > m_dwDeltaTime) {
        cab->origin = ctrl.m_oStyle.state;
        cab->done = 0;
    }
    cab->duration = dur;
    ctrl.change_state(type);
    // 获取目标前景色
    cab->fgcolor2 = naive_style.GetFgColor(ctrl.m_oStyle.state);
    // 标记动画中
    cab->ctrl->setup_basic_animation();
}


#ifndef LUI_DISABLE_STYLE_SUPPORT


/// <summary>
/// Gets the style cached control list.
/// </summary>
/// <returns></returns>
auto LongUI::CUIControlControl::GetStyleCachedControlList() noexcept -> void * {
    return &cc().style_cache;
}


// longui::detail
namespace LongUI { namespace impl {
    // std::set_intersection sp-ver, allow set a keep same unit
    template<typename T, typename U, typename Comp>
    auto set_intersection_sp(const T& set0, const T& set1,
        U d_first, U end_of_buf, Comp comp) noexcept {
        auto first1 = set0.begin(), last1 = set0.end();
        auto first2 = set1.begin(), last2 = set1.end();
        while (first1 != last1 && first2 != last2) {
            if (comp(*first1, *first2)) ++first1;
            else {
                if (!comp(*first2, *first1)) {
                    *d_first++ = *first1++;
                    if (d_first == end_of_buf) {
#ifndef NDEBUG
                        LUIDebug(Warning) << "out of buffer!" << endl;
#endif
                        break;
                    }
                }
                else ++first2;
            }
        }
        return d_first;
    }
}}

/// <summary>
/// Makes the off initstate.
/// </summary>
/// <param name="trigger">The trigger.</param>
/// <param name="state">The state.</param>
/// <param name="buf4">The buf4.</param>
/// <param name="buf8">The buf8.</param>
/// <returns></returns>
void LongUI::UIControl::make_off_initstate(
    UIControl* trigger,
    uint32_t state, UniByte4 buf4[], UniByte8 buf8[]) const noexcept {
    // 判断
    const auto& matched = m_oStyle.matched;
    // 判断匹配规则
    for (auto itr = matched.begin(); itr != matched.end();) {
        // 必须是START
        assert((*itr).type == ValueType::Type_NewOne);
        auto& pcl = reinterpret_cast<const SSValuePCL&>(itr[0]);
        static_assert(sizeof(itr[0]) == sizeof(pcl), "must be same!");
        // 检测长度 
        const auto len = pcl.length; assert(len > 1 && "bad size");
        // 匹配状态
        const auto yes = pcl.yes;
        const auto noo = pcl.noo;
        const auto matched = (state & yes) == yes && (state & noo) == 0;
        // 额外计算
        if (matched) {
            // 遍历状态
            const auto end_itr = itr + len;
            auto being_itr = itr + 1;
            for (; being_itr != end_itr; ++being_itr) {
                const auto index = static_cast<uint32_t>((*being_itr).type);
                buf4[index] = (*being_itr).data4;
                buf8[index] = (*being_itr).data8;
            }
        }
        // 递进
        itr += len;
    }

    // 是被触发者
    if (trigger) {
        // 几乎0代价, 所以可以稍微大点
        StyleTrigger trigger_buf[LongUI::SMALL_BUFFER_LENGTH * 2];
        const auto& set0 = trigger->m_oStyle.trigger;
        const auto& set1 = m_oStyle.trigger;
        // 类似与求交集, 但是集合A的允许重复(因为触发条件不同)
        const auto end_buf = impl::set_intersection_sp(
            set0, set1, std::begin(trigger_buf), std::end(trigger_buf),
            [](const auto&x, const auto& y) noexcept { return x.tid < y.tid - 1; }
        );
        // 遍历
        for (auto itr = trigger_buf; itr != end_buf; ++itr) {
            const auto now = trigger->m_oStyle.state;
            const auto yes = itr->yes;
            const auto noo = itr->noo;
            const auto matched = (now & yes) == yes && (now & noo) == 0;
            if (!matched) continue;
            const auto list = LongUI::RefValuesFromBlock(itr->tid);
            for (const auto& z : list) {
                const auto index = static_cast<uint32_t>(z.type);
                buf4[index] = z.data4;
                buf8[index] = z.data8;
            }
        }
#if 0
        // XXX: 三层？
        for (const auto& x : m_oStyle.trigger) {
            if ((x.tid & 1) == 0) continue;
            for (const auto& y : trigger->m_oStyle.trigger) {
                if (y.tid != x.tid - 1) continue;
                const auto now = reinterpret_cast<const uint32_t&>(trigger->m_oStyle.state);
                const uint32_t yes = y.yes;
                const uint32_t noo = y.noo;
                const auto matched = (now & yes) == yes && (now & noo) == 0;
                if (!matched) continue;
                const auto list = LongUI::RefValuesFromBlock(y.tid);
                for (const auto& z : list) {
                    const auto index = static_cast<uint32_t>(z.type);
                    buf4[index] = z.data4;
                    buf8[index] = z.data8;
                }
            }
        }
#endif
    }
}

/// <summary>
/// Extras the animation callback.
/// </summary>
/// <param name="change">The change.</param>
/// <param name="ptr">The PTR.</param>
/// <param name="blocks">The blocks.</param>
/// <returns></returns>
void LongUI::UIControl::extra_animation_callback(
    StyleStateChange change,
    void* ptr, 
    void* blocks) noexcept {
    // 检测参数有效性
    assert(ptr && "bad ptr");
    auto& vector = *reinterpret_cast<POD::Vector<SSFromTo>*>(ptr);
    auto& blockv = *reinterpret_cast<POD::Vector<uintptr_t>*>(blocks);
    //assert(vector.empty() && "not empty");
    // 判断
    const auto& matched = m_oStyle.matched;
    // 检测状态
    const auto from_state = m_oStyle.state;
    // 没变就算了
    if (!this->will_change_state(change)) return;
    this->change_state(change);
    // 记录目标状态
    const auto to_state = m_oStyle.state;
    // 触发器
    for (const auto& x : m_oStyle.trigger) {
        if (x.tid & 1) continue;
        const auto now = from_state;
        const auto too = to_state;
        const auto yes = x.yes;
        const auto noo = x.noo;
        const auto matched1 = (now & yes) == yes && (now & noo) == 0;
        const auto matched2 = (too & yes) == yes && (too & noo) == 0;
        if (matched1 != matched2)
            blockv.push_back(x.tid + matched2);
    }
    // 修改的不为空
    if (matched.empty()) return;
    // 状态缓冲池
    constexpr auto LAST = static_cast<int>(ValueType::SINGLE_LAST) ;
    UniByte4 state_buf[LAST + 1];
    UniByte8 state_buf_ex[LAST + 1]; 
    // 设置初始状态
    // XXX: 延迟初始化? 优化?
    std::memset(state_buf, 0, sizeof(state_buf));
    std::memset(state_buf_ex, 0, sizeof(state_buf_ex));
    LongUI::InitDefaultState(state_buf, state_buf_ex);
    bool writen_buf = false;
    // 优化flag
    bool need_state_buf = false;
    // OFF SET
    if (!change.state_change) {
        const auto now = to_state;
        this->make_off_initstate(nullptr, now, state_buf, state_buf_ex);
    }
    // 判断匹配规则
    for (auto itr = matched.begin(); itr != matched.end();) {
        // 必须是START
        assert((*itr).type == ValueType::Type_NewOne);
        auto& pcl = reinterpret_cast<const SSValuePCL&>(itr[0]);
        static_assert(sizeof(itr[0]) == sizeof(pcl), "must be same!");
        // 检测长度 
        const auto len = pcl.length; assert(len > 1 && "bad size");
        // 匹配状态
        const auto now = from_state;
        const auto too = to_state;
        const auto yes = pcl.yes;
        const auto noo = pcl.noo;
        // XXX: [优化]
        // matched1 有效表示取消该状态, TO状态需要额外计算
        const auto matched1 = (now & yes) == yes && (now & noo) == 0;
        // matched2 有效表示设置该状态, TO状态可以直接设置
        const auto matched2 = (too & yes) == yes && (too & noo) == 0;
        // 不一致时计算动画
        if (matched1 != matched2) {
            // 遍历状态
            const auto end_itr = itr + len;
            auto being_itr = itr + 1;
            for (; being_itr != end_itr; ++being_itr) {
                SSFromTo ft;
                // FROM状态直接获取
                ft.from.type = (*being_itr).type;
                //ft.from.data.u32 = 0;
                this->GetValue(ft.from);
                // TO 状态在 matched2有效则可以直接设置
                if (matched2) ft.to = *being_itr;

                else {
                    ft.to = { ValueType::Type_Unknown };
                    need_state_buf = true;
                }
                // 压入数组
                vector.push_back(ft);
            }
        }
        // 递进
        itr += len;
    }
    // 没有就算了
    if (vector.empty()) return;
    // 计算额外数据
    if (need_state_buf) {
        for (auto& x : vector) {
            if (x.to.type == ValueType::Type_Unknown) {
                const auto index = static_cast<uint32_t>(x.from.type);
                x.to.data4 = state_buf[index];
                x.to.data8 = state_buf_ex[index];
            }
        }
    }
}

/// <summary>
/// Extras the animation callback.
/// </summary>
/// <param name="values">The values.</param>
/// <param name="ptr">The PTR.</param>
/// <param name="set">if set to <c>true</c> [set].</param>
/// <returns></returns>
void LongUI::UIControl::extra_animation_callback(
    UIControl& trigger,
    const SSValues& values,
    void* ptr,
    bool set) noexcept {
    // 检测参数有效性
    assert(ptr && "bad ptr");
    auto& vector = *reinterpret_cast<POD::Vector<SSFromTo>*>(ptr);
    assert(vector.empty() && "not empty");
    // 状态缓冲池
    constexpr auto LAST = static_cast<int>(ValueType::SINGLE_LAST);
    UniByte4 state_buf[LAST + 1];
    UniByte8 state_buf_ex[LAST + 1];
    // 设置初始状态
    // XXX: 延迟初始化? 优化?
    std::memset(state_buf, 0, sizeof(state_buf));
    std::memset(state_buf_ex, 0, sizeof(state_buf_ex));
    LongUI::InitDefaultState(state_buf, state_buf_ex);
    bool writen_buf = false;
    // 优化flag
    bool need_state_buf = false;
    // OFF SET
    if (!set) {
        const auto state = m_oStyle.state;
        this->make_off_initstate(&trigger, state, state_buf, state_buf_ex);
    }
    // 额外规则
    for (const auto& x : values) {
        SSFromTo ft;
        // FROM状态直接获取
        ft.from.type = x.type;
        //ft.from.data.u32 = 0;
        this->GetValue(ft.from);
        // TO 状态在 matched2有效则可以直接设置
        if (set) ft.to = x;

        else {
            ft.to = { ValueType::Type_Unknown };
            need_state_buf = true;
        }
        // 压入数组
        vector.push_back(ft);
    }
    // 计算额外数据
    if (need_state_buf) {
        for (auto& x : vector) {
            if (x.to.type == ValueType::Type_Unknown) {
                const auto index = static_cast<uint32_t>(x.from.type);
                x.to.data4 = state_buf[index];
                x.to.data8 = state_buf_ex[index];
            }
        }
    }
}

/// <summary>
/// Animations the property filter.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
auto LongUI::UIControl::animation_property_filter(void*ptr)noexcept->uint32_t{
    // 检测参数有效性
    assert(ptr && "bad ptr");
    auto& vector = *reinterpret_cast<POD::Vector<SSFromTo>*>(ptr);
    auto len = static_cast<uint32_t>(vector.size());
    // 遍历数组
    for (auto& x : vector) {
        // 去掉不需要动画的
        const auto ez = LongUI::GetEasyType(x.from.type);
        if (ez == ValueEasyType::Type_NoAnimation 
            || m_oStyle.tduration == 0 ) {
            // 设置即时值
            SSValue value;
            value.type = x.from.type;
            value.data4 = x.to.data4;
            value.data8 = x.to.data8;
            this->ApplyValue(value);
            // 取消这份
            x.from.type = ValueType::Type_Unknown;
            --len;
        }
    }
    // 返回0则需要clear
    return len;
}

/// <summary>
/// Starts the extra animation.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="type">The type.</param>
/// <param name="animation_only">if set to <c>true</c> [animation only].</param>
/// <returns></returns>
/// <remarks>
/// Extra动画允许多个动画不同时间段并行处理
/// </remarks>
void LongUI::CUIControlControl::StartExtraAnimation(UIControl& ctrl, StyleStateChange type) noexcept {
    // 断言类型
    //assert(ctrl.m_oStyle.appearance == AttributeAppearance::Appearance_None);
    // 先判断有没有匹配规则
    if (ctrl.RefStyle().matched.empty() && ctrl.RefStyle().trigger.empty()) {
        ctrl.change_state(type);
        return;
    }
    const auto cacheptr = this->GetStyleCachedControlList();
    auto& blocks = *reinterpret_cast<POD::Vector<uintptr_t>*>(cacheptr);
    blocks.clear();
    // 利用回调获取修改的属性
    auto& from_to_list = cc().from_to;
    from_to_list.clear();
    ctrl.extra_animation_callback(type, &from_to_list, &blocks);
    this->do_animation_in_from_to(ctrl);
    // 检测blocks
    if (!blocks.empty()) {
        for (const auto block : blocks) {
            const bool set = block & 1;
            const auto ptr = block & (~uintptr_t(1));
            const auto& values = LongUI::RefValuesFromBlock(ptr);
            const auto& ctrls = LongUI::RefControlsFromBlock(ptr);
            for (const auto x : ctrls) {
                if (x->IsDescendantOrSiblingFor(ctrl)) {
                    from_to_list.clear();
                    x->extra_animation_callback(ctrl, values, &from_to_list, set);
                    this->do_animation_in_from_to(*x);
                }
            }
        }
    }
}

/// <summary>
/// Does the animation in from to.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::do_animation_in_from_to(UIControl & ctrl) noexcept {
    auto& from_to_list = cc().from_to;
    // 筛选属性, 有些不能用于动画
    const auto len = ctrl.animation_property_filter(&from_to_list);
    // 没有动画
    if (!len) return from_to_list.clear();
    // 删除UNKNOWN
    if (len != from_to_list.size()) {
        // [[nodiscard]] warnning
        std::remove_if(from_to_list.begin(), from_to_list.end(),
            [](const SSFromTo& x) noexcept {
            return x.from.type == ValueType::Type_Unknown;
        });
        from_to_list.resize(len);
    }
    // 逆序查找动画, 直到:
    //  0. 找到合适的
    //  1. 查找到结束点
    //  2. 查找到开始了的动画
    // 就算找到了合适的, 但是动画槽满了也要重新创建

    // !! 辣鸡, 不管了, 直接创建
    auto& animation = cc().extra_anima;
    // 计算对象数量
    const uint32_t new_size = (len + EXTRA_FROM_TO_LIST_LENGTH - 1)
        / EXTRA_FROM_TO_LIST_LENGTH;
    const auto index = animation.size();
    animation.resize(index + new_size);
    // OOM处理: 无视掉
    if (!animation.is_ok()) return;
    // 表格
    auto list_itr = from_to_list.begin();
    // 起始点
    const auto begin_itr = animation.begin() + index;
    const auto end_itr = animation.end();
    auto len_remain = len;
    for (auto itr = begin_itr; itr != end_itr; ++itr) {
        // f(x)
        auto&x = *itr;
        // 初始化
        x.ctrl = &ctrl;
        ctrl.m_oStyle.extra_abcount++;
#ifndef NDEBUG
        if (ctrl.m_oStyle.extra_abcount == (MAX_CONTROL_TREE_DEPTH/2)-1) {
            LUIDebug(Warning) << "EXTRA -> HALF MAX" << endl;
        }
        else if (ctrl.m_oStyle.extra_abcount == MAX_CONTROL_TREE_DEPTH-1) {
            LUIDebug(Error) << "EXTRA -> MAX" << endl;
        }
#endif
        x.done = 0;
        x.duration = ctrl.RefStyle().tduration;
        x.length = len_remain > EXTRA_FROM_TO_LIST_LENGTH
            ? EXTRA_FROM_TO_LIST_LENGTH : len_remain;
        // 写入数组
        std::memcpy(x.list, &(*list_itr), sizeof(*x.list) * x.length);
        len_remain -= x.length;
        list_itr += x.length;
    }
    from_to_list.clear();
}
#endif

/// <summary>
/// will change?
/// </summary>
/// <param name="change"></param>
/// <returns></returns>
bool LongUI::UIControl::will_change_state(StyleStateChange change) const noexcept {
    const auto mask = ~change.state_mask;
    const auto state1 = m_oStyle.state;
    const auto state2 = (state1 & mask) | change.state_change;
    return state1 != state2;
}

/// <summary>
/// just change the state
/// </summary>
/// <param name="change">The change.</param>
/// <returns></returns>
void LongUI::UIControl::change_state(StyleStateChange change) noexcept {
    const auto mask = ~change.state_mask;
    const auto state = m_oStyle.state;
    m_oStyle.state = (state & mask) | change.state_change;
#ifndef NDEBUG
    if (m_oStyle.state == state) {
        LUIDebug(Warning) LUI_FRAMEID
            << this
            << "not changed("
            << change
            << ")"
            << endl;
    }
#endif // !NDEBUG
    //return m_oStyle.state != state;
}

// XML 解析
#include <xul/SimpAX.h>
#include <csetjmp>

namespace LongUI {
    // string pair
    inline auto operator""_pair(const char* str, size_t len) {
        return SimpAX::StrPair{ str, str + len };
    }
    // BKDR Hash Function
    auto BKDRHash(const char* str) noexcept->uint32_t;
    // BKDR Hash Function
    auto BKDRHash(const char* strbgn, const char* strend) noexcept->uint32_t;
    // impl
    namespace impl {
        // eval script for window
        void eval_script_for_window(U8View, CUIWindow*) noexcept;
    }
    /// <summary>
    /// xml stream 
    /// </summary>
    struct CUIXulStream final : SimpAX::CAXStream {
        // string_view like
        using StrPair = SimpAX::StrPair;
    public:
        // ctor
        CUIXulStream(UIControl& root) noexcept : m_root(root) {  }
        // dtor
        ~CUIXulStream() noexcept {};
    public:
        // error element
        StrPair                     error = { nullptr };
        // jmp env
        std::jmp_buf                env;
    private:
        // stack base
        UIControl&                  m_root;
#ifndef LUI_NO_SCRIPT
        // script ok
        bool                        m_script = false;
#endif
#ifdef LUI_STYLE_SUPPORT_EX
        // script ok
        bool                        m_style = false;
#endif
    private:
        // add Processing Instruction
        void add_processing(const PIs& attr) noexcept override;
        // begin element
        void begin_element(const StrPair tag) noexcept override;
        // end element
        void end_element(const StrPair tag) noexcept override;
        // add attribute
        void add_attribute(const ATTRs& attr) noexcept override;
        // add comment
        void add_comment(const StrPair) noexcept override {}
        // add text
        void add_text(const StrPair) noexcept override;
    };
    /// <summary>
    /// Adds the processing.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <returns></returns>
    void CUIXulStream::add_processing(const PIs& attr) noexcept {
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // 加载CSS
        if (attr.target == "xml-stylesheet"_pair) {
            // 查找href
            const auto href = CAXStream::FindEquation(attr.instructions, "href");
            if (href.end() != href.begin()) {
                // 默认 chrome://global/skin
#if 0
                const auto base = "chrome://global/skin"_pair;
                if (href == "chrome://global/skin"_pair) {
#else
                const auto chrome = "chrome"_pair;
                if (!std::strncmp(href.a, chrome.a, chrome.b - chrome.a)) {
                    // TODO: chrome 处理
#endif
                }
                else {
                    // 使用窗口载入CSS
                    const auto wnd = m_root.GetWindow();
                    assert(wnd && "can not set css on window is null");
                    wnd->LoadCssFile({ href.begin(), href.end() });
                }
            }
        }
#endif
    }
    /// <summary>
    /// Begins the element.
    /// </summary>
    /// <param name="view">The view.</param>
    /// <returns></returns>
    void CUIXulStream::begin_element(const StrPair view) noexcept {
        // window节点
        if (view == "window"_pair) {
            // 写入数据
            this->stack_top().user_ptr = &m_root;
        }
#ifdef LUI_STYLE_SUPPORT_EX
        // style节点
        else if (view == "style"_pair) {
            m_style = true;
        }
#endif
        // script节点
        else if (view == "script"_pair) {
            // 检查脚本支持
#ifndef LUI_NO_SCRIPT
            m_script = true;
#endif
        }
        // 其他节点
        else {
            // 不能是第一级
            assert(stack_end() - stack_begin() > 1 && "cannot be first");
            // 获取上一级控件
            const auto parent = static_cast<UIControl*>(stack_end()[-2].user_ptr);
            // 创建控件
            const auto ctrl = UIManager.CreateControl({ view.begin(), view.end() }, parent);
            // 错误处理
            if (!ctrl/* || ctrl->IsCtorFailed()*/) {
                //delete ctrl;
                this->error = view;
                LUIDebug(Error) 
                    << "failed to create control: " 
                    << U8View{ view.begin(), view.end() }
                    << endl;
                std::longjmp(this->env, 1);
            }
            // 写入栈顶
            this->stack_top().user_ptr = ctrl;
        }
    }
    /// <summary>
    /// Ends the element.
    /// </summary>
    /// <param name="tag">The tag.</param>
    /// <returns></returns>
    void CUIXulStream::end_element(const StrPair tag) noexcept {
#ifndef LUI_NO_SCRIPT
        m_script = false;
#endif
#ifdef LUI_STYLE_SUPPORT_EX
        m_style = false;
#endif
    }
    /// <summary>
    /// Adds the text.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    void CUIXulStream::add_text(const StrPair pair) noexcept {
#ifndef LUI_NO_SCRIPT
        if (m_script)
            impl::eval_script_for_window({ pair.begin(), pair.end() }, m_root.GetWindow());
#endif
#ifdef LUI_STYLE_SUPPORT_EX
        else if (m_style)
            m_root.GetWindow()->LoadCssString({ pair.begin(), pair.end() });
#endif
    }
    /// <summary>
    /// Adds the attribute.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <returns></returns>
    void CUIXulStream::add_attribute(const ATTRs& attr) noexcept {
        const auto& top = this->stack_top();
        const auto ctrl = static_cast<UIControl*>(top.user_ptr);
        // 检查有效性
        if (ctrl) {
            // 以on开头?
            if (attr.key.b > attr.key.a + 2) {
                const auto ch1 = attr.key.a[0];
                const auto ch2 = attr.key.a[1];
                if (ch1 == 'o' && ch2 == 'n') {
                    const auto id = CUIEventHost::StrToId({ attr.key.a, attr.key.b });
                    ctrl->SetScript(id, { attr.value.a, attr.value.b });
                    return;
                }
            }
            // 计算键HASH
            const auto key = LongUI::BKDRHash(attr.key.a, attr.key.b);
            // 添加属性
            Unsafe::AddAttrUninited(*ctrl, key, { attr.value.a, attr.value.b });
        }
        // 特殊情况
        else {
            // 目前只有一个script
            // TODO: script
        }
    }
}

/// <summary>
/// Makes the xul.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
bool LongUI::CUIControlControl::MakeXul(UIControl& ctrl, const char* xul) noexcept {
    // 解析器, 因为没有使用异常, 所以在这里使用长跳转模拟异常
    CUIXulStream stream{ ctrl };
#ifndef NDEBUG
    // 错误信息
    const auto& error = stream.error;
#endif
    // 设置长跳转环境   顺便, setjmp是宏, 而非函数
    const auto code = setjmp(stream.env);
    // 先去掉所有子控件
    while (ctrl.GetChildrenCount()) {
        auto& control = *ctrl.begin();
        control.SetParent(nullptr);
        control.DeleteLater();
    }
    // 错误代码 
    bool rv = false;
    // 检查错误
    switch (code)
    {
    default:
        // TODO: 基本错误处理
        break;
    case 0:
        // 然后解析XML字符串
        const auto result = stream.Load(xul);
        // 解析错误
        if (!result.IsOk())
            std::longjmp(stream.env, result.code + 1);
        rv = true;
        break;
    }
    // 返回处理
    return rv;
}

