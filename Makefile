.PHONY: all clean python install uninstall
TARGET: all

PYTHON_BUILD_DIR := python
PYTHON_STAMP := python-stamp
PYTHON := python
SUFFIX :=
CROSS_COMPILE :=
CC := $(CROSS_COMPILE)gcc
RM := rm -f
TOUCH := touch
RMDIR := rm -rf
CFLAGS := -g -pipe -Os -Wall -Wsign-compare -Wcast-align -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn -finline-functions -Wmissing-format-attribute

ALL_OBJ :=
IDL_OBJ := \
	pyelf_ehdr.c pyelf_ehdr.h \
	pyelf_phdr.c pyelf_phdr.h \
	pyelf_shdr.c pyelf_shdr.h
	
ALL_BIN :=

install:

uninstall:

ifeq ($(filter clean, $(MAKECMDGOALS)),clean)
CLEAN_DEP := clean
else
CLEAN_DEP :=
endif

$(PYTHON_STAMP): $(CLEAN_DEP) $(IDL_OBJ)
	$(PYTHON) setup.py build -b $(PYTHON_BUILD_DIR)
	$(TOUCH) $(PYTHON_STAMP)

python: $(CLEAN_DEP) $(PYTHON_STAMP)

pyelf_ehdr.h: pyelf_ehdr.c
pyelf_ehdr.c: pyelf_ehdr.py pyelf_idl.py $(CLEAN_DEP)
	$(PYTHON) pyelf_ehdr.py

pyelf_phdr.h: pyelf_phdr.c
pyelf_phdr.c: pyelf_phdr.py pyelf_idl.py $(CLEAN_DEP)
	$(PYTHON) pyelf_phdr.py

pyelf_shdr.h: pyelf_shdr.c
pyelf_shdr.c: pyelf_shdr.py pyelf_idl.py $(CLEAN_DEP)
	$(PYTHON) pyelf_shdr.py

%.o %.d: %.c
	@echo " [C] $(patsubst .%.d, %.c, $@)"
	@$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o, .%.d, $@) \
		-MT $(patsubst .%.d, %.o, $@) \
		-c -o $(patsubst .%.d, %.o, $@) $<

all: $(ALL_BIN) python
clean:
	$(RM) $(ALL_BIN) $(ALL_OBJ) $(IDL_OBJ) $(PYTHON_STAMP) $(patsubst %.o, .%.d, $(ALL_OBJ))
	$(RMDIR) $(PYTHON_BUILD_DIR)

ifneq ($(MAKECMDGOALS),clean)
-include .*.d
endif
