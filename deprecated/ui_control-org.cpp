#include "uiControl.h"
#include <functional>
#include <algorithm>
#include <cassert>
#include <new>

//0#undef max
//#undef min

// TODO: 调整"优化"过的 bool c = a && b; 语句
// TODO: 优化 bool c = a && b; 语句

/*
CSS支持:
  0. 逗号选择器 button, radio

  1. 基本选择器
    A. 元素选择器 button{  }
    B. 类名选择器 .sysbtn {  }
    C.  ID 选择器 #btn1 {  }

  2. 组合选择器
    A. 过于复杂(前项状态也会影响后项), 不打算支持

  3. 伪类选择器
    A. 基本伪类选择器, 参考: PseudoClasses
      a. :active
      b. :hover
      c. :focus
      d. :empty
      e. :checked
      f. :indeterminate
      g. :disabled = :not(:enabled)
      h. :enabled = :not(:disabled)
      i. :-ui-selected
    B. NOT 选择器, 限制: 只能放置A中的伪类 :not(:checked)

  4. 伪元素选择器
    A. 控件所支持的伪元素 scrollbar::thumb {  }
*/

/*
布局支持:
  1. 盒子布局, box. 调整orient属性或者使用继承元素:
    A. vbox
    B. hbox
  2. 网格布局, grid
  3. 栈式布局, stack
  4. 分页布局, deck
  E. 分割盒子, splitter 只能放置在box中(包括继承元素)

  X. 继承元素
    A. groupbox < box
    B. viewport/window < box
    C. tabpanels/tabbox < deck
    D. tabpanel/tabs < box

*/
/*
容器一览:

<box orient="horizontal">
  <box orient="vertical">
    <xxx/>
  </box>
</box>

<grid>
  <columns>
    <column/>
    <column/>
  </columns>
  <rows>
    <row><xxx/><xxx/></row>
    <row><xxx/><xxx/></row>
    <row><xxx/><xxx/></row>
  </rows>
</grid>

<stack>
  <xxx/>
  <xxx/>
</stack>

<deck>
  <xxx/>
  <xxx/>
</deck>
*/


// Qt
#include <QPainter>

#ifndef NDEBUG
#include <QDebug>
#endif


// UI
#include "util/pod_hash.h"
#include "uiSSValue.h"


// GUID
namespace UI {
    // {5ED32920-2F66-48ED-900E-F78EDDB978DB}
    const GUID UIControl::s_guid = { 0x5ed32920, 0x2f66, 0x48ed,{ 0x90, 0xe, 0xf7, 0x8e, 0xdd, 0xb9, 0x78, 0xdb } };
}

// UI::Private
namespace UI { namespace Private {
    // default name
    static const char* DEFAULT_NAME = "";
    // get renderer
    auto renderer() noexcept->QPainter&;
    // create ui
    void*create_ui(void* ptr);
    // create ui
    void draw_ui(void*);
    // resize ui
    void resize_ui(void*, int, int);
    // mmove ui
    void mmove_ui(void*, int, int);
    // click ui
    void click_ui(void*, int, int, bool down);
    // wheel ui
    void wheel_ui(void*, int, int, bool, float);
    // set root
    void set_root(UIControl* ptr);
    // update window
    void update_window(void* ptr);
}}

// type cast
namespace UI {
    QRectF& qt(QRectF& qr, const RectF& rc) {
        return qr = { rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top };
    }
}

/// <summary>
/// Creates the UI.
/// </summary>
/// <returns></returns>
_declspec(noinline)
void* UI::Private::create_ui(void* ptr) {
#ifndef NDEBUG
    auto z = new(std::nothrow) UI::UIViewport;
    //auto z = new(std::nothrow) UI::UIVBoxLayout;
    set_root(z); z->user_data = reinterpret_cast<uintptr_t>(ptr);
    z->Init(); z->SetBgColor({ RGBA_Black });
    z->name_dbg = "root";
    auto add_spacer = [](UI::UIControl* p) {
        auto b = new(std::nothrow) UI::UISpacer{ p }; 
        b->Init();
    };
    auto add_ctrl = [](UI::UIControl* p, uint32_t color, const char* name) ->UI::UIControl* {
        auto c = new(std::nothrow) UI::UIControl{ p };
        c->Init(); c->SetBgColor({ color }); c->name_dbg = name;
        return c;
    };
    {
        auto vbox = new(std::nothrow) UI::UIVBoxLayout{ z };
        vbox->name_dbg = "root-vbox";
        auto hbox1 = new(std::nothrow) UI::UIHBoxLayout{ vbox };
        hbox1->name_dbg = "vbox-hbox1";
        // hbox1->SpecifyMinSize({ 640, 0 });
        {
            //hbox1->name_dbg = "hbox1";
            auto a = add_ctrl(hbox1, RGBA_Green, "hbox1-a");
            auto b = add_ctrl(hbox1, RGBA_Cyan,  "hbox1-b");
            auto c = add_ctrl(hbox1, RGBA_Blue,  "hbox1-c");

            a->SpecifyMinSize({ 50, 30 });
            b->SpecifyMinSize({ 100, 30 });
            c->SpecifyMinSize({ 200, 1000 });
            c->SetFlex_test(1.f);
        }
        //auto hbox2 = new(std::nothrow) UI::UIHBoxLayout{ vbox };
        //auto hbox3 = new(std::nothrow) UI::UIHBoxLayout{ vbox };

        
        vbox->Init(); 
        hbox1->Init(); 
        //hbox2->Init(); hbox2->color = 0x00ffff;
        //hbox3->Init();

    }
    return z;
#endif
    return nullptr;
}

void func() {

}

/// <summary>
/// Draws the UI.
/// </summary>
/// <param name="ptr">The PTR.</param>
void UI::Private::draw_ui(void* ptr) {
    assert(ptr);
    auto ctrl = static_cast<UIControl*>(ptr);
    UI::L::UpdateControlInList();
    UI::L::RecursiveRender(*ctrl);
}

/// <summary>
/// Resizes the UI.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="w">The w.</param>
/// <param name="h">The h.</param>
void UI::Private::resize_ui(void* ptr, int w, int h) {
    assert(ptr);
    auto ctrl = static_cast<UIControl*>(ptr);
    const float ww = static_cast<float>(w);
    const float hh = static_cast<float>(h);
    ctrl->Resize({ ww, hh });
}

/// <summary>
/// Mmoves the UI.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
void UI::Private::mmove_ui(void * ptr, int x, int y) {
    assert(ptr);
    auto ctrl = static_cast<UIControl*>(ptr);
    const float xx = static_cast<float>(x);
    const float yy = static_cast<float>(y);
    ctrl->DoMouseEvent({ MouseEvent::Event_MouseMove , 0.f, xx, yy });
}

/// <summary>
/// Clicks the UI.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <param name="down">if set to <c>true</c> [down].</param>
void UI::Private::click_ui(void* ptr, int x, int y, bool down) {
    assert(ptr);
    auto ctrl = static_cast<UIControl*>(ptr);
    const float xx = static_cast<float>(x);
    const float yy = static_cast<float>(y);
    ctrl->DoMouseEvent({ 
        down ? MouseEvent::Event_LButtonDown : 
        MouseEvent::Event_LButtonUp,
        0.f, xx, yy 
    });
}

/// <summary>
/// Wheels the UI.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <param name="i">if set to <c>true</c> [i].</param>
/// <param name="d">The d.</param>
void UI::Private::wheel_ui(void* ptr, int x, int y, bool i, float d) {
    assert(ptr);
    auto ctrl = static_cast<UIControl*>(ptr);
    const float xx = static_cast<float>(x);
    const float yy = static_cast<float>(y);
    ctrl->DoMouseEvent({
        !i ? MouseEvent::Event_MouseWheelV :
        MouseEvent::Event_MouseWheelH,
        d, xx, yy
    });
}

/// <summary>
/// Swaps the ab.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void UI::Node::SwapAB(Node& a, Node& b) noexcept {
    // 必须是1-A-B-2
    assert(a.next == &b && "must be 'A-B' near node");
    assert(b.prev == &a && "bad linked");
    // --> 1 - B - A - 2
    const auto node1 = a.prev;
    const auto node2 = b.next;
    // NEXT
    node1->next = &b;
    node2->prev = &a;
    b.next = &a;
    a.next = node2;
    a.prev = &b;
    b.prev = node1;
}

/// <summary>
/// Swaps the node.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void UI::Node::SwapNode(Node& a, Node& b) noexcept {
    assert(a != b && "a == b");
    // A = B
    if (a == b) return;
    // 1 - A - B - 2
    if (a.next == &b) return Node::SwapAB(a, b);
    // 1 - B - A - 2
    if (a.prev == &b) return Node::SwapAB(b, a);
    // 1 - A - 2        3 - B - 4
    const auto node1 = a.prev;
    const auto node2 = a.next;
    const auto node3 = b.prev;
    const auto node4 = b.next;
    // --> 1 - B - 2    3 - A - 4
    // NEXT
    node1->next = &b;
    b.next      = node2;
    node3->next = &a;
    a.next      = node4;
    // PREV
    node4->prev = &a;
    a.prev      = node3;
    node2->prev = &b;
    b.prev      = node1;
}

/// <summary>
/// 
/// </summary>
namespace UI {
    /// <summary>
    /// private function/data to UIControl
    /// </summary>
    class PrivateControl {
    public:
        // Pseudo classes hover
        struct PseudoClassesHover {
            static void Set(UIControl& ctrl, bool v) noexcept { ctrl.m_oStyle.state.hover = v; }
            static bool Get(const UIControl& ctrl) noexcept { return ctrl.m_oStyle.state.hover; }
        };
        // Pseudo classes active
        struct PseudoClassesActive {
            static void Set(UIControl& ctrl, bool v) noexcept { ctrl.m_oStyle.state.active = v; }
            static bool Get(const UIControl& ctrl) noexcept { return ctrl.m_oStyle.state.active; }
        };
        // ensure bgcolor renderer
        static auto EnsureBgcRenderer(UIControl&) noexcept ->Renderer::Background*;
        // eecursive refresh min size
        static void RefreshMinSize(UIControl& ctrl) noexcept;
        // update world
        static void UpdateWorld(UIControl& ctrl) noexcept;
        // update children level
        static void UpdateLevel(UIControl& ctrl) noexcept;
        // do mouse enter event
        static auto DoMouseEnter(UIControl& ctrl, const Point2F& pos) noexcept ->EventAccept;
        // do mouse leave event
        static auto DoMouseLeave(UIControl& ctrl, const Point2F& pos) noexcept->EventAccept;
        // set state
        template<typename State> static void SetState(UIControl&) noexcept;
        // set state
        template<typename State> static void ClearState(UIControl&) noexcept;
        // ----------- INLINE ZONE ----------------
        // add gui event listener
        static bool AddGuiEL(UIControl& ctrl, uintptr_t ownid, GuiEvent e, GuiEventListener&& l) noexcept {
            return ctrl.set_gui_event_listener(e, ownid, std::move(l));
        }
        // remove gui event listener
        static void RemoveGuiEL(UIControl& ctrl, uintptr_t ownid, GuiEvent e) noexcept {
            ctrl.set_gui_event_listener(e, ownid, std::move(*(GuiEventListener*)nullptr));
        }

        static void test();
    };
}

