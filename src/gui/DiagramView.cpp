/**
 * @file DiagramView.cpp
 * @author Lubos and project team
 * @brief Implementation of the diagram view.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/DiagramView.h"

#include <QPainter>
#include <QWheelEvent>

DiagramView::DiagramView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setSceneRect(-1000, -800, 2000, 1600);
}

void DiagramView::wheelEvent(QWheelEvent *event)
{
    const double factor = event->delta() > 0 ? 1.15 : 0.85;
    scale(factor, factor);
}
