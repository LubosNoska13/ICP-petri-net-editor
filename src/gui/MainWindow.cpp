/**
 * @file MainWindow.cpp
 * @author Lubos and project team
 * @brief Implementation of the Petri net editor main window.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/MainWindow.h"

#include "core_api/CoreMapper.h"
#include "validate.hpp"

#include <QActionGroup>
#include <QCloseEvent>
#include <QDateTime>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolBar>

namespace {

QString validationSeverityName(ValidationSeverity severity)
{
    return severity == ValidationSeverity::Error ? "error" : "warning";
}

QString validationLine(const ValidationMessage &message)
{
    return validationSeverityName(message.severity) + ": "
        + QString::fromStdString(message.message);
}

QStringList validationTextLines(const QStringList &conversionErrors,
                                const ValidationResult &validation)
{
    QStringList lines;
    for (int i = 0; i < conversionErrors.size(); ++i) {
        lines << "error: " + conversionErrors.at(i);
    }
    for (std::size_t i = 0; i < validation.messages.size(); ++i) {
        lines << validationLine(validation.messages.at(i));
    }
    return lines;
}

QList<LogEntry> validationLogEntries(const QStringList &conversionErrors,
                                     const ValidationResult &validation)
{
    QList<LogEntry> entries;
    QDateTime now = QDateTime::currentDateTime();

    for (int i = 0; i < conversionErrors.size(); ++i) {
        LogEntry entry;
        entry.time = now;
        entry.text = "validation-error: " + conversionErrors.at(i);
        entries.append(entry);
    }

    for (std::size_t i = 0; i < validation.messages.size(); ++i) {
        const ValidationMessage &message = validation.messages.at(i);
        LogEntry entry;
        entry.time = now;
        entry.text = "validation-" + validationSeverityName(message.severity)
            + ": " + QString::fromStdString(message.message);
        entries.append(entry);
    }

    if (entries.isEmpty()) {
        LogEntry entry;
        entry.time = now;
        entry.text = "validation-ok: No validation errors or warnings.";
        entries.append(entry);
    }

    return entries;
}

int validationWarningCount(const ValidationResult &validation)
{
    int count = 0;
    for (std::size_t i = 0; i < validation.messages.size(); ++i) {
        if (validation.messages.at(i).severity == ValidationSeverity::Warning) {
            ++count;
        }
    }
    return count;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_modified(false)
{
    setWindowTitle("Petri Net Editor");
    resize(1100, 680);

    m_scene = new DiagramScene(this);
    m_scene->setDocument(&m_document);
    m_view = new DiagramView(this);
    m_view->setScene(m_scene);
    setCentralWidget(m_view);

    m_propertyPanel = new PropertyPanel(this);
    m_propertyPanel->setDocument(&m_document);
    m_monitorPanel = new MonitorPanel(this);
    m_monitorPanel->setDocument(&m_document);
    m_logPanel = new LogPanel(this);

    m_runtimeTimer = new QTimer(this);
    m_runtimeTimer->setInterval(500);

    createActions();
    createMenus();
    createToolbar();
    createDockWidgets();

    connect(m_scene, SIGNAL(documentChanged()), this, SLOT(onDocumentChanged()));
    connect(m_scene, SIGNAL(elementSelected(QString,ElementKind)),
            this, SLOT(onElementSelected(QString,ElementKind)));
    connect(m_scene, SIGNAL(statusMessage(QString)), this, SLOT(updateStatusMessage(QString)));
    connect(m_propertyPanel, SIGNAL(documentChanged()), this, SLOT(onDocumentChanged()));
    connect(m_monitorPanel, SIGNAL(startRequested()), this, SLOT(startRuntime()));
    connect(m_monitorPanel, SIGNAL(stopRequested()), this, SLOT(stopRuntime()));
    connect(m_monitorPanel, SIGNAL(refreshRequested()), this, SLOT(refreshRuntime()));
    connect(m_monitorPanel, SIGNAL(inputInjected(QString,QString)),
            this, SLOT(injectInput(QString,QString)));
    connect(m_runtimeTimer, SIGNAL(timeout()), this, SLOT(refreshRuntime()));

    setMode(SelectMode);
    statusBar()->showMessage("Ready.");
}

void MainWindow::newDocument()
{
    if (!maybeSave()) {
        return;
    }
    m_document.clear();
    m_scene->reloadFromDocument();
    m_propertyPanel->setSelection(QString(), UnknownElement);
    m_monitorPanel->setDocument(&m_document);
    setCurrentFile(QString());
    m_modified = false;
}

void MainWindow::openDocument()
{
    if (!maybeSave()) {
        return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open net", QString(),
                                                    "Petri net files (*.pn *.txt);;All files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    QString error;
    if (!DocumentSerializer::load(&m_document, fileName, &error)) {
        QMessageBox::warning(this, "Open failed", error);
        return;
    }
    m_scene->reloadFromDocument();
    m_propertyPanel->setSelection(QString(), UnknownElement);
    m_monitorPanel->setDocument(&m_document);
    setCurrentFile(fileName);
    m_modified = false;
    statusBar()->showMessage("Document loaded.");
}

void MainWindow::saveDocument()
{
    if (m_currentFile.isEmpty()) {
        saveDocumentAs();
        return;
    }
    saveToFile(m_currentFile);
}

void MainWindow::saveDocumentAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save net", QString(),
                                                    "Petri net files (*.pn);;Text files (*.txt);;All files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    saveToFile(fileName);
}

void MainWindow::deleteSelected()
{
    QString id = m_scene->selectedElementId();
    if (id.isEmpty()) {
        return;
    }
    m_document.removeElement(id);
    m_scene->reloadFromDocument();
    m_propertyPanel->setSelection(QString(), UnknownElement);
    onDocumentChanged();
}

void MainWindow::validateDocument()
{
    QStringList conversionErrors;
    PetriNet net = CoreMapper::toCoreNet(m_document, &conversionErrors);
    Validator validator;
    ValidationResult validation = validator.validate(net);

    QStringList lines = validationTextLines(conversionErrors, validation);
    m_logPanel->setEntries(validationLogEntries(conversionErrors, validation));

    if (lines.isEmpty()) {
        QMessageBox::information(this, "Validation",
                                 "No validation errors or warnings found.");
        statusBar()->showMessage("Validation passed.");
    } else if (!conversionErrors.isEmpty() || validation.has_errors()) {
        QMessageBox::warning(this, "Validation", lines.join("\n"));
        int errorCount = conversionErrors.size();
        for (std::size_t i = 0; i < validation.messages.size(); ++i) {
            if (validation.messages.at(i).severity == ValidationSeverity::Error) {
                ++errorCount;
            }
        }
        statusBar()->showMessage(QString::number(errorCount) + " validation errors.");
    } else {
        QMessageBox::information(this, "Validation", lines.join("\n"));
        statusBar()->showMessage(QString::number(validationWarningCount(validation))
                                 + " validation warnings.");
    }
}

void MainWindow::setSelectMode() { setMode(SelectMode); }
void MainWindow::setPlaceMode() { setMode(AddPlaceMode); }
void MainWindow::setTransitionMode() { setMode(AddTransitionMode); }
void MainWindow::setArcMode() { setMode(AddArcMode); }

void MainWindow::onDocumentChanged()
{
    m_modified = true;
    if (sender() == m_propertyPanel) {
        m_scene->reloadFromDocument();
    }
    m_propertyPanel->reload();
    m_monitorPanel->setDocument(&m_document);
    statusBar()->showMessage("Document changed.");
}

void MainWindow::onElementSelected(const QString &id, ElementKind kind)
{
    m_propertyPanel->setSelection(id, kind);
}

void MainWindow::startRuntime()
{
    if (m_runtime.start(&m_document)) {
        m_runtimeTimer->start();
        refreshRuntime();
        statusBar()->showMessage("Runtime started.");
    } else {
        refreshRuntime();
        statusBar()->showMessage("Runtime start failed.");
    }
}

void MainWindow::stopRuntime()
{
    m_runtime.stop();
    m_runtimeTimer->stop();
    refreshRuntime();
    statusBar()->showMessage("Runtime stopped.");
}

void MainWindow::refreshRuntime()
{
    if (m_runtime.isRunning()) {
        m_runtime.advanceTime(m_runtimeTimer->interval());
    }
    RuntimeSnapshot snapshot = m_runtime.snapshot();
    m_monitorPanel->setRunning(m_runtime.isRunning());
    m_monitorPanel->setSnapshot(snapshot);
    m_scene->updateRuntimeMarking(snapshot);
    m_logPanel->setEntries(snapshot.logEntries);
}

void MainWindow::injectInput(const QString &name, const QString &value)
{
    m_runtime.injectInput(name, value);
    refreshRuntime();
}

void MainWindow::updateStatusMessage(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainWindow::createActions()
{
    m_newAction = new QAction("New", this);
    m_openAction = new QAction("Open", this);
    m_saveAction = new QAction("Save", this);
    m_saveAsAction = new QAction("Save As", this);
    m_exitAction = new QAction("Exit", this);
    m_deleteAction = new QAction("Delete Selected", this);
    m_validateAction = new QAction("Validate", this);
    m_selectModeAction = new QAction("Select", this);
    m_placeModeAction = new QAction("Place", this);
    m_transitionModeAction = new QAction("Transition", this);
    m_arcModeAction = new QAction("Arc", this);
    m_startAction = new QAction("Start", this);
    m_stopAction = new QAction("Stop", this);

    m_selectModeAction->setCheckable(true);
    m_placeModeAction->setCheckable(true);
    m_transitionModeAction->setCheckable(true);
    m_arcModeAction->setCheckable(true);

    QActionGroup *modeGroup = new QActionGroup(this);
    modeGroup->addAction(m_selectModeAction);
    modeGroup->addAction(m_placeModeAction);
    modeGroup->addAction(m_transitionModeAction);
    modeGroup->addAction(m_arcModeAction);

    connect(m_newAction, SIGNAL(triggered()), this, SLOT(newDocument()));
    connect(m_openAction, SIGNAL(triggered()), this, SLOT(openDocument()));
    connect(m_saveAction, SIGNAL(triggered()), this, SLOT(saveDocument()));
    connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveDocumentAs()));
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deleteSelected()));
    connect(m_validateAction, SIGNAL(triggered()), this, SLOT(validateDocument()));
    connect(m_selectModeAction, SIGNAL(triggered()), this, SLOT(setSelectMode()));
    connect(m_placeModeAction, SIGNAL(triggered()), this, SLOT(setPlaceMode()));
    connect(m_transitionModeAction, SIGNAL(triggered()), this, SLOT(setTransitionMode()));
    connect(m_arcModeAction, SIGNAL(triggered()), this, SLOT(setArcMode()));
    connect(m_startAction, SIGNAL(triggered()), this, SLOT(startRuntime()));
    connect(m_stopAction, SIGNAL(triggered()), this, SLOT(stopRuntime()));
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    QMenu *editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(m_deleteAction);

    QMenu *toolsMenu = menuBar()->addMenu("Tools");
    toolsMenu->addAction(m_validateAction);

    QMenu *runtimeMenu = menuBar()->addMenu("Runtime");
    runtimeMenu->addAction(m_startAction);
    runtimeMenu->addAction(m_stopAction);
}

void MainWindow::createToolbar()
{
    QToolBar *toolbar = addToolBar("Tools");
    toolbar->addAction(m_selectModeAction);
    toolbar->addAction(m_placeModeAction);
    toolbar->addAction(m_transitionModeAction);
    toolbar->addAction(m_arcModeAction);
    toolbar->addSeparator();
    toolbar->addAction(m_deleteAction);
    toolbar->addSeparator();
    toolbar->addAction(m_startAction);
    toolbar->addAction(m_stopAction);
}

void MainWindow::createDockWidgets()
{
    QDockWidget *propertiesDock = new QDockWidget("Properties", this);
    QScrollArea *propertiesScroll = new QScrollArea;
    propertiesScroll->setWidgetResizable(true);
    propertiesScroll->setWidget(m_propertyPanel);
    propertiesDock->setWidget(propertiesScroll);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);

    QDockWidget *monitorDock = new QDockWidget("Runtime monitor", this);
    QScrollArea *monitorScroll = new QScrollArea;
    monitorScroll->setWidgetResizable(true);
    monitorScroll->setWidget(m_monitorPanel);
    monitorDock->setWidget(monitorScroll);
    addDockWidget(Qt::RightDockWidgetArea, monitorDock);

    QDockWidget *logDock = new QDockWidget("Runtime log", this);
    logDock->setWidget(m_logPanel);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void MainWindow::setMode(DiagramMode mode)
{
    m_scene->setMode(mode);
    m_selectModeAction->setChecked(mode == SelectMode);
    m_placeModeAction->setChecked(mode == AddPlaceMode);
    m_transitionModeAction->setChecked(mode == AddTransitionMode);
    m_arcModeAction->setChecked(mode == AddArcMode);
    statusBar()->showMessage("Mode: " + modeName(mode));
}

bool MainWindow::saveToFile(const QString &fileName)
{
    QStringList conversionErrors;
    PetriNet net = CoreMapper::toCoreNet(m_document, &conversionErrors);
    if (!conversionErrors.isEmpty()) {
        QMessageBox::warning(this, "Save failed",
                             "The document could not be converted to the core model:\n\n"
                             + conversionErrors.join("\n"));
        return false;
    }

    Validator validator;
    ValidationResult validation = validator.validate(net);
    if (!validation.messages.empty()) {
        QStringList lines = validationTextLines(QStringList(), validation);
        QString question = "The network has validation messages.\n"
                           "Do you want to save it anyway?\n\n" + lines.join("\n");
        QMessageBox::StandardButton button = QMessageBox::question(
            this, "Save validation warning", question,
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (button != QMessageBox::Yes) {
            statusBar()->showMessage("Save cancelled after validation.");
            return false;
        }
    }

    QString error;
    if (!DocumentSerializer::save(m_document, fileName, &error)) {
        QMessageBox::warning(this, "Save failed", error);
        return false;
    }
    setCurrentFile(fileName);
    m_modified = false;
    statusBar()->showMessage("Document saved.");
    return true;
}

bool MainWindow::maybeSave()
{
    if (!m_modified) {
        return true;
    }
    QMessageBox::StandardButton button = QMessageBox::question(
        this, "Unsaved changes", "Save current document?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (button == QMessageBox::Cancel) {
        return false;
    }
    if (button == QMessageBox::Yes) {
        saveDocument();
        return !m_modified;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    if (fileName.isEmpty()) {
        setWindowTitle("Petri Net Editor - Untitled");
    } else {
        setWindowTitle("Petri Net Editor - " + fileName);
    }
}

QString MainWindow::modeName(DiagramMode mode) const
{
    if (mode == AddPlaceMode) {
        return "Place";
    }
    if (mode == AddTransitionMode) {
        return "Transition";
    }
    if (mode == AddArcMode) {
        return "Arc";
    }
    return "Select";
}
