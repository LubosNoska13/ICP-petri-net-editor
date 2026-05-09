# Conceptual Design

Autori: Lubos a projektovy tim ICP.

Tento dokument je rucne pisany navrh. Prevzaty kod: ziadny.

## Vrstvy aplikacie

```text
+------------------------+
| Qt GUI                 |
| MainWindow             |
| DiagramScene/View      |
| PropertyPanel          |
| MonitorPanel, LogPanel |
+-----------+------------+
            |
            v
+------------------------+
| GUI document adapter   |
| PetriNetDocument       |
| DocumentSerializer     |
+-----------+------------+
            |
            v
+------------------------+
| Runtime adapter        |
| RuntimeAdapter         |
| DummyRuntimeAdapter    |
+-----------+------------+
            |
            v
+------------------------+
| Future core/runtime    |
| parser, validator,     |
| event-driven runtime   |
+------------------------+
```

## Hlavne triedy

- `MainWindow` sklada aplikaciu, menu, toolbar, dock panely a runtime polling.
- `DiagramScene` vytvara miesta, prechody a hrany nad `PetriNetDocument`.
- `PlaceItem`, `TransitionItem` a `ArcItem` kreslia diagramove prvky.
- `PropertyPanel` edituje vlastnosti vybraneho prvku cez dokumentovy model.
- `MonitorPanel` zobrazuje `RuntimeSnapshot` a posiela vstupy cez adapter.
- `RuntimeAdapter` je stabilne rozhranie medzi GUI a buducim runtime backendom.
- `DummyRuntimeAdapter` je docasny backend na demonstraciu monitora.

## Oddelenie GUI a core

GUI nevyhodnocuje enabled prechody, konflikty, firing ani timery. Tieto pravidla
ma dodat core/runtime cast. GUI pracuje s dokumentom a runtime snapshotom cez
male datove struktury, ktore sa daju neskor nahradit alebo napojit na finalne
core API.

## Textovy format

Docasny format GUI je sekcny textovy subor. Uklada nazov siete, komentar,
vstupy, vystupy, premenne, miesta, prechody a hrany vratane pozicii. Format je
urceny pre prve prepojenie editora so subormi a ma byt neskor zladeny s
finalnym formatom core casti.
