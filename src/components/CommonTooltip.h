#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QHash> 
#include <QColor>

class CommonTooltip : public QWidget {
    Q_OBJECT
public:
    explicit CommonTooltip(QWidget *parent = nullptr);

    // Set content and show the tooltip at a global screen position
    void showTip(const QPoint &pos, const QString &text);

    // Method to receive theme colors
    void applyTheme(const QHash<QString, QColor> &theme);

private:
    QLabel *m_contentLabel;
    QPushButton *m_closeButton;
};