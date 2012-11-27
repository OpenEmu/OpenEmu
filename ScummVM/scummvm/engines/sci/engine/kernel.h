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

#ifndef SCI_ENGINE_KERNEL_H
#define SCI_ENGINE_KERNEL_H

#include "common/scummsys.h"
#include "common/debug.h"
#include "common/rect.h"
#include "common/str-array.h"

#include "sci/engine/selector.h"
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/engine/vm.h"

namespace Sci {

struct Node;	// from segment.h
struct List;	// from segment.h
struct SelectorCache;	// from selector.h
struct SciWorkaroundEntry;	// from workarounds.h

/**
 * @defgroup VocabularyResources	Vocabulary resources in SCI
 *
 * vocab.999 / 999.voc (unneeded) contains names of the kernel functions which
 * are implemented by the interpreter. In Sierra SCI, they are used exclusively
 * by the debugger, which is why keeping the file up to date was less important.
 * This resource is notoriously unreliable, and should not be used. Fortunately,
 * kernel names are the same in major SCI revisions, which is why we have them
 * hardcoded.
 *
 * vocab.998 / 998.voc (unneeded) contains opcode names. Opcodes have remained
 * the same from SCI0 to SCI2.1, and have changed in SCI3, so this is only used
 * on demand for debugging purposes, for showing the opcode names
 *
 * vocab.997 / 997.voc (usually needed) contains the names of every selector in
 * the class hierarchy. Each method and property '''name''' consumes one id, but
 * if a name is shared between classes, one id will do. Some demos do not contain
 * a selector vocabulary, but the selectors used by the engine have stayed more or
 * less static, so we add the selectors we need inside static_selectors.cpp
 * The SCI engine loads vocab.997 on startup, and fills in an internal structure
 * that allows interpreter code to access these selectors via #defined macros. It
 * does not use the file after this initial stage.
 *
 * vocab.996 / 996.voc (required) contains the classes which are used in each
 * script, and is required by the segment manager
 *
 * vocab.995 / 995.voc (unneeded) contains strings for the embedded SCI debugger
 *
 * vocab.994 / 994.voc (unneeded) contains offsets into certain classes of certain
 * properties. This enables the interpreter to update these properties in O(1) time,
 * which was important in the era when SCI was initially conceived. In SCI, we
 * figured out what '''property''' a certain offset refers to (which requires one to
 * guess what class a pointer points to) and then simply use the property name and
 * vocab.997. This results in much more readable code. Thus, this vocabulary isn't
 * used at all.
 *
 * SCI0 parser vocabularies:
 * - vocab.901 / 901.voc - suffix vocabulary
 * - vocab.900 / 900.voc - parse tree branches
 * - vocab.0 / 0.voc - main vocabulary, containing words and their attributes
 *                     (e.g. "onto" - "position")
 *
 * SCI01 parser vocabularies:
 * - vocab.902 / 902.voc - suffix vocabulary
 * - vocab.901 / 901.voc - parse tree branches
 * - vocab.900 / 900.voc - main vocabulary, containing words and their attributes
 *                        (e.g. "onto" - "position")
 *
 */
//@{

//#define DEBUG_PARSER	// enable for parser debugging

// ---- Kernel signatures -----------------------------------------------------

// internal kernel signature data
enum {
	SIG_TYPE_NULL          =  0x01, // may be 0:0       [0]
	SIG_TYPE_INTEGER       =  0x02, // may be 0:*       [i], automatically also allows null
	SIG_TYPE_UNINITIALIZED =  0x04, // may be FFFF:*    -> not allowable, only used for comparison
	SIG_TYPE_OBJECT        =  0x08, // may be object    [o]
	SIG_TYPE_REFERENCE     =  0x10, // may be reference [r]
	SIG_TYPE_LIST          =  0x20, // may be list      [l]
	SIG_TYPE_NODE          =  0x40, // may be node      [n]
	SIG_TYPE_ERROR         =  0x80, // happens, when there is a identification error - only used for comparison
	SIG_IS_INVALID         = 0x100, // ptr is invalid   [!] -> invalid offset
	SIG_IS_OPTIONAL        = 0x200, // is optional
	SIG_NEEDS_MORE         = 0x400, // needs at least one additional parameter following
	SIG_MORE_MAY_FOLLOW    = 0x800  // may have more parameters of the same type following
};

// this does not include SIG_TYPE_UNINITIALIZED, because we can not allow uninitialized values anywhere
#define SIG_MAYBE_ANY    (SIG_TYPE_NULL | SIG_TYPE_INTEGER | SIG_TYPE_OBJECT | SIG_TYPE_REFERENCE | SIG_TYPE_LIST | SIG_TYPE_NODE)

// ----------------------------------------------------------------------------

/* Generic description: */
typedef reg_t KernelFunctionCall(EngineState *s, int argc, reg_t *argv);

struct KernelSubFunction {
	KernelFunctionCall *function;
	const char *name;
	uint16 *signature;
	const SciWorkaroundEntry *workarounds;
	bool debugLogging;
	bool debugBreakpoint;
};

struct KernelFunction {
	KernelFunctionCall *function;
	const char *name;
	uint16 *signature;
	const SciWorkaroundEntry *workarounds;
	KernelSubFunction *subFunctions;
	uint16 subFunctionCount;
	bool debugLogging;
	bool debugBreakpoint;
};

class Kernel {
public:
	/**
	 * Initializes the SCI kernel.
	 */
	Kernel(ResourceManager *resMan, SegManager *segMan);
	~Kernel();

