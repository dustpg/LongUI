// ui
#include <core/ui_manager.h>
#include <text/ui_ctl_impl.h>
#include <input/ui_kminput.h>
#include <util/ui_ctordtor.h>
#include <util/ui_clipboard.h>
#include <core/ui_color_list.h>
#include <control/ui_textbox.h>
#include <graphics/ui_graphics_impl.h>

// TextBC
#include <../TextBC/bc_txtdoc.h>

// C++
#include <type_traits>


// error beep
extern "C" void longui_error_beep();

// uinamespace
namespace LongUI {
    // auto cst TextBC -> LongUI
    auto auto_cast(TextBC::IBCTextContent* ptr) noexcept {
        return reinterpret_cast<I::Text*>(ptr);
    }
    // auto cst LongUI -> TextBC
    auto auto_cast(I::Text* ptr) noexcept {
        return reinterpret_cast<TextBC::IBCTextContent*>(ptr);
    }
    // private text box
    struct UITextBox::Private {
        // doc type
        using doc_t = TextBC::CBCTextDocument;
        // ctor
        Private() noexcept;
        // dtor
        ~Private() noexcept { if (created) this->release_doc(); }
        // recreate cached bitmap
        auto recrete_cached_bitmap() noexcept->Result;
        // get doc object
        auto&document() noexcept { return reinterpret_cast<doc_t&>(docbuf); }
        // create doc
        void create_doc(UITextBox& box) noexcept;
        // release doc
        void release_doc() noexcept;

        // document() buffer
        std::aligned_storage<sizeof(doc_t), alignof(doc_t)>
            ::type                          docbuf;
        // cluster buffer 
        POD::Vector<DWRITE_CLUSTER_METRICS> cluster_buffer;
        // text
        CUIString                           text_cached;
        // text cached synchronized
        bool                                text_need_sync = false;
        // selection cached synchronized
        bool                                selc_need_sync = false;
        // created
        bool                                created = false;
        // cached
        bool                                cached = false;
    };
    /// <summary>
    /// Privates the text box.
    /// </summary>
    /// <param name="box">The box.</param>
    /// <returns></returns>
    UITextBox::Private::Private() noexcept {
        static_assert(alignof(Private) <= alignof(double), "must less than double");
    }
    /// <summary>
    /// Recretes the cached bitmap.
    /// </summary>
    /// <returns></returns>
    auto UITextBox::Private::recrete_cached_bitmap() noexcept -> Result {
        return { Result::RS_OK };
    }
    /// <summary>
    /// Releases the document().
    /// </summary>
    /// <returns></returns>
    void UITextBox::Private::release_doc() noexcept {
        detail::ctor_dtor<doc_t>::delete_obj(&docbuf);
    }
    /// <summary>
    /// Creates the document().
    /// </summary>
    /// <param name="box">The box.</param>
    /// <returns></returns>
    void UITextBox::Private::create_doc(UITextBox& box) noexcept {
        const TextBC::CBCTextDocument::InitArgs args{
            box.m_uMaxLength,
            static_cast<TextBC::CBCTextDocument::Flag>(box.m_flag),
            box.m_chPassword
        };
        TextBC::IBCTextPlatform& plat = box;
        detail::ctor_dtor<doc_t>::create(&docbuf, plat, args);
        created = true;
    }
}



/// <summary>
/// Recreates this instance.
/// </summary>
/// <returns></returns>
auto LongUI::UITextBox::Recreate() noexcept -> Result {
    // 重建父类设备资源
    Result hr = Super::Recreate();
    // 使用了缓存?
    if (hr && m_private->cached) {
        // 重建待使用位图
        hr = m_private->recrete_cached_bitmap();
    }
    // 父类
    return hr;
}

/// <summary>
/// Ons the character input.
/// </summary>
/// <param name="ch">The char</param>
/// <returns></returns>
void LongUI::UITextBox::private_char(char32_t ch) noexcept {
    assert(m_private && "OOM");
    m_private->document().OnChar(ch);
}

/// <summary>
/// Creates the private.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::create_private() noexcept {
    assert(!m_private);
    m_private = new(std::nothrow) Private{};
    this->ctor_failed_if(m_private);
}

