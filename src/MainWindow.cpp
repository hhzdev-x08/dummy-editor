#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Dummy Editor");
    resize(1200, 800);

    // 1. Setup Layout
    QSplitter *splitter = new QSplitter(this);
    setCentralWidget(splitter);

    // 2. Create Components
    m_sidebar = new ProjectSidebar(this);
    m_editorArea = new EditorArea(this);

    // 3. Add to Splitter
    splitter->addWidget(m_sidebar);
    splitter->addWidget(m_editorArea);
    splitter->setSizes(QList<int>() << 250 << 950); // Sidebar width vs Editor width

    // 4. Wiring (The Logic)
    // Connect Sidebar -> Editor
    connect(m_sidebar, &ProjectSidebar::fileClicked, this, &MainWindow::onFileClicked);

    setupMenu();
}

void MainWindow::onFileClicked(const QString &filePath) {
    m_editorArea->openFile(filePath);
}

void MainWindow::onSaveAction() {
    m_editorArea->saveCurrentFile();
}

void MainWindow::setupMenu() {
    QMenu *fileMenu = menuBar()->addMenu("&File");

    // We can access EditorArea's save logic from here
    QAction *saveAct = new QAction("&Save", this);
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveAction);
    
    fileMenu->addAction(saveAct);
    
    // Note: Open/New file are now handled by the Sidebar, 
    // but you could add global menu items calling m_sidebar methods if you made them public.
}