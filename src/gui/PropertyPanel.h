/**
 * @file PropertyPanel.h
 * @author Lubos and project team
 * @brief Property editor for net, places, transitions and arcs.
 * @details Manual code. Borrowed code: none.
 */

#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include "core_api/PetriNetDocument.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QStackedWidget>
#include <QWidget>

/** Dock widget content for editing the selected object. */
class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget *parent = 0);

    void setDocument(PetriNetDocument *document);
    void setSelection(const QString &id, ElementKind kind);
    void reload();

signals:
    void documentChanged();

private slots:
    void applyNet();
    void applyPlace();
    void applyTransition();
    void applyArc();

private:
    QList<IoData> parseIoLines(const QString &text) const;
    QList<VariableData> parseVariableLines(const QString &text) const;
    QString ioLines(const QList<IoData> &items) const;
    QString variableLines(const QList<VariableData> &items) const;

    PetriNetDocument *m_document;
    QString m_selectedId;
    ElementKind m_selectedKind;
    bool m_loading;

    QStackedWidget *m_stack;

    QWidget *m_netPage;
    QLineEdit *m_netName;
    QPlainTextEdit *m_netComment;
    QPlainTextEdit *m_inputs;
    QPlainTextEdit *m_outputs;
    QPlainTextEdit *m_variables;

    QWidget *m_placePage;
    QLineEdit *m_placeName;
    QSpinBox *m_placeTokens;
    QPlainTextEdit *m_placeAction;

    QWidget *m_transitionPage;
    QLineEdit *m_transitionName;
    QLineEdit *m_transitionEvent;
    QLineEdit *m_transitionGuard;
    QLineEdit *m_transitionDelay;
    QSpinBox *m_transitionPriority;
    QPlainTextEdit *m_transitionAction;

    QWidget *m_arcPage;
    QSpinBox *m_arcWeight;
};

#endif
