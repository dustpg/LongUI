// ui
#include <core/ui_control_control.h>
#include <style/ui_native_style.h>
#include <container/pod_vector.h>
#include <control/ui_control.h>
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
    // dirty Update
    struct DirtyUpdate {
        // rect
        //RectF           rect;
        // ctrl
        UIControl*      ctrl;
        // window
        //CUIWindow*      wnd;
    };
}

/// <summary>
/// private data/func for controlcontrol
/// </summary>
struct LongUI::PrivateCC {
    // control animation
    using CtrlAnima = ControlAnimation;
    // ctor
    inline PrivateCC() noexcept;
    // ctor
    inline ~PrivateCC() noexcept { }
    // update list
    POD::Vector<UIControl*> update_list;
    // 2nd update list
    POD::Vector<UIControl*> update_dofor;
    // next frame update list
    POD::Vector<UIControl*> next_update;
    // init list
    POD::Vector<UIControl*> init_list;
    // animation list
    POD::Vector<CtrlAnima>  animaitons;
    // dirty update
    POD::Vector<DirtyUpdate>dirty_update;
    // xul dir
    CUIString               xul_dir;
};


/// <summary>
/// Sets the xul dir.
/// </summary>
/// <param name="dir">The dir.</param>
/// <returns></returns>
void LongUI::CUIControlControl::SetXULDir(const CUIString& dir) noexcept {
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
auto LongUI::CUIControlControl::GetXULDir() const noexcept -> const CUIString& {
    return cc().xul_dir;
}

/// <summary>
/// Initializes a new instance of the <see cref="PrivateCC"/> struct.
/// </summary>
inline LongUI::PrivateCC::PrivateCC() noexcept {
    // TODO: reserve/OOM
    //update_list.reserve;
    // TODO: 次帧刷新列表...貌似不需要
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
        return !(rect && ctrl.m_oBox.visible);
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
    detail::ctor_dtor<PrivateCC>::create(&cc());
    m_dwTimeTick = LongUI::GetTimeTick();
    m_dwDeltaTime = 0;
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIControlControl"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIControlControl::~CUIControlControl() noexcept {
    cc().~PrivateCC();
}

// ui namespace
namespace LongUI {
    enum {
        pc_size = sizeof(PrivateCC),
        cc_size = detail::cc<sizeof(void*)>::size,
        cc_align = detail::cc<sizeof(void*)>::align,
    };
    static_assert(pc_size == cc_size, "must be same");
    static_assert(alignof(PrivateCC) == cc_align, "must be same");
}

/// <summary>
/// Controls the attached.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::ControlAttached(UIControl& ctrl) noexcept {
    int b = 9;
    // 1.为控件链接新的样式表(有的话)
    ctrl;
}

/// <summary>
/// Removes the reference.
/// </summary>
void LongUI::CUIControlControl::ControlDisattached(UIControl& ctrl) noexcept {
    // 1. 移除初始化表中的引用
    if (!ctrl.is_inited()) {
        LongUI::RemovePointerItem(reinterpret_cast<PointerVector&>(cc().init_list), &ctrl);
    }
    // 2. 移除刷新表中的引用
    if (ctrl.is_in_update_list()) {
        ctrl.remove_from_update_list();
        LongUI::RemovePointerItem(reinterpret_cast<PointerVector&>(cc().update_list), &ctrl);
    }
    // 3. 移除在脏矩形列表
    if (ctrl.is_in_dirty_list()) {
        ctrl.m_state.in_dirty_list = false;
        LongUI::RemovePointerItem(reinterpret_cast<PointerVector&>(cc().dirty_update), &ctrl);
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
    assert(!vector.empty() && "vector cannot be empty");
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
        cc().update_list.push_back(&ctrl);
    }
}

/// <summary>
/// Adds the initialize list.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::AddInitList(UIControl & ctrl) noexcept {
    cc().init_list.push_back(&ctrl);
}

/// <summary>
/// Adds the next update list.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::CUIControlControl::AddNextUpdateList(UIControl & ctrl) noexcept {
    cc().next_update.push_back(&ctrl);
}



/// <summary>
/// Invalidates the control.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::CUIControlControl::InvalidateControl(UIControl& ctrl/*, const RectF& rect*/) noexcept {
    // 已经在里面就算了
    if (ctrl.m_state.in_dirty_list) return;
    auto& list = cc().dirty_update;
    //assert(LongUI::GetArea(rect) > 0.f);
    // 窗口储存渲染的矩形信息
    if (const auto wnd = ctrl.GetWindow()) {
        // XXX: [优化]如果已经全渲染则没有必要了
        // 加入脏更新表
        list.push_back({ &ctrl });
        ctrl.m_state.in_dirty_list = true;
        //// 将矩形映射到窗口坐标系
        //auto wndcs = rect; 
        //ctrl.MapToWindow(wndcs);
        //wnd->MarkDirtRect(wndcs);
    }
}

