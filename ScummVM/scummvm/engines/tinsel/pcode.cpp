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
 * Virtual processor.
 */

#include "tinsel/dw.h"
#include "tinsel/drives.h"
#include "tinsel/events.h"	// 'POINTED' etc.
#include "tinsel/handle.h"	// LockMem()
#include "tinsel/dialogs.h"	// for inventory id's
#include "tinsel/pcode.h"	// opcodes etc.
#include "tinsel/scn.h"	// FindChunk()
#include "common/serializer.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"	// Library routines
#include "tinsel/tinsel.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

//----------------- EXTERN FUNCTIONS --------------------

extern int CallLibraryRoutine(CORO_PARAM, int operand, int32 *pp, const INT_CONTEXT *pic, RESUME_STATE *pResumeState);

//----------------- LOCAL DEFINES --------------------

#define	GLOBALS_FILENAME	"gdata"		// name of globals file

/** list of all opcodes */
enum OPCODE {
	OP_HALT = 0,	///< end of program
	OP_IMM = 1,		///< loads signed immediate onto stack
	OP_ZERO = 2,	///< loads zero onto stack
	OP_ONE = 3,		///< loads one onto stack
	OP_MINUSONE = 4,	///< loads minus one onto stack
	OP_STR = 5,		///< loads string offset onto stack
	OP_FILM = 6,	///< loads film offset onto stack
	OP_FONT = 7,	///< loads font offset onto stack
	OP_PAL = 8,		///< loads palette offset onto stack
	OP_LOAD = 9,	///< loads local variable onto stack
	OP_GLOAD = 10,	///< loads global variable onto stack - long offset to variable
	OP_STORE = 11,	///< pops stack and stores in local variable - long offset to variable
	OP_GSTORE = 12,	///< pops stack and stores in global variable - long offset to variable
	OP_CALL = 13,	///< procedure call
	OP_LIBCALL = 14,	///< library procedure call - long offset to procedure
	OP_RET = 15,		///< procedure return
	OP_ALLOC = 16,	///< allocate storage on stack
	OP_JUMP = 17,	///< unconditional jump	- signed word offset
	OP_JMPFALSE = 18,	///< conditional jump	- signed word offset
	OP_JMPTRUE = 19,	///< conditional jump	- signed word offset
	OP_EQUAL = 20,	///< tests top two items on stack for equality
	OP_LESS,	///< tests top two items on stack
	OP_LEQUAL,	///< tests top two items on stack
	OP_NEQUAL,	///< tests top two items on stack
	OP_GEQUAL,	///< tests top two items on stack
	OP_GREAT = 25,	///< tests top two items on stack
	OP_PLUS,	///< adds top two items on stack and replaces with result
	OP_MINUS,	///< subs top two items on stack and replaces with result
	OP_LOR,		///< logical or of top two items on stack and replaces with result
	OP_MULT,	///< multiplies top two items on stack and replaces with result
	OP_DIV = 30,		///< divides top two items on stack and replaces with result
	OP_MOD,		///< divides top two items on stack and replaces with modulus
	OP_AND,		///< bitwise ands top two items on stack and replaces with result
	OP_OR,		///< bitwise ors top two items on stack and replaces with result
	OP_EOR,		///< bitwise exclusive ors top two items on stack and replaces with result
	OP_LAND = 35,	///< logical ands top two items on stack and replaces with result
	OP_NOT,		///< logical nots top item on stack
	OP_COMP,	///< complements top item on stack
	OP_NEG,		///< negates top item on stack
	OP_DUP,		///< duplicates top item on stack
	OP_ESCON = 40,	///< start of escapable sequence
	OP_ESCOFF = 41,	///< end of escapable sequence
	OP_CIMM,	///< loads signed immediate onto stack (special to case statements)
	OP_CDFILM	///< loads film offset onto stack but not in current scene
};

// modifiers for the above opcodes
#define	OPSIZE8		0x40	///< when this bit is set - the operand size is 8 bits
#define	OPSIZE16	0x80	///< when this bit is set - the operand size is 16 bits

#define	OPMASK		0x3F	///< mask to isolate the opcode

bool g_bNoPause = false;

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static int32 *g_pGlobals = 0;		// global vars

static int g_numGlobals = 0;		// How many global variables to save/restore

static INT_CONTEXT *g_icList = 0;

static uint32 g_hMasterScript;

//----------------- SCRIPT BUGS WORKAROUNDS --------------

/**
 * This structure is used to introduce bug fixes into the scripts used by the games.
 */
