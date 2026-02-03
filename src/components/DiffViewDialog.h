#pragma once
#include <QDialog>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QLabel>

#include <QSplitter>
#include <QTextBlock>

#include "utils/DiffHelpers.h"

class DiffViewDialog : public QDialog {
    Q_OBJECT

public:
    // Enum to return which action the user chose
    enum DiffAction {
        ActionCancel,
        ActionMerge,
        ActionInsertBelow,
        ActionReplace
    };

    explicit DiffViewDialog(const QString &original, const QString &incoming, QWidget *parent = nullptr);
    
    // Getter for the result
    DiffAction selectedAction() const { return m_action; }

private slots:
    void syncScroll(int value);

private:
    void setupUI();
    void computeDiff(); // The logic to highlight differences

    QString m_originalText;
    QString m_incomingText;
    DiffAction m_action;

    QTextEdit *m_leftEdit;  // Original (Selection)
    QTextEdit *m_rightEdit; // Incoming (Clipboard)
};