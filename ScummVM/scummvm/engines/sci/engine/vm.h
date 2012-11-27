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
 */

#ifndef SCI_ENGINE_VM_H
#define SCI_ENGINE_VM_H

/* VM and kernel declarations */

#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/resource.h"	// for SciVersion

#include "common/util.h"

namespace Sci {

class SegManager;
struct EngineState;
class Object;
class ResourceManager;

/** Number of bytes to be allocated for the stack */
#define VM_STACK_SIZE 0x1000

/** Magical object identifier */
#define SCRIPT_OBJECT_MAGIC_NUMBER 0x1234

/** Offset of this identifier */
#define SCRIPT_OBJECT_MAGIC_OFFSET (getSciVersion() < SCI_VERSION_1_1 ? -8 : 0)

/** Stack pointer value: Use predecessor's value */
#define CALL_SP_CARRY NULL

/** Types of selectors as returned by lookupSelector() below. */
enum SelectorType {
	kSelectorNone = 0,
	kSelectorVariable,
	kSelectorMethod
};

struct Class {
	int script; ///< number of the script the class is in, -1 for non-existing
	reg_t reg; ///< offset; script-relative offset, segment: 0 if not instantiated
};

// A reference to an object's variable.
// The object is stored as a reg_t, the variable as an index into _variables
struct ObjVarRef {
	reg_t obj;
	int varindex;

	reg_t* getPointer(SegManager *segMan) const;
};

enum ExecStackType {
	EXEC_STACK_TYPE_CALL = 0,
	EXEC_STACK_TYPE_KERNEL = 1,
	EXEC_STACK_TYPE_VARSELECTOR = 2
};

struct ExecStack {
	reg_t objp;  ///< Pointer to the beginning of the current object
	reg_t sendp; ///< Pointer to the object containing the invoked method

	union {
		ObjVarRef varp; // Variable pointer for r/w access
		reg32_t pc;       // Pointer to the initial program counter. Not accurate for the TOS element
	} addr;

	StackPtr fp; // Frame pointer
	StackPtr sp; // Stack pointer

	int argc;
	StackPtr variables_argp; // Argument pointer

	SegmentId local_segment; // local variables etc

	Selector debugSelector;   // The selector which was used to call or -1 if not applicable
	int debugExportId;        // The exportId which was called or -1 if not applicable
	int debugLocalCallOffset; // Local call offset or -1 if not applicable
	int debugOrigin;          // The stack frame position the call was made from, or -1 if it was the initial call
	ExecStackType type;

	reg_t* getVarPointer(SegManager *segMan) const;