struct WorkaroundEntry {
	TinselEngineVersion version;	///< Engine version this workaround applies to
	bool scnFlag;					///< Only applicable for Tinsel 1 (DW 1)
	bool isDemo;					///< Flags whether it's for a demo
	Common::Platform platform;		///< Platform filter
	SCNHANDLE hCode;				///< Script to apply fragment to
	int ip;							///< Script offset to run this fragment before
	int numBytes;					///< Number of bytes in the script
	const byte *script;				///< Instruction(s) to execute
};

#define FRAGMENT_WORD(x)	(byte)(x & 0xFF), (byte)(x >> 8)
#define FRAGMENT_DWORD(x)	(byte)(x & 0xFF), (byte)(x >> 8), (byte)(x >> 16), (byte)(x >> 24)

static const byte fragment1[] = {OP_ZERO, OP_GSTORE | OPSIZE16, 206, 0};
static const byte fragment2[] = {OP_LIBCALL | OPSIZE8, 110};
static const byte fragment3[] = {OP_ZERO, OP_GSTORE | OPSIZE16, FRAGMENT_WORD(490)};
static const byte fragment4[] = {OP_IMM | OPSIZE16, FRAGMENT_WORD(900), OP_JUMP | OPSIZE16, FRAGMENT_WORD(466)};
static const byte fragment5[] = {OP_IMM | OPSIZE16, FRAGMENT_WORD(901), OP_JUMP | OPSIZE16, FRAGMENT_WORD(488)};
static const byte fragment6[] = {OP_IMM | OPSIZE16, FRAGMENT_WORD(903), OP_JUMP | OPSIZE16, FRAGMENT_WORD(516)};
static const byte fragment7[] = {OP_IMM | OPSIZE16, FRAGMENT_WORD(908), OP_JUMP | OPSIZE16, FRAGMENT_WORD(616)};
static const byte fragment8[] = {OP_IMM | OPSIZE16, FRAGMENT_WORD(910), OP_JUMP | OPSIZE16, FRAGMENT_WORD(644)};
static const byte fragment9[] = {OP_JUMP | OPSIZE8, 123};
static const byte fragment10[] = {OP_IMM | OPSIZE16, FRAGMENT_WORD(160), OP_JUMP | OPSIZE16, FRAGMENT_WORD(136)};
static const byte fragment11[] = {OP_JMPTRUE | OPSIZE16, FRAGMENT_WORD(1572),
		OP_ONE, OP_LIBCALL | OPSIZE8, 14,									// Re-show the cursor
		OP_IMM | OPSIZE16, FRAGMENT_WORD(322), OP_LIBCALL | OPSIZE8, 46,	// Give back the whistle
		OP_JUMP | OPSIZE16, FRAGMENT_WORD(1661)};
static const byte fragment12[] = {OP_JMPTRUE | OPSIZE16, FRAGMENT_WORD(1491),
		OP_ONE, OP_LIBCALL | OPSIZE8, 14,									// Re-show the cursor
		OP_IMM | OPSIZE16, FRAGMENT_WORD(322), OP_LIBCALL | OPSIZE8, 46,	// Give back the whistle
		OP_JUMP | OPSIZE16, FRAGMENT_WORD(1568)};
static const byte fragment13[] = {OP_ZERO, OP_GSTORE | OPSIZE16, FRAGMENT_WORD(306)};
static const byte fragment14[] = {OP_LIBCALL | OPSIZE8, 58,
		OP_IMM, FRAGMENT_DWORD((42 << 23)), OP_ONE, OP_ZERO, OP_LIBCALL | OPSIZE8, 44,
		OP_LIBCALL | OPSIZE8, 97, OP_JUMP | OPSIZE16, FRAGMENT_WORD(2220)
};

#undef FRAGMENT_WORD

