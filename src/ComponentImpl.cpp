#include "LongUI.h"
#include <algorithm>

// -------------------- LongUI::Component::ShortText --------------------
// ShortText 构造函数
LongUI::Component::ShortText::ShortText(pugi::xml_node node, const char* prefix) noexcept
    : m_pTextRenderer(nullptr) {
    // 设置
    m_config = {
        nullptr,
        128.f, 64.f, 1.f,
        Helper::XMLGetRichType(node, RichType::Type_None, "richtype", prefix),
        0
    };
    // 有效结点
    if (node) {
        // 检查参数
        assert(prefix && "bad arguments");
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
    }
    // 没有?
    else {
        char name[2]; name[0] = '0'; name[1] = 0;
        m_pTextRenderer = UIManager.GetTextRenderer(name);
        m_config.format = UIManager.GetTextFormat(0);
    }
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
        auto hr = UIManager_DWriteFactory->CreateTextLayout(
            m_text.c_str(),
            string_length_need,
            old_layout ? old_layout : m_config.format,
            m_config.width,
            m_config.height,
            &m_pLayout
            );
        assert(SUCCEEDED(hr) && m_pLayout);
        m_config.text_length = static_cast<decltype(m_config.text_length)>(m_text.length());
        break;
    }
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
void LongUI::Component::EditaleText::recreate_layout(IDWriteTextFormat* fmt) noexcept {
    assert(fmt && "bad argument");
    assert(this->layout == nullptr && "bad action");
    // 创建布局
    auto hr = UIManager_DWriteFactory->CreateTextLayout(
        m_string.c_str(), static_cast<uint32_t>(m_string.length()),
        fmt,
        m_size.width, m_size.height,
        &this->layout
        );
#ifdef _DEBUG
    if (m_pHost->debug_this) {
        UIManager << DL_Hint << L"CODE: " << long(hr) << LongUI::endl;
        assert(hr == S_OK);
    }
#endif
    ShowHR(hr);
}

// 插入字符(串)
auto LongUI::Component::EditaleText::insert(
    uint32_t pos, const wchar_t * str, uint32_t length) noexcept -> HRESULT {
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
            //this->sbcaller.operator()(m_pHost, SubEvent::Event_EditReturned);
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
            if (this->remove_text(hitTestMetrics.textPosition, hitTestMetrics.length)) {
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
    assert(this->layout && "bad action");
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
    // 有效
    if (selection.length) {
        // 打开剪切板
        if (::OpenClipboard(m_pHost->GetWindow()->GetHwnd())) {
            // 清空
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
    IDWriteTextFormat* tmpfmt = nullptr;
    {
        uint32_t format_index = LongUIDefaultTextFormatIndex;
        if ((str = attribute("format"))) {
            format_index = static_cast<uint32_t>(LongUI::AtoI(str));
        }
        tmpfmt = UIManager.GetTextFormat(format_index);
    }
    // 格式特化
    {
        assert(tmpfmt && "bad action");
        /*IDWriteTextLayout* tmplayout = nullptr;
        auto hr = UIManager_DWriteFactory->CreateTextLayout(
            L"", 0,
            tmpfmt,
            1.f, 1.f, &tmplayout
            );
        ::SafeRelease(tmplayout);*/
        // 1. 字体名称
        CUIString fontfamilyname(LongUIDefaultTextFontName);
        DWRITE_FONT_WEIGHT fontweight = DWRITE_FONT_WEIGHT_NORMAL;
        float fontsize = LongUIDefaultTextFontSize;
        // 获取字体名称
        auto changed = Helper::MakeString(attribute("family"), fontfamilyname);
        // 获取字体大小
        if (str = attribute("size")) {
            changed = true;
            fontsize = LongUI::AtoF(str);
        }
        // 获取字体粗细
        if (str = attribute("weight")) {
            changed = true;
            fontweight = static_cast<DWRITE_FONT_WEIGHT>(LongUI::AtoI(str));
        }
        // 字体风格
        auto style = Helper::XMLGetFontStyle(node, DWRITE_FONT_STYLE_NORMAL, "style", prefix);
        // 检查
        changed = style != DWRITE_FONT_STYLE_NORMAL || changed;
        // 字体拉伸
        auto stretch = Helper::XMLGetFontStretch(node, DWRITE_FONT_STRETCH_NORMAL, "stretch", prefix);
        // 检查
        changed = stretch != DWRITE_FONT_STRETCH_NORMAL || changed;
        // Tab宽度
        float tabstop = fontsize * 4.f;
        // 检查Tab宽度
        if (str = attribute("tabstop")) {
            tabstop = LongUI::AtoF(str);
            changed = true;
        }
        // 段落排列方向
        auto direction = Helper::XMLGetFlowDirection(node, DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM, "flowdirection", prefix);
        // 段落(垂直)对齐
        auto valign = Helper::XMLGetVAlignment(node, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        // 文本(水平)对齐
        auto halign = Helper::XMLGetHAlignment(node, DWRITE_TEXT_ALIGNMENT_LEADING);
        // 阅读进行方向
        textformat->SetReadingDirection(
            Helper::XMLGetReadingDirection(node, DWRITE_READING_DIRECTION_LEFT_TO_RIGHT)
            );
        // 设置自动换行
        textformat->SetWordWrapping(
            Helper::XMLGetWordWrapping(node, DWRITE_WORD_WRAPPING_NO_WRAP)
            );
    }
    // 获取文本
    {
        if (str = node.attribute(prefix).value()) {
            m_string.assign(str);
        }
    }
    // 创建布局
    this->recreate_layout(tmpfmt);
    ::SafeRelease(tmpfmt);
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