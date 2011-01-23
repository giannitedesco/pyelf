/*
 * This file is part of pyelf
 * Copyright (c) 2010 Gianni Tedesco <gianni@scaramanga.co.uk>
 * Released under the terms of the GNU GPL version 3
*/

#include "py_elf.h"
#include "pyelf_ehdr.h"
#include "pyelf_phdr.h"

/* Exception hierarchy */
static PyObject *pyelf_err_base;
static PyObject 	*pyelf_err_fmt;

/* Types */
struct pyelf_elf {
	PyObject_HEAD;
	PyObject *file;
	Elf *elf;
};

/* Generic functions */
static void pyelf_error(void)
{
	PyErr_SetString(PyExc_SystemError, elf_errmsg(elf_errno()));
}

/* ELF files */
static int pyelf_elf_init(struct pyelf_elf *self, PyObject *args, PyObject *kwds)
{
	PyObject *file;
	FILE *tmp;

	if ( !PyArg_ParseTuple(args, "O", &file) )
		return -1;
	
	if ( !PyFile_Check(file) ) {
		PyErr_SetString(PyExc_TypeError, "Expected file object");
		return -1;
	}

	tmp = PyFile_AsFile(file);
	if ( NULL == tmp )
		return -1;

	self->elf = elf_begin(fileno(tmp), ELF_C_READ, NULL);
	if ( NULL == self->elf ) {
		pyelf_error();
		return -1;
	}

	switch(elf_kind(self->elf)) {
	case ELF_K_ELF:
		break;
	default:
		PyErr_SetString(pyelf_err_fmt, "Not a valid ELF object");
		return -1;
	}

	self->file = file;
	Py_INCREF(self->file);

	return 0;
}

static void pyelf_elf_dealloc(struct pyelf_elf *self)
{
	if ( self->elf )
		elf_end(self->elf);
	if ( self->file ) {
		Py_DECREF(self->file);
	}
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *pyelf_elf_kind_get(struct pyelf_elf *self)
{
	return PyInt_FromLong(elf_kind(self->elf));
}

static PyObject *pyelf_elf_bits_get(struct pyelf_elf *self)
{
	int ret;

	switch(gelf_getclass(self->elf)) {
	case ELFCLASS32:
		ret = 32;
		break;
	case ELFCLASS64:
		ret = 64;
		break;
	default:
		PyErr_SetString(pyelf_err_fmt, "Undetermined ELF format");
		return NULL;
	}

	return PyInt_FromLong(ret);
}

static PyObject *pyelf_elf_phdr_get(struct pyelf_elf *self)
{
	PyObject *list, *tmp;
	GElf_Ehdr gehdr;
	GElf_Phdr gphdr;
	int i;

	if ( gelf_getehdr(self->elf, &gehdr) == NULL ) {
		pyelf_error();
		return NULL;
	}

	list = PyList_New(gehdr.e_phnum);

	for(i = 0; i < gehdr.e_phnum; i++) {
		if ( gelf_getphdr(self->elf, i, &gphdr) == NULL )
			goto err_free;
		tmp = pyelf_phdr_New(&gphdr);
		if ( NULL == tmp )
			goto err_free;
		PyList_SetItem(list, i, tmp);
	}

	return list;

err_free:
	pyelf_error();
	Py_DECREF(list);
	return NULL;
}

#if 0
static PyObject *pyelf_elf_shdr_get(struct pyelf_elf *self)
{
	PyObject *list, *tmp;
	GElf_Shdr gshdr;
	size_t i, num;

	if ( elf_getshdrnum(self->elf, &num) ) {
		pyelf_error();
		return NULL;
	}

	list = PyList_New(num);

	for(i = 0; i < num; i++) {
		if ( gelf_getshdr(self->elf, i, &gshdr) == NULL )
			goto err_free;
		tmp = pyelf_shdr_New(&gshdr);
		if ( NULL == tmp )
			goto err_free;
		PyList_SetItem(list, i, tmp);
	}

	return list;

err_free:
	pyelf_error();
	Py_DECREF(list);
	return NULL;
}
#endif

static PyObject *pyelf_elf_ehdr_get(struct pyelf_elf *self)
{
	GElf_Ehdr gehdr;

	if ( gelf_getehdr(self->elf, &gehdr) == NULL ) {
		pyelf_error();
		return NULL;
	}

	return pyelf_ehdr_New(&gehdr);
}

static PyMethodDef pyelf_elf_methods[] = {
	{NULL, }
};

static PyGetSetDef pyelf_elf_attribs[] = {
	{"kind", (getter)pyelf_elf_kind_get, NULL, "File format"},
	{"bits", (getter)pyelf_elf_bits_get, NULL, "File format"},
	{"ehdr", (getter)pyelf_elf_ehdr_get, NULL, "File class"},
	{"phdr", (getter)pyelf_elf_phdr_get, NULL, "File class"},
	//{"shdr", (getter)pyelf_elf_shdr_get, NULL, "File class"},
	{NULL, }
};

static PyTypeObject elf_pytype = {
	PyObject_HEAD_INIT(NULL)
	.tp_name = PACKAGE ".elf",
	.tp_basicsize = sizeof(struct pyelf_elf),
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_new = PyType_GenericNew,
	.tp_init = (initproc)pyelf_elf_init,
	.tp_dealloc = (destructor)pyelf_elf_dealloc,
	.tp_methods = pyelf_elf_methods,
	.tp_getset = pyelf_elf_attribs,
	.tp_doc = "ELF file",
};

static PyMethodDef methods[] = {
	{NULL, }
};

PyMODINIT_FUNC initelf(void)
{
	PyObject *m;

	if ( elf_version(EV_CURRENT) == EV_NONE )
		return;

	if ( PyType_Ready(&elf_pytype) < 0 )
		return;
	if ( PyType_Ready(&pyelf_ehdr_pytype) < 0 )
		return;
	if ( PyType_Ready(&pyelf_phdr_pytype) < 0 )
		return;

	pyelf_err_base = PyErr_NewException(PACKAGE ".Error",
						PyExc_RuntimeError, NULL);
	if ( NULL == pyelf_err_base )
		return;
	pyelf_err_fmt = PyErr_NewException(PACKAGE ".FormatError",
						pyelf_err_base, NULL);
	if ( NULL == pyelf_err_fmt )
		return;

	m = Py_InitModule3(PACKAGE, methods, "ELF library");
	if ( NULL == m )
		return;

	PyModule_AddObject(m, "Error", pyelf_err_base);
	PyModule_AddObject(m, "FormatError", pyelf_err_fmt);
	PyModule_AddObject(m, "elf", (PyObject *)&elf_pytype);
	PyModule_AddObject(m, "ehdr", (PyObject *)&pyelf_ehdr_pytype);
	PyModule_AddObject(m, "phdr", (PyObject *)&pyelf_phdr_pytype);
}
