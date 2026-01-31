#pragma once
#include <string>
#include <QMainWindow>
#include <QTextEdit>
#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QVBoxLayout>
#include <QFileDialog>  // Open/Save Dialogs
#include <QMessageBox>  // Error Popups
#include <QTextStream>  // File Reading helper
#include <QTextCursor>  // The "Scalpel" for editing
#include <QTextCharFormat>
#include <QMenuBar>
#include <QToolBar>

#include <QTreeView>
#include <QFileSystemModel>
#include <QSplitter>

#include <QJsonDocument>
#include <QJsonObject>

#include <QStackedWidget>
#include <QInputDialog>

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
    // File Operations
    void fileOpen();
    void fileSave();
    
    // Formatting Operations
    void toggleBold();

    // When user clicks a file in the sidebar
    void onFileClicked(const QModelIndex &index);

    void onCursorPositionChanged();

    void showContextMenu(const QPoint &pos);
    void createNewFile();
    void createNewFolder();
    void deleteItem();
    void renameItem();

private:
    // UI Setup Helpers
    void setupMenu();
    void setupToolbar();
    void setupSidebar(); 
    void loadTheme();
    

    QStackedWidget *m_stack;
    CodeEditor *m_editor;
    QSplitter *m_splitter;
    Highlighter *m_highlighter;
    WelcomeWidget *m_welcome;
    
    // UI Elements
    QToolBar *m_formatToolbar;

    QFileSystemModel *m_fileModel;
    QTreeView *m_treeView;
    QAction *m_actionBold;
    

    QAction *m_actionOpen;
    QAction *m_actionSave;

    // Store the theme so we can re-apply it if needed
    QHash<QString, QColor> m_themeColors;
};