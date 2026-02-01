#pragma once
#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QMenu>

#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QMouseEvent>
#include <QEvent>

class ProjectSidebar : public QWidget {
    Q_OBJECT

public:
    explicit ProjectSidebar(QWidget *parent = nullptr);

signals:
    // Signal to tell MainWindow: "Hey, the user wants to open this file!"
    void fileClicked(const QString &filePath);

protected:
    // More robust way to handle mouse events on the tree view
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onDoubleClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &pos);
    
    // Actions
    void createNewFile();
    void createNewFolder();
    void deleteItem();
    void renameItem();

private:
    QFileSystemModel *m_model;
    QTreeView *m_treeView;
};