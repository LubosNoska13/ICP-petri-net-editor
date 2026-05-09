/**
 * @file MonitorPanel.cpp
 * @author Lubos and project team
 * @brief Implementation of the runtime monitor widget.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/MonitorPanel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

static QTableWidget *makeTable(const QStringList &headers)
{
    QTableWidget *table = new QTableWidget;
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    return table;
}

MonitorPanel::MonitorPanel(QWidget *parent)
    : QWidget(parent), m_document(0)
{
    QVBoxLayout *root = new QVBoxLayout(this);

    m_startButton = new QPushButton("Start");
    m_stopButton = new QPushButton("Stop");
    m_refreshButton = new QPushButton("Refresh");
    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_startButton);
    buttons->addWidget(m_stopButton);
    buttons->addWidget(m_refreshButton);
    root->addLayout(buttons);

    connect(m_startButton, SIGNAL(clicked()), this, SIGNAL(startRequested()));
    connect(m_stopButton, SIGNAL(clicked()), this, SIGNAL(stopRequested()));
    connect(m_refreshButton, SIGNAL(clicked()), this, SIGNAL(refreshRequested()));

    m_markingTable = makeTable(QStringList() << "Place" << "Tokens");
    m_enabledTable = makeTable(QStringList() << "Enabled transitions");
    m_timerTable = makeTable(QStringList() << "Transition" << "Remaining ms");
    m_inputTable = makeTable(QStringList() << "Input" << "Last value");
    m_outputTable = makeTable(QStringList() << "Output" << "Last value");
    m_variableTable = makeTable(QStringList() << "Variable" << "Value");

    root->addWidget(new QLabel("Marking"));
    root->addWidget(m_markingTable);
    root->addWidget(new QLabel("Enabled"));
    root->addWidget(m_enabledTable);
    root->addWidget(new QLabel("Pending timers"));
    root->addWidget(m_timerTable);
    root->addWidget(new QLabel("Inputs"));
    root->addWidget(m_inputTable);
    root->addWidget(new QLabel("Outputs"));
    root->addWidget(m_outputTable);
    root->addWidget(new QLabel("Variables"));
    root->addWidget(m_variableTable);

    QGroupBox *injectBox = new QGroupBox("Inject input");
    QFormLayout *injectLayout = new QFormLayout(injectBox);
    m_inputCombo = new QComboBox;
    m_inputValue = new QLineEdit;
    m_injectButton = new QPushButton("Inject");
    injectLayout->addRow("Input", m_inputCombo);
    injectLayout->addRow("Value", m_inputValue);
    injectLayout->addRow(m_injectButton);
    root->addWidget(injectBox);

    connect(m_injectButton, SIGNAL(clicked()), this, SLOT(onInjectClicked()));
}

void MonitorPanel::setDocument(PetriNetDocument *document)
{
    m_document = document;
    m_inputCombo->clear();
    if (!m_document) {
        return;
    }
    QList<IoData> inputs = m_document->inputs();
    for (int i = 0; i < inputs.size(); ++i) {
        m_inputCombo->addItem(inputs.at(i).name);
    }
}

void MonitorPanel::setSnapshot(const RuntimeSnapshot &snapshot)
{
    m_markingTable->setRowCount(snapshot.marking.size());
    for (int i = 0; i < snapshot.marking.size(); ++i) {
        m_markingTable->setItem(i, 0, new QTableWidgetItem(snapshot.marking.at(i).placeName));
        m_markingTable->setItem(i, 1, new QTableWidgetItem(QString::number(snapshot.marking.at(i).tokens)));
    }

    m_enabledTable->setRowCount(snapshot.enabledTransitions.size());
    for (int i = 0; i < snapshot.enabledTransitions.size(); ++i) {
        m_enabledTable->setItem(i, 0, new QTableWidgetItem(snapshot.enabledTransitions.at(i)));
    }

    m_timerTable->setRowCount(snapshot.pendingTimers.size());
    for (int i = 0; i < snapshot.pendingTimers.size(); ++i) {
        m_timerTable->setItem(i, 0, new QTableWidgetItem(snapshot.pendingTimers.at(i).transitionName));
        m_timerTable->setItem(i, 1, new QTableWidgetItem(QString::number(snapshot.pendingTimers.at(i).remainingMs)));
    }

    fillIoTable(m_inputTable, snapshot.inputs);
    fillIoTable(m_outputTable, snapshot.outputs);

    m_variableTable->setRowCount(snapshot.variables.size());
    for (int i = 0; i < snapshot.variables.size(); ++i) {
        m_variableTable->setItem(i, 0, new QTableWidgetItem(snapshot.variables.at(i).name));
        m_variableTable->setItem(i, 1, new QTableWidgetItem(snapshot.variables.at(i).initialValue));
    }
}

void MonitorPanel::setRunning(bool running)
{
    m_startButton->setEnabled(!running);
    m_stopButton->setEnabled(running);
}

void MonitorPanel::onInjectClicked()
{
    QString name = m_inputCombo->currentText();
    if (name.isEmpty()) {
        name = "input";
    }
    emit inputInjected(name, m_inputValue->text());
}

void MonitorPanel::fillIoTable(QTableWidget *table, const QList<IoData> &items)
{
    table->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(items.at(i).name));
        table->setItem(i, 1, new QTableWidgetItem(items.at(i).lastValue));
    }
}
