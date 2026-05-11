/**
 * @file MainWindow.h
 * @author Lubos and project team
 * @brief Main application window for the Petri net editor.
 * @details Manual code. Borrowed code: none.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core_api/CoreRuntimeAdapter.h"
#include "core_api/DocumentSerializer.h"
#include "gui/DiagramScene.h"
#include "gui/DiagramView.h"
#include "gui/LogPanel.h"
#include "gui/MonitorPanel.h"
#include "gui/PropertyPanel.h"

#include <QAction>
#include <QMainWindow>
#include <QTimer>

/** Main Qt window with diagram, properties, monitor and log. */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

private slots:
    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();
    void deleteSelected();
    void validateDocument();
    void setSelectMode();
    void setPlaceMode();
    void setTransitionMode();
    void setArcMode();
    void onDocumentChanged();
    void onElementSelected(const QString &id, ElementKind kind);
    void startRuntime();
    void stopRuntime();
    void refreshRuntime();
    void injectInput(const QString &name, const QString &value);
    void updateStatusMessage(const QString &message);

private:
    void createActions();
    void createMenus();
    void createToolbar();
    void createDockWidgets();
    void setMode(DiagramMode mode);
    bool saveToFile(const QString &fileName);
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString modeName(DiagramMode mode) const;

    PetriNetDocument m_document;
    CoreRuntimeAdapter m_runtime;
    QString m_currentFile;
    bool m_modified;

    DiagramScene *m_scene;
    DiagramView *m_view;
    PropertyPanel *m_propertyPanel;
    MonitorPanel *m_monitorPanel;
    LogPanel *m_logPanel;
    QTimer *m_runtimeTimer;

    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_saveAsAction;
    QAction *m_exitAction;
    QAction *m_deleteAction;
    QAction *m_validateAction;
    QAction *m_selectModeAction;
    QAction *m_placeModeAction;
    QAction *m_transitionModeAction;
    QAction *m_arcModeAction;
    QAction *m_startAction;
    QAction *m_stopAction;
};

#endif