// UI Window
#include <core/ui_window.h>

/// <summary>
/// Initializes the control in list.
/// </summary>
/// <returns></returns>
bool LongUI::CUIControlControl::init_control_in_list() noexcept {
    auto& cc = this->cc();
    // 遍历初始化列表
    for (auto ctrl : cc.init_list) {
        // 尝试初始化
        assert(!ctrl->is_inited() && "has been inited");
        // TODO: 错误处理
        ctrl->init();
    }
    cc.init_list.clear();
    // 存在更新列表
    return !cc.update_list.empty();
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
    };
    // 交换两者
    cccc.update_dofor.clear();
    cccc.update_list.swap(cccc.update_dofor);
#if 1
    // 单独刷新
    for (auto* ctrl : cccc.update_dofor) update(*ctrl);
#else
    ++cc_debug_counter;
    // 单独刷新
    for (auto* ctrl : cccc.update_dofor) {
        update(*ctrl);
        LUIDebug(Log) LUI_FRAMEID
            << '[' << cc_debug_counter << ']'
            << ctrl->name_dbg
            << endl;
    }
#endif
}

/// <summary>
/// Pushes the next update.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::push_next_update() noexcept {
#ifndef DEBUG
    cc_debug_counter = 0;
#endif
    // 添加次帧刷新
    for (auto ctrl : cc().next_update) this->AddUpdateList(*ctrl);
    cc().next_update.clear();
}


#include <control/ui_viewport.h>

/// <summary>
/// Dirties the update.
/// </summary>
/// <returns></returns>
void LongUI::CUIControlControl::dirty_update() noexcept {
    auto& list = cc().dirty_update;
    // 进行脏更新
    for (auto& x : list) {
        x.ctrl->m_state.in_dirty_list = false;
        const auto wnd = x.ctrl->GetWindow();
        const auto csize = x.ctrl->GetSize();
        RectF rect = { 0, 0, csize.width, csize.height };
        x.ctrl->MapToWindow(rect);
        const auto size = wnd->RefViewport().GetSize();
        // 不在窗口内显示?
        if (rect.right <= 0.f ||
            rect.bottom <= 0.f ||
            rect.left >= size.width ||
            rect.top >= size.height) 
            continue;
        // 面积为0?
        // if (LongUI::GetArea(rect) == 0.f) continue;
        // 标记脏矩形
        wnd->MarkDirtRect(rect);
    }
    // 清空
    list.clear();
}

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
    // 动画为空
    auto& animations = cc().animaitons;
    if (animations.empty()) return;
    // 更新基本动画
    auto basic_update = [delta](ControlAnimation& ca) noexcept {
        ca.basic.done += delta;
        // 完成动画: ctrl 标记为0
        if (ca.basic.done >= ca.basic.duration) {
            ca.basic.done = ca.basic.duration;
            //ca.ctrl->m_oStyle.state = ca.basic.target;
            //ca.ctrl->m_state.style_state_changed = false;
            ca.ctrl->m_pAnimation = nullptr;
            ca.ctrl = nullptr;
        }
    };
    // 更新复杂动画
    //auto extra_update = [](ControlAnimation& ca) noexcept {
    //    assert(!"unsupported yet");
    //};
    // 更新动画
    for (auto& x : animations) {
        // 检查有效
        if (!x.ctrl) continue;
        // 更新矩形 没有动画就没有必要
        if (x.basic.duration) x.ctrl->Invalidate();
        // 额外动画?
        //if (x.extra) extra_update(x);
        //else basic_update(x);
        basic_update(x);
    }
    // 最后一个无效?
    // NOTE: 一帧最多删一个, 但是动画结束频率肯定不高
    if (!animations.back().ctrl) animations.pop_back();
}



