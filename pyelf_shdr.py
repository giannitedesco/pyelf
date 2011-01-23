from pyelf_idl import Struct, FixedBuffer, ULong, Synthetic, CFiles

shdr = Struct('GElf_Shdr', 'shdr', 'pyelf', [
		ULong('sh_name'),
		ULong('sh_type'),
		ULong('sh_flags'),
		ULong('sh_addr'),
		ULong('sh_offset'),
		ULong('sh_size'),
		ULong('sh_link'),
		ULong('sh_info'),
		ULong('sh_addralign'),
		ULong('sh_entsize'),
		Synthetic('type'),
	])

c = CFiles('pyelf_shdr', [shdr])
c.include('py_elf.h', system = False)
c.write()
