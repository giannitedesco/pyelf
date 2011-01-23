from pyelf_idl import Struct, FixedBuffer, ULong, CFiles

ehdr = Struct('GElf_Ehdr', 'ehdr', 'pyelf', [
		FixedBuffer('e_ident', 16),
		ULong('e_type'),
		ULong('e_machine'),
		ULong('e_version'),
		ULong('e_entry'),
		ULong('e_phoff'),
		ULong('e_shoff'),
		ULong('e_flags'),
		ULong('e_ehsize'),
		ULong('e_phentsize'),
		ULong('e_phnum'),
		ULong('e_shentsize'),
		ULong('e_shnum'),
		ULong('e_shstrndx'),
	])

c = CFiles('pyelf_ehdr', [ehdr])
c.include('py_elf.h', system = False)
c.write()
