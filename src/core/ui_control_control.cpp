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
// debug
#include <debugger/ui_debug.h>

// c++
#include <cassert>
#include <algorithm>


namespace LongUI {
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // delete style sheet
    void DeleteStyleSheet(CUIStyleSheet* ptr) noexcept;
    // make style sheet
    auto MakeStyleSheet(U8View view, SSPtr old) noexcept->SSPtr;
    // make style sheet
    auto MakeStyleSheetFromFile(U8View view, SSPtr old) noexcept->SSPtr;
    // get data from block
    auto GetValuesFromBlock(uintptr_t id) noexcept ->const SSValues&;
    // get data from block
    auto GetControlsFromBlock(uintptr_t id) noexcept ->const UIControls&;
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
    // init list
    ControlNode             init_list = { nullptr };
    // update list
    ControlNode             update_list = { nullptr };
    // delete list
    ControlNode             delete_list = { nullptr };
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
};


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
        cc_size = detail::cc<sizeof(void*)>::size,
        cc_align = detail::cc<sizeof(void*)>::align,
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
    // ???: 设定了region裁剪区域
    /*
    块呈现器的堆栈顺序如下：
        background color
        background image
        border
        children
        outline
    */
    // TODO: 脏矩形渲染: 没有交集就不渲染
    if (length && std::all_of(region, region + length, [&](const RectF& rect) {
        return !LongUI::IsOverlap(rect, ctrl.m_oBox.visible);
    })) return;
    // 看不到就不渲染
    const auto csize = ctrl.GetSize();
    if (!ctrl.IsVisible() || csize.width <= 0.f || csize.height <= 0.f)
        return;
    // 渲染器设置本身裁剪矩形以免绘制出去
    //auto& r = LongUI::Private::renderer();
    //r.setClipRect(qt({}, ctrl.m_oBox.rect));

    // 设置世界转换矩阵
    ctrl.apply_world_transform();
    // 设置裁剪矩形
    ctrl.apply_clip_rect();
    // 区域渲染上层已经
    ctrl.Render();

    // TODO: 优化 Attachment_Fixed
    // 渲染子节点: 滚动
    for (auto& child : ctrl) {
        if (child.m_state.attachment == Attachment_Scroll)
            RecursiveRender(child, region, length);
    }
    // 渲染子节点: 固定
    for (auto& child : ctrl) {
        if (child.m_state.attachment != Attachment_Scroll)
            RecursiveRender(child, region, length);
    }

    // 取消裁剪矩形
    ctrl.cancel_clip_rect();

    //ctrl.render_children();
}

/// <summary>
/// Initializes a new instance of the <see cref="CUIControlControl"/> class.
/// </summary>
LongUI::CUIControlControl::CUIControlControl() noexcept {
    m_oHeadTimeCapsule = { nullptr, static_cast<CUITimeCapsule*>(&m_oTailTimeCapsule) };
    m_oTailTimeCapsule = { static_cast<CUITimeCapsule*>(&m_oHeadTimeCapsule), nullptr };
    detail::ctor_dtor<Private>::create(&cc());
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
    // 1. 移除初始化表中的引用
    if (!ctrl.is_inited()) {
        CUIControlControl::RemoveControlInList(ctrl, luiref cc().init_list);
    }
    // 2. 移除刷新表中的引用
    if (ctrl.is_in_update_list() && ctrl.is_inited()) {
        ctrl.remove_from_update_list();
        CUIControlControl::RemoveControlInList(ctrl, luiref cc().update_list);
    }
#if 0
    // 3. 移除在脏矩形列表
    if (ctrl.is_in_dirty_list()) {
        ctrl.m_state.in_dirty_list = false;
        assert(ctrl.GetWindow());
        //auto& list = cc().dirty_update;
        //const auto itr = std::find_if(list.begin(), list.end(), [&ctrl](const auto& x) noexcept {
        //    return x.ctrl == &ctrl;
        //});
        //assert(itr != list.end() && "not found in list");
        //list.erase(itr);
    }
#endif
    // 4. 移除时间胶囊
    if (ctrl.m_pLastEnd) {
        this->DisposeTimeCapsule(ctrl);
        ctrl.m_pLastEnd = nullptr;
    }
}

