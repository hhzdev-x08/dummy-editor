#include "CodeEditor.h"

// =========================================================
// CodeEditor Implementation
// =========================================================

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
    // Enable Mouse Tracking so we get mouseMoveEvent even without clicking
    setMouseTracking(true);

    // Initialize the custom tooltip
    // Parent it to this editor so it destroys when editor destroys, 
    // but the Window flags in CommonTooltip.cpp make it float.
    m_customTooltip = new CommonTooltip(this);

    // Setup the Timer
    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setInterval(3000); // 3 seconds
    m_hoverTimer->setSingleShot(true);
    connect(m_hoverTimer, &QTimer::timeout, this, &CodeEditor::onHoverTimerTimeout);


    // Setup Line Numbers
    lineNumberArea = new LineNumberArea(this);
    
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
            
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
            
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            [this](){ lineNumberArea->update(); }); // Highlight current line number

    // // Connect updateRequest to handle scrolling smoothly
    // connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);

    updateLineNumberAreaWidth(0);

    // Default styling for line numbers (fallback)
    m_lineNumberColor = Qt::gray;
    m_lineNumberBgColor = QColor("#282a36"); // Dracula bg
    
    // Allow scrolling the current line to the center
    // This gives you the "Scroll Beyond Last Line" feel natively.
    setCenterOnScroll(true); 

    // // [OPTIONAL] Strictly scroll until the last line is at the TOP:
    // connect(this->verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min, int max){
    //     // Extend the scrollbar range by the height of the viewport
    //     if (max > 0) {
    //         this->verticalScrollBar()->setMaximum(max + viewport()->height() - fontMetrics().height());
    //     }
    // });
}

void CodeEditor::setTheme(const QHash<QString, QColor> &theme) {
    // Apply theme colors to the editor
    QString bg = theme.value("background").name();
    QString fg = theme.value("foreground").name();
    this->setStyleSheet(QString("background-color: %1; color: %2;").arg(bg, fg));

    // Also update tooltip theme
    if (m_customTooltip) {
        m_customTooltip->applyTheme(theme);
    }

    // Store colors for Line Numbers
    m_lineNumberBgColor = theme.value("background");
    m_lineNumberColor = theme.value("comment"); // Use comment color for line numbers

    // Force repaint of line numbers with new colors
    if (lineNumberArea) lineNumberArea->update();
}

void CodeEditor::wheelEvent(QWheelEvent *e) {
    // Zoom when Ctrl is held
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        // Positive delta = zoom in, negative = zoom out
        if (e->angleDelta().y() > 0) {
            zoomIn(1);
        } else {
            zoomOut(1);
        }
        // Stop further processing
        e->accept();
    } else {
        // Default scroll behavior
        QPlainTextEdit::wheelEvent(e);
    }
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    
    // Calculate desired margins
    int leftMargin = lineNumberAreaWidth();
    // int bottomMargin = cr.height() / 2; // "Scroll Beyond Last Line" (half screen)
    // if (bottomMargin < 50) bottomMargin = 50;

    // // Check if margins actually changed to prevent Infinite Loop
    // QMargins current = viewportMargins();
    // if (current.left() != leftMargin || current.bottom() != bottomMargin) {
    //     setViewportMargins(leftMargin, 0, 0, bottomMargin);
    // }

    // Only set the LEFT margin for line numbers.
    setViewportMargins(leftMargin, 0, 0, 0);

    // Position the line number widget
    // It stays on the left edge, full height of the CONTENT rect (ignoring bottom margin)
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), leftMargin, cr.height()));
}


// ---------------------------------
// Hover Logic
// ---------------------------------

void CodeEditor::mouseMoveEvent(QMouseEvent *e) {
    // Restart hover timer only while Ctrl is held to detect a stable hover
    if (e->modifiers() & Qt::ControlModifier) {
        m_hoverTimer->start();
    } else {
        m_hoverTimer->stop();
    }

    QPlainTextEdit::mouseMoveEvent(e);
}

void CodeEditor::keyPressEvent(QKeyEvent *e) {
    // Start hover timer when Ctrl is pressed
    if (e->key() == Qt::Key_Control) {
        qDebug() << "CTRL Pressed - Starting Hover Timer";
        m_hoverTimer->start();
    }
    QPlainTextEdit::keyPressEvent(e);
}

