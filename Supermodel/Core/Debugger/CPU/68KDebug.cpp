/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * 68KDebug.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "68KDebug.h"

#include <cctype>
#include <string>

#define M68KSPECIAL_SP 0
#define M68KSPECIAL_SR 1

namespace Debugger
{
	C68KDebug::C68KDebug(const char *name) : CCPUDebug("68K", name, 2, 10, true, 24, 7)
	{
		// Exceptions
		AddException("BUS",     2,  "Bus Error");
		AddException("ADDRESS", 3,  "Address Error");
		AddException("ILLEGAL", 4,  "Illegal Instruction");
		AddException("DIVZERO", 5,  "Divide by Zero");
		AddException("CHK",     6,  "CHK Instruction");
		AddException("TRAPV",   7,  "TRAPV Instruction");
		AddException("PRIVEX",  8,  "Privilege Exception");
		AddException("TRACE",   9,  "Trace");
		AddException("L1010",   10, "Line 1010 Emulator");
		AddException("L1111",   11, "Line 1111 Emulator");
		AddException("SPUR",    24, "Spurious Interrupt");
		AddException("AUTO",    25, "Interrupt Autovector");
		AddException("TRAP0",   32, "TRAP #0 Instruction Vector");
		AddException("TRAP1",   33, "TRAP #1 Instruction Vector");
		AddException("TRAP2",   34, "TRAP #2 Instruction Vector");
		AddException("TRAP3",   35, "TRAP #3 Instruction Vector");
		AddException("TRAP4",   36, "TRAP #4 Instruction Vector");
		AddException("TRAP5",   37, "TRAP #5 Instruction Vector");
		AddException("TRAP6",   38, "TRAP #6 Instruction Vector");
		AddException("TRAP7",   39, "TRAP #7 Instruction Vector");
		AddException("TRAP8",   40, "TRAP #8 Instruction Vector");
		AddException("TRAP9",   41, "TRAP #9 Instruction Vector");
		AddException("TRAP10",  42, "TRAP #10 Instruction Vector");
		AddException("TRAP11",  43, "TRAP #11 Instruction Vector");
		AddException("TRAP12",  44, "TRAP #12 Instruction Vector");
		AddException("TRAP13",  45, "TRAP #13 Instruction Vector");
		AddException("TRAP14",  46, "TRAP #14 Instruction Vector");
		AddException("TRAP15",  47, "TRAP #15 Instruction Vector");
		
		// Interrupts
		AddInterrupt("AUTO1", 0, "Level 1 Interrupt Autovector");
		AddInterrupt("AUTO2", 1, "Level 2 Interrupt Autovector");
		AddInterrupt("AUTO3", 2, "Level 3 Interrupt Autovector");
		AddInterrupt("AUTO4", 3, "Level 4 Interrupt Autovector");
		AddInterrupt("AUTO5", 4, "Level 5 Interrupt Autovector");
		AddInterrupt("AUTO6", 5, "Level 6 Interrupt Autovector");
		AddInterrupt("AUTO7", 6, "Level 7 Interrupt Autovector");
	}

