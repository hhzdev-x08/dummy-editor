#pragma once
#include <QAction>
#include <QWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QHash>
#include <QMimeData>
#include <QInputDialog>
#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>

// A special QTextEdit that knows how to handle Image Pasting
class PasteAwareEditor : public QTextEdit {
    Q_OBJECT
public:
    explicit PasteAwareEditor(QWidget *parent = nullptr);

protected:
    // This function is called whenever the user presses Ctrl+V
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;

    void mousePressEvent(QMouseEvent *e) override;

private slots:
    void resizeImageAtCursor();

private:
    // Helper to generate HTML with width limit
    QString processImage(const QImage &img);
};