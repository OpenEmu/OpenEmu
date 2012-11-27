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

 // Console module header file

#ifndef SCI_CONSOLE_H
#define SCI_CONSOLE_H

#include "gui/debugger.h"
#include "sci/engine/vm.h"

namespace Sci {

class SciEngine;
struct List;

reg_t disassemble(EngineState *s, reg32_t pos, bool printBWTag, bool printBytecode);
bool isJumpOpcode(EngineState *s, reg_t pos, reg_t& jumpOffset);

class Console : public GUI::Debugger {
public:
	Console(SciEngine *engine);
	virtual ~Console();

	int printObject(reg_t pos);

private:
	virtual void preEnter();
	virtual void postEnter();

private:
	// General
	bool cmdHelp(int argc, const char **argv);
	// Kernel
//	bool cmdClasses(int argc, const char **argv);	// TODO
	bool cmdOpcodes(int argc, const char **argv);
	bool cmdSelector(int argc, const char **argv);
	bool cmdSelectors(int argc, const char **argv);
	bool cmdKernelFunctions(int argc, const char **argv);
	bool cmdClassTable(int argc, const char **argv);
	// Parser
	bool cmdSuffixes(int argc, const char **argv);
	bool cmdParseGrammar(int argc, const char **argv);
	bool cmdParserNodes(int argc, const char **argv);
	bool cmdParserWords(int argc, const char **argv);
	bool cmdSentenceFragments(int argc, const char **argv);
	bool cmdParse(int argc, const char **argv);
	bool cmdSetParseNodes(int argc, const char **argv);
	bool cmdSaid(int argc, const char **argv);
	// Resources
	bool cmdDiskDump(int argc, const char **argv);
	bool cmdHexDump(int argc, const char **argv);
	bool cmdResourceId(int argc, const char **argv);
	bool cmdResourceInfo(int argc, const char **argv);
	bool cmdResourceTypes(int argc, const char **argv);
	bool cmdList(int argc, const char **argv);
	bool cmdHexgrep(int argc, const char **argv);
	bool cmdVerifyScripts(int argc, const char **argv);
	// Game
	bool cmdSaveGame(int argc, const char **argv);
	bool cmdRestoreGame(int argc, const char **argv);
	bool cmdRestartGame(int argc, const char **argv);
	bool cmdGetVersion(int argc, const char **argv);
	bool cmdRoomNumber(int argc, const char **argv);
	bool cmdQuit(int argc, const char **argv);
	bool cmdListSaves(int argc, const char **argv);
	// Screen
	bool cmdShowMap(int argc, const char **argv);
	// Graphics
	bool cmdSetPalette(int argc, const char **argv);
	bool cmdDrawPic(int argc, const char **argv);
	bool cmdDrawCel(int argc, const char **argv);
	bool cmdUndither(int argc, const char **argv);
	bool cmdPicVisualize(int argc, const char **argv);
	bool cmdPlayVideo(int argc, const char **argv);
	bool cmdAnimateList(int argc, const char **argv);
	bool cmdWindowList(int argc, const char **argv);
	bool cmdPlaneList(int argc, const char **argv);
	bool cmdPlaneItemList(int argc, const char **argv);
	bool cmdSavedBits(int argc, const char **argv);
	bool cmdShowSavedBits(int argc, const char **argv);
	// Segments
	bool cmdPrintSegmentTable(int argc, const char **argv);
	bool cmdSegmentInfo(int argc, const char **argv);
	bool cmdKillSegment(int argc, const char **argv);
	// Garbage collection
	bool cmdGCInvoke(int argc, const char **argv);
	bool cmdGCObjects(int argc, const char **argv);
	bool cmdGCShowReachable(int argc, const char **argv);
	bool cmdGCShowFreeable(int argc, const char **argv);
	bool cmdGCNormalize(int argc, const char **argv);
	// Music/SFX
	bool cmdSongLib(int argc, const char **argv);
	bool cmdSongInfo(int argc, const char **argv);
	bool cmdIsSample(int argc, const char **argv);
	bool cmdStartSound(int argc, const char **argv);
	bool cmdToggleSound(int argc, const char **argv);
	bool cmdStopAllSounds(int argc, const char **argv);
	bool cmdSfx01Header(int argc, const char **argv);
	bool cmdSfx01Track(int argc, const char **argv);
	bool cmdShowInstruments(int argc, const char **argv);
	bool cmdMapInstrument(int argc, const char **argv);
	// Script
	bool cmdAddresses(int argc, const char **argv);
	bool cmdRegisters(int argc, const char **argv);
	bool cmdDissectScript(int argc, const char **argv);
	bool cmdBacktrace(int argc, const char **argv);
	bool cmdTrace(int argc, const char **argv);
	bool cmdStepOver(int argc, const char **argv);
	bool cmdStepEvent(int argc, const char **argv);
	bool cmdStepRet(int argc, const char **argv);
	bool cmdStepGlobal(int argc, const char **argv);
	bool cmdStepCallk(int argc, const char **argv);
	bool cmdDisassemble(int argc, const char **argv);
	bool cmdDisassembleAddress(int argc, const char **argv);
	bool cmdFindKernelFunctionCall(int argc, const char **argv);
	bool cmdSend(int argc, const char **argv);
	bool cmdGo(int argc, const char **argv);
	bool cmdLogKernel(int argc, const char **argv);
	// Breakpoints
	bool cmdBreakpointList(int argc, const char **argv);
	bool cmdBreakpointDelete(int argc, const char **argv);
	bool cmdBreakpointMethod(int argc, const char **argv);
	bool cmdBreakpointRead(int argc, const char **argv);
	bool cmdBreakpointWrite(int argc, const char **argv);
	bool cmdBreakpointKernel(int argc, const char **argv);
	bool cmdBreakpointFunction(int argc, const char **argv);
	// VM
	bool cmdScriptSteps(int argc, const char **argv);
	bool cmdVMVarlist(int argc, const char **argv);
	bool cmdVMVars(int argc, const char **argv);
	bool cmdStack(int argc, const char **argv);
	bool cmdValueType(int argc, const char **argv);
	bool cmdViewListNode(int argc, const char **argv);
	bool cmdViewReference(int argc, const char **argv);
	bool cmdViewObject(int argc, const char **argv);
	bool cmdViewActiveObject(int argc, const char **argv);
	bool cmdViewAccumulatorObject(int argc, const char **argv);

	bool parseInteger(const char *argument, int &result);

	void printBasicVarInfo(reg_t variable);

	bool segmentInfo(int nr);
	void printList(List *list);
	int printNode(reg_t addr);
	void hexDumpReg(const reg_t *data, int len, int regsPerLine = 4, int startOffset = 0, bool isArray = false);

private:
	/**
	 * Prints all the scripts calling the specified kernel function.
	 * NOTE: The results produced by this aren't 100% correct, as it
	 * does not dissect script exports
	 */
	void printKernelCallsFound(int kernelFuncNum, bool showFoundScripts);

	SciEngine *_engine;
	DebugState &_debugState;
	bool _mouseVisible;
	Common::String _videoFile;
	int _videoFrameDelay;
};

} // End of namespace Sci

#endif
