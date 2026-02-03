#include "DiffViewDialog.h"

DiffViewDialog::DiffViewDialog(const QString &original, const QString &incoming, QWidget *parent)
    : QDialog(parent), m_originalText(original), m_incomingText(incoming), m_action(ActionCancel)
{
    setWindowTitle("Diff View: Selection vs Clipboard");
    resize(1000, 600);
    setupUI();
    computeDiff();
}

void DiffViewDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Header labels
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    // Add labels for left/right editors 
    QLabel *lblLeft = new QLabel("Current Selection (Original)", this);
    QLabel *lblRight = new QLabel("Clipboard Content (Incoming)", this);
    lblLeft->setStyleSheet("font-weight: bold; color: #ff5555;"); // Dracula Red
    lblRight->setStyleSheet("font-weight: bold; color: #50fa7b;"); // Dracula Green
    headerLayout->addWidget(lblLeft);
    headerLayout->addWidget(lblRight);
    mainLayout->addLayout(headerLayout);

    // Left/right editor splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    
    // Left Editor
    m_leftEdit = new QTextEdit(this);
    m_leftEdit->setReadOnly(true);
    m_leftEdit->setText(m_originalText);
    // Disable wrap to ensure 1-to-1 vertical alignment
    m_leftEdit->setLineWrapMode(QTextEdit::NoWrap);
    m_leftEdit->setStyleSheet("background-color: #282a36; color: #f8f8f2; font-family: Consolas;");

    // Right Editor
    m_rightEdit = new QTextEdit(this);
    m_rightEdit->setReadOnly(true);
    m_rightEdit->setText(m_incomingText);
    // Disable wrap to ensure 1-to-1 vertical alignment
    m_rightEdit->setLineWrapMode(QTextEdit::NoWrap);
    m_rightEdit->setStyleSheet("background-color: #282a36; color: #f8f8f2; font-family: Consolas;");

    splitter->addWidget(m_leftEdit);
    splitter->addWidget(m_rightEdit);
    // Equal width
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    // Connect scrollbars for synced scrolling
    connect(m_leftEdit->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, &DiffViewDialog::syncScroll);
    connect(m_rightEdit->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, &DiffViewDialog::syncScroll);

    // Sync Horizontal Scrolling as well
    connect(m_leftEdit->horizontalScrollBar(), &QScrollBar::valueChanged, 
            this, [this](int val){
                m_rightEdit->horizontalScrollBar()->setValue(val);
            });
    connect(m_rightEdit->horizontalScrollBar(), &QScrollBar::valueChanged, 
            this, [this](int val){
                m_leftEdit->horizontalScrollBar()->setValue(val);
            });


    
    // Action buttons section
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    QPushButton *btnMerge = new QPushButton("Merge", this);
    QPushButton *btnInsert = new QPushButton("Insert Below", this);
    QPushButton *btnReplace = new QPushButton("Replace", this);
    QPushButton *btnCancel = new QPushButton("Cancel", this);

    // Button styles
    QString btnStyle = "padding: 8px 16px; font-weight: bold;";
    btnMerge->setStyleSheet(btnStyle + "background-color: #bd93f9; color: black;"); // Purple
    btnInsert->setStyleSheet(btnStyle + "background-color: #8be9fd; color: black;"); // Cyan
    btnReplace->setStyleSheet(btnStyle + "background-color: #ff79c6; color: black;"); // Pink
    btnCancel->setStyleSheet(btnStyle);

    btnLayout->addStretch();
    btnLayout->addWidget(btnMerge);
    btnLayout->addWidget(btnInsert);
    btnLayout->addWidget(btnReplace);
    btnLayout->addWidget(btnCancel);

    mainLayout->addLayout(btnLayout);

    // Connect buttons to actions
    connect(btnMerge, &QPushButton::clicked, [this](){ m_action = ActionMerge; accept(); });
    connect(btnInsert, &QPushButton::clicked, [this](){ m_action = ActionInsertBelow; accept(); });
    connect(btnReplace, &QPushButton::clicked, [this](){ m_action = ActionReplace; accept(); });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void DiffViewDialog::syncScroll(int value) {
    // Block signals to prevent infinite loop of A updates B updates A...
    m_leftEdit->verticalScrollBar()->blockSignals(true);
    m_rightEdit->verticalScrollBar()->blockSignals(true);

    m_leftEdit->verticalScrollBar()->setValue(value);
    m_rightEdit->verticalScrollBar()->setValue(value);

    m_leftEdit->verticalScrollBar()->blockSignals(false);
    m_rightEdit->verticalScrollBar()->blockSignals(false);
}

