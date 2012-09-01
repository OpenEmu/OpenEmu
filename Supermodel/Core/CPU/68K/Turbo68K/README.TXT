

        ######  ##  ## ######  ######   #####    ####   #####  ###  ##
        # ## #  ##  ##  ##  ##  ## ### ##   ##  ##     ##   ##  ##  ## 
          ##    ##  ##  ##  ##  ##  ## ##   ## ##      ##   ##  ## ##  
          ##    ##  ##  #####   #####  ##   ## ######   #####   ####   
          ##    ##  ##  ####    ##  ## ##   ## ##   ## ##   ##  ####   
          ##    ##  ##  ## ##   ## ### ##   ## ##   ## ##   ##  ## ##  
         ####    ####  ###  ## ######   #####   #####   #####  ###  ## 

                             [ Turbo68K Readme ]
                   Copyright 2000-2002 Bart Trzynadlowski


This is the documentation for Turbo68K Version 0.6, please read it. Terms of
use are explained a little further below, it is required that you read and
abide by them in order to use this software.
    
Turbo68K is a Motorola 680X0 emulator. It currently emulates the 68000 and
68010 microprocessors. It was written for Intel X86-based (Pentium or better)
systems. Originally designed for Genital it is now available for public use.

Contact Bart Trzynadlowski:
    Email: trzy@mailandnews.com
    WWW:   http://trzy.overclocked.org
           http://trzy.overclocked.org/turbo68k


-------------------
  0. Terms of Use
-------------------

Turbo68K is Copyright 2000, 2001, 2002 Bart Trzynadlowski.

"Turbo68K" refers to: MAKE68K.C, TURBO68K.H, README.TXT, any binaries compiled
from those files, any source code emitted from MAKE68K.C, and any object file
created from such code.

To use Turbo68K, you must agree to the following:

    - Turbo68K shall be distributed freely, unmodified, and intact.
    - Charging money, goods, or services for Turbo68K is forbidden.
    - Claiming ownership or authorship of Turbo68K is forbidden.
    - Turbo68K may not be distributed in any medium for which money, goods, or
      services are solicited.
    - The author, Bart Trzynadlowski, will not be held liable for damages.
    - Credit must be given to the author either in the program itself (where
      the user can see it) or in its documentation.
    
You are encouraged to contact Bart Trzynadlowski if you wish to use Turbo68K
in a commercial product (to negotiate licensing.)

Bart Trzynadlowski reserves the right to use Turbo68K in any way he sees fit.
If you do not agree with all of these terms, please remove Turbo68K from your
possession.


------------------------
  1. Table of Contents
------------------------

    0. Terms of Use
    1. Table of Contents
    2. Making Turbo68K
    3. Command Line Options
    4. Using Turbo68K
        4.1  Basic Data Types
        4.2  Definitions
        4.3  Context
        4.4  Instruction Fetching
        4.5  Data Access
        4.6  PC-Relative Reads
        4.7  Interrupts
        4.8  Byte Swapped Memory
        4.9  The RESET and BKPT Instructions
        4.10 Initializing and Resetting Turbo68K
        4.11 Running the Emulator
    5. Function Reference
    6. Hints and Tips
        6.1  Alternative Identifiers
        6.2  Multiple Processors
        6.3  Maximizing Speed and Minimizing Size
        6.4  Sega Genesis TAS Bug
    7. Additional Notes
        7.1  Change History
        7.2  Un-emulated Features
        7.3  Errata
        7.4  Bibliography
    8. Special Thanks


----------------------
  2. Making Turbo68K
----------------------

The Turbo68K 680X0 emulator is usually distributed in a compressed archive,
simply extract it and you should have MAKE68K.C, TURBO68K.H, and this
README.TXT file.

The first step is to get the source code emitter, Make68K, up and running. It
should compile with any 32-bit C compiler (compilers other than gcc will often
produce warnings, these can be ignored.) The following have been confirmed:

    - DJGPP (gcc v2.8.1)                            [ MS-DOS ]
    - gcc v2.7.2.1                                  [ FreeBSD ]
    - Microsoft VisualC++ 6.0 Professional Edition  [ Windows 95 ]
    - WATCOM C/C++32 Version 10.6                   [ MS-DOS ]

If you get link errors concerning pow(), make sure to include the appropriate
math library. Try "-lm" if you are using GNU tools.

Once Make68K has been built, run it with the name of the file you wish to emit
on the command line. For example:

    make68k turbo68k.asm

Make68K can take a number of options. Try "make68k -h" to see a list. Section
3 of this text describes them in more detail.

Assemble the emitted file with NASM. Version 0.97 or better should work, 0.98
is known to work.

Turbo68K contains identifiers with leading and trailing underscores as
well as without any, so all the object formats supported by NASM should work.

Remember to include TURBO68K.H to use Turbo68K.


---------------------------
  3. Command Line Options
---------------------------

