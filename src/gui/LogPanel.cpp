/**
 * @file LogPanel.cpp
 * @author Lubos and project team
 * @brief Implementation of the runtime log widget.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/LogPanel.h"

#include <QTextCursor>

LogPanel::LogPanel(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
}

void LogPanel::setEntries(const QList<LogEntry> &entries)
{
    QString text;
    for (int i = 0; i < entries.size(); ++i) {
        text += entries.at(i).time.toString("yyyy-MM-dd hh:mm:ss.zzz");
        text += "  ";
        text += entries.at(i).text;
        text += "\n";
    }
    setPlainText(text);
    moveCursor(QTextCursor::End);
}
