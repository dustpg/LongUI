#pragma once

#include <QWidget>
#include "TextBC/bc_txtplat.h"

// document
namespace TextBC { class CBCTextDocument; }

/// <summary>
/// text edit
/// </summary>
class TcF10TextEdit : public QWidget, public TextBC::IBCTextPlatform {
    // as qt object
    Q_OBJECT
    // text
    using Text = TextBC::IBCTextContent;
public:
    // ctor
    TcF10TextEdit(QWidget* parent =nullptr);
    // dtor
    ~TcF10TextEdit();
public:
    // get doc
    TextBC::CBCTextDocument* GetDoc() const noexcept { return m_pDoc; }
    // hit test
    auto HitTest(TextBC::Point2F)->TextBC::HitTest;
    // set selection
    void SetSelection(TextBC::Point2F, bool keep) noexcept;
    // request text
    auto RequestText() noexcept -> const QString&;
    // get selection
    auto GetSelection() const noexcept->QString;
    // set text
    void SetText(const QString&) noexcept;
    // set viewpoint
    void SetViewPoint(TextBC::Point2F) noexcept;
    // get viewpoint
    auto GetViewPoint() const noexcept ->TextBC::Point2F;
    // output debug string for undo-stack
    void DebugOutputUndoStack() noexcept;
protected:
    // paint this
    void paintEvent(QPaintEvent *event) override;
    // resize event
    void resizeEvent(QResizeEvent* event) override;
    // mouse press
    void mousePressEvent(QMouseEvent *event) override;
    // mouse release
    void mouseReleaseEvent(QMouseEvent *event) override;
    // mouse move
    void mouseMoveEvent(QMouseEvent *event) override;
    // input method
    void inputMethodEvent(QInputMethodEvent *event) override;
    // key press event
    void keyPressEvent(QKeyEvent*) override;
signals:
    // onclicked signal
    void clicked(TextBC::Point2F pt);
    // onreturned signal
    void returned();
public:
    // error beep
    void ErrorBeep() noexcept override;
    // is valid password [not support char32_t yet]
    bool IsValidPassword(char16_t) noexcept override;
    // generate text
    void GenerateText(void* string, TextBC::U16View view) noexcept override;
    // need redraw
    void NeedRedraw() noexcept override;
    // draw caret
    void DrawCaret(void* ctx, TextBC::Point2F, const TextBC::RectWHF& rect) noexcept override;
    // draw selection
    void DrawSelection(void* ctx, TextBC::Point2F, const TextBC::RectWHF[], uint32_t len) noexcept override;
    // create content
    auto CreateContent(const char16_t*, uint32_t len, Text&&) noexcept->Text* override;
    // delete content
    void DeleteContent(Text&) noexcept override;
    // draw content
    void DrawContent(Text&, void* ctx, TextBC::Point2F pos) noexcept override;
    // content metrics event
    void ContentEvent(Text&, MetricsEvent, void*) noexcept override;
#ifndef NDEBUG
    // debug output
    void DebugOutput(const char*) noexcept override;
    // debug draw cell
    void DrawCell(void* ctx, const TextBC::RectWHF& rect, int index) noexcept override;
#endif
protected:
    // document
    TextBC::CBCTextDocument*    m_pDoc = nullptr;
    // text
    QString                     m_text;
};
