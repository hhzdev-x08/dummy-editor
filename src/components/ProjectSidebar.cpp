#include "ProjectSidebar.h"

// ProjectSidebar constructor
ProjectSidebar::ProjectSidebar(QWidget *parent) : QWidget(parent) {
    // Create a vertical box layout for the sidebar
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // Remove margins to fit content tightly

    // --- Setup Model ---
    // QFileSystemModel provides a data model for the local filesystem
    m_model = new QFileSystemModel(this);
    // Set the root path of the model to the system's root directory
    m_model->setRootPath(QDir::rootPath());

    // --- Setup View ---
    // QTreeView provides a view for the filesystem model
    m_treeView = new QTreeView(this);
    // Set the model for the tree view
    m_treeView->setModel(m_model);
    // Disable editing triggers in the tree view
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Set the root index of the tree view to the application's current directory
    m_treeView->setRootIndex(m_model->index(QDir::currentPath())); 
    
    // --- Aesthetics ---
    // Hide columns for size, type, and date modified for a cleaner look
    m_treeView->hideColumn(1); // Size
    m_treeView->hideColumn(2); // Type
    m_treeView->hideColumn(3); // Date
    // Hide the header of the tree view
    m_treeView->setHeaderHidden(true); 

    
    
    // --- Context Menu ---
    // Set the context menu policy to custom to allow for a custom context menu
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    

    // Install an event filter on the tree view's viewport to handle mouse events
    m_treeView->viewport()->installEventFilter(this);

    // Add the tree view to the layout
    layout->addWidget(m_treeView);

    // Connect Interactions
    // Connect the doubleClicked signal of the tree view to the onDoubleClicked slot
    connect(m_treeView, &QTreeView::doubleClicked, this, &ProjectSidebar::onDoubleClicked);
    // Connect the clicked signal of the tree view to the onDoubleClicked slot to open files on single click
    connect(m_treeView, &QTreeView::clicked, this, &ProjectSidebar::onDoubleClicked);
    // Connect the customContextMenuRequested signal to the showContextMenu slot
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &ProjectSidebar::showContextMenu);
}

// --- SIGNAL PROPAGATION ---
// Slot to handle double-click events on the tree view
void ProjectSidebar::onDoubleClicked(const QModelIndex &index) {
    // Check if the index is valid before proceeding
    if (index.isValid()) {
        // Get file information for the clicked index
        QFileInfo info = m_model->fileInfo(index);
        // If the item is a file, emit the fileClicked signal with the file path
        if (info.isFile()) {
            emit fileClicked(info.absoluteFilePath());
        }
    }
}

// Handle mouse press events to deselect items when clicking on empty space
bool ProjectSidebar::eventFilter(QObject *object, QEvent *event) {
    // Check if the event is a mouse button press on the tree view's viewport
    if (object == m_treeView->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // Get the item at the position of the mouse click
        QModelIndex index = m_treeView->indexAt(mouseEvent->pos());
        // If the click is not on a valid item, clear the selection
        if (!index.isValid()) {
            m_treeView->clearSelection();
            m_treeView->setCurrentIndex(QModelIndex());
        }
    }
    // Pass the event to the base class for default processing
    return QWidget::eventFilter(object, event);
}

// --- CONTEXT MENU LOGIC ---
// Show a context menu when requested
void ProjectSidebar::showContextMenu(const QPoint &pos) {
    // Get the index of the item at the requested position
    QModelIndex index = m_treeView->indexAt(pos);
    // Create a new context menu
    QMenu menu(this);

    // --- Actions ---
    // Add actions for creating a new file and a new folder
    QAction *newFile = menu.addAction("New File");
    QAction *newFolder = new QAction("New Folder", this);
    menu.addAction(newFolder);
    menu.addSeparator();
    
    QAction *rename = nullptr;
    QAction *del = nullptr;

    // Only show Rename and Delete actions if a valid item is selected
    if (index.isValid()) {
        rename = menu.addAction("Rename");
        del = menu.addAction("Delete");
    }

    // Show the context menu and get the selected action
    QAction *selected = menu.exec(m_treeView->viewport()->mapToGlobal(pos));

    // Perform actions based on the selected item
    if (selected == newFile) createNewFile();
    if (selected == newFolder) createNewFolder();
    if (selected == rename) renameItem();
    if (selected == del) deleteItem();
}

// Create a new file in the selected directory
void ProjectSidebar::createNewFile() {
    // Determine the path for the new file
    QModelIndex index = m_treeView->currentIndex();
    QString path;
    
    if (!index.isValid()) {
        // If no item is selected, use the root path of the tree view
        path = m_model->filePath(m_treeView->rootIndex());
    } else if (m_model->isDir(index)) {
        // If a directory is selected, use its path
        path = m_model->filePath(index);
    } else {
        // If a file is selected, use its parent directory's path
        path = m_model->fileInfo(index).absolutePath();
    }

    // Get the name for the new file from the user
    bool ok;
    QString name = QInputDialog::getText(this, "New File", "Name:", QLineEdit::Normal, "untitled.txt", &ok);
    if (!ok || name.isEmpty()) return;

    // Construct the full path for the new file
    QString fullPath = path + "/" + name;

    // Check if a file with the same name already exists
    if (QFile::exists(fullPath)) {
        QMessageBox::warning(this, "Error", "File already exists!");
        return;
    }

    // Create the new file
    QFile file(fullPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
    } else {
        QMessageBox::warning(this, "Error", "Could not create file: " + file.errorString());
    }
}

// Create a new folder in the selected directory
void ProjectSidebar::createNewFolder() {
    // Similar logic to createNewFile, but for creating a directory
    QModelIndex index = m_treeView->currentIndex();
    QString path;
    if (!index.isValid()) path = m_model->filePath(m_treeView->rootIndex());
    else if (m_model->isDir(index)) path = m_model->filePath(index);
    else path = m_model->fileInfo(index).absolutePath();

    // Get the name for the new folder from the user
    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Name:", QLineEdit::Normal, "NewFolder", &ok);
    if (ok && !name.isEmpty()) {
        QDir dir(path);
        // Check if a folder with the same name already exists
        if (dir.exists(name)) {
            QMessageBox::warning(this, "Error", "Folder already exists!");
            return;
        }
        // Create the new folder
        dir.mkdir(name);
    }
}

// Delete the selected item
void ProjectSidebar::deleteItem() {
    QModelIndex index = m_treeView->currentIndex();
    if (!index.isValid()) return;
    
    // Confirm with the user before deleting
    QString name = m_model->fileName(index);
    int ret = QMessageBox::question(this, "Delete", "Delete " + name + "?", QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Delete the item from the filesystem model
        if (m_model->isDir(index)) m_model->rmdir(index);
        else m_model->remove(index);
    }
}

// Rename the selected item
void ProjectSidebar::renameItem() {
    QModelIndex index = m_treeView->currentIndex();
    if (!index.isValid()) return;
    
    // Get the new name from the user
    QString oldName = m_model->fileName(index);
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, oldName, &ok);
    
    if (ok && !newName.isEmpty()) {
        // Rename the item in the filesystem
        QString path = m_model->fileInfo(index).absolutePath();
        QDir dir(path);
        dir.rename(oldName, newName);
    }
}