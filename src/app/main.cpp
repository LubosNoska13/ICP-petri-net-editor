/**
 * @file main.cpp
 * @author Lubos and project team
 * @brief Application entry point for the Petri net editor.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Petri Net Editor");

    MainWindow window;
    window.show();

    return app.exec();
}
