/*
 * This file is part of pyelf
 * Copyright (c) 2010 Gianni Tedesco <gianni@scaramanga.co.uk>
 * Released under the terms of the GNU GPL version 3
*/

#include "py_elf.h"
#include "pyelf_ehdr.h"
#include "pyelf_phdr.h"
#include "pyelf_shdr.h"

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

PyObject *pyelf_shdr_type_get(struct pyelf_shdr *self)
{
	const char *str;

	switch(self->shdr.sh_type) {
	case SHT_NULL:
		str = "NULL";
		break;
	case SHT_PROGBITS:
		str = "PROGBITS";
		break;
	case SHT_SYMTAB:
		str = "SYMTAB";
		break;
	case SHT_STRTAB:
		str = "STRTAB";
		break;
	case SHT_RELA:
		str = "RELA";
		break;
	case SHT_HASH:
		str = "HASH";
		break;
	case SHT_DYNAMIC:
		str = "DYNAMIC";
		break;
	case SHT_NOTE:
		str = "NOTE";
		break;
	case SHT_NOBITS:
		str = "NOBITS";
		break;
	case SHT_REL:
		str = "REL";
		break;
	case SHT_SHLIB:
		str = "SHLIB";
		break;
	case SHT_DYNSYM:
		str = "DYNSYM";
		break;
	case SHT_INIT_ARRAY:
		str = "INIT";
		break;
	case SHT_FINI_ARRAY:
		str = "FINI";
		break;
	default:
		Py_INCREF(Py_None);
		return Py_None;
	}

	return PyString_FromString(str);
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

static int get_id_byte(struct pyelf_elf *self, size_t ofs, uint8_t *byte)
{
	const char *id;
	size_t sz;

	id = elf_getident(self->elf, &sz);
	if ( sz < ofs + 1 ) {
		pyelf_error();
		return 0;
	}

	*byte = id[ofs];
	return 1;
}

static PyObject *pyelf_elf_data_get(struct pyelf_elf *self)
{
	char *str;
	uint8_t data;

	if ( !get_id_byte(self, EI_DATA, &data) )
		return NULL;

	switch(data) {
	case ELFDATA2LSB:
		str = "2's complement, little endian";
		break;
	case ELFDATA2MSB:
		str = "2's complement, big endian";
		break;
	default:
		Py_INCREF(Py_None);
		return Py_None;
	}

	return PyString_FromString(str);
}

static PyObject *pyelf_elf_machine_get(struct pyelf_elf *self)
{
	GElf_Ehdr gehdr;
	char *str;

	if ( gelf_getehdr(self->elf, &gehdr) == NULL ) {
		pyelf_error();
		return NULL;
	}

	switch(gehdr.e_machine) {
	case EM_386:
		str = "Intel 80386";
		break;
	case EM_X86_64:
		str = "Intel x86-64";
		break;
	case EM_PPC:
		str = "IBM PowerPC";
		break;
	case EM_PPC64:
		str = "IBM PowerPC 64";
		break;
	default:
		Py_INCREF(Py_None);
		return Py_None;
	}

	return PyString_FromString(str);
}

static PyObject *pyelf_elf_type_get(struct pyelf_elf *self)
{
	GElf_Ehdr gehdr;
	char *str;

	if ( gelf_getehdr(self->elf, &gehdr) == NULL ) {
		pyelf_error();
		return NULL;
	}

	switch(gehdr.e_type) {
	case ET_REL:
		str = "REL (Relocatable file)";
		break;
	case ET_EXEC:
		str = "EXEC (Executable file)";
		break;
	case ET_DYN:
		str = "DYN (Shared object file)";
		break;
	case ET_CORE:
		str = "CORE (Core file)";
		break;
	default:
		Py_INCREF(Py_None);
		return Py_None;
	}

	return PyString_FromString(str);
}

static PyObject *pyelf_elf_osabi_get(struct pyelf_elf *self)
{
	char *str;
	uint8_t osabi;

	if ( !get_id_byte(self, EI_OSABI, &osabi) )
		return NULL;

	switch(osabi) {
	case ELFOSABI_SYSV:
		str = "UNIX - System V";
		break;
	case ELFOSABI_HPUX:
		str = "HP/UX";
		break;
	case ELFOSABI_NETBSD:
		str = "NetBSD";
		break;
	case ELFOSABI_LINUX:
		str = "Linux";
		break;
	case ELFOSABI_SOLARIS:
		str = "Solaris";
		break;
	case ELFOSABI_AIX:
		str = "AIX";
		break;
	case ELFOSABI_IRIX:
		str = "Irix";
		break;
	case ELFOSABI_FREEBSD:
		str = "FreeBSD";
		break;
	case ELFOSABI_TRU64:
		str = "Tru64";
		break;
	case ELFOSABI_MODESTO:
		str = "Modesto";
		break;
	case ELFOSABI_OPENBSD:
		str = "OpenBSD";
		break;
	case ELFOSABI_ARM:
		str = "ARM";
		break;
	case ELFOSABI_STANDALONE:
		str = "Standalone";
		break;
	default:
		Py_INCREF(Py_None);
		return Py_None;
	}

	return PyString_FromString(str);
}

static PyObject *pyelf_elf_file_get(struct pyelf_elf *self)
{
	Py_INCREF(self->file);
	return self->file;
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
	int i;

	if ( gelf_getehdr(self->elf, &gehdr) == NULL ) {
		pyelf_error();
		return NULL;
	}

	list = PyList_New(gehdr.e_phnum);

	for(i = 0; i < gehdr.e_phnum; i++) {
		GElf_Phdr gphdr;

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

static PyObject *pyelf_elf_shdr_get(struct pyelf_elf *self)
{
	PyObject *list, *tmp;
	size_t i, num;

	if ( elf_getshdrnum(self->elf, &num) ) {
		pyelf_error();
		return NULL;
	}

	list = PyList_New(num);

	for(i = 0; i < num; i++) {
		Elf_Scn *scn;
		GElf_Shdr gshdr;

		scn = elf_getscn(self->elf, i);
		if ( NULL == scn )
			goto err_free;

		if ( gelf_getshdr(scn, &gshdr) == NULL )
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

static PyObject *pyelf_elf_ehdr_get(struct pyelf_elf *self)
{
	GElf_Ehdr gehdr;

	if ( gelf_getehdr(self->elf, &gehdr) == NULL ) {
		pyelf_error();
		return NULL;
	}

	return pyelf_ehdr_New(&gehdr);
}

static int pyelf_name_from_pyobj(PyObject *arg, size_t *off)
{
	/* Argh, overflows :( */
	if ( PyInt_Check(arg) ) {
		unsigned long l;
		l = PyInt_AsUnsignedLongMask(arg);
		*off = l;
	}else if ( PyLong_Check(arg) ) {
		unsigned long long l;
		l = PyLong_AsUnsignedLongLong(arg);
		*off = l;
	}else if ( pyelf_shdr_Check(arg) ) {
		struct pyelf_shdr *shdr = (struct pyelf_shdr *)arg;
		*off = shdr->shdr.sh_name;
	}else{
		PyErr_SetString(PyExc_TypeError,
				"Expected integer or section header object");
		return 0;
	}

	return 1;
}

static PyObject *pyelf_elf_str(struct pyelf_elf *self, PyObject *args)
{
	size_t off, strtab;
	PyObject *arg;
	char *ret;

	if ( !PyArg_ParseTuple(args, "O", &arg) )
		return NULL;

	if ( !pyelf_name_from_pyobj(arg, &off) )
		return NULL;

	if ( elf_getshdrstrndx(self->elf, &strtab) ) {
		pyelf_error();
		return NULL;
	}

	ret = elf_strptr(self->elf, strtab, off);
	if ( NULL == ret || !strlen(ret) ) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	return PyString_FromString(ret);
}

static int pyelf_off_from_pyobj(PyObject *arg, size_t *off)
{
	/* Argh, overflows :( */
	if ( PyInt_Check(arg) ) {
		unsigned long l;
		l = PyInt_AsUnsignedLongMask(arg);
		*off = l;
	}else if ( PyLong_Check(arg) ) {
		unsigned long long l;
		l = PyLong_AsUnsignedLongLong(arg);
		*off = l;
	}else if ( pyelf_shdr_Check(arg) ) {
		struct pyelf_shdr *shdr = (struct pyelf_shdr *)arg;
		*off = shdr->shdr.sh_offset;
	}else{
		PyErr_SetString(PyExc_TypeError,
				"Expected integer or section header object");
		return 0;
	}

	return 1;
}

static PyObject *pyelf_elf_rawdata(struct pyelf_elf *self, PyObject *args)
{
	size_t off;
	Elf_Scn *scn;
	PyObject *arg;

	if ( !PyArg_ParseTuple(args, "O", &arg) )
		return NULL;

	if ( !pyelf_off_from_pyobj(arg, &off) )
		return NULL;

	for(scn = NULL; (scn = elf_nextscn(self->elf, scn)); ) {
		GElf_Shdr gshdr;
		if ( gelf_getshdr(scn, &gshdr) == NULL )
			continue;
		if ( !gshdr.sh_size || !gshdr.sh_offset )
			continue;
		if ( gshdr.sh_offset == off ) {
			Elf_Data *raw;

			raw = elf_rawdata(scn, NULL);
			if ( NULL == raw ) {
				pyelf_error();
				return NULL;
			}

			return PyByteArray_FromStringAndSize(raw->d_buf,
								raw->d_size);
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *pyelf_elf_data(struct pyelf_elf *self, PyObject *args)
{
	size_t off;
	Elf_Scn *scn;
	PyObject *arg;

	if ( !PyArg_ParseTuple(args, "O", &arg) )
		return NULL;

	if ( !pyelf_off_from_pyobj(arg, &off) )
		return NULL;

	for(scn = NULL; (scn = elf_nextscn(self->elf, scn)); ) {
		GElf_Shdr gshdr;
		if ( gelf_getshdr(scn, &gshdr) == NULL )
			continue;
		if ( !gshdr.sh_size || !gshdr.sh_offset )
			continue;
		if ( gshdr.sh_offset == off ) {
			Elf_Data *raw;

			raw = elf_getdata(scn, NULL);
			if ( NULL == raw ) {
				pyelf_error();
				return NULL;
			}

			return PyByteArray_FromStringAndSize(raw->d_buf,
								raw->d_size);
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef pyelf_elf_methods[] = {
	{"rawdata",(PyCFunction)pyelf_elf_rawdata, METH_VARARGS,
		"elf.rawdata(shdr) - Get raw file data"},
	{"data",(PyCFunction)pyelf_elf_data, METH_VARARGS,
		"elf.data(shdr) - Get translated file data"},
	{"str",(PyCFunction)pyelf_elf_str, METH_VARARGS,
		"elf.str(shdr) - Get string from strtab"},
	{NULL, }
};

static PyGetSetDef pyelf_elf_attribs[] = {
	{"machine", (getter)pyelf_elf_machine_get, NULL, "Machine"},
	{"osabi", (getter)pyelf_elf_osabi_get, NULL, "OS/ABI"},
	{"type", (getter)pyelf_elf_type_get, NULL, "File type"},
	{"file", (getter)pyelf_elf_file_get, NULL, "File class"},
	{"data", (getter)pyelf_elf_data_get, NULL, "Data encoding"},
	{"kind", (getter)pyelf_elf_kind_get, NULL, "File format"},
	{"bits", (getter)pyelf_elf_bits_get, NULL, "Data class"},
	{"ehdr", (getter)pyelf_elf_ehdr_get, NULL, "ELF header"},
	{"phdr", (getter)pyelf_elf_phdr_get, NULL, "Program headers"},
	{"shdr", (getter)pyelf_elf_shdr_get, NULL, "Section headers"},
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

#define PYELF_INT_CONST(m, c) PyModule_AddIntConstant(m, #c, c)
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
	if ( PyType_Ready(&pyelf_shdr_pytype) < 0 )
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
	PyModule_AddObject(m, "shdr", (PyObject *)&pyelf_shdr_pytype);

	PYELF_INT_CONST(m, EV_CURRENT);

	PYELF_INT_CONST(m, EI_MAG0);
	PYELF_INT_CONST(m, ELFMAG0);
	PYELF_INT_CONST(m, EI_MAG1);
	PYELF_INT_CONST(m, ELFMAG1);
	PYELF_INT_CONST(m, EI_MAG2);
	PYELF_INT_CONST(m, ELFMAG2);
	PYELF_INT_CONST(m, EI_MAG3);
	PYELF_INT_CONST(m, ELFMAG3);

	PYELF_INT_CONST(m, EI_CLASS);
	PYELF_INT_CONST(m, ELFCLASS32);
	PYELF_INT_CONST(m, ELFCLASS64);

	PYELF_INT_CONST(m, EI_DATA);
	PYELF_INT_CONST(m, ELFDATA2LSB);
	PYELF_INT_CONST(m, ELFDATA2MSB);

	PYELF_INT_CONST(m, EI_VERSION);

	PYELF_INT_CONST(m, EI_OSABI);
	PYELF_INT_CONST(m, ELFOSABI_SYSV);
	PYELF_INT_CONST(m, ELFOSABI_HPUX);
	PYELF_INT_CONST(m, ELFOSABI_NETBSD);
	PYELF_INT_CONST(m, ELFOSABI_LINUX);
	PYELF_INT_CONST(m, ELFOSABI_SOLARIS);
	PYELF_INT_CONST(m, ELFOSABI_AIX);
	PYELF_INT_CONST(m, ELFOSABI_IRIX);
	PYELF_INT_CONST(m, ELFOSABI_FREEBSD);
	PYELF_INT_CONST(m, ELFOSABI_TRU64);
	PYELF_INT_CONST(m, ELFOSABI_MODESTO);
	PYELF_INT_CONST(m, ELFOSABI_OPENBSD);
	PYELF_INT_CONST(m, ELFOSABI_ARM);
	PYELF_INT_CONST(m, ELFOSABI_STANDALONE);

	PYELF_INT_CONST(m, EI_ABIVERSION);
	PYELF_INT_CONST(m, EI_PAD);

	PYELF_INT_CONST(m, ET_REL);
	PYELF_INT_CONST(m, ET_EXEC);
	PYELF_INT_CONST(m, ET_DYN);
	PYELF_INT_CONST(m, ET_CORE);
	PYELF_INT_CONST(m, ET_LOOS);
	PYELF_INT_CONST(m, ET_HIOS);
	PYELF_INT_CONST(m, ET_LOPROC);
	PYELF_INT_CONST(m, ET_HIPROC);

	PYELF_INT_CONST(m, EM_M32);
	PYELF_INT_CONST(m, EM_SPARC);
	PYELF_INT_CONST(m, EM_386);
	PYELF_INT_CONST(m, EM_68K);
	PYELF_INT_CONST(m, EM_88K);
	PYELF_INT_CONST(m, EM_860);
	PYELF_INT_CONST(m, EM_MIPS);
	PYELF_INT_CONST(m, EM_S370);
	PYELF_INT_CONST(m, EM_MIPS_RS3_LE);

	PYELF_INT_CONST(m, EM_PARISC);
	PYELF_INT_CONST(m, EM_VPP500);
	PYELF_INT_CONST(m, EM_SPARC32PLUS);
	PYELF_INT_CONST(m, EM_960);
	PYELF_INT_CONST(m, EM_PPC);
	PYELF_INT_CONST(m, EM_PPC64);
	PYELF_INT_CONST(m, EM_S390);

	PYELF_INT_CONST(m, EM_V800);
	PYELF_INT_CONST(m, EM_FR20);
	PYELF_INT_CONST(m, EM_RH32);
	PYELF_INT_CONST(m, EM_RCE);
	PYELF_INT_CONST(m, EM_ARM);
	PYELF_INT_CONST(m, EM_FAKE_ALPHA);
	PYELF_INT_CONST(m, EM_SH);
	PYELF_INT_CONST(m, EM_SPARCV9);
	PYELF_INT_CONST(m, EM_TRICORE);
	PYELF_INT_CONST(m, EM_ARC);
	PYELF_INT_CONST(m, EM_H8_300);
	PYELF_INT_CONST(m, EM_H8_300H);
	PYELF_INT_CONST(m, EM_H8S);
	PYELF_INT_CONST(m, EM_H8_500);
	PYELF_INT_CONST(m, EM_IA_64);
	PYELF_INT_CONST(m, EM_MIPS_X);
	PYELF_INT_CONST(m, EM_COLDFIRE);
	PYELF_INT_CONST(m, EM_68HC12);
	PYELF_INT_CONST(m, EM_MMA);
	PYELF_INT_CONST(m, EM_PCP);
	PYELF_INT_CONST(m, EM_NCPU);
	PYELF_INT_CONST(m, EM_NDR1);
	PYELF_INT_CONST(m, EM_STARCORE);
	PYELF_INT_CONST(m, EM_ME16);
	PYELF_INT_CONST(m, EM_ST100);
	PYELF_INT_CONST(m, EM_TINYJ);
	PYELF_INT_CONST(m, EM_X86_64);
	PYELF_INT_CONST(m, EM_PDSP);

	PYELF_INT_CONST(m, EM_FX66);
	PYELF_INT_CONST(m, EM_ST9PLUS);
	PYELF_INT_CONST(m, EM_ST7);
	PYELF_INT_CONST(m, EM_68HC16);
	PYELF_INT_CONST(m, EM_68HC11);
	PYELF_INT_CONST(m, EM_68HC08);
	PYELF_INT_CONST(m, EM_68HC05);
	PYELF_INT_CONST(m, EM_SVX);
	PYELF_INT_CONST(m, EM_ST19);
	PYELF_INT_CONST(m, EM_VAX);
	PYELF_INT_CONST(m, EM_CRIS);
	PYELF_INT_CONST(m, EM_JAVELIN);
	PYELF_INT_CONST(m, EM_FIREPATH);
	PYELF_INT_CONST(m, EM_ZSP);
	PYELF_INT_CONST(m, EM_MMIX);
	PYELF_INT_CONST(m, EM_HUANY);
	PYELF_INT_CONST(m, EM_PRISM);
	PYELF_INT_CONST(m, EM_AVR);
	PYELF_INT_CONST(m, EM_FR30);
	PYELF_INT_CONST(m, EM_D10V);
	PYELF_INT_CONST(m, EM_D30V);
	PYELF_INT_CONST(m, EM_V850);
	PYELF_INT_CONST(m, EM_M32R);
	PYELF_INT_CONST(m, EM_MN10300);
	PYELF_INT_CONST(m, EM_MN10200);
	PYELF_INT_CONST(m, EM_PJ);
	PYELF_INT_CONST(m, EM_OPENRISC);
	PYELF_INT_CONST(m, EM_ARC_A5);
	PYELF_INT_CONST(m, EM_XTENSA);
	PYELF_INT_CONST(m, EM_ALPHA);

	PYELF_INT_CONST(m, SHT_NULL);
	PYELF_INT_CONST(m, SHT_PROGBITS);
	PYELF_INT_CONST(m, SHT_SYMTAB);
	PYELF_INT_CONST(m, SHT_STRTAB);
	PYELF_INT_CONST(m, SHT_RELA);
	PYELF_INT_CONST(m, SHT_HASH);
	PYELF_INT_CONST(m, SHT_DYNAMIC);
	PYELF_INT_CONST(m, SHT_NOTE);
	PYELF_INT_CONST(m, SHT_NOBITS);
	PYELF_INT_CONST(m, SHT_REL);
	PYELF_INT_CONST(m, SHT_SHLIB);
	PYELF_INT_CONST(m, SHT_DYNSYM);
	PYELF_INT_CONST(m, SHT_INIT_ARRAY);
	PYELF_INT_CONST(m, SHT_FINI_ARRAY);
	PYELF_INT_CONST(m, SHT_PREINIT_ARRAY);
	PYELF_INT_CONST(m, SHT_GROUP);
	PYELF_INT_CONST(m, SHT_SYMTAB_SHNDX);
	PYELF_INT_CONST(m, SHT_LOOS);
	PYELF_INT_CONST(m, SHT_GNU_ATTRIBUTES);
	PYELF_INT_CONST(m, SHT_GNU_HASH);
	PYELF_INT_CONST(m, SHT_GNU_LIBLIST);
	PYELF_INT_CONST(m, SHT_CHECKSUM);
	PYELF_INT_CONST(m, SHT_LOSUNW);
	PYELF_INT_CONST(m, SHT_SUNW_move);
	PYELF_INT_CONST(m, SHT_SUNW_COMDAT);
	PYELF_INT_CONST(m, SHT_SUNW_syminfo);
	PYELF_INT_CONST(m, SHT_GNU_verdef);
	PYELF_INT_CONST(m, SHT_GNU_verneed);
	PYELF_INT_CONST(m, SHT_GNU_versym);
}
