#include <core/ui_ctrlmeta.h>
#include <control/ui_caption.h>
#include <debugger/ui_debug.h>
#include <core/ui_color_list.h>
#include <constexpr/const_bkdr.h>


// ui namespace
namespace LongUI {
    // UICaption类 元信息
    LUI_CONTROL_META_INFO(UICaption, "caption");
}


/// <summary>
/// Initializes a new instance of the <see cref="UICaption" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UICaption::UICaption(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(parent, meta) {
    m_oStyle.appearance = Appearance_Caption;
    //this->SetBgColor({ RGBA_White });
}

/// <summary>
/// Finalizes an instance of the <see cref="UICaption"/> class.
/// </summary>
/// <returns></returns>
LongUI::UICaption::~UICaption() noexcept {
    // TODO: 取消groupbox引用
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UICaption::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给父节点
        key = BKDR_VALUE;
        [[fallthrough]];
    default:
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}

