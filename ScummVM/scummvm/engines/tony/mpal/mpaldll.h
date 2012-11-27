/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef __MPALDLL_H
#define __MPALDLL_H

#include "common/file.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/loadmpc.h"
#include "tony/mpal/expr.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Defines
\****************************************************************************/

#define HEX_VERSION               0x0170

#define MAX_ACTIONS_PER_ITEM      40
#define MAX_COMMANDS_PER_ITEM     128
#define MAX_COMMANDS_PER_ACTION   128
#define MAX_DESCRIBE_SIZE         64

#define MAX_MOMENTS_PER_SCRIPT    256
#define MAX_COMMANDS_PER_SCRIPT   256
#define MAX_COMMANDS_PER_MOMENT   32

#define MAX_GROUPS_PER_DIALOG     128
#define MAX_COMMANDS_PER_DIALOG   480
#define MAX_COMMANDS_PER_GROUP    64
#define MAX_CHOICES_PER_DIALOG    64
#define MAX_SELECTS_PER_CHOICE    64
#define MAX_PLAYGROUPS_PER_SELECT 9
#define MAX_PERIODS_PER_DIALOG    400

#define NEED_LOCK_MSGS

/****************************************************************************\
*       Structures
\****************************************************************************/

#include "common/pack-start.h"

/**
 * MPAL global variables
 */
struct MpalVar {
	uint32 _dwVal; // Variable value
	char _lpszVarName[33]; // Variable name
} PACKED_STRUCT;
typedef MpalVar *LpMpalVar;

/**
 * MPAL Messages
 */
struct MpalMsg {
	MpalHandle _hText; // Handle to the message text
	uint16 _wNum; // Message number
} PACKED_STRUCT;
typedef MpalMsg *LpMpalMsg;

/**
 * MPAL Locations
 */
struct MpalLocation {
	uint32 _nObj; // Location number
	uint32 _dwXlen, _dwYlen; // Dimensions
	uint32 _dwPicRes; // Resource that contains the image
} PACKED_STRUCT;
typedef MpalLocation *LpMpalLocation;

/**
 * All the data for a command, ie. tags used by OnAction in the item, the time
 * in the script, and in the group dialog.
 */
struct Command {
  /*
   * Types of commands that are recognized
   *
   *   #1 -> Custom function call		(ITEM, SCRIPT, DIALOG)
   *   #2 -> Variable assignment		(ITEM, SCRIPT, DIALOG)
   *   #3 -> Making a choice			(DIALOG)
   *
   */
	byte _type; // Type of control

	union {
		int32 _nCf; // Custom function call [#1]
		char *_lpszVarName; // Variable name [#2]
		int32 _nChoice; // Number of choice you make [#3]
	};

	union {
		int32 _arg1; // Argument for custom function [#1]
		MpalHandle _expr; // Expression to assign to a variable [#2]
	};

	int32 _arg2, _arg3, _arg4; // Arguments for custom function [#1]
} PACKED_STRUCT;

/**
 * MPAL dialog
 */
struct MpalDialog {
	uint32 _nObj; // Dialog number

	struct Command _command[MAX_COMMANDS_PER_DIALOG];

	struct {
		uint16 _num;
		byte _nCmds;
		uint16 _cmdNum[MAX_COMMANDS_PER_GROUP];

	} _group[MAX_GROUPS_PER_DIALOG];

	struct {
		// The last choice has nChoice == 0
		uint16 _nChoice;

		// The select number (we're pretty stingy with RAM). The last select has dwData == 0
		struct {
			MpalHandle _when;
			uint32 _dwData;
			uint16 _wPlayGroup[MAX_PLAYGROUPS_PER_SELECT];

			// Bit 0=endchoice Bit 1=enddialog
			byte _attr;

			// Modified at run-time: 0 if the select is currently disabled,
			// and 1 if currently active
			byte _curActive;
		} _select[MAX_SELECTS_PER_CHOICE];

	} _choice[MAX_CHOICES_PER_DIALOG];

	uint16 _periodNums[MAX_PERIODS_PER_DIALOG];
	MpalHandle _periods[MAX_PERIODS_PER_DIALOG];

} PACKED_STRUCT;
typedef MpalDialog *LpMpalDialog;

/**
 * MPAL Item
 */
struct ItemAction {
	byte _num; // Action number
	uint16 _wTime; // If idle, the time which must pass
	byte _perc; // Percentage of the idle run
	MpalHandle _when; // Expression to compute. If != 0, then action can be done
	uint16 _wParm; // Parameter for action

	byte _nCmds; // Number of commands to be executed
	uint32 _cmdNum[MAX_COMMANDS_PER_ACTION]; // Commands to execute
} PACKED_STRUCT;

struct MpalItem {
	uint32 _nObj; // Item number

	byte _lpszDescribe[MAX_DESCRIBE_SIZE]; // Name
	byte _nActions; // Number of managed actions
	uint32 _dwRes; // Resource that contains frames and patterns

	struct Command _command[MAX_COMMANDS_PER_ITEM];

	// Pointer to array of structures containing various managed activities. In practice, of
	// every action we know what commands to run, including those defined in structures above
	struct ItemAction *_action;

} PACKED_STRUCT;
typedef MpalItem *LpMpalItem;

/**
 * MPAL Script
 */
struct MpalScript {
	uint32 _nObj;
	uint32 _nMoments;

	struct Command _command[MAX_COMMANDS_PER_SCRIPT];

	struct {
		int32 _dwTime;
		byte _nCmds;
		uint32 _cmdNum[MAX_COMMANDS_PER_MOMENT];

	} _moment[MAX_MOMENTS_PER_SCRIPT];

} PACKED_STRUCT;
typedef MpalScript *LpMpalScript;

#include "common/pack-end.h"

/****************************************************************************\
*       Function prototypes
\****************************************************************************/

/**
 * Returns the current value of a global variable
 *
 * @param lpszVarName		Name of the variable
 * @returns		Current value
 * @remarks		Before using this method, you must call LockVar() to
 * lock the global variablves for use. Then afterwards, you will
 * need to remember to call UnlockVar()
 */
extern int32 varGetValue(const char *lpszVarName);

/**
 * Sets the value of a MPAL global variable
 * @param lpszVarName       Name of the variable
 * @param val				Value to set
 */
extern void varSetValue(const char *lpszVarName, int32 val);

} // end of namespace MPAL

} // end of namespace Tony

#endif