const WorkaroundEntry workaroundList[] = {
	// DW1-SCN: Global 206 is whether Rincewind is trying to take the
	// book back to the present. In the GRA version, it was global 373,
	// and was reset when he is returned to the past, but was forgotten
	// in the SCN version, so this ensures the flag is properly reset.
	{TINSEL_V1, true, false, Common::kPlatformUnknown, 427942095, 1, sizeof(fragment1), fragment1},

	// DW1-GRA: Rincewind exiting the Inn is blocked by the luggage.
	// Whilst you can then move into walkable areas, saving and
	// restoring the game, it will error if you try to move. This
	// fragment turns off NPC blocking for the Outside Inn rooms so that
	// the luggage won't block Past Outside Inn.
	// See bug report #2525010.
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 444622076, 0,  sizeof(fragment2), fragment2},
	// Present Outside Inn
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 352600876, 0,  sizeof(fragment2), fragment2},

	// DW1-GRA: Talking to palace guards in Act 2 gives !!!HIGH
	// STRING||| - this happens if you initiate dialog with one of the
	// guards, but not the other. So these fragments provide the correct
	// talk parameters where needed.
	// See bug report #2831159.
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 310506872, 463, sizeof(fragment4), fragment4},
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 310506872, 485, sizeof(fragment5), fragment5},
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 310506872, 513, sizeof(fragment6), fragment6},
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 310506872, 613, sizeof(fragment7), fragment7},
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 310506872, 641, sizeof(fragment8), fragment8},

	// DW1-SCN: The script for the lovable street-Starfish does a
	// 'StopSample' after flicking the coin to ensure it's sound is
	// stopped, but which also accidentally can stop any active
	// conversation with the Amazon.
	{TINSEL_V1, true, false, Common::kPlatformUnknown, 394640351, 121, sizeof(fragment9), fragment9},

	// DW2: In the garden, global #490 is set when the bees begin their
	// 'out of hive' animation, and reset when done. But if the game is
	// saved/restored during it, the animation sequence is reset without
	// the global being cleared. This causes bugs in several actions
	// which try to disable the bees animation, since they wait
	// indefinitely for the global to be cleared, incorrectly believing
	// the animation is currently playing. This includes:
	//  * Giving the brochure to the beekeeper (bug #2680397)
	//  * Stealing the mallets from the wizards (bug #2820788).
	// This fix ensures that the global is reset when the Garden scene
	// is loaded (both entering and restoring a game).
	{TINSEL_V2, true, false, Common::kPlatformUnknown, 2888147476U, 0, sizeof(fragment3), fragment3},

	// DW1-GRA: Corrects text being drawn partially off-screen during
	// the blackboard description of the Librarian.
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 293831402, 133, sizeof(fragment10), fragment10},

	// DW1-GRA/SCN: Corrects the dead-end of being able to give the
	// whistle back to the pirate before giving him the parrot.
	// See bug report #2934211.
	{TINSEL_V1, true, false, Common::kPlatformUnknown, 352601285, 1569, sizeof(fragment11), fragment11},
	{TINSEL_V1, false, false, Common::kPlatformUnknown, 352602304, 1488, sizeof(fragment12), fragment12},

	// DW2: Corrects a bug with global 306 not being cleared if you leave
	// the marketplace scene whilst D'Blah is talking (even if it's not
	// actually audible); returning to the scene and clicking on him multiple
	// times would cause the game to crash
	{TINSEL_V2, true, false, Common::kPlatformUnknown, 1109294728, 0, sizeof(fragment13), fragment13},

	// DW1 PSX DEMO: Alters a script in the PSX DW1 demo to show the Idle animation scene rather than
	// quitting the game when no user input happens for a while
	{TINSEL_V1, true, true, Common::kPlatformPSX, 0, 2186, sizeof(fragment14), fragment14},

	{TINSEL_V0, false, false, Common::kPlatformUnknown, 0, 0, 0, NULL}
};

//----------------- LOCAL GLOBAL DATA --------------------

/**
 * Keeps the code array pointer up to date.
 */
void LockCode(INT_CONTEXT *ic) {
	if (ic->GSort == GS_MASTER) {
		if (TinselV2)
			// Get the srcipt handle from a specific global chunk
			ic->code = (byte *)LockMem(g_hMasterScript);
		else
			ic->code = (byte *)FindChunk(MASTER_SCNHANDLE, CHUNK_PCODE);
	} else
		ic->code = (byte *)LockMem(ic->hCode);
}

/**
 * Find a free interpret context and allocate it to the calling process.
 */
static INT_CONTEXT *AllocateInterpretContext(GSORT gsort) {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = g_icList; i < NUM_INTERPRET; i++, pic++) {
		if (pic->GSort == GS_NONE) {
			pic->pProc = CoroScheduler.getCurrentProcess();
			pic->GSort = gsort;
			return pic;
		}
#ifdef DEBUG
		else {
			if (pic->pProc == CoroScheduler.getCurrentProcess())
				error("Found unreleased interpret context");
		}
#endif
	}

	error("Out of interpret contexts");
}

static void FreeWaitCheck(PINT_CONTEXT pic, bool bVoluntary) {
	int i;

	// Is this waiting for something?
	if (pic->waitNumber1) {
		for (i = 0; i < NUM_INTERPRET; i++) {
			if ((g_icList + i)->waitNumber2 == pic->waitNumber1) {
				(g_icList + i)->waitNumber2 = 0;
				break;
			}
		}
	}

	// Is someone waiting for this?
	if (pic->waitNumber2) {
		for (i = 0; i < NUM_INTERPRET; i++) {
			if ((g_icList + i)->waitNumber1 == pic->waitNumber2) {
				(g_icList + i)->waitNumber1 = 0;
				(g_icList + i)->resumeCode = bVoluntary ? RES_FINISHED : RES_CUTSHORT;
				CoroScheduler.reschedule((g_icList + i)->pProc);
				break;
			}
		}
		assert(i < NUM_INTERPRET);
	}
}

