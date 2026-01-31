#include "CodeEditor.h"
#include <QApplication> // Needed to check Keyboard modifiers

CodeEditor::CodeEditor(QWidget *parent) : QTextEdit(parent) {}

void CodeEditor::wheelEvent(QWheelEvent *e) {
    // 1. Check if CTRL is held down
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        // 2. Check scroll direction (y delta)
        // Positive = Scroll Up (Zoom In)
        // Negative = Scroll Down (Zoom Out)
        if (e->angleDelta().y() > 0) {
            zoomIn(1); 
        } else {
            zoomOut(1);
        }
        // 3. Accept the event so parent doesn't scroll the page
        e->accept();
    } else {
        // Normal scrolling (no Zoom)
        QTextEdit::wheelEvent(e); 
    }
}