// ui::renderer namespace
namespace UI { namespace Renderer {
    // background color
    class Background : public CUIObject {
    public:
        // ctor
        Background() noexcept;
        // dtor
        ~Background() noexcept;
        // move ctor
        Background(Background&&) = delete;
        // copy ctor
        Background(const Background&) = delete;
        // render color
        void RenderColor(const UI::Box& box) const noexcept;
        // render image
        void RenderImage(const UI::Box& box) const noexcept;
    public:
        // set image with id
        void SetImage(uint32_t id) noexcept;
        // set color, need repaint
        void SetColor(const ColorF& c) noexcept { m_color = c; }
        // set clip, need repaint
        void SetClip(AttributeBox clip) noexcept { m_clip = clip; }
        // set repeat
        void SetRepeat(AttributeRepeat ar) noexcept { m_repeat = ar; }
        // set origin
        void SetOrigin(AttributeBox origin) noexcept { m_origin = origin; }
        // set attachment, need repaint
        void SetAttachment(AttributeAttachment aa) noexcept { m_attachment = aa; }
    private:
        // ------------- GPU-RES ------------
    private:
        // ------------- CPU-RES ------------
        // background-color
        ColorF              m_color;
        // background-image
        uint32_t            m_idImage = 0;
        // background-clip
        AttributeBox        m_clip = Box_BorderBox;
        // background-repeat
        AttributeRepeat     m_repeat = Repeat_Repeat;
        // background-origin 
        AttributeBox        m_origin = Box_PaddingBox;
        // background-attachment
        AttributeAttachment m_attachment = Attachment_Scroll;
    };
}}

/// <summary>
/// Initializes a new instance of the <see cref="Background"/> class.
/// </summary>
UI::Renderer::Background::Background() noexcept {
    m_color = ColorF::FromRGBA_CT<RGBA_Transparent>();
}

/// <summary>
/// Finalizes an instance of the <see cref="Background"/> class.
/// </summary>
/// <returns></returns>
UI::Renderer::Background::~Background() noexcept {

}

/// <summary>
/// Sets the image.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void UI::Renderer::Background::SetImage(uint32_t id) noexcept {
    id;
    assert(!"NOT IMPL");
}

/// <summary>
/// Renders the color.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void UI::Renderer::Background::RenderColor(const UI::Box& box) const noexcept {
    // 渲染背景颜色
    if (m_color.a == 0.0) return;
    // 获取渲染矩阵
    RectF rect;
    switch (m_clip)
    {
    default: // XXX: 去掉注释 [[fallthrough]]
    case UI::Box_BorderBox:  box.GetBorderEdge(rect);  break;
    case UI::Box_PaddingBox: box.GetPaddingEdge(rect); break;
    case UI::Box_ContentBox: box.GetContentEdge(rect); break;
    }
    // 执行渲染
    auto& painter = UI::Private::renderer();
    QColor color;
    color.setAlphaF (m_color.a);
    color.setRedF   (m_color.r);
    color.setGreenF (m_color.g);
    color.setBlueF  (m_color.b);
    painter.fillRect(QRectF{
        rect.left, rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top
    }, color);
}


/// <summary>
/// Renders the image.
/// </summary>
/// <param name="box">The box.</param>
/// <returns></returns>
void UI::Renderer::Background::RenderImage(const UI::Box& box) const noexcept {
    // 渲染背景图片
    if (!m_idImage) return;
    box;
    assert(!"NOT IMPL");
    // 背景可能是:
    //    1. 图片
    //    2. 
}

PCN_NOINLINE
/// <summary>
/// Ensures the BG-Color renderer.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto UI::PrivateControl::EnsureBgcRenderer(
    UIControl& ctrl) noexcept ->Renderer::Background* {
    // ensure 函数尽量保证不内联
    if (ctrl.m_pBgRender)
        return ctrl.m_pBgRender;
    else
        return ctrl.m_pBgRender = new(std::nothrow) Renderer::Background;
}

/// <summary>
/// Updates the level.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void UI::PrivateControl::UpdateLevel(UIControl& ctrl) noexcept {
    for (auto& child : ctrl) {
        child.m_state.level = ctrl.m_state.level + 1;
        assert(child.GetLevel() < 100ui8 && "tree too deep");
        PrivateControl::UpdateLevel(child);
    }
}

/// <summary>
/// Recursives the minimum size of the refresh.
/// </summary>
/// <returns></returns>
void UI::PrivateControl::RefreshMinSize(UIControl& ctrl) noexcept {
    for (auto& child : ctrl) RefreshMinSize(child);
    // 刷新大小
    ctrl.DoEvent(&ctrl, { NoticeEvent::Event_RefreshMinSize });
    // 指定大小作比较
    auto& s = ctrl.m_oStyle;
    s.minsize.width = std::max(s.minsize.width, s.minsize_sp.width);
    s.minsize.height = std::max(s.minsize.height, s.minsize_sp.height);
}

/// <summary>
/// Updates the world.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void UI::PrivateControl::UpdateWorld(UIControl& ctrl) noexcept {
    // 需要刷新
    if (ctrl.m_state.world_changed) {
        Matrix3X2F matrix;
        // TODO: 根节点
        if (ctrl.IsTopLevel()) {

        }
        // 子节点
        else {
            auto parent = ctrl.GetParent();
            matrix = parent->GetWorld();
            matrix._31 += ctrl.GetPos().x;
            matrix._32 += ctrl.GetPos().y;
            // 固定位置?
            if (ctrl.m_state.attachment == Attachment_Scroll) {
                matrix._31 -= parent->m_ptChildOffset.x;
                matrix._32 -= parent->m_ptChildOffset.y;
            }
            ctrl.m_mtWorld = matrix;
        }
        ctrl.m_state.world_changed = false;
        auto& box = ctrl.m_oBox;
        auto ctrl_rect = ctrl.GetBox().GetBorderEdge();
        ctrl.MapToWindow(ctrl_rect);
        // 检查父控件
        if (!ctrl.IsTopLevel()) {
            auto parent = ctrl.m_pParent;
            auto ctn = parent->GetBox().visible;
            ctrl_rect.top = std::max(ctn.top, ctrl_rect.top);
            ctrl_rect.left = std::max(ctn.left, ctrl_rect.left);
            ctrl_rect.right = std::min(ctn.right, ctrl_rect.right);
            ctrl_rect.bottom = std::min(ctn.bottom, ctrl_rect.bottom);
        }
        box.visible = ctrl_rect;
        // 强行刷新子对象
        for (auto& child : ctrl) {
            child.m_state.world_changed = true;
            PrivateControl::UpdateWorld(child);
        }
        return;
    }
    // 刷新子对象
    for (auto& child : ctrl) PrivateControl::UpdateWorld(child);
}

/// <summary>
/// Does the mouse enter.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto UI::PrivateControl::DoMouseEnter(
    UIControl& ctrl, const Point2F& pos) noexcept -> EventAccept {
    return ctrl.DoMouseEvent({ MouseEvent::Event_MouseEnter, 0.f, pos.x, pos.y });
}

/// <summary>
/// Does the mouse enter.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto UI::PrivateControl::DoMouseLeave(
    UIControl& ctrl, const Point2F& pos) noexcept -> EventAccept {
    return ctrl.DoMouseEvent({ MouseEvent::Event_MouseLeave, 0.f, pos.x, pos.y });
}

/// <summary>
/// Sets the state.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
template<typename State>
void UI::PrivateControl::SetState(UIControl& ctrl) noexcept {
    assert(!State::Get(ctrl) && "must be false before set");
    State::Set(ctrl, true);
}

/// <summary>
/// Clears the state.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
template<typename State>
void UI::PrivateControl::ClearState(UIControl& ctrl) noexcept {
    assert(State::Get(ctrl) && "must be true before set");
    State::Set(ctrl, false);
}

/// <summary>
/// string to gui event id
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto UI::CUIEventHost::strtoe(const char*) noexcept -> GuiEvent {
    assert(!"NOT IMPL");
    return GuiEvent(-1);
}

/// <summary>
/// Adds the GUI event listener.
/// </summary>
/// <param name="ownid">The ownid.</param>
/// <param name="e">The e.</param>
/// <param name="listener">The listener.</param>
/// <returns></returns>
bool UI::CUIEventHost::add_gui_event_listener(
    uintptr_t ownid, 
    GuiEvent e, 
    GuiEventListener&& listener) noexcept {
    auto ctrl = static_cast<UIControl*>(this);
    return PrivateControl::AddGuiEL(*ctrl, ownid, e, std::move(listener));
}

/// <summary>
/// Removes the GUI event listener.
/// </summary>
/// <param name="ownid">The ownid.</param>
/// <param name="">The .</param>
/// <returns></returns>
void UI::CUIEventHost::RemoveGuiEventListener(uintptr_t ownid, GuiEvent e) noexcept {
    auto ctrl = static_cast<UIControl*>(this);
    return PrivateControl::RemoveGuiEL(*ctrl, ownid, e);
}

PCN_NOINLINE
/// <summary>
/// Sets the color of the bg.
/// </summary>
/// <param name="color">The color.</param>
/// <returns></returns>
void UI::CUIStyleValue::SetBgColor(RGBA color) noexcept {
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = PrivateControl::EnsureBgcRenderer(*ctrl)) {
        ColorF cf; ColorF::FromRGBA_RT(cf, color);
        r->SetColor(cf);
        ctrl->Repaint();
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the bg image.
/// </summary>
/// <param name="id">The identifier.</param>
/// <returns></returns>
void UI::CUIStyleValue::SetBgImage(uint32_t id) noexcept {
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = PrivateControl::EnsureBgcRenderer(*ctrl)) {
        r->SetImage(id);
        ctrl->Repaint();
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the bg clip.
/// </summary>
/// <param name="clip">The clip.</param>
/// <returns></returns>
void UI::CUIStyleValue::SetBgClip(AttributeBox clip) noexcept {
    assert(clip < AttributeBox::BOX_COUNT && "out of range");
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = PrivateControl::EnsureBgcRenderer(*ctrl)) {
        r->SetClip(clip);
        ctrl->Repaint();
    }
}


PCN_NOINLINE
/// <summary>
/// Sets the bg origin.
/// </summary>
/// <param name="ar">The ar.</param>
/// <returns></returns>
void UI::CUIStyleValue::SetBgOrigin(AttributeBox ab) noexcept {
    assert(ab < AttributeBox::BOX_COUNT && "out of range");
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = PrivateControl::EnsureBgcRenderer(*ctrl)) {
        r->SetOrigin(ab);
        ctrl->Repaint();
    }
}


