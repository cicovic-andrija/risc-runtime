# System software project
# Makefile for managing the build of the entire project.
#

SHELL=/bin/bash

ASSEMBLER_DIR=assembler
EMULATOR_DIR=emulator
LINKER_DIR=linker
DOC_DIR=doc

all: assembler linker emulator doc

assembler:
	$(MAKE) -C $(ASSEMBLER_DIR)

linker:
	$(MAKE) -C $(LINKER_DIR)

emulator:
	$(MAKE) -C $(EMULATOR_DIR)

doc:
	$(MAKE) -C $(DOC_DIR)

OBJDIR=obj
clean:
	$(MAKE) -C $(ASSEMBLER_DIR) clean
	$(MAKE) -C $(LINKER_DIR) clean
	$(MAKE) -C $(EMULATOR_DIR) clean
	$(MAKE) -C $(DOC_DIR) clean

.PHONY: all assembler linker emulator doc clean
