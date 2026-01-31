#pragma once
#include <QTextEdit>
#include <QWheelEvent>

class CodeEditor : public QTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

protected:
    // We override the mouse wheel event
    void wheelEvent(QWheelEvent *e) override;
};