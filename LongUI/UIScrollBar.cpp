#include "LongUI.h"

// TODO: 滚动条优化
//std::atomic_uint32_t g_cScrollBarCount = 0;
//ID2D1PathGeometry*  g_pScrollBar1 = nullptr;
//ID2D1PathGeometry*  g_pScrollBar2 = nullptr;

// 创建描述
auto LongUI::UIScrollBar::CreateDesc(const char * attr, ScrollBarType type) noexcept -> const ScrollBarDesc &{
    static ScrollBarDesc s_desc;
    s_desc.type = type;
    s_desc.size = 16.f;
    return s_desc;
}

// UIScrollBar 构造函数
LongUI::UIScrollBar::UIScrollBar(pugi::xml_node node) noexcept: Super(node) {
    ZeroMemory(&m_desc, sizeof(m_desc));
    /*char32_t buffer[] = {
        9652,    // UP
        9662,    // DOWN
        9666,    // LEFT
        9656,    // RIGHT
    };
    // 获取DWrite工厂
    auto format = UIManager.GetTextFormat(LongUIDefaultTextFormatIndex);
    IDWriteFontFace* fontface = nullptr;
    // 最低位为1即水平
    if (desc.type != ScrollBarType::Type_Vertical) {
        CUIManager::CreateTextPathGeometry(
            buffer + 2, 1,
            format,
            UIManager_D2DFactory,
            &fontface,
            &m_pArrow1Text
            );
        CUIManager::CreateTextPathGeometry(
            buffer + 3, 1,
            format,
            UIManager_D2DFactory,
            &fontface,
            &m_pArrow2Text
            );
    }
    // 最低位为0即垂直
    else {
        CUIManager::CreateTextPathGeometry(
            buffer + 0, 1,
            format,
            UIManager_D2DFactory,
            &fontface,
            &m_pArrow1Text
            );
        CUIManager::CreateTextPathGeometry(
            buffer + 1, 1,
            format,
            UIManager_D2DFactory,
            &fontface,
            &m_pArrow2Text
            );
    }
    ::SafeRelease(format);
    ::SafeRelease(fontface);*/
}

// UIScrollBar 析构函数
LongUI::UIScrollBar::~UIScrollBar() noexcept {
    ::SafeRelease(m_pArrow1Text);
    ::SafeRelease(m_pArrow2Text);
    ::SafeRelease(m_pBrush);
}

// 更新值
void LongUI::UIScrollBar::Refresh() noexcept {
#if 0
    // 边界 > 显示  -> 刻画边界 = 边界
    // 另外:      -> 刻画边界 = 显示
    bool old = false;


    // 垂直?
    if (this->desc.type == ScrollBarType::Type_Vertical) {
        // 更新
        if ((old = m_pOwner->end_of_bottom > m_pOwner->show_zone.height)) {
            m_pOwner->draw_zone.height = m_pOwner->end_of_bottom;
        }
        else {
            m_pOwner->draw_zone.height = m_pOwner->show_zone.height;
        }
        this->max_range = m_pOwner->draw_zone.height;
        // 不同?
        if (m_bEffective != old) {
            m_bEffective = old;
            m_pOwner->RefreshChildLayout(false);
        }
        // 检查上边界

        // 检查下边界
        /*auto lower = m_pOwner->show_zone.top + m_pOwner->show_zone.height;
        if (m_pOwner->draw_zone.top + m_pOwner->draw_zone.height < lower) {
            m_pOwner->draw_zone.top = lower - m_pOwner->draw_zone.height;
        }*/
    }
    // 水平?
    else {
        // 更新
        if ((m_bEffective = m_pOwner->end_of_right > m_pOwner->show_zone.width)) {
            m_pOwner->draw_zone.width = m_pOwner->end_of_right;
        }
        else {
            m_pOwner->draw_zone.width = m_pOwner->show_zone.width;
        }
        this->max_range = m_pOwner->draw_zone.width;
        // 检查左边界

        // 检查右边界
        /*auto right = m_pOwner->show_zone.left + m_pOwner->show_zone.width;
        if (m_pOwner->draw_zone.left + m_pOwner->draw_zone.width < right) {
            m_pOwner->draw_zone.left = right - m_pOwner->draw_zone.width;
        }*/
    }
    // TODO: 更新滚动条状态
#endif
}

