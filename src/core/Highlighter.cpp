#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument *parent, const QHash<QString, QColor> &theme)
    : QSyntaxHighlighter(parent) 
{
    HighlightingRule rule;

    // Define format helper
    auto createFormat = [&](const QString &colorKey, bool bold = false) {
        QTextCharFormat fmt;
        fmt.setForeground(theme.value(colorKey, Qt::black)); // Default to black if key missing
        if (bold) fmt.setFontWeight(QFont::Bold);
        return fmt;
    };

    // Define the Keywords (The Regex)
    // Using "keyword" color from JSON
    QTextCharFormat keywordFmt = createFormat("keyword", true);
    // We look for words bounded by \b (word boundaries)
    QStringList keywords;
    keywords << "\\bclass\\b" << "\\bconst\\b" << "\\benum\\b" << "\\bexplicit\\b"
             << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b" << "\\blong\\b"
             << "\\bnamespace\\b" << "\\boperator\\b" << "\\bprivate\\b" << "\\bprotected\\b"
             << "\\bpublic\\b" << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
             << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
             << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
             << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
             << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bbool\\b";

    // Compile the Rules
    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFmt;
        rules.append(rule);
    }

    // Types (using "type" color)
    QTextCharFormat typeFmt = createFormat("type");
    rule.pattern = QRegularExpression("\\b[A-Z][A-Za-z0-9_]+\\b"); // Simple Regex for ClassNames
    rule.format = typeFmt;
    rules.append(rule);

    // Strings (using "string" color)
    QTextCharFormat stringFmt = createFormat("string");
    rule.pattern = QRegularExpression("\".*\""); 
    rule.format = stringFmt;
    rules.append(rule);

    // Comments (using "comment" color)
    QTextCharFormat commentFmt = createFormat("comment");
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFmt;
    rules.append(rule);
}

void Highlighter::highlightBlock(const QString &text) {
    // Loop through every rule we defined
    for (const HighlightingRule &rule : rules) {
        // Find every match in the current line of text
        QRegularExpressionMatchIterator i = rule.pattern.globalMatch(text);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            // Apply the color to the matched range
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}