/**
 * Normal release of an interpret context.
 * Called from the end of Interpret().
 */
static void FreeInterpretContextPi(INT_CONTEXT *pic) {
	FreeWaitCheck(pic, true);
	if (TinselV2)
		memset(pic, 0, sizeof(INT_CONTEXT));
	pic->GSort = GS_NONE;
}

/**
 * Free interpret context owned by a dying process.
 * Ensures that interpret contexts don't get lost when an Interpret()
 * call doesn't complete.
 */
void FreeInterpretContextPr(Common::PROCESS *pProc) {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = g_icList; i < NUM_INTERPRET; i++, pic++) {
		if (pic->GSort != GS_NONE && pic->pProc == pProc) {
			FreeWaitCheck(pic, false);
			if (TinselV2)
				memset(pic, 0, sizeof(INT_CONTEXT));
			pic->GSort = GS_NONE;
			break;
		}
	}
}

/**
 * Free all interpret contexts except for the master script's
 */
void FreeMostInterpretContexts() {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = g_icList; i < NUM_INTERPRET; i++, pic++) {
		if ((pic->GSort != GS_MASTER) && (pic->GSort != GS_GPROCESS)) {
			memset(pic, 0, sizeof(INT_CONTEXT));
			pic->GSort = GS_NONE;
		}
	}
}

/**
 * Free the master script's interpret context.
 */
void FreeMasterInterpretContext() {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = g_icList; i < NUM_INTERPRET; i++, pic++)	{
		if ((pic->GSort == GS_MASTER) || (pic->GSort == GS_GPROCESS)) {
			memset(pic, 0, sizeof(INT_CONTEXT));
			pic->GSort = GS_NONE;
			return;
		}
	}
}

/**
 * Allocate and initialize an interpret context.
 * Called from a process prior to Interpret().
 * @param gsort			which sort of code
 * @param hCode			Handle to code to execute
 * @param event			Causal event
 * @param hpoly			Associated polygon (if any)
 * @param actorId		Associated actor (if any)
 * @param pinvo			Associated inventory object
 */
INT_CONTEXT *InitInterpretContext(GSORT gsort, SCNHANDLE hCode,	TINSEL_EVENT event,
		HPOLYGON hpoly, int actorid, INV_OBJECT *pinvo, int myEscape) {
	INT_CONTEXT *ic;

	ic = AllocateInterpretContext(gsort);

	// Previously parameters to Interpret()
	ic->hCode = hCode;
	LockCode(ic);
	ic->event = event;
	ic->hPoly = hpoly;
	ic->idActor = actorid;
	ic->pinvo = pinvo;

	// Previously local variables in Interpret()
	ic->bHalt = false;		// set to exit interpeter
	ic->escOn = myEscape > 0;
	ic->myEscape = myEscape;
	ic->sp = 0;
	ic->bp = ic->sp + 1;
	ic->ip = 0;			// start of code

	ic->resumeState = RES_NOT;

	return ic;
}

/**
 * Allocate and initialize an interpret context with restored data.
 */
INT_CONTEXT *RestoreInterpretContext(INT_CONTEXT *ric) {
	INT_CONTEXT *ic;

	ic = AllocateInterpretContext(GS_NONE);	// Sort will soon be overridden

	memcpy(ic, ric, sizeof(INT_CONTEXT));
	ic->pProc = CoroScheduler.getCurrentProcess();
	ic->resumeState = RES_1;

	LockCode(ic);

	return ic;
}

/**
 * Allocates enough RAM to hold the global Glitter variables.
 */
