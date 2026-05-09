# Authors: Lubos and project team
# Description: Qt/qmake wrapper for building the editor.
# Manual/generated: written manually.
# Borrowed code: none.

.PHONY: all clean

QMAKE ?= qmake
BUILD_DIR := ../build
PROJECT := petri_net_editor.pro

all:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(QMAKE) ../src/$(PROJECT)
	$(MAKE) -C $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
