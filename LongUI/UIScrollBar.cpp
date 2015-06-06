#include "LongUI.h"


enum class Unit : size_t {
    Unknown = 0,
    Bitmap ,
    ColorRect,
};

template<Unit... Elements> class Units;
template<Unit Head, Unit... Tail>
class Units<Head, Tail...> : protected Units<Tail...> {
    using Super = Units<Tail...>;
protected:
    Units<Head> head;
public:
    // ctor
    Units(pugi::xml_node node) : Super(node), head(node) {};
    // set unit type
    inline void SetUnitType(Unit unit) { m_type = unit; }
    // render this
    void Render(float x, float y) {
        if (this->now_unit == Head) {
            head.Render(x, y);
        }
        else {
            Super::Render(x, y);
        }
    }
};

template<> class Units<> {
protected:
    Unit        m_type = Unit::Unknown;
};

template<> class Units<Unit::Bitmap> : public Units<> {
public:
    int a;
};

template<> class Units<Unit::ColorRect> : public Units<>{
public:
    float b;
};



// TODO: 滚动条优化
//std::atomic_uint32_t g_cScrollBarCount = 0;
//ID2D1PathGeometry*  g_pScrollBar1 = nullptr;
//ID2D1PathGeometry*  g_pScrollBar2 = nullptr;

// UIScrollBar 构造函数
LongUI::UIScrollBar::UIScrollBar(pugi::xml_node node) noexcept: Super(node) {
    Units<Unit::Bitmap, Unit::ColorRect> unit;
    unit.now_unit = Unit::ColorRect;
    unit.Render(0.f, 0.f);

    constexpr int azz = sizeof(unit);
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


// 更新值
void LongUI::UIScrollBar::Refresh() noexcept {
    // 边界 > 显示  -> 刻画边界 = 边界
    // 另外:      -> 刻画边界 = 显示
    bool old = false;

    // 垂直?
    if (this->type == ScrollBarType::Type_Vertical) {
        // 更新
        if ((old = m_pOwner->end_of_bottom > m_pOwner->show_zone.height)) {
            m_pOwner->draw_zone.height = m_pOwner->end_of_bottom;
        }
        else {
            m_pOwner->draw_zone.height = m_pOwner->show_zone.height;
        }
        m_fMaxRange = m_pOwner->draw_zone.height;
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
        if ((old = m_pOwner->end_of_right > m_pOwner->show_zone.width)) {
            m_pOwner->draw_zone.width = m_pOwner->end_of_right;
        }
        else {
            m_pOwner->draw_zone.width = m_pOwner->show_zone.width;
        }
        m_fMaxRange = m_pOwner->draw_zone.width;
        // 检查左边界

        // 检查右边界
        /*auto right = m_pOwner->show_zone.left + m_pOwner->show_zone.width;
        if (m_pOwner->draw_zone.left + m_pOwner->draw_zone.width < right) {
            m_pOwner->draw_zone.left = right - m_pOwner->draw_zone.width;
        }*/
    }
    // TODO: 更新滚动条状态
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
        // 鼠标移上

    }
    return false;
}

// UIScrollBarA 渲染 
auto LongUI::UIScrollBarA::Render(RenderType type) noexcept -> HRESULT {
    if (type != RenderType::Type_Render) return S_FALSE;
    register float tmpsize = this->GetTakingUpSapce();
    D2D1_RECT_F draw_rect = GetDrawRect(this);
    // 双滚动条修正
    if (this->another) {
        if (this->type == ScrollBarType::Type_Vertical) {
            draw_rect.bottom -= this->another->GetTakingUpSapce();
        }
        else {
            draw_rect.right -= this->another->GetTakingUpSapce();
        }
    }
    m_rtThumb = m_rtArrow2 = m_rtArrow1 = draw_rect;
    // 垂直滚动条
    if (this->type == ScrollBarType::Type_Vertical) {
        m_rtArrow1.bottom = m_rtArrow1.top + tmpsize;
        m_rtArrow2.top = m_rtArrow2.bottom - tmpsize;
        // 计算Thumb
        register auto height = m_pOwner->show_zone.height - tmpsize*2.f;
        register auto bilibili = height / m_fMaxRange;
        m_rtThumb.top = (m_fIndex * bilibili + m_rtArrow1.bottom);
        m_rtThumb.bottom = (m_rtThumb.top + bilibili * height);
        //assert(m_rtThumb.bottom <= m_rtArrow2.top);
    }
    // 水平滚动条
    else {
        m_rtArrow1.right = m_rtArrow1.left + tmpsize;
        m_rtArrow2.left = m_rtArrow2.right - tmpsize;
        // 计算Thumb
        register auto width = m_pOwner->show_zone.width - tmpsize*2.f;
        register auto bilibili = width / m_fMaxRange;
        m_rtThumb.left = m_fIndex * bilibili + m_rtArrow1.right;
        m_rtThumb.right = m_rtThumb.left + bilibili * width;
        //assert(m_rtThumb.right <= m_rtArrow2.left);
    }
    // 背景
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(0xF0F0F0));
    m_pRenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
    // thumb
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
    m_pRenderTarget->FillRectangle(&m_rtThumb, m_pBrush_SetBeforeUse);
    //m_pRenderTarget->FillRectangle(&m_rtArrow1, m_pBrush_SetBeforeUse);
    //m_pRenderTarget->FillRectangle(&m_rtArrow2, m_pBrush_SetBeforeUse);
    return S_OK;
}


// UIScrollBarA::do event 事件处理
bool  LongUI::UIScrollBarA::DoEvent(LongUI::EventArgument& arg) noexcept {
    // 控件消息
    if (arg.sender) {

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
            return true;
        }
    }
    return Super::DoEvent(arg);
}

/*/ UIScrollBaAr 重建 
auto LongUI::UIScrollBarA::Recreate(LongUIRenderTarget* newRT) noexcept ->HRESULT {
    ::SafeRelease(m_pBrush);
    // 设置新的笔刷
    m_pBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    return Super::Recreate(newRT);
}*/

// UIScrollBarA: 需要
void LongUI::UIScrollBarA::OnNeeded(bool need) noexcept {
    m_fTakeSpace = need ? 16.f : 0.f;
    m_fHitSpace = m_fTakeSpace;
    // 检查
}

// UIScrollBarA 析构函数
inline LongUI::UIScrollBarA::~UIScrollBarA() noexcept {
    ::SafeRelease(m_pArrow1Text);
    ::SafeRelease(m_pArrow2Text);
    ::SafeRelease(m_pBrush);
}

// UIScrollBarA 关闭控件
void  LongUI::UIScrollBarA::Close() noexcept {
    delete this;
}

// create 创建
auto WINAPI LongUI::UIScrollBarA::CreateControl(pugi::xml_node node) noexcept ->UIControl* {
    // 申请空间
    auto pControl = LongUI::UIControl::AllocRealControl<LongUI::UIScrollBarA>(
        node,
        [=](void* p) noexcept { new(p) UIScrollBarA(node); }
    );
    if (!pControl) {
        UIManager << DL_Error << L"alloc null" << LongUI::endl;
    }
    return pControl;
}
