// ui
#include <core/ui_ctrlmeta.h>
#include <core/ui_manager.h>
#include <control/ui_image.h>
#include <container/pod_hash.h>
#include <resource/ui_image_res.h>


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
    : Super(impl::ctor_lock(parent), meta) {
    // 构造锁
    impl::ctor_unlock();
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
//auto LongUI::UIImage::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
//    // 左键弹起 修改状态
//    switch (e.type)
//    {
//    case LongUI::MouseEvent::Event_LButtonUp:
//        assert(m_pParent);
//        m_pParent->DoEvent(this, { NoticeEvent::Event_ImageChildClicked });
//        [[fallthrough]];
//    default:
//        return Super::DoMouseEvent(e);
//    }
//}

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
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// Sets the source.
/// </summary>
/// <param name="src">The source.</param>
void LongUI::UIImage::SetSource(U8View src) noexcept {
    constexpr auto RESTYPE = ResourceType::Type_Image;
    const auto id = UIManager.LoadResource(src, /*RESTYPE,*/ true);
#ifndef NDEBUG
    if (!id && src.end() != src.begin()) {
        LUIDebug(Error) LUI_FRAMEID
            << "Resource [not found][create failed]: "
            << src
            << endl;
    }
#endif
    // 重置数据
    if (id != m_idSrc.GetId()) {
        m_idSrc.SetId(id);
        m_idFrame = 0;
        m_uFrameCount = 1;
        if (id) {
            auto& res = m_idSrc.RefResource();
            auto& img = static_cast<CUIImage&>(res);
            m_uFrameCount = img.frame_count;
            if (m_uFrameCount > 1) this->SetTimer(img.delay, 0);
        }
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
        // IMAGE最小尺寸就是图片大小
        minsize_set = { 0.f };
        if (m_idSrc.GetId()) {
            auto& res = m_idSrc.RefResource();
            auto& img = static_cast<CUIImage&>(res);
            assert(res.RefData().GetType() == ResourceType::Type_Image);
            minsize_set.width = static_cast<float>(img.size.width);
            minsize_set.height = static_cast<float>(img.size.height);
        }
        this->set_contect_minsize(minsize_set);
        return Event_Accept;
    case LongUI::NoticeEvent::Event_Timer0:
        // TIMER#0 作为帧动画使用
        m_idFrame = (m_idFrame + 1) % m_uFrameCount;
        this->Invalidate();
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
    if (m_idSrc.GetId()) {
        // 将目标画在内容区
        auto des_rect = this->GetBox().GetContentEdge();
        // 居中显示
#ifdef LUI_IMAGE_ASICON_SUPPORT
        if (m_bAsIcon) {
            const auto usize = m_pSharedSrc->GetSize();
            const auto src_w = static_cast<float>(usize.width);
            const auto src_h = static_cast<float>(usize.height);
            const auto des_w = des_rect.right - des_rect.left;
            const auto des_h = des_rect.bottom - des_rect.top;

            const auto target_h = des_w / src_w * src_h;
            // [target_w, des_h]
            if (target_h > des_h) {
                const auto target_w = des_h / src_h * src_w;
                const auto half = (des_w - target_w) * 0.5f;
                des_rect.left += half;
                des_rect.right -= half;
            }
            // [des_w, target_h]
            else {
                const auto half = (des_h - target_h) * 0.5f;
                des_rect.top += half;
                des_rect.bottom -= half;
            }
        }
#endif
        auto& res = m_idSrc.RefResource();
        auto& img = static_cast<const CUIImage&>(res);
        assert(res.RefData().GetType() == ResourceType::Type_Image);
        img.Render(m_idFrame, UIManager.Ref2DRenderer(), &des_rect);
    }
}

