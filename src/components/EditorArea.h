#pragma once
#include <string>

#include <QWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QHash>

#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "WelcomeWidget.h"
#include "CodeEditor.h"
#include "Highlighter.h"
#include "RichTextEditor.h"


class EditorArea : public QWidget {
    Q_OBJECT

public:
    explicit EditorArea(QWidget *parent = nullptr);
    
    // Public Actions
    void openFile(const QString &filePath);
    void saveCurrentFile();

private slots:
    void onCloseTab(int index);
    void onTextModified(); // To add "*" to tab title

private:
    void loadTheme(); // Helper to load dracula.json
    void setupEditor(CodeEditor *editor, const QString &filePath);

    QStackedWidget *m_stack;
    QTabWidget *m_tabs;
    WelcomeWidget *m_welcome;

    // Theme Data (Cached so we can apply to new tabs)
    QHash<QString, QColor> m_themeColors;
};