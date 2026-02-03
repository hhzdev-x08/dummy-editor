#pragma once
#include <QTextEdit>
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

#include "CommonTooltip.h"
#include "DiffViewDialog.h"

class CodeEditor : public QTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    // Method to set theme
    void setTheme(const QHash<QString, QColor> &theme);

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

private slots:
    void onHoverTimerTimeout();

    // Slot for the action
    void onPasteWithDiff();

private:
    QTimer *m_hoverTimer;
    CommonTooltip *m_customTooltip;

};