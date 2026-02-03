#include "RichTextEditor.h"
#include "ImageResizeWidget.h"
#include "ImageCropDialog.h"
#include <QVBoxLayout>
#include <QFont>
#include <QTextCharFormat>
#include <QFileDialog>
#include <QBuffer>
#include <QImageReader>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTextCursor>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>

// Define fixed widths for the different page size options.
int SMALL_PAGE_WIDTH = 600;
int MEDIUM_PAGE_WIDTH = 800;
int LARGE_PAGE_WIDTH = 1000;

RichTextEditor::RichTextEditor(QWidget *parent) 
    : QWidget(parent), m_resizeWidget(nullptr) {
    // The main layout for this widget is vertical, arranging toolbar and editor top-to-bottom.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    // Create and add the formatting toolbar.
    setupToolbar();
    layout->addWidget(m_toolbar);

    // Create the custom text editor that supports image pasting.
    m_editor = new CustomRichTextBoard(this);
    
    // Set a professional-looking default font for the editor.
    QFont font("Arial", 12);
    m_editor->setFont(font);

    // Remove the default frame around the text editor to make it blend with the toolbar.
    m_editor->setFrameShape(QFrame::NoFrame);
    
    // A horizontal layout is used to center the editor component within the available space.
    // Stretchable spacers push the editor to the center from both sides.
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_editor);
    centerLayout->addStretch();
    layout->addLayout(centerLayout);

    // Set the initial page size to Medium when the editor is first created.
    onPageSizeChanged(1); // 0 = Small | 1 = Medium | 2 = Large
    m_sizeCombo->setCurrentIndex(1); // Sync the combo box display
    
    // Install event filter to detect image clicks
    m_editor->viewport()->installEventFilter(this);

    // Enable mouse tracking to detect hovering
    m_editor->viewport()->setMouseTracking(true);
    
    // Connect the editor's cursor position changes to a slot that updates the toolbar buttons.
    // This ensures the "Bold" button is checked when the cursor is on bold text.
    connect(m_editor, &QTextEdit::cursorPositionChanged, this, &RichTextEditor::onCursorPositionChanged);
}

void RichTextEditor::setupToolbar() {
    m_toolbar = new QToolBar(this);

    // --- Formatting Actions (Bold, Italic, Underline) ---
    m_actBold = m_toolbar->addAction("B");
    m_actBold->setCheckable(true);
    m_actBold->setShortcut(Qt::CTRL | Qt::Key_B);
    QFont boldFont; boldFont.setBold(true);
    m_actBold->setFont(boldFont);
    
    m_actItalic = m_toolbar->addAction("I");
    m_actItalic->setCheckable(true);
    m_actItalic->setShortcut(Qt::CTRL | Qt::Key_I);
    QFont italicFont; italicFont.setItalic(true);
    m_actItalic->setFont(italicFont);
    
    m_actUnderline = m_toolbar->addAction("U");
    m_actUnderline->setCheckable(true);
    m_actUnderline->setShortcut(Qt::CTRL | Qt::Key_U);
    QFont underlineFont; underlineFont.setUnderline(true);
    m_actUnderline->setFont(underlineFont);

    m_toolbar->addSeparator(); // Visually separate text formatting from other actions.

    // --- Object Insertion Actions (Image) ---
    m_actImage = m_toolbar->addAction("Img");
    m_actImage->setToolTip("Insert Image");

    m_toolbar->addSeparator(); // Visually separate actions.

    // --- Page Size Control ---
    m_sizeCombo = new QComboBox(this);
    m_sizeCombo->addItem("Small");
    m_sizeCombo->addItem("Medium");
    m_sizeCombo->addItem("Large");
    m_toolbar->addWidget(m_sizeCombo); // Add the combo box directly to the toolbar.

    // --- Connect Signals to Slots ---
    connect(m_actBold, &QAction::triggered, this, &RichTextEditor::toggleBold);
    connect(m_actItalic, &QAction::triggered, this, &RichTextEditor::toggleItalic);
    connect(m_actUnderline, &QAction::triggered, this, &RichTextEditor::toggleUnderline);
    connect(m_actImage, &QAction::triggered, this, &RichTextEditor::insertImage);
    connect(m_sizeCombo, QOverload<int>::of(&QComboBox::activated), this, &RichTextEditor::onPageSizeChanged);
}