PCN_NOINLINE
/// <summary>
/// Sets the bg attachment.
/// </summary>
/// <param name="aa">The aa.</param>
/// <returns></returns>
void UI::CUIStyleValue::SetBgAttachment(AttributeAttachment aa) noexcept {
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = PrivateControl::EnsureBgcRenderer(*ctrl)) {
        r->SetAttachment(aa);
        ctrl->Repaint();
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the bg repeat.
/// </summary>
/// <param name="ar">The ar.</param>
/// <returns></returns>
void UI::CUIStyleValue::SetBgRepeat(AttributeRepeat ar) noexcept {
    assert(ar < AttributeRepeat::REPEAT_COUNT && "out of range");
    auto ctrl = static_cast<UIControl*>(this);
    if (auto r = PrivateControl::EnsureBgcRenderer(*ctrl)) {
        r->SetRepeat(ar);
        ctrl->Repaint();
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="UIControl"/> class.
/// </summary>
UI::UIControl::UIControl(UIControl* parent) noexcept : 
m_pParent(nullptr), m_pId(Private::DEFAULT_NAME) {
    Node::next = nullptr;
    Node::prev = nullptr;
    m_ptChildOffset = { 0, 0 };
    m_mtWorld = { 1, 0, 0, 1, 0, 0 };
    m_oBox.Init();
    m_state.Init();
    // 初始化一般数据
    m_oHead = { nullptr, &m_oTail };
    m_oTail = { &m_oHead, nullptr };
#ifdef DISABLABLE_OBJECT
    m_oHeadObj = { nullptr, &m_oTailObj };
    m_oTailObj = { &m_oHeadObj, nullptr };
#endif
    // 添加到父节点的子节点链表中
    if (parent) parent->add_child(*this);
    // 更新世界
    m_state.world_changed = true;
}

/// <summary>
/// Applies the value.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void UI::UIControl::ApplyValue(const SSValue& value) noexcept {
    switch (value.type)
    {
    case UI::ValueType::Type_Unknown:
        break;
    case UI::ValueType::Type_PositionCursor:
        break;
    case UI::ValueType::Type_PositionLeft:
        break;
    case UI::ValueType::Type_PositionRight:
        break;
    case UI::ValueType::Type_PositionOverflow:
        break;
    case UI::ValueType::Type_PositionZindex:
        break;
    case UI::ValueType::Type_MarginTop:
        break;
    case UI::ValueType::Type_MarginRight:
        break;
    case UI::ValueType::Type_MarginBottom:
        break;
    case UI::ValueType::Type_MarginLeft:
        break;
    case UI::ValueType::Type_PaddingTop:
        break;
    case UI::ValueType::Type_PaddingRight:
        break;
    case UI::ValueType::Type_PaddingBottom:
        break;
    case UI::ValueType::Type_PaddingLeft:
        break;
    case UI::ValueType::Type_BorderTop:
        break;
    case UI::ValueType::Type_BorderRight:
        break;
    case UI::ValueType::Type_BorderBottom:
        break;
    case UI::ValueType::Type_BorderLeft:
        break;
    case UI::ValueType::Type_BorderTopColor:
        break;
    case UI::ValueType::Type_BorderRightColor:
        break;
    case UI::ValueType::Type_BorderBottomColor:
        break;
    case UI::ValueType::Type_BorderLeftColor:
        break;
    case UI::ValueType::Type_BorderTopWidth:
        break;
    case UI::ValueType::Type_BorderRightWidth:
        break;
    case UI::ValueType::Type_BorderBottomWidth:
        break;
    case UI::ValueType::Type_BorderLeftWidth:
        break;
    case UI::ValueType::Type_BorderTopStyle:
        break;
    case UI::ValueType::Type_BorderRightStyle:
        break;
    case UI::ValueType::Type_BorderBottomStyle:
        break;
    case UI::ValueType::Type_BorderLeftStyle:
        break;
    case UI::ValueType::Type_BorderTopLeftRadius:
        break;
    case UI::ValueType::Type_BorderTopRightRadius:
        break;
    case UI::ValueType::Type_BorderBottomLeftRadius:
        break;
    case UI::ValueType::Type_BorderBottomRightRadius:
        break;
    case UI::ValueType::Type_BorderImageSource:
        break;
    case UI::ValueType::Type_BackgroundColor:
        return this->SetBgColor({ value.u32 });
    case UI::ValueType::Type_BackgroundImage:
        return this->SetBgImage(value.u32);
    case UI::ValueType::Type_BackgroundAttachment:
        return this->SetBgAttachment(size_cast<AttributeAttachment>(value.boolean));
    case UI::ValueType::Type_BackgroundRepeat:
        return this->SetBgRepeat(size_cast<AttributeRepeat>(value.byte));
    case UI::ValueType::Type_BackgroundClip:
        return this->SetBgClip(size_cast<AttributeBox>(value.byte));
    case UI::ValueType::Type_BackgroundOrigin:
        return this->SetBgOrigin(size_cast<AttributeBox>(value.byte));
    default:
        break;
    }
    assert(!"NOT IMPL");
}

/// <summary>
/// Begins the render.
/// </summary>
/// <returns></returns>
void UI::UIControl::begin_render() const noexcept {
    // 做一些渲染前的准备
    auto& painter = UI::Private::renderer();
    QMatrix matrix{
        m_mtWorld._11, m_mtWorld._12,
        m_mtWorld._21, m_mtWorld._22,
        m_mtWorld._31, m_mtWorld._32,
    };
    painter.setMatrix(matrix);
}

/// <summary>
/// Draws the color of the background.
/// </summary>
/// <returns></returns>
void UI::UIControl::draw_bkcolor() const noexcept {
    if (m_pBgRender) {
        m_pBgRender->RenderColor(this->GetBox());
    }
}

/// <summary>
/// Draws the background image.
/// </summary>
/// <returns></returns>
void UI::UIControl::draw_bkimage() const noexcept {

}

/// <summary>
/// Draws the border.
/// </summary>
/// <returns></returns>
void UI::UIControl::draw_border() const noexcept {

}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void UI::UIControl::Render() const noexcept {
    assert(m_state.inited_dbg && "must init control first");
    this->begin_render();
    this->draw_bkcolor();
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto UI::UIControl::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
    sender;
    // ---------------- 刷新最小大小
    auto refresh_min = [this]() noexcept {
        // 该方法仅提供一个最基本的计算: 内容区域为0, 仅仅计算边界
        // 因为一般控件几乎不可能存在子控件
        const RectF nonc = this->GetBox().GetNonContect();
        // 更新值
        m_oStyle.minsize = { nonc.left + nonc.right, nonc.top + nonc.bottom };
    };

    // ---------------- 事件处理分支
    switch (e.nevent)
    {
    case UI::NoticeEvent::Event_RefreshMinSize:
        refresh_min();
        return UI::Event_Accept;
    }
    return UI::Event_Ignore;
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto UI::UIControl::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    using Pc = PrivateControl;
    EventAccept s = EventAccept::Event_Ignore;
    // 自己处理消息
    switch (e.type)
    {
    case UI::MouseEvent::Event_MouseEnter:
        Pc::SetState<Pc::PseudoClassesHover>(*this);
#ifndef NDEBUG
        qDebug() << this << "Enter: " << name_dbg;
#endif
        // 截断ENTER LEAVE消息
        return Event_Accept;
    case UI::MouseEvent::Event_MouseLeave:
        Pc::ClearState<Pc::PseudoClassesHover>(*this);
        if (m_pHovered) {
            PrivateControl::DoMouseLeave(*m_pHovered, { e.px, e.py });
            m_pHovered = nullptr;
        }
        // 截断ENTER LEAVE消息
#ifndef NDEBUG
        qDebug() << this << "Leave: " << name_dbg;
#endif
        return Event_Accept;
    case UI::MouseEvent::Event_MouseMove:
    {
        const auto child = this->FindChild({ e.px, e.py });
        const auto hover = m_pHovered;
        if (hover != child) {
            if (hover) PrivateControl::DoMouseLeave(*hover, { e.px, e.py });
            if (child) PrivateControl::DoMouseEnter(*child, { e.px, e.py });
            m_pHovered = child;
        }
        if (child) return child->DoMouseEvent(e);
        // 处理MOUSE消息
        return Event_Accept;
    }
    case UI::MouseEvent::Event_LButtonDown:
        if (this->IsEnabled()) {
            s = Event_Accept;
            Pc::SetState<Pc::PseudoClassesActive>(*this);
        }
        break;
    case UI::MouseEvent::Event_LButtonUp:
        if (this->IsEnabled()) {
            s = Event_Accept;
            Pc::ClearState<Pc::PseudoClassesActive>(*this);
        }
        break;
    }
    // 子控件处理消息
    //if (this->GetCount()) 
    if (auto child = this->FindChild({ e.px, e.py })) {
        s = child->DoMouseEvent(e);
    }
    return s;
}

/*/// <summary>
/// Renders the children.
/// </summary>
/// <returns></returns>
void UI::UIControl::render_children() const noexcept {
    auto& r = UI::Private::renderer();
    for (auto& child : *this) {
        r.save();
        child.Render();
        r.restore();
    }
}*/

/// <summary>
/// Finalizes an instance of the <see cref="UIControl"/> class.
/// </summary>
/// <returns></returns>
UI::UIControl::~UIControl() noexcept {
    // 移除高层引用
    L::ControlDisattached(*this);
    // 清理子节点
    while (begin() != end()) delete begin();
    // 清理渲染器
    delete m_pBgRender;
    // 清除父节点中的自己
    if (m_pParent) {
        m_pParent->remove_child(*this);
#ifndef NDEBUG
        m_pParent = nullptr;
        m_pParent++;
#endif
    }
#ifdef DISABLABLE_OBJECT
    for (auto itr = m_oHeadObj.next; itr != &m_oTailObj; ) {
        const auto obj = static_cast<NodeDisposeble*>(itr);
        itr = itr->next;
        obj->Dispose();
    }
#endif
    // 释放资源
    assert(m_pId == Private::DEFAULT_NAME);
}

/// <summary>
/// Sets the parent.
/// </summary>
/// <param name="parent">The parent.</param>
/// <returns></returns>
void UI::UIControl::SetParent(UIControl& parent) noexcept {
    assert(this && "bad this ptr");
    parent.add_child(*this);
}


/// <summary>
/// Swaps the children.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
void UI::UIControl::SwapChildren(UIControl& a, UIControl& b) noexcept {
    assert(this && "bad this pointer");
    assert(a.GetParent() == this && "not child for this");
    assert(b.GetParent() == this && "not child for this");
    if (a == b) return;
    m_state.child_i_changed = true;
    L::MarkMinSizeChanged(*this);
    Node::SwapNode(a, b);
    this->NeedRelayout();
}

/// <summary>
/// Finds the child.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto UI::UIControl::FindChild(const Point2F & pos) noexcept -> UIControl * {
    for (auto& ctrl : *this) {
        if (ctrl.IsVisible() && UI::IsInclude(ctrl.GetBox().visible, pos)) {
            return &ctrl;
        }
    }
    return nullptr;
}

/// <summary>
/// Sets the position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void UI::UIControl::SetPos(const Point2F& pos) noexcept {
    if (IsSameInGuiLevel(m_oBox.pos, pos)) return;
    m_state.world_changed = true;
    m_oBox.pos = pos;
    this->NeedUpdate();
}

/// <summary>
/// Sets the visible.
/// </summary>
/// <param name="visible">if set to <c>true</c> [visible].</param>
/// <returns></returns>
void UI::UIControl::SetVisible(bool visible) noexcept {
    if (this->IsVisible() != visible) {
        m_oStyle.state.visible = visible;
        // TODO: 顶级控件?
        if (this->IsTopLevel()) {
        }
        else {
            m_pParent->NeedRelayout();
        }
    }
}

/// <summary>
/// Invalidates the specified rect.
/// </summary>
/// <param name="rect">The rect.</param>
void UI::UIControl::invalidate(const RectF* rect) noexcept {
    auto real = rect ? *rect : m_oBox.GetMarginEdge();
    L::InvalidateControl(*this, real);
}

#ifdef DISABLABLE_OBJECT

/// <summary>
/// Adds the disable object.
/// </summary>
/// <param name="obj">The object.</param>
/// <returns></returns>
void UI::UIControl::AddDisableObject(NodeDisposeble& obj) noexcept {
    assert(this && "bad this ptr");
    // 连接前后节点
    m_oTailObj.prev->next = &obj;
    obj.prev = m_oTailObj.prev;
    obj.next = &m_oTailObj;
    m_oTailObj.prev = &obj;
}


