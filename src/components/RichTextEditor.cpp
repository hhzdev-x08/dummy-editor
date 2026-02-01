#include "RichTextEditor.h"
#include <QVBoxLayout>
#include <QFont>
#include <QTextCharFormat>
#include <QFileDialog>
#include <QBuffer>
#include <QImageReader>
#include <QMessageBox>

// Define fixed widths for the different page size options.
int SMALL_PAGE_WIDTH = 600;
int MEDIUM_PAGE_WIDTH = 800;
int LARGE_PAGE_WIDTH = 1000;

RichTextEditor::RichTextEditor(QWidget *parent) : QWidget(parent) {
    // The main layout for this widget is vertical, arranging toolbar and editor top-to-bottom.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    // Create and add the formatting toolbar.
    setupToolbar();
    layout->addWidget(m_toolbar);

    // Create the custom text editor that supports image pasting.
    m_editor = new PasteAwareEditor(this);
    
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

    // If the image is wider than the editor's viewport, scale it down to fit.
    int maxWidth = m_editor->viewport()->width() - 40; // Subtract padding.
    if (image.width() > maxWidth && maxWidth > 0) {
        image = image.scaledToWidth(maxWidth, Qt::SmoothTransformation);
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
    QString htmlImage = QString("<img src=\"data:image/%1;base64,%2\" width=\"%3\" />")
                        .arg(format)
                        .arg(base64)
                        .arg(image.width());

    // Insert the HTML at the current cursor position.
    m_editor->textCursor().insertHtml(htmlImage);
}