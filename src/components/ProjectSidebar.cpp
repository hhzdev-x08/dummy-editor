#include "ProjectSidebar.h"


ProjectSidebar::ProjectSidebar(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // Tight fit, no wasted space

    // Setup Model
    m_model = new QFileSystemModel(this);
    m_model->setRootPath(QDir::rootPath());

    // Setup View
    m_treeView = new QTreeView(this);

    
    m_treeView->setModel(m_model);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setRootIndex(m_model->index(QDir::currentPath())); // Start at app folder
    
    
    // Aesthetics
    m_treeView->hideColumn(1); // Size
    m_treeView->hideColumn(2); // Type
    m_treeView->hideColumn(3); // Date
    m_treeView->setHeaderHidden(true); // Hide the "Name" header for a cleaner look

    // Connect Interactions
    connect(m_treeView, &QTreeView::doubleClicked, this, &ProjectSidebar::onDoubleClicked);
    
    // Context Menu
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, &ProjectSidebar::showContextMenu);

    layout->addWidget(m_treeView);
}


// --- SIGNAL PROPAGATION ---
void ProjectSidebar::onDoubleClicked(const QModelIndex &index) {
    QFileInfo info = m_model->fileInfo(index);
    if (info.isFile()) {
        // We just verify it's a file, then pass the path up to MainWindow
        emit fileClicked(info.absoluteFilePath());
    }

}


// --- FIX: DESELECT ON EMPTY SPACE CLICK ---
void ProjectSidebar::mousePressEvent(QMouseEvent *event) {
    // If the click is NOT on a valid index (i.e., whitespace), clear selection
    QModelIndex item = m_treeView->indexAt(event->pos());
    if (!item.isValid()) {
        m_treeView->clearSelection();
        m_treeView->setCurrentIndex(QModelIndex());
    }
    QWidget::mousePressEvent(event);

}

// --- CONTEXT MENU LOGIC ---
void ProjectSidebar::showContextMenu(const QPoint &pos) {
    
    QModelIndex index = m_treeView->indexAt(pos);
    QMenu menu(this);

    // Actions
    QAction *newFile = menu.addAction("New File");
    QAction *newFolder = menu.addAction("New Folder");
    menu.addSeparator();
    
    QAction *rename = nullptr;
    QAction *del = nullptr;

    // Only show Rename/Delete if an item is actually selected
    if (index.isValid()) {
        rename = menu.addAction("Rename");
        del = menu.addAction("Delete");
    }

    QAction *selected = menu.exec(m_treeView->viewport()->mapToGlobal(pos));

    if (selected == newFile) createNewFile();
    if (selected == newFolder) createNewFolder();
    if (selected == rename) renameItem();
    if (selected == del) deleteItem();
}

void ProjectSidebar::createNewFile() {
    // Determine Path
    QModelIndex index = m_treeView->currentIndex();
    QString path;
    
    if (!index.isValid()) {
        // path = m_model->rootPath(); // Default to root
        // BETTER: Default to the folder currently viewed
        path = m_model->filePath(m_treeView->rootIndex());
    } else if (m_model->isDir(index)) {
        path = m_model->filePath(index);
    } else {
        path = m_model->fileInfo(index).absolutePath();
    }

    // Get Name
    bool ok;
    QString name = QInputDialog::getText(this, "New File", "Name:", QLineEdit::Normal, "untitled.txt", &ok);
    if (!ok || name.isEmpty()) return;

    QString fullPath = path + "/" + name;

    // OVERWRITE PROTECTION
    if (QFile::exists(fullPath)) {
        // TODO: later create with index incrementaction like "untitled1.txt", "untitled2.txt", etc.
        QMessageBox::warning(this, "Error", "File already exists!");
        return;
    }

    // Create
    QFile file(fullPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
        // Optional: Open it immediately?
        // emit fileClicked(fullPath); 
    } else {
        QMessageBox::warning(this, "Error", "Could not create file: " + file.errorString());
    }
}

void ProjectSidebar::createNewFolder() {
    // Similar logic to createNewFile, but using QDir::mkdir
    QModelIndex index = m_treeView->currentIndex();
    QString path;
    if (!index.isValid()) path = m_model->filePath(m_treeView->rootIndex());
    else if (m_model->isDir(index)) path = m_model->filePath(index);
    else path = m_model->fileInfo(index).absolutePath();

    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Name:", QLineEdit::Normal, "NewFolder", &ok);
    if (ok && !name.isEmpty()) {
        QDir dir(path);
        if (dir.exists(name)) {
            QMessageBox::warning(this, "Error", "Folder already exists!");
            return;
        }
        dir.mkdir(name);
    }
}

void ProjectSidebar::deleteItem() {
    QModelIndex index = m_treeView->currentIndex();
    if (!index.isValid()) return;
    
    QString name = m_model->fileName(index);
    int ret = QMessageBox::question(this, "Delete", "Delete " + name + "?", QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_model->isDir(index)) m_model->rmdir(index);
        else m_model->remove(index);
    }
}

void ProjectSidebar::renameItem() {
    QModelIndex index = m_treeView->currentIndex();
    if (!index.isValid()) return;
    
    QString oldName = m_model->fileName(index);
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, oldName, &ok);
    
    if (ok && !newName.isEmpty()) {
        QString path = m_model->fileInfo(index).absolutePath();
        QDir dir(path);
        dir.rename(oldName, newName);
    }
}