void DiffViewDialog::computeDiff() {
    // Prepare Data by splitting into lines
    QStringList linesLeft = m_originalText.split('\n');
    QStringList linesRight = m_incomingText.split('\n');

    // Remove trailing empty strings often created by splitting text ending in newline
    if (!linesLeft.isEmpty() && linesLeft.last().isEmpty()) linesLeft.removeLast();
    if (!linesRight.isEmpty() && linesRight.last().isEmpty()) linesRight.removeLast();

    // Compute the smart diff using our new helper
    QVector<DiffHelpers::DiffHunk> hunks = DiffHelpers::computeDiff(linesLeft, linesRight);

    // Clear Editors - we will rebuild them line by line
    m_leftEdit->clear();
    m_rightEdit->clear();
    // Disable read-only temporarily so we can insert text programmatically
    m_leftEdit->setReadOnly(false);
    m_rightEdit->setReadOnly(false);

    // Colors (Semi-transparent for highlighting)
    QColor colorDeleted(255, 85, 85, 50);   // Red background for deletions
    QColor colorInserted(80, 250, 123, 50); // Green background for insertions
    QColor colorNone(0, 0, 0, 0);           // Transparent for unchanged lines

    // Initialize cursors
    // Explanation: We use QTextCursor to navigate and format text blocks (lines).
    QTextCursor cursorLeft = m_leftEdit->textCursor();
    QTextCursor cursorRight = m_rightEdit->textCursor();

    // Lambda helper to insert a line with a specific background color
    auto insertStyledLine = [](QTextCursor &cursor, const QString &text, const QColor &bg) {
        QTextBlockFormat fmt = cursor.blockFormat();
        fmt.setBackground(bg);
        cursor.setBlockFormat(fmt);
        // We insert text containing a newline to create the block
        cursor.insertText(text + "\n");
    };

    // Render the Diff
    for (const auto &hunk : hunks) {
        switch (hunk.type) {
            case DiffHelpers::NoChange:
                // Line exists in both. Insert normally with no background.
                insertStyledLine(cursorLeft, hunk.line, colorNone);
                insertStyledLine(cursorRight, hunk.line, colorNone);
                break;

            case DiffHelpers::Deleted:
                // Exists in Left only.
                // Left gets the line highlighted RED. Right gets an empty spacer line.
                insertStyledLine(cursorLeft, hunk.line, colorDeleted);
                insertStyledLine(cursorRight, "", colorNone); 
                break;

            case DiffHelpers::Inserted:
                // Exists in Right only.
                // Left gets an empty spacer line. Right gets the line highlighted GREEN.
                insertStyledLine(cursorLeft, "", colorNone);
                insertStyledLine(cursorRight, hunk.line, colorInserted);
                break;
        }
    }

    // Cleanup
    // Remove the final extra newline inserted by the loop
    cursorLeft.deletePreviousChar();
    cursorRight.deletePreviousChar();

    // Re-enable read-only mode
    m_leftEdit->setReadOnly(true);
    m_rightEdit->setReadOnly(true);

    // Scroll both back to the top
    m_leftEdit->verticalScrollBar()->setValue(0);
    m_rightEdit->verticalScrollBar()->setValue(0);
}
