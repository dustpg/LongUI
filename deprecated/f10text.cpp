#include "f10text.h"
#include "TextBC/bc_txtcell.h"
#include "TextBC/bc_txtdoc.h"

#include <QLabel>
#include <QSlider>
#include <QPainter>
#include <QLineEdit>
#include <QClipboard>
#include <QTextBlock>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#ifndef NDEBUG
#include <QDebug>
#endif

#include <cassert>

QWidget* create_txtedit(QWidget* w) {
    const auto container = new QWidget{ w };
    const auto mainl = new QHBoxLayout;
    const auto layout = new QVBoxLayout;
    const auto layout2 = new QVBoxLayout;
    const auto edit = new TcF10TextEdit;
    const auto display = new QLabel{};
    const auto button = new QPushButton{ "text" };
    const auto button2 = new QPushButton{ "set text" };
    const auto button3 = new QPushButton{ "undo stack" };
    const auto vslider = new QSlider{ Qt::Horizontal };
    const auto hslider = new QSlider{ Qt::Horizontal };
    //display->setStyleSheet(QLatin1String{ "color: white" });
    vslider->setMaximum(600); vslider->setMinimum(0); 
    vslider->setValue(vslider->maximum() / 2);

    hslider->setMaximum(vslider->maximum()); hslider->setMinimum(0);
    hslider->setValue(hslider->maximum() / 2);
    container->setLayout(mainl);
    mainl->addLayout(layout, 1);
    mainl->addLayout(layout2);
    {
        layout2->addWidget(button3);
    }

    layout->addWidget(vslider);
    layout->addWidget(hslider);
    layout->addWidget(edit, 1);
    layout->addWidget(display);
    layout->addWidget(button);
    layout->addWidget(button2);

    QObject::connect(hslider, &QSlider::valueChanged, [=](int a) {
        const auto pos = edit->GetViewPoint();
        const auto haf = vslider->maximum() / 2;
        edit->SetViewPoint({ pos.x, static_cast<float>(haf - a) });
    });
    QObject::connect(vslider, &QSlider::valueChanged, [=](int a) {
        const auto pos = edit->GetViewPoint();
        const auto haf = vslider->maximum() / 2;
        edit->SetViewPoint({ static_cast<float>(haf - a), pos.y });
    });
    QObject::connect(button, &QPushButton::clicked, [=]() {
        button->setText(edit->GetSelection());
    });
    QObject::connect(button2, &QPushButton::clicked, [=]() {
        edit->SetText(u8"文本测试#1 ——文本测试#2——");
    });
    QObject::connect(button3, &QPushButton::clicked, [=]() {
        edit->DebugOutputUndoStack();
    });
    //const auto doc = edit->GetDoc();
    QObject::connect(edit, &TcF10TextEdit::clicked, [=](TextBC::Point2F pt) {
        //const QPointF clicked{ pt.x, pt.y };
        const auto pos = edit->HitTest(pt);
        const auto text = QString::fromLatin1("click@[%1,%2] char@(%3+%4)")
            .arg(pt.x).arg(pt.y).arg(pos.pos).arg(pos.u16_trailing);
        display->setText(text);
        //const auto shift = QGuiApplication::keyboardModifiers() & Qt::ShiftModifier;
        //doc->SetSelection(pt, !!shift);
    });

    QObject::connect(edit, &TcF10TextEdit::returned, [=]() {
        button->setText(edit->RequestText());
    });
    return container;
}

/// <summary>
/// Initializes a new instance of the <see cref="TcF10TextEdit"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
TcF10TextEdit::TcF10TextEdit(QWidget * parent) : QWidget(parent) {
    this->setMinimumHeight(100);
    this->setFocusPolicy(Qt::WheelFocus);
    this->setAttribute(Qt::WA_InputMethodEnabled);
    this->setAttribute(Qt::WA_KeyCompression);
    constexpr auto flag = TextBC::CBCTextDocument::Flag_MultiLine;
    constexpr char16_t ch = 0x25CF;
    m_pDoc = new/*(std::nothrow)*/ TextBC::CBCTextDocument{ *this, {
        200, flag, ch 
    } };
}


/// <summary>
/// Errors the beep.
/// </summary>
/// <returns></returns>
void TcF10TextEdit::ErrorBeep() noexcept {
    QApplication::beep();
}


