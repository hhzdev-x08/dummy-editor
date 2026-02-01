#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class WelcomeWidget : public QWidget {
    Q_OBJECT
public:
    explicit WelcomeWidget(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *layout = new QVBoxLayout(this);
        
        QLabel *title = new QLabel("Dummy Editor", this);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 24px; font-weight: bold; color: #6272a4;"); // Dracula comment color
        
        QLabel *subtitle = new QLabel("Select a file from the sidebar to start editing", this);
        subtitle->setAlignment(Qt::AlignCenter);
        subtitle->setStyleSheet("color: #f8f8f2;");

        layout->addStretch();
        layout->addWidget(title);
        layout->addWidget(subtitle);
        layout->addStretch();
    }
};