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
    {
        const auto b = value.begin();
        const auto e = value.end();
        const auto t = ResourceType::Type_Image;
        const auto id = UIManager.LoadResource(b, e, t);
        // TODO: 错误处理
        assert(id && "bad id");
        m_idSrc.SetId(id);
        break;
    }
    default:
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UIImage::Recreate() noexcept->Result {
    // 获取Image对象
    if (m_idSrc.GetId()) {
        const auto obj = m_idSrc.GetResoureceObj();
        m_pSharedSrc = static_cast<CUIImage*>(obj);
        assert(m_pSharedSrc && "bug");
    }
    // 基类处理
    return Super::Recreate();
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
        assert(m_pSharedSrc && "bug");
        m_pSharedSrc->Render(
            UIManager.Ref2DRenderer()
        );
    }
}

