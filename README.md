# ICP-petri-net-editor

## PROJEKT 

**Názov projektu**: Interpretovaná Petriho sieť

**Popis projektu**: Nástroj pro vizuální editaci, generování kódu a monitorování běhu interpretovaných Petriho sítí 
(event-driven SFC/Grafcet s multiset sémantikou)

**Predmet**: Seminár C++ (ICP)

## AUTORI

### Mená:

Jurišinová Daniela (xjurisd00)

Ľuboš Noska (xnoskal00)

### Popis rozdelenia práce:

Jurišinová Daniela:
- dátový model siete
- parser textového formátu
- writer textového formátu
- validácia siete
- runtime sémantika
- časované prechody, timery, event loop
- vyhodnocovanie stráží + akcií

Ľuboš Noska:
- aplikácia, hlavné okno, menu, panely
- vizuálny editor diagramu
- property editor IN, OUT, premenných, podmienok a akcií
- live monitor
- log udalostí
- panel na injekciu vstupov
- prepojenie GUI s runtime API modelom
- Makefile (make run, make clean, make doxygen, make pack)

## RIEŠENIE

### Známe obmedzenia:

- GUI ukladá pozície prvkov diagramu do pomocného súboru s príponou `.layout`; hlavný `.pn` súbor zostáva v textovom formáte core časti.
- Ak `.layout` súbor chýba, GUI rozmiestni miesta a prechody automaticky.
- Pri ukladaní môže používateľ po upozornení uložiť aj sieť s validačnými chybami, aby sa dala zachovať rozpracovaná práca. Technické chyby prevodu alebo zápisu súboru však uloženie zastavia.
- Validačné správy z menu Validate sa v log paneli zobrazujú ako aktuálny výsledok kontroly. Nie sú trvalo miešané s runtime logom a ďalší refresh runtime monitora ich môže prepísať.
- Runtime monitor v GUI používa Qt timer na periodické volanie `advanceTime(...)`; nejde o plné Qt plánovanie najbližšieho timeoutu podľa udalostí core runtime.
- Testovací adresár je pripravený, ale aktuálny `tests/Makefile` zatiaľ neobsahuje spustiteľný cieľ.

### Vývojové rozhodnutia:

- GUI používa vlastný Qt dokument `PetriNetDocument` iba ako editorový model. Pred uložením, validáciou a spustením sa dokument konvertuje na core `PetriNet`.
- GUI editor priamo nevolá core edit helpery ako `PetriNet::add_place()` alebo `PetriNet::add_transition()`, pretože potrebuje vlastné ID prvkov, pozície v diagrame a grafické hrany. Core sa používa cez `CoreMapper`, `Validator`, `DocumentSerializer` a `CoreRuntimeAdapter`.
- Runtime monitor je pripojený cez `CoreRuntimeAdapter` na `PetriRuntime`. GUI nemení sémantiku enabled prechodov, konfliktov ani firing-u.

### Štruktúra:

.  
├── Doxyfile  
├── Makefile  
├── README.txt  
├── doc  
│   └── koncept  
├── examples  
│   ├── conflict_example.pn  
│   ├── multiset_example.pn  
│   ├── tof_pn.pn  
│   └── tof_pn_5s.pn  
├── src  
│   ├── Makefile  
│   ├── core_api  
│   │   ├── CoreMapper.cpp/h  
│   │   ├── CoreRuntimeAdapter.cpp/h  
│   │   ├── DocumentSerializer.cpp/h  
│   │   └── PetriNetDocument.cpp/h  
│   ├── gui  
│   │   ├── MainWindow.cpp/h  
│   │   ├── DiagramScene.cpp/h  
│   │   └── panely a grafické prvky editora  
│   ├── evaluator.cpp  
│   ├── evaluator.hpp  
│   ├── logger.cpp  
│   ├── logger.hpp  
│   ├── main.cpp  
│   ├── model.cpp  
│   ├── model.hpp  
│   ├── parser.cpp  
│   ├── parser.hpp  
│   ├── runtime.cpp  
│   ├── runtime.hpp  
│   ├── validate.cpp  
│   └── validate.hpp  
└── tests  
    ├── Makefile  
    ├── test_main.cpp  
    ├── test_parser.cpp  
    ├── test_runtime.cpp  
    └── test_validate.cpp  

### Popis implementačných súborov:

1. model - pomocné metódy dátového modelu (vyhľadávanie miest, prechodov, vstupov, výstupov podľa názvu a i.)
2. parser - parser textového .pn formátu (načítanie sekcie ako názov a i.)
3. validate - kontrola správnosti siete (everenie duplicitných názvov, existencia hrán, platné váhy hrán a i.)
4. evaluator - základné vyhodnocovanie podporovanej podmnožiny C/C++ výrazov (spracovanie podmienok, int výrazy, priradenia a i.)
5. runtime - sémantika vykonávania siete (enabled prechody, konflikty, firing, zmeny tokenov a i.)
6. logger - ukladanie a vypisovanie logov (ukladá čas, typ a textový popis)
7. main - konzolový vstupný bod aplikácie (načítanie .pn, spustenie validácie, inicializácia runtime a i.)
8. core_api/CoreMapper - prevod medzi Qt editorovým dokumentom a core modelom PetriNet
9. core_api/CoreRuntimeAdapter - napojenie GUI monitora na PetriRuntime
10. core_api/DocumentSerializer - uloženie a načítanie cez core Parser/Writer a .layout súbor

## TESTOVANIE

### Testované na:

- Linux, Qt 5, g++ s podporou C++17

### Príklady manuálneho testovania:

- `make clean && make`
- `make doxygen`
- `make pack`
- otvorenie `examples/tof_pn_5s.pn` v GUI, spustenie runtime a injektovanie vstupov `in=1`, `in=0`
- overenie, že uloženie vytvorí čistý `.pn` súbor a pomocný `.pn.layout` súbor s pozíciami