Turbo68K can be tailored in a variety of ways to suit your needs through the
use of the following command line options:

    -mpu <type>     Processor type. Valid types are 68000 and 68010.
                    [Default=68000]
    -addr <bits>    Address bus width. This simulates the specified number of
                    address lines by cutting off any unsupported address bits
                    when accessing memory or updating the PC. The 68000 and
                    68010 have a 24-bit address bus. [Default=24]
    -illegal        Emulates illegal instruction traps. Line 1010 and Line
                    1111 emulator exceptions are processed as is the normal
                    illegal instruction exception. [Default]
    -noillegal      Does not emulate illegal instruction traps but instead
                    returns an error from Turbo68KRun(). The ILLEGAL opcode
                    still generates an illegal instruction trap regardless of
                    whether -illegal or -noillegal has been specified.
    -dummyread      Emulates dummy reads for the CLR, Scc, and unprivileged
                    "MOVE from SR" instructions. These instructions perform a
                    read (and do not make use of the data) before writing.
                    Some systems may require this for accurate emulation (for
                    example: some systems will lock up when write-only
                    locations are read.) This is only available if the
                    processor type is 68000. [Default]
    -nodummyread    Does not emulate dummy reads and instead performs a write
                    immediately.
    -skip           Skips over idle DBRA loops by setting the counter register
                    to 0xFFFF, adjusting the timer appropriately, and moving
                    to the next instruction.
    -noskip         Does not skip over idle loops. This eliminates problems
                    with compatibility and is recommended. [Default]
    -brafetch       Uses the fetch memory map array to update the PC on Bcc,
                    BRA, BSR, and DBcc instructions.
    -nobrafetch     Bcc, BRA, BSR, and DBcc branches are handled by adding the
                    displacement to the internal normalized PC. This is less
                    safe, in theory, but you should not normally have any
                    problems. It is also faster and generates less code than
                    -brafetch. [Default]
    -pcfetch        PC-relative reads are handled through a special fetch map.
                    Please read section 4.6 for more information.
    -nopcfetch      PC-relative reads are treated as normal data reads.
                    [Default]
    -multiaddr      Separate address spaces for supervisor and user. This has
                    not been extensively tested, so let me know how it works.
                    [Default]
    -singleaddr     Single address space for supervisor and user.
    -defmap         Definable memory map arrays are used to handle reading and
                    writing to the address space. [Default]
    -handler        Uses handlers for reading/writing bytes, words, and long-
                    words. The fetch memory map array must still be set up.
    -stackcall      Use stack-based calling conventions. [Default]
    -regcall        Use register-based calling conventions. EAX contains the
                    first argument, the second is in EDX.
    -id <string>    Adds the specified string to the beginning of all
                    identifiers. Up to 16 characters are allowed. See section
                    6.1 for more information on this.

Please read further to learn more about the details of some of these options.


---------------------
  4. Using Turbo68K
---------------------

A general overview of Turbo68K is provided here. More information on certain
topics is provided throughout the document.


4.1 Basic Data Types

The following data types are provided by Turbo68K:

    TURBO68K_UINT32             Unsigned 32-bit
    TURBO68K_INT32              Signed 32-bit
    TURBO68K_UINT16             Unsigned 16-bit
    TURBO68K_INT16              Signed 16-bit
    TURBO68K_UINT8              Unsigned 8-bit
    TURBO68K_INT8               Signed 8-bit


4.2 Definitions

Turbo68K functions return and accept defined arguments. Use these instead of
hard-coded values. Different functions return and accept different values, so
make sure to check with the function reference first.

    TURBO68K_NULL               0
    TURBO68K_OKAY               Success
    TURBO68K_SUPERVISOR         Supervisor address space
    TURBO68K_USER               User address space
    TURBO68K_ERROR_FETCH        Fetch error (PC out of bounds)
    TURBO68K_ERROR_INVINST      Invalid instruction
    TURBO68K_ERROR_INTLEVEL     Invalid interrupt level was specified
    TURBO68K_ERROR_INTVECTOR    Invalid interrupt vector was specified
    TURBO68K_ERROR_INTPENDING   Specified interrupt level already pending
    TURBO68K_UNINITIALIZED      Uninitialized interrupt vector
    TURBO68K_SPURIOUS           Spurious interrupt vector
    TURBO68K_AUTOVECTOR         Autovectored interrupt


4.3 Context

Processor contexts are special structures which contain all the data necessary 
to manage the emulated 680X0. The procedure for emulating a 680X0 processor
with Turbo68K involves setting up a context, mapping it in, emulating some
code, and perhaps mapping the context out.

    Context Definitions:
    --------------------

    struct TURBO68K_CONTEXT_68000
    {
        void            *fetch;
        void            *read_byte, *read_word, *read_long;
        void            *write_byte, *write_word, *write_long;
        void            *super_fetch;
        void            *super_read_byte, *super_read_word, *super_read_long;
        void            *super_write_byte, *super_write_word,
                        *super_write_long;
        void            *user_fetch;
        void            *user_read_byte, *user_read_word, *user_read_long;
        void            *user_write_byte, *user_write_word, *user_write_long;
        TURBO68K_UINT32 intr[8], cycles, remaining;
        TURBO68K_UINT32 d[8], a[8], sp, sr, pc, status;
        void            *InterruptAcknowledge;
        void            *Reset;
    };

    struct TURBO68K_CONTEXT_68010
    {
        void            *fetch, *pcfetch;
        void            *read_byte, *read_word, *read_long;
        void            *write_byte, *write_word, *write_long;
        void            *super_fetch, *super_pcfetch;
        void            *super_read_byte, *super_read_word, *super_read_long;
        void            *super_write_byte, *super_write_word,
                        *super_write_long;
        void            *user_fetch, *user_pcfetch;
        void            *user_read_byte, *user_read_word, *user_read_long;
        void            *user_write_byte, *user_write_word, *user_write_long;
        TURBO68K_UINT32 intr[8], cycles, remaining;
        TURBO68K_UINT32 d[8], a[8], sp, sr, pc, fc, vbr, status;
        void            *InterruptAcknowledge;
        void            *Reset, *Bkpt;
    };

If you are emulating different address spaces for supervisor and user modes,
the super_ and user_ pointers must be set to point at the respective memory
maps. The other memory map pointers (fetch, read_byte, write_byte, etc.) will
be managed internally by Turbo68K.

If you are only emulating a single address space for both privilege modes,
you do not have to set up the super_ and user_ pointers but must set up the
others.

The "intr" array holds pending interrupt information. Entries 0-6 are for
interrupt levels 1-7, they contain the vector of the pending interrupt, or 0
if no interrupt at the given level is pending. Entry 7 contains the number of
pending interrupts. You should not modify this yourself, use the interrupt
functions.

The "cycles" and "remaining" members are used to control execution time.
The total amount of request cycles is held in "cycles" and the number left is
held in "remaining."

Data and address registers are kept in the "d" and "a" array, the PC is kept
in "pc", and the SR is kept in "sr" (the low word is used, the upper word
should be kept 0.)

