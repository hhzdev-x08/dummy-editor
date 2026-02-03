#pragma once
#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QMouseEvent>
#include <QPaintEvent>

class QPushButton;

class ResizeHandle : public QWidget {
    Q_OBJECT

public:
    enum Position {
        TopLeft,
        TopCenter,
        TopRight,
        MiddleLeft,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    explicit ResizeHandle(Position pos, QWidget *parent = nullptr);
    
    Position position() const { return m_position; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void handleDragged(QPoint delta, Position pos);
    void handleDragFinished();

private:
    Position m_position;
    QPoint m_dragStartPos;
    bool m_dragging;
};

class ImageResizeWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImageResizeWidget(QWidget *parent = nullptr);
    ~ImageResizeWidget();

    // Set the geometry of the image being resized
    void setImageRect(const QRect &rect);
    QRect imageRect() const { return m_imageRect; }
    
    // Show/hide the widget
    void showAtPosition(const QRect &imageRect);
    void hideWidget();

signals:
    void resizeRequested(QSize newSize);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onHandleDragged(QPoint delta, ResizeHandle::Position pos);
    void onHandleDragFinished();

private:
    void createHandles();
    void updateHandlePositions();
    QRect calculateNewRect(const QRect &current, QPoint delta, ResizeHandle::Position pos);

    QRect m_imageRect;
    QRect m_originalRect;
    ResizeHandle *m_handles[8];
    static const int HANDLE_SIZE = 8;
};
