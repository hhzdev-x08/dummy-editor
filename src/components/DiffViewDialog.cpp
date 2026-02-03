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
    m_leftEdit->setStyleSheet("background-color: #282a36; color: #f8f8f2; font-family: Consolas;");

    // Right Editor
    m_rightEdit = new QTextEdit(this);
    m_rightEdit->setReadOnly(true);
    m_rightEdit->setText(m_incomingText);
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
    // Naive Line-by-Line Diff (Visual Only)
    QStringList linesLeft = m_originalText.split('\n');
    QStringList linesRight = m_incomingText.split('\n');

    // Max lines of both texts for iteration
    int maxLines = std::max(linesLeft.size(), linesRight.size());

    // Colors (Semi-transparent for highlighting)
    QColor colorRemoved(255, 85, 85, 50);   // Red
    QColor colorAdded(80, 250, 123, 50);    // Green

    // Initialize cursors
    // Explanation: We use QTextCursor to navigate and format text blocks (lines).
    QTextCursor cursorLeft = m_leftEdit->textCursor();
    QTextCursor cursorRight = m_rightEdit->textCursor();

    cursorLeft.movePosition(QTextCursor::Start);
    cursorRight.movePosition(QTextCursor::Start);

    for (int i = 0; i < maxLines; ++i) {
        // Get current lines or empty if out of range
        QString txtLeft = (i < linesLeft.size()) ? linesLeft[i] : "";
        QString txtRight = (i < linesRight.size()) ? linesRight[i] : "";

        // Highlight if lines are strictly different
        if (txtLeft != txtRight) {
            
            // Highlight Left (Removed/Changed)
            if (i < linesLeft.size()) {
                QTextBlockFormat fmt = cursorLeft.blockFormat();
                fmt.setBackground(colorRemoved);
                cursorLeft.setBlockFormat(fmt);
            }

            // Highlight Right (Added/Changed)
            if (i < linesRight.size()) {
                QTextBlockFormat fmt = cursorRight.blockFormat();
                fmt.setBackground(colorAdded);
                cursorRight.setBlockFormat(fmt);
            }
        }

        // Move to next block (line)
        cursorLeft.movePosition(QTextCursor::NextBlock);
        cursorRight.movePosition(QTextCursor::NextBlock);
    }
}