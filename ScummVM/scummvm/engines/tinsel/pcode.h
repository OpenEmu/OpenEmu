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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Virtual processor definitions
 */

#ifndef TINSEL_PCODE_H     // prevent multiple includes
#define TINSEL_PCODE_H

#include "tinsel/events.h"	// for TINSEL_EVENT
#include "tinsel/sched.h"	// for Common::PROCESS

namespace Common {
class Serializer;
}

namespace Tinsel {

// forward declaration
struct INV_OBJECT;

enum RESUME_STATE {
	RES_NOT, RES_1, RES_2, RES_SAVEGAME
};

enum {
	PCODE_STACK_SIZE	= 128	///< interpeters stack size
};

enum GSORT {
	GS_NONE, GS_ACTOR, GS_MASTER, GS_POLYGON, GS_INVENTORY, GS_SCENE,
	GS_PROCESS, GS_GPROCESS
};

enum RESCODE {RES_WAITING, RES_FINISHED, RES_CUTSHORT};

struct WorkaroundEntry;

struct INT_CONTEXT {

	// Elements for interpret context management
	Common::PROCESS *pProc;			///< processes owning this context
	GSORT	GSort;			///< sort of this context

	// Previously parameters to Interpret()
	SCNHANDLE	hCode;		///< scene handle of the code to execute
	byte		*code;		///< pointer to the code to execute
	TINSEL_EVENT	event;		///< causal event
	HPOLYGON	hPoly;		///< associated polygon (if any)
	int			idActor;	///< associated actor (if any)
	INV_OBJECT	*pinvo;		///< associated inventory object

	// Previously local variables in Interpret()
	int32 stack[PCODE_STACK_SIZE];	///< interpeters run time stack
	int sp;				///< stack pointer
	int bp;				///< base pointer
	int ip;				///< instruction pointer
	bool bHalt;			///< set to exit interpeter
	bool escOn;
	int myEscape;		///< only initialized to prevent compiler warning!

	uint32 waitNumber1;		// The waiting numbert
	uint32 waitNumber2;		// The wait for number
	RESCODE resumeCode;
	RESUME_STATE resumeState;

	// Used to store execution state within a script workaround fragment
	const WorkaroundEntry *fragmentPtr;

	void syncWithSerializer(Common::Serializer &s);
};
typedef INT_CONTEXT *PINT_CONTEXT;

/*----------------------------------------------------------------------*\
|*			Interpreter Function Prototypes			*|
\*----------------------------------------------------------------------*/

// Interprets the PCODE instructions in the code array
void Interpret(CORO_PARAM, INT_CONTEXT *ic);

INT_CONTEXT *InitInterpretContext(
	GSORT		gsort,
	SCNHANDLE	hCode,		// code to execute
	TINSEL_EVENT	event,		// causal event
	HPOLYGON	hpoly,		// associated polygon (if any)
	int		actorid,	// associated actor (if any)
	INV_OBJECT	*pinvo,
	int myEscape = -1);		// associated inventory object

INT_CONTEXT *RestoreInterpretContext(INT_CONTEXT *ric);

void FreeMostInterpretContexts();
void FreeMasterInterpretContext();

void SaveInterpretContexts(INT_CONTEXT *sICInfo);

void RegisterGlobals(int num);
void FreeGlobals();

void AttachInterpret(INT_CONTEXT *pic, Common::PROCESS *pProc);

void WaitInterpret(CORO_PARAM, Common::PPROCESS pWaitProc, bool *result);

#define NUM_INTERPRET	(CORO_NUM_PROCESS - 20)
#define MAX_INTERPRET	(CORO_MAX_PROCESSES - 20)

/*----------------------------------------------------------------------*\
|*	Library Procedure and Function codes parameter enums		*|
\*----------------------------------------------------------------------*/

#define TAG_DEF		0	// For tagactor()
#define TAG_Q1TO3	1	//	tag types
#define TAG_Q1TO4	2	//	tag types

#define CONV_DEF	0	//
#define CONV_BOTTOM	1	// conversation() parameter
#define CONV_END	2	//

#define CONTROL_OFF	0	// control()
#define CONTROL_ON	1	//	parameter
#define CONTROL_OFFV	2	//
#define CONTROL_OFFV2	3	//
#define CONTROL_STARTOFF 4	//

#define NULL_ACTOR (-1)		// For actor parameters
#define LEAD_ACTOR (-2)		//

#define RAND_NORM	0	// For random() frills
#define RAND_NORPT	1	//

#define D_UP		1
#define D_DOWN		0

#define TW_START	1	// topwindow() parameter
#define TW_END		2	//

#define MIDI_DEF	0
#define MIDI_LOOP	1

#define FM_IN		0	//
#define FM_OUT		1	// fademidi()

#define FG_ON		0	//
#define FG_OFF		1	// FrameGrab()

#define ST_ON		0	//
#define ST_OFF		1	// SubTitles()

} // End of namespace Tinsel

#endif		// TINSEL_PCODE_H