// This slot is called when the user selects a new size from the combo box.
void RichTextEditor::onPageSizeChanged(int index) {
    // Set a fixed width on the editor widget based on the selection.
    // Using setFixedWidth ensures the layout's stretchable spacers respect this size.
    switch (index) {
        case 0: // Small
            m_editor->setFixedWidth(SMALL_PAGE_WIDTH);
            break;
        case 1: // Medium
            m_editor->setFixedWidth(MEDIUM_PAGE_WIDTH);
            break;
        case 2: // Large
            m_editor->setFixedWidth(LARGE_PAGE_WIDTH);
            break;
    }
}

// --- Public API ---

void RichTextEditor::setHtml(const QString &text) {
    m_editor->setHtml(text);
}

QString RichTextEditor::toHtml() const {
    return m_editor->toHtml();
}

QTextDocument* RichTextEditor::document() const {
    return m_editor->document();
}

int RichTextEditor::currentPageSizeIndex() const {
    return m_sizeCombo->currentIndex();
}

void RichTextEditor::setInitialPageSize(int index) {
    onPageSizeChanged(index);
    m_sizeCombo->setCurrentIndex(index);
}

// --- Text Formatting Slots ---

void RichTextEditor::toggleBold() {
    // Use mergeCurrentCharFormat to apply the bold property without overriding other styles.
    QTextCharFormat fmt;
    fmt.setFontWeight(m_actBold->isChecked() ? QFont::Bold : QFont::Normal);
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::toggleItalic() {
    QTextCharFormat fmt;
    fmt.setFontItalic(m_actItalic->isChecked());
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::toggleUnderline() {
    QTextCharFormat fmt;
    fmt.setFontUnderline(m_actUnderline->isChecked());
    m_editor->mergeCurrentCharFormat(fmt);
}

// Updates the toolbar buttons to reflect the formatting of the text at the current cursor position.
void RichTextEditor::onCursorPositionChanged() {
    QTextCharFormat fmt = m_editor->currentCharFormat();
    m_actBold->setChecked(fmt.fontWeight() == QFont::Bold);
    m_actItalic->setChecked(fmt.fontItalic());
    m_actUnderline->setChecked(fmt.fontUnderline());
}

// Applies a new color theme to the editor and its toolbar.
void RichTextEditor::setTheme(const QHash<QString, QColor> &theme) {
    QColor baseBg = theme.value("background");
    QString fg = theme.value("foreground").name();
    QString comment = theme.value("comment").name(); // Use comment color for borders and accents.

    // Style the parent widget (this) with a slightly darker background.
    // This creates a visual "frame" effect for the centered editor page.
    this->setStyleSheet(QString("QWidget { background-color: %1; }")
                        .arg(baseBg.darker(115).name()));

    // Style the main text editor area itself.
    m_editor->setStyleSheet(QString("QTextEdit { background-color: %1; color: %2; border: none; }")
                            .arg(baseBg.name(), fg));

    // Style the toolbar and its child controls to match the theme.
    QString toolbarStyle = QString(
        "QToolBar { background: %1; border-bottom: 1px solid %3; spacing: 5px; padding: 3px; }"
        "QToolButton { "
            "color: %2; "
            "background: transparent; "
            "padding: 4px; "
            "border-radius: 4px; "
            "border: 1px solid transparent; " // Reserve space for the border
            "min-width: 28px; "
            "min-height: 28px; "
        "}"
        "QToolButton:hover { background: %3; }" // Use accent color for hover.
        "QToolButton:checked { background: %3; border-color: %2; }" // Change border color, not size
        "QComboBox { color: %2; background-color: %1; border: 1px solid %3; padding: 4px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background-color: %1; color: %2; border: 1px solid %3; }"
    ).arg(baseBg.name(), fg, comment);

    m_toolbar->setStyleSheet(toolbarStyle);
}

// Opens a file dialog to let the user insert an image from a local file.
void RichTextEditor::insertImage() {
    // Prompt the user to select an image file.
    QString file = QFileDialog::getOpenFileName(this, "Select Image", "", 
        "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    
    if (file.isEmpty()) return; // User cancelled the dialog.

    // Load the image from the selected file path.
    QImage image(file);
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "Could not load image.");
        return;
    }

    // // If the image is wider than the editor's viewport, scale it down to fit.
    // int maxWidth = m_editor->viewport()->width() - 40; // Subtract padding.
    // if (image.width() > maxWidth && maxWidth > 0) {
    //     image = image.scaledToWidth(maxWidth, Qt::SmoothTransformation);
    // }

    // Calculate 80% of the editor's current page width.
    int targetWidth = m_editor->width() * 0.8;
    if (targetWidth <= 0) targetWidth = 500;

    // Scale down if the image is wider than 80% of the page.
    if (image.width() > targetWidth) {
        image = image.scaledToWidth(targetWidth, Qt::SmoothTransformation);
    }

    // Convert the image data to a Base64 string to embed it directly in the HTML.
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    
    // Automatically detect the image format from the file extension. Default to PNG.
    QString format = QFileInfo(file).suffix().toUpper();
    if (format.isEmpty()) format = "PNG";
    
    image.save(&buffer, format.toLatin1());
    QString base64 = byteArray.toBase64();

    // Create the HTML `<img>` tag with the embedded Base64 data.
    QString htmlImage = QString("<img src=\"data:image/%1;base64,%2\" width=\"%3\" height=\"%4\" />")
                        .arg(format)
                        .arg(base64)
                        .arg(image.width())
                        .arg(image.height());

    // Insert the HTML at the current cursor position.
    m_editor->textCursor().insertHtml(htmlImage);
}

// ============================================================================
// Image Manipulation Methods
// ============================================================================

// The eventFilter intercepts low-level mouse events on the editor viewport
// so we can detect clicks directly on image objects embedded in the document.
// 
// By handling clicks at the viewport-level we avoid needing to change the
// editor's own mouse handling and can add/remove our overlay widgets cleanly.
// 
// We return 'true' (i.e., filter out the event) when we have an active
// image cursor (meaning the click targeted an image) so that the editor
// doesn't perform a conflicting selection or cursor move.
bool RichTextEditor::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_editor->viewport()){
        if(event->type() == QEvent::MouseButtonPress) {
            // Cast the generic event to a mouse event to read position and button.
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            
            qDebug() << "[eventFilter][0] Mouse clicked at:" << mouseEvent->pos();
            // Delegate to our click handler that will decide if an image was clicked
            // and will show/hide the resize widget accordingly.
            onEditorClicked(mouseEvent->pos());
            // If we found an image and stored its cursor, swallow the event to
            // prevent the editor from also moving the cursor or changing selection.
            return !m_currentImageCursor.isNull();
        }

        // Hover Handling (Cursor Shape)
        else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint pos = mouseEvent->pos();

            bool isOverImage = false;
            QTextCursor cursor = findImageCursor(pos);

            // Check if we are strictly inside an image
            if (!cursor.isNull()) {

                // Peek Right to check format
                QTextCursor peek = cursor;
                peek.movePosition(QTextCursor::Right);

                if (peek.charFormat().isImageFormat()) {
                    qDebug() << "[eventFilter][0] Hovering over image at pos:" << pos;
                    QRect imageRect = getImageRect(cursor);

                    if (!imageRect.isNull() && imageRect.contains(pos)) {
                        isOverImage = true;
                    }
                }
            }

            if (isOverImage) {
                m_editor->viewport()->setCursor(Qt::ArrowCursor);
                // Block the event if just hovering (no buttons pressed) 
                // to prevent QTextEdit from resetting it to IBeam.
                if (mouseEvent->buttons() == Qt::NoButton) {
                    return true; 
                }
            } else {
                // Restore default text cursor
                m_editor->viewport()->setCursor(Qt::IBeamCursor);
            }
        }
    }
    
    // For all other cases fall back to the default QWidget event filtering.
    return QWidget::eventFilter(obj, event);
}

