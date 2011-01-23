from distutils.core import setup, Extension

incs = ['.']

elf = Extension('elf', ['py_elf.c'],
		libraries=['elf'],
		include_dirs = incs)

exts = [elf]
setup(name = 'elf', version='0.1', ext_modules = exts)
