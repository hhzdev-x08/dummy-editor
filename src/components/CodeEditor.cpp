#include "CodeEditor.h"


CodeEditor::CodeEditor(QWidget *parent) : QTextEdit(parent) {
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
        QTextEdit::wheelEvent(e);
    }
}

void CodeEditor::mouseMoveEvent(QMouseEvent *e) {
    // Restart hover timer only while Ctrl is held to detect a stable hover
    if (e->modifiers() & Qt::ControlModifier) {
        m_hoverTimer->start();
    } else {
        m_hoverTimer->stop();
    }

    QTextEdit::mouseMoveEvent(e);
}

void CodeEditor::keyPressEvent(QKeyEvent *e) {
    // Start hover timer when Ctrl is pressed
    if (e->key() == Qt::Key_Control) {
        qDebug() << "CTRL Pressed - Starting Hover Timer";
        m_hoverTimer->start();
    }
    QTextEdit::keyPressEvent(e);
}

void CodeEditor::keyReleaseEvent(QKeyEvent *e) {
    // Stop hover timer when Ctrl is released
    if (e->key() == Qt::Key_Control) {
        m_hoverTimer->stop();
    }
    QTextEdit::keyReleaseEvent(e);
}

void CodeEditor::leaveEvent(QEvent *e) {
    // Stop hover timer when mouse leaves
    qDebug() << "Mouse Left - Stopping Hover Timer";
    m_hoverTimer->stop();
    QTextEdit::leaveEvent(e);
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