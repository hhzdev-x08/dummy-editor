#pragma once
#include <QPlainTextEdit>
#include <QWheelEvent>
#include <QTimer>
#include <QToolTip>
#include <QHash> 
#include <QColor>
#include <QMenu>
#include <QApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QClipboard>
#include <QMessageBox>

#include <QPainter>
#include <QTextBlock>

#include "CommonTooltip.h"
#include "DiffViewDialog.h"

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    // Method to set theme
    void setTheme(const QHash<QString, QColor> &theme);

    // Helper to be called by LineNumberArea
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    // We override the mouse wheel event
    void wheelEvent(QWheelEvent *e) override;

    // Overrides for the Hover feature
    void mouseMoveEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void leaveEvent(QEvent *e) override;

    // Override context menu
    void contextMenuEvent(QContextMenuEvent *e) override;

    // Override resize event to handle margins + line numbers
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void onHoverTimerTimeout();

    // Slot for the action
    void onPasteWithDiff();

    // Slots for Line Numbers
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QTimer *m_hoverTimer;
    CommonTooltip *m_customTooltip;

    QWidget *lineNumberArea;
    QColor m_lineNumberColor; // To store theme color for line numbers
    QColor m_lineNumberBgColor;
};

// Helper widget to paint the line numbers
class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *codeEditor;
};