// Finds and returns a QTextCursor positioned at the start of the image
// located at the given viewport coordinates. If no image is found, returns
// an invalid cursor.
QTextCursor RichTextEditor::findImageCursor(const QPoint &pos) {
    // 1. Convert Viewport Coordinates (pos) to Document Coordinates
    //    We must add the scrollbar offsets because hitTest works on the full document canvas.
    QPointF docPos = pos;
    docPos.rx() += m_editor->horizontalScrollBar()->value();
    docPos.ry() += m_editor->verticalScrollBar()->value();

    // 2. Ask the Layout Engine: "What is EXACTLY under this pixel?"
    //    Qt::ExactHit ensures we don't snap to nearby text if we are actually on empty space.
    int posInt = m_editor->document()->documentLayout()->hitTest(docPos, Qt::ExactHit);
    
    if (posInt < 0) return QTextCursor();

    // 3. Create a cursor and set it to the found position
    QTextCursor hitCursor = m_editor->textCursor();
    hitCursor.setPosition(posInt);

    // 3. Normalize the result
    //    The cursor returned might be at the Left Edge (start) or Right Edge (end) of the image.
    //    We always want to return the cursor at the Left Edge.

    // CHECK A: Are we standing at the Start? (Image is to the Right)
    QTextCursor peekRight = hitCursor;
    peekRight.movePosition(QTextCursor::Right);
    if (peekRight.charFormat().isImageFormat()) {
        return hitCursor; // Found it!
    }

    // CHECK B: Are we standing at the End? (Image is to the Left)
    //    charFormat() returns the format of the character BEFORE the cursor.
    if (hitCursor.charFormat().isImageFormat()) {
        hitCursor.movePosition(QTextCursor::Left);
        return hitCursor; // Moved to Start, found it!
    }

    // Not an image (maybe text or empty space)
    return QTextCursor();
}