void RegisterGlobals(int num) {
	if (g_pGlobals == NULL) {
		g_numGlobals = num;

		g_hMasterScript = !TinselV2 ? 0 :
			READ_LE_UINT32(FindChunk(MASTER_SCNHANDLE, CHUNK_MASTER_SCRIPT));

		// Allocate RAM for pGlobals and make sure it's allocated
		g_pGlobals = (int32 *)calloc(g_numGlobals, sizeof(int32));
		if (g_pGlobals == NULL) {
			error("Cannot allocate memory for global data");
		}

		// Allocate RAM for interpret contexts and make sure it's allocated
		g_icList = (INT_CONTEXT *)calloc(NUM_INTERPRET, sizeof(INT_CONTEXT));
		if (g_icList == NULL) {
			error("Cannot allocate memory for interpret contexts");
		}
		CoroScheduler.setResourceCallback(FreeInterpretContextPr);
	} else {
		// Check size is still the same
		assert(g_numGlobals == num);

		memset(g_pGlobals, 0, g_numGlobals * sizeof(int32));
		memset(g_icList, 0, NUM_INTERPRET * sizeof(INT_CONTEXT));
	}

	if (TinselV2) {
		// read initial values
		CdCD(Common::nullContext);

		Common::File f;
		if (!f.open(GLOBALS_FILENAME))
			error(CANNOT_FIND_FILE, GLOBALS_FILENAME);

		int32 length = f.readSint32LE();
		if (length != num)
			error(FILE_IS_CORRUPT, GLOBALS_FILENAME);

		for (int i = 0; i < length; ++i)
			g_pGlobals[i] = f.readSint32LE();

		if (f.eos() || f.err())
			error(FILE_IS_CORRUPT, GLOBALS_FILENAME);

		f.close();
	}
}

void FreeGlobals() {
	free(g_pGlobals);
	g_pGlobals = NULL;

	free(g_icList);
	g_icList = NULL;
}

/**
 * (Un)serialize the global data for save/restore game.
 */
void syncGlobInfo(Common::Serializer &s) {
	for (int i = 0; i < g_numGlobals; i++) {
		s.syncAsSint32LE(g_pGlobals[i]);
	}
}

/**
 * (Un)serialize an interpreter context for save/restore game.
 */
void INT_CONTEXT::syncWithSerializer(Common::Serializer &s) {
	if (s.isLoading()) {
		// Null out the pointer fields
		pProc = NULL;
		code = NULL;
		pinvo = NULL;
	}
	// Write out used fields
	s.syncAsUint32LE(GSort);
	s.syncAsUint32LE(hCode);
	s.syncAsUint32LE(event);
	s.syncAsSint32LE(hPoly);
	s.syncAsSint32LE(idActor);

	for (int i = 0; i < PCODE_STACK_SIZE; ++i)
		s.syncAsSint32LE(stack[i]);

	s.syncAsSint32LE(sp);
	s.syncAsSint32LE(bp);
	s.syncAsSint32LE(ip);
	s.syncAsUint32LE(bHalt);
	s.syncAsUint32LE(escOn);
	s.syncAsSint32LE(myEscape);
}

/**
 * Return pointer to and size of global data for save/restore game.
 */
void SaveInterpretContexts(INT_CONTEXT *sICInfo) {
	memcpy(sICInfo, g_icList, NUM_INTERPRET * sizeof(INT_CONTEXT));
}

/**
 * Fetches up to 4 bytes from the code script
 */
static int32 GetBytes(const byte *scriptCode, const WorkaroundEntry* &wkEntry, int &ip, uint numBytes) {
	assert(numBytes <= 4 && numBytes != 3);
	const byte *code = scriptCode;

	if (wkEntry != NULL) {
		if (ip >= wkEntry->numBytes) {
			// Finished the workaround
			ip = wkEntry->ip;
			wkEntry = NULL;
		} else {
			code = wkEntry->script;
		}
	}

	uint32 tmp;
	switch (numBytes) {
	case 0:
		// Instruction byte
		tmp = code[ip++ * (TinselV0 ? 4 : 1)];
		break;
	case 1:
		// Fetch and sign extend a 8 bit value to 32 bits.
		tmp = (int8)code[ip++];
		break;
	case 2:
		// Fetch and sign extend a 16 bit value to 32 bits.
		tmp = (int16)READ_LE_UINT16(code + ip);
		ip += 2;
		break;
	default:
		if (TinselV0)
			tmp = (int32)READ_LE_UINT32(code + ip++ * 4);
		else {
			tmp = (int32)READ_LE_UINT32(code + ip);
			ip += 4;
		}
		break;
	}

	return tmp;
}

/**
 * Fetch (and sign extend, if necessary) a 8/16/32 bit value from the code
 * stream and advance the instruction pointer accordingly.
 */
static int32 Fetch(byte opcode, const byte *code, const WorkaroundEntry* &wkEntry, int &ip) {
	if (TinselV0)
		// Fetch a 32 bit value.
		return GetBytes(code, wkEntry, ip, 4);
	else if (opcode & OPSIZE8)
		// Fetch and sign extend a 8 bit value to 32 bits.
		return GetBytes(code, wkEntry, ip, 1);
	else if (opcode & OPSIZE16)
		return GetBytes(code, wkEntry, ip, 2);

	return GetBytes(code, wkEntry, ip, 4);
}

/**
 * Interprets the PCODE instructions in the code array.
 */