/// <summary>
/// Determines whether [is valid password] [the specified ].
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
bool TcF10TextEdit::IsValidPassword(char16_t ch) noexcept {
    // 自己按需求实现, 由于内部使用UTF-16, 使用char32_t有些麻烦
    return ch < 128 && QChar{ ch }.isPrint();
}

/// <summary>
/// Generates the text.
/// </summary>
/// <param name="string">The string.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
void TcF10TextEdit::GenerateText(void* string, TextBC::U16View view) noexcept {
    auto& text = *reinterpret_cast<QString*>(string);
    try {
        const auto len = view.second - view.first;
        const auto ptr = reinterpret_cast<const QChar*>(view.first);
        static_assert(sizeof(QChar) == sizeof(*view.first), "not utf-16?");
        text.append(ptr, len);
    }
    catch (...) {
        // OOM 处理
    }
}


/// <summary>
/// Finalizes an instance of the <see cref="TcF10TextEdit"/> class.
/// </summary>
TcF10TextEdit::~TcF10TextEdit() {
    delete m_pDoc;
}

/// <summary>
/// Needs the redraw.
/// </summary>
/// <returns></returns>
void TcF10TextEdit::NeedRedraw() noexcept {
    this->update();
}

/// <summary>
/// Hits the test.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto TcF10TextEdit::HitTest(TextBC::Point2F pos) -> TextBC::HitTest {
    return m_pDoc->HitTest(pos);
}

/// <summary>
/// Sets the selection.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="keep">if set to <c>true</c> [keep].</param>
/// <returns></returns>
void TcF10TextEdit::SetSelection(TextBC::Point2F pos, bool keep) noexcept {
    return m_pDoc->SetSelection(pos, keep);
}

/// <summary>
/// Requests the text.
/// </summary>
/// <returns></returns>
const QString& TcF10TextEdit::RequestText() noexcept {
    try {
        if (m_pDoc->IsTextChanged()) {
            m_pDoc->ClearTextChanged();
            m_text.clear();
            m_pDoc->RequestText(m_text);
        }
    }
    catch(...) {

    }
    return m_text;
}


/// <summary>
/// Gets the selection.
/// </summary>
/// <returns></returns>
auto TcF10TextEdit::GetSelection() const noexcept -> QString {
    QString text;
    m_pDoc->RequestSelected(text);
    return text;
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void TcF10TextEdit::SetText(const QString& text) noexcept {
    const auto ptr = reinterpret_cast<const char16_t*>(text.utf16());
    const auto len = text.length();
    m_pDoc->SetText({ ptr, ptr + len });
}


/// <summary>
/// Debugs the output undo stack.
/// </summary>
/// <returns></returns>
void TcF10TextEdit::DebugOutputUndoStack() noexcept {
    m_pDoc->DebugOutUndoStack();
}

/// <summary>
/// Sets the view point.
/// </summary>
/// <param name="pos">The position.</param>
/// <returns></returns>
void TcF10TextEdit::SetViewPoint(TextBC::Point2F pos) noexcept {
    m_pDoc->SetViewportPos(pos);
}

/// <summary>
/// Gets the view point.
/// </summary>
/// <returns></returns>
auto TcF10TextEdit::GetViewPoint() const noexcept -> TextBC::Point2F {
    return m_pDoc->GetViewportPos();
}

/// <summary>
/// Draws the caret.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="offset">The offset.</param>
/// <param name="rect">The rect.</param>
/// <returns></returns>
void TcF10TextEdit::DrawCaret(void* ctx, TextBC::Point2F offset,
    const TextBC::RectWHF& rect) noexcept {
    assert(ctx && "bad context");
    // 正式渲染
    const auto painter = static_cast<QPainter*>(ctx);
    painter->setMatrix({});
    const auto x = rect.x + offset.x;
    const auto y = rect.y + offset.y;
    const auto w = rect.width;
    const auto h = rect.height;
    painter->fillRect(QRectF{ x, y, w, h }, QColor{ Qt::red });
}


/// <summary>
/// Draws the selection.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="offset">The offset.</param>
/// <param name="rects">The rects.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void TcF10TextEdit::DrawSelection(void* ctx, TextBC::Point2F offset, 
    const TextBC::RectWHF rects[], uint32_t len) noexcept {
    assert(ctx && "bad context");
    //static int count = 0;
    //qDebug() << ++count << len;
    // 正式渲染
    const auto painter = static_cast<QPainter*>(ctx);
    painter->setMatrix({});
    for (uint32_t i = 0; i != len; ++i) {
        const auto x = rects[i].x + offset.x;
        const auto y = rects[i].y + offset.y;
        const auto w = rects[i].width;
        const auto h = rects[i].height;
        painter->fillRect(QRectF{ x, y, w, h }, QColor{ 0x66ccff });
    }
}

