from pyelf_idl import Struct, FixedBuffer, ULong, CFiles

phdr = Struct('GElf_Phdr', 'phdr', 'pyelf', [
		ULong('p_type'),
		ULong('p_flags'),
		ULong('p_offset'),
		ULong('p_vaddr'),
		ULong('p_paddr'),
		ULong('p_filesz'),
		ULong('p_memsz'),
		ULong('p_align'),
	])

c = CFiles('pyelf_phdr', [phdr])
c.include('py_elf.h', system = False)
c.write()
