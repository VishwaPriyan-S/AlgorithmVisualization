#ifndef CODE_HIGHLIGHTER_H
#define CODE_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <qregularexpression.h>

class CodeHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CodeHighlighter(QTextDocument *parent = nullptr);

    void highlightLine(int lineNumber);
    void clearHighlight();

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat singleLineCommentFormat;

    int m_highlightedLine;
    QTextCharFormat m_highlightedLineFormat;
};

#endif // CODE_HIGHLIGHTER_H