/// <summary>
/// Removes the disable object.
/// </summary>
/// <param name="obj">The object.</param>
/// <returns></returns>
void UI::UIControl::RemoveDisableObject(NodeDisposeble& obj) noexcept {
#ifndef NDEBUG
    const auto begin = static_cast<NodeDisposeble*>(m_oHeadObj.next);
    const auto end = static_cast<NodeDisposeble*>(&m_oTailObj);
    const auto itr = std::find(
        Node::Iterator<NodeDisposeble>{begin},
        Node::Iterator<NodeDisposeble>{end},
        obj
    );
    assert(itr != Node::Iterator<NodeDisposeble>{end});
#endif
    // 连接前后节点
    obj.prev->next = obj.next;
    obj.next->prev = obj.prev;
}
#endif

/// <summary>
/// Forces the update children.
/// </summary>
/// <returns></returns>
void UI::UIControl::ForceUpdateChildren() noexcept {
    for (auto& child : *this) child.Update();
}

/// <summary>
/// Removes the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void UI::UIControl::remove_child(UIControl& child) noexcept {
    assert(child.m_pParent == this && "no child for this");
    // 检查数量是否一致
#ifndef NDEBUG
    assert(m_cChildrenCount && "bad child");
    const auto c = std::distance(begin(), end());
    assert(m_cChildrenCount == static_cast<uint32_t>(c));
#endif
    // 修改动态样式
    // 移除直接引用
    if (m_pHovered == &child) m_pHovered = nullptr;
    --m_cChildrenCount;
    // 连接前后节点
    child.prev->next = child.next;
    child.next->prev = child.prev;
#ifndef NDEBUG
    if (!m_cChildrenCount) {
        assert(m_oHead.next == &m_oTail);
        assert(m_oTail.prev == &m_oHead);
    }
#endif
    // 要求刷新
    m_state.child_i_changed = true;
    L::MarkMinSizeChanged(*this);
    this->NeedUpdate();
}

/// <summary>
/// Adds the direct child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void UI::UIControl::add_direct_child(UIControl& child) noexcept {
    assert(child.m_pParent == nullptr && "parent must null befor set direct parent");
    child.m_state.directly_managed = true;
    this->add_child(child);
    // 放置在第一个
    //if (this->GetCount() > 1) {
        //this->SwapChildren(*this->begin(), child);
    //}
    child.NeedUpdate();
}

/// <summary>
/// Determines whether [is excluded from the layout].
/// </summary>
/// <returns></returns>
bool UI::UIControl::IsVaildInLayout() const noexcept {
    // 排除: 不可直视
    const bool a = this->IsVisible();
    // 排除: 直接管理
    const bool b = !m_state.directly_managed;
    // 排除: 固定定位
    const bool c = m_state.attachment == Attachment_Scroll;
    // 排除: 相对定位

    // 排除: 绝对定位

    // AND
    return a & b & c;
}

/// <summary>
/// Adds the style class.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
void UI::UIControl::AddStyleClass(PodStringView<char> pair) noexcept {
    // 可能就几条甚至不超过一条, 直接插入即可
    const auto style_class = L::GetUniqueStyleClass(pair);
    const auto b = m_classesStyle.begin();
    const auto e = m_classesStyle.end();
    if (std::find(b, e, style_class) == e) 
        m_classesStyle.push_back(style_class);
}

/// <summary>
/// Removes the style class.
/// </summary>
/// <param name="pair">The pair.</param>
/// <returns></returns>
void UI::UIControl::RemoveStyleClass(PodStringView<char> pair) noexcept {
    // 可能就几条甚至不超过一条, 直接删除即可
    const auto style_class = L::GetUniqueStyleClass(pair);
    const auto b = m_classesStyle.begin();
    const auto e = m_classesStyle.end();
    // UB说明: STL中, 这句是UB(out of range), 这是是自己实现的, 所以不是UB
    m_classesStyle.erase(std::find(b, e, style_class));
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void UI::UIControl::add_child(UIControl& child) noexcept {
    // 这条函数非常重要
    assert(this && "bad this ptr");
    // 无需再次添加
    if (child.m_pParent == this) return;
    // 在之前的父控件移除该控件
    if (child.m_pParent) child.m_pParent->remove_child(child);
    child.m_pParent = this;
    ++m_cChildrenCount;
    // 连接前后节点
    m_oTail.prev->next = &child;
    child.prev = m_oTail.prev;
    child.next = &m_oTail;
    m_oTail.prev = &child;
    // 要求刷新
    child.m_state.level = m_state.level + 1;
    assert(child.GetLevel() < 100ui8 && "tree too deep");
    if (child.GetCount()) PrivateControl::UpdateLevel(child);
    m_state.child_i_changed = true;
    L::MarkMinSizeChanged(*this);
    this->NeedUpdate();
    // 提示管理器新的控件被添加到控件树中
    L::ControlAttached(child);
}

#ifndef NDEBUG
/// <summary>
/// Canbes the cast to.
/// </summary>
/// <param name="cguid">The cguid.</param>
/// <returns></returns>
bool UI::UIControl::RuntimeInfo_dbg(const DebugInfo& info) const noexcept {
    switch (info.type)
    {
    case UI::Type_RuntimeInfo:
    {
        auto& str = static_cast<const DebugInfoRI&>(info).info;
        str.append(L"UIControl[]");
    }
        break;
    case UI::Type_CanbeCasted:
        return static_cast<const DebugInfoCC&>(info).guid == this->s_guid;
    }
    return false;
}
#endif

/// <summary>
/// Resizes the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void UI::UIControl::resize_child(UIControl& child, Size2F size) noexcept {
    // 无需修改
    if (IsSameInGuiLevel(child.m_oBox.size, size)) return;
    // 需要重新布局
    child.NeedRelayout();
    // 确定修改
    child.m_oBox.size = size;
}

/// <summary>
/// Resizes the specified size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
bool UI::UIControl::Resize(Size2F size) noexcept {
    // 无需修改
    if (IsSameInGuiLevel(m_oBox.size, size)) return false;
    // 修改布局
    const auto layout = this->IsTopLevel() ? this : m_pParent;
    layout->NeedRelayout();
    // 修改了数据
    m_oBox.size = size;
    return true;
}

/// <summary>
/// Afters the created.
/// </summary>
/// <returns></returns>
auto UI::UIControl::Init(const Size2F* init_size) noexcept -> Result {
    // TODO: 将本函数移到UPDATE列表中, 减少用户显式调用

    // 空指针
    if (!this) return{ Result::RE_OUTOFMEMORY };
#ifndef NDEBUG
    assert(m_state.inited_dbg == false && "this control has been inited");
    m_state.inited_dbg = true;
#endif
    Result hr;
    // 重建对象
    if (hr = this->Recreate()) {
        // 初始化大小
        const Size2F size = init_size ? *init_size : Size2F{100, 30};
        this->Resize(size);
    }
    return hr;
}

/// <summary>
/// Updates the self.
/// </summary>
/// <returns></returns>
void UI::UIControl::Update() noexcept {
    // 本次已经刷新了?
    //assert(!m_state.updated && "been updated this time");
    assert(m_state.inited_dbg && "must init control first");

    // 最基的不处理子控件索引更改
    m_state.child_i_changed = false;
    L::MarkMinSizeChanged(*this);
}


/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto UI::UIControl::Recreate() noexcept -> Result {
    return{ Result::RS_OK };
}

/// <summary>
/// Parses the specified .
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void UI::UIControl::add_attribute(const XmlAttribute& attr) noexcept {
    // id class style tabindex draggable accesskey contextmenu 
    constexpr auto BKDR_ID          = 0x0000361ful;
    constexpr auto BKDR_TABINDEX    = 0x1c6477b9ul;
    constexpr auto BKDR_CLASS       = 0xd85fe06cul;
    constexpr auto BKDR_CONTEXTMENU = 0xb133f7f6ul;
    constexpr auto BKDR_ACCESSKEY   = 0xba56ab7bul;
    constexpr auto BKDR_DRAGGABLE   = 0xbd13c3b5ul;
    constexpr auto BKDR_STYLE       = 0xf253f789ul;
    // HASH 一致就认为一致即可
    const auto code = UI::BKDRHash(attr.key.first, attr.key.second);
    switch (code & 7)
    {
    case BKDR_ID & 7:
        // ID
        if (code == BKDR_ID) {
            int bk; bk = 9;
        }
        break;
    case BKDR_TABINDEX & 7:
        //case BKDR_STYLE & 7:
        if (code == BKDR_STYLE) {
            int bk; bk = 9;
        }
        // TABINDEX
        else if (code == BKDR_TABINDEX) {
            int bk; bk = 9;
        }
        break;
    case BKDR_CLASS & 7:
        // CLASS
        if (code == BKDR_CLASS) {
            int bk; bk = 9;
        }
        break;
    case BKDR_CONTEXTMENU & 7:
        // CONTEXTMENU
        if (code == BKDR_CONTEXTMENU) {
            int bk; bk = 9;
        }
        break;
    case BKDR_ACCESSKEY & 7:
        // ACCESSKEY
        if (code == BKDR_ACCESSKEY) {
            int bk; bk = 9;
        }
        break;
    case BKDR_DRAGGABLE & 7:
        // DRAGGABLE
        if (code == BKDR_ACCESSKEY) {
            int bk; bk = 9;
        }
        break;
    }
}

/// <summary>
/// Adds/Remove the GUI event listener.
/// </summary>
/// <param name="e">The e.</param>
/// <param name="ownid">The ownid.</param>
/// <param name="listener">The listener.</param>
/// <returns></returns>
bool UI::UIControl::set_gui_event_listener(
    GuiEvent e, 
    uintptr_t ownid,
    GuiEventListener&& listener) noexcept {
    e; ownid; listener;
    assert(!"NOT IMPL");
    return false;
}

/// <summary>
/// Metas the information.
/// </summary>
/// <param name="mi">The mi.</param>
/// <returns></returns>
/*auto UI::UIControl::meta_info(const MetaInfo& mi) const noexcept->EventAccept {
    switch (mi.type)
    {
    case UI::Type_ElementName:
        static_cast<const MetaInfoEN&>(mi).element_name = "ctrl";
        return Event_Accept;
    case UI::Type_ClassName:
        static_cast<const MetaInfoCN&>(mi).class_name = "UIControl";
        return Event_Accept;
    case UI::Type_NameSpace:
        static_cast<const MetaInfoNS&>(mi).name_space = "UI";
        return Event_Accept;
    case UI::Type_GUID:
        static_cast<const MetaInfoGUID&>(mi).guid = s_guid;
        return Event_Accept;
    }
    return Event_Ignore;
}*/

/// <summary>
/// Sets the minimum size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void UI::UIControl::SpecifyMinSize(Size2F size) noexcept {
    m_oStyle.minsize_sp = size;
    assert(m_oStyle.minsize_sp.width <= m_oStyle.maxsize.width);
    assert(m_oStyle.minsize_sp.height <= m_oStyle.maxsize.height);
    L::MarkMinSizeChanged(*this);
    auto& s = m_oStyle;
    s.minsize.width = std::max(s.minsize.width, s.minsize_sp.width);
    s.minsize.height = std::max(s.minsize.height, s.minsize_sp.height);
}