Whichever stack pointer is not in A7 is stored in "sp." Thus, if the processor
is in Supervisor mode, A7 is the SSP and "sp" contains the USP. Turbo68K will
only swap these when it has to switch privilege modes internally, such as with
a MOVE SR instruction, exception processing, etc.

NOTE: If you absolutely must modify the SR outside of Turbo68K (for example,
if you are writing a debugger), you have to swap A7 and "sp" manually if the
privilege mode changed. 

For the 68010, "fc" is both SFC (first 3 bits of byte 0) and DFC (first 3 bits
of byte 1.) The 68010's VBR is kept in "vbr."

Additional information on the processor status is reflected in the "status"
member. Its format is:
    
    Bit 0:  R (Running) flag -- 0 = not running, 1 = running
    Bit 1:  S (Stopped) flag -- 0 = not stopped, 1 = stopped via the STOP
            instruction
    Bit 2:  I (Interrupt) flag -- 0 = not processing interrupts, 1 = process-
            ing interrupts (Turbo68KProcessInterrupts())

The "InterruptAcknowledge" pointer points to an interrupt acknowledge handler
which is called when interrupts are being processed. If the pointer is set to
TURBO68K_NULL, it will not be called. See section 4.7.

Finally, the "Reset" pointer points to a Reset handler. If the RESET
instruction is executed, this handler will be called. If the pointer is set to
TURBO68K_NULL, it will not be called. The "Bkpt" pointer is for the 68010's
BKPT instruction.

Contexts can be mapped in and out of Turbo68K's internal data space by using
Turbo68KSetContext() and Turbo68KGetContext(). Once they are mapped in, no
changes will be reflected in your context structures until you map them out.

NOTE: You CANNOT access context members while the context is mapped in unless
there are functions provided for that purpose (such as Turbo68KSetFetch(),
Turbo68KFreeTimeSlice(), Turbo68KReadPC(), etc.)

If there is no function for the operation you want to perform on the context,
you simply cannot do it until the context is mapped out. You may NOT map
contexts in and out from memory handlers while Turbo68K is running. Please
consult the function reference for more information.

Turbo68K provides an internal context called "turbo68kcontext_68000" if
emulating a 68000, or "turbo68kcontext_68010" if emulating a 68010. These are
where you copy your context when you use Turbo68KSetContext(). They should
only be used if you are emulating one processor.

When setting up memory maps, you can modify context pointers only if the
context was defined by you. If you wish to use the internal context, you will
have to use special functions to perform the same tasks. This is because
Turbo68K expects pointers in the internal context to be mangled (for
performance reasons.) Turbo68KSetContext()/Turbo68KGetContext() handle
this task automatically.

Here is an example of defining a context and mapping it in and out:

    struct TURBO68K_CONTEXT_68000 mycontext;

    /* ... Set up everything ... */

    Turbo68KSetContext(&mycontext);
    /* ... Run some code ... */
    Turbo68KGetContext(&mycontext);


4.4 Instruction Fetching

Turbo68K needs to know how to fetch instruction from 680X0 memory space. An
array of fetch regions must be set up describing where valid program areas are
located.

    Fetch Region Definition:
    ------------------------

    struct TURBO68K_FETCHREGION
    {
        TURBO68K_UINT32 base;
        TURBO68K_UINT32 limit;
        TURBO68K_UINT32 ptr;
    };

The "base" is the start address of the fetch region. The "limit" is the end
address. The "ptr" must be set to a host machine pointer value with the base
subtracted from it.

NOTE: Memory must be byte swapped. See section 4.8 for more information.

Here is an example of how a hypothetical system's fetch map might look:

    struct TURBO68K_FETCHREGION my_fetch[] =
    {
        { 0x000000, 0x3fffff, rom - 0x000000 },
        { 0xff0000, 0xffffff, ram - 0xff0000 },
        { -1,       -1,       TURBO68K_NULL }
    }

The last element is required to let Turbo68K know where the array ends.

To hook the fetch map up to your context, you would code:

    mycontext.super_fetch = my_fetch;
    mycontext.user_fetch = my_fetch;

If you are only using one unified address space for both supervisor and user
modes, all you have to do is:

    mycontext.fetch = my_fetch;

For the internal context, you must use the appropriate functions:

    Turbo68KSetFetch(my_fetch, TURBO68K_SUPERVISOR);
    Turbo68KSetFetch(my_fetch, TURBO68K_USER);

Or, if one address space is being used:

    Turbo68KSetFetch(my_fetch, TURBO68K_NULL);  /* second arg. ignored */


4.5 Data Access

Data accessing (byte, word, and long word read/writes) can be handled in a
number of ways. The first method is handled through definable memory map
arrays, somewhat similar to instruction fetching. This is the default method
(-defmap.)

    Data Region Definition:
    -----------------------

    struct TURBO68K_DATAREGION
    {
        TURBO68K_UINT32 base;
        TURBO68K_UINT32 limit;
        TURBO68K_UINT32 ptr;
        void            *handler;
    };

You may either set up a pointer, exactly the same as with instruction fetch
regions. But, if the pointer is not set up (TURBO68K_NULL), Turbo68K will
assume you want to use a handler. Simply set "handler" to the address of the
function you want to use, nothing has to be mangled.

Handler functions vary for different sizes of data:

    /*
     * Read byte
     */

    TURBO68K_UINT8 ReadByte(TURBO68K_UINT32 addr)
    {
        /* ... */
    }

    /*
     * Read word
     */

    TURBO68K_UINT16 ReadWord(TURBO68K_UINT32 addr)
    {
        /* ... */
    }

    /*
     * Read long word
     */

    TURBO68K_UINT32 ReadLong(TURBO68K_UINT32 addr)
    {
        /* ... */
    }

    /*
     * Write byte
     */

    void WriteByte(TURBO68K_UINT32 addr, TURBO68K_UINT8 data)
    {
        /* ... */
    }

    /*
     * Write word
     */

    void WriteWord(TURBO68K_UINT32 addr, TURBO68K_UINT16 data)
    {
        /* ... */
    }

    /*
     * Write long word
     */

    void WriteLong(TURBO68K_UINT32 addr, TURBO68K_UINT32 data)
    {
        /* ... */
    }

