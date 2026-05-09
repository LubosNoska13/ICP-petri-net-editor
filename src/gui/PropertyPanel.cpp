/**
 * @file PropertyPanel.cpp
 * @author Lubos and project team
 * @brief Implementation of the property editor panel.
 * @details Manual code. Borrowed code: none.
 */

#include "gui/PropertyPanel.h"

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent), m_document(0), m_selectedKind(UnknownElement), m_loading(false)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    m_stack = new QStackedWidget;
    root->addWidget(m_stack);

    m_netPage = new QWidget;
    QFormLayout *netLayout = new QFormLayout(m_netPage);
    m_netName = new QLineEdit;
    m_netComment = new QPlainTextEdit;
    m_inputs = new QPlainTextEdit;
    m_outputs = new QPlainTextEdit;
    m_variables = new QPlainTextEdit;
    QPushButton *applyNetButton = new QPushButton("Apply net");
    netLayout->addRow("Name", m_netName);
    netLayout->addRow("Comment", m_netComment);
    netLayout->addRow("Inputs", m_inputs);
    netLayout->addRow("Outputs", m_outputs);
    netLayout->addRow("Variables", m_variables);
    netLayout->addRow(applyNetButton);
    m_stack->addWidget(m_netPage);

    m_placePage = new QWidget;
    QFormLayout *placeLayout = new QFormLayout(m_placePage);
    m_placeName = new QLineEdit;
    m_placeTokens = new QSpinBox;
    m_placeTokens->setRange(0, 1000000);
    m_placeAction = new QPlainTextEdit;
    QPushButton *applyPlaceButton = new QPushButton("Apply place");
    placeLayout->addRow("Name", m_placeName);
    placeLayout->addRow("Tokens", m_placeTokens);
    placeLayout->addRow("Action", m_placeAction);
    placeLayout->addRow(applyPlaceButton);
    m_stack->addWidget(m_placePage);

    m_transitionPage = new QWidget;
    QFormLayout *transitionLayout = new QFormLayout(m_transitionPage);
    m_transitionName = new QLineEdit;
    m_transitionEvent = new QLineEdit;
    m_transitionGuard = new QLineEdit;
    m_transitionDelay = new QSpinBox;
    m_transitionDelay->setRange(0, 100000000);
    m_transitionPriority = new QSpinBox;
    m_transitionPriority->setRange(-1000000, 1000000);
    m_transitionAction = new QPlainTextEdit;
    QPushButton *applyTransitionButton = new QPushButton("Apply transition");
    transitionLayout->addRow("Name", m_transitionName);
    transitionLayout->addRow("Event", m_transitionEvent);
    transitionLayout->addRow("Guard", m_transitionGuard);
    transitionLayout->addRow("Delay ms", m_transitionDelay);
    transitionLayout->addRow("Priority", m_transitionPriority);
    transitionLayout->addRow("Action", m_transitionAction);
    transitionLayout->addRow(applyTransitionButton);
    m_stack->addWidget(m_transitionPage);

    m_arcPage = new QWidget;
    QFormLayout *arcLayout = new QFormLayout(m_arcPage);
    m_arcWeight = new QSpinBox;
    m_arcWeight->setRange(1, 1000000);
    QPushButton *applyArcButton = new QPushButton("Apply arc");
    arcLayout->addRow("Weight", m_arcWeight);
    arcLayout->addRow(applyArcButton);
    m_stack->addWidget(m_arcPage);

    connect(applyNetButton, SIGNAL(clicked()), this, SLOT(applyNet()));
    connect(applyPlaceButton, SIGNAL(clicked()), this, SLOT(applyPlace()));
    connect(applyTransitionButton, SIGNAL(clicked()), this, SLOT(applyTransition()));
    connect(applyArcButton, SIGNAL(clicked()), this, SLOT(applyArc()));
}

void PropertyPanel::setDocument(PetriNetDocument *document)
{
    m_document = document;
    reload();
}

void PropertyPanel::setSelection(const QString &id, ElementKind kind)
{
    m_selectedId = id;
    m_selectedKind = kind;
    reload();
}