void Interpret(CORO_PARAM, INT_CONTEXT *ic) {
	do {
		int tmp, tmp2;
		int ip = ic->ip;
		const WorkaroundEntry *wkEntry = ic->fragmentPtr;

		if (wkEntry == NULL) {
			// Check to see if a workaround fragment needs to be executed
			for (wkEntry = workaroundList; wkEntry->script != NULL; ++wkEntry) {
				if ((wkEntry->version == TinselVersion) &&
					(wkEntry->hCode == ic->hCode) &&
					(wkEntry->ip == ip) &&
					(wkEntry->isDemo == _vm->getIsADGFDemo()) &&
					((wkEntry->platform == Common::kPlatformUnknown) || (wkEntry->platform == _vm->getPlatform())) &&
					(!TinselV1 || (wkEntry->scnFlag == ((_vm->getFeatures() & GF_SCNFILES) != 0)))) {
					// Point to start of workaround fragment
					ip = 0;
					break;
				}
			}
			if (wkEntry->script == NULL)
				wkEntry = NULL;
		}

		byte opcode = (byte)GetBytes(ic->code, wkEntry, ip, 0);
		if (TinselV0 && ((opcode & OPMASK) > OP_IMM))
			opcode += 3;

		debug(7, "ip=%d  Opcode %d (-> %d)", ic->ip, opcode, opcode & OPMASK);
		switch (opcode & OPMASK) {
		case OP_HALT:			// end of program

			ic->bHalt = true;
			break;

		case OP_IMM:			// loads immediate data onto stack
		case OP_STR:			// loads string handle onto stack
		case OP_FILM:			// loads film handle onto stack
		case OP_CDFILM:			// loads film handle onto stack
		case OP_FONT:			// loads font handle onto stack
		case OP_PAL:			// loads palette handle onto stack

			ic->stack[++ic->sp] = Fetch(opcode, ic->code, wkEntry, ip);
			break;

		case OP_ZERO:			// loads zero onto stack
			ic->stack[++ic->sp] = 0;
			break;

		case OP_ONE:			// loads one onto stack
			ic->stack[++ic->sp] = 1;
			break;

		case OP_MINUSONE:		// loads minus one onto stack
			ic->stack[++ic->sp] = -1;
			break;

		case OP_LOAD:			// loads local variable onto stack

			ic->stack[++ic->sp] = ic->stack[ic->bp + Fetch(opcode, ic->code, wkEntry, ip)];
			break;

		case OP_GLOAD:				// loads global variable onto stack

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			assert(0 <= tmp && tmp < g_numGlobals);
			ic->stack[++ic->sp] = g_pGlobals[tmp];
			break;

		case OP_STORE:				// pops stack and stores in local variable

			ic->stack[ic->bp + Fetch(opcode, ic->code, wkEntry, ip)] = ic->stack[ic->sp--];
			break;

		case OP_GSTORE:				// pops stack and stores in global variable

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			assert(0 <= tmp && tmp < g_numGlobals);
			g_pGlobals[tmp] = ic->stack[ic->sp--];
			break;

		case OP_CALL:				// procedure call

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			//assert(0 <= tmp && tmp < codeSize);	// TODO: Verify jumps are not out of bounds
			ic->stack[ic->sp + 1] = 0;	// static link
			ic->stack[ic->sp + 2] = ic->bp;	// dynamic link
			ic->stack[ic->sp + 3] = ip;	// return address
			ic->bp = ic->sp + 1;		// set new base pointer
			ip = tmp;	// set ip to procedure address
			break;

		case OP_LIBCALL:		// library procedure or function call

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			// NOTE: Interpret() itself is not using the coroutine facilities,
			// but still accepts a CORO_PARAM, so from the outside it looks
			// like a coroutine. In fact it may still acts as a kind of "proxy"
			// for some underlying coroutine. To enable this, we just pass on
			// 'coroParam' to CallLibraryRoutine(). If we then detect that
			// coroParam was set to a non-zero value, this means that some
			// coroutine code did run at some point, and we are now supposed
			// to sleep or die -- hence, we 'return' if coroParam != 0.
			//
			// This works because Interpret() is fully re-entrant: If we return
			// now and are later called again, then we will end up in the very
			// same spot (i.e. here).
			//
			// The reasons we do it this way, instead of turning Interpret into
			// a 'proper' coroutine are (1) we avoid implementation problems
			// (CORO_INVOKE involves adding 'case' statements, but Interpret
			// already has a huge switch/case, so that would not work out of the
			// box), (2) we incurr less overhead, (3) it's easier to debug,
			// (4) it's simply cool ;).
			tmp2 = CallLibraryRoutine(coroParam, tmp, &ic->stack[ic->sp], ic, &ic->resumeState);
			if (coroParam)
				return;
			if (!TinselV0)
				ic->sp += tmp2;
			LockCode(ic);
			if (TinselV2 && (ic->resumeState == RES_1))
				ic->resumeState = RES_NOT;
			break;

		case OP_RET:			// procedure return

			ic->sp = ic->bp - 1;		// restore stack
			ip = ic->stack[ic->sp + 3];	// return address
			ic->bp = ic->stack[ic->sp + 2];	// restore previous base pointer
			break;

		case OP_ALLOC:			// allocate storage on stack

			ic->sp += (int32)Fetch(opcode, ic->code, wkEntry, ip);
			break;

		case OP_JUMP:	// unconditional jump

			ip = Fetch(opcode, ic->code, wkEntry, ip);
			wkEntry = NULL;					// In case a jump occurs from a workaround
			break;

		case OP_JMPFALSE:	// conditional jump

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			if (ic->stack[ic->sp--] == 0) {
				// condition satisfied - do the jump
				ip = tmp;
				wkEntry = NULL;					// In case a jump occurs from a workaround
			}
			break;

		case OP_JMPTRUE:	// conditional jump

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			if (ic->stack[ic->sp--] != 0) {
				// condition satisfied - do the jump
				ip = tmp;
				wkEntry = NULL;					// In case a jump occurs from a workaround
			}
			break;

		case OP_EQUAL:			// tests top two items on stack for equality
		case OP_LESS:			// tests top two items on stack
		case OP_LEQUAL:			// tests top two items on stack
		case OP_NEQUAL:			// tests top two items on stack
		case OP_GEQUAL:			// tests top two items on stack
		case OP_GREAT:			// tests top two items on stack
		case OP_LOR:			// logical or of top two items on stack and replaces with result
		case OP_LAND:			// logical ands top two items on stack and replaces with result

			// pop one operand
			ic->sp--;
			assert(ic->sp >= 0);
			tmp = ic->stack[ic->sp];
			tmp2 = ic->stack[ic->sp + 1];

			// replace other operand with result of operation
			switch (opcode) {
			case OP_EQUAL:  tmp = (tmp == tmp2); break;
			case OP_LESS:   tmp = (tmp <  tmp2); break;
			case OP_LEQUAL: tmp = (tmp <= tmp2); break;
			case OP_NEQUAL: tmp = (tmp != tmp2); break;
			case OP_GEQUAL: tmp = (tmp >= tmp2); break;
			case OP_GREAT:  tmp = (tmp >  tmp2); break;

			case OP_LOR:    tmp = (tmp || tmp2); break;
			case OP_LAND:   tmp = (tmp && tmp2); break;
			}

			ic->stack[ic->sp] = tmp;
			break;

		case OP_PLUS:			// adds top two items on stack and replaces with result
		case OP_MINUS:			// subs top two items on stack and replaces with result
		case OP_MULT:			// multiplies top two items on stack and replaces with result
		case OP_DIV:			// divides top two items on stack and replaces with result
		case OP_MOD:			// divides top two items on stack and replaces with modulus
		case OP_AND:			// bitwise ands top two items on stack and replaces with result
		case OP_OR:				// bitwise ors top two items on stack and replaces with result
		case OP_EOR:			// bitwise exclusive ors top two items on stack and replaces with result

			// pop one operand
			ic->sp--;
			assert(ic->sp >= 0);
			tmp = ic->stack[ic->sp];
			tmp2 = ic->stack[ic->sp + 1];

			// replace other operand with result of operation
			switch (opcode) {
			case OP_PLUS:   tmp += tmp2; break;
			case OP_MINUS:  tmp -= tmp2; break;
			case OP_MULT:   tmp *= tmp2; break;
			case OP_DIV:    tmp /= tmp2; break;
			case OP_MOD:    tmp %= tmp2; break;
			case OP_AND:    tmp &= tmp2; break;
			case OP_OR:     tmp |= tmp2; break;
			case OP_EOR:    tmp ^= tmp2; break;
			}
			ic->stack[ic->sp] = tmp;
			break;

		case OP_NOT:			// logical nots top item on stack

			ic->stack[ic->sp] = !ic->stack[ic->sp];
			break;

		case OP_COMP:			// complements top item on stack
			ic->stack[ic->sp] = ~ic->stack[ic->sp];
			break;

		case OP_NEG:			// negates top item on stack
			ic->stack[ic->sp] = -ic->stack[ic->sp];
			break;

		case OP_DUP:			// duplicates top item on stack
			ic->stack[ic->sp + 1] = ic->stack[ic->sp];
			ic->sp++;
			break;

		case OP_ESCON:
			g_bNoPause = true;
			ic->escOn = true;
			ic->myEscape = GetEscEvents();
			break;

		case OP_ESCOFF:
			ic->escOn = false;
			ic->myEscape = 0;
			break;

		default:
			error("Interpret() - Unknown opcode");
		}

		// check for stack under-overflow
		assert(ic->sp >= 0 && ic->sp < PCODE_STACK_SIZE);
		ic->ip = ip;
		ic->fragmentPtr = wkEntry;
	} while (!ic->bHalt);

	// make sure stack is unwound
	assert(ic->sp == 0);

	FreeInterpretContextPi(ic);
}