All handlers are passed addresses that are clipped to whichever value you
chose when emitting the emulator. By default, the address space is 24-bit.

NOTE: It is not a good idea to try to read the CCR from within a handler. Some
flag calculations are not completed until after data has been read/written.
In fact, do not read any registers besides PC (with the Turbo68KReadPC()
function.)

The 68000 and 68010 break down long word accesses into 2 consecutive word
word accesses, yet Turbo68K still uses long word handlers to provide
opportunities for optimization.

NOTE: Memory must be byte swapped. See section 4.8 for more information.

To hook up the memory maps, you would code:

    mycontext.super_read_byte = my_read_byte;
    mycontext.super_read_word = my_read_word;
    mycontext.super_read_long = my_read_long;
    mycontext.super_write_byte = my_write_byte;
    mycontext.super_write_word = my_write_word;
    mycontext.super_write_long = my_write_long;

    mycontext.user_read_byte = my_read_byte;
    mycontext.user_read_word = my_read_word;
    mycontext.user_read_long = my_read_long;
    mycontext.user_write_byte = my_write_byte;
    mycontext.user_write_word = my_write_word;
    mycontext.user_write_long = my_write_long;

If only one address space is being emulated, try:

    mycontext.read_byte = my_read_byte;
    mycontext.read_word = my_read_word;
    /* ... */

When using an internal context, use the appropriate functions:

    Turbo68KSetReadByte(my_read_byte, TURBO68K_SUPERVISOR);
    Turbo68KSetReadWord(my_read_word, TURBO68K_SUPERVISOR);
    /* ... */


The second method of handling data is to use functions exclusively. Use the
-handler option when emitting Turbo68K. Six functions (as shown above) must
be provided and directly hooked up to the context just as with memory maps.
Now, whenever the address space is read or written, your function will be
called and it is up to you to handle everything.


4.6 PC-Relative Reads

In some rare situations, PC-relative address reading cannot function properly
through the normal data access regions/handlers. An example of when this can
occur is when the emulated system uses partially encrypted ROMs (CPS-2.)

Turbo68K must be emitted with the -pcfetch option and a special fetch region
for PC-relative reads must be set up. Writes are still handled using the data
regions/handlers. The region is a TURBO68K_FETCHREGION and thus no handlers
can be used. Memory pointers must be provided in the exact same way as you
would for a normal fetch region.

The array of fetch regions must be attached to the "pcfetch" pointers of
the context:

    mycontext.pcfetch = pcfetch;
    mycontext.super_pcfetch = pcfetch;
    mycontext.user_pcfetch = pcfetch;

When using an internal context, use Turbo68KSetPCFetch() and
Turbo68KGetPCFetch().

This is a pretty poor hack, but it should do the job in most cases. This
should not be needed for most systems.


4.7 Interrupts

Interrupts may be triggered with the Turbo68KInterrupt() function and when
they are being processed, an option call-back function may be called.

Turbo68KInterrupt() takes 2 arguments, the first is the level number, and the
second is the vector. For the vector, you may use TURBO68K_AUTOVECTOR for an
auto-vectored interrupt, TURBO68K_SPURIOUS for a spurious interrupt, and
TURBO68K_UNINITIALIZED for an uninitialized interrupt.

Interrupts will be checked for and processed at the start of Turbo68KRun() or
when Turbo68KProcessInterrupts() is called.

If the "InterruptAcknowledge" member of the context is null, the function it
points to will not be called, otherwise, the call-back can be used to make
certain that interrupts have been processed, if the hardware being emulated
requires this (for example, on the Sega Genesis, the call-back can be used to
update IRQ-related bit settings in the VDP status register.)

The call-back takes a single argument: The interrupt vector number passed to
Turbo68KInterrupt() (a TURBO68K_UINT32), and returns nothing. If
TURBO68K_AUTOVECTOR was passed to Turbo68KInterrupt(), the call-back will get
the proper vector for the level being processed. If non-autovectored
interrupts are triggered, there is no way to determine which level is being
processed at the time the call-back is invoked.

The InterruptAcknowledge pointer can be directly accessed regardless of
whether you use your own context, or Turbo68K's internal context:

    turbo68kcontext_68000.InterruptAcknowledge = &MyIRQAckHandler;

Make sure to set it to TURBO68K_NULL if you do not wish to make use of the
call-back feature. It is possible to access this member at any time.


4.8 Byte Swapped Memory

Host memory which Turbo68K can access directly must be byte swapped! The
MSB (most significant byte) and LSB (least significant byte) of each 16-bit
aligned word must be reversed.

    Sample Byte Swapping Function:
    ------------------------------

    /*
     * void SwapMemory(TURBO68K_UINT8 *mem, TURBO68K_INT32 size);
     *
     * Byte swaps a given region of memory to work with Turbo68K's native
     * access capabilities.
     */

    void SwapMemory(TURBO68K_UINT8 *mem, TURBO68K_INT32 size)
    {
        TURBO68K_UINT32   i, j;

        /*
         * Swap bytes in each word
         */

        for (i = 0; i < length; i += 2)
        {
            j = buffer[i];
            buffer[i] = buffer[i + 1];
            buffer[i + 1] = j;
        }
    }

Byte swapping is an optimization which greatly benefits Turbo68K in terms of
speed. It relies on the fact that the 68000 and 68010 use a 16-bit data bus.
To access individual bytes in byte swapped memory, XOR the address with 1.
Words can be read or written normally. Long words are best accessed as 2
consecutive words.


4.9 The RESET and BKPT Instructions

When the RESET or BKPT instruction is executed, the respective handler is
called. If the handler is found to be TURBO68K_NULL, no action is taken.        
This is useful for emulating the functionality of the RESET (some systems need
to reset hardware when it is executed) and BKPT (some systems might need to
react to the Breakpoint Acknowledge Cycle) instructions. The BKPT handler is
called before the 68010 processes the breakpoint exception.

