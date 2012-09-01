Help for Supermodel Console-based Debugger
==========================================

Relevant Supermodel Command Line Options
----------------------------------------

	-enter-debugger		Enters the debugger at the start of emulation,

	-disable-debugger	Completely disables the debugger in emulator.

At any point whilst running the emulator, execution can be halted and the debugger entered by pressing Alt+B.

Code Analysis
-------------

At startup, and whenever new code is encountered, the debugger analyses the program's code in order to work out all
reachable code locations from a given set of entry points (reset address, exception handlers etc).  During the analysis, 
it keeps track of all valid code locations and this is used to align instructions for CPUs with variable length instruction
sets.  It also generates a set of auto-labels that identify places of interest such as sub-routines, jump destinations and
exception handlers etc.  Code analysis can be switched off via the 'configure' command if required.

Debugger Commands
=================
		
Execution
---------
	
n		next					[<count>=1]		

	Runs the next single instruction or the next <count> instructions.

nf		nextframe				[<count>=1]

	Runs until the next frame or for the next <count> frames.
	
s		stepover

	Runs the next instruction.  If it is a call to a sub-routine then the sub-routine is 'stepped over' (ie control
	breaks at the return address of the sub-routine).
	
si		stepinto

	Runs the next single instruction, entering any sub-routines (ie same as 'next' above).
	
so		stepout

	Runs until control returns from the current sub-routine or exception/interrupt handler.  Note that if an
	exception/interrupt occurs whilst stepping out then this will break execution too as control will have left
	the sub-routine or handler.
	
c		continue				[<addr>]

	Continues running until a break is forced or if <addr> specified, until the given address is reached.

spc		setpc					<addr>

	Sets the PC of the current CPU to the given address.
	
CPUs
----

lc		listcpus

	Lists all the available CPUs with details about them.
	
sc		switchcpu				(<name>|<num>)

	Switches to another CPU with the given name or number.
	
dc		disablecpu				(<name>|<num>)

	Disables debugging for the CPU with the given name or number.  Once disabled, it is no longer 
	possible to switch to or halt execution for that CPU, ie all breakpoints, watches, monitors and traps
	are ignored.  The current CPU cannot be disabled.
	
ec		enablecpu				(<name>|<num>)

	Enables debugging for the CPU with the given name or number.

Registers
---------
	
lr		listregisters

	Lists all registers for the current CPU and their current values.

pr		printregister			<reg>

	Prints the current value of the given register for the current CPU.
	
sr		setregister				<reg> <value>

	Sets the value of the given register for the current CPU.

lm	listmonitors

	Lists all register monitors for the current CPU with details about them.
	
m		monitor					<reg>
am		addmonitor				<reg>

	Adds a register monitor for the register with the given name, causing execution to break whenever the register's value
	changes.
	
rm		removemonitor			<reg>

	Removes the register monitor with the given name.
	
ram		removeallmonitors

	Removes all register monitors for the current CPU.
		
Exceptions & Interrupts
-----------------------

le		listexceptions

	Lists all known exceptions for the current CPU and details about them.
	
li		listinterrupts

	Lists all known interrupts for the current CPU and details about them.
	
t		trap					((e)xception|(i)nterrupt) <id>
at		addtrap					((e)xception|(i)nterrupt) <id>

	Adds an exception or interrupt trap for the exception/interrupt with the given identifier.
	
rt		removetrap				((e)xception|(i)nterrupt) <id>

	Removes an exception or interrupt trap for the exception/interrupt with the given identifier.
	
rat		removealltraps			[(a)ll|(e)xceptions|(i)nterrupts]

	Removes all exception and/or all interrupt traps for the current CPU.  If no arguments are supplied, all exception and all
	interrupt traps are removed.
	
Disassembly, Labels & Comments
------------------------------
	
