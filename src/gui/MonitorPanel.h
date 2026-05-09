/**
 * @file MonitorPanel.h
 * @author Lubos and project team
 * @brief Runtime monitor and input injection panel.
 * @details Manual code. Borrowed code: none.
 */

#ifndef MONITORPANEL_H
#define MONITORPANEL_H

#include "core_api/PetriNetDocument.h"
#include "core_api/RuntimeSnapshot.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

/** Widget showing runtime state and input injection controls. */
class MonitorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorPanel(QWidget *parent = 0);

    void setDocument(PetriNetDocument *document);
    void setSnapshot(const RuntimeSnapshot &snapshot);
    void setRunning(bool running);

signals:
    void startRequested();
    void stopRequested();
    void refreshRequested();
    void inputInjected(const QString &name, const QString &value);

private slots:
    void onInjectClicked();

private:
    void fillIoTable(QTableWidget *table, const QList<IoData> &items);

    PetriNetDocument *m_document;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_refreshButton;
    QTableWidget *m_markingTable;
    QTableWidget *m_enabledTable;
    QTableWidget *m_timerTable;
    QTableWidget *m_inputTable;
    QTableWidget *m_outputTable;
    QTableWidget *m_variableTable;
    QComboBox *m_inputCombo;
    QLineEdit *m_inputValue;
    QPushButton *m_injectButton;
};

#endif