/// <summary>
/// Paints the event.
/// </summary>
/// <param name="event">The event.</param>
void TcF10TextEdit::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter{ this };
#if 0
    painter.fillRect(this->rect(), Qt::white);
#else
    painter.fillRect(this->rect(), Qt::gray);
    const auto ctsize = m_pDoc->GetContentSize();
    const auto pos = m_pDoc->GetViewportPos();
    painter.fillRect(QRectF{ -pos.x, -pos.y, ctsize.width, ctsize.height }, Qt::white);
#endif
    m_pDoc->Render(&painter, nullptr);
}


/// <summary>
/// Resizes the event.
/// </summary>
/// <param name="event">The event.</param>
void TcF10TextEdit::resizeEvent(QResizeEvent* event) {
    const auto w = static_cast<float>(event->size().width());
    const auto h = static_cast<float>(event->size().height());
    m_pDoc->SetViewportSize({ w, h });
}

/// <summary>
/// Mouses the press event.
/// </summary>
/// <param name="event">The event.</param>
void TcF10TextEdit::mousePressEvent(QMouseEvent* event) {
    const TextBC::Point2F pt = {
        static_cast<float>(event->localPos().x()),
        static_cast<float>(event->localPos().y())
    };

    emit clicked(pt);

    if (event->button() & Qt::LeftButton) {
        const auto shift = QGuiApplication::keyboardModifiers() & Qt::ShiftModifier;
        m_pDoc->OnLButtonDown(pt, !!shift);
    }
}

/// <summary>
/// Mouses the release event.
/// </summary>
/// <param name="event">The event.</param>
void TcF10TextEdit::mouseReleaseEvent(QMouseEvent * event) {
    const TextBC::Point2F pt = {
        static_cast<float>(event->localPos().x()),
        static_cast<float>(event->localPos().y())
    };
    if (event->button() & Qt::LeftButton) {
        m_pDoc->OnLButtonUp(pt);
    }
}

/// <summary>
/// Mouses the move event.
/// </summary>
/// <param name="event">The event.</param>
void TcF10TextEdit::mouseMoveEvent(QMouseEvent * event) {
    const TextBC::Point2F pt = {
        static_cast<float>(event->localPos().x()),
        static_cast<float>(event->localPos().y())
    };
    if (event->buttons() & Qt::LeftButton) {
        m_pDoc->OnLButtonHold(pt);
    }
}


/// <summary>
/// Inputs the method event.
/// </summary>
/// <param name="event">The event.</param>
void TcF10TextEdit::inputMethodEvent(QInputMethodEvent* event) {
    //qDebug() << event->commitString();
    //if (event->commitString().isEmpty() 
    //    && event->preeditString().isEmpty() 
    //    && event->replacementLength() == 0)
    //    return;

}

