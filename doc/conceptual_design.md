# Conceptual Design

Autori: Lubos a projektovy tim ICP.

Tento dokument je rucne pisany navrh. Prevzaty kod: ziadny.

## Vrstvy aplikacie

```text
+-----------------------------+
| Qt GUI                      |
| MainWindow                  |
| DiagramScene / DiagramView  |
| PropertyPanel               |
| MonitorPanel, LogPanel      |
+--------------+--------------+
               |
               v
+-----------------------------+
| GUI editorovy model         |
| PetriNetDocument            |
| PlaceData, TransitionData   |
| ArcData, IoData             |
+--------------+--------------+
               |
               v
+-----------------------------+
| Adaptery medzi GUI a core   |
| CoreMapper                  |
| DocumentSerializer          |
| RuntimeAdapter interface    |
| CoreRuntimeAdapter          |
+--------------+--------------+
               |
               v
+-----------------------------+
| Core bez zavislosti od Qt   |
| PetriNet model              |
| Parser / Writer             |
| Validator                   |
| PetriRuntime, Logger        |
+-----------------------------+
```

## Hlavne triedy

- `MainWindow` sklada aplikaciu, menu, toolbar, dock panely, validaciu,
  ukladanie, nacitanie a runtime monitor.
- `DiagramScene` vytvara miesta, prechody a hrany nad `PetriNetDocument`.
- `PlaceItem`, `TransitionItem` a `ArcItem` kreslia diagramove prvky.
- `PropertyPanel` edituje vlastnosti vybraneho prvku cez dokumentovy model.
- `MonitorPanel` zobrazuje `RuntimeSnapshot` a posiela vstupy cez adapter.
- `PetriNetDocument` je jednoduchy Qt/editorovy model pouzivany v GUI.
- `CoreMapper` prevadza `PetriNetDocument` na core model `PetriNet` a spat.
- `DocumentSerializer` uklada a nacitava core textovy format cez `Writer` a
  `Parser`; pozicie diagramu uklada do samostatneho suboru `.layout`.
- `RuntimeAdapter` je stabilne rozhranie medzi GUI monitorom a runtime
  backendom.
- `CoreRuntimeAdapter` implementuje toto rozhranie nad realnym `PetriRuntime`.
- `PetriRuntime` vykonava runtime semantiku siete, planuje timery, spracuva
  vstupy a poskytuje `StateSnapshot`.
- `Validator` kontroluje zakladne invarianty siete pred ulozenim a spustenim.
- `Logger` uchovava udalosti runtime, ktore adapter prevadza do log panelu.

## Oddelenie GUI a core

GUI nevyhodnocuje enabled prechody, konflikty, firing ani timery. Tieto pravidla
su v core/runtime casti. GUI pouziva `PetriNetDocument` ako editovatelny Qt
model a pri operaciach, ktore potrebuju realnu semantiku, ho prevadza cez
`CoreMapper` na `PetriNet`.

Hranica medzi GUI a core je v troch miestach:

- validacia: `MainWindow` zavola `CoreMapper::toCoreNet(...)` a potom
  `Validator::validate(...)`,
- ulozenie a nacitanie: `DocumentSerializer` pouziva core `Writer` a `Parser`,
- monitorovanie: `CoreRuntimeAdapter` vytvori `PetriRuntime`, posiela mu vstupy
  a konvertuje `StateSnapshot` na `RuntimeSnapshot` pre Qt panely.

Core model, parser, validator a runtime nepouzivaju Qt widgety. GUI naopak
neobsahuje vlastnu kopiu pravidiel Petriho siete; iba zobrazuje dokument,
edituje hodnoty a ukazuje snapshot z runtime.

## Runtime monitor

Monitor je napojeny na realny runtime cez `CoreRuntimeAdapter`. Pri spusteni
adapter:

1. prevedie aktualny `PetriNetDocument` na `PetriNet`,
2. spusti core validaciu,
3. vytvori `PetriRuntime`,
4. zavola `initialize(true)`,
5. vracia `RuntimeSnapshot` pre diagram, monitor panel a log panel.

Vstupy zadane v monitore idu cez `RuntimeAdapter::injectInput(...)` do
`PetriRuntime::inject_input(...)`. Snapshot obsahuje marking, enabled
prechody, pending timery, posledne hodnoty vstupov, vystupov, premennych a log.

Aktualne obmedzenie: GUI pouziva `QTimer` s pevnym intervalom na volanie
`CoreRuntimeAdapter::advanceTime(...)`, ktore nasledne vola
`PetriRuntime::advance_time(...)`. Core runtime tym zostava oddeleny od GUI, ale
GUI cast este neplanuje refresh presne podla najblizsieho timeoutu.

## Textovy format

Subory siete sa ukladaju v core textovom formate cez `Writer` a nacitavaju cez
`Parser`. Format uklada nazov siete, komentar, vstupy, vystupy, premenne,
miesta, prechody, hrany, podmienky, delay a akcie podla rozsahu implementovaneho
v core casti.

Pozicie prvkov v diagrame nie su sucastou core PetriNet modelu. GUI ich uklada
do vedlajsieho suboru s priponou `.layout`, kde su pozicie mapovane podla mien
miest a prechodov. Ak layout subor chyba, `CoreMapper::fromCoreNet(...)` vytvori
automaticke rozlozenie prvkov.

## Zjednoduseny triedny vztah

```text
MainWindow
  |-- DiagramScene / DiagramView
  |-- PropertyPanel
  |-- MonitorPanel
  |-- LogPanel
  |-- PetriNetDocument
  |-- CoreRuntimeAdapter : RuntimeAdapter

DocumentSerializer
  |-- CoreMapper
  |-- Parser
  |-- Writer

CoreRuntimeAdapter
  |-- CoreMapper
  |-- Validator
  |-- PetriRuntime
        |-- PetriNet
        |-- Logger
```
