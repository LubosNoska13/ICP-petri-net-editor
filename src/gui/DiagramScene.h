/**
 * @file DiagramScene.h
 * @author Lubos and project team
 * @brief Scene that creates and displays Petri net elements.
 * @details Manual code. Borrowed code: none.
 */

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include "core_api/PetriNetDocument.h"
#include "core_api/RuntimeSnapshot.h"
#include "gui/ArcItem.h"
#include "gui/PlaceItem.h"
#include "gui/TransitionItem.h"

#include <QGraphicsScene>
#include <QMap>

/** Editing mode of the diagram scene. */
enum DiagramMode {
    SelectMode,
    AddPlaceMode,
    AddTransitionMode,
    AddArcMode
};

/** Diagram scene connected to PetriNetDocument. */
class DiagramScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit DiagramScene(QObject *parent = 0);

    void setDocument(PetriNetDocument *document);
    void setMode(DiagramMode mode);
    DiagramMode mode() const;
    QString selectedElementId() const;
    ElementKind selectedElementKind() const;
    void reloadFromDocument();
    void updateRuntimeMarking(const RuntimeSnapshot &snapshot);

signals:
    void documentChanged();
    void elementSelected(const QString &id, ElementKind kind);
    void statusMessage(const QString &text);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private slots:
    void onSelectionChanged();
    void onItemMoved(const QString &id, const QPointF &position);

private:
    QPointF centerOfElement(const QString &id) const;
    void refreshArcs();

    PetriNetDocument *m_document;
    DiagramMode m_mode;
    QString m_pendingArcSource;
    QMap<QString, QGraphicsItem *> m_elementItems;
    QMap<QString, ArcItem *> m_arcItems;
};

#endif
