#include "CustomRichTextBoard.h"

CustomRichTextBoard::CustomRichTextBoard(QWidget *parent) : QTextEdit(parent) {
}

// Determines if the editor can accept the data being pasted from the clipboard.
bool CustomRichTextBoard::canInsertFromMimeData(const QMimeData *source) const {
    // We can handle images, and for everything else, we defer to the base class implementation.
    return source->hasImage() || QTextEdit::canInsertFromMimeData(source);
}

// Takes a QImage, scales it, converts it to Base64, and wraps it in an HTML `<img>` tag.
QString CustomRichTextBoard::processImage(const QImage &image) {
    if (image.isNull()) return "";

    // // Define a fixed width for all pasted images to ensure consistency.
    // const int fixedWidth = 500;

    // Calculate 80% of the current page width.
    int targetWidth = this->width() * 0.8;

    // Safety fallback if width isn't initialized, though RichTextEditor 
    // sets it to 800 in the constructor.
    if (targetWidth <= 0) targetWidth = 500;

    // Scale the image to the fixed width while preserving its aspect ratio.
    QImage finalImg = image.scaledToWidth(targetWidth, Qt::SmoothTransformation);

    // Convert the image data into a Base64 string for embedding in HTML.
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    finalImg.save(&buffer, "PNG"); // Save in PNG format.
    QString base64 = byteArray.toBase64();

    // Return the complete HTML tag with the embedded image data.
    return QString("<img src=\"data:image/png;base64,%1\" width=\"%2\" height=\"%3\" />")
           .arg(base64)
           .arg(finalImg.width())
           .arg(finalImg.height());
}

// Overrides the default mouse press behavior to detect clicks on images.
void CustomRichTextBoard::mousePressEvent(QMouseEvent *e) {
    // We only care about the left mouse button.
    if (e->button() == Qt::LeftButton) {
        // Get the cursor at the position where the mouse was clicked.
        QTextCursor cursor = cursorForPosition(e->pos());
        
        // To reliably check the format, we must first select the character under the cursor.
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

        // If the selected content is an image, we trigger the resize dialog.
        if (cursor.charFormat().isImageFormat()) {
            setTextCursor(cursor); // Set this as the editor's active cursor.
            // resizeImageAtCursor();
            return; // Consume the event to prevent default text cursor movement.
        }
    }
    // For all other cases (e.g., right-clicks, clicks on text), fall back to default behavior.
    QTextEdit::mousePressEvent(e);
}

// Handles the actual insertion of data from the clipboard.
void CustomRichTextBoard::insertFromMimeData(const QMimeData *source) {
    if (source->hasImage()) {
        // Extract the image from the clipboard data.
        QImage image = qvariant_cast<QImage>(source->imageData());
        // Process the image into an HTML string.
        QString html = processImage(image);
        if (!html.isEmpty()) {
            // Insert the HTML into the document at the current cursor position.
            this->textCursor().insertHtml(html);
        }
    } else {
        // If the data is not an image, let the base QTextEdit class handle it (e.g., pasting text).
        QTextEdit::insertFromMimeData(source);
    }
}

/*
// This slot is triggered to resize the image currently under the text cursor.
void CustomRichTextBoard::resizeImageAtCursor() {
    QTextCursor cursor = textCursor();
    QTextImageFormat imageFmt = cursor.charFormat().toImageFormat();

    if (!imageFmt.isValid()) return; // Exit if the cursor is not on a valid image.

    // To calculate the aspect ratio correctly, we need the original, unscaled image.
    // We retrieve it from the document's resource cache using its unique name.
    QVariant imageData = document()->resource(QTextDocument::ImageResource, imageFmt.name());
    QImage image = imageData.value<QImage>();
    if (image.isNull()) return; // Could not find the original image, so can't proceed.

    bool ok;
    int currentWidth = imageFmt.width();
    
    // Pop up a dialog asking the user for a new width in pixels.
    int newWidth = QInputDialog::getInt(this, "Resize Image", 
                                      "New Width (px):", 
                                      currentWidth, 20, 2000, 10, &ok);
    
    // If the user entered a valid width and clicked OK:
    if (ok && newWidth > 0) {
        // Calculate the new height based on the original image's aspect ratio.
        double aspectRatio = (double)image.height() / image.width();
        int newHeight = newWidth * aspectRatio;

        // Update the image's format in the document with the new dimensions.
        imageFmt.setWidth(newWidth);
        imageFmt.setHeight(newHeight);
        
        // Apply the updated format to the selected image.
        cursor.setCharFormat(imageFmt);
    }
}
*/
