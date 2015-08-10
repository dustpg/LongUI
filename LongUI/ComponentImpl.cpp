#include "LongUI.h"

// -------------------- LongUI::Component::Text --------------------

// Text 构造函数
LongUI::Component::Text::Text(pugi::xml_node node, const char * prefix) noexcept
    : m_pTextRenderer(nullptr) {
    m_config = {
        ::SafeAcquire(UIManager_DWriteFactory),
        nullptr,
        UIManager.inline_handler,
        128.f, 64.f, 1.f, 0
    };
    // 检查参数
    assert(node && prefix && "bad arguments");
    register union { const char* str; const uint32_t* pui32; };
    str = nullptr;
    char attribute_buffer[256];
    // 设置新的属性
    auto set_new_attribute = [prefix](char* buffer, const char* suffix) noexcept {
        ::strcpy(buffer, prefix); ::strcat(buffer, suffix);
    };
    // 获取进度
    {
        set_new_attribute(attribute_buffer, "progress");
        if ((str = node.attribute(attribute_buffer).value())) {
            m_config.progress = LongUI::AtoF(str);
        }
    }
    // 获取渲染器
    {
        int renderer_index = Type_NormalTextRenderer;
        set_new_attribute(attribute_buffer, "renderer");
        if ((str = node.attribute(attribute_buffer).value())) {
            renderer_index = LongUI::AtoI(str);
        }
        auto renderer = UIManager.GetTextRenderer(renderer_index);
        m_pTextRenderer = renderer;
        // 保证缓冲区
        if (renderer) {
            auto length = renderer->GetContextSizeInByte();
            if (length) {
                set_new_attribute(attribute_buffer, "context");
                if ((str = node.attribute(attribute_buffer).value())) {
                    m_buffer.NewSize(length);
                    renderer->CreateContextFromString(m_buffer.data, str);
                }
            }
        }
    }
    {
        // 检查基本颜色
        m_basicColor = D2D1::ColorF(D2D1::ColorF::Black);
        set_new_attribute(attribute_buffer, "color");
        UIControl::MakeColor(node.attribute(attribute_buffer).value(), m_basicColor);
    }
    {
        // 检查格式
        uint32_t format_index = 0;
        set_new_attribute(attribute_buffer, "format");
        if ((str = node.attribute(attribute_buffer).value())) {
            format_index = static_cast<uint32_t>(LongUI::AtoI(str));
        }
        m_config.text_format = UIManager.GetTextFormat(format_index);
    }
    {
        // 检查类型
        this->SetIsRich(false);
        set_new_attribute(attribute_buffer, "type");
        if (str = node.attribute(attribute_buffer).value()) {
            switch (*pui32)
            {
            case "xml"_longui32:
            case "XML"_longui32:
                this->SetIsRich(true);
                this->SetIsXML(true);
                break;
            case "core"_longui32:
            case "Core"_longui32:
            case "CORE"_longui32:
                this->SetIsRich(true);
                this->SetIsXML(false);
                break;
            default:
                this->SetIsRich(false);
                this->SetIsXML(false);
                break;
            }
        }
    }
    // 重建
    this->recreate(node.attribute(prefix).value());
}

// Text = L"***"
LongUI::Component::Text& LongUI::Component::Text::operator=(const wchar_t* new_string) noexcept {
    // 不能是XML模式
    assert(this->GetIsXML() == false && "=(const wchar_t*) must be in core-mode, can't be xml-mode");
    m_text.Set(new_string);
    this->recreate();
    return *this;
}

// Text = "***"
LongUI::Component::Text& LongUI::Component::Text::operator=(const char* str) noexcept {
    if (this->GetIsXML()) {
        this->recreate(str);
        return *this;
    }
    else {
        wchar_t buffer[LongUIStringBufferLength];
        buffer[LongUI::UTF8toWideChar(str, buffer)] = L'\0';
        return this->operator=(buffer);
    }
}


// Text 析构
LongUI::Component::Text::~Text() noexcept {
    m_pTextRenderer.SafeRelease();
    ::SafeRelease(m_pLayout);
    ::SafeRelease(m_config.dw_factory);
    ::SafeRelease(m_config.text_format);
}