You can also perform high level emulation (HLE) by strategically inserting
RESET (or BKPT, if dealing with a 68010) instructions into the emulated code.
Use Turbo68KReadPC() to make sure you know which RESET/BKPT was executed.
Turbo68KReadPC() will point to the next instruction.

When setting up the handlers, no special functions are needed, even if the
internal context is being manipulated.

    mycontext.Reset = &MyRESETHandler;

    turbo68kcontext_68000.Reset = &MyRESETHandler;

The RESET handler takes no arguments and returns nothing. The BKPT handler is
passed the breakpoint vector number (0-7.)

    /*
     * Sample RESET handler
     */

    void MyRESETHandler()
    {
        /* ... Reset some hardware ... */
    }

    /*
     * Sample BKPT handler
     */

    void MyBKPTHandler(TURBO68K_INT32 num)
    {
        /* ... */
    }


4.10 Initializing and Resetting Turbo68K

Prior to doing anything, call Turbo68KInit() to initialize Turbo68K. This
only needs to be done once per program session.

Next, set up your memory maps. Then, reset the 680X0 processor by calling the
Turbo68KReset() function. Turbo68K fetches the initial PC and SP from the
supervisor fetch memory map, not through the long word read map. Turbo68K
will return an error if it could not reset. This means you have set up your
memory map incorrectly.

    /*
     * Initialize Turbo68K (VERY important)
     */

    Turbo68KInit();

    /* ... set up memory maps and interrupt call-back (optional) ... */

    /*
     * Reset the processor
     */

    if (Turbo68KReset() != TURBO68K_OKAY)
        Error();

    /* ... begin emulating ... */


4.11 Running the Emulator

To execute code, use Turbo68KRun(). Pass it the number of cycles you wish
to run for. Turbo68K will execute at least as many cycles as you wanted, but
often executes a bit more. You can use Turbo68KGetElapsedCycles() to find
out exactly how many were emulated. See the function reference for more
information.

Turbo68KInterrupt() can be used to trigger an interrupt. Please see section
4.7 for more information on interrupts.

    /*
     * Run Turbo68K for 1000 cycles
     */

    Turbo68KRun(1000);

    /*
     * Level 6 auto-vectored interrupt
     */

    Turbo68KInterrupt(6, TURBO68K_AUTOVECTOR);

Both Turbo68KRun() and Turbo68KInterrupt() can return error information.
See the function reference for more information.


-------------------------
  5. Function Reference
-------------------------

All of Turbo68K's functions are described here. Functions which are said not
to work under given circumstances will have undefined results if used under
those conditions. Usually, this means the context will be trashed, so be very
careful!