l		list					[<start>=last [#<instrs>=20|<end>]]
ld		listdisassembly			[<start>=last [#<instrs>=20|<end>]]

	Lists the disassembled code of the current CPU for the given address range and/or number of instructions.
	
	If the start address is not supplied, the listing continues on from the last call, or from just before the current PC
	address if this is the first call.
	If the end address or instruction count is not supplied, then the default of 20 instructions are listed.
	
	If code analysis is enabled, then this will be used to align the instructions in the disassembly.  If code analysis is
	off and the current PC address falls within the disassembly address range, then the instructions will line up with this.
	Otherwise, instruction alignment will simply begin at the start address.
	
ll		listlabels				[(d)efault|(c)ustom|(a)utos|(e)ntrypoints|e(x)cephandlers|(i)interhandlers|(j)umptargets|(l)ooppoints]

	Lists all labels for the current CPU.  The default option is to list all custom labels (user-added) and any auto-labels
	(labels generated by code analysis) of interest.  The other options allow the display of particular types of auto-labels.
	
al		addlabel				<addr> <name>

	Adds a custom label at the given address and with the given name.
	
rl		removelabel				[<name>|<addr>]

	Removes a custom label with the given name or at the given address.  If no argument is supplied, then it removes the
	custom label at the current PC address.

ral		removealllabels
	
	Removes all custom labels for the current CPU.
	
ac		addcomment				<addr> <text...>

	Adds a code comment at the given address and with the given text.

rc		removecomment			[<addr>]

	Removes the code comment at the given address.  If no argument is supplied, then it removes the code comment at the 
	current PC address.
	
rac		removeallcomments

	Removes all code comments for the current CPU.
	
Breakpoints
-----------
	
lb		listbreakpoints

	Lists all breakpoints for the current CPU with details about them.
	
b		breakpoint				[<addr> [[s)imple|(c)ount <count>|(p)rint)]]
ab		addbreakpoint			[<addr> [[s)imple|(c)ount <count>|(p)rint)]]

	Adds a breakpoint at the given address.  If no arguments are supplied, then it adds a simple breakpoint at the current PC
	address.

	Types of breakpoint (default is simple):
		(s)imple	- if the location is hit, execution always breaks,
		(c)ount		- if the location is hit the number of times specified by count, execution breaks.
		(p)rint		- if the location is hit, a message is printed to the console, but control does not break.
		
rb		removebreakpoint		[#<num>|<addr>]

	Removes the breakpoint with the given number of at the given address.  If no arguments is supplied, then it removes
	the breakpoint at the current PC address.
	
Memory, I/O & Watches
---------------------

ln		listregions

	Lists all known memory regions in the current CPU's address space.

ly		listmemory				[<start>=last [#<rows>=8|<end>]]

	Lists the memory contents of the current CPU for the given address range and/or number of rows.
	
	If the start address is not supplied, the listing continues on from the last call, or from address 0 if this is the 
	first call.
	If the end address or row count is not supplied, then the default of 8 rows are listed.
	
py		printmemory[.<size>=b]	<addr> [(h)ex|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]

	Prints the current memory value at the given address for the current CPU.  If no format is supplied, the default data
	format is used.
	
	The size specifier may be a number or (b)yte, (w)ord, (l)ong or (v)erylong.  The default if omitted is byte.
		
sy		setmemory[.<size>=b]	<addr> <value>

	Sets the memory value at the given address for the current CPU.
	
	The size specifier may be a number or (b)yte, (w)ord, (l)ong or (v)erylong.  The default if omitted is byte.
		
lo		listios

	Lists all known I/Os (both mapped I/O addresses and I/O ports) for the current CPU and details about them.

lw		listmemwatches	

	Lists all memory watches for the current CPU with details about them.

w		memwatch[.<size>=b]		<addr> [((n)one|(r)ead|(w)rite|(rw)eadwrite) [((s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint)]]
aw		addmemwatch[.<size>=b]	<addr> [((n)one|(r)ead|(w)rite|(rw)eadwrite) [((s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint)]]

	Adds a read/write memory watch at the given address.
	
	If (r)ead, (w)rite or (rw)eadwrite are specified then the watch also triggers an event (see below) when the
	address is read, written or either read or written, respectively.
	If (n)one is specified (the default), the watch does not trigger and simply remembers the last read/write at the 
	given address (details about which are visible when listing memory watches).

	The size specifier may be a number or (b)yte, (w)ord, (l)ong or (v)erylong.  The default if omitted is byte.  If the
	address is mapped I/O address, then the size is ignored.
	
	Types of watch (default is simple):
		(s)imple	- if the watch is triggered, execution always breaks,
		(c)ount		- if the watch is triggered, execution breaks after the number of times specified by count,
		(m)atch		- if the watch is triggered, execution breaks if the series of values read/written matches the given 
					  sequence of data (comma separated),
		captu(r)e	- if the watch is triggered, the value read/written is recorded in a history whose maximum length
					  is as specified,
		(p)rint		- if the watch is triggered, a message with the value read/written is printed to the console.
				  
rw		removememwatch			(#<num>|<addr>)

	Removes the memory watch with the given number or at the given address.
	
raw		removeallmemwatches

	Removes all memory watches for the current CPU.
	
lpw		listportwatches
	
	Lists all I/O port watches for the current CPU with details about them.	

pw		portwatch				<port> [((n)one|(i)nput|(o)utput|(io)nputoutput) [(s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint]]
apw		addportwatch			<port> [((n)one|(i)nput|(o)utput|(io)nputoutput) [(s)imple|(c)ount <count>|(m)atch <sequence>|captu(r)e <maxlen>|(p)rint]]

	Adds an input/output watch for the given I/O port.
	
	If (i)nput, (o)utput or (io)nputoutput are specified then the watch also triggers an event (see below) when the
	port is read (input), written (output) or either read or written, respectively.
	If (n)one is specified (the default), the watch does not trigger and simply remembers the last read/write for the
	given port (details about which are visible when listing port watches).

	Types of watch (default is simple):
		(s)imple	- if the watch is triggered, execution always breaks,
		(c)ount		- if the watch is triggered the number of times specified by count, execution breaks,
		(m)atch		- if the watch is triggered, execution breaks if the series of values input/output matches the given 
					  sequence of data (comma separated),
		captu(r)e	- if the watch is triggered, the value inputted/outputted is recorded in a history whose maximum length
					  is as specified,
		(p)rint		- if the watch is triggered, a message with the value inputted/outputted is printed to the console.
		
rpw		removeportwatch			(#<num>|<port>)

	Removes the port watch with the given number or for the given port.
	
rapw	removeallportwatches
	
	Removes all port watches for the current CPU.
	
General
--------

p		print[.<size>=v]		<expr> [(h)ex|hex(z)ero|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]

	Prints the given expression as a number in the given format.  If no format is supplied, the default data format is used.
	Currently the "expression" can just be a number, label or register name.
	
	The size specifier may be a number or (b)yte, (w)ord, (l)ong or (v)erylong.  The default if omitted depends on the
	type of expression.

cfg		configure				analysis [(o)n|of(f)]
								addrfmt [(h)ex|hex(z)ero|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]
								portfmt [(h)ex|hex(z)ero|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]
								datafmt [(h)ex|hex(z)ero|hexdo(l)lar|hex(p)osth|(d)ecimal|(b)inary]
								showlabels [(o)n|of(f)]
								showopcodes [(o)n|of(f)]
								membytesrow [<num>]
	
	Configures the debugger.
	If no arguments are passed, it outputs all the current settings.
	If a setting is supplied, it outputs the current value for the given setting.
	If both a setting and a value are supplied, it sets the current value for the given setting.
	
ls		loadstate				<file>

	Loads the debugger state (custom labels and code comments) from the given file.
	
ss		savestate				<file>

	Save the debugger state (custom labels and code comments) to the given file.
	
h		help

	Prints the available commands.
	
x		exit

	Exits the debugger and emulator.
	
Emulator
--------

les		loademustate			<file>

	Loads the emulator state from the given file.
	
ses		saveemustate			<file>

	Saves the emulator state to the given file.

res		resetemu

	Resets the emulator.
	
Inputs
------

lip		listinputs

	Lists all available inputs for the current game.
	
pip		printinput				(<id>|<label>)

	Prints details about the input with the given id or label.
	
sip		setinput				(<id>|<label>) <mapping>

	Sets the current mapping for the input with the given id or label.

rip		resetinput				(<id>|<label>)

	Resets the mapping to its default for the input with the given id or label.
	
cip		configinput				(<id>|<label>) [(s)et|(a)ppend]

	Configures the input with the given id or label, in a similar fashion to -config-inputs.  The default is to
	set the mapping but if append is specified the mapping is appended to.
	
caip	configallinputs	

	Configures all the inputs for the current game, in a similar fashion to -config-inputs.
	
Notes
-----

The output of any command can be redirected to a file by adding the > or >> redirection symbols after the command together
with the filename to write or append to, eg:

		listdisassembly MainEntry #50 > c:\disassembly.txt
		
or		listmemory $0000 $1000 >> ramdump.txt