/// <summary>
/// Keys the press event.
/// </summary>
/// <param name="e">The e.</param>
void TcF10TextEdit::keyPressEvent(QKeyEvent *e) {
    // 为空就算了
    const auto text = e->text();
    const auto doc = m_pDoc;
    // 不可视键
    const auto onNonPrintKey = [e, doc, this]() {
        // 复制
        const auto copy_to_clip = [doc]() {
            QString text;
            doc->RequestSelected(text);
            if (text.isEmpty()) return false;
            const auto clip = QApplication::clipboard();
            clip->setText(text);
            return true;
        };
        // 剪切
        const auto cut_to_clip = [copy_to_clip, doc]() {
            if (copy_to_clip()) {
                doc->DeleteSelection();
            }
        };
        // 粘贴
        const auto paste_from_clip = [doc]() {
            const auto clip = QApplication::clipboard();
            const auto text = clip->text();
            if (text.isEmpty()) return;
            const auto ptr = text.utf16();
            const auto u16 = reinterpret_cast<const char16_t*>(ptr);
            static_assert(sizeof(*u16) == sizeof(*ptr), "must be same");
            const auto len = text.length();
            doc->OnText({ u16, u16 + len });
        };
        // 检查按键
        const bool shift = !!(e->modifiers() & Qt::SHIFT);
        const bool ctrl = !!(e->modifiers() & Qt::CTRL);
        switch (e->key())
        {
        case Qt::Key_Backspace:
            doc->OnBackspace(ctrl);
            break;
        case Qt::Key_Delete:
            doc->OnDelete(ctrl);
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (ctrl || doc->OnNewLine())
                emit this->returned();
            break;
        case Qt::Key_Left:
            doc->OnLeft(ctrl, shift);
            break;
        case Qt::Key_Right:
            doc->OnRight(ctrl, shift);
            break;
        case Qt::Key_Up:
            doc->OnUp(shift);
            break;
        case Qt::Key_Down:
            doc->OnDown(shift);
            break;
        case Qt::Key_A:
            if (ctrl) doc->OnSelectAll();
            break;
        case Qt::Key_X:
            if (ctrl) cut_to_clip();
            break;
        case Qt::Key_C:
            if (ctrl) copy_to_clip();
            break;
        case Qt::Key_V:
            if (ctrl) paste_from_clip();
            break;
        case Qt::Key_Z:
            if (ctrl) doc->Undo();
            break;
        case Qt::Key_Y:
            if (ctrl) doc->Redo();
            break;
        default:
            break;
        }
    };
    // 可见按键
    const auto onPrintKey = [&text, doc]() {
        const auto str = text /*+ u8"乱"*/;
        //const auto str = QString::fromUtf16(u"𤭢");
        const auto ptr = reinterpret_cast<const char16_t*>(str.utf16());
        const auto len = str.length();
        doc->OnText({ ptr, ptr + len });
    };

    // 可见不可见
    !text.isEmpty() && text[0].isPrint()
        ? onPrintKey()
        : onNonPrintKey()
        ;
}


// auto_cast
QTextDocument* auto_cast(TextBC::IBCTextContent* ptr) noexcept {
    return reinterpret_cast<QTextDocument*>(ptr);
}

// auto_cast
TextBC::IBCTextContent* auto_cast(QTextDocument* ptr) noexcept {
    return reinterpret_cast<TextBC::IBCTextContent*>(ptr);
}

namespace TextBC {
    // is_surrogate
    inline bool IsSurrogate(uint16_t ch) noexcept { return ((ch) & 0xF800) == 0xD800; }
    // is_low_surrogate
    inline bool IsLowSurrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xDC00; }
    // is_high_surrogate
    inline bool IsHighSurrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xD800; }
}

/// <summary>
/// Creates the content.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <param name="old">The old.</param>
/// <returns></returns>
auto TcF10TextEdit::CreateContent(
    const char16_t* str, 
    uint32_t len, 
    Text&& old) noexcept -> Text* {
    auto ptr = auto_cast(&old);
    if (!ptr) ptr = new(std::nothrow) QTextDocument;
    if (!ptr) return nullptr;
    ptr->setDocumentMargin(0);
    ptr->setDefaultFont({ QLatin1String{ "Microsoft YaHei" } , 15 });
    ptr->setPlainText(QString::fromUtf16(str, len));
    return auto_cast(ptr);
}

/// <summary>
/// Deletes the content.
/// </summary>
/// <param name="obj">The object.</param>
/// <returns></returns>
void TcF10TextEdit::DeleteContent(Text& obj) noexcept {
    const auto ptr = auto_cast(&obj);
    delete ptr;
}


/// <summary>
/// Draws the content.
/// </summary>
/// <param name="obj">The object.</param>
/// <param name="ctx">The CTX.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
void TcF10TextEdit::DrawContent(Text& obj, void* ctx, TextBC::Point2F pos) noexcept {
    const auto ptr = auto_cast(&obj);
    assert(ctx);
    // TODO: 错误策略
    if (!ptr) return;
    // 正式渲染
    const auto painter = static_cast<QPainter*>(ctx);
    QMatrix matrix; 
    painter->setMatrix(matrix.translate(pos.x, pos.y));
    ptr->drawContents(painter);
}


