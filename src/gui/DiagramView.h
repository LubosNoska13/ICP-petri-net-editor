/**
 * @file DiagramView.h
 * @author Lubos and project team
 * @brief QGraphicsView wrapper for the Petri net diagram.
 * @details Manual code. Borrowed code: none.
 */

#ifndef DIAGRAMVIEW_H
#define DIAGRAMVIEW_H

#include <QGraphicsView>

/** View with antialiasing and mouse-wheel zoom. */
class DiagramView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit DiagramView(QWidget *parent = 0);

protected:
    void wheelEvent(QWheelEvent *event);
};

#endif
