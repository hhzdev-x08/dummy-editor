#include "ImageResizeWidget.h"
#include <QPainter>
#include <QCursor>
#include <QPushButton>
#include <QRegion>

// ============================================================================
// ResizeHandle Implementation
// ============================================================================

ResizeHandle::ResizeHandle(Position pos, QWidget *parent)
    : QWidget(parent), m_position(pos), m_dragging(false) {
    setFixedSize(8, 8);
    setMouseTracking(true);
    
    // Set cursor based on position
    switch (pos) {
        case TopLeft:
        case BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case TopRight:
        case BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case TopCenter:
        case BottomCenter:
            setCursor(Qt::SizeVerCursor);
            break;
        case MiddleLeft:
        case MiddleRight:
            setCursor(Qt::SizeHorCursor);
            break;
    }
}

void ResizeHandle::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStartPos = event->globalPosition().toPoint();
    }
}

void ResizeHandle::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPos;
        m_dragStartPos = event->globalPosition().toPoint();
        emit handleDragged(delta, m_position);
    }
}

void ResizeHandle::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        emit handleDragFinished();
    }
}

void ResizeHandle::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw handle as a small filled square with border
    painter.fillRect(rect(), QColor(255, 255, 255, 200));
    painter.setPen(QPen(QColor(0, 120, 215), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

// ============================================================================
// ImageResizeWidget Implementation
// ============================================================================

ImageResizeWidget::ImageResizeWidget(QWidget *parent)
    : QWidget(parent) {
    // Make this a child widget overlay (no top-level window flags)
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    // Don't set window flags - we want this as a child widget, not a top-level window
    
    createHandles();
    hide();
}

ImageResizeWidget::~ImageResizeWidget() {
    for (int i = 0; i < 8; ++i) {
        delete m_handles[i];
    }
}

void ImageResizeWidget::createHandles() {
    m_handles[0] = new ResizeHandle(ResizeHandle::TopLeft, this);
    m_handles[1] = new ResizeHandle(ResizeHandle::TopCenter, this);
    m_handles[2] = new ResizeHandle(ResizeHandle::TopRight, this);
    m_handles[3] = new ResizeHandle(ResizeHandle::MiddleLeft, this);
    m_handles[4] = new ResizeHandle(ResizeHandle::MiddleRight, this);
    m_handles[5] = new ResizeHandle(ResizeHandle::BottomLeft, this);
    m_handles[6] = new ResizeHandle(ResizeHandle::BottomCenter, this);
    m_handles[7] = new ResizeHandle(ResizeHandle::BottomRight, this);
    
    for (int i = 0; i < 8; ++i) {
        connect(m_handles[i], &ResizeHandle::handleDragged,
                this, &ImageResizeWidget::onHandleDragged);
        connect(m_handles[i], &ResizeHandle::handleDragFinished,
                this, &ImageResizeWidget::onHandleDragFinished);
    }
}

void ImageResizeWidget::setImageRect(const QRect &rect) {
    m_imageRect = rect;
    updateHandlePositions();
    update();
}

void ImageResizeWidget::showAtPosition(const QRect &imageRect) {
    m_imageRect = imageRect;
    m_originalRect = imageRect;
    
    // Set widget geometry to encompass the image plus handle space
    QRect widgetRect = imageRect.adjusted(-HANDLE_SIZE, -HANDLE_SIZE, 
                                          HANDLE_SIZE, HANDLE_SIZE);
    setGeometry(widgetRect);
    
    updateHandlePositions();
    show();
    raise();
}


// Handle Placement Logic
// The function uses HANDLE_SIZE (the padding around the image) to calculate a grid. 
// Because the widget is larger than the image to fit the handles, the image itself 
// starts at (HANDLE_SIZE, HANDLE_SIZE) in local coordinates.
// 
// (left, top)      (centerX, top)      (right, top)
//       [0] ------------- [1] ------------- [2]
//        |                                   |
//        |          IMAGE AREA               |
// (left, centerY) [3]             [4] (right, centerY)
//        |                                   |
//        |                                   |
//       [5] ------------- [6] ------------- [7]
// (left, bottom)   (centerX, bottom)   (right, bottom)
// 
// X-Coordinates: left is the start of the image, centerX is the middle, and right is the end.
// Y-Coordinates: top is the top of the image, centerY is the middle, and bottom is the base.
// Centering: Subtracting HANDLE_SIZE / 2 ensures the center of the handle 
// widget aligns perfectly with the edge or corner of the image.

// The Mask components:
// LEGEND:
// [H] = Handle (Interactive)
// [B] = Border (Interactive)
// [.] = Click-through Area (I-Beam shows here)

// [H][B][B][B][B][B][H][B][B][B][B][B][H]  <-- Top Row
// [B].................................[B]
// [B].................................[B]
// [H]......... IMAGE AREA ............[H]  <-- Middle Row
// [B]..........(Click-through)........[B]
// [B].................................[B]
// [H][B][B][B][B][B][H][B][B][B][B][B][H]  <-- Bottom Row
void ImageResizeWidget::updateHandlePositions() {
    if (m_imageRect.isNull()) return;
    
    // Offset is relative to the widget's TopLeft (0,0)
    int offsetX = HANDLE_SIZE;
    int offsetY = HANDLE_SIZE;
    
    int left = offsetX - HANDLE_SIZE / 2;
    int right = m_imageRect.width() + offsetX - HANDLE_SIZE / 2;
    int centerX = m_imageRect.width() / 2 + offsetX - HANDLE_SIZE / 2;
    
    int top = offsetY - HANDLE_SIZE / 2;
    int bottom = m_imageRect.height() + offsetY - HANDLE_SIZE / 2;
    int centerY = m_imageRect.height() / 2 + offsetY - HANDLE_SIZE / 2;
    
    // Position handles
    m_handles[0]->move(left, top);           // TopLeft
    m_handles[1]->move(centerX, top);        // TopCenter
    m_handles[2]->move(right, top);          // TopRight
    m_handles[3]->move(left, centerY);       // MiddleLeft
    m_handles[4]->move(right, centerY);      // MiddleRight
    m_handles[5]->move(left, bottom);        // BottomLeft
    m_handles[6]->move(centerX, bottom);     // BottomCenter
    m_handles[7]->move(right, bottom);       // BottomRight

    // Update Mask to allow click-through in empty spaces
    QRegion maskRegion;

    // Add all handles to the interactive mask
    for (ResizeHandle* handle : m_handles) {
        maskRegion += handle->geometry();
    }

    // 2. Add the visible border to the mask (so the blue frame is visible/interactive)
    // The border is drawn around the image rect in local coordinates.
    QRect localImageRect(HANDLE_SIZE, HANDLE_SIZE, m_imageRect.width(), m_imageRect.height());
    
    // // Create a hollow rectangular region for the 2px border
    // QRegion outer(localImageRect.adjusted(-1, -1, 1, 1)); // 1px outside
    // QRegion inner(localImageRect.adjusted(1, 1, -1, -1)); // 1px inside
    // maskRegion += (outer - inner);

    // By adding the adjusted rectangle (covering both the 2px border and the interior),
    // the widget becomes "solid" to the mouse. This prevents the click-through
    // that was switching the cursor back to an I-Beam.
    maskRegion += localImageRect.adjusted(-1, -1, 1, 1);

    setMask(maskRegion);
}

void ImageResizeWidget::hideWidget() {
    hide();
}

void ImageResizeWidget::onHandleDragged(QPoint delta, ResizeHandle::Position pos) {
    QRect newRect = calculateNewRect(m_imageRect, delta, pos);
    
    // Apply minimum size constraint
    const int MIN_SIZE = 20;
    if (newRect.width() < MIN_SIZE || newRect.height() < MIN_SIZE) {
        return;
    }
    
    m_imageRect = newRect;
    
    // Update widget geometry
    QRect widgetRect = m_imageRect.adjusted(-HANDLE_SIZE, -HANDLE_SIZE,
                                            HANDLE_SIZE, HANDLE_SIZE);
    setGeometry(widgetRect);
    
    updateHandlePositions();
    update();
    
    // Emit resize signal for real-time image update
    emit resizeRequested(m_imageRect.size());
}

void ImageResizeWidget::onHandleDragFinished() {
    // Emit the final size
    emit resizeRequested(m_imageRect.size());
}

QRect ImageResizeWidget::calculateNewRect(const QRect &current, QPoint delta, 
                                          ResizeHandle::Position pos) {
    QRect newRect = current;
    
    switch (pos) {
        case ResizeHandle::TopLeft:
            newRect.setTop(current.top() + delta.y());
            newRect.setLeft(current.left() + delta.x());
            break;
        case ResizeHandle::TopCenter:
            newRect.setTop(current.top() + delta.y());
            break;
        case ResizeHandle::TopRight:
            newRect.setTop(current.top() + delta.y());
            newRect.setRight(current.right() + delta.x());
            break;
        case ResizeHandle::MiddleLeft:
            newRect.setLeft(current.left() + delta.x());
            break;
        case ResizeHandle::MiddleRight:
            newRect.setRight(current.right() + delta.x());
            break;
        case ResizeHandle::BottomLeft:
            newRect.setBottom(current.bottom() + delta.y());
            newRect.setLeft(current.left() + delta.x());
            break;
        case ResizeHandle::BottomCenter:
            newRect.setBottom(current.bottom() + delta.y());
            break;
        case ResizeHandle::BottomRight:
            newRect.setBottom(current.bottom() + delta.y());
            newRect.setRight(current.right() + delta.x());
            break;
    }
    
    return newRect;
}

void ImageResizeWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw selection border around image
    // QRect borderRect = m_imageRect.adjusted(HANDLE_SIZE, HANDLE_SIZE,
    //                                         HANDLE_SIZE, HANDLE_SIZE);
    // Draw border in LOCAL coordinates. 
    // m_imageRect is in Viewport coordinates, but this widget is already offset.
    // The image always starts at (HANDLE_SIZE, HANDLE_SIZE) inside this widget.
    QRect localImageRect(HANDLE_SIZE, HANDLE_SIZE, m_imageRect.width(), m_imageRect.height());

    painter.setPen(QPen(QColor(0, 120, 215), 2, Qt::SolidLine));
    painter.drawRect(localImageRect);
}
