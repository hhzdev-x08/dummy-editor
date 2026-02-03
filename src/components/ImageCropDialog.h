#pragma once
#include <QDialog>
#include <QImage>
#include <QRect>
#include <QPushButton>
#include <QLabel>

class CropPreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit CropPreviewWidget(QWidget *parent = nullptr);
    
    void setImage(const QImage &image);
    void setCropRect(const QRect &rect);
    QRect cropRect() const { return m_cropRect; }

signals:
    void cropRectChanged(const QRect &rect);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum DragMode {
        None,
        Move,
        ResizeTopLeft,
        ResizeTopRight,
        ResizeBottomLeft,
        ResizeBottomRight,
        ResizeLeft,
        ResizeRight,
        ResizeTop,
        ResizeBottom
    };

    DragMode getDragMode(const QPoint &pos);
    void updateCursor(const QPoint &pos);
    QRect constrainRect(const QRect &rect);

    QImage m_image;
    QRect m_cropRect;
    QPoint m_dragStart;
    DragMode m_dragMode;
    QRect m_imageRect;
    static const int HANDLE_SIZE = 10;
};

class ImageCropDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImageCropDialog(const QImage &image, QWidget *parent = nullptr);
    
    QImage croppedImage() const;
    QRect cropRect() const;

private slots:
    void onApply();
    void onCancel();
    void onResetCrop();

private:
    void setupUI();

    QImage m_originalImage;
    CropPreviewWidget *m_previewWidget;
    QPushButton *m_applyButton;
    QPushButton *m_cancelButton;
    QPushButton *m_resetButton;
    QLabel *m_infoLabel;
};