/// <summary>
/// Initializes the private.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::init_private() noexcept {
    assert(m_private && "bad action");
    m_private->create_doc(*this);
    auto& cached = m_private->text_cached;
    if (!cached.empty()) this->SetText(cached.view());
}


/// <summary>
/// Deletes the private.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::delete_private() noexcept {
    if (m_private) delete m_private;
}


/// <summary>
/// Privates the use cached.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_use_cached() noexcept {
    m_private->cached = true;
}

/// <summary>
/// Privates the set text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UITextBox::private_set_text(CUIString&& text) noexcept {
    auto& cached = m_private->text_cached;
    cached = std::move(text);
}

/// <summary>
/// Privates the mark readonly.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_mark_readonly() noexcept {
    m_flag |= TextBC::CBCTextDocument::Flag_ReadOnly;
}


/// <summary>
/// Privates the mark multiline.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_mark_multiline() noexcept {
    m_flag |= TextBC::CBCTextDocument::Flag_MultiLine;
}

/// <summary>
/// Privates the mark password.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_mark_password() noexcept {
    m_flag |= TextBC::CBCTextDocument::Flag_UsePassword;
}


/// <summary>
/// Determines whether [is valid password] [the specified ch].
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
bool LongUI::UITextBox::IsValidPassword(char16_t ch) noexcept {
    return ch > 0x20 && ch < 0x80;
}


/// <summary>
/// Errors the beep.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::ErrorBeep() noexcept {
    ::longui_error_beep();
}


/// <summary>
/// Privates the update.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_update() noexcept {
    auto& doc = m_private->document();
    // 文本修改检查
    if (doc.IsTextChanged()) {
        doc.ClearTextChanged();
        m_private->text_need_sync = true;
        this->TriggrtEvent(_textChanged());
    }
    // 选区修改检查
    if (doc.IsSelectionChanged()) {
        doc.ClearSelectionChanged();
        m_private->selc_need_sync = true;
        this->TriggrtEvent(_selectionChanged());
    }
}

/// <summary>
/// Privates the resize.
/// </summary>
/// <param name="size">The size.</param>
/// <returns></returns>
void LongUI::UITextBox::private_resize(Size2F size) noexcept {
    m_private->document().SetViewportSize({ size.width, size.height });
}



/// <summary>
/// Privates the mouse down.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="shift">if set to <c>true</c> [shift].</param>
/// <returns></returns>
void LongUI::UITextBox::private_mouse_down(Point2F pos, bool shift) noexcept {
    auto& doc = m_private->document();
    const auto lt = this->GetBox().GetContentPos();
    doc.OnLButtonDown({ pos.x - lt.x, pos.y - lt.y }, shift);
}

/// <summary>
/// Privates the mouse up.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void LongUI::UITextBox::private_mouse_up(Point2F pos) noexcept {
    auto& doc = m_private->document();
    const auto lt = this->GetBox().GetContentPos();
    doc.OnLButtonUp({ pos.x - lt.x, pos.y - lt.y });
}


/// <summary>
/// Privates the mouse move.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void LongUI::UITextBox::private_mouse_move(Point2F pos) noexcept {
    auto& doc = m_private->document();
    const auto lt = this->GetBox().GetContentPos();
    doc.OnLButtonHold({ pos.x - lt.x, pos.y - lt.y });
}



#ifdef LUI_TEXTBOX_USE_UNIFIED_INPUT
/// <summary>
/// Privates the left.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_left() noexcept {
    const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnLeft(ctrl, shift);
}

/// <summary>
/// Privates the right.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_right() noexcept {
    const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnRight(ctrl, shift);
}

/// <summary>
/// Privates up.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_up() noexcept {
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnUp(shift);
}

