                                    MUSASHI
                                    =======

                                  Version 3.3

             A portable Motorola M680x0 processor emulation engine.
            Copyright 1998-2001 Karl Stenerud.  All rights reserved.



INTRODUCTION:
------------

Musashi is a Motorola 68000, 68010, 68EC020, and 68020 emulator written in C.
This emulator was written with two goals in mind: portability and speed.

The emulator is written to ANSI C specifications with the exception that I use
inline functions.  This is not compliant to the ANSI spec, but will be
compliant to the ANSI C9X spec.

It has been successfully running in the MAME project (www.mame.net) for over 2
years and so has had time to mature.



LICENSE AND COPYRIGHT:
---------------------

The Musashi M680x0 emulator is copyright 1998-2001 Karl Stenerud.

The source code included in this archive is provided AS-IS, free for any
non-commercial purpose.

If you build a program using this core, please give credit to the author.

If you wish to use this core in a commercial environment, please contact
the author to discuss commercial licensing.



AVAILABILITY:
------------
The latest version of this code can be obtained at:
http://kstenerud.cjb.net



CONTACTING THE AUTHOR:
---------------------
I can be reached at kstenerud@mame.net



BASIC CONFIGURATION:
-------------------
The basic configuration will give you a standard 68000 that has sufficient
functionality to work in a primitive environment.

This setup assumes that you only have 1 device interrupting it, that the
device will always request an autovectored interrupt, and it will always clear
the interrupt before the interrupt service routine finishes (but could
possibly re-assert the interrupt).
You will have only one address space, no tracing, and no instruction prefetch.

To implement the basic configuration:

- Open m68kconf.h and verify that the settings for INLINE and DECL_SPEC will
  work with your compiler. (They are set for gcc)

- In your host program, implement the following functions:
    unsigned int  m68k_read_memory_8(unsigned int address);
    unsigned int  m68k_read_memory_16(unsigned int address);
    unsigned int  m68k_read_memory_32(unsigned int address);
    void m68k_write_memory_8(unsigned int address, unsigned int value);
    void m68k_write_memory_16(unsigned int address, unsigned int value);
    void m68k_write_memory_32(unsigned int address, unsigned int value);

- In your host program, be sure to call m68k_pulse_reset() once before calling
  any of the other functions as this initializes the core.

- Use m68k_execute() to execute instructions and m68k_set_irq() to cause an
  interrupt.



ADDING PROPER INTERRUPT HANDLING:
--------------------------------
The interrupt handling in the basic configuration doesn't emulate the
interrupt acknowledge phase of the CPU and automatically clears an interrupt
request during interrupt processing.
While this works for most systems, you may need more accurate interrupt
handling.

To add proper interrupt handling:

- In m68kconf.h, set M68K_EMULATE_INT_ACK to OPT_SPECIFY_HANDLER

- In m68kconf.h, set M68K_INT_ACK_CALLBACK(A) to your interrupt acknowledge
  routine

- Your interrupt acknowledge routine must return an interrupt vector,
  M68K_INT_ACK_AUTOVECTOR, or M68K_INT_ACK_SPURIOUS.  most m68k
  implementations just use autovectored interrupts.

- When the interrupting device is satisfied, you must call m68k_set_irq(0) to
  remove the interrupt request.



MULTIPLE INTERRUPTS:
-------------------
The above system will work if you have only one device interrupting the CPU,
but if you have more than one device, you must do a bit more.

To add multiple interrupts:

- You must make an interrupt arbitration device that will take the highest
  priority interrupt and encode it onto the IRQ pins on the CPU.

- The interrupt arbitration device should use m68k_set_irq() to set the
  highest pending interrupt, or 0 for no interrupts pending.



SEPARATE IMMEDIATE AND PC-RELATIVE READS:
----------------------------------------
You can write faster memory access functions if you know whether you are
fetching from ROM or RAM.  Immediate reads are always from the program space
(Always in ROM unless it is running self-modifying code).
This will also separate the pc-relative reads, since some systems treat
PROGRAM mode reads and DATA mode reads differently (for program encryption,
for instance).  See the section below (ADDRESS SPACE) for an explanation of
PROGRAM and DATA mode.

To enable separate reads:

- In m68kconf.h, turn on M68K_SEPARATE_READS.

- In your host program, implement the following functions:
    unsigned int  m68k_read_immediate_16(unsigned int address);
    unsigned int  m68k_read_immediate_32(unsigned int address);

    unsigned int  m68k_read_pcrelative_8(unsigned int address);
    unsigned int  m68k_read_pcrelative_16(unsigned int address);
    unsigned int  m68k_read_pcrelative_32(unsigned int address);

- If you need to know the current PC (for banking and such), set
  M68K_MONITOR_PC to OPT_SPECIFY_HANDLER, and set M68K_SET_PC_CALLBACK(A) to
  your routine.



