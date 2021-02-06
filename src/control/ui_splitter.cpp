#include <core/ui_ctrlmeta.h>
#include <control/ui_splitter.h>
#include <event/ui_splitter_event.h>
#include <core/ui_window.h>
#include <debugger/ui_debug.h>


// ui namespace
namespace LongUI {
    // UISplitter类 元信息
    LUI_CONTROL_META_INFO(UISplitter, "splitter");
}


/// <summary>
/// Finalizes an instance of the <see cref="UISplitter"/> class.
/// </summary>
/// <returns></returns>
LongUI::UISplitter::~UISplitter() noexcept {

}


/// <summary>
/// Sets the orient.
/// </summary>
/// <param name="o">The o.</param>
/// <returns></returns>
void LongUI::UISplitter::SetOrient(AttributeOrient o) noexcept {
    const bool booooool = o & 1;
    if (booooool != m_state.orient) {
        m_state.orient = booooool;
        this->change_cursor();
    }
}

/// <summary>
/// Changes the cursor.
/// </summary>
/// <returns></returns>
void LongUI::UISplitter::change_cursor() noexcept {
    if (m_state.orient) {
        m_hovered = CUICursor::Cursor_SizeNS;
        m_oStyle.appearance = Appearance_SplitterV;
    }
    else {
        m_hovered = CUICursor::Cursor_SizeWE;
        m_oStyle.appearance = Appearance_SplitterH;

    }
}


/// <summary>
/// Initializes a new instance of the <see cref="UISplitter" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UISplitter::UISplitter(const MetaControl& meta) noexcept
    : Super(meta), m_hovered(CUICursor::Cursor_Arrow) {
    m_attribute.resizebefore = Resize_Closest;
    m_attribute.resizeafter = Resize_Closest;
    m_attribute.collapse = Resizea_None;
    m_attribute.dragoff = false;
    // 这个根据父节点自行定义?
    // 默认是水平方向拉拽?
    m_state.orient = Orient_Horizontal;
    // 允许焦点?
    m_state.capturable = true;
    m_oStyle.appearance = Appearance_SplitterH;
}

/// <summary>
/// initialize UISplitter
/// </summary>
/// <returns></returns>
void LongUI::UISplitter::initialize() noexcept {
    // 初始化鼠标指针样式
    this->change_cursor();
    // 初始化超类
    Super::initialize();
}


/// <summary>
/// add xml attribute for this
/// </summary>
/// <param name="key"></param>
/// <param name="value"></param>
/// <returns></returns>
void LongUI::UISplitter::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_COLLAPSE        = 0x16808cf1_ui32;
    constexpr auto BKDR_RESIZEAFTER     = 0xa6a1fb98_ui32;
    constexpr auto BKDR_RESIZEBEFORE    = 0x2cfcb42f_ui32;
    switch (key)
    {
    case BKDR_COLLAPSE:
        // collapse
        m_attribute.collapse = AttrParser::Collapse(value);
        break;
    case BKDR_RESIZEAFTER:
        // resizeafter
        m_attribute.resizeafter = AttrParser::Resize(value);
        break;
    case BKDR_RESIZEBEFORE:
        // resizebefore
        m_attribute.resizebefore = AttrParser::Resize(value);
        break;
    default:
        // 超类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// update this
/// </summary>
/// <param name="reason"></param>
/// <returns></returns>
//void LongUI::UISplitter::Update(UpdateReason reason) noexcept {
//    if (reason & Reason_SizeChanged) {
//        const auto size = m_oBox.size;
//        int bk = 9;
//    }
//    Super::Update(reason);
//}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UISplitter::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    assert(m_pParent && "no mother");
    switch (e.type)
    {
    case MouseEvent::Event_MouseEnter:
        assert(m_pWindow && "no window no mouse");
        m_pWindow->SetNowCursor(m_hovered);
        break;
    case MouseEvent::Event_MouseLeave:
        assert(m_pWindow && "no window no mouse");
        m_pWindow->SetNowCursor(nullptr);
        break;
    case MouseEvent::Event_MouseMove:
        // 按住左键不松手
        if (e.modifier & Modifier_LButton) {
            // 鼠标相对本控件位置
            Point2F pt_parent = { e.px, e.py };
            m_pParent->MapFromWindow(luiref pt_parent);
            pt_parent.x -= m_oBox.pos.x;
            pt_parent.y -= m_oBox.pos.y;
            // 计算并记录相对移动
            const auto ox = pt_parent.x - m_ptClickPos.x;
            const auto oy = pt_parent.y - m_ptClickPos.y;
            // 父节点发送事件
            assert(m_pParent && "no parent?!");
            m_pParent->DoEvent(this, EventSplitterArg(ox, oy, m_attribute));
        }
        break;
    case MouseEvent::Event_LButtonDown:
        // 记录开始拖拽的位置
        m_ptClickPos = { e.px, e.py };
        m_pParent->MapFromWindow(luiref m_ptClickPos);
        m_ptClickPos.x -= m_oBox.pos.x;
        m_ptClickPos.y -= m_oBox.pos.y;
        break;
    case MouseEvent::Event_LButtonUp:
        // 如果设定为弹起时候再处理事件?
        //if (m_attribute.dragoff) {

        //}
        break;
    }
    return Super::DoMouseEvent(e);
}

