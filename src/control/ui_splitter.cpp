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
            m_pParent->MapFromWindow(pt_parent);
            // 计算并记录相对移动
            const auto ox = pt_parent.x - m_ptLastPos.x;
            const auto oy = pt_parent.y - m_ptLastPos.y;
            m_ptLastPos = pt_parent;
            // 父节点发送事件
            assert(m_pParent && "no parent?!");
            m_pParent->DoEvent(this, EventSplitterArg{ ox, oy });
        }
        break;
    case MouseEvent::Event_LButtonDown:
        // 记录开始拖拽的位置
        m_ptLastPos = { e.px, e.py };
        m_pParent->MapFromWindow(m_ptLastPos);
        break;
    }
    return Super::DoMouseEvent(e);
}