	uint getSelectorNamesSize() const;
	const Common::String &getSelectorName(uint selector);
	int findKernelFuncPos(Common::String kernelFuncName);

	uint getKernelNamesSize() const;
	const Common::String &getKernelName(uint number) const;

	/**
	 * Determines the selector ID of a selector by its name.
	 * @param selectorName Name of the selector to look up
	 * @return The appropriate selector ID, or -1 on error
	 */
	int findSelector(const char *selectorName) const;

	// Script dissection/dumping functions
	void dissectScript(int scriptNumber, Vocabulary *vocab);
	void dumpScriptObject(char *data, int seeker, int objsize);
	void dumpScriptClass(char *data, int seeker, int objsize);

	SelectorCache _selectorCache; /**< Shortcut list for important selectors. */
	typedef Common::Array<KernelFunction> KernelFunctionArray;
	KernelFunctionArray _kernelFuncs; /**< Table of kernel functions. */

	/**
	 * Determines whether a list of registers matches a given signature.
	 * If no signature is given (i.e., if sig is NULL), this is always
	 * treated as a match.
	 *
	 * @param sig	 signature to test against
	 * @param argc	 number of arguments to test
	 * @param argv	 argument list
	 * @return true if the signature was matched, false otherwise
	 */
	bool signatureMatch(const uint16 *sig, int argc, const reg_t *argv);

	// Prints out debug information in case a signature check fails
	void signatureDebug(const uint16 *sig, int argc, const reg_t *argv);

	/**
	 * Determines the type of the object indicated by reg.
	 * @param reg				register to check
	 * @return one of KSIG_* below KSIG_NULL.
	 *	       KSIG_INVALID set if the type of reg can be determined, but is invalid.
	 *	       0 on error.
	 */
	uint16 findRegType(reg_t reg);

	/******************** Text functionality ********************/
	/**
	 * Looks up text referenced by scripts.
	 * SCI uses two values to reference to text: An address, and an index. The
	 * address determines whether the text should be read from a resource file,
	 * or from the heap, while the index either refers to the number of the
	 * string in the specified source, or to a relative position inside the text.
	 *
	 * @param address The address to look up
	 * @param index The relative index
	 * @return The referenced text, or empty string on error.
	 */
	Common::String lookupText(reg_t address, int index);

	/**
	 * Loads the kernel function names.
	 *
	 * This function reads the kernel function name table from resource_map,
	 * and fills the _kernelNames array with them.
	 * The resulting list has the same format regardless of the format of the
	 * name table of the resource (the format changed between version 0 and 1).
	 */
	void loadKernelNames(GameFeatures *features);

