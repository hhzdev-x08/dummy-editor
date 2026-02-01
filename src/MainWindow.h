#pragma once
#include <QMainWindow>
#include "ProjectSidebar.h"
#include "EditorArea.h"

#include <QSplitter>
#include <QMenuBar>
#include <QKeySequence>

#include "WelcomeWidget.h"
#include "Highlighter.h"
#include "CodeEditor.h"

// We inherit from QMainWindow, not QWidget.
// QMainWindow gives us a layout with a Menu Bar, Toolbar, and "Central Widget" area.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onFileClicked(const QString &filePath);
    void onSaveAction();

private:
    void setupMenu();

    // Components
    ProjectSidebar *m_sidebar;
    EditorArea *m_editorArea;
};