When Turbo68K is running, any handlers you may have set up are considered
part of Turbo68K. Thus, when it is said a function cannot be called while
Turbo68K is running, this means it cannot be called from a handler.

    TURBO68K_INT32 Turbo68KInit();
        Initializes the emulator. This must be called before anything else.
        Input:
            Nothing.
        Returns:
            TURBO68K_OKAY
        Restrictions:
            For future compatibility, call this only once per program session.

    TURBO68K_INT32 Turbo68KReset();
        Resets the 680X0. It uses the supervisor instruction fetch region to
        obtain the start-up vectors. Make sure everything is set up. A fetch
        error indicates either that the reset vectors could not be fetched or
        the PC reset vector points to an unhandled region of memory.
        Input:
            Nothing.
        Returns:
            TURBO68K_OKAY
            TURBO68K_ERROR_FETCH (PC is out of bounds)
        Restrictions:
            Do not use while Turbo68K is running.

    TURBO68K_INT32 Turbo68KRun(TURBO68K_INT32 cycles);
        Executes at least the specified number of cycles. To get the actual
        number of cycles that were emulated, use Turbo68KGetElapsedCycles().
        Input:
            Number of cycles.
        Returns:
            TURBO68K_OKAY
            TURBO68K_ERROR_FETCH (PC is out of bounds)
            TURBO68K_ERROR_INVINST (invalid instruction, if -noillegal was
            specified)
            TURBO68K_ERROR_STACKFRAME (68010 only, unsupported/invalid stack
            frame, see section 7.2)
        Restrictions:
            Do not use while Turbo68K is running.

    TURBO68K_INT32 Turbo68KProcessInterrupts();
        Processes any pending interrupts (unless the priority level does not
        agree with the SR priority bits.)
        Input:
            Nothing.
        Returns:
            TURBO68K_OKAY
            TURBO68K_ERROR_FETCH
        Restrictions:
            None. (If called while Turbo68K is running, it has no effect.)

    
    TURBO68K_INT32 Turbo68KInterrupt(TURBO68K_INT32 level,
                                     TURBO68K_UINT32 vector);
        Generates an interrupt with the specified level and vector. The level
        must be 1-7 and the vector cannot be less than 2. For the vector, you
        may specify the vector itself, TURBO68K_AUTOVECTOR for an autovectored
        interrupt, TURBO68K_SPURIOUS for a spurious interrupt, or
        TURBO68K_UNINITIALIZED for an uninitialized interrupt. If the
        "InterruptAcknowledge" member of the context is not null, the function
        it points to will be called as each interrupt is being taken.
        Input:
            Level.
            Vector.
        Returns:
            TURBO68K_OKAY
            TURBO68K_INTLEVEL (invalid level)
            TURBO68K_INTVECTOR (invalid vector)
            TURBO68K_INTPENDING (interrupt already pending at this level; this
            is not an error and will frequently occur, it is best not to check
            for this)
        Restrictions:
            None.
    
    TURBO68K_INT32 Turbo68KCancelInterrupt(TURBO68K_INT32 level);
        Cancels the pending interrupt at the specified level.
        Input:
            Level.
        Returns:
            TURBO68K_OKAY
            TURBO68K_INTLEVEL (invalid level)
        Restrictions:
            None.

    TURBO68K_UINT32 Turbo68KReadPC();
        Returns the current PC. If this is called from within a handler, the
        PC may point at the next instruction or to one of the operand words.
        Input:
            Nothing.
        Returns:
            PC
        Restrictions:
            None.

    void Turbo68KSetFetch(void *fetch, TURBO68K_INT32 type);
    void Turbo68KSetPCFetch(void *pcfetch, TURBO68K_INT32 type);
    void Turbo68KSetReadByte(void *read_byte, TURBO68K_INT32 type);
    void Turbo68KSetReadWord(void *read_word, TURBO68K_INT32 type);
    void Turbo68KSetReadLong(void *read_long, TURBO68K_INT32 type);
    void Turbo68KSetWriteByte(void *write_byte, TURBO68K_INT32 type);
    void Turbo68KSetWriteWord(void *write_word, TURBO68K_INT32 type);
    void Turbo68KSetWriteLong(void *write_long, TURBO68K_INT32 type);
        These functions will attach memory maps to the internal context. If
        you want to attach memory maps to a context you have created (which is
        not mapped in), you must assign them manually.
        Input:
            Memory map array or function pointer (depending on whether
            Turbo68K was configured for memory map arrays or handlers)
            Type: TURBO68K_SUPERVISOR or TURBO68K_USER. If you are not
            emulating the distinction, this argument can be set to anything
            and will be ignored.
        Returns:
            Nothing.
        Restrictions:
            If different address spaces for supervisor and user modes are
            being emulated and the "type" argument is unrecognized, the
            behavior is undefined.

    void *Turbo68KGetFetch(TURBO68K_INT32 type);
    void *Turbo68KGetPCFetch(TURBO68K_INT32 type);
    void *Turbo68KGetReadByte(TURBO68K_INT32 type);
    void *Turbo68KGetReadWord(TURBO68K_INT32 type);
    void *Turbo68KGetReadLong(TURBO68K_INT32 type);
    void *Turbo68KGetWriteByte(TURBO68K_INT32 type);
    void *Turbo68KGetWriteWord(TURBO68K_INT32 type);
    void *Turbo68KGetWriteLong(TURBO68K_INT32 type);
        Will return the memory map or function pointers that are associated
        with each type of access.
        Input:
            Type: TURBO68K_SUPERVISOR or TURBO68K_USER. If you are not
            emulating the distinction, this argument can be set to anything
            and will be ignored.
        Returns:
            Memory map array or function pointer.
        Restrictions:
            If different address spaces for supervisor and user modes are
            being emulated and the "type" argument is unrecognized, the
            behavior is undefined.

    TURBO68K_UINT8 *Turbo68KFetchPtr(TURBO68K_UINT32 addr);
        Return a host pointer to the specified address in the current address
        space. Turbo68K scans the fetch region and if the address is not
        found, it will return 0. This is useful for disassemblers.
        Input:
            Address.
        Returns:
            Host address.
        Restrictions:
            Only the address space currently being used will be scanned. In
            order to select the supervisor or user address space, you will
            have to modify the privilege mode bit in SR and restore it to its
            original value when finished.         
        
    TURBO68K_UINT8 Turbo68KReadByte(TURBO68K_UINT32 addr);
    TURBO68K_UINT16 Turbo68KReadWord(TURBO68K_UINT32 addr);
    TURBO68K_UINT32 Turbo68KReadLong(TURBO68K_UINT32 addr);
    void Turbo68KWriteByte(TURBO68K_UINT32 addr, TURBO68K_UINT8 data);
    void Turbo68KWriteWord(TURBO68K_UINT32 addr, TURBO68K_UINT16 data);
    void Turbo68KWriteLong(TURBO68K_UINT32 addr, TURBO68K_UINT32 data);
        These functions read/write data from the current Turbo68K address
        space. Thus, they can call handlers you have set up. These are
        particularly useful for debuggers. Be careful, the PC (and possibly
        other context data) will be invalid.
        Input:
            Address (and data if writing.)
        Returns:
            Data (or nothing if writing.)
        Restrictions:
            Do not use while Turbo68K is running.

    void Turbo68KSetContext(struct TURBO68K_CONTEXT *context);
    void Turbo68KGetContext(struct TURBO68K_CONTEXT *context);
        Map contexts in and out of the internal context using this pair of
        functions. Turbo68K operates only on its internal context, so it is
        necessary to copy externally-defined contexts into turbo68kcontext_*
        when you want to emulate the processor they define. These functions
        also make adjustments to the addresses contained in fetch, read_byte,
        etc. in order to speed things up -- therefore you cannot access these
        members, or any other context member, while it is mapped in.
        Input:
            Address of context to map in/out.
        Returns:
            Nothing.
        Restrictions:
            Do not use while Turbo68K is running.
        
    TURBO68K_UINT32 Turbo68KGetContextSize();
        Returns the size, in bytes, of a Turbo68K context. This is useful for
        asserting that your compiler is packing the contexts correctly.
        Input:
            Nothing.
        Returns:
            Size of context in bytes.
        Restrictions:
            None.
        
    void Turbo68KClearCycles();
        Clears both "cycles" and "remaining" causing Turbo68K to exit early.
        Be careful: a call to Turbo68KGetElapsedCycles() after this will
        return 0 or a small number which does not reflect the total amount of
        cycles used since Turbo68K was last called to run.
        Input:
            Nothing.
        Returns:
            Nothing.
        Restrictions:
            None.

    void Turbo68KFreeTimeSlice();
        Causes Turbo68K to stop running early by working some magic on both
        "cycles" and "remaining." Unlike Turbo68KClearCycles(), you can get
        the number of cycles executed with Turbo68KGetElapsedCycles().
        Input:
            Nothing.
        Returns:
            Nothing.
        Restrictions:
            If called from a handler, Turbo68K does not actually stop running
            until the current instruction finishes.

    TURBO68K_INT32 Turbo68KGetElapsedCycles();
        Returns the amount of cycles run.
        Input:
            Nothing.
        Returns:
            Cycles run.
        Restrictions:
            None.
        

---------------------
  6. Hints and Tips
---------------------

For those of you who want to get the most out of Turbo68K, this section aims
to provide you with some neat little tricks to try.