/**
 * Associates an interpret context with the
 * process that will run it.
 */
void AttachInterpret(INT_CONTEXT *pic, Common::PROCESS *pProc) {
	// Attach the process which is using this context
	pic->pProc = pProc;
}

/**
 * Generate a number that isn't being used.
 */
static uint32 UniqueWaitNumber() {
	uint32 retval;
	int i;

	for (retval = DwGetCurrentTime(); 1; retval--) {
		if (retval == 0)
			retval = (uint32)-1;

		for (i = 0; i < NUM_INTERPRET; i++) {
			if ((g_icList+i)->waitNumber1 == retval
			 || (g_icList+i)->waitNumber2 == retval)
				break;
		}

		if (i == NUM_INTERPRET)
			return retval;
	}
}

/**
 * WaitInterpret
 */
void WaitInterpret(CORO_PARAM, Common::PPROCESS pWaitProc, bool *result) {
	int i;
	Common::PPROCESS currentProcess = CoroScheduler.getCurrentProcess();
	assert(currentProcess);
	assert(currentProcess != pWaitProc);
	if (result) *result = false;

	/*
	 * Calling process is the waiter, find its interpret context.
	 */

	CORO_BEGIN_CONTEXT;
		PINT_CONTEXT picWaiter, picWaitee;
	CORO_END_CONTEXT(_ctx);


	CORO_BEGIN_CODE(_ctx);

	for (i = 0, _ctx->picWaiter = g_icList; i < NUM_INTERPRET; i++, _ctx->picWaiter++) {
		if (_ctx->picWaiter->GSort != GS_NONE && _ctx->picWaiter->pProc == currentProcess) {
			break;
		}
	}

	/*
	 * Find the interpret context of the process we're waiting for
	 */
	for (i = 0, _ctx->picWaitee = g_icList; i < NUM_INTERPRET; i++, _ctx->picWaitee++) {
		if (_ctx->picWaitee->GSort != GS_NONE && _ctx->picWaitee->pProc == pWaitProc) {
			break;
		}
	}

	/*
	 * Set the first as waiting for the second
	 */
	assert(_ctx->picWaitee->waitNumber2 == 0);
	_ctx->picWaiter->waitNumber1 = _ctx->picWaitee->waitNumber2 = UniqueWaitNumber();
	_ctx->picWaiter->resumeCode = RES_WAITING;

	/*
	 * Wait for it
	 */
	CORO_GIVE_WAY;
	while (_ctx->picWaiter->resumeCode == RES_WAITING) {
		CORO_SLEEP(1);
	}

	if (result)
		*result = (_ctx->picWaiter->resumeCode == RES_FINISHED);
	CORO_END_CODE;
}

