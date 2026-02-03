#include "CommonTooltip.h"

CommonTooltip::CommonTooltip(QWidget *parent) : QWidget(parent) 
{
    // Window Flags
    // Qt::Tool: Makes it a floating window (on top of parent) but not a top-level app window.
    // Qt::FramelessWindowHint: Removes standard OS title bar/border.
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    // Ensure it doesn't get deleted automatically, we will just hide it.
    setAttribute(Qt::WA_DeleteOnClose, false);

    // Enable CSS background painting for this custom widget
    setAttribute(Qt::WA_StyledBackground);

    // Styling (Dracula Theme match)
    // We hardcode styling here for simplicity, but you could load from the theme JSON.
    setStyleSheet(R"(
        CommonTooltip {
            background-color: #252526; /* VS Code dark tooltip bg */
            border: 1px solid #454545; /* Subtle border */
            border-radius: 4px;
        }
        QLabel {
            color: #cccccc; 
            font-family: Consolas, "Courier New", monospace;
            font-size: 12px;
            padding: 4px;
        }
        QPushButton {
            background: transparent;
            color: #cccccc;
            border: none;
            font-weight: bold;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #c51b25; /* Red background on hover */
            color: white;
        }
    )");

    // 3. Layouts
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4); // Tight padding
    mainLayout->setSpacing(2);

    // -- Header (Close Button) --
    // We use a horizontal layout to push the X to the far right
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addStretch(); // Spacer pushes button to right

    m_closeButton = new QPushButton("✕", this);
    m_closeButton->setFixedSize(20, 20);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setToolTip("Close");
    connect(m_closeButton, &QPushButton::clicked, this, &CommonTooltip::hide);
    
    headerLayout->addWidget(m_closeButton);

    // -- Content --
    m_contentLabel = new QLabel(this);
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Add layouts
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_contentLabel);
    
    // Shadow Effect (for "floating" feel)
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 150)); // Darker shadow
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);
}

void CommonTooltip::showTip(const QPoint &pos, const QString &text) {
    m_contentLabel->setText(text);
    qDebug() << "CommonTooltip::showTip at" << pos << "with text:" << text;
    // Resize to fit content, but limit max width if needed
    adjustSize();
    
    // Move to position (offset slightly so cursor doesn't cover text)
    move(pos + QPoint(10, 10));
    
    show();
    raise(); // Bring to front
}

void CommonTooltip::applyTheme(const QHash<QString, QColor> &theme) {
    // Extract colors (fallback to defaults if missing)
    QString bg = theme.value("background", QColor("#252526")).name();
    QString fg = theme.value("foreground", QColor("#cccccc")).name();
    QString border = theme.value("comment", QColor("#454545")).name(); // Using 'comment' color for borders

    // Re-apply stylesheet with dynamic colors
    setStyleSheet(QString(R"(
        CommonTooltip {
            background-color: %1;
            border: 1px solid %3;
            border-radius: 4px;
        }
        QLabel {
            color: %2;
            font-family: Consolas, "Courier New", monospace;
            font-size: 12px;
            padding: 4px;
        }
        QPushButton {
            background: transparent;
            color: %3; /* Use border color for the X initially */
            border: none;
            font-weight: bold;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #c51b25;
            color: white;
        }
    )").arg(bg, fg, border));
}