/// <summary>
/// Privates down.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::private_down() noexcept {
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    m_private->document().OnDown(shift);
}
#endif
/// <summary>
/// Privates the keydown.
/// </summary>
/// <param name="key">The key.</param>
/// <returns></returns>
void LongUI::UITextBox::private_keydown(uint32_t key) noexcept {
    auto& doc = m_private->document();
    const auto ctrl = CUIInputKM::GetKeyState(CUIInputKM::KB_CONTROL);
    const auto shift = CUIInputKM::GetKeyState(CUIInputKM::KB_SHIFT);
    switch (static_cast<CUIInputKM::KB>(key))
    {
    case CUIInputKM::KB_BACK:
        // 退格键
        doc.OnBackspace(ctrl);
        break;
    case CUIInputKM::KB_DELETE:
        // 删除键
        doc.OnDelete(ctrl);
        break;
    case CUIInputKM::KB_LEFT:
        // 左箭头键
        doc.OnLeft(ctrl, shift);
        break;
    case CUIInputKM::KB_RIGHT:
        // 右箭头键
        doc.OnRight(ctrl, shift);
        break;
    case CUIInputKM::KB_UP:
        // 上箭头键
        doc.OnUp(shift);
        break;
    case CUIInputKM::KB_DOWN:
        // 下箭头键
        doc.OnDown(shift);
        break;
    case CUIInputKM::KB_RETURN:
        // 回车返回键
        doc.OnNewLine();
        break;
    case CUIInputKM::KB_A:
        // A 键
        if (ctrl) doc.OnSelectAll();
        break;
    case CUIInputKM::KB_Z:
        if (ctrl) doc.Undo();
        break;
    case CUIInputKM::KB_Y:
        if (ctrl) doc.Redo();
        break;
    case CUIInputKM::KB_X:
    case CUIInputKM::KB_C:
        // X, C 键
        if (ctrl) {
            // 获取选中文本
            CUIString text;
            doc.RequestSelected(text);
            // 无选择或其他原因
            if (text.empty()) break;
            LongUI::CopyTextToClipboard(text.view());
            // ctrl-x 是剪切
            if (key == CUIInputKM::KB_X)
                doc.DeleteSelection();
        }
        break;
    case CUIInputKM::KB_V:
        // V 键
        if (ctrl) {
            CUIString text;
            LongUI::PasteTextToClipboard(text);
            static_assert(sizeof(*text.data()) == sizeof(char16_t), "");
            const auto ptr = reinterpret_cast<const char16_t*>(text.c_str());
            doc.OnText({ ptr,  ptr + text.length() });
        }
        break;
    }
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::Render() const noexcept {
    Super::Render();
    auto& ctx = UIManager.Ref2DRenderer();
    // 设置渲染偏移
    const auto lt = this->GetBox().GetContentPos();
    Matrix3X2F matrix;
    ctx.GetTransform(&auto_cast(matrix));
    matrix._31 += lt.x;
    matrix._32 += lt.y;
    ctx.SetTransform(&auto_cast(matrix));
    // 设置剪切区域
    const auto sz = this->GetBox().GetContentSize();
    const auto mode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
    ctx.PushAxisAlignedClip({ 0, 0, sz.width, sz.height }, mode);
    // TODO: 是移动到渲染线程加UI锁?
    //       还是移动到UI线程加渲染锁?
    //       还是用一个自己的锁?
    {
        CUIDataAutoLocker locker;
        m_private->document().Render(&ctx, nullptr);
    }
    // 弹出剪切区域
    ctx.PopAxisAlignedClip();
}

/// <summary>
/// Needs the redraw.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::GenerateText(void* string, TextBC::U16View view) noexcept {
    auto& text = *reinterpret_cast<CUIString*>(string);
    using type1 = decltype(*text.c_str());
    using type2 = decltype(*view.first);
    static_assert(sizeof(type1) == 2, "must be same");
    const auto a = reinterpret_cast<decltype(text.c_str())>(view.first);
    const auto b = reinterpret_cast<decltype(text.c_str())>(view.second);
    text.append(a, b);
}


/// <summary>
/// Requests the text.
/// </summary>
/// <returns></returns>
const LongUI::CUIString& LongUI::UITextBox::RequestText() noexcept {
    auto& doc = m_private->document();
    auto& str = m_private->text_cached;
    auto& flag = m_private->text_need_sync;
    if (flag) {
        flag = false;
        str.reserve(doc.GetStringLength());
        str.clear();
        doc.RequestText(str);
    }
    return str;
}

