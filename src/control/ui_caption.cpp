#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
#include <control/ui_caption.h>
#include <core/ui_color_list.h>
#include <control/ui_groupbox.h>
#include <constexpr/const_bkdr.h>
#include <core/ui_unsafe.h>

#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UICaption类 元信息
    LUI_CONTROL_META_INFO(UICaption, "caption");
}


/// <summary>
/// Initializes a new instance of the <see cref="UICaption" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UICaption::UICaption(const MetaControl& meta) noexcept 
#ifndef LUI_UICAPTION_AS_UILABEL
    : Super(meta), m_oLabel(this) {
    this->make_offset_tf_direct(m_oLabel);
#ifdef LUI_ACCESSIBLE
    // 结束逻辑控件
    m_pAccCtrl = &m_oLabel;
#endif
#else 
    : Super(meta) {
#endif
    m_oStyle.appearance = Appearance_Caption;
}

/// <summary>
/// Finalizes an instance of the <see cref="UICaption"/> class.
/// </summary>
/// <returns></returns>
LongUI::UICaption::~UICaption() noexcept {
    // 取消groupbox引用
    if (m_pGroupBox) m_pGroupBox->CaptionRemoved(*this);
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
#ifdef LUI_UICAPTION_AS_UILABEL
        // 传递给 label
        key = BKDR_VALUE;
        [[fallthrough]];
#else
        // 传递给 超类
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        return;
#endif
        
    default:
        // 其他情况, 交给超类处理
        return Super::add_attribute(key, value);
    }
}

#ifndef LUI_UICAPTION_AS_UILABEL


/// <summary>
/// initialize UICaption
/// </summary>
/// <returns></returns>
void LongUI::UICaption::UICaption::initialize() noexcept {
    // 添加了复杂控件
    if (this->GetChildrenCount() > 1) {
        m_oLabel.SetVisible(false);
#ifdef LUI_ACCESSIBLE
        // 再度拥有逻辑控件
        m_pAccCtrl = this;
#endif
    }
    // 超类处理
    return Super::initialize();
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UICaption::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged) {
        m_oLabel.Update(r);
    }
    Super::Update(reason);
}

#endif