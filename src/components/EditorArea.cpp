#include "EditorArea.h"

// EditorArea constructor: sets up the main editing view of the application.
EditorArea::EditorArea(QWidget *parent) : QWidget(parent) {
    // Use a vertical layout to arrange widgets.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0); // Remove padding for a tight fit.

    // A stacked widget allows switching between multiple views, like a card deck.
    m_stack = new QStackedWidget(this);
    
    // A welcome screen, shown when no files are open.
    m_welcome = new WelcomeWidget(this);
    m_stack->addWidget(m_welcome);

    // A tab widget to hold one or more code editors.
    m_tabs = new QTabWidget(this);
    m_tabs->setTabsClosable(true); // Add a close button ('x') to each tab.
    m_tabs->setMovable(true);      // Allow tabs to be reordered via drag-and-drop.
    m_tabs->setDocumentMode(true); // Use a flatter, modern look suitable for document editors.
    // Connect the tabCloseRequested signal to our custom slot to handle closing tabs.
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &EditorArea::onCloseTab);
    m_stack->addWidget(m_tabs);

    // Add the main stack to the layout.
    layout->addWidget(m_stack);
    
    // Load the color theme from JSON so it's ready for syntax highlighting.
    loadTheme();
}

// Opens a file in a new tab.
void EditorArea::openFile(const QString &filePath) {
    QFileInfo info(filePath);
    QString fileName = info.fileName(); // Extract just the filename for the tab title.

    // Check if the file is already open to prevent duplicates.
    for (int i = 0; i < m_tabs->count(); ++i) {
        // We store the full file path in the tab's tooltip.
        if (m_tabs->tabToolTip(i) == filePath) {
            m_tabs->setCurrentIndex(i); // If found, just switch to that tab.
            return;
        }
    }

    // If not open, create a new editor widget.
    QWidget *editorWidget = nullptr;
    QTextDocument *doc = nullptr;

    // --- BRANCHING LOGIC ---
    bool isRichText = filePath.endsWith(".html") || filePath.endsWith(".myformat");
    
    // Load the file content from disk.
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    if (isRichText) {
        RichTextEditor *rich = new RichTextEditor(this);
        
        // --- Metadata Handling for .myformat ---
        int pageSizeIndex = 1; // Default to Medium
        QString htmlContent = content;

        // Check for and parse the pageSize metadata comment.
        if (content.startsWith("<!-- pageSize:")) {
            QString firstLine = content.section('\n', 0, 0); // Read the first line
            QRegularExpression re("<!-- pageSize: (\\d+) -->");
            QRegularExpressionMatch match = re.match(firstLine);
            if (match.hasMatch()) {
                pageSizeIndex = match.captured(1).toInt();
                // The actual HTML content is everything after the first line.
                htmlContent = content.section('\n', 1);
            }
        }
        
        rich->setInitialPageSize(pageSizeIndex);
        rich->setHtml(htmlContent);
        rich->setTheme(m_themeColors);
        doc = rich->document();
        editorWidget = rich;

    } else {
        // Option B: Code
        CodeEditor *code = new CodeEditor(this);
        code->setPlainText(content);
        setupEditor(code, filePath); // Apply Dracula theme
        doc = code->document();
        editorWidget = code;
    }

    // Add the newly created editor to a new tab.
    int index = m_tabs->addTab(editorWidget, fileName);
    m_tabs->setTabToolTip(index, filePath); // Store the full path for later reference.
    m_tabs->setCurrentIndex(index);         // Make the new tab active.
    
    // Connect a signal to detect when the user modifies the text.
    // This is used to show a "*" indicator for unsaved changes.
    connect(doc, &QTextDocument::contentsChanged, this, &EditorArea::onTextModified);

    // If this is the first file, switch the view from the welcome screen to the tabs.
    m_stack->setCurrentWidget(m_tabs);
}