6.1 Alternative Identifiers

The default identifier names have nothing preceding them (Turbo68KInit(),
turbo68kcontext_68000, Turbo68KReset(), etc.) However, if the need arises, you
can change the identifier names by adding a string (up to 16 characters) to
the beginning of them.

Use the -id option to emit Turbo68K with different identifier names. The data
types and macros will not be changed, TURBO68K_CONTEXT_68000 will remain as it
is, so will TURBO68K_OKAY, etc.

You must also use the TURBO68K_ID() macro defined in TURBO68K.H to set the new
identifier names immediately after the header file has been included. For
example, if you want identifiers that start with "foo" you would write:

    #include "turbo68k.h"
    TURBO68K_ID(foo);

You can do this multiple times. If you have linked in 2 copies of Turbo68K,
one with identifiers that start with "foo", and another with identifiers
starting with "bar", you would write:

    #include "turbo68k.h"
    TURBO68K_ID(foo);
    TURBO68K_ID(bar);

With the above example, you would have functions like: fooTurbo68KInit(),
barTurbo68KInit(), and (if you linked in another copy of Turbo68K without
modified identifiers) Turbo68KInit().


6.2 Multiple Processors

Emulating multiple processors is fairly simple (unless extremely accurate
timing is required.) You have to set up a context for each processor. You may
not use the internal context as one of them, since it will be overwritten if
you map in the other processor.

    Simplistic Example of Running 2 68000s:
    ---------------------------------------

    struct TURBO68K_CONTEXT_68000 context[2];

    /* ... */

    for (int i = 0; i < 2; i++)
    {
        Turbo68KSetContext(&context[i]);
        Turbo68KRun(500);
        Turbo68KGetContext(&context[i]);
    }

Since Turbo68K is non-reentrant, you cannot multi-thread 2 processors unless
you link in 2 separate builds of Turbo68K (with different identifier names,
you can use the -id option for this.)


6.3 Maximizing Speed and Minimizing Size