// UIScrollBar 渲染 
auto  LongUI::UIScrollBar::Render(RenderType type) noexcept ->HRESULT {
#if 0
    register float tmpsize = this->desc.size;
    D2D1_RECT_F draw_rect = GetDrawRect(this);
    m_rtThumb = m_rtArrow2 =  m_rtArrow1 = draw_rect;
    // 垂直滚动条
    if (this->desc.type == ScrollBarType::Type_Vertical) {
        m_rtArrow1.bottom = m_rtArrow1.top + tmpsize;
        m_rtArrow2.top = m_rtArrow2.bottom - tmpsize;
        // 计算Thumb
        register auto height = m_pOwner->show_zone.height - tmpsize;
        register auto bilibili = height / this->max_range;
        m_rtThumb.top = (this->index * bilibili + m_rtArrow1.bottom);
        m_rtThumb.bottom = (m_rtThumb.top + bilibili * height);
        //assert(m_rtThumb.bottom <= m_rtArrow2.top);
    }
    // 水平滚动条
    else {
        m_rtArrow1.right = m_rtArrow1.left + tmpsize;
        m_rtArrow2.left = m_rtArrow2.right - tmpsize;
        // 计算Thumb
        register auto width = m_pOwner->show_zone.width;
        register auto bilibili = width / this->max_range;
        m_rtThumb.left = this->index * bilibili;
        m_rtThumb.right = m_rtThumb.left + bilibili * width;
        //assert(m_rtThumb.right <= m_rtArrow2.left);
    }
    // 先画一个矩形
    m_pRenderTarget->DrawRectangle(&m_rtArrow1, m_pBrush, 1.f);
    // -------------
    m_pRenderTarget->FillRectangle(&m_rtThumb, m_pBrush);
    // -------------
    // 先画一个矩形
    m_pRenderTarget->DrawRectangle(&m_rtArrow2, m_pBrush, 1.f);
    // 修改转变矩阵
#endif
    return S_OK;
}


// UIScrollBar 渲染 
auto LongUI::UIScrollBar::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    ::SafeRelease(m_pBrush);
    // 设置新的笔刷
    m_pBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    return Super::Recreate(newRT);
}

// do event 事件处理
bool  LongUI::UIScrollBar::DoEvent(LongUI::EventArgument& arg) noexcept {
    
    // 控件消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_FindControl:
            if (IsPointInRect(this->show_zone, arg.pt)) {
                arg.ctrl = this;
            }
            __fallthrough;
            /*case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        case LongUI::Event::Event_MouseEnter:
            (L"<%S>: MouseEnter\n", __FUNCTION__);
            break;*/
        case LongUI::Event::Event_MouseLeave:
            m_pointType = PointType::Type_None;
            break;
        }
    }
    // 系统消息
    else {
        switch (arg.msg) {
        case WM_MOUSEMOVE:
            // 检查指向类型
            if (IsPointInRect(m_rtArrow1, arg.pt)) {
                m_pointType = PointType::Type_Arrow1;
            }
            else if (IsPointInRect(m_rtArrow2, arg.pt)) {
                m_pointType = PointType::Type_Arrow2;
            }
            else if (IsPointInRect(m_rtThumb, arg.pt)) {
                m_pointType = PointType::Type_Thumb;
            }
            else {
                m_pointType = PointType::Type_Shaft;
            }
            break;
        }
    }
    return false;
}


// UIScrollBar 关闭控件
void  LongUI::UIScrollBar::Close() noexcept {
    delete this;
}