/// <summary>
/// Needs the redraw.
/// </summary>
/// <returns></returns>
void LongUI::UITextBox::NeedRedraw() noexcept {
    // TODO: 脏矩形更新
    this->Invalidate();
}

/// <summary>
/// Draws the caret.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="offset">The offset.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void LongUI::UITextBox::DrawCaret(void* ctx, TextBC::Point2F offset, const TextBC::RectWHF& rect) noexcept {
    assert(ctx && "bad context");
    // 没有焦点 没有头发
    if (!m_oStyle.state.focus) return;
    const auto renderer = static_cast<I::Renderer2D*>(ctx);
    const auto red = ColorF::FromRGBA_CT<RGBA_Red>();
    const auto x = rect.x + offset.x;
    const auto y = rect.y + offset.y;
    renderer->FillRectangle({
        x, y, rect.x + rect.width, rect.y + rect.height
    }, &UIManager.RefCCBrush(red));

}



/// <summary>
/// Draws the selection.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="offset">The offset.</param>
/// <param name="rects">The rects.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void LongUI::UITextBox::DrawSelection(void* ctx, 
    TextBC::Point2F offset, 
    const TextBC::RectWHF rects[], uint32_t len) noexcept {
    assert(ctx && "bad context");
    const auto renderer = static_cast<I::Renderer2D*>(ctx);
    const auto color = ColorF::FromRGBA_CT<RGBA_TianyiBlue>();
    const auto brush = &UIManager.RefCCBrush(color);
    // 渲染矩形
    for (uint32_t i = 0; i!= len; ++i) {
        const auto l = rects[i].x + offset.x;
        const auto t = rects[i].y + offset.y;
        const auto r = l + rects[i].width;
        const auto b = t + rects[i].height;
        renderer->FillRectangle({ l, t, r, b }, brush);
    }
}


/// <summary>
/// Creates the content.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <param name="old">The old ptr.</param>
/// <returns></returns>
auto LongUI::UITextBox::CreateContent(
    const char16_t* str,
    uint32_t len,
    Text&& old
) noexcept -> Text* {
    const auto last_text = auto_cast(&old);
    I::Text* text = nullptr;
    TextArg arg;
    static_assert(sizeof(*arg.string) == sizeof(char16_t), "unsupprted platform");
    // 参数调整
    arg.string = reinterpret_cast<decltype(arg.string)>(str);
    arg.length = len;
    arg.font = I::FontFromText(last_text);
    arg.mwidth = DEFAULT_CONTROL_MAX_SIZE;
    arg.mheight = DEFAULT_CONTROL_MAX_SIZE;
    // TODO: 错误处理
    UIManager.CreateCtlText(arg, luiref text);
    // 释放旧数据
    if (last_text) last_text->Release();
    return auto_cast(text);
}


/// <summary>
/// Deletes the content.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::UITextBox::DeleteContent(Text& text) noexcept {
    const auto ptr = auto_cast(&text);
    ptr->Release();
}

/// <summary>
/// Draws the content.
/// </summary>
/// <param name="txt">The text.</param>
/// <param name="ctx">The CTX.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
void LongUI::UITextBox::DrawContent(Text& txt, void* ctx, TextBC::Point2F pos) noexcept {
    const auto ptr = auto_cast(&txt);
    assert(ctx && "bad context");
    const auto renderer = static_cast<I::Renderer2D*>(ctx);
    // 错误场合处理
    if (ptr) {
        const auto color = ColorF::FromRGBA_CT<RGBA_Black>();
        auto& brush = UIManager.RefCCBrush(color);
        renderer->DrawTextLayout({ pos.x, pos.y }, ptr, &brush);
    }
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UITextBox::SetText(WcView text) noexcept {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "must be same");
    const auto a = reinterpret_cast<const char16_t*>(text.begin());
    const auto b = reinterpret_cast<const char16_t*>(text.end());
    m_private->document().SetText({ a, b });
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UITextBox::SetText(CUIString&& text) noexcept {
    this->SetText(text.view());
    auto& cached = m_private->text_cached;
    cached = std::move(text);
}

#ifndef NDEBUG
/// <summary>
/// Debugs the output.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void LongUI::UITextBox::DebugOutput(const char* txt) noexcept {
    LUIDebug(Hint) << CUIString::FromUtf8(txt) << endl;
}

