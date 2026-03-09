#include "code_highlighter.h"
#include <QTextDocument>

CodeHighlighter::CodeHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), m_highlightedLine(-1)
{
    HighlightingRule rule;

    keywordFormat.setForeground(QColor(86, 156, 214));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bbool\\b"
                    << "\\bfor\\b" << "\\bif\\b" << "\\bwhile\\b" << "\\belse\\b"
                    << "\\breturn\\b" << "\\bbreak\\b" << "\\bcontinue\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    quotationFormat.setForeground(QColor(214, 157, 133));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(QColor(106, 153, 85));
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    m_highlightedLineFormat.setBackground(QColor(60, 60, 20, 150));
    m_highlightedLineFormat.setProperty(QTextFormat::FullWidthSelection, true);
}

void CodeHighlighter::highlightLine(int lineNumber)
{
    if(m_highlightedLine != lineNumber) {
        m_highlightedLine = lineNumber;
        rehighlight();
    }
}

void CodeHighlighter::clearHighlight()
{
    m_highlightedLine = -1;
    rehighlight();
}

void CodeHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    if (m_highlightedLine >= 0 && currentBlock().blockNumber() + 1 == m_highlightedLine) {
        setFormat(0, text.length(), m_highlightedLineFormat);
    }
}