// Computes the visual rectangle of the image represented by the given cursor.
QRect RichTextEditor::getImageRect(const QTextCursor &cursor) {
    if (cursor.isNull()) return QRect();

    // Peek Right to find the image format
    QTextCursor peek = cursor;
    peek.movePosition(QTextCursor::Right);
    if (!peek.charFormat().isImageFormat()) return QRect();
    
    QTextImageFormat imageFormat = peek.charFormat().toImageFormat();
    
    // cursorRect gives us the visual bounding box of the cursor/line.
    // As seen in your logs, this correctly reports the full height (e.g., 367)
    // even when imageFormat.height() reports 0.
    QRect lineRect = m_editor->cursorRect(cursor);
    
    int imgW = imageFormat.width();
    int imgH = imageFormat.height();

    // [FIX] If the format has no height (common on fresh paste), use the line height.
    if (imgH == 0) {
        imgH = lineRect.height();
    }
    
    // [FIX] Align bottom of image with bottom of line
    // Formula: Top = LineTop + (LineHeight - ImageHeight)
    int imageY = lineRect.top() + (lineRect.height() - imgH);
    
    return QRect(lineRect.left(), imageY, imgW, imgH);
}

// Displays the image resize widget positioned over the specified image rectangle.
void RichTextEditor::showImageResizeWidget(const QTextImageFormat &imageFormat, 
                                          const QRect &imageRect) {
    // The imageFormat parameter may contain metadata such as the resource name
    // and current width/height. We don't need it in this implementation but keep
    // the parameter for future use (e.g., displaying original size or ratio).
    Q_UNUSED(imageFormat);

    // Lazily create the resize widget the first time an image is clicked. The
    // widget is parented to the editor's viewport so it moves with the scrolled
    // content and uses viewport coordinates for placement.
    if (!m_resizeWidget) {
        m_resizeWidget = new ImageResizeWidget(m_editor->viewport());
        connect(m_resizeWidget, &ImageResizeWidget::resizeRequested,
                this, &RichTextEditor::onImageResizeRequested);
    }
    
    // Show the visual border/handles at the computed image rectangle. The widget
    // is responsible for drawing its own outline and emitting size changes.
    m_resizeWidget->showAtPosition(imageRect);
}