void PropertyPanel::reload()
{
    if (!m_document) {
        return;
    }
    m_loading = true;

    if (m_selectedKind == PlaceElement) {
        PlaceData place;
        if (m_document->placeById(m_selectedId, &place)) {
            m_placeName->setText(place.name);
            m_placeTokens->setValue(place.tokens);
            m_placeAction->setPlainText(place.action);
            m_stack->setCurrentWidget(m_placePage);
        }
    } else if (m_selectedKind == TransitionElement) {
        TransitionData transition;
        if (m_document->transitionById(m_selectedId, &transition)) {
            m_transitionName->setText(transition.name);
            m_transitionEvent->setText(transition.event);
            m_transitionGuard->setText(transition.guard);
            m_transitionDelay->setValue(transition.delayMs);
            m_transitionPriority->setValue(transition.priority);
            m_transitionAction->setPlainText(transition.action);
            m_stack->setCurrentWidget(m_transitionPage);
        }
    } else if (m_selectedKind == ArcElement) {
        ArcData arc;
        if (m_document->arcById(m_selectedId, &arc)) {
            m_arcWeight->setValue(arc.weight);
            m_stack->setCurrentWidget(m_arcPage);
        }
    } else {
        m_netName->setText(m_document->netName());
        m_netComment->setPlainText(m_document->comment());
        m_inputs->setPlainText(ioLines(m_document->inputs()));
        m_outputs->setPlainText(ioLines(m_document->outputs()));
        m_variables->setPlainText(variableLines(m_document->variables()));
        m_stack->setCurrentWidget(m_netPage);
    }

    m_loading = false;
}

void PropertyPanel::applyNet()
{
    if (!m_document || m_loading) {
        return;
    }
    m_document->setNetName(m_netName->text());
    m_document->setComment(m_netComment->toPlainText());
    m_document->setInputs(parseIoLines(m_inputs->toPlainText()));
    m_document->setOutputs(parseIoLines(m_outputs->toPlainText()));
    m_document->setVariables(parseVariableLines(m_variables->toPlainText()));
    emit documentChanged();
}

void PropertyPanel::applyPlace()
{
    PlaceData place;
    if (!m_document || !m_document->placeById(m_selectedId, &place)) {
        return;
    }
    place.name = m_placeName->text();
    place.tokens = m_placeTokens->value();
    place.action = m_placeAction->toPlainText();
    m_document->updatePlace(place);
    emit documentChanged();
}

void PropertyPanel::applyTransition()
{
    TransitionData transition;
    if (!m_document || !m_document->transitionById(m_selectedId, &transition)) {
        return;
    }
    transition.name = m_transitionName->text();
    transition.event = m_transitionEvent->text();
    transition.guard = m_transitionGuard->text();
    transition.delayMs = m_transitionDelay->value();
    transition.priority = m_transitionPriority->value();
    transition.action = m_transitionAction->toPlainText();
    m_document->updateTransition(transition);
    emit documentChanged();
}

void PropertyPanel::applyArc()
{
    ArcData arc;
    if (!m_document || !m_document->arcById(m_selectedId, &arc)) {
        return;
    }
    arc.weight = m_arcWeight->value();
    m_document->updateArc(arc);
    emit documentChanged();
}

QList<IoData> PropertyPanel::parseIoLines(const QString &text) const
{
    QList<IoData> result;
    QStringList lines = text.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines.at(i).trimmed();
        if (line.isEmpty()) {
            continue;
        }
        QStringList parts = line.split('=');
        IoData item;
        item.name = parts.at(0).trimmed();
        item.lastValue = parts.size() > 1 ? parts.mid(1).join("=").trimmed() : QString();
        result.append(item);
    }
    return result;
}

QList<VariableData> PropertyPanel::parseVariableLines(const QString &text) const
{
    QList<VariableData> result;
    QStringList lines = text.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines.at(i).trimmed();
        if (line.isEmpty()) {
            continue;
        }
        QStringList parts = line.split('=');
        VariableData item;
        item.name = parts.at(0).trimmed();
        item.initialValue = parts.size() > 1 ? parts.mid(1).join("=").trimmed() : QString();
        result.append(item);
    }
    return result;
}

QString PropertyPanel::ioLines(const QList<IoData> &items) const
{
    QStringList lines;
    for (int i = 0; i < items.size(); ++i) {
        lines << (items.at(i).name + "=" + items.at(i).lastValue);
    }
    return lines.join("\n");
}

QString PropertyPanel::variableLines(const QList<VariableData> &items) const
{
    QStringList lines;
    for (int i = 0; i < items.size(); ++i) {
        lines << (items.at(i).name + "=" + items.at(i).initialValue);
    }
    return lines.join("\n");
}