	static const char *opATable0004[] = { "movep.w [DW3](A0),D0","movep.w [DW3](A1),D0","movep.w [DW3](A2),D0",
		"movep.w [DW3](A3),D0","movep.w [DW3](A4),D0","movep.w [DW3](A5),D0","movep.w [DW3](A6),D0",
		"movep.w [DW3](A7),D0" };
	static const char *opATable0005[] = { "movep.l [DW3](A0),D0","movep.l [DW3](A1),D0","movep.l [DW3](A2),D0",
		"movep.l [DW3](A3),D0","movep.l [DW3](A4),D0","movep.l [DW3](A5),D0","movep.l [DW3](A6),D0",
		"movep.l [DW3](A7),D0" };
	static const char *opATable0006[] = { "movep.w D0,[DW3](A0)","movep.w D0,[DW3](A1)","movep.w D0,[DW3](A2)",
		"movep.w D0,[DW3](A3)","movep.w D0,[DW3](A4)","movep.w D0,[DW3](A5)","movep.w D0,[DW3](A6)",
		"movep.w D0,[DW3](A7)" };
	static const char *opATable0007[] = { "movep.l D0,[DW3](A0)","movep.l D0,[DW3](A1)","movep.l D0,[DW3](A2)",
		"movep.l D0,[DW3](A3)","movep.l D0,[DW3](A4)","movep.l D0,[DW3](A5)","movep.l D0,[DW3](A6)",
		"movep.l D0,[DW3](A7)" };
	static const char *opATable000C[] = { "movep.w [DW3](A0),D1","movep.w [DW3](A1),D1","movep.w [DW3](A2),D1",
		"movep.w [DW3](A3),D1","movep.w [DW3](A4),D1","movep.w [DW3](A5),D1","movep.w [DW3](A6),D1",
		"movep.w [DW3](A7),D1" };
	static const char *opATable000D[] = { "movep.l [DW3](A0),D1","movep.l [DW3](A1),D1","movep.l [DW3](A2),D1",
		"movep.l [DW3](A3),D1","movep.l [DW3](A4),D1","movep.l [DW3](A5),D1","movep.l [DW3](A6),D1",
		"movep.l [DW3](A7),D1" };
	static const char *opATable000E[] = { "movep.w D1,[DW3](A0)","movep.w D1,[DW3](A1)","movep.w D1,[DW3](A2)",
		"movep.w D1,[DW3](A3)","movep.w D1,[DW3](A4)","movep.w D1,[DW3](A5)","movep.w D1,[DW3](A6)",
		"movep.w D1,[DW3](A7)" };
	static const char *opATable000F[] = { "movep.l D1,[DW3](A0)","movep.l D1,[DW3](A1)","movep.l D1,[DW3](A2)",
		"movep.l D1,[DW3](A3)","movep.l D1,[DW3](A4)","movep.l D1,[DW3](A5)","movep.l D1,[DW3](A6)",
		"movep.l D1,[DW3](A7)" };
	static const char *opATable0014[] = { "movep.w [DW3](A0),D2","movep.w [DW3](A1),D2","movep.w [DW3](A2),D2",
		"movep.w [DW3](A3),D2","movep.w [DW3](A4),D2","movep.w [DW3](A5),D2","movep.w [DW3](A6),D2",
		"movep.w [DW3](A7),D2" };
	static const char *opATable0015[] = { "movep.l [DW3](A0),D2","movep.l [DW3](A1),D2","movep.l [DW3](A2),D2",
		"movep.l [DW3](A3),D2","movep.l [DW3](A4),D2","movep.l [DW3](A5),D2","movep.l [DW3](A6),D2",
		"movep.l [DW3](A7),D2" };
	static const char *opATable0016[] = { "movep.w D2,[DW3](A0)","movep.w D2,[DW3](A1)","movep.w D2,[DW3](A2)",
		"movep.w D2,[DW3](A3)","movep.w D2,[DW3](A4)","movep.w D2,[DW3](A5)","movep.w D2,[DW3](A6)",
		"movep.w D2,[DW3](A7)" };
	static const char *opATable0017[] = { "movep.l D2,[DW3](A0)","movep.l D2,[DW3](A1)","movep.l D2,[DW3](A2)",
		"movep.l D2,[DW3](A3)","movep.l D2,[DW3](A4)","movep.l D2,[DW3](A5)","movep.l D2,[DW3](A6)",
		"movep.l D2,[DW3](A7)" };
	static const char *opATable001C[] = { "movep.w [DW3](A0),D3","movep.w [DW3](A1),D3","movep.w [DW3](A2),D3",
		"movep.w [DW3](A3),D3","movep.w [DW3](A4),D3","movep.w [DW3](A5),D3","movep.w [DW3](A6),D3",
		"movep.w [DW3](A7),D3" };
	static const char *opATable001D[] = { "movep.l [DW3](A0),D3","movep.l [DW3](A1),D3","movep.l [DW3](A2),D3",
		"movep.l [DW3](A3),D3","movep.l [DW3](A4),D3","movep.l [DW3](A5),D3","movep.l [DW3](A6),D3",
		"movep.l [DW3](A7),D3" };
	static const char *opATable001E[] = { "movep.w D3,[DW3](A0)","movep.w D3,[DW3](A1)","movep.w D3,[DW3](A2)",
		"movep.w D3,[DW3](A3)","movep.w D3,[DW3](A4)","movep.w D3,[DW3](A5)","movep.w D3,[DW3](A6)",
		"movep.w D3,[DW3](A7)" };
	static const char *opATable001F[] = { "movep.l D3,[DW3](A0)","movep.l D3,[DW3](A1)","movep.l D3,[DW3](A2)",
		"movep.l D3,[DW3](A3)","movep.l D3,[DW3](A4)","movep.l D3,[DW3](A5)","movep.l D3,[DW3](A6)",
		"movep.l D3,[DW3](A7)" };
	static const char *opATable0024[] = { "movep.w [DW3](A0),D4","movep.w [DW3](A1),D4","movep.w [DW3](A2),D4",
		"movep.w [DW3](A3),D4","movep.w [DW3](A4),D4","movep.w [DW3](A5),D4","movep.w [DW3](A6),D4",
		"movep.w [DW3](A7),D4" };
	static const char *opATable0025[] = { "movep.l [DW3](A0),D4","movep.l [DW3](A1),D4","movep.l [DW3](A2),D4",
		"movep.l [DW3](A3),D4","movep.l [DW3](A4),D4","movep.l [DW3](A5),D4","movep.l [DW3](A6),D4",
		"movep.l [DW3](A7),D4" };
	static const char *opATable0026[] = { "movep.w D4,[DW3](A0)","movep.w D4,[DW3](A1)","movep.w D4,[DW3](A2)",
		"movep.w D4,[DW3](A3)","movep.w D4,[DW3](A4)","movep.w D4,[DW3](A5)","movep.w D4,[DW3](A6)",
		"movep.w D4,[DW3](A7)" };
	static const char *opATable0027[] = { "movep.l D4,[DW3](A0)","movep.l D4,[DW3](A1)","movep.l D4,[DW3](A2)",
		"movep.l D4,[DW3](A3)","movep.l D4,[DW3](A4)","movep.l D4,[DW3](A5)","movep.l D4,[DW3](A6)",
		"movep.l D4,[DW3](A7)" };
	static const char *opATable002C[] = { "movep.w [DW3](A0),D5","movep.w [DW3](A1),D5","movep.w [DW3](A2),D5",
		"movep.w [DW3](A3),D5","movep.w [DW3](A4),D5","movep.w [DW3](A5),D5","movep.w [DW3](A6),D5",
		"movep.w [DW3](A7),D5" };
	static const char *opATable002D[] = { "movep.l [DW3](A0),D5","movep.l [DW3](A1),D5","movep.l [DW3](A2),D5",
		"movep.l [DW3](A3),D5","movep.l [DW3](A4),D5","movep.l [DW3](A5),D5","movep.l [DW3](A6),D5",
		"movep.l [DW3](A7),D5" };
	static const char *opATable002E[] = { "movep.w D5,[DW3](A0)","movep.w D5,[DW3](A1)","movep.w D5,[DW3](A2)",
		"movep.w D5,[DW3](A3)","movep.w D5,[DW3](A4)","movep.w D5,[DW3](A5)","movep.w D5,[DW3](A6)",
		"movep.w D5,[DW3](A7)" };
	static const char *opATable002F[] = { "movep.l D5,[DW3](A0)","movep.l D5,[DW3](A1)","movep.l D5,[DW3](A2)",
		"movep.l D5,[DW3](A3)","movep.l D5,[DW3](A4)","movep.l D5,[DW3](A5)","movep.l D5,[DW3](A6)",
		"movep.l D5,[DW3](A7)" };
	static const char *opATable0034[] = { "movep.w [DW3](A0),D6","movep.w [DW3](A1),D6","movep.w [DW3](A2),D6",
		"movep.w [DW3](A3),D6","movep.w [DW3](A4),D6","movep.w [DW3](A5),D6","movep.w [DW3](A6),D6",
		"movep.w [DW3](A7),D6" };
	static const char *opATable0035[] = { "movep.l [DW3](A0),D6","movep.l [DW3](A1),D6","movep.l [DW3](A2),D6",
		"movep.l [DW3](A3),D6","movep.l [DW3](A4),D6","movep.l [DW3](A5),D6","movep.l [DW3](A6),D6",
		"movep.l [DW3](A7),D6" };
	static const char *opATable0036[] = { "movep.w D6,[DW3](A0)","movep.w D6,[DW3](A1)","movep.w D6,[DW3](A2)",
		"movep.w D6,[DW3](A3)","movep.w D6,[DW3](A4)","movep.w D6,[DW3](A5)","movep.w D6,[DW3](A6)",
		"movep.w D6,[DW3](A7)" };
	static const char *opATable0037[] = { "movep.l D6,[DW3](A0)","movep.l D6,[DW3](A1)","movep.l D6,[DW3](A2)",
		"movep.l D6,[DW3](A3)","movep.l D6,[DW3](A4)","movep.l D6,[DW3](A5)","movep.l D6,[DW3](A6)",
		"movep.l D6,[DW3](A7)" };
	static const char *opATable003C[] = { "movep.w [DW3](A0),D7","movep.w [DW3](A1),D7","movep.w [DW3](A2),D7",
		"movep.w [DW3](A3),D7","movep.w [DW3](A4),D7","movep.w [DW3](A5),D7","movep.w [DW3](A6),D7",
		"movep.w [DW3](A7),D7" };
	static const char *opATable003D[] = { "movep.l [DW3](A0),D7","movep.l [DW3](A1),D7","movep.l [DW3](A2),D7",
		"movep.l [DW3](A3),D7","movep.l [DW3](A4),D7","movep.l [DW3](A5),D7","movep.l [DW3](A6),D7",
		"movep.l [DW3](A7),D7" };
	static const char *opATable003E[] = { "movep.w D7,[DW3](A0)","movep.w D7,[DW3](A1)","movep.w D7,[DW3](A2)",
		"movep.w D7,[DW3](A3)","movep.w D7,[DW3](A4)","movep.w D7,[DW3](A5)","movep.w D7,[DW3](A6)",
		"movep.w D7,[DW3](A7)" };
	static const char *opATable003F[] = { "movep.l D7,[DW3](A0)","movep.l D7,[DW3](A1)","movep.l D7,[DW3](A2)",
		"movep.l D7,[DW3](A3)","movep.l D7,[DW3](A4)","movep.l D7,[DW3](A5)","movep.l D7,[DW3](A6)",
		"movep.l D7,[DW3](A7)" };
	static const char *opATable0121[] = { "swap D0","swap D1","swap D2","swap D3","swap D4","swap D5",
		"swap D6","swap D7" };
	static const char *opATable0122[] = { "ext.w D0","ext.w D1","ext.w D2","ext.w D3","ext.w D4",
		"ext.w D5","ext.w D6","ext.w D7" };
	static const char *opATable0123[] = { "ext.l D0","ext.l D1","ext.l D2","ext.l D3","ext.l D4",
		"ext.l D5","ext.l D6","ext.l D7" };
	static const char *opATable0139[] = { "trap #0","trap #1","trap #2","trap #3","trap #4","trap #5",
		"trap #6","trap #7","trap #8","trap #9","trap #10","trap #11","trap #12","trap #13",
		"trap #14","trap #15","link A0,#[DW3]","link A1,#[DW3]","link A2,#[DW3]","link A3,#[DW3]",
		"link A4,#[DW3]","link A5,#[DW3]","link A6,#[DW3]","link A7,#[DW3]","unlk A0","unlk A1",
		"unlk A2","unlk A3","unlk A4","unlk A5","unlk A6","unlk A7","move A0,USP","move A1,USP",
		"move A2,USP","move A3,USP","move A4,USP","move A5,USP","move A6,USP","move A7,USP",
		"move USP,A0","move USP,A1","move USP,A2","move USP,A3","move USP,A4","move USP,A5",
		"move USP,A6","move USP,A7","reset","nop","stop #[DW3]","rte",NULL,"rts","trapv","rtr" };
	static const char *opATable0143[] = { "dbt D0,[LL3]","dbt D1,[LL3]","dbt D2,[LL3]","dbt D3,[LL3]",
		"dbt D4,[LL3]","dbt D5,[LL3]","dbt D6,[LL3]","dbt D7,[LL3]" };
	static const char *opATable0147[] = { "dbf D0,[LL3]","dbf D1,[LL3]","dbf D2,[LL3]","dbf D3,[LL3]",
		"dbf D4,[LL3]","dbf D5,[LL3]","dbf D6,[LL3]","dbf D7,[LL3]" };
	static const char *opATable014B[] = { "dbhi D0,[LL3]","dbhi D1,[LL3]","dbhi D2,[LL3]","dbhi D3,[LL3]",
		"dbhi D4,[LL3]","dbhi D5,[LL3]","dbhi D6,[LL3]","dbhi D7,[LL3]" };
	static const char *opATable014F[] = { "dbls D0,[LL3]","dbls D1,[LL3]","dbls D2,[LL3]","dbls D3,[LL3]",
		"dbls D4,[LL3]","dbls D5,[LL3]","dbls D6,[LL3]","dbls D7,[LL3]" };
	static const char *opATable0153[] = { "dbcc D0,[LL3]","dbcc D1,[LL3]","dbcc D2,[LL3]","dbcc D3,[LL3]",
		"dbcc D4,[LL3]","dbcc D5,[LL3]","dbcc D6,[LL3]","dbcc D7,[LL3]" };
	static const char *opATable0157[] = { "dbcs D0,[LL3]","dbcs D1,[LL3]","dbcs D2,[LL3]","dbcs D3,[LL3]",
		"dbcs D4,[LL3]","dbcs D5,[LL3]","dbcs D6,[LL3]","dbcs D7,[LL3]" };
	static const char *opATable015B[] = { "dbne D0,[LL3]","dbne D1,[LL3]","dbne D2,[LL3]","dbne D3,[LL3]",
		"dbne D4,[LL3]","dbne D5,[LL3]","dbne D6,[LL3]","dbne D7,[LL3]" };
	static const char *opATable015F[] = { "dbeq D0,[LL3]","dbeq D1,[LL3]","dbeq D2,[LL3]","dbeq D3,[LL3]",
		"dbeq D4,[LL3]","dbeq D5,[LL3]","dbeq D6,[LL3]","dbeq D7,[LL3]" };
	static const char *opATable0163[] = { "dbvc D0,[LL3]","dbvc D1,[LL3]","dbvc D2,[LL3]","dbvc D3,[LL3]",
		"dbvc D4,[LL3]","dbvc D5,[LL3]","dbvc D6,[LL3]","dbvc D7,[LL3]" };
	static const char *opATable0167[] = { "dbvs D0,[LL3]","dbvs D1,[LL3]","dbvs D2,[LL3]","dbvs D3,[LL3]",
		"dbvs D4,[LL3]","dbvs D5,[LL3]","dbvs D6,[LL3]","dbvs D7,[LL3]" };
	static const char *opATable016B[] = { "dbpl D0,[LL3]","dbpl D1,[LL3]","dbpl D2,[LL3]","dbpl D3,[LL3]",
		"dbpl D4,[LL3]","dbpl D5,[LL3]","dbpl D6,[LL3]","dbpl D7,[LL3]" };
	static const char *opATable016F[] = { "dbmi D0,[LL3]","dbmi D1,[LL3]","dbmi D2,[LL3]","dbmi D3,[LL3]",
		"dbmi D4,[LL3]","dbmi D5,[LL3]","dbmi D6,[LL3]","dbmi D7,[LL3]" };
	static const char *opATable0173[] = { "dbge D0,[LL3]","dbge D1,[LL3]","dbge D2,[LL3]","dbge D3,[LL3]",
		"dbge D4,[LL3]","dbge D5,[LL3]","dbge D6,[LL3]","dbge D7,[LL3]" };
	static const char *opATable0177[] = { "dblt D0,[LL3]","dblt D1,[LL3]","dblt D2,[LL3]","dblt D3,[LL3]",
		"dblt D4,[LL3]","dblt D5,[LL3]","dblt D6,[LL3]","dblt D7,[LL3]" };
	static const char *opATable017B[] = { "dbgt D0,[LL3]","dbgt D1,[LL3]","dbgt D2,[LL3]","dbgt D3,[LL3]",
		"dbgt D4,[LL3]","dbgt D5,[LL3]","dbgt D6,[LL3]","dbgt D7,[LL3]" };
	static const char *opATable017F[] = { "dble D0,[LL3]","dble D1,[LL3]","dble D2,[LL3]","dble D3,[LL3]",
		"dble D4,[LL3]","dble D5,[LL3]","dble D6,[LL3]","dble D7,[LL3]" };
	static const char *opATable0204[] = { "sbcd D0,D0","sbcd D1,D0","sbcd D2,D0","sbcd D3,D0","sbcd D4,D0",
		"sbcd D5,D0","sbcd D6,D0","sbcd D7,D0","sbcd -(A0),-(A0)","sbcd -(A1),-(A0)","sbcd -(A2),-(A0)",
		"sbcd -(A3),-(A0)","sbcd -(A4),-(A0)","sbcd -(A5),-(A0)","sbcd -(A6),-(A0)","sbcd -(A7),-(A0)" };
	static const char *opATable020C[] = { "sbcd D0,D1","sbcd D1,D1","sbcd D2,D1","sbcd D3,D1","sbcd D4,D1",
		"sbcd D5,D1","sbcd D6,D1","sbcd D7,D1","sbcd -(A0),-(A1)","sbcd -(A1),-(A1)","sbcd -(A2),-(A1)",
		"sbcd -(A3),-(A1)","sbcd -(A4),-(A1)","sbcd -(A5),-(A1)","sbcd -(A6),-(A1)","sbcd -(A7),-(A1)" };
	static const char *opATable0214[] = { "sbcd D0,D2","sbcd D1,D2","sbcd D2,D2","sbcd D3,D2","sbcd D4,D2",
		"sbcd D5,D2","sbcd D6,D2","sbcd D7,D2","sbcd -(A0),-(A2)","sbcd -(A1),-(A2)","sbcd -(A2),-(A2)",
		"sbcd -(A3),-(A2)","sbcd -(A4),-(A2)","sbcd -(A5),-(A2)","sbcd -(A6),-(A2)","sbcd -(A7),-(A2)" };
	static const char *opATable021C[] = { "sbcd D0,D3","sbcd D1,D3","sbcd D2,D3","sbcd D3,D3","sbcd D4,D3",
		"sbcd D5,D3","sbcd D6,D3","sbcd D7,D3","sbcd -(A0),-(A3)","sbcd -(A1),-(A3)","sbcd -(A2),-(A3)",
		"sbcd -(A3),-(A3)","sbcd -(A4),-(A3)","sbcd -(A5),-(A3)","sbcd -(A6),-(A3)","sbcd -(A7),-(A3)" };
	static const char *opATable0224[] = { "sbcd D0,D4","sbcd D1,D4","sbcd D2,D4","sbcd D3,D4","sbcd D4,D4",
		"sbcd D5,D4","sbcd D6,D4","sbcd D7,D4","sbcd -(A0),-(A4)","sbcd -(A1),-(A4)","sbcd -(A2),-(A4)",
		"sbcd -(A3),-(A4)","sbcd -(A4),-(A4)","sbcd -(A5),-(A4)","sbcd -(A6),-(A4)","sbcd -(A7),-(A4)" };
	static const char *opATable022C[] = { "sbcd D0,D5","sbcd D1,D5","sbcd D2,D5","sbcd D3,D5","sbcd D4,D5",
		"sbcd D5,D5","sbcd D6,D5","sbcd D7,D5","sbcd -(A0),-(A5)","sbcd -(A1),-(A5)","sbcd -(A2),-(A5)",
		"sbcd -(A3),-(A5)","sbcd -(A4),-(A5)","sbcd -(A5),-(A5)","sbcd -(A6),-(A5)","sbcd -(A7),-(A5)" };
	static const char *opATable0234[] = { "sbcd D0,D6","sbcd D1,D6","sbcd D2,D6","sbcd D3,D6","sbcd D4,D6",
		"sbcd D5,D6","sbcd D6,D6","sbcd D7,D6","sbcd -(A0),-(A6)","sbcd -(A1),-(A6)","sbcd -(A2),-(A6)",
		"sbcd -(A3),-(A6)","sbcd -(A4),-(A6)","sbcd -(A5),-(A6)","sbcd -(A6),-(A6)","sbcd -(A7),-(A6)" };
	static const char *opATable023C[] = { "sbcd D0,D7","sbcd D1,D7","sbcd D2,D7","sbcd D3,D7","sbcd D4,D7",
		"sbcd D5,D7","sbcd D6,D7","sbcd D7,D7","sbcd -(A0),-(A7)","sbcd -(A1),-(A7)","sbcd -(A2),-(A7)",
		"sbcd -(A3),-(A7)","sbcd -(A4),-(A7)","sbcd -(A5),-(A7)","sbcd -(A6),-(A7)","sbcd -(A7),-(A7)" };
	static const char *opATable0244[] = { "subx.b D0,D0","subx.b D1,D0","subx.b D2,D0","subx.b D3,D0",
		"subx.b D4,D0","subx.b D5,D0","subx.b D6,D0","subx.b D7,D0","subx.b -(A0),-(A0)","subx.b -(A1),-(A0)",
		"subx.b -(A2),-(A0)","subx.b -(A3),-(A0)","subx.b -(A4),-(A0)","subx.b -(A5),-(A0)",
		"subx.b -(A6),-(A0)","subx.b -(A7),-(A0)" };
	static const char *opATable0245[] = { "subx.w D0,D0","subx.w D1,D0","subx.w D2,D0","subx.w D3,D0",
		"subx.w D4,D0","subx.w D5,D0","subx.w D6,D0","subx.w D7,D0","subx.w -(A0),-(A0)","subx.w -(A1),-(A0)",
		"subx.w -(A2),-(A0)","subx.w -(A3),-(A0)","subx.w -(A4),-(A0)","subx.w -(A5),-(A0)",
		"subx.w -(A6),-(A0)","subx.w -(A7),-(A0)" };
	static const char *opATable0246[] = { "subx.l D0,D0","subx.l D1,D0","subx.l D2,D0","subx.l D3,D0",
		"subx.l D4,D0","subx.l D5,D0","subx.l D6,D0","subx.l D7,D0","subx.l -(A0),-(A0)","subx.l -(A1),-(A0)",
		"subx.l -(A2),-(A0)","subx.l -(A3),-(A0)","subx.l -(A4),-(A0)","subx.l -(A5),-(A0)",
		"subx.l -(A6),-(A0)","subx.l -(A7),-(A0)" };
	static const char *opATable024C[] = { "subx.b D0,D1","subx.b D1,D1","subx.b D2,D1","subx.b D3,D1",
		"subx.b D4,D1","subx.b D5,D1","subx.b D6,D1","subx.b D7,D1","subx.b -(A0),-(A1)","subx.b -(A1),-(A1)",
		"subx.b -(A2),-(A1)","subx.b -(A3),-(A1)","subx.b -(A4),-(A1)","subx.b -(A5),-(A1)",
		"subx.b -(A6),-(A1)","subx.b -(A7),-(A1)" };
	static const char *opATable024D[] = { "subx.w D0,D1","subx.w D1,D1","subx.w D2,D1","subx.w D3,D1",
		"subx.w D4,D1","subx.w D5,D1","subx.w D6,D1","subx.w D7,D1","subx.w -(A0),-(A1)","subx.w -(A1),-(A1)",
		"subx.w -(A2),-(A1)","subx.w -(A3),-(A1)","subx.w -(A4),-(A1)","subx.w -(A5),-(A1)",
		"subx.w -(A6),-(A1)","subx.w -(A7),-(A1)" };
	static const char *opATable024E[] = { "subx.l D0,D1","subx.l D1,D1","subx.l D2,D1","subx.l D3,D1",
		"subx.l D4,D1","subx.l D5,D1","subx.l D6,D1","subx.l D7,D1","subx.l -(A0),-(A1)","subx.l -(A1),-(A1)",
		"subx.l -(A2),-(A1)","subx.l -(A3),-(A1)","subx.l -(A4),-(A1)","subx.l -(A5),-(A1)",
		"subx.l -(A6),-(A1)","subx.l -(A7),-(A1)" };
	static const char *opATable0254[] = { "subx.b D0,D2","subx.b D1,D2","subx.b D2,D2","subx.b D3,D2",
		"subx.b D4,D2","subx.b D5,D2","subx.b D6,D2","subx.b D7,D2","subx.b -(A0),-(A2)","subx.b -(A1),-(A2)",
		"subx.b -(A2),-(A2)","subx.b -(A3),-(A2)","subx.b -(A4),-(A2)","subx.b -(A5),-(A2)",
		"subx.b -(A6),-(A2)","subx.b -(A7),-(A2)" };
	static const char *opATable0255[] = { "subx.w D0,D2","subx.w D1,D2","subx.w D2,D2","subx.w D3,D2",
		"subx.w D4,D2","subx.w D5,D2","subx.w D6,D2","subx.w D7,D2","subx.w -(A0),-(A2)","subx.w -(A1),-(A2)",
		"subx.w -(A2),-(A2)","subx.w -(A3),-(A2)","subx.w -(A4),-(A2)","subx.w -(A5),-(A2)",
		"subx.w -(A6),-(A2)","subx.w -(A7),-(A2)" };
	static const char *opATable0256[] = { "subx.l D0,D2","subx.l D1,D2","subx.l D2,D2","subx.l D3,D2",
		"subx.l D4,D2","subx.l D5,D2","subx.l D6,D2","subx.l D7,D2","subx.l -(A0),-(A2)","subx.l -(A1),-(A2)",
		"subx.l -(A2),-(A2)","subx.l -(A3),-(A2)","subx.l -(A4),-(A2)","subx.l -(A5),-(A2)",
		"subx.l -(A6),-(A2)","subx.l -(A7),-(A2)" };
	static const char *opATable025C[] = { "subx.b D0,D3","subx.b D1,D3","subx.b D2,D3","subx.b D3,D3",
		"subx.b D4,D3","subx.b D5,D3","subx.b D6,D3","subx.b D7,D3","subx.b -(A0),-(A3)","subx.b -(A1),-(A3)",
		"subx.b -(A2),-(A3)","subx.b -(A3),-(A3)","subx.b -(A4),-(A3)","subx.b -(A5),-(A3)",
		"subx.b -(A6),-(A3)","subx.b -(A7),-(A3)" };
	static const char *opATable025D[] = { "subx.w D0,D3","subx.w D1,D3","subx.w D2,D3","subx.w D3,D3",
		"subx.w D4,D3","subx.w D5,D3","subx.w D6,D3","subx.w D7,D3","subx.w -(A0),-(A3)","subx.w -(A1),-(A3)",
		"subx.w -(A2),-(A3)","subx.w -(A3),-(A3)","subx.w -(A4),-(A3)","subx.w -(A5),-(A3)",
		"subx.w -(A6),-(A3)","subx.w -(A7),-(A3)" };
	static const char *opATable025E[] = { "subx.l D0,D3","subx.l D1,D3","subx.l D2,D3","subx.l D3,D3",
		"subx.l D4,D3","subx.l D5,D3","subx.l D6,D3","subx.l D7,D3","subx.l -(A0),-(A3)","subx.l -(A1),-(A3)",
		"subx.l -(A2),-(A3)","subx.l -(A3),-(A3)","subx.l -(A4),-(A3)","subx.l -(A5),-(A3)",
		"subx.l -(A6),-(A3)","subx.l -(A7),-(A3)" };
	static const char *opATable0264[] = { "subx.b D0,D4","subx.b D1,D4","subx.b D2,D4","subx.b D3,D4",
		"subx.b D4,D4","subx.b D5,D4","subx.b D6,D4","subx.b D7,D4","subx.b -(A0),-(A4)","subx.b -(A1),-(A4)",
		"subx.b -(A2),-(A4)","subx.b -(A3),-(A4)","subx.b -(A4),-(A4)","subx.b -(A5),-(A4)",
		"subx.b -(A6),-(A4)","subx.b -(A7),-(A4)" };
	static const char *opATable0265[] = { "subx.w D0,D4","subx.w D1,D4","subx.w D2,D4","subx.w D3,D4",
		"subx.w D4,D4","subx.w D5,D4","subx.w D6,D4","subx.w D7,D4","subx.w -(A0),-(A4)","subx.w -(A1),-(A4)",
		"subx.w -(A2),-(A4)","subx.w -(A3),-(A4)","subx.w -(A4),-(A4)","subx.w -(A5),-(A4)",
		"subx.w -(A6),-(A4)","subx.w -(A7),-(A4)" };
	static const char *opATable0266[] = { "subx.l D0,D4","subx.l D1,D4","subx.l D2,D4","subx.l D3,D4",
		"subx.l D4,D4","subx.l D5,D4","subx.l D6,D4","subx.l D7,D4","subx.l -(A0),-(A4)","subx.l -(A1),-(A4)",
		"subx.l -(A2),-(A4)","subx.l -(A3),-(A4)","subx.l -(A4),-(A4)","subx.l -(A5),-(A4)",
		"subx.l -(A6),-(A4)","subx.l -(A7),-(A4)" };
	static const char *opATable026C[] = { "subx.b D0,D5","subx.b D1,D5","subx.b D2,D5","subx.b D3,D5",
		"subx.b D4,D5","subx.b D5,D5","subx.b D6,D5","subx.b D7,D5","subx.b -(A0),-(A5)","subx.b -(A1),-(A5)",
		"subx.b -(A2),-(A5)","subx.b -(A3),-(A5)","subx.b -(A4),-(A5)","subx.b -(A5),-(A5)",
		"subx.b -(A6),-(A5)","subx.b -(A7),-(A5)" };
	static const char *opATable026D[] = { "subx.w D0,D5","subx.w D1,D5","subx.w D2,D5","subx.w D3,D5",
		"subx.w D4,D5","subx.w D5,D5","subx.w D6,D5","subx.w D7,D5","subx.w -(A0),-(A5)","subx.w -(A1),-(A5)",
		"subx.w -(A2),-(A5)","subx.w -(A3),-(A5)","subx.w -(A4),-(A5)","subx.w -(A5),-(A5)",
		"subx.w -(A6),-(A5)","subx.w -(A7),-(A5)" };
	static const char *opATable026E[] = { "subx.l D0,D5","subx.l D1,D5","subx.l D2,D5","subx.l D3,D5",
		"subx.l D4,D5","subx.l D5,D5","subx.l D6,D5","subx.l D7,D5","subx.l -(A0),-(A5)","subx.l -(A1),-(A5)",
		"subx.l -(A2),-(A5)","subx.l -(A3),-(A5)","subx.l -(A4),-(A5)","subx.l -(A5),-(A5)",
		"subx.l -(A6),-(A5)","subx.l -(A7),-(A5)" };
	static const char *opATable0274[] = { "subx.b D0,D6","subx.b D1,D6","subx.b D2,D6","subx.b D3,D6",
		"subx.b D4,D6","subx.b D5,D6","subx.b D6,D6","subx.b D7,D6","subx.b -(A0),-(A6)","subx.b -(A1),-(A6)",
		"subx.b -(A2),-(A6)","subx.b -(A3),-(A6)","subx.b -(A4),-(A6)","subx.b -(A5),-(A6)",
		"subx.b -(A6),-(A6)","subx.b -(A7),-(A6)" };
	static const char *opATable0275[] = { "subx.w D0,D6","subx.w D1,D6","subx.w D2,D6","subx.w D3,D6",
		"subx.w D4,D6","subx.w D5,D6","subx.w D6,D6","subx.w D7,D6","subx.w -(A0),-(A6)","subx.w -(A1),-(A6)",
		"subx.w -(A2),-(A6)","subx.w -(A3),-(A6)","subx.w -(A4),-(A6)","subx.w -(A5),-(A6)",
		"subx.w -(A6),-(A6)","subx.w -(A7),-(A6)" };
	static const char *opATable0276[] = { "subx.l D0,D6","subx.l D1,D6","subx.l D2,D6","subx.l D3,D6",
		"subx.l D4,D6","subx.l D5,D6","subx.l D6,D6","subx.l D7,D6","subx.l -(A0),-(A6)","subx.l -(A1),-(A6)",
		"subx.l -(A2),-(A6)","subx.l -(A3),-(A6)","subx.l -(A4),-(A6)","subx.l -(A5),-(A6)",
		"subx.l -(A6),-(A6)","subx.l -(A7),-(A6)" };
	static const char *opATable027C[] = { "subx.b D0,D7","subx.b D1,D7","subx.b D2,D7","subx.b D3,D7",
		"subx.b D4,D7","subx.b D5,D7","subx.b D6,D7","subx.b D7,D7","subx.b -(A0),-(A7)","subx.b -(A1),-(A7)",
		"subx.b -(A2),-(A7)","subx.b -(A3),-(A7)","subx.b -(A4),-(A7)","subx.b -(A5),-(A7)",
		"subx.b -(A6),-(A7)","subx.b -(A7),-(A7)" };
	static const char *opATable027D[] = { "subx.w D0,D7","subx.w D1,D7","subx.w D2,D7","subx.w D3,D7",
		"subx.w D4,D7","subx.w D5,D7","subx.w D6,D7","subx.w D7,D7","subx.w -(A0),-(A7)","subx.w -(A1),-(A7)",
		"subx.w -(A2),-(A7)","subx.w -(A3),-(A7)","subx.w -(A4),-(A7)","subx.w -(A5),-(A7)",
		"subx.w -(A6),-(A7)","subx.w -(A7),-(A7)" };
	static const char *opATable027E[] = { "subx.l D0,D7","subx.l D1,D7","subx.l D2,D7","subx.l D3,D7",
		"subx.l D4,D7","subx.l D5,D7","subx.l D6,D7","subx.l D7,D7","subx.l -(A0),-(A7)","subx.l -(A1),-(A7)",
		"subx.l -(A2),-(A7)","subx.l -(A3),-(A7)","subx.l -(A4),-(A7)","subx.l -(A5),-(A7)",
		"subx.l -(A6),-(A7)","subx.l -(A7),-(A7)" };
	static const char *opATable02C4[] = { "cmpm.b D0,D0","cmpm.b D1,D0","cmpm.b D2,D0","cmpm.b D3,D0",
		"cmpm.b D4,D0","cmpm.b D5,D0","cmpm.b D6,D0","cmpm.b D7,D0","cmpm.b (A0)+,(A0)+","cmpm.b (A1)+,(A0)+",
		"cmpm.b (A2)+,(A0)+","cmpm.b (A3)+,(A0)+","cmpm.b (A4)+,(A0)+","cmpm.b (A5)+,(A0)+",
		"cmpm.b (A6)+,(A0)+","cmpm.b (A7)+,(A0)+" };
	static const char *opATable02C5[] = { "cmpm.w D0,D0","cmpm.w D1,D0","cmpm.w D2,D0","cmpm.w D3,D0",
		"cmpm.w D4,D0","cmpm.w D5,D0","cmpm.w D6,D0","cmpm.w D7,D0","cmpm.w (A0)+,(A0)+","cmpm.w (A1)+,(A0)+",
		"cmpm.w (A2)+,(A0)+","cmpm.w (A3)+,(A0)+","cmpm.w (A4)+,(A0)+","cmpm.w (A5)+,(A0)+",
		"cmpm.w (A6)+,(A0)+","cmpm.w (A7)+,(A0)+" };
	static const char *opATable02C6[] = { "cmpm.l D0,D0","cmpm.l D1,D0","cmpm.l D2,D0","cmpm.l D3,D0",
		"cmpm.l D4,D0","cmpm.l D5,D0","cmpm.l D6,D0","cmpm.l D7,D0","cmpm.l (A0)+,(A0)+","cmpm.l (A1)+,(A0)+",
		"cmpm.l (A2)+,(A0)+","cmpm.l (A3)+,(A0)+","cmpm.l (A4)+,(A0)+","cmpm.l (A5)+,(A0)+",
		"cmpm.l (A6)+,(A0)+","cmpm.l (A7)+,(A0)+" };
	static const char *opATable02CC[] = { "cmpm.b D0,D1","cmpm.b D1,D1","cmpm.b D2,D1","cmpm.b D3,D1",
		"cmpm.b D4,D1","cmpm.b D5,D1","cmpm.b D6,D1","cmpm.b D7,D1","cmpm.b (A0)+,(A1)+","cmpm.b (A1)+,(A1)+",
		"cmpm.b (A2)+,(A1)+","cmpm.b (A3)+,(A1)+","cmpm.b (A4)+,(A1)+","cmpm.b (A5)+,(A1)+",
		"cmpm.b (A6)+,(A1)+","cmpm.b (A7)+,(A1)+" };
	static const char *opATable02CD[] = { "cmpm.w D0,D1","cmpm.w D1,D1","cmpm.w D2,D1","cmpm.w D3,D1",
		"cmpm.w D4,D1","cmpm.w D5,D1","cmpm.w D6,D1","cmpm.w D7,D1","cmpm.w (A0)+,(A1)+","cmpm.w (A1)+,(A1)+",
		"cmpm.w (A2)+,(A1)+","cmpm.w (A3)+,(A1)+","cmpm.w (A4)+,(A1)+","cmpm.w (A5)+,(A1)+",
		"cmpm.w (A6)+,(A1)+","cmpm.w (A7)+,(A1)+" };
	static const char *opATable02CE[] = { "cmpm.l D0,D1","cmpm.l D1,D1","cmpm.l D2,D1","cmpm.l D3,D1",
		"cmpm.l D4,D1","cmpm.l D5,D1","cmpm.l D6,D1","cmpm.l D7,D1","cmpm.l (A0)+,(A1)+","cmpm.l (A1)+,(A1)+",
		"cmpm.l (A2)+,(A1)+","cmpm.l (A3)+,(A1)+","cmpm.l (A4)+,(A1)+","cmpm.l (A5)+,(A1)+",
		"cmpm.l (A6)+,(A1)+","cmpm.l (A7)+,(A1)+" };
	static const char *opATable02D4[] = { "cmpm.b D0,D2","cmpm.b D1,D2","cmpm.b D2,D2","cmpm.b D3,D2",
		"cmpm.b D4,D2","cmpm.b D5,D2","cmpm.b D6,D2","cmpm.b D7,D2","cmpm.b (A0)+,(A2)+","cmpm.b (A1)+,(A2)+",
		"cmpm.b (A2)+,(A2)+","cmpm.b (A3)+,(A2)+","cmpm.b (A4)+,(A2)+","cmpm.b (A5)+,(A2)+",
		"cmpm.b (A6)+,(A2)+","cmpm.b (A7)+,(A2)+" };
	static const char *opATable02D5[] = { "cmpm.w D0,D2","cmpm.w D1,D2","cmpm.w D2,D2","cmpm.w D3,D2",
		"cmpm.w D4,D2","cmpm.w D5,D2","cmpm.w D6,D2","cmpm.w D7,D2","cmpm.w (A0)+,(A2)+","cmpm.w (A1)+,(A2)+",
		"cmpm.w (A2)+,(A2)+","cmpm.w (A3)+,(A2)+","cmpm.w (A4)+,(A2)+","cmpm.w (A5)+,(A2)+",
		"cmpm.w (A6)+,(A2)+","cmpm.w (A7)+,(A2)+" };
	static const char *opATable02D6[] = { "cmpm.l D0,D2","cmpm.l D1,D2","cmpm.l D2,D2","cmpm.l D3,D2",
		"cmpm.l D4,D2","cmpm.l D5,D2","cmpm.l D6,D2","cmpm.l D7,D2","cmpm.l (A0)+,(A2)+","cmpm.l (A1)+,(A2)+",
		"cmpm.l (A2)+,(A2)+","cmpm.l (A3)+,(A2)+","cmpm.l (A4)+,(A2)+","cmpm.l (A5)+,(A2)+",
		"cmpm.l (A6)+,(A2)+","cmpm.l (A7)+,(A2)+" };
	static const char *opATable02DC[] = { "cmpm.b D0,D3","cmpm.b D1,D3","cmpm.b D2,D3","cmpm.b D3,D3",
		"cmpm.b D4,D3","cmpm.b D5,D3","cmpm.b D6,D3","cmpm.b D7,D3","cmpm.b (A0)+,(A3)+","cmpm.b (A1)+,(A3)+",
		"cmpm.b (A2)+,(A3)+","cmpm.b (A3)+,(A3)+","cmpm.b (A4)+,(A3)+","cmpm.b (A5)+,(A3)+",
		"cmpm.b (A6)+,(A3)+","cmpm.b (A7)+,(A3)+" };
	static const char *opATable02DD[] = { "cmpm.w D0,D3","cmpm.w D1,D3","cmpm.w D2,D3","cmpm.w D3,D3",
		"cmpm.w D4,D3","cmpm.w D5,D3","cmpm.w D6,D3","cmpm.w D7,D3","cmpm.w (A0)+,(A3)+","cmpm.w (A1)+,(A3)+",
		"cmpm.w (A2)+,(A3)+","cmpm.w (A3)+,(A3)+","cmpm.w (A4)+,(A3)+","cmpm.w (A5)+,(A3)+",
		"cmpm.w (A6)+,(A3)+","cmpm.w (A7)+,(A3)+" };
	static const char *opATable02DE[] = { "cmpm.l D0,D3","cmpm.l D1,D3","cmpm.l D2,D3","cmpm.l D3,D3",
		"cmpm.l D4,D3","cmpm.l D5,D3","cmpm.l D6,D3","cmpm.l D7,D3","cmpm.l (A0)+,(A3)+","cmpm.l (A1)+,(A3)+",
		"cmpm.l (A2)+,(A3)+","cmpm.l (A3)+,(A3)+","cmpm.l (A4)+,(A3)+","cmpm.l (A5)+,(A3)+",
		"cmpm.l (A6)+,(A3)+","cmpm.l (A7)+,(A3)+" };
	static const char *opATable02E4[] = { "cmpm.b D0,D4","cmpm.b D1,D4","cmpm.b D2,D4","cmpm.b D3,D4",
		"cmpm.b D4,D4","cmpm.b D5,D4","cmpm.b D6,D4","cmpm.b D7,D4","cmpm.b (A0)+,(A4)+","cmpm.b (A1)+,(A4)+",
		"cmpm.b (A2)+,(A4)+","cmpm.b (A3)+,(A4)+","cmpm.b (A4)+,(A4)+","cmpm.b (A5)+,(A4)+",
		"cmpm.b (A6)+,(A4)+","cmpm.b (A7)+,(A4)+" };
	static const char *opATable02E5[] = { "cmpm.w D0,D4","cmpm.w D1,D4","cmpm.w D2,D4","cmpm.w D3,D4",
		"cmpm.w D4,D4","cmpm.w D5,D4","cmpm.w D6,D4","cmpm.w D7,D4","cmpm.w (A0)+,(A4)+","cmpm.w (A1)+,(A4)+",
		"cmpm.w (A2)+,(A4)+","cmpm.w (A3)+,(A4)+","cmpm.w (A4)+,(A4)+","cmpm.w (A5)+,(A4)+",
		"cmpm.w (A6)+,(A4)+","cmpm.w (A7)+,(A4)+" };
	static const char *opATable02E6[] = { "cmpm.l D0,D4","cmpm.l D1,D4","cmpm.l D2,D4","cmpm.l D3,D4",
		"cmpm.l D4,D4","cmpm.l D5,D4","cmpm.l D6,D4","cmpm.l D7,D4","cmpm.l (A0)+,(A4)+","cmpm.l (A1)+,(A4)+",
		"cmpm.l (A2)+,(A4)+","cmpm.l (A3)+,(A4)+","cmpm.l (A4)+,(A4)+","cmpm.l (A5)+,(A4)+",
		"cmpm.l (A6)+,(A4)+","cmpm.l (A7)+,(A4)+" };
	static const char *opATable02EC[] = { "cmpm.b D0,D5","cmpm.b D1,D5","cmpm.b D2,D5","cmpm.b D3,D5",
		"cmpm.b D4,D5","cmpm.b D5,D5","cmpm.b D6,D5","cmpm.b D7,D5","cmpm.b (A0)+,(A5)+","cmpm.b (A1)+,(A5)+",
		"cmpm.b (A2)+,(A5)+","cmpm.b (A3)+,(A5)+","cmpm.b (A4)+,(A5)+","cmpm.b (A5)+,(A5)+",
		"cmpm.b (A6)+,(A5)+","cmpm.b (A7)+,(A5)+" };
	static const char *opATable02ED[] = { "cmpm.w D0,D5","cmpm.w D1,D5","cmpm.w D2,D5","cmpm.w D3,D5",
		"cmpm.w D4,D5","cmpm.w D5,D5","cmpm.w D6,D5","cmpm.w D7,D5","cmpm.w (A0)+,(A5)+","cmpm.w (A1)+,(A5)+",
		"cmpm.w (A2)+,(A5)+","cmpm.w (A3)+,(A5)+","cmpm.w (A4)+,(A5)+","cmpm.w (A5)+,(A5)+",
		"cmpm.w (A6)+,(A5)+","cmpm.w (A7)+,(A5)+" };
	static const char *opATable02EE[] = { "cmpm.l D0,D5","cmpm.l D1,D5","cmpm.l D2,D5","cmpm.l D3,D5",
		"cmpm.l D4,D5","cmpm.l D5,D5","cmpm.l D6,D5","cmpm.l D7,D5","cmpm.l (A0)+,(A5)+","cmpm.l (A1)+,(A5)+",
		"cmpm.l (A2)+,(A5)+","cmpm.l (A3)+,(A5)+","cmpm.l (A4)+,(A5)+","cmpm.l (A5)+,(A5)+",
		"cmpm.l (A6)+,(A5)+","cmpm.l (A7)+,(A5)+" };
	static const char *opATable02F4[] = { "cmpm.b D0,D6","cmpm.b D1,D6","cmpm.b D2,D6","cmpm.b D3,D6",
		"cmpm.b D4,D6","cmpm.b D5,D6","cmpm.b D6,D6","cmpm.b D7,D6","cmpm.b (A0)+,(A6)+","cmpm.b (A1)+,(A6)+",
		"cmpm.b (A2)+,(A6)+","cmpm.b (A3)+,(A6)+","cmpm.b (A4)+,(A6)+","cmpm.b (A5)+,(A6)+",
		"cmpm.b (A6)+,(A6)+","cmpm.b (A7)+,(A6)+" };
	static const char *opATable02F5[] = { "cmpm.w D0,D6","cmpm.w D1,D6","cmpm.w D2,D6","cmpm.w D3,D6",
		"cmpm.w D4,D6","cmpm.w D5,D6","cmpm.w D6,D6","cmpm.w D7,D6","cmpm.w (A0)+,(A6)+","cmpm.w (A1)+,(A6)+",
		"cmpm.w (A2)+,(A6)+","cmpm.w (A3)+,(A6)+","cmpm.w (A4)+,(A6)+","cmpm.w (A5)+,(A6)+",
		"cmpm.w (A6)+,(A6)+","cmpm.w (A7)+,(A6)+" };
	static const char *opATable02F6[] = { "cmpm.l D0,D6","cmpm.l D1,D6","cmpm.l D2,D6","cmpm.l D3,D6",
		"cmpm.l D4,D6","cmpm.l D5,D6","cmpm.l D6,D6","cmpm.l D7,D6","cmpm.l (A0)+,(A6)+","cmpm.l (A1)+,(A6)+",
		"cmpm.l (A2)+,(A6)+","cmpm.l (A3)+,(A6)+","cmpm.l (A4)+,(A6)+","cmpm.l (A5)+,(A6)+",
		"cmpm.l (A6)+,(A6)+","cmpm.l (A7)+,(A6)+" };
	static const char *opATable02FC[] = { "cmpm.b D0,D7","cmpm.b D1,D7","cmpm.b D2,D7","cmpm.b D3,D7",
		"cmpm.b D4,D7","cmpm.b D5,D7","cmpm.b D6,D7","cmpm.b D7,D7","cmpm.b (A0)+,(A7)+","cmpm.b (A1)+,(A7)+",
		"cmpm.b (A2)+,(A7)+","cmpm.b (A3)+,(A7)+","cmpm.b (A4)+,(A7)+","cmpm.b (A5)+,(A7)+",
		"cmpm.b (A6)+,(A7)+","cmpm.b (A7)+,(A7)+" };
	static const char *opATable02FD[] = { "cmpm.w D0,D7","cmpm.w D1,D7","cmpm.w D2,D7","cmpm.w D3,D7",
		"cmpm.w D4,D7","cmpm.w D5,D7","cmpm.w D6,D7","cmpm.w D7,D7","cmpm.w (A0)+,(A7)+","cmpm.w (A1)+,(A7)+",
		"cmpm.w (A2)+,(A7)+","cmpm.w (A3)+,(A7)+","cmpm.w (A4)+,(A7)+","cmpm.w (A5)+,(A7)+",
		"cmpm.w (A6)+,(A7)+","cmpm.w (A7)+,(A7)+" };
	static const char *opATable02FE[] = { "cmpm.l D0,D7","cmpm.l D1,D7","cmpm.l D2,D7","cmpm.l D3,D7",
		"cmpm.l D4,D7","cmpm.l D5,D7","cmpm.l D6,D7","cmpm.l D7,D7","cmpm.l (A0)+,(A7)+","cmpm.l (A1)+,(A7)+",
		"cmpm.l (A2)+,(A7)+","cmpm.l (A3)+,(A7)+","cmpm.l (A4)+,(A7)+","cmpm.l (A5)+,(A7)+",
		"cmpm.l (A6)+,(A7)+","cmpm.l (A7)+,(A7)+" };
	static const char *opATable0304[] = { "abcd D0,D0","abcd D1,D0","abcd D2,D0","abcd D3,D0","abcd D4,D0",
		"abcd D5,D0","abcd D6,D0","abcd D7,D0","abcd -(A0),-(A0)","abcd -(A1),-(A0)","abcd -(A2),-(A0)",
		"abcd -(A3),-(A0)","abcd -(A4),-(A0)","abcd -(A5),-(A0)","abcd -(A6),-(A0)","abcd -(A7),-(A0)" };
	static const char *opATable0305[] = { "exg D0,D0","exg D1,D0","exg D2,D0","exg D3,D0","exg D4,D0",
		"exg D5,D0","exg D6,D0","exg D7,D0","exg A0,A0","exg A1,A0","exg A2,A0","exg A3,A0",
		"exg A4,A0","exg A5,A0","exg A6,A0","exg A7,A0" };
	static const char *opATable0306[] = { "exg A0,D0","exg A1,D0","exg A2,D0","exg A3,D0","exg A4,D0",
		"exg A5,D0","exg A6,D0","exg A7,D0" };
	static const char *opATable030C[] = { "abcd D0,D1","abcd D1,D1","abcd D2,D1","abcd D3,D1","abcd D4,D1",
		"abcd D5,D1","abcd D6,D1","abcd D7,D1","abcd -(A0),-(A1)","abcd -(A1),-(A1)","abcd -(A2),-(A1)",
		"abcd -(A3),-(A1)","abcd -(A4),-(A1)","abcd -(A5),-(A1)","abcd -(A6),-(A1)","abcd -(A7),-(A1)" };
	static const char *opATable030D[] = { "exg D0,D1","exg D1,D1","exg D2,D1","exg D3,D1","exg D4,D1",
		"exg D5,D1","exg D6,D1","exg D7,D1","exg A0,A1","exg A1,A1","exg A2,A1","exg A3,A1",
		"exg A4,A1","exg A5,A1","exg A6,A1","exg A7,A1" };
	static const char *opATable030E[] = { "exg A0,D1","exg A1,D1","exg A2,D1","exg A3,D1","exg A4,D1",
		"exg A5,D1","exg A6,D1","exg A7,D1" };
	static const char *opATable0314[] = { "abcd D0,D2","abcd D1,D2","abcd D2,D2","abcd D3,D2","abcd D4,D2",
		"abcd D5,D2","abcd D6,D2","abcd D7,D2","abcd -(A0),-(A2)","abcd -(A1),-(A2)","abcd -(A2),-(A2)",
		"abcd -(A3),-(A2)","abcd -(A4),-(A2)","abcd -(A5),-(A2)","abcd -(A6),-(A2)","abcd -(A7),-(A2)" };
	static const char *opATable0315[] = { "exg D0,D2","exg D1,D2","exg D2,D2","exg D3,D2","exg D4,D2",
		"exg D5,D2","exg D6,D2","exg D7,D2","exg A0,A2","exg A1,A2","exg A2,A2","exg A3,A2",
		"exg A4,A2","exg A5,A2","exg A6,A2","exg A7,A2" };
	static const char *opATable0316[] = { "exg A0,D2","exg A1,D2","exg A2,D2","exg A3,D2","exg A4,D2",
		"exg A5,D2","exg A6,D2","exg A7,D2" };
	static const char *opATable031C[] = { "abcd D0,D3","abcd D1,D3","abcd D2,D3","abcd D3,D3","abcd D4,D3",
		"abcd D5,D3","abcd D6,D3","abcd D7,D3","abcd -(A0),-(A3)","abcd -(A1),-(A3)","abcd -(A2),-(A3)",
		"abcd -(A3),-(A3)","abcd -(A4),-(A3)","abcd -(A5),-(A3)","abcd -(A6),-(A3)","abcd -(A7),-(A3)" };
	static const char *opATable031D[] = { "exg D0,D3","exg D1,D3","exg D2,D3","exg D3,D3","exg D4,D3",
		"exg D5,D3","exg D6,D3","exg D7,D3","exg A0,A3","exg A1,A3","exg A2,A3","exg A3,A3",
		"exg A4,A3","exg A5,A3","exg A6,A3","exg A7,A3" };
	static const char *opATable031E[] = { "exg A0,D3","exg A1,D3","exg A2,D3","exg A3,D3","exg A4,D3",
		"exg A5,D3","exg A6,D3","exg A7,D3" };
	static const char *opATable0324[] = { "abcd D0,D4","abcd D1,D4","abcd D2,D4","abcd D3,D4","abcd D4,D4",
		"abcd D5,D4","abcd D6,D4","abcd D7,D4","abcd -(A0),-(A4)","abcd -(A1),-(A4)","abcd -(A2),-(A4)",
		"abcd -(A3),-(A4)","abcd -(A4),-(A4)","abcd -(A5),-(A4)","abcd -(A6),-(A4)","abcd -(A7),-(A4)" };
	static const char *opATable0325[] = { "exg D0,D4","exg D1,D4","exg D2,D4","exg D3,D4","exg D4,D4",
		"exg D5,D4","exg D6,D4","exg D7,D4","exg A0,A4","exg A1,A4","exg A2,A4","exg A3,A4",
		"exg A4,A4","exg A5,A4","exg A6,A4","exg A7,A4" };
	static const char *opATable0326[] = { "exg A0,D4","exg A1,D4","exg A2,D4","exg A3,D4","exg A4,D4",
		"exg A5,D4","exg A6,D4","exg A7,D4" };
	static const char *opATable032C[] = { "abcd D0,D5","abcd D1,D5","abcd D2,D5","abcd D3,D5","abcd D4,D5",
		"abcd D5,D5","abcd D6,D5","abcd D7,D5","abcd -(A0),-(A5)","abcd -(A1),-(A5)","abcd -(A2),-(A5)",
		"abcd -(A3),-(A5)","abcd -(A4),-(A5)","abcd -(A5),-(A5)","abcd -(A6),-(A5)","abcd -(A7),-(A5)" };
	static const char *opATable032D[] = { "exg D0,D5","exg D1,D5","exg D2,D5","exg D3,D5","exg D4,D5",
		"exg D5,D5","exg D6,D5","exg D7,D5","exg A0,A5","exg A1,A5","exg A2,A5","exg A3,A5",
		"exg A4,A5","exg A5,A5","exg A6,A5","exg A7,A5" };
	static const char *opATable032E[] = { "exg A0,D5","exg A1,D5","exg A2,D5","exg A3,D5","exg A4,D5",
		"exg A5,D5","exg A6,D5","exg A7,D5" };
	static const char *opATable0334[] = { "abcd D0,D6","abcd D1,D6","abcd D2,D6","abcd D3,D6","abcd D4,D6",
		"abcd D5,D6","abcd D6,D6","abcd D7,D6","abcd -(A0),-(A6)","abcd -(A1),-(A6)","abcd -(A2),-(A6)",
		"abcd -(A3),-(A6)","abcd -(A4),-(A6)","abcd -(A5),-(A6)","abcd -(A6),-(A6)","abcd -(A7),-(A6)" };
	static const char *opATable0335[] = { "exg D0,D6","exg D1,D6","exg D2,D6","exg D3,D6","exg D4,D6",
		"exg D5,D6","exg D6,D6","exg D7,D6","exg A0,A6","exg A1,A6","exg A2,A6","exg A3,A6",
		"exg A4,A6","exg A5,A6","exg A6,A6","exg A7,A6" };
	static const char *opATable0336[] = { "exg A0,D6","exg A1,D6","exg A2,D6","exg A3,D6","exg A4,D6",
		"exg A5,D6","exg A6,D6","exg A7,D6" };
	static const char *opATable033C[] = { "abcd D0,D7","abcd D1,D7","abcd D2,D7","abcd D3,D7","abcd D4,D7",
		"abcd D5,D7","abcd D6,D7","abcd D7,D7","abcd -(A0),-(A7)","abcd -(A1),-(A7)","abcd -(A2),-(A7)",
		"abcd -(A3),-(A7)","abcd -(A4),-(A7)","abcd -(A5),-(A7)","abcd -(A6),-(A7)","abcd -(A7),-(A7)" };
	static const char *opATable033D[] = { "exg D0,D7","exg D1,D7","exg D2,D7","exg D3,D7","exg D4,D7",
		"exg D5,D7","exg D6,D7","exg D7,D7","exg A0,A7","exg A1,A7","exg A2,A7","exg A3,A7",
		"exg A4,A7","exg A5,A7","exg A6,A7","exg A7,A7" };
	static const char *opATable033E[] = { "exg A0,D7","exg A1,D7","exg A2,D7","exg A3,D7","exg A4,D7",
		"exg A5,D7","exg A6,D7","exg A7,D7" };
	static const char *opATable0344[] = { "addx.b D0,D0","addx.b D1,D0","addx.b D2,D0","addx.b D3,D0",
		"addx.b D4,D0","addx.b D5,D0","addx.b D6,D0","addx.b D7,D0","addx.b -(A0),-(A0)","addx.b -(A1),-(A0)",
		"addx.b -(A2),-(A0)","addx.b -(A3),-(A0)","addx.b -(A4),-(A0)","addx.b -(A5),-(A0)",
		"addx.b -(A6),-(A0)","addx.b -(A7),-(A0)" };
	static const char *opATable0345[] = { "addx.w D0,D0","addx.w D1,D0","addx.w D2,D0","addx.w D3,D0",
		"addx.w D4,D0","addx.w D5,D0","addx.w D6,D0","addx.w D7,D0","addx.w -(A0),-(A0)","addx.w -(A1),-(A0)",
		"addx.w -(A2),-(A0)","addx.w -(A3),-(A0)","addx.w -(A4),-(A0)","addx.w -(A5),-(A0)",
		"addx.w -(A6),-(A0)","addx.w -(A7),-(A0)" };
	static const char *opATable0346[] = { "addx.l D0,D0","addx.l D1,D0","addx.l D2,D0","addx.l D3,D0",
		"addx.l D4,D0","addx.l D5,D0","addx.l D6,D0","addx.l D7,D0","addx.l -(A0),-(A0)","addx.l -(A1),-(A0)",
		"addx.l -(A2),-(A0)","addx.l -(A3),-(A0)","addx.l -(A4),-(A0)","addx.l -(A5),-(A0)",
		"addx.l -(A6),-(A0)","addx.l -(A7),-(A0)" };
	static const char *opATable034C[] = { "addx.b D0,D1","addx.b D1,D1","addx.b D2,D1","addx.b D3,D1",
		"addx.b D4,D1","addx.b D5,D1","addx.b D6,D1","addx.b D7,D1","addx.b -(A0),-(A1)","addx.b -(A1),-(A1)",
		"addx.b -(A2),-(A1)","addx.b -(A3),-(A1)","addx.b -(A4),-(A1)","addx.b -(A5),-(A1)",
		"addx.b -(A6),-(A1)","addx.b -(A7),-(A1)" };
	static const char *opATable034D[] = { "addx.w D0,D1","addx.w D1,D1","addx.w D2,D1","addx.w D3,D1",
		"addx.w D4,D1","addx.w D5,D1","addx.w D6,D1","addx.w D7,D1","addx.w -(A0),-(A1)","addx.w -(A1),-(A1)",
		"addx.w -(A2),-(A1)","addx.w -(A3),-(A1)","addx.w -(A4),-(A1)","addx.w -(A5),-(A1)",
		"addx.w -(A6),-(A1)","addx.w -(A7),-(A1)" };
	static const char *opATable034E[] = { "addx.l D0,D1","addx.l D1,D1","addx.l D2,D1","addx.l D3,D1",
		"addx.l D4,D1","addx.l D5,D1","addx.l D6,D1","addx.l D7,D1","addx.l -(A0),-(A1)","addx.l -(A1),-(A1)",
		"addx.l -(A2),-(A1)","addx.l -(A3),-(A1)","addx.l -(A4),-(A1)","addx.l -(A5),-(A1)",
		"addx.l -(A6),-(A1)","addx.l -(A7),-(A1)" };
	static const char *opATable0354[] = { "addx.b D0,D2","addx.b D1,D2","addx.b D2,D2","addx.b D3,D2",
		"addx.b D4,D2","addx.b D5,D2","addx.b D6,D2","addx.b D7,D2","addx.b -(A0),-(A2)","addx.b -(A1),-(A2)",
		"addx.b -(A2),-(A2)","addx.b -(A3),-(A2)","addx.b -(A4),-(A2)","addx.b -(A5),-(A2)",
		"addx.b -(A6),-(A2)","addx.b -(A7),-(A2)" };
	static const char *opATable0355[] = { "addx.w D0,D2","addx.w D1,D2","addx.w D2,D2","addx.w D3,D2",
		"addx.w D4,D2","addx.w D5,D2","addx.w D6,D2","addx.w D7,D2","addx.w -(A0),-(A2)","addx.w -(A1),-(A2)",
		"addx.w -(A2),-(A2)","addx.w -(A3),-(A2)","addx.w -(A4),-(A2)","addx.w -(A5),-(A2)",
		"addx.w -(A6),-(A2)","addx.w -(A7),-(A2)" };
	static const char *opATable0356[] = { "addx.l D0,D2","addx.l D1,D2","addx.l D2,D2","addx.l D3,D2",
		"addx.l D4,D2","addx.l D5,D2","addx.l D6,D2","addx.l D7,D2","addx.l -(A0),-(A2)","addx.l -(A1),-(A2)",
		"addx.l -(A2),-(A2)","addx.l -(A3),-(A2)","addx.l -(A4),-(A2)","addx.l -(A5),-(A2)",
		"addx.l -(A6),-(A2)","addx.l -(A7),-(A2)" };
	static const char *opATable035C[] = { "addx.b D0,D3","addx.b D1,D3","addx.b D2,D3","addx.b D3,D3",
		"addx.b D4,D3","addx.b D5,D3","addx.b D6,D3","addx.b D7,D3","addx.b -(A0),-(A3)","addx.b -(A1),-(A3)",
		"addx.b -(A2),-(A3)","addx.b -(A3),-(A3)","addx.b -(A4),-(A3)","addx.b -(A5),-(A3)",
		"addx.b -(A6),-(A3)","addx.b -(A7),-(A3)" };
	static const char *opATable035D[] = { "addx.w D0,D3","addx.w D1,D3","addx.w D2,D3","addx.w D3,D3",
		"addx.w D4,D3","addx.w D5,D3","addx.w D6,D3","addx.w D7,D3","addx.w -(A0),-(A3)","addx.w -(A1),-(A3)",
		"addx.w -(A2),-(A3)","addx.w -(A3),-(A3)","addx.w -(A4),-(A3)","addx.w -(A5),-(A3)",
		"addx.w -(A6),-(A3)","addx.w -(A7),-(A3)" };
	static const char *opATable035E[] = { "addx.l D0,D3","addx.l D1,D3","addx.l D2,D3","addx.l D3,D3",
		"addx.l D4,D3","addx.l D5,D3","addx.l D6,D3","addx.l D7,D3","addx.l -(A0),-(A3)","addx.l -(A1),-(A3)",
		"addx.l -(A2),-(A3)","addx.l -(A3),-(A3)","addx.l -(A4),-(A3)","addx.l -(A5),-(A3)",
		"addx.l -(A6),-(A3)","addx.l -(A7),-(A3)" };
	static const char *opATable0364[] = { "addx.b D0,D4","addx.b D1,D4","addx.b D2,D4","addx.b D3,D4",
		"addx.b D4,D4","addx.b D5,D4","addx.b D6,D4","addx.b D7,D4","addx.b -(A0),-(A4)","addx.b -(A1),-(A4)",
		"addx.b -(A2),-(A4)","addx.b -(A3),-(A4)","addx.b -(A4),-(A4)","addx.b -(A5),-(A4)",
		"addx.b -(A6),-(A4)","addx.b -(A7),-(A4)" };
	static const char *opATable0365[] = { "addx.w D0,D4","addx.w D1,D4","addx.w D2,D4","addx.w D3,D4",
		"addx.w D4,D4","addx.w D5,D4","addx.w D6,D4","addx.w D7,D4","addx.w -(A0),-(A4)","addx.w -(A1),-(A4)",
		"addx.w -(A2),-(A4)","addx.w -(A3),-(A4)","addx.w -(A4),-(A4)","addx.w -(A5),-(A4)",
		"addx.w -(A6),-(A4)","addx.w -(A7),-(A4)" };
	static const char *opATable0366[] = { "addx.l D0,D4","addx.l D1,D4","addx.l D2,D4","addx.l D3,D4",
		"addx.l D4,D4","addx.l D5,D4","addx.l D6,D4","addx.l D7,D4","addx.l -(A0),-(A4)","addx.l -(A1),-(A4)",
		"addx.l -(A2),-(A4)","addx.l -(A3),-(A4)","addx.l -(A4),-(A4)","addx.l -(A5),-(A4)",
		"addx.l -(A6),-(A4)","addx.l -(A7),-(A4)" };
	static const char *opATable036C[] = { "addx.b D0,D5","addx.b D1,D5","addx.b D2,D5","addx.b D3,D5",
		"addx.b D4,D5","addx.b D5,D5","addx.b D6,D5","addx.b D7,D5","addx.b -(A0),-(A5)","addx.b -(A1),-(A5)",
		"addx.b -(A2),-(A5)","addx.b -(A3),-(A5)","addx.b -(A4),-(A5)","addx.b -(A5),-(A5)",
		"addx.b -(A6),-(A5)","addx.b -(A7),-(A5)" };
	static const char *opATable036D[] = { "addx.w D0,D5","addx.w D1,D5","addx.w D2,D5","addx.w D3,D5",
		"addx.w D4,D5","addx.w D5,D5","addx.w D6,D5","addx.w D7,D5","addx.w -(A0),-(A5)","addx.w -(A1),-(A5)",
		"addx.w -(A2),-(A5)","addx.w -(A3),-(A5)","addx.w -(A4),-(A5)","addx.w -(A5),-(A5)",
		"addx.w -(A6),-(A5)","addx.w -(A7),-(A5)" };
	static const char *opATable036E[] = { "addx.l D0,D5","addx.l D1,D5","addx.l D2,D5","addx.l D3,D5",
		"addx.l D4,D5","addx.l D5,D5","addx.l D6,D5","addx.l D7,D5","addx.l -(A0),-(A5)","addx.l -(A1),-(A5)",
		"addx.l -(A2),-(A5)","addx.l -(A3),-(A5)","addx.l -(A4),-(A5)","addx.l -(A5),-(A5)",
		"addx.l -(A6),-(A5)","addx.l -(A7),-(A5)" };
	static const char *opATable0374[] = { "addx.b D0,D6","addx.b D1,D6","addx.b D2,D6","addx.b D3,D6",
		"addx.b D4,D6","addx.b D5,D6","addx.b D6,D6","addx.b D7,D6","addx.b -(A0),-(A6)","addx.b -(A1),-(A6)",
		"addx.b -(A2),-(A6)","addx.b -(A3),-(A6)","addx.b -(A4),-(A6)","addx.b -(A5),-(A6)",
		"addx.b -(A6),-(A6)","addx.b -(A7),-(A6)" };
	static const char *opATable0375[] = { "addx.w D0,D6","addx.w D1,D6","addx.w D2,D6","addx.w D3,D6",
		"addx.w D4,D6","addx.w D5,D6","addx.w D6,D6","addx.w D7,D6","addx.w -(A0),-(A6)","addx.w -(A1),-(A6)",
		"addx.w -(A2),-(A6)","addx.w -(A3),-(A6)","addx.w -(A4),-(A6)","addx.w -(A5),-(A6)",
		"addx.w -(A6),-(A6)","addx.w -(A7),-(A6)" };
	static const char *opATable0376[] = { "addx.l D0,D6","addx.l D1,D6","addx.l D2,D6","addx.l D3,D6",
		"addx.l D4,D6","addx.l D5,D6","addx.l D6,D6","addx.l D7,D6","addx.l -(A0),-(A6)","addx.l -(A1),-(A6)",
		"addx.l -(A2),-(A6)","addx.l -(A3),-(A6)","addx.l -(A4),-(A6)","addx.l -(A5),-(A6)",
		"addx.l -(A6),-(A6)","addx.l -(A7),-(A6)" };
	static const char *opATable037C[] = { "addx.b D0,D7","addx.b D1,D7","addx.b D2,D7","addx.b D3,D7",
		"addx.b D4,D7","addx.b D5,D7","addx.b D6,D7","addx.b D7,D7","addx.b -(A0),-(A7)","addx.b -(A1),-(A7)",
		"addx.b -(A2),-(A7)","addx.b -(A3),-(A7)","addx.b -(A4),-(A7)","addx.b -(A5),-(A7)",
		"addx.b -(A6),-(A7)","addx.b -(A7),-(A7)" };
	static const char *opATable037D[] = { "addx.w D0,D7","addx.w D1,D7","addx.w D2,D7","addx.w D3,D7",
		"addx.w D4,D7","addx.w D5,D7","addx.w D6,D7","addx.w D7,D7","addx.w -(A0),-(A7)","addx.w -(A1),-(A7)",
		"addx.w -(A2),-(A7)","addx.w -(A3),-(A7)","addx.w -(A4),-(A7)","addx.w -(A5),-(A7)",
		"addx.w -(A6),-(A7)","addx.w -(A7),-(A7)" };
	static const char *opATable037E[] = { "addx.l D0,D7","addx.l D1,D7","addx.l D2,D7","addx.l D3,D7",
		"addx.l D4,D7","addx.l D5,D7","addx.l D6,D7","addx.l D7,D7","addx.l -(A0),-(A7)","addx.l -(A1),-(A7)",
		"addx.l -(A2),-(A7)","addx.l -(A3),-(A7)","addx.l -(A4),-(A7)","addx.l -(A5),-(A7)",
		"addx.l -(A6),-(A7)","addx.l -(A7),-(A7)" };
	static const char *opATable0380[] = { "asrd.b #8,D0","asrd.b #8,D1","asrd.b #8,D2","asrd.b #8,D3",
		"asrd.b #8,D4","asrd.b #8,D5","asrd.b #8,D6","asrd.b #8,D7","lsrd.b #8,D0","lsrd.b #8,D1",
		"lsrd.b #8,D2","lsrd.b #8,D3","lsrd.b #8,D4","lsrd.b #8,D5","lsrd.b #8,D6","lsrd.b #8,D7",
		"roxrd.b #8,D0","roxrd.b #8,D1","roxrd.b #8,D2","roxrd.b #8,D3","roxrd.b #8,D4","roxrd.b #8,D5",
		"roxrd.b #8,D6","roxrd.b #8,D7","rord.b #8,D0","rord.b #8,D1","rord.b #8,D2","rord.b #8,D3",
		"rord.b #8,D4","rord.b #8,D5","rord.b #8,D6","rord.b #8,D7","asrd.b D0,D0","asrd.b D0,D1",
		"asrd.b D0,D2","asrd.b D0,D3","asrd.b D0,D4","asrd.b D0,D5","asrd.b D0,D6","asrd.b D0,D7",
		"lsrd.b D0,D0","lsrd.b D0,D1","lsrd.b D0,D2","lsrd.b D0,D3","lsrd.b D0,D4","lsrd.b D0,D5",
		"lsrd.b D0,D6","lsrd.b D0,D7","roxrd.b D0,D0","roxrd.b D0,D1","roxrd.b D0,D2","roxrd.b D0,D3",
		"roxrd.b D0,D4","roxrd.b D0,D5","roxrd.b D0,D6","roxrd.b D0,D7","rord.b D0,D0","rord.b D0,D1",
		"rord.b D0,D2","rord.b D0,D3","rord.b D0,D4","rord.b D0,D5","rord.b D0,D6","rord.b D0,D7" };
	static const char *opATable0381[] = { "asrd.w #8,D0","asrd.w #8,D1","asrd.w #8,D2","asrd.w #8,D3",
		"asrd.w #8,D4","asrd.w #8,D5","asrd.w #8,D6","asrd.w #8,D7","lsrd.w #8,D0","lsrd.w #8,D1",
		"lsrd.w #8,D2","lsrd.w #8,D3","lsrd.w #8,D4","lsrd.w #8,D5","lsrd.w #8,D6","lsrd.w #8,D7",
		"roxrd.w #8,D0","roxrd.w #8,D1","roxrd.w #8,D2","roxrd.w #8,D3","roxrd.w #8,D4","roxrd.w #8,D5",
		"roxrd.w #8,D6","roxrd.w #8,D7","rord.w #8,D0","rord.w #8,D1","rord.w #8,D2","rord.w #8,D3",
		"rord.w #8,D4","rord.w #8,D5","rord.w #8,D6","rord.w #8,D7","asrd.w D0,D0","asrd.w D0,D1",
		"asrd.w D0,D2","asrd.w D0,D3","asrd.w D0,D4","asrd.w D0,D5","asrd.w D0,D6","asrd.w D0,D7",
		"lsrd.w D0,D0","lsrd.w D0,D1","lsrd.w D0,D2","lsrd.w D0,D3","lsrd.w D0,D4","lsrd.w D0,D5",
		"lsrd.w D0,D6","lsrd.w D0,D7","roxrd.w D0,D0","roxrd.w D0,D1","roxrd.w D0,D2","roxrd.w D0,D3",
		"roxrd.w D0,D4","roxrd.w D0,D5","roxrd.w D0,D6","roxrd.w D0,D7","rord.w D0,D0","rord.w D0,D1",
		"rord.w D0,D2","rord.w D0,D3","rord.w D0,D4","rord.w D0,D5","rord.w D0,D6","rord.w D0,D7" };
	static const char *opATable0382[] = { "asrd.l #8,D0","asrd.l #8,D1","asrd.l #8,D2","asrd.l #8,D3",
		"asrd.l #8,D4","asrd.l #8,D5","asrd.l #8,D6","asrd.l #8,D7","lsrd.l #8,D0","lsrd.l #8,D1",
		"lsrd.l #8,D2","lsrd.l #8,D3","lsrd.l #8,D4","lsrd.l #8,D5","lsrd.l #8,D6","lsrd.l #8,D7",
		"roxrd.l #8,D0","roxrd.l #8,D1","roxrd.l #8,D2","roxrd.l #8,D3","roxrd.l #8,D4","roxrd.l #8,D5",
		"roxrd.l #8,D6","roxrd.l #8,D7","rord.l #8,D0","rord.l #8,D1","rord.l #8,D2","rord.l #8,D3",
		"rord.l #8,D4","rord.l #8,D5","rord.l #8,D6","rord.l #8,D7","asrd.l D0,D0","asrd.l D0,D1",
		"asrd.l D0,D2","asrd.l D0,D3","asrd.l D0,D4","asrd.l D0,D5","asrd.l D0,D6","asrd.l D0,D7",
		"lsrd.l D0,D0","lsrd.l D0,D1","lsrd.l D0,D2","lsrd.l D0,D3","lsrd.l D0,D4","lsrd.l D0,D5",
		"lsrd.l D0,D6","lsrd.l D0,D7","roxrd.l D0,D0","roxrd.l D0,D1","roxrd.l D0,D2","roxrd.l D0,D3",
		"roxrd.l D0,D4","roxrd.l D0,D5","roxrd.l D0,D6","roxrd.l D0,D7","rord.l D0,D0","rord.l D0,D1",
		"rord.l D0,D2","rord.l D0,D3","rord.l D0,D4","rord.l D0,D5","rord.l D0,D6","rord.l D0,D7" };
	static const char *opATable0384[] = { "asld.b #8,D0","asld.b #8,D1","asld.b #8,D2","asld.b #8,D3",
		"asld.b #8,D4","asld.b #8,D5","asld.b #8,D6","asld.b #8,D7","lsld.b #8,D0","lsld.b #8,D1",
		"lsld.b #8,D2","lsld.b #8,D3","lsld.b #8,D4","lsld.b #8,D5","lsld.b #8,D6","lsld.b #8,D7",
		"roxld.b #8,D0","roxld.b #8,D1","roxld.b #8,D2","roxld.b #8,D3","roxld.b #8,D4","roxld.b #8,D5",
		"roxld.b #8,D6","roxld.b #8,D7","rold.b #8,D0","rold.b #8,D1","rold.b #8,D2","rold.b #8,D3",
		"rold.b #8,D4","rold.b #8,D5","rold.b #8,D6","rold.b #8,D7","asld.b D0,D0","asld.b D0,D1",
		"asld.b D0,D2","asld.b D0,D3","asld.b D0,D4","asld.b D0,D5","asld.b D0,D6","asld.b D0,D7",
		"lsld.b D0,D0","lsld.b D0,D1","lsld.b D0,D2","lsld.b D0,D3","lsld.b D0,D4","lsld.b D0,D5",
		"lsld.b D0,D6","lsld.b D0,D7","roxld.b D0,D0","roxld.b D0,D1","roxld.b D0,D2","roxld.b D0,D3",
		"roxld.b D0,D4","roxld.b D0,D5","roxld.b D0,D6","roxld.b D0,D7","rold.b D0,D0","rold.b D0,D1",
		"rold.b D0,D2","rold.b D0,D3","rold.b D0,D4","rold.b D0,D5","rold.b D0,D6","rold.b D0,D7" };
	static const char *opATable0385[] = { "asld.w #8,D0","asld.w #8,D1","asld.w #8,D2","asld.w #8,D3",
		"asld.w #8,D4","asld.w #8,D5","asld.w #8,D6","asld.w #8,D7","lsld.w #8,D0","lsld.w #8,D1",
		"lsld.w #8,D2","lsld.w #8,D3","lsld.w #8,D4","lsld.w #8,D5","lsld.w #8,D6","lsld.w #8,D7",
		"roxld.w #8,D0","roxld.w #8,D1","roxld.w #8,D2","roxld.w #8,D3","roxld.w #8,D4","roxld.w #8,D5",
		"roxld.w #8,D6","roxld.w #8,D7","rold.w #8,D0","rold.w #8,D1","rold.w #8,D2","rold.w #8,D3",
		"rold.w #8,D4","rold.w #8,D5","rold.w #8,D6","rold.w #8,D7","asld.w D0,D0","asld.w D0,D1",
		"asld.w D0,D2","asld.w D0,D3","asld.w D0,D4","asld.w D0,D5","asld.w D0,D6","asld.w D0,D7",
		"lsld.w D0,D0","lsld.w D0,D1","lsld.w D0,D2","lsld.w D0,D3","lsld.w D0,D4","lsld.w D0,D5",
		"lsld.w D0,D6","lsld.w D0,D7","roxld.w D0,D0","roxld.w D0,D1","roxld.w D0,D2","roxld.w D0,D3",
		"roxld.w D0,D4","roxld.w D0,D5","roxld.w D0,D6","roxld.w D0,D7","rold.w D0,D0","rold.w D0,D1",
		"rold.w D0,D2","rold.w D0,D3","rold.w D0,D4","rold.w D0,D5","rold.w D0,D6","rold.w D0,D7" };
	static const char *opATable0386[] = { "asld.l #8,D0","asld.l #8,D1","asld.l #8,D2","asld.l #8,D3",
		"asld.l #8,D4","asld.l #8,D5","asld.l #8,D6","asld.l #8,D7","lsld.l #8,D0","lsld.l #8,D1",
		"lsld.l #8,D2","lsld.l #8,D3","lsld.l #8,D4","lsld.l #8,D5","lsld.l #8,D6","lsld.l #8,D7",
		"roxld.l #8,D0","roxld.l #8,D1","roxld.l #8,D2","roxld.l #8,D3","roxld.l #8,D4","roxld.l #8,D5",
		"roxld.l #8,D6","roxld.l #8,D7","rold.l #8,D0","rold.l #8,D1","rold.l #8,D2","rold.l #8,D3",
		"rold.l #8,D4","rold.l #8,D5","rold.l #8,D6","rold.l #8,D7","asld.l D0,D0","asld.l D0,D1",
		"asld.l D0,D2","asld.l D0,D3","asld.l D0,D4","asld.l D0,D5","asld.l D0,D6","asld.l D0,D7",
		"lsld.l D0,D0","lsld.l D0,D1","lsld.l D0,D2","lsld.l D0,D3","lsld.l D0,D4","lsld.l D0,D5",
		"lsld.l D0,D6","lsld.l D0,D7","roxld.l D0,D0","roxld.l D0,D1","roxld.l D0,D2","roxld.l D0,D3",
		"roxld.l D0,D4","roxld.l D0,D5","roxld.l D0,D6","roxld.l D0,D7","rold.l D0,D0","rold.l D0,D1",
		"rold.l D0,D2","rold.l D0,D3","rold.l D0,D4","rold.l D0,D5","rold.l D0,D6","rold.l D0,D7" };
	static const char *opATable0388[] = { "asrd.b #1,D0","asrd.b #1,D1","asrd.b #1,D2","asrd.b #1,D3",
		"asrd.b #1,D4","asrd.b #1,D5","asrd.b #1,D6","asrd.b #1,D7","lsrd.b #1,D0","lsrd.b #1,D1",
		"lsrd.b #1,D2","lsrd.b #1,D3","lsrd.b #1,D4","lsrd.b #1,D5","lsrd.b #1,D6","lsrd.b #1,D7",
		"roxrd.b #1,D0","roxrd.b #1,D1","roxrd.b #1,D2","roxrd.b #1,D3","roxrd.b #1,D4","roxrd.b #1,D5",
		"roxrd.b #1,D6","roxrd.b #1,D7","rord.b #1,D0","rord.b #1,D1","rord.b #1,D2","rord.b #1,D3",
		"rord.b #1,D4","rord.b #1,D5","rord.b #1,D6","rord.b #1,D7","asrd.b D1,D0","asrd.b D1,D1",
		"asrd.b D1,D2","asrd.b D1,D3","asrd.b D1,D4","asrd.b D1,D5","asrd.b D1,D6","asrd.b D1,D7",
		"lsrd.b D1,D0","lsrd.b D1,D1","lsrd.b D1,D2","lsrd.b D1,D3","lsrd.b D1,D4","lsrd.b D1,D5",
		"lsrd.b D1,D6","lsrd.b D1,D7","roxrd.b D1,D0","roxrd.b D1,D1","roxrd.b D1,D2","roxrd.b D1,D3",
		"roxrd.b D1,D4","roxrd.b D1,D5","roxrd.b D1,D6","roxrd.b D1,D7","rord.b D1,D0","rord.b D1,D1",
		"rord.b D1,D2","rord.b D1,D3","rord.b D1,D4","rord.b D1,D5","rord.b D1,D6","rord.b D1,D7" };
	static const char *opATable0389[] = { "asrd.w #1,D0","asrd.w #1,D1","asrd.w #1,D2","asrd.w #1,D3",
		"asrd.w #1,D4","asrd.w #1,D5","asrd.w #1,D6","asrd.w #1,D7","lsrd.w #1,D0","lsrd.w #1,D1",
		"lsrd.w #1,D2","lsrd.w #1,D3","lsrd.w #1,D4","lsrd.w #1,D5","lsrd.w #1,D6","lsrd.w #1,D7",
		"roxrd.w #1,D0","roxrd.w #1,D1","roxrd.w #1,D2","roxrd.w #1,D3","roxrd.w #1,D4","roxrd.w #1,D5",
		"roxrd.w #1,D6","roxrd.w #1,D7","rord.w #1,D0","rord.w #1,D1","rord.w #1,D2","rord.w #1,D3",
		"rord.w #1,D4","rord.w #1,D5","rord.w #1,D6","rord.w #1,D7","asrd.w D1,D0","asrd.w D1,D1",
		"asrd.w D1,D2","asrd.w D1,D3","asrd.w D1,D4","asrd.w D1,D5","asrd.w D1,D6","asrd.w D1,D7",
		"lsrd.w D1,D0","lsrd.w D1,D1","lsrd.w D1,D2","lsrd.w D1,D3","lsrd.w D1,D4","lsrd.w D1,D5",
		"lsrd.w D1,D6","lsrd.w D1,D7","roxrd.w D1,D0","roxrd.w D1,D1","roxrd.w D1,D2","roxrd.w D1,D3",
		"roxrd.w D1,D4","roxrd.w D1,D5","roxrd.w D1,D6","roxrd.w D1,D7","rord.w D1,D0","rord.w D1,D1",
		"rord.w D1,D2","rord.w D1,D3","rord.w D1,D4","rord.w D1,D5","rord.w D1,D6","rord.w D1,D7" };
	static const char *opATable038A[] = { "asrd.l #1,D0","asrd.l #1,D1","asrd.l #1,D2","asrd.l #1,D3",
		"asrd.l #1,D4","asrd.l #1,D5","asrd.l #1,D6","asrd.l #1,D7","lsrd.l #1,D0","lsrd.l #1,D1",
		"lsrd.l #1,D2","lsrd.l #1,D3","lsrd.l #1,D4","lsrd.l #1,D5","lsrd.l #1,D6","lsrd.l #1,D7",
		"roxrd.l #1,D0","roxrd.l #1,D1","roxrd.l #1,D2","roxrd.l #1,D3","roxrd.l #1,D4","roxrd.l #1,D5",
		"roxrd.l #1,D6","roxrd.l #1,D7","rord.l #1,D0","rord.l #1,D1","rord.l #1,D2","rord.l #1,D3",
		"rord.l #1,D4","rord.l #1,D5","rord.l #1,D6","rord.l #1,D7","asrd.l D1,D0","asrd.l D1,D1",
		"asrd.l D1,D2","asrd.l D1,D3","asrd.l D1,D4","asrd.l D1,D5","asrd.l D1,D6","asrd.l D1,D7",
		"lsrd.l D1,D0","lsrd.l D1,D1","lsrd.l D1,D2","lsrd.l D1,D3","lsrd.l D1,D4","lsrd.l D1,D5",
		"lsrd.l D1,D6","lsrd.l D1,D7","roxrd.l D1,D0","roxrd.l D1,D1","roxrd.l D1,D2","roxrd.l D1,D3",
		"roxrd.l D1,D4","roxrd.l D1,D5","roxrd.l D1,D6","roxrd.l D1,D7","rord.l D1,D0","rord.l D1,D1",
		"rord.l D1,D2","rord.l D1,D3","rord.l D1,D4","rord.l D1,D5","rord.l D1,D6","rord.l D1,D7" };
	static const char *opATable038C[] = { "asld.b #1,D0","asld.b #1,D1","asld.b #1,D2","asld.b #1,D3",
		"asld.b #1,D4","asld.b #1,D5","asld.b #1,D6","asld.b #1,D7","lsld.b #1,D0","lsld.b #1,D1",
		"lsld.b #1,D2","lsld.b #1,D3","lsld.b #1,D4","lsld.b #1,D5","lsld.b #1,D6","lsld.b #1,D7",
		"roxld.b #1,D0","roxld.b #1,D1","roxld.b #1,D2","roxld.b #1,D3","roxld.b #1,D4","roxld.b #1,D5",
		"roxld.b #1,D6","roxld.b #1,D7","rold.b #1,D0","rold.b #1,D1","rold.b #1,D2","rold.b #1,D3",
		"rold.b #1,D4","rold.b #1,D5","rold.b #1,D6","rold.b #1,D7","asld.b D1,D0","asld.b D1,D1",
		"asld.b D1,D2","asld.b D1,D3","asld.b D1,D4","asld.b D1,D5","asld.b D1,D6","asld.b D1,D7",
		"lsld.b D1,D0","lsld.b D1,D1","lsld.b D1,D2","lsld.b D1,D3","lsld.b D1,D4","lsld.b D1,D5",
		"lsld.b D1,D6","lsld.b D1,D7","roxld.b D1,D0","roxld.b D1,D1","roxld.b D1,D2","roxld.b D1,D3",
		"roxld.b D1,D4","roxld.b D1,D5","roxld.b D1,D6","roxld.b D1,D7","rold.b D1,D0","rold.b D1,D1",
		"rold.b D1,D2","rold.b D1,D3","rold.b D1,D4","rold.b D1,D5","rold.b D1,D6","rold.b D1,D7" };
	static const char *opATable038D[] = { "asld.w #1,D0","asld.w #1,D1","asld.w #1,D2","asld.w #1,D3",
		"asld.w #1,D4","asld.w #1,D5","asld.w #1,D6","asld.w #1,D7","lsld.w #1,D0","lsld.w #1,D1",
		"lsld.w #1,D2","lsld.w #1,D3","lsld.w #1,D4","lsld.w #1,D5","lsld.w #1,D6","lsld.w #1,D7",
		"roxld.w #1,D0","roxld.w #1,D1","roxld.w #1,D2","roxld.w #1,D3","roxld.w #1,D4","roxld.w #1,D5",
		"roxld.w #1,D6","roxld.w #1,D7","rold.w #1,D0","rold.w #1,D1","rold.w #1,D2","rold.w #1,D3",
		"rold.w #1,D4","rold.w #1,D5","rold.w #1,D6","rold.w #1,D7","asld.w D1,D0","asld.w D1,D1",
		"asld.w D1,D2","asld.w D1,D3","asld.w D1,D4","asld.w D1,D5","asld.w D1,D6","asld.w D1,D7",
		"lsld.w D1,D0","lsld.w D1,D1","lsld.w D1,D2","lsld.w D1,D3","lsld.w D1,D4","lsld.w D1,D5",
		"lsld.w D1,D6","lsld.w D1,D7","roxld.w D1,D0","roxld.w D1,D1","roxld.w D1,D2","roxld.w D1,D3",
		"roxld.w D1,D4","roxld.w D1,D5","roxld.w D1,D6","roxld.w D1,D7","rold.w D1,D0","rold.w D1,D1",
		"rold.w D1,D2","rold.w D1,D3","rold.w D1,D4","rold.w D1,D5","rold.w D1,D6","rold.w D1,D7" };
	static const char *opATable038E[] = { "asld.l #1,D0","asld.l #1,D1","asld.l #1,D2","asld.l #1,D3",
		"asld.l #1,D4","asld.l #1,D5","asld.l #1,D6","asld.l #1,D7","lsld.l #1,D0","lsld.l #1,D1",
		"lsld.l #1,D2","lsld.l #1,D3","lsld.l #1,D4","lsld.l #1,D5","lsld.l #1,D6","lsld.l #1,D7",
		"roxld.l #1,D0","roxld.l #1,D1","roxld.l #1,D2","roxld.l #1,D3","roxld.l #1,D4","roxld.l #1,D5",
		"roxld.l #1,D6","roxld.l #1,D7","rold.l #1,D0","rold.l #1,D1","rold.l #1,D2","rold.l #1,D3",
		"rold.l #1,D4","rold.l #1,D5","rold.l #1,D6","rold.l #1,D7","asld.l D1,D0","asld.l D1,D1",
		"asld.l D1,D2","asld.l D1,D3","asld.l D1,D4","asld.l D1,D5","asld.l D1,D6","asld.l D1,D7",
		"lsld.l D1,D0","lsld.l D1,D1","lsld.l D1,D2","lsld.l D1,D3","lsld.l D1,D4","lsld.l D1,D5",
		"lsld.l D1,D6","lsld.l D1,D7","roxld.l D1,D0","roxld.l D1,D1","roxld.l D1,D2","roxld.l D1,D3",
		"roxld.l D1,D4","roxld.l D1,D5","roxld.l D1,D6","roxld.l D1,D7","rold.l D1,D0","rold.l D1,D1",
		"rold.l D1,D2","rold.l D1,D3","rold.l D1,D4","rold.l D1,D5","rold.l D1,D6","rold.l D1,D7" };
	static const char *opATable0390[] = { "asrd.b #2,D0","asrd.b #2,D1","asrd.b #2,D2","asrd.b #2,D3",
		"asrd.b #2,D4","asrd.b #2,D5","asrd.b #2,D6","asrd.b #2,D7","lsrd.b #2,D0","lsrd.b #2,D1",
		"lsrd.b #2,D2","lsrd.b #2,D3","lsrd.b #2,D4","lsrd.b #2,D5","lsrd.b #2,D6","lsrd.b #2,D7",
		"roxrd.b #2,D0","roxrd.b #2,D1","roxrd.b #2,D2","roxrd.b #2,D3","roxrd.b #2,D4","roxrd.b #2,D5",
		"roxrd.b #2,D6","roxrd.b #2,D7","rord.b #2,D0","rord.b #2,D1","rord.b #2,D2","rord.b #2,D3",
		"rord.b #2,D4","rord.b #2,D5","rord.b #2,D6","rord.b #2,D7","asrd.b D2,D0","asrd.b D2,D1",
		"asrd.b D2,D2","asrd.b D2,D3","asrd.b D2,D4","asrd.b D2,D5","asrd.b D2,D6","asrd.b D2,D7",
		"lsrd.b D2,D0","lsrd.b D2,D1","lsrd.b D2,D2","lsrd.b D2,D3","lsrd.b D2,D4","lsrd.b D2,D5",
		"lsrd.b D2,D6","lsrd.b D2,D7","roxrd.b D2,D0","roxrd.b D2,D1","roxrd.b D2,D2","roxrd.b D2,D3",
		"roxrd.b D2,D4","roxrd.b D2,D5","roxrd.b D2,D6","roxrd.b D2,D7","rord.b D2,D0","rord.b D2,D1",
		"rord.b D2,D2","rord.b D2,D3","rord.b D2,D4","rord.b D2,D5","rord.b D2,D6","rord.b D2,D7" };
	static const char *opATable0391[] = { "asrd.w #2,D0","asrd.w #2,D1","asrd.w #2,D2","asrd.w #2,D3",
		"asrd.w #2,D4","asrd.w #2,D5","asrd.w #2,D6","asrd.w #2,D7","lsrd.w #2,D0","lsrd.w #2,D1",
		"lsrd.w #2,D2","lsrd.w #2,D3","lsrd.w #2,D4","lsrd.w #2,D5","lsrd.w #2,D6","lsrd.w #2,D7",
		"roxrd.w #2,D0","roxrd.w #2,D1","roxrd.w #2,D2","roxrd.w #2,D3","roxrd.w #2,D4","roxrd.w #2,D5",
		"roxrd.w #2,D6","roxrd.w #2,D7","rord.w #2,D0","rord.w #2,D1","rord.w #2,D2","rord.w #2,D3",
		"rord.w #2,D4","rord.w #2,D5","rord.w #2,D6","rord.w #2,D7","asrd.w D2,D0","asrd.w D2,D1",
		"asrd.w D2,D2","asrd.w D2,D3","asrd.w D2,D4","asrd.w D2,D5","asrd.w D2,D6","asrd.w D2,D7",
		"lsrd.w D2,D0","lsrd.w D2,D1","lsrd.w D2,D2","lsrd.w D2,D3","lsrd.w D2,D4","lsrd.w D2,D5",
		"lsrd.w D2,D6","lsrd.w D2,D7","roxrd.w D2,D0","roxrd.w D2,D1","roxrd.w D2,D2","roxrd.w D2,D3",
		"roxrd.w D2,D4","roxrd.w D2,D5","roxrd.w D2,D6","roxrd.w D2,D7","rord.w D2,D0","rord.w D2,D1",
		"rord.w D2,D2","rord.w D2,D3","rord.w D2,D4","rord.w D2,D5","rord.w D2,D6","rord.w D2,D7" };
	static const char *opATable0392[] = { "asrd.l #2,D0","asrd.l #2,D1","asrd.l #2,D2","asrd.l #2,D3",
		"asrd.l #2,D4","asrd.l #2,D5","asrd.l #2,D6","asrd.l #2,D7","lsrd.l #2,D0","lsrd.l #2,D1",
		"lsrd.l #2,D2","lsrd.l #2,D3","lsrd.l #2,D4","lsrd.l #2,D5","lsrd.l #2,D6","lsrd.l #2,D7",
		"roxrd.l #2,D0","roxrd.l #2,D1","roxrd.l #2,D2","roxrd.l #2,D3","roxrd.l #2,D4","roxrd.l #2,D5",
		"roxrd.l #2,D6","roxrd.l #2,D7","rord.l #2,D0","rord.l #2,D1","rord.l #2,D2","rord.l #2,D3",
		"rord.l #2,D4","rord.l #2,D5","rord.l #2,D6","rord.l #2,D7","asrd.l D2,D0","asrd.l D2,D1",
		"asrd.l D2,D2","asrd.l D2,D3","asrd.l D2,D4","asrd.l D2,D5","asrd.l D2,D6","asrd.l D2,D7",
		"lsrd.l D2,D0","lsrd.l D2,D1","lsrd.l D2,D2","lsrd.l D2,D3","lsrd.l D2,D4","lsrd.l D2,D5",
		"lsrd.l D2,D6","lsrd.l D2,D7","roxrd.l D2,D0","roxrd.l D2,D1","roxrd.l D2,D2","roxrd.l D2,D3",
		"roxrd.l D2,D4","roxrd.l D2,D5","roxrd.l D2,D6","roxrd.l D2,D7","rord.l D2,D0","rord.l D2,D1",
		"rord.l D2,D2","rord.l D2,D3","rord.l D2,D4","rord.l D2,D5","rord.l D2,D6","rord.l D2,D7" };
	static const char *opATable0394[] = { "asld.b #2,D0","asld.b #2,D1","asld.b #2,D2","asld.b #2,D3",
		"asld.b #2,D4","asld.b #2,D5","asld.b #2,D6","asld.b #2,D7","lsld.b #2,D0","lsld.b #2,D1",
		"lsld.b #2,D2","lsld.b #2,D3","lsld.b #2,D4","lsld.b #2,D5","lsld.b #2,D6","lsld.b #2,D7",
		"roxld.b #2,D0","roxld.b #2,D1","roxld.b #2,D2","roxld.b #2,D3","roxld.b #2,D4","roxld.b #2,D5",
		"roxld.b #2,D6","roxld.b #2,D7","rold.b #2,D0","rold.b #2,D1","rold.b #2,D2","rold.b #2,D3",
		"rold.b #2,D4","rold.b #2,D5","rold.b #2,D6","rold.b #2,D7","asld.b D2,D0","asld.b D2,D1",
		"asld.b D2,D2","asld.b D2,D3","asld.b D2,D4","asld.b D2,D5","asld.b D2,D6","asld.b D2,D7",
		"lsld.b D2,D0","lsld.b D2,D1","lsld.b D2,D2","lsld.b D2,D3","lsld.b D2,D4","lsld.b D2,D5",
		"lsld.b D2,D6","lsld.b D2,D7","roxld.b D2,D0","roxld.b D2,D1","roxld.b D2,D2","roxld.b D2,D3",
		"roxld.b D2,D4","roxld.b D2,D5","roxld.b D2,D6","roxld.b D2,D7","rold.b D2,D0","rold.b D2,D1",
		"rold.b D2,D2","rold.b D2,D3","rold.b D2,D4","rold.b D2,D5","rold.b D2,D6","rold.b D2,D7" };
	static const char *opATable0395[] = { "asld.w #2,D0","asld.w #2,D1","asld.w #2,D2","asld.w #2,D3",
		"asld.w #2,D4","asld.w #2,D5","asld.w #2,D6","asld.w #2,D7","lsld.w #2,D0","lsld.w #2,D1",
		"lsld.w #2,D2","lsld.w #2,D3","lsld.w #2,D4","lsld.w #2,D5","lsld.w #2,D6","lsld.w #2,D7",
		"roxld.w #2,D0","roxld.w #2,D1","roxld.w #2,D2","roxld.w #2,D3","roxld.w #2,D4","roxld.w #2,D5",
		"roxld.w #2,D6","roxld.w #2,D7","rold.w #2,D0","rold.w #2,D1","rold.w #2,D2","rold.w #2,D3",
		"rold.w #2,D4","rold.w #2,D5","rold.w #2,D6","rold.w #2,D7","asld.w D2,D0","asld.w D2,D1",
		"asld.w D2,D2","asld.w D2,D3","asld.w D2,D4","asld.w D2,D5","asld.w D2,D6","asld.w D2,D7",
		"lsld.w D2,D0","lsld.w D2,D1","lsld.w D2,D2","lsld.w D2,D3","lsld.w D2,D4","lsld.w D2,D5",
		"lsld.w D2,D6","lsld.w D2,D7","roxld.w D2,D0","roxld.w D2,D1","roxld.w D2,D2","roxld.w D2,D3",
		"roxld.w D2,D4","roxld.w D2,D5","roxld.w D2,D6","roxld.w D2,D7","rold.w D2,D0","rold.w D2,D1",
		"rold.w D2,D2","rold.w D2,D3","rold.w D2,D4","rold.w D2,D5","rold.w D2,D6","rold.w D2,D7" };
	static const char *opATable0396[] = { "asld.l #2,D0","asld.l #2,D1","asld.l #2,D2","asld.l #2,D3",
		"asld.l #2,D4","asld.l #2,D5","asld.l #2,D6","asld.l #2,D7","lsld.l #2,D0","lsld.l #2,D1",
		"lsld.l #2,D2","lsld.l #2,D3","lsld.l #2,D4","lsld.l #2,D5","lsld.l #2,D6","lsld.l #2,D7",
		"roxld.l #2,D0","roxld.l #2,D1","roxld.l #2,D2","roxld.l #2,D3","roxld.l #2,D4","roxld.l #2,D5",
		"roxld.l #2,D6","roxld.l #2,D7","rold.l #2,D0","rold.l #2,D1","rold.l #2,D2","rold.l #2,D3",
		"rold.l #2,D4","rold.l #2,D5","rold.l #2,D6","rold.l #2,D7","asld.l D2,D0","asld.l D2,D1",
		"asld.l D2,D2","asld.l D2,D3","asld.l D2,D4","asld.l D2,D5","asld.l D2,D6","asld.l D2,D7",
		"lsld.l D2,D0","lsld.l D2,D1","lsld.l D2,D2","lsld.l D2,D3","lsld.l D2,D4","lsld.l D2,D5",
		"lsld.l D2,D6","lsld.l D2,D7","roxld.l D2,D0","roxld.l D2,D1","roxld.l D2,D2","roxld.l D2,D3",
		"roxld.l D2,D4","roxld.l D2,D5","roxld.l D2,D6","roxld.l D2,D7","rold.l D2,D0","rold.l D2,D1",
		"rold.l D2,D2","rold.l D2,D3","rold.l D2,D4","rold.l D2,D5","rold.l D2,D6","rold.l D2,D7" };
	static const char *opATable0398[] = { "asrd.b #3,D0","asrd.b #3,D1","asrd.b #3,D2","asrd.b #3,D3",
		"asrd.b #3,D4","asrd.b #3,D5","asrd.b #3,D6","asrd.b #3,D7","lsrd.b #3,D0","lsrd.b #3,D1",
		"lsrd.b #3,D2","lsrd.b #3,D3","lsrd.b #3,D4","lsrd.b #3,D5","lsrd.b #3,D6","lsrd.b #3,D7",
		"roxrd.b #3,D0","roxrd.b #3,D1","roxrd.b #3,D2","roxrd.b #3,D3","roxrd.b #3,D4","roxrd.b #3,D5",
		"roxrd.b #3,D6","roxrd.b #3,D7","rord.b #3,D0","rord.b #3,D1","rord.b #3,D2","rord.b #3,D3",
		"rord.b #3,D4","rord.b #3,D5","rord.b #3,D6","rord.b #3,D7","asrd.b D3,D0","asrd.b D3,D1",
		"asrd.b D3,D2","asrd.b D3,D3","asrd.b D3,D4","asrd.b D3,D5","asrd.b D3,D6","asrd.b D3,D7",
		"lsrd.b D3,D0","lsrd.b D3,D1","lsrd.b D3,D2","lsrd.b D3,D3","lsrd.b D3,D4","lsrd.b D3,D5",
		"lsrd.b D3,D6","lsrd.b D3,D7","roxrd.b D3,D0","roxrd.b D3,D1","roxrd.b D3,D2","roxrd.b D3,D3",
		"roxrd.b D3,D4","roxrd.b D3,D5","roxrd.b D3,D6","roxrd.b D3,D7","rord.b D3,D0","rord.b D3,D1",
		"rord.b D3,D2","rord.b D3,D3","rord.b D3,D4","rord.b D3,D5","rord.b D3,D6","rord.b D3,D7" };
	static const char *opATable0399[] = { "asrd.w #3,D0","asrd.w #3,D1","asrd.w #3,D2","asrd.w #3,D3",
		"asrd.w #3,D4","asrd.w #3,D5","asrd.w #3,D6","asrd.w #3,D7","lsrd.w #3,D0","lsrd.w #3,D1",
		"lsrd.w #3,D2","lsrd.w #3,D3","lsrd.w #3,D4","lsrd.w #3,D5","lsrd.w #3,D6","lsrd.w #3,D7",
		"roxrd.w #3,D0","roxrd.w #3,D1","roxrd.w #3,D2","roxrd.w #3,D3","roxrd.w #3,D4","roxrd.w #3,D5",
		"roxrd.w #3,D6","roxrd.w #3,D7","rord.w #3,D0","rord.w #3,D1","rord.w #3,D2","rord.w #3,D3",
		"rord.w #3,D4","rord.w #3,D5","rord.w #3,D6","rord.w #3,D7","asrd.w D3,D0","asrd.w D3,D1",
		"asrd.w D3,D2","asrd.w D3,D3","asrd.w D3,D4","asrd.w D3,D5","asrd.w D3,D6","asrd.w D3,D7",
		"lsrd.w D3,D0","lsrd.w D3,D1","lsrd.w D3,D2","lsrd.w D3,D3","lsrd.w D3,D4","lsrd.w D3,D5",
		"lsrd.w D3,D6","lsrd.w D3,D7","roxrd.w D3,D0","roxrd.w D3,D1","roxrd.w D3,D2","roxrd.w D3,D3",
		"roxrd.w D3,D4","roxrd.w D3,D5","roxrd.w D3,D6","roxrd.w D3,D7","rord.w D3,D0","rord.w D3,D1",
		"rord.w D3,D2","rord.w D3,D3","rord.w D3,D4","rord.w D3,D5","rord.w D3,D6","rord.w D3,D7" };
	static const char *opATable039A[] = { "asrd.l #3,D0","asrd.l #3,D1","asrd.l #3,D2","asrd.l #3,D3",
		"asrd.l #3,D4","asrd.l #3,D5","asrd.l #3,D6","asrd.l #3,D7","lsrd.l #3,D0","lsrd.l #3,D1",
		"lsrd.l #3,D2","lsrd.l #3,D3","lsrd.l #3,D4","lsrd.l #3,D5","lsrd.l #3,D6","lsrd.l #3,D7",
		"roxrd.l #3,D0","roxrd.l #3,D1","roxrd.l #3,D2","roxrd.l #3,D3","roxrd.l #3,D4","roxrd.l #3,D5",
		"roxrd.l #3,D6","roxrd.l #3,D7","rord.l #3,D0","rord.l #3,D1","rord.l #3,D2","rord.l #3,D3",
		"rord.l #3,D4","rord.l #3,D5","rord.l #3,D6","rord.l #3,D7","asrd.l D3,D0","asrd.l D3,D1",
		"asrd.l D3,D2","asrd.l D3,D3","asrd.l D3,D4","asrd.l D3,D5","asrd.l D3,D6","asrd.l D3,D7",
		"lsrd.l D3,D0","lsrd.l D3,D1","lsrd.l D3,D2","lsrd.l D3,D3","lsrd.l D3,D4","lsrd.l D3,D5",
		"lsrd.l D3,D6","lsrd.l D3,D7","roxrd.l D3,D0","roxrd.l D3,D1","roxrd.l D3,D2","roxrd.l D3,D3",
		"roxrd.l D3,D4","roxrd.l D3,D5","roxrd.l D3,D6","roxrd.l D3,D7","rord.l D3,D0","rord.l D3,D1",
		"rord.l D3,D2","rord.l D3,D3","rord.l D3,D4","rord.l D3,D5","rord.l D3,D6","rord.l D3,D7" };
	static const char *opATable039C[] = { "asld.b #3,D0","asld.b #3,D1","asld.b #3,D2","asld.b #3,D3",
		"asld.b #3,D4","asld.b #3,D5","asld.b #3,D6","asld.b #3,D7","lsld.b #3,D0","lsld.b #3,D1",
		"lsld.b #3,D2","lsld.b #3,D3","lsld.b #3,D4","lsld.b #3,D5","lsld.b #3,D6","lsld.b #3,D7",
		"roxld.b #3,D0","roxld.b #3,D1","roxld.b #3,D2","roxld.b #3,D3","roxld.b #3,D4","roxld.b #3,D5",
		"roxld.b #3,D6","roxld.b #3,D7","rold.b #3,D0","rold.b #3,D1","rold.b #3,D2","rold.b #3,D3",
		"rold.b #3,D4","rold.b #3,D5","rold.b #3,D6","rold.b #3,D7","asld.b D3,D0","asld.b D3,D1",
		"asld.b D3,D2","asld.b D3,D3","asld.b D3,D4","asld.b D3,D5","asld.b D3,D6","asld.b D3,D7",
		"lsld.b D3,D0","lsld.b D3,D1","lsld.b D3,D2","lsld.b D3,D3","lsld.b D3,D4","lsld.b D3,D5",
		"lsld.b D3,D6","lsld.b D3,D7","roxld.b D3,D0","roxld.b D3,D1","roxld.b D3,D2","roxld.b D3,D3",
		"roxld.b D3,D4","roxld.b D3,D5","roxld.b D3,D6","roxld.b D3,D7","rold.b D3,D0","rold.b D3,D1",
		"rold.b D3,D2","rold.b D3,D3","rold.b D3,D4","rold.b D3,D5","rold.b D3,D6","rold.b D3,D7" };
	static const char *opATable039D[] = { "asld.w #3,D0","asld.w #3,D1","asld.w #3,D2","asld.w #3,D3",
		"asld.w #3,D4","asld.w #3,D5","asld.w #3,D6","asld.w #3,D7","lsld.w #3,D0","lsld.w #3,D1",
		"lsld.w #3,D2","lsld.w #3,D3","lsld.w #3,D4","lsld.w #3,D5","lsld.w #3,D6","lsld.w #3,D7",
		"roxld.w #3,D0","roxld.w #3,D1","roxld.w #3,D2","roxld.w #3,D3","roxld.w #3,D4","roxld.w #3,D5",
		"roxld.w #3,D6","roxld.w #3,D7","rold.w #3,D0","rold.w #3,D1","rold.w #3,D2","rold.w #3,D3",
		"rold.w #3,D4","rold.w #3,D5","rold.w #3,D6","rold.w #3,D7","asld.w D3,D0","asld.w D3,D1",
		"asld.w D3,D2","asld.w D3,D3","asld.w D3,D4","asld.w D3,D5","asld.w D3,D6","asld.w D3,D7",
		"lsld.w D3,D0","lsld.w D3,D1","lsld.w D3,D2","lsld.w D3,D3","lsld.w D3,D4","lsld.w D3,D5",
		"lsld.w D3,D6","lsld.w D3,D7","roxld.w D3,D0","roxld.w D3,D1","roxld.w D3,D2","roxld.w D3,D3",
		"roxld.w D3,D4","roxld.w D3,D5","roxld.w D3,D6","roxld.w D3,D7","rold.w D3,D0","rold.w D3,D1",
		"rold.w D3,D2","rold.w D3,D3","rold.w D3,D4","rold.w D3,D5","rold.w D3,D6","rold.w D3,D7" };
	static const char *opATable039E[] = { "asld.l #3,D0","asld.l #3,D1","asld.l #3,D2","asld.l #3,D3",
		"asld.l #3,D4","asld.l #3,D5","asld.l #3,D6","asld.l #3,D7","lsld.l #3,D0","lsld.l #3,D1",
		"lsld.l #3,D2","lsld.l #3,D3","lsld.l #3,D4","lsld.l #3,D5","lsld.l #3,D6","lsld.l #3,D7",
		"roxld.l #3,D0","roxld.l #3,D1","roxld.l #3,D2","roxld.l #3,D3","roxld.l #3,D4","roxld.l #3,D5",
		"roxld.l #3,D6","roxld.l #3,D7","rold.l #3,D0","rold.l #3,D1","rold.l #3,D2","rold.l #3,D3",
		"rold.l #3,D4","rold.l #3,D5","rold.l #3,D6","rold.l #3,D7","asld.l D3,D0","asld.l D3,D1",
		"asld.l D3,D2","asld.l D3,D3","asld.l D3,D4","asld.l D3,D5","asld.l D3,D6","asld.l D3,D7",
		"lsld.l D3,D0","lsld.l D3,D1","lsld.l D3,D2","lsld.l D3,D3","lsld.l D3,D4","lsld.l D3,D5",
		"lsld.l D3,D6","lsld.l D3,D7","roxld.l D3,D0","roxld.l D3,D1","roxld.l D3,D2","roxld.l D3,D3",
		"roxld.l D3,D4","roxld.l D3,D5","roxld.l D3,D6","roxld.l D3,D7","rold.l D3,D0","rold.l D3,D1",
		"rold.l D3,D2","rold.l D3,D3","rold.l D3,D4","rold.l D3,D5","rold.l D3,D6","rold.l D3,D7" };
	static const char *opATable03A0[] = { "asrd.b #4,D0","asrd.b #4,D1","asrd.b #4,D2","asrd.b #4,D3",
		"asrd.b #4,D4","asrd.b #4,D5","asrd.b #4,D6","asrd.b #4,D7","lsrd.b #4,D0","lsrd.b #4,D1",
		"lsrd.b #4,D2","lsrd.b #4,D3","lsrd.b #4,D4","lsrd.b #4,D5","lsrd.b #4,D6","lsrd.b #4,D7",
		"roxrd.b #4,D0","roxrd.b #4,D1","roxrd.b #4,D2","roxrd.b #4,D3","roxrd.b #4,D4","roxrd.b #4,D5",
		"roxrd.b #4,D6","roxrd.b #4,D7","rord.b #4,D0","rord.b #4,D1","rord.b #4,D2","rord.b #4,D3",
		"rord.b #4,D4","rord.b #4,D5","rord.b #4,D6","rord.b #4,D7","asrd.b D4,D0","asrd.b D4,D1",
		"asrd.b D4,D2","asrd.b D4,D3","asrd.b D4,D4","asrd.b D4,D5","asrd.b D4,D6","asrd.b D4,D7",
		"lsrd.b D4,D0","lsrd.b D4,D1","lsrd.b D4,D2","lsrd.b D4,D3","lsrd.b D4,D4","lsrd.b D4,D5",
		"lsrd.b D4,D6","lsrd.b D4,D7","roxrd.b D4,D0","roxrd.b D4,D1","roxrd.b D4,D2","roxrd.b D4,D3",
		"roxrd.b D4,D4","roxrd.b D4,D5","roxrd.b D4,D6","roxrd.b D4,D7","rord.b D4,D0","rord.b D4,D1",
		"rord.b D4,D2","rord.b D4,D3","rord.b D4,D4","rord.b D4,D5","rord.b D4,D6","rord.b D4,D7" };
	static const char *opATable03A1[] = { "asrd.w #4,D0","asrd.w #4,D1","asrd.w #4,D2","asrd.w #4,D3",
		"asrd.w #4,D4","asrd.w #4,D5","asrd.w #4,D6","asrd.w #4,D7","lsrd.w #4,D0","lsrd.w #4,D1",
		"lsrd.w #4,D2","lsrd.w #4,D3","lsrd.w #4,D4","lsrd.w #4,D5","lsrd.w #4,D6","lsrd.w #4,D7",
		"roxrd.w #4,D0","roxrd.w #4,D1","roxrd.w #4,D2","roxrd.w #4,D3","roxrd.w #4,D4","roxrd.w #4,D5",
		"roxrd.w #4,D6","roxrd.w #4,D7","rord.w #4,D0","rord.w #4,D1","rord.w #4,D2","rord.w #4,D3",
		"rord.w #4,D4","rord.w #4,D5","rord.w #4,D6","rord.w #4,D7","asrd.w D4,D0","asrd.w D4,D1",
		"asrd.w D4,D2","asrd.w D4,D3","asrd.w D4,D4","asrd.w D4,D5","asrd.w D4,D6","asrd.w D4,D7",
		"lsrd.w D4,D0","lsrd.w D4,D1","lsrd.w D4,D2","lsrd.w D4,D3","lsrd.w D4,D4","lsrd.w D4,D5",
		"lsrd.w D4,D6","lsrd.w D4,D7","roxrd.w D4,D0","roxrd.w D4,D1","roxrd.w D4,D2","roxrd.w D4,D3",
		"roxrd.w D4,D4","roxrd.w D4,D5","roxrd.w D4,D6","roxrd.w D4,D7","rord.w D4,D0","rord.w D4,D1",
		"rord.w D4,D2","rord.w D4,D3","rord.w D4,D4","rord.w D4,D5","rord.w D4,D6","rord.w D4,D7" };
	static const char *opATable03A2[] = { "asrd.l #4,D0","asrd.l #4,D1","asrd.l #4,D2","asrd.l #4,D3",
		"asrd.l #4,D4","asrd.l #4,D5","asrd.l #4,D6","asrd.l #4,D7","lsrd.l #4,D0","lsrd.l #4,D1",
		"lsrd.l #4,D2","lsrd.l #4,D3","lsrd.l #4,D4","lsrd.l #4,D5","lsrd.l #4,D6","lsrd.l #4,D7",
		"roxrd.l #4,D0","roxrd.l #4,D1","roxrd.l #4,D2","roxrd.l #4,D3","roxrd.l #4,D4","roxrd.l #4,D5",
		"roxrd.l #4,D6","roxrd.l #4,D7","rord.l #4,D0","rord.l #4,D1","rord.l #4,D2","rord.l #4,D3",
		"rord.l #4,D4","rord.l #4,D5","rord.l #4,D6","rord.l #4,D7","asrd.l D4,D0","asrd.l D4,D1",
		"asrd.l D4,D2","asrd.l D4,D3","asrd.l D4,D4","asrd.l D4,D5","asrd.l D4,D6","asrd.l D4,D7",
		"lsrd.l D4,D0","lsrd.l D4,D1","lsrd.l D4,D2","lsrd.l D4,D3","lsrd.l D4,D4","lsrd.l D4,D5",
		"lsrd.l D4,D6","lsrd.l D4,D7","roxrd.l D4,D0","roxrd.l D4,D1","roxrd.l D4,D2","roxrd.l D4,D3",
		"roxrd.l D4,D4","roxrd.l D4,D5","roxrd.l D4,D6","roxrd.l D4,D7","rord.l D4,D0","rord.l D4,D1",
		"rord.l D4,D2","rord.l D4,D3","rord.l D4,D4","rord.l D4,D5","rord.l D4,D6","rord.l D4,D7" };
	static const char *opATable03A4[] = { "asld.b #4,D0","asld.b #4,D1","asld.b #4,D2","asld.b #4,D3",
		"asld.b #4,D4","asld.b #4,D5","asld.b #4,D6","asld.b #4,D7","lsld.b #4,D0","lsld.b #4,D1",
		"lsld.b #4,D2","lsld.b #4,D3","lsld.b #4,D4","lsld.b #4,D5","lsld.b #4,D6","lsld.b #4,D7",
		"roxld.b #4,D0","roxld.b #4,D1","roxld.b #4,D2","roxld.b #4,D3","roxld.b #4,D4","roxld.b #4,D5",
		"roxld.b #4,D6","roxld.b #4,D7","rold.b #4,D0","rold.b #4,D1","rold.b #4,D2","rold.b #4,D3",
		"rold.b #4,D4","rold.b #4,D5","rold.b #4,D6","rold.b #4,D7","asld.b D4,D0","asld.b D4,D1",
		"asld.b D4,D2","asld.b D4,D3","asld.b D4,D4","asld.b D4,D5","asld.b D4,D6","asld.b D4,D7",
		"lsld.b D4,D0","lsld.b D4,D1","lsld.b D4,D2","lsld.b D4,D3","lsld.b D4,D4","lsld.b D4,D5",
		"lsld.b D4,D6","lsld.b D4,D7","roxld.b D4,D0","roxld.b D4,D1","roxld.b D4,D2","roxld.b D4,D3",
		"roxld.b D4,D4","roxld.b D4,D5","roxld.b D4,D6","roxld.b D4,D7","rold.b D4,D0","rold.b D4,D1",
		"rold.b D4,D2","rold.b D4,D3","rold.b D4,D4","rold.b D4,D5","rold.b D4,D6","rold.b D4,D7" };
	static const char *opATable03A5[] = { "asld.w #4,D0","asld.w #4,D1","asld.w #4,D2","asld.w #4,D3",
		"asld.w #4,D4","asld.w #4,D5","asld.w #4,D6","asld.w #4,D7","lsld.w #4,D0","lsld.w #4,D1",
		"lsld.w #4,D2","lsld.w #4,D3","lsld.w #4,D4","lsld.w #4,D5","lsld.w #4,D6","lsld.w #4,D7",
		"roxld.w #4,D0","roxld.w #4,D1","roxld.w #4,D2","roxld.w #4,D3","roxld.w #4,D4","roxld.w #4,D5",
		"roxld.w #4,D6","roxld.w #4,D7","rold.w #4,D0","rold.w #4,D1","rold.w #4,D2","rold.w #4,D3",
		"rold.w #4,D4","rold.w #4,D5","rold.w #4,D6","rold.w #4,D7","asld.w D4,D0","asld.w D4,D1",
		"asld.w D4,D2","asld.w D4,D3","asld.w D4,D4","asld.w D4,D5","asld.w D4,D6","asld.w D4,D7",
		"lsld.w D4,D0","lsld.w D4,D1","lsld.w D4,D2","lsld.w D4,D3","lsld.w D4,D4","lsld.w D4,D5",
		"lsld.w D4,D6","lsld.w D4,D7","roxld.w D4,D0","roxld.w D4,D1","roxld.w D4,D2","roxld.w D4,D3",
		"roxld.w D4,D4","roxld.w D4,D5","roxld.w D4,D6","roxld.w D4,D7","rold.w D4,D0","rold.w D4,D1",
		"rold.w D4,D2","rold.w D4,D3","rold.w D4,D4","rold.w D4,D5","rold.w D4,D6","rold.w D4,D7" };
	static const char *opATable03A6[] = { "asld.l #4,D0","asld.l #4,D1","asld.l #4,D2","asld.l #4,D3",
		"asld.l #4,D4","asld.l #4,D5","asld.l #4,D6","asld.l #4,D7","lsld.l #4,D0","lsld.l #4,D1",
		"lsld.l #4,D2","lsld.l #4,D3","lsld.l #4,D4","lsld.l #4,D5","lsld.l #4,D6","lsld.l #4,D7",
		"roxld.l #4,D0","roxld.l #4,D1","roxld.l #4,D2","roxld.l #4,D3","roxld.l #4,D4","roxld.l #4,D5",
		"roxld.l #4,D6","roxld.l #4,D7","rold.l #4,D0","rold.l #4,D1","rold.l #4,D2","rold.l #4,D3",
		"rold.l #4,D4","rold.l #4,D5","rold.l #4,D6","rold.l #4,D7","asld.l D4,D0","asld.l D4,D1",
		"asld.l D4,D2","asld.l D4,D3","asld.l D4,D4","asld.l D4,D5","asld.l D4,D6","asld.l D4,D7",
		"lsld.l D4,D0","lsld.l D4,D1","lsld.l D4,D2","lsld.l D4,D3","lsld.l D4,D4","lsld.l D4,D5",
		"lsld.l D4,D6","lsld.l D4,D7","roxld.l D4,D0","roxld.l D4,D1","roxld.l D4,D2","roxld.l D4,D3",
		"roxld.l D4,D4","roxld.l D4,D5","roxld.l D4,D6","roxld.l D4,D7","rold.l D4,D0","rold.l D4,D1",
		"rold.l D4,D2","rold.l D4,D3","rold.l D4,D4","rold.l D4,D5","rold.l D4,D6","rold.l D4,D7" };
	static const char *opATable03A8[] = { "asrd.b #5,D0","asrd.b #5,D1","asrd.b #5,D2","asrd.b #5,D3",
		"asrd.b #5,D4","asrd.b #5,D5","asrd.b #5,D6","asrd.b #5,D7","lsrd.b #5,D0","lsrd.b #5,D1",
		"lsrd.b #5,D2","lsrd.b #5,D3","lsrd.b #5,D4","lsrd.b #5,D5","lsrd.b #5,D6","lsrd.b #5,D7",
		"roxrd.b #5,D0","roxrd.b #5,D1","roxrd.b #5,D2","roxrd.b #5,D3","roxrd.b #5,D4","roxrd.b #5,D5",
		"roxrd.b #5,D6","roxrd.b #5,D7","rord.b #5,D0","rord.b #5,D1","rord.b #5,D2","rord.b #5,D3",
		"rord.b #5,D4","rord.b #5,D5","rord.b #5,D6","rord.b #5,D7","asrd.b D5,D0","asrd.b D5,D1",
		"asrd.b D5,D2","asrd.b D5,D3","asrd.b D5,D4","asrd.b D5,D5","asrd.b D5,D6","asrd.b D5,D7",
		"lsrd.b D5,D0","lsrd.b D5,D1","lsrd.b D5,D2","lsrd.b D5,D3","lsrd.b D5,D4","lsrd.b D5,D5",
		"lsrd.b D5,D6","lsrd.b D5,D7","roxrd.b D5,D0","roxrd.b D5,D1","roxrd.b D5,D2","roxrd.b D5,D3",
		"roxrd.b D5,D4","roxrd.b D5,D5","roxrd.b D5,D6","roxrd.b D5,D7","rord.b D5,D0","rord.b D5,D1",
		"rord.b D5,D2","rord.b D5,D3","rord.b D5,D4","rord.b D5,D5","rord.b D5,D6","rord.b D5,D7" };
	static const char *opATable03A9[] = { "asrd.w #5,D0","asrd.w #5,D1","asrd.w #5,D2","asrd.w #5,D3",
		"asrd.w #5,D4","asrd.w #5,D5","asrd.w #5,D6","asrd.w #5,D7","lsrd.w #5,D0","lsrd.w #5,D1",
		"lsrd.w #5,D2","lsrd.w #5,D3","lsrd.w #5,D4","lsrd.w #5,D5","lsrd.w #5,D6","lsrd.w #5,D7",
		"roxrd.w #5,D0","roxrd.w #5,D1","roxrd.w #5,D2","roxrd.w #5,D3","roxrd.w #5,D4","roxrd.w #5,D5",
		"roxrd.w #5,D6","roxrd.w #5,D7","rord.w #5,D0","rord.w #5,D1","rord.w #5,D2","rord.w #5,D3",
		"rord.w #5,D4","rord.w #5,D5","rord.w #5,D6","rord.w #5,D7","asrd.w D5,D0","asrd.w D5,D1",
		"asrd.w D5,D2","asrd.w D5,D3","asrd.w D5,D4","asrd.w D5,D5","asrd.w D5,D6","asrd.w D5,D7",
		"lsrd.w D5,D0","lsrd.w D5,D1","lsrd.w D5,D2","lsrd.w D5,D3","lsrd.w D5,D4","lsrd.w D5,D5",
		"lsrd.w D5,D6","lsrd.w D5,D7","roxrd.w D5,D0","roxrd.w D5,D1","roxrd.w D5,D2","roxrd.w D5,D3",
		"roxrd.w D5,D4","roxrd.w D5,D5","roxrd.w D5,D6","roxrd.w D5,D7","rord.w D5,D0","rord.w D5,D1",
		"rord.w D5,D2","rord.w D5,D3","rord.w D5,D4","rord.w D5,D5","rord.w D5,D6","rord.w D5,D7" };
	static const char *opATable03AA[] = { "asrd.l #5,D0","asrd.l #5,D1","asrd.l #5,D2","asrd.l #5,D3",
		"asrd.l #5,D4","asrd.l #5,D5","asrd.l #5,D6","asrd.l #5,D7","lsrd.l #5,D0","lsrd.l #5,D1",
		"lsrd.l #5,D2","lsrd.l #5,D3","lsrd.l #5,D4","lsrd.l #5,D5","lsrd.l #5,D6","lsrd.l #5,D7",
		"roxrd.l #5,D0","roxrd.l #5,D1","roxrd.l #5,D2","roxrd.l #5,D3","roxrd.l #5,D4","roxrd.l #5,D5",
		"roxrd.l #5,D6","roxrd.l #5,D7","rord.l #5,D0","rord.l #5,D1","rord.l #5,D2","rord.l #5,D3",
		"rord.l #5,D4","rord.l #5,D5","rord.l #5,D6","rord.l #5,D7","asrd.l D5,D0","asrd.l D5,D1",
		"asrd.l D5,D2","asrd.l D5,D3","asrd.l D5,D4","asrd.l D5,D5","asrd.l D5,D6","asrd.l D5,D7",
		"lsrd.l D5,D0","lsrd.l D5,D1","lsrd.l D5,D2","lsrd.l D5,D3","lsrd.l D5,D4","lsrd.l D5,D5",
		"lsrd.l D5,D6","lsrd.l D5,D7","roxrd.l D5,D0","roxrd.l D5,D1","roxrd.l D5,D2","roxrd.l D5,D3",
		"roxrd.l D5,D4","roxrd.l D5,D5","roxrd.l D5,D6","roxrd.l D5,D7","rord.l D5,D0","rord.l D5,D1",
		"rord.l D5,D2","rord.l D5,D3","rord.l D5,D4","rord.l D5,D5","rord.l D5,D6","rord.l D5,D7" };
	static const char *opATable03AC[] = { "asld.b #5,D0","asld.b #5,D1","asld.b #5,D2","asld.b #5,D3",
		"asld.b #5,D4","asld.b #5,D5","asld.b #5,D6","asld.b #5,D7","lsld.b #5,D0","lsld.b #5,D1",
		"lsld.b #5,D2","lsld.b #5,D3","lsld.b #5,D4","lsld.b #5,D5","lsld.b #5,D6","lsld.b #5,D7",
		"roxld.b #5,D0","roxld.b #5,D1","roxld.b #5,D2","roxld.b #5,D3","roxld.b #5,D4","roxld.b #5,D5",
		"roxld.b #5,D6","roxld.b #5,D7","rold.b #5,D0","rold.b #5,D1","rold.b #5,D2","rold.b #5,D3",
		"rold.b #5,D4","rold.b #5,D5","rold.b #5,D6","rold.b #5,D7","asld.b D5,D0","asld.b D5,D1",
		"asld.b D5,D2","asld.b D5,D3","asld.b D5,D4","asld.b D5,D5","asld.b D5,D6","asld.b D5,D7",
		"lsld.b D5,D0","lsld.b D5,D1","lsld.b D5,D2","lsld.b D5,D3","lsld.b D5,D4","lsld.b D5,D5",
		"lsld.b D5,D6","lsld.b D5,D7","roxld.b D5,D0","roxld.b D5,D1","roxld.b D5,D2","roxld.b D5,D3",
		"roxld.b D5,D4","roxld.b D5,D5","roxld.b D5,D6","roxld.b D5,D7","rold.b D5,D0","rold.b D5,D1",
		"rold.b D5,D2","rold.b D5,D3","rold.b D5,D4","rold.b D5,D5","rold.b D5,D6","rold.b D5,D7" };
	static const char *opATable03AD[] = { "asld.w #5,D0","asld.w #5,D1","asld.w #5,D2","asld.w #5,D3",
		"asld.w #5,D4","asld.w #5,D5","asld.w #5,D6","asld.w #5,D7","lsld.w #5,D0","lsld.w #5,D1",
		"lsld.w #5,D2","lsld.w #5,D3","lsld.w #5,D4","lsld.w #5,D5","lsld.w #5,D6","lsld.w #5,D7",
		"roxld.w #5,D0","roxld.w #5,D1","roxld.w #5,D2","roxld.w #5,D3","roxld.w #5,D4","roxld.w #5,D5",
		"roxld.w #5,D6","roxld.w #5,D7","rold.w #5,D0","rold.w #5,D1","rold.w #5,D2","rold.w #5,D3",
		"rold.w #5,D4","rold.w #5,D5","rold.w #5,D6","rold.w #5,D7","asld.w D5,D0","asld.w D5,D1",
		"asld.w D5,D2","asld.w D5,D3","asld.w D5,D4","asld.w D5,D5","asld.w D5,D6","asld.w D5,D7",
		"lsld.w D5,D0","lsld.w D5,D1","lsld.w D5,D2","lsld.w D5,D3","lsld.w D5,D4","lsld.w D5,D5",
		"lsld.w D5,D6","lsld.w D5,D7","roxld.w D5,D0","roxld.w D5,D1","roxld.w D5,D2","roxld.w D5,D3",
		"roxld.w D5,D4","roxld.w D5,D5","roxld.w D5,D6","roxld.w D5,D7","rold.w D5,D0","rold.w D5,D1",
		"rold.w D5,D2","rold.w D5,D3","rold.w D5,D4","rold.w D5,D5","rold.w D5,D6","rold.w D5,D7" };
	static const char *opATable03AE[] = { "asld.l #5,D0","asld.l #5,D1","asld.l #5,D2","asld.l #5,D3",
		"asld.l #5,D4","asld.l #5,D5","asld.l #5,D6","asld.l #5,D7","lsld.l #5,D0","lsld.l #5,D1",
		"lsld.l #5,D2","lsld.l #5,D3","lsld.l #5,D4","lsld.l #5,D5","lsld.l #5,D6","lsld.l #5,D7",
		"roxld.l #5,D0","roxld.l #5,D1","roxld.l #5,D2","roxld.l #5,D3","roxld.l #5,D4","roxld.l #5,D5",
		"roxld.l #5,D6","roxld.l #5,D7","rold.l #5,D0","rold.l #5,D1","rold.l #5,D2","rold.l #5,D3",
		"rold.l #5,D4","rold.l #5,D5","rold.l #5,D6","rold.l #5,D7","asld.l D5,D0","asld.l D5,D1",
		"asld.l D5,D2","asld.l D5,D3","asld.l D5,D4","asld.l D5,D5","asld.l D5,D6","asld.l D5,D7",
		"lsld.l D5,D0","lsld.l D5,D1","lsld.l D5,D2","lsld.l D5,D3","lsld.l D5,D4","lsld.l D5,D5",
		"lsld.l D5,D6","lsld.l D5,D7","roxld.l D5,D0","roxld.l D5,D1","roxld.l D5,D2","roxld.l D5,D3",
		"roxld.l D5,D4","roxld.l D5,D5","roxld.l D5,D6","roxld.l D5,D7","rold.l D5,D0","rold.l D5,D1",
		"rold.l D5,D2","rold.l D5,D3","rold.l D5,D4","rold.l D5,D5","rold.l D5,D6","rold.l D5,D7" };
	static const char *opATable03B0[] = { "asrd.b #6,D0","asrd.b #6,D1","asrd.b #6,D2","asrd.b #6,D3",
		"asrd.b #6,D4","asrd.b #6,D5","asrd.b #6,D6","asrd.b #6,D7","lsrd.b #6,D0","lsrd.b #6,D1",
		"lsrd.b #6,D2","lsrd.b #6,D3","lsrd.b #6,D4","lsrd.b #6,D5","lsrd.b #6,D6","lsrd.b #6,D7",
		"roxrd.b #6,D0","roxrd.b #6,D1","roxrd.b #6,D2","roxrd.b #6,D3","roxrd.b #6,D4","roxrd.b #6,D5",
		"roxrd.b #6,D6","roxrd.b #6,D7","rord.b #6,D0","rord.b #6,D1","rord.b #6,D2","rord.b #6,D3",
		"rord.b #6,D4","rord.b #6,D5","rord.b #6,D6","rord.b #6,D7","asrd.b D6,D0","asrd.b D6,D1",
		"asrd.b D6,D2","asrd.b D6,D3","asrd.b D6,D4","asrd.b D6,D5","asrd.b D6,D6","asrd.b D6,D7",
		"lsrd.b D6,D0","lsrd.b D6,D1","lsrd.b D6,D2","lsrd.b D6,D3","lsrd.b D6,D4","lsrd.b D6,D5",
		"lsrd.b D6,D6","lsrd.b D6,D7","roxrd.b D6,D0","roxrd.b D6,D1","roxrd.b D6,D2","roxrd.b D6,D3",
		"roxrd.b D6,D4","roxrd.b D6,D5","roxrd.b D6,D6","roxrd.b D6,D7","rord.b D6,D0","rord.b D6,D1",
		"rord.b D6,D2","rord.b D6,D3","rord.b D6,D4","rord.b D6,D5","rord.b D6,D6","rord.b D6,D7" };
	static const char *opATable03B1[] = { "asrd.w #6,D0","asrd.w #6,D1","asrd.w #6,D2","asrd.w #6,D3",
		"asrd.w #6,D4","asrd.w #6,D5","asrd.w #6,D6","asrd.w #6,D7","lsrd.w #6,D0","lsrd.w #6,D1",
		"lsrd.w #6,D2","lsrd.w #6,D3","lsrd.w #6,D4","lsrd.w #6,D5","lsrd.w #6,D6","lsrd.w #6,D7",
		"roxrd.w #6,D0","roxrd.w #6,D1","roxrd.w #6,D2","roxrd.w #6,D3","roxrd.w #6,D4","roxrd.w #6,D5",
		"roxrd.w #6,D6","roxrd.w #6,D7","rord.w #6,D0","rord.w #6,D1","rord.w #6,D2","rord.w #6,D3",
		"rord.w #6,D4","rord.w #6,D5","rord.w #6,D6","rord.w #6,D7","asrd.w D6,D0","asrd.w D6,D1",
		"asrd.w D6,D2","asrd.w D6,D3","asrd.w D6,D4","asrd.w D6,D5","asrd.w D6,D6","asrd.w D6,D7",
		"lsrd.w D6,D0","lsrd.w D6,D1","lsrd.w D6,D2","lsrd.w D6,D3","lsrd.w D6,D4","lsrd.w D6,D5",
		"lsrd.w D6,D6","lsrd.w D6,D7","roxrd.w D6,D0","roxrd.w D6,D1","roxrd.w D6,D2","roxrd.w D6,D3",
		"roxrd.w D6,D4","roxrd.w D6,D5","roxrd.w D6,D6","roxrd.w D6,D7","rord.w D6,D0","rord.w D6,D1",
		"rord.w D6,D2","rord.w D6,D3","rord.w D6,D4","rord.w D6,D5","rord.w D6,D6","rord.w D6,D7" };
	static const char *opATable03B2[] = { "asrd.l #6,D0","asrd.l #6,D1","asrd.l #6,D2","asrd.l #6,D3",
		"asrd.l #6,D4","asrd.l #6,D5","asrd.l #6,D6","asrd.l #6,D7","lsrd.l #6,D0","lsrd.l #6,D1",
		"lsrd.l #6,D2","lsrd.l #6,D3","lsrd.l #6,D4","lsrd.l #6,D5","lsrd.l #6,D6","lsrd.l #6,D7",
		"roxrd.l #6,D0","roxrd.l #6,D1","roxrd.l #6,D2","roxrd.l #6,D3","roxrd.l #6,D4","roxrd.l #6,D5",
		"roxrd.l #6,D6","roxrd.l #6,D7","rord.l #6,D0","rord.l #6,D1","rord.l #6,D2","rord.l #6,D3",
		"rord.l #6,D4","rord.l #6,D5","rord.l #6,D6","rord.l #6,D7","asrd.l D6,D0","asrd.l D6,D1",
		"asrd.l D6,D2","asrd.l D6,D3","asrd.l D6,D4","asrd.l D6,D5","asrd.l D6,D6","asrd.l D6,D7",
		"lsrd.l D6,D0","lsrd.l D6,D1","lsrd.l D6,D2","lsrd.l D6,D3","lsrd.l D6,D4","lsrd.l D6,D5",
		"lsrd.l D6,D6","lsrd.l D6,D7","roxrd.l D6,D0","roxrd.l D6,D1","roxrd.l D6,D2","roxrd.l D6,D3",
		"roxrd.l D6,D4","roxrd.l D6,D5","roxrd.l D6,D6","roxrd.l D6,D7","rord.l D6,D0","rord.l D6,D1",
		"rord.l D6,D2","rord.l D6,D3","rord.l D6,D4","rord.l D6,D5","rord.l D6,D6","rord.l D6,D7" };
	static const char *opATable03B4[] = { "asld.b #6,D0","asld.b #6,D1","asld.b #6,D2","asld.b #6,D3",
		"asld.b #6,D4","asld.b #6,D5","asld.b #6,D6","asld.b #6,D7","lsld.b #6,D0","lsld.b #6,D1",
		"lsld.b #6,D2","lsld.b #6,D3","lsld.b #6,D4","lsld.b #6,D5","lsld.b #6,D6","lsld.b #6,D7",
		"roxld.b #6,D0","roxld.b #6,D1","roxld.b #6,D2","roxld.b #6,D3","roxld.b #6,D4","roxld.b #6,D5",
		"roxld.b #6,D6","roxld.b #6,D7","rold.b #6,D0","rold.b #6,D1","rold.b #6,D2","rold.b #6,D3",
		"rold.b #6,D4","rold.b #6,D5","rold.b #6,D6","rold.b #6,D7","asld.b D6,D0","asld.b D6,D1",
		"asld.b D6,D2","asld.b D6,D3","asld.b D6,D4","asld.b D6,D5","asld.b D6,D6","asld.b D6,D7",
		"lsld.b D6,D0","lsld.b D6,D1","lsld.b D6,D2","lsld.b D6,D3","lsld.b D6,D4","lsld.b D6,D5",
		"lsld.b D6,D6","lsld.b D6,D7","roxld.b D6,D0","roxld.b D6,D1","roxld.b D6,D2","roxld.b D6,D3",
		"roxld.b D6,D4","roxld.b D6,D5","roxld.b D6,D6","roxld.b D6,D7","rold.b D6,D0","rold.b D6,D1",
		"rold.b D6,D2","rold.b D6,D3","rold.b D6,D4","rold.b D6,D5","rold.b D6,D6","rold.b D6,D7" };
	static const char *opATable03B5[] = { "asld.w #6,D0","asld.w #6,D1","asld.w #6,D2","asld.w #6,D3",
		"asld.w #6,D4","asld.w #6,D5","asld.w #6,D6","asld.w #6,D7","lsld.w #6,D0","lsld.w #6,D1",
		"lsld.w #6,D2","lsld.w #6,D3","lsld.w #6,D4","lsld.w #6,D5","lsld.w #6,D6","lsld.w #6,D7",
		"roxld.w #6,D0","roxld.w #6,D1","roxld.w #6,D2","roxld.w #6,D3","roxld.w #6,D4","roxld.w #6,D5",
		"roxld.w #6,D6","roxld.w #6,D7","rold.w #6,D0","rold.w #6,D1","rold.w #6,D2","rold.w #6,D3",
		"rold.w #6,D4","rold.w #6,D5","rold.w #6,D6","rold.w #6,D7","asld.w D6,D0","asld.w D6,D1",
		"asld.w D6,D2","asld.w D6,D3","asld.w D6,D4","asld.w D6,D5","asld.w D6,D6","asld.w D6,D7",
		"lsld.w D6,D0","lsld.w D6,D1","lsld.w D6,D2","lsld.w D6,D3","lsld.w D6,D4","lsld.w D6,D5",
		"lsld.w D6,D6","lsld.w D6,D7","roxld.w D6,D0","roxld.w D6,D1","roxld.w D6,D2","roxld.w D6,D3",
		"roxld.w D6,D4","roxld.w D6,D5","roxld.w D6,D6","roxld.w D6,D7","rold.w D6,D0","rold.w D6,D1",
		"rold.w D6,D2","rold.w D6,D3","rold.w D6,D4","rold.w D6,D5","rold.w D6,D6","rold.w D6,D7" };
	static const char *opATable03B6[] = { "asld.l #6,D0","asld.l #6,D1","asld.l #6,D2","asld.l #6,D3",
		"asld.l #6,D4","asld.l #6,D5","asld.l #6,D6","asld.l #6,D7","lsld.l #6,D0","lsld.l #6,D1",
		"lsld.l #6,D2","lsld.l #6,D3","lsld.l #6,D4","lsld.l #6,D5","lsld.l #6,D6","lsld.l #6,D7",
		"roxld.l #6,D0","roxld.l #6,D1","roxld.l #6,D2","roxld.l #6,D3","roxld.l #6,D4","roxld.l #6,D5",
		"roxld.l #6,D6","roxld.l #6,D7","rold.l #6,D0","rold.l #6,D1","rold.l #6,D2","rold.l #6,D3",
		"rold.l #6,D4","rold.l #6,D5","rold.l #6,D6","rold.l #6,D7","asld.l D6,D0","asld.l D6,D1",
		"asld.l D6,D2","asld.l D6,D3","asld.l D6,D4","asld.l D6,D5","asld.l D6,D6","asld.l D6,D7",
		"lsld.l D6,D0","lsld.l D6,D1","lsld.l D6,D2","lsld.l D6,D3","lsld.l D6,D4","lsld.l D6,D5",
		"lsld.l D6,D6","lsld.l D6,D7","roxld.l D6,D0","roxld.l D6,D1","roxld.l D6,D2","roxld.l D6,D3",
		"roxld.l D6,D4","roxld.l D6,D5","roxld.l D6,D6","roxld.l D6,D7","rold.l D6,D0","rold.l D6,D1",
		"rold.l D6,D2","rold.l D6,D3","rold.l D6,D4","rold.l D6,D5","rold.l D6,D6","rold.l D6,D7" };
	static const char *opATable03B8[] = { "asrd.b #7,D0","asrd.b #7,D1","asrd.b #7,D2","asrd.b #7,D3",
		"asrd.b #7,D4","asrd.b #7,D5","asrd.b #7,D6","asrd.b #7,D7","lsrd.b #7,D0","lsrd.b #7,D1",
		"lsrd.b #7,D2","lsrd.b #7,D3","lsrd.b #7,D4","lsrd.b #7,D5","lsrd.b #7,D6","lsrd.b #7,D7",
		"roxrd.b #7,D0","roxrd.b #7,D1","roxrd.b #7,D2","roxrd.b #7,D3","roxrd.b #7,D4","roxrd.b #7,D5",
		"roxrd.b #7,D6","roxrd.b #7,D7","rord.b #7,D0","rord.b #7,D1","rord.b #7,D2","rord.b #7,D3",
		"rord.b #7,D4","rord.b #7,D5","rord.b #7,D6","rord.b #7,D7","asrd.b D7,D0","asrd.b D7,D1",
		"asrd.b D7,D2","asrd.b D7,D3","asrd.b D7,D4","asrd.b D7,D5","asrd.b D7,D6","asrd.b D7,D7",
		"lsrd.b D7,D0","lsrd.b D7,D1","lsrd.b D7,D2","lsrd.b D7,D3","lsrd.b D7,D4","lsrd.b D7,D5",
		"lsrd.b D7,D6","lsrd.b D7,D7","roxrd.b D7,D0","roxrd.b D7,D1","roxrd.b D7,D2","roxrd.b D7,D3",
		"roxrd.b D7,D4","roxrd.b D7,D5","roxrd.b D7,D6","roxrd.b D7,D7","rord.b D7,D0","rord.b D7,D1",
		"rord.b D7,D2","rord.b D7,D3","rord.b D7,D4","rord.b D7,D5","rord.b D7,D6","rord.b D7,D7" };
	static const char *opATable03B9[] = { "asrd.w #7,D0","asrd.w #7,D1","asrd.w #7,D2","asrd.w #7,D3",
		"asrd.w #7,D4","asrd.w #7,D5","asrd.w #7,D6","asrd.w #7,D7","lsrd.w #7,D0","lsrd.w #7,D1",
		"lsrd.w #7,D2","lsrd.w #7,D3","lsrd.w #7,D4","lsrd.w #7,D5","lsrd.w #7,D6","lsrd.w #7,D7",
		"roxrd.w #7,D0","roxrd.w #7,D1","roxrd.w #7,D2","roxrd.w #7,D3","roxrd.w #7,D4","roxrd.w #7,D5",
		"roxrd.w #7,D6","roxrd.w #7,D7","rord.w #7,D0","rord.w #7,D1","rord.w #7,D2","rord.w #7,D3",
		"rord.w #7,D4","rord.w #7,D5","rord.w #7,D6","rord.w #7,D7","asrd.w D7,D0","asrd.w D7,D1",
		"asrd.w D7,D2","asrd.w D7,D3","asrd.w D7,D4","asrd.w D7,D5","asrd.w D7,D6","asrd.w D7,D7",
		"lsrd.w D7,D0","lsrd.w D7,D1","lsrd.w D7,D2","lsrd.w D7,D3","lsrd.w D7,D4","lsrd.w D7,D5",
		"lsrd.w D7,D6","lsrd.w D7,D7","roxrd.w D7,D0","roxrd.w D7,D1","roxrd.w D7,D2","roxrd.w D7,D3",
		"roxrd.w D7,D4","roxrd.w D7,D5","roxrd.w D7,D6","roxrd.w D7,D7","rord.w D7,D0","rord.w D7,D1",
		"rord.w D7,D2","rord.w D7,D3","rord.w D7,D4","rord.w D7,D5","rord.w D7,D6","rord.w D7,D7" };
	static const char *opATable03BA[] = { "asrd.l #7,D0","asrd.l #7,D1","asrd.l #7,D2","asrd.l #7,D3",
		"asrd.l #7,D4","asrd.l #7,D5","asrd.l #7,D6","asrd.l #7,D7","lsrd.l #7,D0","lsrd.l #7,D1",
		"lsrd.l #7,D2","lsrd.l #7,D3","lsrd.l #7,D4","lsrd.l #7,D5","lsrd.l #7,D6","lsrd.l #7,D7",
		"roxrd.l #7,D0","roxrd.l #7,D1","roxrd.l #7,D2","roxrd.l #7,D3","roxrd.l #7,D4","roxrd.l #7,D5",
		"roxrd.l #7,D6","roxrd.l #7,D7","rord.l #7,D0","rord.l #7,D1","rord.l #7,D2","rord.l #7,D3",
		"rord.l #7,D4","rord.l #7,D5","rord.l #7,D6","rord.l #7,D7","asrd.l D7,D0","asrd.l D7,D1",
		"asrd.l D7,D2","asrd.l D7,D3","asrd.l D7,D4","asrd.l D7,D5","asrd.l D7,D6","asrd.l D7,D7",
		"lsrd.l D7,D0","lsrd.l D7,D1","lsrd.l D7,D2","lsrd.l D7,D3","lsrd.l D7,D4","lsrd.l D7,D5",
		"lsrd.l D7,D6","lsrd.l D7,D7","roxrd.l D7,D0","roxrd.l D7,D1","roxrd.l D7,D2","roxrd.l D7,D3",
		"roxrd.l D7,D4","roxrd.l D7,D5","roxrd.l D7,D6","roxrd.l D7,D7","rord.l D7,D0","rord.l D7,D1",
		"rord.l D7,D2","rord.l D7,D3","rord.l D7,D4","rord.l D7,D5","rord.l D7,D6","rord.l D7,D7" };
	static const char *opATable03BC[] = { "asld.b #7,D0","asld.b #7,D1","asld.b #7,D2","asld.b #7,D3",
		"asld.b #7,D4","asld.b #7,D5","asld.b #7,D6","asld.b #7,D7","lsld.b #7,D0","lsld.b #7,D1",
		"lsld.b #7,D2","lsld.b #7,D3","lsld.b #7,D4","lsld.b #7,D5","lsld.b #7,D6","lsld.b #7,D7",
		"roxld.b #7,D0","roxld.b #7,D1","roxld.b #7,D2","roxld.b #7,D3","roxld.b #7,D4","roxld.b #7,D5",
		"roxld.b #7,D6","roxld.b #7,D7","rold.b #7,D0","rold.b #7,D1","rold.b #7,D2","rold.b #7,D3",
		"rold.b #7,D4","rold.b #7,D5","rold.b #7,D6","rold.b #7,D7","asld.b D7,D0","asld.b D7,D1",
		"asld.b D7,D2","asld.b D7,D3","asld.b D7,D4","asld.b D7,D5","asld.b D7,D6","asld.b D7,D7",
		"lsld.b D7,D0","lsld.b D7,D1","lsld.b D7,D2","lsld.b D7,D3","lsld.b D7,D4","lsld.b D7,D5",
		"lsld.b D7,D6","lsld.b D7,D7","roxld.b D7,D0","roxld.b D7,D1","roxld.b D7,D2","roxld.b D7,D3",
		"roxld.b D7,D4","roxld.b D7,D5","roxld.b D7,D6","roxld.b D7,D7","rold.b D7,D0","rold.b D7,D1",
		"rold.b D7,D2","rold.b D7,D3","rold.b D7,D4","rold.b D7,D5","rold.b D7,D6","rold.b D7,D7" };
	static const char *opATable03BD[] = { "asld.w #7,D0","asld.w #7,D1","asld.w #7,D2","asld.w #7,D3",
		"asld.w #7,D4","asld.w #7,D5","asld.w #7,D6","asld.w #7,D7","lsld.w #7,D0","lsld.w #7,D1",
		"lsld.w #7,D2","lsld.w #7,D3","lsld.w #7,D4","lsld.w #7,D5","lsld.w #7,D6","lsld.w #7,D7",
		"roxld.w #7,D0","roxld.w #7,D1","roxld.w #7,D2","roxld.w #7,D3","roxld.w #7,D4","roxld.w #7,D5",
		"roxld.w #7,D6","roxld.w #7,D7","rold.w #7,D0","rold.w #7,D1","rold.w #7,D2","rold.w #7,D3",
		"rold.w #7,D4","rold.w #7,D5","rold.w #7,D6","rold.w #7,D7","asld.w D7,D0","asld.w D7,D1",
		"asld.w D7,D2","asld.w D7,D3","asld.w D7,D4","asld.w D7,D5","asld.w D7,D6","asld.w D7,D7",
		"lsld.w D7,D0","lsld.w D7,D1","lsld.w D7,D2","lsld.w D7,D3","lsld.w D7,D4","lsld.w D7,D5",
		"lsld.w D7,D6","lsld.w D7,D7","roxld.w D7,D0","roxld.w D7,D1","roxld.w D7,D2","roxld.w D7,D3",
		"roxld.w D7,D4","roxld.w D7,D5","roxld.w D7,D6","roxld.w D7,D7","rold.w D7,D0","rold.w D7,D1",
		"rold.w D7,D2","rold.w D7,D3","rold.w D7,D4","rold.w D7,D5","rold.w D7,D6","rold.w D7,D7" };
	static const char *opATable03BE[] = { "asld.l #7,D0","asld.l #7,D1","asld.l #7,D2","asld.l #7,D3",
		"asld.l #7,D4","asld.l #7,D5","asld.l #7,D6","asld.l #7,D7","lsld.l #7,D0","lsld.l #7,D1",
		"lsld.l #7,D2","lsld.l #7,D3","lsld.l #7,D4","lsld.l #7,D5","lsld.l #7,D6","lsld.l #7,D7",
		"roxld.l #7,D0","roxld.l #7,D1","roxld.l #7,D2","roxld.l #7,D3","roxld.l #7,D4","roxld.l #7,D5",
		"roxld.l #7,D6","roxld.l #7,D7","rold.l #7,D0","rold.l #7,D1","rold.l #7,D2","rold.l #7,D3",
		"rold.l #7,D4","rold.l #7,D5","rold.l #7,D6","rold.l #7,D7","asld.l D7,D0","asld.l D7,D1",
		"asld.l D7,D2","asld.l D7,D3","asld.l D7,D4","asld.l D7,D5","asld.l D7,D6","asld.l D7,D7",
		"lsld.l D7,D0","lsld.l D7,D1","lsld.l D7,D2","lsld.l D7,D3","lsld.l D7,D4","lsld.l D7,D5",
		"lsld.l D7,D6","lsld.l D7,D7","roxld.l D7,D0","roxld.l D7,D1","roxld.l D7,D2","roxld.l D7,D3",
		"roxld.l D7,D4","roxld.l D7,D5","roxld.l D7,D6","roxld.l D7,D7","rold.l D7,D0","rold.l D7,D1",
		"rold.l D7,D2","rold.l D7,D3","rold.l D7,D4","rold.l D7,D5","rold.l D7,D6","rold.l D7,D7" };
	static const char *opBTable00[] = { "ori.b #[DB3],[AM2]","ori.w #[DW3],[AM2]","ori.l #[DL3],[AM2]",
		NULL,"btst D0,[AM2]","bchg D0,[AM2]","bclr D0,[AM2]","bset D0,[AM2]","andi.b #[DB3],[AM2]",
		"andi.w #[DW3],[AM2]","andi.l #[DL3],[AM2]",NULL,"btst D1,[AM2]","bchg D1,[AM2]","bclr D1,[AM2]",
		"bset D1,[AM2]","subi.b #[DB3],[AM2]","subi.w #[DW3],[AM2]","subi.l #[DL3],[AM2]",NULL,
		"btst D2,[AM2]","bchg D2,[AM2]","bclr D2,[AM2]","bset D2,[AM2]","addi.b #[DB3],[AM2]",
		"addi.w #[DW3],[AM2]","addi.l #[DL3],[AM2]",NULL,"btst D3,[AM2]","bchg D3,[AM2]","bclr D3,[AM2]",
		"bset D3,[AM2]","btst #[BN3],[AM2]","bchg #[BN3],[AM2]","bclr #[BN3],[AM2]","bset #[BN3],[AM2]",
		"btst D4,[AM2]","bchg D4,[AM2]","bclr D4,[AM2]","bset D4,[AM2]","eori.b #[DB3],[AM2]",
		"eori.w #[DW3],[AM2]","eori.l #[DL3],[AM2]",NULL,"btst D5,[AM2]","bchg D5,[AM2]","bclr D5,[AM2]",
		"bset D5,[AM2]","cmpi.b #[DB3],[AM2]","cmpi.w #[DW3],[AM2]","cmpi.l #[DL3],[AM2]",NULL,
		"btst D6,[AM2]","bchg D6,[AM2]","bclr D6,[AM2]","bset D6,[AM2]",NULL,NULL,NULL,NULL,
		"btst D7,[AM2]","bchg D7,[AM2]","bclr D7,[AM2]","bset D7,[AM2]" };
	static const char *opBTable02[] = { "movea.l as,A0",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.l as,A1",
		NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.l as,A2",NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		"movea.l as,A3",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.l as,A4",NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,"movea.l as,A5",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.l as,A6",NULL,
		NULL,NULL,NULL,NULL,NULL,NULL,"movea.l as,A7" };
	static const char *opBTable03[] = { "movea.w as,A0",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.w as,A1",
		NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.w as,A2",NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		"movea.w as,A3",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.w as,A4",NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,"movea.w as,A5",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"movea.w as,A6",NULL,
		NULL,NULL,NULL,NULL,NULL,NULL,"movea.w as,A7" };
	static const char *opBTable04[] = { "negx.b [AM2]","negx.w [AM2]","negx.l [AM2]",NULL,NULL,NULL,
		"chk.w [AM2],D0","lea [AM2],A0","clr.b [AM2]","clr.w [AM2]","clr.l [AM2]",NULL,NULL,
		NULL,"chk.w [AM2],D1","lea [AM2],A1","neg.b [AM2]","neg.w [AM2]","neg.l [AM2]","move [AM2],CCR",
		NULL,NULL,"chk.w [AM2],D2","lea [AM2],A2","not.b [AM2]","not.w [AM2]","not.l [AM2]",
		"move [AM2],SR",NULL,NULL,"chk.w [AM2],D3","lea [AM2],A3","nbcd [AM2]","pea [AM2]","movem.w [RL3],[AM2]",
		"movem.l [RL3],[AM2]",NULL,NULL,"chk.w [AM2],D4","lea [AM2],A4","tst.b [AM2]","tst.w [AM2]",
		"tst.l [AM2]","tas [AM2]",NULL,NULL,"chk.w [AM2],D5","lea [AM2],A5",NULL,NULL,"movem.w [AM2],[RL3]",
		"movem.l [AM2],[RL3]",NULL,NULL,"chk.w [AM2],D6","lea [AM2],A6",NULL,NULL,"jsr [AM2]",
		"jmp [AM2]",NULL,NULL,"chk.w [AM2],D7","lea [AM2],A7" };
	static const char *opBTable05[] = { "addq.b #8,[AM2]","addq.w #8,[AM2]","addq.l #8,[AM2]","st [AM2]",
		"subq.b #8,[AM2]","subq.w #8,[AM2]","subq.l #8,[AM2]","sf [AM2]","addq.b #1,[AM2]","addq.w #1,[AM2]",
		"addq.l #1,[AM2]","shi [AM2]","subq.b #1,[AM2]","subq.w #1,[AM2]","subq.l #1,[AM2]",
		"sls [AM2]","addq.b #2,[AM2]","addq.w #2,[AM2]","addq.l #2,[AM2]","scc [AM2]","subq.b #2,[AM2]",
		"subq.w #2,[AM2]","subq.l #2,[AM2]","scs [AM2]","addq.b #3,[AM2]","addq.w #3,[AM2]",
		"addq.l #3,[AM2]","sne [AM2]","subq.b #3,[AM2]","subq.w #3,[AM2]","subq.l #3,[AM2]",
		"seq [AM2]","addq.b #4,[AM2]","addq.w #4,[AM2]","addq.l #4,[AM2]","svc [AM2]","subq.b #4,[AM2]",
		"subq.w #4,[AM2]","subq.l #4,[AM2]","svs [AM2]","addq.b #5,[AM2]","addq.w #5,[AM2]",
		"addq.l #5,[AM2]","spl [AM2]","subq.b #5,[AM2]","subq.w #5,[AM2]","subq.l #5,[AM2]",
		"smi [AM2]","addq.b #6,[AM2]","addq.w #6,[AM2]","addq.l #6,[AM2]","sge [AM2]","subq.b #6,[AM2]",
		"subq.w #6,[AM2]","subq.l #6,[AM2]","slt [AM2]","addq.b #7,[AM2]","addq.w #7,[AM2]",
		"addq.l #7,[AM2]","sgt [AM2]","subq.b #7,[AM2]","subq.w #7,[AM2]","subq.l #7,[AM2]",
		"sle [AM2]" };
	static const char *opBTable08[] = { "or.b [AM2],D0","or.w [AM2],D0","or.l [AM2],D0","divu.w [AM2],D0",
		"or.b D0,[AM2]","or.w D0,[AM2]","or.l D0,[AM2]","divs.w [AM2],D0","or.b [AM2],D1","or.w [AM2],D1",
		"or.l [AM2],D1","divu.w [AM2],D1","or.b D1,[AM2]","or.w D1,[AM2]","or.l D1,[AM2]","divs.w [AM2],D1",
		"or.b [AM2],D2","or.w [AM2],D2","or.l [AM2],D2","divu.w [AM2],D2","or.b D2,[AM2]","or.w D2,[AM2]",
		"or.l D2,[AM2]","divs.w [AM2],D2","or.b [AM2],D3","or.w [AM2],D3","or.l [AM2],D3","divu.w [AM2],D3",
		"or.b D3,[AM2]","or.w D3,[AM2]","or.l D3,[AM2]","divs.w [AM2],D3","or.b [AM2],D4","or.w [AM2],D4",
		"or.l [AM2],D4","divu.w [AM2],D4","or.b D4,[AM2]","or.w D4,[AM2]","or.l D4,[AM2]","divs.w [AM2],D4",
		"or.b [AM2],D5","or.w [AM2],D5","or.l [AM2],D5","divu.w [AM2],D5","or.b D5,[AM2]","or.w D5,[AM2]",
		"or.l D5,[AM2]","divs.w [AM2],D5","or.b [AM2],D6","or.w [AM2],D6","or.l [AM2],D6","divu.w [AM2],D6",
		"or.b D6,[AM2]","or.w D6,[AM2]","or.l D6,[AM2]","divs.w [AM2],D6","or.b [AM2],D7","or.w [AM2],D7",
		"or.l [AM2],D7","divu.w [AM2],D7","or.b D7,[AM2]","or.w D7,[AM2]","or.l D7,[AM2]","divs.w [AM2],D7" };
	static const char *opBTable09[] = { "sub.b [AM2],D0","sub.w [AM2],D0","sub.l [AM2],D0","suba.w [AM2],A0",
		"sub.b D0,[AM2]","sub.w D0,[AM2]","sub.l D0,[AM2]","suba.l [AM2],A0","sub.b [AM2],D1",
		"sub.w [AM2],D1","sub.l [AM2],D1","suba.w [AM2],A1","sub.b D1,[AM2]","sub.w D1,[AM2]",
		"sub.l D1,[AM2]","suba.l [AM2],A1","sub.b [AM2],D2","sub.w [AM2],D2","sub.l [AM2],D2",
		"suba.w [AM2],A2","sub.b D2,[AM2]","sub.w D2,[AM2]","sub.l D2,[AM2]","suba.l [AM2],A2",
		"sub.b [AM2],D3","sub.w [AM2],D3","sub.l [AM2],D3","suba.w [AM2],A3","sub.b D3,[AM2]",
		"sub.w D3,[AM2]","sub.l D3,[AM2]","suba.l [AM2],A3","sub.b [AM2],D4","sub.w [AM2],D4",
		"sub.l [AM2],D4","suba.w [AM2],A4","sub.b D4,[AM2]","sub.w D4,[AM2]","sub.l D4,[AM2]",
		"suba.l [AM2],A4","sub.b [AM2],D5","sub.w [AM2],D5","sub.l [AM2],D5","suba.w [AM2],A5",
		"sub.b D5,[AM2]","sub.w D5,[AM2]","sub.l D5,[AM2]","suba.l [AM2],A5","sub.b [AM2],D6",
		"sub.w [AM2],D6","sub.l [AM2],D6","suba.w [AM2],A6","sub.b D6,[AM2]","sub.w D6,[AM2]",
		"sub.l D6,[AM2]","suba.l [AM2],A6","sub.b [AM2],D7","sub.w [AM2],D7","sub.l [AM2],D7",
		"suba.w [AM2],A7","sub.b D7,[AM2]","sub.w D7,[AM2]","sub.l D7,[AM2]","suba.l [AM2],A7" };
	static const char *opBTable0B[] = { "cmp.b [AM2],D0","cmp.w [AM2],D0","cmp.l [AM2],D0","cmpa.w [AM2],A0",
		"eor.b [AM2],D0","eor.w [AM2],D0","eor.l [AM2],D0","cmpa.l [AM2],A0","cmp.b [AM2],D1",
		"cmp.w [AM2],D1","cmp.l [AM2],D1","cmpa.w [AM2],A1","eor.b [AM2],D1","eor.w [AM2],D1",
		"eor.l [AM2],D1","cmpa.l [AM2],A1","cmp.b [AM2],D2","cmp.w [AM2],D2","cmp.l [AM2],D2",
		"cmpa.w [AM2],A2","eor.b [AM2],D2","eor.w [AM2],D2","eor.l [AM2],D2","cmpa.l [AM2],A2",
		"cmp.b [AM2],D3","cmp.w [AM2],D3","cmp.l [AM2],D3","cmpa.w [AM2],A3","eor.b [AM2],D3",
		"eor.w [AM2],D3","eor.l [AM2],D3","cmpa.l [AM2],A3","cmp.b [AM2],D4","cmp.w [AM2],D4",
		"cmp.l [AM2],D4","cmpa.w [AM2],A4","eor.b [AM2],D4","eor.w [AM2],D4","eor.l [AM2],D4",
		"cmpa.l [AM2],A4","cmp.b [AM2],D5","cmp.w [AM2],D5","cmp.l [AM2],D5","cmpa.w [AM2],A5",
		"eor.b [AM2],D5","eor.w [AM2],D5","eor.l [AM2],D5","cmpa.l [AM2],A5","cmp.b [AM2],D6",
		"cmp.w [AM2],D6","cmp.l [AM2],D6","cmpa.w [AM2],A6","eor.b [AM2],D6","eor.w [AM2],D6",
		"eor.l [AM2],D6","cmpa.l [AM2],A6","cmp.b [AM2],D7","cmp.w [AM2],D7","cmp.l [AM2],D7",
		"cmpa.w [AM2],A7","eor.b [AM2],D7","eor.w [AM2],D7","eor.l [AM2],D7","cmpa.l [AM2],A7" };
	static const char *opBTable0C[] = { "and.b [AM2],D0","and.w [AM2],D0","and.l [AM2],D0","mulu.w [AM2],D0",
		"and.b D0,[AM2]","and.w D0,[AM2]","and.l D0,[AM2]","muls.w [AM2],D0","and.b [AM2],D1",
		"and.w [AM2],D1","and.l [AM2],D1","mulu.w [AM2],D1","and.b D1,[AM2]","and.w D1,[AM2]",
		"and.l D1,[AM2]","muls.w [AM2],D1","and.b [AM2],D2","and.w [AM2],D2","and.l [AM2],D2",
		"mulu.w [AM2],D2","and.b D2,[AM2]","and.w D2,[AM2]","and.l D2,[AM2]","muls.w [AM2],D2",
		"and.b [AM2],D3","and.w [AM2],D3","and.l [AM2],D3","mulu.w [AM2],D3","and.b D3,[AM2]",
		"and.w D3,[AM2]","and.l D3,[AM2]","muls.w [AM2],D3","and.b [AM2],D4","and.w [AM2],D4",
		"and.l [AM2],D4","mulu.w [AM2],D4","and.b D4,[AM2]","and.w D4,[AM2]","and.l D4,[AM2]",
		"muls.w [AM2],D4","and.b [AM2],D5","and.w [AM2],D5","and.l [AM2],D5","mulu.w [AM2],D5",
		"and.b D5,[AM2]","and.w D5,[AM2]","and.l D5,[AM2]","muls.w [AM2],D5","and.b [AM2],D6",
		"and.w [AM2],D6","and.l [AM2],D6","mulu.w [AM2],D6","and.b D6,[AM2]","and.w D6,[AM2]",
		"and.l D6,[AM2]","muls.w [AM2],D6","and.b [AM2],D7","and.w [AM2],D7","and.l [AM2],D7",
		"mulu.w [AM2],D7","and.b D7,[AM2]","and.w D7,[AM2]","and.l D7,[AM2]","muls.w [AM2],D7" };
	static const char *opBTable0D[] = { "add.b [AM2],D0","add.w [AM2],D0","add.l [AM2],D0","adda.w [AM2],A0",
		"add.b D0,[AM2]","add.w D0,[AM2]","add.l D0,[AM2]","adda.l [AM2],A0","add.b [AM2],D1",
		"add.w [AM2],D1","add.l [AM2],D1","adda.w [AM2],A1","add.b D1,[AM2]","add.w D1,[AM2]",
		"add.l D1,[AM2]","adda.l [AM2],A1","add.b [AM2],D2","add.w [AM2],D2","add.l [AM2],D2",
		"adda.w [AM2],A2","add.b D2,[AM2]","add.w D2,[AM2]","add.l D2,[AM2]","adda.l [AM2],A2",
		"add.b [AM2],D3","add.w [AM2],D3","add.l [AM2],D3","adda.w [AM2],A3","add.b D3,[AM2]",
		"add.w D3,[AM2]","add.l D3,[AM2]","adda.l [AM2],A3","add.b [AM2],D4","add.w [AM2],D4",
		"add.l [AM2],D4","adda.w [AM2],A4","add.b D4,[AM2]","add.w D4,[AM2]","add.l D4,[AM2]",
		"adda.l [AM2],A4","add.b [AM2],D5","add.w [AM2],D5","add.l [AM2],D5","adda.w [AM2],A5",
		"add.b D5,[AM2]","add.w D5,[AM2]","add.l D5,[AM2]","adda.l [AM2],A5","add.b [AM2],D6",
		"add.w [AM2],D6","add.l [AM2],D6","adda.w [AM2],A6","add.b D6,[AM2]","add.w D6,[AM2]",
		"add.l D6,[AM2]","adda.l [AM2],A6","add.b [AM2],D7","add.w [AM2],D7","add.l [AM2],D7",
		"adda.w [AM2],A7","add.b D7,[AM2]","add.w D7,[AM2]","add.l D7,[AM2]","adda.l [AM2],A7" };
	static const char *opBTable0E[] = { "asrd [AM2]",NULL,NULL,NULL,"asld [AM2]",NULL,NULL,NULL,
		"lsrd [AM2]",NULL,NULL,NULL,"lsld [AM2]",NULL,NULL,NULL,"roxrd [AM2]",NULL,NULL,NULL,
		"roxld [AM2]",NULL,NULL,NULL,"rord [AM2]",NULL,NULL,NULL,"rold [AM2]" };
	static const char *brTable[] = { "bra [LV2]","bsr [LV2]","bhi [LV2]","bls [LV2]","bcc [LV2]","bcs [LV2]",
		"bne [LV2]","beq [LV2]","bvc [LV2]","bvs [LV2]","bpl [LV2]","bmi [LV2]","bge [LV2]",
		"blt [LV2]","bgt [LV2]","ble [LV2]" };

