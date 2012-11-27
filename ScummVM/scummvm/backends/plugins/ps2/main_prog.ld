ENTRY(_start);

SECTIONS {
	.text 0x00100000: {
		_ftext = . ;
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t*)
		KEEP(*(.init))
		KEEP(*(.fini))
		QUAD(0)
	}

	PROVIDE(_etext = .);
	PROVIDE(etext = .);

	.reginfo : { *(.reginfo) }

	/* Global/static constructors and deconstructors. */
	.ctors ALIGN(16): {
		KEEP(*crtbegin*.o(.ctors))
		KEEP(*(EXCLUDE_FILE(*crtend*.o) .ctors))
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
	}
	.dtors ALIGN(16): {
		KEEP(*crtbegin*.o(.dtors))
		KEEP(*(EXCLUDE_FILE(*crtend*.o) .dtors))
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
	}

	/* Static data.  */
	.rodata ALIGN(128): {
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
	}

	.data ALIGN(128): {
		_fdata = . ;
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		SORT(CONSTRUCTORS)
	}

	.rdata ALIGN(128): { *(.rdata) }
	.gcc_except_table ALIGN(128): { *(.gcc_except_table) }

	_gp = ALIGN(128) + 0x7ff0;
	.lit4 ALIGN(128): { *(.lit4) }
	.lit8 ALIGN(128): { *(.lit8) }

	.sdata ALIGN(128): {
		*(.sdata)
		*(.sdata.*)
		*(.gnu.linkonce.s*)
	}

	_edata = .;
	PROVIDE(edata = .);

	/* Uninitialized data.  */
	.sbss ALIGN(128) : {
		_fbss = . ;
		*(.sbss)
		*(.sbss.*)
		*(.gnu.linkonce.sb*)
		*(.scommon)
	}

	/*This "plugin hole" is so the plugins can all have global small data
	  in the same place.*/
	__plugin_hole_start = .;
	. = _gp + 0x7ff0;
	__plugin_hole_end = .;

	COMMON		  :
	{
	 *(COMMON)
	}
	. = ALIGN(128);

	.bss ALIGN(128) : {
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b*)
	}
	_end_bss = .;

	_end = . ;
	PROVIDE(end = .);

	/* Symbols needed by crt0.s.  */
	PROVIDE(_heap_size = -1);
	PROVIDE(_stack = -1);
	PROVIDE(_stack_size = 128 * 1024);
}