PCN_NOINLINE
/// <summary>
/// Removes the control in list.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="list">The list.</param>
/// <returns></returns>
void LongUI::CUIControlControl::RemoveControlInList(
    UIControl& ctrl, ControlNode& list) noexcept {
    //auto node = list.first;
    auto prev_addr = &list.first;
    // 获取节点对象
    const auto get_node_addr = [](UIControl& c) noexcept {
        return &c.m_oManager.next_delinitupd;
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
/// <returns></returns>
void LongUI::CUIControlControl::AddControlToList(UIControl& ctrl, ControlNode& list) noexcept {
    // 获取节点对象
    const auto get_node_addr = [](UIControl& c) noexcept {
        return &c.m_oManager.next_delinitupd;
    };
    const auto ptr = get_node_addr(ctrl);
    assert(*ptr == nullptr);
    // 保险
    *ptr = nullptr;
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
    // TODO: [优化] 将越接近根节点的控件放在前面
    if (!ctrl.is_in_update_list()) {
        ctrl.add_into_update_list();
        // 还没有初始化就算了
        if (ctrl.is_inited())
            CUIControlControl::AddControlToList(ctrl, luiref cc().update_list);
    }
}

/// <summary>
/// Adds the initialize list.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::AddInitList(UIControl& ctrl) noexcept {
    assert(ctrl.is_inited() == false);
    CUIControlControl::AddControlToList(ctrl, luiref cc().init_list);
}

/// <summary>
/// Deletes the later.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::delete_later(UIControl& ctrl) noexcept {
    // 移除初始化表中的引用
    if (!ctrl.is_inited()) {
#ifndef NDEBUG
        LUIDebug(Hint) << "delete_later: " << ctrl
            << "but uninited" << endl;
#endif // !NDEBUG

        ctrl.m_state.inited = true;
        CUIControlControl::RemoveControlInList(ctrl, luiref cc().init_list);
        ctrl.m_oManager.next_delinitupd = nullptr;
    }
    assert(ctrl.m_oManager.next_delinitupd == nullptr);
    CUIControlControl::AddControlToList(ctrl, luiref cc().delete_list);
}



/// <summary>
/// Invalidates the control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIControlControl::InvalidateControl(UIControl& ctrl, const RectF* rect) noexcept {

    if (const auto wnd = ctrl.GetWindow()) wnd->InvalidateControl(ctrl, rect);
    
#if 0
    //auto& list = cc().dirty_update;
    //assert(LongUI::GetArea(rect) > 0.f);
    // 窗口储存渲染的矩形信息
    if (const auto wnd = ctrl.GetWindow()) {
        // XXX: [优化]如果已经全渲染则没有必要了
        
        // 加入脏更新表
        //list.push_back({ &ctrl, ctrl.GetBox().visible });
        //ctrl.m_state.in_dirty_list = true;
        //// 将矩形映射到窗口坐标系
        //auto wndcs = rect; 
        //ctrl.MapToWindow(wndcs);
        //wnd->MarkDirtRect(wndcs);
    }
#endif
}

// UI Window
#include <core/ui_window.h>