/// <summary>
/// Draws the cell.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="rect">The rect.</param>
/// <param name="index">The index.</param>
/// <returns></returns>
void LongUI::UITextBox::DrawCell(void* ctx, const TextBC::RectWHF& rect, int index) noexcept {
    assert(ctx && "bad context");
    if (UIManager.flag & IUIConfigure::Flag_DbgDrawTextCell) {
        const auto renderer = static_cast<I::Renderer2D*>(ctx);
        const auto red = ColorF::FromRGBA_CT<RGBA_Red>();
        const auto blue = ColorF::FromRGBA_CT<RGBA_Blue>();
        auto color = index & 1 ? red : blue;
        color.a = 0.185f;
        renderer->FillRectangle({
            rect.x, rect.y, rect.x + rect.width, rect.y + rect.height
        }, &UIManager.RefCCBrush(color));
    }
}

#endif

/// <summary>
/// Contents the event.
/// </summary>
/// <param name="txt">The text.</param>
/// <param name="event">The event.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
void LongUI::UITextBox::ContentEvent(Text& txt, MetricsEvent event, void* arg) noexcept {
    const auto ptr = auto_cast(&txt);
    switch (event)
    {
    case TextBC::IBCTextPlatform::Event_GetSize:
        // 获取内容大小
        *static_cast<TextBC::SizeF*>(arg) = { 0 };
        if (ptr) {
            // 利用文本测量结构体获取大小
            DWRITE_TEXT_METRICS tm;
            ptr->GetMetrics(&tm);
            static_cast<TextBC::SizeF*>(arg)->width = tm.width;
            static_cast<TextBC::SizeF*>(arg)->height = tm.height;
        }
        break;
    case TextBC::IBCTextPlatform::Event_GetBaseline:
        *static_cast<float*>(arg) = 0.;
        if (ptr) {
            // 利用行测量结构体获取基线偏移量
            DWRITE_LINE_METRICS lm; uint32_t count = 1;
            ptr->GetLineMetrics(&lm, 1, &count);
            *static_cast<float*>(arg) = lm.baseline;
        }
        break;
    case TextBC::IBCTextPlatform::Event_HitTest:
        // 点击检测
        {
            union ht_data { TextBC::HitTest ht; float pos; };
            const float px = reinterpret_cast<ht_data*>(arg)->pos;
            auto& out = reinterpret_cast<ht_data*>(arg)->ht;
            out = {};
            if (ptr) {
                BOOL hint = false, inside = false;
                DWRITE_HIT_TEST_METRICS htm;
                ptr->HitTestPoint(px, 0, &hint, &inside, &htm);
                out.u16_trailing = htm.length;
                out.pos = htm.textPosition + (hint ? out.u16_trailing : 0);
            }
        }
        break;
    case TextBC::IBCTextPlatform::Event_CharMetrics:
        union cm_data { TextBC::CharMetrics cm; uint32_t pos; };
        {
            const auto pos = static_cast<cm_data*>(arg)->pos;
            auto& out = static_cast<cm_data*>(arg)->cm;
            out = {};
            if (ptr) {
                auto& buf = m_private->cluster_buffer;
                const auto size = pos + 1;
                buf.resize(size);
                // TODO: 错误处理
                if (!buf.is_ok()) break;
                uint32_t max_count = 0;
                ptr->GetClusterMetrics(buf.data(), size, &max_count);
                // 遍历位置
                const auto data_ptr = buf.data();
                const auto data_len = max_count;
                float width = 0, last_width = 0;
                uint32_t char_index = 0;
                // 防止万一, 加上 [i != data_len]
                for (uint32_t i = 0; i != data_len; ++i) {
                    const auto&x = data_ptr[i];
                    width += last_width = x.width;
                    // 防止万一用>=
                    if (char_index >= pos) break;
                    char_index += x.length;
                }
                assert(char_index == pos && u8"检查");
                // 写回返回值
                out.width = last_width;
                out.x = width - last_width;
            }
        }
        break;
    default:
        assert(!"bad case");
    }
}


