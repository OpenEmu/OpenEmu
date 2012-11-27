/* PHDRS specifies ELF Program Headers (or segments) to the plugin linker */
PHDRS {
  plugin PT_LOAD ; /* Specifies that the plugin segment should be loaded from file */
  shorts PT_LOAD ; /* Specifies that the shorts segment should be loaded from file */
}

SECTIONS {
	.text 0: {
		_ftext = . ;
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t*)
		KEEP(*(.init))
		KEEP(*(.fini))
		QUAD(0)
	} : plugin /*The ": plugin" tells the linker to assign this and
  	             the following sections to the "plugin" segment*/
	PROVIDE(_etext = .);
	PROVIDE(etext = .);

	.reginfo : { *(.reginfo) }

	/* Global/static constructors and deconstructors. */
	.ctors ALIGN(16): {
	        ___plugin_ctors = .;
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		___plugin_ctors_end = .;
	}
	.dtors ALIGN(16): {
		___plugin_dtors = .;
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		___plugin_dtors_end = .;
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

	.bss ALIGN(128) : {
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b*)
		*(COMMON)
	}
	_end_bss = .;

	_end = . ;
	PROVIDE(end = .);

	/* Symbols needed by crt0.s.  */
	PROVIDE(_heap_size = -1);
	PROVIDE(_stack = -1);
	PROVIDE(_stack_size = 128 * 1024);	

	/*We assign the output location counter to the plugin hole made
   	  in main_prog.ld, then assign the small data sections to the shorts segment*/
	. = __plugin_hole_start;
	.lit4 ALIGN(128): { *(.lit4) } : shorts
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
}