	ExecStack(reg_t objp_, reg_t sendp_, StackPtr sp_, int argc_, StackPtr argp_,
				SegmentId localsSegment_, reg32_t pc_, Selector debugSelector_,
				int debugExportId_, int debugLocalCallOffset_, int debugOrigin_,
				ExecStackType type_) {
		objp = objp_;
		sendp = sendp_;
		// varp is set separately for varselector calls
		addr.pc = pc_;
		fp = sp = sp_;
		argc = argc_;
		variables_argp = argp_;
		*variables_argp = make_reg(0, argc);  // The first argument is argc
		if (localsSegment_ != 0xFFFF)
			local_segment = localsSegment_;
		else
			local_segment = pc_.getSegment();
		debugSelector = debugSelector_;
		debugExportId = debugExportId_;
		debugLocalCallOffset = debugLocalCallOffset_;
		debugOrigin = debugOrigin_;
		type = type_;
	}
};

enum {
	VAR_GLOBAL = 0,
	VAR_LOCAL = 1,
	VAR_TEMP = 2,
	VAR_PARAM = 3
};

/** Number of kernel calls in between gcs; should be < 50000 */
enum {
	GC_INTERVAL = 0x8000
};

enum SciOpcodes {
	op_bnot     = 0x00,	// 000
	op_add      = 0x01,	// 001
	op_sub      = 0x02,	// 002
	op_mul      = 0x03,	// 003
	op_div      = 0x04,	// 004
	op_mod      = 0x05,	// 005
	op_shr      = 0x06,	// 006
	op_shl      = 0x07,	// 007
	op_xor      = 0x08,	// 008
	op_and      = 0x09,	// 009
	op_or       = 0x0a,	// 010
	op_neg      = 0x0b,	// 011
	op_not      = 0x0c,	// 012
	op_eq_      = 0x0d,	// 013
	op_ne_      = 0x0e,	// 014
	op_gt_      = 0x0f,	// 015
	op_ge_      = 0x10,	// 016
	op_lt_      = 0x11,	// 017
	op_le_      = 0x12,	// 018
	op_ugt_     = 0x13,	// 019
	op_uge_     = 0x14,	// 020
	op_ult_     = 0x15,	// 021
	op_ule_     = 0x16,	// 022
	op_bt       = 0x17,	// 023
	op_bnt      = 0x18,	// 024
	op_jmp      = 0x19,	// 025
	op_ldi      = 0x1a,	// 026
	op_push     = 0x1b,	// 027
	op_pushi    = 0x1c,	// 028
	op_toss     = 0x1d,	// 029
	op_dup      = 0x1e,	// 030
	op_link     = 0x1f,	// 031
	op_call     = 0x20,	// 032
	op_callk    = 0x21,	// 033
	op_callb    = 0x22,	// 034
	op_calle    = 0x23,	// 035
	op_ret      = 0x24,	// 036
	op_send     = 0x25,	// 037
	// dummy      0x26,	// 038
	// dummy      0x27,	// 039
	op_class    = 0x28,	// 040
	// dummy      0x29,	// 041
	op_self     = 0x2a,	// 042
	op_super    = 0x2b,	// 043
	op_rest     = 0x2c,	// 044
	op_lea      = 0x2d,	// 045
	op_selfID   = 0x2e,	// 046
	// dummy      0x2f	// 047
	op_pprev    = 0x30,	// 048
	op_pToa     = 0x31,	// 049
	op_aTop     = 0x32,	// 050
	op_pTos     = 0x33,	// 051
	op_sTop     = 0x34,	// 052
	op_ipToa    = 0x35,	// 053
	op_dpToa    = 0x36,	// 054
	op_ipTos    = 0x37,	// 055
	op_dpTos    = 0x38,	// 056
	op_lofsa    = 0x39,	// 057
	op_lofss    = 0x3a,	// 058
	op_push0    = 0x3b,	// 059
	op_push1    = 0x3c,	// 060
	op_push2    = 0x3d,	// 061
	op_pushSelf = 0x3e,	// 062
	op_line     = 0x3f,	// 063
	//
	op_lag      = 0x40,	// 064
	op_lal      = 0x41,	// 065
	op_lat      = 0x42,	// 066
	op_lap      = 0x43,	// 067
	op_lsg      = 0x44,	// 068
	op_lsl      = 0x45,	// 069
	op_lst      = 0x46,	// 070
	op_lsp      = 0x47,	// 071
	op_lagi     = 0x48,	// 072
	op_lali     = 0x49,	// 073
	op_lati     = 0x4a,	// 074
	op_lapi     = 0x4b,	// 075
	op_lsgi     = 0x4c,	// 076
	op_lsli     = 0x4d,	// 077
	op_lsti     = 0x4e,	// 078
	op_lspi     = 0x4f,	// 079
	//
	op_sag      = 0x50,	// 080
	op_sal      = 0x51,	// 081
	op_sat      = 0x52,	// 082
	op_sap      = 0x53,	// 083
	op_ssg      = 0x54,	// 084
	op_ssl      = 0x55,	// 085
	op_sst      = 0x56,	// 086
	op_ssp      = 0x57,	// 087
	op_sagi     = 0x58,	// 088
	op_sali     = 0x59,	// 089
	op_sati     = 0x5a,	// 090
	op_sapi     = 0x5b,	// 091
	op_ssgi     = 0x5c,	// 092
	op_ssli     = 0x5d,	// 093
	op_ssti     = 0x5e,	// 094
	op_sspi     = 0x5f,	// 095
	//
	op_plusag   = 0x60,	// 096
	op_plusal   = 0x61,	// 097
	op_plusat   = 0x62,	// 098
	op_plusap   = 0x63,	// 099
	op_plussg   = 0x64,	// 100
	op_plussl   = 0x65,	// 101
	op_plusst   = 0x66,	// 102
	op_plussp   = 0x67,	// 103
	op_plusagi  = 0x68,	// 104
	op_plusali  = 0x69,	// 105
	op_plusati  = 0x6a,	// 106
	op_plusapi  = 0x6b,	// 107
	op_plussgi  = 0x6c,	// 108
	op_plussli  = 0x6d,	// 109
	op_plussti  = 0x6e,	// 110
	op_plusspi  = 0x6f,	// 111
	//
	op_minusag  = 0x70,	// 112
	op_minusal  = 0x71,	// 113
	op_minusat  = 0x72,	// 114
	op_minusap  = 0x73,	// 115
	op_minussg  = 0x74,	// 116
	op_minussl  = 0x75,	// 117
	op_minusst  = 0x76,	// 118
	op_minussp  = 0x77,	// 119
	op_minusagi = 0x78,	// 120
	op_minusali = 0x79,	// 121
	op_minusati = 0x7a,	// 122
	op_minusapi = 0x7b,	// 123
	op_minussgi = 0x7c,	// 124
	op_minussli = 0x7d,	// 125
	op_minussti = 0x7e,	// 126
	op_minusspi = 0x7f	// 127
};

void script_adjust_opcode_formats();

/**
 * Executes function pubfunct of the specified script.
 * @param[in] s				The state which is to be executed with
 * @param[in] script		The script which is called
 * @param[in] pubfunct		The exported script function which is to
 * 							be called
 * @param[in] sp			Stack pointer position
 * @param[in] calling_obj	The heap address of the object that
 * 							executed the call
 * @param[in] argc			Number of arguments supplied
 * @param[in] argp			Pointer to the first supplied argument
 * @return					A pointer to the new exec stack TOS entry
 */
ExecStack *execute_method(EngineState *s, uint16 script, uint16 pubfunct,
		StackPtr sp, reg_t calling_obj, uint16 argc, StackPtr argp);


/**
 * Executes a "send" or related operation to a selector.
 * @param[in] s			The EngineState to operate on
 * @param[in] send_obj	Heap address of the object to send to
 * @param[in] work_obj	Heap address of the object initiating the send
 * @param[in] sp		Stack pointer position
 * @param[in] framesize	Size of the send as determined by the "send"
 * 						operation
 * @param[in] argp		Pointer to the beginning of the heap block
 * 						containing the data to be sent. This area is a
 * 						succession of one or more sequences of
 * 						[selector_number][argument_counter] and then
 * 						"argument_counter" word entries with the
 * 						parameter values.
 * @return				A pointer to the new execution stack TOS entry
 */
ExecStack *send_selector(EngineState *s, reg_t send_obj, reg_t work_obj,
	StackPtr sp, int framesize, StackPtr argp);


/**
 * This function executes SCI bytecode
 * It executes the code on s->heap[pc] until it hits a 'ret' operation
 * while (stack_base == stack_pos). Requires s to be set up correctly.
 * @param[in] s			The state to use
 */
void run_vm(EngineState *s);

/**
 * Debugger functionality
 * @param[in] s					The state at which debugging should take place
 */
void script_debug(EngineState *s);

/**
 * Looks up a selector and returns its type and value
 * varindex is written to iff it is non-NULL and the selector indicates a property of the object.
 * @param[in] segMan		The Segment Manager
 * @param[in] obj			Address of the object to look the selector up in
 * @param[in] selectorid	The selector to look up
 * @param[out] varp			A reference to the selector, if it is a
 * 							variable.
 * @param[out] fptr			A reference to the function described by that
 * 							selector, if it is a valid function selector.
 * 							fptr is written to iff it is non-NULL and the
 * 							selector indicates a member function of that
 * 							object.
 * @return					kSelectorNone if the selector was not found in
 * 							the object or its superclasses.
 * 							kSelectorVariable if the selector represents an
 * 							object-relative variable.
 * 							kSelectorMethod if the selector represents a
 * 							method
 */
SelectorType lookupSelector(SegManager *segMan, reg_t obj, Selector selectorid,
		ObjVarRef *varp, reg_t *fptr);

/**
 * Read a PMachine instruction from a memory buffer and return its length.
 *
 * @param[in] src		address from which to start parsing
 * @param[out] extOpcode	"extended" opcode of the parsed instruction
 * @param[out] opparams	parameter for the parsed instruction
 * @return the length in bytes of the instruction
 *
 * @todo How about changing opparams from int16 to int / int32 to preserve
 *       unsigned 16bit words as read for Script_Word? In the past, this
 *       was irrelevant as only a debug opcode used Script_Word. But with
 *       SCI32 we are now using Script_Word for more opcodes. Maybe this is
 *       just a mistake and those opcodes should used Script_SWord -- but if
 *       not then we definitely should change this to int, else we might run
 *       into trouble if we encounter high value words. *If* those exist at all.
 */
int readPMachineInstruction(const byte *src, byte &extOpcode, int16 opparams[4]);

} // End of namespace Sci

#endif // SCI_ENGINE_VM_H
