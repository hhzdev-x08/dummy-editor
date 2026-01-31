#include "MainWindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Setup the Window
    setWindowTitle("Dummy Editor");
    resize(1000, 600);

    // Create the Splitter (The Container)
    m_splitter = new QSplitter(this);
    setCentralWidget(m_splitter); // The Splitter now owns the window center

    // Setup Sidebar (Left side)
    setupSidebar();

    // Setup Editor (Right side)
    m_editor = new CodeEditor(this);
    
    m_stack = new QStackedWidget(this);

    m_welcome = new WelcomeWidget(this);
    
    // Add "cards" to the stack
    m_stack->addWidget(m_welcome); // Index 0
    m_stack->addWidget(m_editor);  // Index 1
    
    connect(m_editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::onCursorPositionChanged);

    // Add the STACK to the splitter
    m_splitter->addWidget(m_stack);

    // Set initial sizes (Sidebar: 200px, Editor: 800px)
    m_splitter->setSizes(QList<int>() << 200 << 800);

    // Load Theme and setup Highlighter
    loadTheme();
    m_highlighter = new Highlighter(m_editor->document(), m_themeColors);


    // Setup UI Elements
    setupMenu();
    setupToolbar();

    // Default Mode: Code Mode (Hide toolbar, Enable highlighter)
    m_formatToolbar->setVisible(false);
}

void MainWindow::setupSidebar() {
    // Create the Model (The Backend)
    m_fileModel = new QFileSystemModel(this);
    
    // Install File Watcher on "My Computer" (Root)
    // This effectively "unjails" the model.
    // Set the root path to the current directory (where the app is running)
    m_fileModel->setRootPath(QDir::rootPath()); 

    // Create the View (The Frontend)
    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_fileModel); // Connect View to Model

    // ENABLE CONTEXT MENU
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::showContextMenu);

    // Point the View to the Root too
    // On Windows, this shows C:/, D:/ etc. On Linux, it shows /
    m_treeView->setRootIndex(m_fileModel->index(QDir::rootPath()));

    // Aesthetics: Hide columns we don't need (Size, Type, Date)
    // We only want the "Name" column (Column 0)
    m_treeView->hideColumn(1);
    m_treeView->hideColumn(2);
    m_treeView->hideColumn(3);

    // Set the tree to look at our root path
    m_treeView->setRootIndex(m_fileModel->index(QDir::currentPath()));

    // Connect Click Signal
    // When user double clicks a file, call our slot
    connect(m_treeView, &QTreeView::doubleClicked, this, &MainWindow::onFileClicked);

    // Add to Splitter
    m_splitter->addWidget(m_treeView);
}



void MainWindow::setupMenu() {
    // Create "File" Menu
    QMenu *fileMenu = menuBar()->addMenu("&File");

    // Add Open Action
    m_actionOpen = new QAction("&Open", this);
    m_actionOpen->setShortcut(QKeySequence::Open); // Ctrl+O automatically
    connect(m_actionOpen, &QAction::triggered, this, &MainWindow::fileOpen);
    fileMenu->addAction(m_actionOpen);

    // Add Save Action
    m_actionSave = new QAction("&Save", this);
    m_actionSave->setShortcut(QKeySequence::Save); // Ctrl+S automatically
    connect(m_actionSave, &QAction::triggered, this, &MainWindow::fileSave);
    fileMenu->addAction(m_actionSave);
}

void MainWindow::setupToolbar() {
    m_formatToolbar = addToolBar("Format");

    // Add Bold Action
    m_actionBold = new QAction("B", this);
    m_actionBold->setCheckable(true); // It works like a toggle switch
    
    // Make it look bold
    QFont boldFont;
    boldFont.setBold(true);
    m_actionBold->setFont(boldFont);
    
    connect(m_actionBold, &QAction::triggered, this, &MainWindow::toggleBold);
    m_formatToolbar->addAction(m_actionBold);
}

// --- SLOT IMPLEMENTATIONS ---

