# Authors: Lubos and project team
# Description: Root build commands for the Petri net editor project.
# Manual/generated: written manually.
# Borrowed code: none.

.PHONY: all run clean doxygen pack

QMAKE ?= qmake
APP := build/petri-net-editor
PACK_NAME := petri-net-editor.tar.gz
PACK_TMP := /tmp/$(PACK_NAME)

all:
	$(MAKE) -C src QMAKE="$(QMAKE)"

run: all
	./$(APP)

clean:
	$(MAKE) -C src clean
	rm -rf build $(PACK_NAME)

doxygen:
	doxygen Doxyfile

pack: clean
	rm -f $(PACK_TMP)
	tar --exclude='./.git' \
	    --exclude='./.codex' \
	    --exclude='./.agents' \
	    --exclude='./build' \
	    --exclude='./doc/html' \
	    --exclude='./$(PACK_NAME)' \
	    -czf $(PACK_TMP) .
	mv $(PACK_TMP) $(PACK_NAME)