// Saves the content of the currently active tab to its file.
void EditorArea::saveCurrentFile() {
    // Get the current widget from the tab bar.
    QWidget *current = m_tabs->currentWidget();
    if (!current) return; // No tab is open.

    QString filePath = m_tabs->tabToolTip(m_tabs->currentIndex());

    // Write the editor's content back to the file.
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not save file.");
        return;
    }
    
    QTextStream out(&file);
    
    // --- POLYMORPHIC SAVE ---
    if (auto *rich = qobject_cast<RichTextEditor*>(current)) {
        // For our custom format, prepend the page size metadata.
        if (filePath.endsWith(".myformat")) {
            int sizeIndex = rich->currentPageSizeIndex();
            out << "<!-- pageSize: " << sizeIndex << " -->\n";
        }
        out << rich->toHtml();

    } else if (auto *code = qobject_cast<CodeEditor*>(current)) {
        out << code->toPlainText();
    }

    file.close();
    
    // Remove the "*" from the tab title to indicate that the file is saved.
    QString title = m_tabs->tabText(m_tabs->currentIndex());
    if (title.endsWith("*")) {
        m_tabs->setTabText(m_tabs->currentIndex(), title.chopped(1));
    }
}

// Slot called when a tab's close button is clicked.
void EditorArea::onCloseTab(int index) {
    // TODO: Check for unsaved changes here before closing.
    QWidget *widget = m_tabs->widget(index);
    m_tabs->removeTab(index);
    delete widget; // Important: free the memory of the closed editor.

    // If all tabs are closed, switch back to the welcome screen.
    if (m_tabs->count() == 0) {
        m_stack->setCurrentWidget(m_welcome);
    }
}

// Slot called when the content of an editor changes.
void EditorArea::onTextModified() {
    int index = m_tabs->currentIndex();
    QString title = m_tabs->tabText(index);
    // Add a "*" to the end of the tab title if it's not already there.
    if (!title.endsWith("*")) {
        m_tabs->setTabText(index, title + "*");
    }
}

// Applies theme colors and sets up syntax highlighting for a new editor.
void EditorArea::setupEditor(CodeEditor *editor, const QString &filePath) {
    // Apply base theme colors (background and foreground) using a stylesheet.
    // Use the proper setter which handles the Editor AND the Tooltip
    editor->setTheme(m_themeColors);

    // Set up syntax highlighter, but only for plain text code files.
    bool isRichText = filePath.endsWith(".html") || filePath.endsWith(".myformat");
    if (!isRichText) {
        // The Highlighter is a custom class that applies colors to patterns in the text.
        // It's parented to the editor's document, so it's cleaned up automatically.
        new Highlighter(editor->document(), m_themeColors);
        
        // Use a monospaced font for code.
        QFont font("Consolas", 11);
        font.setStyleHint(QFont::Monospace);
        editor->setFont(font);
    } else {
        // Use a standard proportional font for rich text.
        QFont font("Arial", 12);
        editor->setFont(font);
    }
}

// Loads color definitions from a JSON theme file.
void EditorArea::loadTheme() {
    // Note: This assumes the dracula.json file is in the application's working directory.
    std::string themePath = "dracula.json";
    QFile file(themePath.c_str());
    if (file.open(QIODevice::ReadOnly)) {
        // Parse the JSON file.
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject obj = doc.object();
        
        // Store the colors in a map for easy lookup.
        m_themeColors["background"] = QColor(obj["background"].toString());
        m_themeColors["foreground"] = QColor(obj["foreground"].toString());
        m_themeColors["keyword"] = QColor(obj["keyword"].toString());
        m_themeColors["type"] = QColor(obj["type"].toString());
        m_themeColors["string"] = QColor(obj["string"].toString());
        m_themeColors["comment"] = QColor(obj["comment"].toString());
    } else {
        // If the theme can't be loaded, use simple black-on-white defaults.
        m_themeColors["background"] = Qt::white;
        m_themeColors["foreground"] = Qt::black;
    }
}