	/**
	 * Sets debug flags for a kernel function
	 */
	bool debugSetFunction(const char *kernelName, int logging, int breakpoint);

private:
	/**
	 * Loads the kernel selector names.
	 */
	void loadSelectorNames();

	/**
	 * Check for any hardcoded selector table we might have that can be used
	 * if a game is missing the selector names.
	 */
	Common::StringArray checkStaticSelectorNames();

	/**
	 * Automatically find specific selectors
	 */
	void findSpecificSelectors(Common::StringArray &selectorNames);

	/**
	 * Maps special selectors.
	 */
	void mapSelectors();

	/**
	 * Maps kernel functions.
	 */
	void mapFunctions();

	ResourceManager *_resMan;
	SegManager *_segMan;

	// Kernel-related lists
	Common::StringArray _selectorNames;
	Common::StringArray _kernelNames;

	const Common::String _invalid;
};

/******************** Kernel functions ********************/

reg_t kStrLen(EngineState *s, int argc, reg_t *argv);
reg_t kGetFarText(EngineState *s, int argc, reg_t *argv);
reg_t kReadNumber(EngineState *s, int argc, reg_t *argv);
reg_t kStrCat(EngineState *s, int argc, reg_t *argv);
reg_t kStrCmp(EngineState *s, int argc, reg_t *argv);
reg_t kSetSynonyms(EngineState *s, int argc, reg_t *argv);
reg_t kLock(EngineState *s, int argc, reg_t *argv);
reg_t kPalette(EngineState *s, int argc, reg_t *argv);
reg_t kPalVary(EngineState *s, int argc, reg_t *argv);
reg_t kAssertPalette(EngineState *s, int argc, reg_t *argv);
reg_t kPortrait(EngineState *s, int argc, reg_t *argv);
reg_t kNumCels(EngineState *s, int argc, reg_t *argv);
reg_t kNumLoops(EngineState *s, int argc, reg_t *argv);
reg_t kDrawCel(EngineState *s, int argc, reg_t *argv);
reg_t kCoordPri(EngineState *s, int argc, reg_t *argv);
reg_t kPriCoord(EngineState *s, int argc, reg_t *argv);
reg_t kShakeScreen(EngineState *s, int argc, reg_t *argv);
reg_t kSetCursor(EngineState *s, int argc, reg_t *argv);
reg_t kMoveCursor(EngineState *s, int argc, reg_t *argv);
reg_t kPicNotValid(EngineState *s, int argc, reg_t *argv);
reg_t kOnControl(EngineState *s, int argc, reg_t *argv);
reg_t kDrawPic(EngineState *s, int argc, reg_t *argv);
reg_t kGetPort(EngineState *s, int argc, reg_t *argv);
reg_t kSetPort(EngineState *s, int argc, reg_t *argv);
reg_t kNewWindow(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeWindow(EngineState *s, int argc, reg_t *argv);
reg_t kCelWide(EngineState *s, int argc, reg_t *argv);
reg_t kCelHigh(EngineState *s, int argc, reg_t *argv);
reg_t kSetJump(EngineState *s, int argc, reg_t *argv);
reg_t kDirLoop(EngineState *s, int argc, reg_t *argv);
reg_t kDoAvoider(EngineState *s, int argc, reg_t *argv);
reg_t kGetAngle(EngineState *s, int argc, reg_t *argv);
reg_t kGetDistance(EngineState *s, int argc, reg_t *argv);
reg_t kRandom(EngineState *s, int argc, reg_t *argv);
reg_t kAbs(EngineState *s, int argc, reg_t *argv);
reg_t kSqrt(EngineState *s, int argc, reg_t *argv);
reg_t kTimesSin(EngineState *s, int argc, reg_t *argv);
reg_t kTimesCos(EngineState *s, int argc, reg_t *argv);
reg_t kCosMult(EngineState *s, int argc, reg_t *argv);
reg_t kSinMult(EngineState *s, int argc, reg_t *argv);
reg_t kTimesTan(EngineState *s, int argc, reg_t *argv);
reg_t kTimesCot(EngineState *s, int argc, reg_t *argv);
reg_t kCosDiv(EngineState *s, int argc, reg_t *argv);
reg_t kSinDiv(EngineState *s, int argc, reg_t *argv);
reg_t kValidPath(EngineState *s, int argc, reg_t *argv);
reg_t kMapKeyToDir(EngineState *s, int argc, reg_t *argv);
reg_t kGlobalToLocal(EngineState *s, int argc, reg_t *argv);
reg_t kLocalToGlobal(EngineState *s, int argc, reg_t *argv);
reg_t kWait(EngineState *s, int argc, reg_t *argv);
reg_t kRestartGame(EngineState *s, int argc, reg_t *argv);
reg_t kDeviceInfo(EngineState *s, int argc, reg_t *argv);
reg_t kGetEvent(EngineState *s, int argc, reg_t *argv);
reg_t kCheckFreeSpace(EngineState *s, int argc, reg_t *argv);
reg_t kFlushResources(EngineState *s, int argc, reg_t *argv);
reg_t kGetSaveFiles(EngineState *s, int argc, reg_t *argv);
reg_t kSetDebug(EngineState *s, int argc, reg_t *argv);
reg_t kCheckSaveGame(EngineState *s, int argc, reg_t *argv);
reg_t kSaveGame(EngineState *s, int argc, reg_t *argv);
reg_t kRestoreGame(EngineState *s, int argc, reg_t *argv);
reg_t kFileIO(EngineState *s, int argc, reg_t *argv);
reg_t kGetTime(EngineState *s, int argc, reg_t *argv);
reg_t kHaveMouse(EngineState *s, int argc, reg_t *argv);
reg_t kJoystick(EngineState *s, int argc, reg_t *argv);
reg_t kGameIsRestarting(EngineState *s, int argc, reg_t *argv);
reg_t kGetCWD(EngineState *s, int argc, reg_t *argv);
reg_t kSort(EngineState *s, int argc, reg_t *argv);
reg_t kStrEnd(EngineState *s, int argc, reg_t *argv);
reg_t kMemory(EngineState *s, int argc, reg_t *argv);
reg_t kAvoidPath(EngineState *s, int argc, reg_t *argv);
reg_t kParse(EngineState *s, int argc, reg_t *argv);
reg_t kSaid(EngineState *s, int argc, reg_t *argv);
reg_t kStrCpy(EngineState *s, int argc, reg_t *argv);
reg_t kStrAt(EngineState *s, int argc, reg_t *argv);
reg_t kEditControl(EngineState *s, int argc, reg_t *argv);
reg_t kDrawControl(EngineState *s, int argc, reg_t *argv);
reg_t kHiliteControl(EngineState *s, int argc, reg_t *argv);
reg_t kClone(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeClone(EngineState *s, int argc, reg_t *argv);
reg_t kCanBeHere(EngineState *s, int argc, reg_t *argv);
reg_t kCantBeHere(EngineState *s, int argc, reg_t *argv);
reg_t kSetNowSeen(EngineState *s, int argc, reg_t *argv);
reg_t kInitBresen(EngineState *s, int argc, reg_t *argv);
reg_t kDoBresen(EngineState *s, int argc, reg_t *argv);
reg_t kBaseSetter(EngineState *s, int argc, reg_t *argv);
reg_t kAddToPic(EngineState *s, int argc, reg_t *argv);
reg_t kAnimate(EngineState *s, int argc, reg_t *argv);
reg_t kDisplay(EngineState *s, int argc, reg_t *argv);
reg_t kGraph(EngineState *s, int argc, reg_t *argv);
reg_t kFormat(EngineState *s, int argc, reg_t *argv);
reg_t kDoSound(EngineState *s, int argc, reg_t *argv);
reg_t kAddMenu(EngineState *s, int argc, reg_t *argv);
reg_t kSetMenu(EngineState *s, int argc, reg_t *argv);
reg_t kGetMenu(EngineState *s, int argc, reg_t *argv);
reg_t kDrawStatus(EngineState *s, int argc, reg_t *argv);
reg_t kDrawMenuBar(EngineState *s, int argc, reg_t *argv);
reg_t kMenuSelect(EngineState *s, int argc, reg_t *argv);

reg_t kLoad(EngineState *s, int argc, reg_t *argv);
reg_t kUnLoad(EngineState *s, int argc, reg_t *argv);
reg_t kScriptID(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeScript(EngineState *s, int argc, reg_t *argv);
reg_t kIsObject(EngineState *s, int argc, reg_t *argv);
reg_t kRespondsTo(EngineState *s, int argc, reg_t *argv);
reg_t kNewList(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeList(EngineState *s, int argc, reg_t *argv);
reg_t kNewNode(EngineState *s, int argc, reg_t *argv);
reg_t kFirstNode(EngineState *s, int argc, reg_t *argv);
reg_t kLastNode(EngineState *s, int argc, reg_t *argv);
reg_t kEmptyList(EngineState *s, int argc, reg_t *argv);
reg_t kNextNode(EngineState *s, int argc, reg_t *argv);
reg_t kPrevNode(EngineState *s, int argc, reg_t *argv);
reg_t kNodeValue(EngineState *s, int argc, reg_t *argv);
reg_t kAddAfter(EngineState *s, int argc, reg_t *argv);
reg_t kAddToFront(EngineState *s, int argc, reg_t *argv);
reg_t kAddToEnd(EngineState *s, int argc, reg_t *argv);
reg_t kFindKey(EngineState *s, int argc, reg_t *argv);
reg_t kDeleteKey(EngineState *s, int argc, reg_t *argv);
reg_t kMemoryInfo(EngineState *s, int argc, reg_t *argv);
reg_t kGetSaveDir(EngineState *s, int argc, reg_t *argv);
reg_t kTextSize(EngineState *s, int argc, reg_t *argv);
reg_t kIsItSkip(EngineState *s, int argc, reg_t *argv);
reg_t kGetMessage(EngineState *s, int argc, reg_t *argv);
reg_t kMessage(EngineState *s, int argc, reg_t *argv);
reg_t kDoAudio(EngineState *s, int argc, reg_t *argv);
reg_t kDoSync(EngineState *s, int argc, reg_t *argv);
reg_t kMemorySegment(EngineState *s, int argc, reg_t *argv);
reg_t kIntersections(EngineState *s, int argc, reg_t *argv);
reg_t kMergePoly(EngineState *s, int argc, reg_t *argv);
reg_t kResCheck(EngineState *s, int argc, reg_t *argv);
reg_t kSetQuitStr(EngineState *s, int argc, reg_t *argv);
reg_t kShowMovie(EngineState *s, int argc, reg_t *argv);
reg_t kSetVideoMode(EngineState *s, int argc, reg_t *argv);
reg_t kStrSplit(EngineState *s, int argc, reg_t *argv);
reg_t kPlatform(EngineState *s, int argc, reg_t *argv);
reg_t kTextColors(EngineState *s, int argc, reg_t *argv);
reg_t kTextFonts(EngineState *s, int argc, reg_t *argv);
reg_t kShow(EngineState *s, int argc, reg_t *argv);
reg_t kRemapColors(EngineState *s, int argc, reg_t *argv);
reg_t kDummy(EngineState *s, int argc, reg_t *argv);
reg_t kEmpty(EngineState *s, int argc, reg_t *argv);
reg_t kStub(EngineState *s, int argc, reg_t *argv);
reg_t kStubNull(EngineState *s, int argc, reg_t *argv);

#ifdef ENABLE_SCI32
// SCI2 Kernel Functions
reg_t kIsHiRes(EngineState *s, int argc, reg_t *argv);
reg_t kArray(EngineState *s, int argc, reg_t *argv);
reg_t kListAt(EngineState *s, int argc, reg_t *argv);
reg_t kString(EngineState *s, int argc, reg_t *argv);
reg_t kMulDiv(EngineState *s, int argc, reg_t *argv);
reg_t kCantBeHere32(EngineState *s, int argc, reg_t *argv);
reg_t kRemapColors32(EngineState *s, int argc, reg_t *argv);
// "Screen items" in SCI32 are views
reg_t kAddScreenItem(EngineState *s, int argc, reg_t *argv);
reg_t kUpdateScreenItem(EngineState *s, int argc, reg_t *argv);
reg_t kDeleteScreenItem(EngineState *s, int argc, reg_t *argv);
// Text
reg_t kCreateTextBitmap(EngineState *s, int argc, reg_t *argv);
reg_t kDisposeTextBitmap(EngineState *s, int argc, reg_t *argv);
// "Planes" in SCI32 are pictures
reg_t kAddPlane(EngineState *s, int argc, reg_t *argv);
reg_t kDeletePlane(EngineState *s, int argc, reg_t *argv);
reg_t kUpdatePlane(EngineState *s, int argc, reg_t *argv);
reg_t kSetShowStyle(EngineState *s, int argc, reg_t *argv);
reg_t kGetHighPlanePri(EngineState *s, int argc, reg_t *argv);
reg_t kFrameOut(EngineState *s, int argc, reg_t *argv);
reg_t kIsOnMe(EngineState *s, int argc, reg_t *argv); // kOnMe for SCI2, kIsOnMe for SCI2.1
reg_t kListIndexOf(EngineState *s, int argc, reg_t *argv);
reg_t kListEachElementDo(EngineState *s, int argc, reg_t *argv);
reg_t kListFirstTrue(EngineState *s, int argc, reg_t *argv);
reg_t kListAllTrue(EngineState *s, int argc, reg_t *argv);
reg_t kInPolygon(EngineState *s, int argc, reg_t *argv);
reg_t kObjectIntersect(EngineState *s, int argc, reg_t *argv);
reg_t kEditText(EngineState *s, int argc, reg_t *argv);
reg_t kMakeSaveCatName(EngineState *s, int argc, reg_t *argv);
reg_t kMakeSaveFileName(EngineState *s, int argc, reg_t *argv);
reg_t kSetScroll(EngineState *s, int argc, reg_t *argv);
reg_t kPalCycle(EngineState *s, int argc, reg_t *argv);

// SCI2.1 Kernel Functions
reg_t kText(EngineState *s, int argc, reg_t *argv);
reg_t kSave(EngineState *s, int argc, reg_t *argv);
reg_t kAutoSave(EngineState *s, int argc, reg_t *argv);
reg_t kList(EngineState *s, int argc, reg_t *argv);
reg_t kRobot(EngineState *s, int argc, reg_t *argv);
reg_t kPlayVMD(EngineState *s, int argc, reg_t *argv);
reg_t kCD(EngineState *s, int argc, reg_t *argv);
reg_t kAddPicAt(EngineState *s, int argc, reg_t *argv);
reg_t kAddBefore(EngineState *s, int argc, reg_t *argv);
reg_t kMoveToFront(EngineState *s, int argc, reg_t *argv);
reg_t kMoveToEnd(EngineState *s, int argc, reg_t *argv);
reg_t kGetWindowsOption(EngineState *s, int argc, reg_t *argv);
reg_t kWinHelp(EngineState *s, int argc, reg_t *argv);
reg_t kGetConfig(EngineState *s, int argc, reg_t *argv);
reg_t kGetSierraProfileInt(EngineState *s, int argc, reg_t *argv);
reg_t kCelInfo(EngineState *s, int argc, reg_t *argv);
reg_t kSetLanguage(EngineState *s, int argc, reg_t *argv);
reg_t kScrollWindow(EngineState *s, int argc, reg_t *argv);
reg_t kSetFontRes(EngineState *s, int argc, reg_t *argv);
reg_t kFont(EngineState *s, int argc, reg_t *argv);
reg_t kBitmap(EngineState *s, int argc, reg_t *argv);
reg_t kAddLine(EngineState *s, int argc, reg_t *argv);
reg_t kUpdateLine(EngineState *s, int argc, reg_t *argv);
reg_t kDeleteLine(EngineState *s, int argc, reg_t *argv);

// SCI3 Kernel functions
reg_t kPlayDuck(EngineState *s, int argc, reg_t *argv);
#endif

reg_t kDoSoundInit(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundPlay(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundRestore(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundDispose(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundMute(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundStop(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundStopAll(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundPause(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundResumeAfterRestore(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundMasterVolume(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundUpdate(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundFade(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundGetPolyphony(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundUpdateCues(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundSendMidi(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundGlobalReverb(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundSetHold(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundDummy(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundGetAudioCapability(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundSuspend(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundSetVolume(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundSetPriority(EngineState *s, int argc, reg_t *argv);
reg_t kDoSoundSetLoop(EngineState *s, int argc, reg_t *argv);

reg_t kGraphGetColorCount(EngineState *s, int argc, reg_t *argv);
reg_t kGraphDrawLine(EngineState *s, int argc, reg_t *argv);
reg_t kGraphSaveBox(EngineState *s, int argc, reg_t *argv);
reg_t kGraphRestoreBox(EngineState *s, int argc, reg_t *argv);
reg_t kGraphFillBoxBackground(EngineState *s, int argc, reg_t *argv);
reg_t kGraphFillBoxForeground(EngineState *s, int argc, reg_t *argv);
reg_t kGraphFillBoxAny(EngineState *s, int argc, reg_t *argv);
reg_t kGraphUpdateBox(EngineState *s, int argc, reg_t *argv);
reg_t kGraphRedrawBox(EngineState *s, int argc, reg_t *argv);
reg_t kGraphAdjustPriority(EngineState *s, int argc, reg_t *argv);
reg_t kGraphSaveUpscaledHiresBox(EngineState *s, int argc, reg_t *argv);

reg_t kPalVaryInit(EngineState *s, int argc, reg_t *argv);
reg_t kPalVaryReverse(EngineState *s, int argc, reg_t *argv);
reg_t kPalVaryGetCurrentStep(EngineState *s, int argc, reg_t *argv);
reg_t kPalVaryDeinit(EngineState *s, int argc, reg_t *argv);
reg_t kPalVaryChangeTarget(EngineState *s, int argc, reg_t *argv);
reg_t kPalVaryChangeTicks(EngineState *s, int argc, reg_t *argv);
reg_t kPalVaryPauseResume(EngineState *s, int argc, reg_t *argv);
reg_t kPalVaryUnknown(EngineState *s, int argc, reg_t *argv);

reg_t kPaletteSetFromResource(EngineState *s, int argc, reg_t *argv);
reg_t kPaletteSetFlag(EngineState *s, int argc, reg_t *argv);
reg_t kPaletteUnsetFlag(EngineState *s, int argc, reg_t *argv);
reg_t kPaletteSetIntensity(EngineState *s, int argc, reg_t *argv);
reg_t kPaletteFindColor(EngineState *s, int argc, reg_t *argv);
reg_t kPaletteAnimate(EngineState *s, int argc, reg_t *argv);
reg_t kPaletteSave(EngineState *s, int argc, reg_t *argv);
reg_t kPaletteRestore(EngineState *s, int argc, reg_t *argv);

reg_t kFileIOOpen(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOClose(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOReadRaw(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOWriteRaw(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOUnlink(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOReadString(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOWriteString(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOSeek(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOFindFirst(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOFindNext(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOExists(EngineState *s, int argc, reg_t *argv);
reg_t kFileIORename(EngineState *s, int argc, reg_t *argv);
#ifdef ENABLE_SCI32
reg_t kFileIOReadByte(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOWriteByte(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOReadWord(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOWriteWord(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOCreateSaveSlot(EngineState *s, int argc, reg_t *argv);
reg_t kFileIOIsValidDirectory(EngineState *s, int argc, reg_t *argv);
#endif

//@}

} // End of namespace Sci

#endif // SCI_ENGINE_KERNEL_H