/// <summary>
/// Specifies the maximum size.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void UI::UIControl::SpecifyMaxSize(Size2F size) noexcept {
    m_oStyle.maxsize = size;
    assert(m_oStyle.minsize.width <= m_oStyle.maxsize.width);
    assert(m_oStyle.minsize.height <= m_oStyle.maxsize.height);
}

/// <summary>
/// Maps to window.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void UI::UIControl::MapToWindow(RectF& rect) const noexcept {
    assert(m_state.world_changed == false && "world changed!");
    auto ptr = reinterpret_cast<Point2F*>(&rect);
    ptr[0] = UI::TransformPoint(m_mtWorld, ptr[0]);
    ptr[1] = UI::TransformPoint(m_mtWorld, ptr[1]);
}

/// <summary>
/// Maps to parent.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void UI::UIControl::MapToParent(RectF& rect) const noexcept {
    rect;
    if (this->IsTopLevel()) {
        assert(!"NOT IMPL");
    }
    else {
        assert(!"NOT IMPL");
    }
}

/// <summary>
/// Maps to window.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void UI::UIControl::MapToWindow(Point2F& rect) const noexcept {
    assert(m_state.world_changed == false && "world changed!");
    rect = UI::TransformPoint(m_mtWorld, rect);
}

/// <summary>
/// Maps to parent.
/// </summary>
/// <param name="point">The point.</param>
/// <returns></returns>
void UI::UIControl::MapToParent(Point2F& point) const noexcept {
    point;
    assert(!"NOT IMPL");
}


/// <summary>
/// Initializes a new instance of the <see cref="UISpacer"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UISpacer::UISpacer(UIControl* parent) noexcept : Super(parent) {

}

/// <summary>
/// Finalizes an instance of the <see cref="UISpacer"/> class.
/// </summary>
/// <returns></returns>
UI::UISpacer::~UISpacer() noexcept {
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void UI::UISpacer::Render() const noexcept {

}


/// <summary>
/// Initializes a new instance of the <see cref="UIScrollArea"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UIScrollArea::UIScrollArea(UIControl* parent) noexcept : Super(parent) {
    // overflow: auto, 滚动条会显示在布局中
    // overflow: scroll, 滚动条会显示在布局外
    m_szSingleStep = { 16, 16 };
    m_minContentSize = {};
}


/// <summary>
/// Finalizes an instance of the <see cref="UIScrollArea"/> class.
/// </summary>
/// <returns></returns>
UI::UIScrollArea::~UIScrollArea() noexcept {
#ifndef NDEBUG
    m_pVerticalSB = nullptr;
    m_pVerticalSB++;
    m_pHorizontalSB = nullptr;
    m_pHorizontalSB++;
#endif
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto UI::UIScrollArea::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    // -------------- 滚动
    auto wheel = [&, this](int index) noexcept {
        const auto content_size = this->GetBox().GetContentSize();
        const auto cremainw = index[&m_minContentSize.width] - index[&content_size.width];
        if (cremainw <= 0.f) return;
        auto& pos_o = index[&m_ptChildOffset.x];
        auto pos = pos_o - index[&m_szSingleStep.width] * e.wheel;
        pos = std::max(std::min(pos, cremainw), 0.f);
        if (pos != pos_o) {
            pos_o = pos;
            /*if (this->GetCount()) {
                auto&first = *this->begin();
                this->mark_child_world_changed(first);
                first.NeedUpdate();
            }*/
            m_state.world_changed = true;
            this->NeedUpdate();
        }
        this->sync_scroll_bar();
    };
    // -------------- 分类判断
    switch (e.type)
    {
    case UI::MouseEvent::Event_MouseWheelV:
        wheel(1);
        break;
    case UI::MouseEvent::Event_MouseWheelH:
        wheel(0);
        break;
    }
    return Super::DoMouseEvent(e);
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void UI::UIScrollArea::Update() noexcept {
    // 污了?
    if (m_state.dirty) {
        // 存在子控件才计算
        if (this->GetCount()) {
            // 更新布局
            this->relayout();
            // 为了避免下帧才个布局孩儿们, 
            // 下下帧布局孙儿们....子子孙孙无穷尽也
            // 在这里强制更新孩儿们
            for (auto& child : *this) child.Update();
        }
        // 不污
        m_state.dirty = false;
        // 这里, 世界不再改变
        //assert(m_state.world_changed == false);
    }
    // 链式调用
    Super::Update();
}

/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void UI::UIScrollArea::relayout() noexcept {
    // TODO: 基本布局
}

/// <summary>
/// Synchronizes the scroll bar.
/// </summary>
/// <returns></returns>
void UI::UIScrollArea::sync_scroll_bar() noexcept {
    const bool hok = m_pHorizontalSB && m_pHorizontalSB->IsVisible();
    const bool vok = m_pVerticalSB && m_pVerticalSB->IsVisible();
    // 交界区
    Size2F cross_zone;
    cross_zone.width = vok ? m_pVerticalSB->GetMinSize().width : 0.f;
    cross_zone.height = hok ? m_pHorizontalSB->GetMinSize().height : 0.f;
    auto csize = m_oBox.GetContentSize();
    // 水平滚动条
    if (hok) {
        m_pHorizontalSB->SetValue(m_ptChildOffset.x);
        m_pHorizontalSB->SetPageStep(csize.width - cross_zone.width);
        m_pHorizontalSB->SetSingleStep(m_szSingleStep.width);
        m_pHorizontalSB->SetMax(m_minContentSize.width - csize.width);
    }
    // 垂直滚动条
    if (vok) {
        m_pVerticalSB->SetValue(m_ptChildOffset.y);
        m_pVerticalSB->SetPageStep(csize.height - cross_zone.height);
        m_pVerticalSB->SetSingleStep(m_szSingleStep.height);
        m_pVerticalSB->SetMax(m_minContentSize.height - csize.height);
    }
}

/// <summary>
/// Layouts the size of the content.
/// </summary>
/// <param name="min">The minimum.</param>
/// <returns></returns>
auto UI::UIScrollArea::layout_scroll_bar() noexcept -> Size2F {
    auto min = m_minContentSize;
    // 创建SB
    auto create_sb = [this](AttributeOrient o) noexcept ->UIScrollBar* {
        if (auto bar = new(std::nothrow) UIScrollBar{ o, nullptr }) {
#ifndef NDEBUG
            bar->name_dbg = "sbar";
#endif
            this->add_direct_child(*bar);
            this->set_child_fixed_attachment(*bar);
            bar->Init();
            return bar;
        }
        return nullptr;
    };
    // 水平SB大小/高度
    auto get_hsb_size = [this]() noexcept ->float {
        if (m_pHorizontalSB && m_pHorizontalSB->IsVisible()) {
            return m_pHorizontalSB->GetMinSize().height;
        }
        return 0.0f;
    };
    // 垂直SB大小/宽度
    auto get_vsb_size = [this]() noexcept ->float {
        if (m_pVerticalSB && m_pVerticalSB->IsVisible()) {
            return m_pVerticalSB->GetMinSize().width;
        }
        return 0.0f;
    };
    // 设置SB不可视
    auto set_sb_visible = [](UIScrollBar* sb_ptr, bool visible) noexcept {
        if (sb_ptr) sb_ptr->SetVisible(visible);
    };
    // 保证垂直SB
    auto ensure_vsb = [=]() noexcept {
        // TODO: 看情况是不是创建
        if (!m_pVerticalSB) m_pVerticalSB = create_sb(Orient_Vertical);
    };
    // 保证水平SB
    auto ensure_hsb = [=]() noexcept {
        // TODO: 看情况是不是创建
        if (!m_pHorizontalSB) m_pHorizontalSB = create_sb(Orient_Horizontal);
    };
    // 内容大小
    const auto content_size = this->GetBox().GetContentSize();
    constexpr float MDW = MIN_SCROLLBAR_DISPLAY_SIZE;
    constexpr float MDH = MIN_SCROLLBAR_DISPLAY_SIZE;
    set_sb_visible(m_pHorizontalSB, false);
    set_sb_visible(m_pVerticalSB, false);
    // 太小不显示
    if (content_size.width > MDW && content_size.height > MDH) {
        // 4种情况
        if (min.width <= content_size.width) {
            // 1. MW <= CW && MH <= CH
            if (min.height <= content_size.height) {
                // HSB-X    VSB-X
                set_sb_visible(m_pHorizontalSB, false);
                set_sb_visible(m_pVerticalSB, false);
            }
            // 2. MW <= CW && MH > CH
            else {
                ensure_vsb();
                // VSB-O
                set_sb_visible(m_pVerticalSB, true);
                // 2-1: MW + SB > CW : HSB-O/X
                set_sb_visible(m_pHorizontalSB, min.width + get_vsb_size() > content_size.width);
            }
        }
        else {
            // 3. MW > CW && MH > CW
            if (min.height > content_size.height) {
                // HSB-O    VSB-O
                ensure_hsb();  ensure_vsb();
                set_sb_visible(m_pVerticalSB, true);
                set_sb_visible(m_pHorizontalSB, true);
            }
            // 4  MW > CW && MH <= CW
            else {
                ensure_hsb();
                // HSB-O
                set_sb_visible(m_pHorizontalSB, true);
                // 2-1: MW + SB > CW : HSB-O/X
                set_sb_visible(m_pVerticalSB, min.height + get_hsb_size() > content_size.height);
            }
        }
    }
    const bool hok = m_pHorizontalSB && m_pHorizontalSB->IsVisible();
    const bool vok = m_pVerticalSB && m_pVerticalSB->IsVisible();
    // 交界区
    Size2F cross_zone;
    cross_zone.width = vok ? m_pVerticalSB->GetMinSize().width : 0.f;
    cross_zone.height = hok ? m_pHorizontalSB->GetMinSize().height : 0.f;
    //const bool cross_zone = false;
    // 设置HSB位置: 底部
    if (hok) {
        m_pHorizontalSB->Resize({ 
            content_size.width - cross_zone.width,
            cross_zone.height 
        });
        m_pHorizontalSB->SetPos({ 
            m_state.layout_dir == Dir_Normal ? 0.f : cross_zone.width,
            content_size.height - cross_zone.height 
        });
    }
    // 设置VSB位置: 正向-右侧 反向-左侧
    if (vok) {
        m_pVerticalSB->Resize({ 
            cross_zone.width, 
            content_size.height - cross_zone.height,
        });
        Point2F pos = {};
        if (m_state.layout_dir == Dir_Normal)
            pos.x = content_size.width - cross_zone.width;
        m_pVerticalSB->SetPos(pos);
    }
    // 同步
    this->sync_scroll_bar();
    return { content_size.width - get_vsb_size() , content_size.height - get_hsb_size() };
}

/// <summary>
/// Gets the layout position.
/// </summary>
/// <returns></returns>
auto UI::UIScrollArea::get_layout_position() const noexcept -> Point2F {
    // 正向- 左上角
    if (m_state.layout_dir == Dir_Normal) {

    }
    // 反向- 左上角+[VSB, 0]
    else {
        if (m_pVerticalSB && m_pVerticalSB->IsVisible()) {
            return{ m_pVerticalSB->GetSize().width, 0.f };
        }
    }
    return{ 0.f, 0.f };
}

/*/// <summary>
/// Refreshes the minimum size of the content.
/// </summary>
/// <returns></returns>
void UI::UIBoxLayout::refresh_min_content_size() noexcept {

}*/

/// <summary>
/// Relayouts this instance.
/// </summary>
/// <returns></returns>
void UI::UIBoxLayout::relayout() noexcept {
    /*
        偏向于小的大小进行布局

        A(0)__B(0)__C(0)    布局A时, 只有B, B权重0, 
                |___D(0)    没有设置最小, 则B设为0
                |___E(1)     

        B(0)__C(0)          布局B时, 有B,C,D, E权重1,
          |___D(0)          没有设置最小, 则E设为B一
          |___E(1)          样大小, 其余设为0

    */
    // 1. 遍历子控件, 将有效可变权重加起来
    float flex_sum = 0.f;
    for (auto& child : *this)
        if (child.IsVaildInLayout())
            flex_sum += child.GetStyle().flex;
    // 2. 加入SB布局
    const bool horlay = m_state.layout_orient == Orient_Horizontal;
    const auto remaining = this->layout_scroll_bar();
    const auto remainw = remaining.width - m_minContentSize.width;
    const auto remainh = remaining.height - m_minContentSize.height;
    // 3. 计算每权重宽度
    Size2F per_unit = {};
    if (flex_sum > 0.f) {
        if (remainw > 0.f) per_unit.width = remainw / flex_sum;
        if (remainh > 0.f) per_unit.height = remainh / flex_sum;
    }
    Point2F pos = this->get_layout_position();
    // 遍历控件
    UIControl* last = nullptr;
    for (auto& child : *this) {
        // 可见才处理
        if (child.IsVaildInLayout()) {
            last = &child;
            auto size = child.GetMinSize();
            size.width += child.GetStyle().flex * per_unit.width;
            size.height += child.GetStyle().flex * per_unit.height;
            size.width = std::min(child.GetMaxSize().width, size.width);
            size.height = std::min(child.GetMaxSize().height, size.height);
            child.SetPos(pos); this->resize_child(child, size);
            horlay ? pos.x += size.width : pos.y += size.height;
        }
    }
    // XUL兼容: 垂直布局中, 宽度尽可能大
    if (!horlay && last && remainw > 0.f) {
        this->resize_child(*last, {
            last->GetSize().width + remainw, last->GetSize().height
        });
    }
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto UI::UIBoxLayout::DoEvent(
    UIControl* sender, const EventArg& e) noexcept -> EventAccept {
    // ---------------- 刷新最小大小
    auto refresh_min = [this]() noexcept {
        // TODO: 添加滚动条判断???
        RectF nonc = this->GetBox().GetNonContect();
        Size2F edge = { nonc.left + nonc.right, nonc.top + nonc.bottom };
        const bool ishor = m_state.layout_orient == Orient_Horizontal;
        Size2F minsize = {};
        // 遍历控件
        for (auto& child : *this) {
            // 可见即可
            if (child.IsVaildInLayout()) {
                const auto ms = child.GetMinSize();
                // 水平布局
                if (ishor) {
                    minsize.width += ms.width;
                    minsize.height = std::max(minsize.height, ms.height);
                }
                // 垂直布局
                else {
                    minsize.height += ms.height;
                    minsize.width = std::max(minsize.width, ms.width);
                }
            }
        }
        m_minContentSize = minsize;
        // 添加边界值
        minsize.width += edge.width;
        minsize.height += edge.height;
        // 更新值
        m_oStyle.minsize = minsize;
    };

    // ---------------- 事件处理分支
    switch (e.nevent)
    {
    case UI::NoticeEvent::Event_RefreshMinSize:
        refresh_min();
        return UI::Event_Accept;
    }
    return Super::DoEvent(sender, e);
}


/// <summary>
/// Initializes a new instance of the <see cref="UIBoxLayout"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UIBoxLayout::UIBoxLayout(UIControl* parent) noexcept : Super(parent) {
}

/// <summary>
/// Finalizes an instance of the <see cref="UIBoxLayout"/> class.
/// </summary>
/// <returns></returns>
UI::UIBoxLayout::~UIBoxLayout() noexcept {
}

/// <summary>
/// Sets the orient.
/// </summary>
/// <param name="o">The o.</param>
/// <returns></returns>
void UI::UIBoxLayout::SetOrient(AttributeOrient o) noexcept {
    if (static_cast<bool>(o) != m_state.layout_orient) {
        m_state.layout_orient = static_cast<bool>(o);
        this->NeedRelayout();
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="UIVBoxLayout"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UIVBoxLayout::UIVBoxLayout(UIControl* parent) noexcept : Super(parent) {
    m_state.layout_orient = Orient_Vertical;
}


/// <summary>
/// Finalizes an instance of the <see cref="UIVBoxLayout"/> class.
/// </summary>
/// <returns></returns>
UI::UIVBoxLayout::~UIVBoxLayout() noexcept {
}

/// <summary>
/// Initializes a new instance of the <see cref="UIHBoxLayout"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UIHBoxLayout::UIHBoxLayout(UIControl* parent) noexcept : Super(parent) {
    m_state.layout_orient = Orient_Horizontal;
}


/// <summary>
/// Finalizes an instance of the <see cref="UIHBoxLayout"/> class.
/// </summary>
/// <returns></returns>
UI::UIHBoxLayout::~UIHBoxLayout() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIButton"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UIButton::UIButton(UIControl* parent) noexcept : Super(parent) {

}

/// <summary>
/// Finalizes an instance of the <see cref="UIButton"/> class.
/// </summary>
/// <returns></returns>
UI::UIButton::~UIButton() noexcept {

}

/// <summary>
/// Initializes a new instance of the <see cref="UIScrollBar"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UIScrollBar::UIScrollBar(AttributeOrient o, 
    UIControl* parent) noexcept : Super(parent), m_orient(o) {
    this->SetBgColor({ RGBA_Red });
    // TODO: 最小大小
    this->SpecifyMinSize({ 30, 30 });
}

