#!/usr/bin/python

import elf

def do_file(f):
	print 'Opening %s'%f.name
	e = elf.elf(f)

	ehdr = e.ehdr
	print 'ELF Header:'
	print '  Magic: %s'%(' '.join(map(lambda x:'%.2x'%x, ehdr.e_ident)))
	print '  Class:                        ELF%d'%e.bits
	print '  Data:                         %s'%e.data
	print '  Version:                      %d'%ehdr.e_ident[elf.EI_VERSION]
	print '  OS/ABI:                       %s'%e.osabi
	print '  ABI Version:                  %d'%ehdr.e_ident[elf.EI_ABIVERSION]
	print '  Type:                         %s'%e.type
	print '  Machine:                      %s'%e.machine

	print
	print 'Section Headers:'
	print '  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al'
	i = 0
	for s in e.shdr:
		name = e.str(s)
		if name is None:
			name = ''
		t = 'NULL'
		print '  [%2d] %s %s %08x %06x %06x ,,,'%(i,
							name.ljust(17),
							t.ljust(15),
							s.sh_addr,
							s.sh_offset,
							s.sh_size)
		i += 1

	print """Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings)
  I (info), L (link order), G (group), x (unknown)
  O (extra OS processing required) o (OS specific), p (processor specific)
"""
	print 'Program Headers: %d'%ehdr.e_phnum
	print '  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align'
	for p in e.phdr:
		print '  ... %r'%p
	print

def main(argv):
	print 'Invoked as %s'%argv[0]
	for fn in argv[1:]:
		try:
			do_file(file(fn, 'r'))
		except elf.Error, e:
			print ' *** ELF %s: %s'%(e.__class__.__name__, ':'.join(e.args))

if __name__ == '__main__':
	from sys import argv
	main(argv)