void LongUI::Component::Text::recreate(const char* utf8) noexcept {
    wchar_t text_buffer[LongUIStringBufferLength];
    // 转换为核心模式
    if (this->GetIsXML() && this->GetIsRich()) {
        LongUI::DX::XMLToCoreFormat(utf8, text_buffer);
    }
    else if (utf8) {
        // 直接转码
        register auto length = LongUI::UTF8toWideChar(utf8, text_buffer);
        text_buffer[length] = L'\0';
        m_text.Set(text_buffer, length);
    }
    // 创建布局
    ::SafeRelease(m_pLayout);
    // 富文本
    if (this->GetIsRich()) {
        m_pLayout = LongUI::DX::FormatTextCore(
            m_config,
            m_text.c_str(),
            nullptr
            );
    }
    // 平台文本
    else {
        register auto string_length_need = static_cast<uint32_t>(static_cast<float>(m_text.length() + 1) *
            m_config.progress);
        LongUIClamp(string_length_need, 0, m_text.length());
        m_config.dw_factory->CreateTextLayout(
            m_text.c_str(),
            string_length_need,
            m_config.text_format,
            m_config.width,
            m_config.height,
            &m_pLayout
            );
        m_config.text_length = m_text.length();
    }
}


// -------------------- LongUI::Component::EditaleText --------------------

// LongUI 命名空间
namespace LongUI {
    // Component 命名空间
    namespace Component {
        // 移除字符串
        auto __fastcall RemoveText(
            LongUI::DynamicString& text,
            uint32_t pos,
            uint32_t len,
            bool readonly
            ) noexcept {
            HRESULT hr = S_OK;
            // 只读?
            if (readonly) {
                hr = S_FALSE;
                ::MessageBeep(MB_ICONERROR);
            }
            else {
                try { text.erase(pos, len); } CATCH_HRESULT(hr);
            }
            return hr;
        }
    }
}

// DWrite部分代码参考: 
// http://msdn.microsoft.com/zh-cn/library/windows/desktop/dd941792(v=vs.85).aspx



// 刷新布局
auto LongUI::Component::EditaleText::refresh(bool update) const noexcept ->UIWindow* {
    if (!m_bThisFocused) return nullptr;
    RectLTWH_F rect; this->GetCaretRect(rect);
    register auto* window = m_pHost->GetWindow();
    window->CreateCaret(rect.width, rect.height);
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
    m_pFactory->CreateTextLayout(
        m_text.c_str(), static_cast<uint32_t>(m_text.length()),
        m_pBasicFormat,
        m_size.width, m_size.height,
        &this->layout
        );
}

