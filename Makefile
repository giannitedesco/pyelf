.PHONY: all clean python install uninstall
TARGET: all

PYTHON_BUILD_DIR = python
SUFFIX :=
CROSS_COMPILE :=
CC := $(CROSS_COMPILE)gcc
RM := rm -f
RMDIR := rm -rf
CFLAGS := -g -pipe -Os -Wall -Wsign-compare -Wcast-align -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn -finline-functions -Wmissing-format-attribute

ALL_OBJ :=
ALL_BIN :=

install:

uninstall:

python:
	python setup.py build -b $(PYTHON_BUILD_DIR)

%.o %.d: %.c
	@echo " [C] $(patsubst .%.d, %.c, $@)"
	@$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o, .%.d, $@) \
		-MT $(patsubst .%.d, %.o, $@) \
		-c -o $(patsubst .%.d, %.o, $@) $<

all: $(ALL_BIN) python
clean:
	$(RM) $(ALL_BIN) $(ALL_OBJ) $(patsubst %.o, .%.d, $(ALL_OBJ))
	$(RMDIR) $(PYTHON_BUILD_DIR)

-include .*.d
