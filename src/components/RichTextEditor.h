#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QAction>
#include <QTextDocument>
#include <QComboBox>
#include "PasteAwareEditor.h"

class RichTextEditor : public QWidget {
    Q_OBJECT

public:
    explicit RichTextEditor(QWidget *parent = nullptr);

    // Common Interface
    void setHtml(const QString &text);
    QString toHtml() const;
    QTextDocument* document() const; // Expose doc for "unsaved changes" signal
    void setTheme(const QHash<QString, QColor> &theme);
    void setInitialPageSize(int index);
    int currentPageSizeIndex() const;

private slots:
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void insertImage();
    void onCursorPositionChanged(); // Sync buttons with cursor state
    void onPageSizeChanged(int index);

private:
    void setupToolbar();

    PasteAwareEditor *m_editor;
    QToolBar *m_toolbar;
    
    QAction *m_actBold;
    QAction *m_actItalic;
    QAction *m_actUnderline;
    QAction *m_actImage;
    QComboBox *m_sizeCombo;
};