/// <summary>
/// Contents the event.
/// </summary>
/// <param name="obj">The object.</param>
/// <param name="e">The e.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
void TcF10TextEdit::ContentEvent(Text& obj,
    MetricsEvent e, void* arg) noexcept {
    const auto ptr = auto_cast(&obj);
    switch (e)
    {
    case TextBC::IBCTextPlatform::Event_GetSize:
    {
        const auto size = ptr->size();
        static_cast<TextBC::SizeF*>(arg)->width = size.width();
        static_cast<TextBC::SizeF*>(arg)->height = size.height();
        break;
    }
    case TextBC::IBCTextPlatform::Event_GetBaseline:
    {
        QFontMetrics fm{ ptr->defaultFont() };
        *static_cast<float*>(arg) = static_cast<float>(fm.ascent());
        break;
    }
    case TextBC::IBCTextPlatform::Event_HitTest:
    {
        union ht_data { TextBC::HitTest ht; float pos; };
        const float x = reinterpret_cast<ht_data*>(arg)->pos;
        auto& out = reinterpret_cast<ht_data*>(arg)->ht;
        // 无效布局
        if (!ptr) {
            out.pos = 0;
            out.u16_trailing = 0;
            return;
        }
        const auto layout = ptr->documentLayout();
        // 进行检查
#if 0
        const auto pos = layout->hitTest({ x, 0 }, Qt::ExactHit);
        out.pos = pos < 0 ? layout->hitTest({ x, 0 }, Qt::FuzzyHit) : pos;
#else
        out.pos = layout->hitTest({ x, 0 }, Qt::FuzzyHit);
#endif
        const auto ch = ptr->characterAt(out.pos);
        uint32_t base_len = 1;
        // 32位表示字符
        if (TextBC::IsSurrogate(ch.unicode())) {
            // TODO: 高低位检查
            assert(TextBC::IsSurrogate(ptr->characterAt(out.pos + 1).unicode()));
            base_len = 2;
        }
        static_assert(sizeof ch == sizeof(char16_t), "utf-16 supported only");
        out.u16_trailing = base_len;
        break;
    }
    case TextBC::IBCTextPlatform::Event_CharMetrics:
    {
        union cm_data { uint32_t u32; TextBC::CharMetrics cm; } ;
        const auto pos = reinterpret_cast<cm_data*>(arg)->u32;
        auto& out = reinterpret_cast<cm_data*>(arg)->cm;
        QTextCursor cursor{ ptr };
        cursor.setPosition(pos, QTextCursor::MoveAnchor);
        const QTextBlock block = cursor.block();
        //const auto rect = content->documentLayout()->blockBoundingRect(block);
        auto rootFrameFormat = ptr->rootFrame()->frameFormat();

        const QTextLayout *textCursorBlockLayout = block.layout();
        const int relativeCursorPositionInBlock = cursor.position() - cursor.block().position();
        QTextLine textLine = textCursorBlockLayout->lineForTextPosition(relativeCursorPositionInBlock);
        assert(textLine.isValid());
        //this->content->cur
        {
            const auto left = rootFrameFormat.leftMargin() + rootFrameFormat.padding();
            const auto offset = cursor.position() - block.position();
            //mx.y = textLine.lineNumber() * textLine.height() + rect.top();
            out.x = textLine.cursorToX(offset + 0) + left;
            out.width = textLine.cursorToX(offset + 1) + left - out.x;
        }
        break;
    }
    }
}

#ifndef NDEBUG


/// <summary>
/// Debugs the output.
/// </summary>
/// <param name="txt">The text.</param>
/// <returns></returns>
void TcF10TextEdit::DebugOutput(const char* txt) noexcept {
    qDebug() << txt;
}


/// <summary>
/// Draws the cell.
/// </summary>
/// <param name="ctx">The CTX.</param>
/// <param name="rect">The rect.</param>
/// <param name="index">The index.</param>
/// <returns></returns>
void TcF10TextEdit::DrawCell(void* ctx, const TextBC::RectWHF& rect, int index) noexcept {
    assert(ctx && "bad context");
    // 正式渲染
    const auto painter = static_cast<QPainter*>(ctx);
    painter->setMatrix({});
    const auto x = rect.x;
    const auto y = rect.y;
    const auto w = rect.width;
    const auto h = rect.height;
    QColor color{ index & 1 ? Qt::red : Qt::blue };
    color.setAlpha(0x30);
    painter->fillRect(QRectF{ x, y, w, h }, color);
}
#endif