void MainWindow::fileOpen() {
    // Ask OS for a filename
    // Note: On WSL, this opens the GTK (Linux) dialog. On Windows, the Win32 dialog.
    QString filter = "All Files (*.*);;Text Files (*.txt);;HTML Files (*.html);;My Format (*.myformat)";
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", filter);

    if (fileName.isEmpty()) return; // User pressed Cancel

    // Open the file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file: " + file.errorString());
        return;
    }

    // Read content
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Put into Editor (Render Logic)
    // Check for .myformat or .html to trigger Rich Text rendering
    bool isRichText = fileName.endsWith(".html") || fileName.endsWith(".myformat");
    
    // Reuse the logic from sidebar to switch modes (Optional but good UX)
    // Extract the "Mode Switching" logic from onFileClicked into a helper function later.
    if (isRichText) {
         // Switch to Rich Text Mode
         m_formatToolbar->setVisible(true);
         m_highlighter->setDocument(nullptr);
         m_editor->setFont(QFont("Arial", 15));
         
         m_editor->setHtml(content);
    } else {
         // Switch to Code Mode
         m_formatToolbar->setVisible(false);
         m_highlighter->setDocument(m_editor->document());
         QFont codeFont("Consolas", 14);
         codeFont.setStyleHint(QFont::Monospace);
         m_editor->setFont(codeFont);

         m_editor->setPlainText(content);
    }
}

void MainWindow::fileSave() {
    QString filter = "All Files (*.*);;HTML Files (*.html);;Text Files (*.txt);;My Format (*.myformat)";
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", filter);

    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot save file: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    
    // Logic: Save as HTML to keep the formatting (Bold), or plain text to lose it.
    if (fileName.endsWith(".txt")) {
        out << m_editor->toPlainText();
    } else {
        out << m_editor->toHtml();
    }
    
    file.close();
}

void MainWindow::toggleBold() {
    // 1. Get the "Scalpel" (Cursor)
    QTextCursor cursor = m_editor->textCursor();

    // 2. Define the Format we want
    QTextCharFormat format;
    
    if (m_actionBold->isChecked()) {
        format.setFontWeight(QFont::Bold);
    } else {
        format.setFontWeight(QFont::Normal);
    }

    // 3. Apply it to the CURRENT selection
    cursor.mergeCharFormat(format);
    
    // 4. Update the editor state
    m_editor->mergeCurrentCharFormat(format);
}

void MainWindow::onFileClicked(const QModelIndex &index) {
    // Ask the model: "Is this a folder or a file?"
    // QFileInfo contains metadata about the item
    QFileInfo fileInfo = m_fileModel->fileInfo(index);

    // If it's a folder, do nothing (or maybe expand it)
    if (fileInfo.isDir()) return; 
    

    QString filePath = fileInfo.absoluteFilePath();
    qDebug() << "Opening:" << filePath;
    
    // --- MODE SWITCHING LOGIC ---
    bool isRichText = filePath.endsWith(".myformat") || filePath.endsWith(".html");

    if (isRichText) {
        // RICH TEXT MODE
        // 1. Show the Formatting Toolbar
        m_formatToolbar->setVisible(true);
        
        // 2. Disable Syntax Highlighting (It interferes with rich text)
        m_highlighter->setDocument(nullptr); 

        // 3. Change Font to "Document style" (e.g., Arial)
        QFont docFont("Arial", 12);
        m_editor->setFont(docFont);
    } 
    else {
        // CODE MODE
        // 1. Hide the Formatting Toolbar
        m_formatToolbar->setVisible(false);

        // 2. Enable Syntax Highlighting
        m_highlighter->setDocument(m_editor->document());

        // 3. Change Font to "Code style" (Monospace)
        QFont codeFont("Consolas", 11); // Or "Courier New"
        codeFont.setStyleHint(QFont::Monospace);
        m_editor->setFont(codeFont);
    }

    // Reuse our existing code to load the file
    // We manually load it here.
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // Fail silently or show error
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Check extension to determine how to render
    if (isRichText) {
        m_editor->setHtml(content);
    } else {
        m_editor->setPlainText(content);
    }

    m_stack->setCurrentWidget(m_editor);

}

void MainWindow::onCursorPositionChanged() {
    // 1. Ask the editor: "What is the font weight right here?"
    // currentFont() returns the font of the character strictly under the cursor.
    if (m_editor->currentFont().bold()) {
        m_actionBold->setChecked(true);
    } else {
        m_actionBold->setChecked(false);
    }
}