	int C68KDebug::Disassemble(UINT32 addr, char *mnemonic, char *operands)
	{
		// Read opcode head word
		UINT16 opcode = (UINT16)ReadMem(addr, 2);
		int offset = 2;

		const char *instr;
		char moveStr[50];
		char dataStr[20];
		INT8 i8;
		INT16 i16;
		INT32 i32;
		UINT8 u8;
		UINT16 u16;
		UINT32 u32;

		// First check for special cases that don't fit into op tables A & B
		UINT16 hd = (opcode>>12);
		UINT16 tl;
		switch (hd)
		{
			case 0x1:
				// move.b as,ad
				instr = "move.b [AM2],[AR1]";
				break;
			case 0x2:
				if ((opcode&0x01C0) == 0x0040)
				{
					// movea.l as,ad
					sprintf(moveStr, "movea.l [AM2],A%u", (opcode>>9)&0x7);
					instr = moveStr;
				}
				else
				{
					// move.l as,Ad
					instr = "move.l [AM2],[AR1]";
				}
				break;
			case 0x3:
				if ((opcode&0x01C0) == 0x0040)
				{
					// movea.w as,ad
					sprintf(moveStr, "movea.w [AM2],A%u", (opcode>>9)&0x7);
					instr = moveStr;
				}
				else
				{
					// move.w as,Ad
					instr = "move.w [AM2],[AR1]";
				}
				break;
			case 0x4:
				if ((opcode&0x0B80) == 0x0880)
				{
					// movem.z reg-list,a or movem.z a,reg-list
					u16 = (UINT16)ReadMem(addr + offset, 2);
					offset += 2;
					char sizeC = (opcode&0x40 ? 'l' : 'w');
					char regList[50];
					char *p = regList;
					int range = 0;
					for (unsigned r = 0; r < 17; r++)
					{
						// Get bit position of register r (if address mode is pre-decrement, then bits are in reverse order)
						unsigned bitPos = ((opcode&0x38) == 0x20 ? 15 - r : r);
						// Check given bit is set
						if (r < 16 && ((u16>>bitPos)&0x0001))
						{
							// Check not in middle of register range
							if (range == 0)
							{
								if (p > regList)
									*p++ = '/';
								// If address mode is pre-decrement, then reverse order of registers
								if (r < 8)
								{
									*p++ = 'D';
									*p++ = '0' + r;
								}
								else
								{
									*p++ = 'A';
									*p++ = '0' + r - 8;
								}
							}
							range++;
						}
						else 
						{
							if (range > 1)
							{
								// Close off a register range
								*p++ = '-';
								// If address mode is pre-decrement, then reverse order of registers
								unsigned prevR = r - 1;
								if (prevR < 8)
								{
									*p++ = 'D';
									*p++ = '0' + prevR;
								}
								else
								{
									*p++ = 'A';
									*p++ = '0' + prevR - 8;
								}
							}
							range = 0;
						}
					}
					*p++ = '\0';
					if (opcode&0x0400)
						sprintf(moveStr, "movem.%c [AM2],%s", sizeC, regList); 
					else
						sprintf(moveStr, "movem.%c %s,[AM2]", sizeC, regList); 
					instr = moveStr;
				}
				else
					instr = NULL;
				break;
			case 0x6:
				// bra label, bsr label && bCC label
				tl = (opcode>>8)&0xF;
				instr = brTable[tl];
				break;
			case 0x7:
				// moveq #data8,Dn
				if (opcode&0x100)
					goto invalid;
				u8 = (UINT8)opcode&0xFF;
				FormatData(dataStr, 1, u8);
				sprintf(moveStr, "moveq #%s,D%u", dataStr, (opcode>>9)&0x7);
				//i8 = (INT8)opcode&0xFF;
				//if (i8 < 0)
				//{
				//	FormatData(dataStr, 1, -i8);
				//	sprintf(moveStr, "moveq #-%s,D%u", dataStr, (opcode>>9)&0x7);
				//	//sprintf(moveStr, "moveq #-0x%02X,D%u", -i8, (opcode>>9)&0x7);
				//}
				//else
				//{
				//	FormatData(dataStr, 1, i8);
				//	sprintf(moveStr, "moveq #%s,D%u", dataStr, (opcode>>9)&0x7);
				//	//sprintf(moveStr, "moveq #0x%02X,D%u", i8, (opcode>>9)&0x7);
				//}
				instr = moveStr;
				break;
			default:
				instr = NULL;
				break;
		}

		// Next, try op table A if no match found
		if (instr == NULL)
		{
			UINT16 hdA = (opcode>>6);
			UINT16 tlA = opcode & 0x3F;
			switch (hdA)
			{
				case 0x0000: instr = (tlA == 0x3C ? "ori.b #[DB3],ccr" : NULL); break;
				case 0x0001: instr = (tlA == 0x3C ? "ori.w #[DW3],sr" : NULL); break;
				case 0x0004: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0004[tlA - 0x08] : NULL); break;
				case 0x0005: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0005[tlA - 0x08] : NULL); break;
				case 0x0006: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0006[tlA - 0x08] : NULL); break;
				case 0x0007: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0007[tlA - 0x08] : NULL); break;
				case 0x0008: instr = (tlA == 0x3C ? "andi.b #[DB3],ccr" : NULL); break;
				case 0x0009: instr = (tlA == 0x3C ? "andi.w #[DW3],sr" : NULL); break;
				case 0x000C: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable000C[tlA - 0x08] : NULL); break;
				case 0x000D: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable000D[tlA - 0x08] : NULL); break;
				case 0x000E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable000E[tlA - 0x08] : NULL); break;
				case 0x000F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable000F[tlA - 0x08] : NULL); break;
				case 0x0014: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0014[tlA - 0x08] : NULL); break;
				case 0x0015: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0015[tlA - 0x08] : NULL); break;
				case 0x0016: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0016[tlA - 0x08] : NULL); break;
				case 0x0017: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0017[tlA - 0x08] : NULL); break;
				case 0x001C: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable001C[tlA - 0x08] : NULL); break;
				case 0x001D: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable001D[tlA - 0x08] : NULL); break;
				case 0x001E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable001E[tlA - 0x08] : NULL); break;
				case 0x001F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable001F[tlA - 0x08] : NULL); break;
				case 0x0024: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0024[tlA - 0x08] : NULL); break;
				case 0x0025: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0025[tlA - 0x08] : NULL); break;
				case 0x0026: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0026[tlA - 0x08] : NULL); break;
				case 0x0027: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0027[tlA - 0x08] : NULL); break;
				case 0x0028: instr = (tlA == 0x3C ? "eori.b #[DB3],ccr" : NULL); break;
				case 0x0029: instr = (tlA == 0x3C ? "eori.w #[DW3],sr" : NULL); break;
				case 0x002C: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable002C[tlA - 0x08] : NULL); break;
				case 0x002D: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable002D[tlA - 0x08] : NULL); break;
				case 0x002E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable002E[tlA - 0x08] : NULL); break;
				case 0x002F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable002F[tlA - 0x08] : NULL); break;
				case 0x0034: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0034[tlA - 0x08] : NULL); break;
				case 0x0035: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0035[tlA - 0x08] : NULL); break;
				case 0x0036: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0036[tlA - 0x08] : NULL); break;
				case 0x0037: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0037[tlA - 0x08] : NULL); break;
				case 0x003C: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable003C[tlA - 0x08] : NULL); break;
				case 0x003D: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable003D[tlA - 0x08] : NULL); break;
				case 0x003E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable003E[tlA - 0x08] : NULL); break;
				case 0x003F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable003F[tlA - 0x08] : NULL); break;
				case 0x0121: instr = (tlA <= 0x07 ? opATable0121[tlA] : NULL); break;
				case 0x0122: instr = (tlA <= 0x07 ? opATable0122[tlA] : NULL); break;
				case 0x0123: instr = (tlA <= 0x07 ? opATable0123[tlA] : NULL); break;
				case 0x012B: instr = (tlA == 0x3C ? "illegal" : NULL); break;
				case 0x0139: instr = (tlA <= 0x37 ? opATable0139[tlA] : NULL); break;
				case 0x0143: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0143[tlA - 0x08] : NULL); break;
				case 0x0147: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0147[tlA - 0x08] : NULL); break;
				case 0x014B: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable014B[tlA - 0x08] : NULL); break;
				case 0x014F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable014F[tlA - 0x08] : NULL); break;
				case 0x0153: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0153[tlA - 0x08] : NULL); break;
				case 0x0157: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0157[tlA - 0x08] : NULL); break;
				case 0x015B: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable015B[tlA - 0x08] : NULL); break;
				case 0x015F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable015F[tlA - 0x08] : NULL); break;
				case 0x0163: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0163[tlA - 0x08] : NULL); break;
				case 0x0167: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0167[tlA - 0x08] : NULL); break;
				case 0x016B: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable016B[tlA - 0x08] : NULL); break;
				case 0x016F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable016F[tlA - 0x08] : NULL); break;
				case 0x0173: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0173[tlA - 0x08] : NULL); break;
				case 0x0177: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0177[tlA - 0x08] : NULL); break;
				case 0x017B: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable017B[tlA - 0x08] : NULL); break;
				case 0x017F: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable017F[tlA - 0x08] : NULL); break;
				case 0x0204: instr = (tlA <= 0x0F ? opATable0204[tlA] : NULL); break;
				case 0x020C: instr = (tlA <= 0x0F ? opATable020C[tlA] : NULL); break;
				case 0x0214: instr = (tlA <= 0x0F ? opATable0214[tlA] : NULL); break;
				case 0x021C: instr = (tlA <= 0x0F ? opATable021C[tlA] : NULL); break;
				case 0x0224: instr = (tlA <= 0x0F ? opATable0224[tlA] : NULL); break;
				case 0x022C: instr = (tlA <= 0x0F ? opATable022C[tlA] : NULL); break;
				case 0x0234: instr = (tlA <= 0x0F ? opATable0234[tlA] : NULL); break;
				case 0x023C: instr = (tlA <= 0x0F ? opATable023C[tlA] : NULL); break;
				case 0x0244: instr = (tlA <= 0x0F ? opATable0244[tlA] : NULL); break;
				case 0x0245: instr = (tlA <= 0x0F ? opATable0245[tlA] : NULL); break;
				case 0x0246: instr = (tlA <= 0x0F ? opATable0246[tlA] : NULL); break;
				case 0x024C: instr = (tlA <= 0x0F ? opATable024C[tlA] : NULL); break;
				case 0x024D: instr = (tlA <= 0x0F ? opATable024D[tlA] : NULL); break;
				case 0x024E: instr = (tlA <= 0x0F ? opATable024E[tlA] : NULL); break;
				case 0x0254: instr = (tlA <= 0x0F ? opATable0254[tlA] : NULL); break;
				case 0x0255: instr = (tlA <= 0x0F ? opATable0255[tlA] : NULL); break;
				case 0x0256: instr = (tlA <= 0x0F ? opATable0256[tlA] : NULL); break;
				case 0x025C: instr = (tlA <= 0x0F ? opATable025C[tlA] : NULL); break;
				case 0x025D: instr = (tlA <= 0x0F ? opATable025D[tlA] : NULL); break;
				case 0x025E: instr = (tlA <= 0x0F ? opATable025E[tlA] : NULL); break;
				case 0x0264: instr = (tlA <= 0x0F ? opATable0264[tlA] : NULL); break;
				case 0x0265: instr = (tlA <= 0x0F ? opATable0265[tlA] : NULL); break;
				case 0x0266: instr = (tlA <= 0x0F ? opATable0266[tlA] : NULL); break;
				case 0x026C: instr = (tlA <= 0x0F ? opATable026C[tlA] : NULL); break;
				case 0x026D: instr = (tlA <= 0x0F ? opATable026D[tlA] : NULL); break;
				case 0x026E: instr = (tlA <= 0x0F ? opATable026E[tlA] : NULL); break;
				case 0x0274: instr = (tlA <= 0x0F ? opATable0274[tlA] : NULL); break;
				case 0x0275: instr = (tlA <= 0x0F ? opATable0275[tlA] : NULL); break;
				case 0x0276: instr = (tlA <= 0x0F ? opATable0276[tlA] : NULL); break;
				case 0x027C: instr = (tlA <= 0x0F ? opATable027C[tlA] : NULL); break;
				case 0x027D: instr = (tlA <= 0x0F ? opATable027D[tlA] : NULL); break;
				case 0x027E: instr = (tlA <= 0x0F ? opATable027E[tlA] : NULL); break;
				case 0x02C4: instr = (tlA <= 0x0F ? opATable02C4[tlA] : NULL); break;
				case 0x02C5: instr = (tlA <= 0x0F ? opATable02C5[tlA] : NULL); break;
				case 0x02C6: instr = (tlA <= 0x0F ? opATable02C6[tlA] : NULL); break;
				case 0x02CC: instr = (tlA <= 0x0F ? opATable02CC[tlA] : NULL); break;
				case 0x02CD: instr = (tlA <= 0x0F ? opATable02CD[tlA] : NULL); break;
				case 0x02CE: instr = (tlA <= 0x0F ? opATable02CE[tlA] : NULL); break;
				case 0x02D4: instr = (tlA <= 0x0F ? opATable02D4[tlA] : NULL); break;
				case 0x02D5: instr = (tlA <= 0x0F ? opATable02D5[tlA] : NULL); break;
				case 0x02D6: instr = (tlA <= 0x0F ? opATable02D6[tlA] : NULL); break;
				case 0x02DC: instr = (tlA <= 0x0F ? opATable02DC[tlA] : NULL); break;
				case 0x02DD: instr = (tlA <= 0x0F ? opATable02DD[tlA] : NULL); break;
				case 0x02DE: instr = (tlA <= 0x0F ? opATable02DE[tlA] : NULL); break;
				case 0x02E4: instr = (tlA <= 0x0F ? opATable02E4[tlA] : NULL); break;
				case 0x02E5: instr = (tlA <= 0x0F ? opATable02E5[tlA] : NULL); break;
				case 0x02E6: instr = (tlA <= 0x0F ? opATable02E6[tlA] : NULL); break;
				case 0x02EC: instr = (tlA <= 0x0F ? opATable02EC[tlA] : NULL); break;
				case 0x02ED: instr = (tlA <= 0x0F ? opATable02ED[tlA] : NULL); break;
				case 0x02EE: instr = (tlA <= 0x0F ? opATable02EE[tlA] : NULL); break;
				case 0x02F4: instr = (tlA <= 0x0F ? opATable02F4[tlA] : NULL); break;
				case 0x02F5: instr = (tlA <= 0x0F ? opATable02F5[tlA] : NULL); break;
				case 0x02F6: instr = (tlA <= 0x0F ? opATable02F6[tlA] : NULL); break;
				case 0x02FC: instr = (tlA <= 0x0F ? opATable02FC[tlA] : NULL); break;
				case 0x02FD: instr = (tlA <= 0x0F ? opATable02FD[tlA] : NULL); break;
				case 0x02FE: instr = (tlA <= 0x0F ? opATable02FE[tlA] : NULL); break;
				case 0x0304: instr = (tlA <= 0x0F ? opATable0304[tlA] : NULL); break;
				case 0x0305: instr = (tlA <= 0x0F ? opATable0305[tlA] : NULL); break;
				case 0x0306: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0306[tlA - 0x08] : NULL); break;
				case 0x030C: instr = (tlA <= 0x0F ? opATable030C[tlA] : NULL); break;
				case 0x030D: instr = (tlA <= 0x0F ? opATable030D[tlA] : NULL); break;
				case 0x030E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable030E[tlA - 0x08] : NULL); break;
				case 0x0314: instr = (tlA <= 0x0F ? opATable0314[tlA] : NULL); break;
				case 0x0315: instr = (tlA <= 0x0F ? opATable0315[tlA] : NULL); break;
				case 0x0316: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0316[tlA - 0x08] : NULL); break;
				case 0x031C: instr = (tlA <= 0x0F ? opATable031C[tlA] : NULL); break;
				case 0x031D: instr = (tlA <= 0x0F ? opATable031D[tlA] : NULL); break;
				case 0x031E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable031E[tlA - 0x08] : NULL); break;
				case 0x0324: instr = (tlA <= 0x0F ? opATable0324[tlA] : NULL); break;
				case 0x0325: instr = (tlA <= 0x0F ? opATable0325[tlA] : NULL); break;
				case 0x0326: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0326[tlA - 0x08] : NULL); break;
				case 0x032C: instr = (tlA <= 0x0F ? opATable032C[tlA] : NULL); break;
				case 0x032D: instr = (tlA <= 0x0F ? opATable032D[tlA] : NULL); break;
				case 0x032E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable032E[tlA - 0x08] : NULL); break;
				case 0x0334: instr = (tlA <= 0x0F ? opATable0334[tlA] : NULL); break;
				case 0x0335: instr = (tlA <= 0x0F ? opATable0335[tlA] : NULL); break;
				case 0x0336: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable0336[tlA - 0x08] : NULL); break;
				case 0x033C: instr = (tlA <= 0x0F ? opATable033C[tlA] : NULL); break;
				case 0x033D: instr = (tlA <= 0x0F ? opATable033D[tlA] : NULL); break;
				case 0x033E: instr = (tlA >= 0x08 && tlA <= 0x0F ? opATable033E[tlA - 0x08] : NULL); break;
				case 0x0344: instr = (tlA <= 0x0F ? opATable0344[tlA] : NULL); break;
				case 0x0345: instr = (tlA <= 0x0F ? opATable0345[tlA] : NULL); break;
				case 0x0346: instr = (tlA <= 0x0F ? opATable0346[tlA] : NULL); break;
				case 0x034C: instr = (tlA <= 0x0F ? opATable034C[tlA] : NULL); break;
				case 0x034D: instr = (tlA <= 0x0F ? opATable034D[tlA] : NULL); break;
				case 0x034E: instr = (tlA <= 0x0F ? opATable034E[tlA] : NULL); break;
				case 0x0354: instr = (tlA <= 0x0F ? opATable0354[tlA] : NULL); break;
				case 0x0355: instr = (tlA <= 0x0F ? opATable0355[tlA] : NULL); break;
				case 0x0356: instr = (tlA <= 0x0F ? opATable0356[tlA] : NULL); break;
				case 0x035C: instr = (tlA <= 0x0F ? opATable035C[tlA] : NULL); break;
				case 0x035D: instr = (tlA <= 0x0F ? opATable035D[tlA] : NULL); break;
				case 0x035E: instr = (tlA <= 0x0F ? opATable035E[tlA] : NULL); break;
				case 0x0364: instr = (tlA <= 0x0F ? opATable0364[tlA] : NULL); break;
				case 0x0365: instr = (tlA <= 0x0F ? opATable0365[tlA] : NULL); break;
				case 0x0366: instr = (tlA <= 0x0F ? opATable0366[tlA] : NULL); break;
				case 0x036C: instr = (tlA <= 0x0F ? opATable036C[tlA] : NULL); break;
				case 0x036D: instr = (tlA <= 0x0F ? opATable036D[tlA] : NULL); break;
				case 0x036E: instr = (tlA <= 0x0F ? opATable036E[tlA] : NULL); break;
				case 0x0374: instr = (tlA <= 0x0F ? opATable0374[tlA] : NULL); break;
				case 0x0375: instr = (tlA <= 0x0F ? opATable0375[tlA] : NULL); break;
				case 0x0376: instr = (tlA <= 0x0F ? opATable0376[tlA] : NULL); break;
				case 0x037C: instr = (tlA <= 0x0F ? opATable037C[tlA] : NULL); break;
				case 0x037D: instr = (tlA <= 0x0F ? opATable037D[tlA] : NULL); break;
				case 0x037E: instr = (tlA <= 0x0F ? opATable037E[tlA] : NULL); break;
				case 0x0380: instr = opATable0380[tlA]; break;
				case 0x0381: instr = opATable0381[tlA]; break;
				case 0x0382: instr = opATable0382[tlA]; break;
				case 0x0384: instr = opATable0384[tlA]; break;
				case 0x0385: instr = opATable0385[tlA]; break;
				case 0x0386: instr = opATable0386[tlA]; break;
				case 0x0388: instr = opATable0388[tlA]; break;
				case 0x0389: instr = opATable0389[tlA]; break;
				case 0x038A: instr = opATable038A[tlA]; break;
				case 0x038C: instr = opATable038C[tlA]; break;
				case 0x038D: instr = opATable038D[tlA]; break;
				case 0x038E: instr = opATable038E[tlA]; break;
				case 0x0390: instr = opATable0390[tlA]; break;
				case 0x0391: instr = opATable0391[tlA]; break;
				case 0x0392: instr = opATable0392[tlA]; break;
				case 0x0394: instr = opATable0394[tlA]; break;
				case 0x0395: instr = opATable0395[tlA]; break;
				case 0x0396: instr = opATable0396[tlA]; break;
				case 0x0398: instr = opATable0398[tlA]; break;
				case 0x0399: instr = opATable0399[tlA]; break;
				case 0x039A: instr = opATable039A[tlA]; break;
				case 0x039C: instr = opATable039C[tlA]; break;
				case 0x039D: instr = opATable039D[tlA]; break;
				case 0x039E: instr = opATable039E[tlA]; break;
				case 0x03A0: instr = opATable03A0[tlA]; break;
				case 0x03A1: instr = opATable03A1[tlA]; break;
				case 0x03A2: instr = opATable03A2[tlA]; break;
				case 0x03A4: instr = opATable03A4[tlA]; break;
				case 0x03A5: instr = opATable03A5[tlA]; break;
				case 0x03A6: instr = opATable03A6[tlA]; break;
				case 0x03A8: instr = opATable03A8[tlA]; break;
				case 0x03A9: instr = opATable03A9[tlA]; break;
				case 0x03AA: instr = opATable03AA[tlA]; break;
				case 0x03AC: instr = opATable03AC[tlA]; break;
				case 0x03AD: instr = opATable03AD[tlA]; break;
				case 0x03AE: instr = opATable03AE[tlA]; break;
				case 0x03B0: instr = opATable03B0[tlA]; break;
				case 0x03B1: instr = opATable03B1[tlA]; break;
				case 0x03B2: instr = opATable03B2[tlA]; break;
				case 0x03B4: instr = opATable03B4[tlA]; break;
				case 0x03B5: instr = opATable03B5[tlA]; break;
				case 0x03B6: instr = opATable03B6[tlA]; break;
				case 0x03B8: instr = opATable03B8[tlA]; break;
				case 0x03B9: instr = opATable03B9[tlA]; break;
				case 0x03BA: instr = opATable03BA[tlA]; break;
				case 0x03BC: instr = opATable03BC[tlA]; break;
				case 0x03BD: instr = opATable03BD[tlA]; break;
				case 0x03BE: instr = opATable03BE[tlA]; break;
				default: instr = NULL; break;
			}
		}

		// Lastly, try op table B if no match found
		if (instr == NULL)
		{
			UINT16 hdB = opcode>>12;
			UINT16 tlB = (opcode>>6) & 0x3F;
			switch (hdB)
			{
				case 0x00: instr = opBTable00[tlB]; break;
				case 0x02: instr = (tlB >= 0x01 && tlB <= 0x39 ? opBTable02[tlB - 0x01] : NULL); break;
				case 0x03: instr = (tlB >= 0x01 && tlB <= 0x39 ? opBTable03[tlB - 0x01] : NULL); break;
				case 0x04: instr = opBTable04[tlB]; break;
				case 0x05: instr = opBTable05[tlB]; break;
				case 0x08: instr = opBTable08[tlB]; break;
				case 0x09: instr = opBTable09[tlB]; break;
				case 0x0B: instr = opBTable0B[tlB]; break;
				case 0x0C: instr = opBTable0C[tlB]; break;
				case 0x0D: instr = opBTable0D[tlB]; break;
				case 0x0E: instr = (tlB >= 0x03 && tlB <= 0x1F ? opBTable0E[tlB - 0x03] : NULL); break;
				default: instr = NULL; break;
			}
		}
		
		if (instr == NULL)
			goto invalid;

		// Split instruction into mnemonic and operands and substitute any data and address mode tags
		const char *p, *q;
		char *r;
		char opsCopy[255];
		char sizeC;
		UINT8 addrMode;
		int part;
		EOpFlags opFlags;
			
		// TODO - address mode masks for each instruction

		q = instr;
		if (p = strchr(q, ' '))
		{
			// Split instruction
			strncpy(mnemonic, instr, p - q);
			mnemonic[p - q] = '\0';
			operands[0] = '\0';
			opsCopy[0] = '\0';
			
			// Get size character in nmnemonic, if any, otherwise assume word
			if (q = strchr(mnemonic, '.'))
				sizeC = tolower(q[1]);
			else
				sizeC = 'w';

			// Substitute any data tags
			q = p + 1;
			r = opsCopy;
			while (p = strchr(q, '['))
			{
				strncpy(r, q, p - q);
				r[p - q] = '\0';
				r += strlen(r);

				// TODO - get rid of part indices - not needed for 68000 instruction set (only needed for 020 or 030 etc)
				if (sscanf(p, "[BN%d]", &part) == 1 || 
					sscanf(p, "[DB%d]", &part) == 1 || sscanf(p, "[DW%d]", &part) == 1 || sscanf(p, "[DL%d]", &part) == 1)
				{
					// Bit number or byte, word or long immediate data
					if (p[2] == 'N')
					{
						// Check first byte is zero
						u8 = (UINT8)ReadMem(addr + offset++, 1);
						if (u8 != 0)
						{
							offset++;
							goto invalid;
						}
						// Get data byte
						u8 = (UINT8)ReadMem(addr + offset++, 1);
						// Check top 3 bits are zero
						if (u8&0xE0)
							goto invalid;
						FormatData(r, 1, u8);
						//sprintf(r, "0x%02X", u8);
					}
					else if (p[2] == 'B')
					{
						// Check first byte is zero
						u8 = (UINT8)ReadMem(addr + offset++, 1);
						if (u8 != 0)
						{
							offset++;
							goto invalid;
						}
						// Get data byte
						u8 = (UINT8)ReadMem(addr + offset++, 1);
						FormatData(r, 1, u8);
						//i8 = (INT8)ReadMem(addr + offset++, 1);
						//if (i8 < 0)
						//{
						//	*r++ = '-';
						//	FormatData(r, 1, -i8);
						//	//sprintf(r, "-0x%02X", -i8);
						//}
						//else 
						//{
						//	FormatData(r, 1, i8);
						//	//sprintf(r, "0x%02X", i8);
						//}
					}
					else if (p[2] == 'W')
					{
						u16 = (UINT16)ReadMem(addr + offset, 2);
						offset += 2;
						FormatData(r, 2, u16);
						//i16 = (INT16)ReadMem(addr + offset, 2);
						//offset += 2;
						//if (i16 < 0)
						//{
						//	*r++ = '-';
						//	FormatData(r, 2, -i16);
						//	//sprintf(r, "-0x%04X", -i16);
						//}
						//else
						//{
						//	FormatData(r, 2, i16);
						//	//sprintf(r, "0x%04X", i16);
						//}
					}
					else
					{
						u32 = (UINT32)ReadMem(addr + offset, 4);
						offset += 4;
						FormatData(r, 4, u32);
						//i32 = (INT32)ReadMem(addr + offset, 4);
						//offset += 4;
						//if (i32 < 0)
						//{
						//	*r++ = '-';
						//	FormatData(r, 4, -i32);
						//	//sprintf(r, "-0x%08X", -i32);
						//}
						//else
						//{
						//	FormatData(r, 4, i32);                                       
						//	//sprintf(r, "0x%08X", i32);
						//}
					}
					q = p + 5;
					r += strlen(r);
				}
				else if (sscanf(p, "[LL%d]", &part) == 1) // TODO - this should be LW
				{
					// Fixed width label offset
					i16 = (INT16)ReadMem(addr + offset, 2);
					offset += 2;
					// Offset is from PC + 2
					opFlags = GetOpFlags(addr, opcode);
					FormatJumpAddress(r, addr + 2 + i16, opFlags);
					//sprintf(r, "0x%06X", addr + 2 + i16); // Format this way as memory bus width is only 24-bits
					q = p + 5;
					r += strlen(r);
				}
				else if (sscanf(p, "[LV%d]", &part) == 1) 
				{
					// Variable width label offset
					i8 = (INT8)(opcode&0xFF);
					INT32 labOffset;
					if (i8 == 0x00)
					{
						i16 = (INT16)ReadMem(addr + offset, 2);
						offset += 2;
						labOffset = i16;
					}
					// Following 68020+ only
					//else if (i8 == 0xFF)
					//{
					//	i32 = (INT32)ReadMem(addr + offset, 4);
					//	offset += 4;
					//	labOffset = i32;
					//*/
					else
						labOffset = i8;
					// Offset is from PC + 2
					opFlags = GetOpFlags(addr, opcode);
					FormatJumpAddress(r, addr + 2 + labOffset, opFlags);
					//sprintf(r, "0x%06X", addr + 2 + labOffset); // Format this way as memory bus width is only 24-bits
					q = p + 5;
					r += strlen(r);
				}
				else
				{
					// TODO - if not AM or AR, goto invalid
					q = p + 1;
					r += strlen(r);
					*r++ = '[';
					*r = '\0';
				}
			}
			strcat(r, q);

			// Substitute any address mode tags
			q = opsCopy;
			r = operands;
			while (p = strchr(q, '['))
			{
				strncpy(r, q, p - q);
				r[p - q] = '\0';
				r += strlen(r);

				if (sscanf(p, "[AM%d]", &part) == 1 || sscanf(p, "[AR%d]", &part) == 1)
				{
					// Part index must be 1 or 2				
					if (part == 1)
						addrMode = (UINT8)((opcode >> 6) & 0x3F);
					else if (part == 2)
						addrMode = (UINT8)(opcode & 0x3F);
					else 
						goto invalid;
					// Check if address mode parts are reversed
					if (p[2] == 'R')
					{
						// If so, swap 3-bit mode and register info
						addrMode = ((addrMode&0x7)<<3) | ((addrMode>>3)&0x7);
					}
					if (!FormatAddrMode(addr, opcode, offset, addrMode, sizeC, r))
						goto invalid;
					q = p + 5;
					r += strlen(r);
				}
				else
				{
					q = p + 1;
					r += strlen(r);
					*r++ = '[';
					*r = '\0';
				}
			}
			strcat(r, q);
		}
		else
		{
			// Empty operands
			strcpy(mnemonic, instr);
			operands[0] = '\0';
		}
		return offset;

	invalid:
		mnemonic[0] = '\0';
		operands[0] = '\0';
		return -offset;
	}

	bool C68KDebug::FormatAddrMode(UINT32 addr, UINT32 opcode, int &offset, UINT8 addrMode, char sizeC, char *dest)
	{
		UINT8 mode = (addrMode>>3)&0x07;
		UINT8 reg = addrMode&0x07;
		char dataStr[20];
		UINT8 ofsReg;
		char ofsRegC, ofsSizeC;
		INT8 i8;
		INT16 i16;
		INT32 i32;
		UINT8 u8;
		UINT16 u16;
		UINT32 u32;
		EOpFlags opFlags;
		switch (mode)
		{
			case 0x00: // Dn
				sprintf(dest, "D%u", reg);
				break;
			case 0x01: // An
				sprintf(dest, "A%u", reg);
				break;
			case 0x02: // (An)
				sprintf(dest, "(A%u)", reg);
				break;
			case 0x03: // (An)+
				sprintf(dest, "(A%u)+", reg);
				break;
			case 0x04: // -(An)
				sprintf(dest, "-(A%u)", reg);
				break;
			case 0x05: // (d16,An)
				i16 = (INT16)ReadMem(addr + offset, 2);
				offset += 2;
				if (i16 < 0)
				{
					FormatData(dataStr, 2, -i16);
					sprintf(dest, "(-%s,A%u)", dataStr, reg);
					//sprintf(dest, "(-0x%04X,A%u)", -i16, reg);
				}
				else
				{
					FormatData(dataStr, 2, i16);
					sprintf(dest, "(%s,A%u)", dataStr, reg);
					//sprintf(dest, "(0x%04X,A%u)", i16, reg);
				}
				break;
			case 0x06: // (d8,An,Ri.z)
				u8 = (UINT8)ReadMem(addr + offset++, 1);
				// Check first 3-bits of first byte are zero
				if (u8&0x7)
				{
					offset++;
					return false;
				}
				// Get second offset register type, number and size (w or l)
				ofsReg = (u8>>4)&0xF;
				if (ofsReg < 8)
					ofsRegC = 'D';
				else
				{
					ofsRegC = 'A';
					ofsReg -= 8;
				}
				ofsSizeC = (u8&0x8 ? 'l' : 'w');
				i8 = (INT8)ReadMem(addr + offset++, 1);
				if (i8 < 0)
				{
					FormatData(dataStr, 1, -i8);
					sprintf(dest, "(-%s,A%u,%c%u.%c)", dataStr, reg, ofsRegC, ofsReg, ofsSizeC);
					//sprintf(dest, "(-0x%02X,A%u,%c%u.%c)", -i8, reg, ofsRegC, ofsReg, ofsSizeC);
				}
				else
				{
					FormatData(dataStr, 1, i8);
					sprintf(dest, "(%s,A%u,%c%u.%c)", dataStr, reg, ofsRegC, ofsReg, ofsSizeC);
					//sprintf(dest, "(0x%02X,A%u,%c%u.%c)", i8, reg, ofsRegC, ofsReg, ofsSizeC);
				}
				break;
			case 0x07: 
				switch (reg)
				{
					case 0x00: // addr16
						u16 = (UINT16)ReadMem(addr + offset, 2);
						offset += 2;
						opFlags = GetOpFlags(addr, opcode);
						FormatJumpAddress(dest, u16, opFlags); 
						//sprintf(dest, "0x%06X", u16); // Format this way as memory bus width is 24-bits
						break;
					case 0x01: // addr32
						u32 = (UINT32)ReadMem(addr + offset, 4);
						offset += 4;
						opFlags = GetOpFlags(addr, opcode);
						FormatJumpAddress(dest, u32, opFlags); 
						//sprintf(dest, "0x%06X", u32); // Format this way as memory bus width is only 24-bits
						break;
					case 0x02: // d16(PC)
						i16 = (INT16)ReadMem(addr + offset, 2);
						offset += 2;
						if (i16 < 0)
						{
							FormatData(dataStr, 2, -i16);
							sprintf(dest, "-%s(PC)", dataStr);
							//sprintf(dest, "-0x%04X(PC)", -i16);
						}
						else
						{
							FormatData(dataStr, 2, i16);
							sprintf(dest, "%s(PC)", dataStr);
							//sprintf(dest, "0x%04X(PC)", i16);
						}
						break;
					case 0x03: // u8(PC,Ri.x)
						u8 = (UINT8)ReadMem(addr + offset++, 1);
						// Check first 3-bits of first byte are zero
						if (u8&0x7)
						{
							offset++;
							return false;
						}
						// Get second offset register type, number and size (w or l)
						ofsReg = (u8>>4)&0xF;
						if (ofsReg < 8)
							ofsRegC = 'D';
						else
						{
							ofsRegC = 'A';
							ofsReg -= 8;
						}
						ofsSizeC = (u8&0x8 ? 'l' : 'w');
						i8 = (INT8)ReadMem(addr + offset++, 1);
						if (i8 < 0)
						{
							FormatData(dataStr, 1, -i8);
							sprintf(dest, "-%s(PC,%c%u.%c)", dataStr, ofsRegC, ofsReg, ofsSizeC);
							//sprintf(dest, "-0x%02X(PC,%c%u.%c)", -i8, ofsRegC, ofsReg, ofsSizeC);
						}
						else
						{
							FormatData(dataStr, 1, i8);
							sprintf(dest, "%s(PC,%c%u.%c)", dataStr, ofsRegC, ofsReg, ofsSizeC);
							//sprintf(dest, "0x%02X(PC,%c%u.%c)", i8, ofsRegC, ofsReg, ofsSizeC);
						}
						break;
					case 0x04: // immediate data, implied size
						if (sizeC == 'b')
						{
							// Check first byte is zero
							u8 = (UINT8)ReadMem(addr + offset++, 1);
							if (u8 != 0)
							{
								offset++;
								return false;
							}
							// Get data byte
							u8 = (UINT8)ReadMem(addr + offset++, 1);
							FormatData(dataStr, 1, u8);
							sprintf(dest, "#%s", dataStr);
							//i8 = (INT8)ReadMem(addr + offset++, 1);
							//if (i8 < 0)
							//{
							//	FormatData(dataStr, 1, -i8);
							//	sprintf(dest, "#-%s", dataStr);
							//	//sprintf(dest, "#-0x%02X", -i8);
							//}
							//else
							//{
							//	FormatData(dataStr, 1, i8);
							//	sprintf(dest, "#%s", dataStr);
							//	//sprintf(dest, "#0x%02X", i8);
							//}
						}
						else if (sizeC == 'w')
						{
							u16 = (UINT16)ReadMem(addr + offset, 2);
							offset += 2;
							FormatData(dataStr, 2, u16);
							sprintf(dest, "#%s", dataStr);
							//i16 = (INT16)ReadMem(addr + offset, 2);
							//offset += 2;
							//if (i16 < 0)
							//{
							//	FormatData(dataStr, 2, -i16);
							//	sprintf(dest, "#-%s", dataStr);
							//	//sprintf(dest, "#-0x%04X", -i16);
							//}
							//else
							//{
							//	FormatData(dataStr, 2, i16);
							//	sprintf(dest, "#%s", dataStr);
							//	//sprintf(dest, "#0x%04X", i16);
							//}
						}
						else if (sizeC == 'l')
						{
							u32 = (UINT32)ReadMem(addr + offset, 4);
							offset += 4;
							FormatData(dataStr, 4, u32);
							sprintf(dest, "#%s", dataStr);
							//i32 = (INT32)ReadMem(addr + offset, 4);
							//offset += 4;
							//if (i32 < 0)
							//{
							//	FormatData(dataStr, 4, -i32);
							//	sprintf(dest, "#-%s", dataStr);
							//	//sprintf(dest, "#-0x%08X", -i32);
							//}
							//else
							//{
							//	FormatData(dataStr, 4, i32);
							//	sprintf(dest, "#%s", dataStr);
							//	//sprintf(dest, "#0x%08X", i32);
							//}
						}
						else					
							return false;
						break;
					default:
						return false;
				}
				break;
			default:
				return false;
		}
		return true;
	}

	EOpFlags C68KDebug::GetOpFlags(UINT32 addr, UINT32 opcode)
	{
		UINT32 head = opcode>>6;
		if (head == 0x013A) // Instruction is jmp
			return JumpSimple; 
		else if (head == 0x013B) // Instruction is jsr
			return JumpSub;
		else if ((opcode>>12) == 0x0006) // Instruction is bra, bsr or bCC
		{
			UINT32 cond = (opcode>>8)&0x000F;
			if (cond == 0x0000) // Instruction is bra
				return (EOpFlags)(JumpSimple | Relative);
			else if (cond == 0x0001) // Instruction is bsr
				return (EOpFlags)(JumpSub | Relative);
			else // Instruction is bCC
				return (EOpFlags)(JumpSimple | Relative | Conditional);
		}			
		else if ((opcode&0xF0F8) == 0x50C8) // Instruction is dbCC
			return (EOpFlags)(JumpLoop | Relative | Conditional);
		else if (opcode == 0x4E74 || opcode == 0x4E75) // Instruction is rtr or rts
			return ReturnSub;
		else if (opcode == 0x4E73) // Instruction is rte
			return ReturnEx;
		else
			return NormalOp;
	}

	bool C68KDebug::GetJumpAddr(UINT32 addr, UINT32 opcode, UINT32 &jumpAddr)
	{
		if ((opcode>>7) == 0x009D)
		{
			// Instruction is jmp or jsr
			UINT8 addrMode = (UINT8)(opcode&0x3F);
			UINT8 mode = (addrMode>>3)&0x07;
			UINT8 reg = addrMode&0x07;
			if (mode == 0x07)
			{
				if (reg == 0x00)
				{
					jumpAddr = (UINT32)ReadMem(addr + 2, 4);
					return true;
				}
				else if (reg == 0x01)
				{
					jumpAddr = (UINT32)ReadMem(addr + 2, 4);
					return true;
				}
			}
			return false;
		}
		else if ((opcode>>12) == 0x0006)
		{
			// Instruction is bra, bsr or bCC
			INT8 i8 = (INT8)(opcode&0xFF);
			if (i8 == 0x00)
				jumpAddr = addr + 2 + (INT16)ReadMem(addr + 2, 2);
			else
				jumpAddr = addr + 2 + i8;
			return true;
		}
		else if ((opcode&0xF0F8) == 0x50C8)
		{
			// Instruction is dbCC
			jumpAddr = addr + 2 + (INT16)ReadMem(addr + 2, 2);
			return true;
		}
		else
			return false;
	}

	bool C68KDebug::GetJumpRetAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr)
	{
		if ((opcode>>6) == 0x013B)
		{
			// Instruction is jsr
			UINT8 addrMode = (UINT8)(opcode&0x3F);
			UINT8 mode = (addrMode>>3)&0x07;
			UINT8 reg = addrMode&0x07;
			if (mode == 0x05 || mode == 0x06)
				retAddr = addr + 2;
			else if (mode == 0x07)
			{
				if (reg == 0x00 || reg == 0x02 || reg == 0x03)
					retAddr = addr + 2;
				else if (reg == 0x01)
					retAddr = addr + 4;
				else
					retAddr = addr;
			}
			else
				retAddr = addr;
			return true;
		}
		else if ((opcode>>8) == 0x0061)
		{
			// Instruction is bsr
			if ((opcode&0xFF) == 0x00)
				retAddr = addr + 4;
			else
				retAddr = addr + 2;
			return true;
		}
		return false;
	}

	bool C68KDebug::GetReturnAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr)
	{
		// Check opcode is rtr, rts or rte
		if (opcode != 0x4E74 && opcode != 0x4E75 && opcode != 0x4E73)
			return false;
		// Return address will be at top of stack for rts and stack + 2 for rtr or rte
		UINT32 sp = GetSP();
		if (opcode == 0x4E75)
			retAddr = (UINT32)ReadMem(sp, 4);
		else
			retAddr = (UINT32)ReadMem(sp + 2, 4);
		return true;
	}

	bool C68KDebug::GetHandlerAddr(CException *ex, UINT32 &handlerAddr)
	{
		UINT32 vecAddr = ex->code * 4;
		handlerAddr = (UINT32)ReadMem(vecAddr, 4);
		return !!handlerAddr;
	}

	bool C68KDebug::GetHandlerAddr(CInterrupt *in, UINT32 &handlerAddr)
	{
		UINT32 vecAddr = (in->code + 25) * 4;
		handlerAddr = (UINT32)ReadMem(vecAddr, 4);
		return !!handlerAddr;
	}
}

#endif  // SUPERMODEL_DEBUGGER