/// <summary>
/// Finalizes an instance of the <see cref="UIScrollBar"/> class.
/// </summary>
/// <returns></returns>
UI::UIScrollBar::~UIScrollBar() noexcept {

}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void UI::UIScrollBar::Render() const noexcept {
    assert(m_state.inited_dbg && "must init control first");
    return Super::Render();
    /*this->begin_render();
    auto& painter = UI::Private::renderer();
    if (this->color) {
        QColor bgcolor{ Qt::white };
        QColor color{ QRgb{ this->color } };
        RectF rect = this->GetBox().GetContentEdge();
        Size2F csize = {
            rect.right - rect.left,
            rect.bottom - rect.top
        };
        painter.fillRect(
            QRectF{ rect.left, rect.top, csize.width, csize.height },
            bgcolor
        );
        const auto len = m_fMax - m_fMin + m_fPageStep;
        const auto www = m_orient == Orient_Horizontal ? csize.width : csize.height;
        const auto x = m_fValue / len * www;
        const auto l = m_fPageStep / len * www;
        if (m_orient == Orient_Horizontal) {
            painter.fillRect(QRectF{
                x, rect.top,
                l,
                csize.height
            }, color);
        }
        else {
            painter.fillRect(QRectF{
                rect.left, x,
                csize.width,
                l
            }, color);

        }
    }*/
}


/// <summary>
/// Sets the value.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void UI::UIScrollBar::SetValue(float v) noexcept {
    if (IsSameInGuiLevel(m_fValue, v)) return;
    m_fValue = v;
    this->Repaint();
}


/// <summary>
/// Sets the maximum.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void UI::UIScrollBar::SetMax(float v) noexcept {
    if (IsSameInGuiLevel(m_fMax, v)) return;
    m_fMax = v;
    this->Repaint();
}

/// <summary>
/// Sets the minimum.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void UI::UIScrollBar::SetMin(float v) noexcept {
    if (IsSameInGuiLevel(m_fMin, v)) return;
    m_fMin = v;
    this->Repaint();
}

/// <summary>
/// Sets the page step.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void UI::UIScrollBar::SetPageStep(float v) noexcept {
    if (IsSameInGuiLevel(m_fPageStep, v)) return;
    m_fPageStep = v;
    this->Repaint();
}

/// <summary>
/// Sets the page step.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void UI::UIScrollBar::SetSingleStep(float v) noexcept {
    if (IsSameInGuiLevel(m_fSingleStep, v)) return;
    m_fSingleStep = v;
    this->Repaint();
}



/// <summary>
/// Initializes a new instance of the <see cref="UIViewport"/> class.
/// </summary>
UI::UIViewport::UIViewport(XUIWindow* parent) noexcept :
m_window(parent), Super(nullptr) {
    this->SetOrient(Orient_Vertical);
}

/// <summary>
/// Finalizes an instance of the <see cref="UIViewport"/> class.
/// </summary>
/// <returns></returns>
UI::UIViewport::~UIViewport() noexcept {
}

// ----------------------------------------------------------------------------
// -----------------------------  UI::Box  ------------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the <see cref="Box"/> struct.
/// </summary>
void UI::Box::Init() noexcept {
    pos = {};
    size = {};
    visible = {};
    border = {};
    margin = {};
    padding = {};
}

PCN_NOINLINE
/// <summary>
/// Gets the non contect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void UI::Box::GetNonContect(RectF& rect) const noexcept {
    rect.top = margin.top + border.top + padding.top;
    rect.left = margin.left + border.left + padding.left;
    rect.right = margin.right + border.right + padding.right;
    rect.bottom = margin.bottom + border.bottom + padding.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the margin rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void UI::Box::GetMarginEdge(RectF& rect) const noexcept {
    rect.top = 0.0f;
    rect.left = 0.0f;
    rect.right = 0.0f + this->size.width;
    rect.bottom = 0.0f + this->size.height;
}


