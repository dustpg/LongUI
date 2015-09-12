#include "LongUI.h"
#include <algorithm>
#include "dcomp.h"
                   
                    

// -------------------- LongUI::Component::ShortText --------------------
// ShortText 构造函数
LongUI::Component::ShortText::ShortText(pugi::xml_node node, const char * prefix) noexcept
    : m_pTextRenderer(nullptr) {
    // 设置
    m_config = {
        nullptr,
        128.f, 64.f, 1.f,
        Helper::XMLGetRichType(node, RichType::Type_None, "richtype", prefix),
        0
    };
    // 检查参数
    assert(node && prefix && "bad arguments");
    // 属性
    auto attribute = [&node, prefix](const char* attr) {
        return Helper::XMLGetValue(node, attr, prefix);
    };
    const char*str = nullptr;
    // 获取进度
    if ((str = attribute("progress"))) {
        m_config.progress = LongUI::AtoF(str);
    }
    // 获取渲染器
    m_pTextRenderer = UIManager.GetTextRenderer(attribute("renderer"));
    // 保证缓冲区
    if (m_pTextRenderer) {
        auto length = m_pTextRenderer->GetContextSizeInByte();
        if (length) {
            if ((str = attribute("context"))) {
                m_buffer.NewSize(length);
                m_pTextRenderer->CreateContextFromString(m_buffer.GetData(), str);
            }
        }
    }
    // 检查基本颜色
    m_basicColor = D2D1::ColorF(D2D1::ColorF::Black);
    Helper::MakeColor(attribute("color"), m_basicColor);
    {
        // 检查格式
        uint32_t format_index = 0;
        if ((str = attribute("format"))) {
            format_index = static_cast<uint32_t>(LongUI::AtoI(str));
        }
        m_config.format = UIManager.GetTextFormat(format_index);
    }
    // 重建
    m_text.Set(node.attribute(prefix).value());
    this->RecreateLayout();
}

// ShortText 析构
LongUI::Component::ShortText::~ShortText() noexcept {
    ::SafeRelease(m_pLayout);
    ::SafeRelease(m_pTextRenderer);
    ::SafeRelease(m_config.format);
}

// ShortText 重建布局
void LongUI::Component::ShortText::RecreateLayout() noexcept {
    // 保留数据
    auto old_layout = m_pLayout;
    m_pLayout = nullptr;
    // 看情况
    switch (m_config.rich_type)
    {
    case LongUI::RichType::Type_None:
    {
        register auto string_length_need = static_cast<uint32_t>(
            static_cast<float>(m_text.length() + 1) * m_config.progress
            );
        // clamp it
        if (string_length_need < 0) string_length_need = 0;
        else if (string_length_need > m_text.length()) string_length_need = m_text.length();
        // create it
        UIManager_DWriteFactory->CreateTextLayout(
            m_text.c_str(),
            string_length_need,
            old_layout ? old_layout : m_config.format,
            m_config.width,
            m_config.height,
            &m_pLayout
            );
    }
        m_config.text_length = static_cast<decltype(m_config.text_length)>(m_text.length());
        break;
    case LongUI::RichType::Type_Core:
        m_pLayout = DX::FormatTextCore(m_config, m_text.c_str());
        break;
    case LongUI::RichType::Type_Xml:
        m_pLayout = DX::FormatTextXML(m_config, m_text.c_str());
        break;
    case LongUI::RichType::Type_Custom:
        m_pLayout = UIManager.configure->CustomRichType(m_config, m_text.c_str());
        break;
    }
    ::SafeRelease(old_layout);
    // sad
    assert(m_pLayout);
}


// -------------------- LongUI::Component::EditaleText --------------------

// DWrite部分代码参考: 
// http://msdn.microsoft.com/zh-cn/library/windows/desktop/dd941792(v=vs.85).aspx

// 刷新布局
auto LongUI::Component::EditaleText::refresh(bool update) const noexcept ->UIWindow* {
    if (!m_bThisFocused) return nullptr;
    RectLTWH_F rect; this->GetCaretRect(rect);
    register auto* window = m_pHost->GetWindow();
    window->CreateCaret(m_pHost, rect.width, rect.height);
    window->SetCaretPos(m_pHost, rect.left, rect.top);
    if (update) {
        window->Invalidate(m_pHost);
    }
    return window;
}

// 重新创建布局
void LongUI::Component::EditaleText::recreate_layout() noexcept {
    ::SafeRelease(this->layout);
    // 创建布局
    UIManager_DWriteFactory->CreateTextLayout(
        m_string.c_str(), static_cast<uint32_t>(m_string.length()),
        m_pBasicFormat,
        m_size.width, m_size.height,
        &this->layout
        );
}

// 插入字符(串)
auto LongUI::Component::EditaleText::insert(
    uint32_t pos, const wchar_t * str,  uint32_t length) noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 只读
    if (this->IsReadOnly()) {
        ::MessageBeep(MB_ICONERROR);
        return S_FALSE;
    }
    // 插入字符
    m_string.insert(pos, str, length); 
    auto old_length = static_cast<uint32_t>(m_string.length());
    // 保留旧布局
    auto old_layout = ::SafeAcquire(this->layout);
    // 重新创建布局
    this->recreate_layout();
    // 富文本情况下?
    if (old_layout && this->IsRiched() && SUCCEEDED(hr)) {
        // 复制全局属性
        Component::EditaleText::CopyGlobalProperties(old_layout, this->layout);
        // 对于每种属性, 获取并应用到新布局
        // 在首位?
        if (pos) {
            // 第一块
            Component::EditaleText::CopyRangedProperties(old_layout, this->layout, 0, pos, 0);
            // 插入块
            Component::EditaleText::CopySinglePropertyRange(old_layout, pos - 1, this->layout, pos, length);
            // 结束块
            Component::EditaleText::CopyRangedProperties(old_layout, this->layout, pos, old_length, length);
        }
        else {
            // 插入块
            Component::EditaleText::CopySinglePropertyRange(old_layout, 0, this->layout, 0, length);
            // 结束块
            Component::EditaleText::CopyRangedProperties(old_layout, this->layout, 0, old_length, length);
        }
        // 末尾
        Component::EditaleText::CopySinglePropertyRange(old_layout, old_length, this->layout, static_cast<uint32_t>(m_string.length()), UINT32_MAX);
    }
    ::SafeRelease(old_layout);
    return hr;
}


// 返回当前选择区域
auto LongUI::Component::EditaleText::GetSelectionRange() const noexcept-> DWRITE_TEXT_RANGE {
    // 返回当前选择返回
    auto caretBegin = m_u32CaretAnchor;
    auto caretEnd = m_u32CaretPos + m_u32CaretPosOffset;
    // 相反则交换
    if (caretBegin > caretEnd) {
        std::swap(caretBegin, caretEnd);
    }
    // 限制范围在文本长度之内
    auto textLength = static_cast<uint32_t>(m_string.size());
    caretBegin = std::min(caretBegin, textLength);
    caretEnd = std::min(caretEnd, textLength);
    // 返回范围
    return { caretBegin, caretEnd - caretBegin };
}

// 设置选择区
auto LongUI::Component::EditaleText::SetSelection(
    SelectionMode mode, uint32_t advance, bool exsel, bool update) noexcept-> HRESULT {
    //uint32_t line = uint32_t(-1);
    uint32_t absolute_position = m_u32CaretPos + m_u32CaretPosOffset;
    uint32_t oldabsolute_position = absolute_position;
    uint32_t old_caret_anchor = m_u32CaretAnchor;
    DWRITE_TEXT_METRICS textMetrics;
    // CASE
    switch (mode)
    {
    case SelectionMode::Mode_Left:
        m_u32CaretPos += m_u32CaretPosOffset;
        if (m_u32CaretPos > 0) {
            --m_u32CaretPos;
            this->AlignCaretToNearestCluster(false, true);
            absolute_position = m_u32CaretPos + m_u32CaretPosOffset;
            // 检查换行符
            absolute_position = m_u32CaretPos + m_u32CaretPosOffset;
            if (absolute_position >= 1
                && absolute_position < m_string.size()
                && m_string[absolute_position - 1] == L'\r'
                &&  m_string[absolute_position] == L'\n')
            {
                m_u32CaretPos = absolute_position - 1;
                this->AlignCaretToNearestCluster(false, true);
            }
        }
        break;

    case SelectionMode::Mode_Right:
        m_u32CaretPos = absolute_position;
        this->AlignCaretToNearestCluster(true, true);
        absolute_position = m_u32CaretPos + m_u32CaretPosOffset;
        if (absolute_position >= 1
            && absolute_position < m_string.size()
            && m_string[absolute_position - 1] == '\r'
            &&  m_string[absolute_position] == '\n')
        {
            m_u32CaretPos = absolute_position + 1;
            this->AlignCaretToNearestCluster(false, true);
        }
        break;
    case SelectionMode::Mode_LeftChar:
        m_u32CaretPos = absolute_position;
        m_u32CaretPos -= std::min(advance, absolute_position);
        m_u32CaretPosOffset = 0;
        break;
    case SelectionMode::Mode_RightChar:
        m_u32CaretPos = absolute_position + advance;
        m_u32CaretPosOffset = 0;
        {
            // Use hit-testing to limit text position.
            DWRITE_HIT_TEST_METRICS hitTestMetrics;
            float caretX, caretY;
            this->layout->HitTestTextPosition(
                m_u32CaretPos,
                false,
                &caretX,
                &caretY,
                &hitTestMetrics
                );
            m_u32CaretPos = std::min(m_u32CaretPos, hitTestMetrics.textPosition + hitTestMetrics.length);
        }
        break;
    case SelectionMode::Mode_Up:
    case SelectionMode::Mode_Down:
    {
        EzContainer::SmallBuffer<DWRITE_LINE_METRICS, 10> metrice_buffer;
        // 获取行指标
        this->layout->GetMetrics(&textMetrics);
        metrice_buffer.NewSize(textMetrics.lineCount);
        this->layout->GetLineMetrics(
            metrice_buffer.GetData(),
            textMetrics.lineCount,
            &textMetrics.lineCount
            );
        // 获取行
        uint32_t line, linePosition;
        Component::EditaleText::GetLineFromPosition(
            metrice_buffer.GetData(),
            metrice_buffer.GetCount(),
            m_u32CaretPos,
            &line,
            &linePosition
            );
        // 下移或上移
        if (mode == SelectionMode::Mode_Up) {
            if (line <= 0) break;
            line--;
            linePosition -= metrice_buffer[line].length;
        }
        else {
            linePosition += metrice_buffer[line].length;
            line++;
            if (line >= metrice_buffer.GetCount())  break;
        }
        DWRITE_HIT_TEST_METRICS hitTestMetrics;
        float caretX, caretY, dummyX;
        // 获取当前文本X位置
        this->layout->HitTestTextPosition(
            m_u32CaretPos,
            m_u32CaretPosOffset > 0, // trailing if nonzero, else leading edge
            &caretX,
            &caretY,
            &hitTestMetrics
            );
        // 获取新位置Y坐标
        this->layout->HitTestTextPosition(
            linePosition,
            false, // leading edge
            &dummyX,
            &caretY,
            &hitTestMetrics
            );
        // 获取新x, y 的文本位置
        BOOL isInside, isTrailingHit;
        this->layout->HitTestPoint(
            caretX, caretY,
            &isTrailingHit,
            &isInside,
            &hitTestMetrics
            );
        m_u32CaretPos = hitTestMetrics.textPosition;
        m_u32CaretPosOffset = isTrailingHit ? (hitTestMetrics.length > 0) : 0;
    }
    break;
    case SelectionMode::Mode_LeftWord:
    case SelectionMode::Mode_RightWord:
    {
        // 计算所需字符串集
        EzContainer::SmallBuffer<DWRITE_CLUSTER_METRICS, 64> metrice_buffer;
        UINT32 clusterCount;
        this->layout->GetClusterMetrics(nullptr, 0, &clusterCount);
        if (clusterCount == 0) break;
        // 重置大小
        metrice_buffer.NewSize(clusterCount);
        this->layout->GetClusterMetrics(metrice_buffer.GetData(), clusterCount, &clusterCount);
        m_u32CaretPos = absolute_position;
        UINT32 clusterPosition = 0;
        UINT32 oldCaretPosition = m_u32CaretPos;
        // 左移
        if (mode == SelectionMode::Mode_LeftWord) {
            m_u32CaretPos = 0;
            m_u32CaretPosOffset = 0; // leading edge
            for (UINT32 cluster = 0; cluster < clusterCount; ++cluster) {
                clusterPosition += metrice_buffer[cluster].length;
                if (metrice_buffer[cluster].canWrapLineAfter) {
                    if (clusterPosition >= oldCaretPosition)
                        break;
                    // 刷新.
                    m_u32CaretPos = clusterPosition;
                }
            }
        }
        else {
            // 之后
            for (UINT32 cluster = 0; cluster < clusterCount; ++cluster) {
                UINT32 clusterLength = metrice_buffer[cluster].length;
                m_u32CaretPos = clusterPosition;
                m_u32CaretPosOffset = clusterLength; // trailing edge
                if (clusterPosition >= oldCaretPosition &&
                    metrice_buffer[cluster].canWrapLineAfter) {
                    break;
                }
                clusterPosition += clusterLength;
            }
        }
        //int a = 0;
    }
    break;
    case SelectionMode::Mode_Home:
    case SelectionMode::Mode_End:
    {
        // 获取预知的首位置或者末位置
        EzContainer::SmallBuffer<DWRITE_LINE_METRICS, 10> metrice_buffer;
        // 获取行指标
        this->layout->GetMetrics(&textMetrics);
        metrice_buffer.NewSize(textMetrics.lineCount);
        this->layout->GetLineMetrics(
            metrice_buffer.GetData(),
            textMetrics.lineCount,
            &textMetrics.lineCount
            );
        uint32_t line;
        Component::EditaleText::GetLineFromPosition(
            metrice_buffer.GetData(),
            metrice_buffer.GetCount(),
            m_u32CaretPos,
            &line,
            &m_u32CaretPos
            );
        m_u32CaretPosOffset = 0;
        if (mode == SelectionMode::Mode_End) {
            // 放置插入符号
            UINT32 lineLength = metrice_buffer[line].length -
                metrice_buffer[line].newlineLength;
            m_u32CaretPosOffset = std::min(lineLength, 1u);
            m_u32CaretPos += lineLength - m_u32CaretPosOffset;
            this->AlignCaretToNearestCluster(true);
        }
    }
    break;
    case SelectionMode::Mode_First:
        m_u32CaretPos = 0;
        m_u32CaretPosOffset = 0;
        break;

    case SelectionMode::Mode_SelectAll:
        m_u32CaretAnchor = 0;
        exsel = true;
        __fallthrough;
    case SelectionMode::Mode_Last:
        m_u32CaretPos = UINT32_MAX;
        m_u32CaretPosOffset = 0;
        this->AlignCaretToNearestCluster(true);
        break;
    case SelectionMode::Mode_Leading:
        m_u32CaretPos = advance;
        m_u32CaretPosOffset = 0;
        break;
    case SelectionMode::Mode_Trailing:
        m_u32CaretPos = advance;
        this->AlignCaretToNearestCluster(true);
        break;
    }
    absolute_position = m_u32CaretPos + m_u32CaretPosOffset;
    // 附加选择
    if (!exsel) {
        m_u32CaretAnchor = absolute_position;
    }
    // 检查移动
    bool caretMoved = (absolute_position != oldabsolute_position)
        || (m_u32CaretAnchor != old_caret_anchor);
    // 移动了?
    if (caretMoved) {
        // 更新格式
        if (update) {

        }
        // 刷新插入符号
        this->refresh(true);
    }

    return caretMoved ? S_OK : S_FALSE;
    //return S_OK;
}

// 删除选择区文字
auto LongUI::Component::EditaleText::DeleteSelection() noexcept-> HRESULT {
    DWRITE_TEXT_RANGE selection = this->GetSelectionRange();
    if (selection.length == 0 || this->IsReadOnly()) return S_FALSE;
    // 删除成功的话设置选择区
    if (this->remove_text(selection.startPosition, selection.length)) {
        return this->SetSelection(Mode_Leading, selection.startPosition, false);
    }
    else {
        return S_FALSE;
    }
}


// 设置选择区
bool LongUI::Component::EditaleText::SetSelectionFromPoint(float x, float y, bool exsel) noexcept {
    BOOL isTrailingHit;
    BOOL isInside;
    DWRITE_HIT_TEST_METRICS caret_metrics;
    // 获取当前点击位置
    this->layout->HitTestPoint(
        x, y,
        &isTrailingHit,
        &isInside,
        &caret_metrics
        );
    // 更新当前选择区
    this->SetSelection(
        isTrailingHit ? SelectionMode::Mode_Trailing : SelectionMode::Mode_Leading,
        caret_metrics.textPosition,
        exsel
        );
    return true;
}


// 拖入
bool LongUI::Component::EditaleText::OnDragEnter(IDataObject* data, DWORD* effect) noexcept {
    m_bDragFormatOK = false;
    m_bThisFocused = true;
    m_bDragFromThis = m_pDataObject == data;
    assert(data && effect && "bad argument");
    UNREFERENCED_PARAMETER(effect);
    m_pHost->GetWindow()->ShowCaret();
    ::ReleaseStgMedium(&m_recentMedium);
    // 检查支持格式: Unicode-ShortText
    FORMATETC fmtetc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    if (SUCCEEDED(data->GetData(&fmtetc, &m_recentMedium))) {
        m_bDragFormatOK = true;
    }
    return m_bDragFromThis ? false : m_bDragFormatOK;
}


// 拖上
bool LongUI::Component::EditaleText::OnDragOver(float x, float y) noexcept {
    // 自己的?并且在选择范围内?
    if (m_bDragFromThis) {
        register auto range = m_dragRange;
        BOOL trailin, inside;
        DWRITE_HIT_TEST_METRICS caret_metrics;
        // 获取当前点击位置
        this->layout->HitTestPoint(x, y, &trailin, &inside, &caret_metrics);
        register bool inzone = caret_metrics.textPosition >= range.startPosition &&
            caret_metrics.textPosition < range.startPosition + range.length;
        if (inzone) return false;
    }
    // 选择位置
    if (m_bDragFormatOK) {
        this->SetSelectionFromPoint(x, y, false);
        // 显示插入符号
        this->refresh(false);
        return true;
    }
    return false;
}

// 重建
void LongUI::Component::EditaleText::Recreate() noexcept {
    // 重新创建资源
    ::SafeRelease(m_pSelectionColor);
    UIManager_RenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightSkyBlue),
        &m_pSelectionColor
        );
}

// 键入一个字符时
void LongUI::Component::EditaleText::OnChar(char32_t ch) noexcept {
    // 字符有效
    if ((ch >= 0x20 || ch == 9)) {
        if (this->IsReadOnly()) {
            ::MessageBeep(MB_ICONERROR);
            return;
        }
        // 删除选择区字符串
        this->DeleteSelection();
        //
        uint32_t length = 1;
        wchar_t chars[] = { static_cast<wchar_t>(ch), 0, 0 };
        // sizeof(wchar_t) == sizeof(char32_t) 情况下
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change it");
#if 0 // sizeof(wchar_t) == sizeof(char32_t)

#else
        // 检查是否需要转换
        if (ch > 0xFFFF) {
            // From http://unicode.org/faq/utf_bom.html#35
            chars[0] = wchar_t(0xD800 + (ch >> 10) - (0x10000 >> 10));
            chars[1] = wchar_t(0xDC00 + (ch & 0x3FF));
            length++;
        }
#endif
        // 插入
        this->insert(m_u32CaretPos + m_u32CaretPosOffset, chars, length);
        // 设置选择区
        this->SetSelection(SelectionMode::Mode_Right, length, false, false);
        // 刷新
        this->refresh(true);
    }
}


// 按键时
void LongUI::Component::EditaleText::OnKey(uint32_t keycode) noexcept {
    // 检查按键 maybe IUIInput
    bool heldShift = (::GetKeyState(VK_SHIFT) & 0x80) != 0;
    bool heldControl = (::GetKeyState(VK_CONTROL) & 0x80) != 0;
    // 绝对位置
    UINT32 absolutePosition = m_u32CaretPos + m_u32CaretPosOffset;

    switch (keycode)
    {
    case VK_RETURN:     // 回车键
        // 多行 - 键CRLF字符
        if (this->IsMultiLine()) {
            this->DeleteSelection();
            this->insert(absolutePosition, L"\r\n", 2);
            this->SetSelection(SelectionMode::Mode_Leading, absolutePosition + 2, false, false);
            // 修改
            this->refresh();
        }
        // 单行 - 向窗口发送输入完毕消息
        else {
            // sb!
            this->sbcaller.operator()(m_pHost, SubEvent::Event_EditReturned);
        }
        break;
    case VK_BACK:       // 退格键
                        // 有选择的话
        if (absolutePosition != m_u32CaretAnchor) {
            // 删除选择区
            this->DeleteSelection();
            // 重建布局
            this->recreate_layout();
        }
        else if (absolutePosition > 0) {
            UINT32 count = 1;
            // 对于CR/LF 特别处理
            if (absolutePosition >= 2 && absolutePosition <= m_string.size()) {
                auto* __restrict base = m_string.data() + absolutePosition;
                if (base[-2] == L'\r' && base[-1] == L'\n') {
                    count = 2;
                }
            }
            // 左移
            this->SetSelection(SelectionMode::Mode_LeftChar, count, false);
            // 字符串: 删除count个字符
            if (this->remove_text(m_u32CaretPos, count)) {
                this->recreate_layout();
            }
        }
        // 修改
        this->refresh();
        break;
    case VK_DELETE:     // 删除键
                        // 有选择的话
        if (absolutePosition != m_u32CaretAnchor) {
            // 删除选择区
            this->DeleteSelection();
            // 重建布局
            this->recreate_layout();
        }
        // 删除下一个的字符
        else {
            DWRITE_HIT_TEST_METRICS hitTestMetrics;
            float caretX, caretY;
            // 获取集群大小
            this->layout->HitTestTextPosition(
                absolutePosition,
                false,
                &caretX,
                &caretY,
                &hitTestMetrics
                );
            // CR-LF?
            if (hitTestMetrics.textPosition + 2 < m_string.length()) {
                auto* __restrict base = m_string.data() + hitTestMetrics.textPosition;
                if (base[0] == L'\r' && base[1] == L'\n') {
                    ++hitTestMetrics.length;
                }
            }
            // 修改
            this->SetSelection(SelectionMode::Mode_Leading, hitTestMetrics.textPosition, false);
            // 删除字符
            if(this->remove_text(hitTestMetrics.textPosition, hitTestMetrics.length)) {
                this->recreate_layout();
            }
        }
        // 修改
        this->refresh();
        break;
    case VK_TAB:        // Tab键
        break;
    case VK_LEFT:       // 光标左移一个字符/集群
        this->SetSelection(heldControl ? SelectionMode::Mode_LeftWord : SelectionMode::Mode_Left, 1, heldShift);
        break;
    case VK_RIGHT:      // 光标右移一个字符/集群
        this->SetSelection(heldControl ? SelectionMode::Mode_RightWord : SelectionMode::Mode_Right, 1, heldShift);
        break;
    case VK_UP:         // 多行模式: 上移一行
        if (this->IsMultiLine())
            this->SetSelection(SelectionMode::Mode_Up, 1, heldShift);
        break;
    case VK_DOWN:       // 多行模式: 下移一行
        if (this->IsMultiLine())
            this->SetSelection(SelectionMode::Mode_Down, 1, heldShift);
        break;
    case VK_HOME:       // HOME键
        this->SetSelection(heldControl ? SelectionMode::Mode_First : SelectionMode::Mode_Home, 0, heldShift);
        break;
    case VK_END:        // END键
        this->SetSelection(heldControl ? SelectionMode::Mode_Last : SelectionMode::Mode_End, 0, heldShift);
        break;
    case 'C':           // 'C'键 Ctrl+C 复制
        if (heldControl) this->CopyToClipboard();
        break;
    case VK_INSERT:     // Insert键
        if (heldControl)    this->CopyToClipboard();
        else if (heldShift) this->PasteFromClipboard();
        break;
    case 'V':           // 'V'键 Ctrl+V 粘贴
        if (heldControl)   this->PasteFromClipboard();
        break;
    case 'X':           // 'X'键 Ctrl+X 剪切
        if (heldControl) {
            this->CopyToClipboard();
            this->DeleteSelection();
            this->recreate_layout();
            this->refresh();
        }
        break;
    case 'A':           // 'A'键 Ctrl+A 全选
        if (heldControl)
            this->SetSelection(SelectionMode::Mode_SelectAll, 0, true);
        break;
    case 'Z':
        break;
    case 'Y':
        break;
    default:
        break;
    }
}

// 当设置焦点时
void LongUI::Component::EditaleText::OnSetFocus() noexcept {
    m_bThisFocused = true;
    this->refresh()->ShowCaret();
}

// 当失去焦点时
void LongUI::Component::EditaleText::OnKillFocus() noexcept {
    register auto* window = m_pHost->GetWindow();
    window->HideCaret();
    m_bThisFocused = false;
}

// 左键弹起时
void LongUI::Component::EditaleText::OnLButtonUp(float x, float y) noexcept {
    // 检查
    if (m_bClickInSelection && m_ptStart.x == x && m_ptStart.y == y) {
        // 选择
        this->SetSelectionFromPoint(x, y, false);
        // 显示插入符号
        this->refresh(false);
    }
    m_pHost->GetWindow()->ReleaseCapture();

}

// 左键按下时
void LongUI::Component::EditaleText::OnLButtonDown(float x, float y, bool shfit_hold) noexcept {
    // 设置鼠标捕获
    m_pHost->GetWindow()->SetCapture(m_pHost);
    // 刷新
    auto range = this->GetSelectionRange();
    this->RefreshSelectionMetrics(range);
    // 记录点击位置
    m_ptStart = { x, y };
    // 选择区中?
    if (m_bufMetrice.GetCount()) {
        // 计算
        BOOL trailin, inside;
        DWRITE_HIT_TEST_METRICS caret_metrics;
        // 获取当前点击位置
        this->layout->HitTestPoint(x, y, &trailin, &inside, &caret_metrics);
        m_bClickInSelection = caret_metrics.textPosition >= range.startPosition &&
            caret_metrics.textPosition < range.startPosition + range.length;
    }
    else {
        m_bClickInSelection = false;
    }
    // 插入
    if (!m_bClickInSelection) {
        // 选择
        this->SetSelectionFromPoint(x, y, shfit_hold);
    }
}

// 左键按住时
void LongUI::Component::EditaleText::OnLButtonHold(float x, float y, bool shfit_hold) noexcept {
    // 起点在选择区
    if (!shfit_hold && m_bClickInSelection) {
        // 开始拖拽
        if (m_ptStart.x != x || m_ptStart.y != y) {
            // 检查范围
            m_dragRange = this->GetSelectionRange();
            // 去除鼠标捕获
            m_pHost->GetWindow()->ReleaseCapture();
            // 设置
            m_pDataObject->SetUnicodeText(this->CopyToGlobal());
            // 开始拖拽
            DWORD effect = DROPEFFECT_COPY;
            if (!(this->IsReadOnly())) effect |= DROPEFFECT_MOVE;
            const register HRESULT hr = ::SHDoDragDrop(
                m_pHost->GetWindow()->GetHwnd(),
                m_pDataObject, m_pDropSource, effect, &effect
                );
            // 拖放成功 且为移动
            if (hr == DRAGDROP_S_DROP && effect == DROPEFFECT_MOVE) {
                //自己的？
                if (m_bDragFromThis) {
                    m_dragRange.startPosition += m_dragRange.length;
                }
                // 删除
                if (this->remove_text(m_dragRange.startPosition, m_dragRange.length)) {
                    this->recreate_layout();
                    this->SetSelection(Mode_Left, 1, false);
                    this->SetSelection(Mode_Right, 1, false);
                }

            }
            // 回归?
            //m_pHost->GetWindow()->SetCapture(m_pHost);
        }
    }
    else {
        this->SetSelectionFromPoint(x, y, true);
    }
}

// 对齐最近字符集
void LongUI::Component::EditaleText::AlignCaretToNearestCluster(bool hit, bool skip) noexcept {
    DWRITE_HIT_TEST_METRICS hitTestMetrics;
    float caretX, caretY;
    // 对齐最近字符集
    this->layout->HitTestTextPosition(
        m_u32CaretPos,
        false,
        &caretX,
        &caretY,
        &hitTestMetrics
        );
    // 跳过0
    m_u32CaretPos = hitTestMetrics.textPosition;
    m_u32CaretPosOffset = (hit) ? hitTestMetrics.length : 0;
    // 对于不可视的
    if (skip && hitTestMetrics.width == 0) {
        m_u32CaretPos += m_u32CaretPosOffset;
        m_u32CaretPosOffset = 0;
    }
}

// 获取插入符号矩形
void LongUI::Component::EditaleText::GetCaretRect(RectLTWH_F& rect)const noexcept {
    // 检查布局
    if (this->layout) {
        // 获取 f(文本偏移) -> 坐标
        DWRITE_HIT_TEST_METRICS caretMetrics;
        float caretX, caretY;
        this->layout->HitTestTextPosition(
            m_u32CaretPos,
            m_u32CaretPosOffset > 0,
            &caretX,
            &caretY,
            &caretMetrics
            );
        // 获取当前选择范围
        DWRITE_TEXT_RANGE selectionRange = this->GetSelectionRange();
        if (selectionRange.length > 0) {
            UINT32 actualHitTestCount = 1;
            this->layout->HitTestTextRange(
                m_u32CaretPos,
                0, // length
                0, // x
                0, // y
                &caretMetrics,
                1,
                &actualHitTestCount
                );
            caretY = caretMetrics.top;
        }
        // 获取插入符号宽度
        DWORD caretIntThickness = 2;
        ::SystemParametersInfoW(SPI_GETCARETWIDTH, 0, &caretIntThickness, FALSE);
        register float caretThickness = static_cast<float>(caretIntThickness);
        // 计算相对位置
        // XXX: 检查draw_zone
        rect.left = caretX - caretThickness * 0.5f;
        rect.width = caretThickness;
        rect.top = caretY;
        rect.height = caretMetrics.height;
    }
}

// 刷新
void LongUI::Component::EditaleText::Update() noexcept {
    // s
    this->refresh(false);
    // 检查选择区
    this->RefreshSelectionMetrics(this->GetSelectionRange());
}

// 渲染
void LongUI::Component::EditaleText::Render(float x, float y)const noexcept {
#ifdef _DEBUG
    if (m_pHost->debug_this) {
        UIManager << DL_Log
            << "m_bufMetrice.data_length: "
            << long(m_bufMetrice.GetCount())
            << LongUI::endl;
    }
#endif
    // 选择区域
    if (m_bufMetrice.GetCount()) { 
        UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        // 遍历
        for (const auto& htm : m_bufMetrice) {
            D2D1_RECT_F highlightRect = {
                htm.left + x,
                htm.top + y,
                htm.left + htm.width + x,
                htm.top + htm.height + y
            };
            UIManager_RenderTarget->FillRectangle(highlightRect, m_pSelectionColor);
        }
        UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }
    // 刻画字体
    this->layout->Draw(m_buffer.GetDataVoid(), m_pTextRenderer, x, y);
}

// 复制到 目标全局句柄
auto LongUI::Component::EditaleText::CopyToGlobal() noexcept-> HGLOBAL {
    HGLOBAL global = nullptr;
    auto selection = this->GetSelectionRange();
    // 有效
    if (selection.length) {
        // 获取选择字符长度
        size_t byteSize = sizeof(wchar_t) * (selection.length + 1);
        global = ::GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, byteSize);
        // 有效?
        if (global) {
            auto* memory = reinterpret_cast<wchar_t*>(::GlobalLock(global));
            // 申请全局内存成功
            if (memory) {
                const wchar_t* text = m_string.c_str();
                ::memcpy(memory, text + selection.startPosition, byteSize);
                memory[selection.length] = L'\0';
                ::GlobalUnlock(global);
            }
        }
    }
    return global;
}

// 复制到 剪切板
auto LongUI::Component::EditaleText::CopyToClipboard() noexcept-> HRESULT {
    HRESULT hr = E_FAIL;
    auto selection = this->GetSelectionRange();
    if (selection.length) {
        // 打开剪切板
        if (::OpenClipboard(m_pHost->GetWindow()->GetHwnd())) {
            if (::EmptyClipboard()) {
                // 获取旋转字符长度
                size_t byteSize = sizeof(wchar_t) * (selection.length + 1);
                HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, byteSize);
                // 有效?
                if (hClipboardData) {
                    auto* memory = reinterpret_cast<wchar_t*>(::GlobalLock(hClipboardData));
                    // 申请全局内存成功
                    if (memory) {
                        const wchar_t* text = m_string.c_str();
                        ::memcpy(memory, text + selection.startPosition, byteSize);
                        memory[selection.length] = L'\0';
                        ::GlobalUnlock(hClipboardData);
                        // 获取富文本数据
                        if (this->IsRiched()) {
                            assert(!"Unsupported Now");
                            // TODO: 富文本
                        }
                        if (::SetClipboardData(CF_UNICODETEXT, hClipboardData) != nullptr) {
                            hClipboardData = nullptr;
                            hr = S_OK;
                        }
                    }
                }
                if (hClipboardData) {
                    ::GlobalFree(hClipboardData);
                }
            }
            //  关闭剪切板
            ::CloseClipboard();
        }
    }
    return hr;
}



// 从 目标全局句柄 黏贴
auto LongUI::Component::EditaleText::PasteFromGlobal(HGLOBAL global) noexcept-> HRESULT {
    // 正式开始
    size_t byteSize = ::GlobalSize(global);
    // 获取数据
    void* memory = ::GlobalLock(global);
    HRESULT hr = E_INVALIDARG;
    // 数据有效?
    if (memory) {
        // 替换选择区
        this->DeleteSelection();
        const wchar_t* text = reinterpret_cast<const wchar_t*>(memory);
        // 计算长度
        auto characterCount = static_cast<UINT32>(::wcsnlen(text, byteSize / sizeof(wchar_t)));
        // 插入
        hr = this->insert(m_u32CaretPos + m_u32CaretPosOffset, text, characterCount);
        ::GlobalUnlock(global);
        // 移动
        this->SetSelection(SelectionMode::Mode_RightChar, characterCount, true);
    }
    return hr;
}

// 从 剪切板 黏贴
auto LongUI::Component::EditaleText::PasteFromClipboard() noexcept-> HRESULT {
    // 正式开始
    HRESULT hr = S_OK;  //uint32_t characterCount = 0ui32;
                        // 打开剪切板
    if (::OpenClipboard(m_pHost->GetWindow()->GetHwnd())) {
        // 获取富文本数据
        if (this->IsRiched()) {
            assert(!"Unsupported Now");
            // TODO
        }
        // 获取Unicode数据
        else {
            auto data = ::GetClipboardData(CF_UNICODETEXT);
            hr = this->PasteFromGlobal(data);
        }
        // 关闭剪切板
        ::CloseClipboard();
    }
    return hr;
}

// 获取行编号
void LongUI::Component::EditaleText::GetLineFromPosition(
    const DWRITE_LINE_METRICS * lineMetrics,
    uint32_t lineCount, uint32_t textPosition,
    OUT uint32_t * lineOut,
    OUT uint32_t * linePositionOut) noexcept {
    uint32_t line = 0;
    uint32_t linePosition = 0;
    uint32_t nextLinePosition = 0;
    for (; line < lineCount; ++line) {
        linePosition = nextLinePosition;
        nextLinePosition = linePosition + lineMetrics[line].length;
        if (nextLinePosition > textPosition) {
            // 不需要.
            break;
        }
    }
    *linePositionOut = linePosition;
    *lineOut = std::min(line, lineCount - 1);
}

// 更新选择区点击测试区块
void LongUI::Component::EditaleText::RefreshSelectionMetrics(DWRITE_TEXT_RANGE selection) noexcept {
    //UIManager << DL_Hint << "selection.length: " << long(selection.length) << endl;
    // 有选择的情况下
    if (selection.length == 0) {
        m_bufMetrice.NewSize(0);
        return;
    };
    // 检查选择的区块数量
    uint32_t actualHitTestCount = 0;
    this->layout->HitTestTextRange(
        selection.startPosition,
        selection.length,
        0.f, // x
        0.f, // y
        nullptr,
        0, // metrics count
        &actualHitTestCount
        );
    // 保证数据正确
    m_bufMetrice.NewSize(actualHitTestCount);
    if (!actualHitTestCount) return;
    // 正式获取
    this->layout->HitTestTextRange(
        selection.startPosition,
        selection.length,
        0.f, // x
        0.f, // y
        m_bufMetrice.GetData(),
        m_bufMetrice.GetCount(),
        &actualHitTestCount
        );
}



// EditaleText 析构函数
LongUI::Component::EditaleText::~EditaleText() noexcept {
    ::ReleaseStgMedium(&m_recentMedium);
    ::SafeRelease(this->layout);
    ::SafeRelease(m_pBasicFormat);
    ::SafeRelease(m_pTextRenderer);
    ::SafeRelease(m_pSelectionColor);
    ::SafeRelease(m_pDropSource);
    ::SafeRelease(m_pDataObject);
}


// 可编辑文本: 配置构造函数
LongUI::Component::EditaleText::EditaleText(UIControl* host, pugi::xml_node node,
    const char* prefix) noexcept : m_pHost(host) {
    m_dragRange = { 0, 0 };
    // 检查参数
    assert(node && prefix && "bad arguments");
    ZeroMemory(&m_recentMedium, sizeof(m_recentMedium));
    // 属性
    auto attribute = [&node, prefix](const char* attr) {
        return Helper::XMLGetValue(node, attr, prefix);
    };
    // bool属性
    auto bool_attribute = [&node, prefix](const char* attr, bool def) {
        return pugi::impl::get_value_bool(Helper::XMLGetValue(node, attr, prefix), def);
    };
    const char* str = nullptr;
    // 检查类型
    {
        uint32_t tmptype = Type_None;
        // 富文本
        if (bool_attribute("rich", false)) {
            tmptype |= Type_Riched;
        }
        // 多行显示
        if (bool_attribute("multiline", false)) {
            tmptype |= Type_MultiLine;
        }
        // 只读
        if (bool_attribute("readonly", false)) {
            tmptype |= Type_ReadOnly;
        }
        // 加速键
        if (bool_attribute("accelerator", false)) {
            tmptype |= Type_Accelerator;
        }
        // 密码
        if ((str = attribute("password"))) {
            tmptype |= Type_Password;
            // TODO: UTF8 char(s) to UTF32 char;
            // this->password = 
        }
        this->type = static_cast<EditaleTextType>(tmptype);
    }
    // 获取渲染器
    {
        m_pTextRenderer = UIManager.GetTextRenderer(attribute("renderer"));
        // 保证缓冲区
        if (m_pTextRenderer) {
            auto length = m_pTextRenderer->GetContextSizeInByte();
            if (length) {
                if ((str = attribute("context"))) {
                    m_buffer.NewSize(length);
                    m_pTextRenderer->CreateContextFromString(m_buffer.GetDataVoid(), str);
                }
            }
        }
    }
    // 检查基本颜色
    {
        m_basicColor = D2D1::ColorF(D2D1::ColorF::Black);
        Helper::MakeColor(attribute("color"), m_basicColor);
    }
    // 检查格式
    {
        uint32_t format_index = LongUIDefaultTextFormatIndex;
        if ((str = attribute("format"))) {
            format_index = static_cast<uint32_t>(LongUI::AtoI(str));
        }
        m_pBasicFormat = UIManager.GetTextFormat(format_index);
    }
    // 获取文本
    {
        if (str = node.attribute(prefix).value()) {
            m_string.assign(str);
        }
    }
    // 创建布局
    this->recreate_layout();
}

// 复制全局属性
void LongUI::Component::EditaleText::CopyGlobalProperties(
    IDWriteTextLayout* old_layout,
    IDWriteTextLayout* new_layout) noexcept {
    // 基本属性
    new_layout->SetTextAlignment(old_layout->GetTextAlignment());
    new_layout->SetParagraphAlignment(old_layout->GetParagraphAlignment());
    new_layout->SetWordWrapping(old_layout->GetWordWrapping());
    new_layout->SetReadingDirection(old_layout->GetReadingDirection());
    new_layout->SetFlowDirection(old_layout->GetFlowDirection());
    new_layout->SetIncrementalTabStop(old_layout->GetIncrementalTabStop());
    // 额外属性
#ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
    DWRITE_TRIMMING trimmingOptions = {};
    IDWriteInlineObject* inlineObject = nullptr;
    old_layout->GetTrimming(&trimmingOptions, &inlineObject);
    new_layout->SetTrimming(&trimmingOptions, inlineObject);
    ::SafeRelease(inlineObject);

    DWRITE_LINE_SPACING_METHOD lineSpacingMethod = DWRITE_LINE_SPACING_METHOD_DEFAULT;
    float lineSpacing = 0.f, baseline = 0.f;
    old_layout->GetLineSpacing(&lineSpacingMethod, &lineSpacing, &baseline);
    new_layout->SetLineSpacing(lineSpacingMethod, lineSpacing, baseline);
#endif
}

// 对范围复制单个属性
void LongUI::Component::EditaleText::CopySinglePropertyRange(
    IDWriteTextLayout* old_layout, uint32_t old_start,
    IDWriteTextLayout* new_layout, uint32_t new_start, uint32_t length) noexcept {
    // 计算范围
    DWRITE_TEXT_RANGE range = { new_start,  std::min(length, UINT32_MAX - new_start) };
    // 字体集
#ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
    IDWriteFontCollection* fontCollection = nullptr;
    old_layout->GetFontCollection(old_start, &fontCollection);
    new_layout->SetFontCollection(fontCollection, range);
    ::SafeRelease(fontCollection);
#endif
    {
        // 字体名
        wchar_t fontFamilyName[100];
        fontFamilyName[0] = L'\0';
        old_layout->GetFontFamilyName(old_start, &fontFamilyName[0], ARRAYSIZE(fontFamilyName));
        new_layout->SetFontFamilyName(fontFamilyName, range);
        // 一般属性
        DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
        old_layout->GetFontWeight(old_start, &weight);
        old_layout->GetFontStyle(old_start, &style);
        old_layout->GetFontStretch(old_start, &stretch);
        new_layout->SetFontWeight(weight, range);
        new_layout->SetFontStyle(style, range);
        new_layout->SetFontStretch(stretch, range);
        // 字体大小
        FLOAT fontSize = 12.0f;
        old_layout->GetFontSize(old_start, &fontSize);
        new_layout->SetFontSize(fontSize, range);
        // 下划线. 删除线
        BOOL value = FALSE;
        old_layout->GetUnderline(old_start, &value);
        new_layout->SetUnderline(value, range);
        old_layout->GetStrikethrough(old_start, &value);
        new_layout->SetStrikethrough(value, range);
#ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
        // 地区名
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
        localeName[0] = L'\0';
        old_layout->GetLocaleName(old_start, &localeName[0], ARRAYSIZE(localeName));
        new_layout->SetLocaleName(localeName, range);
#endif
    }
    // 刻画效果
    IUnknown* drawingEffect = nullptr;
    old_layout->GetDrawingEffect(old_start, &drawingEffect);
    new_layout->SetDrawingEffect(drawingEffect, range);
    ::SafeRelease(drawingEffect);
    // 内联对象
    IDWriteInlineObject* inlineObject = nullptr;
    old_layout->GetInlineObject(old_start, &inlineObject);
    new_layout->SetInlineObject(inlineObject, range);
    ::SafeRelease(inlineObject);
#ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
    // 排版
    IDWriteTypography* typography = nullptr;
    old_layout->GetTypography(old_start, &typography);
    new_layout->SetTypography(typography, range);
    ::SafeRelease(typography);
#endif
}

// 范围复制AoE!
void LongUI::Component::EditaleText::CopyRangedProperties(
    IDWriteTextLayout* old_layout, IDWriteTextLayout* new_layout,
    uint32_t begin, uint32_t end, uint32_t new_offset, bool negative) noexcept {
    auto current = begin;
    // 遍历
    while (current < end) {
        // 计算范围长度
        DWRITE_TEXT_RANGE increment = { current, 1 };
        DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
        old_layout->GetFontWeight(current, &weight, &increment);
        UINT32 rangeLength = increment.length - (current - increment.startPosition);
        // 检查有效性
        rangeLength = std::min(rangeLength, end - current);
        // 复制单个
        Component::EditaleText::CopySinglePropertyRange(
            old_layout,
            current,
            new_layout,
            negative ? (current - new_offset) : (current + new_offset),
            rangeLength
            );
        // 推进
        current += rangeLength;
    }
}



// -------------------------------------------------------------


// 返回FALSE
HRESULT LongUI::CUIBasicTextRenderer::IsPixelSnappingDisabled(void *, BOOL * isDisabled) noexcept {
    *isDisabled = false;
    return S_OK;
}

// 从目标渲染呈现器获取
HRESULT LongUI::CUIBasicTextRenderer::GetCurrentTransform(void *, DWRITE_MATRIX * mat) noexcept {
    assert(UIManager_RenderTarget);
    // XXX: 优化 Profiler 就这1行 0.05%
    UIManager_RenderTarget->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(mat));
    return S_OK;
}

// 始终如一, 方便转换
HRESULT LongUI::CUIBasicTextRenderer::GetPixelsPerDip(void *, FLOAT * bilibili) noexcept {
    *bilibili = 1.f;
    return S_OK;
}

// 渲染内联对象
HRESULT LongUI::CUIBasicTextRenderer::DrawInlineObject(
    void * clientDrawingContext,
    FLOAT originX, FLOAT originY,
    IDWriteInlineObject * inlineObject,
    BOOL isSideways, BOOL isRightToLeft,
    IUnknown * clientDrawingEffect) noexcept {
    UNREFERENCED_PARAMETER(isSideways);
    UNREFERENCED_PARAMETER(isRightToLeft);
    assert(inlineObject && "bad argument");
    // 内联对象必须是LongUI内联对象
    // 渲染
    inlineObject->Draw(
        clientDrawingContext,
        this,
        originX, originY,
        false, false,
        clientDrawingEffect
        );
    return S_OK;
}
// v-table
#define LONGUIVTABLE(pointer)  (*reinterpret_cast<const size_t * const>(pointer))
// same v-table?
#define LONGUISAMEVT(a, b) (LONGUIVTABLE(a) == LONGUIVTABLE(b))

// ------------------CUINormalTextRender-----------------------
// 刻画字形
HRESULT LongUI::CUINormalTextRender::DrawGlyphRun(
    void * clientDrawingContext,
    FLOAT baselineOriginX, FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN * glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription,
    IUnknown * effect) noexcept {
    UNREFERENCED_PARAMETER(clientDrawingContext);
    UNREFERENCED_PARAMETER(glyphRunDescription);
    // 获取颜色
    register D2D1_COLOR_F* color = nullptr;
    // 检查
    if (effect && LONGUISAMEVT(effect, &this->basic_color)) {
        color = &static_cast<CUIColorEffect*>(effect)->color;
    }
    else {
        color = &this->basic_color.color;
    }
    // 设置颜色
    m_pBrush->SetColor(color);
    // 利用D2D接口直接渲染字形
    UIManager_RenderTarget->DrawGlyphRun(
        D2D1::Point2(baselineOriginX, baselineOriginY),
        glyphRun,
        m_pBrush,
        measuringMode
        );
    return S_OK;
}

// 刻画下划线
HRESULT LongUI::CUINormalTextRender::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_UNDERLINE* underline,
    IUnknown* effect
    ) noexcept {
    UNREFERENCED_PARAMETER(clientDrawingContext);
    // 获取颜色
    register D2D1_COLOR_F* color = nullptr;
    if (effect && LONGUISAMEVT(effect, &this->basic_color)) {
        color = &static_cast<CUIColorEffect*>(effect)->color;
    }
    else {
        color = &this->basic_color.color;
    }
    // 设置颜色
    m_pBrush->SetColor(color);
    // 计算矩形
    D2D1_RECT_F rectangle = {
        baselineOriginX,
        baselineOriginY + underline->offset,
        baselineOriginX + underline->width,
        baselineOriginY + underline->offset + underline->thickness
    };
    // 填充矩形
    UIManager_RenderTarget->FillRectangle(&rectangle, m_pBrush);
    return S_OK;
}

// 刻画删除线
HRESULT LongUI::CUINormalTextRender::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_STRIKETHROUGH* strikethrough,
    IUnknown* effect
    ) noexcept {
    UNREFERENCED_PARAMETER(clientDrawingContext);
    // 获取颜色
    register D2D1_COLOR_F* color = nullptr;
    if (effect && LONGUISAMEVT(effect, &this->basic_color)) {
        color = &static_cast<CUIColorEffect*>(effect)->color;
    }
    else {
        color = &this->basic_color.color;
    }
    // 设置颜色
    m_pBrush->SetColor(color);
    // 计算矩形
    D2D1_RECT_F rectangle = {
        baselineOriginX,
        baselineOriginY + strikethrough->offset,
        baselineOriginX + strikethrough->width,
        baselineOriginY + strikethrough->offset + strikethrough->thickness
    };
    // 填充矩形
    UIManager_RenderTarget->FillRectangle(&rectangle, m_pBrush);
    return S_OK;
}

// -------------------- LongUI::Component::Elements --------------------
// 实现

// Elements<Basic> Init
void LongUI::Component::Elements<LongUI::Element_Basic>::
Init(pugi::xml_node node, const char* prefix) noexcept {
    // 无效?
    const char* str = nullptr;
    // 动画类型
    animation.type = Helper::XMLGetAnimationType(
        node,
        AnimationType::Type_QuadraticEaseOut,
        "animationtype",
        prefix
        );
    // 动画持续时间
    if ((str = Helper::XMLGetValue(node, "animationduration", prefix))) {
        animation.duration = LongUI::AtoF(str);
    }
}

// 设置新的状态
auto LongUI::Component::Elements<LongUI::Element_Basic>::
SetNewStatus(LongUI::ControlStatus new_status) noexcept ->float {
    m_state = m_stateTartget;
    m_stateTartget = new_status;
    // 剩余值
    animation.start = 0.f;
    animation.value = 0.f;
    // 剩余时间
    return animation.time = animation.duration;
}

// Elements<Meta> 构造函数
LongUI::Component::Elements<LongUI::Element_Meta>::
Elements(pugi::xml_node node, const char* prefix) noexcept: Super(node, prefix) {
    ZeroMemory(m_metas, sizeof(m_metas));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 禁用状态Meta ID
    m_aID[Status_Disabled] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "disabledmeta", prefix)
        ));
    // 通常状态Meta ID
    m_aID[Status_Normal] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "normalmeta", prefix)
        ));
    // 移上状态Meta ID
    m_aID[Status_Hover] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "hovermeta", prefix)
        ));
        // 按下状态Meta ID
    m_aID[Status_Pushed] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "pushedmeta", prefix)
        ));
}


// Elements<Meta> 重建
auto LongUI::Component::Elements<LongUI::Element_Meta>::
Recreate() noexcept ->HRESULT {
    for (auto i = 0u; i < STATUS_COUNT; ++i) {
        // 有效
        register auto id = m_aID[i];
        if (id) {
            UIManager.GetMeta(id, m_metas[i]);
        }
    }
    return S_OK;
}

// Elements<Meta> 渲染
void LongUI::Component::Elements<LongUI::Element_Meta>::Render(const D2D1_RECT_F& rect) const noexcept {
    assert(UIManager_RenderTarget);
    // 先绘制当前状态
    if (this->animation.value < this->animation.end) {
        LongUI::Meta_Render(
            m_metas[m_state], UIManager_RenderTarget, rect, this->animation.end
            );
    }
    // 再绘制目标状态
    LongUI::Meta_Render(
        m_metas[m_stateTartget], UIManager_RenderTarget, rect, this->animation.value
        );
}



// Elements<BrushRect> 构造函数
LongUI::Component::Elements<LongUI::Element_BrushRect>::
Elements(pugi::xml_node node, const char* prefix) noexcept :Super(node, prefix) {
    ZeroMemory(m_apBrushes, sizeof(m_apBrushes));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 禁用状态笔刷ID
    m_aID[Status_Disabled] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "disabledbrush", prefix)
        ));
    // 通常状态笔刷ID
    m_aID[Status_Normal] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "normalbrush", prefix)
        ));
    // 移上状态笔刷ID
    m_aID[Status_Hover] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "hoverbrush", prefix)
        ));
    // 按下状态笔刷ID
    m_aID[Status_Pushed] = static_cast<uint16_t>(LongUI::AtoI(
        Helper::XMLGetValue(node, "pushedbrush", prefix)
        ));
}

// 释放数据
void LongUI::Component::Elements<LongUI::Element_BrushRect>::release_data() noexcept {
    for (auto& brush : m_apBrushes) {
        ::SafeRelease(brush);
    }
}

// Elements<BrushRectta> 渲染
void LongUI::Component::Elements<LongUI::Element_BrushRect>::Render(const D2D1_RECT_F& rect) const noexcept {
    assert(UIManager_RenderTarget);
    // 先绘制当前状态
    if (animation.value < animation.end) {
        LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, m_apBrushes[m_state], rect);
    }
    // 后绘制目标状态
    auto brush = m_apBrushes[m_stateTartget];
    brush->SetOpacity(animation.value);
    LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, brush, rect);
    brush->SetOpacity(1.f);
}

// Elements<BrushRect> 重建
auto LongUI::Component::Elements<LongUI::Element_BrushRect>::
Recreate() noexcept ->HRESULT {
    this->release_data();
    for (auto i = 0u; i < STATUS_COUNT; ++i) {
        register auto id = m_aID[i];
        m_apBrushes[i] = id ? UIManager.GetBrush(id) : UIManager.GetSystemBrush(i);
    }
    return S_OK;
}

// Elements<ColorRect> 构造函数
LongUI::Component::Elements<LongUI::Element_ColorRect>::
Elements(pugi::xml_node node, const char* prefix) noexcept: Super(node, prefix) {
    // 初始值
    colors[Status_Disabled] = D2D1::ColorF(0xDEDEDEDE);
    colors[Status_Normal] = D2D1::ColorF(0xCDCDCDCD);
    colors[Status_Hover] = D2D1::ColorF(0xA9A9A9A9);
    colors[Status_Pushed] = D2D1::ColorF(0x78787878);
    // 禁用状态颜色
    Helper::MakeColor(Helper::XMLGetValue(node, "disabledcolor", prefix), colors[Status_Disabled]);
    // 通常状态颜色
    Helper::MakeColor(Helper::XMLGetValue(node, "normalcolor", prefix), colors[Status_Normal]);
    // 移上状态颜色
    Helper::MakeColor(Helper::XMLGetValue(node, "hovercolor", prefix), colors[Status_Hover]);
    // 按下状态颜色
    Helper::MakeColor(Helper::XMLGetValue(node, "pushedcolor", prefix), colors[Status_Pushed]);
}

// Elements<ColorRect> 重建
auto LongUI::Component::Elements<LongUI::Element_ColorRect>::
Recreate() noexcept ->HRESULT {
    ::SafeRelease(m_pBrush);
    m_pBrush = static_cast<ID2D1SolidColorBrush*>(UIManager.GetBrush(LongUICommonSolidColorBrushIndex));
    return S_OK;
}

// Elements<ColorRect> 渲染
void LongUI::Component::Elements<LongUI::Element_ColorRect>::Render(const D2D1_RECT_F& rect) const noexcept {
    assert(UIManager_RenderTarget && m_pBrush);
    // 先绘制当前状态
    if (animation.value < animation.end) {
        m_pBrush->SetColor(colors + m_state);
        UIManager_RenderTarget->FillRectangle(&rect, m_pBrush);
    }
    // 再绘制目标状态
    m_pBrush->SetOpacity(animation.value);
    m_pBrush->SetColor(colors + m_stateTartget);
    UIManager_RenderTarget->FillRectangle(&rect, m_pBrush);
    m_pBrush->SetOpacity(1.f);
}
                   
                    


// -------------------------- UIVerticalLayout -------------------------
// UIVerticalLayout 创建
auto LongUI::UIVerticalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIVerticalLayout>(
            node,
            [=](void* p) noexcept { new(p) UIVerticalLayout(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}

// 更新子控件布局
void LongUI::UIVerticalLayout::Update() noexcept {
    // 前向刷新
    this->BeforeUpdate();
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    if (this->IsControlSizeChanged()) {
        // 初始化
        float base_width = 0.f, base_height = 0.f;
        float basic_weight = 0.f;
        /*
        if (m_strControlName == L"V") {
            int bk = 0;
        }*/
        // 第一次
        for (auto ctrl : (*this)) {
            // 非浮点控件
            if (!(ctrl->flags & Flag_Floating)) {
                // 宽度固定?
                if (ctrl->flags & Flag_WidthFixed) {
                    base_width = std::max(base_width, ctrl->GetTakingUpWidth());
                }
                // 高度固定?
                if (ctrl->flags & Flag_HeightFixed) {
                    base_height += ctrl->GetTakingUpHeight();
                }
                // 未指定高度?
                else {
                    basic_weight += ctrl->weight;
                }
            }
        }
        // 带入控件本身宽度计算
        base_width = std::max(base_width, this->GetViewWidthZoomed());
        // 剩余高度富余
        register auto height_remain = std::max(this->GetViewHeightZoomed() - base_height, 0.f);
        // 单位权重高度
        auto height_in_unit_weight = basic_weight > 0.f ? height_remain / basic_weight : 0.f ;
        // 基线Y
        float position_y = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 浮点控
            if (ctrl->flags & Flag_Floating) continue;
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                ctrl->SetWidth(base_width);
            }
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                ctrl->SetHeight(std::max(height_in_unit_weight * ctrl->weight, float(LongUIAutoControlMinSize)));
            }
            // 容器?
            // 不管如何, 修改!
            ctrl->SetControlSizeChanged();
            ctrl->SetLeft(0.f);
            ctrl->SetTop(position_y);
            position_y += ctrl->GetTakingUpHeight();
        }
        // 修改
        m_2fContentSize.width = base_width * m_2fZoom.width;
        m_2fContentSize.height = position_y * m_2fZoom.height;
        /*if (m_strControlName == L"MainWindow") {
            int a = 0;
        }*/
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 父类刷新
    Super::Update();
}

// UIVerticalLayout 关闭控件
void LongUI::UIVerticalLayout::Cleanup() noexcept {
    delete this;
}

// -------------------------- UIHorizontalLayout -------------------------
// UIHorizontalLayout 创建
auto LongUI::UIHorizontalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIHorizontalLayout>(
            node,
            [=](void* p) noexcept { new(p) UIHorizontalLayout(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// 更新子控件布局
void LongUI::UIHorizontalLayout::Update() noexcept {
    // 前向刷新
    this->BeforeUpdate();
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    if (this->IsControlSizeChanged()) {
        // 初始化
        float base_width = 0.f, base_height = 0.f;
        float basic_weight = 0.f;
        // 第一次
        for (auto ctrl : (*this)) {
            // 非浮点控件
            if (!(ctrl->flags & Flag_Floating)) {
                // 高度固定?
                if (ctrl->flags & Flag_HeightFixed) {
                    base_height = std::max(base_height, ctrl->GetTakingUpHeight());
                }
                // 宽度固定?
                if (ctrl->flags & Flag_WidthFixed) {
                    base_width += ctrl->GetTakingUpWidth();
                }
                // 未指定宽度?
                else {
                    basic_weight += ctrl->weight;
                }
            }
        }
        // 计算
        base_height = std::max(base_height, this->GetViewHeightZoomed());
        // 剩余宽度富余
        register auto width_remain = std::max(this->GetViewWidthZoomed() - base_width, 0.f);
        // 单位权重宽度
        auto width_in_unit_weight = basic_weight > 0.f ? width_remain / basic_weight : 0.f;
        // 基线X
        float position_x = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 跳过浮动控件
            if (ctrl->flags & Flag_Floating) continue;
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                ctrl->SetHeight(base_height);
            }
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                ctrl->SetWidth(std::max(width_in_unit_weight * ctrl->weight, float(LongUIAutoControlMinSize)));
            }
            // 不管如何, 修改!
            ctrl->SetControlSizeChanged();
            ctrl->SetLeft(position_x);
            ctrl->SetTop(0.f);
            position_x += ctrl->GetTakingUpWidth();
        }
        // 修改
        m_2fContentSize.width = position_x;
        m_2fContentSize.height = base_height;
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 父类刷新
    Super::Update();
}


// UIHorizontalLayout 关闭控件
void LongUI::UIHorizontalLayout::Cleanup() noexcept {
    delete this;
}


                   
                    

// Core Contrl for UIControl, UIMarginalable, UIContainer, UINull

// 系统按钮:
/*
Win8/8.1/10.0.10158之前
焦点: 0x3399FF 矩形描边, 并且内边有虚线矩形
0. 禁用: 0xD9灰度 矩形描边; 中心 0xEF灰色
1. 普通: 0xAC灰度 矩形描边; 中心 从上到下0xF0灰色到0xE5灰色渐变
2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变
*/

/// <summary>
/// Initializes a new instance of the <see cref="LongUI::UIControl"/>
/// class with xml node
/// </summary>
/// <param name="node" type="pugi::xml_node">The xml node</param>
/// <remarks>
/// For this function, param 'node' could be null node
/// 对于本函数, 参数'node'允许为空
/// <see cref="LongUINullXMLNode"/>
/// </remarks>
LongUI::UIControl::UIControl(pugi::xml_node node) noexcept {
    // 构造默认
    auto flag = LongUIFlag::Flag_None;
    // 有效?
    if (node) {
        // 调试
#ifdef _DEBUG
        this->debug_this = node.attribute("debug").as_bool(false);
#endif
        const char* data = nullptr;
        // 检查脚本
        if ((data = node.attribute(XMLAttribute::Script).value()) && UIManager.script) {
            m_script = UIManager.script->AllocScript(data);
        }
        // 检查权重
        if (data = node.attribute(LongUI::XMLAttribute::LayoutWeight).value()) {
            force_cast(this->weight) = LongUI::AtoF(data);
        }
        // 检查背景笔刷
        if (data = node.attribute(LongUI::XMLAttribute::BackgroudBrush).value()) {
            m_idBackgroudBrush = uint32_t(LongUI::AtoI(data));
            if (m_idBackgroudBrush) {
                assert(!m_pBackgroudBrush);
                m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
            }
        }
        // 检查可视性
        this->visible = node.attribute(LongUI::XMLAttribute::Visible).as_bool(true);
        // 渲染优先级
        if (data = node.attribute(LongUI::XMLAttribute::RenderingPriority).value()) {
            force_cast(this->priority) = uint8_t(LongUI::AtoI(data));
        }
        // 检查名称
        Helper::MakeString(
            node.attribute(LongUI::XMLAttribute::ControlName).value(),
            m_strControlName
            );
        // 检查外边距
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::Margin).value(),
            const_cast<float*>(&margin_rect.left),
            sizeof(margin_rect) / sizeof(margin_rect.left)
            );
        // 检查渲染父控件
        if (node.attribute(LongUI::XMLAttribute::IsRenderParent).as_bool(false)) {
            flag |= LongUI::Flag_RenderParent;
        }
        // 检查裁剪规则
        if (node.attribute(LongUI::XMLAttribute::IsClipStrictly).as_bool(true)) {
            flag |= LongUI::Flag_ClipStrictly;
        }
        // 边框大小
        if (data = node.attribute(LongUI::XMLAttribute::BorderWidth).value()) {
            m_fBorderWidth = LongUI::AtoF(data);
        }
        // 边框圆角
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::BorderRound).value(),
            &m_2fBorderRdius.width,
            sizeof(m_2fBorderRdius) / sizeof(m_2fBorderRdius.width)
            );
        // 检查控件大小
        {
            float size[] = { 0.f, 0.f };
            Helper::MakeFloats(
                node.attribute(LongUI::XMLAttribute::AllSize).value(),
                size, lengthof(size)
                );
            // 视口区宽度固定?
            if (size[0] > 0.f) {
                flag |= LongUI::Flag_WidthFixed;
                this->SetWidth(size[0]);
            }
            // 视口区高度固固定?
            if (size[1] > 0.f) {
                flag |= LongUI::Flag_HeightFixed;
                this->SetHeight(size[1]);
            }
        }
        // 检查控件位置
        {
            float pos[] = { 0.f, 0.f };
            Helper::MakeFloats(
                node.attribute(LongUI::XMLAttribute::LeftTopPosotion).value(),
                pos, lengthof(pos)
                );
            // 指定X轴
            if (pos[0] != 0.f) {
                this->SetLeft(pos[0]);
                flag |= Flag_Floating;
            }
            // 指定Y轴
            if (pos[1] != 0.f) {
                this->SetTop(pos[1]);
                flag |= Flag_Floating;
            }
        }
    }
    // 修改flag
    force_cast(this->flags) |= flag;
}

// 析构函数
LongUI::UIControl::~UIControl() noexcept {
    ::SafeRelease(m_pBrush_SetBeforeUse);
    ::SafeRelease(m_pBackgroudBrush);
    // 释放脚本占用空间
    if (m_script.script) {
        assert(UIManager.script && "no script interface but data");
        UIManager.script->FreeScript(m_script);
    }
    // 反注册
    if (this->flags & Flag_NeedRegisterOffScreenRender) {
        m_pWindow->UnRegisterOffScreenRender(this);
    }
}


/// <summary>
/// Render control via specified render-type.
/// </summary>
/// <param name="_type" type="enum LongUI::RenderType">The _type.</param>
/// <returns></returns>
void LongUI::UIControl::Render(RenderType type) const noexcept {
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 背景
        if (m_pBackgroudBrush) {
            D2D1_RECT_F rect; this->GetViewRect(rect);
            LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, m_pBackgroudBrush, rect);
        }
        // 背景中断
        if (type == RenderType::Type_RenderBackground) {
            break;
        }
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 渲染边框
        if (m_fBorderWidth > 0.f) {
            D2D1_ROUNDED_RECT brect; this->GetBorderRect(brect.rect);
            m_pBrush_SetBeforeUse->SetColor(&m_colorBorderNow);
            if (m_2fBorderRdius.width > 0.f && m_2fBorderRdius.height > 0.f) {
                brect.radiusX = m_2fBorderRdius.width;
                brect.radiusY = m_2fBorderRdius.height;
                //UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                UIManager_RenderTarget->DrawRoundedRectangle(&brect, m_pBrush_SetBeforeUse, m_fBorderWidth);
                //UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
            else {
                UIManager_RenderTarget->DrawRectangle(&brect.rect, m_pBrush_SetBeforeUse, m_fBorderWidth);
            }
        }
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}


// UI控件: 刷新
void LongUI::UIControl::Update() noexcept {
    // 控件大小处理了
    if (m_bool16.Test(Index_ChangeSizeHandled)) {
        m_bool16.SetFalse(Index_ChangeSize);
        m_bool16.SetFalse(Index_ChangeSizeHandled);
    }
    // 世界转换处理了
    if (m_bool16.Test(Index_ChangeWorldHandled)) {
        m_bool16.SetFalse(Index_ChangeWorld);
        m_bool16.SetFalse(Index_ChangeWorldHandled);
    }
}

// UI控件: 重建
auto LongUI::UIControl::Recreate() noexcept ->HRESULT {
    // 设备重置再说
    ::SafeRelease(m_pBrush_SetBeforeUse);
    ::SafeRelease(m_pBackgroudBrush);
    m_pBrush_SetBeforeUse = static_cast<decltype(m_pBrush_SetBeforeUse)>(
        UIManager.GetBrush(LongUICommonSolidColorBrushIndex)
        );
    if (m_idBackgroudBrush) {
        m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
    }
    return S_OK;
}

// LongUI::UIControl 注册回调事件
void LongUI::UIControl::SetSubEventCallBack(
    const wchar_t* control_name, LongUI::SubEvent event, SubEventCallBack call) noexcept {
    assert(control_name && call&&  "bad argument");
    UIControl* control = m_pWindow->FindControl(control_name);
    assert(control && " no control found");
    if (!control) return;
    CUISubEventCaller caller(call, this);
    // 自定义消息?
    if (event >= LongUI::SubEvent::Event_Custom) {
        UIManager.configure->SetSubEventCallBack(event, caller, control);
        return;
    }
    // 检查
    switch (event)
    {
    case LongUI::SubEvent::Event_ButtonClicked:
        longui_cast<UIButton*>(control)->SetClickEvent(caller);
        break;
    case LongUI::SubEvent::Event_EditReturned:
        longui_cast<UIEditBasic*>(control)->SetReturnEvent(caller);
        break;
    case LongUI::SubEvent::Event_SliderValueChanged:
        longui_cast<UISlider*>(control)->SetValueChangedEvent(caller);
        break;
    }
}

// 获取占用宽度
auto LongUI::UIControl::GetTakingUpWidth() const noexcept -> float {
    return this->view_size.width
        + margin_rect.left
        + margin_rect.right
        + m_fBorderWidth * 2.f;
}

// 获取占用高度
auto LongUI::UIControl::GetTakingUpHeight() const noexcept -> float {
    return this->view_size.height
        + margin_rect.top
        + margin_rect.bottom
        + m_fBorderWidth * 2.f;
}

// 获取非内容区域总宽度
auto LongUI::UIControl::GetNonContentWidth() const noexcept -> float {
    return margin_rect.left
        + margin_rect.right
        + m_fBorderWidth * 2.f;
}

// 获取非内容区域总高度
auto LongUI::UIControl::GetNonContentHeight() const noexcept -> float {
    return margin_rect.top
        + margin_rect.bottom
        + m_fBorderWidth * 2.f;
}

// 设置占用宽度
auto LongUI::UIControl::SetWidth(float width) noexcept -> void {
    // 设置
    auto new_vwidth = width - this->GetNonContentWidth();
    if (new_vwidth != this->view_size.width) {
        force_cast(this->view_size.width) = new_vwidth;
        this->SetControlSizeChanged();
    }
    // 检查
    if (this->view_size.width < 0.f) {
        UIManager << DL_Hint << this
            << "viewport's width < 0: " << this->view_size.width << endl;
    }
}

// 设置占用高度
auto LongUI::UIControl::SetHeight(float height) noexcept -> void LongUINoinline {
    // 设置
    auto new_vheight = height - this->GetNonContentHeight();
    if (new_vheight != this->view_size.height) {
        force_cast(this->view_size.height) = new_vheight;
        this->SetControlSizeChanged();
    }
    // 检查
    if (this->view_size.height < 0.f) {
        UIManager << DL_Hint << this
            << "viewport's height < 0: " << this->view_size.height << endl;
    }
}

// 设置控件左坐标
auto LongUI::UIControl::SetLeft(float left) noexcept -> void {
    auto new_left = left + this->margin_rect.left + m_fBorderWidth;
    // 修改了位置?
    if (this->view_pos.x != new_left) {
        force_cast(this->view_pos.x) = new_left;
        this->SetControlWorldChanged();
    }
}

// 设置控件顶坐标
auto LongUI::UIControl::SetTop(float top) noexcept -> void {
    auto new_top = top + this->margin_rect.top + m_fBorderWidth;
    // 修改了位置?
    if (this->view_pos.y != new_top) {
        force_cast(this->view_pos.y) = new_top;
        this->SetControlWorldChanged();
    }
}

// 获取占用/剪切矩形
void LongUI::UIControl::GetClipRect(D2D1_RECT_F& rect) const noexcept {
    rect.left = -(this->margin_rect.left + m_fBorderWidth);
    rect.top = -(this->margin_rect.top + m_fBorderWidth);
    rect.right = this->view_size.width + this->margin_rect.right + m_fBorderWidth;
    rect.bottom = this->view_size.height + this->margin_rect.bottom + m_fBorderWidth;
}

// 获取边框矩形
void LongUI::UIControl::GetBorderRect(D2D1_RECT_F& rect) const noexcept {
    rect.left = -m_fBorderWidth * 0.5f;
    rect.top = -m_fBorderWidth * 0.5f;
    rect.right = this->view_size.width + m_fBorderWidth * 0.5f;
    rect.bottom = this->view_size.height + m_fBorderWidth * 0.5f;
}

// 获取视口刻画矩形
void LongUI::UIControl::GetViewRect(D2D1_RECT_F& rect) const noexcept {
    rect.left = 0.f;
    rect.top = 0.f;
    rect.right = this->view_size.width;
    rect.bottom = this->view_size.height;
}

// 获得世界转换矩阵
void LongUI::UIControl::RefreshWorld() noexcept {
    float xx = this->view_pos.x /*+ this->margin_rect.left + m_fBorderWidth*/;
    float yy = this->view_pos.y /*+ this->margin_rect.top + m_fBorderWidth*/;
    // 顶级控件
    if (this->IsTopLevel()) {
        this->world = D2D1::Matrix3x2F::Translation(xx, yy);
    }
    // 非顶级控件
    else {
#if 1
        // 检查
        xx += this->parent->GetOffsetXZoomed();
        yy += this->parent->GetOffsetYZoomed();
        // 转换
        this->world = 
            D2D1::Matrix3x2F::Translation(xx, yy) 
            *D2D1::Matrix3x2F::Scale(
                this->parent->GetZoomX(), this->parent->GetZoomY()
                )
            * this->parent->world;
#else
        this->world = 
            D2D1::Matrix3x2F::Translation(xx, yy)
            * D2D1::Matrix3x2F::Scale(
                this->parent->GetZoomX(), this->parent->GetZoomY()
                )
            * D2D1::Matrix3x2F::Translation(
                this->parent->GetOffsetX(), this->parent->GetOffsetY()
                ) 
            * this->parent->world;
#endif
    }
    // 修改了
    this->ControlWorldChangeHandled();
#ifdef _DEBUG
    if (this->debug_this) {
        UIManager << DL_Log << this << "WORLD: " << this->world << LongUI::endl;
    }
#endif
}

// 获得世界转换矩阵 for 边缘控件
void LongUI::UIMarginalable::RefreshWorldMarginal() noexcept {
    float xx = this->view_pos.x /*+ this->margin_rect.left + m_fBorderWidth*/;
    float yy = this->view_pos.y /*+ this->margin_rect.top + m_fBorderWidth*/;
    D2D1_MATRIX_3X2_F identity;
    D2D1_MATRIX_3X2_F* parent_world = &identity;
    // 顶级
    identity = D2D1::Matrix3x2F::Identity();
    if (this->parent->IsTopLevel()) {
        identity = D2D1::Matrix3x2F::Identity();
    }
    else {
        parent_world = &this->parent->world;
    }
    // 计算矩阵
    this->world = D2D1::Matrix3x2F::Translation(xx, yy) ** parent_world;
    // 自己不能是顶级的
    assert(this->IsTopLevel() == false);
    constexpr int aa = sizeof(UIContainer);
}

// ----------------------------------------------------------------------------
// UINull
// ----------------------------------------------------------------------------

// LongUI namespace
namespace LongUI {
    // null control
    class UINull : public UIControl {
    private:
        // 父类申明
        using Super = UIControl;
    public:
        // Render 渲染
        virtual void Render(RenderType) const noexcept override {}
        // update 刷新
        virtual void Update() noexcept override {}
        // do event 事件处理
        virtual bool DoEvent(const LongUI::EventArgument&) noexcept override { return false; }
        // close this control 关闭控件
        virtual void Cleanup() noexcept override { delete this; }
    public:
        // 创建控件
        static auto CreateControl(pugi::xml_node node) noexcept {
            UIControl* pControl = nullptr;
            // 判断
            if (!node) {
                UIManager << DL_Warning << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = LongUI::UIControl::AllocRealControl<LongUI::UINull>(
                node,
                [=](void* p) noexcept { new(p) UINull(node); }
            );
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            return pControl;
        }
    public:
        // constructor 构造函数
        UINull(pugi::xml_node node) noexcept : Super(node) {}
        // destructor 析构函数
        ~UINull() noexcept { }
    };
}

// 创建空控件
auto WINAPI LongUI::CreateNullControl(CreateEventType type, pugi::xml_node node) noexcept -> UIControl * {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        pControl = LongUI::UINull::CreateControl(node);
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// ------------------------------ UIContainer -----------------------------
// UIContainer 构造函数
LongUI::UIContainer::UIContainer(pugi::xml_node node) noexcept : Super(node), marginal_control() {
    ::memset(force_cast(marginal_control), 0, sizeof(marginal_control));
    // LV
    /*if (m_strControlName == L"V") {
        m_2fZoom = { 1.0f, 1.0f };
    }*/
    // 保留原始外间距
    m_orgMargin = this->margin_rect;
    auto flag = this->flags | Flag_UIContainer;
    // 有效
    if (node) {
        // 模板大小
        Helper::MakeFloats( 
            node.attribute(LongUI::XMLAttribute::TemplateSize).value(),
            &m_2fTemplateSize.width, 2
            );
        // 检查边缘控件: 属性ID
        const char* const attname[] = {
            LongUI::XMLAttribute::LeftMarginalControl,
            LongUI::XMLAttribute::TopMarginalControl,
            LongUI::XMLAttribute::RightMarginalControl,
            LongUI::XMLAttribute::BottomMarginalControl,
        };
        bool exist_marginal_control = false;
        // ONLY MY LOOPGUN
        for (auto i = 0u; i < UIMarginalable::MARGINAL_CONTROL_SIZE; ++i) {
            const char* str = nullptr;
            // 获取指定属性值
            if ((str = node.attribute(attname[i]).value())) {
                Helper::CC cc = { 0 };
                auto cccount = Helper::MakeCC(str, &cc);
                assert(cccount == 1);
                // 有效
                if (cc.func) {
                    // 创建控件
                    auto control = UIManager.CreateControl(cc.id, cc.func);
                    // XXX: 检查
                    force_cast(this->marginal_control[i]) = longui_cast<UIMarginalable*>(control);
                }
                else {
                    assert(!"cc.func -> null");
                }
                // 优化flag
                if (this->marginal_control[i]) {
                    exist_marginal_control = true;
                }
            }
        }
        // 存在
        if (exist_marginal_control) {
            flag |= Flag_Container_ExistMarginalControl;
        }
        // 渲染依赖属性
        if (node.attribute(XMLAttribute::IsHostChildrenAlways).as_bool(false)) {
            flag |= LongUI::Flag_Container_HostChildrenRenderingDirectly;
        }
        // 渲染依赖属性
        if (node.attribute(XMLAttribute::IsHostPosterityAlways).as_bool(false)) {
            flag |= LongUI::Flag_Container_HostPosterityRenderingDirectly;
        }
        // 边缘控件缩放
        if (node.attribute(XMLAttribute::IsZoomMarginalControl).as_bool(true)) {
            flag |= LongUI::Flag_Container_ZoomMarginalControl;
        }
    }
    // 修改完毕
    force_cast(this->flags) = flag;
}

// UIContainer 析构函数
LongUI::UIContainer::~UIContainer() noexcept {
    // 关闭边缘控件
    // 只有一次 Flag_Container_ExistMarginalControl 可用可不用
    for (auto ctrl : this->marginal_control) {
        if (ctrl) {
            ctrl->Cleanup();
        }
    }
    // 关闭子控件
    {
        auto ctrl = m_pHead;
        while (ctrl) {
            auto next_ctrl = ctrl->next;
            ctrl->Cleanup();
            ctrl = next_ctrl;
        }
    }
}

// 插入后处理
void LongUI::UIContainer::AfterInsert(UIControl* child) noexcept {
    assert(child && "bad argument");
    // 大小判断
    if (this->GetCount() >= 10'000) {
        UIManager << DL_Warning << "the count of children must be"
            " less than 10k because of the precision of float" << LongUI::endl;
        assert(!"because of the precision of float, the count of children must be less than 10k");
    }
    // 检查flag
    const auto host_flag = Flag_Container_HostChildrenRenderingDirectly 
        | Flag_Container_HostPosterityRenderingDirectly;
    if (this->flags & host_flag) {
        force_cast(child->flags) |= Flag_RenderParent;
    }
    // 子控件也是容器?
    if (this->flags & Flag_Container_HostPosterityRenderingDirectly
        && child->flags & Flag_UIContainer) {
        force_cast(child->flags) |= Flag_Container_HostPosterityRenderingDirectly;
    }
    // 设置父节点
    force_cast(child->parent) = this;
    // 设置窗口节点
    child->m_pWindow = m_pWindow;
    // 重建资源
    child->Recreate();
    // 修改
    child->SetControlSizeChanged();
    // 修改
    this->SetControlSizeChanged();
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIContainer::FindControl(const D2D1_POINT_2F pt) noexcept->UIControl* {
    // 查找边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl && IsPointInRect(ctrl->visible_rect, pt)) {
                return ctrl;
            }
        }
    }
    this->AssertMarginalControl();
    UIControl* control_out = nullptr;
    // XXX: 优化
    assert(this->GetCount() < 100 && "too huge, wait for optimization please");
    for (auto ctrl : (*this)) {
        /*if (m_strControlName == L"MainWindow") {
        int a = 9;
        }*/
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            if (ctrl->flags & Flag_UIContainer) {
                control_out = static_cast<UIContainer*>(ctrl)->FindControl(pt);
            }
            else {
                control_out = ctrl;
            }
            break;
        }
    }
    return control_out;
}


// do event 事件处理
bool LongUI::UIContainer::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // TODO: 参数EventArgument改为const
    bool done = false;
    // 转换坐标
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 初始化边缘控件 
            for (auto i = 0; i < lengthof(this->marginal_control); ++i) {
                auto ctrl = this->marginal_control[i];
                if (ctrl) {
                    this->AfterInsert(ctrl);
                    // 初始化
                    ctrl->InitMarginalControl(static_cast<UIMarginalable::MarginalControl>(i));
                    // 完成控件树
                    ctrl->DoEvent(arg);
                }
            }
            // 初次完成空间树建立
            for (auto ctrl : (*this)) {
                ctrl->DoEvent(arg);
            }
            done = true;
            break;
        }
    }
    // 扳回来
    return done;
}

// UIContainer 渲染函数
void LongUI::UIContainer::Render(RenderType type) const noexcept {
    //  正确渲染控件
    auto do_render = [](const UIControl* const ctrl) {
        // 可渲染?
        if (ctrl->visible && ctrl->visible_rect.right > ctrl->visible_rect.left
            && ctrl->visible_rect.bottom > ctrl->visible_rect.top) {
            // 修改世界转换矩阵
            UIManager_RenderTarget->SetTransform(&ctrl->world);
            // 检查剪切规则
            if (ctrl->flags & Flag_ClipStrictly) {
                D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
                UIManager_RenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
            ctrl->Render(LongUI::RenderType::Type_Render);
            // 检查剪切规则
            if (ctrl->flags & Flag_ClipStrictly) {
                UIManager_RenderTarget->PopAxisAlignedClip();
            }
        }
    };
    // 查看
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
        // 普通子控件仅仅允许渲染在内容区域上
        {
            D2D1_RECT_F clip_rect; this->GetViewRect(clip_rect);
            UIManager_RenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        // 渲染所有子部件
        for (const auto* ctrl : (*this)) {
            do_render(ctrl);
        }
        // 弹出
        UIManager_RenderTarget->PopAxisAlignedClip();
        // 渲染边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            for (auto ctrl : this->marginal_control) {
                if (ctrl) {
                    do_render(ctrl);
                }
            }
        }
        this->AssertMarginalControl();
        // 回退转变
        UIManager_RenderTarget->SetTransform(&this->world);
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// 更新边缘控件
void LongUI::UIContainer::refresh_marginal_controls() noexcept {
    // 获取宽度
    auto get_marginal_width = [](UIMarginalable* ctrl) noexcept {
        return ctrl ? ctrl->marginal_width : 0.f;
    };
    // 利用规则获取宽度
    auto get_marginal_width_with_rule = [](UIMarginalable* a, UIMarginalable* b) noexcept {
        return a->rule == UIMarginalable::Rule_Greedy ? 0.f : (b ? b->marginal_width : 0.f);
    };
    // 计算宽度
    auto caculate_container_width = [this, get_marginal_width]() noexcept {
        // 基本宽度
        return this->view_size.width
            + m_orgMargin.left
            + m_orgMargin.right
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Left])
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Right])
            + m_fBorderWidth * 2.f;
    };
    // 计算高度
    auto caculate_container_height = [this, get_marginal_width]() noexcept {
        // 基本宽度
        return this->view_size.height
            + m_orgMargin.top
            + m_orgMargin.bottom
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Top])
            + get_marginal_width(this->marginal_control[UIMarginalable::Control_Bottom])
            + m_fBorderWidth * 2.f;
    };
    // 保留信息
    const float this_container_width = caculate_container_width();
    const float this_container_height = caculate_container_height();
    /*if (m_strControlName == L"V") {
        int bk = 9;
    }*/
    // 循环
    while (true) {
        for (auto i = 0u; i < lengthof(this->marginal_control); ++i) {
            // 获取控件
            auto ctrl = this->marginal_control[i]; if (!ctrl) continue;
            //float view[] = { 0.f, 0.f, 0.f, 0.f };
            // TODO: 计算cross 大小
            switch (i)
            {
            case 0: // Left
            {
                const auto tmptop = m_orgMargin.top +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Top]);
                // 坐标
                ctrl->SetLeft(m_orgMargin.left);
                ctrl->SetTop(tmptop);
                // 大小
                ctrl->SetWidth(ctrl->marginal_width);
                ctrl->SetHeight(
                    this_container_height - tmptop - m_orgMargin.bottom -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Bottom])
                    );
            }
            break;
            case 1: // Top
            {
                const float tmpleft = m_orgMargin.left +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Left]);
                // 坐标
                ctrl->SetLeft(tmpleft);
                ctrl->SetTop(m_orgMargin.top);
                // 大小
                ctrl->SetWidth(
                    this_container_width - tmpleft - m_orgMargin.right -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Right])
                    );
                ctrl->SetHeight(ctrl->marginal_width);
            }
            break;
            case 2: // Right
            {
                const auto tmptop = m_orgMargin.top +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Top]);
                // 坐标
                ctrl->SetLeft(this_container_width - m_orgMargin.right - ctrl->marginal_width);
                ctrl->SetTop(tmptop);
                // 大小
                ctrl->SetWidth(ctrl->marginal_width);
                ctrl->SetHeight(
                    this_container_height - tmptop - m_orgMargin.bottom -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Bottom])
                    );
            }
            break;
            case 3: // Bottom
            {
                const float tmpleft = m_orgMargin.left +
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Left]);
                // 坐标
                ctrl->SetLeft(tmpleft);
                ctrl->SetTop(this_container_height - m_orgMargin.bottom - ctrl->marginal_width);
                // 大小
                ctrl->SetWidth(
                    this_container_width - tmpleft - m_orgMargin.right -
                    get_marginal_width_with_rule(ctrl, this->marginal_control[UIMarginalable::Control_Right])
                    );
                ctrl->SetHeight(ctrl->marginal_width);
            }
            break;
            }
            // 更新边界
            ctrl->UpdateMarginalWidth();
        }
        // 退出检查
        {
            // 计算
            const float latest_width = caculate_container_width();
            const float latest_height = caculate_container_height();
            /*UIManager << DL_Hint
                << " latest_width: " << latest_width
                << " this_container_width: " << this_container_width
                << " latest_height: " << latest_height
                << " this_container_height: " << this_container_height
                << endl;*/
            // 一样就退出
            if (latest_width == this_container_width && latest_height == this_container_height) {
                break;
            }
            // 修改外边距
            force_cast(this->margin_rect.left) = m_orgMargin.left
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Left]);
            force_cast(this->margin_rect.top) = m_orgMargin.top
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Top]);
            force_cast(this->margin_rect.right) = m_orgMargin.right
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Right]);
            force_cast(this->margin_rect.bottom) = m_orgMargin.bottom
                + get_marginal_width(this->marginal_control[UIMarginalable::Control_Bottom]);
            // 修改大小
            this->SetWidth(this_container_width);
            this->SetHeight(this_container_height);
        }
    }
}

// UI容器: 刷新前
void LongUI::UIContainer::BeforeUpdate() noexcept {
    // 需要刷新
    if (this->IsNeedRefreshWorld()) {
        auto code = ((this->m_2fTemplateSize.width > 0.f) << 1) | 
            (this->m_2fTemplateSize.height > 0.f);
        auto tmpw = this->GetWidth() / m_2fTemplateSize.width;
        auto tmph = this->GetHeight() / m_2fTemplateSize.width;
        switch (code)
        {
        case 0:
            // do nothing
            break;
        case 1:
            // this->m_2fTemplateSize.height > 0.f, only
            this->m_2fZoom.width = this->m_2fZoom.height = tmph;
            break;
        case 2:
            // this->m_2fTemplateSize.width > 0.f, only
            this->m_2fZoom.height = this->m_2fZoom.width = tmpw;
            break;
        case 3:
            // both
            this->m_2fZoom.width =  tmpw;
            this->m_2fZoom.height = tmph;
            break;
        }
    }
}

// UI容器: 刷新
void LongUI::UIContainer::Update() noexcept {
    // 修改边界
    if (this->IsControlSizeChanged()) {
        // 刷新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            this->refresh_marginal_controls();
        }
        // 刷新
        /*if (should_update) {
            this->SetControlWorldChanged();
            this->Update();
        }*/
#ifdef _DEBUG
        if (this->debug_this) {
            UIManager << DL_Log << L"Container" << this
                << LongUI::Formated(L"Resize(%.1f, %.1f) Zoom(%.1f, %.1f)",
                    this->GetWidth(), this->GetHeight(),
                    m_2fZoom.width, m_2fZoom.height
                    ) << LongUI::endl;
        }
#endif
    }
    // 修改可视化区域
    if (this->IsNeedRefreshWorld()) {
        // 更新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            for (auto ctrl : this->marginal_control) {
                // 刷新
                if (ctrl) {
                    ctrl->Update();
                    // 更新世界矩阵
                    ctrl->SetControlWorldChanged();
                    ctrl->RefreshWorldMarginal();
                    // 坐标转换
                    D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
                    auto lt = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.left));
                    auto rb = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.right));
                    // 修改可视区域
                    ctrl->visible_rect.left = std::max(lt.x, this->visible_rect.left);
                    ctrl->visible_rect.top = std::max(lt.y, this->visible_rect.top);
                    ctrl->visible_rect.right = std::min(rb.x, this->visible_rect.right);
                    ctrl->visible_rect.bottom = std::min(rb.y, this->visible_rect.bottom);
                    //UIManager << DL_Hint << ctrl->visible_rect << endl;
                }
            }
        }
        // 本容器内容限制
        D2D1_RECT_F limit_of_this = {
            this->visible_rect.left + this->margin_rect.left * this->world._11,
            this->visible_rect.top + this->margin_rect.top * this->world._22,
            this->visible_rect.right - this->margin_rect.right * this->world._11,
            this->visible_rect.bottom - this->margin_rect.bottom * this->world._22,
        };
        // 更新一般控件
        for (auto ctrl : (*this)) {
            // 更新世界矩阵
            ctrl->SetControlWorldChanged();
            ctrl->RefreshWorld();
            // 坐标转换
            D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
            auto lt = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.left));
            auto rb = LongUI::TransformPoint(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(clip_rect.right));
            // 限制
            /*if (ctrl->IsCanbeCastedTo(LongUI::GetIID<UIVerticalLayout>())) {
            int bk = 9;
            }*/
            ctrl->visible_rect.left = std::max(lt.x, limit_of_this.left);
            ctrl->visible_rect.top = std::max(lt.y, limit_of_this.top);
            ctrl->visible_rect.right = std::min(rb.x, limit_of_this.right);
            ctrl->visible_rect.bottom = std::min(rb.y, limit_of_this.bottom);
            // 调试信息
            //UIManager << DL_Hint << ctrl << ctrl->visible_rect << endl;
        }
        // 调试信息
        if (this->IsTopLevel()) {
            //UIManager << DL_Log << "Handle: ControlSizeChanged" << LongUI::endl;
        }
        // 已处理该消息
        this->ControlSizeChangeHandled();
    }
    // 刷新边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
#if 1
        for (auto ctrl : this->marginal_control) {
            if (ctrl) ctrl->Update();
        }
#else
    {
        UIMarginalable* ctrl = nullptr;
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Left])) ctrl->Update();
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Top])) ctrl->Update();
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Right])) ctrl->Update();
        if ((ctrl = this->marginal_control[UIMarginalable::Control_Bottom])) ctrl->Update();
    }
#endif
    }
    this->AssertMarginalControl();
    // 刷新一般子控件
    for (auto ctrl : (*this)) ctrl->Update();
    // 刷新父类
    return Super::Update();
}


// UIContainer 重建
auto LongUI::UIContainer::Recreate() noexcept ->HRESULT {
    auto hr = S_OK;
    // 重建边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl && SUCCEEDED(hr)) {
                hr = ctrl->Recreate();
            }
        }
    }
    this->AssertMarginalControl();
    // 重建子类
    for (auto ctrl : (*this)) {
        if (SUCCEEDED(hr)) {
            hr = ctrl->Recreate();
        }
    }
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    return hr;
}

// 设置水平偏移值
LongUINoinline void LongUI::UIContainer::SetOffsetX(float value) noexcept {
    assert(value > -1'000'000.f && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    register float target = value;
    if (target != m_2fOffset.x) {
        m_2fOffset.x = target;
        this->SetControlWorldChanged();
    }
}

// 设置垂直偏移值
LongUINoinline void LongUI::UIContainer::SetOffsetY(float value) noexcept {
    assert(value > (-1'000'000.f) && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    register float target = value ;
    if (target != m_2fOffset.y) {
        m_2fOffset.y = target;
        this->SetControlWorldChanged();
    }
}

// 随机访问控件
auto LongUI::UIContainer::GetAt(uint32_t i) const noexcept -> UIControl * {
    // 性能警告
    if (i > 8) {
        UIManager << DL_Warning
            << L"Performance Warning! random accessig is not fine for list"
            << LongUI::endl;
    }
    // 检查范围
    if (i >= this->GetCount()) {
        UIManager << DL_Error << L"out of range" << LongUI::endl;
        return nullptr;
    }
    // 只有一个?
    if (this->GetCount() == 1) return m_pHead;
    // 前半部分?
    UIControl * control;
    if (i < this->GetCount() / 2) {
        control = m_pHead;
        while (i) {
            assert(control && "null pointer");
            control = control->next;
            --i;
        }
    }
    // 后半部分?
    else {
        control = m_pTail;
        i = static_cast<uint32_t>(this->GetCount()) - i - 1;
        while (i) {
            assert(control && "null pointer");
            control = control->prev;
            --i;
        }
    }
    return control;
}

// 插入控件
void LongUI::UIContainer::Insert(Iterator itr, UIControl* ctrl) noexcept {
    const auto end_itr = this->end();
    assert(ctrl && "bad arguments");
    if (ctrl->prev) {
        UIManager << DL_Warning
            << L"the 'prev' attr of the control: ["
            << ctrl->GetNameStr()
            << "] that to insert is not null"
            << LongUI::endl;
    }
    if (ctrl->next) {
        UIManager << DL_Warning
            << L"the 'next' attr of the control: ["
            << ctrl->GetNameStr()
            << "] that to insert is not null"
            << LongUI::endl;
    }
    // 插入尾部?
    if (itr == end_itr) {
        // 链接
        force_cast(ctrl->prev) = m_pTail;
        // 无尾?
        if (m_pTail) force_cast(m_pTail->next) = ctrl;
        // 无头?
        if (!m_pHead) m_pHead = ctrl;
        // 设置尾
        m_pTail = ctrl;
    }
    else {
        force_cast(ctrl->next) = itr.Ptr();
        force_cast(ctrl->prev) = itr->prev;
        if (itr->prev) {
            force_cast(itr->prev) = ctrl;
        }
        force_cast(itr->prev) = ctrl;
    }
    ++m_cChildrenCount;
    // 添加之后的处理
    this->AfterInsert(ctrl);
}


// 仅移除控件
bool LongUI::UIContainer::RemoveJust(Iterator itr) noexcept {
    // 检查是否属于本容器
#ifdef _DEBUG
    bool ok = false;
    for (auto i : (*this)) {
        if (itr == i) {
            ok = true;
            break;
        }
    }
    if (!ok) {
        UIManager << DL_Error << "control:[" << itr->GetNameStr()
            << "] not in this container: " << this->GetNameStr() << LongUI::endl;
        return false;
    }
#endif
    UIControl* child = itr.Ptr();
    {
        // 连接前后节点
        register auto prev_tmp = itr->prev;
        register auto next_tmp = itr->next;
        // 检查, 头
        (prev_tmp ? force_cast(prev_tmp->next) : m_pHead) = next_tmp;
        // 检查, 尾
        (next_tmp ? force_cast(next_tmp->prev) : m_pTail) = prev_tmp;
        // 减少
        force_cast(itr->prev) = force_cast(itr->next) = nullptr;
        --m_cChildrenCount;
        // 修改
        this->SetControlSizeChanged();
    }
    this->AfterRemove(child);
    return true;
}
 
                   

// ----------------------------------------------------------------------------
// **** UIText
// ----------------------------------------------------------------------------

// UI文本: 事件处理
bool LongUI::UIText::DoEvent(const LongUI::EventArgument& arg) noexcept {
    UNREFERENCED_PARAMETER(arg);
    return false;
}

// UI文本: 渲染
void LongUI::UIText::Render(RenderType type) const noexcept {
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 渲染文字
        m_text.Render(0.f, 0.f);
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UIText: 刷新
void LongUI::UIText::Update() noexcept {
    // 改变了大小
    if(this->IsControlSizeChanged()) {
        // 设置大小
        m_text.Resize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    return Super::Update();
}


/*/ UIText 构造函数
LongUI::UIText::UIText(pugi::xml_node node) noexcept: Super(node), m_text(node) {
    //m_bInitZoneChanged = true;
}
*/

// UIText::CreateControl 函数
auto LongUI::UIText::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIText>(
            node,
            [=](void* p) noexcept { new(p) UIText(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// recreate 重建
/*HRESULT LongUI::UIText::Recreate() noexcept {
// 断言
return Super::Recreate();
}*/

// close this control 关闭控件
void LongUI::UIText::Cleanup() noexcept {
    delete this;
}

// ----------------------------------------------------------------------------
// **** UIButton
// ----------------------------------------------------------------------------

// Render 渲染 
void LongUI::UIButton::Render(RenderType type) const noexcept {
    switch (type)
    {
        D2D1_RECT_F draw_rect;
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景 按钮需要刻画背景 所以不再渲染父类背景
        //Super::Render(LongUI::RenderType::Type_RenderBackground);
        // 本类背景, 更新刻画地区
        this->GetViewRect(draw_rect);
        // 渲染部件
        m_uiElement.Render(draw_rect);
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
        /*if (false) {
            AutoLocker;
            if (m_strControlName == L"1") {
                this->world;
                auto ctrl = this;
                ctrl->parent->RefreshWorld();
                const_cast<UIButton*>(ctrl)->RefreshWorld();
                UIManager << DL_Hint << this->visible_rect << endl;
                int bk = 9;
            }
        }*/
        __fallthrough;
    case LongUI::RenderType::Type_RenderForeground:
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UI按钮: 刷新
void LongUI::UIButton::Update() noexcept {
    // 更新计时器
    m_uiElement.Update();
    return Super::Update();
}

// UIButton 构造函数
LongUI::UIButton::UIButton(pugi::xml_node node) noexcept: Super(node), m_uiElement(node) {
    // 初始化
    Helper::SetBorderColor(node, m_aBorderColor);
    // 初始化代码
    m_uiElement.GetByType<Element_Basic>().Init(node);
    if (m_uiElement.GetByType<Element_Meta>().IsOK()) {
        m_uiElement.SetElementType(Element_Meta);
    }
    else {
        m_uiElement.SetElementType(Element_BrushRect);
    }
    // init
    m_uiElement.GetByType<Element_Basic>().SetNewStatus(Status_Normal);
    // need twices because of aniamtion
    m_uiElement.GetByType<Element_Basic>().SetNewStatus(Status_Normal);
    constexpr int azz = sizeof(m_uiElement);
}


// UIButton::CreateControl 函数
auto LongUI::UIButton::CreateControl(CreateEventType type,pugi::xml_node node) noexcept ->UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIButton>(
            node,
            [=](void* p) noexcept { new(p) UIButton(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// do event 事件处理
bool LongUI::UIButton::DoEvent(const LongUI::EventArgument& arg) noexcept {
    //--------------------------------------------------
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // longui 消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetFocus:
            // 设置焦点: 要求焦点
            return true;
        case LongUI::Event::Event_KillFocus:
            // 释放焦点:
            m_tarStatusClick = LongUI::Status_Normal;
            return true;
        case LongUI::Event::Event_MouseEnter:
            // 鼠标移进: 设置UI元素状态
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Hover);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Hover];
            break;
        case LongUI::Event::Event_MouseLeave:
            // 鼠标移出: 设置UI元素状态
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Normal);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Normal];
            break;
        }
    }
    else {
        switch (arg.msg)
        {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Pushed);
            m_colorBorderNow = m_aBorderColor[LongUI::Status_Pushed];
            break;
        case WM_LBUTTONUP:
            if (m_pWindow->IsReleasedControl(this)) {
                bool rec = m_caller(this, SubEvent::Event_ButtonClicked);
                rec = false;
                // 设置状态
                UIElement_SetNewStatus(m_uiElement, m_tarStatusClick);
                m_colorBorderNow = m_aBorderColor[m_tarStatusClick];
                m_pWindow->ReleaseCapture();
            }
            break;
        }
    }
    return Super::DoEvent(arg);
}

// recreate 重建
auto LongUI::UIButton::Recreate() noexcept ->HRESULT {
    // 重建元素
    m_uiElement.Recreate();
    // 父类处理
    return Super::Recreate();
}

// 关闭控件
void LongUI::UIButton::Cleanup() noexcept {
    delete this;
}


// ----------------------------------------------------------------------------
// **** UIEdit
// ----------------------------------------------------------------------------

// UI基本编辑控件
void LongUI::UIEditBasic::Render(RenderType type) const noexcept {
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        __fallthrough;
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
    case LongUI::RenderType::Type_RenderForeground:
        m_text.Render(0.f, 0.f);
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}

// UI基本编辑框: 刷新
void LongUI::UIEditBasic::Update() noexcept {
    // 改变了大小
    if (this->IsControlSizeChanged()) {
        // 设置大小
        m_text.Resize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 刷新
    m_text.Update();
    return Super::Update();
}

// UI基本编辑控件
bool  LongUI::UIEditBasic::DoEvent(const LongUI::EventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // LongUI 消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            __fallthrough;
        case LongUI::Event::Event_SubEvent:
            return true;
        case LongUI::Event::Event_DragEnter:
            return m_text.OnDragEnter(arg.cf.dataobj, arg.cf.outeffect);
        case LongUI::Event::Event_DragOver:
            return m_text.OnDragOver(pt4self.x, pt4self.y);
        case LongUI::Event::Event_DragLeave:
            return m_text.OnDragLeave();
        case LongUI::Event::Event_Drop:
            return m_text.OnDrop(arg.cf.dataobj, arg.cf.outeffect);
        case LongUI::Event::Event_MouseEnter:
            m_pWindow->now_cursor = m_hCursorI;
            return true;
        case LongUI::Event::Event_MouseLeave:
            m_pWindow->now_cursor = m_pWindow->default_cursor;
            return true;
        case LongUI::Event::Event_SetFocus:
            m_text.OnSetFocus();
            return true;
        case LongUI::Event::Event_KillFocus:
            m_text.OnKillFocus();
            return true;
        }
    }
    // 系统消息
    else {
        switch (arg.msg)
        {
        default:
            return false;
        case WM_KEYDOWN:
            m_text.OnKey(static_cast<uint32_t>(arg.sys.wParam));
            break;
        case WM_CHAR:
            m_text.OnChar(static_cast<char32_t>(arg.sys.wParam));
            break;
        case WM_MOUSEMOVE:
            // 拖拽?
            if (arg.sys.wParam & MK_LBUTTON) {
                m_text.OnLButtonHold(pt4self.x, pt4self.y);
            }
            break;
        case WM_LBUTTONDOWN:
            m_text.OnLButtonDown(pt4self.x, pt4self.y, !!(arg.sys.wParam & MK_SHIFT));
            break;
        case WM_LBUTTONUP:
            m_text.OnLButtonUp(pt4self.x, pt4self.y);
            break;
        }
    }
    return true;
}

// close this control 关闭控件
HRESULT LongUI::UIEditBasic::Recreate() noexcept {
    m_text.Recreate();
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UIEditBasic::Cleanup() noexcept {
    delete this;
}


// UIEditBasic::CreateControl 函数
LongUI::UIControl* LongUI::UIEditBasic::CreateControl(CreateEventType type,pugi::xml_node node) noexcept {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIEditBasic>(
            node,
            [=](void* p) noexcept { new(p) UIEditBasic(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// 构造函数
LongUI::UIEditBasic::UIEditBasic(pugi::xml_node node) noexcept
    :  Super(node), m_text(this, node) {
}


// 调试区域
#ifdef LongUIDebugEvent
// longui 转换

// UI控件: 调试信息
bool LongUI::UIControl::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIControl";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIControl";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIControl>();
    default:
        break;
    }
    return false;
}

// 类型转换断言
void LongUI::UIControl::AssertTypeCasting(const IID& iid) const noexcept {
    LongUI::DebugEventInformation info;
    info.infomation = LongUI::DebugInformation::Information_CanbeCasted;
    info.iid = &iid; info.id = 0;
    assert(this->debug_do_event(info) && "bad casting");
}

// 类型转换判断
auto LongUI::UIControl::IsCanbeCastedTo(const IID& iid) const noexcept ->bool {
    LongUI::DebugEventInformation info;
    info.infomation = LongUI::DebugInformation::Information_CanbeCasted;
    info.iid = &iid; info.id = 0;
    return this->debug_do_event(info);
}

// 获取控件类名
auto LongUI::UIControl::GetControlClassName(bool full) const noexcept ->const wchar_t* {
    LongUI::DebugEventInformation info;
    info.infomation = full ? LongUI::DebugInformation::Information_GetFullClassName
        : LongUI::DebugInformation::Information_GetClassName;
    info.iid = nullptr; info.str = L"";
    this->debug_do_event(info);
    return info.str;
}

// UI标签: 调试信息
bool LongUI::UIText::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIText";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIText";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIText>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI按钮: 调试信息
bool LongUI::UIButton::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIButton";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIButton";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIButton>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI边缘控件: 调试信息
bool LongUI::UIMarginalable::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIMarginalable";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIMarginalable";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIMarginalable>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI滚动条: 调试信息
bool LongUI::UIScrollBar::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIScrollBar";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIScrollBar";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIScrollBar>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI滚动条-A型: 调试信息
bool LongUI::UIScrollBarA::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIScrollBarA";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIScrollBarA";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIScrollBarA>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI滚动条-A型: 调试信息
bool LongUI::UIScrollBarB::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIScrollBarB";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIScrollBarB";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIScrollBarB>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI容器: 调试信息
bool LongUI::UIContainer::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIContainer";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIContainer";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIContainer>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI 基本编辑控件: 调试信息
bool LongUI::UIEditBasic::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIEditBasic";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIEditBasic";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIEditBasic>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI水平布局: 调试信息
bool LongUI::UIHorizontalLayout::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIHorizontalLayout";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIHorizontalLayout";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIHorizontalLayout>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI垂直布局: 调试信息
bool LongUI::UIVerticalLayout::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIVerticalLayout";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIVerticalLayout";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIVerticalLayout>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI窗口: 调试信息
bool LongUI::UIWindow::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIWindow";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIWindow";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIWindow>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}


// --------------------------- 单独 ---------------------
// UI滑动条: 调试信息
bool LongUI::UISlider::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UISlider";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UISlider";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UISlider>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI列表: 调试信息
bool LongUI::UIList::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIList";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIList";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIList>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI列表元素: 调试信息
bool LongUI::UIListLine::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIListLine";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIListLine";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIListLine>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// UI列表头: 调试信息
bool LongUI::UIListHeader::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIListHeader";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIListHeader";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIListHeader>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}
#endif
                   


// CUIRubyCharacter 构造函数
LongUI::CUIRubyCharacter::
CUIRubyCharacter(const CtorContext& ctx) noexcept : Super(CUIInlineObject::Type_Ruby) {
    UNREFERENCED_PARAMETER(ctx);
}

// CUIRubyCharacter 析构函数
LongUI::CUIRubyCharacter::~CUIRubyCharacter() noexcept {
    ::SafeRelease(m_pBaseLayout);
    ::SafeRelease(m_pRubyLayout);
}

// CUIRubyCharacter 刻画
auto LongUI::CUIRubyCharacter::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
    ) noexcept ->HRESULT {
    UNREFERENCED_PARAMETER(clientDrawingContext);
    UNREFERENCED_PARAMETER(renderer);
    UNREFERENCED_PARAMETER(originX);
    UNREFERENCED_PARAMETER(originY);
    UNREFERENCED_PARAMETER(isSideways);
    UNREFERENCED_PARAMETER(isRightToLeft);
    UNREFERENCED_PARAMETER(clientDrawingEffect);
    return E_NOTIMPL;
}

// 获取 Metrics
auto LongUI::CUIRubyCharacter::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS* metrics) noexcept ->HRESULT {
    DWRITE_INLINE_OBJECT_METRICS inlineMetrics = { 0 };
    inlineMetrics;

    *metrics = inlineMetrics;
    return S_OK;
}


// 获取 Overhang Metrics
auto LongUI::CUIRubyCharacter::GetOverhangMetrics(
     DWRITE_OVERHANG_METRICS* overhangs) noexcept ->HRESULT {
    overhangs->left = 0;
    overhangs->top = 0;
    overhangs->right = 0;
    overhangs->bottom = 0;
    return S_OK;
}


auto LongUI::CUIRubyCharacter::GetBreakConditions(
    DWRITE_BREAK_CONDITION* breakConditionBefore,
    DWRITE_BREAK_CONDITION* breakConditionAfter
    ) noexcept ->HRESULT {
    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
    return S_OK;
}

                   

// Memory leak detector
#if defined(_DEBUG) && defined(_MSC_VER)
ID3D11Debug*    g_pd3dDebug_longui = nullptr;
struct MemoryLeakDetector {
    // ctor
    MemoryLeakDetector() {
        ::_CrtMemCheckpoint(memstate + 0);
        constexpr int sa = sizeof(_CrtMemState);
    }
    // dtor
    ~MemoryLeakDetector() {
        ::_CrtMemCheckpoint(memstate + 1);
        if (::_CrtMemDifference(memstate + 2, memstate + 0, memstate + 1)) {
            ::_CrtDumpMemoryLeaks();
            assert(!"OOps! Memory leak detected");
        }
        if (g_pd3dDebug_longui) {
            auto count = g_pd3dDebug_longui->Release();
            if (count) {
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL)\r\n\r\n");
                g_pd3dDebug_longui->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS(1 | 2 | 4));
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL)\r\n\r\n");
                g_pd3dDebug_longui->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS(4));
                ::OutputDebugStringW(L"\r\nLongUI Memory Leak Debug: End. If you saw this message, check 'KnownIssues.md' please \r\n\r\n");
            }
            g_pd3dDebug_longui = nullptr;
        }
    }
    // mem state
    _CrtMemState memstate[3];
} g_detector;
#endif




static_assert(sizeof(std::atomic_bool) == sizeof(char), "really bad");
#define InitStaticVar(v)  decltype(v) v = nullptr
// 初始化静态变量

InitStaticVar(LongUI::UIRichEdit::IID_ITextServices2);
InitStaticVar(LongUI::UIRichEdit::CreateTextServices);
InitStaticVar(LongUI::UIRichEdit::ShutdownTextServices);
InitStaticVar(LongUI::Dll::D3D11CreateDevice);
InitStaticVar(LongUI::Dll::D2D1CreateFactory);
InitStaticVar(LongUI::Dll::DCompositionCreateDevice);
InitStaticVar(LongUI::Dll::DWriteCreateFactory);
InitStaticVar(LongUI::Dll::CreateDXGIFactory1);


/*// 复制构造
LongUI::EventArgument::EventArgument(const EventArgument& arg) noexcept {
    this->event = arg.event;
    this->sender = arg.sender;
    this->sys.wParam = arg.sys.wParam;
    this->sys.lParam = arg.sys.lParam;
    this->pt.x = arg.pt.x;
    this->pt.y = arg.pt.y;
    this->lr = arg.lr;
}*/


#define LoadFunction(a, b, c) { if(c) a = reinterpret_cast<decltype(a)>(::GetProcAddress(c, #b)); }

// 初始化库
class InitializeLibrary {
    typedef enum PROCESS_DPI_AWARENESS {
        PROCESS_DPI_UNAWARE = 0,
        PROCESS_SYSTEM_DPI_AWARE = 1,
        PROCESS_PER_MONITOR_DPI_AWARE = 2
    } PROCESS_DPI_AWARENESS;
    // SetProcessDpiAwareness
    static HRESULT STDAPICALLTYPE SetProcessDpiAwarenessF(PROCESS_DPI_AWARENESS);
public:
    //
    InitializeLibrary() {
        // < Win8 ?
        if (!LongUI::IsWindows8OrGreater()) {
            ::MessageBoxA(nullptr, "Windows8 at least!", "Unsupported System", MB_ICONERROR);
            ::ExitProcess(1);
            return;
        }
        // >= Win8.1 ?
        if (LongUI::IsWindows8Point1OrGreater()) {
            m_hDllShcore = ::LoadLibraryW(L"Shcore.dll");
            assert(m_hDllShcore);
            if (m_hDllShcore) {
                auto setProcessDpiAwareness =
                    reinterpret_cast<decltype(&InitializeLibrary::SetProcessDpiAwarenessF)>(
                        ::GetProcAddress(m_hDllShcore, "SetProcessDpiAwareness")
                        );
                assert(setProcessDpiAwareness);
                if (setProcessDpiAwareness) {
                    setProcessDpiAwareness(InitializeLibrary::PROCESS_PER_MONITOR_DPI_AWARE);
                }
            }
        }
        LoadFunction(LongUI::UIRichEdit::IID_ITextServices2, IID_ITextServices2, m_hDllMsftedit);
        LoadFunction(LongUI::UIRichEdit::CreateTextServices, CreateTextServices, m_hDllMsftedit);
        LoadFunction(LongUI::UIRichEdit::ShutdownTextServices, ShutdownTextServices, m_hDllMsftedit);
        LoadFunction(LongUI::Dll::DCompositionCreateDevice, DCompositionCreateDevice, m_hDlldcomp);
        LoadFunction(LongUI::Dll::D2D1CreateFactory, D2D1CreateFactory, m_hDlld2d1);
        LoadFunction(LongUI::Dll::D3D11CreateDevice, D3D11CreateDevice, m_hDlld3d11);
        LoadFunction(LongUI::Dll::DWriteCreateFactory, DWriteCreateFactory, m_hDlldwrite);
        LoadFunction(LongUI::Dll::CreateDXGIFactory1, CreateDXGIFactory1, m_hDlldxgi);
    };
    //
    ~InitializeLibrary() {
        if (m_hDllMsftedit) {
            ::FreeLibrary(m_hDllMsftedit);
            m_hDllMsftedit = nullptr;
        }
        if (m_hDlldcomp) {
            ::FreeLibrary(m_hDlldcomp);
            m_hDlldcomp = nullptr;
        }
        if (m_hDlld2d1) {
            ::FreeLibrary(m_hDlld2d1);
            m_hDlld2d1 = nullptr;
        }
        if (m_hDlld3d11) {
            ::FreeLibrary(m_hDlld3d11);
            m_hDlld3d11 = nullptr;
        }
        if (m_hDlldwrite) {
            ::FreeLibrary(m_hDlldwrite);
            m_hDlldwrite = nullptr;
        }
        if (m_hDlldxgi) {
            ::FreeLibrary(m_hDlldxgi);
            m_hDlldxgi = nullptr;
        }
        if (m_hDllShcore) {
            ::FreeLibrary(m_hDllShcore);
            m_hDllShcore = nullptr;
        }
    }
private:
    // Msftedit
    HMODULE     m_hDllMsftedit = ::LoadLibraryW(L"Msftedit.dll");
    // dcomp
    HMODULE     m_hDlldcomp = ::LoadLibraryW(L"dcomp.dll");
    // d2d1
    HMODULE     m_hDlld2d1 = ::LoadLibraryW(L"d2d1.dll");
    // d3d11
    HMODULE     m_hDlld3d11 = ::LoadLibraryW(L"d3d11.dll");
    // dwrite
    HMODULE     m_hDlldwrite = ::LoadLibraryW(L"dwrite.dll");
    // dxgi
    HMODULE     m_hDlldxgi = ::LoadLibraryW(L"dxgi.dll");
    // Shcore
    HMODULE     m_hDllShcore = nullptr;
} instance;


// 初始化静态变量
LongUI::CUIManager          LongUI::CUIManager::s_instance;

// load libraries
#if defined(_MSC_VER)
#pragma comment(lib, "winmm")
#pragma comment(lib, "dxguid")
#endif



                   



// Render 渲染 
void LongUI::UICheckBox::Render(RenderType type) const noexcept  {
    /*D2D1_RECT_F draw_rect = this->GetDrawRect();;
    draw_rect.left += 1.f;
    // 计算渲染区
    draw_rect.top = (draw_rect.bottom + draw_rect.top - m_szCheckBox.height) * 0.5f;
    draw_rect.bottom = draw_rect.top + m_szCheckBox.height;
    draw_rect.right = draw_rect.left + m_szCheckBox.width;
    // 渲染框
    UIManager_RenderTarget->DrawRectangle(
        draw_rect, m_pBrush, 1.5f
        );
    // 渲染箭头
    switch (this->state)
    {
    case CheckBoxState::State_Checked:
        D2D1_MATRIX_3X2_F matrix;
        UIManager_RenderTarget->GetTransform(&matrix);
        UIManager_RenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(draw_rect.left, draw_rect.bottom) * matrix);
        UIManager_RenderTarget->FillGeometry(m_pCheckedGeometry, m_pBrush);
        UIManager_RenderTarget->SetTransform(&matrix);
        break;
    case CheckBoxState::State_Indeterminate:
        // 收缩范围
        draw_rect.left += m_szCheckBox.width * 0.2f;
        draw_rect.right -= m_szCheckBox.width * 0.2f;
        draw_rect.top += m_szCheckBox.height * 0.2f;
        draw_rect.bottom -= m_szCheckBox.height * 0.2f;
        // 渲染框
        UIManager_RenderTarget->FillRectangle(draw_rect, m_pBrush);
        break;
    }
    // 调节文本范围 +
    //this->show_zone.left += m_szCheckBox.width;
    // 刻画文本
    Super::Render(RenderType::Type_Render);
    // 调节文本范围 -
    //this->show_zone.left -= m_szCheckBox.width;
    return S_OK;*/
    return Super::Render(type);
}

// UI检查框: 刷新
void LongUI::UICheckBox::Update() noexcept {
    return Super::Update();
}

// UICheckBox 构造函数
LongUI::UICheckBox::UICheckBox(pugi::xml_node node) noexcept: Super(node) {
    // √ symbol
    auto format = UIManager.GetTextFormat(LongUIDefaultTextFormatIndex);
    char32_t chars = U'√';
    LongUI::DX::CreateTextPathGeometry(
        &chars, 1,
        format,
        UIManager_D2DFactory,
        nullptr,
        &m_pCheckedGeometry
        );
    ::SafeRelease(format);
}

// UICheckBox 析构函数
LongUI::UICheckBox::~UICheckBox() noexcept {
    ::SafeRelease(m_pCheckedGeometry);
    ::SafeRelease(m_pBrush);
}


// UICheckBox::CreateControl 函数
auto LongUI::UICheckBox::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UICheckBox>(
            node,
            [=](void* p) noexcept { new(p) UICheckBox(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UICheckBox::DoEvent(const LongUI::EventArgument& arg) noexcept {
   // D2D1_COLOR_F* color = nullptr;
    if (arg.sender) {
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl:
            if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
                arg.ctrl = this;
            }
            __fallthrough;*/
        case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        case LongUI::Event::Event_MouseEnter:
            m_pWindow->now_cursor = m_hCursorHand;
            break;
        case LongUI::Event::Event_MouseLeave:
            m_pWindow->now_cursor = m_pWindow->default_cursor;
            break;
        }
    }
    else {
        switch (arg.msg)
        {
        case WM_LBUTTONUP:
            // 有效
            if (arg.pt.x < this->view_size.width && arg.pt.y) {
                // 检查flag
                if (this->IsCanbeIndeterminate()) {
                    if (this->state == CheckBoxState::State_UnChecked) {
                        force_cast(this->state) = CheckBoxState::State_Checked;
                    }
                    else if (this->state == CheckBoxState::State_Checked) {
                        force_cast(this->state) = CheckBoxState::State_Indeterminate;
                    }
                    else {
                        force_cast(this->state) = CheckBoxState::State_UnChecked;
                    }
                }
                else {
                    force_cast(this->state) = static_cast<decltype(this->state)>
                        (!static_cast<uint32_t>(this->state));
                }
                m_pWindow->Invalidate(this);
            }
            break;
        }
    }
    return Super::DoEvent(arg);
}

// recreate 重建
HRESULT LongUI::UICheckBox::Recreate() noexcept {
    // 有效
    ::SafeRelease(m_pBrush);
    m_pBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    // 父类处理
    return Super::Recreate();
}

// 关闭控件
void LongUI::UICheckBox::Cleanup() noexcept {
    delete this;
}
                   

// ----------------------------------------------------------------------------
// -------------------------------- UIList ------------------------------------
// ----------------------------------------------------------------------------
// UI列表控件: 构造函数
LongUI::UIList::UIList(pugi::xml_node node) noexcept :Super(node) {
    // TODO: 删除预测
    if (node) {
        const char* str = nullptr;
        // 行高度
        if ((str = node.attribute("lineheight").value())) {
            m_fLineHeight = LongUI::AtoF(str);
        }
        // 行模板
        if ((str = node.attribute("linetemplate").value())) {
            // 检查长度
            register auto len = Helper::MakeCC(str);
            // 有效
            if (len) {
                m_bufLineTemplate.NewSize(len);
                Helper::MakeCC(str, m_bufLineTemplate.GetData());
            }
        }
    }
}

// UI列表控件: 析构函数
LongUI::UIList::~UIList() noexcept {

}

// 插入后
inline void LongUI::UIList::after_insert(UIControl* child) noexcept {
    UNREFERENCED_PARAMETER(child);

    assert(!(child->flags & Flag_Floating) 
        && "child of UIList cannot keep flag 'Flag_Floating'");
    assert(!(child->flags & Flag_HeightFixed) 
        && "child of UIList cannot keep flag 'Flag_WidthFixed'");
}

// 插入一个行模板
void LongUI::UIList::InsertInlineTemplate(Iterator itr) noexcept {
    auto ctrl = static_cast<UIListLine*>(UIListLine::CreateControl(
        Type_CreateControl, pugi::xml_node()));
    if (ctrl) {
        // 添加子控件
        for (const auto& data : m_bufLineTemplate) {
            ctrl->Insert(ctrl->end(), UIManager.CreateControl(data.id, data.func));
        }
        // 插入
        this->Insert(itr, ctrl);
    }
}

// 修改元素权重
void LongUI::UIList::ChangeElementWights(float weights[]) noexcept {
    for (auto cline : (*this)) {
        auto line = longui_cast<UIListLine*>(cline);
        auto index = 0u;
        for (auto ele : (*line)) {
            if (weights[index] >= 0.f) {
                force_cast(ele->weight) = weights[index];
            }
            ++index;
        }
    }
}

// 设置元素数量
void LongUI::UIList::SetElementCount(uint32_t length) noexcept {
    auto old = m_bufLineTemplate.GetCount();
    m_bufLineTemplate.NewSize(length);
    // 变长了
    if (old < m_bufLineTemplate.GetCount()) {
        for (auto i = old; i < m_bufLineTemplate.GetCount(); ++i) {
            m_bufLineTemplate[i].id = 0;
            m_bufLineTemplate[i].func = UIText::CreateControl;
        }
    }
}


// Render 渲染 
void LongUI::UIList::Render(RenderType t) const noexcept {
    return Super::Render(t);
}

// 更新子控件布局
void LongUI::UIList::Update() noexcept {
    // 前向刷新
    this->BeforeUpdate();
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    if (this->IsControlSizeChanged()) {
        // 第二次
        float index = 0.f;
        for (auto ctrl : (*this)) {
            // 宽度无效?
            if (ctrl->view_size.width == 0.f) {
                ctrl->SetWidth(this->GetViewWidthZoomed());
            }
            // 设置控件高度
            ctrl->SetHeight(m_fLineHeight);
            // 不管如何, 修改!
            ctrl->SetControlSizeChanged();
            ctrl->SetLeft(0.f);
            ctrl->SetTop(m_fLineHeight * index);
            ++index;
        }
        // 设置
        auto tmp = this->get_referent_control();
        if (tmp) {
            m_2fContentSize.width = tmp->GetWidth();
        }
        m_2fContentSize.height = m_fLineHeight * this->GetCount();
        // 已经处理
        this->ControlSizeChangeHandled();
    }
    // 父类刷新
    Super::Update();
}

// 清理UI列表控件
void LongUI::UIList::Cleanup() noexcept {
    delete this;
}


#ifdef LONGUI_UILIST_VECTOR
// 插入后
void LongUI::UIList::AfterInsert(UIControl* child) noexcept {
    Super::AfterInsert(child);
    this->after_insert(child);
    try {
        auto itr = m_controls.begin();
        if (!child->next) {
            auto itr = m_controls.end();
        }
        else if (child->prev) {
            itr = std::find(m_controls.begin(), m_controls.end(), child->prev);
        }
        m_controls.insert(
            std::find(m_controls.begin(), m_controls.end(), child->prev),
            child
            );
    }
    catch (...) {

    }
}

// 移除后
void LongUI::UIList::AfterRemove(UIControl* child) noexcept  {
    Super::AfterRemove(child);
    m_controls.erase(std::find(m_controls.begin(), m_controls.end(), child));
}

// 根据索引获取控件
auto LongUI::UIList::GetAt(uint32_t index) const noexcept -> UIControl* {
    assert(index < this->GetCount());
    if (index < this->GetCount()) {
        return static_cast<UIControl*>(m_controls.at(index));
    }
    return nullptr;
}
#else
// 插入后
void LongUI::UIList::AfterInsert(UIControl* child) noexcept  {
    Super::AfterInsert(child);
    this->after_insert(child);
}
#endif

// UI列表控件: 创建控件
auto LongUI::UIList::CreateControl(CreateEventType type, pugi::xml_node node) 
noexcept -> UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIList>(
            node,
            [=](void* p) noexcept { new(p) UIList(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// ---------------------------- UIListLine! --------------------------------
// ----------------------------------------------------------------------------

// UI列表元素控件: 构造函数
LongUI::UIListLine::UIListLine(pugi::xml_node node) noexcept:Super(node){
    // listline 特性: 宽度必须固定
    auto flag = this->flags | Flag_WidthFixed;
    if (node) {

    }
    force_cast(this->flags) = flag;
}

// 刷新UI列表元素控件
void LongUI::UIListLine::Update() noexcept {
    return Super::Update();
}

// 清理UI列表元素控件
void LongUI::UIListLine::Cleanup() noexcept {
    delete this;
}

// UI列表元素控件: 创建控件
auto LongUI::UIListLine::CreateControl(CreateEventType type, pugi::xml_node node) 
noexcept -> UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIListLine>(
            node,
            [=](void* p) noexcept { new(p) UIListLine(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}




// ----------------------------------------------------------------------------
// ----------------------------- UIListHeader ---------------------------------
// ----------------------------------------------------------------------------

// UI列表头控件: 构造函数
LongUI::UIListHeader::UIListHeader(pugi::xml_node node) noexcept: Super(node) {
    if (node) {

    }
}

// UI列表头: 事件处理
bool LongUI::UIListHeader::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 设置列表头
            longui_cast<UIList*>(this->parent)->SetHeader(this);
            return true;
        default:
            break;
        }
    }
    return false;
}

// 清理UI列表头控件
void LongUI::UIListHeader::Cleanup() noexcept {
    delete this;
}

// 创建UI列表头
auto LongUI::UIListHeader::CreateControl(CreateEventType type, pugi::xml_node node) 
noexcept -> UIControl* {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIListHeader>(
            node,
            [=](void* p) noexcept { new(p) UIListHeader(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// --------------------------------- Menu -------------------------------------
// ----------------------------------------------------------------------------

// 摧毁弹出菜单
void LongUI::CUIMenu::Destroy() noexcept {
    if (m_hMenu) {
        ::DestroyMenu(m_hMenu);
        m_hMenu = nullptr;
    }
}

// 直接创建才菜单
bool LongUI::CUIMenu::Create() noexcept {
    assert(!m_hMenu && "cannot create again!");
    m_hMenu = ::CreatePopupMenu();
    return !!m_hMenu;
}

// 使用XML字符串创建菜单
bool LongUI::CUIMenu::Create(const char * xml) noexcept {
    pugi::xml_document document;
    auto re = document.load_string(xml);
    // 错误
    if (re.status) {
        assert(!"failed to load string");
        ::MessageBoxA(
            nullptr, 
            re.description(), 
            "<LongUI::CUIMenu::Create>: Failed to Parse/Load XML",
            MB_ICONERROR
            );
        return false;
    }
    // 创建节点
    return this->Create(document.first_child());
}

// 使用XML节点创建菜单
bool LongUI::CUIMenu::Create(pugi::xml_node node) noexcept {
    UNREFERENCED_PARAMETER(node);
    assert(!m_hMenu && "cannot create again!");
    m_hMenu = ::CreatePopupMenu();
    return !!m_hMenu;
}

// 添加物品
bool LongUI::CUIMenu::AppendItem(const ItemProperties& prop) noexcept {
    UNREFERENCED_PARAMETER(prop);
    return false;
}

// 显示菜单
void LongUI::CUIMenu::Show(HWND parent, POINT* OPTIONAL pos) noexcept {
    // 获取坐标
    POINT pt = { 0,0 };  if (pos) pt = *pos; else ::GetCursorPos(&pt);
    // 置前
    ::SetForegroundWindow(parent);
    // 跟踪菜单项的选择
    auto index = ::TrackPopupMenu(m_hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, parent, nullptr);
    if (m_pItemProc) {
        m_pItemProc(index);
    }
}

                   

// node->Attribute\((.+?)\)
// node.attribute($1).value()

#define LONGUI_D3D_DEBUG
//#define LONGUI_RENDER_IN_STD_THREAD

// CUIManager 初始化
auto LongUI::CUIManager::Initialize(IUIConfigure* config) noexcept->HRESULT {
    m_szLocaleName[0] = L'\0';
    ::memset(m_apWindows, 0, sizeof(m_apWindows));
    // 开始计时
    m_uiTimer.Start();
    // 检查
    if (!config) {
#ifdef LONGUI_WITH_DEFAULT_CONFIG
        config = &m_config;
#else
        return E_INVALIDARG;
#endif
    }
    // 获取信息
    force_cast(this->configure) = config;
    // 获取flag
    this->flag = this->configure->GetConfigureFlag();
    // 获取资源加载器
    config->CreateInterface(LongUI_IID_PV_ARGS(m_pResourceLoader));
    // 获取脚本
    config->CreateInterface(LongUI_IID_PV_ARGS(force_cast(this->script)));
    // 本地字符集名称
    config->GetLocaleName(m_szLocaleName);
    // 初始化其他
    ZeroMemory(m_apTextRenderer, sizeof(m_apTextRenderer));
    ZeroMemory(m_apSystemBrushes, sizeof(m_apSystemBrushes));
    // 获取实例句柄
    auto hInstance = ::GetModuleHandleW(nullptr);
    // 注册窗口类 | CS_DBLCLKS
    WNDCLASSEXW wcex = { 0 };
    {
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = CUIManager::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(void*);
        wcex.hInstance = hInstance;
        wcex.hCursor = nullptr;
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = LongUI::WindowClassName;
        wcex.hIcon = nullptr;// ::LoadIconW(hInstance, MAKEINTRESOURCEW(101));
        // 注册普通窗口
        ::RegisterClassExW(&wcex);
    }
   /*{
        wcex.cbWndExtra = 0;
        wcex.lpszClassName = L"LongUIManager";
        wcex.hIcon = nullptr;
        wcex.lpfnWndProc = CUIManager::InvisibleWndProc;
        // 注册不可见窗口
        ::RegisterClassExW(&wcex);
        // 创建窗口
        m_hInvisibleHosted = ::CreateWindowExW(
            0,
            L"LongUIManager", L"LongUI UIManager Invisible Hosted Window",
            0,
            0, 0, 0, 0,
            nullptr, nullptr,
            ::GetModuleHandleW(nullptr),
            this
            );
        ::ShowWindow(m_hInvisibleHosted, SW_SHOW);
    }*/
    HRESULT hr = S_OK;
    // 位图缓存
    if (SUCCEEDED(hr)) {
        m_pBitmap0Buffer = reinterpret_cast<uint8_t*>(LongUI::NormalAlloc(
            sizeof(RGBQUAD) * LongUIDefaultBitmapSize * LongUIDefaultBitmapSize)
            );
        // 内存不足
        if (!m_pBitmap0Buffer) {
            hr = E_OUTOFMEMORY;
        }
    }
    // 加载控件模板
    if (SUCCEEDED(hr)) {
        m_cCountCtrlTemplate = 1;
        hr = this->load_control_template_string(this->configure->GetTemplateString());
    }
    // 资源数据缓存
    if (SUCCEEDED(hr)) {
        // 获取缓存数据
        auto get_buffer_length = [this]() {
            size_t buffer_length =
                sizeof(void*) * m_cCountBmp +
                sizeof(void*) * m_cCountBrs +
                sizeof(void*) * m_cCountTf +
                sizeof(pugi::xml_node) * m_cCountCtrlTemplate +
                (sizeof(HICON) + sizeof(LongUI::Meta)) * m_cCountMt;
            return buffer_length;
        };
        m_cCountBmp = m_cCountBrs = m_cCountTf = m_cCountMt = 1;
        // 检查资源缓存
        if (!m_pResourceBuffer) {
            // 查询资源数量
            if (m_pResourceLoader) {
                m_cCountBmp += static_cast<decltype(m_cCountBmp)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_Bitmap));
                m_cCountBrs += static_cast<decltype(m_cCountBrs)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_Brush));
                m_cCountTf += static_cast<decltype(m_cCountTf)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_TextFormat));
                m_cCountMt += static_cast<decltype(m_cCountMt)>(m_pResourceLoader->GetResourceCount(IUIResourceLoader::Type_Meta));
            }
            // 申请内存
            m_pResourceBuffer = LongUI::NormalAlloc(get_buffer_length());
        }
        // 修改资源
        if (m_pResourceBuffer) {
            ::memset(m_pResourceBuffer, 0, get_buffer_length());
            m_ppBitmaps = reinterpret_cast<decltype(m_ppBitmaps)>(m_pResourceBuffer);
            m_ppBrushes = reinterpret_cast<decltype(m_ppBrushes)>(m_ppBitmaps + m_cCountBmp);
            m_ppTextFormats = reinterpret_cast<decltype(m_ppTextFormats)>(m_ppBrushes + m_cCountBrs);
            m_pMetasBuffer = reinterpret_cast<decltype(m_pMetasBuffer)>(m_ppTextFormats + m_cCountTf);
            m_phMetaIcon = reinterpret_cast<decltype(m_phMetaIcon)>(m_pMetasBuffer + m_cCountMt);
            m_pTemplateNodes = reinterpret_cast<decltype(m_pTemplateNodes)>(m_phMetaIcon + m_cCountMt);
            // 初始化
            for (auto itr = m_pTemplateNodes; itr < m_pTemplateNodes + m_cCountCtrlTemplate; ++itr) {
                LongUI::CreateObject(*itr);
            }
        }
        // 内存不足
        else {
            hr = E_OUTOFMEMORY;
        }
    }
    // 设置控件模板
    if (SUCCEEDED(hr)) {
        hr = this->set_control_template_string();
    }
    // 创建D2D工厂
    if (SUCCEEDED(hr)) {
        D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
#ifdef _DEBUG
        options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
        hr = LongUI::Dll::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            IID_ID2D1Factory1,
            &options,
            reinterpret_cast<void**>(&m_pd2dFactory)
            );
    }
    // 创建 DirectWrite 工厂.
    if (SUCCEEDED(hr)) {
        hr = LongUI::Dll::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            LongUI_IID_PV_ARGS_Ex(m_pDWriteFactory)
            );
    }
    // 创建帮助器
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_DragDropHelper,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pDropTargetHelper)
            );
    }
    // 创建字体集
    if (SUCCEEDED(hr)) {
       // m_pFontCollection = config->CreateFontCollection();
        // 失败获取系统字体集
        if (!m_pFontCollection) {
            hr = m_pDWriteFactory->GetSystemFontCollection(&m_pFontCollection);
        }
    }
    // 注册渲染器
    if (SUCCEEDED(hr)) {
        // 普通渲染器
        if (this->RegisterTextRenderer(&m_normalTRenderer, "normal") != Type_NormalTextRenderer) {
            assert(!"Type_NormalTextRenderer");
            hr = E_FAIL;
        }
    }
    // 添加控件
    if (SUCCEEDED(hr)) {
        // 添加默认控件创建函数
        this->RegisterControl(CreateNullControl, L"Null");
        this->RegisterControl(UIText::CreateControl, L"Text");
        this->RegisterControl(UIList::CreateControl, L"List");
        this->RegisterControl(UISlider::CreateControl, L"Slider");
        this->RegisterControl(UIButton::CreateControl, L"Button");
        this->RegisterControl(UIListLine::CreateControl, L"ListLine");
        this->RegisterControl(UICheckBox::CreateControl, L"CheckBox");
        this->RegisterControl(UIRichEdit::CreateControl, L"RichEdit");
        this->RegisterControl(UIEditBasic::CreateControl, L"Edit");
        this->RegisterControl(UIListHeader::CreateControl, L"ListHeader");
        this->RegisterControl(UIScrollBarA::CreateControl, L"ScrollBarA");
        this->RegisterControl(UIScrollBarB::CreateControl, L"ScrollBarB");
        this->RegisterControl(UIVerticalLayout::CreateControl, L"VerticalLayout");
        this->RegisterControl(UIHorizontalLayout::CreateControl, L"HorizontalLayout");
        // 添加自定义控件
        config->AddCustomControl();
    }
    // 创建资源
    if (SUCCEEDED(hr)) {
        hr = this->RecreateResources();
    }
    // 初始化事件
    if (SUCCEEDED(hr)) {
        this->do_creating_event(LongUI::CreateEventType::Type_Initialize);
    }
    // 检查错误
    else {
        this->ShowError(hr);
    }
    return hr;
}


// CUIManager  反初始化
void LongUI::CUIManager::UnInitialize() noexcept {
    this->do_creating_event(LongUI::CreateEventType::Type_Uninitialize);
    // 释放文本渲染器
    for (auto& renderer : m_apTextRenderer) {
        ::SafeRelease(renderer);
    }
    // 释放公共设备无关资源
    {
        // 释放文本格式
        for (auto itr = m_ppTextFormats; itr != m_ppTextFormats + m_cCountTf; ++itr) {
            ::SafeRelease(*itr);
        }
        // 摧毁META图标
        for (auto itr = m_phMetaIcon; itr != m_phMetaIcon + m_cCountMt; ++itr) {
            if (*itr) {
                ::DestroyIcon(*itr);
                *itr = nullptr;
            }
        }
        // 控件模板
        for (auto itr = m_pTemplateNodes; itr != m_pTemplateNodes + m_cCountCtrlTemplate; ++itr) {
            if (*itr) {
                LongUI::DestoryObject(*itr);
            }
        }
    }
    // 释放资源
    ::SafeRelease(m_pFontCollection);
    ::SafeRelease(m_pDWriteFactory);
    ::SafeRelease(m_pDropTargetHelper);
    ::SafeRelease(m_pd2dFactory);
    // 释放脚本
    ::SafeRelease(force_cast(script));
    // 释放读取器
    ::SafeRelease(m_pResourceLoader);
    // 释放配置
    ::SafeRelease(force_cast(this->configure));
    // 释放设备相关资源
    this->discard_resources();
    // 释放内存
    if (m_pBitmap0Buffer) {
        LongUI::NormalFree(m_pBitmap0Buffer);
        m_pBitmap0Buffer = nullptr;
    }
    // 释放资源缓存
    if (m_pResourceBuffer) {
        LongUI::NormalFree(m_pResourceBuffer);
        m_pResourceBuffer = nullptr;
    }
    m_cCountMt = m_cCountTf = m_cCountBmp = m_cCountBrs = 0;
}

// 创建事件
void LongUI::CUIManager::do_creating_event(CreateEventType type) noexcept {
    assert(type != LongUI::Type_CreateControl);
    try {
        for (const auto& pair : m_mapString2CreateFunction) {
            reinterpret_cast<CreateControlFunction>(pair.second)(type, LongUINullXMLNode);
        }
    }
    catch (...) {
        assert(!"some error");
    }
}


// CUIManager 创建控件树
// 默认消耗 64kb+, 导致栈(默认1~2M)溢出几率较低
void LongUI::CUIManager::make_control_tree(LongUI::UIWindow* window, pugi::xml_node node) noexcept {
    // 断言
    assert(window && node && "bad argument");
    // 添加窗口
    //add_control(window, node);
    // 队列 -- 顺序遍历树
    LongUI::EzContainer::FixedCirQueue<pugi::xml_node, LongUIMaxControlInited> xml_queue;
    LongUI::EzContainer::FixedCirQueue<UIContainer*, LongUIMaxControlInited> parents_queue;
    // 
    UIControl* now_control = nullptr;
    UIContainer* parent_node = window;
    // 唯一名称
    std::pair<CUIString, void*> control_name;
    // 遍历算法: 1.压入所有子节点 2.依次弹出 3.重复1
    while (true) {
        // 压入/入队 所有子节点
        node = node.first_child();
        while (node) {
            xml_queue.push(node);
            parents_queue.push(parent_node);
            node = node.next_sibling();
        }
    recheck:
        // 为空则退出
        if (xml_queue.empty()) break;
        // 弹出/出队 第一个节点
        node = *xml_queue.front;  xml_queue.pop();
        parent_node = *parents_queue.front; parents_queue.pop();
        // 根据名称创建控件
        if (!(now_control = this->CreateControl(node, nullptr))) {
            parent_node = nullptr;
#ifdef _DEBUG
            const char* node_name = node.name();
            UIManager << DL_Error << L" Control Class Not Found: " << node_name << LongUI::endl;
#endif
            continue;
        }
        // 添加到表
        if (Helper::MakeString(node.attribute("name").value(), control_name.first)) {
            control_name.second = now_control;
            window->AddControl(control_name);
        }
        // 添加子节点
        parent_node->Insert(parent_node->end(), now_control);
        // 设置节点为下次父节点
        parent_node = static_cast<decltype(parent_node)>(now_control);
        // 检查本控件是否需要XML子节点信息
        if (now_control->flags & Flag_ControlNeedFullXMLNode) {
            goto recheck;
        }
    }
}

// 获取创建控件函数指针
auto LongUI::CUIManager::GetCreateFunc(const char* class_name) noexcept -> CreateControlFunction {
    // 缓冲区
    wchar_t buffer[LongUIStringBufferLength];
    auto* __restrict itra = class_name; auto* __restrict itrb = buffer;
    // 类名一定是英文的
    for (; *itra; ++itra, ++itrb) {
        assert(*itra >= 0 && "bad name, class name must be english char");
        *itrb = *itra;
    }
    // null 结尾字符串
    *itrb = L'\0';
    // 获取
    return this->GetCreateFunc(buffer, static_cast<uint32_t>(itra - class_name));
}

// 获取创建控件函数指针
auto LongUI::CUIManager::GetCreateFunc(const CUIString& name) noexcept -> CreateControlFunction {
    // 查找
    try {
        const auto itr = m_mapString2CreateFunction.find(name);
        if (itr != m_mapString2CreateFunction.end()) {
            return reinterpret_cast<CreateControlFunction>(itr->second);
        }
    }
    catch (...)  {

    }
    return nullptr;
}

/*
if (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
    // 两种方式退出 
    if (msg.message == WM_QUIT) {
        m_exitFlag = true;
        break;
    }
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
}
*/

#ifdef LONGUI_RENDER_IN_STD_THREAD
#include <thread>
#else
#include <process.h>
#endif
// 消息循环
void LongUI::CUIManager::Run() noexcept {
    // 开始!
    m_dwWaitVSStartTime = ::timeGetTime();
    // 渲染线程函数
#pragma region Rendering thread function
    auto render_thread_func = [](void*) noexcept ->unsigned {
        UIWindow* windows[LongUIMaxWindow];
        HANDLE waitvs[LongUIMaxWindow];
        // 不退出?
        while (!UIManager.m_exitFlag) {
            uint32_t vslen = 0;
            // 锁住
            UIManager.Lock();
            // 有窗口
            uint32_t wndlen = UIManager.m_cCountWindow;
            // 复制数据
            for (auto i = 0u; i < wndlen; ++i) {
                windows[i] = UIManager.m_apWindows[i];
            }
            // 更新计时器
            UIManager.m_fDeltaTime = UIManager.m_uiTimer.Delta_s<float>();
            UIManager.m_uiTimer.MovStartEnd();
            // 刷新窗口
            for (auto i = 0u; i < wndlen; ++i) {
                // 刷新
                windows[i]->Update();
                // 下一帧
                windows[i]->NextFrame();
                // 获取等待垂直同步事件
                waitvs[vslen] = windows[i]->GetVSyncEvent();
                // 刷新?
                if (!windows[i]->IsRendered()) {
                    windows[i] = nullptr;
                }
                // 有效
                else if (waitvs[vslen]) {
                    ++vslen;
                }
            }
            // 解锁
            UIManager.Unlock();
            // 渲染窗口
            for (auto i = 0u; i < wndlen; ++i) {
                if (windows[i]) {
                    windows[i]->RenderWindow();
                }
            }
            // 等待垂直同步
            UIManager.WaitVS(waitvs, vslen);
        }
        return 0;
    };
#pragma endregion
    // 需要std::thread?
#ifdef LONGUI_RENDER_IN_STD_THREAD
    std::thread thread(render_thread_func, nullptr);
#else
    auto thread = reinterpret_cast<HANDLE>(
        ::_beginthreadex(nullptr, 0, render_thread_func, nullptr, 0, nullptr)
        );
    assert(thread && "failed to create thread");
#endif
    // 消息响应
    MSG msg;
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
    // 退出
    m_exitFlag = true;
    // 等待线程
#ifdef LONGUI_RENDER_IN_STD_THREAD
    try { if (thread.joinable()) { thread.join(); } }
    catch (...) { }
#else
    if (thread) {
        ::WaitForSingleObject(thread, INFINITE);
        ::CloseHandle(thread);
        thread = nullptr;
    }
#endif
    // 尝试强行关闭
    if (m_cCountWindow) {
        UIWindow* windows[LongUIMaxWindow];
        ::memcpy(windows, m_apWindows, sizeof(m_apWindows));
        auto count = m_cCountWindow;
        // 清理窗口
        for (auto i = 0u; i < count; ++i) {
            windows[count - i - 1]->Cleanup();
        }
    }
    assert(!m_cCountWindow && "bad");
    m_cCountWindow = 0;
}

// 等待垂直同步
auto LongUI::CUIManager::WaitVS(HANDLE events[], uint32_t length) noexcept ->void {
    // 获取屏幕刷新率
    DEVMODEW mode = { 0 };
    ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
    // 保留刷新
    ++m_dwWaitVSCount;
    auto end_time_of_sleep = m_dwWaitVSCount * 1000 / mode.dmDisplayFrequency;
    end_time_of_sleep += m_dwWaitVSStartTime;
    // 等待事件
    if (length) {
        ::WaitForMultipleObjects(length, events, TRUE, INFINITE);
    }
    // 保证等待
    while (::timeGetTime() < end_time_of_sleep) {
        ::Sleep(1);
    }
}

// 利用现有资源创建控件
auto LongUI::CUIManager::create_control(CreateControlFunction function, pugi::xml_node node, size_t id) noexcept -> UIControl * {
    // 检查参数 function
    if (!function) {
        assert(node && "bad argument");
        if (node) {
            function = this->GetCreateFunc(node.name());
        }
    }
    // 节点有效并且没有指定模板ID则尝试获取
    if (node && !id) {
        id = static_cast<decltype(id)>(LongUI::AtoI(
            node.attribute(LongUI::XMLAttribute::TemplateID).value())
            );
    }
    // 利用id查找模板控件
    if (id) {
        // 节点有效则添加属性
        if (node) {
            auto attribute = m_pTemplateNodes[id].first_attribute();
            // 遍历属性
            while (attribute) {
                // 添加属性
                auto name = attribute.name();
                if (!node.attribute(name)) {
                    node.insert_attribute_after(name, node.last_attribute()).set_value(attribute.value());
                }
                // 推进
                attribute = attribute.next_attribute();
            }
        }
        // 没有则直接设置
        else {
            node = m_pTemplateNodes[id];
        }
    }
    assert(function && "bad idea");
    if (!function) return nullptr;
    return function(CreateEventType::Type_CreateControl, node);
}


// 创建UI窗口
auto LongUI::CUIManager::create_ui_window(
    const pugi::xml_node node,
    UIWindow * parent, 
    callback_for_creating_window call, 
    void * user_data) noexcept -> UIWindow* {
    // 有效情况
    if (call && node) {
        // 创建窗口
        auto window = call(node, parent, user_data);
        // 查错
        assert(window); if (!window) return nullptr;
        // 重建资源
        auto hr = window->Recreate();
        AssertHR(hr);
        // 创建控件树
        this->make_control_tree(window, node);
        // 完成创建
        LongUI::EventArgument arg; ::memset(&arg, 0, sizeof(arg));
        arg.sender = window;
        arg.event = LongUI::Event::Event_TreeBulidingFinished;
        window->DoEvent(arg);
        // 返回
        return window;
    }
    return nullptr;
}


// 窗口过程函数
LRESULT LongUI::CUIManager::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    // 设置参数
    LongUI::EventArgument arg;
    // 系统消息
    arg.msg = message;  arg.sender = nullptr;
    // 返回值
    LRESULT recode = 0;
    // 创建窗口时设置指针
    if (message == WM_CREATE)    {
        // 获取指针
        LongUI::UIWindow *pUIWindow = reinterpret_cast<LongUI::UIWindow*>(
            (reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams
            );
        // 设置窗口指针
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(pUIWindow));
        // 创建完毕
        pUIWindow->OnCreated(hwnd);
        // 返回1
        recode = 1;
    }
    else {
        // 世界鼠标坐标
        POINT pt; ::GetCursorPos(&pt); ::ScreenToClient(hwnd, &pt);
        arg.pt.x = static_cast<float>(pt.x); arg.pt.y = static_cast<float>(pt.y);
        // 参数
        arg.sys.wParam = wParam; arg.sys.lParam = lParam; arg.lr = 0;
        // 获取储存的指针
        auto* pUIWindow = reinterpret_cast<LongUI::UIWindow *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA))
            );
        // 检查是否处理了
        bool wasHandled = false;
        //指针有效的情况
        if (pUIWindow) {
            AutoLocker;
            wasHandled = pUIWindow->DoEvent(arg);
        }
        // 默认处理
        recode = wasHandled ? arg.lr : ::DefWindowProcW(hwnd, message, wParam, lParam);
    }
    return recode;
}

// 获取主题颜色
auto LongUI::CUIManager::GetThemeColor(D2D1_COLOR_F& colorf) noexcept -> HRESULT {
    union { DWORD color; uint8_t argb[4]; };
    color = DWORD(-1); auto hr = S_OK; DWORD buffer_size = sizeof(DWORD);
    // 获取Colorization颜色
    ::RegGetValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\DWM", "ColorizationColor",
        RRF_RT_DWORD, nullptr, &color, &buffer_size
        );
    DWORD balance = 50; buffer_size = sizeof(DWORD);
    // 获取Colorization混合标准
    ::RegGetValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\DWM", "ColorizationColorBalance",
        RRF_RT_DWORD, nullptr, &balance, &buffer_size
        );
    {
        // 混合通道
        auto blend_channel = [](float ch1, float ch2, float prec) {
            register auto data = ch1 + (ch2 - ch1) * prec;
            return data > 1.f ? 1.f : (data < 0.f ? 0.f : data);
        };
        colorf.a = 1.f; auto prec = 1.f - float(balance) / 100.f;
        constexpr float basegrey = float(217) / 255.f;
        colorf.r = blend_channel(float(argb[2]) / 255.f, basegrey, prec);
        colorf.g = blend_channel(float(argb[1]) / 255.f, basegrey, prec);
        colorf.b = blend_channel(float(argb[0]) / 255.f, basegrey, prec);
    }
    return hr;
}


// 获取操作系统版本
namespace LongUI { auto GetWindowsVersion() noexcept->CUIManager::WindowsVersion; }

// CUIManager 构造函数
LongUI::CUIManager::CUIManager() noexcept : 
m_config(*this), 
version(LongUI::GetWindowsVersion()) {

}

// CUIManager 析构函数
LongUI::CUIManager::~CUIManager() noexcept {
    this->discard_resources();
}

// 获取控件 wchar_t指针
auto LongUI::CUIManager::
RegisterControl(CreateControlFunction func, const wchar_t* name) noexcept ->HRESULT {
    if (!name || !(*name)) return S_FALSE;
    // 超过了容器限制
    if (m_mapString2CreateFunction.size() >= LongUIMaxControlClass) {
        assert(!"out of sapce for control");
        return E_ABORT;
    }
    // 创建pair
    std::pair<LongUI::CUIString, CreateControlFunction> pair(name, func);
    HRESULT hr = S_OK;
    // 插入
    try {
        m_mapString2CreateFunction.insert(pair);
    }
    // 创建失败
    CATCH_HRESULT(hr);
    return hr;
}

// 显示错误代码
void LongUI::CUIManager::ShowError(HRESULT hr, const wchar_t* str_b) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    if (!::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,  hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        buffer, LongUIStringBufferLength,
        nullptr)) {
        // 处理
        std::swprintf(
            buffer, LongUIStringBufferLength,
            L"Error! HRESULT Code: 0x%08X", hr
            );
    }
    // 错误
    this->ShowError(buffer, str_b);
}


// 注册文本渲染器
auto LongUI::CUIManager::RegisterTextRenderer(
    CUIBasicTextRenderer* renderer, const char name[LongUITextRendererNameMaxLength]
    ) noexcept -> int32_t {
    assert(m_uTextRenderCount < lengthof(m_apTextRenderer) && "buffer too small");
    assert(!white_space(name[0]) && "name cannot begin with white space");
    // 满了
    if (m_uTextRenderCount == lengthof(m_apTextRenderer)) {
        return -1;
    }
    register const auto count = m_uTextRenderCount;
    assert((std::strlen(name) + 1) < LongUITextRendererNameMaxLength && "buffer too small");
    std::strcpy(m_aszTextRendererName[count].name, name);
    m_apTextRenderer[count] = ::SafeAcquire(renderer);
    ++m_uTextRenderCount;
    return count;
}


// 创建0索引资源
auto LongUI::CUIManager::create_indexzero_resources() noexcept->HRESULT {
    assert(m_pResourceBuffer && "bad alloc");
    HRESULT hr = S_OK;
    // 索引0位图: 可MAP位图
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateBitmap(
            D2D1::SizeU(LongUIDefaultBitmapSize, LongUIDefaultBitmapSize),
            nullptr, LongUIDefaultBitmapSize * 4,
            D2D1::BitmapProperties1(
                static_cast<D2D1_BITMAP_OPTIONS>(LongUIDefaultBitmapOptions),
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
                ),
            m_ppBitmaps + LongUIDefaultBitmapIndex
            );
    }
    // 索引0笔刷: 全控件共享用前写纯色笔刷
    if (SUCCEEDED(hr)) {
        ID2D1SolidColorBrush* brush = nullptr;
        D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Black);
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(&color, nullptr, &brush);
        m_ppBrushes[LongUICommonSolidColorBrushIndex] = ::SafeAcquire(brush);
        ::SafeRelease(brush);
    }
    // 索引0文本格式: 默认格式
    if (SUCCEEDED(hr)) {
        hr = m_pDWriteFactory->CreateTextFormat(
            LongUIDefaultTextFontName,
            m_pFontCollection,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            LongUIDefaultTextFontSize,
            m_szLocaleName,
            m_ppTextFormats + LongUIDefaultTextFormatIndex
            );
    }
    // 设置
    if (SUCCEEDED(hr)) {
        m_ppTextFormats[LongUIDefaultTextFormatIndex]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        m_ppTextFormats[LongUIDefaultTextFormatIndex]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    // 索引0META: 暂无
    if (SUCCEEDED(hr)) {

    }
    return hr;
}


// 载入模板字符串
auto LongUI::CUIManager::load_control_template_string(const char* str) noexcept->HRESULT {
    // 检查参数
    if (str && *str) {
        // 载入字符串
        auto code = m_docTemplate.load_string(str);
        if (code.status) {
            assert(!"load error");
            ::MessageBoxA(nullptr, code.description(), "<LongUI::CUIManager::load_control_template_string>: Failed to Parse/Load XML", MB_ICONERROR);
            return E_FAIL;
        }
        // 获取子节点数量
        auto get_children_count = [](pugi::xml_node node) {
            node = node.first_child();
            auto count = 0ui16;
            while (node) { node = node.next_sibling(); ++count; }
            return count;
        };
        m_cCountCtrlTemplate = 1 + get_children_count(m_docTemplate.root().first_child());
        // 解析
        return S_OK;
    }
    else {
        return S_FALSE;
    }
}


// 设置模板字符串
auto LongUI::CUIManager::set_control_template_string() noexcept->HRESULT {
    // 有效情况
    if (m_cCountCtrlTemplate > 1) {
        auto itr = m_pTemplateNodes + 1;
        auto node = m_docTemplate.root().first_child().first_child();
        // 写入索引
        while (node) {
            *itr = node;
            node = node.next_sibling();
            ++itr;
        }
    }
    return S_OK;
}


// UIManager 创建设备相关资源
auto LongUI::CUIManager::create_device_resources() noexcept ->HRESULT {
    // 待用适配器
    IDXGIAdapter1* ready2use = nullptr;
    // 枚举显示适配器
    if (!(this->flag & IUIConfigure::Flag_RenderByCPU)) {
        IDXGIFactory1* temp_factory = nullptr;
        // 创建一个临时工程
        register auto hr = LongUI::Dll::CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(&temp_factory));
        if (SUCCEEDED(hr)) {
            uint32_t adnum = 0;
            IDXGIAdapter1* apAdapters[256];
            // 枚举适配器
            for (adnum = 0; adnum < lengthof(apAdapters); ++adnum) {
                if (temp_factory->EnumAdapters1(adnum, apAdapters + adnum) == DXGI_ERROR_NOT_FOUND) {
                    break;
                }
            }
            // 选择适配器
            auto index = this->configure->ChooseAdapter(apAdapters, adnum);
            if (index < adnum) {
                ready2use = ::SafeAcquire(apAdapters[index]);
            }
            // 释放适配器
            for (size_t i = 0; i < adnum; ++i) {
                ::SafeRelease(apAdapters[i]);
            }
        }
        ::SafeRelease(temp_factory);
    }
    // 创建设备资源
    register HRESULT hr /*= m_docResource.Error() ? E_FAIL :*/ S_OK;
    // 创建 D3D11设备与设备上下文 
    if (SUCCEEDED(hr)) {
        // D3D11 创建flag 
        // 一定要有D3D11_CREATE_DEVICE_BGRA_SUPPORT
        // 否则创建D2D设备上下文会失败
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG) && defined(LONGUI_D3D_DEBUG)
        // Debug状态 有D3D DebugLayer就可以取消注释
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        auto tmpflag = D3D11_CREATE_DEVICE_FLAG(creationFlags);
        tmpflag = D3D11_CREATE_DEVICE_FLAG(0);
#endif
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };
        // 创建设备
        hr = LongUI::Dll::D3D11CreateDevice(
            // 设置为渲染
            ready2use,
            // 根据情况选择类型
            (this->flag & IUIConfigure::Flag_RenderByCPU) ? D3D_DRIVER_TYPE_WARP : 
                (ready2use ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE),
            // 没有软件接口
            nullptr,
            // 创建flag
            creationFlags,
            // 欲使用的特性等级列表
            featureLevels,
            // 特性等级列表长度
            lengthof(featureLevels),
            // SDK 版本
            D3D11_SDK_VERSION,
            // 返回的D3D11设备指针
            &m_pd3dDevice,
            // 返回的特性等级
            &m_featureLevel,
            // 返回的D3D11设备上下文指针
            &m_pd3dDeviceContext
            );
        // 检查
        if (FAILED(hr)) {
            UIManager << DL_Hint << L"Create D3D11 Device Failed,"
                L" Now, Try to Create In WARP Mode" << LongUI::endl;
        }
        // 创建失败则尝试使用软件
        if (FAILED(hr)) {
            hr = LongUI::Dll::D3D11CreateDevice(
                // 设置为渲染
                nullptr,
                // 根据情况选择类型
                D3D_DRIVER_TYPE_WARP,
                // 没有软件接口
                nullptr,
                // 创建flag
                creationFlags,
                // 欲使用的特性等级列表
                featureLevels,
                // 特性等级列表长度
                lengthof(featureLevels),
                // SDK 版本
                D3D11_SDK_VERSION,
                // 返回的D3D11设备指针
                &m_pd3dDevice,
                // 返回的特性等级
                &m_featureLevel,
                // 返回的D3D11设备上下文指针
                &m_pd3dDeviceContext
                );
        }
    }
    ::SafeRelease(ready2use);
#if defined(_DEBUG) && defined(LONGUI_D3D_DEBUG)
    // 创建 ID3D11Debug
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pd3dDebug));
    }
#endif
    // 创建 IDXGIDevice
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pDxgiDevice));
    }
    // 创建 D2D设备
    if (SUCCEEDED(hr)) {
        hr = m_pd2dFactory->CreateDevice(m_pDxgiDevice, &m_pd2dDevice);
    }
    // 创建 D2D设备上下文
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pd2dDeviceContext
            );
    }
    // 获取 Dxgi适配器 可以获取该适配器信息
    if (SUCCEEDED(hr)) {
        // 顺带使用像素作为单位
        m_pd2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        hr = m_pDxgiDevice->GetAdapter(&m_pDxgiAdapter);
    }
#ifdef _DEBUG
    // 输出显卡信息
    if (SUCCEEDED(hr)) {
        DXGI_ADAPTER_DESC desc = { 0 }; 
        m_pDxgiAdapter->GetDesc(&desc);
        UIManager << DL_Log << desc << LongUI::endl;
    }
#endif
    // 获取 Dxgi工厂
    if (SUCCEEDED(hr)) {
        hr = m_pDxgiAdapter->GetParent(LongUI_IID_PV_ARGS(m_pDxgiFactory));
    }
#ifdef LONGUI_VIDEO_IN_MF
    UINT token = 0;
    // 多线程
    if (SUCCEEDED(hr)) {
        ID3D10Multithread* mt = nullptr;
        hr = m_pd3dDevice->QueryInterface(IID_ID3D10Multithread, (void**)&mt);
        // 保护
        if (SUCCEEDED(hr)) {
            mt->SetMultithreadProtected(TRUE);
        }
        ::SafeRelease(mt);
    }
    // 设置 MF
    if (SUCCEEDED(hr)) {
        hr = ::MFStartup(MF_VERSION);
    }
    // 创建 MF Dxgi 设备管理器
    if (SUCCEEDED(hr)) {
        hr = ::MFCreateDXGIDeviceManager(&token, &m_pMFDXGIManager);
    }
    // 重置设备
    if (SUCCEEDED(hr)) {
        hr = m_pMFDXGIManager->ResetDevice(m_pd3dDevice, token);
    }
    // 创建 MF媒体类工厂
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_MFMediaEngineClassFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pMediaEngineFactory)
            );
    }
#endif
    // 创建系统笔刷
    if (SUCCEEDED(hr)) {
        hr = this->create_system_brushes();
    }
    // 创建资源描述资源
    if (SUCCEEDED(hr)) {
        hr = this->create_indexzero_resources();
    }
    // 事件
    if (SUCCEEDED(hr)) {
        this->do_creating_event(LongUI::CreateEventType::Type_Recreate);
    }
    // 设置文本渲染器数据
    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0u; i < m_uTextRenderCount; ++i) {
            m_apTextRenderer[i]->SetNewTarget(m_pd2dDeviceContext);
            m_apTextRenderer[i]->SetNewBrush(
                static_cast<ID2D1SolidColorBrush*>(m_ppBrushes[LongUICommonSolidColorBrushIndex])
                );
        }
        // 重建所有窗口
        for (auto itr = m_apWindows; itr < m_apWindows + m_cCountWindow; ++itr) {
            (*itr)->Recreate();
        }
    }
    // 断言 HR
    AssertHR(hr);
    return hr;
}


// 创建系统笔刷
auto LongUI::CUIManager::create_system_brushes() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    /*
    焦点: 0x3399FF 矩形描边, 并且内边有虚线矩形
        0. 禁用: 0xD9灰度 矩形描边; 中心 0xEF灰色
        1. 普通: 0xAC灰度 矩形描边; 中心 从上到下0xF0灰色到0xE5灰色渐变
        2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
        3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变
    */
    // 禁用
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(0xEFEFEF),
            reinterpret_cast<ID2D1SolidColorBrush**>(m_apSystemBrushes + Status_Disabled)
            );
    }
    // 普通
    if (SUCCEEDED(hr)) {
        ID2D1GradientStopCollection* collection = nullptr;
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f, D2D1::ColorF(0xF0F0F0) },
            { 1.f, D2D1::ColorF(0xE5E5E5) }
        };
        // 渐变关键点集
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateGradientStopCollection(
                stops, lengthof(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + Status_Normal)
                );
        }
        ::SafeRelease(collection);
    }
    // 移上
    if (SUCCEEDED(hr)) {
        ID2D1GradientStopCollection* collection = nullptr;
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f, D2D1::ColorF(0xECF4FC) },
            { 1.f, D2D1::ColorF(0xDCECFC) }
        };
        // 渐变关键点集
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateGradientStopCollection(
                stops, lengthof(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + Status_Hover)
                );
        }
        ::SafeRelease(collection);
    }
    // 按下
    if (SUCCEEDED(hr)) {
        ID2D1GradientStopCollection* collection = nullptr;
        D2D1_GRADIENT_STOP stops[] = {
            { 0.f, D2D1::ColorF(0xDAECFC) } ,
            { 1.f, D2D1::ColorF(0xC4E0FC) } ,
        };
        // 渐变关键点集
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateGradientStopCollection(
                stops, lengthof(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + Status_Pushed)
                );
        }
        ::SafeRelease(collection);
    }
    return hr;
}

#if defined(_DEBUG) && defined(_MSC_VER)
extern ID3D11Debug*    g_pd3dDebug_longui;
#endif

// UIManager 丢弃
void LongUI::CUIManager::discard_resources() noexcept {
    // 释放系统笔刷
    for (auto& brush : m_apSystemBrushes) {
        ::SafeRelease(brush);
    }
    // 释放公共设备相关资源
    {
        // 释放 位图
        for (auto itr = m_ppBitmaps; itr != m_ppBitmaps + m_cCountBmp; ++itr) {
            ::SafeRelease(*itr);
        }
        // 释放 笔刷
        for (auto itr = m_ppBrushes; itr != m_ppBrushes + m_cCountBrs; ++itr) {
            ::SafeRelease(*itr);
        }
        // META
        for (auto itr = m_pMetasBuffer; itr != m_pMetasBuffer + m_cCountMt; ++itr) {
            LongUI::DestoryObject(*itr);
            itr->bitmap = nullptr;
        }
    }
    // 清除
    if (m_pd2dDevice) {
        m_pd2dDevice->ClearResources();
    }
    // 释放 设备
    ::SafeRelease(m_pDxgiFactory);
    ::SafeRelease(m_pd2dDeviceContext);
    ::SafeRelease(m_pd2dDevice);
    ::SafeRelease(m_pDxgiAdapter);
    ::SafeRelease(m_pDxgiDevice);
    ::SafeRelease(m_pd3dDevice);
    ::SafeRelease(m_pd3dDeviceContext);
#ifdef LONGUI_VIDEO_IN_MF
    ::SafeRelease(m_pMFDXGIManager);
    ::SafeRelease(m_pMediaEngineFactory);
    ::MFShutdown();
#endif
#ifdef _DEBUG
#ifdef _MSC_VER
    __try {
        if (m_pd3dDebug) {
            ::SafeRelease(g_pd3dDebug_longui);
            g_pd3dDebug_longui = m_pd3dDebug;
            m_pd3dDebug = nullptr; 
        }
    }
    __finally {
        m_pd3dDebug = nullptr;
    }
#else
    if (m_pd3dDebug) {
        m_pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
    }
    ::SafeRelease(m_pd3dDebug);
#endif
    this;
#endif
}

// 获取位图
auto LongUI::CUIManager::GetBitmap(size_t index) noexcept ->ID2D1Bitmap1* {
    // 越界
    if (index >= m_cCountBmp) {
        UIManager << DL_Warning 
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0" 
            << LongUI::endl;
        index = 0;
    }
    auto bitmap = m_ppBitmaps[index];
    // 没有数据则载入
    if (!bitmap) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        // 载入资源
        m_ppBitmaps[index] = static_cast<ID2D1Bitmap1*>(
            m_pResourceLoader->GetResourcePointer(m_pResourceLoader->Type_Bitmap, index - 1)
            );
        bitmap = m_ppBitmaps[index];
    }
    // 再没有数据则报错
    if (!bitmap) {
        UIManager << DL_Error << L"index @ " << long(index) << L"bitmap is null" << LongUI::endl;
    }
    return ::SafeAcquire(bitmap);
}

// 获取笔刷
auto LongUI::CUIManager::GetBrush(size_t index) noexcept -> ID2D1Brush* {
    // 越界
    if (index >= m_cCountBrs) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
    }
    auto brush = m_ppBrushes[index];
    // 没有数据则载入
    if (!brush) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        // 载入资源
        m_ppBrushes[index] = static_cast<ID2D1Brush*>(
            m_pResourceLoader->GetResourcePointer(m_pResourceLoader->Type_Brush, index - 1)
            );
        brush = m_ppBrushes[index];
    }
    // 再没有数据则报错
    if (!brush) {
        UIManager << DL_Error << L"index @ " << long(index) << L"brush is null" << LongUI::endl;
    }
    return ::SafeAcquire(brush);
}

// CUIManager 获取文本格式
auto LongUI::CUIManager::GetTextFormat(size_t index) noexcept ->IDWriteTextFormat* {
    // 越界
    if (index >= m_cCountTf) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
    }
    auto format = m_ppTextFormats[index];
    // 没有数据则载入
    if (!format) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        // 载入资源
        m_ppTextFormats[index] = static_cast<IDWriteTextFormat*>(
            m_pResourceLoader->GetResourcePointer(m_pResourceLoader->Type_TextFormat, index - 1)
            );
        format = m_ppTextFormats[index];
    }
    // 再没有数据则报错
    if (!format) {
        UIManager << DL_Error << L"index @ " << long(index) << L"text format is null" << LongUI::endl;
    }
    return ::SafeAcquire(format);
}

// 利用名称获取
auto LongUI::CUIManager::GetTextRenderer(const char* name) const noexcept -> CUIBasicTextRenderer* {
    int index = 0;
    if (name && name[0]) {
        // 跳过空白
        while (white_space(*name)) ++name;
        // 检查数字
        if (*name >= '0' && *name <= '9') {
            index = LongUI::AtoI(name);
        }
        // 线性查找
        else {
            for (int i = 0; i < int(m_uTextRenderCount); ++i) {
                if (!std::strcmp(m_aszTextRendererName[i].name, name)) {
                    index = i;
                    break;
                }
            }
        }
    }
    return this->GetTextRenderer(index);
}

// 获取图元
void LongUI::CUIManager::GetMeta(size_t index, LongUI::Meta& meta) noexcept {
    // 越界
    if (index >= m_cCountMt) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
        ZeroMemory(&meta, sizeof(meta));
        return;
    }
    meta = m_pMetasBuffer[index];
    // 没有位图数据则载入
    if (!meta.bitmap) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        DeviceIndependentMeta meta_raw;
        ::ZeroMemory(&meta_raw, sizeof(meta_raw));
        // 载入资源
        m_pResourceLoader->GetMeta(index - 1, meta_raw);
        meta.interpolation = meta_raw.interpolation;
        meta.src_rect = meta_raw.src_rect;
        meta.rule = meta_raw.rule;
        meta.bitmap = this->GetBitmap(meta_raw.bitmap_index);
        // 减少计数
        if (meta.bitmap) {
            meta.bitmap->Release();
        }
    }
    // 再没有数据则报错
    if (!meta.bitmap) {
        UIManager << DL_Error << L"index @ " << long(index) << L"meta is null" << LongUI::endl;
    }
}

// 获取Meta的图标句柄
auto LongUI::CUIManager::GetMetaHICON(size_t index) noexcept -> HICON {
    // 越界
    if (index >= m_cCountMt) {
        UIManager << DL_Warning
            << L"[index @ " << long(index)
            << L"]is out of range \t\tNow set to 0"
            << LongUI::endl;
        index = 0;
    }
    // 有就直接返回
    if (m_phMetaIcon[index]) return m_phMetaIcon[index];
    LongUI::Meta meta; this->GetMeta(index, meta);
    assert(meta.bitmap);
    ID2D1Bitmap1* bitmap = this->GetBitmap(LongUIDefaultBitmapIndex);
    D2D1_RECT_U src_rect = {
        static_cast<uint32_t>(meta.src_rect.left),
        static_cast<uint32_t>(meta.src_rect.top),
        static_cast<uint32_t>(meta.src_rect.right),
        static_cast<uint32_t>(meta.src_rect.bottom)
    };
    HRESULT hr = S_OK;
    // 宽度不够?
    if (SUCCEEDED(hr)) {
        if (src_rect.right - src_rect.left > LongUIDefaultBitmapSize ||
            src_rect.bottom - src_rect.top > LongUIDefaultBitmapSize) {
            assert(!"width/height is too large");
            hr = E_FAIL;
        }
    }
    // 检查错误
    if (SUCCEEDED(hr)) {
        if (!(bitmap && meta.bitmap)) {
            hr = E_POINTER;
        }
    }
    // 复制数据
    if (SUCCEEDED(hr)) {
        hr = bitmap->CopyFromBitmap(nullptr, meta.bitmap, &src_rect);
    }
    // 映射数据
    if (SUCCEEDED(hr)) {
        D2D1_MAPPED_RECT mapped_rect = {
            LongUIDefaultBitmapSize * sizeof(RGBQUAD) ,
            m_pBitmap0Buffer
        };
        hr = bitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped_rect);
    }
    // 取消映射
    if (SUCCEEDED(hr)) {
        hr = bitmap->Unmap();
    }
    // 转换数据
    HICON hAlphaIcon = nullptr;
    if (SUCCEEDED(hr)) {
        auto meta_width = src_rect.right - src_rect.left;
        auto meta_height = src_rect.bottom - src_rect.top;
#if 1
        BITMAPV5HEADER bi; ZeroMemory(&bi, sizeof(BITMAPV5HEADER));
        bi.bV5Size = sizeof(BITMAPV5HEADER);
        bi.bV5Width = meta_width;
        bi.bV5Height = meta_height;
        bi.bV5Planes = 1;
        bi.bV5BitCount = 32;
        bi.bV5Compression = BI_BITFIELDS;
        // 掩码填写
        bi.bV5RedMask = 0x00FF0000;
        bi.bV5GreenMask = 0x0000FF00;
        bi.bV5BlueMask = 0x000000FF;
        bi.bV5AlphaMask = 0xFF000000;
        HDC hdc = ::GetDC(nullptr);
        uint8_t* pTargetBuffer = nullptr;
        // 创建带Alpha通道DIB
        auto hBitmap = ::CreateDIBSection(
            hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS,
            reinterpret_cast<void **>(&pTargetBuffer), nullptr,
            (DWORD)0
            );
        auto hMemDC = ::CreateCompatibleDC(hdc);
        ::ReleaseDC(nullptr, hdc);
        // 写入数据
        auto hOldBitmap = static_cast<HBITMAP*>(::SelectObject(hMemDC, hBitmap));
        ::PatBlt(hMemDC, 0, 0, meta_width, meta_height, WHITENESS);
        ::SelectObject(hMemDC, hOldBitmap);
        ::DeleteDC(hMemDC);
        // 创建掩码位图
        HBITMAP hMonoBitmap = ::CreateBitmap(meta_width, meta_height, 1, 1, nullptr);
        // 输入
        auto lpdwPixel = reinterpret_cast<DWORD*>(pTargetBuffer);
        for (auto y = 0u; y < meta_height; ++y) {
            auto src_buffer = m_pBitmap0Buffer + LongUIDefaultBitmapSize * sizeof(RGBQUAD) * y;
            for (auto x = 0u; x < meta_width; ++x) {
                *lpdwPixel = *src_buffer;
                src_buffer++;
                lpdwPixel++;
            }
        }
        // 填写
        ICONINFO ii;
        ii.fIcon = TRUE; ii.xHotspot = 0; ii.yHotspot = 0;
        ii.hbmMask = hMonoBitmap; ii.hbmColor = hBitmap;
        // 创建图标
        hAlphaIcon = ::CreateIconIndirect(&ii);
        ::DeleteObject(hBitmap);
        ::DeleteObject(hMonoBitmap);
#else
        assert(!"CreateIcon just AND & XOR, no alpha channel");
#endif
    }
    AssertHR(hr);
    ::SafeRelease(bitmap);
    return m_phMetaIcon[index] = hAlphaIcon;
}



// 添加窗口
void LongUI::CUIManager::RegisterWindow(UIWindow * wnd) noexcept {
    assert(wnd && "bad argument");
    // 检查剩余空间
    if (m_cCountWindow >= LongUIMaxWindow) {
        assert(!"ABORT! OUT OF SPACE! m_cCountWindow >= LongUIMaxWindow");
        return;
    }
    // 检查是否已经存在
#ifdef _DEBUG
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) != endwindow) {
            assert(!"target window has been registered.");
        }
    }
#endif
    // 添加窗口
    m_apWindows[m_cCountWindow] = wnd; ++m_cCountWindow;
}

// 移出窗口
void LongUI::CUIManager::RemoveWindow(UIWindow * wnd, bool cleanup) noexcept {
    assert(m_cCountWindow); assert(wnd && "bad argument");
    // 清理?
    if (cleanup) {
        wnd->Cleanup();
#ifdef _DEBUG
        // 现在已经不再数组中了, 不过需要检查一下
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) != endwindow) {
            assert(!"remove window failed!");
        }
#endif
        return;
    }
    // 检查时是不是在本数组中
#ifdef _DEBUG
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) == endwindow) {
            assert(!"target window not in windows array!");
            return;
        }
    }
#endif
    // 一次循环就搞定
    {
        const register auto count = m_cCountWindow;
        bool found = false;
        for (auto i = 0u; i < m_cCountWindow; ++i) {
            // 找到后, 后面的元素依次前移
            if (found) {
                m_apWindows[i] = m_apWindows[i + 1];
            }
            // 没找到就尝试
            else if(m_apWindows[i] == wnd) {
                found = true;
                m_apWindows[i] = m_apWindows[i + 1];
            }
        }
        assert(found && "window not found");
        --m_cCountWindow;
        m_cCountWindow[m_apWindows] = nullptr;
    }
}

// 是否以管理员权限运行
bool LongUI::CUIManager::IsRunAsAdministrator() noexcept {
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    ::AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup
    );
    // 成功? 清理资源
    if (pAdministratorsGroup) {
        ::CheckTokenMembership(nullptr, pAdministratorsGroup, &fIsRunAsAdmin);
        ::FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = nullptr;
    }
    // 返回结果
    return fIsRunAsAdmin != 0;
}

// 提升权限
bool LongUI::CUIManager::TryElevateUACNow(const wchar_t* parameters, bool exit) noexcept {
    if (!CUIManager::IsRunAsAdministrator()) {
        wchar_t szPath[MAX_PATH];
        // 获取实例句柄
        if (::GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
            // Launch itself as admin
            SHELLEXECUTEINFOW sei = { 0 };
            sei.cbSize = sizeof(sei);
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.lpParameters = parameters;
            sei.hwnd = nullptr;
            sei.nShow = SW_NORMAL;
            // 执行
            if (!::ShellExecuteExW(&sei)) {
#ifdef _DEBUG
                DWORD dwError = ::GetLastError();
                assert(dwError == ERROR_CANCELLED && "anyelse?");
#endif
                return false;
            }
            else if(exit) {
                // 退出
                UIManager.Exit();
            }
        }
    }
    return true;
}

//#include <valarray>

#ifdef _DEBUG

// 传递可视化东西
auto LongUI::Formated(const wchar_t* format, ...) noexcept -> const wchar_t* {
    static wchar_t buffer[LongUIStringBufferLength];
    va_list ap;
    va_start(ap, format);
    std::vswprintf(buffer, LongUIStringBufferLength, format, ap);
    va_end(ap);
    return buffer;
}

// 换行刷新重载
auto LongUI::CUIManager::operator<<(const LongUI::EndL) noexcept ->CUIManager& {
    wchar_t chs[3] = { L'\r',L'\n', 0 }; 
    this->Output(m_lastLevel, chs);
    return *this;
}

auto LongUI::CUIManager::operator<<(const DXGI_ADAPTER_DESC& desc) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"Adapter:   { \r\n\t Description: %ls\r\n\t VendorId: 0x%08X"
        L"\t\t DeviceId: 0x%08X\r\n\t SubSysId: 0x%08X\t\t Revision: 0x%08X\r\n"
        L"\t DedicatedVideoMemory: %.3lfMB\r\n"
        L"\t DedicatedSystemMemory: %.3lfMB\r\n"
        L"\t SharedSystemMemory: %.3lfMB\r\n"
        L"\t AdapterLuid: 0x%08X%08X\r\n }",
        desc.Description,
        desc.VendorId,
        desc.DeviceId,
        desc.SubSysId,
        desc.Revision,
        static_cast<double>(desc.DedicatedVideoMemory) / (1024.*1024.),
        static_cast<double>(desc.DedicatedSystemMemory) / (1024.*1024.),
        static_cast<double>(desc.SharedSystemMemory) / (1024.*1024.),
        desc.AdapterLuid.HighPart,
        desc.AdapterLuid.LowPart
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const RectLTWH_F& rect) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_WH(%7.2f, %7.2f, %7.2f, %7.2f)",
        rect.left, rect.top, rect.width, rect.height
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_MATRIX_3X2_F& matrix) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"MATRIX (%7.2f, %7.2f, %7.2f, %7.2f, %7.2f, %7.2f)",
        matrix._11, matrix._12, 
        matrix._21, matrix._22, 
        matrix._31, matrix._32
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_RECT_F& rect) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_RB(%7.2f, %7.2f, %7.2f, %7.2f)",
        rect.left, rect.top, rect.right, rect.bottom
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_POINT_2F& pt) noexcept->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"POINT(%7.2f, %7.2f)",
        pt.x, pt.y
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 输出UTF-8字符串 并刷新
void LongUI::CUIManager::Output(DebugStringLevel l, const char * s) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    buffer[LongUI::UTF8toWideChar(s, buffer)] = 0;
    this->Output(l, buffer);
}

// 输出UTF-8字符串
void LongUI::CUIManager::OutputNoFlush(DebugStringLevel l, const char * s) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    buffer[LongUI::UTF8toWideChar(s, buffer)] = 0;
    this->OutputNoFlush(l, buffer);
}

// 浮点重载
auto LongUI::CUIManager::operator<<(const float f) noexcept ->CUIManager&  {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(buffer, LongUIStringBufferLength, L"%.2f", f);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 控件
auto LongUI::CUIManager::operator<<(const UIControl* ctrl) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    if (ctrl) {
#if 1
        std::swprintf(
            buffer, LongUIStringBufferLength,
            L"[Control:%ls@%ls@0x%p] ",
            ctrl->GetNameStr(),
            ctrl->GetControlClassName(false),
            ctrl
            );
#else
        std::swprintf(
            buffer, LongUIStringBufferLength,
            L"[Control:%ls@0x%p] ",
            ctrl->GetNameStr(),
            ctrl
            );
#endif
    }
    else {
        std::swprintf(buffer, LongUIStringBufferLength, L"[Control:null] ");
    }
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 整型重载
auto LongUI::CUIManager::operator<<(const long l) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(buffer, LongUIStringBufferLength, L"%d", l);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 整型重载
auto LongUI::CUIManager::operator<<(const bool b) noexcept ->CUIManager& {
    this->OutputNoFlush(m_lastLevel, b ? "true" : "false");
    return *this;
}

#endif
                   


// 渲染队列 构造函数
LongUI::CUIRenderQueue::CUIRenderQueue(UIWindow* window) noexcept {
    m_unitLike.length = 0; m_unitLike.window = window;
}

// 渲染队列 析构函数
LongUI::CUIRenderQueue::~CUIRenderQueue() noexcept {
    // 释放数据
    if (m_pUnitsDataBegin) {
        LongUI::NormalFree(m_pUnitsDataBegin);
    }
    m_pUnitsDataBegin = m_pUnitsDataEnd = m_pCurrentUnit = nullptr;
}

// 重置
void LongUI::CUIRenderQueue::Reset(uint32_t freq) noexcept {
    // 一样就不处理
    if (m_wDisplayFrequency == freq) return;
    // 修改
    m_wDisplayFrequency = static_cast<decltype(m_wDisplayFrequency)>(freq);
    // 创建
    CUIRenderQueue::UNIT* data = nullptr;
    if (freq) {
        data = LongUI::NormalAllocT(data, LongUIPlanRenderingTotalTime * freq);
        if (data) {
            for (auto i = 0u; i < LongUIPlanRenderingTotalTime * freq; ++i) {
                data[i].length = 0;
            }
        }
    }
    // TODO: 完成转化
    if (m_pUnitsDataBegin && data) {
        assert(!"NOTIMPL");
    }
    // 释放
    if (m_pUnitsDataBegin) LongUI::NormalFree(m_pUnitsDataBegin);
    // 转移
    if (data) {
        m_pUnitsDataBegin = data;
        m_pUnitsDataEnd = data + LongUIPlanRenderingTotalTime * freq;
        // XXX
        m_pCurrentUnit = data;
    }
    // 开始渲染
    m_dwStartTime = ::timeGetTime();
}

// ++ 操作符
void LongUI::CUIRenderQueue::operator++() noexcept {
    // 渲染队列模式
    if (m_pCurrentUnit) {
        ++m_pCurrentUnit;
        if (m_pCurrentUnit == m_pUnitsDataEnd) {
            m_pCurrentUnit = m_pUnitsDataBegin;
            // 检查误差
            register auto time = m_dwStartTime;
            m_dwStartTime = ::timeGetTime();
            time = m_dwStartTime - time;
            int16_t dev = int16_t(int16_t(time) - int16_t(LongUIPlanRenderingTotalTime * 1000));
            m_sTimeDeviation += dev;
            UIManager << DL_Log << "Time Deviation: "
                << long(dev) << " ms    Totle: " << long(m_sTimeDeviation)
                << " ms" << endl;
            // TODO: 时间校正
        }
    }
    // 简单模式
    else {
        assert(!"error");
    }
}

// 计划渲染
void LongUI::CUIRenderQueue::PlanToRender(float wait, float render, UIControl* ctrl) noexcept {
    // 保留刷新
    if (render != 0.0f) render += 0.05f;
    assert((wait + render) < float(LongUIPlanRenderingTotalTime) && "time overflow");
    // 当前窗口
    auto window = m_unitLike.window;
    // 设置单元
    auto set_unit = [window](UNIT* unit, UIControl* ctrl) noexcept {
        // 已经全渲染了就不干
        if (unit->length && unit->units[0] == window) {
            return;
        }
        // 单元满了就设置为全渲染
        if (unit->length == LongUIDirtyControlSize) {
            unit->length = 1;
            unit->units[0] = window;
            return;
        }
        // 获取真正窗口
        auto get_real_render_control = [window](UIControl* control) noexcept {
            // 获取真正
            while (control != window) {
                if (control->flags & Flag_RenderParent) control = control->parent;
                else break;
            }
            return control;
        };
        // 渲染窗口也设置为全渲染
        ctrl = get_real_render_control(ctrl);
        if (ctrl == window) {
            unit->length = 1;
            unit->units[0] = window;
            return;
        }
        // 可视化区域 > 有的 且自己为容器  -> 替换
        // 可视化区域 < 有的 且有的为容器  -> 不干
        // 不在里面                        -> 加入
        // XXX: 有BUG
        const auto test_container = [](const UIControl* ctrl1, const UIControl* ctrl2) noexcept {
            return ctrl1->flags & Flag_UIContainer &&
                ctrl1->visible_rect.left <= ctrl2->visible_rect.left &&
                ctrl1->visible_rect.top <= ctrl2->visible_rect.top &&
                ctrl1->visible_rect.right <= ctrl2->visible_rect.right &&
                ctrl1->visible_rect.bottom <= ctrl2->visible_rect.bottom;
        };
        // 检查是否在单元里面
        register bool not_in = true;
        for (auto unit_ctrl = unit->units; unit_ctrl < unit->units + unit->length; ++unit_ctrl) {
            // 在里面了
            if (*unit_ctrl == ctrl) {
                not_in = false;
                break;
            }
            // 可视化区域 > 有的 且自己为容器  -> 替换
            else if(test_container(ctrl, *unit_ctrl)){
                *unit_ctrl = ctrl;
                not_in = false;
                break;
            }
            // 可视化区域 < 有的 且有的为容器  -> 不干
            else if (test_container(*unit_ctrl, ctrl)) {
                not_in = false;
                break;
            }
        }
        // 不在单元里面就加入
        if (not_in) {
            unit->units[unit->length] = ctrl;
            ++unit->length;
        }
    };
    // 渲染队列模式
    if (m_pCurrentUnit) {
        // 时间片计算
        auto frame_offset = long(wait * float(m_wDisplayFrequency));
        auto frame_count = long(render * float(m_wDisplayFrequency)) + 1;
        auto start = m_pCurrentUnit + frame_offset;
        for (long i = 0; i < frame_count; ++i) {
            if (start >= m_pUnitsDataEnd) {
                start -= LongUIPlanRenderingTotalTime * m_wDisplayFrequency;
            }
            set_unit(start, ctrl);
            ++start;
        }
    }
    // 简单模式
    else {
        assert(!"error");
    }
}

// ---------------- VERSION HELPER -------------------

// longui namespace
namespace LongUI {
     // 检查Windows版本
     bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor) noexcept {
        RTL_OSVERSIONINFOEXW verInfo = { 0 };
        verInfo.dwOSVersionInfoSize = sizeof(verInfo);
        // 获取地址
        static auto RtlGetVersion = reinterpret_cast<LongUI::fnRtlGetVersion>(
            ::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")
            );
        // 比较版本
        if (RtlGetVersion && RtlGetVersion((PRTL_OSVERSIONINFOW)&verInfo) == 0) {
            if (verInfo.dwMajorVersion > wMajorVersion)
                return true;
            else if (verInfo.dwMajorVersion < wMajorVersion)
                return false;
            if (verInfo.dwMinorVersion > wMinorVersion)
                return true;
            else if (verInfo.dwMinorVersion < wMinorVersion)
                return false;
            if (verInfo.wServicePackMajor >= wServicePackMajor)
                return true;
        }
        return false;
    }
    // 获取Windows版本
    auto GetWindowsVersion() noexcept->CUIManager::WindowsVersion {
        CUIManager::WindowsVersion version = CUIManager::WindowsVersion::Version_Win7SP1;
        // >= Win10 ?
        if (LongUI::IsWindows10OrGreater()) {
            version = CUIManager::WindowsVersion::Version_Win10;
        }
        // >= Win8.1 ?
        else if (LongUI::IsWindows8Point1OrGreater()) {
            version = CUIManager::WindowsVersion::Version_Win8_1;
        }
        // >= Win8 ?
        else if (LongUI::IsWindows8OrGreater()) {
            version = CUIManager::WindowsVersion::Version_Win8;
        }
        return version;
    }
}

#ifdef LONGUI_WITH_DEFAULT_CONFIG
#include <wincodec.h>
//  ---------- Resource Loader for XML -----------------
namespace LongUI {
    // IWICImagingFactory2 "7B816B45-1996-4476-B132-DE9E247C8AF0"
    static const IID IID_IWICImagingFactory2 =
    { 0x7B816B45, 0x1996, 0x4476,{ 0xB1, 0x32, 0xDE, 0x9E, 0x24, 0x7C, 0x8A, 0xF0 } };
    template<> LongUIInline const IID& GetIID<IWICImagingFactory2>() {
        return LongUI::IID_IWICImagingFactory2;
    }
    // CUIResourceLoaderXML, default impl for IUIResourceLoader
    class CUIResourceLoaderXML : public IUIResourceLoader {
    public:
        // add ref count
        auto STDMETHODCALLTYPE AddRef() noexcept->ULONG override final { return ++m_dwCounter; }
        // release this
        auto STDMETHODCALLTYPE Release() noexcept->ULONG override final { 
            auto old = --m_dwCounter; 
            if (!old) { delete this; } 
            return old; 
        };
    public:
        // get resouce count with type
        auto GetResourceCount(ResourceType type) const noexcept->size_t override;
        // get resouce by index, index in range [0, count)
        // for Type_Bitmap, Type_Brush, Type_TextFormat
        auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* override;
        // get meta by index, index in range [0, count)
        auto GetMeta(size_t index, DeviceIndependentMeta&) noexcept ->void override;
    private:
        // get resouce count from doc
        void get_resource_count_from_xml() noexcept;
        // get bitmap
        auto get_bitmap(pugi::xml_node node) noexcept->ID2D1Bitmap1*;
        // get brush
        auto get_brush(pugi::xml_node node) noexcept->ID2D1Brush*;
        // get text format
        auto get_text_format(pugi::xml_node node) noexcept->IDWriteTextFormat*;
        // find node with index
        static auto find_node_with_index(pugi::xml_node node, const size_t index) noexcept {
            pugi::xml_node found_node;
            size_t i = 0;
            for (auto itr = node.first_child(); itr; itr = itr.next_sibling()) {
                if (i == index) {
                    found_node = itr;
                    break;
                }
                ++i;
            }
            return found_node;
        }
    public:
        // ctor
        CUIResourceLoaderXML(CUIManager& manager, const char* xml) noexcept;
        // dtor
        ~CUIResourceLoaderXML() noexcept;
    private:
        //  ui manager
        CUIManager&             m_manager;
        // WIC factory
        IWICImagingFactory2*    m_pWICFactory = nullptr;
        // node for reource
        pugi::xml_node          m_aNodes[RESOURCE_TYPE_COUNT];
        // xml doc for resource
        pugi::xml_document      m_docResource;
        // resource count
        uint32_t                m_aResourceCount[RESOURCE_TYPE_COUNT];
        // ref counter for this
        uint32_t                m_dwCounter = 1;
    };
    // --------------------------------------------------------------------
    // create resource loader
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) 
        noexcept -> IUIResourceLoader* {
        return new(std::nothrow) CUIResourceLoaderXML(manager, xml);
    }
    // ctor for CUIResourceLoaderXML
    LongUI::CUIResourceLoaderXML::CUIResourceLoaderXML(
        CUIManager& manager, const char* xml)  noexcept : m_manager(manager) {
        // 初始化
        ::memset(m_aResourceCount, 0, sizeof(m_aResourceCount));
        auto hr = S_OK;
        // 创建 WIC 工厂.
        if (SUCCEEDED(hr)) {
            hr = ::CoCreateInstance(
                CLSID_WICImagingFactory2,
                nullptr,
                CLSCTX_INPROC_SERVER,
                LongUI_IID_PV_ARGS(m_pWICFactory)
                );
        }
        // 载入
        if (SUCCEEDED(hr) && xml) {
            auto re = m_docResource.load_string(xml);
            // 错误
            if (re.status) {
                assert(!"failed to load string");
                ::MessageBoxA(nullptr, re.description(), "<LongUI::CUIResourceLoaderXML::CUIResourceLoaderXML>: Failed to Parse/Load XML", MB_ICONERROR);
                hr = E_FAIL;
            }
            // 遍历
            else {
                this->get_resource_count_from_xml();
            }
        }
        // 显示错误
        if (FAILED(hr)) {
            manager.ShowError(hr);
        }
    }
    // dtor for CUIResourceLoaderXML
    LongUI::CUIResourceLoaderXML::~CUIResourceLoaderXML() noexcept {
        ::SafeRelease(m_pWICFactory);
    }
    // get reource count
    auto LongUI::CUIResourceLoaderXML::GetResourceCount(ResourceType type) const noexcept -> size_t {
        assert(type < this->RESOURCE_TYPE_COUNT);
        return static_cast<size_t>(m_aResourceCount[type]);
    }
    // get reource
    auto LongUI::CUIResourceLoaderXML::GetResourcePointer(ResourceType type, size_t index) noexcept -> void * {
        void* data = nullptr;
        auto node = this->find_node_with_index(m_aNodes[type], index);
        switch (type)
        {
        case LongUI::IUIResourceLoader::Type_Bitmap:
            data = this->get_bitmap(node);
            break;
        case LongUI::IUIResourceLoader::Type_Brush:
            data = this->get_brush(node);
            break;
        case LongUI::IUIResourceLoader::Type_TextFormat:
            data = this->get_bitmap(node);
            break;
        case LongUI::IUIResourceLoader::Type_Meta:
            __fallthrough;
        case LongUI::IUIResourceLoader::Type_Null:
            __fallthrough;
        default:
            assert(!"unknown resource type");
            break;
        }
        return data;
    }
    // get meta
    auto LongUI::CUIResourceLoaderXML::GetMeta(size_t index, DeviceIndependentMeta& meta_raw) noexcept -> void {
        auto node = this->find_node_with_index(m_aNodes[this->Type_Meta], index);
        assert(node && "node not found");
        meta_raw = {
            { 0.f, 0.f, 1.f, 1.f },
            uint32_t(LongUI::AtoI(node.attribute("bitmap").value())),
            Helper::XMLGetBitmapRenderRule(node, BitmapRenderRule::Rule_Scale),
            uint16_t(Helper::XMLGetD2DInterpolationMode(node, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR))
        };
        assert(meta_raw.bitmap_index && "bad bitmap index");
        // 获取矩形
        Helper::MakeFloats(node.attribute("rect").value(), &meta_raw.src_rect.left, 4);
    }
    // get reource count from doc
    void LongUI::CUIResourceLoaderXML::get_resource_count_from_xml() noexcept {
        // 初始化
        for (auto& node : m_aNodes) { node = pugi::xml_node(); }
        // pugixml 使用的是句柄式, 所以下面的代码是安全的.
        register auto now_node = m_docResource.first_child().first_child();
        while (now_node) {
            // 获取子节点数量
            auto get_children_count = [](pugi::xml_node node) {
                node = node.first_child();
                auto count = 0ui32;
                while (node) { node = node.next_sibling(); ++count; }
                return count;
            };
            // 位图?
            if (!::strcmp(now_node.name(), "Bitmap")) {
                m_aNodes[Type_Bitmap] = now_node;
                m_aResourceCount[this->Type_Bitmap] = get_children_count(now_node);
            }
            // 笔刷?
            else if (!::strcmp(now_node.name(), "Brush")) {
                m_aNodes[Type_Brush] = now_node;
                m_aResourceCount[this->Type_Brush] = get_children_count(now_node);
            }
            // 文本格式?
            else if (!::strcmp(now_node.name(), "Font") ||
                !::strcmp(now_node.name(), "TextFormat")) {
                m_aNodes[Type_TextFormat] = now_node;
                m_aResourceCount[this->Type_TextFormat] = get_children_count(now_node);
            }
            // 图元?
            else if (!::strcmp(now_node.name(), "Meta")) {
                m_aNodes[Type_Meta] = now_node;
                m_aResourceCount[this->Type_Meta] = get_children_count(now_node);
            }
            // 动画图元?
            else if (!::strcmp(now_node.name(), "MetaEx")) {
                assert(!"unsupport yet");
            }
            // 推进
            now_node = now_node.next_sibling();
        }
    }
    // 获取位图
    auto LongUI::CUIResourceLoaderXML::get_bitmap(pugi::xml_node node) noexcept -> ID2D1Bitmap1* {
        assert(node && "node not found");
        // 获取路径
        const char* uri = node.attribute("res").value();
        assert(uri && *uri && "Error URI of Bitmap");
        // 从文件载入位图
        auto load_bitmap_from_file = [](
            LongUIRenderTarget *pRenderTarget,
            IWICImagingFactory *pIWICFactory,
            PCWSTR uri,
            UINT destinationWidth,
            UINT destinationHeight,
            ID2D1Bitmap1 **ppBitmap
            ) noexcept -> HRESULT {
            IWICBitmapDecoder *pDecoder = nullptr;
            IWICBitmapFrameDecode *pSource = nullptr;
            IWICStream *pStream = nullptr;
            IWICFormatConverter *pConverter = nullptr;
            IWICBitmapScaler *pScaler = nullptr;
            // 创建解码器
            register HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
                uri,
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                &pDecoder
                );
            // 获取第一帧
            if (SUCCEEDED(hr)) {
                hr = pDecoder->GetFrame(0, &pSource);
            }
            // 创建格式转换器
            if (SUCCEEDED(hr)) {
                hr = pIWICFactory->CreateFormatConverter(&pConverter);
            }
            // 尝试缩放
            if (SUCCEEDED(hr)) {
                if (destinationWidth != 0 || destinationHeight != 0) {
                    UINT originalWidth, originalHeight;
                    // 获取大小
                    hr = pSource->GetSize(&originalWidth, &originalHeight);
                    if (SUCCEEDED(hr)) {
                        // 设置基本分辨率
                        if (destinationWidth == 0) {
                            FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                            destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                        }
                        else if (destinationHeight == 0) {
                            FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                            destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                        }
                        // 创建缩放器
                        hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                        // 初始化
                        if (SUCCEEDED(hr)) {
                            hr = pScaler->Initialize(
                                pSource,
                                destinationWidth,
                                destinationHeight,
                                WICBitmapInterpolationModeCubic
                                );
                        }
                        if (SUCCEEDED(hr)) {
                            hr = pConverter->Initialize(
                                pScaler,
                                GUID_WICPixelFormat32bppPBGRA,
                                WICBitmapDitherTypeNone,
                                nullptr,
                                0.f,
                                WICBitmapPaletteTypeMedianCut
                                );
                        }
                    }
                }
                else {
                    // 直接初始化
                    hr = pConverter->Initialize(
                        pSource,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        nullptr,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                        );
                }
            }
#if 0
            // 读取位图数据
            if (SUCCEEDED(hr)) {
                hr = pRenderTarget->CreateBitmapFromWicBitmap(
                    pConverter,
                    nullptr,
                    ppBitmap
                    );
            }
#elif 0
            // 计算
            constexpr UINT basic_step = 4;
            pConverter->CopyPixels()
#else
            {
                ID2D1Bitmap1* tmp_bitmap = nullptr;
                ID2D1Bitmap1* tar_bitmap = nullptr;
                // 读取位图数据
                if (SUCCEEDED(hr)) {
                    hr = pRenderTarget->CreateBitmapFromWicBitmap(
                        pConverter,
                        nullptr,
                        &tmp_bitmap
                        );
                }
                // 创建位图
                if (SUCCEEDED(hr)) {
                    tmp_bitmap->GetOptions();
                    hr = pRenderTarget->CreateBitmap(
                        tmp_bitmap->GetPixelSize(),
                        nullptr, 0,
                        D2D1::BitmapProperties1(
                            tmp_bitmap->GetOptions(),
                            tmp_bitmap->GetPixelFormat()
                            ),
                        &tar_bitmap
                        );
                }
                // 复制数据
                if (SUCCEEDED(hr)) {
                    hr = tar_bitmap->CopyFromBitmap(nullptr, tmp_bitmap, nullptr);
                }
                // 嫁接
                if (SUCCEEDED(hr)) {
                    *ppBitmap = tar_bitmap;
                    tar_bitmap = nullptr;
                }
                ::SafeRelease(tmp_bitmap);
                ::SafeRelease(tar_bitmap);
            }
#endif
            ::SafeRelease(pDecoder);
            ::SafeRelease(pSource);
            ::SafeRelease(pStream);
            ::SafeRelease(pConverter);
            ::SafeRelease(pScaler);
            return hr;
        };
        ID2D1Bitmap1* bitmap = nullptr;
        // 转换路径
        wchar_t path_buffer[LongUIStringBufferLength];
        path_buffer[LongUI::UTF8toWideChar(uri, path_buffer)] = 0;
        // 载入
        auto hr = load_bitmap_from_file(
            m_manager, m_pWICFactory, path_buffer, 0u, 0u, &bitmap
            );
        // 失败?
        if (FAILED(hr)) {
            m_manager.ShowError(hr);
        }
        return bitmap;
    }
    // 获取笔刷
    auto LongUI::CUIResourceLoaderXML::get_brush(pugi::xml_node node) noexcept -> ID2D1Brush* {
        union {
            ID2D1SolidColorBrush*       scb;
            ID2D1LinearGradientBrush*   lgb;
            ID2D1RadialGradientBrush*   rgb;
            ID2D1BitmapBrush1*          b1b;
            ID2D1Brush*                 brush;
        };
        brush = nullptr; const char* str = nullptr;
        assert(node && "bad argument");
        // 笔刷属性
        D2D1_BRUSH_PROPERTIES brush_prop = D2D1::BrushProperties();
        if (str = node.attribute("opacity").value()) {
            brush_prop.opacity = static_cast<float>(::LongUI::AtoF(str));
        }
        if (str = node.attribute("transform").value()) {
            Helper::MakeFloats(str, &brush_prop.transform._11, 6);
        }
        // 检查类型
        auto type = BrushType::Type_SolidColor;
        if (str = node.attribute("type").value()) {
            type = static_cast<decltype(type)>(::LongUI::AtoI(str));
        }
        switch (type)
        {
        case LongUI::BrushType::Type_SolidColor:
        {
            D2D1_COLOR_F color;
            // 获取颜色
            if (!Helper::MakeColor(node.attribute("color").value(), color)) {
                color = D2D1::ColorF(D2D1::ColorF::Black);
            }
            static_cast<LongUIRenderTarget*>(m_manager)->CreateSolidColorBrush(&color, &brush_prop, &scb);
        }
        break;
        case LongUI::BrushType::Type_LinearGradient:
            __fallthrough;
        case LongUI::BrushType::Type_RadialGradient:
            if (str = node.attribute("stops").value()) {
                // 语法 [pos0, color0] [pos1, color1] ....
                uint32_t stop_count = 0;
                ID2D1GradientStopCollection * collection = nullptr;
                D2D1_GRADIENT_STOP stops[LongUIMaxGradientStop];
                D2D1_GRADIENT_STOP* now_stop = stops;
                // 缓冲池
                char buffer[LongUIStringBufferLength];
                // 复制到缓冲区
                std::strcpy(buffer, str);
                char* index = buffer;
                const char* paragraph = nullptr;
                register char ch = 0;
                bool ispos = false;
                // 遍历检查
                while (ch = *index) {
                    // 查找第一个浮点数做为位置
                    if (ispos) {
                        // ,表示位置段结束, 该解析了
                        if (ch == ',') {
                            *index = 0;
                            now_stop->position = LongUI::AtoF(paragraph);
                            ispos = false;
                            paragraph = index + 1;
                        }
                    }
                    // 查找后面的数值做为颜色
                    else {
                        // [ 做为位置段标识开始
                        if (ch == '[') {
                            paragraph = index + 1;
                            ispos = true;
                        }
                        // ] 做为颜色段标识结束 该解析了
                        else if (ch == ']') {
                            *index = 0;
                            Helper::MakeColor(paragraph, now_stop->color);
                            ++now_stop;
                            ++stop_count;
                        }
                    }
                }
                // 创建StopCollection
                static_cast<LongUIRenderTarget*>(m_manager)->CreateGradientStopCollection(stops, stop_count, &collection);
                if (collection) {
                    // 线性渐变?
                    if (type == LongUI::BrushType::Type_LinearGradient) {
                        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES lgbprop = {
                            { 0.f, 0.f },{ 0.f, 0.f }
                        };
                        // 检查属性
                        Helper::MakeFloats(node.attribute("start").value(), &lgbprop.startPoint.x, 2);
                        Helper::MakeFloats(node.attribute("end").value(), &lgbprop.endPoint.x, 2);
                        // 创建笔刷
                        static_cast<LongUIRenderTarget*>(m_manager)->CreateLinearGradientBrush(
                            &lgbprop, &brush_prop, collection, &lgb
                            );
                    }
                    // 径向渐变笔刷
                    else {
                        D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES rgbprop = {
                            { 0.f, 0.f },{ 0.f, 0.f }, 0.f, 0.f
                        };
                        // 检查属性
                        Helper::MakeFloats(node.attribute("center").value(), &rgbprop.center.x, 2);
                        Helper::MakeFloats(node.attribute("offset").value(), &rgbprop.gradientOriginOffset.x, 2);
                        Helper::MakeFloats(node.attribute("rx").value(), &rgbprop.radiusX, 1);
                        Helper::MakeFloats(node.attribute("ry").value(), &rgbprop.radiusY, 1);
                        // 创建笔刷
                        static_cast<LongUIRenderTarget*>(m_manager)->CreateRadialGradientBrush(
                            &rgbprop, &brush_prop, collection, &rgb
                            );
                    }
                    collection->Release();
                    collection = nullptr;
                }
            }
            break;
        case LongUI::BrushType::Type_Bitmap:
            // 有效数据
            if (str = node.attribute("bitmap").value()) {
                // 创建笔刷
                auto bitmap = m_manager.GetBitmap(size_t(LongUI::AtoI(str)));
                // 基本参数
                D2D1_BITMAP_BRUSH_PROPERTIES1 bbprop = {
                    Helper::XMLGetD2DExtendMode(node, D2D1_EXTEND_MODE_CLAMP, "extendx"),
                    Helper::XMLGetD2DExtendMode(node, D2D1_EXTEND_MODE_CLAMP, "extendy"),
                    Helper::XMLGetD2DInterpolationMode(node, D2D1_INTERPOLATION_MODE_LINEAR, "interpolation"),
                };
                // 创建位图笔刷
                static_cast<LongUIRenderTarget*>(m_manager)->CreateBitmapBrush(
                    bitmap, &bbprop, &brush_prop, &b1b
                    );
                ::SafeRelease(bitmap);
            }
            break;
        }
        assert(brush && "unknown error but error");
        return brush;
    }
    // get textformat
    auto LongUI::CUIResourceLoaderXML::get_text_format(pugi::xml_node node) noexcept -> IDWriteTextFormat* {
        const char* str = nullptr;
        assert(node && "node not found");
        CUIString fontfamilyname(L"Arial");
        DWRITE_FONT_WEIGHT fontweight = DWRITE_FONT_WEIGHT_NORMAL;
        float fontsize = 12.f;
        // 获取字体名称
        Helper::MakeString(node.attribute("family").value(), fontfamilyname);
        // 获取字体大小
        if (str = node.attribute("size").value()) {
            fontsize = LongUI::AtoF(str);
        }
        // 获取字体粗细
        if (str = node.attribute("weight").value()) {
            fontweight = static_cast<DWRITE_FONT_WEIGHT>(LongUI::AtoI(str));
        }
        // 创建基本字体
        IDWriteTextFormat* textformat = nullptr;
        m_manager.CreateTextFormat(
            fontfamilyname.c_str(),
            fontweight,
            Helper::XMLGetFontStyle(node, DWRITE_FONT_STYLE_NORMAL),
            Helper::XMLGetFontStretch(node, DWRITE_FONT_STRETCH_NORMAL),
            fontsize,
            &textformat
            );
        // 成功获取则再设置
        if (textformat) {
            // Tab宽度
            float tabstop = fontsize * 4.f;
            // 检查Tab宽度
            if (str = node.attribute("tabstop").value()) {
                tabstop = LongUI::AtoF(str);
            }
            // 设置段落排列方向
            textformat->SetFlowDirection(
                Helper::XMLGetFlowDirection(node, DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM)
                );
            // 设置Tab宽度
            textformat->SetIncrementalTabStop(tabstop);
            // 设置段落(垂直)对齐
            textformat->SetParagraphAlignment(
                Helper::XMLGetVAlignment(node, DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
                );
            // 设置文本(水平)对齐
            textformat->SetTextAlignment(
                Helper::XMLGetHAlignment(node, DWRITE_TEXT_ALIGNMENT_LEADING)
                );
            // 设置阅读进行方向
            textformat->SetReadingDirection(
                Helper::XMLGetReadingDirection(node, DWRITE_READING_DIRECTION_LEFT_TO_RIGHT)
                );
            // 设置自动换行
            textformat->SetWordWrapping(
                Helper::XMLGetWordWrapping(node, DWRITE_WORD_WRAPPING_NO_WRAP)
                );
        }
        return textformat;
    }
}
#endif

                   

#if defined(_DEBUG)  && 1
#define TRACE_FUCTION UIManager << DL_Log << L"Trace: called" << LongUI::endl
#else
#define TRACE_FUCTION
#endif


// UI富文本编辑框: Render 渲染 
void LongUI::UIRichEdit::Render(RenderType type) const noexcept {
    /*HRESULT hr = S_OK;
    RECT draw_rect = { 0, 0, 100, 100 }; //AdjustRectT(LONG);
    if (m_pTextServices) {
        hr = m_pTextServices->TxDrawD2D(
            UIManager_RenderTarget,
            reinterpret_cast<RECTL*>(&draw_rect),
            nullptr,
            TXTVIEW_ACTIVE
            );
    }
    // 刻画光标
    if (SUCCEEDED(hr) && m_unused[Unused_ShowCaret]) {
        D2D1_RECT_F caretRect = {
            m_ptCaret.x, m_ptCaret.y,
            m_ptCaret.x + m_sizeCaret.width,m_ptCaret.y + m_sizeCaret.height
        };
        UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        UIManager_RenderTarget->FillRectangle(caretRect, m_pFontBrush);
        UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }
    return S_OK;*/
    return Super::Render(type);
}

// UI富文本编辑框: Render 刷新
void LongUI::UIRichEdit::Update() noexcept {

}

// UIRichEdit 构造函数
inline LongUI::UIRichEdit::UIRichEdit(pugi::xml_node node) noexcept: Super(node){ }

// UIRichEdit 析构函数
LongUI::UIRichEdit::~UIRichEdit() noexcept {
    ::SafeRelease(m_pFontBrush);
    if (m_pTextServices) {
        m_pTextServices->OnTxInPlaceDeactivate();
    }
    // 关闭服务
    UIRichEdit::ShutdownTextServices(m_pTextServices);
    //::SafeRelease(m_pTextServices);
}

// UIRichEdit::CreateControl 函数
LongUI::UIControl* LongUI::UIRichEdit::CreateControl(CreateEventType type, pugi::xml_node node) noexcept {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIRichEdit>(
            node,
            [=](void* p) noexcept { new(p) UIRichEdit(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UIRichEdit::DoEvent(const LongUI::EventArgument& arg) noexcept {
    if (arg.sender) {
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl: // 查找本空间
            if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
                arg.ctrl = this;
            }
            return true;*/
        case LongUI::Event::Event_MouseEnter:
            m_pWindow->now_cursor = m_hCursorI;
            break;
        case LongUI::Event::Event_MouseLeave:
            m_pWindow->now_cursor = m_pWindow->default_cursor;
            break;
        case LongUI::Event::Event_SetFocus:
            if (m_pTextServices) {
                m_pTextServices->OnTxUIActivate();
                m_pTextServices->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
            }
            return true;
        case LongUI::Event::Event_KillFocus:
            if (m_pTextServices) {
                m_pTextServices->OnTxUIDeactivate();
                m_pTextServices->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
                this->TxShowCaret(FALSE);
            }
            return true;
        }
    }
    // 处理系统消息
    else if(m_pTextServices) {
        // 检查
        if (m_pTextServices->TxSendMessage(arg.msg, arg.sys.wParam, arg.sys.lParam, &arg.lr) != S_FALSE) {
            // 已经处理了
            return true;
        }
    }
    return false;
}

// recreate 重建
HRESULT LongUI::UIRichEdit::Recreate() noexcept {
    HRESULT hr = S_OK;
    if (m_pTextServices) {
        m_pTextServices->OnTxInPlaceDeactivate();
    }
    ::SafeRelease(m_pTextServices);
    ::SafeRelease(m_pFontBrush);
    // 设置新的笔刷
    m_pFontBrush = UIManager.GetBrush(LongUIDefaultTextFormatIndex);
    // 获取窗口句柄
    m_hwnd = m_pWindow->GetHwnd();
    IUnknown* pUk = nullptr;
    hr = UIRichEdit::CreateTextServices(nullptr, this, &pUk);
    // 创建文本服务
    if (SUCCEEDED(hr)) {
        hr = pUk->QueryInterface(
            *UIRichEdit::IID_ITextServices2, reinterpret_cast<void**>(&m_pTextServices)
            );
    }
    if (SUCCEEDED(hr)) {
        hr = m_pTextServices->TxSetText(L"Hello, World!");
    }
    // 就地激活富文本控件
    if (SUCCEEDED(hr)) {
        hr = m_pTextServices->OnTxInPlaceActivate(nullptr);
    }
    ::SafeRelease(pUk);
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UIRichEdit::Cleanup() noexcept {
    delete this;
}

// ----- ITextHost
// ITextHost::TxGetDC 实现: 获取DC
HDC LongUI::UIRichEdit::TxGetDC() {
    // 不支持!!
    assert(!"- GDI MODE - Not Supported");
    TRACE_FUCTION;
    return nullptr;
}

// ITextHost::TxReleaseDC 实现: 释放DC
INT LongUI::UIRichEdit::TxReleaseDC(HDC hdc){
    UNREFERENCED_PARAMETER(hdc);
    assert(!"- GDI MODE - Not Supported");
    TRACE_FUCTION;
    return 0;
}

// ITextHost::TxShowScrollBar 实现: 显示滚动条
BOOL LongUI::UIRichEdit::TxShowScrollBar(INT fnBar, BOOL fShow){
    UNREFERENCED_PARAMETER(fnBar);
    UNREFERENCED_PARAMETER(fShow);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags){
    UNREFERENCED_PARAMETER(fuSBFlags);
    UNREFERENCED_PARAMETER(fuArrowflags);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw){
    UNREFERENCED_PARAMETER(fnBar);
    UNREFERENCED_PARAMETER(nMinPos);
    UNREFERENCED_PARAMETER(nMaxPos);
    UNREFERENCED_PARAMETER(fRedraw);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw){
    UNREFERENCED_PARAMETER(fnBar);
    UNREFERENCED_PARAMETER(nPos);
    UNREFERENCED_PARAMETER(fRedraw);
    TRACE_FUCTION;
    return FALSE;
}

void LongUI::UIRichEdit::TxInvalidateRect(LPCRECT prc, BOOL fMode){
    UNREFERENCED_PARAMETER(prc);
    UNREFERENCED_PARAMETER(fMode);
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxViewChange(BOOL fUpdate){
    UNREFERENCED_PARAMETER(fUpdate);
    TRACE_FUCTION;
    //if (fUpdate) {
        m_pWindow->Invalidate(this);
    //}
}

// ITextHost::TxCreateCaret 实现:创建光标
BOOL LongUI::UIRichEdit::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight){
    UNREFERENCED_PARAMETER(hbmp);
    TRACE_FUCTION;
    m_sizeCaret = {static_cast<float>(xWidth), static_cast<float>(yHeight) };
    // 创建傀儡
    ::DestroyCaret();
    ::CreateCaret(m_hwnd, nullptr, xWidth, yHeight);
    return TRUE;
}

// ITextHost::TxShowCaret 实现:显示/隐藏光标
BOOL LongUI::UIRichEdit::TxShowCaret(BOOL fShow){
    UNREFERENCED_PARAMETER(fShow);
    TRACE_FUCTION;
    //m_unused[Unused_ShowCaret] = (fShow != 0);
    return TRUE;
}

BOOL LongUI::UIRichEdit::TxSetCaretPos(INT _x, INT _y){
    TRACE_FUCTION;
    m_ptCaret = {static_cast<float>(_x), static_cast<float>(_y) };
    m_pWindow->Invalidate(this);
    ::SetCaretPos(_x, _y);
    return TRUE;
}

BOOL LongUI::UIRichEdit::TxSetTimer(UINT idTimer, UINT uTimeout){
    ::SetTimer(m_hwnd, idTimer, uTimeout, nullptr);
    TRACE_FUCTION;
    return FALSE;
}

void LongUI::UIRichEdit::TxKillTimer(UINT idTimer){
    ::KillTimer(m_hwnd, idTimer);
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll,
    LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll){
    UNREFERENCED_PARAMETER(dx);
    UNREFERENCED_PARAMETER(dy);
    UNREFERENCED_PARAMETER(lprcScroll);
    UNREFERENCED_PARAMETER(lprcClip);
    UNREFERENCED_PARAMETER(hrgnUpdate);
    UNREFERENCED_PARAMETER(lprcUpdate);
    UNREFERENCED_PARAMETER(fuScroll);
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxSetCapture(BOOL fCapture) {
    TRACE_FUCTION;
    fCapture ? void(::SetCapture(m_hwnd)) : void(::ReleaseCapture());
}

void LongUI::UIRichEdit::TxSetFocus(){
    TRACE_FUCTION;
}

void LongUI::UIRichEdit::TxSetCursor(HCURSOR hcur, BOOL fText){
    UNREFERENCED_PARAMETER(hcur);
    UNREFERENCED_PARAMETER(fText);
    TRACE_FUCTION;
}

BOOL LongUI::UIRichEdit::TxScreenToClient(LPPOINT lppt){
    UNREFERENCED_PARAMETER(lppt);
    TRACE_FUCTION;
    return FALSE;
}

BOOL LongUI::UIRichEdit::TxClientToScreen(LPPOINT lppt){
    UNREFERENCED_PARAMETER(lppt);
    TRACE_FUCTION;
    return FALSE;
}

HRESULT LongUI::UIRichEdit::TxActivate(LONG* plOldState){
    UNREFERENCED_PARAMETER(plOldState);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::TxDeactivate(LONG lNewState){
    UNREFERENCED_PARAMETER(lNewState);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::TxGetClientRect(LPRECT prc){
    //TRACE_FUCTION;
    *prc = { 0, 0, 100, 100 };
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetViewInset(LPRECT prc){
    // Set zero sized margins
    *prc = { 0, 0, 0, 0 };
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetCharFormat(const CHARFORMATW **ppCF){
    UNREFERENCED_PARAMETER(ppCF);
    /*METHOD_PROLOGUE(CRichDrawText, TextHost)

        // Return the default character format set up in the constructor
        *ppCF = &(pThis->m_CharFormat);*/
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetParaFormat(const PARAFORMAT **ppPF){
    UNREFERENCED_PARAMETER(ppPF);
    /* METHOD_PROLOGUE(CRichDrawText, TextHost)

        // Return the default paragraph format set up in the constructor
        *ppPF = &(pThis->m_ParaFormat);*/
    TRACE_FUCTION;
    return S_OK;
}

COLORREF LongUI::UIRichEdit::TxGetSysColor(int nIndex){
    // Pass requests for colours on to Windows
    TRACE_FUCTION;
    return ::GetSysColor(nIndex);
}

HRESULT LongUI::UIRichEdit::TxGetBackStyle(TXTBACKSTYLE *pstyle){
    // Do not erase what is underneath the drawing area
    *pstyle = TXTBACK_TRANSPARENT;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetMaxLength(DWORD *plength){
    // Set the maximum size of text to be arbitrarily large
    *plength = 1024 * 1024 * 16;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetScrollBars(DWORD *pdwScrollBar){
    *pdwScrollBar = 0;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetPasswordChar(TCHAR *pch) {
    *pch = L'*';
    TRACE_FUCTION;
    return S_FALSE;
}

HRESULT LongUI::UIRichEdit::TxGetAcceleratorPos(LONG *pcp){
    *pcp = -1;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxGetExtent(LPSIZEL lpExtent){
    UNREFERENCED_PARAMETER(lpExtent);
    TRACE_FUCTION;
    return E_NOTIMPL;
}

HRESULT LongUI::UIRichEdit::OnTxCharFormatChange(const CHARFORMATW * pcf){
    UNREFERENCED_PARAMETER(pcf);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::OnTxParaFormatChange(const PARAFORMAT * ppf){
    UNREFERENCED_PARAMETER(ppf);
    TRACE_FUCTION;
    return E_FAIL;
}

HRESULT LongUI::UIRichEdit::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits){
    // Set the windowless control as being multiple lines of wrapping rich text
    DWORD bits = TXTBIT_MULTILINE | TXTBIT_RICHTEXT | TXTBIT_WORDWRAP | TXTBIT_D2DDWRITE;
    *pdwBits = bits & dwMask;
    TRACE_FUCTION;
    return S_OK;
}

HRESULT LongUI::UIRichEdit::TxNotify(DWORD iNotify, void *pv){
    // Claim to have handled the notifcation, even though we always ignore it
    UNREFERENCED_PARAMETER(iNotify);
    UNREFERENCED_PARAMETER(pv);
    TRACE_FUCTION;
    return S_OK;
}

HIMC LongUI::UIRichEdit::TxImmGetContext(){
    TRACE_FUCTION;
    return 0;
}

void LongUI::UIRichEdit::TxImmReleaseContext(HIMC himc){
    UNREFERENCED_PARAMETER(himc);
    TRACE_FUCTION;
}

HRESULT LongUI::UIRichEdit::TxGetSelectionBarWidth(LONG *lSelBarWidth){
    UNREFERENCED_PARAMETER(lSelBarWidth);
    // No selection bar
    *lSelBarWidth = 0;
    TRACE_FUCTION;
    return S_OK;
}

// ----- ITextHost2
/*#undef TRACE_FUCTION
#define TRACE_FUCTION (L"Trace:<%S> called\n", __FUNCTION__)*/

//@cmember Is a double click in the message queue?
BOOL    LongUI::UIRichEdit::TxIsDoubleClickPending() {
    TRACE_FUCTION;
    return FALSE;
}

//@cmember Get the overall window for this control     
HRESULT LongUI::UIRichEdit::TxGetWindow(HWND *phwnd) {
    *phwnd = m_hwnd;
    TRACE_FUCTION;
    return S_OK;
}

//@cmember Set controlwindow to foreground
HRESULT LongUI::UIRichEdit::TxSetForegroundWindow() {
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Set control window to foreground
HPALETTE LongUI::UIRichEdit::TxGetPalette() {
    TRACE_FUCTION;
    return nullptr;
}

//@cmember Get East Asian flags
HRESULT LongUI::UIRichEdit::TxGetEastAsianFlags(LONG *pFlags) {
    *pFlags = ES_NOIME;
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Routes the cursor change to the winhost
HCURSOR LongUI::UIRichEdit::TxSetCursor2(HCURSOR hcur, BOOL bText) {
    UNREFERENCED_PARAMETER(hcur);
    UNREFERENCED_PARAMETER(bText);
    TRACE_FUCTION;
    return hcur;
}

//@cmember Notification that text services is freed
void    LongUI::UIRichEdit::TxFreeTextServicesNotification() {

    TRACE_FUCTION;
}

//@cmember Get Edit Style flags
HRESULT LongUI::UIRichEdit::TxGetEditStyle(DWORD dwItem, DWORD *pdwData) {
    UNREFERENCED_PARAMETER(dwItem);
    UNREFERENCED_PARAMETER(pdwData);
    TRACE_FUCTION;
    return FALSE;
}

//@cmember Get Window Style bits
HRESULT LongUI::UIRichEdit::TxGetWindowStyles(DWORD *pdwStyle, DWORD *pdwExStyle) {
    *pdwStyle = WS_OVERLAPPEDWINDOW;
    *pdwExStyle = 0;
    TRACE_FUCTION;
    return S_OK;
}

//@cmember Show / hide drop caret (D2D-only)
HRESULT LongUI::UIRichEdit::TxShowDropCaret(BOOL fShow, HDC hdc, LPCRECT prc) {
    UNREFERENCED_PARAMETER(fShow);
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(prc);
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Destroy caret (D2D-only)
HRESULT LongUI::UIRichEdit::TxDestroyCaret() {
    TRACE_FUCTION;
    return S_FALSE;
}

//@cmember Get Horizontal scroll extent
HRESULT LongUI::UIRichEdit::TxGetHorzExtent(LONG *plHorzExtent) {
    UNREFERENCED_PARAMETER(plHorzExtent);
    TRACE_FUCTION;
    return S_FALSE;
}

/*
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetScrollBars> called
Trace:<TxGetScrollBars> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetScrollBars> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxShowCaret> called
Trace:<TxCreateCaret> called
Trace:<TxSetCaretPos> called
Trace:<TxShowCaret> called
Trace:<TxInvalidateRect> called
Trace:<TxInvalidateRect> called
Trace:<TxViewChange> called
Trace:<TxShowCaret> called
Trace:<TxGetScrollBars> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxShowCaret> called
Trace:<TxCreateCaret> called
Trace:<TxSetCaretPos> called
Trace:<TxShowCaret> called
Trace:<TxGetWindow> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxGetViewInset> called
Trace:<TxGetSelectionBarWidth> called
Trace:<TxGetWindowStyles> called
Trace:<TxNotify> called
Trace:<TxGetSysColor> called
Trace:<TxGetSysColor> called
Trace:<TxDestroyCaret> called
Trace:<TxNotify> called
Trace:<TxShowCaret> called
*/
                   
                    

// 获取相对数值
#define UISB_OffsetVaule(f) ((&(f))[int(this->bartype)])


// UIScrollBar 构造函数
LongUI::UIScrollBar::UIScrollBar(pugi::xml_node node) noexcept: 
Super(node), m_uiAnimation(AnimationType::Type_QuadraticEaseIn) {
    // 修改
    m_uiAnimation.duration = 0.5f;
    if (node) {
        wheel_step = LongUI::AtoF(node.attribute("wheelstep").value());
        m_uiAnimation.duration = LongUI::AtoF(node.attribute("aniamtionduration").value());
        register const char* str = nullptr;
        if ((str = node.attribute("aniamtionduration").value())) {
            m_uiAnimation.duration = LongUI::AtoF(str);;
        }
        if ((str = node.attribute("aniamtionbartype").value())) {
            m_uiAnimation.type = static_cast<AnimationType>(LongUI::AtoI(str));
        }
    }
    m_uiAnimation.start = m_uiAnimation.end = m_uiAnimation.value = 0.f;
}


// 设置新的索引位置
void LongUI::UIScrollBar::SetIndex(float new_index) noexcept {
    //return this->set_index(new_index);
    // 阈值检查
    new_index = std::min(std::max(new_index, 0.f), m_fMaxIndex);
    m_uiAnimation.start = m_uiAnimation.value = m_fIndex;
    m_uiAnimation.end = new_index;
    m_uiAnimation.time = m_uiAnimation.duration;
    m_pWindow->StartRender(m_uiAnimation.time, this->parent);
    m_bAnimation = true;
}

// 设置新的索引位置
void LongUI::UIScrollBar::set_index(float new_index) noexcept {
    //UIManager << DL_Hint << "new_index: " << new_index << endl;
    new_index = std::min(std::max(new_index, 0.f), m_fMaxIndex);
    // 不同就修改
    if (new_index != m_fIndex) {
        m_fIndex = new_index;
        // 修改父类属性
        this->parent->SetOffsetZoomed(int(this->bartype), -new_index);
        // 刷新拥有着
        m_pWindow->Invalidate(this->parent);
    }
}

// do event 事件处理
bool LongUI::UIScrollBar::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // 控件消息
    if (arg.sender) {
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl:
            if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
                arg.ctrl = this;
            }
            __fallthrough;*/
            /*case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        case LongUI::Event::Event_MouseEnter:
            (L"<%S>: MouseEnter\n", __FUNCTION__);
            break;*/
            break;
        case LongUI::Event::Event_MouseLeave:
            m_pointType = PointType::Type_None;
            m_lastPointType = PointType::Type_None;
            return true;
        }
    }
    // 系统消息
    else {
        // 鼠标移上

    }
    return false;
}

/// <summary>
/// Updates the width of the marginal.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollBar::UpdateMarginalWidth() noexcept {
    // 水平
    if (this->bartype == ScrollBarType::Type_Horizontal) {
        m_fMaxRange = this->parent->GetContentWidthZoomed();
        m_fMaxIndex = m_fMaxRange - this->parent->GetViewWidthZoomed();
    }
    // 垂直
    else {
        m_fMaxRange = this->parent->GetContentHeightZoomed();
        m_fMaxIndex = m_fMaxRange - this->parent->GetViewHeightZoomed();
    }
    // 限制
    m_fMaxRange = std::max(m_fMaxRange, 0.f);
    m_fMaxIndex = std::max(m_fMaxIndex, 0.f);
    if (m_fIndex > m_fMaxIndex) {
        this->set_index(m_fMaxIndex);
    }
    return Super::UpdateMarginalWidth();
}

/// <summary>
/// Updates the width of the marginal.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollBarA::UpdateMarginalWidth() noexcept {
    // 加强父类方法
    Super::UpdateMarginalWidth();
    // 需要?
#ifdef _DEBUG
    if (this->debug_this && m_fMaxIndex > 0.f) {
        UIManager << DL_Log  << this << "m_fMaxIndex: "
            << m_fMaxIndex << " -- scrollbar standby?"<< endl;
    }
#endif
#if 0
    if (m_fMaxIndex < BASIC_SIZE * 0.5f) {
#else
    if (m_fMaxIndex < 0.5f) {
#endif
        this->marginal_width = 0.f;
        this->visible = false;
    }
    else {
        this->marginal_width = BASIC_SIZE;
        this->visible = true;
    }
}



// UIScrollBarA 构造函数
LongUI::UIScrollBarA::UIScrollBarA(pugi::xml_node node) noexcept: Super(node), 
m_uiArrow1(node, "arrow1"), m_uiArrow2(node, "arrow2"), m_uiThumb(node, "thumb"){
    // 修改颜色
    if (node) {
        m_fArrowStep = LongUI::AtoF(node.attribute("arrowstep").value());
    }
    // 修改颜色
    else {
        D2D1_COLOR_F normal_color = D2D1::ColorF(0xF0F0F0);
        m_uiArrow1.GetByType<Element_ColorRect>().colors[Status_Normal] = normal_color;
        m_uiArrow2.GetByType<Element_ColorRect>().colors[Status_Normal] = normal_color;
        normal_color = D2D1::ColorF(0x2F2F2F);
        m_uiArrow1.GetByType<Element_ColorRect>().colors[Status_Pushed] = normal_color;
        m_uiArrow2.GetByType<Element_ColorRect>().colors[Status_Pushed] = normal_color;

    }
    // 初始化代码
    m_uiArrow1.GetByType<Element_Basic>().Init(node, "arrow1");
    m_uiArrow2.GetByType<Element_Basic>().Init(node, "arrow2");
    m_uiThumb.GetByType<Element_Basic>().Init(node, "thumb");
    // 检查
    BarElement* elements[] = { &m_uiArrow1, &m_uiArrow2, &m_uiThumb };
    for (auto element : elements) {
        if (element->GetByType<Element_Meta>().IsOK()) {
            element->SetElementType(Element_Meta);
        }
        else {
            element->SetElementType(Element_ColorRect);
        }
        element->GetByType<Element_Basic>().SetNewStatus(Status_Normal);
        element->GetByType<Element_Basic>().SetNewStatus(Status_Normal);
    }
    // 检查属性
    m_bArrow1InColor = m_uiArrow1.GetByType<Element_Basic>().type == Element_ColorRect;
    m_bArrow2InColor = m_uiArrow2.GetByType<Element_Basic>().type == Element_ColorRect;

}

// UI滚动条(类型A): 刷新
void LongUI::UIScrollBarA::Update() noexcept {
    // 索引不一致?
#ifdef _DEBUG
    auto offset = -(this->bartype == ScrollBarType::Type_Horizontal ?
        this->parent->GetOffsetXZoomed() : this->parent->GetOffsetYZoomed());
    if (std::abs(m_fIndex - offset) > 0.5f) {
        m_fIndex = offset;
        UIManager << DL_Hint << "diffence with offset(I: " 
            << m_fIndex << " O: " << offset 
            << " ), set new index" << LongUI::endl;
    }
#else
    m_fIndex = -(this->bartype == ScrollBarType::Type_Horizontal ?
        this->parent->GetOffsetXZoomed() : this->parent->GetOffsetYZoomed());
#endif
    // 先刷新父类
    D2D1_RECT_F draw_rect; this->GetViewRect(draw_rect);
    // 双滚动条修正
    m_rtThumb = m_rtArrow2 = m_rtArrow1 = draw_rect;
    register float length_of_thumb, start_offset;
    {
        register float tmpsize = UISB_OffsetVaule(this->view_size.width) - BASIC_SIZE*2.f;
        start_offset = tmpsize * m_fIndex / m_fMaxRange;
        length_of_thumb = tmpsize * (1.f - m_fMaxIndex / m_fMaxRange);
    }
    // 这段有点长, 使用UISB_OffsetVaule效率可能反而不如if
    // 垂直滚动条
    if (this->bartype == ScrollBarType::Type_Vertical) {
        m_rtArrow1.bottom = m_rtArrow1.top + BASIC_SIZE;
        m_rtArrow2.top = m_rtArrow2.bottom - BASIC_SIZE;
        m_rtThumb.top = m_rtArrow1.bottom + start_offset;
        m_rtThumb.bottom = m_rtThumb.top + length_of_thumb;
    }
    // 水平滚动条
    else {
        m_rtArrow1.right = m_rtArrow1.left + BASIC_SIZE;
        m_rtArrow2.left = m_rtArrow2.right - BASIC_SIZE;
        m_rtThumb.left = m_rtArrow1.right + start_offset;
        m_rtThumb.right = m_rtThumb.left + length_of_thumb;
    }
    // 刷新
    m_uiArrow1.Update();
    m_uiArrow2.Update();
    m_uiThumb.Update();
    // 刷新
    if (m_bAnimation) {
        m_uiAnimation.Update();
        this->set_index(m_uiAnimation.value);
        if (m_uiAnimation.time <= 0.f) {
            m_bAnimation = false;
        }
    }
    // 刷新
    return Super::Update();
}

// UIScrollBarA 渲染 
void LongUI::UIScrollBarA::Render(RenderType _bartype) const noexcept  {
    if (_bartype != RenderType::Type_Render) return;
    // 更新
    D2D1_RECT_F draw_rect; this->GetViewRect(draw_rect);
    // 双滚动条修正
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(0xF0F0F0));
    UIManager_RenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
    //
    //this->parent;
    //UIManager << DL_Hint << m_rtArrow2 << endl;

    // 渲染部件
    m_uiArrow1.Render(m_rtArrow1);
    m_uiArrow2.Render(m_rtArrow2);
    m_uiThumb.Render(m_rtThumb);
    // 前景
    auto render_geo = [](ID2D1RenderTarget* const target, ID2D1Brush* const bush,
        ID2D1Geometry* const geo, const D2D1_RECT_F& rect) noexcept {
        D2D1_MATRIX_3X2_F matrix; target->GetTransform(&matrix);
        target->SetTransform(
            D2D1::Matrix3x2F::Translation(rect.left, rect.top) * matrix
            );
        target->DrawGeometry(geo, bush, 2.33f);
        // 修改
        target->SetTransform(&matrix);
    };
    // 渲染几何体
    if (m_bArrow1InColor) {
        D2D1_COLOR_F tcolor = m_uiArrow1.GetByType<Element_ColorRect>().colors[
            m_uiArrow1.GetByType<Element_Basic>().GetStatus()
        ];
        tcolor.r = 1.f - tcolor.r; tcolor.g = 1.f - tcolor.g; tcolor.b = 1.f - tcolor.b;
        m_pBrush_SetBeforeUse->SetColor(&tcolor);
        render_geo(UIManager_RenderTarget, m_pBrush_SetBeforeUse, m_pArrow1Geo, m_rtArrow1);
    }
    // 渲染几何体
    if (m_bArrow2InColor) {
        D2D1_COLOR_F tcolor = m_uiArrow2.GetByType<Element_ColorRect>().colors[
            m_uiArrow2.GetByType<Element_Basic>().GetStatus()
        ];
        tcolor.r = 1.f - tcolor.r; tcolor.g = 1.f - tcolor.g; tcolor.b = 1.f - tcolor.b;
        m_pBrush_SetBeforeUse->SetColor(&tcolor);
        render_geo(UIManager_RenderTarget, m_pBrush_SetBeforeUse, m_pArrow2Geo, m_rtArrow2);
    }
    // 前景
    Super::Render(RenderType::Type_RenderForeground);
}


// UIScrollBarA::do event 事件处理
bool  LongUI::UIScrollBarA::DoEvent(const LongUI::EventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // 控件消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_MouseLeave:
            this->set_status(m_lastPointType, LongUI::Status_Normal);
            break;
        }
    }
    // 系统消息
    else {
        switch (arg.msg) {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            m_bCaptured = true; 
            this->set_status(m_pointType, LongUI::Status_Pushed);
            switch (m_pointType)
            {
            case LongUI::UIScrollBar::PointType::Type_Arrow1:
                // 左/上移动
                this->SetIndex(m_uiAnimation.end - m_fArrowStep);
                break;
            case LongUI::UIScrollBar::PointType::Type_Arrow2:
                // 右/下移动
                this->SetIndex(m_uiAnimation.end + m_fArrowStep);
                break;
            case LongUI::UIScrollBar::PointType::Type_Thumb:
                m_fOldPoint = UISB_OffsetVaule(pt4self.x);
                m_fOldIndex = m_fIndex;
                break;
            case LongUI::UIScrollBar::PointType::Type_Shaft:
                // 设置目标
            {
                auto rate = UISB_OffsetVaule(pt4self.x) / (this->view_size.width - BASIC_SIZE * 2.f);
                this->SetIndex(rate * m_fMaxIndex);
            }
                break;
            default:
                break;
            }
            break;
        case WM_LBUTTONUP:
            this->set_status(m_pointType, LongUI::Status_Hover);
            m_bCaptured = false;
            m_pWindow->ReleaseCapture();
            break;
        case WM_MOUSEMOVE:
            // Captured状态
            if (m_bCaptured) {
                // 指向thumb?
                if (m_pointType == PointType::Type_Thumb) {
                    // 计算移动距离
                    register auto pos = UISB_OffsetVaule(pt4self.x);
                    register auto rate = (1.f - m_fMaxIndex  / (m_fMaxRange - BASIC_SIZE)) 
                        * this->parent->GetZoom(int(this->bartype));
                    //UIManager << DL_Hint << rate << endl;
                    this->set_index((pos - m_fOldPoint) / rate + m_fOldIndex);
                    m_uiAnimation.end = m_fIndex;
#ifdef _DEBUG
                    rate = 0.f;
#endif
                }
            }
            //  检查指向类型
            else {
                if (IsPointInRect(m_rtArrow1, pt4self)) {
                    m_pointType = PointType::Type_Arrow1;
                }
                else if (IsPointInRect(m_rtArrow2, pt4self)) {
                    m_pointType = PointType::Type_Arrow2;
                }
                else if (IsPointInRect(m_rtThumb, pt4self)) {
                    m_pointType = PointType::Type_Thumb;
                }
                else {
                    m_pointType = PointType::Type_Shaft;
                }
                // 修改
                if (m_lastPointType != m_pointType) {
                    this->set_status(m_lastPointType, LongUI::Status_Normal);
                    this->set_status(m_pointType, LongUI::Status_Hover);
                    m_lastPointType = m_pointType;
                }
            }
            return true;
        }
    }
    return Super::DoEvent(arg);
}

// UIScrollBarA:: 重建
auto LongUI::UIScrollBarA::Recreate() noexcept -> HRESULT {
    m_uiArrow1.Recreate();
    m_uiArrow2.Recreate();
    m_uiThumb.Recreate();
    return Super::Recreate();
}

// UIScrollBarA: 初始化时
void LongUI::UIScrollBarA::InitMarginalControl(MarginalControl _type) noexcept {
    // 初始化
    Super::InitMarginalControl(_type);
    // 创建几何
    if (this->bartype == ScrollBarType::Type_Horizontal) {
        m_pArrow1Geo = ::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Left]);
        m_pArrow2Geo = ::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Right]);
    }
    // 垂直滚动条
    else {
        m_pArrow1Geo = ::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Top]);
        m_pArrow2Geo = ::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Bottom]);
    }
    assert(m_pArrow1Geo && m_pArrow2Geo);
}


// UIScrollBarA 析构函数
inline LongUI::UIScrollBarA::~UIScrollBarA() noexcept {
    ::SafeRelease(m_pArrow1Geo);
    ::SafeRelease(m_pArrow2Geo);
}

// UIScrollBarA 关闭控件
void  LongUI::UIScrollBarA::Cleanup() noexcept {
    delete this;
}


// 设置状态
void LongUI::UIScrollBarA::set_status(PointType _bartype, ControlStatus state) noexcept {
    BarElement* elements[] = { &m_uiArrow1, &m_uiArrow2, &m_uiThumb };
    // 检查
    if (_bartype >= PointType::Type_Arrow1 && _bartype <= PointType::Type_Thumb) {
        auto index = static_cast<uint32_t>(_bartype) - static_cast<uint32_t>(PointType::Type_Arrow1);
        auto& element = *(elements[index]);
        UIElement_SetNewStatus(element, state);
    }
}

// 静态变量
ID2D1PathGeometry* LongUI::UIScrollBarA::
s_apArrowPathGeometry[LongUI::UIScrollBarA::ARROW_SIZE] = { nullptr };

// create 创建
auto WINAPI LongUI::UIScrollBarA::CreateControl(CreateEventType bartype, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (bartype)
    {
    case Type_CreateControl:
        // 获取模板节点
        if (!node) {
            // SB允许无节点创建
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIScrollBarA>(
            node,
            [=](void* p) noexcept { new(p) UIScrollBarA(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
    {
        // 创建设备无关资源
        auto create_geo = [](D2D1_POINT_2F* list, uint32_t length) {
            auto hr = S_OK;
            ID2D1PathGeometry* geometry = nullptr;
            ID2D1GeometrySink* sink = nullptr;
            // 创建几何体
            if (SUCCEEDED(hr)) {
                hr = UIManager_D2DFactory->CreatePathGeometry(&geometry);
            }
            // 打开
            if (SUCCEEDED(hr)) {
                hr = geometry->Open(&sink);
            }
            // 开始绘制
            if (SUCCEEDED(hr)) {
                sink->BeginFigure(list[0], D2D1_FIGURE_BEGIN_HOLLOW);
                sink->AddLines(list + 1, length - 1);
                sink->EndFigure(D2D1_FIGURE_END_OPEN);
                hr = sink->Close();
            }
            AssertHR(hr);
            ::SafeRelease(sink);
            return geometry;
        };
        D2D1_POINT_2F point_list[3];
        constexpr float BASIC_SIZE_MID = BASIC_SIZE * 0.5f;
        constexpr float BASIC_SIZE_NEAR = BASIC_SIZE_MID * 0.5f;
        constexpr float BASIC_SIZE_FAR = BASIC_SIZE - BASIC_SIZE_NEAR;
        // LEFT 左箭头
        {
            point_list[0] = { BASIC_SIZE_MID , BASIC_SIZE_NEAR };
            point_list[1] = { BASIC_SIZE_NEAR , BASIC_SIZE_MID };
            point_list[2] = { BASIC_SIZE_MID , BASIC_SIZE_FAR };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Left]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Left] = create_geo(point_list, lengthof(point_list));
        }
        // TOP 上箭头
        {
            point_list[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
            point_list[1] = { BASIC_SIZE_MID, BASIC_SIZE_NEAR };
            point_list[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Top]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Top] = create_geo(point_list, lengthof(point_list));
        }
        // RIGHT 右箭头
        {

            point_list[0] = { BASIC_SIZE_MID , BASIC_SIZE_NEAR };
            point_list[1] = { BASIC_SIZE_FAR , BASIC_SIZE_MID };
            point_list[2] = { BASIC_SIZE_MID , BASIC_SIZE_FAR };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Right]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Right] = create_geo(point_list, lengthof(point_list));
        }
        // BOTTOM 下箭头
        {
            point_list[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
            point_list[1] = { BASIC_SIZE_MID, BASIC_SIZE_FAR };
            point_list[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Bottom]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Bottom] = create_geo(point_list, lengthof(point_list));
        }
    }
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        // 释放资源
        for (auto& geo : s_apArrowPathGeometry) {
            ::SafeRelease(geo);
        }
        break;
    }
    return pControl;
}


// UIScrollBarB 构造函数
LongUI::UIScrollBarB::UIScrollBarB(pugi::xml_node node) noexcept: Super(node) {

}

// UIScrollBarB 创建函数
auto WINAPI LongUI::UIScrollBarB::CreateControl(CreateEventType bartype, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (bartype)
    {
    case Type_CreateControl:
        // 获取模板节点
        if (!node) {

        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UIScrollBarB>(
            node,
            [=](void* p) noexcept { new(p) UIScrollBarB(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}

// UIScrollBarB 关闭控件
void  LongUI::UIScrollBarB::Cleanup() noexcept {
    delete this;
}

                   


// Render 渲染 
void LongUI::UISlider::Render(RenderType type) const noexcept {
    //D2D1_RECT_F draw_rect;
    switch (type)
    {
    case LongUI::RenderType::Type_RenderBackground:
        __fallthrough;
    case LongUI::RenderType::Type_Render:
        // 默认背景?
        if(m_bDefaultBK) {
            constexpr float SLIDER_TRACK_BORDER_WIDTH = 1.f;
            constexpr float SLIDER_TRACK_WIDTH = 3.f;
            constexpr UINT SLIDER_TRACK_BORDER_COLOR = 0xD6D6D6;
            constexpr UINT SLIDER_TRACK_COLOR = 0xE7EAEA;
            D2D1_RECT_F border_rect; this->GetViewRect(border_rect);
            // 垂直滑块
            if (this->IsVerticalSlider()) {
                auto half = this->thumb_size.height * 0.5f;
                border_rect.left = (border_rect.left + border_rect.right) * 0.5f -
                    SLIDER_TRACK_BORDER_WIDTH - SLIDER_TRACK_WIDTH * 0.5f;
                border_rect.right = border_rect.left + 
                    SLIDER_TRACK_BORDER_WIDTH * 2.f + SLIDER_TRACK_WIDTH;
                border_rect.top += half;
                border_rect.bottom -= half;
            }
            // 水平滑块
            else {
                auto half = this->thumb_size.width * 0.5f;
                border_rect.left += half;
                border_rect.right -= half;
                border_rect.top = (border_rect.top + border_rect.bottom) * 0.5f -
                    SLIDER_TRACK_BORDER_WIDTH - SLIDER_TRACK_WIDTH * 0.5f;
                border_rect.bottom = border_rect.top + 
                    SLIDER_TRACK_BORDER_WIDTH * 2.f + SLIDER_TRACK_WIDTH;
            }
            // 渲染滑槽边框
            m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(SLIDER_TRACK_BORDER_COLOR));
            UIManager_RenderTarget->FillRectangle(&border_rect, m_pBrush_SetBeforeUse);
            // 渲染滑槽
            m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(SLIDER_TRACK_COLOR));
            border_rect.left += SLIDER_TRACK_BORDER_WIDTH;
            border_rect.top += SLIDER_TRACK_BORDER_WIDTH;
            border_rect.right -= SLIDER_TRACK_BORDER_WIDTH;
            border_rect.bottom -= SLIDER_TRACK_BORDER_WIDTH;
            UIManager_RenderTarget->FillRectangle(&border_rect, m_pBrush_SetBeforeUse);
        }
        // 父类背景
        Super::Render(LongUI::RenderType::Type_RenderBackground);
        __fallthrough;
        // 背景中断
        if (type == LongUI::RenderType::Type_RenderBackground) {
            break;
        }
    case LongUI::RenderType::Type_RenderForeground:
        m_uiElement.Render(m_rcThumb);
        // 边框
        {
            constexpr float THUMB_BORDER_WIDTH = 1.f;
            D2D1_RECT_F thumb_border = {
                m_rcThumb.left + THUMB_BORDER_WIDTH * 0.5f,
                m_rcThumb.top + THUMB_BORDER_WIDTH * 0.5f,
                m_rcThumb.right - THUMB_BORDER_WIDTH * 0.5f,
                m_rcThumb.bottom - THUMB_BORDER_WIDTH * 0.5f,
            };
            m_pBrush_SetBeforeUse->SetColor(&m_colorBorderNow);
            UIManager_RenderTarget->DrawRectangle(&thumb_border, m_pBrush_SetBeforeUse);
        }
        // 父类前景
        Super::Render(LongUI::RenderType::Type_RenderForeground);
        break;
    case LongUI::RenderType::Type_RenderOffScreen:
        break;
    }
}


// UI滑动条: 刷新
void LongUI::UISlider::Update() noexcept {
    // 更新计时器
    m_uiElement.Update();
    // 垂直滑块
    if (this->IsVerticalSlider()) {
        // 根据 value 计算滑块位置
        m_rcThumb.left = (this->view_size.width - this->thumb_size.width) *0.5f;
        m_rcThumb.right = m_rcThumb.left + this->thumb_size.width;
        {
            auto slider_height = this->view_size.height - this->thumb_size.height;
            m_rcThumb.top = slider_height * m_fValue;
        }
        m_rcThumb.bottom = m_rcThumb.top + this->thumb_size.height;
    }
    // 水平滑块
    else {
        // 根据 value 计算滑块位置
        m_rcThumb.top = (this->view_size.height - this->thumb_size.height) *0.5f;
        m_rcThumb.bottom = m_rcThumb.top + this->thumb_size.height;
        {
            auto slider_width = this->view_size.width - this->thumb_size.width;
            m_rcThumb.left = slider_width * m_fValue;
        }
        m_rcThumb.right = m_rcThumb.left + this->thumb_size.width;
    }
}

// UISlider 构造函数
LongUI::UISlider::UISlider(pugi::xml_node node) noexcept: Super(node), m_uiElement(node) {
    // 设置
    if (node) {
        const char* str = nullptr;
        // 起始值
        if ((str = node.attribute("start").value())) {
            m_fStart = LongUI::AtoF(str);
        }
        // 终止值
        if ((str = node.attribute("end").value())) {
            m_fEnd = LongUI::AtoF(str);
        }
        // 滑块大小
        Helper::MakeFloats(
            node.attribute("thumbsize").value(),
            &force_cast(thumb_size.width), 
            sizeof(thumb_size)/sizeof(thumb_size.width)
            );
        // 默认背景
        m_bDefaultBK = node.attribute("defaultbk").as_bool(true);
    }
    // 初始化代码
    m_uiElement.GetByType<Element_Basic>().Init(node);
    if (m_uiElement.GetByType<Element_Meta>().IsOK()) {
        m_uiElement.SetElementType(Element_Meta);
    }
    else {
        m_uiElement.SetElementType(Element_BrushRect);
    }
    // init
    m_uiElement.GetByType<Element_Basic>().SetNewStatus(Status_Normal);
    // need twices because of aniamtion
    m_uiElement.GetByType<Element_Basic>().SetNewStatus(Status_Normal);
}

// UISlider::CreateControl 函数
LongUI::UIControl* LongUI::UISlider::CreateControl(CreateEventType type, pugi::xml_node node) noexcept {
    UIControl* pControl = nullptr;
    switch (type)
    {
    case Type_CreateControl:
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        pControl = LongUI::UIControl::AllocRealControl<LongUI::UISlider>(
            node,
            [=](void* p) noexcept { new(p) UISlider(node); }
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        break;
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UISlider::DoEvent(const LongUI::EventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    //--------------------------------------------------
    if (arg.sender){
        switch (arg.event)
        {
        /*case LongUI::Event::Event_FindControl:
            if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(pt4self.x < this->width && pt4self.y < this->width && "check it");
                arg.ctrl = this;
            }
            __fallthrough;*/
        case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        case LongUI::Event::Event_MouseEnter:
            break;
        case LongUI::Event::Event_MouseLeave:
            // 鼠标移出: 设置UI元素状态
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Normal);
            m_bMouseClickIn = false;
            m_bMouseMoveIn = false;
            break;
        }
    }
    else {
        switch (arg.msg)
        {
        case WM_LBUTTONDOWN:
            m_pWindow->SetCapture(this);
            if (IsPointInRect(m_rcThumb, pt4self)){
                m_bMouseClickIn = true;
                m_fClickPosition = this->IsVerticalSlider() ?
                    (pt4self.y - m_rcThumb.top) : (pt4self.x - m_rcThumb.left);
                UIElement_SetNewStatus(m_uiElement, LongUI::Status_Pushed);
            }
            break;
        case WM_MOUSEMOVE:
            // 点中并且移动
            if (arg.sys.wParam & MK_LBUTTON) {
                if (m_bMouseClickIn) {
                    // 获取基本值
                    if (this->IsVerticalSlider()) {
                        auto slider_height = this->view_size.height - this->thumb_size.height;
                        m_fValue = (pt4self.y - m_fClickPosition) / slider_height;
                    }
                    else {
                        auto slider_width = this->view_size.width - this->thumb_size.width;
                        m_fValue = (pt4self.x - m_fClickPosition) / slider_width;
                    }
                    // 阈值检查
                    if (m_fValue > 1.f) m_fValue = 1.f;
                    else if (m_fValue < 0.f) m_fValue = 0.f;
                }
            }
            // 移动
            else {
                if (IsPointInRect(m_rcThumb, pt4self)){
                    // 鼠标移进:
                    if (!m_bMouseMoveIn) {
                        // 设置UI元素状态
                        UIElement_SetNewStatus(m_uiElement, LongUI::Status_Hover);
                        m_bMouseMoveIn = true;
                    }
                }
                else {
                    // 鼠标移出:
                    if (m_bMouseMoveIn) {
                        // 设置UI元素状态
                        UIElement_SetNewStatus(m_uiElement, LongUI::Status_Normal);
                        m_bMouseMoveIn = false;
                    }
                }
            }
            break;
        case WM_LBUTTONUP:
            m_bMouseClickIn = false;
            m_pWindow->ReleaseCapture();
            UIElement_SetNewStatus(m_uiElement, LongUI::Status_Hover);
            break;
        }
        // 检查事件
        if (m_fValueOld != m_fValue) {
            m_fValueOld = m_fValue;
            // 调用
            m_caller(this, SubEvent::Event_SliderValueChanged);
            // 刷新
            m_pWindow->Invalidate(this);
        }
    }
    return false;
}

// recreate 重建
auto LongUI::UISlider::Recreate() noexcept ->HRESULT {
    m_uiElement.Recreate();
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UISlider::Cleanup() noexcept {
    delete this;
}
                   
                    


/// <summary>
/// float4 color ---> 32-bit ARGB uint color
/// 将浮点颜色转换成32位ARGB排列整型
/// </summary>
/// <param name="color">The d2d color</param>
/// <returns>32-bit ARGB 颜色</returns>
auto __fastcall LongUI::PackTheColorARGB(D2D1_COLOR_F& IN color) noexcept -> uint32_t {
    constexpr uint32_t ALPHA_SHIFT = 24;
    constexpr uint32_t RED_SHIFT = 16;
    constexpr uint32_t GREEN_SHIFT = 8;
    constexpr uint32_t BLUE_SHIFT = 0;

    register uint32_t colorargb =
        ((uint32_t(color.a * 255.f) & 0xFF) << ALPHA_SHIFT) |
        ((uint32_t(color.r * 255.f) & 0xFF) << RED_SHIFT) |
        ((uint32_t(color.g * 255.f) & 0xFF) << GREEN_SHIFT) |
        ((uint32_t(color.b * 255.f) & 0xFF) << BLUE_SHIFT);
    return colorargb;
}

/// <summary>
/// 32-bit ARGB uint color ---> float4 color
/// 将32位ARGB排列整型转换成浮点颜色
/// </summary>
/// <param name="color32">The 32-bit color.</param>
/// <param name="color4f">The float4 color.</param>
/// <returns>void</returns>
auto __fastcall LongUI::UnpackTheColorARGB(uint32_t IN color32, D2D1_COLOR_F& OUT color4f) noexcept->void {
    // 位移量
    constexpr uint32_t ALPHA_SHIFT = 24;
    constexpr uint32_t RED_SHIFT = 16;
    constexpr uint32_t GREEN_SHIFT = 8;
    constexpr uint32_t BLUE_SHIFT = 0;
    // 掩码
    constexpr uint32_t ALPHA_MASK = 0xFFU << ALPHA_SHIFT;
    constexpr uint32_t RED_MASK = 0xFFU << RED_SHIFT;
    constexpr uint32_t GREEN_MASK = 0xFFU << GREEN_SHIFT;
    constexpr uint32_t BLUE_MASK = 0xFFU << BLUE_SHIFT;
    // 计算
    color4f.r = static_cast<float>((color32 & RED_MASK) >> RED_SHIFT) / 255.f;
    color4f.g = static_cast<float>((color32 & GREEN_MASK) >> GREEN_SHIFT) / 255.f;
    color4f.b = static_cast<float>((color32 & BLUE_MASK) >> BLUE_SHIFT) / 255.f;
    color4f.a = static_cast<float>((color32 & ALPHA_MASK) >> ALPHA_SHIFT) / 255.f;
}



// Meta 渲染
/// <summary>
/// render the meta 渲染Meta
/// </summary>
/// <param name="meta">The meta.</param>
/// <param name="target">The render target.</param>
/// <param name="des_rect">The des_rect.</param>
/// <param name="opacity">The opacity.</param>
/// <returns></returns>
void __fastcall LongUI::Meta_Render(
    const Meta& meta, LongUIRenderTarget* target,
    const D2D1_RECT_F& des_rect, float opacity) noexcept {
    // 无效位图
    if (!meta.bitmap) {
        UIManager << DL_Warning << "bitmap->null" << LongUI::endl;
        return;
    }
    switch (meta.rule)
    {
    case LongUI::BitmapRenderRule::Rule_Scale:
        __fallthrough;
    default:
    case LongUI::BitmapRenderRule::Rule_ButtonLike:
        // 直接缩放:
        target->DrawBitmap(
            meta.bitmap,
            des_rect, opacity,
            static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
            meta.src_rect,
            nullptr
            );
        break;
    /*case LongUI::BitmapRenderRule::Rule_ButtonLike:
        // 类按钮
        // - 原矩形, 宽 > 高 ->
        // - 原矩形, 宽 < 高 ->
    {
        constexpr float MARKER = 0.25f;
        auto width = meta.src_rect.right - meta.src_rect.left;
        auto bilibili = width * MARKER / (meta.src_rect.bottom - meta.src_rect.top) *
            (des_rect.bottom - des_rect.top);
        D2D1_RECT_F des_rects[3]; D2D1_RECT_F src_rects[3]; D2D1_RECT_F clip_rects[3];
        // ---------------------------------------
        des_rects[0] = {
            des_rect.left, des_rect.top,
            des_rect.left + bilibili, des_rect.bottom
        };
        des_rects[1] = {
            des_rects[0].right, des_rect.top,
            des_rect.right - bilibili, des_rect.bottom
        };
        des_rects[2] = {
            des_rects[1].right, des_rect.top,
            des_rect.right, des_rect.bottom
        };
        // ---------------------------------------
        ::memcpy(clip_rects, des_rects, sizeof(des_rects));
        if (clip_rects[1].left > des_rects[1].right) {
            std::swap(clip_rects[1].right, des_rects[1].left);
            std::swap(des_rects[1].right, des_rects[1].left);
            clip_rects[0].right = des_rects[1].left;
            clip_rects[2].left = des_rects[1].right;
        }
        // ---------------------------------------
        src_rects[0] = {
            meta.src_rect.left, meta.src_rect.top,
            meta.src_rect.left + width * MARKER, meta.src_rect.bottom
        };
        src_rects[1] = {
            src_rects[0].right, meta.src_rect.top,
            meta.src_rect.right - width * MARKER, meta.src_rect.bottom
        };
        src_rects[2] = {
            src_rects[1].right, meta.src_rect.top,
            meta.src_rect.right, meta.src_rect.bottom
        };
        // 正式渲染
        for (auto i = 0u; i < lengthof(src_rects); ++i) {
            target->PushAxisAlignedClip(clip_rects + i, D2D1_ANTIALIAS_MODE_ALIASED);
            target->DrawBitmap(
                meta.bitmap,
                des_rects[i], opacity,
                static_cast<D2D1_INTERPOLATION_MODE>(meta.interpolation),
                src_rects[i],
                nullptr
                );
            target->PopAxisAlignedClip();
        }
    }
    break;*/
    }
}

// CUISubEventCaller () operator
LongUINoinline bool LongUI::CUISubEventCaller::operator()(UIControl* sender, SubEvent subevent) noexcept {
    assert(sender && subevent != LongUI::SubEvent::Event_Null && "bad arguments");
    // 事件
    LongUI::EventArgument arg;
    arg.event = LongUI::Event::Event_SubEvent;
    arg.sender = sender;
    arg.ui.subevent = subevent;
    arg.ui.pointer = nullptr;
    arg.pt = { 0.f, 0.f };
    arg.ctrl = nullptr;
    // 脚本优先
    if (UIManager.script && sender->GetScript().script) {
        return UIManager.script->Evaluation(sender->GetScript(), arg);
    }
    // 回调其次
    if (m_pCallback) {
        return m_pCallback(m_pRecver, sender);
    }
    // 事件最低
    return sender->GetWindow()->DoEvent(arg);
}


// 构造对象
LongUI::CUIDataObject* LongUI::CUIDataObject::New() noexcept {
    auto* pointer = reinterpret_cast<LongUI::CUIDataObject*>(LongUI::SmallAlloc(sizeof(LongUI::CUIDataObject)));
    if (pointer) {
        pointer->CUIDataObject::CUIDataObject();
    }
    return pointer;
}



// CUIDataObject 构造函数
LongUI::CUIDataObject::CUIDataObject() noexcept {
    ZeroMemory(&m_dataStorage, sizeof(m_dataStorage));
}


// CUIDataObject 析构函数
LongUI::CUIDataObject::~CUIDataObject() noexcept {
    // 释放数据
    if (m_dataStorage.formatEtc.cfFormat) {
        ::ReleaseStgMedium(&m_dataStorage.stgMedium);
    }
}


// 设置Unicode
HRESULT LongUI::CUIDataObject::SetUnicodeText(HGLOBAL hGlobal) noexcept {
    assert(hGlobal && "hGlobal -> null");
    // 释放数据
    if (m_dataStorage.formatEtc.cfFormat) {
        ::ReleaseStgMedium(&m_dataStorage.stgMedium);
    }
    m_dataStorage.formatEtc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    m_dataStorage.stgMedium = { TYMED_HGLOBAL,{ 0 }, 0 };
    m_dataStorage.stgMedium.hGlobal = hGlobal;
    return S_OK;
}

// 设置Unicode字符
HRESULT LongUI::CUIDataObject::SetUnicodeText(const wchar_t* str, size_t length) noexcept {
    HRESULT hr = S_OK;
    if (!length) length = ::wcslen(str);
    // 全局
    HGLOBAL hGlobal = nullptr;
    // 申请成功
    auto size = sizeof(wchar_t) * (length + 1);
    if (hGlobal = ::GlobalAlloc(GMEM_FIXED, size)) {
        LPVOID pdest = ::GlobalLock(hGlobal);
        if (pdest) {
            ::memcpy(pdest, str, size);
        }
        else {
            hr = E_FAIL;
        }
        ::GlobalUnlock(hGlobal);
    }
    else {
        hr = E_OUTOFMEMORY;
    }
    // 设置数据
    if (SUCCEEDED(hr)) {
        hr = this->SetUnicodeText(hGlobal);
    }
    return hr;
}


// IDataObject::GetData 实现: 
HRESULT LongUI::CUIDataObject::GetData(FORMATETC * pFormatetcIn, STGMEDIUM * pMedium) noexcept {
    // 参数检查
    if (!pFormatetcIn || !pMedium) return E_INVALIDARG;
    // 
    pMedium->hGlobal = nullptr;
    // 检查数据
    if (m_dataStorage.formatEtc.cfFormat) {
        // 返回需要获取的格式
        if ((pFormatetcIn->tymed & m_dataStorage.formatEtc.tymed) &&
            (pFormatetcIn->dwAspect == m_dataStorage.formatEtc.dwAspect) &&
            (pFormatetcIn->cfFormat == m_dataStorage.formatEtc.cfFormat))
        {
            return this->CopyMedium(pMedium, &m_dataStorage.stgMedium, &m_dataStorage.formatEtc);
        }
    }
    return DV_E_FORMATETC;
}

// IDataObject::GetDataHere 实现
HRESULT LongUI::CUIDataObject::GetDataHere(FORMATETC * pFormatetcIn, STGMEDIUM * pMedium) noexcept {
    UNREFERENCED_PARAMETER(pFormatetcIn);
    UNREFERENCED_PARAMETER(pMedium);
    return E_NOTIMPL;
}

// IDataObject::QueryGetData 实现: 查询支持格式数据
HRESULT LongUI::CUIDataObject::QueryGetData(FORMATETC * pFormatetc) noexcept {
    // 检查参数
    if (!pFormatetc) return E_INVALIDARG;
    // 
    if (!(DVASPECT_CONTENT & pFormatetc->dwAspect)) {
        return DV_E_DVASPECT;
    }
    HRESULT hr = DV_E_TYMED;
    // 遍历数据
    if (m_dataStorage.formatEtc.cfFormat && m_dataStorage.formatEtc.tymed & pFormatetc->tymed) {
        if (m_dataStorage.formatEtc.cfFormat == pFormatetc->cfFormat) {
            hr = S_OK;
        }
        else {
            hr = DV_E_CLIPFORMAT;
        }
    }
    else {
        hr = DV_E_TYMED;
    }
    return hr;
}

// IDataObject::GetCanonicalFormatEtc 实现
HRESULT LongUI::CUIDataObject::GetCanonicalFormatEtc(FORMATETC * pFormatetcIn, FORMATETC * pFormatetcOut) noexcept {
    UNREFERENCED_PARAMETER(pFormatetcIn);
    UNREFERENCED_PARAMETER(pFormatetcOut);
    return E_NOTIMPL;
}

// IDataObject::SetData 实现
HRESULT LongUI::CUIDataObject::SetData(FORMATETC * pFormatetc, STGMEDIUM * pMedium, BOOL fRelease) noexcept {
    // 检查参数
    if (!pFormatetc || !pMedium) return E_INVALIDARG;
    UNREFERENCED_PARAMETER(fRelease);
    return E_NOTIMPL;
}

// IDataObject::EnumFormatEtc 实现: 枚举支持格式
HRESULT LongUI::CUIDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC ** ppEnumFormatEtc) noexcept {
    // 检查参数
    if (!ppEnumFormatEtc) return E_INVALIDARG;
    *ppEnumFormatEtc = nullptr;
    HRESULT hr = E_NOTIMPL;
    if (DATADIR_GET == dwDirection) {
        // 设置支持格式
        // 暂时仅支持 Unicode字符(UTF-16 on WindowsDO)
        static FORMATETC rgfmtetc[] = {
            { CF_UNICODETEXT, nullptr, DVASPECT_CONTENT, 0, TYMED_HGLOBAL },
        };
        hr = ::SHCreateStdEnumFmtEtc(lengthof(rgfmtetc), rgfmtetc, ppEnumFormatEtc);
    }
    return hr;
}

// IDataObject::DAdvise 实现
HRESULT LongUI::CUIDataObject::DAdvise(FORMATETC * pFormatetc, DWORD advf,
    IAdviseSink * pAdvSnk, DWORD * pdwConnection) noexcept {
    UNREFERENCED_PARAMETER(pFormatetc);
    UNREFERENCED_PARAMETER(advf);
    UNREFERENCED_PARAMETER(pAdvSnk);
    UNREFERENCED_PARAMETER(pdwConnection);
    return E_NOTIMPL;
}

// IDataObject::DUnadvise 实现
HRESULT LongUI::CUIDataObject::DUnadvise(DWORD dwConnection) noexcept {
    UNREFERENCED_PARAMETER(dwConnection);
    return E_NOTIMPL;
}

// IDataObject::EnumDAdvise 实现
HRESULT LongUI::CUIDataObject::EnumDAdvise(IEnumSTATDATA ** ppenumAdvise) noexcept {
    UNREFERENCED_PARAMETER(ppenumAdvise);
    return E_NOTIMPL;
}

// 复制媒体数据
HRESULT LongUI::CUIDataObject::CopyMedium(STGMEDIUM * pMedDest, STGMEDIUM * pMedSrc, FORMATETC * pFmtSrc) noexcept {
    // 检查参数
    if (!pMedDest || !pMedSrc || !pFmtSrc) return E_INVALIDARG;
    // 按类型处理
    switch (pMedSrc->tymed)
    {
    case TYMED_HGLOBAL:
        pMedDest->hGlobal = (HGLOBAL)OleDuplicateData(pMedSrc->hGlobal, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_GDI:
        pMedDest->hBitmap = (HBITMAP)OleDuplicateData(pMedSrc->hBitmap, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_MFPICT:
        pMedDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData(pMedSrc->hMetaFilePict, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_ENHMF:
        pMedDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData(pMedSrc->hEnhMetaFile, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_FILE:
        pMedSrc->lpszFileName = (LPOLESTR)OleDuplicateData(pMedSrc->lpszFileName, pFmtSrc->cfFormat, 0);
        break;
    case TYMED_ISTREAM:
        pMedDest->pstm = pMedSrc->pstm;
        pMedSrc->pstm->AddRef();
        break;
    case TYMED_ISTORAGE:
        pMedDest->pstg = pMedSrc->pstg;
        pMedSrc->pstg->AddRef();
        break;
    case TYMED_NULL:
        __fallthrough;
    default:
        break;
    }
    //
    pMedDest->tymed = pMedSrc->tymed;
    pMedDest->pUnkForRelease = nullptr;
    if (pMedSrc->pUnkForRelease) {
        pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;
        pMedSrc->pUnkForRelease->AddRef();
    }
    return S_OK;
}

// 设置Blob
HRESULT LongUI::CUIDataObject::SetBlob(CLIPFORMAT cf, const void * pvBlob, UINT cbBlob) noexcept {
    void *pv = GlobalAlloc(GPTR, cbBlob);
    HRESULT hr = pv ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr)) {
        CopyMemory(pv, pvBlob, cbBlob);
        FORMATETC fmte = { cf, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        // The STGMEDIUM structure is used to define how to handle a global memory transfer.  
        // This structure includes a flag, tymed, which indicates the medium  
        // to be used, and a union comprising pointers and a handle for getting whichever  
        // medium is specified in tymed.  
        STGMEDIUM medium = {};
        medium.tymed = TYMED_HGLOBAL;
        medium.hGlobal = pv;
        hr = this->SetData(&fmte, &medium, TRUE);
        if (FAILED(hr)) {
            ::GlobalFree(pv);
        }
    }
    return hr;
}



// ----------------------------------------------------------------------------------


// 构造对象
LongUI::CUIDropSource* LongUI::CUIDropSource::New() noexcept {
    auto* pointer = reinterpret_cast<LongUI::CUIDropSource*>(LongUI::SmallAlloc(sizeof(LongUI::CUIDropSource)));
    if (pointer) {
        pointer->CUIDropSource::CUIDropSource();
    }
    return pointer;
}

// 析构函数
LongUI::CUIDropSource::~CUIDropSource() noexcept {
};

// CUIDropSource::QueryContinueDrag 实现: 
HRESULT LongUI::CUIDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) noexcept {
    // Esc按下或者鼠标右键按下 : 取消拖拽
    if (fEscapePressed || (grfKeyState & MK_RBUTTON)) {
        return DRAGDROP_S_CANCEL;
    }
    // 鼠标左键弹起: 拖拽结束
    if (!(grfKeyState & MK_LBUTTON)) {
        return DRAGDROP_S_DROP;
    }
    return S_OK;
}

// CUIDropSource::GiveFeedback 实现
HRESULT LongUI::CUIDropSource::GiveFeedback(DWORD dwEffect) noexcept {
    UNREFERENCED_PARAMETER(dwEffect);
    return DRAGDROP_S_USEDEFAULTCURSORS;
}


// CUIConstString: 析构函数
LongUI::CUIConstString::~CUIConstString() {
    // 有效
    if (m_cLength) {
        // 计算
        const auto buffer_length = (m_cLength + 1) * sizeof(wchar_t);
        if (buffer_length < SMALL_THRESHOLD) {
            LongUI::SmallFree(m_pString);
        }
        else {
            LongUI::SmallFree(m_pString);
        }
    }
}

// UIConstString: 设置
void LongUI::CUIConstString::set(const wchar_t* str, size_t length) noexcept {
    assert(!m_pString && !m_cLength);
    // 未知则计算
    if (!length && *str) { length = static_cast<uint32_t>(::wcslen(str)); }
    // 有效
    if (length) {
        // 计算长度
        const auto buffer_length = (length + 1)*sizeof(wchar_t);
        if (buffer_length < SMALL_THRESHOLD) {
            m_pString = reinterpret_cast<wchar_t*>(LongUI::SmallAlloc(buffer_length));
        }
        else {
            m_pString = reinterpret_cast<wchar_t*>(LongUI::NormalAlloc(buffer_length));
        }
        // OK
        if (m_pString) {
            m_cLength = length;
        }
        // OOM
        else {
            m_pString = L"";
            m_cLength = 0;
        }
    }
    else {
        m_pString = L"";
        m_cLength = 0;
    }
}


// UIString 设置字符串
void LongUI::CUIString::Set(const wchar_t* str, uint32_t length) noexcept {
    assert(str && "bad argument");
    // 内存不足
    if (!this->m_pString) {
        this->m_pString = m_aDataStatic;
        m_cBufferLength = LongUIStringFixedLength;
        m_aDataStatic[0] = wchar_t(0);
    }
    // 未知则计算
    if (!length && *str) { length = static_cast<uint32_t>(::wcslen(str)); }
    // 超长的话
    if (length > m_cBufferLength) {
        m_cBufferLength = static_cast<uint32_t>(this->nice_buffer_length(length));
        // 尝试释放
        this->safe_free_bufer();
        // 申请内存
        m_pString = this->alloc_bufer(m_cBufferLength);
    }
    // 复制数据
    assert(str && "<bad");
    ::wcscpy(m_pString, str);
    m_cLength = length;
}

// UIString 设置字符串
void LongUI::CUIString::Set(const char* str, uint32_t len) noexcept {
    assert(str && "bad argument");
    // 固定缓存
    wchar_t buffer[LongUIStringBufferLength];
    // 动态缓存
    wchar_t* huge_buffer = nullptr;
    uint32_t buffer_length = LongUIStringBufferLength;
    // 内存不足
    if (!this->m_pString) {
        this->m_pString = m_aDataStatic;
        m_cBufferLength = LongUIStringFixedLength;
        m_aDataStatic[0] = wchar_t(0);
    }
    // 未知则计算
    if (!len && *str) { len = static_cast<uint32_t>(::strlen(str)); }
    // 假设全是英文字母, 超长的话
    if (len > LongUIStringBufferLength) {
        buffer_length = static_cast<uint32_t>(this->nice_buffer_length(len));
        huge_buffer = this->alloc_bufer(m_cBufferLength);
        // OOM ?
        if (!huge_buffer) return this->OnOOM();
    }
    {
        auto real_buffer = huge_buffer ? huge_buffer : buffer;
        auto length_got = LongUI::UTF8toWideChar(str, real_buffer);
        real_buffer[length_got] = 0;
        // 动态申请?
        if (huge_buffer) {
            this->safe_free_bufer();
            m_pString = huge_buffer;
            huge_buffer = nullptr;
            m_cLength = length_got;
            m_cBufferLength = buffer_length;
        }
        // 设置
        else {
            this->Set(real_buffer, length_got);
        }
    }
    // sad
    assert(m_pString);
}

// UIString 添加字符串
void LongUI::CUIString::Append(const wchar_t* str, uint32_t len) noexcept {
    assert(str && "bad argument");
    // 无需
    if (!(*str)) return;
    // 未知则计算
    if (!len) { len = static_cast<uint32_t>(::wcslen(str)); }
    // 超过缓存?
    const auto target_lenth = m_cLength + len + 1;
    if (target_lenth > m_cBufferLength) {
        m_cBufferLength = static_cast<uint32_t>(this->nice_buffer_length(target_lenth));
        // 申请内存
        auto alloced_buffer = this->alloc_bufer(m_cBufferLength);
        if (!alloced_buffer) {
            return this->OnOOM();
        }
        // 复制数据
        this->copy_string_ex(alloced_buffer, m_pString, m_cLength);
        this->copy_string(alloced_buffer + m_cLength, str, len);
        // 释放
        this->safe_free_bufer();
        m_pString = alloced_buffer;
    }
    // 继续使用旧缓存
    else {
        // 复制数据
        this->copy_string(m_pString + m_cLength, str, len);
    }
    // 添加长度
    m_cLength += len;
}


// 设置保留缓存
void LongUI::CUIString::Reserve(uint32_t len) noexcept {
    assert(len && "bad argument");
    // 小于等于就什么都不做
    if (len > m_cBufferLength) {
        // 换成偶数
        auto nice_length = len + (len & 1);
        // 申请空间
        auto buffer = this->alloc_bufer(nice_length);
        // OOM
        if (!buffer) {
            return this->OnOOM();
        }
        // 复制数据
        this->copy_string(buffer, m_pString, m_cLength);
        m_cBufferLength = nice_length;
        this->safe_free_bufer();
        m_pString = buffer;
    }
}

// 插入字符串
void LongUI::CUIString::Insert(uint32_t off, const wchar_t* str, uint32_t len) noexcept {
    assert(str && "bad argument");
    assert(off <= m_cLength && "out of range");
    // 插入尾巴
    if (off >= m_cLength) return this->Append(str, len);
    // 无需
    if (!(*str)) return;
    // 未知则计算
    if (!len) { len = static_cast<uint32_t>(::wcslen(str)); }
    // 需要申请内存?
    const auto target_lenth = m_cLength + len + 1;
    if (target_lenth > m_cBufferLength) {
        m_cBufferLength = static_cast<uint32_t>(this->nice_buffer_length(target_lenth));
        // 申请内存
        auto alloced_buffer = this->alloc_bufer(m_cBufferLength);
        if (!alloced_buffer) {
            return this->OnOOM();
        }
        // 复制数据
        this->copy_string_ex(alloced_buffer, m_pString, off);
        this->copy_string_ex(alloced_buffer + off, str, len);
        this->copy_string(alloced_buffer + off + len, m_pString, m_cLength - off);
        // 释放
        this->safe_free_bufer();
        m_pString = alloced_buffer;
    }
    // 继续使用旧缓存
    else {
        // memcpy:restrict 要求, 手动循环
        auto src_end = m_pString + m_cLength;
        auto des_end = src_end + len;
        for (uint32_t i = 0; i < (m_cLength - off + 1); ++i) {
            *des_end = *src_end;
            --des_end; --src_end;
        }
        // 复制数据
        this->copy_string_ex(m_pString + off, str, len);
    }
    // 添加长度
    m_cLength += len;
}


// UIString 字符串析构函数
LongUI::CUIString::~CUIString() noexcept {
    // 释放数据
    this->safe_free_bufer();
    m_cLength = 0;
}

// 复制构造函数
LongUI::CUIString::CUIString(const LongUI::CUIString& obj) noexcept {
    assert(obj.m_pString && "bad");
    if (!obj.m_pString) {
        this->OnOOM();
        return;
    }
    // 构造
    if (obj.m_pString != obj.m_aDataStatic) {
        m_pString = this->alloc_bufer(obj.m_cBufferLength);
        m_cBufferLength = obj.m_cBufferLength;
    }
    // 复制数据
    assert(m_pString && "out of memory");
    if (m_pString) {
        this->copy_string(m_pString, obj.m_pString, obj.m_cLength);
        m_cLength = obj.m_cLength;
    }
    // 内存不足
    else {
        this->OnOOM();
    }
}

// move构造函数
LongUI::CUIString::CUIString(LongUI::CUIString&& obj) noexcept {
    // 构造
    if (obj.m_pString != obj.m_aDataStatic) {
        m_pString = obj.m_pString;
    }
    else {
        // 复制数据
        this->copy_string(m_aDataStatic, obj.m_aDataStatic, obj.m_cLength);
    }
    m_cLength = obj.m_cLength;
    obj.m_cLength = 0;
    obj.m_pString = obj.m_aDataStatic;
    obj.m_aDataStatic[0] = wchar_t(0);
}

// 删除字符串
void LongUI::CUIString::Remove(uint32_t offset, uint32_t length) noexcept {
    assert(offset + length <= m_cLength && "out of range");
    // 有可能直接删除后面, 优化
    if (offset + length >= m_cLength) {
        m_cLength = std::min(m_cLength, offset);
        return;
    }
    // 将后面的字符串复制过来即可
    // memcpy:restrict 要求, 手动循环
    auto des = m_pString + offset;
    auto src = des + length;
    for (uint32_t i = 0; i < (m_cLength - offset - length + 1); ++i) {
        *des = *src;
        ++des; ++src;
    }
    m_cLength -= length;
}

// 格式化
void LongUI::CUIString::Format(const wchar_t* format, ...) noexcept {
    wchar_t buffer[LongUIStringBufferLength];
    buffer[0] = 0;
    va_list ap;
    va_start(ap, format);
    auto length = std::vswprintf(buffer, LongUIStringBufferLength, format, ap);
    // error
    if (length == -1) {
        UIManager << DL_Warning << "std::vswprintf return -1 for out of space" << endl;
        length = LongUIStringBufferLength - 1;
    }
    this->Set(buffer, length);
    va_end(ap);
}

// CUIString 内存不足
void LongUI::CUIString::OnOOM() noexcept {
    constexpr auto length = 13ui32;
    // 内存
    if (LongUIStringFixedLength > length) {
        this->Set(L"Out of Memory", length);
    }
    else if(LongUIStringFixedLength > 3) {
        this->Set(L"OOM", 3);
    }
    // 显示错误
    UIManager.ShowError(E_OUTOFMEMORY, L"<LongUI::CUIString::OnOOM()>");
}


// += 操作
//const LongUI::CUIString& LongUI::CUIString::operator+=(const wchar_t*);

// CUIAnimation ---------- BEGIN -------------

#define UIAnimation_Template_A      \
    register auto v = LongUI::EasingFunction(this->type, this->time / this->duration)
#define UIAnimation_Template_B(m)   \
    this->value.m = v * (this->start.m - this->end.m) + this->end.m;

// for D2D1_POINT_2F or Float1
template<> void LongUI::CUIAnimation<float>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    // 计算
    this->value = LongUI::EasingFunction(this->type, this->time / this->duration)
        * (this->start - this->end) + this->end;
    // 减少时间
    this->time -= t;
}


// for D2D1_POINT_2F or Float2
template<> void LongUI::CUIAnimation<D2D1_POINT_2F>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    UIAnimation_Template_A;
    UIAnimation_Template_B(x);
    UIAnimation_Template_B(y);
    // 减少时间
    this->time -= t;
}


// for D2D1_COLOR_F or Float4
template<> void LongUI::CUIAnimation<D2D1_COLOR_F>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    UIAnimation_Template_A;
    UIAnimation_Template_B(r);
    UIAnimation_Template_B(g);
    UIAnimation_Template_B(b);
    UIAnimation_Template_B(a);
    // 减少时间
    this->time -= t;
}

// for D2D1_MATRIX_3X2_F or Float6
template<> void LongUI::CUIAnimation<D2D1_MATRIX_3X2_F>::Update(float t) noexcept {
    if (this->time <= 0.f) {
        this->value = this->end;
        return;
    }
    UIAnimation_Template_A;
    UIAnimation_Template_B(_11);
    UIAnimation_Template_B(_12);
    UIAnimation_Template_B(_21);
    UIAnimation_Template_B(_22);
    UIAnimation_Template_B(_31);
    UIAnimation_Template_B(_32);
    // 减少时间
    this->time -= t;
}
#undef UIAnimation_Template_A
#undef UIAnimation_Template_B
// CUIAnimation ----------  END  -------------

/// <summary>
/// string to int, 字符串转整型, std::atoi自己实现版
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto __fastcall LongUI::AtoI(const char* __restrict str) noexcept -> int {
    if (!str) return 0;
    register bool negative = false;
    register int value = 0;
    register char ch = 0;
    while (ch = *str) {
        if (!white_space(ch)) {
            if (ch == '-') {
                negative = true;
            }
            else if (valid_digit(ch)) {
                value *= 10;
                value += ch - '0';
            }
            else {
                break;
            }
        }
        ++str;
    }
    // 负数
    if (negative) {
        value = -value;
    }
    return value;
}


/// <summary>
/// string to float.字符串转浮点, std::atof自己实现版
/// </summary>
/// <param name="p">The string.</param>
/// <returns></returns>
auto __fastcall LongUI::AtoF(const char* __restrict p) noexcept -> float {
    bool negative = false;
    float value, scale;
    // 跳过空白
    while (white_space(*p)) ++p;
    // 检查符号
    if (*p == '-') {
        negative = true;
        ++p;
    }
    else if (*p == '+') {
        ++p;
    }
    // 获取小数点或者指数之前的数字(有的话)
    for (value = 0.0f; valid_digit(*p); ++p) {
        value = value * 10.0f + static_cast<float>(*p - '0');
    }
    // 获取小数点或者指数之后的数字(有的话)
    if (*p == '.') {
        float pow10 = 10.0f;
        ++p;
        while (valid_digit(*p)) {
            value += (*p - '0') / pow10;
            pow10 *= 10.0f;
            ++p;
        }
    }
    // 处理指数(有的话)
    bool frac = false;
    scale = 1.0f;
    if ((*p == 'e') || (*p == 'E')) {
        // 获取指数的符号(有的话)
        ++p;
        if (*p == '-') {
            frac = true;
            ++p;
        }
        else if (*p == '+') {
            ++p;
        }
        unsigned int expon;
        // 获取指数的数字(有的话)
        for (expon = 0; valid_digit(*p); ++p) {
            expon = expon * 10 + (*p - '0');
        }
        // float 最大38 double 最大308
        if (expon > 38) expon = 38;
        // 计算比例因数
        while (expon >= 8) { scale *= 1E8f;  expon -= 8; }
        while (expon) { scale *= 10.0f; --expon; }
    }
    // 返回
    register float returncoude = (frac ? (value / scale) : (value * scale));
    if (negative) {
        // float
        returncoude = -returncoude;
    }
    return returncoude;
}



// 源: http://llvm.org/svn/llvm-project/llvm/trunk/lib/Support/ConvertUTF.c
// 有修改

static constexpr int halfShift = 10;

static constexpr char32_t halfBase = 0x0010000UL;
static constexpr char32_t halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START      (char32_t)0xD800
#define UNI_SUR_HIGH_END        (char32_t)0xDBFF
#define UNI_SUR_LOW_START       (char32_t)0xDC00
#define UNI_SUR_LOW_END         (char32_t)0xDFFF

#define UNI_REPLACEMENT_CHAR    (char32_t)0x0000FFFD
#define UNI_MAX_BMP             (char32_t)0x0000FFFF
#define UNI_MAX_UTF16           (char32_t)0x0010FFFF
#define UNI_MAX_UTF32           (char32_t)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32     (char32_t)0x0010FFFF

#define UNI_MAX_UTF8_BYTES_PER_CODE_POINT 4

#define UNI_UTF16_BYTE_ORDER_MARK_NATIVE  0xFEFF
#define UNI_UTF16_BYTE_ORDER_MARK_SWAPPED 0xFFFE

// 转换表
static constexpr char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
* Magic values subtracted from a buffer value during UTF8 conversion.
* This table contains as many values as there might be trailing bytes
* in a UTF-8 sequence.
*/
static constexpr char32_t offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
* Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
* into the first byte, depending on how many bytes follow.  There are
* as many entries in this table as there are UTF-8 sequence types.
* (I.e., one byte sequence, two byte... etc.). Remember that sequencs
* for *legal* UTF-8 will be 4 or fewer bytes total.
*/
static constexpr char firstByteMark[7] = { 0x00i8, 0x00i8, 0xC0i8, 0xE0i8, 0xF0i8, 0xF8i8, 0xFCi8 };


/// <summary>
/// Base64 : encode, 编码
/// </summary>
/// <param name="bindata">The source binary data.</param>
/// <param name="binlen">The length of source binary data in byte</param>
/// <param name="base64">The out data</param>
/// <returns></returns>
auto __fastcall LongUI::Base64Encode(IN const uint8_t* __restrict bindata, IN size_t binlen, OUT char* __restrict const base64 ) noexcept -> char * {
    register uint8_t current;
    register auto base64_index = base64;
    // 
    for (size_t i = 0; i < binlen; i += 3) {
        current = (bindata[i] >> 2);
        current &= static_cast<uint8_t>(0x3F);
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>((bindata)[i] << 4)) & (static_cast<uint8_t>(0x30));
        if (i + 1 >= binlen) {
            *base64_index = Base64Chars[current]; ++base64_index;
            *base64_index = '='; ++base64_index;
            *base64_index = '='; ++base64_index;
            break;
        }
        current |= (static_cast<uint8_t>((bindata)[i + 1] >> 4)) & (static_cast<uint8_t>(0x0F));
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>((bindata)[i + 1] << 2)) & (static_cast<uint8_t>(0x3C));
        if (i + 2 >= binlen) {
            *base64_index = Base64Chars[current]; ++base64_index;
            *base64_index = '='; ++base64_index;
            break;
        }
        current |= (static_cast<uint8_t>((bindata)[i + 2] >> 6)) & (static_cast<uint8_t>(0x03));
        *base64_index = Base64Chars[current]; ++base64_index;

        current = (static_cast<uint8_t>(bindata[i + 2])) & (static_cast<uint8_t>(0x3F));
        *base64_index = Base64Chars[current]; ++base64_index;
    }
    *base64_index = 0;
    return base64;
}

// 解码
auto __fastcall LongUI::Base64Decode(IN const char * __restrict base64, OUT uint8_t * __restrict bindata) noexcept -> size_t{
    // 二进制长度
    register union { uint8_t temp[4]; uint32_t temp_u32; };
    register uint8_t* bindata_index = bindata;
    // 主循环
    while (*base64) {
        temp_u32 = uint32_t(-1);
        // 基本转换
        temp[0] = Base64Datas[base64[0]];  temp[1] = Base64Datas[base64[1]];
        temp[2] = Base64Datas[base64[2]];  temp[3] = Base64Datas[base64[3]];
        // 第一个二进制数据
        *bindata_index = ((temp[0] << 2) & uint8_t(0xFC)) | ((temp[1] >> 4) & uint8_t(0x03));
        ++bindata_index;
        if (base64[2] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((temp[1] << 4) & uint8_t(0xF0)) | ((temp[2] >> 2) & uint8_t(0x0F));
        ++bindata_index;
        if (base64[3] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((temp[2] << 6) & uint8_t(0xF0)) | ((temp[2] >> 0) & uint8_t(0x3F));
        ++bindata_index;
        base64 += 4;
    }
    return bindata_index - bindata;
}

// UTF-16 to UTF-8
// Return: UTF-8 string length, 0 maybe error
auto __fastcall LongUI::UTF16toUTF8(const char16_t* __restrict pUTF16String, char* __restrict pUTF8String) noexcept->uint32_t {
    UINT32 length = 0;
    const char16_t* source = pUTF16String;
    char* target = pUTF8String;
    //char* targetEnd = pUTF8String + uBufferLength;
    // 转换
    while (*source) {
        char32_t ch;
        unsigned short bytesToWrite = 0;
        const char32_t byteMask = 0xBF;
        const char32_t byteMark = 0x80;
       // const char16_t* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if (*source) {
                char32_t ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
                        + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                }
            }
            else {
                --source;
                length = 0;
                assert(!"end of string");
                break;
            }
#ifdef STRICT_CONVERSION
        else { /* it's an unpaired high surrogate */
            --source; /* return to the illegal value itself */
            result = sourceIllegal;
            break;
        }
#endif
        }
#ifdef STRICT_CONVERSION
        else {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                break;
            }
        }
#endif
        /* Figure out how many bytes the result will require */
        if (ch < (char32_t)0x80) {
            bytesToWrite = 1;
        }
        else if (ch < (char32_t)0x800) {
            bytesToWrite = 2;
        }
        else if (ch < (char32_t)0x10000) {
            bytesToWrite = 3;
        }
        else if (ch < (char32_t)0x110000) {
            bytesToWrite = 4;
        }
        else {
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        /*if (target > targetEnd) {
            source = oldSource; // Back up source pointer!
            target -= bytesToWrite;
            length = 0; break;
        }*/
        switch (bytesToWrite) { /* note: everything falls through. */
        case 4: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 3: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 2: *--target = (char)((ch | byteMark) & byteMask); ch >>= 6;
        case 1: *--target = (char)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
        length += bytesToWrite;
    }
    return length;
}



// UTF-8 to UTF-16
// Return: UTF-16 string length, 0 maybe error
auto __fastcall LongUI::UTF8toUTF16(const char* __restrict pUTF8String, char16_t* __restrict pUTF16String) noexcept -> uint32_t {
    UINT32 length = 0;
    auto source = reinterpret_cast<const unsigned char*>(pUTF8String);
    char16_t* target = pUTF16String;
    //char16_t* targetEnd = pUTF16String + uBufferLength;
    // 遍历
    while (*source) {
        char32_t ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        /*if (extraBytesToRead >= sourceEnd - source) {
        result = sourceExhausted; break;
        }*/
        /* Do this check whether lenient or strict */
        /*if (!isLegalUTF8(source, extraBytesToRead + 1)) {
        result = sourceIllegal;
        break;
        }*/
        /*
        * The cases all fall through. See "Note A" below.
        */
        switch (extraBytesToRead) {
        case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
        case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
        case 3: ch += *source++; ch <<= 6;
        case 2: ch += *source++; ch <<= 6;
        case 1: ch += *source++; ch <<= 6;
        case 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        /*if (target >= targetEnd) {
            source -= (extraBytesToRead + 1); // Back up source pointer!
            length = 0; break;
        }*/
        if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
                                 /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
#ifdef STRICT_CONVERSION
                source -= (extraBytesToRead + 1); /* return to the illegal value itself */
                length = 0;
                break;
#else
                *target++ = UNI_REPLACEMENT_CHAR;
                ++length;
#endif
            }
            else {
                *target++ = (char16_t)ch; /* normal case */
                ++length;
            }
        }
        else if (ch > UNI_MAX_UTF16) {
#ifdef STRICT_CONVERSION
            length = 0;
            source -= (extraBytesToRead + 1); /* return to the start */
            break; /* Bail out; shouldn't continue */
#else
            *target++ = UNI_REPLACEMENT_CHAR;
            ++length;
#endif
        }
        else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            /*if (target + 1 >= targetEnd) {
                source -= (extraBytesToRead + 1); // Back up source pointer!
                length = 0; break;
            }*/
            ch -= halfBase;
            *target++ = (char16_t)((ch >> halfShift) + UNI_SUR_HIGH_START);
            *target++ = (char16_t)((ch & halfMask) + UNI_SUR_LOW_START);
            length += 2;
        }
    }
    // 最后修正
    return length;
}

// CUIFileLoader 构造函数
LongUI::CUIFileLoader::CUIFileLoader() noexcept { }

// CUIFileLoader 析构函数
LongUI::CUIFileLoader::~CUIFileLoader() noexcept { 
    if (m_pData) {
        LongUI::NormalFree(m_pData);
        m_pData = nullptr;
    }
}

// CUIFileLoader 读取文件
bool LongUI::CUIFileLoader::ReadFile(WCHAR* file_name) noexcept {
    // 打开文件
    FILE* file = nullptr;
    if (file = ::_wfopen(file_name, L"rb")) {
        // 获取文件大小
        ::fseek(file, 0L, SEEK_END);
        m_cLength = ::ftell(file);
        ::fseek(file, 0L, SEEK_SET);
        // 缓存不足?
        if (m_cLength > m_cLengthReal) {
            m_cLengthReal = m_cLength;
            if (m_pData) LongUI::NormalFree(m_pData);
            m_pData = LongUI::NormalAlloc(m_cLength);
        }
        // 读取文件
        if (m_pData) ::fread(m_pData, 1, m_cLength, file);
    }
    // 关闭文件
    if (file) ::fclose(file);
    return file && m_pData;
}


// --------------  CUIConsole ------------
// CUIConsole 构造函数
LongUI::CUIConsole::CUIConsole() noexcept {
    //::InitializeCriticalSection(&m_cs);  
    m_name[0] = L'\0';
    { if (m_hConsole != INVALID_HANDLE_VALUE) this->Cleanup(); }
}

// CUIConsole 析构函数
LongUI::CUIConsole::~CUIConsole() noexcept {
    this->Cleanup();
    // 关闭
    if (m_hConsole != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
    }
    //::DeleteCriticalSection(&m_cs);
}

// CUIConsole 关闭
long LongUI::CUIConsole::Cleanup() noexcept {
    if (!(*this))
        return -1;
    else
        return ::DisconnectNamedPipe(m_hConsole);
}

// CUIConsole 输出
long LongUI::CUIConsole::Output(const wchar_t * str, bool flush, long len) noexcept {
    if (len == -1) len = static_cast<long>(::wcslen(str));
    // 过长则分批
    if (len > LongUIStringBufferLength) {
        // 直接递归
        while (len) {
            auto len_in = len > LongUIStringBufferLength ? LongUIStringBufferLength : len;
            this->Output(str, true, len_in);
            len -= len_in;
            str += len_in;
        }
        return 0;
    }
    // 计算目标
    if (m_length + len > LongUIStringBufferLength) {
        flush = true;
    }
    // 写入
    if (m_length + len < LongUIStringBufferLength) {
        ::memcpy(m_buffer + m_length, str, len * sizeof(wchar_t));
        m_length += len;
        str = nullptr;
        // 不flush
        if(!flush) return 0;
    }
    DWORD dwWritten = DWORD(-1);
    // 写入
    auto safe_write_file = [this, &dwWritten]() {
        return ::WriteFile(m_hConsole, m_buffer, static_cast<uint32_t>(m_length * sizeof(wchar_t)), &dwWritten, nullptr);
    };
    // 先写入缓冲区
    if (m_length) {
        safe_write_file();
        m_length = 0;
    }
    // 再写入目标
    if (str) {
        len *= sizeof(wchar_t);
        return (!safe_write_file() || (int)dwWritten != len) ? -1 : (int)dwWritten;
    }
    return 0;
}

// CUIConsole 创建
long LongUI::CUIConsole::Create(const wchar_t* lpszWindowTitle, Config& config) noexcept {
    // 二次创建?
    if (m_hConsole != INVALID_HANDLE_VALUE) {
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
    }
    // 先复制
    ::wcscpy(m_name, LR"(\\.\pipe\)");
    wchar_t logger_name_buffer[128];
    // 未给logger?
    if (!config.logger_name) {
        static float s_times = 1.f;
        std::swprintf(
            logger_name_buffer, lengthof(logger_name_buffer),
            L"logger_%7.5f", 
            float(::GetTickCount()) / float(1000 * 60 * 60) *
            (float(::rand()) / float(RAND_MAX)) * s_times
            );
        config.logger_name = logger_name_buffer;
        ++s_times;
    }
    ::wcscat(m_name, config.logger_name);
    // 创建管道
    m_hConsole = ::CreateNamedPipeW(
        m_name,
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        4096,   // 输出缓存
        0,      // 输入缓存
        1,
        nullptr
    );
    // 无效
    if (m_hConsole == INVALID_HANDLE_VALUE) {
        ::MessageBoxW(nullptr, L"CreateNamedPipe failed", L"CUIConsole::Create failed", MB_ICONERROR);
        return -1;
    }
    // 创建控制台
    PROCESS_INFORMATION pi;
    STARTUPINFOW si; ::GetStartupInfoW(&si);
    const wchar_t* DEFAULT_HELPER_EXE = L"ConsoleHelper.exe";

    wchar_t cmdline[MAX_PATH];;
    if (!config.helper_executable)
        config.helper_executable = DEFAULT_HELPER_EXE;

    std::swprintf(cmdline, MAX_PATH, L"%ls %ls", config.helper_executable, config.logger_name);
    BOOL bRet = ::CreateProcessW(nullptr, cmdline, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
    if (!bRet)  {
        auto path = ::_wgetenv(L"ConsoleLoggerHelper");
        if (path) {
            std::swprintf(cmdline, MAX_PATH, L"%ls %ls", path, config.logger_name);
            bRet = ::CreateProcessW(nullptr, nullptr, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
        }
        if (!bRet) {
            ::MessageBoxW(nullptr, L"Helper executable not found", L"ConsoleLogger failed", MB_ICONERROR);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }


    BOOL bConnected = ::ConnectNamedPipe(m_hConsole, nullptr) ?
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    // 连接失败
    if (!bConnected) {
        ::MessageBoxW(nullptr, L"ConnectNamedPipe failed", L"ConsoleLogger failed", MB_ICONERROR);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    DWORD cbWritten;

    // 特性

    wchar_t buffer[128];
    // 传送标题
    if (!lpszWindowTitle) lpszWindowTitle = m_name + 9;
    std::swprintf(buffer, lengthof(buffer), L"TITLE: %ls\r\n", lpszWindowTitle);
    uint32_t len_in_byte = static_cast<uint32_t>(::wcslen(buffer) * sizeof(wchar_t));
    ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
    if (cbWritten != len_in_byte) {
        ::MessageBoxW(nullptr, L"WriteFile failed(1)", L"ConsoleLogger failed", MB_ICONERROR);
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    // 传送位置
    if (config.position_xy != -1) {
        std::swprintf(buffer, lengthof(buffer), L"POS: %d\r\n", config.position_xy);
        len_in_byte = static_cast<uint32_t>(::wcslen(buffer) * sizeof(wchar_t));
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(1.1)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }
    // 传送属性
    if (config.atribute) {
        std::swprintf(buffer, lengthof(buffer), L"ATTR: %d\r\n", config.atribute);
        len_in_byte = static_cast<uint32_t>(::wcslen(buffer) * sizeof(wchar_t));
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(1.2)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }

    // 传送缓存区大小
    if (config.buffer_size_x != -1 && config.buffer_size_y != -1)  {
        std::swprintf(buffer, lengthof(buffer), L"BUFFER-SIZE: %dx%d\r\n", config.buffer_size_x, config.buffer_size_y);
        len_in_byte = static_cast<uint32_t>(::wcslen(buffer) * sizeof(wchar_t));
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(2)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
    }

    // 添加头
    if (false)  {
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }

    // 传送完毕

    buffer[0] = 0;
    ::WriteFile(m_hConsole, buffer, 2, &cbWritten, nullptr);
    if (cbWritten != 2) {
        ::MessageBoxW(nullptr, L"WriteFile failed(3)", L"ConsoleLogger failed", MB_ICONERROR);
        ::DisconnectNamedPipe(m_hConsole);
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
        return -1;
    }
    return 0;
}



// --------------  CUIDefaultConfigure ------------
#ifdef LONGUI_WITH_DEFAULT_CONFIG

// longui
namespace LongUI {
    // 创建XML资源读取器
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) noexcept->IUIResourceLoader*;
}

// 创建接口
auto LongUI::CUIDefaultConfigure::CreateInterface(const IID & riid, void ** ppvObject) noexcept -> HRESULT {
    // 资源读取器
    if (riid == LongUI::IID_IUIResourceLoader) {
        *ppvObject =  LongUI::CreateResourceLoaderForXML(m_manager, this->resource);
    }
    // 脚本
    else if (riid == LongUI::IID_IUIScript) {

    }
    // 检查
    return (*ppvObject) ? S_OK : E_NOINTERFACE;
}

auto LongUI::CUIDefaultConfigure::ChooseAdapter(IDXGIAdapter1 * adapters[], size_t const length) noexcept -> size_t {
    UNREFERENCED_PARAMETER(adapters);
    // 核显卡优先 
#ifdef LONGUI_NUCLEAR_FIRST
    for (size_t i = 0; i < length; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapters[i]->GetDesc1(&desc);
        if (!::wcsncmp(L"NVIDIA", desc.Description, 6))
            return i;
    }
#endif
    return length;
}

// CUIDefaultConfigure 显示错误信息
auto LongUI::CUIDefaultConfigure::ShowError(const wchar_t * str_a, const wchar_t* str_b) noexcept -> void {
    assert(str_a && "bad argument!");
    if(!str_b) str_b = L"Error!";
    ::MessageBoxW(::GetForegroundWindow(), str_a, str_b, MB_ICONERROR);
#ifdef _DEBUG
    assert(!"error");
#endif
}

#ifdef _DEBUG
#include <ctime>
// 输出调试字符串
auto LongUI::CUIDefaultConfigure::OutputDebugStringW(
    DebugStringLevel level, const wchar_t * string, bool flush) noexcept -> void {
    auto& console = this->consoles[level];
    // 无效就创建
    if (!console) {
        this->CreateConsole(level);
    }
    // 有效就输出
    if (console) {
        console.Output(string, flush);
    }
    // 输出到日志?
    if (m_pLogFile && level == DebugStringLevel::DLevel_Log) {
        // 五秒精度
        constexpr uint32_t UNIT = 5'000;
        auto now = ::GetTickCount();
        if ((now / UNIT) != (static_cast<uint32_t>(m_timeTick) / UNIT)) {
            m_timeTick = static_cast<size_t>(now);
            // 不一样则输出时间
            std::time_t time = std::time(nullptr);
            wchar_t buffer[LongUIStringBufferLength];
            std::wcsftime(
                buffer, LongUIStringBufferLength,
                L"[%c]\r\n", std::localtime(&time)
                );
            ::fwrite(buffer, sizeof(wchar_t), std::wcslen(buffer), m_pLogFile);
        }
        ::fwrite(string, sizeof(wchar_t), std::wcslen(string), m_pLogFile);

    }
}

void LongUI::CUIDefaultConfigure::CreateConsole(DebugStringLevel level) noexcept {
    CUIConsole::Config config;
    config.x = -5;
    config.y = int16_t(level) * 128;
    switch (level)
    {
    case LongUI::DLevel_None:
        break;
    case LongUI::DLevel_Log:
        break;
    case LongUI::DLevel_Hint:
        break;
    case LongUI::DLevel_Warning:
        config.atribute = FOREGROUND_RED | FOREGROUND_GREEN;
        break;
    case LongUI::DLevel_Error:
    case LongUI::DLevel_Fatal:
        config.atribute = FOREGROUND_RED;
        break;
    }
    assert(level < LongUI::DLEVEL_SIZE);
    // 名称
    const wchar_t* strings[LongUI::DLEVEL_SIZE] = {
        L"None      Console", 
        L"Log       Console", 
        L"Hint      Console", 
        L"Warning   Console", 
        L"Error     Console", 
        L"Fatal     Console"
    };

    this->consoles[level].Create(strings[level], config);
}

#endif
#endif

// ------------------- Video -----------------------
#ifdef LONGUI_VIDEO_IN_MF
// Video 事件通知
HRESULT LongUI::Component::Video::EventNotify(DWORD event, DWORD_PTR param1, DWORD param2) noexcept {
    UNREFERENCED_PARAMETER(param2);
    switch (event)
    {
    case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
        m_bEOS = false;
        break;
    case MF_MEDIA_ENGINE_EVENT_CANPLAY:
        this->Play();
        break;
    case MF_MEDIA_ENGINE_EVENT_PLAY:
        m_bPlaying = true;
        break;
    case MF_MEDIA_ENGINE_EVENT_PAUSE:
        m_bPlaying = false;
        break;
    case MF_MEDIA_ENGINE_EVENT_ENDED:
        m_bPlaying = false;
        m_bEOS = true;
        break;
    case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
        break;
    case MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE:
        ::SetEvent(reinterpret_cast<HANDLE>(param1));
        break;
    case MF_MEDIA_ENGINE_EVENT_ERROR:
    {
        //auto err = MF_MEDIA_ENGINE_ERR(param1);
        //auto hr = HRESULT(param2);
        //int a = 9;
    }
        break;
    }
    return S_OK;
}


// Video 初始化
auto LongUI::Component::Video::Initialize() noexcept ->HRESULT {
    HRESULT hr = S_OK;
    IMFAttributes* attributes = nullptr;
    // 创建MF属性
    if (SUCCEEDED(hr)) {
        hr = ::MFCreateAttributes(&attributes, 1);
    }
    // 设置属性: DXGI管理器
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, UIManager_MFDXGIDeviceManager);
    }
    // 设置属性: 事件通知
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, this);
    }
    // 设置属性: 输出格式
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT_B8G8R8A8_UNORM);
    }
    // 创建媒体引擎
    if (SUCCEEDED(hr)) {
        constexpr DWORD flags = MF_MEDIA_ENGINE_WAITFORSTABLE_STATE;
        hr = UIManager_MFMediaEngineClassFactory->CreateInstance(flags, attributes, &m_pMediaEngine);
    }
    // 获取Ex版
    if (SUCCEEDED(hr)) {
        hr = m_pMediaEngine->QueryInterface(LongUI_IID_PV_ARGS(m_pEngineEx));
    }
    assert(SUCCEEDED(hr));
    ::SafeRelease(attributes);
    return hr;
}

// Video: 重建
auto LongUI::Component::Video::Recreate() noexcept ->HRESULT {
    ::SafeRelease(m_pTargetSurface);
    ::SafeRelease(m_pDrawSurface);
    return this->recreate_surface();
}

// Video: 渲染
void LongUI::Component::Video::Render(D2D1_RECT_F* dst) const noexcept {
    UNREFERENCED_PARAMETER(dst);
    /*const MFARGB bkColor = { 0,0,0,0 };
    assert(m_pMediaEngine);
    // 表面无效
    if (!m_pDrawSurface) {
        this->recreate_surface();
    }
    // 表面有效
    if (m_pDrawSurface) {
        LONGLONG pts;
        if ((m_pMediaEngine->OnVideoStreamTick(&pts)) == S_OK) {
            D3D11_TEXTURE2D_DESC desc;
            m_pTargetSurface->GetDesc(&desc);
            m_pMediaEngine->TransferVideoFrame(m_pTargetSurface, nullptr, &dst_rect, &bkColor);
            m_pDrawSurface->CopyFromBitmap(nullptr, m_pSharedSurface, nullptr);
        }
        D2D1_RECT_F src = { 0.f, 0.f,  float(dst_rect.right), float(dst_rect.bottom) };
        UIManager_RenderTarget->DrawBitmap(m_pDrawSurface, dst, 1.f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &src
            );
    }*/
}

// Component::Video 构造函数
LongUI::Component::Video::Video() noexcept {
    force_cast(dst_rect) = { 0 };
}

// Component::Video 析构函数
LongUI::Component::Video::~Video() noexcept {
    if (m_pMediaEngine) {
        m_pMediaEngine->Shutdown();
    }
    ::SafeRelease(m_pMediaEngine);
    ::SafeRelease(m_pEngineEx);
    ::SafeRelease(m_pTargetSurface);
    ::SafeRelease(m_pSharedSurface);
    ::SafeRelease(m_pDrawSurface);
}

// 重建表面
auto LongUI::Component::Video::recreate_surface() noexcept ->HRESULT {
    // 有效情况下
    DWORD w, h; HRESULT hr = S_FALSE;
    if (this->HasVideo() && SUCCEEDED(hr = m_pMediaEngine->GetNativeVideoSize(&w, &h))) {
        force_cast(dst_rect.right) = w;
        force_cast(dst_rect.bottom) = h;
        // 获取规范大小
        w = LongUI::MakeAsUnit(w); h = LongUI::MakeAsUnit(h);
        // 检查承载大小
        D2D1_SIZE_U size = m_pDrawSurface ? m_pDrawSurface->GetPixelSize() : D2D1::SizeU();
        // 重建表面
        if (w > size.width || h > size.height) {
            size = { w, h };
            ::SafeRelease(m_pTargetSurface);
            ::SafeRelease(m_pSharedSurface);
            ::SafeRelease(m_pDrawSurface);
            IDXGISurface* surface = nullptr;
#if 0
            D3D11_TEXTURE2D_DESC desc = {
                w, h, 1, 1, DXGI_FORMAT_B8G8R8A8_UNORM, {1, 0}, D3D11_USAGE_DEFAULT, 
                D3D11_BIND_RENDER_TARGET, 0, 0
            };
            hr = UIManager_D3DDevice->CreateTexture2D(&desc, nullptr, &m_pTargetSurface);
            // 获取Dxgi表面
            if (SUCCEEDED(hr)) {
                hr = m_pTargetSurface->QueryInterface(LongUI_IID_PV_ARGS(surface));
            }
            // 从Dxgi表面创建位图
            if (SUCCEEDED(hr)) {
                hr = UIManager_RenderTarget->CreateBitmapFromDxgiSurface(
                    surface, nullptr, &m_pDrawSurface
                    );
            }
#else
            // 创建D2D位图
            D2D1_BITMAP_PROPERTIES1 prop = {
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                LongUI::GetDpiX(),
                LongUI::GetDpiY(),
                D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_TARGET, nullptr
            };
            hr = UIManager_RenderTarget->CreateBitmap(size, nullptr, size.width * 4, &prop, &m_pSharedSurface);
            // 获取Dxgi表面
            if (SUCCEEDED(hr)) {
                hr = m_pSharedSurface->GetSurface(&surface);
            }
            // 获取D3D11 2D纹理
            if (SUCCEEDED(hr)) {
                hr = surface->QueryInterface(LongUI_IID_PV_ARGS(m_pTargetSurface));
            }
            // 创建刻画位图
            if (SUCCEEDED(hr)) {
                prop.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
                hr = UIManager_RenderTarget->CreateBitmap(size, nullptr, size.width * 4, &prop, &m_pDrawSurface);
            }
#endif
            ::SafeRelease(surface);
        }
    }
    return hr;
}

#endif



// -----------------------------



// π
static constexpr float EZ_PI   = 3.1415296F;
// 二分之一π
static constexpr float EZ_PI_2 = 1.5707963F;

// 反弹渐出
float inline __fastcall BounceEaseOut(float p) noexcept {
    if (p < 4.f / 11.f) {
        return (121.f * p * p) / 16.f;
    }
    else if (p < 8.f / 11.f) {
        return (363.f / 40.f * p * p) - (99.f / 10.f * p) + 17.f / 5.f;
    }
    else if (p < 9.f / 10.f) {
        return (4356.f / 361.f * p * p) - (35442.f / 1805.f * p) + 16061.f / 1805.f;
    }
    else {
        return (54.f / 5.f * p * p) - (513.f / 25.f * p) + 268.f / 25.f;
    }
}


// CUIAnimation 缓动函数
float __fastcall LongUI::EasingFunction(AnimationType type, float p) noexcept {
    assert((p >= 0.f && p <= 1.f) && "bad argument");
    switch (type)
    {
    default:
        assert(!"type unknown");
        __fallthrough;
    case LongUI::AnimationType::Type_LinearInterpolation:
        // 线性插值     f(x) = x
        return p;
    case LongUI::AnimationType::Type_QuadraticEaseIn:
        // 平次渐入     f(x) = x^2
        return p * p;
    case LongUI::AnimationType::Type_QuadraticEaseOut:
        // 平次渐出     f(x) =  -x^2 + 2x
        return -(p * (p - 2.f));
    case LongUI::AnimationType::Type_QuadraticEaseInOut:
        // 平次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^2)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-1)*(2x-3)-1) ; 
        return p < 0.5f ? (p * p * 2.f) : ((-2.f * p * p) + (4.f * p) - 1.f);
    case LongUI::AnimationType::Type_CubicEaseIn:
        // 立次渐入     f(x) = x^3;
        return p * p * p;
    case LongUI::AnimationType::Type_CubicEaseOut:
        // 立次渐出     f(x) = (x - 1)^3 + 1
    {
        register float f = p - 1.f;
        return f * f * f + 1.f;
    }
    case LongUI::AnimationType::Type_CubicEaseInOut:
        // 立次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^3) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^3 + 2) 
        if (p < 0.5f) {
            return p * p * p * 2.f;
        }
        else {
            register float f = (2.f * p) - 2.f;
            return 0.5f * f * f * f + 1.f;
        }
    case LongUI::AnimationType::Type_QuarticEaseIn:
        // 四次渐入     f(x) = x^4
    {
        register float f = p * p;
        return f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseOut:
        // 四次渐出     f(x) = 1 - (x - 1)^4
    {
        register float f = (p - 1.f); f *= f;
        return 1.f - f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseInOut:
        // 四次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^4)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-2)^4 - 2)
        if (p < 0.5f) {
            register float f = p * p;
            return 8.f * f * f;
        }
        else {
            register float f = (p - 1.f); f *= f;
            return 1.f - 8.f * f * f;
        }
    case LongUI::AnimationType::Type_QuinticEaseIn:
        // 五次渐入     f(x) = x^5
    {
        register float f = p * p;
        return f * f * p;
    }
    case LongUI::AnimationType::Type_QuinticEaseOut:
        // 五次渐出     f(x) = (x - 1)^5 + 1
    {
        register float f = (p - 1.f);
        return f * f * f * f * f + 1.f;
    }
    case LongUI::AnimationType::Type_QuinticEaseInOut:
        // 五次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^5) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^5 + 2)
        if (p < 0.5) {
            register float f = p * p;
            return 16.f * f * f * p;
        }
        else {
            register float f = ((2.f * p) - 2.f);
            return  f * f * f * f * f * 0.5f + 1.f;
        }
    case LongUI::AnimationType::Type_SineEaseIn:
        // 正弦渐入     
        return ::sin((p - 1.f) * EZ_PI_2) + 1.f;
    case LongUI::AnimationType::Type_SineEaseOut:
        // 正弦渐出     
        return ::sin(p * EZ_PI_2);
    case LongUI::AnimationType::Type_SineEaseInOut:
        // 正弦出入     
        return 0.5f * (1.f - ::cos(p * EZ_PI));
    case LongUI::AnimationType::Type_CircularEaseIn:
        // 四象圆弧
        return 1.f - ::sqrt(1.f - (p * p));
    case LongUI::AnimationType::Type_CircularEaseOut:
        // 二象圆弧
        return ::sqrt((2.f - p) * p);
    case LongUI::AnimationType::Type_CircularEaseInOut:
        // 圆弧出入
        if (p < 0.5f) {
            return 0.5f * (1.f - ::sqrt(1.f - 4.f * (p * p)));
        }
        else {
            return 0.5f * (::sqrt(-((2.f * p) - 3.f) * ((2.f * p) - 1.f)) + 1.f);
        }
    case LongUI::AnimationType::Type_ExponentialEaseIn:
        // 指数渐入     f(x) = 2^(10(x - 1))
        return (p == 0.f) ? (p) : (::pow(2.f, 10.f * (p - 1.f)));
    case LongUI::AnimationType::Type_ExponentialEaseOut:
        // 指数渐出     f(x) =  -2^(-10x) + 1
        return (p == 1.f) ? (p) : (1.f - ::powf(2.f, -10.f * p));
    case LongUI::AnimationType::Type_ExponentialEaseInOut:
        // 指数出入
        // [0,0.5)      f(x) = (1/2)2^(10(2x - 1)) 
        // [0.5,1.f]    f(x) = -(1/2)*2^(-10(2x - 1))) + 1 
        if (p == 0.0f || p == 1.0f) return p;
        if (p < 0.5f) {
            return 0.5f * ::powf(2.f, (20.f * p) - 10.f);
        }
        else {
            return -0.5f * ::powf(2.f, (-20.f * p) + 1.f) + 1.f;
        }
    case LongUI::AnimationType::Type_ElasticEaseIn:
        // 弹性渐入
        return ::sin(13.f * EZ_PI_2 * p) * ::pow(2.f, 10.f * (p - 1.f));
    case LongUI::AnimationType::Type_ElasticEaseOut:
        // 弹性渐出
        return ::sin(-13.f * EZ_PI_2 * (p + 1.f)) * ::powf(2.f, -10.f * p) + 1.f;
    case LongUI::AnimationType::Type_ElasticEaseInOut:
        // 弹性出入
        if (p < 0.5f) {
            return 0.5f * ::sin(13.f * EZ_PI_2 * (2.f * p)) * ::pow(2.f, 10.f * ((2.f * p) - 1.f));
        }
        else {
            return 0.5f * (::sin(-13.f * EZ_PI_2 * ((2.f * p - 1.f) + 1.f)) * ::pow(2.f, -10.f * (2.f * p - 1.f)) + 2.f);
        }
    case LongUI::AnimationType::Type_BackEaseIn:
        // 回退渐入
        return  p * p * p - p * ::sin(p * EZ_PI);
    case LongUI::AnimationType::Type_BackEaseOut:
        // 回退渐出
    {
        register float f = (1.f - p);
        return 1.f - (f * f * f - f * ::sin(f * EZ_PI));
    }
    case LongUI::AnimationType::Type_BackEaseInOut:
        // 回退出入
        if (p < 0.5f) {
            register float f = 2.f * p;
            return 0.5f * (f * f * f - f * ::sin(f * EZ_PI));
        }
        else {
            register float f = (1.f - (2 * p - 1.f));
            return 0.5f * (1.f - (f * f * f - f * ::sin(f * EZ_PI))) + 0.5f;
        }
    case LongUI::AnimationType::Type_BounceEaseIn:
        // 反弹渐入
        return 1.f - ::BounceEaseOut(1.f - p);
    case LongUI::AnimationType::Type_BounceEaseOut:
        // 反弹渐出
        return ::BounceEaseOut(p);
    case LongUI::AnimationType::Type_BounceEaseInOut:
        // 反弹出入
        if (p < 0.5f) {
            return 0.5f * (1.f - ::BounceEaseOut(1.f - (p*2.f)));
        }
        else {
            return 0.5f * ::BounceEaseOut(p * 2.f - 1.f) + 0.5f;
        }
    }
}

                   

// 创建 CC
auto LongUI::Helper::MakeCC(const char* str, CC* OPTIONAL data) noexcept -> uint32_t {
    assert(str && "bad argument");
    uint32_t count = 0;
    // 缓存
    char temp_buffer[LongUIStringFixedLength * 2];
    // 正式解析
    const char* word_begin = nullptr;
    for (auto itr = str;; ++itr) {
        // 获取
        register char ch = *itr;
        // 段结束?
        if (ch == ',' || !ch) {
            assert(word_begin && "bad string");
            // 有效
            if (word_begin && data) {
                CC& cc = data[count - 1];
                size_t length = size_t(itr - word_begin);
                assert(length < lengthof(temp_buffer));
                ::memcpy(temp_buffer, word_begin, length);
                temp_buffer[length] = 0;
                // 数字?
                if (word_begin[0] >= '0' && word_begin[0] <= '9') {
                    assert(!cc.id && "'cc.id' had been set, maybe more than 1 consecutive-id");
                    cc.id = size_t(LongUI::AtoI(temp_buffer));
                }
                // 英文
                else {
                    assert(!cc.func && "'cc.func' had been set");
                    cc.func = UIManager.GetCreateFunc(temp_buffer);
                    assert(cc.func && "bad func address");
                }
            }
            // 清零
            word_begin = nullptr;
            // 看看
            if(ch) continue;
            else break;
        }
        // 空白
        else if (white_space(ch)) {
            continue;
        }
        // 无效字段起始?
        if (!word_begin) {
            word_begin = itr;
            // 查看
            if ((word_begin[0] >= 'A' && word_begin[0] <= 'Z') ||
                word_begin[0] >= 'a' && word_begin[0] <= 'z') {
                if (data) {
                    data[count].func = nullptr;
                    data[count].id = 0;
                }
                ++count;
            }
        }
    }
    return count;
}

// 命名空间
namespace LongUI { namespace Helper {
    // 创建浮点
    bool MakeFloats(const char* sdata, float* fdata, int size) noexcept {
        if (!sdata || !*sdata) return false;
        // 断言
        assert(fdata && size && "bad argument");
        // 拷贝数据
        char buffer[LongUIStringBufferLength];
        ::strcpy_s(buffer, sdata);
        char* index = buffer;
        const char* to_parse = buffer;
        // 遍历检查
        bool new_float = true;
        while (size) {
            char ch = *index;
            // 分段符?
            if (ch == ',' || white_space(ch) || !ch) {
                if (new_float) {
                    *index = 0;
                    *fdata = ::LongUI::AtoF(to_parse);
                    ++fdata;
                    --size;
                    new_float = false;
                }
            }
            else if (!new_float) {
                to_parse = index;
                new_float = true;
            }
            // 退出
            if (!ch) break;
            ++index;
        }
        return true;
    }
}}

// 16进制
unsigned int __fastcall LongUI::Hex2Int(char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    else {
        return c - '0';
    }
}

// 获取颜色表示
bool LongUI::Helper::MakeColor(const char* data, D2D1_COLOR_F& color) noexcept {
    if (!data || !*data) return false;
    // 获取有效值
    while (white_space(*data)) ++data;
    // 以#开头?
    if (*data == '#') {
        color.a = 1.f;
        // #RGB
        if (white_space(data[4]) || !data[4]) {
            color.r = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.g = static_cast<float>(Hex2Int(*++data)) / 15.f;
            color.b = static_cast<float>(Hex2Int(*++data)) / 15.f;
        }
        // #RRGGBB
        else if (white_space(data[7]) || !data[7]) {
            color.r = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
        }
        // #AARRGGBB
        else {
            color.a = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.r = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.g = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
            color.b = static_cast<float>((Hex2Int(*++data) << 4) | (Hex2Int(*++data))) / 255.f;
        }
        return true;
    }
    // 浮点数组
    else {
        return Helper::MakeFloats(data, reinterpret_cast<float*>(&color), 4);
    }
}


// 创建字符串
bool LongUI::Helper::MakeString(const char* data, CUIString& str) noexcept {
    if (!data || !*data) return false;
    wchar_t buffer[LongUIStringBufferLength];
    // 转码
    register auto length = LongUI::UTF8toWideChar(data, buffer);
    buffer[length] = L'\0';
    // 设置字符串
    str.Set(buffer, length);
    return true;
}

// 设置边框颜色
bool LongUI::Helper::SetBorderColor(pugi::xml_node node, D2D1_COLOR_F color[STATUS_COUNT]) noexcept {
    // 边框颜色
    color[Status_Disabled] = D2D1::ColorF(0xD9D9D9);
    color[Status_Normal] = D2D1::ColorF(0xACACAC);
    color[Status_Hover] = D2D1::ColorF(0x7EB4EA);
    color[Status_Pushed] = D2D1::ColorF(0x569DE5);
    // 检查
    if (node) {
        const char* attr[] = {
            "disabledbordercolor", "normalbordercolor",
            "hoverbordercolor",  "pushedbordercolor",
        };
        for (auto i = 0u; i < STATUS_COUNT; ++i) {
            Helper::MakeColor(node.attribute(attr[i]).value(), color[i]);
        }
    }
    return true;
}



// --------------------------------------------------------------------------------------------------------

// 获取XML值
auto LongUI::Helper::XMLGetValue(
    pugi::xml_node node, const char* attribute, const char* prefix
    ) noexcept -> const char* {
    if (!node) return nullptr;
    assert(attribute && "bad argument");
    char buffer[LongUIStringBufferLength];
    // 前缀有效?
    if (prefix) {
        ::strcpy(buffer, prefix); 
        ::strcat(buffer, attribute);
        attribute = buffer;
    }
    return node.attribute(attribute).value();
}

// 获取XML值作为枚举值
auto LongUI::Helper::XMLGetValueEnum(pugi::xml_node node, 
    const XMLGetValueEnumProperties& prop, uint32_t bad_match) noexcept->uint32_t {
    // 获取属性值
    auto value = Helper::XMLGetValue(node, prop.attribute, prop.prefix);
    // 有效
    if (value && *value) {
        auto first_digital = [](const char* str) {
            register char ch = 0;
            while ((ch = *str)) {
                if (white_space(ch)) {
                    ++str;
                }
                else if (ch >= '0' && ch <= '9') {
                    return true;
                }
                else {
                    break;
                }
            }
            return false;
        };
        // 数字?
        if (first_digital(value)) {
            return uint32_t(LongUI::AtoI(value));
        }
        // 遍历
        for (size_t i = 0; i < prop.values_length; ++i) {
            if (!::strcmp(value, prop.values[i])) {
                return uint32_t(i);
            }
        }
    }
    // 匹配无效
    return bad_match;
}

// 获取动画类型
auto LongUI::Helper::XMLGetAnimationType(
    pugi::xml_node node,
    AnimationType bad_match,
    const char* attribute ,
    const char* prefix
    ) noexcept->AnimationType {
    // 属性值列表
    static const char* type_list[] = {
        "linear",
        "quadraticim",
        "quadraticout",
        "quadraticinout",
        "cubicin",
        "cubicout",
        "cubicoinout",
        "quarticin",
        "quarticout",
        "quarticinout",
        "quinticcin",
        "quinticcout",
        "quinticinout",
        "sincin",
        "sincout",
        "sininout",
        "circularcin",
        "circularcout",
        "circularinout",
        "exponentiacin",
        "exponentiaout",
        "exponentiainout",
        "elasticin",
        "elasticout",
        "elasticinout",
        "backin",
        "backout",
        "backinout",
        "bouncein",
        "bounceout",
        "bounceinout",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = type_list;
    prop.values_length = lengthof(type_list);
    // 调用
    return static_cast<AnimationType>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取插值模式
auto LongUI::Helper::XMLGetD2DInterpolationMode(
    pugi::xml_node node, D2D1_INTERPOLATION_MODE bad_match, 
    const char* attribute, const char* prefix
    ) noexcept->D2D1_INTERPOLATION_MODE {
    // 属性值列表
    const char* mode_list[] = {
        "neighbor",
        "linear",
        "cubic",
        "mslinear",
        "anisotropic",
        "highcubic",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = mode_list;
    prop.values_length = lengthof(mode_list);
    // 调用
    return static_cast<D2D1_INTERPOLATION_MODE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取扩展模式
auto LongUI::Helper::XMLGetD2DExtendMode(
    pugi::xml_node node, D2D1_EXTEND_MODE bad_match, 
    const char* attribute, const char* prefix
    ) noexcept->D2D1_EXTEND_MODE {
    // 属性值列表
    const char* mode_list[] = {
        "clamp",
        "wrap",
        "mirror",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = mode_list;
    prop.values_length = lengthof(mode_list);
    // 调用
    return static_cast<D2D1_EXTEND_MODE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取位图渲染规则
auto LongUI::Helper::XMLGetBitmapRenderRule(
    pugi::xml_node node, BitmapRenderRule bad_match,
    const char* attribute, const char* prefix
    ) noexcept->BitmapRenderRule {
    // 属性值列表
    const char* rule_list[] = {
        "scale",
        "button",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<BitmapRenderRule>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取富文本类型
auto LongUI::Helper::XMLGetRichType(
    pugi::xml_node node, RichType bad_match,
    const char* attribute, const char* prefix
    ) noexcept->RichType {
    // 属性值列表
    const char* rule_list[] = {
        "none",
        "core",
        "xml",
        "custom",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<RichType>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取字体类型
auto LongUI::Helper::XMLGetFontStyle(
    pugi::xml_node node, DWRITE_FONT_STYLE bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_FONT_STYLE {
    // 属性值列表
    const char* rule_list[] = {
        "normal",
        "oblique",
        "italic",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_FONT_STYLE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取字体拉伸
auto LongUI::Helper::XMLGetFontStretch(
    pugi::xml_node node, DWRITE_FONT_STRETCH bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_FONT_STRETCH {
    // 属性值列表
    const char* rule_list[] = {
        "undefined",
        "ultracondensed",
        "extracondensed",
        "condensed",
        "semicondensed",
        "normal",
        "semiexpanded",
        "expanded",
        "extraexpanded",
        "ultraexpanded",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_FONT_STRETCH>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取
auto LongUI::Helper::XMLGetFlowDirection(
    pugi::xml_node node, DWRITE_FLOW_DIRECTION bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_FLOW_DIRECTION {
    // 属性值列表
    const char* rule_list[] = {
        "top2bottom",
        "bottom2top",
        "left2right",
        "right2left",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_FLOW_DIRECTION>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取阅读方向
auto LongUI::Helper::XMLGetReadingDirection(
    pugi::xml_node node, DWRITE_READING_DIRECTION bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_READING_DIRECTION {
    // 属性值列表
    const char* rule_list[] = {
        "left2right",
        "right2left",
        "top2bottom",
        "bottom2top",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_READING_DIRECTION>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取换行标志
auto LongUI::Helper::XMLGetWordWrapping(
    pugi::xml_node node, DWRITE_WORD_WRAPPING bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_WORD_WRAPPING {
    // 属性值列表
    const char* rule_list[] = {
        "wrap",
        "nowrap",
        "break",
        "word",
        "character",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_WORD_WRAPPING>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取段落对齐方式
auto LongUI::Helper::XMLGetVAlignment(
    pugi::xml_node node, DWRITE_PARAGRAPH_ALIGNMENT bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_PARAGRAPH_ALIGNMENT {
    // 属性值列表
    const char* rule_list[] = {
        "top",
        "bottom",
        "middle",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}


// 获取段落对齐方式
auto LongUI::Helper::XMLGetHAlignment(
    pugi::xml_node node, DWRITE_TEXT_ALIGNMENT bad_match,
    const char* attribute, const char* prefix
    ) noexcept->DWRITE_TEXT_ALIGNMENT {
    // 属性值列表
    const char* rule_list[] = {
        "left",
        "right",
        "center",
        "justify",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = attribute;
    prop.prefix = prefix;
    prop.values = rule_list;
    prop.values_length = lengthof(rule_list);
    // 调用
    return static_cast<DWRITE_TEXT_ALIGNMENT>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}

// 获取文本抗锯齿模式
auto LongUI::Helper::XMLGetD2DTextAntialiasMode(
    pugi::xml_node node, D2D1_TEXT_ANTIALIAS_MODE bad_match
    ) noexcept->D2D1_TEXT_ANTIALIAS_MODE {
    // 属性值列表
    const char* mode_list[] = {
        "default",
        "cleartype",
        "grayscale",
        "aliased",
    };
    // 设置
    XMLGetValueEnumProperties prop;
    prop.attribute = LongUI::XMLAttribute::WindowTextAntiMode;
    prop.prefix = nullptr;
    prop.values = mode_list;
    prop.values_length = lengthof(mode_list);
    // 调用
    return static_cast<D2D1_TEXT_ANTIALIAS_MODE>(XMLGetValueEnum(node, prop, uint32_t(bad_match)));
}


// Render Common Brush
void LongUI::FillRectWithCommonBrush(ID2D1RenderTarget* target, ID2D1Brush* brush, const D2D1_RECT_F& rect) noexcept {
    assert(target && brush && "bad arguments");
    // 保存状态
    D2D1_MATRIX_3X2_F matrix; target->GetTransform(&matrix);
    auto height = rect.bottom - rect.top;
    D2D1_RECT_F unit_rect = {
        0.f, 0.f, (rect.right - rect.left) / height , 1.f
    };
    // 新的
    target->SetTransform(
        D2D1::Matrix3x2F::Scale(height, height) *
        D2D1::Matrix3x2F::Translation(rect.left, rect.top) *
        matrix
        );
    // 填写
    target->FillRectangle(unit_rect, brush);
    // 恢复
    target->SetTransform(&matrix);
}
                   


// 创建LongUI的字体集: 本函数会进行I/O, 所以程序开始调用一次即可
auto LongUI::DX::CreateFontCollection(
    IDWriteFactory* factory, const wchar_t * filename, const wchar_t * folder)
    noexcept -> IDWriteFontCollection* {
    // 字体文件枚举
    class LongUIFontFileEnumerator final : public Helper::ComStatic<
        Helper::QiList<IDWriteFontFileEnumerator >> {
    public:
        // 获取当前字体文件
        HRESULT STDMETHODCALLTYPE GetCurrentFontFile(IDWriteFontFile **ppFontFile) noexcept override {
            if (!ppFontFile) return E_INVALIDARG;
            if (!m_pFilePath || !m_pFactory)  return E_FAIL;
            *ppFontFile = ::SafeAcquire(m_pCurFontFie);
            return m_pCurFontFie ? S_OK : E_FAIL;
        }
        // 移动到下一个文件
        HRESULT STDMETHODCALLTYPE MoveNext(BOOL *pHasCurrentFile) noexcept override {
            if (!pHasCurrentFile)return E_INVALIDARG;
            if (!m_pFilePath || !m_pFactory) return E_FAIL;
            HRESULT hr = S_OK;
            if (*pHasCurrentFile = *m_pFilePathNow) {
                ::SafeRelease(m_pCurFontFie);
                hr = m_pFactory->CreateFontFileReference(m_pFilePathNow, nullptr, &m_pCurFontFie);
                if (*pHasCurrentFile = SUCCEEDED(hr)) {
                    m_pFilePathNow += ::wcslen(m_pFilePathNow);
                    ++m_pFilePathNow;
                }
            }
            return hr;
        }
    public:
        // 构造函数
        LongUIFontFileEnumerator(IDWriteFactory* f) :m_pFactory(::SafeAcquire(f)) {}
        // 析构函数
        ~LongUIFontFileEnumerator() { ::SafeRelease(m_pCurFontFie); ::SafeRelease(m_pFactory); }
        // 初始化
        auto Initialize(const wchar_t* path) { m_pFilePathNow = m_pFilePath = path; };
    private:
        // 文件路径 连续字符串
        const wchar_t*              m_pFilePath = nullptr;
        // 当前文件路径
        const wchar_t*              m_pFilePathNow = nullptr;
        // 当前Direct Write Font File
        IDWriteFontFile*            m_pCurFontFie = nullptr;
        // DWrite 工厂
        IDWriteFactory*             m_pFactory;
    };
    // 字体文件载入器
    class LongUIFontCollectionLoader final : public Helper::ComStatic<
        Helper::QiList<IDWriteFontCollectionLoader >> {
    public:
        // 创建枚举器
        HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
            IDWriteFactory *pFactory,
            const void *collectionKey,
            UINT32 collectionKeySize,
            IDWriteFontFileEnumerator **ppFontFileEnumerator
            ) noexcept override {
            UNREFERENCED_PARAMETER(collectionKeySize);
            if (!pFactory || !ppFontFileEnumerator) return E_INVALIDARG;
            m_enumerator.LongUIFontFileEnumerator::~LongUIFontFileEnumerator();
            m_enumerator.LongUIFontFileEnumerator::LongUIFontFileEnumerator(pFactory);
            m_enumerator.Initialize(reinterpret_cast<const wchar_t*>(collectionKey));
            *ppFontFileEnumerator = &m_enumerator;
            return S_OK;
        }
    public:
        // 构造函数
        LongUIFontCollectionLoader() :m_enumerator(nullptr) {}
        // 析构函数
        ~LongUIFontCollectionLoader() = default;
    private:
        // 枚举器
        LongUIFontFileEnumerator        m_enumerator;
    };
    IDWriteFontCollection* collection = nullptr;
    constexpr size_t buffer_length = 256 * 256;
    // 申请足够的空间
    wchar_t* const buffer(new(std::nothrow) wchar_t[buffer_length]);
    if (buffer) {
        wchar_t* index = buffer; *buffer = 0;
        WIN32_FIND_DATAW fileinfo;
        wchar_t file_name_path[MAX_PATH]; std::swprintf(file_name_path, MAX_PATH, L"%ls\\%ls", folder, filename);
        HANDLE hFile = ::FindFirstFileW(file_name_path, &fileinfo);
        DWORD errorcode = ::GetLastError();
        // 遍历文件
        while (hFile != INVALID_HANDLE_VALUE && errorcode != ERROR_NO_MORE_FILES) {
            std::swprintf(index, MAX_PATH, L"%ls\\%ls", folder, fileinfo.cFileName);
            index += ::wcslen(index) + 1; *index = 0;
            if (index + MAX_PATH >= buffer + buffer_length) {
                break;
            }
            ::FindNextFileW(hFile, &fileinfo);
            errorcode = ::GetLastError();
        }
        ::FindClose(hFile);
        // 当存在符合标准的文件时
        if (index != buffer) {
            LongUIFontCollectionLoader loader;
            factory->RegisterFontCollectionLoader(&loader);
            factory->CreateCustomFontCollection(
                &loader,
                buffer, 
                static_cast<uint32_t>(reinterpret_cast<uint8_t*>(index) - reinterpret_cast<uint8_t*>(buffer)),
                &collection
                );
            factory->UnregisterFontCollectionLoader(&loader);
        }
        delete[] buffer;
    }
    return collection;
}

// 从 文本格式创建几何
auto LongUI::DX::CreateTextPathGeometry(
    IN const char32_t* utf32_string,
    IN uint32_t string_length,
    IN IDWriteTextFormat* format,
    IN ID2D1Factory* factory,
    IN OUT OPTIONAL IDWriteFontFace** _fontface,
    OUT ID2D1PathGeometry** geometry
    ) noexcept -> HRESULT {
    // 参数检查
    assert(utf32_string && string_length && format && geometry && factory && "bad arugments");
    if (!utf32_string || !string_length || !format || !geometry || !factory) return E_INVALIDARG;
    // 字体集
    IDWriteFontCollection* collection = nullptr;
    IDWriteFontFamily* family = nullptr;
    IDWriteFont* font = nullptr;
    IDWriteFontFace* fontface = nullptr;
    ID2D1PathGeometry* pathgeometry = nullptr;
    if (_fontface) fontface = ::SafeAcquire(*_fontface);
    // 字体名称缓存
    wchar_t fontname_buffer[MAX_PATH]; *fontname_buffer = 0;
    // 必要缓存
    uint16_t glyph_indices_buffer[1024];
    // 保证空间
    uint16_t* glyph_indices = string_length > lengthof(glyph_indices_buffer) ?
        new(std::nothrow) uint16_t[string_length * sizeof(uint16_t)] : glyph_indices_buffer;
    HRESULT hr = glyph_indices ? S_OK : E_OUTOFMEMORY;
    // 创建字形
    if (!fontface) {
        // 获取字体名称
        if (SUCCEEDED(hr)) {
            hr = format->GetFontFamilyName(fontname_buffer, MAX_PATH);
        }
        // 获取字体集
        if (SUCCEEDED(hr)) {
            hr = format->GetFontCollection(&collection);
        }
        // 查找索引
        uint32_t index = 0; BOOL exists = FALSE;
        if (SUCCEEDED(hr)) {
            hr = collection->FindFamilyName(fontname_buffer, &index, &exists);
        }
        // 获取字体族
        if (SUCCEEDED(hr)) {
            // 不存在也算错误
            if (exists) {
                hr = collection->GetFontFamily(index, &family);
            }
            else {
                hr = E_FAIL;
            }
        }
        // 获取字体
        if (SUCCEEDED(hr)) {
            hr = family->GetFirstMatchingFont(
                format->GetFontWeight(),
                format->GetFontStretch(),
                format->GetFontStyle(),
                &font
                );
        }
        // 创建字形
        if (SUCCEEDED(hr)) {
            hr = font->CreateFontFace(&fontface);
        }
    }
    // 创建几何
    if (SUCCEEDED(hr)) {
        hr = factory->CreatePathGeometry(&pathgeometry);
        ID2D1GeometrySink* sink = nullptr;
        // 打开Sink
        if (SUCCEEDED(hr)) {
            hr = pathgeometry->Open(&sink);
        }
        // 创建索引编号
        if (SUCCEEDED(hr)) {
            static_assert(sizeof(uint32_t) == sizeof(char32_t), "32 != 32 ?!");
            hr = fontface->GetGlyphIndices(
                reinterpret_cast<const uint32_t*>(utf32_string), string_length, glyph_indices
                );
        }
        // 创建轮廓路径几何
        if (SUCCEEDED(hr)) {
            hr = fontface->GetGlyphRunOutline(
                format->GetFontSize(),
                glyph_indices,
                nullptr, nullptr,
                string_length,
                true, true, sink
                );
        }
        // 关闭路径
        if (SUCCEEDED(hr)) {
            sink->Close();
        }
        ::SafeRelease(sink);
    }
    // 扫尾
    ::SafeRelease(collection);
    ::SafeRelease(family);
    ::SafeRelease(font);
    if (_fontface && !(*_fontface)) {
        *_fontface = fontface;
    }
    else {
        ::SafeRelease(fontface);
    }
    if (glyph_indices && glyph_indices != glyph_indices_buffer) {
        delete[] glyph_indices;
        glyph_indices = nullptr;
    }
    *geometry = pathgeometry;
#ifdef _DEBUG
    if (pathgeometry) {
        float float_var = 0.f;
        pathgeometry->ComputeLength(nullptr, &float_var);
        pathgeometry->ComputeArea(nullptr, &float_var);
        float_var = 0.f;
    }
#endif
    return hr;
}

// 利用几何体创建网格
auto LongUI::DX::CreateMeshFromGeometry(ID2D1Geometry* geometry, ID2D1Mesh** mesh) noexcept -> HRESULT {
    UNREFERENCED_PARAMETER(geometry);
    UNREFERENCED_PARAMETER(mesh);
    return E_NOTIMPL;
}

// 直接使用
auto LongUI::DX::FormatTextXML(
    const FormatTextConfig& config, 
    const wchar_t* format
    ) noexcept->IDWriteTextLayout*{
    UNREFERENCED_PARAMETER(config);
    UNREFERENCED_PARAMETER(format);
    return nullptr;
}


// 格式化文字
/*
control char    C-Type      Infomation                                  StringInlineParamSupported

%%               [none]      As '%' Character(like %% in ::printf)                 ---
%a %A      [const wchar_t*] string add(like %ls in ::printf)                Yes but no "," char

%C              [float4*]    new font color range start                            Yes
%c              [uint32_t]   new font color range start, with alpha                Yes
!! color is also a drawing effect

%d %D         [IUnknown*]    new drawing effect range start                 ~Yes and Extensible~

%S %S            [float]     new font size range start                             Yes

%n %N       [const wchar_t*] new font family name range start               Yes but No "," char

%h %H            [enum]      new font stretch range start                          Yes

%y %Y            [enum]      new font style range start                            Yes

%w %W            [enum]      new font weight range start                           Yes

%u %U            [BOOL]      new underline range start                          Yes(0 or 1)

%t %T            [BOOL]      new strikethrough range start                      Yes(0 or 1)

%i %I            [IDIO*]     new inline object range start                  ~Yes and Extensible~

%] %}            [none]      end of the last range                                 ---

//  Unsupported
%f %F   [UNSPT]  [IDFC*]     new font collection range start                       ---
IDWriteFontCollection*

%g %G   [UNSPT]  [IDT*]      new ypography range start                         ---
IDWriteTypography*

%l %L   [UNSPT] [char_t*]    new locale name range start                           ---

FORMAT IN STRING
the va_list(ap) can be nullptr while string format
include the PARAMETERS,
using %p or %P to mark PARAMETERS start

*/

// 创建格式文本
auto __cdecl LongUI::DX::FormatTextCoreC(
    const FormatTextConfig& config, 
    const wchar_t* format, 
    ...) noexcept->IDWriteTextLayout* {
    va_list ap;
    va_start(ap, format);
    return DX::FormatTextCore(config, format, ap);
}

// find next param
template<typename T>
auto __fastcall FindNextToken(T* buffer, const wchar_t* stream, size_t token_num) {
    register wchar_t ch;
    while ((ch = *stream)) {
        ++stream;
        if (ch == L',' && !(--token_num)) {
            break;
        }
        *buffer = static_cast<T>(ch);
        ++buffer;
    }
    *buffer = 0;
    return stream;
}


#define DXHelper_GetNextTokenW(n) param = FindNextToken(param_buffer, param, n)
#define DXHelper_GetNextTokenA(n) param = FindNextToken(reinterpret_cast<char*>(param_buffer), param, n)


// 创建格式文本
// 效率本函数耗时参考:
// 包含释放数据(::SafeRelease(layout))
// 1. L"%cHello%], world!%p#FFFF0000"
// Debug    : 循环 1000000(一百万)次，耗时8750ms(精确到16ms)
// Release  : 循环 1000000(一百万)次，耗时3484ms(精确到16ms)
// 2. L"%cHello%], world!%cHello%], world!%p#FFFF0000, #FF00FF00"
// Debug    : 循环 1000000(一百万)次，耗时13922ms(精确到16ms)
// Release  : 循环 1000000(一百万)次，耗时 6812ms(精确到16ms)
// 结论: Release版每处理一个字符(包括格式与参数)平均消耗0.12微秒, Debug版加倍
// 假设: 60Hz每帧16ms 拿出8ms处理本函数, 可以处理6万6个字符
//一般论: 不可能每帧调用6万字, 一般可能每帧处理数百字符(忙碌时), 可以忽略不计
auto LongUI::DX::FormatTextCore( 
    const FormatTextConfig& config, 
    const wchar_t* format,
    va_list ap
    ) noexcept->IDWriteTextLayout* {
    
    // 参数
    const wchar_t* param = nullptr;
    // 检查是否带参数
    if (!ap) {
        register auto format_param_tmp = format;
        register wchar_t ch;
        while (ch = *format_param_tmp) {
            if (ch == L'%') {
                ++format_param_tmp;
                ch = *format_param_tmp;
                if (ch == L'p' || ch == L'p') {
                    param = format_param_tmp + 1;
                    break;
                }
            }
            ++format_param_tmp;
        }
        assert(param && "ap set to nullptr, but none param found.");
    }
    // Range Type
    enum class R : size_t { N, W, Y, H, S, U, T, D, I };
    // Range Data
    struct RangeData {
        DWRITE_TEXT_RANGE       range;
        union {
            const wchar_t*      name;       // N
            DWRITE_FONT_WEIGHT  weight;     // W
            DWRITE_FONT_STYLE   style;      // Y
            DWRITE_FONT_STRETCH stretch;    // H
            float               size;       // S
            BOOL                underline;  // U
            BOOL                strikethr;  // T
            IUnknown*           draweffect; // D
            IDWriteInlineObject*inlineobj;  // I
                                            // ----------------------------
            D2D1_COLOR_F*       color;      // C
            uint32_t            u32;        // c
        };
        R                       range_type;
    } range_data;
    ::memset(&range_data, 0, sizeof(range_data));
    assert(format && "bad argument");
    IDWriteTextLayout* layout = nullptr;
    register CUIColorEffect* tmp_color = nullptr;
    // 缓存字符串长度
    uint32_t string_length = 0;
    // 当前字符
    wchar_t ch;
    // 缓冲区索引
    wchar_t* buffer_index;
    // 参数缓冲区
    wchar_t param_buffer[256];
    // 缓冲区
    wchar_t buffer[LongUIStringBufferLength];
    // 缓冲区
    wchar_t fontname_buffer[LongUIStringBufferLength];
    auto fontname_buffer_index = fontname_buffer;
    // 使用栈
    LongUI::EzContainer::FixedStack<RangeData, 1024> stack_check, statck_set;
    // 缓存起点
    buffer_index = buffer;
    // 便利
    while (ch = *format) {
        // 为%时, 检查下一字符
        if (ch == L'%' && (++format, ch = *format)) {
            switch (ch)
            {
            case L'%':
                // 添加%
                *buffer_index = L'%';
                ++buffer_index;
                ++string_length;
                break;
            case L'A': case L'a': // [A]dd string
            // 复制字符串
            {
                register const wchar_t* i;
                if (ap) {
                    i = va_arg(ap, const wchar_t*);
                }
                else {
                    DXHelper_GetNextTokenW(1);
                    i = param_buffer;
                }
                for (; *i; ++i) {
                    *buffer_index = *i;
                    ++string_length;
                    ++range_data.name;
                }
            }
            break;
            case L'C': // [C]olor in float4
                // 浮点数组颜色开始标记: 
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.color = va_arg(ap, D2D1_COLOR_F*);
                }
                range_data.range_type = R::D;
                // 动态创建颜色效果
                tmp_color = CUIColorEffect::Create();
                assert(tmp_color && "C");
                // 从范围数据中获取
                if (ap) {
                    tmp_color->color = *range_data.color;
                }
                // 直接设置
                else {
                    DXHelper_GetNextTokenA(4);
                    Helper::MakeColor(reinterpret_cast<char*>(param_buffer), tmp_color->color);
                }
                range_data.draweffect = tmp_color;
                stack_check.push(range_data);
                break;
            case L'c': // [C]olor in uint32
                // 32位颜色开始标记: 
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.u32 = va_arg(ap, uint32_t);
                }
                range_data.range_type = R::D;
                // 动态创建颜色效果
                tmp_color = CUIColorEffect::Create();
                assert(tmp_color && "c");
                if (ap) {
                    LongUI::UnpackTheColorARGB(range_data.u32, tmp_color->color);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    Helper::MakeColor(reinterpret_cast<char*>(param_buffer), tmp_color->color);
                }
                range_data.draweffect = tmp_color;
                stack_check.push(range_data);
                break;
            case 'D': case 'd': // [D]rawing effect
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.draweffect = va_arg(ap, IUnknown*);
                }
                else {
                    DXHelper_GetNextTokenW(1);
                    IUnknown* result = nullptr;
                    assert(!"noimpl");
                    range_data.draweffect = result;
                }
                range_data.range_type = R::D;
                stack_check.push(range_data);
                break;
            case 'T': case 't': // strike[T]hrough
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.strikethr = va_arg(ap, BOOL);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    range_data.strikethr = static_cast<BOOL>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::T;
                stack_check.push(range_data);
                break;
            case 'H': case 'h': // stretc[H]
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.stretch = va_arg(ap, DWRITE_FONT_STRETCH);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    range_data.stretch = static_cast<DWRITE_FONT_STRETCH>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::H;
                stack_check.push(range_data);
                break;
            case 'I': case 'i': // [I]nline object
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.inlineobj = va_arg(ap, IDWriteInlineObject*);
                }
                else {
                    DXHelper_GetNextTokenW(1);
                    IDWriteInlineObject* result = nullptr;
                    assert(!"noimpl");
                    range_data.inlineobj = result;
                }
                range_data.range_type = R::I;
                stack_check.push(range_data);
                break;
            case 'N': case 'n': // family [N]ame
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.name = va_arg(ap, const wchar_t*);
                }
                else {
                    // 复制字体名称 并去除前后空白
                    register wchar_t now_ch;
                    auto param_buffer_index = param_buffer;
                    wchar_t* last_firststart_while = nullptr;
                    const wchar_t* firststart_notwhile = nullptr;
                    bool nameless = true;
                    while (now_ch = *param_buffer) {
                        *fontname_buffer_index = now_ch;
                        if (nameless && (now_ch == L' ' || now_ch == L'\t')) {
                            last_firststart_while = fontname_buffer_index;
                            nameless = false;
                        }
                        else {
                            nameless = true;
                            if (!firststart_notwhile) {
                                param_buffer_index = fontname_buffer_index;
                            }
                        }
                        ++fontname_buffer_index;
                    }
                    *last_firststart_while = 0;
                    fontname_buffer_index = last_firststart_while + 1;
                    range_data.name = firststart_notwhile;
                }
                range_data.range_type = R::N;
                stack_check.push(range_data);
                break;
            case 'S': case 's': // [S]ize
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.size = va_arg(ap, float);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    range_data.size = LongUI::AtoF(
                        reinterpret_cast<char*>(param_buffer)
                        );
                }
                range_data.range_type = R::S;
                stack_check.push(range_data);
                break;
            case 'U': case 'u': // [U]nderline
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.underline = va_arg(ap, BOOL);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    range_data.underline = static_cast<BOOL>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::U;
                stack_check.push(range_data);
                break;
            case 'W': case 'w': // [W]eight
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.weight = va_arg(ap, DWRITE_FONT_WEIGHT);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    range_data.weight = static_cast<DWRITE_FONT_WEIGHT>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::W;
                stack_check.push(range_data);
                break;
            case L'Y': case L'y': // st[Y]le
                range_data.range.startPosition = string_length;
                if (ap) {
                    range_data.style = va_arg(ap, DWRITE_FONT_STYLE);
                }
                else {
                    DXHelper_GetNextTokenA(1);
                    range_data.style = static_cast<DWRITE_FONT_STYLE>(
                        LongUI::AtoI(reinterpret_cast<char*>(param_buffer))
                        );
                }
                range_data.range_type = R::Y;
                stack_check.push(range_data);
                break;
            case L'P': case L'p': // end of main string, then, is the param
                goto force_break;
            case L']': case L'}': // end of all range type
                                  // 检查栈弹出
                stack_check.pop();
                // 计算长度
                stack_check.top->range.length = string_length - stack_check.top->range.startPosition;
                // 压入设置栈
                statck_set.push(*stack_check.top);
                break;
            }
        }
        // 添加
        else {
            *buffer_index = ch;
            ++buffer_index;
            ++string_length;
        }
        ++format;
    }
force_break:
    // 尾巴0
    *buffer_index = 0;
    // 计算长度
    assert(string_length < lengthof(buffer));
    // 计算需要长度
    config.text_length = static_cast<decltype(config.text_length)>(string_length);
    register auto string_length_need = static_cast<uint32_t>(static_cast<float>(string_length + 1) * config.progress);
    // clamp it
    if (string_length_need < 0) string_length_need = 0;
    else if (string_length_need > string_length) string_length_need = string_length;
    // 修正
    va_end(ap);
    auto hr = S_OK;
    // 创建布局
    if (SUCCEEDED(hr)) {
        hr = UIManager_DWriteFactory->CreateTextLayout(
            buffer,
            string_length_need,
            config.format,
            config.width, config.height,
            &layout
            );
    }
    // 正式创建
    if (SUCCEEDED(hr)) {
        // 创建
        while (!statck_set.empty()) {
            statck_set.pop();
            // 检查进度(progress)范围 释放数据
            if (statck_set.top->range.startPosition
                + statck_set.top->range.length > string_length_need) {
                if (statck_set.top->range_type == R::D || statck_set.top->range_type == R::I) {
                    ::SafeRelease(statck_set.top->draweffect);
                }
                continue;
            };
            // enum class R :size_t { N, W, Y, H, S, U, E, D, I };
            switch (statck_set.top->range_type)
            {
            case R::N:
                layout->SetFontFamilyName(statck_set.top->name, statck_set.top->range);
                break;
            case R::W:
                layout->SetFontWeight(statck_set.top->weight, statck_set.top->range);
                break;
            case R::Y:
                layout->SetFontStyle(statck_set.top->style, statck_set.top->range);
                break;
            case R::H:
                layout->SetFontStretch(statck_set.top->stretch, statck_set.top->range);
                break;
            case R::S:
                layout->SetFontSize(statck_set.top->size, statck_set.top->range);
                break;
            case R::U:
                layout->SetUnderline(statck_set.top->underline, statck_set.top->range);
                break;
            case R::T:
                layout->SetStrikethrough(statck_set.top->strikethr, statck_set.top->range);
                break;
            case R::D:
                layout->SetDrawingEffect(statck_set.top->draweffect, statck_set.top->range);
                break;
            case R::I:
                layout->SetInlineObject(statck_set.top->inlineobj, statck_set.top->range);
                break;
            }
        }
    }
    // 错误信息
    if (FAILED(hr)) {
        UIManager << DL_Warning << L"HR Code: " << long(hr) << LongUI::endl;
    }
    return layout;
}

// 保存图片
auto LongUI::DX::SaveAsImageFile(
    ID2D1Bitmap1* bitmap, IWICImagingFactory* factory, 
    const wchar_t* file_name, const GUID* container_format
    ) noexcept -> HRESULT {
    // 参数检查
    assert(bitmap && factory && file_name && file_name[0]);
    if (!(bitmap && factory && file_name && file_name[0])) {
        return E_INVALIDARG;
    }
    // 初始化
    D2D1_MAPPED_RECT rect = { 0 };
    HRESULT hr = S_OK;
    ID2D1Bitmap1* readable_bitmap = nullptr;
    auto bitmap_size = bitmap->GetPixelSize();
    auto dxgi_format = bitmap->GetPixelFormat();
    // 创建CPU可读位图
    if (SUCCEEDED(hr)) {
        // CPU 可读?
        if (bitmap->GetOptions() & D2D1_BITMAP_OPTIONS_CPU_READ) {
            readable_bitmap = ::SafeAcquire(bitmap);
        }
        else {
            D2D1_BITMAP_PROPERTIES1 prop;
            prop.pixelFormat = dxgi_format;
            bitmap->GetDpi(&prop.dpiX, &prop.dpiY);
            prop.colorContext = nullptr;
            prop.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
            // 创建位图
            hr = UIManager_RenderTarget->CreateBitmap(
                bitmap_size,
                nullptr,
                0,
                &prop,
                &readable_bitmap
                );
        }
    }
    // 复制位图
    if (SUCCEEDED(hr)) {
        hr = readable_bitmap->CopyFromBitmap(nullptr, bitmap, nullptr);
    }
    // 映射数据
    if (SUCCEEDED(hr)) {
        hr = readable_bitmap->Map(D2D1_MAP_OPTIONS_READ, &rect);
    }
    // 处理
    if (SUCCEEDED(hr)) {
        SaveAsImageFileProperties prop;
        prop.bits = rect.bits;
        prop.factory = factory;
        prop.data_format = LongUI::DX::DXGIToWIC(dxgi_format.format);
        prop.container_format = container_format;
        prop.width = bitmap_size.width;
        prop.height = bitmap_size.height;
        prop.pitch = rect.pitch;
        auto hr1 = LongUI::DX::SaveAsImageFile(prop, file_name);
        auto hr2 = readable_bitmap->Unmap();
        // 检查错误
        if (SUCCEEDED(hr1)) {
            hr = SUCCEEDED(hr2) ? S_OK : hr2;
        }
        else {
            hr = hr1;
        }
    }
    // 扫尾处理
    ::SafeRelease(readable_bitmap);
    // 返回结果
    return hr;
}

#include <Wincodec.h>

// longui namespace
namespace LongUI {
    // dx namespace
    namespace DX {
        // 保存数据为图片格式
        auto SaveAsImageFile(const SaveAsImageFileProperties& prop, const wchar_t* file_name) noexcept -> HRESULT {
            // 参数检查
            assert(prop.bits && prop.factory && file_name && file_name[0]);
            if (!(prop.bits && prop.factory && file_name && file_name[0])) {
                return E_INVALIDARG;
            }
            DXGI_FORMAT;
            // 初始化
            HRESULT hr = S_OK;
            IWICBitmapEncoder *pEncoder = nullptr;
            IWICBitmapFrameEncode *pFrameEncode = nullptr;
            IWICStream *pStream = nullptr;
            IWICBitmap *pWICBitmap = nullptr;
            // 创建WIC位图
            if (SUCCEEDED(hr)) {
                hr = prop.factory->CreateBitmapFromMemory(
                    prop.width,
                    prop.height,
                    prop.data_format ? *prop.data_format : GUID_WICPixelFormat32bppBGRA,
                    prop.pitch,
                    prop.pitch * prop.height,
                    prop.bits,
                    &pWICBitmap
                    );
            }
            // 创建流
            if (SUCCEEDED(hr)) {
                hr = prop.factory->CreateStream(&pStream);
            }
            // 从文件初始化
            if (SUCCEEDED(hr)) {
                hr = pStream->InitializeFromFilename(file_name, GENERIC_WRITE);
            }
            // 创建编码器
            if (SUCCEEDED(hr)) {
                hr = prop.factory->CreateEncoder(
                    prop.container_format ? *prop.container_format : GUID_ContainerFormatPng,
                    nullptr,
                    &pEncoder
                    );
            }
            // 初始化编码器
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
            }
            // 创建新的一帧
            if (SUCCEEDED(hr)) {
                hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr);
            }
            // 初始化帧编码器
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->Initialize(nullptr);
            }
            // 设置大小
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->SetSize(prop.width, prop.height);
            }
            // 设置格式
            WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->SetPixelFormat(&format);
            }
            // 写入源数据
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->WriteSource(pWICBitmap, nullptr);
            }
            // 提交帧编码器
            if (SUCCEEDED(hr)) {
                hr = pFrameEncode->Commit();
            }
            // 提交编码
            if (SUCCEEDED(hr)) {
                hr = pEncoder->Commit();
            }
            // 扫尾处理
            ::SafeRelease(pWICBitmap);
            ::SafeRelease(pStream);
            ::SafeRelease(pFrameEncode);
            ::SafeRelease(pEncoder);
            // 返回结果
            return hr;
        }
        // WIC GUID <--> DXGI_FORMAT
        struct WICTranslate { GUID wic; DXGI_FORMAT format; };
        // data
        static const WICTranslate s_WICFormats[] =  {
            { GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

            { GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
            { GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

            { GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
            { GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
            { GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },

            { GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
            { GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },
            { GUID_WICPixelFormat32bppRGBE,             DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

            { GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
            { GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

            { GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
            { GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
            { GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
            { GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

            { GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

            { GUID_WICPixelFormatBlackWhite,            DXGI_FORMAT_R1_UNORM },

            { GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
        };
        // DXGI格式转换为 WIC GUID 格式
        auto DXGIToWIC(DXGI_FORMAT format) noexcept ->const GUID*{
            const GUID* outformat = nullptr;
            // 检查
            for (const auto& data : s_WICFormats) {
                if (data.format == format) {
                    outformat = &data.wic;
                    break;
                }
            }
            // 特殊
            if (!outformat) {
                switch (format)
                {
                case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                    outformat = &GUID_WICPixelFormat32bppRGBA;
                    break;
                case DXGI_FORMAT_D32_FLOAT:
                    outformat = &GUID_WICPixelFormat32bppGrayFloat;
                    break;
                case DXGI_FORMAT_D16_UNORM:
                    outformat = &GUID_WICPixelFormat16bppGray;
                    break;
                case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                    outformat = &GUID_WICPixelFormat32bppBGRA;
                    break;
                case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                    outformat = &GUID_WICPixelFormat32bppBGR;
                    break;
                }
            }
            // 返回数据
            return outformat;
        }
    }
}

                   

// 解析path
auto LongUI::SVG::ParserPath(const char* path, ID2D1PathGeometry1** out) noexcept ->HRESULT {
    assert(path && out && "bad arguemnts");
    // 无效参数
    if (!(path && out)) return E_INVALIDARG;
    HRESULT hr = S_OK;
    ID2D1PathGeometry1* path_geometry = nullptr;
    // 创建路径几何
    if (SUCCEEDED(hr)) {
        hr = UIManager_D2DFactory->CreatePathGeometry(&path_geometry);
    }
    // 打开sink
    if (SUCCEEDED(hr)) {
        hr = ParserPath(path, path_geometry);
    }
    ::SafeRelease(path_geometry);
    return hr;
}



// 解析path
auto LongUI::SVG::ParserPath(const char* path, ID2D1PathGeometry* geometry) noexcept ->HRESULT {
    assert(path && geometry && "bad arguemnts");
    // 无效参数
    if (!(path && geometry)) return E_INVALIDARG;
    HRESULT hr = S_OK;
    ID2D1GeometrySink* sink = nullptr;
    // 打开sink
    if (SUCCEEDED(hr)) {
        hr = geometry->Open(&sink);
    }
    // 正式解析
    if (SUCCEEDED(hr)) {
        register char ch = 0;
        while ((ch = *path)) {
            ++path;
        }
    }
    // 关闭sink ??
    if (SUCCEEDED(hr)) {
        hr = sink->Close();
    }
    ::SafeRelease(sink);
    return hr;
}
                   
                    
                  

// 任务按钮创建消息
const UINT LongUI::UIWindow::s_uTaskbarBtnCreatedMsg = ::RegisterWindowMessageW(L"TaskbarButtonCreated");

// UIWindow 构造函数
LongUI::UIWindow::UIWindow(pugi::xml_node node, UIWindow* parent_window) 
noexcept : Super(node), m_uiRenderQueue(this), window_parent(parent_window) {
    assert(node && "<LongUI::UIWindow::UIWindow> window_node null");
    ZeroMemory(&m_curMedium, sizeof(m_curMedium));
    CUIString titlename(m_strControlName);
    {
        Helper::MakeString(
            node.attribute(LongUI::XMLAttribute::WindowTitleName).value(),
            titlename
            );
    }
    // flag 区
    {
        auto flag = WindowFlag::Flag_None;
        // 检查FullRendering标记
        if (node.attribute("fullrender").as_bool(false)) {
            flag |= WindowFlag::Flag_FullRendering;
        }
        else {
            //this->reset_renderqueue();
        }
        // 检查alwaysrendering
        if (node.attribute("alwaysrendering").as_bool(false)) {
            flag |= WindowFlag::Flag_AlwaysRendering;
        }
        force_cast(this->window_flags) = flag;
        // XXX:
        force_cast(this->window_type) = Type_Layered;
    }
    // Debug Zone
#ifdef _DEBUG
    {
        debug_show = this->debug_this || node.attribute("debugshow").as_bool(false);
    }
#endif
    // 其他属性
    {
        // 最小大小
        float size[] = { LongUIWindowMinSize, LongUIWindowMinSize };
        Helper::MakeFloats(node.attribute("minisize").value(), size, 2);
        m_miniSize.width = static_cast<decltype(m_miniSize.width)>(size[0]);
        m_miniSize.height = static_cast<decltype(m_miniSize.height)>(size[1]);
        // 清理颜色
        Helper::MakeColor(
            node.attribute(LongUI::XMLAttribute::WindowClearColor).value(),
            this->clear_color
            );
        // 文本抗锯齿
        m_textAntiMode = uint16_t(Helper::XMLGetD2DTextAntialiasMode(node, D2D1_TEXT_ANTIALIAS_MODE_DEFAULT));
    }
    // 窗口区
    {
        // 默认样式
        DWORD window_style = WS_OVERLAPPEDWINDOW;
        // 设置窗口大小
        RECT window_rect = { 0, 0, LongUIDefaultWindowWidth, LongUIDefaultWindowHeight };
        // 默认
        if (this->view_size.width == 0.f) {
            force_cast(this->view_size.width) = static_cast<float>(LongUIDefaultWindowWidth);
        }
        else {
            window_rect.right = static_cast<LONG>(this->view_size.width);
        }
        // 更新
        if (this->view_size.height == 0.f) {
            force_cast(this->view_size.height) = static_cast<float>(LongUIDefaultWindowHeight);
        }
        else {
            window_rect.bottom = static_cast<LONG>(this->view_size.height);
        }
        force_cast(this->window_size.width) = window_rect.right;
        force_cast(this->window_size.height) = window_rect.bottom;
        visible_rect.right = this->view_size.width;
        visible_rect.bottom = this->view_size.height;
        m_2fContentSize = this->view_size;
        // 调整大小
        ::AdjustWindowRect(&window_rect, window_style, FALSE);
        // 居中
        window_rect.right -= window_rect.left;
        window_rect.bottom -= window_rect.top;
        window_rect.left = (::GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
        window_rect.top = (::GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;
        // 创建窗口
        m_hwnd = ::CreateWindowExW(
            //WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            //(this->flags & Flag_Window_DComposition) ? WS_EX_NOREDIRECTIONBITMAP : 0,
            WS_EX_NOREDIRECTIONBITMAP,
            LongUI::WindowClassName, 
            titlename.length() ? titlename.c_str() : L"LongUI",
            WS_OVERLAPPEDWINDOW,
            window_rect.left, window_rect.top, window_rect.right, window_rect.bottom,
            parent_window ? parent_window->GetHwnd() : nullptr,
            nullptr,
            ::GetModuleHandleW(nullptr),
            this
            );
        // 禁止 Alt + Enter 全屏
        if (m_hwnd) {
            UIManager_DXGIFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
        }
        // 创建失败
        else {
            UIManager.ShowError(L"Error! Failed to Create Window", L"LongUI::UIWindow::UIWindow");
        }
    }
    //SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    // 设置Hover
    m_csTME.cbSize = sizeof(m_csTME);
    m_csTME.dwFlags = TME_HOVER | TME_LEAVE;
    m_csTME.hwndTrack = m_hwnd;
    m_csTME.dwHoverTime = LongUIDefaultHoverTime;
    // 创建闪烁计时器
    m_idBlinkTimer = ::SetTimer(m_hwnd, BLINK_EVENT_ID, ::GetCaretBlinkTime(), nullptr);
    // 添加窗口
    UIManager.RegisterWindow(this);
    // 拖放帮助器
    m_pDropTargetHelper = UIManager.GetDropTargetHelper();
    // 注册拖拽目标
    ::RegisterDragDrop(m_hwnd, this);
    // 所在窗口就是自己
    m_pWindow = this;
    // 自己的UI父类就是自己以保证parent不为null
    force_cast(this->parent) = this;
    // 清零
    ::memset(m_dirtyRects, 0, sizeof(m_dirtyRects));
    // 自动显示窗口
    if (node.attribute("autoshow").as_bool(true)) {
        ::ShowWindow(m_hwnd, SW_SHOW);
    }
}

// UIWindow 析构函数
LongUI::UIWindow::~UIWindow() noexcept {
    // 取消注册
    ::RevokeDragDrop(m_hwnd);
    // 杀掉!
    ::KillTimer(m_hwnd, m_idBlinkTimer);
    // 摧毁窗口
    ::DestroyWindow(m_hwnd);
    // 移除窗口
    UIManager.RemoveWindow(this);
    // 释放资源
    this->release_data();
    // 释放数据
    ::SafeRelease(m_pTaskBarList);
    ::SafeRelease(m_pDropTargetHelper);
    ::SafeRelease(m_pCurDataObject);
}


// 注册
void LongUI::UIWindow::RegisterOffScreenRender(UIControl* c, bool is3d) noexcept {
    // 检查
#ifdef _DEBUG
    auto itr = std::find(m_vRegisteredControl.begin(), m_vRegisteredControl.end(), c);
    if (itr != m_vRegisteredControl.end()) {
        UIManager << DL_Warning << L"control: [" << c->GetNameStr() << L"] existed" << LongUI::endl;
        return;
    }
#endif
    try {
        if (is3d) {
            m_vRegisteredControl.insert(m_vRegisteredControl.begin(), c);
        }
        else {
            m_vRegisteredControl.push_back(c);
        }
    }
    catch (...) {
        UIManager << DL_Warning << L"insert failed" << LongUI::endl;
    }
}

// 反注册
void LongUI::UIWindow::UnRegisterOffScreenRender(UIControl* c) noexcept {
    auto itr = std::find(m_vRegisteredControl.begin(), m_vRegisteredControl.end(), c);
    if (itr != m_vRegisteredControl.end()) {
        m_vRegisteredControl.erase(itr);
    }
#ifdef _DEBUG
    else {
        UIManager << DL_Warning << L"control: [" << c->GetNameStr() << L"] not found" << LongUI::endl;
    }
#endif
}


// 设置插入符号
void LongUI::UIWindow::SetCaretPos(UIControl* ctrl, float _x, float _y) noexcept {
    if (!m_cShowCaret) return;
    assert(ctrl && "bad argument") ;
    // 转换为像素坐标
    auto pt = D2D1::Point2F(_x, _y);
    if (ctrl) {
        // FIXME
        // TODO: FIX IT
        pt = LongUI::TransformPoint(ctrl->world, pt);
    }
#ifdef _DEBUG
    if (this->debug_this) {
        UIManager << DL_Log << ctrl
            << LongUI::Formated(L"(%.1f, %.1f)", pt.x, pt.y)
            << LongUI::endl;
    }
#endif
    m_baBoolWindow.SetTrue(Index_CaretIn);
    m_baBoolWindow.SetTrue(Index_DoCaret);
    const register auto intx = static_cast<LONG>(pt.x);
    const register auto inty = static_cast<LONG>(pt.y);
    const register auto oldx = static_cast<LONG>(m_rcCaretPx.left);
    const register auto oldy = static_cast<LONG>(m_rcCaretPx.top);
    if (oldx != intx || oldy != inty) {
        this->refresh_caret();
        m_rcCaretPx.left = intx; m_rcCaretPx.top = inty;
        ::SetCaretPos(intx, inty);
    }
}

// 创建插入符号
void LongUI::UIWindow::CreateCaret(UIControl* ctrl, float width, float height) noexcept {
    assert(ctrl && "bad argument") ;
    this->refresh_caret();
    // 转换为像素单位
    m_rcCaretPx.width = static_cast<uint32_t>(width * ctrl->world._11);
    m_rcCaretPx.height = static_cast<uint32_t>(height * ctrl->world._22);
#ifdef _DEBUG
    if (this->debug_this) {
        UIManager << DL_Log << ctrl
            << LongUI::Formated(L"(%d, %d)", int(m_rcCaretPx.width), int(m_rcCaretPx.height))
            << LongUI::endl;
    }
#endif
    // 阈值检查
    m_rcCaretPx.width = std::max(m_rcCaretPx.width, 1i32);
    m_rcCaretPx.height = std::max(m_rcCaretPx.height, 1i32);
}

// 显示插入符号
void LongUI::UIWindow::ShowCaret() noexcept {
    ++m_cShowCaret;
    // 创建AE位图
    //if (!m_pd2dBitmapAE) {
        //this->recreate_ae_bitmap();
    //}
}

// 异常插入符号
void LongUI::UIWindow::HideCaret() noexcept { 
    if (m_cShowCaret) {
        --m_cShowCaret;
    }
#ifdef _DEBUG
    else {
        UIManager << DL_Warning << L"m_cShowCaret alread to 0" << LongUI::endl;
    }
    if (!m_cShowCaret) {
        UIManager << DL_Log << this << "Caret Hided" << LongUI::endl;
    }
#endif
    if (!m_cShowCaret) {
        m_baBoolWindow.SetFalse(Index_CaretIn);
        m_baBoolWindow.SetTrue(Index_DoCaret);
    }
}

// 查找控件
auto LongUI::UIWindow::FindControl(const CUIString& str) noexcept -> UIControl * {
    // 查找控件
    const auto itr = m_mapString2Control.find(str);
    // 未找到返回空
    if (itr == m_mapString2Control.cend()) {
        // 警告
        UIManager << DL_Warning << L"Control Not Found:\n  " << str << LongUI::endl;
        return nullptr;
    }
    // 找到就返回指针
    else {
        return reinterpret_cast<LongUI::UIControl*>(itr->second);
    }
}

// 添加控件
void LongUI::UIWindow::AddControl(const std::pair<CUIString, void*>& pair) noexcept {
    // 有效
    if (pair.first != L"") {
        try {
#ifdef _DEBUG
            // 先检查
            {
                auto itr = m_mapString2Control.find(pair.first);
                if (itr != m_mapString2Control.end()) {
                    UIManager << DL_Warning << "Exist: " << pair.first << LongUI::endl;
                    assert(!"Control Has been existed!");
                }
            }
#endif
            m_mapString2Control.insert(pair);
        }
        catch (...) {
            ShowErrorWithStr(L"Failed to add control");
        }
    }
}

// 设置图标
void LongUI::UIWindow::SetIcon(HICON hIcon) noexcept {
    ::DefWindowProcW(m_hwnd, WM_SETICON, TRUE, reinterpret_cast<LPARAM>(hIcon));
    ::DefWindowProcW(m_hwnd, WM_SETICON, FALSE, reinterpret_cast<LPARAM>(hIcon));
}


// release data
void LongUI::UIWindow::release_data() noexcept {
    if (m_hVSync) {
        ::CloseHandle(m_hVSync);
        m_hVSync = nullptr;
    }
    // 释放资源
    ::SafeRelease(m_pTargetBimtap);
    ::SafeRelease(m_pSwapChain);
    ::SafeRelease(m_pDcompDevice);
    ::SafeRelease(m_pDcompTarget);
    ::SafeRelease(m_pDcompVisual);
}

// 刻画插入符号
void LongUI::UIWindow::draw_caret() noexcept {
    /*// 不能在BeginDraw/EndDraw之间调用
    D2D1_POINT_2U pt = { m_rcCaretPx.left, m_rcCaretPx.top };
    D2D1_RECT_U src_rect;
    src_rect.top = LongUIWindowPlanningBitmap / 2;
    src_rect.left = m_bCaretIn ? 0 : LongUIWindowPlanningBitmap / 4;
    src_rect.right = src_rect.left + m_rcCaretPx.width;
    src_rect.bottom = src_rect.top + m_rcCaretPx.height;
    m_pTargetBimtap->CopyFromBitmap(
        &pt, m_pBitmapPlanning, &src_rect
        );*/
}

// 更新插入符号
void LongUI::UIWindow::refresh_caret() noexcept {
    // 不能在BeginDraw/EndDraw之间调用
    // TODO: 完成位图复制
}

// 设置呈现
void LongUI::UIWindow::set_present_parameters(DXGI_PRESENT_PARAMETERS& present) const noexcept {
    present.DirtyRectsCount = static_cast<uint32_t>(m_aUnitNow.length);
    // 存在脏矩形?
    if(!m_baBoolWindow.Test(Index_FullRenderingThisFrame)){
        // 插入符号?
        if (m_baBoolWindow.Test(Index_DoCaret)) {
            present.pDirtyRects[present.DirtyRectsCount] = { 
                m_rcCaretPx.left, m_rcCaretPx.top,
                m_rcCaretPx.left + m_rcCaretPx.width,
                m_rcCaretPx.top + m_rcCaretPx.height,
            };
            ++present.DirtyRectsCount;
        }
#ifdef _DEBUG
        static RECT s_rects[LongUIDirtyControlSize + 2];
        if (this->debug_show) {
            ::memcpy(s_rects, present.pDirtyRects, present.DirtyRectsCount * sizeof(RECT));
            present.pDirtyRects = s_rects;
            s_rects[present.DirtyRectsCount] = { 0, 0, 128, 35 };
            ++present.DirtyRectsCount;
        }
#endif
    }
    // 全刷新
    else {
        present.pScrollRect = nullptr;
        present.DirtyRectsCount = 0;
    }
}

// begin draw
void LongUI::UIWindow::BeginDraw() const noexcept {
    // 设置文本渲染策略
    UIManager_RenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE(m_textAntiMode));
    // 离屏渲染
    if (!m_vRegisteredControl.empty()) {
        for (auto i : m_vRegisteredControl) {
            auto ctrl = reinterpret_cast<UIControl*>(i);
            assert(ctrl->parent && "check it");
            UIManager_RenderTarget->SetTransform(&ctrl->parent->world);
            ctrl->Render(RenderType::Type_RenderOffScreen);
        }
    }
    // 设为当前渲染对象
    UIManager_RenderTarget->SetTarget(m_pTargetBimtap);
    // 开始渲染
    UIManager_RenderTarget->BeginDraw();
    // 设置转换矩阵
#if 0
    UIManager_RenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
#else
    UIManager_RenderTarget->SetTransform(&this->world);
#endif
    // 清空背景
    UIManager_RenderTarget->Clear(this->clear_color);
}

// 结束渲染
void LongUI::UIWindow::EndDraw() const noexcept {
    // 结束渲染
    UIManager_RenderTarget->EndDraw();
    // 呈现参数设置
    RECT rcScroll = { 0, 0, LONG(this->window_size.width), LONG(this->window_size.height) };
    RECT dirtyRects[LongUIDirtyControlSize + 1]; 
    ::memcpy(dirtyRects, m_dirtyRects, sizeof(dirtyRects));
    DXGI_PRESENT_PARAMETERS present_parameters;
    present_parameters.DirtyRectsCount = 0;
    present_parameters.pDirtyRects = dirtyRects;
    present_parameters.pScrollRect = &rcScroll;
    present_parameters.pScrollOffset = nullptr;
    // 设置参数
    this->set_present_parameters(present_parameters);
    // 呈现
    HRESULT hr = m_pSwapChain->Present1(1, 0, &present_parameters);
    // 收到重建消息时 重建UI
#ifdef _DEBUG
    assert(SUCCEEDED(hr));
    if (hr == DXGI_ERROR_DEVICE_REMOVED 
        || hr == DXGI_ERROR_DEVICE_RESET 
        || test_D2DERR_RECREATE_TARGET) {
        force_cast(test_D2DERR_RECREATE_TARGET) = false;
        UIManager << DL_Hint << L"D2DERR_RECREATE_TARGET!" << LongUI::endl;
        hr = UIManager.RecreateResources();
        if (FAILED(hr)) {
            UIManager << DL_Hint << L"But, Recreate Failed!!!" << LongUI::endl;
            UIManager << DL_Error << L"Recreate Failed!!!" << LongUI::endl;
        }
    }
#else
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        hr = UIManager.RecreateResources();
    }
#endif
    // 检查
    AssertHR(hr);
}


// UI窗口: 刷新
void LongUI::UIWindow::Update() noexcept {
    m_baBoolWindow.SetFalse(Index_FullRenderingThisFrame);
    m_baBoolWindow.SetFalse(Index_DoCaret);
    // 新窗口大小?
    if (m_baBoolWindow.Test(Index_NewSize)) {
        this->OnResize();
        m_baBoolWindow.SetFalse(Index_NewSize);
    }
    {
        auto current_unit = m_uiRenderQueue.GetCurrentUnit();
        m_aUnitNow.length = current_unit->length;
        ::memcpy(m_aUnitNow.units, current_unit->units, sizeof(*m_aUnitNow.units) * m_aUnitNow.length);
    }
    // 刷新前
    if (this->IsControlSizeChanged()) {
        this->SetWidth(this->visible_rect.right);
        this->SetHeight(this->visible_rect.bottom);
    }
    this->UpdateWorld();
    // 没有就不刷新了
    m_baBoolWindow.SetTo(Index_Rendered, m_aUnitNow.length);
    if (!m_aUnitNow.length) return;
    // 全刷新?
    if (m_aUnitNow.units[0] == static_cast<UIControl*>(this)) {
        m_baBoolWindow.SetTrue(Index_FullRenderingThisFrame);
        //UIManager << DL_Hint << "m_present.DirtyRectsCount = 0;" << endl;
        // 交给父类处理
        Super::Update();
    }
    // 部分刷新
    else {
        m_baBoolWindow.SetFalse(Index_FullRenderingThisFrame);
        // 更新脏矩形
        for (uint32_t i = 0ui32; i < m_aUnitNow.length; ++i) {
            auto ctrl = m_aUnitNow.units[i];
            assert(ctrl->parent && "check it");
            // 设置转换矩阵
            ctrl->Update();
            // 限制转换
            m_dirtyRects[i].left = static_cast<LONG>(ctrl->visible_rect.left);
            m_dirtyRects[i].top = static_cast<LONG>(ctrl->visible_rect.top);
            m_dirtyRects[i].right = static_cast<LONG>(std::ceil(ctrl->visible_rect.right));
            m_dirtyRects[i].bottom = static_cast<LONG>(std::ceil(ctrl->visible_rect.bottom));
        }
    }
    // 调试
#ifdef _DEBUG
    if (m_baBoolWindow.Test(Index_FullRenderingThisFrame)) {
        ++full_render_counter;
    }
    else {
        ++dirty_render_counter;
    }
#endif
}

// UIWindow 渲染 
void LongUI::UIWindow::Render(RenderType type) const noexcept  {
    if (type != RenderType::Type_Render) return ;
    // 全刷新: 继承父类
    if (m_baBoolWindow.Test(Index_FullRenderingThisFrame)) {
        Super::Render(RenderType::Type_Render);
        //UIManager << DL_Hint << "FULL" << endl;
    }
    // 部分刷新:
    else {
        //UIManager << DL_Hint << "DIRT" << endl;
#if 1
        // 先排序
        UIControl* units[LongUIDirtyControlSize];
        size_t length_for_units = 0;
        // 数据
        {
            assert(m_aUnitNow.length < LongUIDirtyControlSize);
            length_for_units = m_aUnitNow.length;
            ::memcpy(units, m_aUnitNow.units, length_for_units * sizeof(void*));
            // 一般就几个, 冒泡完爆std::sort
            LongUI::BubbleSort(units, units + length_for_units, [](UIControl* a, UIControl* b) noexcept {
                return a->priority > b->priority;
            });
            if (m_aUnitNow.length >= 2) {
                assert(units[0]->priority >= units[1]->priority);
            }
        }
        // 再渲染
        auto init_transfrom = D2D1::Matrix3x2F::Identity();
        for (auto unit = units; unit < units + length_for_units; ++unit) {
            auto ctrl = *unit;
            assert(ctrl != this);
            // 设置转换矩阵
            UIManager_RenderTarget->SetTransform(&init_transfrom);
            UIManager_RenderTarget->PushAxisAlignedClip(&ctrl->visible_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            // 设置转换矩阵
            UIManager_RenderTarget->SetTransform(&ctrl->world);
            // 正常渲染
            ctrl->Render(RenderType::Type_Render);
            // 回来
            UIManager_RenderTarget->PopAxisAlignedClip();
    }
#else
        // 再渲染
        for (uint32_t i = 0ui32; i < m_aUnitNow.length; ++i) {
            auto ctrl = m_aUnitNow.units[i];
            // 设置转换矩阵
            D2D1_MATRIX_3X2_F matrix; ctrl->GetWorldTransform(matrix);
            UIManager_RenderTarget->SetTransform(&matrix);
            ctrl->Render(RenderType::Type_Render);
        }
#endif
    }
    // 插入符号
    if (m_baBoolWindow.Test(Index_DoCaret) && m_baBoolWindow.Test(Index_CaretIn)) {
        UIManager_RenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        D2D1_RECT_F rect;
        rect.left = static_cast<float>(m_rcCaretPx.left);
        rect.top = static_cast<float>(m_rcCaretPx.top);
        rect.right = rect.left + static_cast<float>(m_rcCaretPx.width);
        rect.bottom = rect.top + static_cast<float>(m_rcCaretPx.height);
        UIManager_RenderTarget->PushAxisAlignedClip(&rect, D2D1_ANTIALIAS_MODE_ALIASED);
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        UIManager_RenderTarget->FillRectangle(&rect, m_pBrush_SetBeforeUse);
        UIManager_RenderTarget->PopAxisAlignedClip();
        UIManager_RenderTarget->SetTransform(&this->world);
    }
#ifdef _DEBUG
    // 调试输出
    if (this->debug_show) {
        D2D1_MATRIX_3X2_F nowMatrix, iMatrix = D2D1::Matrix3x2F::Scale(0.45f, 0.45f);
        UIManager_RenderTarget->GetTransform(&nowMatrix);
        UIManager_RenderTarget->SetTransform(&iMatrix);
        wchar_t buffer[1024];
        auto length = std::swprintf(
            buffer, 1024,
            L"Full Rendering Count: %d\nDirty Rendering Count: %d\nThis DirtyRectsCount:%d",
            int(full_render_counter),
            int(dirty_render_counter),
            int(m_aUnitNow.length)
            );
        auto tf = UIManager.GetTextFormat(LongUIDefaultTextFormatIndex);
        auto ta = tf->GetTextAlignment();
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        tf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        UIManager_RenderTarget->DrawText(
            buffer, length, tf,
            D2D1::RectF(0.f, 0.f, 1000.f, 70.f),
            m_pBrush_SetBeforeUse
            );
        tf->SetTextAlignment(ta);
        ::SafeRelease(tf);
        UIManager_RenderTarget->SetTransform(&nowMatrix);
    }
#endif
}

// UIWindow 事件处理
bool LongUI::UIWindow::DoEvent(const LongUI::EventArgument& _arg) noexcept {
    // 自己一般不处理LongUI事件
    if (_arg.sender) return Super::DoEvent(_arg);
    // 其他LongUI事件
    bool handled = false; UIControl* control_got = nullptr;
    // 特殊事件
    if (_arg.msg == s_uTaskbarBtnCreatedMsg) {
        ::SafeRelease(m_pTaskBarList);
        UIManager << DL_Log << "TaskbarButtonCreated" << endl;
        auto hr = ::CoCreateInstance(
            CLSID_TaskbarList,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pTaskBarList)
            );
        AssertHR(hr);
        return true;
    }
    // 处理事件
    switch (_arg.msg)
    {
        LongUI::EventArgument new_arg;
    case WM_SETCURSOR:
        // 设置光标
        ::SetCursor(now_cursor);
        break;
    /*case WM_DWMCOLORIZATIONCOLORCHANGED:
    {
        D2D_COLOR_F theme_color;
        CUIManager::GetThemeColor(theme_color);
    }
    break;*/
    case WM_MOUSEMOVE:
        handled = this->OnMouseMove(_arg);
        break;
    case WM_MOUSEWHEEL:
        handled = this->OnMouseWheel(_arg);
        break;
    case WM_TIMER:
        // 闪烁?
        if (_arg.sys.wParam == BLINK_EVENT_ID) {
            if (m_cShowCaret) {
                m_baBoolWindow.SetNot(Index_CaretIn);
                m_baBoolWindow.SetTrue(Index_DoCaret);
            }
            handled = true;
        }
        break;
    case WM_LBUTTONDOWN:    // 按下鼠标左键
        // 查找子控件
        control_got = this->FindControl(_arg.pt);
        // 控件有效
        if (control_got && control_got != m_pFocusedControl) {
            new_arg = _arg;
            new_arg.sender = this;
            if (m_pFocusedControl){
                new_arg.event = LongUI::Event::Event_KillFocus;
                m_pFocusedControl->DoEvent(new_arg);
            }
            new_arg.event = LongUI::Event::Event_SetFocus;
            // 控件响应了?
            m_pFocusedControl = control_got->DoEvent(new_arg) ? control_got : nullptr;
        }
        break;
    /*case WM_NCHITTEST:
        _arg.lr = HTCAPTION;
        handled = true;
        break;*/
    case WM_SETFOCUS:
        ::CreateCaret(m_hwnd, nullptr, 1, 1);
        handled = true;
        break;
    case WM_KILLFOCUS:
        // 存在焦点控件
        if (m_pFocusedControl){
            new_arg = _arg;
            new_arg.sender = this;
            new_arg.event = LongUI::Event::Event_KillFocus;
            m_pFocusedControl->DoEvent(new_arg);
            m_pFocusedControl = nullptr;
        }
        ::DestroyCaret();
        handled = true;
        break;
    case WM_MOUSELEAVE:     // 鼠标移出窗口
        if (m_pPointedControl){
            new_arg = _arg;
            new_arg.sender = this;
            new_arg.event = LongUI::Event::Event_MouseLeave;
            m_pPointedControl->DoEvent(new_arg);
            m_pPointedControl = nullptr;
        }
        handled = true;
        break;
    case WM_SIZE:           // 改变大小
    {
        uint32_t wwidth, wheight;
        {
            RECT rect; ::GetClientRect(m_hwnd, &rect);
            wwidth = rect.right - rect.left;
            wheight = rect.bottom - rect.top;
        }
        // 数据有效?
        if (wwidth && wheight && (wwidth != this->window_size.width ||
            wheight != this->window_size.height)) {
            force_cast(this->window_size.width) = wwidth;
            force_cast(this->window_size.height) = wheight;
            m_baBoolWindow.SetTrue(Index_NewSize);
        }
    }
        handled = true;
        break;
    case WM_GETMINMAXINFO:  // 获取限制大小
        reinterpret_cast<MINMAXINFO*>(_arg.sys.lParam)->ptMinTrackSize.x = m_miniSize.width;
        reinterpret_cast<MINMAXINFO*>(_arg.sys.lParam)->ptMinTrackSize.y = m_miniSize.height;
        break;
    case WM_DISPLAYCHANGE:
        UIManager << DL_Hint << "WM_DISPLAYCHANGE" << endl;
        {
            // 获取屏幕刷新率
            DEVMODEW mode = { 0 };
            ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
            m_uiRenderQueue.Reset(mode.dmDisplayFrequency);
        }
        // 强行刷新一帧
        this->Invalidate(this);
        break;
    case WM_CLOSE:          // 关闭窗口
        // 窗口关闭
        handled = this->OnClose();
        break;
    }
    // 处理
    if (handled) return true;
    // 处理控件
    register UIControl* processor = nullptr;
    // 鼠标事件交由捕获控件(优先)或者鼠标指向控件处理
    if (_arg.msg >= WM_MOUSEFIRST && _arg.msg <= WM_MOUSELAST) {
        processor = m_pCapturedControl ? m_pCapturedControl : m_pPointedControl;
    }
    // 其他事件交由焦点控件处理
    else {
        processor = m_pFocusedControl;
    }
    // 有就处理
    if (processor && processor->DoEvent(_arg)) {
        return true;
    }
    // 还是没有处理就交给父类处理
    return Super::DoEvent(_arg);
}

// 重置窗口大小
void LongUI::UIWindow::OnResize(bool force) noexcept {
    assert(this->window_type != Type_RenderOnParent);
    if (this->window_type != Type_Layered) {
        force = true;
    }
    //UIManager << DL_Log << "called" << endl;
    // 修改大小, 需要取消目标
    UIManager_RenderTarget->SetTarget(nullptr);
    // 修改
    visible_rect.right = static_cast<float>(this->window_size.width);
    visible_rect.bottom = static_cast<float>(this->window_size.height);
    this->SetWidth(visible_rect.right / m_2fZoom.width);
    this->SetHeight(visible_rect.bottom / m_2fZoom.height);
    // 设置
    auto rect_right = LongUI::MakeAsUnit(this->window_size.width);
    auto rect_bottom = LongUI::MakeAsUnit(this->window_size.height);
    if (force) {
        rect_right = this->window_size.width;
        rect_bottom = this->window_size.height;
    }
    auto old_size = m_pTargetBimtap->GetPixelSize();
    register HRESULT hr = S_OK;
    // 强行 或者 小于才Resize
    if (force || old_size.width < uint32_t(rect_right) || old_size.height < uint32_t(rect_bottom)) {
        UIManager << DL_Hint << L"Window: [" << this->GetNameStr() << L"] \r\n\t\tTarget Bitmap Resize to " 
            << long(rect_right) << ", " << long(rect_bottom) << LongUI::endl;
        IDXGISurface* pDxgiBackBuffer = nullptr;
        ::SafeRelease(m_pTargetBimtap);
        hr = m_pSwapChain->ResizeBuffers(
            2, rect_right, rect_bottom, DXGI_FORMAT_B8G8R8A8_UNORM, 
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
            );
        // 检查
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            UIManager.RecreateResources();
            UIManager << DL_Hint << L"Recreate device" << LongUI::endl;
        }
        // 利用交换链获取Dxgi表面
        if (SUCCEEDED(hr)) {
            hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
        }
        // 利用Dxgi表面创建位图
        if (SUCCEEDED(hr)) {
            D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                LongUI::GetDpiX(),
                LongUI::GetDpiY()
                );
            hr = UIManager_RenderTarget->CreateBitmapFromDxgiSurface(
                pDxgiBackBuffer,
                &bitmapProperties,
                &m_pTargetBimtap
                );
        }
        // 重建失败?
        if (FAILED(hr)) {
            UIManager << DL_Error << L" Recreate FAILED!" << LongUI::endl;
            AssertHR(hr);
        }
        ::SafeRelease(pDxgiBackBuffer);
    }
    // 强行刷新一帧
    this->Invalidate(this);
}

// UIWindow 重建
auto LongUI::UIWindow::Recreate() noexcept ->HRESULT {
    this->release_data();
    // DXGI Surface 后台缓冲
    IDXGISurface*                       pDxgiBackBuffer = nullptr;
    IDXGISwapChain1*                    pSwapChain = nullptr;
    // 创建交换链
    HRESULT hr = S_OK;
    // 创建交换链
    if (SUCCEEDED(hr)) {
        RECT rect = { 0 }; ::GetClientRect(m_hwnd, &rect);
        // 交换链信息
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.Width = LongUI::MakeAsUnit(rect.right - rect.left);
        swapChainDesc.Height = LongUI::MakeAsUnit(rect.bottom - rect.top);
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        // XXX: Fixit
        if (this->window_type == Type_Layered) {
            // DirectComposition桌面应用程序
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            // 创建DirectComposition交换链
            hr = UIManager_DXGIFactory->CreateSwapChainForComposition(
                UIManager_DXGIDevice,
                &swapChainDesc,
                nullptr,
                &pSwapChain
                );
        }
        else {
            // 一般桌面应用程序
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            // 利用窗口句柄创建交换链
            hr = UIManager_DXGIFactory->CreateSwapChainForHwnd(
                UIManager_D3DDevice,
                m_hwnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                &pSwapChain
                );
        }
    }
    // 获取交换链V2
    if (SUCCEEDED(hr)) {
#ifdef LONGUI_USE_SDK_8_1
#define DEFINE_GUID_LONGUI(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name  = { l, w1, w2,{ b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
        DEFINE_GUID_LONGUI(IID_IDXGISwapChain2_Auto, 0xa8be2ac4, 0x199f, 0x4946, 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7);
        hr = pSwapChain->QueryInterface(
            IID_IDXGISwapChain2_Auto,
            reinterpret_cast<void**>(&m_pSwapChain)
            );
#else
        hr = pSwapChain->QueryInterface(
            IID_IDXGISwapChain2,
            reinterpret_cast<void**>(&m_pSwapChain)
            );
#endif
    }
    // 获取垂直等待事件
    if (SUCCEEDED(hr)) {
        m_hVSync = m_pSwapChain->GetFrameLatencyWaitableObject();
    }
    /*// 确保DXGI队列里边不会超过一帧
    if (SUCCEEDED(hr)) {
        hr = UIManager_DXGIDevice->SetMaximumFrameLatency(1);
    }*/
    // 利用交换链获取Dxgi表面
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
    }
    // 利用Dxgi表面创建位图
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            LongUI::GetDpiX(),
            LongUI::GetDpiY()
            );
        hr = UIManager_RenderTarget->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pTargetBimtap
            );
    }
    // 使用DComp
    if (this->window_type == Type_Layered) {
        // 创建直接组合(Direct Composition)设备
        if (SUCCEEDED(hr)) {
            hr = LongUI::Dll::DCompositionCreateDevice(
                UIManager_DXGIDevice,
                LongUI_IID_PV_ARGS(m_pDcompDevice)
                );
        }
        // 创建直接组合(Direct Composition)目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateTargetForHwnd(
                m_hwnd, true, &m_pDcompTarget
                );
        }
        // 创建直接组合(Direct Composition)视觉
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateVisual(&m_pDcompVisual);
        }
        // 设置当前交换链为视觉内容
        if (SUCCEEDED(hr)) {
            hr = m_pDcompVisual->SetContent(m_pSwapChain);
        }
        // 设置当前视觉为窗口目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompTarget->SetRoot(m_pDcompVisual);
        }
        // 向系统提交
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->Commit();
        }
    }
    // 错误
    if (FAILED(hr)){
        UIManager << L"Recreate Failed!" << LongUI::endl;
        AssertHR(hr);
    }
    ::SafeRelease(pDxgiBackBuffer);
    ::SafeRelease(pSwapChain);
    {
        // 获取屏幕刷新率
        DEVMODEW mode = { 0 };
        ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
        m_uiRenderQueue.Reset(mode.dmDisplayFrequency);
        // 强行刷新一帧
        this->Invalidate(this);
    }
    // 重建 子控件UI
    return Super::Recreate();
}

// UIWindow 关闭控件
void LongUI::UIWindow::Cleanup() noexcept {
    // 删除对象
    delete this;
}

// 窗口创建时
bool LongUI::UIWindow::OnCreated(HWND hwnd) noexcept {
    // 权限提升?保证
    CHANGEFILTERSTRUCT cfs = { sizeof(CHANGEFILTERSTRUCT) };
    ::ChangeWindowMessageFilterEx(hwnd, s_uTaskbarBtnCreatedMsg, MSGFLT_ALLOW, &cfs);
    return true;
}

// 鼠标移动时候
bool LongUI::UIWindow::OnMouseMove(const LongUI::EventArgument& arg) noexcept {
    bool handled = false;
    do {
        ::TrackMouseEvent(&m_csTME);
        if (m_normalLParam != arg.sys.lParam) {
            m_normalLParam = arg.sys.lParam;
        }
        else {
            handled = true;
            break;
        }
        // 有待捕获控件
        if (m_pCapturedControl) {
            m_pCapturedControl->DoEvent(arg);
            handled = true;
            break;
        }
        // 查找子控件
        auto control_got = this->FindControl(arg.pt);
        if (control_got) {
            //UIManager << DL_Hint << "FIND: " << control_got << endl;
        }
        // 不同
        if (control_got != m_pPointedControl) {
            auto new_arg = arg;
            new_arg.sender = this;
            // 有效
            if (m_pPointedControl) {
                new_arg.event = LongUI::Event::Event_MouseLeave;
                m_pPointedControl->DoEvent(new_arg);
            }
            // 有效
            if ((m_pPointedControl = control_got)) {
                new_arg.event = LongUI::Event::Event_MouseEnter;
                m_pPointedControl->DoEvent(new_arg);
            }
        }
        // 相同
        else if (control_got) {
            control_got->DoEvent(arg);
        }
        handled = true;
    } while (false);
    return handled;
}

// 鼠标滚轮
bool LongUI::UIWindow::OnMouseWheel(const LongUI::EventArgument& arg) noexcept {
    auto loww = LOWORD(arg.sys.wParam);
    //auto delta = float(GET_WHEEL_DELTA_WPARAM(arg.sys.wParam)) / float(WHEEL_DELTA);
    // 鼠标滚轮事件交由有滚动条的容器处理
    if (loww & MK_CONTROL) {

    }
    // Alt + wheel?
    else if (loww & MK_ALT) {

    }
    // 水平滚动条
    else if (loww & MK_SHIFT) {
        /*auto basic_control = this->FindControl(arg.pt);
        if (basic_control) {
            // 获取滚动条容器
            while (true) {
                if (basic_control->IsTopLevel()) {
                    break;
                }
                if (basic_control->flags & Flag_UIContainer) {
                    if (static_cast<UIContainer*>(basic_control)->scrollbar_h) {
                        break;
                    }
                }
                basic_control = basic_control->parent;
            }
            // 存在
            if (static_cast<UIContainer*>(basic_control)->scrollbar_h) {
                static_cast<UIContainer*>(basic_control)->scrollbar_h->OnWheelX(-delta);
            }
        }*/
    }
    // 垂直滚动条
    else {
        /*auto basic_control = this->FindControl(arg.pt);
        if (basic_control) {
            // 获取滚动条容器
            while (true) {
                if (basic_control->IsTopLevel()) {
                    break;
                }
                if (basic_control->flags & Flag_UIContainer) {
                    if (static_cast<UIContainer*>(basic_control)->scrollbar_v) {
                        break;
                    }
                }
                basic_control = basic_control->parent;
            }
            // 存在
            if (static_cast<UIContainer*>(basic_control)->scrollbar_v) {
                static_cast<UIContainer*>(basic_control)->scrollbar_v->OnWheelX(-delta);
            }
        }*/
    }
    return true;
}

// ----------------- IDropTarget!!!! Yooooooooooo~-----

// 设置参数
void __fastcall SetLongUIEventArgument(LongUI::EventArgument& arg, HWND hwnd, POINTL pt) {
    // 获取窗口位置
    RECT rc = { 0 }; ::GetWindowRect(hwnd, &rc);
    // 映射到窗口坐标
    POINT ppt = { pt.x, pt.y };  ::ScreenToClient(hwnd, &ppt);
    // 查找对应控件
    arg = { 0 };
    arg.pt.x = static_cast<float>(ppt.x);
    arg.pt.y = static_cast<float>(ppt.y);

}

// 获取拖放效果
DWORD __fastcall GetDropEffect(DWORD grfKeyState, DWORD dwAllowed) {
    register DWORD dwEffect = 0;
    // 1. 检查pt来看是否允许drop操作在某个位置
    // 2. 计算出基于grfKeyState的drop效果
    if (grfKeyState & MK_CONTROL) {
        dwEffect = dwAllowed & DROPEFFECT_COPY;
    }
    else if (grfKeyState & MK_SHIFT) {
        dwEffect = dwAllowed & DROPEFFECT_MOVE;
    }
    // 3. 非键盘修饰符指定(或drop效果不允许), 因此基于drop源的效果
    if (dwEffect == 0) {
        if (dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
        if (dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
    }
    return dwEffect;
}

// IDropTarget::DragEnter 实现
HRESULT  LongUI::UIWindow::DragEnter(IDataObject* pDataObj,
    DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) noexcept {
    UNREFERENCED_PARAMETER(grfKeyState);
    m_baBoolWindow.SetTrue(Index_InDraging);
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    // 取消聚焦窗口
    if(m_pFocusedControl){
        LongUI::EventArgument arg = { 0 };
        arg.sender = this;
        arg.event = LongUI::Event::Event_KillFocus;
        m_pFocusedControl->DoEvent(arg);
        m_pFocusedControl = nullptr;
    }
    // 保留数据
    ::SafeRelease(m_pCurDataObject);
    m_pCurDataObject = ::SafeAcquire(pDataObj);
    // 由帮助器处理
    POINT ppt = { pt.x, pt.y };
    if (m_pDropTargetHelper) {
        m_pDropTargetHelper->DragEnter(m_hwnd, pDataObj, &ppt, *pdwEffect);
    }
    return S_OK;
}


// IDropTarget::DragOver 实现
HRESULT LongUI::UIWindow::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    UNREFERENCED_PARAMETER(grfKeyState);
    D2D1_POINT_2F pt2f = { static_cast<float>(pt.x), static_cast<float>(pt.y) };
    UIControl* control = nullptr;
    // 检查控件支持
    if ((control = this->FindControl(pt2f))) {
        LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
        arg.sender = this;
        // 第一个控件?
        if (m_pDragDropControl == control) {
            // 一样就是Over
            arg.event = LongUI::Event::Event_DragOver;
        }
        else {
            // 对老控件发送离开事件
            if (m_pDragDropControl) {
                arg.event = LongUI::Event::Event_DragLeave;
                m_pDragDropControl->DoEvent(arg);
            }
            // 新控件发送进入
            arg.event = LongUI::Event::Event_DragEnter;
            m_pDragDropControl = control;
        }
        arg.cf.dataobj = m_pCurDataObject;
        arg.cf.outeffect = pdwEffect;
        if (!control->DoEvent(arg)) *pdwEffect = DROPEFFECT_NONE;
    }
    else {
        // 不支持
        *pdwEffect = DROPEFFECT_NONE;
    }
    // 由帮助器处理
    if (m_pDropTargetHelper) {
        POINT ppt = { pt.x, pt.y };
        m_pDropTargetHelper->DragOver(&ppt, *pdwEffect);
    }
    return S_OK;
}

// IDropTarget::DragLeave 实现
HRESULT LongUI::UIWindow::DragLeave(void) noexcept {
    // 发送事件
    if (m_pDragDropControl) {
        LongUI::EventArgument arg = { 0 };
        arg.sender = this;
        arg.event = LongUI::Event::Event_DragLeave;
        m_pDragDropControl->DoEvent(arg);
        m_pDragDropControl = nullptr;
        // 存在捕获控件?
        /*if (m_pCapturedControl) {
            this->ReleaseCapture();
            /*arg.sender = nullptr;
            arg.msg = WM_LBUTTONUP;
            m_pCapturedControl->DoEvent(arg);
        }*/
    }
    /*OnDragLeave(m_hTargetWnd);*/
    m_pDragDropControl = nullptr;
    //m_isDataAvailable = TRUE;
    if (m_pDropTargetHelper) {
        m_pDropTargetHelper->DragLeave();
    }
    m_baBoolWindow.SetFalse(Index_InDraging);
    return S_OK;
}

// IDropTarget::Drop 实现
HRESULT LongUI::UIWindow::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept {
    // 发送事件
    if (m_pDragDropControl) {
        LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
        arg.sender = this;
        arg.event = LongUI::Event::Event_Drop;
        arg.cf.dataobj = m_pCurDataObject;
        arg.cf.outeffect = pdwEffect;
        // 发送事件
        m_pDragDropControl->DoEvent(arg);
        m_pDragDropControl = nullptr;
        
    }
    // 检查参数
    if (!pDataObj) return E_INVALIDARG;
    if (m_pDropTargetHelper){
        POINT ppt = { pt.x, pt.y };
        m_pDropTargetHelper->Drop(pDataObj, &ppt, *pdwEffect);
    }
    *pdwEffect = ::GetDropEffect(grfKeyState, *pdwEffect);
    return S_OK;
}

