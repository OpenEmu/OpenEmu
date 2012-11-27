/*
 * Linkscript for Wii
 */

OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc", "elf32-powerpc");
OUTPUT_ARCH(powerpc:common);

PHDRS
{
  plugin PT_LOAD FLAGS(7);
}

SECTIONS
{
	. = 0x81000000;

	/* Program */
	.init          :
	{
		KEEP (*crt0.o(*.init))
		KEEP (*(.init))
	} :plugin = 0
	.plt      : { *(.plt)	}
	.interp			: { *(.interp) 	}
	.hash			: { *(.hash) }
	.dynsym			: { *(.dynsym) }
	.dynstr			: { *(.dynstr) }
	.gnu.version	: { *(.gnu.version) }
	.gnu.version_d	: { *(.gnu.version_d) }
	.gnu.version_r	: { *(.gnu.version_r) }
	.rel.init		: { *(.rel.init) }
	.rela.init		: { *(.rela.init) }
	.rel.text		: { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) }
	.rela.text		: { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }
	.rel.fini		: { *(.rel.fini) }
	.rela.fini		: { *(.rela.fini) }
	.rel.rodata		: { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) }
	.rela.rodata	: { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) }
	.rel.data		: { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) }
	.rela.data		: { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) }
	.rel.tdata		: { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) }
	.rela.tdata		: { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) }
	.rel.tbss		: { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) }
	.rela.tbss		: { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) }
	.rel.ctors		: { *(.rel.ctors) }
	.rela.ctors		: { *(.rela.ctors) }
	.rel.dtors		: { *(.rel.dtors) }
	.rela.dtors		: { *(.rela.dtors) }
	.rel.got		: { *(.rel.got)	}
	.rela.got		: { *(.rela.got) }
	.rela.got1		: { *(.rela.got1) }
	.rela.got2		: { *(.rela.got2) }
	.rel.sdata		: { *(.rel.sdata .rel.sdata.* .rel.gnu.linkonce.s.*) }
	.rela.sdata		: { *(.rela.sdata .rela.sdata.* .rela.gnu.linkonce.s.*) }
	.rel.sbss		: { *(.rel.sbss .rel.sbss.* .rel.gnu.linkonce.sb.*) }
	.rela.sbss		: { *(.rela.sbss .rela.sbss.* .rel.gnu.linkonce.sb.*) }
	.rel.sdata2		: { *(.rel.sdata2 .rel.sdata2.* .rel.gnu.linkonce.s2.*) }
	.rela.sdata2	: { *(.rela.sdata2 .rela.sdata2.* .rela.gnu.linkonce.s2.*) }
	.rel.sbss2		: { *(.rel.sbss2 .rel.sbss2.* .rel.gnu.linkonce.sb2.*) }
	.rela.sbss2		: { *(.rela.sbss2 .rela.sbss2.* .rela.gnu.linkonce.sb2.*) }
	.rel.bss		: { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*) }
	.rela.bss		: { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*) }
	.rel.plt		: { *(.rel.plt) }
	.rela.plt		: { *(.rela.plt) }

	.text      :
	{
		*(.text)
		*(.text.*)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.gnu.linkonce.t.*)
		. = ALIGN(32);   /* REQUIRED. LD is flaky without it. */
	} = 0

	.fini      :
	{
		KEEP (*(.fini))
		. = ALIGN(32);   /* REQUIRED. LD is flaky without it. */
	} = 0
	
	PROVIDE (__etext = .);
	PROVIDE (_etext = .);
	PROVIDE (etext = .);

	.rodata   : { *(.rodata) *(.rodata.*) *(.gnu.linkonce.r.*) }
	.rodata1   : { *(.rodata1) }
	.sdata2   : { *(.sdata2) *(.sdata2.*) *(.gnu.linkonce.s2.*) }
	.sbss2   : { *(.sbss2) *(.sbss2.*) *(.gnu.linkonce.sb2.*) }
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  /* Ensure the __preinit_array_start label is properly aligned.  We
     could instead move the label definition inside the section, but
     the linker would then create the section even if it turns out to
     be empty, which isn't pretty.  */
	. = ALIGN(32 / 8);
	PROVIDE (__preinit_array_start = .);
	.preinit_array     : { *(.preinit_array) }
	PROVIDE (__preinit_array_end = .);
	PROVIDE (__init_array_start = .);
	.init_array     : { *(.init_array) }
	PROVIDE (__init_array_end = .);
	PROVIDE (__fini_array_start = .);
	.fini_array     : { *(.fini_array) }
	PROVIDE (__fini_array_end = .);
	.data    :
	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d.*)
		SORT(CONSTRUCTORS)
		. = ALIGN(32);   /* REQUIRED. LD is flaky without it. */
	}

	.data1   : { *(.data1) }
	.tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
	.tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
	.eh_frame : { KEEP (*(.eh_frame)) }
	.gcc_except_table : { *(.gcc_except_table) }
	.fixup          : { *(.fixup) }
	.got1           : { *(.got1) }
	.got2           : { *(.got2) }
	.dynamic       : { *(.dynamic) }

	.ctors   :
	{
		___plugin_ctors = .;
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		___plugin_ctors_end = .;
		. = ALIGN(32);   /* REQUIRED. LD is flaky without it. */
	}

	.dtors         :
	{
		___plugin_dtors = .;
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		___plugin_dtors_end = .;
		. = ALIGN(32);   /* REQUIRED. LD is flaky without it. */
	}

	.jcr            : { KEEP (*(.jcr)) }
	.got		  : { *(.got.plt) *(.got) }


	/*	We want the small data sections together, so single-instruction offsets
		can access them all, and initialized data all before uninitialized, so
		we can shorten the on-disk segment size.  */

	.sdata     :
	{
		*(.sdata)
		*(.sdata.*)
		*(.gnu.linkonce.s.*)
		. = ALIGN(32);   /* REQUIRED. LD is flaky without it. */
	}

	_edata = .;
	PROVIDE (edata = .);
	
	.sbss      :
	{
		__sbss_start = .;
		PROVIDE (__sbss_start = .);
		PROVIDE (___sbss_start = .);
		*(.dynsbss)
		*(.sbss)
		*(.sbss.*)
		*(.gnu.linkonce.sb.*)
		*(.scommon)
		PROVIDE (__sbss_end = .);
		PROVIDE (___sbss_end = .);
		. = ALIGN(32);   /* REQUIRED. LD is flaky without it. */
		__sbss_end = .;
	}

	.bss       :
	{
		__bss_start = .;
		PROVIDE (__bss_start = .);
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
	/*	Align here to ensure that the .bss section occupies space up to
		_end.  Align after .bss to ensure correct alignment even if the
		.bss section disappears because there are no input sections.  */

		. = ALIGN(32);

		PROVIDE (__bss_end = .);
		__bss_end = .;
	}

	_end = .;
	PROVIDE(end = .);
	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/*	DWARF debug sections.
		Symbols in the DWARF debugging sections are relative to the beginning
		of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	/* These must appear regardless of  .  */
}