PCN_NOINLINE
/// <summary>
/// Gets the border rect.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void UI::Box::GetBorderEdge(RectF& rect) const noexcept {
    this->GetMarginEdge(rect);
    rect.top += this->margin.top;
    rect.left += this->margin.left;
    rect.right -= this->margin.right;
    rect.bottom -= this->margin.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the padding rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void UI::Box::GetPaddingEdge(RectF& rect) const noexcept {
    this->GetBorderEdge(rect);
    rect.top += this->border.top;
    rect.left += this->border.left;
    rect.right -= this->border.right;
    rect.bottom -= this->border.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the content rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void UI::Box::GetContentEdge(RectF& rect) const noexcept {
    this->GetBorderEdge(rect);
    rect.top += this->padding.top;
    rect.left += this->padding.left;
    rect.right -= this->padding.right;
    rect.bottom -= this->padding.bottom;
}

PCN_NOINLINE
/// <summary>
/// Gets the size of the content.
/// </summary>
/// <returns></returns>
auto UI::Box::GetContentSize() const noexcept -> Size2F {
    RectF rect; this->GetNonContect(rect);
    return {
        size.width - rect.left - rect.right,
        size.height - rect.top - rect.bottom
    };
}

#include <vector>
#include <util/pod_hash.h>

namespace UI {
    /// <summary>
    /// L L
    /// </summary>
    struct LL {
        // add default control
        void add_defctrl() noexcept;
        // ctor
        LL() { add_defctrl(); };
        // dtor
        ~LL() { delete root; }
        // is in updated
        bool is_in_update() const { return !ll.update_clear_list.empty(); }
        // update list
        std::vector<UIControl*> update_list;
        // 2nd update list
        std::vector<UIControl*> update_list_dofor;
        // clear list
        std::vector<UIControl*> update_clear_list;
        // min size changed root
        UIControl*              min_size_root = nullptr;
        // root test
        UIControl*              root = nullptr;
        // unique style classes
        POD::HashMap<void*>     sclasses;
        // unique control classes
        POD::HashMap<void*>     cclasses;
    } ll;
    /// <summary>
    /// Adds the defctrl.
    /// </summary>
    /// <returns></returns>
    void UI::LL::add_defctrl() noexcept {
        cclasses.insert({ "colorpicker", nullptr });
        cclasses.insert({ "scrollbar", nullptr });
        cclasses.insert({ "groupbox", nullptr });
        cclasses.insert({ "checkbox", nullptr });
        cclasses.insert({ "textbox", nullptr });
        cclasses.insert({ "button", nullptr });
        cclasses.insert({ "radio", nullptr });
        cclasses.insert({ "vbox", nullptr });
        cclasses.insert({ "hbox", nullptr });
        cclasses.insert({ "box", nullptr });
    }
    /// <summary>
    /// Sets the root.
    /// </summary>
    /// <param name="ptr">The PTR.</param>
    void Private::set_root(UIControl * ptr) {
        assert(ll.root == nullptr);
        ll.root = ptr;
    }
    /// <summary>
    /// Controls the attached.
    /// </summary>
    /// <param name="ctrl">The control.</param>
    /// <returns></returns>
    void UI::L::ControlAttached(UIControl& ctrl) noexcept {
        // 1.为控件链接新的样式表(有的话)
        ctrl;
    }
    /// <summary>
    /// Removes the reference.
    /// </summary>
    void L::ControlDisattached(UIControl& ctrl) noexcept {
        assert(!ll.is_in_update() && "remove in updated");
        ll.update_list.erase(std::remove(
            ll.update_list.begin(), ll.update_list.end(), &ctrl),
            ll.update_list.end()
        );
    }
    /// <summary>
    /// Marks the minimum size changed.
    /// </summary>
    /// <param name="ctrl">The control.</param>
    /// <returns></returns>
    void UI::L::MarkMinSizeChanged(UIControl& ctrl) noexcept {
        // TODO: O(1)优化
        auto ptr = &ctrl; while (!ptr->IsTopLevel()) ptr = ptr->GetParent();
        ll.min_size_root = ptr;
    }
    /// <summary>
    /// Adds the update list.
    /// </summary>
    /// <returns></returns>
    void L::AddUpdateList(UIControl& ctrl) noexcept {
        try { 
            // TODO: [优化] 将越接近根节点的控件放在前面
            if (!ctrl.is_in_update_list()) {
                ctrl.add_into_update_list();
                ll.update_list.push_back(&ctrl);
            }
        }
        catch (...) {}
    }

    /// <summary>
    /// Gets the unique style class.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    auto L::GetUniqueStyleClass(PodStringView<char> pair) noexcept -> const char * {
        assert(pair.second > pair.first && "bad string");
        return ll.sclasses.insert(pair.first, pair.second, nullptr).first->first;
    }
    /// <summary>
    /// Gets the unique control class.
    /// </summary>
    /// <param name="pair">The pair.</param>
    /// <returns></returns>
    auto L::GetUniqueControlClass(PodStringView<char> pair) noexcept -> const char* {
        assert(pair.second > pair.first && "bad string");
        return ll.cclasses.insert(pair.first, pair.second, nullptr).first->first;
        /*const auto itr = ll.cclasses.find(pair.first, pair.second);
        if (itr != ll.cclasses.end()) return itr->first;
        return nullptr;*/
    }
    /// <summary>
    /// Invalidates the control.
    /// </summary>
    /// <param name="ctrl">The control.</param>
    /// <param name="rect">The rect.</param>
    /// <returns></returns>
    void L::InvalidateControl(UIControl & ctrl, const RectF& rect) noexcept {
        ctrl; rect;
        Private::update_window(reinterpret_cast<void*>(ll.root->user_data));
    }
    /// <summary>
    /// Updates the control in list.
    /// </summary>
    /// <returns></returns>
    void L::UpdateControlInList() noexcept {
        if (ll.min_size_root) {
            PrivateControl::RefreshMinSize(*ll.min_size_root);
            ll.min_size_root = nullptr;
        }
        // 不为空就继续刷新
        while(!ll.update_list.empty()) {
            // 交换两者
            ll.update_list_dofor.clear();
            ll.update_list.swap(ll.update_list_dofor);
            // 单独刷新
            for (auto* ctrl : ll.update_list_dofor) {
                ll.update_clear_list.push_back(ctrl);
                ctrl->Update();
            }
        }
        // 清理状态
        {
            UIControl* top = nullptr;
            for (auto ctrl : ll.update_clear_list) {
                if (ctrl->m_state.world_changed) {
                    if (!top || top->GetLevel() > ctrl->GetLevel()) {
                        top = ctrl;
                    }
                }
                ctrl->remove_from_update_list();
            }
            // 更新世界
            if (top) {
                auto toptop = top->IsTopLevel() ? top : top->GetParent();
                PrivateControl::UpdateWorld(*toptop);
            }
        }
        ll.update_clear_list.clear();
    }

    /// <summary>
    /// Gets the delta time.
    /// </summary>
    /// <returns></returns>
    auto L::GetDeltaTime() noexcept -> float {
        return (16.f + float(std::rand() % 1000) / 1000.f) / 1000.f;
    }
    
    /// <summary>
    /// Optimizeds the render.
    /// </summary>
    /// <param name="ctrl">The control.</param>
    /// <param name="region">The region.</param>
    /// <param name="length">The length.</param>
    /// <returns></returns>
    void L::RecursiveRender(
        const UIControl& ctrl,
        const RectF region[], 
        uint32_t length) noexcept {
        /*
        块呈现器的堆栈顺序如下：
            background color
            background image
            border
            children
            outline
        */
        // TODO: 脏矩形渲染: 没有交集就不渲染
        /*if (length && std::all_of(region, region + length, [&](const RectF& rect) {
            return !(rect && ctrl.m_oBox.rect);
        })) return;*/
        // 看不到就不渲染
        const auto csize = ctrl.GetSize();
        if (!ctrl.IsVisible() || csize.width <= 0.f || csize.height <= 0.f)
            return;
        // 渲染器设置本身裁剪矩形以免绘制出去
        //auto& r = UI::Private::renderer();
        //r.setClipRect(qt({}, ctrl.m_oBox.rect));
        // 区域渲染上层已经设定了region裁剪区域
        ctrl.Render();
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
        //ctrl.render_children();
    }
}




#include "../util/pod_vector.h"
#include "../util/pod_fixedqueue.h"

struct Teest {
    int a;
    char* operator &() { return reinterpret_cast <char*>(this); }
};

template<typename T> inline T& force_cast(const T& x) noexcept { 
    return const_cast<T&>(x); 
}


extern "C"
__declspec(dllimport)
void _stdcall OutputDebugStringW(const wchar_t*);

int test() { UI::PrivateControl::test(); return 0; }

void dump(const void* buf, size_t len, const char* fname) {
    if (const auto file = std::fopen(fname, "wb")) {
        std::fwrite(buf, len, 1, file);
        std::fclose(file);
    }
}
template<typename T> void dump(const T& t, const char* fname) {
    return dump(&t, sizeof(T), fname);
}

void UI::PrivateControl::test() {
    GuiEventListener listener{ [](UIControl& ) ->EventAccept {
        return EventAccept::Event_Accept;
    }, 1};
    listener += [](UIControl&) ->EventAccept {
        return EventAccept::Event_Ignore;
    };
    UIControl ctrl;
    sizeof(Box);
    sizeof(Style);
    sizeof(Matrix3X2F);
    int c = sizeof(ctrl);
    wchar_t buf[32]; std::swprintf(buf, 32, L"%d\n", c);
    ::OutputDebugStringW(buf);
    listener(ctrl);
    listener.RemoveCallChain(1);
    listener(ctrl);
    {
        UI::PseudoClasses a;
        std::scanf("%c", &a);
        a.Zero();
        std::scanf("%c", &a);
        a = !a;
        std::scanf("%c", &a);

        auto b = a;
        b.hover = true;
        std::scanf("%c", &b);
        bool c = (a & b) & ((!a) & b);
        std::printf("%d", int(c));
    }
    {
        UIControl a;
        auto& classes = a.GetStyleClasses();
        a.AddStyleClass(PodStringView<char>::FromCStyle("system-tab"));
        a.AddStyleClass(PodStringView<char>::FromCStyle("system-radio"));
        a.AddStyleClass(PodStringView<char>::FromCStyle("system-button"));
        a.AddStyleClass(PodStringView<char>::FromCStyle("system-scrollbar"));
        a.AddStyleClass(PodStringView<char>::FromCStyle("system-tab\0\n\r\n"));
        a.RemoveStyleClass(PodStringView<char>::FromCStyle("system-tab\0\n"));
        a.RemoveStyleClass(PodStringView<char>::FromCStyle("system-tab\0\n\n"));
        a.RemoveStyleClass(PodStringView<char>::FromCStyle("system-radio\0\n\n"));
        a.RemoveStyleClass(PodStringView<char>::FromCStyle("system-button\0\n\n"));
        a.RemoveStyleClass(PodStringView<char>::FromCStyle("system-scrollbar\0\n\n"));

        force_cast(classes).shrink_to_fit();

    }
    /*UI::RectF ra{ 0, 0, 100, 100 };
    UI::RectF rb{ 100, 100, 200, 200 };
    UI::RectF rc{ 50, 50, 150, 150 };
    UI::RectF rd{ 25, 25, 75, 75 };
    auto ab = ra && rb;
    auto ac = ra && rc;
    auto bc = rb && rc;
    auto ad = ra && rd;
    auto da = rd && ra;
    return 0;*/
}

// ----------------------------------------------------------------------------
// ---------------------------  UI::CtrlState  --------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the <see cref="CtrlState"/> struct.
/// </summary>
void UI::CtrlState::Init() noexcept {
    static_assert(sizeof(CtrlState) == sizeof(uint32_t), "bad size");
    directly_managed = false;
    child_i_changed = true;
    in_update_list = false;
    world_changed = false;
    layout_orient = false;
    layout_custom = false;
    attachment = false;
    layout_dir = false;
    //updated = false;
    dirty = false;
    level = 0;
#ifndef NDEBUG
    inited_dbg = false;
#endif
    /*
    minsize_changed:
        子控件增删换位
        子控件最小大小修改
    */
}

/// <summary>
/// Initializes a new instance of the <see cref="StyleState"/> struct.
/// </summary>
void UI::StyleState::Init() noexcept {
    static_assert(sizeof(CtrlState) == sizeof(uint32_t), "bad size");
    focusable = false;
    visible = true;
    disabled = true;
    active = false;
    hover = false;
    focus = false;
    checked = false;
    indeterminate = false;
}


/// <summary>
/// Initializes this instance.
/// </summary>
/// <returns></returns>
UI::Style::Style() noexcept {
    state.Init();
    flex = 0.0f;
    minsize = {};
    maxsize = { DEFAULT_MAX_SIZE, DEFAULT_MAX_SIZE };
    minsize_sp = {};
}

/// <summary>
/// Finalizes an instance of the <see cref="Style"/> class.
/// </summary>
/// <returns></returns>
UI::Style::~Style() noexcept {
}

