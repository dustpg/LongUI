// ui
#include <core/ui_manager.h>
#include <control/ui_image.h>
#include <resource/ui_image.h>
#include <container/pod_hash.h>
#include <control/ui_ctrlmeta.h>


// ui namespace
namespace LongUI {
    // UIImage类 元信息
    LUI_CONTROL_META_INFO(UIImage, "image");
}

/// <summary>
/// Initializes a new instance of the <see cref="UIImage" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIImage::UIImage(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {

}


/// <summary>
/// Finalizes an instance of the <see cref="UIImage"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIImage::~UIImage() noexcept {
}



/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIImage::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    // 左键弹起 修改状态
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonUp:
        assert(m_pParent);
        m_pParent->DoEvent(this, { NoticeEvent::Event_ImageChildClicked });
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIImage::add_attribute(uint32_t key, U8View value) noexcept {
    // 本控件添加属性
    constexpr auto BKDR_SRC = 0x001E57C4_ui32;
    // 计算HASH
    switch (key)
    {
    case BKDR_SRC:
        // src  : 图像源
        this->SetSource(value);
        break;
    default:
        // 父类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// Sets the source.
/// </summary>
/// <param name="src">The source.</param>
void LongUI::UIImage::SetSource(U8View src) noexcept {
    const auto old_src = m_pSharedSrc;
    constexpr auto RESTYPE = ResourceType::Type_Image;
    const auto id = UIManager.LoadResource(src, RESTYPE, true);
#ifndef NDEBUG
    if (!id && src.end() != src.begin()) {
        LUIDebug(Error) LUI_FRAMEID
            << "Resource not found: "
            << src
            << endl;
    }
#endif
    // 重置数据
    m_pSharedSrc = nullptr;
    m_idSrc.SetId(id);
    // 数据有效
    if (id) {
        assert(m_idSrc.GetResoureceType() == RESTYPE);
        const auto obj = m_idSrc.GetResoureceObj();
        m_pSharedSrc = static_cast<CUIImage*>(obj);
        assert(m_pSharedSrc && "bug");
    }
    // 要求重新计算以及重绘
    if (m_pSharedSrc != old_src) {
        this->mark_window_minsize_changed();
        this->NeedUpdate();
        this->Invalidate();
    }
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
//auto LongUI::UIImage::Recreate() noexcept->Result {
//    // 获取Image对象
//    if (m_idSrc.GetId()) {
//    }
//    // 基类处理
//    return Super::Recreate();
//}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIImage::DoEvent(UIControl* sender, const EventArg& e) noexcept -> EventAccept {
    switch (e.nevent)
    {
        Size2F minsize_set;
    case LongUI::NoticeEvent::Event_RefreshBoxMinSize:
        minsize_set = { 0.f };
        if (m_pSharedSrc) {
            const auto size = m_pSharedSrc->GetSize();
            minsize_set.width = static_cast<float>(size.width);
            minsize_set.height = static_cast<float>(size.height);
        }
        this->set_contect_minsize(minsize_set);
        return Event_Accept;
    }
    return Super::DoEvent(sender, e);
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIImage::Render() const noexcept {
    // 渲染背景
    Super::Render();
    // 图像有效
    if (m_pSharedSrc) {
        // 将目标画在内容区
        const auto des_rect = this->GetBox().GetContentEdge();
        m_pSharedSrc->Render(UIManager.Ref2DRenderer(), &des_rect);
    }
}