// 插入字符(串)
auto LongUI::Component::EditaleText::insert(
    const wchar_t * str, uint32_t pos, uint32_t length) noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 只读
    if (this->IsReadOnly()) {
        ::MessageBeep(MB_ICONERROR);
        return S_FALSE;
    }
    auto old_length = static_cast<uint32_t>(m_text.length());
    // 插入字符
    try { m_text.insert(pos, str, length); }
    CATCH_HRESULT(hr);
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
        Component::EditaleText::CopySinglePropertyRange(old_layout, old_length, this->layout, static_cast<uint32_t>(m_text.length()), UINT32_MAX);
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
    auto textLength = static_cast<uint32_t>(m_text.size());
    caretBegin = std::min(caretBegin, textLength);
    caretEnd = std::min(caretEnd, textLength);
    // 返回范围
    return{ caretBegin, caretEnd - caretBegin };
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
                && absolute_position < m_text.size()
                && m_text[absolute_position - 1] == '\r'
                &&  m_text[absolute_position] == '\n')
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
            && absolute_position < m_text.size()
            && m_text[absolute_position - 1] == '\r'
            &&  m_text[absolute_position] == '\n')
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
        EzContainer::SimpleSmallBuffer<DWRITE_LINE_METRICS, 10> metrice_buffer;
        // 获取行指标
        this->layout->GetMetrics(&textMetrics);
        metrice_buffer.NewSize(textMetrics.lineCount);
        this->layout->GetLineMetrics(
            metrice_buffer.data,
            textMetrics.lineCount,
            &textMetrics.lineCount
            );
        // 获取行
        uint32_t line, linePosition;
        Component::EditaleText::GetLineFromPosition(
            metrice_buffer.data,
            metrice_buffer.data_length,
            m_u32CaretPos,
            &line,
            &linePosition
            );
        // 下移或上移
        if (mode == SelectionMode::Mode_Up) {
            if (line <= 0) break;
            line--;
            linePosition -= metrice_buffer.data[line].length;
        }
        else {
            linePosition += metrice_buffer.data[line].length;
            line++;
            if (line >= metrice_buffer.data_length)  break;
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
        EzContainer::SimpleSmallBuffer<DWRITE_CLUSTER_METRICS, 64> metrice_buffer;
        UINT32 clusterCount;
        this->layout->GetClusterMetrics(nullptr, 0, &clusterCount);
        if (clusterCount == 0) break;
        // 重置大小
        metrice_buffer.NewSize(clusterCount);
        this->layout->GetClusterMetrics(metrice_buffer.data, clusterCount, &clusterCount);
        m_u32CaretPos = absolute_position;
        UINT32 clusterPosition = 0;
        UINT32 oldCaretPosition = m_u32CaretPos;
        // 左移
        if (mode == SelectionMode::Mode_LeftWord) {
            m_u32CaretPos = 0;
            m_u32CaretPosOffset = 0; // leading edge
            for (UINT32 cluster = 0; cluster < clusterCount; ++cluster) {
                clusterPosition += metrice_buffer.data[cluster].length;
                if (metrice_buffer.data[cluster].canWrapLineAfter) {
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
                UINT32 clusterLength = metrice_buffer.data[cluster].length;
                m_u32CaretPos = clusterPosition;
                m_u32CaretPosOffset = clusterLength; // trailing edge
                if (clusterPosition >= oldCaretPosition &&
                    metrice_buffer.data[cluster].canWrapLineAfter) {
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
        EzContainer::SimpleSmallBuffer<DWRITE_LINE_METRICS, 10> metrice_buffer;
        // 获取行指标
        this->layout->GetMetrics(&textMetrics);
        metrice_buffer.NewSize(textMetrics.lineCount);
        this->layout->GetLineMetrics(
            metrice_buffer.data,
            textMetrics.lineCount,
            &textMetrics.lineCount
            );
        uint32_t line;
        Component::EditaleText::GetLineFromPosition(
            metrice_buffer.data,
            metrice_buffer.data_length,
            m_u32CaretPos,
            &line,
            &m_u32CaretPos
            );
        m_u32CaretPosOffset = 0;
        if (mode == SelectionMode::Mode_End) {
            // 放置插入符号
            UINT32 lineLength = metrice_buffer.data[line].length -
                metrice_buffer.data[line].newlineLength;
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
    HRESULT hr = S_FALSE;
    DWRITE_TEXT_RANGE selection = this->GetSelectionRange();
    if (selection.length == 0 || this->IsReadOnly()) return hr;
    // 删除
    hr = LongUI::Component::RemoveText(
        m_text, selection.startPosition, selection.length,
        this->IsReadOnly()
        );
    // 成功的话设置选择区
    if (hr == S_OK) {
        hr = this->SetSelection(Mode_Leading, selection.startPosition, false);
    }
    return hr;
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
    // 检查支持格式: Unicode-Text
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
void LongUI::Component::EditaleText::Recreate(ID2D1RenderTarget* target) noexcept {
    // 无效参数
    assert(target); if (!target) return;
    // 重新创建资源
    ::SafeRelease(m_pRenderTarget);
    ::SafeRelease(m_pSelectionColor);
    m_pRenderTarget = ::SafeAcquire(target);
    target->CreateSolidColorBrush(
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
        this->insert(chars, m_u32CaretPos + m_u32CaretPosOffset, length);
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
            this->insert(L"\r\n", absolutePosition, 2);
            this->SetSelection(SelectionMode::Mode_Leading, absolutePosition + 2, false, false);
            // 修改
            this->refresh();
        }
        // 单行 - 向窗口发送输入完毕消息
        else {
            LongUI::EventArgument arg = { 0 };
            arg.event = LongUI::Event::Event_EditReturn;
            arg.sender = m_pHost;
            m_pHost->GetWindow()->DoEvent(arg);
            // TODO: single line
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
            if (absolutePosition >= 2 && absolutePosition <= m_text.size()) {
                auto* __restrict base = m_text.data() + absolutePosition;
                if (base[-2] == L'\r' && base[-1] == L'\n') {
                    count = 2;
                }
            }
            // 左移
            this->SetSelection(SelectionMode::Mode_LeftChar, count, false);
            // 字符串: 删除count个字符
            if (LongUI::Component::RemoveText(m_text, m_u32CaretPos, count, this->IsReadOnly()) == S_OK) {
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
            if (hitTestMetrics.textPosition + 2 < m_text.length()) {
                auto* __restrict base = m_text.data() + hitTestMetrics.textPosition;
                if (base[0] == L'\r' && base[1] == L'\n') {
                    ++hitTestMetrics.length;
                }
            }
            // 修改
            this->SetSelection(SelectionMode::Mode_Leading, hitTestMetrics.textPosition, false);
            // 删除字符
            if (LongUI::Component::RemoveText(m_text,
                hitTestMetrics.textPosition, hitTestMetrics.length,
                this->IsReadOnly()
                ) == S_OK) {
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
    if (m_metriceBuffer.data_length) {
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
        // 显示插入符号
        //Component::CUIEditaleText_ShowTheCaret
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
                if (LongUI::Component::RemoveText(
                    m_text, m_dragRange.startPosition, m_dragRange.length,
                    this->IsReadOnly()
                    ) == S_OK) {
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
    auto range = this->GetSelectionRange();
    // 有效
    if (range.length > 0) {
        this->RefreshSelectionMetrics(range);
    }
}

// 渲染
void LongUI::Component::EditaleText::Render(float x, float y)const noexcept {
    assert(m_pRenderTarget);
    if (m_metriceBuffer.data_length) {
        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        // 遍历
        for (auto itr = m_metriceBuffer.data;
        itr != m_metriceBuffer.data + m_metriceBuffer.data_length; ++itr) {
            const DWRITE_HIT_TEST_METRICS& htm = *itr;
            D2D1_RECT_F highlightRect = {
                htm.left + x,
                htm.top + y,
                htm.left + htm.width + x,
                htm.top + htm.height + y
            };
            m_pRenderTarget->FillRectangle(highlightRect, m_pSelectionColor);
        }
        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }
    // 刻画字体
    this->layout->Draw(m_buffer.data, m_pTextRenderer, x, y);
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
                const wchar_t* text = m_text.c_str();
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
                        const wchar_t* text = m_text.c_str();
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
        hr = this->insert(text, m_u32CaretPos + m_u32CaretPosOffset, characterCount);
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
    // 有选择的情况下
    if (selection.length == 0) {
        m_metriceBuffer.data_length = 0;
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
    m_metriceBuffer.NewSize(actualHitTestCount);
    if (!actualHitTestCount) return;
    // 正式获取
    this->layout->HitTestTextRange(
        selection.startPosition,
        selection.length,
        0.f, // x
        0.f, // y
        m_metriceBuffer.data,
        m_metriceBuffer.data_length,
        &actualHitTestCount
        );
}



// EditaleText 析构函数
LongUI::Component::EditaleText::~EditaleText() noexcept {
    ::ReleaseStgMedium(&m_recentMedium);
    ::SafeRelease(this->layout);
    ::SafeRelease(m_pBasicFormat);
    ::SafeRelease(m_pTextRenderer);
    ::SafeRelease(m_pRenderTarget);
    ::SafeRelease(m_pSelectionColor);
    ::SafeRelease(m_pDropSource);
    ::SafeRelease(m_pDataObject);
}




// 配置构造函数
LongUI::Component::EditaleText::EditaleText(UIControl* host, pugi::xml_node node,
    const char* prefix) noexcept : m_pHost(host) {
    m_dragRange = { 0, 0 };
    // 检查参数
    assert(node && prefix && "bad arguments");
    ZeroMemory(&m_recentMedium, sizeof(m_recentMedium));
    m_pFactory = ::SafeAcquire(UIManager_DWriteFactory);
    register const char* str = nullptr;
    char attribute_buffer[256];
    // 设置新的属性
    auto set_new_attribute = [prefix](char* buffer, const char* suffix) noexcept {
        ::strcpy(buffer, prefix); ::strcat(buffer, suffix);
    };
    // 检查类型
    {
        uint32_t tmptype = Type_None;
        // 富文本
        set_new_attribute(attribute_buffer, "rich");
        if (node.attribute(attribute_buffer).as_bool(false)) {
            tmptype |= Type_Riched;
        }
        // 多行显示
        set_new_attribute(attribute_buffer, "multiline");
        if (node.attribute(attribute_buffer).as_bool(false)) {
            tmptype |= Type_MultiLine;
        }
        // 只读
        set_new_attribute(attribute_buffer, "readonly");
        if (node.attribute(attribute_buffer).as_bool(false)) {
            tmptype |= Type_ReadOnly;
        }
        // 只读
        set_new_attribute(attribute_buffer, "accelerator");
        if (node.attribute(attribute_buffer).as_bool(false)) {
            tmptype |= Type_Accelerator;
        }
        // 密码
        set_new_attribute(attribute_buffer, "password");
        if (str = node.attribute(attribute_buffer).value()) {
            tmptype |= Type_Password;
            // TODO: UTF8 char(s) to UTF32 char;
            // this->password = 
        }
        this->type = static_cast<EditaleTextType>(tmptype);
    }
    // 获取渲染器
    {
        int renderer_index = Type_NormalTextRenderer;
        set_new_attribute(attribute_buffer, "renderer");
        if ((str = node.attribute(attribute_buffer).value())) {
            renderer_index = LongUI::AtoI(str);
        }
        auto renderer = UIManager.GetTextRenderer(renderer_index);
        m_pTextRenderer = renderer;
        // 保证缓冲区
        if (renderer) {
            auto length = renderer->GetContextSizeInByte();
            if (length) {
                set_new_attribute(attribute_buffer, "context");
                if ((str = node.attribute(attribute_buffer).value())) {
                    m_buffer.NewSize(length);
                    renderer->CreateContextFromString(m_buffer.data, str);
                }
            }
        }
    }
    // 检查基本颜色
    {
        m_basicColor = D2D1::ColorF(D2D1::ColorF::Black);
        set_new_attribute(attribute_buffer, "color");
        UIControl::MakeColor(node.attribute(attribute_buffer).value(), m_basicColor);
    }
    // 检查格式
    {
        uint32_t format_index = LongUIDefaultTextFormatIndex;
        set_new_attribute(attribute_buffer, "format");
        if ((str = node.attribute(attribute_buffer).value())) {
            format_index = static_cast<uint32_t>(LongUI::AtoI(str));
        }
        m_pBasicFormat = UIManager.GetTextFormat(format_index);
    }
    // 获取文本
    {
        wchar_t buffer[LongUIStringBufferLength];
        if (str = node.attribute(prefix).value()) {
            try {
                m_text.assign(buffer, LongUI::UTF8toWideChar(str, buffer));
            }
            catch (...) {
                UIManager << DL_Error << L"FAILED" << LongUI::endl;
            }
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
    assert(m_pRenderTarget);
    // XXX: 优化 Profiler 就这1行 0.05%
    m_pRenderTarget->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(mat));
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
    m_pRenderTarget->DrawGlyphRun(
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
    m_pRenderTarget->FillRectangle(&rectangle, m_pBrush);
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
    m_pRenderTarget->FillRectangle(&rectangle, m_pBrush);
    return S_OK;
}

// -------------------- LongUI::Component::Elements --------------------

// 实现
#define UIElements_Prefix if (!node) return; if(!prefix) prefix = ""; char buffer[256];
#define UIElements_NewAttribute(a) { ::strcpy(buffer, prefix); ::strcat(buffer, a); }
#define UIElements_Attribute node.attribute(buffer).value()

// Elements<Basic> Init
void LongUI::Component::Elements<LongUI::Element::Basic>::
Init(pugi::xml_node node, const char* prefix) noexcept {
    // 无效?
    UIElements_Prefix;
    const char* str = nullptr;
    // 动画类型
    UIElements_NewAttribute("animationtype");
    if (str = UIElements_Attribute) {
        animation.type = static_cast<AnimationType>(LongUI::AtoI(str));
    }
    // 动画持续时间
    UIElements_NewAttribute("animationduration");
    if (str = UIElements_Attribute) {
        animation.duration = LongUI::AtoF(str);
    }
}

// 设置新的状态
auto LongUI::Component::Elements<LongUI::Element::Basic>::
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
LongUI::Component::Elements<LongUI::Element::Meta>::
Elements(pugi::xml_node node, const char* prefix) noexcept: Super(node, prefix) {
    ZeroMemory(m_metas, sizeof(m_metas));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 无效?
    UIElements_Prefix;
    // 禁用状态Meta ID
    UIElements_NewAttribute("disabledmeta");
    m_aID[Status_Disabled] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
    // 通常状态Meta ID
    UIElements_NewAttribute("normalmeta");
    m_aID[Status_Normal] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
    // 移上状态Meta ID
    UIElements_NewAttribute("hovermeta");
    m_aID[Status_Hover] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
    // 按下状态Meta ID
    UIElements_NewAttribute("pushedmeta");
    m_aID[Status_Pushed] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
}


// Elements<Meta> 重建
auto LongUI::Component::Elements<LongUI::Element::Meta>::
Recreate(LongUIRenderTarget* target) noexcept ->HRESULT {
    UNREFERENCED_PARAMETER(target);
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
void LongUI::Component::Elements<LongUI::Element::Meta>::Render(const D2D1_RECT_F& rect) const noexcept {
    assert(m_pRenderTarget);
    // 先绘制当前状态
    if (this->animation.value < this->animation.end) {
        LongUI::Meta_Render(
            m_metas[m_state], m_pRenderTarget, rect, this->animation.end
            );
    }
    // 再绘制目标状态
    LongUI::Meta_Render(
        m_metas[m_stateTartget], m_pRenderTarget, rect, this->animation.value
        );
}



// Elements<BrushRect> 构造函数
LongUI::Component::Elements<LongUI::Element::BrushRect>::
Elements(pugi::xml_node node, const char* prefix) noexcept :Super(node, prefix) {
    ZeroMemory(m_apBrushes, sizeof(m_apBrushes));
    ZeroMemory(m_aID, sizeof(m_aID));
    // 无效?
    UIElements_Prefix;
    // 禁用状态笔刷ID
    UIElements_NewAttribute("disabledbrush");
    m_aID[Status_Disabled] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
    // 通常状态笔刷ID
    UIElements_NewAttribute("normalbrush");
    m_aID[Status_Normal] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
    // 移上状态笔刷ID
    UIElements_NewAttribute("hoverbrush");
    m_aID[Status_Hover] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
    // 按下状态笔刷ID
    UIElements_NewAttribute("pushedbrush");
    m_aID[Status_Pushed] = static_cast<uint16_t>(LongUI::AtoI(UIElements_Attribute));
}

// 释放数据
void LongUI::Component::Elements<LongUI::Element::BrushRect>::release_data() noexcept {
    for (auto& brush : m_apBrushes) {
        ::SafeRelease(brush);
    }
}

// Elements<BrushRectta> 渲染
void LongUI::Component::Elements<LongUI::Element::BrushRect>::Render(const D2D1_RECT_F& rect) const noexcept {
    assert(m_pRenderTarget);
    D2D1_MATRIX_3X2_F matrix; m_pRenderTarget->GetTransform(&matrix);
#if 1
    // 计算转换后的矩形
    auto height = rect.bottom - rect.top;
    D2D1_RECT_F rect2 = {
        0.f, 0.f, (rect.right - rect.left) / height , 1.f
    };
    // 计算转换后的矩阵
    m_pRenderTarget->SetTransform(
        D2D1::Matrix3x2F::Scale(height, height) *
        D2D1::Matrix3x2F::Translation(rect.left, rect.top) *
        matrix
        );
    // 先绘制当前状态
    if (animation.value < animation.end) {
        m_pRenderTarget->FillRectangle(rect2, m_apBrushes[m_state]);
    }
    // 后绘制目标状态
    auto brush = m_apBrushes[m_stateTartget];
    brush->SetOpacity(animation.value);
    m_pRenderTarget->FillRectangle(rect2, brush);
    brush->SetOpacity(1.f);
    m_pRenderTarget->SetTransform(&matrix);
#else
    //m_pRenderTarget->SetTransform(D2D1::IdentityMatrix());
    // 先绘制当前状态
    if (animation.value < animation.end) {
        m_apBrushes[m_state]->SetTransform(&matrix);
        m_pRenderTarget->FillRectangle(rect, m_apBrushes[m_state]);
    }
    // 后绘制目标状态
    auto brush = m_apBrushes[m_stateTartget];
    brush->SetOpacity(animation.value);
    //brush->SetTransform(&matrix);
    m_pRenderTarget->FillRectangle(rect, brush);
    brush->SetOpacity(1.f);
    //m_pRenderTarget->SetTransform(&matrix);
#endif
}
// Elements<BrushRect> 重建
auto LongUI::Component::Elements<LongUI::Element::BrushRect>::
Recreate(LongUIRenderTarget* target) noexcept ->HRESULT {
    UNREFERENCED_PARAMETER(target);
    this->release_data();
    for (auto i = 0u; i < STATUS_COUNT; ++i) {
        register auto id = m_aID[i];
        m_apBrushes[i] = id ? UIManager.GetBrush(id) : UIManager.GetSystemBrush(i);
    }
    return S_OK;
}

// Elements<ColorRect> 构造函数
LongUI::Component::Elements<LongUI::Element::ColorRect>::
Elements(pugi::xml_node node, const char* prefix) noexcept: Super(node, prefix) {
    colors[Status_Disabled] = D2D1::ColorF(0xDEDEDEDE);
    colors[Status_Normal] = D2D1::ColorF(0xCDCDCDCD);
    colors[Status_Hover] = D2D1::ColorF(0xA9A9A9A9);
    colors[Status_Pushed] = D2D1::ColorF(0x78787878);
    // 无效?
    UIElements_Prefix;
    // 禁用状态颜色
    UIElements_NewAttribute("disabledcolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Disabled]);
    // 通常状态颜色
    UIElements_NewAttribute("normalcolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Normal]);
    // 移上状态颜色
    UIElements_NewAttribute("hovercolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Hover]);
    // 按下状态颜色
    UIElements_NewAttribute("pushedcolor");
    UIControl::MakeColor(UIElements_Attribute, colors[Status_Pushed]);
}

// Elements<ColorRect> 重建
auto LongUI::Component::Elements<LongUI::Element::ColorRect>::
Recreate(LongUIRenderTarget* target) noexcept ->HRESULT {
    UNREFERENCED_PARAMETER(target);
    ::SafeRelease(m_pBrush);
    m_pBrush = static_cast<ID2D1SolidColorBrush*>(UIManager.GetBrush(LongUICommonSolidColorBrushIndex));
    return S_OK;
}

// Elements<ColorRect> 渲染
void LongUI::Component::Elements<LongUI::Element::ColorRect>::Render(const D2D1_RECT_F& rect) const noexcept {
    assert(m_pRenderTarget && m_pBrush);
    // 先绘制当前状态
    if (animation.value < animation.end) {
        m_pBrush->SetColor(colors + m_state);
        m_pRenderTarget->FillRectangle(&rect, m_pBrush);
    }
    // 再绘制目标状态
    m_pBrush->SetOpacity(animation.value);
    m_pBrush->SetColor(colors + m_stateTartget);
    m_pRenderTarget->FillRectangle(&rect, m_pBrush);
    m_pBrush->SetOpacity(1.f);
}