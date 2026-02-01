#include "EditorArea.h"


EditorArea::EditorArea(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    m_stack = new QStackedWidget(this);
    
    // View 1: Welcome Screen
    m_welcome = new WelcomeWidget(this);
    m_stack->addWidget(m_welcome);

    // View 2: Tabs
    m_tabs = new QTabWidget(this);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setDocumentMode(true); // Makes it look like VS Code (flat)
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &EditorArea::onCloseTab);
    m_stack->addWidget(m_tabs);

    layout->addWidget(m_stack);
    
    // Load theme immediately so it's ready for the first file
    loadTheme();
}

void EditorArea::openFile(const QString &filePath) {
    QFileInfo info(filePath);
    QString fileName = info.fileName();

    // 1. Check if already open
    for (int i = 0; i < m_tabs->count(); ++i) {
        if (m_tabs->tabToolTip(i) == filePath) {
            m_tabs->setCurrentIndex(i);
            return;
        }
    }

    // 2. Create New Editor
    CodeEditor *editor = new CodeEditor(this);
    
    // 3. Load Content
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        
        // Handle Rich Text vs Plain Text
        if (filePath.endsWith(".html") || filePath.endsWith(".myformat")) {
            editor->setHtml(content);
        } else {
            editor->setPlainText(content);
        }
        file.close();
    } else {
        QMessageBox::warning(this, "Error", "Could not open file.");
        delete editor;
        return;
    }

    // 4. Setup Theme & Highlighting
    setupEditor(editor, filePath);

    // 5. Add to Tab
    int index = m_tabs->addTab(editor, fileName);
    m_tabs->setTabToolTip(index, filePath);
    m_tabs->setCurrentIndex(index);
    
    // 6. Connect Modification Signal (for the "*" indicator)
    connect(editor->document(), &QTextDocument::contentsChanged, this, &EditorArea::onTextModified);

    // 7. Switch Stack View
    m_stack->setCurrentWidget(m_tabs);
}

void EditorArea::saveCurrentFile() {
    QWidget *current = m_tabs->currentWidget();
    if (!current) return;

    CodeEditor *editor = qobject_cast<CodeEditor*>(current);
    QString filePath = m_tabs->tabToolTip(m_tabs->currentIndex());

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        
        if (filePath.endsWith(".html") || filePath.endsWith(".myformat")) {
            out << editor->toHtml();
        } else {
            out << editor->toPlainText();
        }
        file.close();
        
        // Remove the "*" from title to indicate saved
        QString title = m_tabs->tabText(m_tabs->currentIndex());
        if (title.endsWith("*")) {
            m_tabs->setTabText(m_tabs->currentIndex(), title.chopped(1));
        }
    } else {
        QMessageBox::warning(this, "Error", "Could not save file.");
    }
}

void EditorArea::onCloseTab(int index) {
    // Optional: Check for unsaved changes here (if title contains "*")
    QWidget *widget = m_tabs->widget(index);
    m_tabs->removeTab(index);
    delete widget;

    if (m_tabs->count() == 0) {
        m_stack->setCurrentWidget(m_welcome);
    }
}

void EditorArea::onTextModified() {
    int index = m_tabs->currentIndex();
    QString title = m_tabs->tabText(index);
    if (!title.endsWith("*")) {
        m_tabs->setTabText(index, title + "*");
    }
}

void EditorArea::setupEditor(CodeEditor *editor, const QString &filePath) {
    // 1. Apply Base CSS
    QString bg = m_themeColors.value("background").name();
    QString fg = m_themeColors.value("foreground").name();
    editor->setStyleSheet(QString("background-color: %1; color: %2;").arg(bg, fg));

    // 2. Setup Highlighter (Only for code files)
    bool isRichText = filePath.endsWith(".html") || filePath.endsWith(".myformat");
    if (!isRichText) {
        // We create the highlighter. It attaches itself to the doc.
        // We don't need to store the pointer because Qt's Object tree handles deletion.
        new Highlighter(editor->document(), m_themeColors);
        
        QFont font("Consolas", 11);
        font.setStyleHint(QFont::Monospace);
        editor->setFont(font);
    } else {
        QFont font("Arial", 12);
        editor->setFont(font);
    }
}

void EditorArea::loadTheme() {
    // Note: Assuming dracula.json is in the build directory
    std::string themePath = "dracula.json";
    QFile file(themePath.c_str());
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject obj = doc.object();
        
        m_themeColors["background"] = QColor(obj["background"].toString());
        m_themeColors["foreground"] = QColor(obj["foreground"].toString());
        m_themeColors["keyword"] = QColor(obj["keyword"].toString());
        m_themeColors["type"] = QColor(obj["type"].toString());
        m_themeColors["string"] = QColor(obj["string"].toString());
        m_themeColors["comment"] = QColor(obj["comment"].toString());
    } else {
        // Fallback defaults
        m_themeColors["background"] = Qt::white;
        m_themeColors["foreground"] = Qt::black;
    }
}