/// <summary>
/// Starts the animation.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::CUIControlControl::StartAnimation(
    UIControl& ctrl, 
    StyleStateTypeChange type) noexcept {
    //LUIDebug(Hint) << ctrl << type << endl;
    auto& anima = cc().animaitons;
    // 对比函数
    const auto finder = [&ctrl](ControlAnimation& ca) noexcept {
        return ca.ctrl == &ctrl;
    };
    // 查找已有动画
    const auto itr = std::find_if(anima.begin(), anima.end(), finder);
    ControlAnimation* ca;
    if (itr != anima.end()) ca = &(*itr);
    else {
        ControlAnimation init_ca;
        init_ca.ctrl = &ctrl;
        init_ca.basic.done = 0;
        init_ca.basic.origin = ctrl.m_oStyle.state;
        //init_ca.extra = nullptr;
        anima.push_back(init_ca);
        // TODO: OOM处理
        if (!anima) return;
        ca = &anima.back();
    }
    // 检测类型
    const auto native_type = ctrl.m_oStyle.appearance;
    // 自带类型
    if (native_type != AttributeAppearance::Appearance_None) {
        // 超过一帧的直接修改状态
        if (ca->basic.done > m_dwDeltaTime) {
            ca->basic.origin = ctrl.m_oStyle.state;
            ca->basic.done = 0;
        }
        // 基本动画直接复写已存在动画数据
        const auto changed = ctrl.m_oStyle.state.Change(type);
        assert(changed && "not changed");
        if (changed) ctrl.m_state.style_state_changed = true;
        //ctrl.NeedUpdate();
        ca->basic.duration = LongUI::NativeStyleDuration({ native_type });
    }
    // 自定义类型
    else {
        assert(!"unsupported yet");
    }
    // 设置动画
    ctrl.m_pAnimation = ca;
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
    }
    /// <summary>
    /// xml stream 
    /// </summary>
    struct CUIControlControl::CUIXulStream final : SimpAX::CAXStream {
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
    private:
        // add Processing Instruction
        void add_processing(const PIs& attr) noexcept override;
        // begin element
        void begin_element(const StrPair tag) noexcept override;
        // end element
        void end_element(const StrPair tag) noexcept override {}
        // add attribute
        void add_attribute(const ATTRs& attr) noexcept override;
        // add comment
        void add_comment(const StrPair) noexcept override {}
        // add text
        void add_text(const StrPair) noexcept override {}
    };
    /// <summary>
    /// Adds the processing.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <returns></returns>
    void CUIControlControl::CUIXulStream::add_processing(const PIs& attr) noexcept {
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
                    wnd->LoadCSSFile({ href.begin(), href.end() });
                }
            }
        }
    }
    /// <summary>
    /// Begins the element.
    /// </summary>
    /// <param name="view">The view.</param>
    /// <returns></returns>
    void CUIControlControl::CUIXulStream::begin_element(const StrPair view) noexcept {
        // window节点
        if (view == "window"_pair) {
            // 写入数据
            this->stack_top().user_ptr = &m_root;
        }
        // script节点
        else if (view == "script"_pair) {
            // 检查脚本支持

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
            if (!ctrl || ctrl->IsCtorFailed()) {
                delete ctrl;
                this->error = view;
                LUIDebug(Error) 
                    << "failed to create control: " 
                    << U8View{ view.begin(), view.end() }
                    << endl;
                std::longjmp(this->env, 1);
            }
            // 测试数据
            //ctrl->SetFlex_test(1.f);
            // 写入栈顶
            this->stack_top().user_ptr = ctrl;
        }
    }
    /// <summary>
    /// Adds the attribute.
    /// </summary>
    /// <param name="attr">The attribute.</param>
    /// <returns></returns>
    void CUIControlControl::CUIXulStream::add_attribute(const ATTRs& attr) noexcept {
        const auto& top = this->stack_top();
        const auto ctrl = static_cast<UIControl*>(top.user_ptr);
        // 检查有效性
        if (ctrl) {
            // 计算键HSH
            const auto key = LongUI::BKDRHash(attr.key.a, attr.key.b);
            // 添加属性
            ctrl->add_attribute(key, { attr.value.a, attr.value.b });
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
bool LongUI::CUIControlControl::MakeXUL(UIControl& ctrl, const char* xul) noexcept {
    // 解析器, 因为没有使用异常, 所以在这里使用长跳转模拟异常
    CUIXulStream stream{ ctrl };
#ifndef NDEBUG
    // 错误信息
    const auto& error = stream.error;
#endif
    // 设置长跳转环境   顺便, setjmp是宏, 而非函数
    const auto code = setjmp(stream.env);
    // 先去掉所有子控件
    while (ctrl.GetCount()) (*ctrl.begin()).SetParent(nullptr);
    // 错误代码 
    bool rv = false;
    // 检查错误
    switch (code)
    {
    case 0:
        // 然后解析XML字符串
        const auto result = stream.Load(xul);
        // 解析错误
        if (!result.IsOk())
            std::longjmp(stream.env, result.code + 1);
        rv = true;
        break;
    default:
        // 基本错误处理
        assert(!"TODO");
        break;
    }
    // 返回处理
    return rv;
}