void CodeEditor::keyReleaseEvent(QKeyEvent *e) {
    // Stop hover timer when Ctrl is released
    if (e->key() == Qt::Key_Control) {
        m_hoverTimer->stop();
    }
    QPlainTextEdit::keyReleaseEvent(e);
}

void CodeEditor::leaveEvent(QEvent *e) {
    // Stop hover timer when mouse leaves
    qDebug() << "Mouse Left - Stopping Hover Timer";
    m_hoverTimer->stop();
    QPlainTextEdit::leaveEvent(e);
}

void CodeEditor::onHoverTimerTimeout() {
    qDebug() << "Hover Timer Timeout occurred";
    // Verify Ctrl is still down then show tooltip
    if (QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) {
        qDebug() << "CTRL is still held - showing tooltip";
        m_customTooltip->showTip(QCursor::pos(), "Hello world\n(Click X to close)");
    }
}

// ---------------------------------
// Line Number Area Logic
// ---------------------------------

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    // Width of character '9' in current font + padding
    int space = 15 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}


void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/) {
    // We just trigger a margin update, the resizeEvent handles the rest
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    
    // Fill background
    painter.fillRect(event->rect(), m_lineNumberBgColor);

    // Iterate over visible blocks
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    // Loop until past the paint event area
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            
            // Highlight current line number
            bool isCurrentLine = (textCursor().blockNumber() == blockNumber);
            painter.setPen(isCurrentLine ? Qt::white : m_lineNumberColor);
            
            // Bold font for current line
            QFont f = font();
            if (isCurrentLine) f.setBold(true);
            painter.setFont(f);

            painter.drawText(0, top, lineNumberArea->width() - 5, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

// ---------------------------------
// Paste with Diff Logic
// ---------------------------------

// Custom Context Menu
void CodeEditor::contextMenuEvent(QContextMenuEvent *e) {
    // Build standard context menu
    QMenu *menu = createStandardContextMenu();

    // Add custom "Paste with Diff" action
    menu->addSeparator();
    
    QAction *diffAction = menu->addAction("Paste with Diff");
    // Enable only when selection and clipboard text exist
    bool hasSelection = this->textCursor().hasSelection();
    bool hasClipboard = !QApplication::clipboard()->text().isEmpty();
    
    diffAction->setEnabled(hasSelection && hasClipboard);
    
    connect(diffAction, &QAction::triggered, this, &CodeEditor::onPasteWithDiff);

    // Show menu then clean up
    menu->exec(e->globalPos());
    delete menu;
}

// Logic for Paste with Diff
void CodeEditor::onPasteWithDiff() {
    QTextCursor cursor = this->textCursor();
    
    QString selectedText = cursor.selectedText();
    // Normalize selection (U+2029 -> \n) for accurate diffing
    selectedText.replace(QChar(0x2029), QChar('\n'));

    QString clipboardText = QApplication::clipboard()->text();
    clipboardText.replace("\r\n", "\n"); // Normalize Windows line endings

    // Show diff dialog
    DiffViewDialog dlg(selectedText, clipboardText, this);
    
    if (dlg.exec() == QDialog::Accepted) {
        DiffViewDialog::DiffAction action = dlg.selectedAction();
        
        // Wrap changes in an edit block for single undo
        cursor.beginEditBlock();

        if (action == DiffViewDialog::ActionMerge || action == DiffViewDialog::ActionReplace) {
            // Merge/Replace: replace selection with clipboard
            cursor.insertText(clipboardText);
        }
        else if (action == DiffViewDialog::ActionInsertBelow) {
            // InsertBelow: append clipboard after selection
            int endPos = cursor.selectionEnd();
            cursor.setPosition(endPos);
            cursor.insertText("\n" + clipboardText);
        }

        cursor.endEditBlock();
    }
}



// =========================================================
// LineNumberArea Implementation
// =========================================================

LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}

QSize LineNumberArea::sizeHint() const {
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    codeEditor->lineNumberAreaPaintEvent(event);
}