/**
 * CheckOutWaiters
 */
void CheckOutWaiters() {
	int i, j;

	// Check all waited for have someone waiting
	for (i = 0; i < NUM_INTERPRET; i++)	{
		// If someone is supposedly waiting for this one
		if ((g_icList + i)->GSort != GS_NONE && (g_icList + i)->waitNumber2) {
			// Someone really must be waiting for this one
			for (j = 0; j < NUM_INTERPRET; j++) {
				if ((g_icList + j)->GSort != GS_NONE
				 && (g_icList + j)->waitNumber1 == (g_icList + i)->waitNumber2) {
					break;
				}
			}
			assert(j < NUM_INTERPRET);
		}
	}

	// Check waiting for someone to wait for
	for (i = 0; i < NUM_INTERPRET; i++) {
		// If someone is supposedly waiting for this one
		if ((g_icList + i)->GSort != GS_NONE && (g_icList + i)->waitNumber1) {
			// Someone really must be waiting for this one
			for (j = 0; j < NUM_INTERPRET; j++) {
				if ((g_icList + j)->GSort != GS_NONE
				 && (g_icList + j)->waitNumber2 == (g_icList + i)->waitNumber1) {
					break;
				}
			}
			assert(j < NUM_INTERPRET);
		}
	}
}

} // End of namespace Tinsel
