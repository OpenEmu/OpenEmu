
---------------------------------------------------------
source-readme.txt
---------------------------------------------------------

This file is a simple attempt at explaining some subtleties of the
NeoPop source code. It will eventually turn into a sort of FAQ.

If you feel I have omitted something that isn't as obvious as I thought
it was then let me know! It's difficult to see this project from the
point of view of an outsider...



1.	Additional Include paths
============================

In order to simplify the core layout and readability, I have chosen
to define some additional include paths. These are (for all builds):

	Core\,
	Core\TLCS900h\,
	Core\Z80\



2.	The Debugger
================

A build including the debugger is activated by defining 'NEOPOP_DEBUG'
globally. In the Windows port this is done using the project settings,
but GCC can use the -D command line option (IIRC).

I don't expect many people to bother porting the Windows debugger -
it would require a complete re-write. I'm not sure this is to anyone's
advantage because I don't believe there are many people that use it...
I may be wrong.

I have tried to make it as easy as possible to remove the debugger
from the code base - simply remove the Debugger folder. 
The "#ifdef NEOPOP_DEBUG" directives will prevent any debugging code from
being compiled.



3.	ZIP Support
===============

ZIP support (provided by zLIB 1.1.4) can optionally be omitted by changing
a #define in 'system_rom.h' and removing the 'zLIB' folder.



4.	Flash Support
=================

Flash support can be omitted initially by just returning 'false' when reading
and 'true' when writing - this will keep the core happy. I hope that porters
can manage the unpredictably long flash files - only an issue for consoles,
i'd say. I guess they could be padded out to a fixed 2 / 4 / 8 kb if need be.
Or they could be zipped, this crushes the "Sonic" data from 6168 bytes to 309!



5.	Porting and Core Consistency
================================

The source code for the O/S specific wrapper should be put into a folder
called 'SYSTEM_xxx', where xxx is the name of the system. For example
"SYSTEM_LINUX-SDL", "SYSTEM_DC", "SYSTEM_MACOS", etc...

My aim is to have only one commmon core code for all ports. To facilitate this
the predefined symbol 'SYSTEM_xxx' should be used to mark out any core componenets
that only apply to a single system. Hopefully these can be kept to a minimum,
but there may be cause for this approach on some systems.

Makefiles should indicate the port system in their file extension. For example
the DreamCast port would have a makefile called: "makefile.dc".

THE CORE IS SUBJECT TO CHANGE AT ANY TIME! The only file to rely on
is 'neopop.h', this is the only core file that should be included by a system
file. If there is something missing from this file, please e-mail me.