// Hides the image resize widget and clears any tracked image state.
void RichTextEditor::hideImageResizeWidget() {
    // Hide the overlay widget if present and clear any stored image state so a
    // subsequent click is treated as a normal editor click.
    if (m_resizeWidget) {
        m_resizeWidget->hideWidget();
    }
    // Reset the image tracking state (no image selected)
    m_currentImageCursor = QTextCursor();
    m_currentImageName.clear();
}

void RichTextEditor::onImageResizeRequested(QSize newSize) {
    // If there's no selected/tracked image, ignore the resize request.
    if (m_currentImageCursor.isNull()) return;
    
    // 1. Setup cursor to select the image
    // We assume m_currentImageCursor is at the START (Left) of the image
    QTextCursor cursor = m_currentImageCursor;
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    
    // Read the format from the selection. If this fails (e.g. cursor was just
    // after the image) try selecting the previous character instead.
    QTextImageFormat imageFormat = cursor.charFormat().toImageFormat();
    // if (!imageFormat.isValid()) {
    //     // // Fallback: move left then re-select the single character to try again.
    //     cursor = m_currentImageCursor;
    //     cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    //     cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    //     imageFormat = cursor.charFormat().toImageFormat();
    //     return;
    // }
    
    // If still invalid there is nothing to update.
    if (!imageFormat.isValid()) return;
    
    // 2. Apply the new size
    imageFormat.setWidth(newSize.width());
    imageFormat.setHeight(newSize.height());
    cursor.setCharFormat(imageFormat);
    
    // [CRITICAL FIX 1]: Maintain Cursor Position
    // The 'cursor' object is now at the RIGHT side of the image (because of the selection).
    // We must reset m_currentImageCursor back to the LEFT side (Start).
    // Otherwise, subsequent calculations (getImageRect) will use the wrong side.
    cursor.setPosition(cursor.anchor()); 
    m_currentImageCursor = cursor;

    // [CRITICAL FIX 2]: Visual Sync
    // Ask the text engine: "Where is the image NOW?" (after layout update)
    // Then force the Resize Widget to snap to that exact location.
    QRect realImageRect = getImageRect(m_currentImageCursor);
    
    if (!realImageRect.isNull() && m_resizeWidget) {
        // Overwrite the mouse-predicted position with the real text-layout position
        m_resizeWidget->showAtPosition(realImageRect);
    }
}


// Handles clicks within the editor to determine if an image was clicked.
void RichTextEditor::onEditorClicked(QPoint pos) {
    qDebug() << "[onEditorClicked][1] ------ click pos:" << pos;
    QTextCursor cursor = findImageCursor(pos);

    if (!cursor.isNull()) {
        // We found a valid image and confirmed the click is inside it.
        // We just need to get the name/format for the resize widget.
        qDebug() << "[onEditorClicked][1] ------ Image clicked (inside rect)";

        QTextCursor peek = cursor;
        peek.movePosition(QTextCursor::Right);
        QTextImageFormat imageFormat = peek.charFormat().toImageFormat();
        
        m_currentImageCursor = cursor;
        m_currentImageName = imageFormat.name();
        
        // Show widget
        QRect imageRect = getImageRect(cursor);
        showImageResizeWidget(imageFormat, imageRect);
        return;
    }

    qDebug() << "[onEditorClicked][1] Not an image click - hiding resize widget";
    hideImageResizeWidget();
}