/// <summary>
/// Initializes the control in list.
/// </summary>
/// <returns></returns>
bool LongUI::CUIControlControl::init_control_in_list() noexcept {
    auto& cc = this->cc();
    auto node = cc.init_list.first;
    cc.init_list = { nullptr, nullptr };
    // 遍历初始化列表
    while (node) {
        // 本次处理
        const auto ctrl = node;
        node = ctrl->m_oManager.next_delinitupd;
        ctrl->m_oManager.next_delinitupd = nullptr;
        // 尝试初始化
        assert(!ctrl->is_inited() && "has been inited");
        // TODO: 错误处理
        ctrl->init();
        // 添加到更新列表
        if (ctrl->is_in_update_list())
            // TODO: [优化] 将越接近根节点的控件放在前面
            CUIControlControl::AddControlToList(*ctrl, luiref cc.update_list);
    }
    // 存在更新列表
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
/// Updates the control in list.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::update_control_in_list() noexcept {
    auto& cccc = cc();
    // 更新控件
    auto update = [](UIControl& ctrl) noexcept {
        //LUIDebug(Log) << ctrl << (void*)&ctrl << endl;
        // 标记移除(可能在Update里面再次标记)
        ctrl.remove_from_update_list();
        //const auto casdasd = ctrl.is_in_update_list();
        // 进行刷新
        ctrl.Update();
        // 检测世界修改
        if (ctrl.m_state.world_changed && ctrl.GetWindow())
            ctrl.GetWindow()->SetControlWorldChanged(ctrl);
#if 0
        LUIDebug(Log) LUI_FRAMEID
            << '[' << cc_debug_counter << ']'
            << ctrl.name_dbg
            << endl;
#endif
    };
    // 遍历更新表
    auto node = cccc.update_list.first;
    cccc.update_list = { nullptr, nullptr };

    // 遍历初始化列表
    while (node) {
        // 本次处理
        const auto ctrl = node;
        node = ctrl->m_oManager.next_delinitupd;
        ctrl->m_oManager.next_delinitupd = nullptr;
        // 更新
        update(*ctrl);
    }

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
            x.ctrl->GetBox().visible,
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
            x.ctrl = nullptr;
        }
        // 获取比率P
        const auto at = static_cast<AnimationType>(ctrl.GetStyle().tfunction);
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
    const auto finder = [&ctrl](
        const ControlAnimationBasic& ca) noexcept {
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
void LongUI::CUIControlControl::StartBasicAnimation(
    UIControl& ctrl, 
    StyleStateTypeChange type) noexcept {
    // 检测类型
    const auto native_type = ctrl.m_oStyle.appearance;
    assert(native_type != AttributeAppearance::Appearance_None);
    // 获取动画时间
    const auto dur = LongUI::NativeStyleDuration({ native_type });
    // 没有动画
    if (!dur) return static_cast<void>(ctrl.start_animation_change(type));
    // TODO: 整理代码, 比如先保证vector有效性
    auto& anima = cc().basic_anima;
    using cab_t = ControlAnimationBasic * ;
    cab_t cab = const_cast<cab_t>(this->FindBasicAnimation(ctrl));
    if (!cab) {
        ControlAnimationBasic init_ca;
        init_ca.ctrl = &ctrl;
        init_ca.done = 0;
        init_ca.origin = ctrl.m_oStyle.state;
        init_ca.fgcolor1 = LongUI::NativeFgColor(init_ca.origin);
        init_ca.fgcolor2 = init_ca.fgcolor1;
        anima.push_back(init_ca);
        // OOM处理: 变了就不管了
        if (!anima.is_ok()) {
            ctrl.start_animation_change(type);
            ctrl.NeedUpdate();
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
    ctrl.start_animation_change(type);
    // 获取目标前景色
    cab->fgcolor2 = LongUI::NativeFgColor(ctrl.m_oStyle.state);
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
namespace LongUI { namespace detail {
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
        static_assert(sizeof(StyleState) == sizeof(uint32_t), "must be same");
        const auto yes = reinterpret_cast<const uint32_t&>(pcl.yes);
        const auto noo = reinterpret_cast<const uint32_t&>(pcl.noo);
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
        const auto end_buf = detail::set_intersection_sp(
            set0, set1, std::begin(trigger_buf), std::end(trigger_buf),
            [](const auto&x, const auto& y) noexcept { return x.tid < y.tid - 1; }
        );
        // 遍历
        for (auto itr = trigger_buf; itr != end_buf; ++itr) {
            const auto now = reinterpret_cast<const uint32_t&>(trigger->m_oStyle.state);
            const uint32_t yes = itr->yes;
            const uint32_t noo = itr->noo;
            const auto matched = (now & yes) == yes && (now & noo) == 0;
            if (!matched) continue;
            const auto list = LongUI::GetValuesFromBlock(itr->tid);
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
                const auto list = LongUI::GetValuesFromBlock(y.tid);
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
    StyleStateTypeChange change, 
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
    if (!this->start_animation_change(change)) return;
    // 记录目标状态
    const auto to_state = m_oStyle.state;
    // 触发器
    for (const auto& x : m_oStyle.trigger) {
        if (x.tid & 1) continue;
        const auto now = reinterpret_cast<const uint32_t&>(from_state);
        const auto too = reinterpret_cast<const uint32_t&>(to_state);
        const auto yes = reinterpret_cast<const uint32_t&>(x.yes);
        const auto noo = reinterpret_cast<const uint32_t&>(x.noo);
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
    if (!change.change) {
        const auto now = reinterpret_cast<const uint32_t&>(to_state);
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
        static_assert(sizeof(StyleState) == sizeof(uint32_t), "must be same");
        const auto now = reinterpret_cast<const uint32_t&>(from_state);
        const auto too = reinterpret_cast<const uint32_t&>(to_state);
        const auto yes = reinterpret_cast<const uint32_t&>(pcl.yes);
        const auto noo = reinterpret_cast<const uint32_t&>(pcl.noo);
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
        const auto state = reinterpret_cast<const uint32_t&>(m_oStyle.state);
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
void LongUI::CUIControlControl::StartExtraAnimation(
    UIControl& ctrl, StyleStateTypeChange type) noexcept {
    // 断言类型
    //assert(ctrl.m_oStyle.appearance == AttributeAppearance::Appearance_None);
    // 先判断有没有匹配规则
    if (ctrl.GetStyle().matched.empty() && ctrl.GetStyle().trigger.empty()) {
        ctrl.start_animation_change(type);
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
            const auto& values = LongUI::GetValuesFromBlock(ptr);
            const auto& ctrls = LongUI::GetControlsFromBlock(ptr);
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
        x.done = 0;
        x.duration = ctrl.GetStyle().tduration;
        x.length = len_remain > EXTRA_FROM_TO_LIST_LENGTH
            ? EXTRA_FROM_TO_LIST_LENGTH : len_remain;
        // 写入数组
        std::memcpy(
            x.list, &(*list_itr), sizeof(x.list[0]) * x.length
        );
        len_remain -= x.length;
        list_itr += x.length;
    }
    from_to_list.clear();
}
#endif

/// <summary>
/// Starts the but no animation.
/// </summary>
/// <param name="change">The change.</param>
/// <returns></returns>
bool LongUI::UIControl::start_animation_change(StyleStateTypeChange change) noexcept {
    // 没有改变?
    const auto changed = m_oStyle.state.Change(change);
#ifndef NDEBUG
    if (!changed) {
        LUIDebug(Warning) LUI_FRAMEID
            << "animation not changed("
            << change
            << ")"
            << endl;
    }
#endif // !NDEBUG
    return changed;
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
        // create control
        UIControl* create_control(const char*a, const char*b, UIControl*p) noexcept;
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
                if (href == "chrome://global/skin"_pair) {

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
            const auto ctrl = impl::create_control(view.begin(), view.end(), parent);
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
    while (ctrl.GetCount()) {
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

