OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
OUTPUT_ARCH(sh)
PHDRS
{
   plugin PT_LOAD ;
}
SECTIONS
{
  . = 0;
  .text           :
  {
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(.gnu.warning)
  } :plugin =0
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .rodata1        : { *(.rodata1) }
  .sdata2         : { *(.sdata2 .sdata2.* .gnu.linkonce.s2.*) }
  .sbss2          : { *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*) }
  .data           :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }
  .tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .eh_frame       : { KEEP (*(.eh_frame)) }
  .gcc_except_table   : { *(.gcc_except_table) }
  .ctors          :
  {
    ___plugin_ctors = .;
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    ___plugin_ctors_end = .;
  }
  .dtors          :
  {
    ___plugin_dtors = .;
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    ___plugin_dtors_end = .;
  }
  .sdata          :
  {
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }
  .sbss           :
  {
    *(.dynsbss)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
  }
  .bss            :
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
  }
}
