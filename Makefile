.PHONY: all clean python install uninstall
TARGET: all

PYTHON_BUILD_DIR := python
PYTHON := python
SUFFIX :=
CROSS_COMPILE :=
CC := $(CROSS_COMPILE)gcc
RM := rm -f
RMDIR := rm -rf
CFLAGS := -g -pipe -Os -Wall -Wsign-compare -Wcast-align -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn -finline-functions -Wmissing-format-attribute

ALL_OBJ :=
IDL_OBJ := \
	pyelf_ehdr.c pyelf_ehdr.h \
	pyelf_phdr.c pyelf_phdr.h
	
ALL_BIN :=

install:

uninstall:

python: $(IDL_OBJ)
	python setup.py build -b $(PYTHON_BUILD_DIR)

pyelf_ehdr.h: pyelf_ehdr.c
pyelf_ehdr.c: pyelf_idl.py pyelf_ehdr.py
	$(PYTHON) pyelf_ehdr.py

pyelf_phdr.h: pyelf_phdr.c
pyelf_phdr.c: pyelf_idl.py pyelf_phdr.py
	$(PYTHON) pyelf_phdr.py

%.o %.d: %.c
	@echo " [C] $(patsubst .%.d, %.c, $@)"
	@$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o, .%.d, $@) \
		-MT $(patsubst .%.d, %.o, $@) \
		-c -o $(patsubst .%.d, %.o, $@) $<

all: $(ALL_BIN) python
clean:
	$(RM) $(ALL_BIN) $(ALL_OBJ) $(IDL_OBJ) $(patsubst %.o, .%.d, $(ALL_OBJ))
	$(RMDIR) $(PYTHON_BUILD_DIR)

-include .*.d