ADDRESS SPACES:
--------------
Most systems will only implement one address space, placing ROM at the lower
addresses and RAM at the higher.  However, there is the possibility that a
system will implement ROM and RAM in the same address range, but in different
address spaces, or will have different mamory types that require different
handling for the program and the data.

The 68k accomodates this by allowing different program spaces, the most
important to us being PROGRAM and DATA space.  Here is a breakdown of
how information is fetched:

- All immediate reads are fetched from PROGRAM space.

- All PC-relative reads are fetched from PROGRAM space.

- The initial stack pointer and program counter are fetched from PROGRAM space.

- All other reads (except for those from the moves instruction for 68020)
   are fetched from DATA space.

The m68k deals with this by encoding the requested address space on the
function code pins:

                       FC
    Address Space      210
    ------------------ ---
    USER DATA          001
    USER PROGRAM       010
    SUPERVISOR DATA    101
    SUPERVISOR PROGRAM 110
    CPU SPACE          111 <-- not emulated in this core since we emulate
                               interrupt acknowledge in another way.

Problems arise here if you need to emulate this distinction (if, for example,
your ROM and RAM are at the same address range, with RAM and ROM enable
wired to the function code pins).

There are 2 ways to deal with this situation using Musashi:

1. If you only need the distinction between PROGRAM and DATA (the most common),
   you can just separate the reads (see the preceeding section).  This is the
   faster solution.

2. You can emulate the function code pins entirely.

To emulate the function code pins:

- In m68kconf.h, set M68K_EMULATE_FC to OPT_SPECIFY_HANDLER and set
  M68K_SET_FC_CALLBACK(A) to your function code handler function.

- Your function code handler should select the proper address space for
  subsequent calls to m68k_read_xx (and m68k_write_xx for 68010+).

Note: immediate reads are always done from program space, so technically you
      don't need to implement the separate immediate reads, although you could
      gain more speed improvements leaving them in and doing some clever
      programming.



USING DIFFERENT CPU TYPES:
-------------------------
The default is to enable only the 68000 cpu type.  To change this, change the
settings for M68K_EMULATE_010 etc in m68kconf.h.

To set the CPU type you want to use:

- Make sure it is enabled in m68kconf.h.  Current switches are:
    M68K_EMULATE_010
    M68K_EMULATE_EC020
    M68K_EMULATE_020

- In your host program, call m68k_set_cpu_type() and then call
  m68k_pulse_reset().  Valid CPU types are:
    M68K_CPU_TYPE_68000,
    M68K_CPU_TYPE_68010,
    M68K_CPU_TYPE_68EC020,
    M68K_CPU_TYPE_68020



CLOCK FREQUENCY:
---------------
In order to emulate the correct clock frequency, you will have to calculate
how long it takes the emulation to execute a certain number of "cycles" and
vary your calls to m68k_execute() accordingly.
As well, it is a good idea to take away the CPU's timeslice when it writes to
a memory-mapped port in order to give the device it wrote to a chance to
react.

You can use the functions m68k_cycles_run(), m68k_cycles_remaining(),
m68k_modify_timeslice(), and m68k_end_timeslice() to do this.
Try to use large cycle values in your calls to m68k_execute() since it will
increase throughput.  You can always take away the timeslice later.



MORE CORRECT EMULATION:
----------------------
You may need to enable these in order to properly emulate some of the more
obscure functions of the m68k:

- M68K_EMULATE_BKPT_ACK causes the CPU to call a breakpoint handler on a BKPT
  instruction

- M68K_EMULATE_TRACE causes the CPU to generate trace exceptions when the
  trace bits are set

- M68K_EMULATE_RESET causes the CPU to call a reset handler on a RESET
  instruction.

- M68K_EMULATE_PREFETCH emulates the 4-word instruction prefetch that is part
  of the 68000/68010 (needed for Amiga emulation).

- call m68k_pulse_halt() to emulate the HALT pin.



CONVENIENCE FUNCTIONS:
---------------------
These are in here for programmer convenience:

- M68K_INSTRUCTION_HOOK lets you call a handler before each instruction.

- M68K_LOG_ENABLE and M68K_LOG_1010_1111 lets you log illegal and A/F-line
  instructions.



MULTIPLE CPU EMULATION:
----------------------
The default is to use only one CPU.  To use more than one CPU in this core,
there are some things to keep in mind:

- To have different cpus call different functions, use OPT_ON instead of
  OPT_SPECIFY_HANDLER, and use the m68k_set_xxx_callback() functions to set
  your callback handlers on a per-cpu basis.

- Be sure to call set_cpu_type() for each CPU you use.

- Use m68k_set_context() and m68k_get_context() to switch to another CPU.



LOAD AND SAVE CPU CONTEXTS FROM DISK:
------------------------------------
You can use them68k_load_context() and m68k_save_context() functions to load
and save the CPU state to disk.



GET/SET INFORMATION FROM THE CPU:
--------------------------------
You can use m68k_get_reg() and m68k_set_reg() to gain access to the internals
of the CPU.



EXAMPLE:
-------

I have included a file example.zip that contains a full example.
