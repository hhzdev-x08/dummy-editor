#pragma once
#include <QSyntaxHighlighter>
#include <QRegularExpression>

struct ThemeRule {
    QString pattern;
    QString colorKey; // e.g., "keyword" or "type"
};

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent, const QHash<QString, QColor> &theme);

protected:
    // This is the ONLY function we need to override.
    // Qt calls this automatically for every block of text.
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> rules;
};