/*// <summary>
/// Applies the value.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void UI::Style::ApplyValue(const SSValue& value) noexcept {
    switch (value.type)
    {
    case UI::ValueType::Type_Unknown:
    default:
        assert(!"Unknown Type");
        break;
    case UI::ValueType::Type_BackgroundColor:
        break;
    case UI::ValueType::Type_BackgroundImage:
        break;
    case UI::ValueType::Type_BackgroundAttachment:
        break;
    case UI::ValueType::Type_BackgroundRepeat:
        break;
    case UI::ValueType::Type_BackgroundClip:
        break;
    case UI::ValueType::Type_BackgroundOrigin:
        break;
    }
}*/

// ----------------------------------------------------------------------------
// ------------------------------  Until  -------------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Determines whether the specified a is overlap.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool UI::IsOverlap(const RectF & a, const RectF & b) noexcept {
    return a.left < b.right && a.right > b.left && 
        a.top < b.bottom && a.bottom > b.top;
}

/// <summary>
/// Determines whether the specified a is include.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool UI::IsInclude(const RectF & a, const RectF & b) noexcept {
    return a.left >= b.left && a.top >= b.top && 
        a.right <= b.right && a.bottom <= b.bottom;
}

/// <summary>
/// Determines whether the specified a is include.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool UI::IsInclude(const RectF & a, const Point2F& b) noexcept {
    return b.x >= a.left && b.x <= a.right
        && b.y >= a.top && b.y <= a.bottom;;
}

/// <summary>
/// Determines whether the specified a is same.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
bool UI::IsSame(const RectF& a, const RectF& b) noexcept {
    return a.left == b.left && a.top == b.top &&
        a.right == b.right && a.bottom && b.bottom;
}


/// <summary>
/// Initializes a new instance of the <see cref="UIGraphicsScene"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::UIGraphicsScene::UIGraphicsScene(UIControl* parent) noexcept : Super(parent) {

}


/// <summary>
/// Finalizes an instance of the <see cref="UIGraphicsScene"/> class.
/// </summary>
/// <returns></returns>
UI::UIGraphicsScene::~UIGraphicsScene() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="StaticText"/> class.
/// </summary>
/*UI::Component::StaticText::StaticText() noexcept {
}

/// <summary>
/// Finalizes an instance of the <see cref="StaticText"/> class.
/// </summary>
/// <returns></returns>
UI::Component::StaticText::~StaticText() noexcept {
}

/// <summary>
/// Initializes a new instance of the <see cref="StaticText"/> class.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
UI::Component::StaticText::StaticText(
    const wchar_t* str, 
    std::size_t len
) noexcept : m_strText(str, str+len) {
    
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void UI::Component::StaticText::SetText(
    const wchar_t * str, std::size_t len) noexcept {
    m_strText.assign(str, str + len);
}*/

PCN_NOINLINE
/// <summary>
/// To the RGBA.
/// </summary>
/// <returns></returns>
auto UI::ColorF::ToRGBA() const noexcept -> RGBA {
    // 写入
    RGBA argb;
    argb.r = uint8_t(uint32_t(this->r * 255.f) & 0xFF);
    argb.g = uint8_t(uint32_t(this->g * 255.f) & 0xFF);
    argb.b = uint8_t(uint32_t(this->b * 255.f) & 0xFF);
    argb.a = uint8_t(uint32_t(this->a * 255.f) & 0xFF);
    return argb;
}

PCN_NOINLINE
/// <summary>
/// Froms the RGBA in run time
/// </summary>
/// <param name="color32">The color32.</param>
/// <returns></returns>
void UI::ColorF::FromRGBA_RT(ColorF& color, RGBA color32) noexcept {
#ifdef UI_NO_SSE2
    // 位移量
    constexpr RGBA RED_SHIFT    = CHAR_BIT * helper::color_order::r;
    constexpr RGBA GREEN_SHIFT  = CHAR_BIT * helper::color_order::g;
    constexpr RGBA BLUE_SHIFT   = CHAR_BIT * helper::color_order::b;
    constexpr RGBA ALPHA_SHIFT  = CHAR_BIT * helper::color_order::a;
    // 掩码
    constexpr RGBA RED_MASK     = 0xFFU << RED_SHIFT;
    constexpr RGBA GREEN_MASK   = 0xFFU << GREEN_SHIFT;
    constexpr RGBA BLUE_MASK    = 0xFFU << BLUE_SHIFT;
    constexpr RGBA ALPHA_MASK   = 0xFFU << ALPHA_SHIFT;
    // 计算
    color.r = static_cast<float>((color32.pri & RED_MASK)   >> RED_SHIFT)   / 255.f;
    color.g = static_cast<float>((color32.pri & GREEN_MASK) >> GREEN_SHIFT) / 255.f;
    color.b = static_cast<float>((color32.pri & BLUE_MASK)  >> BLUE_SHIFT)  / 255.f;
    color.a = static_cast<float>((color32.pri & ALPHA_MASK) >> ALPHA_SHIFT) / 255.f;
#else
    constexpr float f256_255 = 256.0f / 255.0f;
    constexpr int32_t magic32 = 0x47004700;
    const __m128i magic  = _mm_set_epi32(magic32, magic32, magic32, magic32);
    const __m128 i16max  = _mm_set_ps(32768.0f, 32768.0f, 32768.0f, 32768.0f);
    const __m128 u8ratio = _mm_set_ps(f256_255, f256_255, f256_255, f256_255);
    // 单个转换其实比上面的慢了 10%, 但是生成的代码少了一半
    // p.s. 同时转换4个就会比上面的快 一倍
    __m128 in1, out1;
    __m128i in, tmplo; reinterpret_cast<uint32_t&>(in) = color32.primitive;
    tmplo = _mm_unpacklo_epi8(in, _mm_set_epi32(0, 0, 0, 0));
    in1   = _mm_castsi128_ps(_mm_unpacklo_epi16(tmplo, magic));
    out1  = _mm_mul_ps(_mm_sub_ps(in1, i16max), u8ratio);
    _mm_storeu_ps(reinterpret_cast<float*>(&color), out1);
#endif
}

#if 0
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the <see cref="CUIBaseAnimation"/> class.
/// </summary>
/// <param name="t">The t.</param>
UI::CUIBaseAnimation::CUIBaseAnimation(AnimationType t, 
    uint32_t count) noexcept : type(t), m_uValueCount(count) {
    assert(count < uint32_t(MAX_VALUE_COUNT) && "too large");
    std::memset(m_afStarts, 0, sizeof(m_afStarts));
    std::memset(m_afEnds, 0, sizeof(m_afEnds));
}


/// <summary>
/// Finalizes an instance of the <see cref="CUIBaseAnimation"/> class.
/// </summary>
/// <returns></returns>
UI::CUIBaseAnimation::~CUIBaseAnimation() noexcept {

}

/// <summary>
/// Sets the starts.
/// </summary>
/// <param name="starts">The starts.</param>
/// <returns></returns>
void UI::CUIBaseAnimation::SetStarts(float starts[]) noexcept {
    std::copy(starts, starts + m_uValueCount, m_afStarts);
}

/// <summary>
/// Sets the ends.
/// </summary>
/// <param name="ends">The ends.</param>
/// <returns></returns>
void UI::CUIBaseAnimation::SetEnds(float ends[]) noexcept {
    std::copy(ends, ends + m_uValueCount, m_afEnds);
}

/// <summary>
/// Updates the specified t.
/// </summary>
/// <param name="t">The t.</param>
/// <returns></returns>
void UI::CUIBaseAnimation::Update(float t) noexcept {
    // 到点
    if (this->time <= 0.f) {
        std::copy(m_afEnds, m_afEnds + m_uValueCount, m_pOutput);
        return;
    }
    // 计算
    for (uint32_t i = 0; i < m_uValueCount; ++i) {
        const auto v = UI::EasingFunction(type, time / duration);
        m_pOutput[i] = v * (m_afStarts[i] - m_afEnds[i]) + m_afEnds[i];
    }
    // 减少时间
    this->time -= t;
}

#endif


/// <summary>
/// Parses the stylesheet.
/// </summary>
/// <param name="">The .</param>
/// <param name="vs">The vs.</param>
/// <param name="ss">The ss.</param>
/// <returns></returns>
bool UI::ParseStylesheet(
    const char* str, 
    SSValues& vs, 
    SSSelectors& ss
) noexcept {
    SSSelector sss;
    str;
    /* 测试样式表
    button {
        background-color: green;
    }
    */
    vs.push_back({
        ValueType::Type_BackgroundColor,
        RGBA_Green,
    });
    // button [0, 1)
    sss = {
        nullptr,
        L::GetUniqueElementName(PodStringView<char>::FromCStyle("button")),
        nullptr,
        0, 1-0
    };
    ss.push_back(sss);
    return true;
}


// ui namespace
namespace UI {
    /// <summary>
    /// private data for XUIWindow
    /// </summary>
    class XUIWindowPrivate {
    public:
        // ctor
        XUIWindowPrivate() noexcept;
        // dtor
        ~XUIWindowPrivate() noexcept;
    private:
    public:
        // position of window
        Point2L         pos = {};
        // stylesheets: values
        SSValues        values;
        // stylesheets: selectors
        SSSelectors     selectors;
    };
    /// <summary>
    /// Xuis the window private.
    /// </summary>
    /// <returns></returns>
    UI::XUIWindowPrivate::XUIWindowPrivate() noexcept {
        UI::ParseStylesheet("", values, selectors);
    }
    /// <summary>
    /// Finalizes an instance of the <see cref=""/> class.
    /// </summary>
    /// <returns></returns>
    UI::XUIWindowPrivate::~XUIWindowPrivate() noexcept {
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="XUIWindow"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
UI::XUIWindow::XUIWindow(XUIWindow* parent) noexcept :
m_pParent(parent), m_private(new(std::nothrow)XUIWindowPrivate) {
    // TODO: 致命错误: 内存不足
    //if (!m_private)
}

/// <summary>
/// Finalizes an instance of the <see cref="XUIWindow"/> class.
/// </summary>
/// <returns></returns>
UI::XUIWindow::~XUIWindow() noexcept {
    delete m_private;
}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/// <summary>
/// Sets the position.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void UI::XUIWindow::SetPos(Point2L pos) noexcept {
    assert(m_private && "bug: you shall not pass");
    auto& this_pos = m_private->pos;
    // 无需移动窗口
    if (this_pos.x == pos.x && this_pos.y == pos.y) return; this_pos = pos;
    // 内联窗口
    if (this->IsInlineWindow()) {
        assert(!"NOT IMPL");
    }
    // 系统窗口
    else {
        ::SetWindowPos(m_hwnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE);
    }
}




static_assert(CHAR_BIT == 8, "not support if char_bit != 8");

#ifndef __GNUC__
#ifndef NDEBUG

namespace UI {
    enum : uint32_t {
        LITTLE_ENDIAN   = 0x03020100ul,
        BIG_ENDIAN      = 0x00010203ul,
        PDP_ENDIAN      = 0x01000302ul,
    };
    struct endian_runtime_assert {
        endian_runtime_assert() noexcept {
            const union { unsigned char bytes[4]; uint32_t value; }
                host_order{ { 0, 1, 2, 3 } };
            const bool le = host_order.value == LITTLE_ENDIAN;
            const bool be = host_order.value == BIG_ENDIAN;
            const bool me = host_order.value == PDP_ENDIAN;
            assert(le == helper::is_little_endian::value);
            assert(be == helper::is_big_endian::value);
            assert(me == helper::is_pdp_endian::value);
        }
    } endian_runtime_checker;
}

#endif
#endif