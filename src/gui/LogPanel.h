/**
 * @file LogPanel.h
 * @author Lubos and project team
 * @brief Runtime log widget.
 * @details Manual code. Borrowed code: none.
 */

#ifndef LOGPANEL_H
#define LOGPANEL_H

#include "core_api/RuntimeSnapshot.h"

#include <QPlainTextEdit>

/** Read-only text panel for runtime events. */
class LogPanel : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget *parent = 0);
    void setEntries(const QList<LogEntry> &entries);
};

#endif
