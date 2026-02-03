#include "ImageCropDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

// ============================================================================
// CropPreviewWidget Implementation
// ============================================================================

CropPreviewWidget::CropPreviewWidget(QWidget *parent)
    : QWidget(parent), m_dragMode(None) {
    setMinimumSize(400, 300);
    setMouseTracking(true);
}

void CropPreviewWidget::setImage(const QImage &image) {
    m_image = image;
    
    // Calculate scaled image rect to fit widget
    QSize scaledSize = m_image.size();
    scaledSize.scale(size(), Qt::KeepAspectRatio);
    
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;
    m_imageRect = QRect(QPoint(x, y), scaledSize);
    
    // Initialize crop rect to full image
    m_cropRect = m_imageRect;
    
    update();
}

void CropPreviewWidget::setCropRect(const QRect &rect) {
    m_cropRect = constrainRect(rect);
    emit cropRectChanged(m_cropRect);
    update();
}

CropPreviewWidget::DragMode CropPreviewWidget::getDragMode(const QPoint &pos) {
    if (!m_cropRect.contains(pos) && 
        !m_cropRect.adjusted(-HANDLE_SIZE, -HANDLE_SIZE, HANDLE_SIZE, HANDLE_SIZE).contains(pos)) {
        return None;
    }
    
    // Check corners
    QRect topLeft(m_cropRect.topLeft() - QPoint(HANDLE_SIZE/2, HANDLE_SIZE/2), 
                  QSize(HANDLE_SIZE, HANDLE_SIZE));
    if (topLeft.contains(pos)) return ResizeTopLeft;
    
    QRect topRight(m_cropRect.topRight() - QPoint(HANDLE_SIZE/2, HANDLE_SIZE/2), 
                   QSize(HANDLE_SIZE, HANDLE_SIZE));
    if (topRight.contains(pos)) return ResizeTopRight;
    
    QRect bottomLeft(m_cropRect.bottomLeft() - QPoint(HANDLE_SIZE/2, HANDLE_SIZE/2), 
                     QSize(HANDLE_SIZE, HANDLE_SIZE));
    if (bottomLeft.contains(pos)) return ResizeBottomLeft;
    
    QRect bottomRight(m_cropRect.bottomRight() - QPoint(HANDLE_SIZE/2, HANDLE_SIZE/2), 
                      QSize(HANDLE_SIZE, HANDLE_SIZE));
    if (bottomRight.contains(pos)) return ResizeBottomRight;
    
    // Check edges
    if (qAbs(pos.x() - m_cropRect.left()) < HANDLE_SIZE/2) return ResizeLeft;
    if (qAbs(pos.x() - m_cropRect.right()) < HANDLE_SIZE/2) return ResizeRight;
    if (qAbs(pos.y() - m_cropRect.top()) < HANDLE_SIZE/2) return ResizeTop;
    if (qAbs(pos.y() - m_cropRect.bottom()) < HANDLE_SIZE/2) return ResizeBottom;
    
    // Inside crop rect = move
    if (m_cropRect.contains(pos)) return Move;
    
    return None;
}

void CropPreviewWidget::updateCursor(const QPoint &pos) {
    DragMode mode = getDragMode(pos);
    
    switch (mode) {
        case ResizeTopLeft:
        case ResizeBottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case ResizeTopRight:
        case ResizeBottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case ResizeLeft:
        case ResizeRight:
            setCursor(Qt::SizeHorCursor);
            break;
        case ResizeTop:
        case ResizeBottom:
            setCursor(Qt::SizeVerCursor);
            break;
        case Move:
            setCursor(Qt::SizeAllCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
    }
}

QRect CropPreviewWidget::constrainRect(const QRect &rect) {
    QRect constrained = rect;
    
    // Constrain to image bounds
    if (constrained.left() < m_imageRect.left())
        constrained.setLeft(m_imageRect.left());
    if (constrained.top() < m_imageRect.top())
        constrained.setTop(m_imageRect.top());
    if (constrained.right() > m_imageRect.right())
        constrained.setRight(m_imageRect.right());
    if (constrained.bottom() > m_imageRect.bottom())
        constrained.setBottom(m_imageRect.bottom());
    
    // Minimum size
    const int MIN_SIZE = 20;
    if (constrained.width() < MIN_SIZE)
        constrained.setWidth(MIN_SIZE);
    if (constrained.height() < MIN_SIZE)
        constrained.setHeight(MIN_SIZE);
    
    return constrained;
}

void CropPreviewWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragMode = getDragMode(event->pos());
        m_dragStart = event->pos();
    }
}

void CropPreviewWidget::mouseMoveEvent(QMouseEvent *event) {
    updateCursor(event->pos());
    
    if (m_dragMode == None) return;
    
    QPoint delta = event->pos() - m_dragStart;
    QRect newRect = m_cropRect;
    
    switch (m_dragMode) {
        case Move:
            newRect.translate(delta);
            break;
        case ResizeTopLeft:
            newRect.setTopLeft(m_cropRect.topLeft() + delta);
            break;
        case ResizeTopRight:
            newRect.setTopRight(m_cropRect.topRight() + delta);
            break;
        case ResizeBottomLeft:
            newRect.setBottomLeft(m_cropRect.bottomLeft() + delta);
            break;
        case ResizeBottomRight:
            newRect.setBottomRight(m_cropRect.bottomRight() + delta);
            break;
        case ResizeLeft:
            newRect.setLeft(m_cropRect.left() + delta.x());
            break;
        case ResizeRight:
            newRect.setRight(m_cropRect.right() + delta.x());
            break;
        case ResizeTop:
            newRect.setTop(m_cropRect.top() + delta.y());
            break;
        case ResizeBottom:
            newRect.setBottom(m_cropRect.bottom() + delta.y());
            break;
        default:
            break;
    }
    
    m_cropRect = constrainRect(newRect);
    m_dragStart = event->pos();
    
    emit cropRectChanged(m_cropRect);
    update();
}

void CropPreviewWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragMode = None;
    }
}

void CropPreviewWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), Qt::gray);
    
    if (m_image.isNull()) return;
    
    // Draw scaled image
    painter.drawImage(m_imageRect, m_image);
    
    // Draw dimmed overlay outside crop area
    painter.fillRect(m_imageRect, QColor(0, 0, 0, 100));
    
    // Clear the crop area
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(m_cropRect, Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    
    // Draw the non-dimmed crop area
    QRect sourceCropRect = m_cropRect.translated(-m_imageRect.topLeft());
    qreal scaleX = (qreal)m_image.width() / m_imageRect.width();
    qreal scaleY = (qreal)m_image.height() / m_imageRect.height();
    
    QRect imageSourceRect(
        sourceCropRect.x() * scaleX,
        sourceCropRect.y() * scaleY,
        sourceCropRect.width() * scaleX,
        sourceCropRect.height() * scaleY
    );
    
    painter.drawImage(m_cropRect, m_image, imageSourceRect);
    
    // Draw crop rectangle border
    painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
    painter.drawRect(m_cropRect);
    
    // Draw corner handles
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    
    painter.drawRect(m_cropRect.topLeft().x() - HANDLE_SIZE/2,
                     m_cropRect.topLeft().y() - HANDLE_SIZE/2,
                     HANDLE_SIZE, HANDLE_SIZE);
    painter.drawRect(m_cropRect.topRight().x() - HANDLE_SIZE/2,
                     m_cropRect.topRight().y() - HANDLE_SIZE/2,
                     HANDLE_SIZE, HANDLE_SIZE);
    painter.drawRect(m_cropRect.bottomLeft().x() - HANDLE_SIZE/2,
                     m_cropRect.bottomLeft().y() - HANDLE_SIZE/2,
                     HANDLE_SIZE, HANDLE_SIZE);
    painter.drawRect(m_cropRect.bottomRight().x() - HANDLE_SIZE/2,
                     m_cropRect.bottomRight().y() - HANDLE_SIZE/2,
                     HANDLE_SIZE, HANDLE_SIZE);
}

// ============================================================================
// ImageCropDialog Implementation
// ============================================================================

ImageCropDialog::ImageCropDialog(const QImage &image, QWidget *parent)
    : QDialog(parent), m_originalImage(image) {
    setWindowTitle("Crop Image");
    setModal(true);
    setupUI();
}

void ImageCropDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Preview widget
    m_previewWidget = new CropPreviewWidget(this);
    m_previewWidget->setImage(m_originalImage);
    mainLayout->addWidget(m_previewWidget);
    
    // Info label
    m_infoLabel = new QLabel(this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_infoLabel);
    
    // Update info when crop rect changes
    connect(m_previewWidget, &CropPreviewWidget::cropRectChanged, this, [this](const QRect &rect) {
        QRect imageRect = m_previewWidget->cropRect();
        m_infoLabel->setText(QString("Crop Area: %1 x %2")
                            .arg(rect.width()).arg(rect.height()));
    });
    
    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton("Reset", this);
    m_cancelButton = new QPushButton("Cancel", this);
    m_applyButton = new QPushButton("Apply", this);
    m_applyButton->setDefault(true);
    
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_applyButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_applyButton, &QPushButton::clicked, this, &ImageCropDialog::onApply);
    connect(m_cancelButton, &QPushButton::clicked, this, &ImageCropDialog::onCancel);
    connect(m_resetButton, &QPushButton::clicked, this, &ImageCropDialog::onResetCrop);
    
    // Initial info
    m_infoLabel->setText(QString("Image Size: %1 x %2")
                        .arg(m_originalImage.width()).arg(m_originalImage.height()));
    
    resize(600, 500);
}

void ImageCropDialog::onApply() {
    accept();
}

void ImageCropDialog::onCancel() {
    reject();
}

void ImageCropDialog::onResetCrop() {
    m_previewWidget->setImage(m_originalImage);
}

QImage ImageCropDialog::croppedImage() const {
    QRect cropRect = m_previewWidget->cropRect();
    QRect imageRect = m_previewWidget->rect();
    
    // Calculate the actual crop rectangle in image coordinates
    qreal scaleX = (qreal)m_originalImage.width() / imageRect.width();
    qreal scaleY = (qreal)m_originalImage.height() / imageRect.height();
    
    QRect actualCropRect(
        cropRect.x() * scaleX,
        cropRect.y() * scaleY,
        cropRect.width() * scaleX,
        cropRect.height() * scaleY
    );
    
    return m_originalImage.copy(actualCropRect);
}

QRect ImageCropDialog::cropRect() const {
    return m_previewWidget->cropRect();
}