void MainWindow::loadTheme() {
    std::string themeFile = "..\\..\\dracula.json";
    QFile file(themeFile.c_str());
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open theme file";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    // 1. Parse Colors
    m_themeColors["keyword"] = QColor(obj["keyword"].toString());
    m_themeColors["type"] = QColor(obj["type"].toString());
    m_themeColors["string"] = QColor(obj["string"].toString());
    m_themeColors["comment"] = QColor(obj["comment"].toString());

    // 2. Apply Background/Foreground to Editor immediately
    QString bg = obj["background"].toString();
    QString fg = obj["foreground"].toString();
    
    // CSS for Qt Widgets
    QString style = QString("QTextEdit { background-color: %1; color: %2; }").arg(bg, fg);
    m_editor->setStyleSheet(style);
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QModelIndex index = m_treeView->indexAt(pos);
    
    // Create the Menu
    QMenu contextMenu(this);
    
    // Actions always available
    QAction *newFileAct = contextMenu.addAction("New File");
    QAction *newFolderAct = contextMenu.addAction("New Folder");
    
    // Actions only if an item is selected
    QAction *renameAct = nullptr;
    QAction *deleteAct = nullptr;

    if (index.isValid()) {
        contextMenu.addSeparator();
        renameAct = contextMenu.addAction("Rename");
        deleteAct = contextMenu.addAction("Delete");
    }

    // Show Menu and wait for selection
    QAction *selectedItem = contextMenu.exec(m_treeView->viewport()->mapToGlobal(pos));

    // Handle Selection
    if (selectedItem == newFileAct)    createNewFile();
    if (selectedItem == newFolderAct)  createNewFolder();
    if (selectedItem == renameAct)     renameItem();
    if (selectedItem == deleteAct)     deleteItem();
}

void MainWindow::createNewFile() {
    // Get current directory from sidebar selection
    QModelIndex index = m_treeView->currentIndex();
    QString path;
    
    if (!index.isValid()) {
        // Nothing selected, use root
        path = m_fileModel->rootPath(); // Default to root if nothing selected
    } else if (m_fileModel->isDir(index)) {
        // If a folder is selected, use it
        path = m_fileModel->filePath(index);
    } else {
        // If a file is selected, use its parent folder
        path = m_fileModel->fileInfo(index).absolutePath(); // Use parent folder of file
    }

    // Ask User for Name
    bool ok;
    QString fileName = QInputDialog::getText(this, "New File", "Name:", QLineEdit::Normal, "untitled.txt", &ok);
    
    if (ok && !fileName.isEmpty()) {
        QFile file(path + "/" + fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
        } else {
            QMessageBox::warning(this, "Error", "Could not create file.");
        }
    }
}


void MainWindow::createNewFolder() {
    // Get current directory from sidebar selection
    QModelIndex index = m_treeView->currentIndex();
    QString path;

    // Determine the directory to create the new folder in
    // If nothing is selected, use root
    if (!index.isValid()) path = m_fileModel->rootPath();

    else if (m_fileModel->isDir(index)) path = m_fileModel->filePath(index);
    else path = m_fileModel->fileInfo(index).absolutePath();

    bool ok;
    // Ask user for folder name
    QString name = QInputDialog::getText(this, "New Folder", "Name:", QLineEdit::Normal, "NewFolder", &ok);
    if (ok && !name.isEmpty()) {
        QDir dir(path);
        dir.mkdir(name);
    }
}

void MainWindow::deleteItem() {
    // Get current directory from sidebar selection
    QModelIndex index = m_treeView->currentIndex();
    if (!index.isValid()) return;

    QString path = m_fileModel->filePath(index);
    
    // Safety check
    int ret = QMessageBox::question(this, "Delete", "Are you sure you want to delete: " + m_fileModel->fileName(index) + "?", QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_fileModel->isDir(index)) {
            m_fileModel->rmdir(index); // Remove Directory
        } else {
            m_fileModel->remove(index); // Remove File
        }
    }
}

void MainWindow::renameItem() {
    // Get current directory from sidebar selection
    QModelIndex index = m_treeView->currentIndex();
    if (!index.isValid()) return;

    bool ok;
    QString oldName = m_fileModel->fileName(index);
    QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty()) {
        // QFileSystemModel handles the OS renaming for us!
        QString parentPath = m_fileModel->fileInfo(index).absolutePath();
        QDir dir(parentPath);
        if (!dir.rename(oldName, newName)) {
             QMessageBox::warning(this, "Error", "Could not rename.");
        }
    }
}