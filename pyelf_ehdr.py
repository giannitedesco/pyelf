from pyelf_idl import Struct, FixedBuffer, Long, CFiles

ehdr = Struct('GElf_Ehdr', 'ehdr', 'pyelf', [
		FixedBuffer('e_ident', 16),
		Long('e_type'),
		Long('e_machine'),
		Long('e_version'),
		Long('e_entry'),
		Long('e_phoff'),
		Long('e_shoff'),
		Long('e_flags'),
		Long('e_ehsize'),
		Long('e_phentsize'),
		Long('e_phnum'),
		Long('e_shentsize'),
		Long('e_shnum'),
		Long('e_shstrndx'),
	])

c = CFiles('pyelf_ehdr', [ehdr])
c.include('py_elf.h', system = False)
c.write()