What better reasons to write a CPU emulator in assembly language than for
speed and size? To get the most speed and least size (the 2 don't always co-
exist) out of Turbo68K, here are a few things you can try:

    1. If you are using memory map arrays, set them up wisely. If you have
       memory regions bordering each other, try turning them into a single
       large region. This not only cuts down the time it takes to scan through
       the map, but lets code flow between the regions. Put the most commonly
       accessed regions at the beginning of the array, and the least commonly
       accessed at the end.
    2. Consider using handlers and not memory map arrays (-handler option.) If
       you can write a fast and clean set of handlers, you can speed things up
       by taking a load of work off of Turbo68K's back. Optimize your long-
       word handlers as well. It is perfectly okay to have a long-word handler
       call your word handlers twice and return the combined data, but it is
       faster to write the handlers specifically for long-word accesses. 
    3. Execute code in large time slices. The overhead of Turbo68KRun() is
       significant but it can be compensated by spending more time emulating.
       Throw Turbo68KSetContext()/Turbo68KGetContext() in the mix and the
       gains are even greater.
    4. Try using -nodummyread if you can get away with it. For most systems,
       it is unnecessary to emulate this tiny detail.
    5. If possible, use -nobrafetch (the default) instead of -brafetch. The
       branch instructions are used a lot. If -skip does not impact compatib-
       ility, consider using it, too.
    6. If you can get away with sneaking in a 32-bit address bus, do it. The
       68000 and 68010 actuall use a 24-bit address bus, which means that
       Turbo68K has to mask off the unused bits when fetching data. The
       overhead of doing this is negligible, but if you can get away with
       using the full 32-bits, go for it.
    7. Don't allow separate address spaces for supervisor and user if you can
       get away with it (with most systems, it isn't needed.) This reduces
       some overhead in some of the instruction handlers and functions.
    8. Link the Turbo68K object file close to the object files which call it
       most frequently. The theory behind this is that the processor will be
       able to easily cache in code areas that use each other often. It might
       not always yield any speed increase, but I have observed slightly
       quicker performance when optimizing the link order in Genital.
    9. Strip the program you compile, this will reduce the executable tremend-
       ously. Turbo68K leaves behind a lot of symbols and junk.

There is a lot to do on my part. Turbo68K features some really poor code, but
usually in uncommon instructions. When/if I have time, I will optimize what I
can, but not all at once.


6.4 Sega Genesis TAS Bug

The TAS instruction is usually used in multi-processor systems to synchronize
execution between multiple 68Ks because it takes the bus. But on the Sega
Genesis, the VDP will not release the bus and TAS instructions consequently
do not write to memory.

In order to emulate this, comment out the call to WriteByte() in the TAS()
function in make68k.c. This will fix Gargoyles and possible some other games
which use the TAS instruction. The Genesis 3 apparently fixes this little bug
and games like Gargoyles no longer work.


-----------------------
  7. Additional Notes
-----------------------

This is a collection of miscellaneous notes, points of interest, and errata.


7.1 Change History

January 19, 2002: Version 0.6
    - CMPI and CMP no longer write back to memory
    - Turbo68KCancelInterrut() fixed (uses EAX*4 as an index into the
      interrupt array, so it now actually cancels interrupts)
    - Optmized Turbo68KCancelInterrupt() slightly
    - Added the "InterruptAcknowledge" call-back to facilitate more accurate
      interrupt handling on some systems (such as the Sega Genesis)
    - API functions now use the stack to save registers. Turbo68KInterrupt()
      can now be called from within a memory handler
    - Turbo68KGetXXX() functions now properly retrieve arguments passed to
      them
    - Some instruction handlers now use the stack to save registers

    ( Thanks to Charles MacDonald for submitting most of these fixes and
      additions! )
    
June 29, 2001: Version 0.5
    - Changed the name to "Turbo68K"
    - Undocumented flags emulated (verified against real 68000 processors)
    - N flag calculation for ABCD, SBCD, and NBCD instructions is now truly
      accurate
    - Dummy read emulation added for CLR, Scc, and the unprivileged version of
      "MOVE from SR." It is on by default and works only when the processor
      type is 68000
    - CHK comparisons are now signed as they should be
    - Fixed internal calls to API functions to work with register calling
      conventions

June 15, 2001: Version 0.4
    - Now uses proper identifier names when internally calling
      Genital68KUpdateFetchPtr()
    - N flag is now calculated for SBCD (thanks to ElSemi for this!)
    - Optimized C flag calculation for SBCD -(Ax),-(Ay) and ABCD -(Ax),-(Ay)
    - N flag for ABCD is calculated the same way as for SBCD (not sure if this
      is correct, I'll verify it once my Saturn 68K debugger is complete)

May 7, 2001: Version 0.35
    - Output file is now properly closed on exit
    - RTD is no longer emitted as a 68000 instruction (68010 only)
    - Initial PC and SP are now read from the instruction fetch memory maps
    - Instructions are emitted in a more optimal order, cache performance
      might be better

April 29, 2001: Version 0.3
    - Fixed a MOVEM bug (example: MOVEM.L (A7)+,A7)
    - Added rudimentary 68010 support
    - "MOVE from SR" is no longer privileged for 68000 processors
    - Rewrote documentation
    - Possibly other minor changes I forgot to list

February 11, 2001: Version 0.22
    - Added a PC-relative read mode (useful in very special circumstances)
    - Removed some unused data
    - Removed sign-extended byte read functions (they were not being used)

January 13, 2001: Version 0.21
    - Fixed Genital68KReadXXX() and Genital68KWriteXXX() functions, they no
      longer trash the PC

January 7, 2001: Version 0.2
    - Slightly modified Genital68KReadXXX() and Genital68KWriteXXX() functions
    - Registers are now saved when Genital68K functions are called
    - Minor optimizations here and there

December 8, 2000: Version 0.12
    - Fixed a bug with register calling conventions and read/write handlers

December 4, 2000: Version 0.11
    - Fixed a bug in Genital68KGetContext() and Genital68KSetContext() which
      corrupted user and supervisor address spaces
    - Corrected a misleading comment in MAKE68K.C and updated the macro refer-
      ence in this document

December 3, 2000: Version 0.1
    - Added user/supervisor address space distinction (optional)
    - Changed address space functions
    - Added a GENITAL68K_NULL macro
    - Fixed some mistakes in this document

November 23, 2000: Version 0.0
    - Initial release


7.2 Un-emulated Features

The following are not emulated:

    Tracing         Turbo68K does not emulate trace exceptions.
    Address Errors  Turbo68K does not emulate address error exceptions. The
                    X86 allows for misaligned accesses (unless alignment
                    checking is on), and consequently, so does Turbo68K.
    Bus Errors      Bus errors are not emulated.
    Undefined Bits  Undefined bits in the SR are usually kept 0.
    FC Output Lines The real 68000 and 68010 classify address space accesses
                    as either "User Data", "User Program", "Supervisor Data",
                    "Supervisor Program", or "CPU Space." Turbo68K
                    simplifies this by supporting a user address space and a
                    supervisor address space. There is also an option to
                    disable this distinction and use one address space.    
    MOVES           The effects of the alternate function code registers (SFC
                    and DFC) are not emulated. This includes the 68010 MOVES
                    instruction.
    Loop Mode       Loop mode (68010) is not emulated. Perhaps if someone
                    supplies me with very detailed information on how it
                    works, I might consider adding it.
    Stack Frames    Only stack frame 0 is supported. Stack frame 8 (associated
                    with group 0 exceptions: bus and address errors) is never
                    used and if encountered, Turbo68KRun() returns a stack
                    frame error. When such an error occurs, the processor
                    state will look as if the problematic RTE instruction had
                    never been executed.
    68010 Timing    For instructions common to both the 68000 and 68010 (most
                    of them), 68000 timing is used.


7.3 Errata

Although I have attempted to fix every bug I came across, there is bound to be
something obscure, or possibly major, left undiscovered. There are a few
unimportant inaccuracies I do know about, but have not bothered to fix yet:

    DIVS, DIVU      Timing is off by up to 10% in worst-case scenarios.
    MULS, MULU      Timing is off by an unknown amount. The multiplication
                    algorithm used by the 68000 and 68010 affects timing in
                    pretty odd ways.
    Timing          Some of the instructions have slightly inacurrate timing,
                    usually because of human error. Timing for exceptions and
                    interrupts is often wrong (but not TOO far off.)
    Exceptions      Priority between interrupts and other exceptions (such as
                    privilege violation, division by 0, TRAP, TRAPV, etc.) is
                    not fully emulated. Priority among interrupts themselves
                    seems to be properly handled, and that is most important.
                    I have not seen any situations where interrupts and other
                    exceptions affect each other.
    
More information on the priority of spurious and uninitialized interrupts as
well as level 7 would be appreciated. I am going off what little the Motorola
manuals say.


7.4 Bibliography

Most of what I know about 680X0 processors comes from the 2 free manuals I
ordered from Motorola. The rest comes from individuals who kindly helped me
and are mentioned in section 8.

    Documents Used:
    ---------------

    - "Programmer's Reference Manual" by Motorola (M68000PM/AD REV 1)
    - "M68000 8-/16-/32-Bit Microprocessor User's Manual Ninth Edition" by
      Motorola (M68000UM/AD REV 8)


---------------------
  8. Special Thanks
---------------------

Many thanks go out to those who helped me out with Turbo68K and contributed
to the project in any form.

    - Neil Bradley
    - Victor Moya
    - Dynarec Mailing List: M.I.K.e, Neil Griffiths, Gwenole, etc.
    - Quintesson
    - Steve Snake
    - Tim Meekins
    - Kuwanger
    - Charles MacDonald
    - Stephane Dallongeville
    - Eli Dayan
    - Neill Corlett
    - Pete Dabbs
    - ElSemi

And, of course, thank YOU! Remember, comments, suggestions, and contributions
are more than welcome, so get in touch with me. See you next time!                                                               