__isIPL = 0;
__stack_addr = (__bss_start + SIZEOF(.bss) + 0x20000 + 7) & (-8);
__stack_end = (__bss_start + SIZEOF(.bss));
__intrstack_addr = (__stack_addr + 0x4000);
__intrstack_end = (__stack_addr);
__Arena1Lo = (__intrstack_addr + 31) & (-32);
__Arena1Hi = (0x816ffff0);
__Arena2Lo = (0x90002000);
__Arena2Hi = (0x933E0000);

__gxregs = (__Arena1Hi + 31) & (-32);
__ipcbufferLo = (0x933e0000);
__ipcbufferHi = (0x93400000);

/* for backward compatibility with old crt0 */
PROVIDE (__stack = (0x816ffff0));

PROVIDE(__isIPL = __isIPL);
PROVIDE(__stack_addr = __stack_addr);
PROVIDE(__stack_end = __stack_end);
PROVIDE(__intrstack_addr = __intrstack_addr);
PROVIDE(__intrstack_end = __intrstack_end);
PROVIDE(__Arena1Lo = __Arena1Lo);
PROVIDE(__Arena1Hi = __Arena1Hi);
PROVIDE(__Arena2Lo = __Arena2Lo);
PROVIDE(__Arena2Hi = __Arena2Hi);
PROVIDE(__ipcbufferLo = __ipcbufferLo);
PROVIDE(__ipcbufferHi = __ipcbufferHi);
PROVIDE(__gxregs = __gxregs);
