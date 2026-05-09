# Authors: Lubos and project team
# Description: qmake project file for the Qt Widgets application.
# Manual/generated: written manually.
# Borrowed code: none.

QT += widgets
CONFIG += c++17
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -std=c++17
TEMPLATE = app
TARGET = petri-net-editor

INCLUDEPATH += $$PWD

SOURCES += \
    app/main.cpp \
    core_api/DocumentSerializer.cpp \
    core_api/DummyRuntimeAdapter.cpp \
    core_api/PetriNetDocument.cpp \
    gui/ArcItem.cpp \
    gui/DiagramScene.cpp \
    gui/DiagramView.cpp \
    gui/LogPanel.cpp \
    gui/MainWindow.cpp \
    gui/MonitorPanel.cpp \
    gui/PlaceItem.cpp \
    gui/PropertyPanel.cpp \
    gui/TransitionItem.cpp

HEADERS += \
    core_api/DocumentSerializer.h \
    core_api/DummyRuntimeAdapter.h \
    core_api/NetTypes.h \
    core_api/PetriNetDocument.h \
    core_api/RuntimeAdapter.h \
    core_api/RuntimeSnapshot.h \
    gui/ArcItem.h \
    gui/DiagramScene.h \
    gui/DiagramView.h \
    gui/LogPanel.h \
    gui/MainWindow.h \
    gui/MonitorPanel.h \
    gui/PlaceItem.h \
    gui/PropertyPanel.h \
    gui/TransitionItem.h
