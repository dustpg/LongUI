#include "LongUI.h"
#include <algorithm>
#include "dcomp.h"
                   
                    

// longui::component
LONGUI_NAMESPACE_BEGIN namespace Component {
    // --------------------- LongUI::Component::ShortText ---------------------
    /// <summary>
    /// ShortText 初始化
    /// </summary>
    /// <param name="node">The xml node.</param>
    /// <param name="prefix">The prefix.</param>
    /// <returns></returns>
    void ShortText::Init(pugi::xml_node node, const char* prefix) noexcept {
        LongUI::SafeRelease(m_pTextRenderer);
        // 设置
        m_config = {
            nullptr,
            128.f, 64.f, 1.f,
            Helper::GetEnumFromXml(node, RichType::Type_None, "richtype", prefix),
            0
        };
        // 初始化
        this->color[State_Disabled] = D2D1::ColorF(D2D1::ColorF::Gray);
        this->color[State_Normal] = D2D1::ColorF(D2D1::ColorF::Black);
        this->color[State_Hover] = D2D1::ColorF(0xA9A9A9A9);
        this->color[State_Pushed] = D2D1::ColorF(0x78787878);
        // 有效结点
        if (node) {
            // 颜色
            Helper::MakeStateBasedColor(node, prefix, this->color);
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
            {
                // 检查格式
                uint32_t format_index = 0;
                if ((str = attribute("format"))) {
                    format_index = static_cast<uint32_t>(LongUI::AtoI(str));
                }
                // 模板
                auto fmt = UIManager.GetTextFormat(format_index);
                auto hr = DX::MakeTextFormat(node, &m_config.format, fmt, prefix);
                UNREFERENCED_PARAMETER(hr);
                assert(SUCCEEDED(hr));
                LongUI::SafeRelease(fmt);
            }
            // 重建
            {
                auto text = node.attribute(prefix).value();
                m_text.Set(text ? text : "");
            }
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
    ShortText::~ShortText() noexcept {
        LongUI::SafeRelease(m_pLayout);
        LongUI::SafeRelease(m_pTextRenderer);
        LongUI::SafeRelease(m_config.format);
    }
    // ShortText 重建布局
    void ShortText::RecreateLayout() noexcept {
        // 保留数据
        auto old_layout = m_pLayout;
        m_pLayout = nullptr;
        // 看情况
        switch (m_config.rich_type)
        {
        case LongUI::RichType::Type_None:
        {
            auto string_length_need = static_cast<uint32_t>(
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
            UNREFERENCED_PARAMETER(hr);
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
        LongUI::SafeRelease(old_layout);
        // sad
        assert(m_pLayout);
    }
    // -------------------- LongUI::Component::EditaleText --------------------
    // DWrite部分代码参考: 
    // http://msdn.microsoft.com/zh-cn/library/windows/desktop/dd941792(v=vs.85).aspx
    /// <summary>
    /// 刷新文本布局
    /// </summary>
    /// <param name="update">if set to <c>true</c> [update].</param>
    /// <returns></returns>
    auto EditaleText::refresh(bool update) const noexcept ->UIWindow* {
        if (!m_bThisFocused) return nullptr;
        RectLTWH_F rect; this->GetCaretRect(rect);
        auto* window = m_pHost->GetWindow();
        window->CreateCaret(m_pHost, rect.width, rect.height);
        window->SetCaretPos(m_pHost, rect.left, rect.top);
        if (update) {
            window->Invalidate(m_pHost);
        }
        return window;
    }
    // 重新创建布局
    void EditaleText::recreate_layout(IDWriteTextFormat* fmt) noexcept {
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
    auto EditaleText::insert(
        uint32_t pos, const wchar_t * str, uint32_t length) noexcept -> HRESULT {
        HRESULT hr = S_OK;
        // 只读
        if (this->IsReadOnly()) {
            LongUI::BeepError();
            return S_FALSE;
        }
        // 插入字符
        m_string.insert(pos, str, length);
        auto old_length = static_cast<uint32_t>(m_string.length());
        // 保留旧布局
        auto old_layout = LongUI::SafeAcquire(this->layout);
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
        LongUI::SafeRelease(old_layout);
        return hr;
    }
    // 返回当前选择区域
    auto EditaleText::GetSelectionRange() const noexcept -> DWRITE_TEXT_RANGE {
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
    auto EditaleText::SetSelection(
        SelectionMode mode, uint32_t advance, bool exsel, bool update) noexcept -> HRESULT {
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
    auto EditaleText::DeleteSelection() noexcept -> HRESULT {
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
    bool EditaleText::SetSelectionFromPoint(float x, float y, bool exsel) noexcept {
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
    bool EditaleText::OnDragEnter(IDataObject* data, DWORD* effect) noexcept {
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
    bool EditaleText::OnDragOver(float x, float y) noexcept {
        // 自己的?并且在选择范围内?
        if (m_bDragFromThis) {
            auto range = m_dragRange;
            BOOL trailin, inside;
            DWRITE_HIT_TEST_METRICS caret_metrics;
            // 获取当前点击位置
            this->layout->HitTestPoint(x, y, &trailin, &inside, &caret_metrics);
            bool inzone = caret_metrics.textPosition >= range.startPosition &&
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
    void EditaleText::Recreate() noexcept {
        // 重新创建资源
        LongUI::SafeRelease(m_pSelectionColor);
        UIManager_RenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightSkyBlue),
            &m_pSelectionColor
            );
    }

    // 键入一个字符时
    void EditaleText::OnChar(char32_t ch) noexcept {
        // 字符有效
        if ((ch >= 0x20 || ch == 9)) {
            if (this->IsReadOnly()) {
                LongUI::BeepError();
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
    void EditaleText::OnKey(uint32_t keycode) noexcept {
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
    void EditaleText::OnSetFocus() noexcept {
        m_bThisFocused = true;
        this->refresh()->ShowCaret();
    }

    // 当失去焦点时
    void EditaleText::OnKillFocus() noexcept {
        //auto window = m_pHost->GetWindow();
        m_pHost->GetWindow()->HideCaret();
        m_bThisFocused = false;
    }

    // 左键弹起时
    void EditaleText::OnLButtonUp(float x, float y) noexcept {
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
    void EditaleText::OnLButtonDown(float x, float y, bool shfit_hold) noexcept {
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
    void EditaleText::OnLButtonHold(float x, float y, bool shfit_hold) noexcept {
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
                const HRESULT hr = ::SHDoDragDrop(
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
    void EditaleText::AlignCaretToNearestCluster(bool hit, bool skip) noexcept {
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
    void EditaleText::GetCaretRect(RectLTWH_F& rect)const noexcept {
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
            float caretThickness = static_cast<float>(caretIntThickness);
            // 计算相对位置
            // XXX: 检查draw_zone
            rect.left = caretX - caretThickness * 0.5f;
            rect.width = caretThickness;
            rect.top = caretY;
            rect.height = caretMetrics.height;
        }
    }
    // 刷新
    void EditaleText::Update() noexcept {
        // s
        this->refresh(false);
        // 检查选择区
        this->RefreshSelectionMetrics(this->GetSelectionRange());
    }
    // 渲染
    void EditaleText::Render(float x, float y)const noexcept {
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
        m_pTextRenderer->basic_color.color = *m_pColor;
        this->layout->Draw(m_buffer.GetDataVoid(), m_pTextRenderer, x, y);
    }
    // 复制到 目标全局句柄
    auto EditaleText::CopyToGlobal() noexcept -> HGLOBAL {
        // 获取选择区
        auto selection = this->GetSelectionRange();
        // 有选择区域
        if (selection.length) {
            // 断言检查
            assert(selection.startPosition < m_string.length() && "bad selection range");
            assert((selection.startPosition + selection.length) < m_string.length() && "bad selection range");
            // 获取富文本数据
            if (this->IsRiched()) {
                assert(!"Unsupported Now");
                // TODO: 富文本
            }
            // 全局申请
            auto str = m_string.c_str() + selection.startPosition;
            auto len = static_cast<size_t>(selection.length);
            return Helper::GlobalAllocString(str, len);
        }
        // TODO: 复制选中行
        return nullptr;
    }

    // 复制到 剪切板
    auto EditaleText::CopyToClipboard() noexcept -> HRESULT {
        HRESULT hr = E_FAIL;
        // 打开剪切板
        if (::OpenClipboard(m_pHost->GetWindow()->GetHwnd())) {
            // 清空
            if (::EmptyClipboard()) {
                // 全局申请
                auto hClipboardData = this->CopyToGlobal();
                // 申请成功
                if (hClipboardData) {
                    // 成功
                    if (::SetClipboardData(CF_UNICODETEXT, hClipboardData) != nullptr) {
                        hr = S_OK;
                    }
                    // 失败
                    else {
                        ::GlobalFree(hClipboardData);
                    }
                }
            }
            //  关闭剪切板
            ::CloseClipboard();
        }
        return hr;
    }



    // 从 目标全局句柄 黏贴
    auto EditaleText::PasteFromGlobal(HGLOBAL global) noexcept -> HRESULT {
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
            auto characterCount = static_cast<UINT32>(std::min(std::wcslen(text), byteSize / sizeof(wchar_t)));
            // 插入
            hr = this->insert(m_u32CaretPos + m_u32CaretPosOffset, text, characterCount);
            ::GlobalUnlock(global);
            // 移动
            this->SetSelection(SelectionMode::Mode_RightChar, characterCount, true);
        }
        return hr;
    }

    // 从 剪切板 黏贴
    auto EditaleText::PasteFromClipboard() noexcept -> HRESULT {
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
    void EditaleText::GetLineFromPosition(
        const DWRITE_LINE_METRICS * lineMetrics,
        uint32_t lineCount, uint32_t textPosition,
        OUT uint32_t * lineOut,
        OUT uint32_t * linePositionOut) noexcept {
        // 检查
        uint32_t line = 0;
        uint32_t linePosition = 0;
        uint32_t nextLinePosition = 0;
        // 遍历数据
        for (; line < lineCount; ++line) {
            // 更新数据
            linePosition = nextLinePosition;
            nextLinePosition = linePosition + lineMetrics[line].length;
            // 超过?
            if (nextLinePosition > textPosition) {
                // 不需要.
                break;
            }
        }
        *linePositionOut = linePosition;
        *lineOut = std::min(line, lineCount - 1);
    }
    /// <summary>
    /// 更新选择区点击测试区块
    /// </summary>
    /// <param name="selection">The selection.</param>
    /// <returns></returns>
    void EditaleText::RefreshSelectionMetrics(DWRITE_TEXT_RANGE selection) noexcept {
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
    /// <summary>
    /// Finalizes an instance of the <see cref="EditaleText"/> class.
    /// </summary>
    /// <returns></returns>
    EditaleText::~EditaleText() noexcept {
        ::ReleaseStgMedium(&m_recentMedium);
        LongUI::SafeRelease(this->layout);
        LongUI::SafeRelease(m_pTextRenderer);
        LongUI::SafeRelease(m_pSelectionColor);
        LongUI::SafeRelease(m_pDropSource);
        LongUI::SafeRelease(m_pDataObject);
    }
    /// <summary>
    /// Initializes a new instance of the <see cref="EditaleText"/> class.
    /// </summary>
    /// <param name="host">The host control</param>
    EditaleText::EditaleText(UIControl* host) noexcept : m_pHost(host) {

    }
    /// <summary>
    /// Initializes without specified xml node.
    /// </summary>
    /// <param name="node">The node.</param>
    /// <param name="prefix">The prefix.</param>
    /// <returns></returns>
    void EditaleText::Init() noexcept {
        m_dragRange = { 0, 0 };
        // 颜色
        this->color[State_Disabled] = D2D1::ColorF(D2D1::ColorF::Gray);
        this->color[State_Normal] = D2D1::ColorF(D2D1::ColorF::Black);
        this->color[State_Hover] = D2D1::ColorF(0xA9A9A9A9);
        this->color[State_Pushed] = D2D1::ColorF(0x78787878);
        std::memset(&m_recentMedium, 0, sizeof(m_recentMedium));
        // 设置类型
        this->type = Type_None;
        // 设置渲染器
        m_pTextRenderer = UIManager.GetTextRenderer(0);
        // 检查格式
        IDWriteTextFormat* fmt = UIManager.GetTextFormat(LongUIDefaultTextFormatIndex);
        // 创建布局
        this->recreate_layout(fmt);
        // 释放数据
        LongUI::SafeRelease(fmt);
    }
    /// <summary>
    /// Initializes with specified xml node.
    /// </summary>
    /// <param name="node">The node.</param>
    /// <param name="prefix">The prefix.</param>
    /// <returns></returns>
    void EditaleText::Init(pugi::xml_node node, const char* prefix) noexcept {
        m_dragRange = { 0, 0 };
        // 初始化
        this->color[State_Disabled] = D2D1::ColorF(D2D1::ColorF::Gray);
        this->color[State_Normal] = D2D1::ColorF(D2D1::ColorF::Black);
        this->color[State_Hover] = D2D1::ColorF(0xA9A9A9A9);
        this->color[State_Pushed] = D2D1::ColorF(0x78787878);
        // 检查参数
        assert(node && prefix && "bad arguments");
        // 颜色
        Helper::MakeStateBasedColor(node, prefix, this->color);
        std::memset(&m_recentMedium, 0, sizeof(m_recentMedium));
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
        // 检查格式
        IDWriteTextFormat* fmt = nullptr;
        {
            uint32_t format_index = LongUIDefaultTextFormatIndex;
            if ((str = attribute("format"))) {
                format_index = static_cast<uint32_t>(LongUI::AtoI(str));
            }
            auto template_format = UIManager.GetTextFormat(format_index);
            auto hr = DX::MakeTextFormat(node, &fmt, template_format, prefix);
            UNREFERENCED_PARAMETER(hr);
            assert(SUCCEEDED(hr));
            LongUI::SafeRelease(template_format);
        }
        // 格式特化
        {
            assert(fmt && "bad action");
        }
        // 获取文本
        {
            if (str = node.attribute(prefix).value()) {
                m_string.assign(str);
            }
        }
        // 创建布局
        this->recreate_layout(fmt);
        LongUI::SafeRelease(fmt);
    }
    /// <summary>
    /// Copies the global properties. 复制全局属性
    /// </summary>
    /// <param name="old_layout">The old_layout.</param>
    /// <param name="new_layout">The new_layout.</param>
    /// <returns></returns>
    void EditaleText::CopyGlobalProperties(
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
#   ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
        DWRITE_TRIMMING trimmingOptions = {};
        IDWriteInlineObject* inlineObject = nullptr;
        old_layout->GetTrimming(&trimmingOptions, &inlineObject);
        new_layout->SetTrimming(&trimmingOptions, inlineObject);
        LongUI::SafeRelease(inlineObject);

        DWRITE_LINE_SPACING_METHOD lineSpacingMethod = DWRITE_LINE_SPACING_METHOD_DEFAULT;
        float lineSpacing = 0.f, baseline = 0.f;
        old_layout->GetLineSpacing(&lineSpacingMethod, &lineSpacing, &baseline);
        new_layout->SetLineSpacing(lineSpacingMethod, lineSpacing, baseline);
#   endif
    }

    // 对范围复制单个属性
    void EditaleText::CopySinglePropertyRange(
        IDWriteTextLayout* old_layout, uint32_t old_start,
        IDWriteTextLayout* new_layout, uint32_t new_start, uint32_t length) noexcept {
        // 计算范围
        DWRITE_TEXT_RANGE range = { new_start,  std::min(length, UINT32_MAX - new_start) };
        // 字体集
#   ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
        IDWriteFontCollection* fontCollection = nullptr;
        old_layout->GetFontCollection(old_start, &fontCollection);
        new_layout->SetFontCollection(fontCollection, range);
        LongUI::SafeRelease(fontCollection);
#   endif
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
#   ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
            // 地区名
            wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
            localeName[0] = L'\0';
            old_layout->GetLocaleName(old_start, &localeName[0], ARRAYSIZE(localeName));
            new_layout->SetLocaleName(localeName, range);
#   endif
        }
        // 刻画效果
        IUnknown* drawingEffect = nullptr;
        old_layout->GetDrawingEffect(old_start, &drawingEffect);
        new_layout->SetDrawingEffect(drawingEffect, range);
        LongUI::SafeRelease(drawingEffect);
        // 内联对象
        IDWriteInlineObject* inlineObject = nullptr;
        old_layout->GetInlineObject(old_start, &inlineObject);
        new_layout->SetInlineObject(inlineObject, range);
        LongUI::SafeRelease(inlineObject);
#   ifndef LONGUI_EDITCORE_COPYMAINPROPERTYONLY
        // 排版
        IDWriteTypography* typography = nullptr;
        old_layout->GetTypography(old_start, &typography);
        new_layout->SetTypography(typography, range);
        LongUI::SafeRelease(typography);
#   endif
    }

    // 范围复制AoE!
    void EditaleText::CopyRangedProperties(
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
    // --------------------- LongUI::Component::Elements ---------------------
    // 动画
    constexpr float ANIMATION_END = 1.f;
    // 基本动画状态机 - 构造函数
    LongUINoinline AnimationStateMachine::AnimationStateMachine() noexcept
        : m_aniBasic(AnimationType::Type_QuadraticEaseOut),
        m_aniExtra(AnimationType::Type_QuadraticEaseOut) {
        m_aniBasic.end = ANIMATION_END;
        m_aniExtra.end = ANIMATION_END;
    }
    // 基本动画状态机 - 初始化
    LongUINoinline void AnimationStateMachine::Init(
        State basic, State extra, pugi::xml_node node, const char* prefix) noexcept {
        // 初始化状态
        m_sttBasicNow = m_sttBasicOld = basic;
        m_sttExtraNow = m_sttExtraOld = extra;
        // 动画类型
        auto atype = Helper::GetEnumFromXml(node, AnimationType::Type_QuadraticEaseOut, "animationtype", prefix);
        m_aniBasic.type = m_aniExtra.type = atype;
        // 动画持续时间
        const char* str = nullptr;
        if ((str = Helper::XMLGetValue(node, "animationduration", prefix))) {
            m_aniBasic.duration = m_aniExtra.duration = LongUI::AtoF(str);
        }
    }
    // 基本动画状态机 - 设置新的基本状态
    LongUINoinline auto AnimationStateMachine::SetBasicState(State state) noexcept ->float {
        m_sttBasicOld = m_sttBasicNow;
        m_sttBasicNow = state;
        // 剩余值
        m_aniBasic.start = 0.f;
        m_aniBasic.value = 0.f;
        // 剩余时间
        return m_aniBasic.time = m_aniBasic.duration;
    }
    // 基本动画状态机 - 设置新的额外状态
    LongUINoinline auto AnimationStateMachine::SetExtraState(State state) noexcept ->float {
        m_sttExtraOld = m_sttExtraNow;
        m_sttExtraNow = state;
        // 剩余值
        m_aniExtra.start = 0.f;
        m_aniExtra.value = 0.f;
        // 剩余时间
        return m_aniExtra.time = m_aniExtra.duration;
    }
    // GIMetaBasic 帮助器 -- META重建
    LongUINoinline auto GIHelper::Recreate(
        Meta metas[], const uint16_t ids[], size_t count) noexcept -> HRESULT {
        // 遍历
        for (size_t i = 0; i < count; ++i) {
            // 有效
            if (ids[i]) {
                LongUI::DestoryObject(metas[i]);
                UIManager.GetMeta(ids[i], metas[i]);
            }
        }
        return S_OK;
    }
    // GIMetaBasic 帮助器 -- 笔刷重建
    LongUINoinline auto GIHelper::Recreate(
        ID2D1Brush* brushes[], const uint16_t ids[], size_t count) noexcept -> HRESULT {
        // 遍历
        for (size_t i = 0; i < count; ++i) {
            // 重建笔刷
            LongUI::SafeRelease(brushes[i]);
            auto id = ids[i];
            brushes[i] = id ? UIManager.GetBrush(id) : UIManager.GetSystemBrush(static_cast<uint32_t>(i));
        }
        return S_OK;
    }
    // GIMetaBasic 帮助器 -- 接口清理
    LongUINoinline void GIHelper::Clean(IUnknown** itfs, size_t size) noexcept {
        for (size_t i = 0; i < size; ++i) LongUI::SafeRelease(itfs[i]);
    }
    // GIMetaBasic 帮助器 -- META 渲染
    LongUINoinline void GIHelper::Render(
        const D2D1_RECT_F& rect, const AnimationStateMachine& sm, const Meta metas[], uint16_t basic) noexcept {
        assert(UIManager_RenderTarget);
        // 检查动画
        const auto& baani = sm.GetBasicAnimation();
        const auto& exani = sm.GetExtraAnimation();
        auto bastt1 = sm.GetOldBasicState();
        auto bastt2 = sm.GetNowBasicState();
        auto exstt1 = sm.GetOldExtraState();
        auto exstt2 = sm.GetNowExtraState();
        // 额外状态动画中
        if (exani.value > 0.f && exani.value < ANIMATION_END) {
            // 渲染下层
            metas[exstt1 * basic + bastt2].Render(UIManager_RenderTarget, rect, ANIMATION_END);
            // 渲染上层
            metas[exstt2 * basic + bastt2].Render(UIManager_RenderTarget, rect, exani.value);
        }
        // 基本动画状态
        else {
            // 绘制旧的状态
            if (baani.value < ANIMATION_END) {
                metas[exstt2 * basic + bastt1].Render(UIManager_RenderTarget, rect, ANIMATION_END);
            }
            // 再绘制目标状态
            metas[exstt2 * basic + bastt2].Render(UIManager_RenderTarget, rect, baani.value);
        }
    }
    // GIMetaBasic 帮助器 -- 矩形笔刷渲染
    void GIHelper::Render(
        const D2D1_RECT_F& rect, const AnimationStateMachine& sm, ID2D1Brush* const brushes[], uint16_t basic) noexcept{
        assert(UIManager_RenderTarget);
        // 检查动画
        const auto& baani = sm.GetBasicAnimation();
        const auto& exani = sm.GetExtraAnimation();
        auto bastt1 = sm.GetOldBasicState();
        auto bastt2 = sm.GetNowBasicState();
        auto exstt1 = sm.GetOldExtraState();
        auto exstt2 = sm.GetNowExtraState();
        // 额外状态动画中
        if (exani.value > 0.f && exani.value < ANIMATION_END) {
            // 渲染下层
            LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, brushes[exstt1*basic+bastt2], rect);
            // 渲染上层
            auto brush = brushes[exstt2*basic + bastt2];
            brush->SetOpacity(exani.value);
            LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, brush, rect);
            brush->SetOpacity(1.f);
        }
        // 基本动画状态
        else {
            // 绘制旧的状态
            if (baani.value < ANIMATION_END) {
                LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, brushes[exstt2*basic+bastt1], rect);
            }
            // 再绘制目标状态
            auto brush = brushes[exstt2 * basic + bastt2];
            brush->SetOpacity(baani.value);
            LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, brush, rect);
            brush->SetOpacity(1.f);
        }
    }
    // GIMetaBasic 帮助器 -- 颜色 渲染
    LongUINoinline void GIHelper::Render(
        const D2D1_RECT_F& rect, const AnimationStateMachine& sm, 
        const D2D1_COLOR_F clrs[], uint16_t basic
        ) noexcept {
        auto brush = static_cast<ID2D1SolidColorBrush*>(UIManager.GetBrush(LongUICommonSolidColorBrushIndex));
        assert(UIManager_RenderTarget && brush && "bad action");
        // 检查动画
        const auto& baani = sm.GetBasicAnimation();
        const auto& exani = sm.GetExtraAnimation();
        auto bastt1 = sm.GetOldBasicState();
        auto bastt2 = sm.GetNowBasicState();
        auto exstt1 = sm.GetOldExtraState();
        auto exstt2 = sm.GetNowExtraState();
        D2D1_COLOR_F color;
        // 额外状态动画中
        if (exani.value > 0.f && exani.value < ANIMATION_END) {
            float exalpha = ANIMATION_END - exani.value;
            // Alpha混合
            color.a = clrs[exstt2*basic + bastt2].a * exani.value
                + clrs[exstt1*basic + bastt2].a * exalpha;
            color.r = clrs[exstt2*basic + bastt2].r * exani.value
                + clrs[exstt1*basic + bastt2].r * exalpha;
            color.g = clrs[exstt2*basic + bastt2].g * exani.value
                + clrs[exstt1*basic + bastt2].g * exalpha;
            color.b = clrs[exstt2*basic + bastt2].b * exani.value
                + clrs[exstt1*basic + bastt2].b * exalpha;
        }
        // 基本动画状态
        else {
            float baalpha = ANIMATION_END - baani.value;
            // Alpha混合
            color.a = clrs[exstt2*basic + bastt2].a * baani.value
                + clrs[exstt2*basic + bastt1].a * baalpha;
            color.r = clrs[exstt2*basic + bastt2].r * baani.value
                + clrs[exstt2*basic + bastt1].r * baalpha;
            color.g = clrs[exstt2*basic + bastt2].g * baani.value
                + clrs[exstt2*basic + bastt1].g * baalpha;
            color.b = clrs[exstt2*basic + bastt2].b * baani.value
                + clrs[exstt2*basic + bastt1].b * baalpha;
        }
        // 渲染
        brush->SetColor(&color);
        UIManager_RenderTarget->FillRectangle(rect, brush);
        LongUI::SafeRelease(brush);
    }
    // 复选框图像接口 -- 渲染
    void GICheckBox::Init(pugi::xml_node /*node*/, const char* /*prefix*/) noexcept {
        colors[State_Disabled]  = D2D1::ColorF(0xBFBFBFui32);
        colors[State_Normal]    = D2D1::ColorF(0x000000ui32);
        colors[State_Hover]     = D2D1::ColorF(0x7EB4EAui32);
        colors[State_Pushed]    = D2D1::ColorF(0xACACACui32);
    }
    // 复选框图像接口 -- 渲染
    void GICheckBox::Render(const D2D1_RECT_F& rect, const Component::AnimationStateMachine& sm) const noexcept {
        assert(UIManager_RenderTarget);
        auto brush = static_cast<ID2D1SolidColorBrush*>(UIManager.GetBrush(LongUICommonSolidColorBrushIndex));
        assert(UIManager_RenderTarget && brush && "bad action");
#ifdef _DEBUG
        auto width = rect.right - rect.left;
        auto height = rect.bottom - rect.top;
        if ((width - UICheckBox::BOX_SIZE) <= -1.f || (width - UICheckBox::BOX_SIZE) >= 1.f) {
            UIManager << DL_Error
                << L"width of box for checkbox must be same as 'UICheckBox::BOX_SIZE'"
                << LongUI::endl;
        }
        if ((height - UICheckBox::BOX_SIZE) <= -1.f || (height - UICheckBox::BOX_SIZE) >= 1.f) {
            UIManager << DL_Error
                << L"height of box for checkbox must be same as 'UICheckBox::BOX_SIZE'"
                << LongUI::endl;
        }
#endif
        // 检查动画
        constexpr uint16_t basic = ControlState::STATE_COUNT;
        const auto& baani = sm.GetBasicAnimation();
        const auto& exani = sm.GetExtraAnimation();
        auto bastt1 = sm.GetOldBasicState();
        auto bastt2 = sm.GetNowBasicState();
        auto exstt1 = sm.GetOldExtraState();
        auto exstt2 = sm.GetNowExtraState();
        D2D1_COLOR_F color;
        auto* clrs = this->colors;
        // 额外状态动画中
        if (exani.value > 0.f && exani.value < ANIMATION_END) {
            float exalpha = ANIMATION_END - exani.value;
            // Alpha混合
            color.a = clrs[bastt2].a * exani.value + clrs[bastt2].a * exalpha;
            color.r = clrs[bastt2].r * exani.value + clrs[bastt2].r * exalpha;
            color.g = clrs[bastt2].g * exani.value + clrs[bastt2].g * exalpha;
            color.b = clrs[bastt2].b * exani.value + clrs[bastt2].b * exalpha;
        }
        // 基本动画状态
        else {
            float baalpha = ANIMATION_END - baani.value;
            // Alpha混合
            color.a = clrs[bastt2].a * baani.value + clrs[bastt1].a * baalpha;
            color.r = clrs[bastt2].r * baani.value + clrs[bastt1].r * baalpha;
            color.g = clrs[bastt2].g * baani.value + clrs[bastt1].g * baalpha;
            color.b = clrs[bastt2].b * baani.value + clrs[bastt1].b * baalpha;
        }
        brush->SetColor(&color);
        // 渲染 边框
        {
            auto tprc = rect;
            tprc.left += 0.5f;
            tprc.top += 0.5f;
            tprc.right -= 0.5f;
            tprc.bottom -= 0.5f;
            UIManager_RenderTarget->DrawRectangle(tprc, brush);
        }
        {
            float rate = sm.GetExtraAnimation().value;
            // 解开?
            if (CheckBoxState(exstt2) == CheckBoxState::State_Unchecked) {
                rate = 1.f - rate;
                exstt2 = exstt1;
            }
            // 打勾
            if (CheckBoxState(exstt2) == CheckBoxState::State_Checked) {
                // 坐标定义
                constexpr float P1X = UICheckBox::BOX_SIZE * 0.15f;
                constexpr float P1Y = UICheckBox::BOX_SIZE * 0.50f;
                constexpr float P2X = UICheckBox::BOX_SIZE * 0.33f;
                constexpr float P2Y = UICheckBox::BOX_SIZE * 0.85f;
                constexpr float P3X = UICheckBox::BOX_SIZE * 0.85f;
                constexpr float P3Y = UICheckBox::BOX_SIZE * 0.33f;
                // 笔触宽度
                constexpr float STROKE_WIDTH = UICheckBox::BOX_SIZE * 0.1f;
                // 仅动画第一阶段
                if (rate < 0.5f) {
                    auto temp = rate * 2.f;
                    D2D1_POINT_2F pt1 = D2D1::Point2F(rect.left + P1X, rect.top + P1Y);
                    D2D1_POINT_2F pt2 = D2D1::Point2F(pt1.x + (P2X - P1X)*temp, pt1.y + (P2Y - P1Y)*temp);
                    UIManager_RenderTarget->DrawLine(pt1, pt2, brush, STROKE_WIDTH);
                }
                // 仅动画第二阶段
                else {
                    // 第一阶段
                    D2D1_POINT_2F pt1 = D2D1::Point2F(rect.left + P1X, rect.top + P1Y);
                    D2D1_POINT_2F pt2 = D2D1::Point2F(rect.left + P2X, rect.top + P2Y);
                    UIManager_RenderTarget->DrawLine(pt1, pt2, brush, STROKE_WIDTH);
                    // 第二阶段
                    auto temp = (rate - 0.5f) * 2.f;
                    UIManager_RenderTarget->DrawLine(
                        pt2,
                        D2D1::Point2F(pt2.x + (P3X - P2X)*temp, pt2.y + (P3Y - P2Y)*temp),
                        brush,
                        STROKE_WIDTH
                        );
                }
            }
            // 中间情况
            else if (CheckBoxState(exstt2) == CheckBoxState::State_Indeterminate) {
                // 矩形填充
                float cx = (rect.left + rect.right) * 0.5f;
                float cy = (rect.bottom + rect.top) * 0.5f;
                D2D1_RECT_F dwrc;
                float sz = cx + UICheckBox::BOX_SIZE * 0.4f * rate;
                dwrc.left   = cx - sz;
                dwrc.top    = cy - sz;
                dwrc.right  = cx + sz;
                dwrc.bottom = cy + sz;
                UIManager_RenderTarget->FillRectangle(dwrc, brush);
            }
        }
        // 扫尾处理
        LongUI::SafeRelease(brush);
    }
}
LONGUI_NAMESPACE_END



// -------------------------------------------------------------


// 返回FALSE
HRESULT LongUI::XUIBasicTextRenderer::IsPixelSnappingDisabled(void*, BOOL * isDisabled) noexcept {
    *isDisabled = false;
    return S_OK;
}

// 从目标渲染呈现器获取
HRESULT LongUI::XUIBasicTextRenderer::GetCurrentTransform(void*, DWRITE_MATRIX * mat) noexcept {
    assert(UIManager_RenderTarget);
    // XXX: 优化 Profiler 就这1行 0.05%
    UIManager_RenderTarget->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(mat));
    return S_OK;
}

// 始终如一, 方便转换
HRESULT LongUI::XUIBasicTextRenderer::GetPixelsPerDip(void*, FLOAT * bilibili) noexcept {
    *bilibili = 1.f;
    return S_OK;
}

// 渲染内联对象
HRESULT LongUI::XUIBasicTextRenderer::DrawInlineObject(
    _In_opt_ void* clientDrawingContext,
    FLOAT originX, FLOAT originY,
    _In_ IDWriteInlineObject * inlineObject,
    BOOL isSideways, BOOL isRightToLeft,
    _In_opt_ IUnknown * clientDrawingEffect) noexcept {
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

// longui 
namespace LongUI {
    // same v-table?
    template<class A, class B> auto same_vtable(const A* a, const B* b) noexcept {
        auto table1 = (*reinterpret_cast<const size_t * const>(a));
        auto table2 = (*reinterpret_cast<const size_t * const>(b));
        return table1 == table2;
    }
}

// ------------------CUINormalTextRender-----------------------
// 刻画字形
HRESULT LongUI::CUINormalTextRender::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX, FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN * glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription,
    IUnknown * effect) noexcept {
    UNREFERENCED_PARAMETER(clientDrawingContext);
    UNREFERENCED_PARAMETER(glyphRunDescription);
    // 获取颜色
    D2D1_COLOR_F* color = nullptr;
    // 检查
    if (effect && same_vtable(effect, &this->basic_color)) {
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
    D2D1_COLOR_F* color = nullptr;
    if (effect && same_vtable(effect, &this->basic_color)) {
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
    D2D1_COLOR_F* color = nullptr;
    if (effect && same_vtable(effect, &this->basic_color)) {
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

                   
                    

// ------------------------- UIContainerBuiltIn ------------------------
// UIContainerBuiltIn: 事件处理
bool LongUI::UIContainerBuiltIn::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 初始化边缘控件 
            Super::DoEvent(arg);
            // 初次完成空间树建立
            for (auto ctrl : (*this)) {
                ctrl->DoEvent(arg);
            }
            return true;
        }
    }
    return Super::DoEvent(arg);
}

// UIContainerBuiltIn: 主景渲染
void LongUI::UIContainerBuiltIn::render_chain_main() const noexcept {
    // 渲染帮助器
    Super::RenderHelper(this->begin(), this->end());
    // 父类主景
    Super::render_chain_main();
}

// UIContainerBuiltIn: 渲染函数
void LongUI::UIContainerBuiltIn::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// LongUI内建容器: 刷新
void LongUI::UIContainerBuiltIn::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(this->begin(), this->end());
}


// UIContainerBuiltIn: 重建
auto LongUI::UIContainerBuiltIn::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    for (auto ctrl : (*this)) {
        hr = ctrl->Recreate();
        assert(SUCCEEDED(hr));
    }
    return Super::Recreate();
}


/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIContainerBuiltIn::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
    // 父类(边缘控件)
    auto mctrl = Super::FindChild(pt);
    if (mctrl) return mctrl;
    // 性能警告
#ifdef _DEBUG
    if (this->GetCount() > 100) {
        UIManager << DL_Warning
            << "Performance Warning: O(n) algorithm"
            << " is not fine for container that over 100 children"
            << LongUI::endl;
    }
#endif
    for (auto ctrl : (*this)) {
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            return ctrl;
        }
    }
    return nullptr;
}


// UIContainerBuiltIn: 推入♂最后
void LongUI::UIContainerBuiltIn::PushBack(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::PushBack(child);
    }
    // 一般的就自己处理
    else {
        this->Insert(this->end(), child);
    }
}

// UIContainerBuiltIn: 仅插入控件
void LongUI::UIContainerBuiltIn::insert_only(Iterator itr, UIControl* ctrl) noexcept {
    const auto end_itr = this->end();
    assert(ctrl && "bad arguments");
    if (ctrl->prev) {
        UIManager << DL_Warning
            << L"the 'prev' attr of the control: ["
            << ctrl->name
            << "] that to insert is not null"
            << LongUI::endl;
    }
    if (ctrl->next) {
        UIManager << DL_Warning
            << L"the 'next' attr of the control: ["
            << ctrl->name
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
        // 一般般
        force_cast(ctrl->next) = itr.Ptr();
        force_cast(ctrl->prev) = itr->prev;
        if (itr->prev) {
            force_cast(itr->prev) = ctrl;
        }
        force_cast(itr->prev) = ctrl;
    }
    ++m_cChildrenCount;
}


// UIContainerBuiltIn: 仅移除控件
void LongUI::UIContainerBuiltIn::RemoveJust(UIControl* ctrl) noexcept {
    // 检查是否属于本容器
#ifdef _DEBUG
    bool ok = false;
    for (auto i : (*this)) {
        if (ctrl == i) {
            ok = true;
            break;
        }
    }
    if (!ok) {
        UIManager << DL_Error 
            << "control:[" 
            << ctrl->name
            << "] not in this container: " 
            << this->name 
            << LongUI::endl;
        return;
    }
#endif
    {
        // 连接前后结点
        auto prev_tmp = ctrl->prev;
        auto next_tmp = ctrl->next;
        // 检查, 头
        (prev_tmp ? force_cast(prev_tmp->next) : m_pHead) = next_tmp;
        // 检查, 尾
        (next_tmp ? force_cast(next_tmp->prev) : m_pTail) = prev_tmp;
        // 减少
        force_cast(ctrl->prev) = force_cast(ctrl->next) = nullptr;
        --m_cChildrenCount;
        // 修改
        this->SetControlLayoutChanged();
    }
    Super::RemoveJust(ctrl);
}


// UIContainerBuiltIn: 析构函数
LongUI::UIContainerBuiltIn::~UIContainerBuiltIn() noexcept {
    // 关闭子控件
    auto ctrl = m_pHead;
    while (ctrl) {
        auto next_ctrl = ctrl->next;
        this->cleanup_child(ctrl);
        ctrl = next_ctrl;
    }
}

// 获取控件索引
auto LongUI::UIContainerBuiltIn::GetIndexOf(UIControl* child) const noexcept ->uint32_t {
    assert(this == child->parent && "不是亲生的");
    uint32_t index = 0;
    for (auto ctrl : (*this)) {
        if (ctrl == child) break;
        ++index;
    }
    return index;
}

// 随机访问控件
auto LongUI::UIContainerBuiltIn::GetAt(uint32_t i) const noexcept -> UIControl * {
   // 超出
    if (i >= m_cChildrenCount) return nullptr;
    // 第一个
    if (!i) return m_pHead;
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

// 交换
void LongUI::UIContainerBuiltIn::SwapChild(Iterator itr1, Iterator itr2) noexcept {
    auto ctrl1 = *itr1; auto ctrl2 = *itr2;
    assert(ctrl1 && ctrl2 && "bad arguments");
    assert(ctrl1->parent == this && ctrl2->parent == this && L"隔壁老王!");
    // 不一致时
    if (ctrl1 != ctrl2) {
        // A link B
        const bool a___b = ctrl1->next == ctrl2;
        // B link A
        const bool b___a = ctrl2->next == ctrl1;
        // A存在前驱
        if (ctrl1->prev) {
            if(!b___a) force_cast(ctrl1->prev->next) = ctrl2;
        }
        // A为头结点
        else {
            m_pHead = ctrl2;
        }
        // A存在后驱
        if (ctrl1->next) {
            if(!a___b) force_cast(ctrl1->next->prev) = ctrl2;
        }
        // A为尾结点
        else {
            m_pTail = ctrl2;
        }
        // B存在前驱
        if (ctrl2->prev) {
            if(!a___b) force_cast(ctrl2->prev->next) = ctrl1;
        }
        // B为头结点
        else {
            m_pHead = ctrl1;
        }
        // B存在后驱
        if (ctrl2->next) {
            if(!b___a) force_cast(ctrl2->next->prev) = ctrl1;
        }
        // B为尾结点
        else {
            m_pTail = ctrl1;
        }
        // 相邻交换
        auto swap_neibergs = [](UIControl* a, UIControl* b) noexcept {
            assert(a->next == b && "bad neibergs");
            force_cast(a->next) = b->next;
            force_cast(b->next) = a;
            force_cast(b->prev) = a->prev;
            force_cast(a->prev) = b;
        };
        // 相邻则结点?
        if (a___b) {
            swap_neibergs(ctrl1, ctrl2);
        }
        // 相邻则结点?
        else if (b___a) {
            swap_neibergs(ctrl2, ctrl1);
        }
        // 不相邻:交换前驱后驱
        else {
            std::swap(force_cast(ctrl1->prev), force_cast(ctrl2->prev));
            std::swap(force_cast(ctrl1->next), force_cast(ctrl2->next));
        }
#ifdef _DEBUG
        // 检查链表是否成环
        {
            auto count = m_cChildrenCount;
            auto debug_ctrl = m_pHead;
            while (debug_ctrl) {
                debug_ctrl = debug_ctrl->next;
                assert(count && "bad action 0 in swaping children");
                count--;
            }
            assert(!count && "bad action 1 in swaping children");
        }
        {
            auto count = m_cChildrenCount;
            auto debug_ctrl = m_pTail;
            while (debug_ctrl) {
                debug_ctrl = debug_ctrl->prev;
                assert(count && "bad action 2 in swaping children");
                count--;
            }
            assert(!count && "bad action 3 in swaping children");
        }
#endif
        // 刷新
        this->SetControlLayoutChanged();
        m_pWindow->Invalidate(this);
    }
    // 给予警告
    else {
        UIManager << DL_Warning
            << L"wanna to swap 2 children but just one"
            << LongUI::endl;
    }
}

// -------------------------- UIVerticalLayout -------------------------
// UIVerticalLayout 创建
auto LongUI::UIVerticalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIVerticalLayout* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIVerticalLayout, pControl, type, node);
    }
    return pControl;
}

// 更新子控件布局
void LongUI::UIVerticalLayout::RefreshLayout() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    {
        // 初始化
        float base_width = 0.f, base_height = 0.f, basic_weight = 0.f;
        // 第一次遍历
        for (auto ctrl : (*this)) {
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
        // 带入控件本身宽度计算
        base_width = std::max(base_width, this->GetViewWidthZoomed());
        // 剩余高度富余
        auto height_remain = std::max(this->GetViewHeightZoomed() - base_height, 0.f);
        // 单位权重高度
        auto height_in_unit_weight = basic_weight > 0.f ? height_remain / basic_weight : 0.f;
        // 基线Y
        float position_y = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
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
            ctrl->SetControlLayoutChanged();
            ctrl->SetLeft(0.f);
            ctrl->SetTop(position_y);
            ctrl->world;
            //ctrl->RefreshWorld();
            position_y += ctrl->GetTakingUpHeight();
        }
        // 修改
        m_2fContentSize.width = base_width * m_2fZoom.width;
        m_2fContentSize.height = position_y * m_2fZoom.height;
    }
    this->RefreshWorld();
}

// UIVerticalLayout 关闭控件
void LongUI::UIVerticalLayout::cleanup() noexcept {
    delete this;
}

// -------------------------- UIHorizontalLayout -------------------------
// UIHorizontalLayout 创建
auto LongUI::UIHorizontalLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIHorizontalLayout* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIHorizontalLayout, pControl, type, node);
    }
    return pControl;
}


// 更新子控件布局
void LongUI::UIHorizontalLayout::RefreshLayout() noexcept {
    // 基本算法:
    // 1. 去除浮动控件影响
    // 2. 一次遍历, 检查指定高度的控件, 计算基本高度/宽度
    // 3. 计算实际高度/宽度
    {
        // 初始化
        float base_width = 0.f, base_height = 0.f;
        float basic_weight = 0.f;
        // 第一次
        for (auto ctrl : (*this)) {
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
        // 计算
        base_height = std::max(base_height, this->GetViewHeightZoomed());
        // 剩余宽度富余
        auto width_remain = std::max(this->GetViewWidthZoomed() - base_width, 0.f);
        // 单位权重宽度
        auto width_in_unit_weight = basic_weight > 0.f ? width_remain / basic_weight : 0.f;
        // 基线X
        float position_x = 0.f;
        // 第二次
        for (auto ctrl : (*this)) {
            // 设置控件高度
            if (!(ctrl->flags & Flag_HeightFixed)) {
                ctrl->SetHeight(base_height);
            }
            // 设置控件宽度
            if (!(ctrl->flags & Flag_WidthFixed)) {
                ctrl->SetWidth(std::max(width_in_unit_weight * ctrl->weight, float(LongUIAutoControlMinSize)));
            }
            // 不管如何, 修改!
            ctrl->SetControlLayoutChanged();
            ctrl->SetLeft(position_x);
            ctrl->SetTop(0.f);
            //ctrl->RefreshWorld();
            position_x += ctrl->GetTakingUpWidth();
        }
        // 修改
        //UIManager << DL_Hint << this << position_x << endl;
        m_2fContentSize.width = position_x;
        m_2fContentSize.height = base_height;
        // 已经处理
        this->ControlLayoutChangeHandled();
    }
}


// UIHorizontalLayout 关闭控件
void LongUI::UIHorizontalLayout::cleanup() noexcept {
    delete this;
}

// --------------------- Single Layout ---------------
// UISingle 析构函数
LongUI::UISingle::~UISingle() noexcept {
    assert(m_pChild && "UISingle must host a child");
    this->cleanup_child(m_pChild);
}

// UISingle: 事件处理
bool LongUI::UISingle::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 初始化边缘控件 
            Super::DoEvent(arg);
            // 初次完成空间树建立
            assert(m_pChild && "UISingle must host a child");
            m_pChild->DoEvent(arg);
            return true;
        }
    }
    return Super::DoEvent(arg);
}

// UISingle 重建
auto LongUI::UISingle::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    assert(m_pChild && "UISingle must host a child");
    hr = m_pChild->Recreate();
    // 检查
    assert(SUCCEEDED(hr));
    return Super::Recreate();
}

// UISingle: 主景渲染
void LongUI::UISingle::render_chain_main() const noexcept {
    // 渲染帮助器
    Super::RenderHelper(this->begin(), this->end());
    // 父类主景
    Super::render_chain_main();
}

// UISingle: 渲染函数
void LongUI::UISingle::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UISingle: 刷新
void LongUI::UISingle::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(this->begin(), this->end());
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UISingle::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
    // 父类(边缘控件)
    auto mctrl = Super::FindChild(pt);
    if (mctrl) return mctrl;
    assert(m_pChild && "UISingle must host a child");
    // 检查
    if (IsPointInRect(m_pChild->visible_rect, pt)) {
        return m_pChild;
    }
    return nullptr;
}


// UISingle: 推入最后
void LongUI::UISingle::PushBack(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::PushBack(child);
    }
    // 一般的就自己处理
    else {
        // 检查
#ifdef _DEBUG
        auto old = UIControl::GetPlaceholder();
        this->cleanup_child(old);
        if (old != m_pChild) {
            UIManager << DL_Warning
                << L"m_pChild exist:"
                << m_pChild
                << LongUI::endl;
        }
#endif
        // 移除之前的
        this->cleanup_child(m_pChild);
        this->after_insert(m_pChild = child);
    }
}

// UISingle: 仅移除
void LongUI::UISingle::RemoveJust(UIControl* child) noexcept {
    assert(m_pChild == child && "bad argment");
    this->cleanup_child(child);
    m_pChild = UIControl::GetPlaceholder();
    Super::RemoveJust(child);
}

// UISingle: 更新布局
void LongUI::UISingle::RefreshLayout() noexcept {
    // 设置控件宽度
    if (!(m_pChild->flags & Flag_WidthFixed)) {
        m_pChild->SetWidth(this->GetViewWidthZoomed());
    }
    // 设置控件高度
    if (!(m_pChild->flags & Flag_HeightFixed)) {
        m_pChild->SetHeight(this->GetViewHeightZoomed());
    }
    // 不管如何, 修改!
    m_pChild->SetControlLayoutChanged();
    m_pChild->SetLeft(0.f);
    m_pChild->SetTop(0.f);
}

// UISingle 清理
void LongUI::UISingle::cleanup() noexcept {
    delete this;
}

// UISingle 创建空间
auto LongUI::UISingle::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UISingle* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UISingle, pControl, type, node);
    }
    return pControl;
}

// --------------------- Page Layout ---------------
// UIPage 构造函数
LongUI::UIPage::UIPage(UIContainer* cp) noexcept : Super(cp), 
    m_animation(AnimationType::Type_QuadraticEaseIn) {
    
}

// UIPage 析构函数
LongUI::UIPage::~UIPage() noexcept {
    for (auto ctrl : m_vChildren) {
        this->cleanup_child(ctrl);
    }
}

// UIPage: 事件处理
bool LongUI::UIPage::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 初始化边缘控件 
            Super::DoEvent(arg);
            // 子控件
            for (auto ctrl : m_vChildren) {
                ctrl->DoEvent(arg);
            }
            return true;
        }
    }
    return Super::DoEvent(arg);
}

// UIPage 重建
auto LongUI::UIPage::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    for (auto ctrl : (*this)) {
        hr = ctrl->Recreate();
        // 稍微检查一下
        assert(SUCCEEDED(hr));
    }
    return Super::Recreate();
}

// UIPage: 主景渲染
void LongUI::UIPage::render_chain_main() const noexcept {
    // 渲染帮助器
    Super::RenderHelper(&m_pNowDisplay, &m_pNowDisplay + 1);
    // 父类主景
    Super::render_chain_main();
}

// UIPage: 渲染函数
void LongUI::UIPage::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UIPage: 刷新
void LongUI::UIPage::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(this->begin(), this->end());
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIPage::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
    // 父类(边缘控件)
    auto mctrl = Super::FindChild(pt);
    if (mctrl) return mctrl;
    // 检查
    if (m_pNowDisplay && IsPointInRect(m_pNowDisplay->visible_rect, pt)) {
        return m_pNowDisplay;
    }
    return nullptr;
}


// UIPage: 推入最后
void LongUI::UIPage::PushBack(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::PushBack(child);
    }
    // 一般的就自己处理
    else {
        this->Insert(m_cChildrenCount, child);
    }
}

// UIPage: 插入
LongUINoinline void LongUI::UIPage::Insert(uint32_t index, UIControl* child) noexcept {
    assert(child && "bad argument");
    if (child) {
        m_pNowDisplay = child;
#ifdef _DEBUG
        auto dgb_result = false;
        dgb_result = (child->flags & Flag_HeightFixed) == 0;
        assert(dgb_result && "child of UIPage can not keep flag: Flag_HeightFixed");
        dgb_result = (child->flags & Flag_WidthFixed) == 0;
        assert(dgb_result && "child of UIPage can not keep flag: Flag_WidthFixed");
#endif
        m_vChildren.insert(index, child);
        this->after_insert(child);
        ++m_cChildrenCount;
        assert(m_vChildren.isok());
    }
}

// UIPage: 仅移除
void LongUI::UIPage::RemoveJust(UIControl* child) noexcept {
    auto itr = std::find(this->begin(), this->end(), child);
    // 没找到
    if (itr == this->end()) {
        UIManager << DL_Error
            << L"CHILD: " << child
            << L", NOT FOUND"
            << LongUI::endl;
    }
    // 找到了
    else {
        // 修改
        m_vChildren.erase(itr);
        --m_cChildrenCount;
        this->SetControlLayoutChanged();
        Super::RemoveJust(child);
    }
}

// UIPage: 更新布局
void LongUI::UIPage::RefreshLayout() noexcept {
    // 遍历
    for (auto ctrl : m_vChildren) {
        // 设置控件左边坐标
        ctrl->SetLeft(0.f);
        // 设置控件顶部坐标
        ctrl->SetTop(0.f);
        // 设置控件宽度
        ctrl->SetWidth(this->GetViewWidthZoomed());
        // 设置控件高度
        ctrl->SetHeight(this->GetViewHeightZoomed());
        // 不管如何, 修改!
        ctrl->SetControlLayoutChanged();
    }
}

// UIPage 清理
void LongUI::UIPage::cleanup() noexcept {
    delete this;
}

// UIPage 创建空间
auto LongUI::UIPage::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIPage* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIPage, pControl, type, node);
    }
    return pControl;
}

// --------------------- Floating Layout ---------------
// UIFloatLayout 创建
auto LongUI::UIFloatLayout::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UIFloatLayout* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIFloatLayout, pControl, type, node);
    }
    return pControl;
}

// 更新子控件布局
void LongUI::UIFloatLayout::RefreshLayout() noexcept {
    // 布局上下文: 做为left-top坐标
    // 布局权重:   暂时无用
    for (auto ctrl : (*this)) {
        ctrl->SetLeft(ctrl->context[0]);
        ctrl->SetTop(ctrl->context[1]);
        ctrl->SetControlLayoutChanged();
    }
}

// UIFloatLayout 关闭控件
void LongUI::UIFloatLayout::cleanup() noexcept {
    delete this;
}
                   
                    

#ifdef _DEBUG
void longui_dbg_update(LongUI::UIControl* control) noexcept {
    assert(control && "bad argments");
    if (control->debug_updated) {
        auto name = control->name;
        name = nullptr;
    }
}

// longui naemspace
namespace LongUI {
    // debug var
    extern std::atomic_uintptr_t g_dbg_last_proc_window_pointer;
    extern std::atomic<UINT> g_dbg_last_proc_message;
}

// debug functin -- 
void longui_dbg_locked(const LongUI::CUILocker&) noexcept {
    std::uintptr_t ptr = LongUI::g_dbg_last_proc_window_pointer;
    UINT msg = LongUI::g_dbg_last_proc_message;
    auto window = reinterpret_cast<LongUI::UIWindow*>(ptr);
#if 0
    UIManager << DL_Log
        << L"main locker locked @"
        << window
        << L" on message id: "
        << long(msg)
        << LongUI::endl;
#else
    ::OutputDebugStringW(LongUI::Formated(
        L"Main Locker Locked On Msg: %4u @ Window[0x%p - %S]\r\n",
        msg, window, window->name.c_str()
        ));
#endif
}

#endif

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

// longui::impl namespace
namespace LongUI { namespace impl { static float const floatx2[] = { 0.f, 0.f }; } }

/// <summary>
/// Initializes a new instance of the <see cref="LongUI::UIControl" />
/// class with xml node
/// </summary>
/// <param name="parent">The parent for self in control-level</param>
/// <returns></returns>
/// <remarks>
/// For this function, param 'node' could be null node
/// 对于本函数, 参数'node'允许为空
/// <see cref="LongUINullXMLNode" />
/// </remarks>
LongUI::UIControl::UIControl(UIContainer* parent) noexcept :
    parent(parent), 
    context(),
    m_stateBasic(uint8_t(-1)),
    level(parent ? (parent->level + 1ui8) : 0ui8),
    m_pWindow(parent ? parent->GetWindow() : nullptr) {
    // 溢出错误
    if (this->level == 255) {
        UIManager << DL_Error
            << L"too deep for this tree"
            << LongUI::endl;
        assert(!"too deep");
    }
}

/// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
/// <remarks>
/// For this function, param 'node' could be null node
/// 对于本函数, 参数'node'允许为空
/// <see cref="LongUINullXMLNode" />
/// </remarks>
void LongUI::UIControl::initialize(pugi::xml_node node) noexcept  {
#ifdef _DEBUG
    // 没有被初始化
    assert(this->debug_checker.Test(DEBUG_CHECK_INIT) == false && "had been initialized");
#endif
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
        // 检查布局上下文
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::LayoutContext).value(),
            force_cast(this->context), 
            lengthof<uint32_t>(this->context)
            );
        // 检查背景笔刷
        if (data = node.attribute(LongUI::XMLAttribute::BackgroudBrush).value()) {
            m_idBackgroudBrush = uint16_t(LongUI::AtoI(data));
            if (m_idBackgroudBrush) {
                assert(!m_pBackgroudBrush);
                m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
            }
        }
        // 检查可视性
        this->SetVisible(node.attribute(LongUI::XMLAttribute::Visible).as_bool(true));
        // 检查名称
        if (m_pWindow) {
            auto basestr = node.attribute(LongUI::XMLAttribute::ControlName).value();
#ifdef _DEBUG
            char buffer[128];
            if (!basestr) {
                static long s_dbg_longui_index = 0;
                buffer[0] = 0;
                ++s_dbg_longui_index;
                auto c = std::snprintf(
                    buffer, 128, 
                    "dbg_longui_%s_id_%ld", 
                    node.name(),
                    s_dbg_longui_index
                    );
                assert(c > 0 && "bad std::snprintf call");
                // 小写
                auto mystrlow = [](char* str) noexcept {
                    while (*str) {
                        if (*str >= 'A' && *str <= 'Z') *str += 'a' - 'A';
                        ++str;
                    }
                };
                mystrlow(buffer);
                basestr = buffer;
            }
#endif
            if (basestr) {
                auto namestr = m_pWindow->CopyStringSafe(basestr);
                force_cast(this->name) = namestr;
            }
        }
        // 检查外边距
        Helper::MakeFloats(
            node.attribute(LongUI::XMLAttribute::Margin).value(),
            const_cast<float*>(&margin_rect.left),
            sizeof(margin_rect) / sizeof(margin_rect.left)
            );
        // 检查渲染父控件
        if (node.attribute(LongUI::XMLAttribute::IsRenderParent).as_bool(false)) {
            assert(this->parent && "RenderParent but no parent");
            force_cast(this->prerender) = this->parent->prerender;
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
                size, lengthof<uint32_t>(size)
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
        // 禁止
        if (!node.attribute(LongUI::XMLAttribute::Enabled).as_bool(true)) {
            this->SetEnabled(false);
        }
    }
    // 修改flag
    force_cast(this->flags) |= flag;
#ifdef _DEBUG
    // 被初始化
    this->debug_checker.SetTrue(DEBUG_CHECK_INIT);
#endif
}

// 析构函数
LongUI::UIControl::~UIControl() noexcept {
    if (this->parent) 
        this->parent->RemoveChildReference(this);
    m_pWindow->RemoveControlReference(this);
    LongUI::SafeRelease(m_pBrush_SetBeforeUse);
    LongUI::SafeRelease(m_pBackgroudBrush);
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

// UIControl:: 渲染调用链: 背景
void LongUI::UIControl::render_chain_background() const noexcept {
#ifdef _DEBUG
    // 重复调用检查
    if (this->debug_checker.Test(DEBUG_CHECK_BACK)) {
        AutoLocker;
        UIManager << DL_Error
            << L"check logic: called twice in one time"
            << this
            << endl;
    }
    force_cast(this->debug_checker).SetTrue(DEBUG_CHECK_BACK);
#endif
    if (m_pBackgroudBrush) {
        D2D1_RECT_F rect; this->GetViewRect(rect);
        LongUI::FillRectWithCommonBrush(UIManager_RenderTarget, m_pBackgroudBrush, rect);
    }
}

// UIControl:: 渲染调用链: 前景
void LongUI::UIControl::render_chain_foreground() const noexcept {
#ifdef _DEBUG
    // 重复调用检查
    if (this->debug_checker.Test(DEBUG_CHECK_FORE)) {
        AutoLocker;
        UIManager << DL_Error
            << L"check logic: called twice in one time"
            << this
            << endl;
    }
    force_cast(this->debug_checker).SetTrue(DEBUG_CHECK_FORE);
#endif
    // 后继结点判断, B控件深度必须比A深
    auto is_successor = [](const UIControl* const a, const UIControl* b) noexcept {
        const auto target = a->level;
        while (b->level > target) b = b->parent;
        return a == b;
    };
    // 渲染边框
    if (m_fBorderWidth > 0.f) {
        UIManager_RenderTarget->SetTransform(&this->world);
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
}


// UI控件: 刷新
void LongUI::UIControl::AfterUpdate() noexcept {
    // 控件大小处理了
    if (m_bool16.Test(Index_ChangeSizeHandled)) {
        m_bool16.SetFalse(Index_ChangeLayout);
        m_bool16.SetFalse(Index_ChangeSizeHandled);
    }
    // 世界转换处理了
    if (m_bool16.Test(Index_ChangeWorldHandled)) {
        m_bool16.SetFalse(Index_ChangeWorld);
        m_bool16.SetFalse(Index_ChangeWorldHandled);
    }
#ifdef _DEBUG
    assert(debug_updated && "must call Update() before this");
    debug_updated = false;
    this->debug_checker.SetFalse(DEBUG_CHECK_BACK);
    this->debug_checker.SetFalse(DEBUG_CHECK_MAIN);
    this->debug_checker.SetFalse(DEBUG_CHECK_FORE);
#endif
}

// UI控件: 重建
auto LongUI::UIControl::Recreate() noexcept ->HRESULT {
    // 增加计数
#ifdef _DEBUG
    ++this->debug_recreate_count;
    if (this->debug_this) {
        UIManager << DL_Log
            << "create count: "
            << long(this->debug_recreate_count)
            << LongUI::endl;
    }
    if (debug_recreate_count > 1 && this->debug_this) {
        UIManager << DL_Hint
            << "create count: "
            << long(this->debug_recreate_count)
            << LongUI::endl;
    }
#endif
    // 设备重置再说
    LongUI::SafeRelease(m_pBrush_SetBeforeUse);
    LongUI::SafeRelease(m_pBackgroudBrush);
    m_pBrush_SetBeforeUse = static_cast<decltype(m_pBrush_SetBeforeUse)>(
        UIManager.GetBrush(LongUICommonSolidColorBrushIndex)
        );
    if (m_idBackgroudBrush) {
        m_pBackgroudBrush = UIManager.GetBrush(m_idBackgroudBrush);
    }
    return S_OK;
}

// 测试是否为子孙结点
bool LongUI::UIControl::IsPosterityForSelf(const UIControl* test) const noexcept {
    const auto target = this->level;
    while (test->level > target) test = test->parent;
    return this == test;
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
        this->SetControlLayoutChanged();
    }
    // 检查
    if (this->view_size.width < 0.f && this->parent->view_size.width > 0.f) {
        UIManager << DL_Hint << this
            << "viewport's width < 0: " << this->view_size.width
            << endl;
    }
}

// 设置占用高度
auto LongUI::UIControl::SetHeight(float height) noexcept -> void LongUINoinline {
    // 设置
    auto new_vheight = height - this->GetNonContentHeight();
    if (new_vheight != this->view_size.height) {
        force_cast(this->view_size.height) = new_vheight;
        this->SetControlLayoutChanged();
    }
    // 检查
    if (this->view_size.height < 0.f && this->parent->view_size.height > 0.f) {
        UIManager << DL_Hint << this
            << "viewport's height < 0: " << this->view_size.height
            << endl;
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

// 父控件视角: 获取占用/剪切矩形
void LongUI::UIControl::GetClipRectFP(D2D1_RECT_F& rect) const noexcept {
    rect.left = -(this->margin_rect.left + m_fBorderWidth);
    rect.top = -(this->margin_rect.top + m_fBorderWidth);
    rect.right = this->view_size.width + this->margin_rect.right + m_fBorderWidth;
    rect.bottom = this->view_size.height + this->margin_rect.bottom + m_fBorderWidth;
    rect.left += this->view_pos.x;
    rect.top += this->view_pos.y;
    rect.right += this->view_pos.x;
    rect.bottom += this->view_pos.y;
}


// 获得世界转换矩阵
void LongUI::UIControl::RefreshWorld() noexcept {
    float xx = this->view_pos.x;
    float yy = this->view_pos.y;
    // 顶级控件
    if (this->IsTopLevel()) {
        this->world = DX::Matrix3x2F::Translation(xx, yy);
    }
    // 非顶级控件
    else {
#if 1
        // 检查
        xx += this->parent->GetOffsetXZoomed();
        yy += this->parent->GetOffsetYZoomed();
        // 转换
        this->world =
            DX::Matrix3x2F::Translation(xx, yy)
            *DX::Matrix3x2F::Scale(
                this->parent->GetZoomX(), this->parent->GetZoomY()
                )
            * this->parent->world;
#else
        this->world =
            DX::Matrix3x2F::Translation(xx, yy)
            * DX::Matrix3x2F::Scale(
                this->parent->GetZoomX(), this->parent->GetZoomY()
                )
            * DX::Matrix3x2F::Translation(
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

/// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIMarginalable::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 有效结点
    if (node) {
        // 获取类型
        auto get_type = [](pugi::xml_node node, MarginalControl bad_match) noexcept {
            // 属性值列表
            const char* mode_list[] = { "left", "top", "right", "bottom", };
            // 设置
            Helper::GetEnumProperties prop;
            prop.values_list = mode_list;
            prop.values_length = lengthof<uint32_t>(mode_list);
            prop.bad_match = static_cast<uint32_t>(bad_match);
            auto value = node.attribute(XMLAttribute::MarginalDirection).value();
            // 调用
            return static_cast<MarginalControl>(GetEnumFromString(value, prop));
        };
        // 获取类型
        force_cast(this->marginal_type) = get_type(node, Control_Unknown);
    }
    // 检查类型
    if (this->marginal_type != Control_Unknown) {
        assert(this->marginal_type < MARGINAL_CONTROL_SIZE && "bad marginal_type");
        force_cast(this->flags) |= Flag_MarginalControl;
    }
    // 本类已被初始化
#ifdef _DEBUG
#endif
}


// 获得世界转换矩阵 for 边缘控件
void LongUI::UIMarginalable::RefreshWorldMarginal() noexcept {
    float xx = this->view_pos.x /*+ this->margin_rect.left + m_fBorderWidth*/;
    float yy = this->view_pos.y /*+ this->margin_rect.top + m_fBorderWidth*/;
    D2D1_MATRIX_3X2_F identity;
    D2D1_MATRIX_3X2_F* parent_world = &identity;
    // 顶级
    identity = DX::Matrix3x2F::Identity();
    if (this->parent->IsTopLevel()) {
        identity = DX::Matrix3x2F::Identity();
    }
    else {
        auto pp = this->parent;
        xx -= pp->GetLeftMarginOffset();
        yy -= pp->GetTopMarginOffset();
        // 检查
        parent_world = &pp->world;
    }
    // 计算矩阵
    this->world = DX::Matrix3x2F::Translation(xx, yy) ** parent_world;
    // 自己不能是顶级的
    assert(this->IsTopLevel() == false);
    constexpr long aa = sizeof(UIContainer);
}

// ----------------------------------------------------------------------------
// UINull
// ----------------------------------------------------------------------------

// LongUI namespace
namespace LongUI {
    // null control
    class UINull : public UIControl {
        // 父类申明
        using Super = UIControl;
        // clean this control 清除控件
        virtual void cleanup() noexcept override { delete this; }
    public:
        // Render 渲染
        virtual void Render() const noexcept override {}
        // update 刷新
        virtual void Update() noexcept override { Super::Update(); }
        // do event 事件处理
        //virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override { return false; }
    public:
        // 创建控件
        static auto CreateControl(UIContainer* ctrlparent, pugi::xml_node node) noexcept {
            UIControl* pControl = nullptr;
            // 判断
            if (!node) {
                UIManager << DL_Hint << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = new(std::nothrow) UINull(ctrlparent);
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            return pControl;
        }
    public:
        // constructor 构造函数
        UINull(UIContainer* cp) noexcept : Super(cp) {}
        // destructor 析构函数
        ~UINull() noexcept { }
    protected:
        // init
        void initialize(pugi::xml_node node) noexcept { Super::initialize(node); }
    };
    // space holder
    class UISpaceHolder final : public UINull {
        // 父类申明
        using Super = UINull;
        // clean this control 清除控件
        virtual void cleanup() noexcept override { }
    public:
        // create
        virtual auto Recreate() noexcept ->HRESULT override { return S_OK; }
    public:
        // constructor 构造函数
        UISpaceHolder() noexcept : Super(nullptr) { Super::initialize(LongUINullXMLNode); }
        // destructor 析构函数
        ~UISpaceHolder() noexcept { }
    };
    // 占位控件
    static char g_control[sizeof(UISpaceHolder)];
    // 占位控件初始化
    struct CUISpaceHolderInit { CUISpaceHolderInit() noexcept { reinterpret_cast<UISpaceHolder*>(g_control)->UISpaceHolder::UISpaceHolder(); } } g_init_holder;
    // 获取占位控件
    auto LongUI::UIControl::GetPlaceholder() noexcept -> UIControl* {
        return reinterpret_cast<UIControl*>(&g_control);
    }
}

// 创建空控件
auto WINAPI LongUI::CreateNullControl(CreateEventType type, pugi::xml_node node) noexcept -> UIControl * {
    // 分类判断
    UIControl* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        pControl = UINull::CreateControl(reinterpret_cast<UIContainer*>(type), node);
    }
    return pControl;
}


// ------------------------------ UIContainer -----------------------------
/// <summary>
/// UIs the container.
/// </summary>
/// <param name="cp">The parent for self in control-level</param>
/// <returns></returns>
LongUI::UIContainer::UIContainer(UIContainer* cp) noexcept : Super(cp), marginal_control() {
    std::memset(force_cast(marginal_control), 0, sizeof(marginal_control));
}

/// <summary>
/// Initializes with specified cxml-node
/// </summary>
/// <param name="cp">The cp.</param>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIContainer::initialize(pugi::xml_node node) noexcept {
#ifdef _DEBUG
    for (auto ctrl : marginal_control) {
        assert(ctrl == nullptr && "bad action");
    }
#endif
    // 链式调用
    Super::initialize(node);
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
        // 渲染依赖属性
        /*if (node.attribute(XMLAttribute::IsHostChildrenAlways).as_bool(false)) {
            flag |= LongUI::Flag_Container_HostChildrenRenderingDirectly;
        }*/
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
            ctrl->cleanup();
        }
    }
}

// 插入后处理
void LongUI::UIContainer::after_insert(UIControl* child) noexcept {
    assert(child && "bad argument");
    // 添加到窗口速查表
    if (child->name[0]) {
        m_pWindow->AddNamedControl(child);
    };
    // 大小判断
    if (this->GetCount() >= 10'000) {
        UIManager << DL_Warning << "the count of children must be"
            " less than 10k because of the precision of float" << LongUI::endl;
    }
    // 检查flag
    if (this->flags & Flag_Container_HostPosterityRenderingDirectly) {
        force_cast(child->prerender) = this->prerender;
        // 子控件也是容器?(不是也无所谓了)
        force_cast(child->flags) |= Flag_Container_HostPosterityRenderingDirectly;
    }
    // 设置父结点
    assert(child->parent == this);
    // 设置窗口结点
    assert(child->m_pWindow == m_pWindow);
    // 重建资源
    child->Recreate();
    // 修改
    child->SetControlLayoutChanged();
    // 修改
    this->SetControlLayoutChanged();
}

/// <summary>
/// Find the control via mouse point
/// </summary>
/// <param name="pt">The wolrd mouse point.</param>
/// <returns>the control pointer, maybe nullptr</returns>
auto LongUI::UIContainer::FindChild(const D2D1_POINT_2F& pt) noexcept ->UIControl* {
    // 查找边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl && IsPointInRect(ctrl->visible_rect, pt)) {
                return ctrl;
            }
        }
    }
    this->AssertMarginalControl();
    return nullptr;
}


// do event 事件处理
bool LongUI::UIContainer::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // ------------------------------------ 主函数
    bool done = false;
    // 处理窗口事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 边界控件
            for (auto mctrl : marginal_control) {
                if(mctrl) mctrl->DoEvent(arg);
            }
            done = true;
            break;
        }
    }
    // 扳回来
    return done;
}

// 处理鼠标事件
bool LongUI::UIContainer::DoMouseEvent(const LongUI::MouseEventArgument& arg) noexcept {
    // 离开
    if (arg.event == LongUI::MouseEvent::Event_MouseLeave) {
        if (m_pMousePointed) {
            m_pMousePointed->DoMouseEvent(arg);
            m_pMousePointed = nullptr;
        }
        return true;
    }
    // 查找子控件
    auto control_got = this->FindChild(arg.pt);
    // 不可视算没有
    if (control_got && !control_got->GetVisible()) control_got = nullptr;
    // 不同
    if (control_got != m_pMousePointed && arg.event == LongUI::MouseEvent::Event_MouseMove) {
        auto newarg = arg;
        // 有效
        if (m_pMousePointed) {
            newarg.event = LongUI::MouseEvent::Event_MouseLeave;
            m_pMousePointed->DoMouseEvent(newarg);
        }
        // 有效
        if ((m_pMousePointed = control_got)) {
            newarg.event = LongUI::MouseEvent::Event_MouseEnter;
            m_pMousePointed->DoMouseEvent(newarg);
        }
    }
    // 有效
    if (control_got) {
        // 左键点击设置键盘焦点
        if (arg.event == LongUI::MouseEvent::Event_LButtonDown) {
            m_pWindow->SetFocus(control_got);
        }
        // 鼠标移动设置hover跟踪
        else if (arg.event == LongUI::MouseEvent::Event_MouseMove) {
            m_pWindow->SetHoverTrack(control_got);
        }
        // 相同
        if (control_got->DoMouseEvent(arg)) {
            return true;
        }
    }
    // 滚轮事件允许边缘控件后处理
    if (arg.event <= MouseEvent::Event_MouseWheelH && this->flags & Flag_Container_ExistMarginalControl) {
        // 优化
        for (auto ctrl : this->marginal_control) {
            if (ctrl && ctrl->DoMouseEvent(arg)) {
                return true;
            }
        }
        this->AssertMarginalControl();
    }
    return false;
}

// 渲染子控件
void LongUI::UIContainer::child_do_render(const UIControl* ctrl) noexcept {
    // 可渲染?
    if (ctrl->GetVisible() && ctrl->visible_rect.right > ctrl->visible_rect.left
        && ctrl->visible_rect.bottom > ctrl->visible_rect.top) {
        // 修改世界转换矩阵
        UIManager_RenderTarget->SetTransform(&ctrl->world);
        // 检查剪切规则
        if (ctrl->flags & Flag_ClipStrictly) {
            D2D1_RECT_F clip_rect; ctrl->GetClipRect(clip_rect);
            UIManager_RenderTarget->PushAxisAlignedClip(&clip_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        // 渲染
        ctrl->Render();
        // 检查剪切规则
        if (ctrl->flags & Flag_ClipStrictly) {
            UIManager_RenderTarget->PopAxisAlignedClip();
        }
    }
}

// UIContainer: 主景渲染
void LongUI::UIContainer::UIContainer::render_chain_main() const noexcept {
    // 渲染边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl) {
                this->child_do_render(ctrl);
            }
        }
    }
    this->AssertMarginalControl();
    // 回退转变
    UIManager_RenderTarget->SetTransform(&this->world);
    // 父类
    Super::render_chain_main();
}

// 添加边界控件
void LongUI::UIContainer::PushBack(UIControl* child) noexcept {
    assert(child && "bad argment");
    assert((child->flags & Flag_MarginalControl) && "bad argment");
    if (child && (child->flags & Flag_MarginalControl)) {
        auto mctrl = longui_cast<UIMarginalable*>(child);
        assert(mctrl->marginal_type != UIMarginalable::Control_Unknown && "bad marginal control");
        assert(mctrl->parent == this && "bad child");
        // 错误
        if (this->marginal_control[mctrl->marginal_type]) {
            UIManager << DL_Error
                << "target marginal control has been existd, check xml-layout"
                << LongUI::endl;
            this->cleanup_child(this->marginal_control[mctrl->marginal_type]);
        }
        // 写入
        force_cast(this->marginal_control[mctrl->marginal_type]) = mctrl;
        // 插♂入后
        this->after_insert(mctrl);
        // 推入flag
        force_cast(this->flags) |= Flag_Container_ExistMarginalControl;
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
    const float this_container_left = this->GetLeft();
    const float this_container_top = this->GetTop();
    assert(this_container_width == this->GetWidth());
    assert(this_container_height == this->GetHeight());
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
            this->SetLeft(this_container_left);
            this->SetTop(this_container_top);
            this->SetWidth(this_container_width);
            this->SetHeight(this_container_height);
        }
    }
    this->RefreshWorld();
    this->RefreshLayout();
}

// UI容器: 刷新
void LongUI::UIContainer::Update() noexcept {
    // 修改自动缩放控件
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
            this->m_2fZoom.width = tmpw;
            this->m_2fZoom.height = tmph;
            break;
        }
    }
    // 修改边界
    if (this->IsControlLayoutChanged()) {
        // 更新布局
        this->RefreshLayout();
        // 刷新边缘控件
        if (this->flags & Flag_Container_ExistMarginalControl) {
            this->refresh_marginal_controls();
        }
        // 处理
        this->ControlLayoutChangeHandled();
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
    if (this->IsNeedRefreshWorld() && (this->flags & Flag_Container_ExistMarginalControl)) {
        for (auto ctrl : this->marginal_control) {
            // 刷新
            if (ctrl) {
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
#ifdef _DEBUG
                if (ctrl->debug_this) {
                    UIManager << DL_Log << ctrl
                        << " visible rect changed to: "
                        << ctrl->visible_rect << endl;
                }
#endif
            }
        }
        // 已处理该消息
        this->ControlLayoutChangeHandled();
    }
    // 刷新父类
    return Super::Update();
}

// UIContainer 重建
auto LongUI::UIContainer::Recreate() noexcept ->HRESULT {
    auto hr = S_OK;
    // 重建边缘控件
    if (this->flags & Flag_Container_ExistMarginalControl) {
        for (auto ctrl : this->marginal_control) {
            if (ctrl) {
                hr = ctrl->Recreate();
                assert(SUCCEEDED(hr));
            }
        }
    }
    // skip if before 'control-tree-finshed'
    // this->AssertMarginalControl();
    // 重建父类
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    return hr;
}

// 设置水平偏移值
void LongUI::UIContainer::SetOffsetX(float value) noexcept {
    assert(value > -1'000'000.f && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    float target = value;
    if (target != m_2fOffset.x) {
        m_2fOffset.x = target;
        this->SetControlWorldChanged();
    }
}

// 设置垂直偏移值
void LongUI::UIContainer::SetOffsetY(float value) noexcept {
    assert(value > (-1'000'000.f) && value < 1'000'000.f &&
        "maybe so many children in this container that over single float's precision");
    float target = value;
    if (target != m_2fOffset.y) {
        m_2fOffset.y = target;
        this->SetControlWorldChanged();
    }
}


// ------------------------ HELPER ---------------------------
// sb调用帮助器
bool LongUI::UIControl::call_uievent(const UICallBack& call, SubEvent sb) noexcept(noexcept(call.operator())) {
    // 事件
    LongUI::EventArgument arg;
    arg.event = LongUI::Event::Event_SubEvent;
    arg.sender = this;
    arg.ui.subevent = sb;
    arg.ui.pointer = nullptr;
    arg.ctrl = nullptr;
    // 返回值
    auto code = false;
    // 脚本最先
    if (UIManager.script && m_script.script) {
        auto rc = UIManager.script->Evaluation(this->GetScript(), arg);
        code = rc || code;
    }
    // 回调其次
    if (call.IsOK()) {
        auto rc = call(this);
        code = rc || code;
    }
    // 事件最低
    auto rc  = m_pWindow->DoEvent(arg);
    return rc || code;
}

// 延迟清理
void LongUI::UIControl::delay_cleanup() noexcept {
    UIManager.PushDelayCleanup(this);
}
                   

// ----------------------------------------------------------------------------
// **** UIText
// ----------------------------------------------------------------------------

// 前景渲染
void LongUI::UIText::render_chain_foreground() const noexcept {
    // 文本算前景
    m_text.Render(0.f, 0.f);
    // 父类
    Super::render_chain_foreground();
}

// UI文本: 渲染
void LongUI::UIText::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UIText: 刷新
void LongUI::UIText::Update() noexcept {
    // 改变了大小
    if(this->IsControlLayoutChanged()) {
        // 设置大小
        m_text.Resize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlLayoutChangeHandled();
    }
    return Super::Update();
}

// UIText: 事件响应
bool LongUI::UIText::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LONGUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetText:
            m_text = arg.stt.text;
            m_pWindow->Invalidate(this);
            __fallthrough;
        case LongUI::Event::Event_GetText:
            arg.str = m_text.c_str();
            return true;
        case LongUI::Event::Event_SetEnabled:
            // 修改状态
            m_text.SetState(arg.ste.enabled ? State_Normal : State_Disabled);
        }
    }
    return Super::DoEvent(arg);
}


/*/ UIText 构造函数
LongUI::UIText::UIText(pugi::xml_node node) noexcept: Super(node), m_text(node) {
    //m_bInitZoneChanged = true;
}
*/

// UIText::CreateControl 函数
auto LongUI::UIText::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIText* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIText, pControl, type, node);
    }
    return pControl;
}


// recreate 重建
/*HRESULT LongUI::UIText::Recreate() noexcept {
// 断言
return Super::Recreate();
}*/


// close this control 关闭控件
void LongUI::UIText::cleanup() noexcept {
    delete this;
}

// ----------------------------------------------------------------------------
// **** UIButton
// ----------------------------------------------------------------------------

// UIButton: 前景渲染
void LongUI::UIButton::render_chain_background() const noexcept {
    // UI部分算作前景
    D2D1_RECT_F draw_rect;
    this->GetViewRect(draw_rect);
    m_uiElement.Render(draw_rect);
    // 父类前景
    Super::render_chain_background();
}

// Render 渲染 
void LongUI::UIButton::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI按钮: 刷新
void LongUI::UIButton::Update() noexcept {
    // 更新计时器
    m_uiElement.Update();
    return Super::Update();
}

// UIButton 构造函数
void LongUI::UIButton::initialize(pugi::xml_node node) noexcept {
    // 链式初始化
    Super::initialize(node);
    m_uiElement.Init(this->check_state(), 0, node);
    // 允许键盘焦点
    auto flag = this->flags | Flag_Focusable;
    // 初始化
    Helper::SetBorderColor(node, m_aBorderColor);
    constexpr int azz = sizeof(m_uiElement);
    // 修改
    force_cast(this->flags) = flag;
}


// UIButton::CreateControl 函数
auto LongUI::UIButton::CreateControl(CreateEventType type,pugi::xml_node node) noexcept ->UIControl* {
    UIButton* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIButton, pControl, type, node);
    }
    return pControl;
}


// do event 事件处理
bool LongUI::UIButton::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // longui 消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetFocus:
            // 设置焦点: 要求焦点
            return true;
        case LongUI::Event::Event_KillFocus:
            // 释放焦点:
            m_tarStateClick = LongUI::State_Normal;
            return true;
        case LongUI::Event::Event_SetEnabled:
            // 修改状态
            m_uiElement.SetBasicState(arg.ste.enabled ? State_Normal : State_Disabled);
            return Super::DoEvent(arg);
        }
    }
    return Super::DoEvent(arg);
}

// 鼠标事件处理
bool LongUI::UIButton::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // 禁用状态禁用鼠标消息
    if (!this->GetEnabled()) return true;
    // 转换坐标
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // 鼠标 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        // 鼠标移进: 设置UI元素状态
        this->SetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        return true;
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出: 设置UI元素状态
        this->SetControlState(LongUI::State_Normal);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        // 左键按下:
        m_pWindow->SetCapture(this);
        this->SetControlState(LongUI::State_Pushed);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Pushed];
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 左键弹起:
        if (m_pWindow->IsReleasedControl(this)) {
            bool rec = this->call_uievent(m_event, SubEvent::Event_ItemClicked);
            rec = false;
            // 设置状态
            this->SetControlState(m_tarStateClick);
            m_colorBorderNow = m_aBorderColor[m_tarStateClick];
            m_pWindow->ReleaseCapture();
        }
        return true;
    }
    // 未处理的消息
    return false;
}

// recreate 重建
auto LongUI::UIButton::Recreate() noexcept ->HRESULT {
    // 重建元素
    m_uiElement.Recreate();
    // 父类处理
    return Super::Recreate();
}

// 添加事件监听器(雾)
bool LongUI::UIButton::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    // 点击
    if (sb == SubEvent::Event_ItemClicked) {
        m_event += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

// 关闭控件
void LongUI::UIButton::cleanup() noexcept {
    delete this;
}


// ----------------------------------------------------------------------------
// **** UIEdit
// ----------------------------------------------------------------------------

// UI基本编辑控件: 前景渲染
void LongUI::UIEditBasic::render_chain_foreground() const noexcept {
    // 文本算前景
    m_text.Render(0.f, 0.f);
    // 父类
    Super::render_chain_foreground();
}

// UI基本编辑控件: 渲染
void LongUI::UIEditBasic::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI基本编辑框: 刷新
void LongUI::UIEditBasic::Update() noexcept {
    // 改变了大小
    if (this->IsControlLayoutChanged()) {
        // 设置大小
        m_text.Resize(this->view_size.width, this->view_size.height);
        // 已经处理
        this->ControlLayoutChangeHandled();
    }
    // 刷新
    m_text.Update();
    return Super::Update();
}

// UI基本编辑控件
bool  LongUI::UIEditBasic::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI 消息
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            __fallthrough;
        case LongUI::Event::Event_SubEvent:
            return true;
        case LongUI::Event::Event_SetFocus:
            m_text.OnSetFocus();
            return true;
        case LongUI::Event::Event_KillFocus:
            m_text.OnKillFocus();
            return true;
        case LongUI::Event::Event_SetText:
            assert(!"NOIMPL");
            __fallthrough;
        case LongUI::Event::Event_GetText:
            arg.str = m_text.c_str();
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
        }
    }
    return true;
}

// UI基本编辑控件: 鼠标事件
bool  LongUI::UIEditBasic::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // LongUI 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_DragEnter:
        m_text.OnDragEnter(arg.cf.dataobj, arg.cf.outeffect);
        break;
    case LongUI::MouseEvent::Event_DragOver:
        m_text.OnDragOver(pt4self.x, pt4self.y);
        break;
    case LongUI::MouseEvent::Event_DragLeave:
        m_text.OnDragLeave();
        break;
    case LongUI::MouseEvent::Event_Drop:
        m_text.OnDrop(arg.cf.dataobj, arg.cf.outeffect);
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        m_pWindow->now_cursor = m_hCursorI;
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        m_pWindow->now_cursor = m_pWindow->default_cursor;
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        // 拖拽?
        if (arg.sys.wParam & MK_LBUTTON) {
            m_text.OnLButtonHold(pt4self.x, pt4self.y);
        }
        break;
    case LongUI::MouseEvent::Event_LButtonDown:
        m_text.OnLButtonDown(pt4self.x, pt4self.y, !!(arg.sys.wParam & MK_SHIFT));
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        m_text.OnLButtonUp(pt4self.x, pt4self.y);
        break;
    }
    return true;
}

// close this control 关闭控件
HRESULT LongUI::UIEditBasic::Recreate() noexcept {
    m_text.Recreate();
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UIEditBasic::cleanup() noexcept {
    delete this;
}

// 构造函数
void LongUI::UIEditBasic::initialize(pugi::xml_node node) noexcept {
    // 链式初始化
    Super::initialize(node);
    m_text.Init(node);
    // 允许键盘焦点
    auto flag = this->flags | Flag_Focusable;
    if (node) {

    }
    // 修改
    force_cast(this->flags) = flag;
}

// UIEditBasic::CreateControl 函数
LongUI::UIControl* LongUI::UIEditBasic::CreateControl(CreateEventType type,pugi::xml_node node) noexcept {
    // 分类判断
    UIEditBasic* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIEditBasic, pControl, type, node);
    }
    return pControl;
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

// UI复选框: 调试信息
bool LongUI::UICheckBox::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UICheckBox";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UICheckBox";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UICheckBox>()
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

// LongUI内建容器: 调试信息
bool LongUI::UIContainerBuiltIn::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIContainerBuiltIn";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIContainerBuiltIn";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIContainerBuiltIn>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// LongUI单独容器: 调试信息
bool LongUI::UISingle::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UISingle";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UISingle";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UISingle>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}

// LongUI页面容器: 调试信息
bool LongUI::UIPage::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIPage";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIPage";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIPage>()
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

// UI浮动布局: 调试信息
bool LongUI::UIFloatLayout::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIFloatLayout";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::LongUI::UIFloatLayout";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::LongUI::UIFloatLayout>()
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
    LongUI::SafeRelease(m_pBaseLayout);
    LongUI::SafeRelease(m_pRubyLayout);
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
InitStaticVar(LongUI::Dll::D2D1InvertMatrix);
InitStaticVar(LongUI::Dll::D2D1MakeSkewMatrix);
InitStaticVar(LongUI::Dll::D2D1IsMatrixInvertible);
InitStaticVar(LongUI::Dll::DWriteCreateFactory);
InitStaticVar(LongUI::Dll::CreateDXGIFactory1);


// longui namespace
namespace LongUI {
    // primes list
    const uint32_t EzContainer::PRIMES_LIST[14] = {
        19, 79, 149, 263, 457, 787, 1031, 2333,
        5167, 11369, 24989, 32491, 42257, 54941,
    };
    // Base64 DataChar: Map 0~63 to visible char
    const char Base64Chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // Base64 DataChar: Map visible char to 0~63
    const uint8_t Base64Datas[128] = {
        // [  0, 16)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 16, 32)
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0, 0, 0, 0, 0, 0,
        // [ 32, 48)                            43 44 45 46 47
        0, 0, 0, 0,   0, 0 ,0, 0,       0, 0, 0,62, 0, 0, 0,64,
        // [ 48, 64)
        52,53,54,55, 56,57,58,59,      60,61, 0, 0, 0, 0, 0, 0,
        // [ 64, 80)
        0, 0, 1, 2,   3, 4, 5, 6,       7, 8, 9,10,11,12,13,14,
        // [ 80, 96)
        15,16,17,18, 19,20,21,22,      23,24,25, 0, 0, 0, 0, 0,
        // [ 96,112)
        0,26,27,28,  29,30,31,32,      33,34,35,36,37,38,39,40,
        // [112,128)
        41,42,43,44, 45,46,47,48,      49,50,51, 0, 0, 0, 0, 0,
    };
    // IUIScript: {09B531BD-2E3B-4C98-985C-1FD6B406E53D}
    const GUID IID_IUIScript = { 
        0x9b531bd, 0x2e3b, 0x4c98, { 0x98, 0x5c, 0x1f, 0xd6, 0xb4, 0x6, 0xe5, 0x3d }
    };
    // IUIResourceLoader: {16222E4B-9AC8-4756-8CA9-75A72D2F4F60}
    const GUID IID_IUIResourceLoader = { 
        0x16222e4b, 0x9ac8, 0x4756,{ 0x8c, 0xa9, 0x75, 0xa7, 0x2d, 0x2f, 0x4f, 0x60 } 
    };
    // IMFMediaEngineClassFactor: uuid
    const GUID IID_IMFMediaEngineClassFactory = { 
        0x4D645ACE, 0x26AA, 0x4688,{ 0x9B, 0xE1, 0xDF, 0x35, 0x16, 0x99, 0x0B, 0x93 } 
    };
    // IMFMediaEngine: "98a1b0bb-03eb-4935-ae7c-93c1fa0e1c93"
    const GUID IID_IMFMediaEngine = {
        0x98A1B0BB, 0x03EB, 0x4935,{ 0xAE, 0x7C, 0x93, 0xC1, 0xFA, 0x0E, 0x1C, 0x93 } 
    };
    // IMFMediaEngineEx "83015ead-b1e6-40d0-a98a-37145ffe1ad1"
    const GUID IID_IMFMediaEngineEx = {
        0x83015EAD, 0xB1E6, 0x40D0,{ 0xA9, 0x8A, 0x37, 0x14, 0x5F, 0xFE, 0x1A, 0xD1 } 
    };
    // IMFMediaEngineNotify "fee7c112-e776-42b5-9bbf-0048524e2bd5"
    const GUID IID_IMFMediaEngineNotify = {
        0xfee7c112, 0xe776, 0x42b5,{ 0x9B, 0xBF, 0x00, 0x48, 0x52, 0x4E, 0x2B, 0xD5 } 
    };
    // IDCompositionDevice "C37EA93A-E7AA-450D-B16F-9746CB0407F3"
    const GUID IID_IDCompositionDevice = {
        0xC37EA93A, 0xE7AA, 0x450D,{ 0xB1, 0x6F, 0x97, 0x46, 0xCB, 0x04, 0x07, 0xF3 } 
    };
    // IDWriteTextRenderer
    const GUID IID_IDWriteTextRenderer = {
        0xef8a8135, 0x5cc6, 0x45fe,{ 0x88, 0x25, 0xc5, 0xa0, 0x72, 0x4e, 0xb8, 0x19 } 
    };
    // IID_IDWriteInlineObject 
    const GUID IID_IDWriteInlineObject = {
        0x8339FDE3, 0x106F, 0x47ab,{ 0x83, 0x73, 0x1C, 0x62, 0x95, 0xEB, 0x10, 0xB3 } 
    };
    // IDWriteFactory1 ("30572f99-dac6-41db-a16e-0486307e606a")
    const GUID IID_IDWriteFactory1 = {
        0x30572f99, 0xdac6, 0x41db,{ 0xa1, 0x6e, 0x04, 0x86, 0x30, 0x7e, 0x60, 0x6a } 
    };
    // IID_IDWriteFontCollection(a84cee02-3eea-4eee-a827-87c1a02a0fcc)
    const GUID IID_IDWriteFontCollection = {
        0xa84cee02, 0x3eea, 0x4eee,{ 0xa8, 0x27, 0x87, 0xc1, 0xa0, 0x2a, 0x0f, 0xcc }
    };
    // IDWriteFontFileEnumerator("72755049-5ff7-435d-8348-4be97cfa6c7c") 
    const GUID IID_IDWriteFontFileEnumerator = {
        0x72755049, 0x5ff7, 0x435d,{ 0x83, 0x48, 0x4b, 0xe9, 0x7c, 0xfa, 0x6c, 0x7c }
    };
    // IDWriteFontCollectionLoader("cca920e4-52f0-492b-bfa8-29c72ee0a468") 
    const GUID IID_IDWriteFontCollectionLoader = {
        0xcca920e4, 0x52f0, 0x492b,{ 0xbf, 0xa8, 0x29, 0xc7, 0x2e, 0xe0, 0xa4, 0x68 }
    };
    // ITextHost2 ("13E670F5-1A5A-11CF-ABEB-00AA00B65EA1")
    const GUID IID_ITextHost2 = {
        0x13E670F5, 0x1A5A, 0x11CF,{ 0xAB, 0xEB, 0x00, 0xAA, 0x00, 0xB6, 0x5E, 0xA1 }
    };
    // XUIBasicTextRenderer {EDAB1E53-C1CF-4F5A-9533-9946904AD63C}
    const GUID IID_CUIBasicTextRenderer = {
        0xedab1e53, 0xc1cf, 0x4f5a,{ 0x95, 0x33, 0x99, 0x46, 0x90, 0x4a, 0xd6, 0x3c }
    };
    // IID_IDXGISwapChain2 0xa8be2ac4, 0x199f, 0x4946, 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7
    const GUID IID_IDXGISwapChain2 = {
        0xa8be2ac4, 0x199f, 0x4946,{ 0xb3, 0x31, 0x79, 0x59, 0x9f, 0xb9, 0x8d, 0xe7 }
    };
}

// longui
namespace LongUI {
    // 安全释放
    auto SafeFreeLibrary(HMODULE& dll) noexcept { if (dll) ::FreeLibrary(dll);  dll = nullptr; }
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
        // ctor
        InitializeLibrary() noexcept {
            this->load_shcore();
            LongUI::LoadProc(LongUI::UIRichEdit::IID_ITextServices2, m_hDllMsftedit, "IID_ITextServices2");
            LongUI::LoadProc(LongUI::UIRichEdit::CreateTextServices, m_hDllMsftedit, "CreateTextServices");
            LongUI::LoadProc(LongUI::UIRichEdit::ShutdownTextServices, m_hDllMsftedit, "ShutdownTextServices");
            LongUI::LoadProc(LongUI::Dll::DCompositionCreateDevice, m_hDlldcomp, "DCompositionCreateDevice");
            LongUI::LoadProc(LongUI::Dll::D2D1CreateFactory, m_hDlld2d1, "D2D1CreateFactory");
            LongUI::LoadProc(LongUI::Dll::D2D1MakeSkewMatrix, m_hDlld2d1, "D2D1MakeSkewMatrix");
            LongUI::LoadProc(LongUI::Dll::D2D1InvertMatrix, m_hDlld2d1, "D2D1InvertMatrix");
            LongUI::LoadProc(LongUI::Dll::D2D1IsMatrixInvertible, m_hDlld2d1, "D2D1IsMatrixInvertible");
            LongUI::LoadProc(LongUI::Dll::D3D11CreateDevice, m_hDlld3d11, "D3D11CreateDevice");
            LongUI::LoadProc(LongUI::Dll::DWriteCreateFactory, m_hDlldwrite, "DWriteCreateFactory");
            LongUI::LoadProc(LongUI::Dll::CreateDXGIFactory1, m_hDlldxgi, "CreateDXGIFactory1");
        };
        //
        ~InitializeLibrary() noexcept {
            LongUI::SafeFreeLibrary(m_hDllMsftedit);
            LongUI::SafeFreeLibrary(m_hDlldcomp);
            LongUI::SafeFreeLibrary(m_hDlld2d1);
            LongUI::SafeFreeLibrary(m_hDlld3d11);
            LongUI::SafeFreeLibrary(m_hDlldwrite);
            LongUI::SafeFreeLibrary(m_hDlldxgi);
            LongUI::SafeFreeLibrary(m_hDllShcore);
        }
    private:
        // load for Shcore
        void load_shcore() noexcept {
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
        }
    private:
        // Msftedit
        HMODULE     m_hDllMsftedit  = ::LoadLibraryW(L"Msftedit.dll");
        // dcomp
        HMODULE     m_hDlldcomp     = ::LoadLibraryW(L"dcomp.dll");
        // d2d1
        HMODULE     m_hDlld2d1      = ::LoadLibraryW(L"d2d1.dll");
        // d3d11
        HMODULE     m_hDlld3d11     = ::LoadLibraryW(L"d3d11.dll");
        // dwrite
        HMODULE     m_hDlldwrite    = ::LoadLibraryW(L"dwrite.dll");
        // dxgi
        HMODULE     m_hDlldxgi      = ::LoadLibraryW(L"dxgi.dll");
        // Shcore
        HMODULE     m_hDllShcore    = nullptr;
    } instance;
}


// 初始化静态变量
LongUI::CUIManager          LongUI::CUIManager::s_instance;

// load libraries
#if defined(_MSC_VER)
#pragma comment(lib, "winmm")
#pragma comment(lib, "dxguid")
#endif

                   

// 主要景渲染
void LongUI::UICheckBox::render_chain_main() const noexcept {
    D2D1_RECT_F rect;
    rect.left = 0.f; rect.right = BOX_SIZE;
    rect.top = (this->view_size.height - BOX_SIZE) * 0.5f;
    rect.bottom = rect.top + BOX_SIZE;
    m_uiElement.Render(rect);
}

// 背景渲染
void LongUI::UICheckBox::render_chain_foreground() const noexcept {
    // 文本算前景
    m_text.Render(BOX_SIZE, 0.f);
    // 父类-父类
    UIControl::render_chain_foreground();
}

// Render 渲染 
void LongUI::UICheckBox::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UI检查框: 刷新
void LongUI::UICheckBox::Update() noexcept {
    m_uiElement.Update();
    return Super::Update();
}

// 设置状态
void LongUI::UICheckBox::SetCheckBoxState(CheckBoxState state) noexcept {
    // 修改状态
    if (state != this->GetCheckBoxState()) {
        m_pWindow->StartRender(m_uiElement.SetExtraState(state), this);
        bool rec = this->call_uievent(m_event, SubEvent::Event_ValueChanged);
        rec = false;
#ifdef _DEBUG
        const wchar_t* const list[] = {
            L"checked", L"indeterminate", L"unchecked"
        };
        UIManager << DL_Log << this
            << L"Checkbox change to ["
            << list[size_t(state)]
            << L']' 
            << LongUI::endl;
#endif
    }
}

// UICheckBox 初始化
void LongUI::UICheckBox::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    m_uiElement.Init(
        State_Normal,
        Helper::GetEnumFromXml(node, CheckBoxState::State_Unchecked),
        node
        );
    // 初始化
    Helper::SetBorderColor(node, m_aBorderColor);
    constexpr int azz = sizeof(m_uiElement);
}

// UICheckBox 析构函数
LongUI::UICheckBox::~UICheckBox() noexcept {

}


// UICheckBox::CreateControl 函数
auto LongUI::UICheckBox::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UICheckBox* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UICheckBox, pControl, type, node);
    }
    return pControl;
}



// do event 事件处理
bool LongUI::UICheckBox::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI消息
    /*if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_SetFocus:
            __fallthrough;
        case LongUI::Event::Event_KillFocus:
            return true;
        }
    }*/
    return Super::DoEvent(arg);
}


// do mouse event 鼠标事件处理
bool LongUI::UICheckBox::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    //UIManager << DL_Hint << this << this->GetEnabled() << endl;
    // 禁用状态禁用鼠标消息
    if (!this->GetEnabled()) return true;
    // 转换坐标
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // 鼠标 消息
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseEnter:
        // 鼠标移进: 设置UI元素状态
        this->SetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        m_pWindow->now_cursor = m_hCursorHand;
        return true;
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出: 设置UI元素状态
        this->SetControlState(LongUI::State_Normal);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Normal];
        m_pWindow->now_cursor = m_pWindow->default_cursor;
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        // 左键按下:
        m_pWindow->SetCapture(this);
        this->SetControlState(LongUI::State_Pushed);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Pushed];
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 左键弹起:
        this->SetControlState(LongUI::State_Hover);
        m_colorBorderNow = m_aBorderColor[LongUI::State_Hover];
        // 检查的是本控件
        if (m_pWindow->IsReleasedControl(this)) {
            // 检查flag
            auto target = CheckBoxState::State_Checked;
            if (this->GetCheckBoxState() == CheckBoxState::State_Checked) {
                target = CheckBoxState::State_Unchecked;
            }
            // 修改复选框状态
            this->SetCheckBoxState(target);
            // 释放
            m_pWindow->ReleaseCapture();
        }
        return true;
    }
    // 未处理的消息
    return false;
}


// 添加事件监听器
bool LongUI::UICheckBox::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    // 点击
    if (sb == SubEvent::Event_ValueChanged) {
        m_event += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

// recreate 重建
auto LongUI::UICheckBox::Recreate() noexcept ->HRESULT {
    // 有效
    m_uiElement.Recreate();
    // 父类处理
    return Super::Recreate();
}

// 关闭控件
void LongUI::UICheckBox::cleanup() noexcept {
    delete this;
}


                   
                    

// ----------------------------------------------------------------------------
// -------------------------------- UIList ------------------------------------
// ----------------------------------------------------------------------------
// UI列表控件: 初始化
void LongUI::UIList::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 初始
    m_vLines.reserve(100);
    m_vSelectedIndex.reserve(16);
    m_vLineTemplate.reserve(16);
    // OOM or BAD ACTION
    if(!m_vLines.isok() && !m_vLineTemplate.isok()) {
        UIManager << DL_Warning << "OOM for less 1KB memory" << endl;
    }
    // MAIN PROC
    auto listflag = this->list_flag | Flag_MultiSelect;
    if (node) {
        const char* str = nullptr;
        // 行高度
        if ((str = node.attribute("lineheight").value())) {
            m_fLineHeight = LongUI::AtoF(str);
        }
        // 双击时间
        if ((str = node.attribute("dbclicktime").value())) {
            m_hlpDbClick.time = uint32_t(LongUI::AtoI(str));
        }
        // 行模板
        if ((str = node.attribute("linetemplate").value())) {
            // 检查长度
            auto len = Helper::MakeCC(str);
            m_vLineTemplate.newsize(len);
            // 有效
            if (len && m_vLineTemplate.isok()) {
                Helper::MakeCC(str, m_vLineTemplate.data());
            }
            // 没有则给予警告
            else {
                UIManager << DL_Warning
                    << L"BAD TEMPLATE: {"
                    << str << L"} or OOM"
                    << endl;
            }
        }
        // 给予提示
        else {
            UIManager << DL_Hint
                << L"recommended to set 'linetemplate'. Now, set 'Text, 0' as template"
                << endl;
        }
        // 允许排序
        if (node.attribute("sort").as_bool(false)) {
            listflag |= this->Flag_SortableLineWithUserDataPtr;
        }
        // 普通背景颜色
        Helper::MakeColor(node.attribute("linebkcolor").value(), m_colorLineNormal1);
        // 普通背景颜色2 - step 1
        m_colorLineNormal2 = m_colorLineNormal1;
        // 普通背景颜色2 - step 2
        Helper::MakeColor(node.attribute("linebkcolor2").value(), m_colorLineNormal2);
        // 悬浮颜色
        Helper::MakeColor(node.attribute("linebkcolorhover").value(), m_colorLineHover);
        // 选中颜色
        Helper::MakeColor(node.attribute("linebkcolorselected").value(), m_colorLineSelected);
    }
    // 修改
    this->list_flag = listflag;

    // TEST: INIT COLOR DATA
    m_colorLineNormal1 = D2D1::ColorF(0xffffffui32, 0.5f);
    m_colorLineNormal2 = D2D1::ColorF(0xeeeeeeui32, 0.5f);
}

// 添加事件监听器(雾)
bool LongUI::UIList::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    switch (sb)
    {
    case LongUI::SubEvent::Event_ItemClicked:
        m_callLineClicked += std::move(call);
        return true;
    case LongUI::SubEvent::Event_ItemDbClicked:
        m_callLineDBClicked += std::move(call);
        return true;
    case LongUI::SubEvent::Event_ContextMenu:
        break;
    case LongUI::SubEvent::Event_EditReturned:
        break;
    case LongUI::SubEvent::Event_ValueChanged:
        break;
    case LongUI::SubEvent::Event_Custom:
        break;
    default:
        break;
    }
    return Super::uniface_addevent(sb, std::move(call));
}


// 获取参考控件
auto LongUI::UIList::get_referent_control() const noexcept -> UIListLine* {
    if (m_pHeader) {
        return m_pHeader;
    }
    else {
        if (m_vLines.empty()) return nullptr;
        return m_vLines.front();
    }
}

// 依靠鼠标位置获取列表行索引
auto LongUI::UIList::find_line_index(const D2D1_POINT_2F& pt) const noexcept ->uint32_t {
    uint32_t index = 0;
    // XXX: 利用list特性优化
    for (auto ctrl : m_vLines) {
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            break;
        }
        ++index;
    }
    return index;
}

// 依靠鼠标位置获取列表行
auto LongUI::UIList::find_line(const D2D1_POINT_2F& pt) const noexcept ->UIListLine* {
    // XXX: 利用list特性优化
    for (auto ctrl : m_vLines) {
        // 区域内判断
        if (IsPointInRect(ctrl->visible_rect, pt)) {
            return ctrl;
        }
    }
    return nullptr;
}


// UIList: 重建
auto LongUI::UIList::Recreate() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    // 重建子控件
    for (auto ctrl : m_vLines) {
        hr = ctrl->Recreate();
        assert(SUCCEEDED(hr));
    }
    return Super::Recreate();
}

// 查找子控件
auto LongUI::UIList::FindChild(const D2D1_POINT_2F& pt) noexcept -> UIControl* {
    auto ctrl = Super::FindChild(pt);
    if (ctrl) return ctrl;
    return this->find_line(pt);
}

// push!
void LongUI::UIList::PushBack(UIControl* child) noexcept {
    // 边界控件交给父类处理
    if (child && (child->flags & Flag_MarginalControl)) {
        Super::PushBack(child);
    }
    // 一般的就自己处理
    else {
        return this->Insert(m_cChildrenCount, longui_cast<UIListLine*>(child));
    }
}

// 插入
LongUINoinline void LongUI::UIList::Insert(uint32_t index, UIListLine* child) noexcept {
    assert(child && "bad argument");
    if (child) {
        // 对齐操作
        auto line = this->get_referent_control();
        if (line) {
            auto itr1 = child->begin();
            for (auto itr2 = line->begin(); itr2 != line->end(); ++itr1, ++itr2) {
                auto ctrl_new = *itr1, ctrl_ref = *itr2;
                force_cast(ctrl_new->flags) = ctrl_ref->flags;
                force_cast(ctrl_new->weight) = ctrl_ref->weight;
                ctrl_new->SetWidth(ctrl_ref->GetWidth());
            }
        }
        m_vLines.insert(index, child);
        this->after_insert(child);
        ++m_cChildrenCount;
        this->reset_select();
        assert(m_vLines.isok());
    }
}

// 排序
void LongUI::UIList::Sort(uint32_t index, UIControl* child) noexcept {
    // 修改
    m_pToBeSortedHeaderChild = child;
    // 有必要再说
    if ((this->list_flag & Flag_SortableLineWithUserDataPtr) 
        && m_vLines.size() > 1 
        && index < m_vLines.front()->GetCount()) {
        assert(child && "bad argument");
        assert(m_vLines.front()->flags & Flag_UIContainer);
        // 设置待排序控件
        for (auto ctrl : m_vLines) {
            ctrl->SetToBeSorted(index);
        }
        // 排序前
        m_callBeforSort(this);
        // 普通排序
        auto cmp_user_data = [](UIControl* a, UIControl* b) noexcept {
            assert(a && b && "bad arguments");
            auto ctrla = longui_cast<UIListLine*>(a)->GetToBeSorted();
            auto ctrlb = longui_cast<UIListLine*>(b)->GetToBeSorted();
            assert(ctrla && ctrlb && "bad action");
            return ctrla->user_data < ctrlb->user_data;
        };
        // 字符串排序
        auto cmp_user_ptr = [](UIControl* a, UIControl* b) noexcept {
            assert(a && b && "bad arguments");
            auto ctrla = longui_cast<UIListLine*>(a)->GetToBeSorted();
            auto ctrlb = longui_cast<UIListLine*>(b)->GetToBeSorted();
            assert(ctrla && ctrlb && "bad action");
            auto stra = static_cast<const wchar_t*>(ctrla->user_ptr);
            auto strb = static_cast<const wchar_t*>(ctrlb->user_ptr);
            assert(stra && strb && "bad action");
            return std::wcscmp(stra, strb) < 0;
        };
        // 普通排序
        bool(*cmp_alg)(UIControl*, UIControl*) = cmp_user_data;
        // 字符串排序?
        if (m_vLines.front()->GetToBeSorted()->user_ptr) {
            cmp_alg = cmp_user_ptr;
        }
        // 进行排序
        this->sort_line(cmp_alg);
        // 刷新
        m_pWindow->Invalidate(this);
    }
    m_pToBeSortedHeaderChild = nullptr;
}

// UI列表控件: 析构函数
LongUI::UIList::~UIList() noexcept {
    // 线性容器就是不用考虑next指针
    for (auto ctrl : m_vLines) {
        this->cleanup_child(ctrl);
    }
}

// [UNTESTED]移除
void LongUI::UIList::RemoveJust(UIControl* child) noexcept {
    auto itr = std::find(m_vLines.cbegin(), m_vLines.cend(), child);
    if (itr == m_vLines.cend()) {
        assert("control not found");
        return;
    }
    this->reset_select();
    m_vLines.erase(itr);
    --m_cChildrenCount;
    Super::RemoveJust(child);
}

// 对列表插入一个行模板至指定位置
auto LongUI::UIList::InsertLineTemplateToList(uint32_t index) noexcept ->UIListLine* {
    auto ctrl = static_cast<UIListLine*>(UIListLine::CreateControl(this->CET(), pugi::xml_node()));
    if (ctrl) {
        // 添加子控件
        for (const auto& data : m_vLineTemplate) {
            ctrl->Insert(ctrl->end(), UIManager.CreateControl(ctrl, data.id, data.func));
        }
        // 插入
        this->Insert(index, ctrl);
    }
    return ctrl;
}

// [UNTESTED] 利用索引移除行模板中一个元素
void LongUI::UIList::RemoveLineElementInEachLine(uint32_t index) noexcept {
    assert(index < m_vLineTemplate.size() && "out of range");
    if (index < m_vLineTemplate.size()) {
        // 刷新
        m_pWindow->Invalidate(this);
        // 交换列表
        for (auto line : m_vLines) {
            auto child = line->GetAt(index);
            line->RemoveClean(child);
        }
        // 模板
        m_vLineTemplate.erase(index);
    }
}

// [UNTESTED] 交换行模板中元素
void LongUI::UIList::SwapLineElementsInEachLine(uint32_t index1, uint32_t index2) noexcept {
    assert(index1 < m_vLineTemplate.size() && index2 < m_vLineTemplate.size() && "out of range");
    assert(index1 != index2 && "bad arguments");
    if (!(index1 < m_vLineTemplate.size() && index2 < m_vLineTemplate.size())) return;
    if (index1 == index2) return;
    // 刷新
    m_pWindow->Invalidate(this);
    // 交换列表
    for (auto line : m_vLines) {
        auto child1 = line->GetAt(index1);
        auto child2 = line->GetAt(index2);
        line->SwapChild(child1, child2);
    }
    // 交换模板
    std::swap(m_vLineTemplate[index1], m_vLineTemplate[index2]);
}

// [UNTESTED]插入一个新的行元素
void LongUI::UIList::InsertNewElementToEachLine(uint32_t index, CreateControlFunction func, size_t tid) noexcept {
    assert(index <= m_vLineTemplate.size() && "out of range");
    assert(func && "bad argument");
    // 有效
    if (index <= m_vLineTemplate.size() && func) {
        // 刷新
        m_pWindow->Invalidate(this);
        // 交换列表
        for (auto line : m_vLines) {
            auto ctrl = UIManager.CreateControl(line, tid, func);
            if (ctrl) {
                auto itr = MakeIteratorBI(line->GetAt(index));
                line->Insert(itr, ctrl);
            }
            else {
                UIManager << DL_Error
                    << "CreateControl failed. OOM or BAD ACTION"
                    << LongUI::endl;
            }
        }
        // 插入模板
        Helper::CC cc = { func, tid };
        m_vLineTemplate.insert(index, cc);
    }
}

// 设置
void LongUI::UIList::SetCCElementInLineTemplate(uint32_t index, CreateControlFunction func, size_t tid ) noexcept {
    assert(index < m_vLineTemplate.size() && "out of range");
    assert(func && "bad argument");
    if (index < m_vLineTemplate.size() && func) {
        m_vLineTemplate[index].func = func;
        m_vLineTemplate[index].id = tid;
    }
}


// 设置元素宽度
void LongUI::UIList::SetElementWidth(uint32_t index, float width) noexcept {
    // 循环
    for (auto ctrl : m_vLines) {
        assert(ctrl && "bad");
        assert(index < ctrl->GetCount() && "out of range");
        if (index < ctrl->GetCount()) {
            auto ele = ctrl->GetAt(index);
            ele->SetWidth(width);
            ctrl->SetControlLayoutChanged();
        }
    }
    this->SetControlLayoutChanged();
}

// UI列表: 事件处理
bool LongUI::UIList::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // LongUI 事件
    if (arg.sender) {
        switch (arg.event)
        {
        case LongUI::Event::Event_TreeBulidingFinished:
            // 由父类创建边缘控件
            Super::DoEvent(arg);
            this->init_layout();
            return true;
        default:
            break;
        }
    }
    return Super::DoEvent(arg);
}

// UI列表: 鼠标事件处理
bool LongUI::UIList::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // -------------------  L-Button Down  ---------------
    auto lbutton_down = [this, &arg]() noexcept {
        auto index = this->find_line_index(arg.pt);
        // SHIFT优先
        if (arg.sys.wParam & MK_SHIFT) {
            this->SelectTo(m_ixLastClickedLine, index);
            return;
        }
        // 修改
        m_ixLastClickedLine = index;
        // UNCTRLed
        bool unctrled = !(arg.sys.wParam & MK_CONTROL);
        // 双击?
        if (m_hlpDbClick.Click(arg.pt)) {
            UIManager << DL_Log << "DB Clicked" << endl;
            this->call_uievent(m_callLineDBClicked, SubEvent::Event_ItemDbClicked);
        }
        // 单击?
        else {
            this->SelectChild(m_ixLastClickedLine, unctrled);
            this->call_uievent(m_callLineClicked, SubEvent::Event_ItemClicked);
        }
    };
    // ---------------------------------------------------
    auto old_hover_line = m_pHoveredLine;
    // 分类
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_None:
        break;
    case LongUI::MouseEvent::Event_MouseWheelV:
        break;
    case LongUI::MouseEvent::Event_MouseWheelH:
        break;
    case LongUI::MouseEvent::Event_DragEnter:
        break;
    case LongUI::MouseEvent::Event_DragOver:
        break;
    case LongUI::MouseEvent::Event_DragLeave:
        break;
    case LongUI::MouseEvent::Event_Drop:
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        m_pHoveredLine = nullptr;
        break;
    case LongUI::MouseEvent::Event_MouseHover:
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        m_pHoveredLine = this->find_line(arg.pt);
        break;
    case LongUI::MouseEvent::Event_LButtonDown:
        lbutton_down();
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        break;
    case LongUI::MouseEvent::Event_RButtonDown:
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        break;
    case LongUI::MouseEvent::Event_MButtonDown:
        break;
    case LongUI::MouseEvent::Event_MButtonUp:
        break;
    default:
        break;
    }
    // 不同就渲染
    if (old_hover_line != m_pHoveredLine) {
#ifdef _DEBUG
        // 调试输出
        if (this->debug_this) {
            UIManager << DL_Hint
                << L"OLD: " << old_hover_line
                << L"NEW: " << m_pHoveredLine
                << endl;
        }
#endif
        m_pWindow->Invalidate(this);
    }
    return Super::DoMouseEvent(arg);
}

// 排序算法
void LongUI::UIList::sort_line(bool(*cmp)(UIControl* a, UIControl* b) ) noexcept {
    // 无需排列
    if (this->GetCount() <= 1) return;
#ifdef _DEBUG
    // cmp 会比较复杂, 模板带来的性能提升还不如用函数指针来节约代码大小
    auto timest = ::timeGetTime();
#endif
    const auto bn = &*m_vLines.begin();
    const auto ed = &*m_vLines.end();
    bool just_reverse = true;
    // 检查逆序状态
    for (auto itr = bn; itr < (ed -1); ++itr) {
        if (cmp(*(itr + 1), *itr)) {
            just_reverse = false;
            break;
        }
    }
    // 直接逆序
    if (just_reverse) {
        std::reverse(bn, ed);
    }
    // 排序
    else {
        // 快速排序
        if (this->GetCount() >= m_cFastSortThreshold) {
            std::sort(bn, ed, cmp);
        }
        // 冒泡排序
        else {
            LongUI::BubbleSort(bn, ed, cmp);
        }
    }
#ifdef _DEBUG
    timest = ::timeGetTime() - timest;
    if (timest) {
        int bk = 9;
        UIManager << DL_Hint
            << "sort take time: "
            << long(timest)
            << " ms"
            << LongUI::endl;
        bk = 0;
    }
    if (this->debug_this) {
        UIManager << DL_Log
            << "sort take time: "
            << long(timest)
            << " ms"
            << LongUI::endl;
    }
#endif
    // 修改了
    this->reset_select();
    this->SetControlLayoutChanged();
}


// 选择子控件(对外)
void LongUI::UIList::SelectChild(uint32_t index, bool new_select) noexcept {
    if (index < m_cChildrenCount) {
        this->select_child(index, new_select);
        m_pWindow->Invalidate(this);
    }
}

// 选择子控件到(对外)
void LongUI::UIList::SelectTo(uint32_t index1, uint32_t index2) noexcept {
    // 交换
    if (index1 > index2) std::swap(index1, index2);
    // 限制
    index2 = std::min(index2, m_cChildrenCount - 1);
    // 有效
    if (index1 < index2) {
        this->select_to(index1, index2);
        m_pWindow->Invalidate(this);
    }
}

// 选择子控件
LongUINoinline void LongUI::UIList::select_child(uint32_t index, bool new_select) noexcept {
    assert(index < m_cChildrenCount && "out of range for selection");
    // 检查是否多选
    if (!new_select && !(this->list_flag & this->Flag_MultiSelect)) {
        UIManager << DL_Hint
            << "cannot do multi-selection"
            << LongUI::endl;
        new_select = true;
    }
    // 新的重置
    if (new_select) {
        this->reset_select();
    }
    // 选择
    auto line = m_vLines[index];
    if (line->IsSelected()) {
        line->SetSelected(false);
        // 移除
        auto itr = std::find(m_vSelectedIndex.cbegin(), m_vSelectedIndex.cend(), index);
        if (itr == m_vSelectedIndex.cend()) {
            assert(!"NOT FOUND");
        }
        else {
            m_vSelectedIndex.erase(itr);
        }
    }
    else {
        line->SetSelected(true);
        m_vSelectedIndex.push_back(index);
    }
}

// 选择子控件到
LongUINoinline void LongUI::UIList::select_to(uint32_t index1, uint32_t index2) noexcept {
    assert(index1 < m_cChildrenCount && index2 < m_cChildrenCount && "out of range for selection");
    // 检查是否多选
    if (!(this->list_flag & this->Flag_MultiSelect)) {
        UIManager << DL_Hint
            << "cannot do multi-selection"
            << LongUI::endl;
        index1 = index2;
    }
    // 交换
    if (index1 > index2) std::swap(index1, index2);
    // 选择
    auto itr_1st = m_vLines.data() + index1;
    auto itr_lst = m_vLines.data() + index2;
    auto i = index1;
    for (auto itr = itr_1st; itr <= itr_lst; ++itr) {
        auto line = *itr;
        line->SetSelected(true);
        m_vSelectedIndex.push_back(i);
        ++i;
    }
}

// UIList: 重置选择
void LongUI::UIList::reset_select() noexcept {
    //m_pHoveredLine = nullptr;
    //m_ixLastClickedLine = uint32_t(-1);
    for (auto i : m_vSelectedIndex) {
        m_vLines[i]->SetSelected(false);
    }
    m_vSelectedIndex.clear();
}

// UIList: 初始化布局
void LongUI::UIList::init_layout() noexcept {
    uint32_t element_count_init = 1;
    auto rctrl = this->get_referent_control();
    if (rctrl) {
        // 检查不和谐的地方
#ifdef _DEBUG
        if (rctrl->GetCount() != m_vLineTemplate.size()) {
            if (m_vLineTemplate.size()) {
                UIManager << DL_Warning
                    << L"inconsistent line-element count: SET "
                    << long(m_vLineTemplate.size())
                    << L", BUT "
                    << long(rctrl->GetCount())
                    << LongUI::endl;
            }
        }
#endif
        element_count_init = rctrl->GetCount();
    }
    // 没有就给予警告
    else {
        UIManager << DL_Warning
            << L"NO CHILD FOUND. line-element set to 1"
            << LongUI::endl;
    }
    this->set_element_count(element_count_init);
}

// 设置元素数量
void LongUI::UIList::set_element_count(uint32_t length) noexcept {
    auto old = m_vLineTemplate.size();
    m_vLineTemplate.newsize(length);
    // 变长了
    if (old < m_vLineTemplate.size()) {
        for (auto i = old; i < m_vLineTemplate.size(); ++i) {
            m_vLineTemplate[i].id = 0;
            m_vLineTemplate[i].func = UIText::CreateControl;
        }
    }
}

// UIList: 前景渲染
void LongUI::UIList::render_chain_background() const noexcept {
    // 独立背景- - 可视优化
    if (this->GetCount()) {
        // 保留转变
        D2D1_MATRIX_3X2_F matrix;
        UIManager_RenderTarget->GetTransform(&matrix);
        UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
        // 第一个可视列表行 = (-Y偏移) / 行高
        int first_visible = static_cast<int>((-m_2fOffset.y) / m_fLineHeight);
        first_visible = std::max(first_visible, int(0));
        // 最后一个可视列表行 = 第一个可视列表行 + 1 + 可视区域高度 / 行高
        int last_visible = static_cast<int>(this->view_size.height / m_fLineHeight);
        last_visible = last_visible + first_visible + 1;
        last_visible = std::min(last_visible, int(this->GetCount()));
        // 背景索引
        int bkindex1 = !(first_visible & 1);
        // 循环
        const auto first_itr = m_vLines.data() + first_visible;
        const auto last_itr = m_vLines.data() + last_visible;
        for (auto itr = first_itr; itr < last_itr; ++itr) {
            auto line = *itr;
            // REMOVE THIS LINE?
            const D2D1_COLOR_F* color;
            // 选择色优先
            if (line->IsSelected()) {
                color = &m_colorLineSelected;
            }
            // 悬浮色其次
            else if (line == m_pHoveredLine) {
                color = &m_colorLineHover;
            }
            // 背景色最后
            else {
                color = &m_colorLineNormal1 + bkindex1;
            }
            // 设置
            if (color->a > 0.f) {
                m_pBrush_SetBeforeUse->SetColor(color);
                UIManager_RenderTarget->FillRectangle(
                    &line->visible_rect, m_pBrush_SetBeforeUse
                    );
            }
            bkindex1 = !bkindex1;
        }
        // 还原
        UIManager_RenderTarget->SetTransform(&matrix);
    }
    // 父类主景
    Super::render_chain_background();
}

// UIList: 主景渲染
void LongUI::UIList::render_chain_main() const noexcept {
    // 渲染帮助器
    Super::RenderHelper(this->begin(), this->end());
    // 父类主景
    Super::render_chain_main();
}

// UIList: 渲染函数
void LongUI::UIList::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}

// UIList: 刷新
void LongUI::UIList::Update() noexcept {
    // 帮助器
    Super::UpdateHelper<Super>(m_vLines.begin(), m_vLines.end());
#ifdef _DEBUG
    // 必须一致
    if (this->IsNeedRefreshWorld() && m_pHeader && m_vLines.size() && m_vLines.front()) {
        assert(m_pHeader->GetCount() == m_vLines.front()->GetCount() && "out of sync for child number");
    }
#endif
    //this->world;
    //this->RefreshWorld();
}


// 更新子控件布局
void LongUI::UIList::RefreshLayout() noexcept {
    if (m_vLines.empty()) return;
    // 第二次
    float index = 0.f;
    float widthtt = m_vLines.front()->GetContentWidthZoomed();
    if (widthtt == 0.f) widthtt = this->GetViewWidthZoomed();
    for (auto ctrl : m_vLines) {
        // 设置控件高度
        ctrl->SetWidth(widthtt);
        ctrl->SetHeight(m_fLineHeight);
        // 不管如何, 修改!
        ctrl->SetControlLayoutChanged();
        ctrl->SetLeft(0.f);
        ctrl->SetTop(m_fLineHeight * index);
        ctrl->visible_rect;
        ctrl->world;
        ++index;
    }
    // 设置
    m_2fContentSize.width = widthtt;
    m_2fContentSize.height = m_fLineHeight * this->GetCount();
}

// 清理UI列表控件
void LongUI::UIList::cleanup() noexcept {
    delete this;
}

// UI列表控件: 创建控件
auto LongUI::UIList::CreateControl(CreateEventType type, pugi::xml_node node)
noexcept -> UIControl* {
    UIList* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIList, pControl, type, node);
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// ---------------------------- UIListLine! --------------------------------
// ----------------------------------------------------------------------------

// UI列表元素控件: 初始化
void LongUI::UIListLine::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // listline 特性: 宽度必须固定
    //auto flag = this->flags | Flag_WidthFixed;
    if (node) {

    }
    //force_cast(this->flags) = flag;
}

// 刷新UI列表元素控件
void LongUI::UIListLine::Update() noexcept {
    // 检查宽度
    if (m_bFirstUpdate) {
        m_bFirstUpdate = false;
        // 取消属性
        for (auto ctrl : (*this)) {
            if (ctrl->view_size.width <= 0.f) {
                force_cast(ctrl->flags) &= (~Flag_WidthFixed);
            }
        }
        Super::Update();
        // 添加属性
        for (auto ctrl : (*this)) {
            force_cast(ctrl->flags) |= Flag_WidthFixed;
        }
        return;
    }
    return Super::Update();
}

// 清理UI列表元素控件
void LongUI::UIListLine::cleanup() noexcept {
    delete this;
}

// UI列表元素控件: 创建控件
auto LongUI::UIListLine::CreateControl(CreateEventType type, pugi::xml_node node)
noexcept -> UIControl* {
    UIListLine* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIListLine, pControl, type, node);
    }
    return pControl;
}


// ----------------------------------------------------------------------------
// ----------------------------- UIListHeader ---------------------------------
// ----------------------------------------------------------------------------

// UI列表头控件: 构造函数
void LongUI::UIListHeader::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 本类必须为边界控件
    assert((this->flags & Flag_MarginalControl) && "'UIListHeader' must be marginal-control");
    // 设置表头
    longui_cast<UIList*>(this->parent)->SetHeader(this);
    // 支持模板子结点
    //auto flag = this->flags;
    if (node) {
        const char* str = nullptr;
        // 行高度
        if ((str = node.attribute("lineheight").value())) {
            m_fLineHeight = LongUI::AtoF(str);
        }
        // 分隔符宽度
        if ((str = node.attribute("sepwidth").value())) {
            m_fSepwidth = LongUI::AtoF(str);
        }
    }
    //force_cast(this->flags) = flag;
}

// UI列表头: 事件处理
void LongUI::UIListHeader::Update() noexcept {
    // 与父对象保持一样的X偏移量
    this->SetOffsetX(this->parent->GetOffsetX());
    // 父类刷新
    return Super::Update();
}

// UI列表头: 鼠标事件处理
bool LongUI::UIListHeader::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // -------------------------- hover it?
    auto get_sep_hovered_control = [this, &arg]() noexcept {
        auto realsep = m_fSepwidth * this->world._11 * this->GetZoomX();
        // 区间修正
        float data[2] = { arg.pt.x, arg.pt.x };
        data[realsep < 0.f] -= realsep;
        // 循环查找
        auto index = 0ui32;
        for (auto ctrl : (*this)) {
            if (ctrl->GetVisible() && ctrl->visible_rect.right > ctrl->visible_rect.left
                && ctrl->visible_rect.right >= data[0]
                && ctrl->visible_rect.right < data[1]) {
                m_indexSepHovered = index;
                return ctrl;
            }
            ++index;
        }
        return static_cast<UIControl*>(nullptr);
    };
    // -------------------------- set sort data
    auto set_sort_data = [this, &arg]() noexcept {
        // 遍历子控件
        for (auto ctrl : (*this)) {
            // 悬浮在鼠标处
            if (IsPointInRect(ctrl->visible_rect, arg.pt)) {
                // 没有设置
                if (!ctrl->TestParentState()) {
                    // 设置点击事件
                    ctrl->AddEventCall([this](UIControl* child) noexcept {
                        longui_cast<UIList*>(this->parent)->Sort(this->GetIndexOf(child), child);
                        return true;
                    }, SubEvent::Event_ItemClicked);
                    // 设置了
                    ctrl->SetParentState(true);
                }
                break;
            }
        }
    };
    // -------------------------- on mouse move
    auto on_mouse_move = [this, &arg](UIControl* hovered) noexcept {
        if (hovered) {
            m_pWindow->now_cursor = m_hCursor;
            // 拖拽刷新
            if (m_pSepHovered && (arg.sys.wParam & MK_LBUTTON)) {
                auto distance = arg.pt.x - m_fLastMousePosX;
                distance *= m_pSepHovered->world._11;
                auto tarwidth = m_pSepHovered->GetWidth() + distance;
                // 有效
                if (tarwidth > m_fLineHeight) {
                    m_fLastMousePosX = arg.pt.x;
                    m_pSepHovered->SetWidth(m_pSepHovered->GetWidth() + distance);
                    longui_cast<LongUI::UIList*>(this->parent)->SetElementWidth(m_indexSepHovered, tarwidth);
                    m_pWindow->Invalidate(this->parent);
                    this->SetControlLayoutChanged();
                }
            }
        }
        else {
            m_pWindow->ResetCursor();
        }
    };
    // -------------------------- real method
    // 有效
    if (m_fSepwidth != 0.f) {
        // 查找控件
        auto hover_sep = m_pSepHovered;
        if (!hover_sep) hover_sep = get_sep_hovered_control();
        // XXX: 逻辑
        bool handled = !!hover_sep;
        // 分类处理
        switch (arg.event)
        {
        case LongUI::MouseEvent::Event_MouseLeave:
            m_pWindow->ResetCursor();
            handled = false;
            m_indexSepHovered = 0;
            break;
        case LongUI::MouseEvent::Event_MouseMove:
            on_mouse_move(hover_sep);
            if (hover_sep) {
            }
            break;
        case LongUI::MouseEvent::Event_LButtonDown:
            // 边界拖拽
            if (hover_sep) {
                m_pSepHovered = hover_sep;
                m_pWindow->SetCapture(this);
                m_fLastMousePosX = arg.pt.x;
            }
            else {
                // 设置排序
                if ((static_cast<UIList*>(this->parent)->list_flag
                    & UIList::Flag_SortableLineWithUserDataPtr)) {
                    set_sort_data();
                }
            }
            break;
        case LongUI::MouseEvent::Event_LButtonUp:
            // 边界拖拽
            if (m_pSepHovered) {
                m_pWindow->ReleaseCapture();
                m_pSepHovered = nullptr;
                m_indexSepHovered = 0;
            }
            break;
        case LongUI::MouseEvent::Event_RButtonUp:
            break;
        }
        // 处理了
        if (handled) return true;
    }
    return Super::DoMouseEvent(arg);
}

// 清理UI列表头控件
void LongUI::UIListHeader::cleanup() noexcept {
    delete this;
}


// 刷新UI列表头控件边界宽度
void LongUI::UIListHeader::UpdateMarginalWidth() noexcept {
    this->marginal_width = m_fLineHeight + this->margin_rect.top + this->margin_rect.bottom;
}

// 创建UI列表头
auto LongUI::UIListHeader::CreateControl(CreateEventType type, pugi::xml_node node) noexcept -> UIControl* {
    UIListHeader* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIListHeader, pControl, type, node);
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
    // 创建结点
    return this->Create(document.first_child());
}

// 使用XML结点创建菜单
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

                   

#define LONGUI_D3D_DEBUG
//#define LONGUI_RENDER_IN_STD_THREAD

// CUIManager 初始化
auto LongUI::CUIManager::Initialize(IUIConfigure* config) noexcept ->HRESULT {
    // 检查GUID
#if defined(_DEBUG) && defined(_MSC_VER)
#define CHECK_GUID(x)  assert(LongUI::IID_##x == __uuidof(x) && "bad guid")
    CHECK_GUID(IDWriteTextRenderer);
    CHECK_GUID(IDWriteInlineObject);
    CHECK_GUID(IDWriteFactory1);
    CHECK_GUID(IDWriteFontCollection);
    CHECK_GUID(IDWriteFontFileEnumerator);
    CHECK_GUID(IDWriteFontCollectionLoader);
    CHECK_GUID(IDXGISwapChain2);
#undef CHECK_GUID
#endif
    m_szLocaleName[0] = L'\0';
    m_vDelayCleanup.reserve(100);
    std::memset(m_apWindows, 0, sizeof(m_apWindows));
    // 开始计时
    m_uiTimer.Start();
    this->RefreshDisplayFrequency();
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
    std::memset(m_apTextRenderer, 0, sizeof(m_apTextRenderer));
    std::memset(m_apSystemBrushes, 0, sizeof(m_apSystemBrushes));
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
        wcex.lpszClassName = LongUI::WindowClassNameA;
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
        longui_debug_hr(hr, L"load_control_template_string faild");
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
            std::memset(m_pResourceBuffer, 0, get_buffer_length());
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
        longui_debug_hr(hr, L"set_control_template_string faild");
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
        longui_debug_hr(hr, L"D2D1CreateFactory faild");
    }
    // 创建TSF线程管理器
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_TF_ThreadMgr,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pTsfThreadManager)
            );
        longui_debug_hr(hr, L"CoCreateInstance CLSID_TF_ThreadMgr faild");
    }
    // 激活客户ID
    if (SUCCEEDED(hr)) {
        hr = m_pTsfThreadManager->Activate(&m_idTsfClient);
        longui_debug_hr(hr, L"m_pTsfThreadManager->Activate faild");
    }
    // 创建 DirectWrite 工厂.
    if (SUCCEEDED(hr)) {
        hr = LongUI::Dll::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_ISOLATED,
            LongUI_IID_PV_ARGS_Ex(m_pDWriteFactory)
            );
        longui_debug_hr(hr, L"DWriteCreateFactory faild");
    }
    // 创建帮助器
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_DragDropHelper,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pDropTargetHelper)
            );
        longui_debug_hr(hr, L"CoCreateInstance CLSID_DragDropHelper faild");
    }
    // 创建字体集
    if (SUCCEEDED(hr)) {
        // 获取脚本
        config->CreateInterface(LongUI_IID_PV_ARGS(m_pFontCollection));
        // 失败获取系统字体集
        if (!m_pFontCollection) {
            hr = m_pDWriteFactory->GetSystemFontCollection(&m_pFontCollection);
            longui_debug_hr(hr, L"m_pDWriteFactory->GetSystemFontCollection faild");
        }
    }
#ifdef _DEBUG
    // 枚举字体
    if (SUCCEEDED(hr) && (this->flag & IUIConfigure::Flag_DbgOutputFontFamily)) {
        auto count = m_pFontCollection->GetFontFamilyCount();
        UIManager << DL_Log << "Font found: " << long(count) << L"\r\n";
        // 遍历所有字体
        for (auto i = 0u; i < count; ++i) {
            IDWriteFontFamily* family = nullptr;
            // 获取字体信息
            if (SUCCEEDED(m_pFontCollection->GetFontFamily(i, &family))) {
                IDWriteLocalizedStrings* string = nullptr;
                // 获取字体名称
                if (SUCCEEDED(family->GetFamilyNames(&string))) {
                    wchar_t buffer[LongUIStringBufferLength];
                    auto tc = string->GetCount();
                    UIManager << DLevel_Log << Formated(L"%4d[%d]: ", int(i), int(tc));
                    // 遍历所有字体名称
#if 0
                    for (auto j = 0u; j < 1u; j++) {
                        string->GetLocaleName(j, buffer, LongUIStringBufferLength);
                        UIManager << DLevel_Log << buffer << " => ";
                        // 有些语言在我的机器上显示不了(比如韩语), 会出现bug略过不少东西, 就显示第一个了
                        string->GetString(j, buffer, LongUIStringBufferLength);
                        UIManager << DLevel_Log << buffer << "; ";
                    }
#else
                    // 显示第一个
                    string->GetLocaleName(0, buffer, LongUIStringBufferLength);
                    UIManager << DLevel_Log << buffer << " => ";
                    string->GetString(0, buffer, LongUIStringBufferLength);
                    UIManager << DLevel_Log << buffer << ";\r\n";
#endif
                }
                LongUI::SafeRelease(string);
            }
            LongUI::SafeRelease(family);
        }
        // 刷新
        UIManager << DL_Log << LongUI::endl;
    }
#endif
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
        this->RegisterControlClass(CreateNullControl, "Null");
        this->RegisterControlClass(UIText::CreateControl, "Text");
        this->RegisterControlClass(UIList::CreateControl, "List");
        this->RegisterControlClass(UIPage::CreateControl, "Page");
        this->RegisterControlClass(UISlider::CreateControl, "Slider");
        this->RegisterControlClass(UIButton::CreateControl, "Button");
        this->RegisterControlClass(UISingle::CreateControl, "Single");
        this->RegisterControlClass(UIListLine::CreateControl, "ListLine");
        this->RegisterControlClass(UICheckBox::CreateControl, "CheckBox");
        this->RegisterControlClass(UIRichEdit::CreateControl, "RichEdit");
        this->RegisterControlClass(UIEditBasic::CreateControl, "Edit");
        this->RegisterControlClass(UIListHeader::CreateControl, "ListHeader");
        this->RegisterControlClass(UIScrollBarA::CreateControl, "ScrollBarA");
        this->RegisterControlClass(UIScrollBarB::CreateControl, "ScrollBarB");
        this->RegisterControlClass(UIFloatLayout::CreateControl, "FloatLayout");
        this->RegisterControlClass(UIVerticalLayout::CreateControl, "VerticalLayout");
        this->RegisterControlClass(UIHorizontalLayout::CreateControl, "HorizontalLayout");
        // 添加自定义控件
        config->RegisterSome();
    }
    // 创建资源
    if (SUCCEEDED(hr)) {
        hr = this->RecreateResources();
        longui_debug_hr(hr, L"RecreateResources faild");
    }
    // 初始化事件
    if (SUCCEEDED(hr)) {
        this->do_creating_event(LongUI::CreateEventType::Type_Initialize);
        longui_debug_hr(hr, L"do_creating_event(init) faild");
    }
    // 检查错误
    else {
        this->ShowError(hr);
    }
    // 检查当前路径
#ifdef _DEBUG
    wchar_t buffer[MAX_PATH * 4]; buffer[0] = 0;
    ::GetCurrentDirectoryW(lengthof<uint32_t>(buffer), buffer);
    UIManager << DL_Log << L" Current Directory: " << buffer << LongUI::endl;
#endif
    return hr;
}


// CUIManager  反初始化
void LongUI::CUIManager::Uninitialize() noexcept {
    this->do_creating_event(LongUI::CreateEventType::Type_Uninitialize);
    // 释放文本渲染器
    for (auto& renderer : m_apTextRenderer) {
        LongUI::SafeRelease(renderer);
    }
    // 释放公共设备无关资源
    {
        // 释放文本格式
        for (auto itr = m_ppTextFormats; itr != m_ppTextFormats + m_cCountTf; ++itr) {
            LongUI::SafeRelease(*itr);
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
    LongUI::SafeRelease(m_pFontCollection);
    LongUI::SafeRelease(m_pDWriteFactory);
    LongUI::SafeRelease(m_pDropTargetHelper);
    LongUI::SafeRelease(m_pd2dFactory);
    LongUI::SafeRelease(m_pTsfThreadManager);
    // 释放脚本
    LongUI::SafeRelease(force_cast(script));
    // 释放读取器
    LongUI::SafeRelease(m_pResourceLoader);
    // 释放配置
    LongUI::SafeRelease(force_cast(this->configure));
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
    // 清理
    m_hashStr2CreateFunc.Clear();
}

// 创建事件
void LongUI::CUIManager::do_creating_event(CreateEventType type) noexcept {
    assert(type < LongUI::TypeGreater_CreateControl_ReinterpretParentPointer &&
        type > Type_CreateControl_NullParentPointer);
    m_hashStr2CreateFunc.ForEach([type](StringTable::Unit* unit) noexcept {
        assert(unit);
        auto func = reinterpret_cast<CreateControlFunction>(unit->value);
        func(type, LongUINullXMLNode);
    });
}

// CUIManager 创建控件树
void LongUI::CUIManager::make_control_tree(LongUI::UIWindow* window, pugi::xml_node node) noexcept {
    // 断言
    assert(window && node && "bad argument");
    // 添加窗口
    //add_control(window, node);
    // 队列 -- 顺序遍历树
    LongUI::EzContainer::FixedCirQueue<pugi::xml_node, LongUIMaxControlInited> xml_queue;
    LongUI::EzContainer::FixedCirQueue<UIContainer*, LongUIMaxControlInited> parents_queue;
    UIControl* now_control = nullptr;
    UIContainer* parent_node = window;
    // 遍历算法: 1.压入所有子结点 2.依次弹出 3.重复1
    while (true) {
        // 压入/入队 所有子结点
        node = node.first_child();
        while (node) {
            xml_queue.Push(node);
            parents_queue.Push(parent_node);
            node = node.next_sibling();
        }
        // 为空则退出
        if (xml_queue.IsEmpty()) break;
        // 弹出/出队 第一个结点
        node = xml_queue.Front();  xml_queue.Pop();
        parent_node = parents_queue.Front(); parents_queue.Pop();
        assert(node && "bad xml node");
        // 根据名称创建控件
        if (!(now_control = this->CreateControl(parent_node, node, nullptr))) {
            // 错误
            parent_node = nullptr;
            UIManager << DL_Error
                << L" control class not found: "
                << node.name()
                << L".or OOM"
                << LongUI::endl;
            continue;
        }
        // 添加子结点
        parent_node->PushBack(now_control);
        // 设置结点为下次父结点
        parent_node = static_cast<decltype(parent_node)>(now_control);
    }
}

// 获取创建控件函数指针
auto LongUI::CUIManager::GetCreateFunc(const char* clname) noexcept -> CreateControlFunction {
    // 检查
    assert(clname && clname[0] && "bad argment");
    // 查找
    auto result = m_hashStr2CreateFunc.Find(clname);
    // 检查
    assert(result && "404 not found");
    // 返回
    return reinterpret_cast<CreateControlFunction>(*result);
}

// 创建文本格式
auto LongUI::CUIManager::CreateTextFormat(
    WCHAR const * fontFamilyName, 
    DWRITE_FONT_WEIGHT fontWeight, 
    DWRITE_FONT_STYLE fontStyle,
    DWRITE_FONT_STRETCH fontStretch, 
    FLOAT fontSize, 
    IDWriteTextFormat ** textFormat) noexcept -> HRESULT {
    auto hr = S_OK;
#ifdef _DEBUG
    // 检查字体名称
    UINT32 index = 0; BOOL exist = FALSE;
    hr = m_pFontCollection->FindFamilyName(fontFamilyName, &index, &exist);
    if (SUCCEEDED(hr)) {
        // 字体不存在, 则给予警告
        if (!exist) {
            UIManager << DL_Hint
                << Formated(L"font family(%ls) not found", fontFamilyName)
                << LongUI::endl;
            int bk; bk = 9;
        }
    }
    longui_debug_hr(hr, L"m_pFontCollection->FindFamilyName failed");
#endif
    // 创建文本格式
    hr = m_pDWriteFactory->CreateTextFormat(
        fontFamilyName,
        m_pFontCollection,
        fontWeight,
        fontStyle,
        fontStretch,
        fontSize,
        m_szLocaleName,
        textFormat
        );
    // 检查错误
    longui_debug_hr(hr, Formated(L"CreateTextFormat(%ls) faild", fontFamilyName));
    return hr;
}


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
#ifdef _DEBUG
            ++UIManager.frame_id;
#endif
            // 延迟清理
            UIManager.cleanup_delay_cleanup_chain();
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
    // 再次清理
    this->cleanup_delay_cleanup_chain();
    // 尝试强行关闭
    if (m_cCountWindow) {
        UIWindow* windows[LongUIMaxWindow];
        std::memcpy(windows, m_apWindows, sizeof(m_apWindows));
        auto count = m_cCountWindow;
        // 清理窗口
        for (auto i = 0u; i < count; ++i) {
            windows[count - i - 1]->cleanup();
        }
    }
    assert(!m_cCountWindow && "bad");
    m_cCountWindow = 0;
}

// 等待垂直同步
void LongUI::CUIManager::WaitVS(HANDLE events[], uint32_t length) noexcept {
    // 一直刷新
    if (this->flag & IUIConfigure::Flag_RenderInAnytime) 
        return static_cast<void>(::WaitForMultipleObjects(length, events, TRUE, INFINITE));
    // 保留刷新时间点
    auto end_time_of_sleep = m_dwWaitVSStartTime + 
        ((++m_dwWaitVSCount) * 1000ui32) / static_cast<uint16_t>(m_dDisplayFrequency);
    // 等待事件
    if (length) ::WaitForMultipleObjects(length, events, TRUE, INFINITE);
    // 保证等待
    while (::timeGetTime() < end_time_of_sleep) ::Sleep(1);
}

// 利用现有资源创建控件
auto LongUI::CUIManager::create_control(UIContainer* cp, CreateControlFunction function, pugi::xml_node node, size_t tid) noexcept -> UIControl * {
    // 检查参数 function
    if (!function) {
        assert(node && "bad argument");
        if (node) {
            function = this->GetCreateFunc(node.name());
        }
    }
    // 结点有效并且没有指定模板ID则尝试获取
    if (node && !tid) {
        tid = static_cast<decltype(tid)>(LongUI::AtoI(
            node.attribute(LongUI::XMLAttribute::TemplateID).value())
            );
    }
    // 利用id查找模板控件
    if (tid) {
        assert(tid < m_cCountCtrlTemplate && "out of range");
        if (tid >= m_cCountCtrlTemplate) tid = 0;
        // 结点有效则添加属性
        if (node) {
            auto attribute = m_pTemplateNodes[tid].first_attribute();
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
            node = m_pTemplateNodes[tid];
        }
    }
    // 检查
    assert(function && "bad idea");
    if (!function) return nullptr;
    auto ctrl = function(cp->CET(), node);
    return ctrl;
}

// 创建UI窗口
auto LongUI::CUIManager::create_ui_window(pugi::xml_node node, 
    UIWindow* pat, callback_for_creating_window func, void* buf) noexcept -> UIWindow* {
    assert(node && "bad argument");
    // 有效情况
    if (func && node) {
        // 创建窗口
        auto window = func(node, pat, buf);
        // 查错
        assert(window); if (!window) return nullptr;
        // 重建资源
        auto hr = window->Recreate();
        ShowHR(hr);
#ifdef _DEBUG
        //::Sleep(5000);
        CUITimerH dbg_timer; dbg_timer.Start();
#endif
        // 创建控件树
        this->make_control_tree(window, node);
        // 完成创建
        window->DoLongUIEvent(Event::Event_TreeBulidingFinished);
#ifdef _DEBUG
        auto time = dbg_timer.Delta_ms<double>();
        UIManager << DL_Log
            << Formated(L" took time %.3lf ms for making tree ", time)
            << window
            << LongUI::endl;
#endif
        //::Sleep(5000);
        // 返回
        return window;
    }
    return nullptr;
}

// 消息转换
void LongUI::CUIManager::WindowsMsgToMouseEvent(MouseEventArgument& arg, 
    UINT message, WPARAM wParam, LPARAM lParam) noexcept {
    // 范围检查
    assert((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) || 
        message == WM_MOUSELEAVE || message == WM_MOUSEHOVER);
    // 获取X
    auto get_x = [lParam]() { return float(int16_t(LOWORD(lParam))); };
    // 获取Y
    auto get_y = [lParam]() { return float(int16_t(HIWORD(lParam))); };
    // 设置
    arg.sys.wParam = wParam;
    arg.sys.lParam = lParam;
    arg.pt.x = get_x();
    arg.pt.y = get_y();
    arg.last = nullptr;
    MouseEvent me;
    // 检查信息
    switch (message)
    {
    case WM_MOUSEMOVE:
        me = MouseEvent::Event_MouseMove;
        break;
    case WM_LBUTTONDOWN:
        me = MouseEvent::Event_LButtonDown;
        break;
    case WM_LBUTTONUP:
        me = MouseEvent::Event_LButtonUp;
        break;
    case WM_RBUTTONDOWN:
        me = MouseEvent::Event_RButtonDown;
        break;
    case WM_RBUTTONUP:
        me = MouseEvent::Event_RButtonUp;
        break;
    case WM_MBUTTONDOWN:
        me = MouseEvent::Event_MButtonDown;
        break;
    case WM_MBUTTONUP:
        me = MouseEvent::Event_MButtonUp;
        break;
    case WM_MOUSEWHEEL:
        me = MouseEvent::Event_MouseWheelV;
        break;
    case WM_MOUSEHWHEEL:
        me = MouseEvent::Event_MouseWheelH;
        break;
    case WM_MOUSEHOVER:
        me = MouseEvent::Event_MouseHover;
        break;
    case WM_MOUSELEAVE:
        me = MouseEvent::Event_MouseLeave;
        break;
    default:
        me = MouseEvent::Event_None;
        break;
    }
    arg.event = me;
}

#ifdef _DEBUG
namespace LongUI {
    std::atomic_uintptr_t g_dbg_last_proc_window_pointer = 0;
    std::atomic<UINT> g_dbg_last_proc_message = 0;
}
#endif

// 窗口过程函数
LRESULT LongUI::CUIManager::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept {
#ifdef _DEBUG
    g_dbg_last_proc_message = message;
#endif
    // 返回值
    LRESULT recode = 0;
    // 创建窗口时设置指针
    if (message == WM_CREATE) {
        // 获取指针
        auto* window = reinterpret_cast<LongUI::UIWindow*>(
            (reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams
            );
        // 设置窗口指针
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(window));
        // 创建完毕
        window->OnCreated(hwnd);
        // 返回1
        recode = 1;
    }
    else {
        // 获取储存的指针
        auto* window = reinterpret_cast<LongUI::UIWindow *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA))
            );
        // 无效
        if (!window) return ::DefWindowProcW(hwnd, message, wParam, lParam);
#ifdef _DEBUG
        g_dbg_last_proc_window_pointer = reinterpret_cast<std::uintptr_t>(window);
#endif
        auto handled = false;
        // 鼠标事件?
        if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
            message == WM_MOUSELEAVE || message == WM_MOUSEHOVER) {
            MouseEventArgument arg;
            CUIManager::WindowsMsgToMouseEvent(arg, message, wParam, lParam);
            // 有效
            if (arg.event != MouseEvent::Event_None) {
                {
                    AutoLocker;
                    // 需要修正坐标
                    if (arg.event <= MouseEvent::Event_MouseWheelH) {
                        arg.pt = window->last_point;
                    }
                    // 位置有效->修改
                    if (arg.event >= MouseEvent::Event_MouseMove) {
                        window->last_point = arg.pt;
                    }
                    window->DoMouseEvent(arg);
                    // 总是处理了鼠标事件
                    handled = true;
                }
            }
        }
        // 一般事件
        else {
            // 设置参数
            LongUI::EventArgument arg;
            // 系统消息
            arg.msg = message;  arg.sender = nullptr;
            arg.sys.wParam = wParam; arg.sys.lParam = lParam; 
            arg.lr = 0;
#ifdef _DEBUG
            static std::atomic_int s_times = 0;
            // 不用推荐递归调用
            if (s_times) {
                UIManager << DL_Hint 
                    << L"recursive called. [UNRECOMMENDED]: depending on locker implementation." 
                    << endl;
            }
            ++s_times;
#endif
            {
                AutoLocker;
                // 默认处理
                if ((handled = window->DoEvent(arg))) {
                    recode = arg.lr;
                }
            }
#ifdef _DEBUG
            --s_times;
#endif
        }
        // 未处理
        if (!handled) {
            recode = ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
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
            auto data = ch1 + (ch2 - ch1) * prec;
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

// CUIManager 构造函数
LongUI::CUIManager::CUIManager() noexcept : m_config(*this) {

}

// CUIManager 析构函数
LongUI::CUIManager::~CUIManager() noexcept {
    this->discard_resources();
}

// 获取控件 wchar_t指针
auto LongUI::CUIManager::RegisterControlClass(
    CreateControlFunction func, const char* clname) noexcept ->HRESULT {
    if (!clname || !(*clname)) {
        assert(!"bad argument");
        return S_FALSE;
    }
    // 插入
    auto result = m_hashStr2CreateFunc.Insert(m_oStringAllocator.CopyString(clname), func);
    // 插入失败的原因只有一个->OOM
    return result ? S_OK : E_OUTOFMEMORY;
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
    XUIBasicTextRenderer* renderer, const char name[LongUITextRendererNameMaxLength]
    ) noexcept -> int32_t {
    assert(m_uTextRenderCount < lengthof(m_apTextRenderer) && "buffer too small");
    assert(!white_space(name[0]) && "name cannot begin with white space");
    // 满了
    if (m_uTextRenderCount == lengthof(m_apTextRenderer)) {
        return -1;
    }
    const auto count = m_uTextRenderCount;
    assert((std::strlen(name) + 1) < LongUITextRendererNameMaxLength && "buffer too small");
    std::strcpy(m_aszTextRendererName[count].name, name);
    m_apTextRenderer[count] = LongUI::SafeAcquire(renderer);
    ++m_uTextRenderCount;
    return count;
}


// 创建0索引资源
auto LongUI::CUIManager::create_indexzero_resources() noexcept ->HRESULT {
    assert(m_pResourceBuffer && "bad alloc");
    HRESULT hr = S_OK;
    // 索引0位图: 可MAP位图
    if (SUCCEEDED(hr)) {
        assert(m_ppBitmaps[LongUIDefaultBitmapIndex] == nullptr && "bad action");
        hr = m_pd2dDeviceContext->CreateBitmap(
            D2D1::SizeU(LongUIDefaultBitmapSize, LongUIDefaultBitmapSize),
            nullptr, LongUIDefaultBitmapSize * 4,
            D2D1::BitmapProperties1(
                static_cast<D2D1_BITMAP_OPTIONS>(LongUIDefaultBitmapOptions),
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
                ),
            m_ppBitmaps + LongUIDefaultBitmapIndex
            );
        longui_debug_hr(hr, L"_pd2dDeviceContext->CreateBitmap failed");
    }
    // 索引0笔刷: 全控件共享用前写纯色笔刷
    if (SUCCEEDED(hr)) {
        assert(m_ppBrushes[LongUICommonSolidColorBrushIndex] == nullptr && "bad action");
        ID2D1SolidColorBrush* brush = nullptr;
        D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Black);
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(&color, nullptr, &brush);
        m_ppBrushes[LongUICommonSolidColorBrushIndex] = LongUI::SafeAcquire(brush);
        LongUI::SafeRelease(brush);
        longui_debug_hr(hr, L"_pd2dDeviceContext->CreateSolidColorBrush failed");
    }
    // 索引0文本格式: 默认格式
    if (SUCCEEDED(hr)) {
        assert(m_ppTextFormats[LongUIDefaultTextFormatIndex] == nullptr && "bad action");
        hr = this->CreateTextFormat(
            LongUIDefaultTextFontName,
            //L"Microsoft YaHei",
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            LongUIDefaultTextFontSize,
            //12.f,
            m_ppTextFormats + LongUIDefaultTextFormatIndex
            );
        longui_debug_hr(hr, L"this->CreateTextFormat failed");
    }
    // 设置
    if (SUCCEEDED(hr)) {
        m_ppTextFormats[LongUIDefaultTextFormatIndex]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        m_ppTextFormats[LongUIDefaultTextFormatIndex]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    // 索引0图元: 暂无
    if (SUCCEEDED(hr)) {

    }
    return hr;
}

// 清理延迟清理链
void LongUI::CUIManager::cleanup_delay_cleanup_chain() noexcept {
    for (auto ctrl : m_vDelayCleanup) {
        ctrl->cleanup();
    }
    m_vDelayCleanup.clear();
}

// 载入模板字符串
auto LongUI::CUIManager::load_control_template_string(const char* str) noexcept ->HRESULT {
    // 检查参数
    if (str && *str) {
        // 载入字符串
        auto code = m_docTemplate.load_string(str);
        if (code.status) {
            assert(!"load error");
            ::MessageBoxA(nullptr, code.description(), "<LongUI::CUIManager::load_control_template_string>: Failed to Parse/Load XML", MB_ICONERROR);
            return E_FAIL;
        }
        // 获取子结点数量
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
auto LongUI::CUIManager::set_control_template_string() noexcept ->HRESULT {
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
    // 重新获取flag
    this->flag = this->configure->GetConfigureFlag();
    // 待用适配器
    IDXGIAdapter1* adapter = nullptr;
    // 枚举显示适配器
    if (!(this->flag & IUIConfigure::Flag_RenderByCPU)) {
        IDXGIFactory1* dxgifactory = nullptr;
        // 创建一个临时工程
        if (SUCCEEDED(LongUI::Dll::CreateDXGIFactory1(
            IID_IDXGIFactory1, reinterpret_cast<void**>(&dxgifactory)
            ))) {
            uint32_t adnum = 0;
            IDXGIAdapter1* apAdapters[LongUIMaxAdaptersSize];
            DXGI_ADAPTER_DESC1 descs[LongUIMaxAdaptersSize];
            // 枚举适配器
            for (adnum = 0; adnum < lengthof(apAdapters); ++adnum) {
                if (dxgifactory->EnumAdapters1(adnum, apAdapters + adnum) == DXGI_ERROR_NOT_FOUND) {
                    break;
                }
                apAdapters[adnum]->GetDesc1(descs + adnum);
            }
            // 选择适配器
            auto index = this->configure->ChooseAdapter(descs, adnum);
            if (index < adnum) {
                adapter = LongUI::SafeAcquire(apAdapters[index]);
            }
            // 释放适配器
            for (size_t i = 0; i < adnum; ++i) {
                LongUI::SafeRelease(apAdapters[i]);
            }
        }
        LongUI::SafeRelease(dxgifactory);
    }
    // 创建设备资源
    HRESULT hr /*= m_docResource.Error() ? E_FAIL :*/ S_OK;
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
        // 根据情况检查驱动类型
        D3D_DRIVER_TYPE dtype = (this->flag & IUIConfigure::Flag_RenderByCPU) ? D3D_DRIVER_TYPE_WARP :
            (adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE);
        // 创建设备
        hr = LongUI::Dll::D3D11CreateDevice(
            // 设置为渲染
            adapter,
            // 驱动类型
            dtype,
            // 没有软件接口
            nullptr,
            // 创建flag
            creationFlags,
            // 欲使用的特性等级列表
            featureLevels,
            // 特性等级列表长度
            static_cast<UINT>(lengthof(featureLevels)),
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
            UIManager << DL_Warning 
                << L"create d3d11 device failed, now, try to create in warp mode" 
                << LongUI::endl;
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
                static_cast<UINT>(lengthof(featureLevels)),
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
        // 再次检查错误
        if (FAILED(hr)) {
            UIManager << DL_Error
                << L" create d3d11-device in warp modd, but failed."
                << LongUI::endl;
            this->ShowError(hr);
        }
    }
    LongUI::SafeRelease(adapter);
    // 创建 ID3D11Debug对象
#if defined(_DEBUG) && defined(LONGUI_D3D_DEBUG)
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pd3dDebug));
        longui_debug_hr(hr, L"m_pd3dDevice->QueryInterface(m_pd3dDebug) faild");
    }
#endif
    // 创建 IDXGIDevice
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(m_pDxgiDevice));
        longui_debug_hr(hr, L"m_pd3dDevice->QueryInterface(m_pd3dDebug) faild");
    }
    // 创建 D2D设备
    if (SUCCEEDED(hr)) {
        hr = m_pd2dFactory->CreateDevice(m_pDxgiDevice, &m_pd2dDevice);
        longui_debug_hr(hr, L"m_pd2dFactory->CreateDevice faild");
    }
    // 创建 D2D设备上下文
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pd2dDeviceContext
            );
        longui_debug_hr(hr, L"m_pd2dDevice->CreateDeviceContext faild");
    }
    // 获取 Dxgi适配器 可以获取该适配器信息
    if (SUCCEEDED(hr)) {
        // 顺带使用像素作为单位
        m_pd2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        // 获取
        hr = m_pDxgiDevice->GetAdapter(&m_pDxgiAdapter);
        longui_debug_hr(hr, L"m_pDxgiDevice->GetAdapter faild");
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
#ifdef LONGUI_WITH_MMFVIDEO
    UINT token = 0;
    // 多线程
    if (SUCCEEDED(hr)) {
        ID3D10Multithread* mt = nullptr;
        hr = m_pd3dDevice->QueryInterface(LongUI_IID_PV_ARGS(mt));
        longui_debug_hr(hr, L"m_pd3dDevice->QueryInterface ID3D10Multithread faild");
        // 保护
        if (SUCCEEDED(hr)) {
            mt->SetMultithreadProtected(TRUE);
        }
        LongUI::SafeRelease(mt);
    }
    // 设置 MF
    if (SUCCEEDED(hr)) {
        hr = ::MFStartup(MF_VERSION);
        longui_debug_hr(hr, L"MFStartup faild");
    }
    // 创建 MF Dxgi 设备管理器
    if (SUCCEEDED(hr)) {
        hr = ::MFCreateDXGIDeviceManager(&token, &m_pMFDXGIManager);
        longui_debug_hr(hr, L"MFCreateDXGIDeviceManager faild");
    }
    // 重置设备
    if (SUCCEEDED(hr)) {
        hr = m_pMFDXGIManager->ResetDevice(m_pd3dDevice, token);
        longui_debug_hr(hr, L"m_pMFDXGIManager->ResetDevice faild");
    }
    // 创建 MF媒体类工厂
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_MFMediaEngineClassFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pMediaEngineFactory)
            );
        longui_debug_hr(hr, L"CoCreateInstance CLSID_MFMediaEngineClassFactory faild");
    }
#endif
    // 创建系统笔刷
    if (SUCCEEDED(hr)) {
        hr = this->create_system_brushes();
        longui_debug_hr(hr, L"create_system_brushes faild");
    }
    // 创建资源描述资源
    if (SUCCEEDED(hr)) {
        hr = this->create_indexzero_resources();
        longui_debug_hr(hr, L"create_indexzero_resources faild");
    }
    // 事件
    if (SUCCEEDED(hr)) {
        this->do_creating_event(LongUI::CreateEventType::Type_Recreate);
        longui_debug_hr(hr, L"do_creating_event(recreate) faild");
    }
    // 设置文本渲染器数据
    if (SUCCEEDED(hr)) {
        for (uint32_t i = 0u; i < m_uTextRenderCount; ++i) {
            m_apTextRenderer[i]->SetNewTarget(m_pd2dDeviceContext);
            m_apTextRenderer[i]->SetNewBrush(
                static_cast<ID2D1SolidColorBrush*>(m_ppBrushes[LongUICommonSolidColorBrushIndex])
                );
        }
    }
    // 重建所有窗口
    for (auto itr = m_apWindows; itr < m_apWindows + m_cCountWindow; ++itr) {
        auto wnd = *itr;
        if (SUCCEEDED(hr)) {
            hr = wnd->Recreate();
            longui_debug_hr(hr, wnd << L"wnd->Recreate");
        }
    }
    // 断言 HR
    ShowHR(hr);
    return hr;
}


// 创建系统笔刷
auto LongUI::CUIManager::create_system_brushes() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    /*
    焦点: 0x3399FF 矩形描边, 并且内边有虚线矩形
        0. 禁用: 0xBF灰度 矩形描边; 中心 0xCC灰色
        1. 普通: 0xAC灰度 矩形描边; 中心 从上到下0xF0灰色到0xE5灰色渐变
        2. 移上: 0x7EB4EA 矩形描边; 中心 从上到下0xECF4FC到0xDCECFC渐变
        3. 按下: 0x569DE5 矩形描边; 中心 从上到下0xDAECFC到0xC4E0FC渐变
    */
    // 禁用
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(0xCCCCCC),
            reinterpret_cast<ID2D1SolidColorBrush**>(m_apSystemBrushes + State_Disabled)
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
                stops, lengthof<uint32_t>(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + State_Normal)
                );
        }
        LongUI::SafeRelease(collection);
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
                stops, lengthof<uint32_t>(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + State_Hover)
                );
        }
        LongUI::SafeRelease(collection);
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
                stops, lengthof<uint32_t>(stops), &collection
                );
        }
        // 创建笔刷
        if (SUCCEEDED(hr)) {
            hr = m_pd2dDeviceContext->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(), D2D1::Point2F(0.f, 1.f)
                    ),
                collection,
                reinterpret_cast<ID2D1LinearGradientBrush**>(m_apSystemBrushes + State_Pushed)
                );
        }
        LongUI::SafeRelease(collection);
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
        LongUI::SafeRelease(brush);
    }
    // 释放公共设备相关资源
    {
        // 释放 位图
        for (auto itr = m_ppBitmaps; itr != m_ppBitmaps + m_cCountBmp; ++itr) {
            LongUI::SafeRelease(*itr);
        }
        // 释放 笔刷
        for (auto itr = m_ppBrushes; itr != m_ppBrushes + m_cCountBrs; ++itr) {
            LongUI::SafeRelease(*itr);
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
    LongUI::SafeRelease(m_pDxgiFactory);
    LongUI::SafeRelease(m_pd2dDeviceContext);
    LongUI::SafeRelease(m_pd2dDevice);
    LongUI::SafeRelease(m_pDxgiAdapter);
    LongUI::SafeRelease(m_pDxgiDevice);
    LongUI::SafeRelease(m_pd3dDevice);
    LongUI::SafeRelease(m_pd3dDeviceContext);
#ifdef LONGUI_WITH_MMFVIDEO
    LongUI::SafeRelease(m_pMFDXGIManager);
    LongUI::SafeRelease(m_pMediaEngineFactory);
    ::MFShutdown();
#endif
#ifdef _DEBUG
#ifdef _MSC_VER
    __try {
        if (m_pd3dDebug) {
            LongUI::SafeRelease(g_pd3dDebug_longui);
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
    LongUI::SafeRelease(m_pd3dDebug);
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
    return LongUI::SafeAcquire(bitmap);
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
    return LongUI::SafeAcquire(brush);
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
    return LongUI::SafeAcquire(format);
}

// 利用名称获取
auto LongUI::CUIManager::GetTextRenderer(const char* name) const noexcept -> XUIBasicTextRenderer* {
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
        std::memset(&meta, 0, sizeof(meta));
        return;
    }
    meta = m_pMetasBuffer[index];
    // 没有位图数据则载入
    if (!meta.bitmap) {
        // 没有数据并且没有资源加载器则?
        assert(m_pResourceLoader);
        DeviceIndependentMeta meta_raw;
        ::std::memset(&meta_raw, 0, sizeof(meta_raw));
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
    // 大小保证
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
        longui_debug_hr(hr, L"bitmap->CopyFromBitmap failed");
    }
    // 映射数据
    if (SUCCEEDED(hr)) {
        D2D1_MAPPED_RECT mapped_rect = {
            LongUIDefaultBitmapSize * sizeof(RGBQUAD) ,
            m_pBitmap0Buffer
        };
        hr = bitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped_rect);
        longui_debug_hr(hr, L"bitmap->Map failed");
    }
    // 取消映射
    if (SUCCEEDED(hr)) {
        hr = bitmap->Unmap();
        longui_debug_hr(hr, L"bitmap->Unmap failed");
    }
    // 转换数据
    HICON hAlphaIcon = nullptr;
    if (SUCCEEDED(hr)) {
        auto meta_width = src_rect.right - src_rect.left;
        auto meta_height = src_rect.bottom - src_rect.top;
#if 1
        BITMAPV5HEADER bi; std::memset(&bi, 0, sizeof(BITMAPV5HEADER));
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
        // 错误
        if (!hBitmap) {
            hr = LongUI::WinCode2HRESULT(::GetLastError());
            longui_debug_hr(hr, L"CreateDIBSection failed");
        }
        // 成功
        else {
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
    }
    ShowHR(hr);
    LongUI::SafeRelease(bitmap);
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

// 刷新屏幕刷新率
void LongUI::CUIManager::RefreshDisplayFrequency() noexcept {
    // 获取屏幕刷新率
    DEVMODEW mode; std::memset(&mode, 0, sizeof(mode));
    ::EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode);
    m_dDisplayFrequency = static_cast<uint16_t>(mode.dmDisplayFrequency);
    // 稍微检查
    if (!m_dDisplayFrequency) {
        UIManager << DL_Error
            << L"EnumDisplaySettingsW failed: got zero for DEVMODEW::dmDisplayFrequency"
            << L", now assume as 60Hz"
            << LongUI::endl;
        m_dDisplayFrequency = 60;
    }
}

// 移出窗口
void LongUI::CUIManager::RemoveWindow(UIWindow* wnd, bool cleanup) noexcept {
    assert(m_cCountWindow); assert(wnd && "bad argument");
    // 清理?
    if (cleanup) {
        wnd->cleanup();
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
    // 正式移除
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        auto itr = std::find(m_apWindows, endwindow, wnd);
        if (itr != endwindow) {
            std::memmove(itr, itr + 1, sizeof(void*));
            --m_cCountWindow;
            m_apWindows[m_cCountWindow] = nullptr;
        }
    }
    // 检查时是不是在本数组中
#ifdef _DEBUG
    {
        auto endwindow = m_apWindows + m_cCountWindow;
        if (std::find(m_apWindows, endwindow, wnd) != endwindow) {
            assert(!"target window in windows array!");
            return;
        }
    }
#endif
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

#ifdef _DEBUG

// 传递可视化东西
auto LongUI::Formated(const wchar_t* format, ...) noexcept -> const wchar_t* {
    static thread_local wchar_t buffer[LongUIStringBufferLength];
    va_list ap;
    va_start(ap, format);
    std::vswprintf(buffer, LongUIStringBufferLength, format, ap);
    va_end(ap);
    return buffer;
}

// 传递可视化东西
auto LongUI::Interfmt(const wchar_t* format, ...) noexcept -> const wchar_t* {
    static thread_local wchar_t buffer[LongUIStringBufferLength];
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

auto LongUI::CUIManager::operator<<(const DXGI_ADAPTER_DESC& desc) noexcept ->CUIManager& {
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

auto LongUI::CUIManager::operator<<(const RectLTWH_F& rect) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_WH(%7.2f, %7.2f, %7.2f, %7.2f)",
        rect.left, rect.top, rect.width, rect.height
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_MATRIX_3X2_F& matrix) noexcept ->CUIManager& {
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

auto LongUI::CUIManager::operator<<(const D2D1_RECT_F& rect) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(
        buffer, LongUIStringBufferLength,
        L"RECT_RB(%7.2f, %7.2f, %7.2f, %7.2f)",
        rect.left, rect.top, rect.right, rect.bottom
        );
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

auto LongUI::CUIManager::operator<<(const D2D1_POINT_2F& pt) noexcept ->CUIManager& {
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
        std::swprintf(
            buffer, LongUIStringBufferLength,
            L"[0x%p{%S}%ls] ",
            ctrl,
            ctrl->name.c_str(),
            ctrl->GetControlClassName(false)
            );
    }
    else {
        std::swprintf(buffer, LongUIStringBufferLength, L"[null] ");
    }
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

// 控件
auto LongUI::CUIManager::operator<<(const ControlVector& ctrls) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    int index = 0;
    for (auto ctrl : ctrls) {
        std::swprintf(
            buffer, lengthof(buffer),
            L"\r\n\t\t[%4d][0x%p{%S}%ls] ",
            index,
            ctrl,
            ctrl->name.c_str(),
            ctrl->GetControlClassName(false)
            );
        this->OutputNoFlush(m_lastLevel, buffer);
        ++index;
    }
    return *this;
}

// 整型重载
auto LongUI::CUIManager::operator<<(const long l) noexcept ->CUIManager& {
    wchar_t buffer[LongUIStringBufferLength];
    std::swprintf(buffer, LongUIStringBufferLength, L"%ld", l);
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
    // 之前的
    auto oldf = m_wDisplayFrequency;
    // 修改
    m_wDisplayFrequency = static_cast<decltype(m_wDisplayFrequency)>(freq);
    // 创建
    CUIRenderQueue::UNIT* data = nullptr;
    if (freq && (data = LongUI::NormalAllocT<UNIT>(LongUIPlanRenderingTotalTime * freq))) {
        for (auto i = 0u; i < LongUIPlanRenderingTotalTime * freq; ++i) {
            data[i].length = 0;
        }
    }
    // XXX: 完成转化
    if (m_pUnitsDataBegin && data) {
        // 偷懒直接不管
        oldf = oldf;
        UIManager << DL_Hint
            << L"hard to code it, unfinished yet, empty the rendering queue now"
            << LongUI::endl;
    }
    // 释放
    if (m_pUnitsDataBegin) LongUI::NormalFree(m_pUnitsDataBegin);
    // 转移
    if (data) {
        m_pUnitsDataBegin = data;
        m_pUnitsDataEnd = data + LongUIPlanRenderingTotalTime * freq;
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
            auto time = m_dwStartTime;
            m_dwStartTime = ::timeGetTime();
            time = m_dwStartTime - time;
            int16_t dev = int16_t(int16_t(time) - int16_t(LongUIPlanRenderingTotalTime * 1000));
            m_sTimeDeviation += dev;
#ifdef _DEBUG
            if (m_unitLike.window->debug_this) {
                UIManager << DL_Log
                    << Formated(L"Time Deviation: %4ldms    Totle: %4ldms", 
                        long(dev), long(m_sTimeDeviation))
                    << endl;
            }
#endif
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
    // XXX: 待优化。
    // 当前窗口
    auto window = m_unitLike.window;
#ifdef _DEBUG
    if (window->debug_this) {
        UIManager << DL_Log
            << L"INDEX:[" << long(m_pCurrentUnit - m_pUnitsDataBegin) << L']'
            << ctrl << ctrl->visible_rect
            << L"from " << wait << L" to " << render
            << endl;
    }
#endif
    // 保留刷新
    if (render != 0.0f) render += 0.02f;
    assert((wait + render) < float(LongUIPlanRenderingTotalTime) && "time overflow");
    // 设置单元
    auto set_unit = [window](CUIRenderQueue::UNIT& unit, UIControl* ctrl) noexcept {
        // 已经全渲染了就不干
        if (unit.length && unit.units[0] == window) {
            return;
        }
        // 单元满了/直接渲染窗口 就设置为全渲染
        if (unit.length == LongUIDirtyControlSize || ctrl == window) {
            unit.length = 1;
            unit.units[0] = window;
            return;
        }
        // 保存
        auto old_length = unit.length;
        bool changed = false;
        CUIRenderQueue::UNIT tmp; std::memset(&tmp, 0, sizeof(tmp));
        std::memcpy(tmp.units, unit.units, sizeof(tmp.units[0]) * old_length);
#ifdef _DEBUG
        // 调试信息
        CUIRenderQueue::UNIT debug_backup;
        std::memcpy(&debug_backup, &tmp, sizeof(debug_backup));
        debug_backup.length = unit.length;
        bool jmp = true;
        if ((jmp = false)) {
            std::memcpy(&tmp, &debug_backup, sizeof(tmp));
        }
#endif
        // 一次检查
        for (auto itr = tmp.units; itr < tmp.units + old_length; ++itr) {
            // 已存在的空间
            auto existd = *itr;
            // 一样? --> 不干
            if (existd == ctrl) return;
            // 存在深度 < 插入深度 -> 检查插入的是否为存在的子孙结点
            if (existd->level < ctrl->level) {
                // 是 -> 什么不干
                if (existd->IsPosterityForSelf(ctrl)) return;
                // 否 -> 继续
                else {

                }
            }
            // 存在深度 > 插入深度 -> 检查存在的是否为插入的子孙结点
            else if(existd->level > ctrl->level) {
                // 是 -> 替换所有
                if (ctrl->IsPosterityForSelf(existd)) {
                    *itr = nullptr;
                    changed = true;
                }
                // 否 -> 继续
                else {

                }
            }
            // 深度一致 -> 继续
            else {

            }
        }
#ifdef _DEBUG
        if (window->debug_this) {
            UIManager << DLevel_Log << L"\r\n [INSERT]: " << ctrl << endl;
        }
#endif
        // 二次插入
        if (changed) {
            unit.length = 0; auto witr = unit.units;
            for (auto ritr = tmp.units; ritr < tmp.units + old_length; ++ritr) {
                if (*ritr) {
                    *witr = *ritr;
                    ++witr;
                    ++unit.length;
                }
            }
        }
#ifdef _DEBUG
        // 断言调试
        auto endt = unit.units + unit.length;
        assert(std::find(unit.units, endt, ctrl) == endt);
        std::for_each(unit.units, endt, [ctrl](UIControl* tmpc) noexcept {
            assert(tmpc->IsPosterityForSelf(ctrl) == false && "bad ship");
            assert(ctrl->IsPosterityForSelf(tmpc) == false && "bad ship");
        });
#endif
        // 添加到最后
        unit.units[unit.length++] = ctrl;
    };
    // 渲染队列模式
    if (m_pCurrentUnit) {
        // 该控件渲染
        auto rerendered = ctrl->prerender;
        // 时间片计算
        auto frame_offset = uint32_t(wait * float(m_wDisplayFrequency));
        auto frame_count = uint32_t(render * float(m_wDisplayFrequency)) + 1;
        auto start = m_pCurrentUnit + frame_offset;
        for (uint32_t i = 0; i < frame_count; ++i) {
            if (start == m_pUnitsDataEnd) {
                start = m_pUnitsDataBegin;
            }
#ifdef _DEBUG
            if (window->debug_this) {
                UIManager << DLevel_Log << L" [TRY] ";
            }
#endif
            set_unit(*start, rerendered);
            ++start;
        }
#ifdef _DEBUG
        if (window->debug_this) {
            UIManager << DLevel_Log << L"\r\n";
        }
#endif
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
}

#ifdef LONGUI_WITH_DEFAULT_CONFIG
#include <wincodec.h>
//  ---------- Resource Loader for XML -----------------
namespace LongUI {
    // IWICImagingFactory2 "7B816B45-1996-4476-B132-DE9E247C8AF0"
    static const IID IID_IWICImagingFactory2 = {
        0x7B816B45, 0x1996, 0x4476,{ 0xB1, 0x32, 0xDE, 0x9E, 0x24, 0x7C, 0x8A, 0xF0 }
    };
    template<> LongUIInline const IID& GetIID<IWICImagingFactory2>() {
        return LongUI::IID_IWICImagingFactory2;
    }
    // CUIResourceLoaderXML, default impl for IUIResourceLoader
    class CUIResourceLoaderXML : public IUIResourceLoader {
    public:
        // add ref count
        auto STDMETHODCALLTYPE AddRef() noexcept ->ULONG override final { return ++m_dwCounter; }
        // release this
        auto STDMETHODCALLTYPE Release() noexcept ->ULONG override final { 
            auto old = --m_dwCounter; 
            if (!old) { delete this; } 
            return old; 
        };
    public:
        // get resouce count with type
        auto GetResourceCount(ResourceType type) const noexcept ->size_t override;
        // get resouce by index, index in range [0, count)
        // for Type_Bitmap, Type_Brush, Type_TextFormat
        auto GetResourcePointer(ResourceType type, size_t index) noexcept ->void* override;
        // get meta by index, index in range [0, count)
        auto GetMeta(size_t index, DeviceIndependentMeta&) noexcept ->void override;
    private:
        // get resouce count from doc
        void get_resource_count_from_xml() noexcept;
        // get bitmap
        auto get_bitmap(pugi::xml_node node) noexcept ->ID2D1Bitmap1*;
        // get brush
        auto get_brush(pugi::xml_node node) noexcept ->ID2D1Brush*;
        // get text format
        auto get_text_format(pugi::xml_node node) noexcept ->IDWriteTextFormat*;
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
        std::memset(m_aResourceCount, 0, sizeof(m_aResourceCount));
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
        LongUI::SafeRelease(m_pWICFactory);
    }
    // get reource count
    auto LongUI::CUIResourceLoaderXML::GetResourceCount(ResourceType type) const noexcept -> size_t {
        assert(type < this->RESOURCE_TYPE_COUNT);
        return static_cast<size_t>(m_aResourceCount[type]);
    }
    // get reource
    auto LongUI::CUIResourceLoaderXML::GetResourcePointer(ResourceType type, size_t index) noexcept -> void* {
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
            data = this->get_text_format(node);
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
            Helper::GetEnumFromXml(node, BitmapRenderRule::Rule_Scale),
            uint16_t(Helper::GetEnumFromXml(node, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR))
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
        auto now_node = m_docResource.first_child().first_child();
        while (now_node) {
            // 获取子结点数量
            auto get_children_count = [](pugi::xml_node node) {
                node = node.first_child();
                auto count = 0ui32;
                while (node) { node = node.next_sibling(); ++count; }
                return count;
            };
            // 位图?
            if (!std::strcmp(now_node.name(), "Bitmap")) {
                m_aNodes[Type_Bitmap] = now_node;
                m_aResourceCount[this->Type_Bitmap] = get_children_count(now_node);
            }
            // 笔刷?
            else if (!std::strcmp(now_node.name(), "Brush")) {
                m_aNodes[Type_Brush] = now_node;
                m_aResourceCount[this->Type_Brush] = get_children_count(now_node);
            }
            // 文本格式?
            else if (!std::strcmp(now_node.name(), "Font") ||
                !std::strcmp(now_node.name(), "TextFormat")) {
                m_aNodes[Type_TextFormat] = now_node;
                m_aResourceCount[this->Type_TextFormat] = get_children_count(now_node);
            }
            // 图元?
            else if (!std::strcmp(now_node.name(), "Meta")) {
                m_aNodes[Type_Meta] = now_node;
                m_aResourceCount[this->Type_Meta] = get_children_count(now_node);
            }
            // 动画图元?
            else if (!std::strcmp(now_node.name(), "MetaEx")) {
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
            ID2D1DeviceContext *pRenderTarget,
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
            HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
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
                LongUI::SafeRelease(tmp_bitmap);
                LongUI::SafeRelease(tar_bitmap);
            }
#endif
            LongUI::SafeRelease(pDecoder);
            LongUI::SafeRelease(pSource);
            LongUI::SafeRelease(pStream);
            LongUI::SafeRelease(pConverter);
            LongUI::SafeRelease(pScaler);
            return hr;
        };
        ID2D1Bitmap1* bitmap = nullptr;
        // 转换路径
        wchar_t path_buffer[LongUIStringBufferLength];
        path_buffer[LongUI::UTF8toWideChar(uri, path_buffer)] = 0;
        // 载入
        auto hr = load_bitmap_from_file(
            m_manager.GetRenderTargetNoAddRef(), 
            m_pWICFactory, path_buffer, 0u, 0u, &bitmap
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
            m_manager.GetRenderTargetNoAddRef()->CreateSolidColorBrush(&color, &brush_prop, &scb);
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
                char ch = 0;
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
                m_manager.GetRenderTargetNoAddRef()->CreateGradientStopCollection(stops, stop_count, &collection);
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
                        m_manager.GetRenderTargetNoAddRef()->CreateLinearGradientBrush(
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
                        m_manager.GetRenderTargetNoAddRef()->CreateRadialGradientBrush(
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
                    Helper::GetEnumFromXml(node, D2D1_EXTEND_MODE_CLAMP, "extendx"),
                    Helper::GetEnumFromXml(node, D2D1_EXTEND_MODE_CLAMP, "extendy"),
                    Helper::GetEnumFromXml(node, D2D1_INTERPOLATION_MODE_LINEAR, "interpolation"),
                };
                // 创建位图笔刷
                m_manager.GetRenderTargetNoAddRef()->CreateBitmapBrush(
                    bitmap, &bbprop, &brush_prop, &b1b
                    );
                LongUI::SafeRelease(bitmap);
            }
            break;
        }
        assert(brush && "unknown error but error");
        return brush;
    }
    // get textformat
    auto LongUI::CUIResourceLoaderXML::get_text_format(pugi::xml_node node) noexcept -> IDWriteTextFormat* {
        assert(node && "node not found");
        IDWriteTextFormat* fmt = nullptr;
        auto hr = DX::MakeTextFormat(node, &fmt);
        UNREFERENCED_PARAMETER(hr);
        assert(SUCCEEDED(hr));
        return fmt;
    }
}
#endif

                   

#if defined(_DEBUG)  && 1
#define TRACE_FUCTION UIManager << DL_Log << L"Trace: called" << LongUI::endl
#else
#define TRACE_FUCTION
#endif


// UI富文本编辑框: Render 渲染 
void LongUI::UIRichEdit::Render() const noexcept {
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
}

// UI富文本编辑框: Render 刷新
void LongUI::UIRichEdit::Update() noexcept {

}

// UIRichEdit 构造函数
inline LongUI::UIRichEdit::UIRichEdit(UIContainer* cp) noexcept: Super(cp){ }

// UIRichEdit 析构函数
LongUI::UIRichEdit::~UIRichEdit() noexcept {
    LongUI::SafeRelease(m_pFontBrush);
    if (m_pTextServices) {
        m_pTextServices->OnTxInPlaceDeactivate();
    }
    // 关闭服务
    UIRichEdit::ShutdownTextServices(m_pTextServices);
    //LongUI::SafeRelease(m_pTextServices);
}

// UIRichEdit::CreateControl 函数
LongUI::UIControl* LongUI::UIRichEdit::CreateControl(CreateEventType type, pugi::xml_node node) noexcept {
    // 分类判断
    UIRichEdit* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIRichEdit, pControl, type, node);
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
        /*case LongUI::Event::Event_MouseEnter:
            m_pWindow->now_cursor = m_hCursorI;
            break;
        case LongUI::Event::Event_MouseLeave:
            m_pWindow->now_cursor = m_pWindow->default_cursor;
            break;*/
        case LongUI::Event::Event_SetFocus:
            if (m_pTextServices) {
                m_pTextServices->OnTxUIActivate();
                m_pTextServices->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
            }
            return true;
        case LongUI::Event::Event_KillFocus:
            if (m_pTextServices) {
                m_pTextServices->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
                m_pTextServices->OnTxUIDeactivate();
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
    LongUI::SafeRelease(m_pTextServices);
    LongUI::SafeRelease(m_pFontBrush);
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
    LongUI::SafeRelease(pUk);
    return Super::Recreate();
}

// close this control 关闭控件
void LongUI::UIRichEdit::cleanup() noexcept {
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

HRESULT LongUI::UIRichEdit::TxNotify(DWORD iNotify, void*pv){
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
LongUI::UIScrollBar::UIScrollBar(UIContainer* cp) noexcept : 
    Super(cp),m_uiAnimation(AnimationType::Type_QuadraticEaseIn) {

}

/// <summary>
/// Initalizes with specified xml-node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void LongUI::UIScrollBar::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 边界相关
    assert((this->flags & Flag_MarginalControl) && "'UIScrollBar' must be marginal-control");
    auto sbtype = (this->marginal_type & 1U) ? ScrollBarType::Type_Horizontal : ScrollBarType::Type_Vertical;
    force_cast(this->bartype) = sbtype;
    // 修改
    m_uiAnimation.duration = 0.4f;
    // 结点有效
    if (node) {
        const char* str = nullptr;
        // 滚轮步长
        if ((str = node.attribute("wheelstep").value())) {
            this->wheel_step = LongUI::AtoF(str);
        }
        // 动画时间
        if ((str = node.attribute("aniamtionduration").value())) {
            m_uiAnimation.duration = LongUI::AtoF(str);;
        }
        // 动画类型
        if ((str = node.attribute("aniamtionbartype").value())) {
            m_uiAnimation.type = static_cast<AnimationType>(LongUI::AtoI(str));
        }
    }
    // 初始化
    m_uiAnimation.start = m_uiAnimation.end = m_uiAnimation.value = 0.f;
}


// 设置新的索引位置
void LongUI::UIScrollBar::SetIndex(float new_index) noexcept {
    // 阈值检查
    new_index = std::min(std::max(new_index, 0.f), m_fMaxIndex);
    // 无需设定
    if (new_index == m_uiAnimation.end) return;
    // 设定位置
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
        UIManager << DL_Log  << this << L"m_fMaxIndex: "
            << m_fMaxIndex << L" -- scrollbar standby?"<< endl;
    }
#endif
#if 0
    if (m_fMaxIndex < BASIC_SIZE * 0.5f) {
#else
    if (m_fMaxIndex < 0.5f) {
#endif
        this->marginal_width = 0.f;
        this->SetVisible(false);
    }
    else {
        this->marginal_width = BASIC_SIZE;
        this->SetVisible(true);
    }
}

// UIScrollBarA 初始化
void LongUI::UIScrollBarA::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    auto stt = this->check_state();
    m_uiArrow1.Init(stt, 0, node,"arrow1");
    m_uiArrow2.Init(stt, 0, node, "arrow2");
    m_uiThumb.Init(stt, 0, node, "thumb");
    // 创建几何
    if (this->bartype == ScrollBarType::Type_Horizontal) {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Left]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Right]);
    }
    // 垂直滚动条
    else {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Top]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Bottom]);
    }
    assert(m_pArrow1Geo && m_pArrow2Geo);
    // 修改颜色
    if (node) {
        auto str = node.attribute("arrowstep").value();
        if (str) {
            m_fArrowStep = LongUI::AtoF(str);
        }
    }
    // 修改颜色
    /*else*/ {
        D2D1_COLOR_F color;
        color = D2D1::ColorF(0xF0F0F0);
        m_uiArrow1.GetBasicInterface().colors[State_Normal] = color;
        m_uiArrow2.GetBasicInterface().colors[State_Normal] = color;
        color = D2D1::ColorF(0x2F2F2F);
        m_uiArrow1.GetBasicInterface().colors[State_Pushed] = color;
        m_uiArrow2.GetBasicInterface().colors[State_Pushed] = color;
    }
    // 检查属性
    m_bArrow1InColor = !m_uiArrow1.IsExtraInterfaceValid();
    m_bArrow2InColor = !m_uiArrow2.IsExtraInterfaceValid();
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
    float length_of_thumb, start_offset;
    {
        float tmpsize = UISB_OffsetVaule(this->view_size.width) - BASIC_SIZE*2.f;
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
void LongUI::UIScrollBarA::Render() const noexcept {
    // 更新
    D2D1_RECT_F draw_rect; this->GetViewRect(draw_rect);
    // 双滚动条修正
    m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(0xF0F0F0));
    UIManager_RenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
    // 渲染部件
    m_uiArrow1.Render(m_rtArrow1);
    m_uiThumb.Render(m_rtThumb);
    m_uiArrow2.Render(m_rtArrow2);
    // 前景
    auto render_geo = [](ID2D1RenderTarget* const target, ID2D1Brush* const bush,
        ID2D1Geometry* const geo, const D2D1_RECT_F& rect) noexcept {
        D2D1_MATRIX_3X2_F matrix; target->GetTransform(&matrix);
        target->SetTransform(
            DX::Matrix3x2F::Translation(rect.left, rect.top) * matrix
            );
        target->DrawGeometry(geo, bush, 2.33333f);
        // 修改
        target->SetTransform(&matrix);
    };
    // 渲染几何体
    {
        D2D1_COLOR_F color;
        if (m_bArrow1InColor) {
            color = m_uiArrow1.GetBasicInterface().colors[m_uiArrow1.GetNowBasicState()];
            color.r = 1.f - color.r; color.g = 1.f - color.g; color.b = 1.f - color.b;
            m_pBrush_SetBeforeUse->SetColor(&color);
            render_geo(UIManager_RenderTarget, m_pBrush_SetBeforeUse, m_pArrow1Geo, m_rtArrow1);
        }
        // 渲染几何体
        if (m_bArrow2InColor) {
            color = m_uiArrow2.GetBasicInterface().colors[m_uiArrow2.GetNowBasicState()];
            color.r = 1.f - color.r; color.g = 1.f - color.g; color.b = 1.f - color.b;
            m_pBrush_SetBeforeUse->SetColor(&color);
            render_geo(UIManager_RenderTarget, m_pBrush_SetBeforeUse, m_pArrow2Geo, m_rtArrow2);
        }
    }
}

// UIScrollBarA::do event 事件处理
bool  LongUI::UIScrollBarA::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    // -------------------- on mouse move --------------------
    auto on_mouse_move = [this, &pt4self]() {
        // Captured状态
        if (m_bCaptured) {
            // 指向thumb?
            if (m_pointType == PointType::Type_Thumb) {
                // 计算移动距离
                auto pos = UISB_OffsetVaule(pt4self.x);
                auto zoom = this->parent->GetZoom(int(this->bartype));
                auto rate = (1.f - m_fMaxIndex / (m_fMaxRange - BASIC_SIZE*2.f)) * zoom;
                this->set_index((pos - m_fOldPoint) / rate + m_fOldIndex);
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
                this->set_state(m_lastPointType, LongUI::State_Normal);
                this->set_state(m_pointType, LongUI::State_Hover);
                m_lastPointType = m_pointType;
            }
        }
    };
    // -------------------- on l-button down --------------------
    auto on_lbutton_down = [this, &pt4self]() {
        m_pWindow->SetCapture(this);
        m_bCaptured = true;
        this->set_state(m_pointType, LongUI::State_Pushed);
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
            // 拖拽
            m_fOldPoint = UISB_OffsetVaule(pt4self.x);
            m_fOldIndex = m_fIndex;
            break;
        case LongUI::UIScrollBar::PointType::Type_Shaft:
        {
            auto tmpx = (UISB_OffsetVaule(pt4self.x) - BASIC_SIZE);
            auto tmpl = (UISB_OffsetVaule(this->view_size.width) - BASIC_SIZE * 2.f);
            // 设置目标
            this->SetIndex(tmpx / tmpl * m_fMaxIndex);
        }
            break;
        }
    };
    // --------------------     main proc    --------------------
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
    case LongUI::MouseEvent::Event_MouseWheelH:
    {
        // 滚动条
        auto test1 = arg.event == LongUI::MouseEvent::Event_MouseWheelV;
        auto test2 = !!(arg.sys.wParam & MK_SHIFT);
        if ((test1 && test2) == (this->bartype == ScrollBarType::Type_Horizontal)) {
            auto wheel = (float(GET_WHEEL_DELTA_WPARAM(arg.sys.wParam))) / float(WHEEL_DELTA);
            this->SetIndex(m_uiAnimation.end - wheel_step * wheel);
            return true;
        }
        return false;
    }
    case LongUI::MouseEvent::Event_MouseLeave:
        this->set_state(m_lastPointType, LongUI::State_Normal);
        m_pointType = PointType::Type_None;
        m_lastPointType = PointType::Type_None;
        return true;
    case LongUI::MouseEvent::Event_MouseMove:
        on_mouse_move();
        return true;
    case LongUI::MouseEvent::Event_LButtonDown:
        on_lbutton_down();
        return true;
    case LongUI::MouseEvent::Event_LButtonUp:
        this->set_state(m_pointType, LongUI::State_Hover);
        m_bCaptured = false;
        m_pWindow->ReleaseCapture();
        return true;
    case LongUI::MouseEvent::Event_RButtonDown:
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        break;
    case LongUI::MouseEvent::Event_MButtonDown:
        break;
    case LongUI::MouseEvent::Event_MButtonUp:
        break;
    default:
        break;
    }
    return false;
}

// UIScrollBarA:: 重建
auto LongUI::UIScrollBarA::Recreate() noexcept -> HRESULT {
    m_uiArrow1.Recreate();
    m_uiArrow2.Recreate();
    m_uiThumb.Recreate();
    return Super::Recreate();
}

// UIScrollBarA: 初始化时
/*void LongUI::UIScrollBarA::InitMarginalControl(MarginalControl _type) noexcept {
    // 初始化
    Super::InitMarginalControl(_type);
    // 创建几何
    if (this->bartype == ScrollBarType::Type_Horizontal) {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Left]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Right]);
    }
    // 垂直滚动条
    else {
        m_pArrow1Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Top]);
        m_pArrow2Geo = LongUI::SafeAcquire(s_apArrowPathGeometry[this->Arrow_Bottom]);
    }
    assert(m_pArrow1Geo && m_pArrow2Geo);
}
*/

// UIScrollBarA 析构函数
inline LongUI::UIScrollBarA::~UIScrollBarA() noexcept {
    LongUI::SafeRelease(m_pArrow1Geo);
    LongUI::SafeRelease(m_pArrow2Geo);
}

// UIScrollBarA 关闭控件
void  LongUI::UIScrollBarA::cleanup() noexcept {
    delete this;
}

// 设置状态
void LongUI::UIScrollBarA::set_state(PointType _bartype, ControlState state) noexcept {
    // 检查
    Component::Element4Bar* elements[] = { &m_uiArrow1, &m_uiArrow2, &m_uiThumb };
    // 检查
    if (_bartype >= PointType::Type_Arrow1 && _bartype <= PointType::Type_Thumb) {
        auto index = static_cast<uint32_t>(_bartype) - static_cast<uint32_t>(PointType::Type_Arrow1);
        m_pWindow->StartRender(elements[index]->SetBasicState(state), this);
    }
}

// 静态变量
ID2D1PathGeometry* LongUI::UIScrollBarA::
s_apArrowPathGeometry[LongUI::UIScrollBarA::ARROW_SIZE] = { nullptr };

// create 创建
auto WINAPI LongUI::UIScrollBarA::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIScrollBarA* pControl = nullptr;
    switch (type)
    {
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
            ShowHR(hr);
            LongUI::SafeRelease(sink);
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
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Left] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
        // TOP 上箭头
        {
            point_list[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
            point_list[1] = { BASIC_SIZE_MID, BASIC_SIZE_NEAR };
            point_list[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Top]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Top] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
        // RIGHT 右箭头
        {

            point_list[0] = { BASIC_SIZE_MID , BASIC_SIZE_NEAR };
            point_list[1] = { BASIC_SIZE_FAR , BASIC_SIZE_MID };
            point_list[2] = { BASIC_SIZE_MID , BASIC_SIZE_FAR };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Right]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Right] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
        // BOTTOM 下箭头
        {
            point_list[0] = { BASIC_SIZE_NEAR, BASIC_SIZE_MID };
            point_list[1] = { BASIC_SIZE_MID, BASIC_SIZE_FAR };
            point_list[2] = { BASIC_SIZE_FAR, BASIC_SIZE_MID };
            assert(!s_apArrowPathGeometry[UIScrollBarA::Arrow_Bottom]);
            s_apArrowPathGeometry[UIScrollBarA::Arrow_Bottom] = 
                create_geo(point_list, lengthof<uint32_t>(point_list));
        }
    }
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        // 释放资源
        for (auto& geo : s_apArrowPathGeometry) {
            LongUI::SafeRelease(geo);
        }
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIScrollBarA, pControl, type, node);
    }
    return pControl;
}


/*// UIScrollBarB 构造函数
LongUI::UIScrollBarB::UIScrollBarB(UIContainer* cp) noexcept : Super(cp) {

}*/

// UIScrollBarB 创建函数
auto WINAPI LongUI::UIScrollBarB::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    // 分类判断
    UIScrollBarB* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIScrollBarB, pControl, type, node);
    }
    return pControl;
}

// UIScrollBarB: 刷新
void  LongUI::UIScrollBarB::Update() noexcept {

}


// UIScrollBarB 关闭控件
void  LongUI::UIScrollBarB::cleanup() noexcept {
    delete this;
}

                   

// UISlider 背景渲染
void LongUI::UISlider::render_chain_background() const noexcept {
    Super::render_chain_background();
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
}

// UISlider 前景
void LongUI::UISlider::render_chain_foreground() const noexcept {
    // 边框
    m_uiElement.Render(m_rcThumb);
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
    // 父类
    Super::render_chain_foreground();
}


// Render 渲染 
void LongUI::UISlider::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
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
    // 父类刷新
    return Super::Update();
}

// UISlider 构造函数
void LongUI::UISlider::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    m_uiElement.Init(this->check_state(), 0, node);
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
    // init
    m_uiElement.SetBasicState(State_Normal);
    // need twices because of aniamtion
    m_uiElement.SetBasicState(State_Normal);
}


// UISlider::CreateControl 函数
auto LongUI::UISlider::CreateControl(CreateEventType type, pugi::xml_node node) noexcept ->UIControl* {
    UISlider* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UISlider, pControl, type, node);
    }
    return pControl;
}


// 鼠标事件
bool LongUI::UISlider::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
    bool nocontinued = false;
    // 分类
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseLeave:
        // 鼠标移出: 设置UI元素状态
        this->SetControlState(LongUI::State_Normal);
        m_bMouseClickIn = false;
        m_bMouseMoveIn = false;
        nocontinued = true;
        break;
    case  LongUI::MouseEvent::Event_MouseMove:
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
                    this->SetControlState(LongUI::State_Hover);
                    m_bMouseMoveIn = true;
                }
            }
            else {
                // 鼠标移出:
                if (m_bMouseMoveIn) {
                    // 设置UI元素状态
                    this->SetControlState(LongUI::State_Normal);
                    m_bMouseMoveIn = false;
                }
            }
        }
        nocontinued = true;
        break;
    case  LongUI::MouseEvent::Event_LButtonDown:
        // 左键按下
        m_pWindow->SetCapture(this);
        if (IsPointInRect(m_rcThumb, pt4self)){
            m_bMouseClickIn = true;
            m_fClickPosition = this->IsVerticalSlider() ?
                (pt4self.y - m_rcThumb.top) : (pt4self.x - m_rcThumb.left);
            this->SetControlState(LongUI::State_Pushed);
        }
        nocontinued = true;
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 右键按下
        m_bMouseClickIn = false;
        m_pWindow->ReleaseCapture();
        this->SetControlState(LongUI::State_Hover);
        nocontinued = true;
        break;
    }
    // 检查事件
    if (m_fValueOld != m_fValue) {
        m_fValueOld = m_fValue;
        // 调用
        this->call_uievent(m_event, SubEvent::Event_ValueChanged);
        // 刷新
        m_pWindow->Invalidate(this);
    }
    return nocontinued;
}

// recreate 重建
auto LongUI::UISlider::Recreate() noexcept ->HRESULT {
    m_uiElement.Recreate();
    return Super::Recreate();
}

// 添加事件监听器(雾)
bool LongUI::UISlider::uniface_addevent(SubEvent sb, UICallBack&& call) noexcept {
    if (sb == SubEvent::Event_ValueChanged) {
        m_event += std::move(call);
        return true;
    }
    return Super::uniface_addevent(sb, std::move(call));
}

// close this control 关闭控件
void LongUI::UISlider::cleanup() noexcept {
    delete this;
}

                   
                    


// 忙等
LongUINoinline void LongUI::usleep(long usec) noexcept {
    LARGE_INTEGER lFrequency;
    LARGE_INTEGER lEndTime;
    LARGE_INTEGER lCurTime;
    ::QueryPerformanceFrequency(&lFrequency);
    ::QueryPerformanceCounter(&lEndTime);
    lEndTime.QuadPart += (LONGLONG)usec * lFrequency.QuadPart / 1000000;
    do { ::QueryPerformanceCounter(&lCurTime); } while (lCurTime.QuadPart < lEndTime.QuadPart);
}


/// <summary>
/// float4 color ---> 32-bit ARGB uint color
/// 将浮点颜色转换成32位ARGB排列整型
/// </summary>
/// <param name="color">The d2d color</param>
/// <returns>32-bit ARGB 颜色</returns>
LongUINoinline auto LongUI::PackTheColorARGB(D2D1_COLOR_F& IN color) noexcept -> uint32_t {
    // 常量
    constexpr uint32_t ALPHA_SHIFT  = CHAR_BIT * 3;
    constexpr uint32_t RED_SHIFT    = CHAR_BIT * 2;
    constexpr uint32_t GREEN_SHIFT  = CHAR_BIT * 1;
    constexpr uint32_t BLUE_SHIFT   = CHAR_BIT * 0;
    // 写入
    uint32_t colorargb =
        ((uint32_t(color.a * 255.f) & 0xFF) << ALPHA_SHIFT) |
        ((uint32_t(color.r * 255.f) & 0xFF) << RED_SHIFT)   |
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
LongUINoinline auto LongUI::UnpackTheColorARGB(uint32_t IN color32, D2D1_COLOR_F& OUT color4f) noexcept ->void {
    // 位移量
    constexpr uint32_t ALPHA_SHIFT  = CHAR_BIT * 3;
    constexpr uint32_t RED_SHIFT    = CHAR_BIT * 2;
    constexpr uint32_t GREEN_SHIFT  = CHAR_BIT * 1;
    constexpr uint32_t BLUE_SHIFT   = CHAR_BIT * 0;
    // 掩码
    constexpr uint32_t ALPHA_MASK   = 0xFFU << ALPHA_SHIFT;
    constexpr uint32_t RED_MASK     = 0xFFU << RED_SHIFT;
    constexpr uint32_t GREEN_MASK   = 0xFFU << GREEN_SHIFT;
    constexpr uint32_t BLUE_MASK    = 0xFFU << BLUE_SHIFT;
    // 计算
    color4f.r = static_cast<float>((color32 & RED_MASK)   >> RED_SHIFT)   / 255.f;
    color4f.g = static_cast<float>((color32 & GREEN_MASK) >> GREEN_SHIFT) / 255.f;
    color4f.b = static_cast<float>((color32 & BLUE_MASK)  >> BLUE_SHIFT)  / 255.f;
    color4f.a = static_cast<float>((color32 & ALPHA_MASK) >> ALPHA_SHIFT) / 255.f;
}



// Meta 渲染
/// <summary>
/// render the meta 渲染Meta
/// </summary>
/// <param name="target">The render target.</param>
/// <param name="des_rect">The des_rect.</param>
/// <param name="opacity">The opacity.</param>
/// <returns></returns>
void LongUI::Meta::Render(ID2D1DeviceContext* target, const D2D1_RECT_F& des_rect, float opacity) const noexcept {
    // 无需渲染
    if (opacity < 0.001f) return;
    // 无效位图
    if (!this->bitmap) {
        UIManager << DL_Warning << "bitmap->null" << LongUI::endl;
        return;
    }
    switch (this->rule)
    {
    case LongUI::BitmapRenderRule::Rule_Scale:
        __fallthrough;
    default:
    case LongUI::BitmapRenderRule::Rule_ButtonLike:
        // 直接缩放:
        target->DrawBitmap(
            this->bitmap,
            des_rect, opacity,
            static_cast<D2D1_INTERPOLATION_MODE>(this->interpolation),
            this->src_rect,
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
            std::memcpy(clip_rects, des_rects, sizeof(des_rects));
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
    std::memset(&m_dataStorage, 0, sizeof(m_dataStorage));
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
    if (!length) length = std::wcslen(str);
    // 全局
    HGLOBAL hGlobal = nullptr;
    // 申请成功
    auto size = sizeof(wchar_t) * (length + 1);
    if (hGlobal = ::GlobalAlloc(GMEM_FIXED, size)) {
        LPVOID pdest = ::GlobalLock(hGlobal);
        if (pdest) {
            std::memcpy(pdest, str, size);
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
        hr = ::SHCreateStdEnumFmtEtc(static_cast<UINT>(lengthof(rgfmtetc)), rgfmtetc, ppEnumFormatEtc);
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
HRESULT LongUI::CUIDataObject::SetBlob(CLIPFORMAT cf, const void* pvBlob, UINT cbBlob) noexcept {
    void*pv = GlobalAlloc(GPTR, cbBlob);
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


// UIString 设置字符串
void LongUI::CUIString::Set(const wchar_t* str, uint32_t length) noexcept {
    assert(str && "bad argument");
    // 内存不足
    if (!this->m_pString) {
        this->m_pString = m_aDataStatic;
        m_cBufferLength = LongUIStringFixedLength;
        m_aDataStatic[0] = wchar_t(0);
    }
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
    this->copy_string(m_pString, str, length);
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
        // memcpy:__restrict 要求, 手动循环
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
        UIManager << DL_Warning << "copy ctor is not suit for CUIString" << LongUI::endl;
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
        // 警告
        UIManager << DL_Warning << "move ctor is not suit for CUIString" << LongUI::endl;
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
    // memcpy:__restrict 要求, 手动循环
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
    // 初始化数据
    wchar_t buffer[LongUIStringBufferLength]; buffer[0] = 0;
    va_list ap; va_start(ap, format);
    // 格式化字符串
    auto length = std::vswprintf(buffer, LongUIStringBufferLength, format, ap);
    // 发生错误
    if (length < 0) {
        UIManager << DL_Warning 
            << L"std::vswprintf return " << long(length) 
            << L" for out of space or some another error" 
            << LongUI::endl;
        length = LongUIStringBufferLength - 1;
    }
    // 设置
    this->Set(buffer, length);
    // 收尾
    va_end(ap);
}

// CUIString 内存不足
void LongUI::CUIString::OnOOM() noexcept {
    constexpr auto length = 13ui32;
    // 内存
    if (LongUIStringFixedLength > length) {
        this->Set(L"Out of Memory", length);
    }
    else if (LongUIStringFixedLength > 3) {
        this->Set(L"OOM", 3);
    }
    // 显示错误
    UIManager.ShowError(E_OUTOFMEMORY, L"<LongUI::CUIString::OnOOM()>");
}


// += 操作
//const LongUI::CUIString& LongUI::CUIString::operator+=(const wchar_t*);

// CUIAnimation ---------- BEGIN -------------

#define UIAnimation_Template_A      \
    auto v = LongUI::EasingFunction(this->type, this->time / this->duration)
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


// get transformed pointer
LongUINoinline auto LongUI::TransformPointInverse(const D2D1_MATRIX_3X2_F& matrix, const D2D1_POINT_2F& point) noexcept ->D2D1_POINT_2F {
    D2D1_POINT_2F result;
    // x = (bn-dm) / (bc-ad)
    // y = (an-cm) / (ad-bc)
    // a : m_matrix._11
    // b : m_matrix._21
    // c : m_matrix._12
    // d : m_matrix._22
    auto bc_ad = matrix._21 * matrix._12 - matrix._11 * matrix._22;
    auto m = point.x - matrix._31;
    auto n = point.y - matrix._32;
    result.x = (matrix._21*n - matrix._22 * m) / bc_ad;
    result.y = (matrix._12*m - matrix._11 * n) / bc_ad;
    return result;
}

// longui::impl 命名空间
namespace LongUI { namespace impl {
    // 字符串转数字
    template<typename T> LongUINoinline auto atoi(const T* str) noexcept ->int {
        assert(str && "bad argument");
        // 初始化
        bool negative = false; int value = 0; T ch = 0;
        // 遍历
        while (ch = *str) {
            // 空白?
            if (!white_space(ch)) {
                if (ch == '-') {
                    negative = true;
                }
                else if (valid_digit(ch)) {
                    value *= 10;
                    value += ch - static_cast<T>('0');
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
    // 字符串转浮点
    template<typename T> LongUINoinline auto atof(const T* p) noexcept ->float {
        assert(p && "bad argument");
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
            value = value * 10.0f + static_cast<float>(*p - static_cast<T>('0'));
        }
        // 获取小数点或者指数之后的数字(有的话)
        if (*p == '.') {
            float pow10 = 10.0f; ++p;
            while (valid_digit(*p)) {
                value += (*p - static_cast<T>('0')) / pow10;
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
                expon = expon * 10 + (*p - static_cast<T>('0'));
            }
            // float 最大38 double 最大308
            if (expon > 38) expon = 38;
            // 计算比例因数
            while (expon >= 8) { scale *= 1E8f;  expon -= 8; }
            while (expon) { scale *= 10.0f; --expon; }
        }
        // 返回
        float returncoude = (frac ? (value / scale) : (value * scale));
        if (negative) {
            // float
            returncoude = -returncoude;
        }
        return returncoude;
    }
}}

/// <summary>
/// string to float.字符串转浮点, std::atof自己实现版
/// </summary>
/// <param name="p">The string. in const char*</param>
/// <returns></returns>
auto LongUI::AtoF(const char* __restrict p) noexcept -> float {
    if (!p) return 0.0f;
    return impl::atof(p);
}


/// <summary>
/// string to float.字符串转浮点, std::atof自己实现版
/// </summary>
/// <param name="p">The string.in const wchar_t*</param>
/// <returns></returns>
auto LongUI::AtoF(const wchar_t* __restrict p) noexcept -> float {
    if (!p) return 0.0f;
    return impl::atof(p);
}

/// <summary>
/// string to int, 字符串转整型, std::atoi自己实现版
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::AtoI(const char* __restrict str) noexcept -> int {
    if (!str) return 0;
    return impl::atoi(str);
}

/// <summary>
/// string to int, 字符串转整型, std::atoi自己实现版
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::AtoI(const wchar_t* __restrict str) noexcept -> int {
    if (!str) return 0;
    return impl::atoi(str);
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
* (I.event., one byte sequence, two byte... etc.). Remember that sequencs
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
auto LongUI::Base64Encode(IN const uint8_t* __restrict bindata, IN size_t binlen, OUT char* __restrict const base64) noexcept -> char * {
    uint8_t current;
    auto base64_index = base64;
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
auto LongUI::Base64Decode(IN const char * __restrict base64, OUT uint8_t * __restrict bindata) noexcept -> size_t {
    // 二进制长度
    uint8_t* bindata_index = bindata;
    // 主循环
    while (*base64) {
        // 基本转换
        uint8_t a = Base64Datas[base64[0]];  
        uint8_t b = Base64Datas[base64[1]];
        uint8_t c = Base64Datas[base64[2]];  
        uint8_t d = Base64Datas[base64[3]];
        // 第一个二进制数据
        *bindata_index = ((a << 2) & 0xFCui8) | ((b >> 4) & 0x03ui8);
        ++bindata_index;
        if (base64[2] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((b << 4) & 0xF0ui8) | ((c >> 2) & 0x0Fui8);
        ++bindata_index;
        if (base64[3] == '=') break;
        // 第三个二进制数据
        *bindata_index = ((c << 6) & 0xF0ui8) | ((d >> 0) & 0x3Fui8);
        ++bindata_index;
        base64 += 4;
    }
    return bindata_index - bindata;
}

// UTF-16 to UTF-8
// Return: UTF-8 string length, 0 maybe error
auto LongUI::UTF16toUTF8(const char16_t* __restrict pUTF16String, char* __restrict pUTF8String) noexcept ->uint32_t {
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
auto LongUI::UTF8toUTF16(const char* __restrict pUTF8String, char16_t* __restrict pUTF16String) noexcept -> uint32_t {
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

// --------------  CUIConsole ------------
// CUIConsole 构造函数
LongUI::CUIConsole::CUIConsole() noexcept {
    //::InitializeCriticalSection(&m_cs);  
    m_name[0] = L'\0';
    { if (m_hConsole != INVALID_HANDLE_VALUE) this->Close(); }
}

// CUIConsole 析构函数
LongUI::CUIConsole::~CUIConsole() noexcept {
    this->Close();
    // 关闭
    if (m_hConsole != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_hConsole);
        m_hConsole = INVALID_HANDLE_VALUE;
    }
    //::DeleteCriticalSection(&m_cs);
}

// CUIConsole 关闭
long LongUI::CUIConsole::Close() noexcept {
    if (!(*this))
        return -1;
    else
        return ::DisconnectNamedPipe(m_hConsole);
}

// CUIConsole 输出
long LongUI::CUIConsole::Output(const wchar_t * str, bool flush, size_t len) noexcept {
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
        std::memcpy(m_buffer + m_length, str, len * sizeof(wchar_t));
        m_length += len;
        str = nullptr;
        // 不用flush
        if (!flush) return 0;
    }
    DWORD dwWritten = DWORD(-1);
    // 写入
    auto safe_write_file = [this, &dwWritten]() {
        return ::WriteFile(
            m_hConsole, m_buffer, 
            static_cast<uint32_t>(m_length * sizeof(wchar_t)), 
            &dwWritten, nullptr
            );
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
    std::wcscpy(m_name, LR"(\\.\pipe\)");
    wchar_t logger_name_buffer[128];
    // 未给logger?
    if (!config.logger_name) {
        static float s_times = 1.f;
        std::swprintf(
            logger_name_buffer, lengthof(logger_name_buffer),
            L"logger_%7.5f",
            float(::GetTickCount()) / float(1000 * 60 * 60) *
            (float(std::rand()) / float(RAND_MAX)) * s_times
            );
        config.logger_name = logger_name_buffer;
        ++s_times;
    }
    std::wcscat(m_name, config.logger_name);
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
    if (!bRet) {
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
    // 连接
    BOOL bConnected = ::ConnectNamedPipe(m_hConsole, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
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
    uint32_t len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
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
        len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
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
        len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
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
    if (config.buffer_size_x != -1 && config.buffer_size_y != -1) {
        std::swprintf(buffer, lengthof(buffer), L"BUFFER-SIZE: %dx%d\r\n", config.buffer_size_x, config.buffer_size_y);
        len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
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
    if (false) {
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
    // 关闭进程句柄
    if(bRet) {
        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
    }
    return 0;
}



// --------------  CUIDefaultConfigure ------------
#ifdef LONGUI_WITH_DEFAULT_CONFIG

// longui
namespace LongUI {
    // 创建XML资源读取器
    auto CreateResourceLoaderForXML(CUIManager& manager, const char* xml) noexcept ->IUIResourceLoader*;
}

// 创建接口
auto LongUI::CUIDefaultConfigure::CreateInterface(const IID & riid, void** ppvObject) noexcept -> HRESULT {
    // 资源读取器
    if (riid == LongUI::GetIID<LongUI::IUIResourceLoader>()) {
        *ppvObject = LongUI::CreateResourceLoaderForXML(m_manager, this->resource);
    }
    // 脚本
    else if (riid == LongUI::GetIID<LongUI::IUIScript>()) {

    }
    // 字体集
    else if (riid == LongUI::GetIID<IDWriteFontCollection>()) {

    }
    // 检查
    return (*ppvObject) ? S_OK : E_NOINTERFACE;
}

auto LongUI::CUIDefaultConfigure::ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept -> size_t {
    UNREFERENCED_PARAMETER(adapters);
    // 核显卡优先 
#ifdef LONGUI_NUCLEAR_FIRST
    for (size_t i = 0; i < length; ++i) {
        DXGI_ADAPTER_DESC1& desc = adapters[i];
        if (!std::wcsncmp(L"NVIDIA", desc.Description, 6))
            return i;
    }
#endif
    return length;
}

// CUIDefaultConfigure 显示错误信息
auto LongUI::CUIDefaultConfigure::ShowError(const wchar_t * str_a, const wchar_t* str_b) noexcept -> void {
    assert(str_a && "bad argument!");
    if (!str_b) str_b = L"Error!";
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
            std::fwrite(buffer, sizeof(wchar_t), std::wcslen(buffer), m_pLogFile);
        }
        std::fwrite(string, sizeof(wchar_t), std::wcslen(string), m_pLogFile);

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

// ------------------- MMFVideo -----------------------
#ifdef LONGUI_WITH_MMFVIDEO
// MMFVideo 事件通知
HRESULT LongUI::Component::MMFVideo::EventNotify(DWORD event, DWORD_PTR param1, DWORD param2) noexcept {
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


// MMFVideo 初始化
auto LongUI::Component::MMFVideo::Init() noexcept ->HRESULT {
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
    LongUI::SafeRelease(attributes);
    return hr;
}

// MMFVideo: 重建
auto LongUI::Component::MMFVideo::Recreate() noexcept ->HRESULT {
    LongUI::SafeRelease(m_pTargetSurface);
    LongUI::SafeRelease(m_pDrawSurface);
    return this->recreate_surface();
}

// MMFVideo: 渲染
void LongUI::Component::MMFVideo::Render(D2D1_RECT_F* dst) const noexcept {
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

// Component::MMFVideo 构造函数
LongUI::Component::MMFVideo::MMFVideo() noexcept {
    force_cast(dst_rect) = { 0 };
}

// Component::MMFVideo 析构函数
LongUI::Component::MMFVideo::~MMFVideo() noexcept {
    if (m_pMediaEngine) {
        m_pMediaEngine->Shutdown();
    }
    LongUI::SafeRelease(m_pMediaEngine);
    LongUI::SafeRelease(m_pEngineEx);
    LongUI::SafeRelease(m_pTargetSurface);
    LongUI::SafeRelease(m_pSharedSurface);
    LongUI::SafeRelease(m_pDrawSurface);
}

// 重建表面
auto LongUI::Component::MMFVideo::recreate_surface() noexcept ->HRESULT {
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
            LongUI::SafeRelease(m_pTargetSurface);
            LongUI::SafeRelease(m_pSharedSurface);
            LongUI::SafeRelease(m_pDrawSurface);
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
            LongUI::SafeRelease(surface);
        }
    }
    return hr;
}

#endif



// -----------------------------



// π
constexpr float EZ_PI = 3.1415296F;
// 二分之一π
constexpr float EZ_PI_2 = 1.5707963F;

// 反弹渐出
float inline BounceEaseOut(float p) noexcept {
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
float LongUI::EasingFunction(AnimationType type, float p) noexcept {
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
        float f = p - 1.f;
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
            float f = (2.f * p) - 2.f;
            return 0.5f * f * f * f + 1.f;
        }
    case LongUI::AnimationType::Type_QuarticEaseIn:
        // 四次渐入     f(x) = x^4
    {
        float f = p * p;
        return f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseOut:
        // 四次渐出     f(x) = 1 - (x - 1)^4
    {
        float f = (p - 1.f); f *= f;
        return 1.f - f * f;
    }
    case LongUI::AnimationType::Type_QuarticEaseInOut:
        // 四次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^4)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-2)^4 - 2)
        if (p < 0.5f) {
            float f = p * p;
            return 8.f * f * f;
        }
        else {
            float f = (p - 1.f); f *= f;
            return 1.f - 8.f * f * f;
        }
    case LongUI::AnimationType::Type_QuinticEaseIn:
        // 五次渐入     f(x) = x^5
    {
        float f = p * p;
        return f * f * p;
    }
    case LongUI::AnimationType::Type_QuinticEaseOut:
        // 五次渐出     f(x) = (x - 1)^5 + 1
    {
        float f = (p - 1.f);
        return f * f * f * f * f + 1.f;
    }
    case LongUI::AnimationType::Type_QuinticEaseInOut:
        // 五次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^5) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^5 + 2)
        if (p < 0.5) {
            float f = p * p;
            return 16.f * f * f * p;
        }
        else {
            float f = ((2.f * p) - 2.f);
            return  f * f * f * f * f * 0.5f + 1.f;
        }
    case LongUI::AnimationType::Type_SineEaseIn:
        // 正弦渐入     
        return std::sin((p - 1.f) * EZ_PI_2) + 1.f;
    case LongUI::AnimationType::Type_SineEaseOut:
        // 正弦渐出     
        return std::sin(p * EZ_PI_2);
    case LongUI::AnimationType::Type_SineEaseInOut:
        // 正弦出入     
        return 0.5f * (1.f - std::cos(p * EZ_PI));
    case LongUI::AnimationType::Type_CircularEaseIn:
        // 四象圆弧
        return 1.f - std::sqrt(1.f - (p * p));
    case LongUI::AnimationType::Type_CircularEaseOut:
        // 二象圆弧
        return std::sqrt((2.f - p) * p);
    case LongUI::AnimationType::Type_CircularEaseInOut:
        // 圆弧出入
        if (p < 0.5f) {
            return 0.5f * (1.f - std::sqrt(1.f - 4.f * (p * p)));
        }
        else {
            return 0.5f * (std::sqrt(-((2.f * p) - 3.f) * ((2.f * p) - 1.f)) + 1.f);
        }
    case LongUI::AnimationType::Type_ExponentialEaseIn:
        // 指数渐入     f(x) = 2^(10(x - 1))
        return (p == 0.f) ? (p) : (std::pow(2.f, 10.f * (p - 1.f)));
    case LongUI::AnimationType::Type_ExponentialEaseOut:
        // 指数渐出     f(x) =  -2^(-10x) + 1
        return (p == 1.f) ? (p) : (1.f - std::powf(2.f, -10.f * p));
    case LongUI::AnimationType::Type_ExponentialEaseInOut:
        // 指数出入
        // [0,0.5)      f(x) = (1/2)2^(10(2x - 1)) 
        // [0.5,1.f]    f(x) = -(1/2)*2^(-10(2x - 1))) + 1 
        if (p == 0.0f || p == 1.0f) return p;
        if (p < 0.5f) {
            return 0.5f * std::powf(2.f, (20.f * p) - 10.f);
        }
        else {
            return -0.5f * std::powf(2.f, (-20.f * p) + 1.f) + 1.f;
        }
    case LongUI::AnimationType::Type_ElasticEaseIn:
        // 弹性渐入
        return std::sin(13.f * EZ_PI_2 * p) * std::pow(2.f, 10.f * (p - 1.f));
    case LongUI::AnimationType::Type_ElasticEaseOut:
        // 弹性渐出
        return std::sin(-13.f * EZ_PI_2 * (p + 1.f)) * std::powf(2.f, -10.f * p) + 1.f;
    case LongUI::AnimationType::Type_ElasticEaseInOut:
        // 弹性出入
        if (p < 0.5f) {
            return 0.5f * std::sin(13.f * EZ_PI_2 * (2.f * p)) * std::pow(2.f, 10.f * ((2.f * p) - 1.f));
        }
        else {
            return 0.5f * (std::sin(-13.f * EZ_PI_2 * ((2.f * p - 1.f) + 1.f)) * std::pow(2.f, -10.f * (2.f * p - 1.f)) + 2.f);
        }
    case LongUI::AnimationType::Type_BackEaseIn:
        // 回退渐入
        return  p * p * p - p * std::sin(p * EZ_PI);
    case LongUI::AnimationType::Type_BackEaseOut:
        // 回退渐出
    {
        float f = (1.f - p);
        return 1.f - (f * f * f - f * std::sin(f * EZ_PI));
    }
    case LongUI::AnimationType::Type_BackEaseInOut:
        // 回退出入
        if (p < 0.5f) {
            float f = 2.f * p;
            return 0.5f * (f * f * f - f * std::sin(f * EZ_PI));
        }
        else {
            float f = (1.f - (2 * p - 1.f));
            return 0.5f * (1.f - (f * f * f - f * std::sin(f * EZ_PI))) + 0.5f;
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

// longui namespace
namespace LongUI {
    // BKDR 哈希
    auto BKDRHash(const char* str) noexcept -> uint32_t {
        constexpr uint32_t seed = 131;
        uint32_t code = 0;
        auto p = reinterpret_cast<const unsigned char*>(str);
        while (*p) code = code * seed + (*p++);
        return code;
    }
    // BKDR 哈希
    auto BKDRHash(const wchar_t* str) noexcept -> uint32_t {
        constexpr uint32_t seed = 131;
        uint32_t code = 0;
        while (*str) code = code * seed + (*str++);
        return code;
    }
}
                   

// 双击
LongUINoinline bool LongUI::Helper::DoubleClick::Click(const D2D1_POINT_2F& pt) noexcept {
    // 懒得解释了
    auto now = ::timeGetTime();
    bool result = ((now - last) <= time) && pt.x == this->ptx && pt.y == this->pty ;
    last = result ? 0ui32 : now;
    this->ptx = pt.x;
    this->pty = pt.y;
    return result;
}

// longui::impl 命名空间
namespace LongUI { namespace impl { 
    // 申请全局字符串
    template<typename T> 
    inline auto alloc_global_string(const T* src, size_t len) noexcept {
        // 申请
        auto global = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(T)*(len + 1));
        // 有效?
        if (global) {
            // 锁住!
            auto* des = reinterpret_cast<T*>(::GlobalLock(global));
            // 申请全局内存成功
            if (des) {
                // 复制
                std::memcpy(des, src, sizeof(T)*(len));
                // null结尾
                des[len] = 0;
                // 解锁
                ::GlobalUnlock(global);
            }
#ifdef _DEBUG
            else {
                UIManager << DL_Error
                    << L" GlobalLock --> Failed"
                    << LongUI::endl;
            }
#endif
        }
#ifdef _DEBUG
        else {
            UIManager << DL_Error
                << L" GlobalAlloc --> Failed, try alloc from"
                << Formated(L"%p in %zu bytes", src, len)
                << LongUI::endl;
        }
#endif
        return global;
    }
}}

// 申请全局字符串
LongUINoinline auto LongUI::Helper::GlobalAllocString(const wchar_t* src, size_t len) noexcept ->HGLOBAL {
    return impl::alloc_global_string(src, len);
}

// 申请全局字符串
LongUINoinline auto LongUI::Helper::GlobalAllocString(const char* src, size_t len) noexcept ->HGLOBAL {
    return impl::alloc_global_string(src, len);
}

// 查找多个文件
LongUINoinline auto LongUI::Helper::FindFilesToBuffer(
    wchar_t* buf, size_t buf_len, 
    const wchar_t* folder,
    const wchar_t* name
    ) noexcept ->wchar_t* {
    // 初始化
    WIN32_FIND_DATAW fileinfo;
#ifdef _DEBUG
    std::memset(&fileinfo, 0, sizeof(fileinfo));
#endif
    HANDLE hFile = INVALID_HANDLE_VALUE;
    ::SetLastError(0);
    {
        wchar_t path[MAX_PATH * 2]; path[0] = 0;
        std::swprintf(path, lengthof(path), L"%ls\\%ls", folder, name);
        hFile = ::FindFirstFileW(path, &fileinfo);
    }
    // 有效
    if (hFile != INVALID_HANDLE_VALUE) {
        // 清空
        DWORD errorcode = ::GetLastError();
        // 遍历文件
        while (errorcode != ERROR_NO_MORE_FILES) {
            // 跳过.开头
            if (fileinfo.cFileName[0] != '.') {
                // 写入
                auto code = std::swprintf(buf, buf_len - 1, L"%ls\\%ls", folder, fileinfo.cFileName);
                // 缓存不足
                if (code < 0) break;
                // 写入
                buf += code + 1; buf[0] = 0; buf_len -= code + 1;
            }
            // 推进
            ::FindNextFileW(hFile, &fileinfo);
            // 检查
            errorcode = ::GetLastError();
        }
    }
    ::FindClose(hFile);
    return buf;
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
        char ch = *itr;
        // 段结束?
        if (ch == ',' || !ch) {
            assert(word_begin && "bad string");
            // 有效
            if (word_begin && data) {
                CC& cc = data[count - 1];
                size_t length = size_t(itr - word_begin);
                assert(length < lengthof(temp_buffer));
                std::memcpy(temp_buffer, word_begin, length);
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

// longui::impl 命名空间
namespace LongUI { namespace impl {
    template<size_t C> struct make_units_helper { };
    // 创建单元帮助器
    template<> struct make_units_helper<2> { 
        // 创建单元
        template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
        static LongUIInline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE* units, uint32_t size) noexcept {
            // 参数检查
            assert(str && units && size && "bad arguments");
            // 数据
            CHAR_TYPE buf[LongUIStringBufferLength]; auto itr = buf;
            // 遍历
            while (size) {
                auto ch = (*str);
                // 获取到了分隔符号
                if (ch == static_cast<CHAR_TYPE>(SEPARATOR) || ch == 0) {
                    *itr = 0;
                    caster(units++, buf);
                    itr = buf; --size;
                    if (ch == 0) {
                        assert(size == 0 && "bad string given!");
                        break;
                    }
                }
                // 继续复制
                else {
                    *itr = ch;
                    ++itr;
                }
                ++str;
            }
        }
    };
    // 创建单元帮助器
    template<> struct make_units_helper<3> { 
        // 创建单元
        template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
        static LongUIInline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE* units, uint32_t size) noexcept {
            // 参数检查
            assert(str && units && size && "bad arguments");
            // 数据
            auto old = str;
            // 遍历
            while (size) {
                // 获取到了分隔符号
                auto ch = (*str);
                if (ch == static_cast<CHAR_TYPE>(SEPARATOR) || ch == 0) {
                    caster(units++, old, str);
                    old = str + 1; --size;
                    if (ch == 0) {
                        assert(size == 0 && "bad string given!");
                        break;
                    }
                }
                ++str;
            }
        }
    };
    // 创建单元
    template<char32_t SEPARATOR, typename CHAR_TYPE, typename OUT_TYPE, typename Lam>
    inline void make_units(Lam caster, const CHAR_TYPE* str, OUT_TYPE units[], uint32_t size) noexcept {
        using caster_type = Helper::type_helper<Lam>;
        return make_units_helper<caster_type::arity>::make_units<SEPARATOR>(caster, str, units, size);
    }
}}

// 命名空间
namespace LongUI { namespace Helper {
    // 创建浮点
    LongUINoinline bool MakeFloats(const char* str, float fary[], uint32_t size) noexcept {
        // 检查字符串
        if (!str || !*str) return false;
        impl::make_units<','>([](float* out, const char* begin, const char* end) noexcept {
            auto len = static_cast<size_t>(end - begin);
            char buf[128]; assert(len < lengthof(buf));
            std::memcpy(buf, begin, len); len[buf] = 0;
            *out = LongUI::AtoF(buf);
        }, str, fary, size);
        return true;
    }
    // 创建整数
    LongUINoinline bool MakeInts(const char* str, int iary[], uint32_t size) noexcept {
        // 检查字符串
        if (!str || !*str) return false;
        impl::make_units<','>([](int* out, const char* begin, const char* end) noexcept {
            auto len = static_cast<size_t>(end - begin);
            char buf[128]; assert(len < lengthof(buf));
            std::memcpy(buf, begin, len); len[buf] = 0;
            *out = LongUI::AtoI(buf);
        }, str, iary, size);
        return true;
    }
    // 颜色属性名字符串集
    const char* const COLOR_BUTTON[] = {
        "disabledcolor", "normalcolor", "hovercolor", "pushedcolor"
    };
    /// <summary>
    ///  创建基于状态的颜色数组
    /// </summary>
    /// <param name="node">pugixml 节点</param>
    /// <param name="prefix">颜色属性前缀</param>
    /// <param name="color">颜色数组</param>
    /// <returns></returns>
    LongUINoinline bool MakeStateBasedColor(pugi::xml_node node, const char * prefix, D2D1_COLOR_F color[4]) noexcept {
        // 初始值
        if (color->a < 0.0f || color->a > 1.f) {
            color[State_Disabled]   = D2D1::ColorF(0xDEDEDEDEui32);
            color[State_Normal]     = D2D1::ColorF(0xCDCDCDCDui32);
            color[State_Hover]      = D2D1::ColorF(0xA9A9A9A9ui32);
            color[State_Pushed]     = D2D1::ColorF(0x78787878ui32);
        }
        bool rc = false;
        // 循环设置
        for (int i = 0; i < STATE_COUNT; ++i) {
            rc = rc | Helper::MakeColor(Helper::XMLGetValue(node, COLOR_BUTTON[i], prefix), color[i]);
        }
        return rc;
    }
    /// <summary>
    /// 创建meta组
    /// </summary>
    /// <param name="node">xml node</param>
    /// <param name="prefix">属性名前缀</param>
    /// <param name="fary">float array of meta id</param>
    /// <param name="count">count of it</param>
    /// <returns>成功设置则返回true, 没有或者错误返回false</returns>
    bool MakeMetaGroup(pugi::xml_node node, const char* prefix, uint16_t fary[], uint32_t count) noexcept {
        // 检查
        constexpr int BUFFER_COUNT = 64; float tmp[BUFFER_COUNT];
        assert(count < BUFFER_COUNT && "out of buffer length");
        // 初始化
        std::memset(tmp, 0, sizeof(tmp));
        // 计算结果
        auto result = Helper::MakeFloats(Helper::XMLGetValue(node, "metagroup", prefix), tmp, count);
        // 转换数据
        for (uint32_t i = 0; i < count; ++i) fary[i] = static_cast<uint16_t>(tmp[i]);
        // 返回结果
        return result;
    }
}}

// 16进制
unsigned int LongUI::Hex2Int(char c) noexcept {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    else return c - '0';
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
    auto length = LongUI::UTF8toWideChar(data, buffer);
    buffer[length] = L'\0';
    // 设置字符串
    str.Set(buffer, length);
    return true;
}


// longui::helper
LONGUI_NAMESPACE_BEGIN namespace Helper {
    // 边框
    const char* const BORDER_COLOR_ATTR[] = {
        "disabledbordercolor", "normalbordercolor",
        "hoverbordercolor",  "pushedbordercolor",
    };
}
LONGUI_NAMESPACE_END

// 设置边框颜色
bool LongUI::Helper::SetBorderColor(pugi::xml_node node, D2D1_COLOR_F color[STATE_COUNT]) noexcept {
    // 边框颜色
    color[State_Disabled]   = D2D1::ColorF(0xBFBFBFui32);
    color[State_Normal]     = D2D1::ColorF(0xACACACui32);
    color[State_Hover]      = D2D1::ColorF(0x7EB4EAui32);
    color[State_Pushed]     = D2D1::ColorF(0x569DE5ui32);
    // 检查
    if (node) {
        for (auto i = 0u; i < STATE_COUNT; ++i) {
            Helper::MakeColor(node.attribute(BORDER_COLOR_ATTR[i]).value(), color[i]);
        }
    }
    return true;
}



// --------------------------------------------------------------------------------------------------------
// longui::helper name space
namespace LongUI { namespace Helper {
    // 获取XML值
    auto XMLGetValue(pugi::xml_node node, const char* att, const char* pfx) noexcept -> const char* {
        if (!node) return nullptr;
        assert(att && "bad argument");
        char buffer[LongUIStringBufferLength];
        // 前缀有效?
        if (pfx) {
            std::strcpy(buffer, pfx);
            std::strcat(buffer, att);
            att = buffer;
        }
        return node.attribute(att).value();
    }
    // 解析字符串数据作为枚举值
    auto GetEnumFromString(const char* value, const GetEnumProperties& prop) noexcept ->uint32_t {
        // 首个为数字?
        auto first_digital = [](const char* str) noexcept {
            // 遍历
            while (*str) {
                // 空白: 跳过
                if (white_space(*str))  ++str;
                // 数字: true
                else if (valid_digit(*str))  return true;
                // 其他: false
                else  break;
            }
            return false;
        };
        // 有效
        if (value && *value) {
            // 数字?
            if (first_digital(value)) {
                return uint32_t(LongUI::AtoI(value));
            }
            // 遍历
            for (size_t i = 0; i < prop.values_length; ++i) {
                if (!std::strcmp(value, prop.values_list[i])) {
                    return uint32_t(i);
                }
            }
            // 失败: 给予警告
            UIManager << DL_Warning
                << L"Bad matched for: "
                << value
                << LongUI::endl;
        }
        // 匹配无效
        return prop.bad_match;
    }
    // 帮助器 GetEnumFromString
    template<typename T, typename Ary> 
    LongUIInline auto GetEnumFromStringHelper(const char* value, T bad_match, const Ary& ary) noexcept {
        // 设置
        GetEnumProperties prop;
        prop.values_list = ary;
        prop.values_length = lengthof<uint32_t>(ary);
        prop.bad_match = static_cast<uint32_t>(bad_match);
        // 调用
        return static_cast<T>(GetEnumFromString(value, prop));
    }
    // 动画类型属性值列表
    const char* const cg_listAnimationType[] = {
        "linear",
        "quadraticim",    "quadraticout",   "quadraticinout",
        "cubicin",        "cubicout",       "cubicoinout",
        "quarticin",      "quarticout",     "quarticinout",
        "quinticcin",     "quinticcout",    "quinticinout",
        "sincin",         "sincout",        "sininout",
        "circularcin",    "circularcout",   "circularinout",
        "exponentiacin",  "exponentiaout",  "exponentiainout",
        "elasticin",      "elasticout",     "elasticinout",
        "backin",         "backout",        "backinout",
        "bouncein",       "bounceout",      "bounceinout",
    };
    // 位图渲染模式 属性值列表
    const char* const cg_listBitmapRenderRule[] = {
        "scale", "button",
    };
    // 渲染模式 属性值列表
    const char* const cg_listRenderRule[] = {
        "scale", "button",
    };
    // 富文本类型 属性值列表
    const char* const cg_listRichType[] = {
        "none", "core", "xml", "custom",
    };
    // D2D 插值模式 属性值列表
    const char* const cg_listInterpolationMode[] = {
        "neighbor", "linear",       "cubic",
        "mslinear", "anisotropic",  "highcubic",
    };
    // D2D 扩展模式 属性值列表
    const char* const cg_listExtendMode[] = {
        "clamp", "wrap", "mirror",
    };
    // D2D 文本抗锯齿模式 属性值列表
    const char* const cg_listTextAntialiasMode[] = {
        "default",  "cleartype",  "grayscale", "aliased",
    };
    // DWrite 字体风格 属性值列表
    const char* const cg_listFontStyle[] = {
        "normal", "oblique", "italic",
    };
    // DWrite 字体拉伸 属性值列表
    const char* const cg_listFontStretch[] = {
        "undefined",
        "ultracondensed",  "extracondensed",  "condensed",
        "semicondensed",   "normal",          "semiexpanded",
        "expanded",        "extraexpanded",   "ultraexpanded",
    };
    // DWrite 排列方向 属性值列表
    const char* const cg_listFlowDirection[] = {
        "top2bottom",  "bottom2top",  "left2right",  "right2left",
    };
    // DWrite 阅读方向 属性值列表
    const char* const cg_listReadingDirection[] = {
        "left2right",  "right2left",  "top2bottom",  "bottom2top",
    };
    // DWrite 换行方式 属性值列表
    const char* const cg_listWordWrapping[] = {
        "wrap", "nowrap",  "break",  "word",  "character",
    };
    // DWrite 段落对齐 属性值列表
    const char* const cg_listParagraphAlignment[] = {
        "top",  "bottom",  "middle",
    };
    // DWrite 文本对齐 属性值列表
    const char* const cg_listTextAlignment[] = {
        "left",  "right",  "center",  "justify",
    };
    // 复选框状态
    const char* const cg_listCheckBoxState[] = {
        "checked", "indeterminate", "unchecked"
    };
    // 获取动画类型
    LongUINoinline auto GetEnumFromString(const char* value, AnimationType bad_match) noexcept ->AnimationType {
        return GetEnumFromStringHelper(value, bad_match, cg_listAnimationType);
    }
    // 获取插值模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_INTERPOLATION_MODE bad_match) noexcept ->D2D1_INTERPOLATION_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listInterpolationMode);
    }
    // 获取扩展模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_EXTEND_MODE bad_match) noexcept ->D2D1_EXTEND_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listExtendMode);
    }
    // 获取位图渲染规则
    LongUINoinline auto GetEnumFromString(const char* value, BitmapRenderRule bad_match) noexcept ->BitmapRenderRule {
        return GetEnumFromStringHelper(value, bad_match, cg_listBitmapRenderRule);
    }
    // 获取富文本类型
    LongUINoinline auto GetEnumFromString(const char* value, RichType bad_match) noexcept ->RichType {
        return GetEnumFromStringHelper(value, bad_match, cg_listRichType);
    }
    // 获取字体风格
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_FONT_STYLE bad_match) noexcept ->DWRITE_FONT_STYLE {
        return GetEnumFromStringHelper(value, bad_match, cg_listFontStyle);
    }
    // 获取字体拉伸
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_FONT_STRETCH bad_match) noexcept ->DWRITE_FONT_STRETCH {
        return GetEnumFromStringHelper(value, bad_match, cg_listFontStretch);
    }
    // 获取排列方向
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_FLOW_DIRECTION bad_match) noexcept ->DWRITE_FLOW_DIRECTION {
        return GetEnumFromStringHelper(value, bad_match, cg_listFlowDirection);
    }
    // 获取阅读方向
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_READING_DIRECTION bad_match) noexcept ->DWRITE_READING_DIRECTION {
        return GetEnumFromStringHelper(value, bad_match, cg_listReadingDirection);
    }
    // 获取换行方式
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_WORD_WRAPPING bad_match) noexcept ->DWRITE_WORD_WRAPPING {
        return GetEnumFromStringHelper(value, bad_match, cg_listWordWrapping);
    }
    // 获取段落对齐方式
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_PARAGRAPH_ALIGNMENT bad_match) noexcept ->DWRITE_PARAGRAPH_ALIGNMENT {
        return GetEnumFromStringHelper(value, bad_match, cg_listParagraphAlignment);
    }
    // 获取文本对齐方式
    LongUINoinline auto GetEnumFromString(const char* value, DWRITE_TEXT_ALIGNMENT bad_match) noexcept ->DWRITE_TEXT_ALIGNMENT {
        return GetEnumFromStringHelper(value, bad_match, cg_listTextAlignment);
    }
    // 获取文本抗锯齿模式
    LongUINoinline auto GetEnumFromString(const char* value, D2D1_TEXT_ANTIALIAS_MODE bad_match) noexcept ->D2D1_TEXT_ANTIALIAS_MODE {
        return GetEnumFromStringHelper(value, bad_match, cg_listTextAntialiasMode);
    }
    // 获取复选框状态
    LongUINoinline auto GetEnumFromString(const char* value, CheckBoxState bad_match) noexcept ->CheckBoxState {
        return GetEnumFromStringHelper(value, bad_match, cg_listCheckBoxState);
    }
}}

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
        DX::Matrix3x2F::Scale(height, height) *
        DX::Matrix3x2F::Translation(rect.left, rect.top) *
        matrix
        );
    // 填写
    target->FillRectangle(unit_rect, brush);
    // 恢复
    target->SetTransform(&matrix);
}


                   

// longui::impl 命名空间
namespace LongUI { namespace impl {
    // 字体文件枚举
    class LongUIFontFileEnumerator final : public Helper::ComStatic<
        Helper::QiList<IDWriteFontFileEnumerator >> {
    public:
        // 获取当前字体文件
        HRESULT STDMETHODCALLTYPE GetCurrentFontFile(IDWriteFontFile **ppFontFile) noexcept override {
            if (!ppFontFile) return E_INVALIDARG;
            if (!m_pFilePath || !m_pFactory)  return E_FAIL;
            *ppFontFile = LongUI::SafeAcquire(m_pCurFontFie);
            return m_pCurFontFie ? S_OK : E_FAIL;
        }
        // 移动到下一个文件
        HRESULT STDMETHODCALLTYPE MoveNext(BOOL *pHasCurrentFile) noexcept override {
            if (!pHasCurrentFile)return E_INVALIDARG;
            if (!m_pFilePath || !m_pFactory) return E_FAIL;
            HRESULT hr = S_OK;
            if (*pHasCurrentFile = *m_pFilePathNow) {
                LongUI::SafeRelease(m_pCurFontFie);
                hr = m_pFactory->CreateFontFileReference(m_pFilePathNow, nullptr, &m_pCurFontFie);
                longui_debug_hr(hr, L" m_pFactory->CreateFontFileReference faild");
                if (*pHasCurrentFile = SUCCEEDED(hr)) {
                    m_pFilePathNow += std::wcslen(m_pFilePathNow);
                    ++m_pFilePathNow;
                }
            }
            return hr;
        }
    public:
        // 构造函数
        LongUIFontFileEnumerator(IDWriteFactory* f) :m_pFactory(LongUI::SafeAcquire(f)) {}
        // 析构函数
        ~LongUIFontFileEnumerator() { LongUI::SafeRelease(m_pCurFontFie); LongUI::SafeRelease(m_pFactory); }
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
            const void*collectionKey,
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
}}

// 创建LongUI的字体集: 本函数会进行I/O, 所以程序开始调用一次即可
auto LongUI::DX::CreateFontCollection(
    const wchar_t * filename, 
    const wchar_t * folder,
    bool include_system
    ) noexcept -> IDWriteFontCollection* {
    IDWriteFontCollection* collection = nullptr;
    constexpr size_t buffer_length = 256 * 256 * 16;
    // 申请足够的空间
    wchar_t* const buffer = LongUI::NormalAllocT<wchar_t>(buffer_length);
    // 内存不足
    if (!buffer) return collection;
    // 初始化
    buffer[0] = 0; auto itr = buffer;
    // 本地路径
    {
        itr = Helper::FindFilesToBuffer(itr, buffer_length, folder, filename);
    }
    // 系统路径
    if (include_system && itr != buffer) {
        wchar_t winpath[MAX_PATH];
        winpath[0] = 0;
        auto len = ::GetWindowsDirectoryW(winpath, lengthof<uint32_t>(winpath));
        assert(len && len < lengthof(winpath) && "buffer to small");
        std::wcscpy(winpath + len, L"\\Fonts");
        itr = Helper::FindFilesToBuffer(itr, buffer_length - size_t(itr - buffer), winpath, L"*.*tf");
        itr = Helper::FindFilesToBuffer(itr, buffer_length - size_t(itr - buffer), winpath, L"*.fon");
    }
    // 当存在符合标准的文件时
    if (itr != buffer) {
        auto hr = S_OK;
        impl::LongUIFontCollectionLoader loader;
        hr = UIManager_DWriteFactory->RegisterFontCollectionLoader(&loader);
        longui_debug_hr(hr, L"UIManager_DWriteFactory->RegisterFontCollectionLoader faild");
        hr = UIManager_DWriteFactory->CreateCustomFontCollection(
            &loader,
            buffer,
            static_cast<uint32_t>(reinterpret_cast<uint8_t*>(itr + 1) - reinterpret_cast<uint8_t*>(buffer)),
            &collection
            );
        longui_debug_hr(hr, L" UIManager_DWriteFactory->CreateCustomFontCollection faild");
        hr = UIManager_DWriteFactory->UnregisterFontCollectionLoader(&loader);
        longui_debug_hr(hr, L" UIManager_DWriteFactory->UnregisterFontCollectionLoader faild");
    }
    LongUI::NormalFree(buffer);
    return collection;
}

// 创建文本格式
auto LongUI::DX::CreateTextFormat(const TextFormatProperties& prop, IDWriteTextFormat** OUT fmt) noexcept -> HRESULT {
    // 参数检查
    assert(fmt && "bad argment"); if (!fmt) return E_INVALIDARG;
#ifdef _DEBUG
    if (*fmt) {
        UIManager << DL_Warning
            << L"pointer 'fmt' pointed a non-nullptr, check it please."
            << endl;
    }
#endif
    // 创建
    auto hr = UIManager.CreateTextFormat(
        prop.name,
        static_cast<DWRITE_FONT_WEIGHT>(prop.weight),
        static_cast<DWRITE_FONT_STYLE>(prop.style),
        static_cast<DWRITE_FONT_STRETCH>(prop.stretch),
        prop.size,
        fmt
        );
    // 成功
    if (SUCCEEDED(hr)) {
        auto format = *fmt;
        // 设置 Tab宽度
        hr = format->SetIncrementalTabStop(prop.tab == 0.f ? prop.size * 4.f : prop.tab);
        longui_debug_hr(hr, L"failed format->SetIncrementalTabStop  " << ((prop.tab == 0.f ? prop.size * 4.f : prop.tab)));
        // 设置段落排列方向
        hr = format->SetFlowDirection(static_cast<DWRITE_FLOW_DIRECTION>(prop.flow));
        longui_debug_hr(hr, L"failed format->SetFlowDirection  " << long(prop.flow));
        // 设置段落(垂直)对齐
        hr = format->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(prop.valign));
        longui_debug_hr(hr, L"failed format->SetParagraphAlignment  " << long(prop.valign));
        // 设置文本(水平)对齐
        hr = format->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(prop.halign));
        longui_debug_hr(hr, L"failed format->SetTextAlignmen  t" << long(prop.halign));
        // 设置阅读进行方向
        hr = format->SetReadingDirection(static_cast<DWRITE_READING_DIRECTION>(prop.reading));
        longui_debug_hr(hr, L"failed format->SetReadingDirection  " << long(prop.reading));
        // 设置自动换行
        hr = format->SetWordWrapping(static_cast<DWRITE_WORD_WRAPPING>(prop.wrapping));
        longui_debug_hr(hr, L"failed format->SetWordWrapping  " << long(prop.wrapping));
        // 设置
        hr = S_OK;
    }
    return hr;
}


// 初始化TextFormatProperties
LongUINoinline void LongUI::DX::InitTextFormatProperties(TextFormatProperties& prop, size_t name_buf_len) noexcept {
    UNREFERENCED_PARAMETER(name_buf_len);
#ifdef _DEBUG
    auto length = std::wcslen(LongUI::LongUIDefaultTextFontName) + 1;
    assert(name_buf_len > length && "buffer too small");
#endif
    // 复制数据
    prop.size = LongUIDefaultTextFontSize;
    prop.tab = 0.f;
    prop.weight = static_cast<uint16_t>(DWRITE_FONT_WEIGHT_NORMAL);
    prop.style = static_cast<uint8_t>(DWRITE_FONT_STYLE_NORMAL);
    prop.stretch = static_cast<uint8_t>(DWRITE_FONT_STRETCH_NORMAL);
    prop.valign = static_cast<uint8_t>(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    prop.halign = static_cast<uint8_t>(DWRITE_TEXT_ALIGNMENT_LEADING);
    prop.flow = static_cast<uint8_t>(DWRITE_FLOW_DIRECTION_TOP_TO_BOTTOM);
    prop.reading = static_cast<uint8_t>(DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);
    prop.wrapping = static_cast<uint32_t>(DWRITE_WORD_WRAPPING_NO_WRAP);
    std::wcscpy(prop.name, LongUIDefaultTextFontName);
}

// 做一个文本格式
auto LongUI::DX::MakeTextFormat(
    IN pugi::xml_node node, 
    OUT IDWriteTextFormat** fmt, 
    IN OPTIONAL IDWriteTextFormat* template_fmt, 
    IN OPTIONAL const char* prefix) noexcept -> HRESULT {
    // 参数检查
    assert(fmt && "bad argment"); if (!fmt) return E_INVALIDARG;
#ifdef _DEBUG
    if (*fmt) {
        UIManager << DL_Warning
            << L"pointer 'fmt' pointed a non-nullptr, check it please."
            << endl;
    }
#endif
    // 数据
    struct { TextFormatProperties prop; wchar_t buffer[MAX_PATH]; } data;
    // 创建新的?
    bool create_a_new_one = false;
    // 存在模板?
    if (template_fmt) {
        // 模板初始化
        auto len = template_fmt->GetFontFamilyNameLength();
        assert(len < MAX_PATH && "buffer too small");
        template_fmt->GetFontFamilyName(data.prop.name, len + 1);
        data.prop.size = template_fmt->GetFontSize();
        data.prop.tab = template_fmt->GetIncrementalTabStop();
        data.prop.weight = static_cast<uint16_t>(template_fmt->GetFontWeight());
        data.prop.style = static_cast<uint8_t>(template_fmt->GetFontStyle());
        data.prop.stretch = static_cast<uint8_t>(template_fmt->GetFontStretch());
        data.prop.valign = static_cast<uint8_t>(template_fmt->GetParagraphAlignment());
        data.prop.halign = static_cast<uint8_t>(template_fmt->GetTextAlignment());
        data.prop.flow = static_cast<uint8_t>(template_fmt->GetFlowDirection());
        data.prop.reading = static_cast<uint8_t>(template_fmt->GetReadingDirection());
        data.prop.wrapping = static_cast<uint32_t>(template_fmt->GetWordWrapping());
    }
    else {
        // 默认初始化
        DX::InitTextFormatProperties(data.prop, MAX_PATH);
        create_a_new_one = true;
    }
    // xml 节点
    {
        auto get_attribute = [=](const char* name) noexcept {
            return Helper::XMLGetValue(node, name, prefix);
        };
        // 字体名称
        auto str = get_attribute("family");
        if (str) {
            // 假设设置字体名称就是修改了
            data.prop.name[LongUI::UTF8toWideChar(str, data.prop.name)] = 0;
            create_a_new_one = true;
        }
        // 字体大小
        if (str = get_attribute("size")) {
            auto tmp = LongUI::AtoF(str);
            create_a_new_one = tmp != data.prop.size || create_a_new_one;
            data.prop.size = tmp;
        }
        // 获取字体粗细
        if (str = get_attribute("weight")) {
            auto tmp = static_cast<uint16_t>(LongUI::AtoI(str));
            create_a_new_one = tmp != data.prop.weight || create_a_new_one;
            data.prop.weight = tmp;
        }
        // 字体风格
        {
            auto tmp = static_cast<uint8_t>(Helper::GetEnumFromXml(node, static_cast<DWRITE_FONT_STYLE>(data.prop.style), "style", prefix));
            create_a_new_one = tmp != data.prop.style || create_a_new_one;
            data.prop.style = tmp;
        }
        // 字体拉伸
        {
            auto tmp = static_cast<uint8_t>(Helper::GetEnumFromXml(node, static_cast<DWRITE_FONT_STRETCH>(data.prop.stretch), "stretch", prefix));
            create_a_new_one = tmp != data.prop.stretch || create_a_new_one;
            data.prop.stretch = tmp;
        }
        // Tab宽度
        float tabstop = data.prop.size * 4.f;
        // 检查Tab宽度
        if (str = get_attribute("tabstop")) {
            // 假设设置字体名称就是修改了
            tabstop = LongUI::AtoF(str);
            create_a_new_one = true;
        }
        // 段落排列方向
        {
            auto tmp = static_cast<uint8_t>(Helper::GetEnumFromXml(node, static_cast<DWRITE_FLOW_DIRECTION>(data.prop.flow), "flowdirection", prefix));
            create_a_new_one = tmp != data.prop.flow || create_a_new_one;
            data.prop.flow = tmp;
        }
        // 段落(垂直)对齐
        {
            auto tmp = static_cast<uint8_t>(Helper::GetEnumFromXml(node, static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(data.prop.valign), "valign", prefix));
            create_a_new_one = tmp != data.prop.valign || create_a_new_one;
            data.prop.valign = tmp;
        }
        // 文本(水平)对齐
        {
            auto tmp = static_cast<uint8_t>(Helper::GetEnumFromXml(node, static_cast<DWRITE_TEXT_ALIGNMENT>(data.prop.halign), "align", prefix));
            create_a_new_one = tmp != data.prop.halign || create_a_new_one;
            data.prop.halign = tmp;
        }
        // 阅读进行方向
        {
            auto tmp = static_cast<uint8_t>(Helper::GetEnumFromXml(node, static_cast<DWRITE_READING_DIRECTION>(data.prop.reading), "readingdirection", prefix));
            create_a_new_one = tmp != data.prop.reading || create_a_new_one;
            data.prop.reading = tmp;
        }
        // 设置自动换行
        {
            auto tmp = static_cast<uint32_t>(Helper::GetEnumFromXml(node, static_cast<DWRITE_WORD_WRAPPING>(data.prop.wrapping), "wordwrapping", prefix));
            create_a_new_one = tmp != data.prop.wrapping || create_a_new_one;
            data.prop.wrapping = tmp;
        }
    }
    // 创建新的
    if (create_a_new_one) {
        return DX::CreateTextFormat(data.prop, fmt);
    }
    // 使用旧的, 检查逻辑
    assert(template_fmt && "check logic");
    template_fmt->AddRef();
    *fmt = template_fmt;
    return S_FALSE;
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
    if (_fontface) fontface = LongUI::SafeAcquire(*_fontface);
    // 字体名称缓存
    wchar_t fontname_buffer[MAX_PATH]; *fontname_buffer = 0;
    // 必要缓存
    EzContainer::SmallBuffer<uint16_t, 1024> glyph_indices_buffer;
    glyph_indices_buffer.NewSize(string_length);
    // 内存不足?
    HRESULT hr = glyph_indices_buffer.GetCount() < string_length ? S_OK : E_OUTOFMEMORY;
    // 创建字形
    if (!fontface) {
        // 获取字体名称
        if (SUCCEEDED(hr)) {
            hr = format->GetFontFamilyName(fontname_buffer, MAX_PATH);
            longui_debug_hr(hr, L"failed: format->GetFontFamilyName " << fontname_buffer);
        }
        // 获取字体集
        if (SUCCEEDED(hr)) {
            hr = format->GetFontCollection(&collection);
            longui_debug_hr(hr, L"failed: format->GetFontCollection ");
        }
        // 查找索引
        uint32_t index = 0; BOOL exists = FALSE;
        if (SUCCEEDED(hr)) {
            hr = collection->FindFamilyName(fontname_buffer, &index, &exists);
            longui_debug_hr(hr, L"failed: collection->FindFamilyName " << fontname_buffer);
        }
        // 获取字体族
        if (SUCCEEDED(hr)) {
            // 不存在也算错误
            if (exists) {
                hr = collection->GetFontFamily(index, &family);
                longui_debug_hr(hr, L"failed: collection->GetFontFamily " << long(index));
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
            longui_debug_hr(hr, L"failed: family->GetFirstMatchingFont ");

        }
        // 创建字形
        if (SUCCEEDED(hr)) {
            hr = font->CreateFontFace(&fontface);
            longui_debug_hr(hr, L"failed: font->CreateFontFace ");
        }
    }
    // 创建几何
    if (SUCCEEDED(hr)) {
        hr = factory->CreatePathGeometry(&pathgeometry);
        longui_debug_hr(hr, L"failed: factory->CreatePathGeometry ");
        ID2D1GeometrySink* sink = nullptr;
        // 打开Sink
        if (SUCCEEDED(hr)) {
            hr = pathgeometry->Open(&sink);
            longui_debug_hr(hr, L"failed: pathgeometry->Open ");
        }
        // 创建索引编号
        if (SUCCEEDED(hr)) {
            static_assert(sizeof(uint32_t) == sizeof(char32_t), "32 != 32 ?!");
            hr = fontface->GetGlyphIndices(
                reinterpret_cast<const uint32_t*>(utf32_string), 
                string_length, 
                glyph_indices_buffer.GetData()
                );
            longui_debug_hr(hr, L"failed: fontface->GetGlyphIndices ");
        }
        // 创建轮廓路径几何
        if (SUCCEEDED(hr)) {
            hr = fontface->GetGlyphRunOutline(
                format->GetFontSize(),
                glyph_indices_buffer.GetData(),
                nullptr, nullptr,
                string_length,
                true, true, sink
                );
            longui_debug_hr(hr, L"failed: fontface->GetGlyphRunOutline ");
        }
        // 关闭路径
        if (SUCCEEDED(hr)) {
            hr = sink->Close();
            longui_debug_hr(hr, L"failed: sink->Close ");
        }
        LongUI::SafeRelease(sink);
    }
    // 扫尾
    LongUI::SafeRelease(collection);
    LongUI::SafeRelease(family);
    LongUI::SafeRelease(font);
    if (_fontface && !(*_fontface)) {
        *_fontface = fontface;
    }
    else {
        LongUI::SafeRelease(fontface);
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
    assert(mesh && "bad arguemnt"); if (!mesh) return E_INVALIDARG;
#ifdef _DEBUG
    if (*mesh) {
        UIManager << DL_Warning
            << L"pointer 'mesh' pointed a non-nullptr, check it please."
            << endl;
    }
#endif
    return E_NOTIMPL;
}

// 直接使用
auto LongUI::DX::FormatTextXML(
    const FormatTextConfig& config, 
    const wchar_t* format
    ) noexcept ->IDWriteTextLayout*{
    UNREFERENCED_PARAMETER(config);
    UNREFERENCED_PARAMETER(format);
    return nullptr;
}


// 格式化文字
/*
control char    C-Type      Infomation                                   StringParamSupported

%%              [none]      As '%' Character(like %% in std::printf)             ---

%a         [const wchar_t*] string add(like %ls in std::printf)                  ---

%c            [float4*]   new font color range start, with alpha                 Yes

%e           [IUnknown*]    new drawing effect range start                       ---

%s             [float]     new font size range start                             Yes

%f        [const wchar_t*] new font family name range start            Yes but without "," char

%h             [enum]      new font stretch range start                          Yes

%y             [enum]      new font style range start                            Yes

%w             [enum]      new font weight range start                           Yes

%u             [---]      new underline range start                              ---

%d             [---]      new strikethrough(delete line) range start             ---

%i            [IDIO*]     new inline object range start                          ---

%]             [---]       end of the last range                                 ---

%t            [IDWT*]      new typography range start                            ---

%l       [const wchar_t*]    new locale name range start                         YES

//  Unsupported
%f %F   [UNSPT]  [IDFC*]     new font collection range start                     ---
IDWriteFontCollection*

FORMAT IN STRING
the va_list(ap) can be nullptr while string format include the PARAMETERS, 
use %p to mark PARAMETERS start

*/

// longui::dx namespace
namespace LongUI { namespace DX {
    // 范围类型
    enum class RANGE_TYPE : size_t { F, W, Y, H, S, U, D, E, I, T, L };
    // 范围数据
    struct RANGE_DATA {
        // 具体数据
        union {
            const wchar_t*          wstr;       // FL
            IUnknown*               effect;     // E
            IDWriteInlineObject*    inlineobj;  // I
            IDWriteTypography*      typography; // T
            DWRITE_FONT_WEIGHT      weight;     // W
            DWRITE_FONT_STYLE       style;      // Y
            DWRITE_FONT_STRETCH     stretch;    // D
            float                   size;       // S
            //BOOL                    underline;  // U
            //BOOL                    strikethr;  // T
        };
        // 类型
        RANGE_TYPE                  type;
        // 范围
        DWRITE_TEXT_RANGE           range;
    };
    // C参数
    struct CoreMLParamC {
        // 构造函数
        CoreMLParamC(va_list va) noexcept : list(va) {};
        // 获取刻画效果
        auto GetEffect() noexcept { auto p = va_arg(list, IUnknown*); assert(p); p->AddRef(); return p; }
        // 获取内联对象
        auto GetInlineObject() noexcept {  auto p va_arg(list, IDWriteInlineObject*);  assert(p); p->AddRef(); return p; }
        // 获取版式功能
        auto GetTypography() noexcept {  auto p va_arg(list, IDWriteTypography*);  assert(p); p->AddRef(); return p; }
        // 获取字符串
        auto GetString() noexcept { return va_arg(list, const wchar_t*); }
        // 获取字体名称
        auto GetStringEx() noexcept { return va_arg(list, const wchar_t*); }
        // 获取颜色
        void GetColor(D2D1_COLOR_F& color) noexcept { color = *(va_arg(list, D2D1_COLOR_F*)); }
        // 获取浮点, float 经过可变参数会提升至double
        auto GetFloat() noexcept { return static_cast<float>(va_arg(list, double)); }
        // 获取字体粗细
        auto GetFontWeight() noexcept { return va_arg(list, DWRITE_FONT_WEIGHT); }
        // 获取字体风格
        auto GetFontStyle() noexcept { return va_arg(list, DWRITE_FONT_STYLE); }
        // 获取字体伸缩
        auto GetFontStretch() noexcept  { return va_arg(list, DWRITE_FONT_STRETCH); }
        // 可变参数列表
        va_list             list;
    };
    // 字符串参数
    struct CoreMLParamString {
        // 构造函数
        CoreMLParamString(const wchar_t* p) noexcept;
        // 获取刻画效果
        auto GetEffect() noexcept -> IUnknown* { assert(!"unsupported for string param!"); return nullptr; }
        // 获取内联对象
        auto GetInlineObject() noexcept -> IDWriteInlineObject*  { assert(!"unsupported for string param!"); return nullptr; }
        // 获取版式功能
        auto GetTypography() noexcept -> IDWriteTypography*;
        // 获取字符串
        auto GetString() noexcept -> const wchar_t* { assert(!"unsupported for string param!"); return nullptr; }
        // 获取字符串Ex
        auto GetStringEx() noexcept -> const wchar_t*;
        // 获取颜色
        void GetColor(D2D1_COLOR_F& color) noexcept;
        // 获取浮点
        auto GetFloat() noexcept ->float;
        // 获取字体粗细
        auto GetFontWeight() noexcept ->DWRITE_FONT_WEIGHT;
        // 获取字体风格
        auto GetFontStyle() noexcept ->DWRITE_FONT_STYLE;
        // 获取字体伸缩
        auto GetFontStretch() noexcept ->DWRITE_FONT_STRETCH;
        // 参数地址
        const wchar_t*              param = nullptr;
        // 字体名迭代器
        wchar_t*                    family_itr = family_buffer;
        // 字体名缓存
        wchar_t                     family_buffer[1024];
    private:
        // 复制字符串, 返回最后非空白字符串数据
        template<typename T> auto copy_string_sp(T* __restrict des) noexcept {
            // 获取数据
            const wchar_t* __restrict src = this->param;
            // 跳过空白
            while (white_space(*src)) ++src;
            // 复制数据
            while ((*src) && ((*src)) != ',') { *des = static_cast<T>(*src); ++src; ++des; }
            // 检查最后有效字符串
            auto last = des; while (white_space(last[-1])) --last;
            // 写入
            this->param = src + 1;
#ifdef _DEBUG
            // 调试
            if (!src[0]) this->param = nullptr;
#endif
            // 返回
            return last;
        }
    };
    // CoreMLParamString 构造函数
    inline CoreMLParamString::CoreMLParamString(const wchar_t* p) noexcept {
        while (*p) {
            if (p[0] == '%' && p[1] == 'p') {
                this->param = p + 2;
                break;
            }
            ++p;
        }
    }
    // 获取浮点数
    inline auto CoreMLParamString::GetFloat() noexcept -> float {
        assert(this->param && "bad param, ungiven parameters, nor wrong number of parameters");
        char buffer[1024]; 
        auto end = this->copy_string_sp(buffer); end[0] = 0;
        assert((size_t(end - buffer) < lengthof(buffer)) && "buffer to small");
        return LongUI::AtoF(buffer);
    }
    // 获取字体粗细
    inline auto CoreMLParamString::GetFontWeight() noexcept -> DWRITE_FONT_WEIGHT {
        assert(this->param && "bad param, ungiven parameters, nor wrong number of parameters");
        char buffer[1024]; 
        auto end = this->copy_string_sp(buffer); end[0] = 0;
        assert((size_t(end - buffer) < lengthof(buffer)) && "buffer to small");
        return static_cast<DWRITE_FONT_WEIGHT>(LongUI::AtoI(buffer));
    }
    // 获取字体风格
    inline auto CoreMLParamString::GetFontStyle() noexcept -> DWRITE_FONT_STYLE {
        assert(this->param && "bad param, ungiven parameters, nor wrong number of parameters");
        char buffer[1024]; 
        auto end = this->copy_string_sp(buffer); end[0] = 0;
        assert((size_t(end - buffer) < lengthof(buffer)) && "buffer to small");
        return Helper::GetEnumFromString(buffer, DWRITE_FONT_STYLE_NORMAL);
    }
    // 获取字体伸缩
    inline auto CoreMLParamString::GetFontStretch() noexcept -> DWRITE_FONT_STRETCH {
        assert(this->param && "bad param, ungiven parameters, nor wrong number of parameters");
        char buffer[1024]; 
        auto end = this->copy_string_sp(buffer); end[0] = 0;
        assert((size_t(end - buffer) < lengthof(buffer)) && "buffer to small");
        return Helper::GetEnumFromString(buffer, DWRITE_FONT_STRETCH_NORMAL);
    }
    // 获取颜色
    inline void CoreMLParamString::GetColor(D2D1_COLOR_F& color) noexcept {
        assert(this->param && "bad param, ungiven parameters, nor wrong number of parameters");
        char buffer[1024]; 
        auto end = this->copy_string_sp(buffer); end[0] = 0;
        assert((size_t(end - buffer) < lengthof(buffer)) && "buffer to small");
        Helper::MakeColor(buffer, color);
    }
    // 获取字体名称
    auto CoreMLParamString::GetStringEx() noexcept -> const wchar_t* {
        auto old = this->family_itr;
        auto end = this->copy_string_sp(old); end[0] = 0;
        this->family_itr = end + 1;
        return old;
    }
    // 获取排版样式
    inline auto CoreMLParamString::GetTypography() noexcept -> IDWriteTypography* {
        IDWriteTypography* typography = nullptr;
        // 设置参数
        assert(this->param && "bad param, ungiven parameters, nor wrong number of parameters");
        char buffer[1024]; 
        auto end = this->copy_string_sp(buffer);
        end[0] = ' '; end[1] = 0;
        assert((size_t(end - buffer) < lengthof(buffer)) && "buffer to small");
        // 创建 Typography
        auto hr = UIManager_DWriteFactory->CreateTypography(&typography);
        longui_debug_hr(hr, L"failed: UIManager_DWriteFactory->CreateTypography ");
        assert(std::strlen(buffer) % 5 == 0 && "bad font feature tag");
        // CPU 大小端检查
        static_assert(uint32_t(DWRITE_FONT_FEATURE_TAG_CASE_SENSITIVE_FORMS) == uint32_t("case"_longui32), "check cpu type");
        static_assert(sizeof(uint32_t) == sizeof(DWRITE_FONT_FEATURE_TAG), "check enum type");
        // 添加 OpenTypoe 特性
        if (SUCCEEDED(hr)) {
            DWRITE_FONT_FEATURE feature;
            feature.parameter = 1;
            // 遍历字符串
            for (auto itr = buffer; *itr; itr += 5) {
                // 稍微检查一下
                assert(itr[0] && itr[1] && itr[2] && itr[3] && itr[4] == ' ' && "bad argments");
                // 一般视为二进制数据
                auto tmp = *reinterpret_cast<int32_t*>(itr);
                feature.nameTag = static_cast<DWRITE_FONT_FEATURE_TAG>(tmp);
                auto thr = typography->AddFontFeature(feature);
                UNREFERENCED_PARAMETER(thr);
                longui_debug_hr(thr, L"failed: typography->AddFontFeature " << long(feature.nameTag));
            }
        }
        return typography;
    }
    // 创建格式文本
    template<typename T>
    auto FormatTextViaCoreML(const FormatTextConfig& cfg, const wchar_t* fmt, T& param) noexcept {
        using cctype = wchar_t;
        cctype ch = 0;
        cctype text[LongUIStringBufferLength]; auto text_itr = text;
        EzContainer::FixedStack<RANGE_DATA, 1024> stack_check, stack_set;
        // 遍历字符串
        while ((ch = *fmt)) {
            // 出现%标记
            if (ch == '%') {
                switch ((ch = fmt[1]))
                {
                case '%':
                    // %% --> 添加字符"%"
                    *text_itr = '%';
                    ++text_itr;
                    break;
                case 'p': 
                    // %p --> 结束
                    goto force_break;
                case ']':
                    // %] --> 结束一个范围
                    // 检查栈弹出
                    stack_check.pop();
                    // 计算长度
                    stack_check.top->range.length = static_cast<UINT32>(text_itr - text) - stack_check.top->range.startPosition;
                    // 压入设置栈
                    stack_set.push_back(*stack_check.top);
                    break;
                case 'a':
                    // %a --> 添加字符串
                    // 直接写入字符串
                    for (auto str = param.GetString(); *str; ++str, ++text_itr) {
                        *text_itr = *str;
                    }
                    break;
                case 'c': case 'e':
                    // %c --> 添加颜色
                    // %e --> 添加效果
                {
                    RANGE_DATA range;
                    if (ch == 'c') {
                        D2D1_COLOR_F color; param.GetColor(color);
                        range.effect = CUIColorEffect::Create(color);
                    }
                    else {
                        range.effect = param.GetEffect();
                    }
                    assert(range.effect && "OOM or bad action");
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::E;
                    stack_check.push_back(range);
                    break;
                }
                case 'i':
                    // %i --> 添加内联对象
                {
                    RANGE_DATA range;
                    range.effect = param.GetInlineObject();
                    assert(range.effect && "OOM or bad action");
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::I;
                    stack_check.push_back(range);
                    break;
                }
                case 't':
                    // %t --> 添加版式功能
                {
                    RANGE_DATA range;
                    range.typography = param.GetTypography();
                    assert(range.effect && "OOM or bad action");
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::T;
                    stack_check.push_back(range);
                    break;
                }
                case 'f': case 'l':
                    // %f --> 字体名称
                    // %l --> 本地名称
                {
                    RANGE_DATA range;
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = ch == 'f' ? RANGE_TYPE::F : RANGE_TYPE::L;
                    range.wstr = param.GetStringEx();
                    stack_check.push_back(range);
                    break;
                }
                case 's':
                    // %s --> 字体大小
                {
                    RANGE_DATA range;
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::S;
                    range.size = param.GetFloat();
                    stack_check.push_back(range);
                    break;
                }
                case 'w':
                    // %w --> 字体粗细
                {
                    RANGE_DATA range;
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::W;
                    range.weight = param.GetFontWeight();
                    stack_check.push_back(range);
                    break;
                }
                case 'y':
                    // %y --> 字体风格
                {
                    RANGE_DATA range;
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::Y;
                    range.style = param.GetFontStyle();
                    stack_check.push_back(range);
                    break;
                }
                case 'h':
                    // %h --> 字体伸缩
                {
                    RANGE_DATA range;
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::H;
                    range.stretch = param.GetFontStretch();
                    stack_check.push_back(range);
                    break;
                }
                case 'u':
                    // %u --> 设置下划线
                {
                    RANGE_DATA range;
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::U;
                    stack_check.push_back(range);
                    break;
                }
                case 'd':
                    // %d --> 设置删除线
                {
                    RANGE_DATA range;
                    range.range.startPosition = static_cast<UINT32>(text_itr - text);
                    range.type = RANGE_TYPE::D;
                    stack_check.push_back(range);
                    break;
                }
                }
                // 写入数据
                fmt += 2;
            }
            else {
                // 写入数据
                assert((size_t(text_itr - text) < lengthof(text)) && "buffer too small");
                *text_itr = *fmt;
                ++fmt;
                ++text_itr;
            }
        }
    force_break:
        auto hr = S_OK;
        assert(stack_check.empty() == true && "unmatched maker");
        // 计算
        IDWriteTextLayout* layout = nullptr;
        auto length = static_cast<UINT32>(text_itr - text);
        auto needed = static_cast<uint32_t>(static_cast<float>(length + 1) * cfg.progress);
        if (needed > length) needed = length;
        // 创建布局
        if (SUCCEEDED(hr)) {
            hr = UIManager_DWriteFactory->CreateTextLayout(
                text,
                length,
                cfg.format,
                cfg.width, cfg.height,
                &layout
                );
#ifdef _DEBUG
            text[length] = 0;
            longui_debug_hr(hr, L"failed: UIManager_DWriteFactory->CreateTextLayout  " << text);
#endif
        }
        // 数据末尾
        auto setend = stack_set.top;
        // 正式创建
        if (SUCCEEDED(hr)) {
            // 创建
            while (!stack_set.empty()) {
                stack_set.pop();
                // 检查进度(progress)范围 释放数据
                auto end = stack_set.top->range.startPosition + stack_set.top->range.length;
                if (end > needed) continue;
                // 检查类型
                switch (stack_set.top->type)
                {
                case RANGE_TYPE::F:
                    layout->SetFontFamilyName(stack_set.top->wstr, stack_set.top->range);
                    break;
                case RANGE_TYPE::W:
                    layout->SetFontWeight(stack_set.top->weight, stack_set.top->range);
                    break;
                case RANGE_TYPE::Y:
                    layout->SetFontStyle(stack_set.top->style, stack_set.top->range);
                    break;
                case RANGE_TYPE::H:
                    layout->SetFontStretch(stack_set.top->stretch, stack_set.top->range);
                    break;
                case RANGE_TYPE::S:
                    layout->SetFontSize(stack_set.top->size, stack_set.top->range);
                    break;
                case RANGE_TYPE::U:
                    layout->SetUnderline(TRUE, stack_set.top->range);
                    break;
                case RANGE_TYPE::D:
                    layout->SetStrikethrough(TRUE, stack_set.top->range);
                    break;
                case RANGE_TYPE::E:
                    layout->SetDrawingEffect(stack_set.top->effect, stack_set.top->range);
                    break;
                case RANGE_TYPE::I:
                    layout->SetInlineObject(stack_set.top->inlineobj, stack_set.top->range);
                    break;
                case RANGE_TYPE::T:
                    layout->SetTypography(stack_set.top->typography, stack_set.top->range);
                    break;
                case RANGE_TYPE::L:
                    layout->SetLocaleName(stack_set.top->wstr, stack_set.top->range);
                    break;
                }
            }
        }
        // 错误信息
        if (FAILED(hr)) {
            UIManager << DL_Error << L"HR Code: " << long(hr) << LongUI::endl;
        }
        // 释放数据
        for (auto itr = stack_set.data; itr != setend; ++itr) {
            if (itr->type == RANGE_TYPE::E || itr->type == RANGE_TYPE::I ||
                itr->type == RANGE_TYPE::T) {
                LongUI::SafeRelease(itr->effect);
            }
        }
        return layout;
    }
    // 创建格式文本
    auto FormatTextCoreC(const FormatTextConfig& cfg, const wchar_t* fmt, ...) noexcept ->IDWriteTextLayout* {
        va_list ap;
        va_start(ap, fmt);
        CoreMLParamC param(ap);
        auto lyt = DX::FormatTextViaCoreML(cfg, fmt, param);
        va_end(ap);
        return lyt;
    }
    // 创建格式文本
    auto FormatTextCore(const FormatTextConfig& cfg, const wchar_t* fmt, va_list) noexcept ->IDWriteTextLayout* {
        CoreMLParamString param(fmt);
        return DX::FormatTextViaCoreML(cfg, fmt, param);
    }
}}

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
            readable_bitmap = LongUI::SafeAcquire(bitmap);
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
#ifdef _DEBUG
            D2D1_POINT_2F ppppt;
            ppppt.x = float(bitmap_size.width);
            ppppt.y = float(bitmap_size.height);
            longui_debug_hr(hr, L"failed: UIManager_RenderTarget->CreateBitmap " << ppppt);
#endif
        }
    }
    // 复制位图
    if (SUCCEEDED(hr)) {
        hr = readable_bitmap->CopyFromBitmap(nullptr, bitmap, nullptr);
        longui_debug_hr(hr, L"failed: readable_bitmap->CopyFromBitmap");
    }
    // 映射数据
    if (SUCCEEDED(hr)) {
        hr = readable_bitmap->Map(D2D1_MAP_OPTIONS_READ, &rect);
        longui_debug_hr(hr, L"failed: readable_bitmap->Map");
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
        longui_debug_hr(hr1, L"failed: DX::SaveAsImageFile");
        auto hr2 = readable_bitmap->Unmap();
        longui_debug_hr(hr2, L"failed: readable_bitmap->Unmap");
        // 检查错误
        if (SUCCEEDED(hr1)) {
            hr = SUCCEEDED(hr2) ? S_OK : hr2;
        }
        else {
            hr = hr1;
        }
    }
    // 扫尾处理
    LongUI::SafeRelease(readable_bitmap);
    // 返回结果
    return hr;
}

#include <Wincodec.h>

// longui::dx namespace
LONGUI_NAMESPACE_BEGIN namespace DX {
    // 保存数据为图片格式
    auto SaveAsImageFile(const SaveAsImageFileProperties& prop, const wchar_t* file_name) noexcept -> HRESULT {
        // 参数检查
        assert(prop.bits && prop.factory && file_name && file_name[0]);
        if (!(prop.bits && prop.factory && file_name && file_name[0])) {
            return E_INVALIDARG;
        }
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
            longui_debug_hr(hr, L"failed: prop.factory->CreateBitmapFromMemory");
        }
        // 创建流
        if (SUCCEEDED(hr)) {
            hr = prop.factory->CreateStream(&pStream);
            longui_debug_hr(hr, L"failed: prop.factory->CreateStream");
        }
        // 从文件初始化
        if (SUCCEEDED(hr)) {
            hr = pStream->InitializeFromFilename(file_name, GENERIC_WRITE);
            longui_debug_hr(hr, L"failed: pStream->InitializeFromFilename  " << file_name);
        }
        // 创建编码器
        if (SUCCEEDED(hr)) {
            hr = prop.factory->CreateEncoder(
                prop.container_format ? *prop.container_format : GUID_ContainerFormatPng,
                nullptr,
                &pEncoder
                );
            longui_debug_hr(hr, L"failed: prop.factory->CreateEncoder");
        }
        // 初始化编码器
        if (SUCCEEDED(hr)) {
            hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
            longui_debug_hr(hr, L"failed: pEncoder->Initialize");
        }
        // 创建新的一帧
        if (SUCCEEDED(hr)) {
            hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr);
            longui_debug_hr(hr, L"failed: pEncoder->CreateNewFrame");
        }
        // 初始化帧编码器
        if (SUCCEEDED(hr)) {
            hr = pFrameEncode->Initialize(nullptr);
            longui_debug_hr(hr, L"failed: pFrameEncode->Initialize");
        }
        // 设置大小
        if (SUCCEEDED(hr)) {
            hr = pFrameEncode->SetSize(prop.width, prop.height);
            longui_debug_hr(hr, L"failed: pFrameEncode->SetSize " << long(prop.width) << L", " << long(prop.height));
        }
        // 设置格式
        WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
        if (SUCCEEDED(hr)) {
            hr = pFrameEncode->SetPixelFormat(&format);
            longui_debug_hr(hr, L"failed: pFrameEncode->SetPixelFormat");
        }
        // 写入源数据
        if (SUCCEEDED(hr)) {
            hr = pFrameEncode->WriteSource(pWICBitmap, nullptr);
            longui_debug_hr(hr, L"failed: pFrameEncode->WriteSource");
        }
        // 提交帧编码器
        if (SUCCEEDED(hr)) {
            hr = pFrameEncode->Commit();
            longui_debug_hr(hr, L"failed: pFrameEncode->Commit");
        }
        // 提交编码
        if (SUCCEEDED(hr)) {
            hr = pEncoder->Commit();
            longui_debug_hr(hr, L"failed: pEncoder->Commit");
        }
        // 扫尾处理
        LongUI::SafeRelease(pWICBitmap);
        LongUI::SafeRelease(pStream);
        LongUI::SafeRelease(pFrameEncode);
        LongUI::SafeRelease(pEncoder);
        // 返回结果
        return hr;
    }
    // WIC GUID <--> DXGI_FORMAT
    struct WICTranslate { GUID wic; DXGI_FORMAT format; };
    // data
    static const WICTranslate s_WICFormats[] = {
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
    auto DXGIToWIC(DXGI_FORMAT format) noexcept ->CPGUID {
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
    // d2d1 ------------------------
    void LongUINoinline D2D1MakeRotateMatrix(float angle, D2D1_POINT_2F center, D2D1_MATRIX_3X2_F& matrix) noexcept {
        constexpr float pi = 3.141592654f;
        float theta = angle * (pi / 180.0f);
        float sin_theta = std::sin(theta);
        float cos_theta = std::cos(theta);
        matrix._11 = cos_theta;
        matrix._12 = sin_theta;
        matrix._21 = -sin_theta;
        matrix._22 = cos_theta;
        matrix._31 = center.x - center.x * cos_theta + center.y * sin_theta;
        matrix._32 = center.y - center.x * sin_theta - center.y * cos_theta;
    }
#ifdef _DEBUG
    long g_dbg_product_counter = 0;
#endif
    // 设置乘
    void Matrix3x2F::SetProduct(const D2D1_MATRIX_3X2_F& a, const D2D1_MATRIX_3X2_F& b) noexcept {
#ifdef _DEBUG
        ++g_dbg_product_counter;
#endif
        this->_11 = a._11 * b._11 + a._12 * b._21;
        this->_12 = a._11 * b._12 + a._12 * b._22;
        this->_21 = a._21 * b._11 + a._22 * b._21;
        this->_22 = a._21 * b._12 + a._22 * b._22;
        this->_31 = a._31 * b._11 + a._32 * b._21 + b._31;
        this->_32 = a._31 * b._12 + a._32 * b._22 + b._32;
    }
}
LONGUI_NAMESPACE_END

                   

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
    LongUI::SafeRelease(path_geometry);
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
        char ch = 0;
        while ((ch = *path)) {
            ++path;
        }
    }
    // 关闭sink ??
    if (SUCCEEDED(hr)) {
        hr = sink->Close();
    }
    LongUI::SafeRelease(sink);
    return hr;
}

/*
0000
0
WM_NULL
0001
1
WM_CREATE
0002
2
WM_DESTROY
0003
3
WM_MOVE
0005
5
WM_SIZE
0006
6
WM_ACTIVATE
0007
7
WM_SETFOCUS
0008
8
WM_KILLFOCUS
000a
10
WM_ENABLE
000b
11
WM_SETREDRAW
000c
12
WM_SETTEXT
000d
13
WM_GETTEXT
000e
14
WM_GETTEXTLENGTH
000f
15
WM_PAINT
0010
16
WM_CLOSE
0011
17
WM_QUERYENDSESSION
0012
18
WM_QUIT
0013
19
WM_QUERYOPEN
0014
20
WM_ERASEBKGND
0015
21
WM_SYSCOLORCHANGE
0016
22
WM_ENDSESSION
0018
24
WM_SHOWWINDOW
0019
25
WM_CTLCOLOR
001a
26
WM_WININICHANGE
001b
27
WM_DEVMODECHANGE
001c
28
WM_ACTIVATEAPP
001d
29
WM_FONTCHANGE
001e
30
WM_TIMECHANGE
001f
31
WM_CANCELMODE
0020
32
WM_SETCURSOR
0021
33
WM_MOUSEACTIVATE
0022
34
WM_CHILDACTIVATE
0023
35
WM_QUEUESYNC
0024
36
WM_GETMINMAXINFO
0026
38
WM_PAINTICON
0027
39
WM_ICONERASEBKGND
0028
40
WM_NEXTDLGCTL
002a
42
WM_SPOOLERSTATE
002b
43
WM_DRAWITEM
002c
44
WM_MEASUREITEM
002d
45
WM_DELETEITEM
002e
46
WM_VKEYTOITEM
002f
47
WM_CHARTOITEM
0030
48
WM_SETFONT
0031
49
WM_GETFONT
0032
50
WM_SETHOTKEY
0033
51
WM_GETHOTKEY
0037
55
WM_QUERYDRAGICON
0039
57
WM_COMPAREITEM
003d
61
WM_GETOBJECT
0041
65
WM_COMPACTING
0044
68
WM_COMMNOTIFY
0046
70
WM_WINDOWPOSCHANGING
0047
71
WM_WINDOWPOSCHANGED
0048
72
WM_POWER
0049
73
WM_COPYGLOBALDATA
004a
74
WM_COPYDATA
004b
75
WM_CANCELJOURNAL
004e
78
WM_NOTIFY
0050
80
WM_INPUTLANGCHANGEREQUEST
0051
81
WM_INPUTLANGCHANGE
0052
82
WM_TCARD
0053
83
WM_HELP
0054
84
WM_USERCHANGED
0055
85
WM_NOTIFYFORMAT
007b
123
WM_CONTEXTMENU
007c
124
WM_STYLECHANGING
007d
125
WM_STYLECHANGED
007e
126
WM_DISPLAYCHANGE
007f
127
WM_GETICON
0080
128
WM_SETICON
0081
129
WM_NCCREATE
0082
130
WM_NCDESTROY
0083
131
WM_NCCALCSIZE
0084
132
WM_NCHITTEST
0085
133
WM_NCPAINT
0086
134
WM_NCACTIVATE
0087
135
WM_GETDLGCODE
0088
136
WM_SYNCPAINT
00a0
160
WM_NCMOUSEMOVE
00a1
161
WM_NCLBUTTONDOWN
00a2
162
WM_NCLBUTTONUP
00a3
163
WM_NCLBUTTONDBLCLK
00a4
164
WM_NCRBUTTONDOWN
00a5
165
WM_NCRBUTTONUP
00a6
166
WM_NCRBUTTONDBLCLK
00a7
167
WM_NCMBUTTONDOWN
00a8
168
WM_NCMBUTTONUP
00a9
169
WM_NCMBUTTONDBLCLK
00ab
171
WM_NCXBUTTONDOWN
00ac
172
WM_NCXBUTTONUP
00ad
173
WM_NCXBUTTONDBLCLK
00b0
176
EM_GETSEL
00b1
177
EM_SETSEL
00b2
178
EM_GETRECT
00b3
179
EM_SETRECT
00b4
180
EM_SETRECTNP
00b5
181
EM_SCROLL
00b6
182
EM_LINESCROLL
00b7
183
EM_SCROLLCARET
00b8
185
EM_GETMODIFY
00b9
187
EM_SETMODIFY
00ba
188
EM_GETLINECOUNT
00bb
189
EM_LINEINDEX
00bc
190
EM_SETHANDLE
00bd
191
EM_GETHANDLE
00be
192
EM_GETTHUMB
00c1
193
EM_LINELENGTH
00c2
194
EM_REPLACESEL
00c3
195
EM_SETFONT
00c4
196
EM_GETLINE
00c5
197
EM_LIMITTEXT
00c5
197
EM_SETLIMITTEXT
00c6
198
EM_CANUNDO
00c7
199
EM_UNDO
00c8
200
EM_FMTLINES
00c9
201
EM_LINEFROMCHAR
00ca
202
EM_SETWORDBREAK
00cb
203
EM_SETTABSTOPS
00cc
204
EM_SETPASSWORDCHAR
00cd
205
EM_EMPTYUNDOBUFFER
00ce
206
EM_GETFIRSTVISIBLELINE
00cf
207
EM_SETREADONLY
00d0
209
EM_SETWORDBREAKPROC
00d1
209
EM_GETWORDBREAKPROC
00d2
210
EM_GETPASSWORDCHAR
00d3
211
EM_SETMARGINS
00d4
212
EM_GETMARGINS
00d5
213
EM_GETLIMITTEXT
00d6
214
EM_POSFROMCHAR
00d7
215
EM_CHARFROMPOS
00d8
216
EM_SETIMESTATE
00d9
217
EM_GETIMESTATE
00e0
224
SBM_SETPOS
00e1
225
SBM_GETPOS
00e2
226
SBM_SETRANGE
00e3
227
SBM_GETRANGE
00e4
228
SBM_ENABLE_ARROWS
00e6
230
SBM_SETRANGEREDRAW
00e9
233
SBM_SETSCROLLINFO
00ea
234
SBM_GETSCROLLINFO
00eb
235
SBM_GETSCROLLBARINFO
00f0
240
BM_GETCHECK
00f1
241
BM_SETCHECK
00f2
242
BM_GETSTATE
00f3
243
BM_SETSTATE
00f4
244
BM_SETSTYLE
00f5
245
BM_CLICK
00f6
246
BM_GETIMAGE
00f7
247
BM_SETIMAGE
00f8
248
BM_SETDONTCLICK
00ff
255
WM_INPUT
0100
256
WM_KEYDOWN
0100
256
WM_KEYFIRST
0101
257
WM_KEYUP
0102
258
WM_CHAR
0103
259
WM_DEADCHAR
0104
260
WM_SYSKEYDOWN
0105
261
WM_SYSKEYUP
0106
262
WM_SYSCHAR
0107
263
WM_SYSDEADCHAR
0108
264
WM_KEYLAST
0109
265
WM_UNICHAR
0109
265
WM_WNT_CONVERTREQUESTEX
010a
266
WM_CONVERTREQUEST
010b
267
WM_CONVERTRESULT
010c
268
WM_INTERIM
010d
269
WM_IME_STARTCOMPOSITION
010e
270
WM_IME_ENDCOMPOSITION
010f
271
WM_IME_COMPOSITION
010f
271
WM_IME_KEYLAST
0110
272
WM_INITDIALOG
0111
273
WM_COMMAND
0112
274
WM_SYSCOMMAND
0113
275
WM_TIMER
0114
276
WM_HSCROLL
0115
277
WM_VSCROLL
0116
278
WM_INITMENU
0117
279
WM_INITMENUPOPUP
0118
280
WM_SYSTIMER
011f
287
WM_MENUSELECT
0120
288
WM_MENUCHAR
0121
289
WM_ENTERIDLE
0122
290
WM_MENURBUTTONUP
0123
291
WM_MENUDRAG
0124
292
WM_MENUGETOBJECT
0125
293
WM_UNINITMENUPOPUP
0126
294
WM_MENUCOMMAND
0127
295
WM_CHANGEUISTATE
0128
296
WM_UPDATEUISTATE
0129
297
WM_QUERYUISTATE
0132
306
WM_CTLCOLORMSGBOX
0133
307
WM_CTLCOLOREDIT
0134
308
WM_CTLCOLORLISTBOX
0135
309
WM_CTLCOLORBTN
0136
310
WM_CTLCOLORDLG
0137
311
WM_CTLCOLORSCROLLBAR
0138
312
WM_CTLCOLORSTATIC
0200
512
WM_MOUSEFIRST
0200
512
WM_MOUSEMOVE
0201
513
WM_LBUTTONDOWN
0202
514
WM_LBUTTONUP
0203
515
WM_LBUTTONDBLCLK
0204
516
WM_RBUTTONDOWN
0205
517
WM_RBUTTONUP
0206
518
WM_RBUTTONDBLCLK
0207
519
WM_MBUTTONDOWN
0208
520
WM_MBUTTONUP
0209
521
WM_MBUTTONDBLCLK
0209
521
WM_MOUSELAST
020a
522
WM_MOUSEWHEEL
020b
523
WM_XBUTTONDOWN
020c
524
WM_XBUTTONUP
020d
525
WM_XBUTTONDBLCLK
0210
528
WM_PARENTNOTIFY
0211
529
WM_ENTERMENULOOP
0212
530
WM_EXITMENULOOP
0213
531
WM_NEXTMENU
0214
532
WM_SIZING
0215
533
WM_CAPTURECHANGED
0216
534
WM_MOVING
0218
536
WM_POWERBROADCAST
0219
537
WM_DEVICECHANGE
0220
544
WM_MDICREATE
0221
545
WM_MDIDESTROY
0222
546
WM_MDIACTIVATE
0223
547
WM_MDIRESTORE
0224
548
WM_MDINEXT
0225
549
WM_MDIMAXIMIZE
0226
550
WM_MDITILE
0227
551
WM_MDICASCADE
0228
552
WM_MDIICONARRANGE
0229
553
WM_MDIGETACTIVE
0230
560
WM_MDISETMENU
0231
561
WM_ENTERSIZEMOVE
0232
562
WM_EXITSIZEMOVE
0233
563
WM_DROPFILES
0234
564
WM_MDIREFRESHMENU
0280
640
WM_IME_REPORT
0281
641
WM_IME_SETCONTEXT
0282
642
WM_IME_NOTIFY
0283
643
WM_IME_CONTROL
0284
644
WM_IME_COMPOSITIONFULL
0285
645
WM_IME_SELECT
0286
646
WM_IME_CHAR
0288
648
WM_IME_REQUEST
0290
656
WM_IMEKEYDOWN
0290
656
WM_IME_KEYDOWN
0291
657
WM_IMEKEYUP
0291
657
WM_IME_KEYUP
02a0
672
WM_NCMOUSEHOVER
02a1
673
WM_MOUSEHOVER
02a2
674
WM_NCMOUSELEAVE
02a3
675
WM_MOUSELEAVE
0300
768
WM_CUT
0301
769
WM_COPY
0302
770
WM_PASTE
0303
771
WM_CLEAR
0304
772
WM_UNDO
0305
773
WM_RENDERFORMAT
0306
774
WM_RENDERALLFORMATS
0307
775
WM_DESTROYCLIPBOARD
0308
776
WM_DRAWCLIPBOARD
0309
777
WM_PAINTCLIPBOARD
030a
778
WM_VSCROLLCLIPBOARD
030b
779
WM_SIZECLIPBOARD
030c
780
WM_ASKCBFORMATNAME
030d
781
WM_CHANGECBCHAIN
030e
782
WM_HSCROLLCLIPBOARD
030f
783
WM_QUERYNEWPALETTE
0310
784
WM_PALETTEISCHANGING
0311
785
WM_PALETTECHANGED
0312
786
WM_HOTKEY
0317
791
WM_PRINT
0318
792
WM_PRINTCLIENT
0319
793
WM_APPCOMMAND
0358
856
WM_HANDHELDFIRST
035f
863
WM_HANDHELDLAST
0360
864
WM_AFXFIRST
037f
895
WM_AFXLAST
0380
896
WM_PENWINFIRST
0381
897
WM_RCRESULT
0382
898
WM_HOOKRCRESULT
0383
899
WM_GLOBALRCCHANGE
0383
899
WM_PENMISCINFO
0384
900
WM_SKB
0385
901
WM_HEDITCTL
0385
901
WM_PENCTL
0386
902
WM_PENMISC
0387
903
WM_CTLINIT
0388
904
WM_PENEVENT
038f
911
WM_PENWINLAST
0400
1024
DDM_SETFMT
0400
1024
DM_GETDEFID
0400
1024
NIN_SELECT
0400
1024
TBM_GETPOS
0400
1024
WM_PSD_PAGESETUPDLG
0400
1024
WM_USER
0401
1025
CBEM_INSERTITEMA
0401
1025
DDM_DRAW
0401
1025
DM_SETDEFID
0401
1025
HKM_SETHOTKEY
0401
1025
PBM_SETRANGE
0401
1025
RB_INSERTBANDA
0401
1025
SB_SETTEXTA
0401
1025
TB_ENABLEBUTTON
0401
1025
TBM_GETRANGEMIN
0401
1025
TTM_ACTIVATE
0401
1025
WM_CHOOSEFONT_GETLOGFONT
0401
1025
WM_PSD_FULLPAGERECT
0402
1026
CBEM_SETIMAGELIST
0402
1026
DDM_CLOSE
0402
1026
DM_REPOSITION
0402
1026
HKM_GETHOTKEY
0402
1026
PBM_SETPOS
0402
1026
RB_DELETEBAND
0402
1026
SB_GETTEXTA
0402
1026
TB_CHECKBUTTON
0402
1026
TBM_GETRANGEMAX
0402
1026
WM_PSD_MINMARGINRECT
0403
1027
CBEM_GETIMAGELIST
0403
1027
DDM_BEGIN
0403
1027
HKM_SETRULES
0403
1027
PBM_DELTAPOS
0403
1027
RB_GETBARINFO
0403
1027
SB_GETTEXTLENGTHA
0403
1027
TBM_GETTIC
0403
1027
TB_PRESSBUTTON
0403
1027
TTM_SETDELAYTIME
0403
1027
WM_PSD_MARGINRECT
0404
1028
CBEM_GETITEMA
0404
1028
DDM_END
0404
1028
PBM_SETSTEP
0404
1028
RB_SETBARINFO
0404
1028
SB_SETPARTS
0404
1028
TB_HIDEBUTTON
0404
1028
TBM_SETTIC
0404
1028
TTM_ADDTOOLA
0404
1028
WM_PSD_GREEKTEXTRECT
0405
1029
CBEM_SETITEMA
0405
1029
PBM_STEPIT
0405
1029
TB_INDETERMINATE
0405
1029
TBM_SETPOS
0405
1029
TTM_DELTOOLA
0405
1029
WM_PSD_ENVSTAMPRECT
0406
1030
CBEM_GETCOMBOCONTROL
0406
1030
PBM_SETRANGE32
0406
1030
RB_SETBANDINFOA
0406
1030
SB_GETPARTS
0406
1030
TB_MARKBUTTON
0406
1030
TBM_SETRANGE
0406
1030
TTM_NEWTOOLRECTA
0406
1030
WM_PSD_YAFULLPAGERECT
0407
1031
CBEM_GETEDITCONTROL
0407
1031
PBM_GETRANGE
0407
1031
RB_SETPARENT
0407
1031
SB_GETBORDERS
0407
1031
TBM_SETRANGEMIN
0407
1031
TTM_RELAYEVENT
0408
1032
CBEM_SETEXSTYLE
0408
1032
PBM_GETPOS
0408
1032
RB_HITTEST
0408
1032
SB_SETMINHEIGHT
0408
1032
TBM_SETRANGEMAX
0408
1032
TTM_GETTOOLINFOA
0409
1033
CBEM_GETEXSTYLE
0409
1033
CBEM_GETEXTENDEDSTYLE
0409
1033
PBM_SETBARCOLOR
0409
1033
RB_GETRECT
0409
1033
SB_SIMPLE
0409
1033
TB_ISBUTTONENABLED
0409
1033
TBM_CLEARTICS
0409
1033
TTM_SETTOOLINFOA
040a
1034
CBEM_HASEDITCHANGED
040a
1034
RB_INSERTBANDW
040a
1034
SB_GETRECT
040a
1034
TB_ISBUTTONCHECKED
040a
1034
TBM_SETSEL
040a
1034
TTM_HITTESTA
040a
1034
WIZ_QUERYNUMPAGES
040b
1035
CBEM_INSERTITEMW
040b
1035
RB_SETBANDINFOW
040b
1035
SB_SETTEXTW
040b
1035
TB_ISBUTTONPRESSED
040b
1035
TBM_SETSELSTART
040b
1035
TTM_GETTEXTA
040b
1035
WIZ_NEXT
040c
1036
CBEM_SETITEMW
040c
1036
RB_GETBANDCOUNT
040c
1036
SB_GETTEXTLENGTHW
040c
1036
TB_ISBUTTONHIDDEN
040c
1036
TBM_SETSELEND
040c
1036
TTM_UPDATETIPTEXTA
040c
1036
WIZ_PREV
040d
1037
CBEM_GETITEMW
040d
1037
RB_GETROWCOUNT
040d
1037
SB_GETTEXTW
040d
1037
TB_ISBUTTONINDETERMINATE
040d
1037
TTM_GETTOOLCOUNT
040e
1038
CBEM_SETEXTENDEDSTYLE
040e
1038
RB_GETROWHEIGHT
040e
1038
SB_ISSIMPLE
040e
1038
TB_ISBUTTONHIGHLIGHTED
040e
1038
TBM_GETPTICS
040e
1038
TTM_ENUMTOOLSA
040f
1039
SB_SETICON
040f
1039
TBM_GETTICPOS
040f
1039
TTM_GETCURRENTTOOLA
0410
1040
RB_IDTOINDEX
0410
1040
SB_SETTIPTEXTA
0410
1040
TBM_GETNUMTICS
0410
1040
TTM_WINDOWFROMPOINT
0411
1041
RB_GETTOOLTIPS
0411
1041
SB_SETTIPTEXTW
0411
1041
TBM_GETSELSTART
0411
1041
TB_SETSTATE
0411
1041
TTM_TRACKACTIVATE
0412
1042
RB_SETTOOLTIPS
0412
1042
SB_GETTIPTEXTA
0412
1042
TB_GETSTATE
0412
1042
TBM_GETSELEND
0412
1042
TTM_TRACKPOSITION
0413
1043
RB_SETBKCOLOR
0413
1043
SB_GETTIPTEXTW
0413
1043
TB_ADDBITMAP
0413
1043
TBM_CLEARSEL
0413
1043
TTM_SETTIPBKCOLOR
0414
1044
RB_GETBKCOLOR
0414
1044
SB_GETICON
0414
1044
TB_ADDBUTTONSA
0414
1044
TBM_SETTICFREQ
0414
1044
TTM_SETTIPTEXTCOLOR
0415
1045
RB_SETTEXTCOLOR
0415
1045
TB_INSERTBUTTONA
0415
1045
TBM_SETPAGESIZE
0415
1045
TTM_GETDELAYTIME
0416
1046
RB_GETTEXTCOLOR
0416
1046
TB_DELETEBUTTON
0416
1046
TBM_GETPAGESIZE
0416
1046
TTM_GETTIPBKCOLOR
0417
1047
RB_SIZETORECT
0417
1047
TB_GETBUTTON
0417
1047
TBM_SETLINESIZE
0417
1047
TTM_GETTIPTEXTCOLOR
0418
1048
RB_BEGINDRAG
0418
1048
TB_BUTTONCOUNT
0418
1048
TBM_GETLINESIZE
0418
1048
TTM_SETMAXTIPWIDTH
0419
1049
RB_ENDDRAG
0419
1049
TB_COMMANDTOINDEX
0419
1049
TBM_GETTHUMBRECT
0419
1049
TTM_GETMAXTIPWIDTH
041a
1050
RB_DRAGMOVE
041a
1050
TBM_GETCHANNELRECT
041a
1050
TB_SAVERESTOREA
041a
1050
TTM_SETMARGIN
041b
1051
RB_GETBARHEIGHT
041b
1051
TB_CUSTOMIZE
041b
1051
TBM_SETTHUMBLENGTH
041b
1051
TTM_GETMARGIN
041c
1052
RB_GETBANDINFOW
041c
1052
TB_ADDSTRINGA
041c
1052
TBM_GETTHUMBLENGTH
041c
1052
TTM_POP
041d
1053
RB_GETBANDINFOA
041d
1053
TB_GETITEMRECT
041d
1053
TBM_SETTOOLTIPS
041d
1053
TTM_UPDATE
041e
1054
RB_MINIMIZEBAND
041e
1054
TB_BUTTONSTRUCTSIZE
041e
1054
TBM_GETTOOLTIPS
041e
1054
TTM_GETBUBBLESIZE
041f
1055
RB_MAXIMIZEBAND
041f
1055
TBM_SETTIPSIDE
041f
1055
TB_SETBUTTONSIZE
041f
1055
TTM_ADJUSTRECT
0420
1056
TBM_SETBUDDY
0420
1056
TB_SETBITMAPSIZE
0420
1056
TTM_SETTITLEA
0421
1057
MSG_FTS_JUMP_VA
0421
1057
TB_AUTOSIZE
0421
1057
TBM_GETBUDDY
0421
1057
TTM_SETTITLEW
0422
1058
RB_GETBANDBORDERS
0423
1059
MSG_FTS_JUMP_QWORD
0423
1059
RB_SHOWBAND
0423
1059
TB_GETTOOLTIPS
0424
1060
MSG_REINDEX_REQUEST
0424
1060
TB_SETTOOLTIPS
0425
1061
MSG_FTS_WHERE_IS_IT
0425
1061
RB_SETPALETTE
0425
1061
TB_SETPARENT
0426
1062
RB_GETPALETTE
0427
1063
RB_MOVEBAND
0427
1063
TB_SETROWS
0428
1064
TB_GETROWS
0429
1065
TB_GETBITMAPFLAGS
042a
1066
TB_SETCMDID
042b
1067
RB_PUSHCHEVRON
042b
1067
TB_CHANGEBITMAP
042c
1068
TB_GETBITMAP
042d
1069
MSG_GET_DEFFONT
042d
1069
TB_GETBUTTONTEXTA
042e
1070
TB_REPLACEBITMAP
042f
1071
TB_SETINDENT
0430
1072
TB_SETIMAGELIST
0431
1073
TB_GETIMAGELIST
0432
1074
TB_LOADIMAGES
0432
1074
EM_CANPASTE
0432
1074
TTM_ADDTOOLW
0433
1075
EM_DISPLAYBAND
0433
1075
TB_GETRECT
0433
1075
TTM_DELTOOLW
0434
1076
EM_EXGETSEL
0434
1076
TB_SETHOTIMAGELIST
0434
1076
TTM_NEWTOOLRECTW
0435
1077
EM_EXLIMITTEXT
0435
1077
TB_GETHOTIMAGELIST
0435
1077
TTM_GETTOOLINFOW
0436
1078
EM_EXLINEFROMCHAR
0436
1078
TB_SETDISABLEDIMAGELIST
0436
1078
TTM_SETTOOLINFOW
0437
1079
EM_EXSETSEL
0437
1079
TB_GETDISABLEDIMAGELIST
0437
1079
TTM_HITTESTW
0438
1080
EM_FINDTEXT
0438
1080
TB_SETSTYLE
0438
1080
TTM_GETTEXTW
0439
1081
EM_FORMATRANGE
0439
1081
TB_GETSTYLE
0439
1081
TTM_UPDATETIPTEXTW
043a
1082
EM_GETCHARFORMAT
043a
1082
TB_GETBUTTONSIZE
043a
1082
TTM_ENUMTOOLSW
043b
1083
EM_GETEVENTMASK
043b
1083
TB_SETBUTTONWIDTH
043b
1083
TTM_GETCURRENTTOOLW
043c
1084
EM_GETOLEINTERFACE
043c
1084
TB_SETMAXTEXTROWS
043d
1085
EM_GETPARAFORMAT
043d
1085
TB_GETTEXTROWS
043e
1086
EM_GETSELTEXT
043e
1086
TB_GETOBJECT
043f
1087
EM_HIDESELECTION
043f
1087
TB_GETBUTTONINFOW
0440
1088
EM_PASTESPECIAL
0440
1088
TB_SETBUTTONINFOW
0441
1089
EM_REQUESTRESIZE
0441
1089
TB_GETBUTTONINFOA
0442
1090
EM_SELECTIONTYPE
0442
1090
TB_SETBUTTONINFOA
0443
1091
EM_SETBKGNDCOLOR
0443
1091
TB_INSERTBUTTONW
0444
1092
EM_SETCHARFORMAT
0444
1092
TB_ADDBUTTONSW
0445
1093
EM_SETEVENTMASK
0445
1093
TB_HITTEST
0446
1094
EM_SETOLECALLBACK
0446
1094
TB_SETDRAWTEXTFLAGS
0447
1095
EM_SETPARAFORMAT
0447
1095
TB_GETHOTITEM
0448
1096
EM_SETTARGETDEVICE
0448
1096
TB_SETHOTITEM
0449
1097
EM_STREAMIN
0449
1097
TB_SETANCHORHIGHLIGHT
044a
1098
EM_STREAMOUT
044a
1098
TB_GETANCHORHIGHLIGHT
044b
1099
EM_GETTEXTRANGE
044b
1099
TB_GETBUTTONTEXTW
044c
1100
EM_FINDWORDBREAK
044c
1100
TB_SAVERESTOREW
044d
1101
EM_SETOPTIONS
044d
1101
TB_ADDSTRINGW
044e
1102
EM_GETOPTIONS
044e
1102
TB_MAPACCELERATORA
044f
1103
EM_FINDTEXTEX
044f
1103
TB_GETINSERTMARK
0450
1104
EM_GETWORDBREAKPROCEX
0450
1104
TB_SETINSERTMARK
0451
1105
EM_SETWORDBREAKPROCEX
0451
1105
TB_INSERTMARKHITTEST
0452
1106
EM_SETUNDOLIMIT
0452
1106
TB_MOVEBUTTON
0453
1107
TB_GETMAXSIZE
0454
1108
EM_REDO
0454
1108
TB_SETEXTENDEDSTYLE
0455
1109
EM_CANREDO
0455
1109
TB_GETEXTENDEDSTYLE
0456
1110
EM_GETUNDONAME
0456
1110
TB_GETPADDING
0457
1111
EM_GETREDONAME
0457
1111
TB_SETPADDING
0458
1112
EM_STOPGROUPTYPING
0458
1112
TB_SETINSERTMARKCOLOR
0459
1113
EM_SETTEXTMODE
0459
1113
TB_GETINSERTMARKCOLOR
045a
1114
EM_GETTEXTMODE
045a
1114
TB_MAPACCELERATORW
045b
1115
EM_AUTOURLDETECT
045b
1115
TB_GETSTRINGW
045c
1116
EM_GETAUTOURLDETECT
045c
1116
TB_GETSTRINGA
045d
1117
EM_SETPALETTE
045e
1118
EM_GETTEXTEX
045f
1119
EM_GETTEXTLENGTHEX
0460
1120
EM_SHOWSCROLLBAR
0461
1121
EM_SETTEXTEX
0463
1123
TAPI_REPLY
0464
1124
ACM_OPENA
0464
1124
BFFM_SETSTATETEXTA
0464
1124
CDM_FIRST
0464
1124
CDM_GETSPEC
0464
1124
EM_SETPUNCTUATION
0464
1124
IPM_CLEARADDRESS
0464
1124
WM_CAP_UNICODE_START
0465
1125
ACM_PLAY
0465
1125
BFFM_ENABLEOK
0465
1125
CDM_GETFILEPATH
0465
1125
EM_GETPUNCTUATION
0465
1125
IPM_SETADDRESS
0465
1125
PSM_SETCURSEL
0465
1125
UDM_SETRANGE
0465
1125
WM_CHOOSEFONT_SETLOGFONT
0466
1126
ACM_STOP
0466
1126
BFFM_SETSELECTIONA
0466
1126
CDM_GETFOLDERPATH
0466
1126
EM_SETWORDWRAPMODE
0466
1126
IPM_GETADDRESS
0466
1126
PSM_REMOVEPAGE
0466
1126
UDM_GETRANGE
0466
1126
WM_CAP_SET_CALLBACK_ERRORW
0466
1126
WM_CHOOSEFONT_SETFLAGS
0467
1127
ACM_OPENW
0467
1127
BFFM_SETSELECTIONW
0467
1127
CDM_GETFOLDERIDLIST
0467
1127
EM_GETWORDWRAPMODE
0467
1127
IPM_SETRANGE
0467
1127
PSM_ADDPAGE
0467
1127
UDM_SETPOS
0467
1127
WM_CAP_SET_CALLBACK_STATEW
0468
1128
BFFM_SETSTATETEXTW
0468
1128
CDM_SETCONTROLTEXT
0468
1128
EM_SETIMECOLOR
0468
1128
IPM_SETFOCUS
0468
1128
PSM_CHANGED
0468
1128
UDM_GETPOS
0469
1129
CDM_HIDECONTROL
0469
1129
EM_GETIMECOLOR
0469
1129
IPM_ISBLANK
0469
1129
PSM_RESTARTWINDOWS
0469
1129
UDM_SETBUDDY
046a
1130
CDM_SETDEFEXT
046a
1130
EM_SETIMEOPTIONS
046a
1130
PSM_REBOOTSYSTEM
046a
1130
UDM_GETBUDDY
046b
1131
EM_GETIMEOPTIONS
046b
1131
PSM_CANCELTOCLOSE
046b
1131
UDM_SETACCEL
046c
1132
EM_CONVPOSITION
046c
1132
EM_CONVPOSITION
046c
1132
PSM_QUERYSIBLINGS
046c
1132
UDM_GETACCEL
046d
1133
MCIWNDM_GETZOOM
046d
1133
PSM_UNCHANGED
046d
1133
UDM_SETBASE
046e
1134
PSM_APPLY
046e
1134
UDM_GETBASE
046f
1135
PSM_SETTITLEA
046f
1135
UDM_SETRANGE32
0470
1136
PSM_SETWIZBUTTONS
0470
1136
UDM_GETRANGE32
0470
1136
WM_CAP_DRIVER_GET_NAMEW
0471
1137
PSM_PRESSBUTTON
0471
1137
UDM_SETPOS32
0471
1137
WM_CAP_DRIVER_GET_VERSIONW
0472
1138
PSM_SETCURSELID
0472
1138
UDM_GETPOS32
0473
1139
PSM_SETFINISHTEXTA
0474
1140
PSM_GETTABCONTROL
0475
1141
PSM_ISDIALOGMESSAGE
0476
1142
MCIWNDM_REALIZE
0476
1142
PSM_GETCURRENTPAGEHWND
0477
1143
MCIWNDM_SETTIMEFORMATA
0477
1143
PSM_INSERTPAGE
0478
1144
EM_SETLANGOPTIONS
0478
1144
MCIWNDM_GETTIMEFORMATA
0478
1144
PSM_SETTITLEW
0478
1144
WM_CAP_FILE_SET_CAPTURE_FILEW
0479
1145
EM_GETLANGOPTIONS
0479
1145
MCIWNDM_VALIDATEMEDIA
0479
1145
PSM_SETFINISHTEXTW
0479
1145
WM_CAP_FILE_GET_CAPTURE_FILEW
047a
1146
EM_GETIMECOMPMODE
047b
1147
EM_FINDTEXTW
047b
1147
MCIWNDM_PLAYTO
047b
1147
WM_CAP_FILE_SAVEASW
047c
1148
EM_FINDTEXTEXW
047c
1148
MCIWNDM_GETFILENAMEA
047d
1149
EM_RECONVERSION
047d
1149
MCIWNDM_GETDEVICEA
047d
1149
PSM_SETHEADERTITLEA
047d
1149
WM_CAP_FILE_SAVEDIBW
047e
1150
EM_SETIMEMODEBIAS
047e
1150
MCIWNDM_GETPALETTE
047e
1150
PSM_SETHEADERTITLEW
047f
1151
EM_GETIMEMODEBIAS
047f
1151
MCIWNDM_SETPALETTE
047f
1151
PSM_SETHEADERSUBTITLEA
0480
1152
MCIWNDM_GETERRORA
0480
1152
PSM_SETHEADERSUBTITLEW
0481
1153
PSM_HWNDTOINDEX
0482
1154
PSM_INDEXTOHWND
0483
1155
MCIWNDM_SETINACTIVETIMER
0483
1155
PSM_PAGETOINDEX
0484
1156
PSM_INDEXTOPAGE
0485
1157
DL_BEGINDRAG
0485
1157
MCIWNDM_GETINACTIVETIMER
0485
1157
PSM_IDTOINDEX
0486
1158
DL_DRAGGING
0486
1158
PSM_INDEXTOID
0487
1159
DL_DROPPED
0487
1159
PSM_GETRESULT
0488
1160
DL_CANCELDRAG
0488
1160
PSM_RECALCPAGESIZES
048c
1164
MCIWNDM_GET_SOURCE
048d
1165
MCIWNDM_PUT_SOURCE
048e
1166
MCIWNDM_GET_DEST
048f
1167
MCIWNDM_PUT_DEST
0490
1168
MCIWNDM_CAN_PLAY
0491
1169
MCIWNDM_CAN_WINDOW
0492
1170
MCIWNDM_CAN_RECORD
0493
1171
MCIWNDM_CAN_SAVE
0494
1172
MCIWNDM_CAN_EJECT
0495
1173
MCIWNDM_CAN_CONFIG
0496
1174
IE_GETINK
0496
1174
IE_MSGFIRST
0496
1174
MCIWNDM_PALETTEKICK
0497
1175
IE_SETINK
0498
1176
IE_GETPENTIP
0499
1177
IE_SETPENTIP
049a
1178
IE_GETERASERTIP
049b
1179
IE_SETERASERTIP
049c
1180
IE_GETBKGND
049d
1181
IE_SETBKGND
049e
1182
IE_GETGRIDORIGIN
049f
1183
IE_SETGRIDORIGIN
04a0
1184
IE_GETGRIDPEN
04a1
1185
IE_SETGRIDPEN
04a2
1186
IE_GETGRIDSIZE
04a3
1187
IE_SETGRIDSIZE
04a4
1188
IE_GETMODE
04a5
1189
IE_SETMODE
04a6
1190
IE_GETINKRECT
04a6
1190
WM_CAP_SET_MCI_DEVICEW
04a7
1191
WM_CAP_GET_MCI_DEVICEW
04b4
1204
WM_CAP_PAL_OPENW
04b5
1205
WM_CAP_PAL_SAVEW
04b8
1208
IE_GETAPPDATA
04b9
1209
IE_SETAPPDATA
04ba
1210
IE_GETDRAWOPTS
04bb
1211
IE_SETDRAWOPTS
04bc
1212
IE_GETFORMAT
04bd
1213
IE_SETFORMAT
04be
1214
IE_GETINKINPUT
04bf
1215
IE_SETINKINPUT
04c0
1216
IE_GETNOTIFY
04c1
1217
IE_SETNOTIFY
04c2
1218
IE_GETRECOG
04c3
1219
IE_SETRECOG
04c4
1220
IE_GETSECURITY
04c5
1221
IE_SETSECURITY
04c6
1222
IE_GETSEL
04c7
1223
IE_SETSEL
04c8
1224
CDM_LAST
04c8
1224
EM_SETBIDIOPTIONS
04c8
1224
IE_DOCOMMAND
04c8
1224
MCIWNDM_NOTIFYMODE
04c9
1225
EM_GETBIDIOPTIONS
04c9
1225
IE_GETCOMMAND
04ca
1226
EM_SETTYPOGRAPHYOPTIONS
04ca
1226
IE_GETCOUNT
04cb
1227
EM_GETTYPOGRAPHYOPTIONS
04cb
1227
IE_GETGESTURE
04cb
1227
MCIWNDM_NOTIFYMEDIA
04cc
1228
EM_SETEDITSTYLE
04cc
1228
IE_GETMENU
04cd
1229
EM_GETEDITSTYLE
04cd
1229
IE_GETPAINTDC
04cd
1229
MCIWNDM_NOTIFYERROR
04ce
1230
IE_GETPDEVENT
04cf
1231
IE_GETSELCOUNT
04d0
1232
IE_GETSELITEMS
04d1
1233
IE_GETSTYLE
04db
1243
MCIWNDM_SETTIMEFORMATW
04dc
1244
EM_OUTLINE
04dc
1244
EM_OUTLINE
04dc
1244
MCIWNDM_GETTIMEFORMATW
04dd
1245
EM_GETSCROLLPOS
04dd
1245
EM_GETSCROLLPOS
04de
1246
EM_SETSCROLLPOS
04de
1246
EM_SETSCROLLPOS
04df
1247
EM_SETFONTSIZE
04df
1247
EM_SETFONTSIZE
04e0
1248
EM_GETZOOM
04e0
1248
MCIWNDM_GETFILENAMEW
04e1
1249
EM_SETZOOM
04e1
1249
MCIWNDM_GETDEVICEW
04e2
1250
EM_GETVIEWKIND
04e3
1251
EM_SETVIEWKIND
04e4
1252
EM_GETPAGE
04e4
1252
MCIWNDM_GETERRORW
04e5
1253
EM_SETPAGE
04e6
1254
EM_GETHYPHENATEINFO
04e7
1255
EM_SETHYPHENATEINFO
04eb
1259
EM_GETPAGEROTATE
04ec
1260
EM_SETPAGEROTATE
04ed
1261
EM_GETCTFMODEBIAS
04ee
1262
EM_SETCTFMODEBIAS
04f0
1264
EM_GETCTFOPENSTATE
04f1
1265
EM_SETCTFOPENSTATE
04f2
1266
EM_GETIMECOMPTEXT
04f3
1267
EM_ISIME
04f4
1268
EM_GETIMEPROPERTY
050d
1293
EM_GETQUERYRTFOBJ
050e
1294
EM_SETQUERYRTFOBJ
0600
1536
FM_GETFOCUS
0601
1537
FM_GETDRIVEINFOA
0602
1538
FM_GETSELCOUNT
0603
1539
FM_GETSELCOUNTLFN
0604
1540
FM_GETFILESELA
0605
1541
FM_GETFILESELLFNA
0606
1542
FM_REFRESH_WINDOWS
0607
1543
FM_RELOAD_EXTENSIONS
0611
1553
FM_GETDRIVEINFOW
0614
1556
FM_GETFILESELW
0615
1557
FM_GETFILESELLFNW
0659
1625
WLX_WM_SAS
07e8
2024
SM_GETSELCOUNT
07e8
2024
UM_GETSELCOUNT
07e8
2024
WM_CPL_LAUNCH
07e9
2025
SM_GETSERVERSELA
07e9
2025
UM_GETUSERSELA
07e9
2025
WM_CPL_LAUNCHED
07ea
2026
SM_GETSERVERSELW
07ea
2026
UM_GETUSERSELW
07eb
2027
SM_GETCURFOCUSA
07eb
2027
UM_GETGROUPSELA
07ec
2028
SM_GETCURFOCUSW
07ec
2028
UM_GETGROUPSELW
07ed
2029
SM_GETOPTIONS
07ed
2029
UM_GETCURFOCUSA
07ee
2030
UM_GETCURFOCUSW
07ef
2031
UM_GETOPTIONS
07f0
2032
UM_GETOPTIONS2
1000
4096
LVM_FIRST
1000
4096
LVM_GETBKCOLOR
1001
4097
LVM_SETBKCOLOR
1002
4098
LVM_GETIMAGELIST
1003
4099
LVM_SETIMAGELIST
1004
4100
LVM_GETITEMCOUNT
1005
4101
LVM_GETITEMA
1006
4102
LVM_SETITEMA
1007
4103
LVM_INSERTITEMA
1008
4104
LVM_DELETEITEM
1009
4105
LVM_DELETEALLITEMS
100a
4106
LVM_GETCALLBACKMASK
100b
4107
LVM_SETCALLBACKMASK
100c
4108
LVM_GETNEXTITEM
100d
4109
LVM_FINDITEMA
100e
4110
LVM_GETITEMRECT
100f
4111
LVM_SETITEMPOSITION
1010
4112
LVM_GETITEMPOSITION
1011
4113
LVM_GETSTRINGWIDTHA
1012
4114
LVM_HITTEST
1013
4115
LVM_ENSUREVISIBLE
1014
4116
LVM_SCROLL
1015
4117
LVM_REDRAWITEMS
1016
4118
LVM_ARRANGE
1017
4119
LVM_EDITLABELA
1018
4120
LVM_GETEDITCONTROL
1019
4121
LVM_GETCOLUMNA
101a
4122
LVM_SETCOLUMNA
101b
4123
LVM_INSERTCOLUMNA
101c
4124
LVM_DELETECOLUMN
101d
4125
LVM_GETCOLUMNWIDTH
101e
4126
LVM_SETCOLUMNWIDTH
101f
4127
LVM_GETHEADER
1021
4129
LVM_CREATEDRAGIMAGE
1022
4130
LVM_GETVIEWRECT
1023
4131
LVM_GETTEXTCOLOR
1024
4132
LVM_SETTEXTCOLOR
1025
4133
LVM_GETTEXTBKCOLOR
1026
4134
LVM_SETTEXTBKCOLOR
1027
4135
LVM_GETTOPINDEX
1028
4136
LVM_GETCOUNTPERPAGE
1029
4137
LVM_GETORIGIN
102a
4138
LVM_UPDATE
102b
4139
LVM_SETITEMSTATE
102c
4140
LVM_GETITEMSTATE
102d
4141
LVM_GETITEMTEXTA
102e
4142
LVM_SETITEMTEXTA
102f
4143
LVM_SETITEMCOUNT
1030
4144
LVM_SORTITEMS
1031
4145
LVM_SETITEMPOSITION32
1032
4146
LVM_GETSELECTEDCOUNT
1033
4147
LVM_GETITEMSPACING
1034
4148
LVM_GETISEARCHSTRINGA
1035
4149
LVM_SETICONSPACING
1036
4150
LVM_SETEXTENDEDLISTVIEWSTYLE
1037
4151
LVM_GETEXTENDEDLISTVIEWSTYLE
1038
4152
LVM_GETSUBITEMRECT
1039
4153
LVM_SUBITEMHITTEST
103a
4154
LVM_SETCOLUMNORDERARRAY
103b
4155
LVM_GETCOLUMNORDERARRAY
103c
4156
LVM_SETHOTITEM
103d
4157
LVM_GETHOTITEM
103e
4158
LVM_SETHOTCURSOR
103f
4159
LVM_GETHOTCURSOR
1040
4160
LVM_APPROXIMATEVIEWRECT
1041
4161
LVM_SETWORKAREAS
1042
4162
LVM_GETSELECTIONMARK
1043
4163
LVM_SETSELECTIONMARK
1044
4164
LVM_SETBKIMAGEA
1045
4165
LVM_GETBKIMAGEA
1046
4166
LVM_GETWORKAREAS
1047
4167
LVM_SETHOVERTIME
1048
4168
LVM_GETHOVERTIME
1049
4169
LVM_GETNUMBEROFWORKAREAS
104a
4170
LVM_SETTOOLTIPS
104b
4171
LVM_GETITEMW
104c
4172
LVM_SETITEMW
104d
4173
LVM_INSERTITEMW
104e
4174
LVM_GETTOOLTIPS
1053
4179
LVM_FINDITEMW
1057
4183
LVM_GETSTRINGWIDTHW
105f
4191
LVM_GETCOLUMNW
1060
4192
LVM_SETCOLUMNW
1061
4193
LVM_INSERTCOLUMNW
1073
4211
LVM_GETITEMTEXTW
1074
4212
LVM_SETITEMTEXTW
1075
4213
LVM_GETISEARCHSTRINGW
1076
4214
LVM_EDITLABELW
108b
4235
LVM_GETBKIMAGEW
108c
4236
LVM_SETSELECTEDCOLUMN
108d
4237
LVM_SETTILEWIDTH
108e
4238
LVM_SETVIEW
108f
4239
LVM_GETVIEW
1091
4241
LVM_INSERTGROUP
1093
4243
LVM_SETGROUPINFO
1095
4245
LVM_GETGROUPINFO
1096
4246
LVM_REMOVEGROUP
1097
4247
LVM_MOVEGROUP
109a
4250
LVM_MOVEITEMTOGROUP
109b
4251
LVM_SETGROUPMETRICS
109c
4252
LVM_GETGROUPMETRICS
109d
4253
LVM_ENABLEGROUPVIEW
109e
4254
LVM_SORTGROUPS
109f
4255
LVM_INSERTGROUPSORTED
10a0
4256
LVM_REMOVEALLGROUPS
10a1
4257
LVM_HASGROUP
10a2
4258
LVM_SETTILEVIEWINFO
10a3
4259
LVM_GETTILEVIEWINFO
10a4
4260
LVM_SETTILEINFO
10a5
4261
LVM_GETTILEINFO
10a6
4262
LVM_SETINSERTMARK
10a7
4263
LVM_GETINSERTMARK
10a8
4264
LVM_INSERTMARKHITTEST
10a9
4265
LVM_GETINSERTMARKRECT
10aa
4266
LVM_SETINSERTMARKCOLOR
10ab
4267
LVM_GETINSERTMARKCOLOR
10ad
4269
LVM_SETINFOTIP
10ae
4270
LVM_GETSELECTEDCOLUMN
10af
4271
LVM_ISGROUPVIEWENABLED
10b0
4272
LVM_GETOUTLINECOLOR
10b1
4273
LVM_SETOUTLINECOLOR
10b3
4275
LVM_CANCELEDITLABEL
10b4
4276
LVM_MAPINDEXTOID
10b5
4277
LVM_MAPIDTOINDEX
10b6
4278
LVM_ISITEMVISIBLE
2000
8192
OCM__BASE
2005
8197
LVM_SETUNICODEFORMAT
2006
8198
LVM_GETUNICODEFORMAT
2019
8217
OCM_CTLCOLOR
202b
8235
OCM_DRAWITEM
202c
8236
OCM_MEASUREITEM
202d
8237
OCM_DELETEITEM
202e
8238
OCM_VKEYTOITEM
202f
8239
OCM_CHARTOITEM
2039
8249
OCM_COMPAREITEM
204e
8270
OCM_NOTIFY
2111
8465
OCM_COMMAND
2114
8468
OCM_HSCROLL
2115
8469
OCM_VSCROLL
2132
8498
OCM_CTLCOLORMSGBOX
2133
8499
OCM_CTLCOLOREDIT
2134
8500
OCM_CTLCOLORLISTBOX
2135
8501
OCM_CTLCOLORBTN
2136
8502
OCM_CTLCOLORDLG
2137
8503
OCM_CTLCOLORSCROLLBAR
2138
8504
OCM_CTLCOLORSTATIC
2210
8720
OCM_PARENTNOTIFY
8000
32768
WM_APP
cccd
52429
WM_RASDIALEVENT
*/
                   
                    
                  

// 任务按钮创建消息
const UINT LongUI::UIWindow::s_uTaskbarBtnCreatedMsg = ::RegisterWindowMessageW(L"TaskbarButtonCreated");

/// <summary>
/// Initializes a new instance of the <see cref="UIWindow"/> class.
/// </summary>
/// <param name="parent">The parent for self in window-level</param>
LongUI::UIWindow::UIWindow(UIWindow* parent) noexcept : Super(nullptr), m_uiRenderQueue(this), wndparent(parent) {
    std::memset(&m_curMedium, 0, sizeof(m_curMedium));
}

/// <summary>
/// Initializes with specified xml-node.
/// </summary>
/// <param name="node">The xml node.</param>
/// <returns></returns>
void LongUI::UIWindow::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    // 初始化
    assert(node && "<LongUI::UIWindow::initialize> window_node null");
    std::memset(&m_curMedium, 0, sizeof(m_curMedium));
    // 检查名称
    {
        auto basestr = node.attribute(LongUI::XMLAttribute::ControlName).value();
        if (basestr) {
            auto namestr = this->CopyStringSafe(basestr);
            force_cast(this->name) = namestr;
        }
    }
    CUIString titlename;
    titlename.Set(this->name.c_str());
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
        force_cast(this->wndflags) = flag;
        // XXX:
        force_cast(this->wnd_type) = Type_Layered;
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
        m_textAntiMode = uint16_t(Helper::GetEnumFromXml(node, D2D1_TEXT_ANTIALIAS_MODE_DEFAULT));
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
            WS_EX_NOREDIRECTIONBITMAP,
            LongUI::WindowClassNameA, 
            titlename.length() ? titlename.c_str() : L"LongUI",
            window_style,
            window_rect.left, window_rect.top, window_rect.right, window_rect.bottom,
            this->wndparent ? this->wndparent->GetHwnd() : nullptr,
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
            UIManager.ShowError(L"Error! Failed to Create Window", __FUNCTIONW__);
            return;
        }
    }
    //SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    // 设置Hover
    m_csTME.cbSize = sizeof(m_csTME);
    m_csTME.dwFlags = TME_HOVER | TME_LEAVE;
    m_csTME.hwndTrack = m_hwnd;
    m_csTME.dwHoverTime = 0;
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
    // 清零
    std::memset(m_dirtyRects, 0, sizeof(m_dirtyRects));
    // 自动显示窗口
    if (node.attribute("autoshow").as_bool(true)) {
        ::ShowWindow(m_hwnd, SW_SHOW);
    }
}

// UIWindow 析构函数
LongUI::UIWindow::~UIWindow() noexcept {
    // 设置窗口指针
    ::SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, LONG_PTR(0));
    // 解锁
    UIManager.Unlock();
    {
        // 取消注册
        ::RevokeDragDrop(m_hwnd);
        // 杀掉!
        ::KillTimer(m_hwnd, m_idBlinkTimer);
        // 释放资源
        this->release_data();
        // 释放数据
        LongUI::SafeRelease(m_pTaskBarList);
        LongUI::SafeRelease(m_pDropTargetHelper);
        LongUI::SafeRelease(m_pCurDataObject);
        // 关闭
        this->CloseWindowLater();
    }
    // 加锁
    UIManager.Lock();
    // 移除窗口
    UIManager.RemoveWindow(this);
}

// 移除控件引用
void LongUI::UIWindow::RemoveControlReference(UIControl * ctrl) noexcept {
    auto remove_reference = [ctrl](UIControl*& cref) { 
        if (cref == ctrl) cref = nullptr; 
    };
    // 移除引用
    remove_reference(m_pHoverTracked);
    remove_reference(m_pFocusedControl);
    remove_reference(m_pDragDropControl);
    remove_reference(m_pCapturedControl);
}


// 注册
void LongUI::UIWindow::RegisterOffScreenRender(UIControl* c, bool is3d) noexcept {
    // 检查
#ifdef _DEBUG
    auto itr = std::find(m_vRegisteredControl.begin(), m_vRegisteredControl.end(), c);
    if (itr != m_vRegisteredControl.end()) {
        UIManager << DL_Warning << L"control: [" << c->name << L"] existed" << LongUI::endl;
        return;
    }
#endif
    if (is3d) {
        m_vRegisteredControl.insert(m_vRegisteredControl.begin(), c);
    }
    else {
        m_vRegisteredControl.push_back(c);
    }
    // some error
    if (!m_vRegisteredControl.isok()) {
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
        UIManager << DL_Warning << L"control: [" << c->name << L"] not found" << LongUI::endl;
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
    if (this->debug_this || ctrl->debug_this) {
        UIManager << DL_Log << ctrl
            << LongUI::Formated(L"(%.1f, %.1f)", pt.x, pt.y)
            << LongUI::endl;
    }
#endif
    m_baBoolWindow.SetTrue(Index_CaretIn);
    m_baBoolWindow.SetTrue(Index_DoCaret);
    const auto intx = static_cast<LONG>(pt.x);
    const auto inty = static_cast<LONG>(pt.y);
    const auto oldx = static_cast<LONG>(m_rcCaretPx.left);
    const auto oldy = static_cast<LONG>(m_rcCaretPx.top);
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
    //::ShowCaret(m_hwnd);
    ++m_cShowCaret;
    // 创建AE位图
    //if (!m_pd2dBitmapAE) {
        //this->recreate_ae_bitmap();
    //}
}

// 异常插入符号
void LongUI::UIWindow::HideCaret() noexcept { 
    //::HideCaret(m_hwnd);
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
auto LongUI::UIWindow::FindControl(const char* cname) noexcept -> UIControl * {
    // 查找控件
    auto result = m_hashStr2Ctrl.Find(cname);
    // 未找到返回空
    if (!result) {
        // 给予警告
        UIManager << DL_Warning << L" Control Not Found: " << cname << LongUI::endl;
        return nullptr;
    }
    else {
        return reinterpret_cast<LongUI::UIControl*>(*result);
    }
}

// 添加命名控件
void LongUI::UIWindow::AddNamedControl(UIControl* ctrl) noexcept {
    assert(ctrl && "bad argumrnt");
    const auto cname = ctrl->name.c_str();
    // 有效
    if (cname[0]) {
        // 插入
        if(!m_hashStr2Ctrl.Insert(cname, ctrl)) {
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
        ::SetEvent(m_hVSync);
        ::CloseHandle(m_hVSync);
        m_hVSync = nullptr;
    }
    // 释放资源
    LongUI::SafeRelease(m_pTargetBimtap);
    LongUI::SafeRelease(m_pSwapChain);
    LongUI::SafeRelease(m_pDcompDevice);
    LongUI::SafeRelease(m_pDcompTarget);
    LongUI::SafeRelease(m_pDcompVisual);
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
            std::memcpy(s_rects, present.pDirtyRects, present.DirtyRectsCount * sizeof(RECT));
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
    for (auto ctrl : m_vRegisteredControl) {
        assert(!"NOIMPL");
        assert(ctrl->parent && "check it");
        //UIManager_RenderTarget->SetTransform(&ctrl->parent->world);
        //ctrl->Render(RenderType::Type_RenderOffScreen);
    }
    // 设为当前渲染对象
    UIManager_RenderTarget->SetTarget(m_pTargetBimtap);
    // 开始渲染
    UIManager_RenderTarget->BeginDraw();
    // 设置转换矩阵
#if 0
    UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
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
    std::memcpy(dirtyRects, m_dirtyRects, sizeof(dirtyRects));
    DXGI_PRESENT_PARAMETERS present_parameters;
    present_parameters.DirtyRectsCount = 0;
    present_parameters.pDirtyRects = dirtyRects;
    present_parameters.pScrollRect = &rcScroll;
    present_parameters.pScrollOffset = nullptr;
    // 设置参数
    this->set_present_parameters(present_parameters);
    // 呈现
    HRESULT hr = m_pSwapChain->Present1(1, 0, &present_parameters);
    longui_debug_hr(hr, L"m_pSwapChain->Present1 faild");
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
    ShowHR(hr);
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
        std::memcpy(m_aUnitNow.units, current_unit->units, sizeof(*m_aUnitNow.units) * m_aUnitNow.length);
    }
    // 刷新前
    if (this->IsControlLayoutChanged()) {
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
            ctrl->AfterUpdate();
            // 限制转换
            m_dirtyRects[i].left = static_cast<LONG>(ctrl->visible_rect.left);
            m_dirtyRects[i].top = static_cast<LONG>(ctrl->visible_rect.top);
            m_dirtyRects[i].right = static_cast<LONG>(std::ceil(ctrl->visible_rect.right));
            m_dirtyRects[i].bottom = static_cast<LONG>(std::ceil(ctrl->visible_rect.bottom));
        }
        // control update
        UIControl::Update();
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
    this->AfterUpdate();
}

// UIWindow 渲染 
void LongUI::UIWindow::Render() const noexcept  {
    // 全刷新: 继承父类
    if (m_baBoolWindow.Test(Index_FullRenderingThisFrame)) {
        Super::Render();
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
            std::memcpy(units, m_aUnitNow.units, length_for_units * sizeof(void*));
        }
        // 再渲染
        auto init_transfrom = DX::Matrix3x2F::Identity();
        for (auto unit = units; unit < units + length_for_units; ++unit) {
            auto ctrl = *unit; assert(ctrl != this);
            UIManager_RenderTarget->SetTransform(&ctrl->world);
            D2D1_POINT_2F clipr[2];
            clipr[0] = LongUI::TransformPointInverse(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(ctrl->visible_rect.left));
            clipr[1] = LongUI::TransformPointInverse(ctrl->world, reinterpret_cast<D2D1_POINT_2F&>(ctrl->visible_rect.right));
            UIManager_RenderTarget->PushAxisAlignedClip(reinterpret_cast<D2D1_RECT_F*>(clipr), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#if 0 // def _DEBUG
            if (this->debug_this) {
                AutoLocker;
                UIManager << DL_Log << "RENDER: " << ctrl << LongUI::endl;
            }
#endif
            // 正常渲染
            ctrl->Render();
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
        UIManager_RenderTarget->SetTransform(DX::Matrix3x2F::Identity());
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
    // 调试输出
#ifdef _DEBUG
    if (this->debug_show) {
        D2D1_MATRIX_3X2_F nowMatrix, iMatrix = DX::Matrix3x2F::Scale(0.45f, 0.45f);
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
        LongUI::SafeRelease(tf);
        UIManager_RenderTarget->SetTransform(&nowMatrix);
    }
#endif
}

// UIWindow 事件处理
bool LongUI::UIWindow::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // 有了匿名函数妈妈再也不用担心一条函数有N行了
    // -------------------- On  Timer   ------------
    auto on_timer = [this](WPARAM wParam) {
        // 小于1K认为是常量数据
        if (static_cast<UINT_PTR>(wParam) < 1024) {
            // 闪烁?
            if (wParam == BLINK_EVENT_ID) {
                if (m_cShowCaret) {
                    m_baBoolWindow.SetNot(Index_CaretIn);
                    m_baBoolWindow.SetTrue(Index_DoCaret);
                }
                return true;
            }
            return false;
        }
        // 大于1K认为是指针
        else {
            assert((wParam & 3) == 0 && "bad action");
            auto ctrl = reinterpret_cast<UIControl*>(wParam);
            return ctrl->DoLongUIEvent(Event::Event_Timer);
        }
    };
    // -------------------- Main DoEvent------------
    // 这里就不处理LongUI事件了 交给父类吧
    if (arg.sender) return Super::DoEvent(arg);
    // 其他LongUI事件
    bool handled = false;
    // 特殊事件
    if (arg.msg == s_uTaskbarBtnCreatedMsg) {
        LongUI::SafeRelease(m_pTaskBarList);
        UIManager << DL_Log << "TaskbarButtonCreated" << endl;
        auto hr = ::CoCreateInstance(
            CLSID_TaskbarList,
            nullptr,
            CLSCTX_INPROC_SERVER,
            LongUI_IID_PV_ARGS(m_pTaskBarList)
            );
        ShowHR(hr);
        return true;
    }
    // 处理事件
    switch (arg.msg)
    {
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
    case WM_TIMER:
        handled = on_timer(arg.sys.wParam);
        break;
    /*case WM_NCHITTEST:
        arg.lr = HTCAPTION;
        handled = true;
        break;*/
    case WM_SETFOCUS:
        ::CreateCaret(m_hwnd, nullptr, 1, 1);
        handled = true;
        break;
    case WM_KILLFOCUS:
        // 存在焦点控件
        if (m_pFocusedControl){
            force_cast(arg.sender) = this;
            force_cast(arg.event) = LongUI::Event::Event_KillFocus;
            m_pFocusedControl->DoEvent(arg);
            m_pFocusedControl = nullptr;
            force_cast(arg.sender) = nullptr;
            force_cast(arg.msg) = WM_KILLFOCUS;
        }
        ::DestroyCaret();
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
        reinterpret_cast<MINMAXINFO*>(arg.sys.lParam)->ptMinTrackSize.x = m_miniSize.width;
        reinterpret_cast<MINMAXINFO*>(arg.sys.lParam)->ptMinTrackSize.y = m_miniSize.height;
        break;
    case WM_DISPLAYCHANGE:
        UIManager << DL_Hint << "WM_DISPLAYCHANGE" << endl;
        {
            // 获取屏幕刷新率
            auto old = UIManager.GetDisplayFrequency();
            UIManager.RefreshDisplayFrequency();
            auto now = UIManager.GetDisplayFrequency();
            if (old != now) {
                m_uiRenderQueue.Reset(static_cast<uint32_t>(now));
            }
        }
        // 强行刷新一帧
        this->Invalidate(this);
        break;
    case WM_CLOSE:
        // 窗口关闭
        handled = this->OnClose();
        break;
    }
    // 处理
    if (handled) return true;
    // 有就处理
    if (m_pFocusedControl && m_pFocusedControl->DoEvent(arg)) {
        return true;
    }
    // 还是没有处理就交给父类处理
    return Super::DoEvent(arg);
}

// 鼠标事件
bool LongUI::UIWindow::DoMouseEvent(const MouseEventArgument& arg) noexcept {
    // hover跟踪
    if (arg.event == MouseEvent::Event_MouseHover && m_pHoverTracked) {
        return m_pHoverTracked->DoMouseEvent(arg);
    }
    // 存在捕获控件
    if (m_pCapturedControl) {
        return m_pCapturedControl->DoMouseEvent(arg);
    }
    // 父类
    auto code = Super::DoMouseEvent(arg);
    // 设置跟踪
    if (arg.event == MouseEvent::Event_MouseMove) {
        m_csTME.dwHoverTime = m_pHoverTracked ? DWORD(m_pHoverTracked->GetHoverTrackTime()) : DWORD(0);
        ::TrackMouseEvent(&m_csTME);
    }
    return code;
}


// 设置鼠标焦点
void LongUI::UIWindow::SetFocus(UIControl* ctrl) noexcept {
    // 无效
    assert(ctrl && "bad argument");
    // 可聚焦的
    if (ctrl && ctrl->flags & Flag_Focusable) {
        // 有效
        if (m_pFocusedControl) {
            m_pFocusedControl->DoLongUIEvent(Event::Event_KillFocus);
        }
        // 有效
        if ((m_pFocusedControl = ctrl)) {
            m_pFocusedControl->DoLongUIEvent(Event::Event_SetFocus);
        }
    }
}

// 重置窗口大小
void LongUI::UIWindow::OnResize(bool force) noexcept {
    assert(this->wnd_type != Type_RenderOnParent);
    if (this->wnd_type != Type_Layered) {
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
    HRESULT hr = S_OK;
    // 强行 或者 小于才Resize
    if (force || old_size.width < uint32_t(rect_right) || old_size.height < uint32_t(rect_bottom)) {
        UIManager << DL_Hint << L"Window: [" 
            << this->name 
            << L"]\tTarget Bitmap Resize to " 
            << LongUI::Formated(L"(%d, %d)", int(rect_right), int(rect_bottom)) 
            << LongUI::endl;
        IDXGISurface* pDxgiBackBuffer = nullptr;
        LongUI::SafeRelease(m_pTargetBimtap);
        hr = m_pSwapChain->ResizeBuffers(
            2, rect_right, rect_bottom, DXGI_FORMAT_B8G8R8A8_UNORM, 
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
            );
        longui_debug_hr(hr, L"m_pSwapChain->ResizeBuffers faild");
        // 检查
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            UIManager.RecreateResources();
            UIManager << DL_Hint << L"Recreate device" << LongUI::endl;
        }
        // 利用交换链获取Dxgi表面
        if (SUCCEEDED(hr)) {
            hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
            longui_debug_hr(hr, L"m_pSwapChain->GetBuffer faild");
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
            longui_debug_hr(hr, L"UIManager_RenderTarget->CreateBitmapFromDxgiSurface faild");
        }
        // 重建失败?
        if (FAILED(hr)) {
            UIManager << DL_Error << L" Recreate FAILED!" << LongUI::endl;
            ShowHR(hr);
        }
        LongUI::SafeRelease(pDxgiBackBuffer);
    }
    // 强行刷新一帧
    this->Invalidate(this);
}

// UIWindow 重建
auto LongUI::UIWindow::Recreate() noexcept ->HRESULT {
    // 跳过
    if (m_baBoolWindow.Test(Index_SkipRender)) return S_OK;
    // 释放数据
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
        if (this->wnd_type == Type_Layered) {
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
            longui_debug_hr(hr, L"UIManager_DXGIFactory->CreateSwapChainForComposition faild");
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
            longui_debug_hr(hr, L"UIManager_DXGIFactory->CreateSwapChainForHwnd faild");
        }
    }
    // 获取交换链V2
    if (SUCCEEDED(hr)) {
        hr = pSwapChain->QueryInterface(
            LongUI::IID_IDXGISwapChain2,
            reinterpret_cast<void**>(&m_pSwapChain)
            );
        longui_debug_hr(hr, L"pSwapChain->QueryInterface LongUI::IID_IDXGISwapChain2 faild");
    }
    // 获取垂直等待事件
    if (SUCCEEDED(hr)) {
        m_hVSync = m_pSwapChain->GetFrameLatencyWaitableObject();
    }
    /*// 确保DXGI队列里边不会超过一帧
    if (SUCCEEDED(hr)) {
        hr = UIManager_DXGIDevice->SetMaximumFrameLatency(1);
        longui_debug_hr(hr, L"UIManager_DXGIDevice->SetMaximumFrameLatency faild");
    }*/
    // 利用交换链获取Dxgi表面
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->GetBuffer(0, LongUI_IID_PV_ARGS(pDxgiBackBuffer));
        longui_debug_hr(hr, L"m_pSwapChain->GetBuffer faild");
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
        longui_debug_hr(hr, L"UIManager_RenderTarget->CreateBitmapFromDxgiSurface faild");
    }
    // 使用DComp
    if (this->wnd_type == Type_Layered) {
        // 创建直接组合(Direct Composition)设备
        if (SUCCEEDED(hr)) {
            hr = LongUI::Dll::DCompositionCreateDevice(
                UIManager_DXGIDevice,
                LongUI_IID_PV_ARGS(m_pDcompDevice)
                );
            longui_debug_hr(hr, L"DCompositionCreateDevice faild");
        }
        // 创建直接组合(Direct Composition)目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateTargetForHwnd(
                m_hwnd, true, &m_pDcompTarget
                );
            longui_debug_hr(hr, L"m_pDcompDevice->CreateTargetForHwnd faild");
        }
        // 创建直接组合(Direct Composition)视觉
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->CreateVisual(&m_pDcompVisual);
            longui_debug_hr(hr, L"m_pDcompDevice->CreateVisual faild");
        }
        // 设置当前交换链为视觉内容
        if (SUCCEEDED(hr)) {
            hr = m_pDcompVisual->SetContent(m_pSwapChain);
            longui_debug_hr(hr, L"m_pDcompVisual->SetContent faild");
        }
        // 设置当前视觉为窗口目标
        if (SUCCEEDED(hr)) {
            hr = m_pDcompTarget->SetRoot(m_pDcompVisual);
            longui_debug_hr(hr, L"m_pDcompTarget->SetRoot faild");
        }
        // 向系统提交
        if (SUCCEEDED(hr)) {
            hr = m_pDcompDevice->Commit();
            longui_debug_hr(hr, L"m_pDcompDevice->Commit faild");
        }
    }
    // 错误
    if (FAILED(hr)){
        UIManager << L"Recreate Failed!" << LongUI::endl;
        ShowHR(hr);
    }
    LongUI::SafeRelease(pDxgiBackBuffer);
    LongUI::SafeRelease(pSwapChain);
    {
        // 获取屏幕刷新率
        m_uiRenderQueue.Reset(UIManager.GetDisplayFrequency());
        // 强行刷新一帧
        this->Invalidate(this);
    }
    // 重建 子控件UI
    return Super::Recreate();
}

// UIWindow 关闭控件
void LongUI::UIWindow::cleanup() noexcept {
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
/*bool LongUI::UIWindow::OnMouseMove(const LongUI::EventArgument& arg) noexcept {
}

// 鼠标滚轮
bool LongUI::UIWindow::OnMouseWheel(const LongUI::EventArgument& arg) noexcept {
}*/

// ----------------- IDropTarget!!!! Yooooooooooo~-----

// 获取拖放效果
DWORD GetDropEffect(DWORD grfKeyState, DWORD dwAllowed) {
    DWORD dwEffect = 0;
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
        m_pFocusedControl->DoLongUIEvent(Event::Event_KillFocus);
        m_pFocusedControl = nullptr;
    }
    // 保留数据
    LongUI::SafeRelease(m_pCurDataObject);
    m_pCurDataObject = LongUI::SafeAcquire(pDataObj);
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
    UIControl* control; control = nullptr;
    // TODO:
    // 检查控件支持
    if (false) {
        /*LongUI::EventArgument arg;
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
        if (!control->DoEvent(arg)) *pdwEffect = DROPEFFECT_NONE;*/
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
        /*LongUI::EventArgument arg = { 0 };
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
        /*LongUI::EventArgument arg;
        ::SetLongUIEventArgument(arg, m_hwnd, pt);
        arg.sender = this;
        arg.event = LongUI::Event::Event_Drop;
        arg.cf.dataobj = m_pCurDataObject;
        arg.cf.outeffect = pdwEffect;
        // 发送事件
        m_pDragDropControl->DoEvent(arg);
        m_pDragDropControl = nullptr;*/
        
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

