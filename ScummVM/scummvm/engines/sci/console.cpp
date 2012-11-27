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

// Console module

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"
#include "sci/event.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/savegame.h"
#include "sci/engine/gc.h"
#include "sci/engine/features.h"
#include "sci/sound/midiparser_sci.h"
#include "sci/sound/music.h"
#include "sci/sound/drivers/mididriver.h"
#include "sci/sound/drivers/map-mt32-to-gm.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/view.h"

#include "sci/parser/vocabulary.h"

#include "video/avi_decoder.h"
#include "sci/video/seq_decoder.h"
#ifdef ENABLE_SCI32
#include "sci/graphics/frameout.h"
#include "video/coktel_decoder.h"
#include "sci/video/robot_decoder.h"
#endif

#include "common/file.h"
#include "common/savefile.h"

#include "engines/util.h"

namespace Sci {

int g_debug_sleeptime_factor = 1;
int g_debug_simulated_key = 0;
bool g_debug_track_mouse_clicks = false;

// Refer to the "addresses" command on how to pass address parameters
static int parse_reg_t(EngineState *s, const char *str, reg_t *dest, bool mayBeValue);

Console::Console(SciEngine *engine) : GUI::Debugger(),
	_engine(engine), _debugState(engine->_debugState) {

	assert(_engine);
	assert(_engine->_gamestate);

	// Variables
	DVar_Register("sleeptime_factor",	&g_debug_sleeptime_factor, DVAR_INT, 0);
	DVar_Register("gc_interval",		&engine->_gamestate->scriptGCInterval, DVAR_INT, 0);
	DVar_Register("simulated_key",		&g_debug_simulated_key, DVAR_INT, 0);
	DVar_Register("track_mouse_clicks",	&g_debug_track_mouse_clicks, DVAR_BOOL, 0);
	DVar_Register("script_abort_flag",	&_engine->_gamestate->abortScriptProcessing, DVAR_INT, 0);

	// General
	DCmd_Register("help",				WRAP_METHOD(Console, cmdHelp));
	// Kernel
//	DCmd_Register("classes",			WRAP_METHOD(Console, cmdClasses));	// TODO
	DCmd_Register("opcodes",			WRAP_METHOD(Console, cmdOpcodes));
	DCmd_Register("selector",			WRAP_METHOD(Console, cmdSelector));
	DCmd_Register("selectors",			WRAP_METHOD(Console, cmdSelectors));
	DCmd_Register("functions",			WRAP_METHOD(Console, cmdKernelFunctions));
	DCmd_Register("class_table",		WRAP_METHOD(Console, cmdClassTable));
	// Parser
	DCmd_Register("suffixes",			WRAP_METHOD(Console, cmdSuffixes));
	DCmd_Register("parse_grammar",		WRAP_METHOD(Console, cmdParseGrammar));
	DCmd_Register("parser_nodes",		WRAP_METHOD(Console, cmdParserNodes));
	DCmd_Register("parser_words",		WRAP_METHOD(Console, cmdParserWords));
	DCmd_Register("sentence_fragments",	WRAP_METHOD(Console, cmdSentenceFragments));
	DCmd_Register("parse",				WRAP_METHOD(Console, cmdParse));
	DCmd_Register("set_parse_nodes",	WRAP_METHOD(Console, cmdSetParseNodes));
	DCmd_Register("said",				WRAP_METHOD(Console, cmdSaid));
	// Resources
	DCmd_Register("diskdump",			WRAP_METHOD(Console, cmdDiskDump));
	DCmd_Register("hexdump",			WRAP_METHOD(Console, cmdHexDump));
	DCmd_Register("resource_id",		WRAP_METHOD(Console, cmdResourceId));
	DCmd_Register("resource_info",		WRAP_METHOD(Console, cmdResourceInfo));
	DCmd_Register("resource_types",		WRAP_METHOD(Console, cmdResourceTypes));
	DCmd_Register("list",				WRAP_METHOD(Console, cmdList));
	DCmd_Register("hexgrep",			WRAP_METHOD(Console, cmdHexgrep));
	DCmd_Register("verify_scripts",		WRAP_METHOD(Console, cmdVerifyScripts));
	// Game
	DCmd_Register("save_game",			WRAP_METHOD(Console, cmdSaveGame));
	DCmd_Register("restore_game",		WRAP_METHOD(Console, cmdRestoreGame));
	DCmd_Register("restart_game",		WRAP_METHOD(Console, cmdRestartGame));
	DCmd_Register("version",			WRAP_METHOD(Console, cmdGetVersion));
	DCmd_Register("room",				WRAP_METHOD(Console, cmdRoomNumber));
	DCmd_Register("quit",				WRAP_METHOD(Console, cmdQuit));
	DCmd_Register("list_saves",			WRAP_METHOD(Console, cmdListSaves));
	// Graphics
	DCmd_Register("show_map",			WRAP_METHOD(Console, cmdShowMap));
	DCmd_Register("set_palette",		WRAP_METHOD(Console, cmdSetPalette));
	DCmd_Register("draw_pic",			WRAP_METHOD(Console, cmdDrawPic));
	DCmd_Register("draw_cel",			WRAP_METHOD(Console, cmdDrawCel));
	DCmd_Register("undither",           WRAP_METHOD(Console, cmdUndither));
	DCmd_Register("pic_visualize",		WRAP_METHOD(Console, cmdPicVisualize));
	DCmd_Register("play_video",         WRAP_METHOD(Console, cmdPlayVideo));
	DCmd_Register("animate_list",       WRAP_METHOD(Console, cmdAnimateList));
	DCmd_Register("al",                 WRAP_METHOD(Console, cmdAnimateList));	// alias
	DCmd_Register("window_list",        WRAP_METHOD(Console, cmdWindowList));
	DCmd_Register("wl",                 WRAP_METHOD(Console, cmdWindowList));	// alias
	DCmd_Register("plane_list",         WRAP_METHOD(Console, cmdPlaneList));
	DCmd_Register("pl",                 WRAP_METHOD(Console, cmdPlaneList));	// alias
	DCmd_Register("plane_items",        WRAP_METHOD(Console, cmdPlaneItemList));
	DCmd_Register("pi",                 WRAP_METHOD(Console, cmdPlaneItemList));	// alias
	DCmd_Register("saved_bits",         WRAP_METHOD(Console, cmdSavedBits));
	DCmd_Register("show_saved_bits",    WRAP_METHOD(Console, cmdShowSavedBits));
	// Segments
	DCmd_Register("segment_table",		WRAP_METHOD(Console, cmdPrintSegmentTable));
	DCmd_Register("segtable",			WRAP_METHOD(Console, cmdPrintSegmentTable));	// alias
	DCmd_Register("segment_info",		WRAP_METHOD(Console, cmdSegmentInfo));
	DCmd_Register("seginfo",			WRAP_METHOD(Console, cmdSegmentInfo));			// alias
	DCmd_Register("segment_kill",		WRAP_METHOD(Console, cmdKillSegment));
	DCmd_Register("segkill",			WRAP_METHOD(Console, cmdKillSegment));			// alias
	// Garbage collection
	DCmd_Register("gc",					WRAP_METHOD(Console, cmdGCInvoke));
	DCmd_Register("gc_objects",			WRAP_METHOD(Console, cmdGCObjects));
	DCmd_Register("gc_reachable",		WRAP_METHOD(Console, cmdGCShowReachable));
	DCmd_Register("gc_freeable",		WRAP_METHOD(Console, cmdGCShowFreeable));
	DCmd_Register("gc_normalize",		WRAP_METHOD(Console, cmdGCNormalize));
	// Music/SFX
	DCmd_Register("songlib",			WRAP_METHOD(Console, cmdSongLib));
	DCmd_Register("songinfo",			WRAP_METHOD(Console, cmdSongInfo));
	DCmd_Register("is_sample",			WRAP_METHOD(Console, cmdIsSample));
	DCmd_Register("startsound",			WRAP_METHOD(Console, cmdStartSound));
	DCmd_Register("togglesound",		WRAP_METHOD(Console, cmdToggleSound));
	DCmd_Register("stopallsounds",		WRAP_METHOD(Console, cmdStopAllSounds));
	DCmd_Register("sfx01_header",		WRAP_METHOD(Console, cmdSfx01Header));
	DCmd_Register("sfx01_track",		WRAP_METHOD(Console, cmdSfx01Track));
	DCmd_Register("show_instruments",	WRAP_METHOD(Console, cmdShowInstruments));
	DCmd_Register("map_instrument",		WRAP_METHOD(Console, cmdMapInstrument));
	// Script
	DCmd_Register("addresses",			WRAP_METHOD(Console, cmdAddresses));
	DCmd_Register("registers",			WRAP_METHOD(Console, cmdRegisters));
	DCmd_Register("dissect_script",		WRAP_METHOD(Console, cmdDissectScript));
	DCmd_Register("backtrace",			WRAP_METHOD(Console, cmdBacktrace));
	DCmd_Register("bt",					WRAP_METHOD(Console, cmdBacktrace));	// alias
	DCmd_Register("trace",				WRAP_METHOD(Console, cmdTrace));
	DCmd_Register("t",					WRAP_METHOD(Console, cmdTrace));		// alias
	DCmd_Register("s",					WRAP_METHOD(Console, cmdTrace));		// alias
	DCmd_Register("stepover",			WRAP_METHOD(Console, cmdStepOver));
	DCmd_Register("p",					WRAP_METHOD(Console, cmdStepOver));		// alias
	DCmd_Register("step_ret",			WRAP_METHOD(Console, cmdStepRet));
	DCmd_Register("pret",				WRAP_METHOD(Console, cmdStepRet));		// alias
	DCmd_Register("step_event",			WRAP_METHOD(Console, cmdStepEvent));
	DCmd_Register("se",					WRAP_METHOD(Console, cmdStepEvent));	// alias
	DCmd_Register("step_global",		WRAP_METHOD(Console, cmdStepGlobal));
	DCmd_Register("sg",					WRAP_METHOD(Console, cmdStepGlobal));	// alias
	DCmd_Register("step_callk",			WRAP_METHOD(Console, cmdStepCallk));
	DCmd_Register("snk",				WRAP_METHOD(Console, cmdStepCallk));	// alias
	DCmd_Register("disasm",				WRAP_METHOD(Console, cmdDisassemble));
	DCmd_Register("disasm_addr",		WRAP_METHOD(Console, cmdDisassembleAddress));
	DCmd_Register("find_callk",			WRAP_METHOD(Console, cmdFindKernelFunctionCall));
	DCmd_Register("send",				WRAP_METHOD(Console, cmdSend));
	DCmd_Register("go",					WRAP_METHOD(Console, cmdGo));
	DCmd_Register("logkernel",          WRAP_METHOD(Console, cmdLogKernel));
	// Breakpoints
	DCmd_Register("bp_list",			WRAP_METHOD(Console, cmdBreakpointList));
	DCmd_Register("bplist",				WRAP_METHOD(Console, cmdBreakpointList));			// alias
	DCmd_Register("bl",					WRAP_METHOD(Console, cmdBreakpointList));			// alias
	DCmd_Register("bp_del",				WRAP_METHOD(Console, cmdBreakpointDelete));
	DCmd_Register("bpdel",				WRAP_METHOD(Console, cmdBreakpointDelete));			// alias
	DCmd_Register("bc",					WRAP_METHOD(Console, cmdBreakpointDelete));			// alias
	DCmd_Register("bp_method",			WRAP_METHOD(Console, cmdBreakpointMethod));
	DCmd_Register("bpx",				WRAP_METHOD(Console, cmdBreakpointMethod));			// alias
	DCmd_Register("bp_read",			WRAP_METHOD(Console, cmdBreakpointRead));
	DCmd_Register("bpr",				WRAP_METHOD(Console, cmdBreakpointRead));			// alias
	DCmd_Register("bp_write",			WRAP_METHOD(Console, cmdBreakpointWrite));
	DCmd_Register("bpw",				WRAP_METHOD(Console, cmdBreakpointWrite));			// alias
	DCmd_Register("bp_kernel",			WRAP_METHOD(Console, cmdBreakpointKernel));
	DCmd_Register("bpk",				WRAP_METHOD(Console, cmdBreakpointKernel));			// alias
	DCmd_Register("bp_function",		WRAP_METHOD(Console, cmdBreakpointFunction));
	DCmd_Register("bpe",				WRAP_METHOD(Console, cmdBreakpointFunction));		// alias
	// VM
	DCmd_Register("script_steps",		WRAP_METHOD(Console, cmdScriptSteps));
	DCmd_Register("vm_varlist",			WRAP_METHOD(Console, cmdVMVarlist));
	DCmd_Register("vmvarlist",			WRAP_METHOD(Console, cmdVMVarlist));				// alias
	DCmd_Register("vl",					WRAP_METHOD(Console, cmdVMVarlist));				// alias
	DCmd_Register("vm_vars",			WRAP_METHOD(Console, cmdVMVars));
	DCmd_Register("vmvars",				WRAP_METHOD(Console, cmdVMVars));					// alias
	DCmd_Register("vv",					WRAP_METHOD(Console, cmdVMVars));					// alias
	DCmd_Register("stack",				WRAP_METHOD(Console, cmdStack));
	DCmd_Register("value_type",			WRAP_METHOD(Console, cmdValueType));
	DCmd_Register("view_listnode",		WRAP_METHOD(Console, cmdViewListNode));
	DCmd_Register("view_reference",		WRAP_METHOD(Console, cmdViewReference));
	DCmd_Register("vr",					WRAP_METHOD(Console, cmdViewReference));			// alias
	DCmd_Register("view_object",		WRAP_METHOD(Console, cmdViewObject));
	DCmd_Register("vo",					WRAP_METHOD(Console, cmdViewObject));				// alias
	DCmd_Register("active_object",		WRAP_METHOD(Console, cmdViewActiveObject));
	DCmd_Register("acc_object",			WRAP_METHOD(Console, cmdViewAccumulatorObject));

	_debugState.seeking = kDebugSeekNothing;
	_debugState.seekLevel = 0;
	_debugState.runningStep = 0;
	_debugState.stopOnEvent = false;
	_debugState.debugging = false;
	_debugState.breakpointWasHit = false;
	_debugState._breakpoints.clear(); // No breakpoints defined
	_debugState._activeBreakpointTypes = 0;
}

Console::~Console() {
}

void Console::preEnter() {
	_engine->pauseEngine(true);
}

extern void playVideo(Video::VideoDecoder *videoDecoder, VideoState videoState);

void Console::postEnter() {
	if (!_videoFile.empty()) {
		Video::VideoDecoder *videoDecoder = 0;

#ifdef ENABLE_SCI32
		bool duckMode = false;
#endif

		if (_videoFile.hasSuffix(".seq")) {
			videoDecoder = new SEQDecoder(_videoFrameDelay);
#ifdef ENABLE_SCI32
		} else if (_videoFile.hasSuffix(".vmd")) {
			videoDecoder = new Video::AdvancedVMDDecoder();
		} else if (_videoFile.hasSuffix(".rbt")) {
			videoDecoder = new RobotDecoder(_engine->getPlatform() == Common::kPlatformMacintosh);
		} else if (_videoFile.hasSuffix(".duk")) {
			duckMode = true;
			videoDecoder = new Video::AVIDecoder();
#endif
		} else if (_videoFile.hasSuffix(".avi")) {
			videoDecoder = new Video::AVIDecoder();
		} else {
			warning("Unrecognized video type");
		}

		if (videoDecoder && videoDecoder->loadFile(_videoFile)) {
			_engine->_gfxCursor->kernelHide();

#ifdef ENABLE_SCI32
			// Duck videos are 16bpp, so we need to change pixel formats
			int oldWidth = g_system->getWidth();
			int oldHeight = g_system->getHeight();
			if (duckMode) {
				Common::List<Graphics::PixelFormat> formats;
				formats.push_back(videoDecoder->getPixelFormat());
				initGraphics(640, 480, true, formats);

				if (g_system->getScreenFormat().bytesPerPixel != videoDecoder->getPixelFormat().bytesPerPixel)
					error("Could not switch screen format for the duck video");
			}
#endif

			VideoState emptyState;
			emptyState.fileName = _videoFile;
			emptyState.flags = kDoubled;	// always allow the videos to be double sized
			playVideo(videoDecoder, emptyState);

#ifdef ENABLE_SCI32
			// Switch back to 8bpp if we played a duck video
			if (duckMode)
				initGraphics(oldWidth, oldHeight, oldWidth > 320);
#endif

			_engine->_gfxCursor->kernelShow();
		} else
			warning("Could not play video %s\n", _videoFile.c_str());

		_videoFile.clear();
		_videoFrameDelay = 0;
	}

	_engine->pauseEngine(false);
}

bool Console::cmdHelp(int argc, const char **argv) {
	DebugPrintf("\n");
	DebugPrintf("Variables\n");
	DebugPrintf("---------\n");
	DebugPrintf("sleeptime_factor: Factor to multiply with wait times in kWait()\n");
	DebugPrintf("gc_interval: Number of kernel calls in between garbage collections\n");
	DebugPrintf("simulated_key: Add a key with the specified scan code to the event list\n");
	DebugPrintf("track_mouse_clicks: Toggles mouse click tracking to the console\n");
	DebugPrintf("weak_validations: Turns some validation errors into warnings\n");
	DebugPrintf("script_abort_flag: Set to 1 to abort script execution. Set to 2 to force a replay afterwards\n");
	DebugPrintf("\n");
	DebugPrintf("Debug flags\n");
	DebugPrintf("-----------\n");
	DebugPrintf("debugflag_list - Lists the available debug flags and their status\n");
	DebugPrintf("debugflag_enable - Enables a debug flag\n");
	DebugPrintf("debugflag_disable - Disables a debug flag\n");
	DebugPrintf("\n");
	DebugPrintf("Commands\n");
	DebugPrintf("--------\n");
	DebugPrintf("Kernel:\n");
	DebugPrintf(" opcodes - Lists the opcode names\n");
	DebugPrintf(" selectors - Lists the selector names\n");
	DebugPrintf(" selector - Attempts to find the requested selector by name\n");
	DebugPrintf(" functions - Lists the kernel functions\n");
	DebugPrintf(" class_table - Shows the available classes\n");
	DebugPrintf("\n");
	DebugPrintf("Parser:\n");
	DebugPrintf(" suffixes - Lists the vocabulary suffixes\n");
	DebugPrintf(" parse_grammar - Shows the parse grammar, in strict GNF\n");
	DebugPrintf(" parser_nodes - Shows the specified number of nodes from the parse node tree\n");
	DebugPrintf(" parser_words - Shows the words from the parse node tree\n");
	DebugPrintf(" sentence_fragments - Shows the sentence fragments (used to build Parse trees)\n");
	DebugPrintf(" parse - Parses a sequence of words and prints the resulting parse tree\n");
	DebugPrintf(" set_parse_nodes - Sets the contents of all parse nodes\n");
	DebugPrintf(" said - Match a string against a said spec\n");
	DebugPrintf("\n");
	DebugPrintf("Resources:\n");
	DebugPrintf(" diskdump - Dumps the specified resource to disk as a patch file\n");
	DebugPrintf(" hexdump - Dumps the specified resource to standard output\n");
	DebugPrintf(" resource_id - Identifies a resource number by splitting it up in resource type and resource number\n");
	DebugPrintf(" resource_info - Shows info about a resource\n");
	DebugPrintf(" resource_types - Shows the valid resource types\n");
	DebugPrintf(" list - Lists all the resources of a given type\n");
	DebugPrintf(" hexgrep - Searches some resources for a particular sequence of bytes, represented as hexadecimal numbers\n");
	DebugPrintf(" verify_scripts - Performs sanity checks on SCI1.1-SCI2.1 game scripts (e.g. if they're up to 64KB in total)\n");
	DebugPrintf("\n");
	DebugPrintf("Game:\n");
	DebugPrintf(" save_game - Saves the current game state to the hard disk\n");
	DebugPrintf(" restore_game - Restores a saved game from the hard disk\n");
	DebugPrintf(" list_saves - List all saved games including filenames\n");
	DebugPrintf(" restart_game - Restarts the game\n");
	DebugPrintf(" version - Shows the resource and interpreter versions\n");
	DebugPrintf(" room - Gets or sets the current room number\n");
	DebugPrintf(" quit - Quits the game\n");
	DebugPrintf("\n");
	DebugPrintf("Graphics:\n");
	DebugPrintf(" show_map - Switches to visual, priority, control or display screen\n");
	DebugPrintf(" set_palette - Sets a palette resource\n");
	DebugPrintf(" draw_pic - Draws a pic resource\n");
	DebugPrintf(" draw_cel - Draws a cel from a view resource\n");
	DebugPrintf(" pic_visualize - Enables visualization of the drawing process of EGA pictures\n");
	DebugPrintf(" undither - Enable/disable undithering\n");
	DebugPrintf(" play_video - Plays a SEQ, AVI, VMD, RBT or DUK video\n");
	DebugPrintf(" animate_list / al - Shows the current list of objects in kAnimate's draw list (SCI0 - SCI1.1)\n");
	DebugPrintf(" window_list / wl - Shows a list of all the windows (ports) in the draw list (SCI0 - SCI1.1)\n");
	DebugPrintf(" plane_list / pl - Shows a list of all the planes in the draw list (SCI2+)\n");
	DebugPrintf(" plane_items / pi - Shows a list of all items for a plane (SCI2+)\n");
	DebugPrintf(" saved_bits - List saved bits on the hunk\n");
	DebugPrintf(" show_saved_bits - Display saved bits\n");
	DebugPrintf("\n");
	DebugPrintf("Segments:\n");
	DebugPrintf(" segment_table / segtable - Lists all segments\n");
	DebugPrintf(" segment_info / seginfo - Provides information on the specified segment\n");
	DebugPrintf(" segment_kill / segkill - Deletes the specified segment\n");
	DebugPrintf("\n");
	DebugPrintf("Garbage collection:\n");
	DebugPrintf(" gc - Invokes the garbage collector\n");
	DebugPrintf(" gc_objects - Lists all reachable objects, normalized\n");
	DebugPrintf(" gc_reachable - Lists all addresses directly reachable from a given memory object\n");
	DebugPrintf(" gc_freeable - Lists all addresses freeable in a given segment\n");
	DebugPrintf(" gc_normalize - Prints the \"normal\" address of a given address\n");
	DebugPrintf("\n");
	DebugPrintf("Music/SFX:\n");
	DebugPrintf(" songlib - Shows the song library\n");
	DebugPrintf(" songinfo - Shows information about a specified song in the song library\n");
	DebugPrintf(" togglesound - Starts/stops a sound in the song library\n");
	DebugPrintf(" stopallsounds - Stops all sounds in the playlist\n");
	DebugPrintf(" startsound - Starts the specified sound resource, replacing the first song in the song library\n");
	DebugPrintf(" is_sample - Shows information on a given sound resource, if it's a PCM sample\n");
	DebugPrintf(" sfx01_header - Dumps the header of a SCI01 song\n");
	DebugPrintf(" sfx01_track - Dumps a track of a SCI01 song\n");
	DebugPrintf(" show_instruments - Shows the instruments of a specific song, or all songs\n");
	DebugPrintf(" map_instrument - Dynamically maps an MT-32 instrument to a GM instrument\n");
	DebugPrintf("\n");
	DebugPrintf("Script:\n");
	DebugPrintf(" addresses - Provides information on how to pass addresses\n");
	DebugPrintf(" registers - Shows the current register values\n");
	DebugPrintf(" dissect_script - Examines a script\n");
	DebugPrintf(" backtrace / bt - Dumps the send/self/super/call/calle/callb stack\n");
	DebugPrintf(" trace / t / s - Executes one operation (no parameters) or several operations (specified as a parameter) \n");
	DebugPrintf(" stepover / p - Executes one operation, skips over call/send\n");
	DebugPrintf(" step_ret / pret - Steps forward until ret is called on the current execution stack level.\n");
	DebugPrintf(" step_event / se - Steps forward until a SCI event is received.\n");
	DebugPrintf(" step_global / sg - Steps until the global variable with the specified index is modified.\n");
	DebugPrintf(" step_callk / snk - Steps forward until it hits the next callk operation, or a specific callk (specified as a parameter)\n");
	DebugPrintf(" disasm - Disassembles a method by name\n");
	DebugPrintf(" disasm_addr - Disassembles one or more commands\n");
	DebugPrintf(" send - Sends a message to an object\n");
	DebugPrintf(" go - Executes the script\n");
	DebugPrintf(" logkernel - Logs kernel calls\n");
	DebugPrintf("\n");
	DebugPrintf("Breakpoints:\n");
	DebugPrintf(" bp_list / bplist / bl - Lists the current breakpoints\n");
	DebugPrintf(" bp_del / bpdel / bc - Deletes a breakpoint with the specified index\n");
	DebugPrintf(" bp_method / bpx - Sets a breakpoint on the execution of a specified method/selector\n");
	DebugPrintf(" bp_read / bpr - Sets a breakpoint on reading of a specified selector\n");
	DebugPrintf(" bp_write / bpw - Sets a breakpoint on writing to a specified selector\n");
	DebugPrintf(" bp_kernel / bpk - Sets a breakpoint on execution of a kernel function\n");
	DebugPrintf(" bp_function / bpe - Sets a breakpoint on the execution of the specified exported function\n");
	DebugPrintf("\n");
	DebugPrintf("VM:\n");
	DebugPrintf(" script_steps - Shows the number of executed SCI operations\n");
	DebugPrintf(" vm_varlist / vmvarlist / vl - Shows the addresses of variables in the VM\n");
	DebugPrintf(" vm_vars / vmvars / vv - Displays or changes variables in the VM\n");
	DebugPrintf(" stack - Lists the specified number of stack elements\n");
	DebugPrintf(" value_type - Determines the type of a value\n");
	DebugPrintf(" view_listnode - Examines the list node at the given address\n");
	DebugPrintf(" view_reference / vr - Examines an arbitrary reference\n");
	DebugPrintf(" view_object / vo - Examines the object at the given address\n");
	DebugPrintf(" active_object - Shows information on the currently active object or class\n");
	DebugPrintf(" acc_object - Shows information on the object or class at the address indexed by the accumulator\n");
	DebugPrintf("\n");
	return true;
}

ResourceType parseResourceType(const char *resid) {
	// Gets the resource number of a resource string, or returns -1
	ResourceType res = kResourceTypeInvalid;

	for (int i = 0; i < kResourceTypeInvalid; i++)
		if (strcmp(getResourceTypeName((ResourceType)i), resid) == 0)
			res = (ResourceType)i;

	return res;
}

bool Console::cmdGetVersion(int argc, const char **argv) {
	const char *viewTypeDesc[] = { "Unknown", "EGA", "Amiga ECS 32 colors", "Amiga AGA 64 colors", "VGA", "VGA SCI1.1" };

	bool hasVocab997 = g_sci->getResMan()->testResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SELECTORS)) ? true : false;
	Common::String gameVersion = "N/A";

	Common::File versionFile;
	if (versionFile.open("VERSION")) {
		gameVersion = versionFile.readLine();
		versionFile.close();
	}

	DebugPrintf("Game ID: %s\n", _engine->getGameIdStr());
	DebugPrintf("Emulated interpreter version: %s\n", getSciVersionDesc(getSciVersion()));
	DebugPrintf("\n");
	DebugPrintf("Detected features:\n");
	DebugPrintf("------------------\n");
	DebugPrintf("Sound type: %s\n", getSciVersionDesc(_engine->_features->detectDoSoundType()));
	DebugPrintf("Graphics functions type: %s\n", getSciVersionDesc(_engine->_features->detectGfxFunctionsType()));
	DebugPrintf("Lofs type: %s\n", getSciVersionDesc(_engine->_features->detectLofsType()));
	DebugPrintf("Move count type: %s\n", (_engine->_features->handleMoveCount()) ? "increment" : "ignore");
	DebugPrintf("SetCursor type: %s\n", getSciVersionDesc(_engine->_features->detectSetCursorType()));
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2)
		DebugPrintf("kString type: %s\n", (_engine->_features->detectSci2StringFunctionType() == kSci2StringFunctionOld) ? "SCI2 (old)" : "SCI2.1 (new)");
	if (getSciVersion() == SCI_VERSION_2_1)
		DebugPrintf("SCI2.1 kernel table: %s\n", (_engine->_features->detectSci21KernelType() == SCI_VERSION_2) ? "modified SCI2 (old)" : "SCI2.1 (new)");
#endif
	DebugPrintf("View type: %s\n", viewTypeDesc[g_sci->getResMan()->getViewType()]);
	DebugPrintf("Uses palette merging: %s\n", g_sci->_gfxPalette->isMerging() ? "yes" : "no");
	DebugPrintf("Resource volume version: %s\n", g_sci->getResMan()->getVolVersionDesc());
	DebugPrintf("Resource map version: %s\n", g_sci->getResMan()->getMapVersionDesc());
	DebugPrintf("Contains selector vocabulary (vocab.997): %s\n", hasVocab997 ? "yes" : "no");
	DebugPrintf("Has CantBeHere selector: %s\n", g_sci->getKernel()->_selectorCache.cantBeHere != -1 ? "yes" : "no");
	DebugPrintf("Game version (VERSION file): %s\n", gameVersion.c_str());
	DebugPrintf("\n");

	return true;
}

bool Console::cmdOpcodes(int argc, const char **argv) {
	// Load the opcode table from vocab.998 if it exists, to obtain the opcode names
	Resource *r = _engine->getResMan()->findResource(ResourceId(kResourceTypeVocab, 998), 0);

	// If the resource couldn't be loaded, leave
	if (!r) {
		DebugPrintf("unable to load vocab.998");
		return true;
	}

	int count = READ_LE_UINT16(r->data);

	DebugPrintf("Opcode names in numeric order [index: type name]:\n");

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset) - 2;
		int type = READ_LE_UINT16(r->data + offset + 2);
		// QFG3 has empty opcodes
		Common::String name = len > 0 ? Common::String((const char *)r->data + offset + 4, len) : "Dummy";
		DebugPrintf("%03x: %03x %20s | ", i, type, name.c_str());
		if ((i % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSelector(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Attempts to find the requested selector by name.\n");
		DebugPrintf("Usage: %s <selector name>\n", argv[0]);
		return true;
	}

	Common::String name = argv[1];
	int seeker = _engine->getKernel()->findSelector(name.c_str());
	if (seeker >= 0) {
		DebugPrintf("Selector %s found at %03x (%d)\n", name.c_str(), seeker, seeker);
		return true;
	}

	DebugPrintf("Selector %s wasn't found\n", name.c_str());

	return true;
}

bool Console::cmdSelectors(int argc, const char **argv) {
	DebugPrintf("Selector names in numeric order:\n");
	Common::String selectorName;
	for (uint seeker = 0; seeker < _engine->getKernel()->getSelectorNamesSize(); seeker++) {
		selectorName = _engine->getKernel()->getSelectorName(seeker);
		if (selectorName != "BAD SELECTOR")
			DebugPrintf("%03x: %20s | ", seeker, selectorName.c_str());
		else
			continue;
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

#if 0
	// For debug/development

	// If we ever need to modify static_selectors.cpp, this code will print the selectors
	// in a ready to use format
	Common::DumpFile *outFile = new Common::DumpFile();
	outFile->open("selectors.txt");
	char buf[50];
	Common::String selName;
	uint totalSize = _engine->getKernel()->getSelectorNamesSize();
	uint seeker = 0;
	while (seeker < totalSize) {
		selName = "\"" + _engine->getKernel()->getSelectorName(seeker) + "\"";
		sprintf(buf, "%15s, ", selName.c_str());
		outFile->writeString(buf);

		if (!((seeker + 1) % 5) && seeker)
			outFile->writeByte('\n');
		seeker++;
	}
	outFile->finalize();
	outFile->close();
#endif

	return true;
}

bool Console::cmdKernelFunctions(int argc, const char **argv) {
	DebugPrintf("Kernel function names in numeric order:\n");
	for (uint seeker = 0; seeker <  _engine->getKernel()->getKernelNamesSize(); seeker++) {
		DebugPrintf("%03x: %20s | ", seeker, _engine->getKernel()->getKernelName(seeker).c_str());
		if ((seeker % 3) == 2)
			DebugPrintf("\n");
	}

	DebugPrintf("\n");

	return true;
}

bool Console::cmdSuffixes(int argc, const char **argv) {
	_engine->getVocabulary()->printSuffixes();

	return true;
}

bool Console::cmdParserWords(int argc, const char **argv) {
	_engine->getVocabulary()->printParserWords();

	return true;
}

bool Console::cmdSetParseNodes(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Sets the contents of all parse nodes.\n");
		DebugPrintf("Usage: %s <parse node1> <parse node2> ... <parse noden>\n", argv[0]);
		DebugPrintf("Tokens should be separated by blanks and enclosed in parentheses\n");
		return true;
	}

	int i = 0;
	int pos = -1;
	int nextToken = 0, nextValue = 0;

	const char *token = argv[i++];

	if (!strcmp(token, "(")) {
		nextToken = kParseOpeningParenthesis;
	} else if (!strcmp(token, ")")) {
		nextToken = kParseClosingParenthesis;
	} else if (!strcmp(token, "nil")) {
		nextToken = kParseNil;
	} else {
		nextValue = strtol(token, NULL, 0);
		nextToken = kParseNumber;
	}

	if (_engine->getVocabulary()->parseNodes(&i, &pos, nextToken, nextValue, argc, argv) == -1)
		return 1;

	_engine->getVocabulary()->dumpParseTree();

	return true;
}

bool Console::cmdRegisters(int argc, const char **argv) {
	EngineState *s = _engine->_gamestate;
	DebugPrintf("Current register values:\n");
	DebugPrintf("acc=%04x:%04x prev=%04x:%04x &rest=%x\n", PRINT_REG(s->r_acc), PRINT_REG(s->r_prev), s->r_rest);

	if (!s->_executionStack.empty()) {
		DebugPrintf("pc=%04x:%04x obj=%04x:%04x fp=ST:%04x sp=ST:%04x\n",
					PRINT_REG(s->xs->addr.pc), PRINT_REG(s->xs->objp),
					(unsigned)(s->xs->fp - s->stack_base), (unsigned)(s->xs->sp - s->stack_base));
	} else
		DebugPrintf("<no execution stack: pc,obj,fp omitted>\n");

	return true;
}

bool Console::cmdDiskDump(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Dumps the specified resource to disk as a patch file\n");
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		cmdResourceTypes(argc, argv);
		return true;
	}

	int resNum = atoi(argv[2]);
	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _engine->getResMan()->findResource(ResourceId(res, resNum), 0);
		if (resource) {
			char outFileName[50];
			sprintf(outFileName, "%s.%03d", getResourceTypeName(res), resNum);
			Common::DumpFile *outFile = new Common::DumpFile();
			outFile->open(outFileName);
			resource->writeToStream(outFile);
			outFile->finalize();
			outFile->close();
			delete outFile;
			DebugPrintf("Resource %s.%03d (located in %s) has been dumped to disk\n", argv[1], resNum, resource->getResourceLocation().c_str());
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdHexDump(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Dumps the specified resource to standard output\n");
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		cmdResourceTypes(argc, argv);
		return true;
	}

	int resNum = atoi(argv[2]);
	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _engine->getResMan()->findResource(ResourceId(res, resNum), 0);
		if (resource) {
			Common::hexdump(resource->data, resource->size, 16, 0);
			DebugPrintf("Resource %s.%03d has been dumped to standard output\n", argv[1], resNum);
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceId(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Identifies a resource number by splitting it up in resource type and resource number\n");
		DebugPrintf("Usage: %s <resource number>\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	DebugPrintf("%s.%d (0x%x)\n", getResourceTypeName((ResourceType)(id >> 11)), id & 0x7ff, id & 0x7ff);

	return true;
}

bool Console::cmdDissectScript(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Examines a script\n");
		DebugPrintf("Usage: %s <script number>\n", argv[0]);
		return true;
	}

	_engine->getKernel()->dissectScript(atoi(argv[1]), _engine->getVocabulary());

	return true;
}

bool Console::cmdRoomNumber(int argc, const char **argv) {
	// The room number is stored in global var 13
	// The same functionality is provided by "vmvars g 13" (but this one is more straighforward)

	if (argc != 2) {
		DebugPrintf("Current room number is %d\n", _engine->_gamestate->currentRoomNumber());
		DebugPrintf("Calling this command with the room number (in decimal or hexadecimal) changes the room\n");
	} else {
		Common::String roomNumberStr = argv[1];
		int roomNumber = strtol(roomNumberStr.c_str(), NULL, roomNumberStr.hasSuffix("h") ? 16 : 10);
		_engine->_gamestate->setRoomNumber(roomNumber);
		DebugPrintf("Room number changed to %d (%x in hex)\n", roomNumber, roomNumber);
	}

	return true;
}

bool Console::cmdResourceInfo(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Shows information about a resource\n");
		DebugPrintf("Usage: %s <resource type> <resource number>\n", argv[0]);
		return true;
	}

	int resNum = atoi(argv[2]);
	ResourceType res = parseResourceType(argv[1]);

	if (res == kResourceTypeInvalid)
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
	else {
		Resource *resource = _engine->getResMan()->findResource(ResourceId(res, resNum), 0);
		if (resource) {
			DebugPrintf("Resource size: %d\n", resource->size);
			DebugPrintf("Resource location: %s\n", resource->getResourceLocation().c_str());
		} else {
			DebugPrintf("Resource %s.%03d not found\n", argv[1], resNum);
		}
	}

	return true;
}

bool Console::cmdResourceTypes(int argc, const char **argv) {
	DebugPrintf("The %d valid resource types are:\n", kResourceTypeInvalid);
	for (int i = 0; i < kResourceTypeInvalid; i++) {
		DebugPrintf("%s", getResourceTypeName((ResourceType) i));
		DebugPrintf((i < kResourceTypeInvalid - 1) ? ", " : "\n");
	}

	return true;
}

bool Console::cmdHexgrep(int argc, const char **argv) {
	if (argc < 4) {
		DebugPrintf("Searches some resources for a particular sequence of bytes, represented as decimal or hexadecimal numbers.\n");
		DebugPrintf("Usage: %s <resource type> <resource number> <search string>\n", argv[0]);
		DebugPrintf("<resource number> can be a specific resource number, or \"all\" for all of the resources of the specified type\n");
		DebugPrintf("EXAMPLES:\n  hexgrep script all 0xe8 0x03 0xc8 0x00\n  hexgrep pic 0x42 0xfe\n");
		cmdResourceTypes(argc, argv);
		return true;
	}

	ResourceType restype = parseResourceType(argv[1]);
	int resNumber = 0, resMax = 0;
	Resource *script = NULL;

	if (restype == kResourceTypeInvalid) {
		DebugPrintf("Resource type '%s' is not valid\n", argv[1]);
		return true;
	}

	if (!scumm_stricmp(argv[2], "all")) {
		resNumber = 0;
		resMax = 65535;
	} else {
		resNumber = resMax = atoi(argv[2]);
	}

	// Convert the bytes
	Common::Array<int> byteString;
	byteString.resize(argc - 3);

	for (uint i = 0; i < byteString.size(); i++)
		if (!parseInteger(argv[i + 3], byteString[i]))
			return true;

	for (; resNumber <= resMax; resNumber++) {
		script = _engine->getResMan()->findResource(ResourceId(restype, resNumber), 0);
		if (script) {
			unsigned int seeker = 0, seekerold = 0;
			uint32 comppos = 0;
			int output_script_name = 0;

			while (seeker < script->size) {
				if (script->data[seeker] == byteString[comppos]) {
					if (comppos == 0)
						seekerold = seeker;

					comppos++;

					if (comppos == byteString.size()) {
						comppos = 0;
						seeker = seekerold + 1;

						if (!output_script_name) {
							DebugPrintf("\nIn %s.%03d:\n", getResourceTypeName((ResourceType)restype), resNumber);
							output_script_name = 1;
						}
						DebugPrintf("   0x%04x\n", seekerold);
					}
				} else
					comppos = 0;

				seeker++;
			}
		}
	}

	return true;
}

bool Console::cmdVerifyScripts(int argc, const char **argv) {
	if (getSciVersion() < SCI_VERSION_1_1) {
		DebugPrintf("This script check is only meant for SCI1.1-SCI3 games\n");
		return true;
	}

	Common::List<ResourceId> resources = _engine->getResMan()->listResources(kResourceTypeScript);
	Common::sort(resources.begin(), resources.end());

	DebugPrintf("%d SCI1.1-SCI3 scripts found, performing sanity checks...\n", resources.size());

	Resource *script, *heap;
	Common::List<ResourceId>::iterator itr;
	for (itr = resources.begin(); itr != resources.end(); ++itr) {
		script = _engine->getResMan()->findResource(*itr, false);
		if (!script)
			DebugPrintf("Error: script %d couldn't be loaded\n", itr->getNumber());

		if (getSciVersion() <= SCI_VERSION_2_1) {
			heap = _engine->getResMan()->findResource(ResourceId(kResourceTypeHeap, itr->getNumber()), false);
			if (!heap)
				DebugPrintf("Error: script %d doesn't have a corresponding heap\n", itr->getNumber());

			if (script && heap && (script->size + heap->size > 65535))
				DebugPrintf("Error: script and heap %d together are larger than 64KB (%d bytes)\n",
				itr->getNumber(), script->size + heap->size);
		} else {	// SCI3
			if (script && script->size > 65535)
				DebugPrintf("Error: script %d is larger than 64KB (%d bytes)\n",
				itr->getNumber(), script->size);
		}
	}

	DebugPrintf("SCI1.1-SCI2.1 script check finished\n");

	return true;
}

// Same as in sound/drivers/midi.cpp
uint8 getGmInstrument(const Mt32ToGmMap &Mt32Ins) {
	if (Mt32Ins.gmInstr == MIDI_MAPPED_TO_RHYTHM)
		return Mt32Ins.gmRhythmKey + 0x80;
	else
		return Mt32Ins.gmInstr;
}

bool Console::cmdShowInstruments(int argc, const char **argv) {
	int songNumber = -1;

	if (argc == 2)
		songNumber = atoi(argv[1]);

	SciVersion doSoundVersion = _engine->_features->detectDoSoundType();
	MidiPlayer *player = MidiPlayer_Midi_create(doSoundVersion);
	MidiParser_SCI *parser = new MidiParser_SCI(doSoundVersion, 0);
	parser->setMidiDriver(player);

	Common::List<ResourceId> resources = _engine->getResMan()->listResources(kResourceTypeSound);
	Common::sort(resources.begin(), resources.end());
	int instruments[128];
	bool instrumentsSongs[128][1000];

	for (int i = 0; i < 128; i++)
		instruments[i] = 0;

	for (int i = 0; i < 128; i++)
		for (int j = 0; j < 1000; j++)
			instrumentsSongs[i][j] = false;

	if (songNumber == -1) {
		DebugPrintf("%d sounds found, checking their instrument mappings...\n", resources.size());
		DebugPrintf("Instruments:\n");
		DebugPrintf("============\n");
	}

	Common::List<ResourceId>::iterator itr;
	for (itr = resources.begin(); itr != resources.end(); ++itr) {
		if (songNumber >= 0 && itr->getNumber() != songNumber)
			continue;

		SoundResource sound(itr->getNumber(), _engine->getResMan(), doSoundVersion);
		int channelFilterMask = sound.getChannelFilterMask(player->getPlayId(), player->hasRhythmChannel());
		SoundResource::Track *track = sound.getTrackByType(player->getPlayId());
		if (track->digitalChannelNr != -1) {
			// Skip digitized sound effects
			continue;
		}

		parser->loadMusic(track, NULL, channelFilterMask, doSoundVersion);
		const byte *channelData = parser->getMixedData();

		byte curEvent = 0, prevEvent = 0, command = 0;
		bool endOfTrack = false;
		bool firstOneShown = false;

		DebugPrintf("Song %d: ", itr->getNumber());

		do {
			while (*channelData == 0xF8)
				channelData++;

			channelData++;	// delta

			if ((*channelData & 0xF0) >= 0x80)
				curEvent = *(channelData++);
			else
				curEvent = prevEvent;
			if (curEvent < 0x80)
				continue;

			prevEvent = curEvent;
			command = curEvent >> 4;

			byte channel;

			switch (command) {
			case 0xC:	// program change
				channel = curEvent & 0x0F;
				if (channel != 15) {	// SCI special
					byte instrument = *channelData++;
					if (!firstOneShown)
						firstOneShown = true;
					else
						DebugPrintf(",");

					DebugPrintf(" %d", instrument);
					instruments[instrument]++;
					instrumentsSongs[instrument][itr->getNumber()] = true;
				} else {
					channelData++;
				}
				break;
			case 0xD:
				channelData++;	// param1
				break;
			case 0xB:
			case 0x8:
			case 0x9:
			case 0xA:
			case 0xE:
				channelData++;	// param1
				channelData++;	// param2
				break;
			case 0xF:
				if ((curEvent & 0x0F) == 0x2) {
					channelData++;	// param1
					channelData++;	// param2
				} else if ((curEvent & 0x0F) == 0x3) {
					channelData++;	// param1
				} else if ((curEvent & 0x0F) == 0xF) {	// META
					byte type = *channelData++;
					if (type == 0x2F) {// end of track reached
						endOfTrack = true;
					} else {
						// no further processing necessary
					}
				}
				break;
			default:
				break;
			}
		} while (!endOfTrack);

		DebugPrintf("\n");
	}

	delete parser;
	delete player;

	DebugPrintf("\n");

	if (songNumber == -1) {
		DebugPrintf("Used instruments: ");
		for (int i = 0; i < 128; i++) {
			if (instruments[i] > 0)
				DebugPrintf("%d, ", i);
		}
		DebugPrintf("\n\n");
	}

	DebugPrintf("Instruments not mapped in the MT32->GM map: ");
	for (int i = 0; i < 128; i++) {
		if (instruments[i] > 0 && getGmInstrument(Mt32MemoryTimbreMaps[i]) == MIDI_UNMAPPED)
			DebugPrintf("%d, ", i);
	}
	DebugPrintf("\n\n");

	if (songNumber == -1) {
		DebugPrintf("Used instruments in songs:\n");
		for (int i = 0; i < 128; i++) {
			if (instruments[i] > 0) {
				DebugPrintf("Instrument %d: ", i);
				for (int j = 0; j < 1000; j++) {
					if (instrumentsSongs[i][j])
						DebugPrintf("%d, ", j);
				}
				DebugPrintf("\n");
			}
		}

		DebugPrintf("\n\n");
	}

	return true;
}

bool Console::cmdMapInstrument(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Maps an MT-32 custom instrument to a GM instrument on the fly\n\n");
		DebugPrintf("Usage %s <MT-32 instrument name> <GM instrument> <GM rhythm key>\n", argv[0]);
		DebugPrintf("Each MT-32 instrument is always 10 characters and is mapped to either a GM instrument, or a GM rhythm key\n");
		DebugPrintf("A value of 255 (0xff) signifies an unmapped instrument\n");
		DebugPrintf("Please replace the spaces in the instrument name with underscores (\"_\"). They'll be converted to spaces afterwards\n\n");
		DebugPrintf("Example: %s test_0__XX 1 255\n", argv[0]);
		DebugPrintf("The above example will map the MT-32 instrument \"test 0  XX\" to GM instrument 1\n\n");
	} else {
		if (Mt32dynamicMappings != NULL) {
			Mt32ToGmMap newMapping;
			char *instrumentName = new char[11];
			Common::strlcpy(instrumentName, argv[1], 11);

			for (uint16 i = 0; i < strlen(instrumentName); i++)
				if (instrumentName[i] == '_')
					instrumentName[i] = ' ';

			newMapping.name = instrumentName;
			newMapping.gmInstr = atoi(argv[2]);
			newMapping.gmRhythmKey = atoi(argv[3]);
			Mt32dynamicMappings->push_back(newMapping);
		}
	}

	DebugPrintf("Current dynamic mappings:\n");
	if (Mt32dynamicMappings != NULL) {
		const Mt32ToGmMapList::iterator end = Mt32dynamicMappings->end();
		for (Mt32ToGmMapList::iterator it = Mt32dynamicMappings->begin(); it != end; ++it) {
			DebugPrintf("\"%s\" -> %d / %d\n", (*it).name, (*it).gmInstr, (*it).gmRhythmKey);
		}
	}

	return true;
}

bool Console::cmdList(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Lists all the resources of a given type\n");
		cmdResourceTypes(argc, argv);
		return true;
	}


	ResourceType res = parseResourceType(argv[1]);
	if (res == kResourceTypeInvalid)
		DebugPrintf("Unknown resource type: '%s'\n", argv[1]);
	else {
		int number = -1;

		if ((res == kResourceTypeAudio36) || (res == kResourceTypeSync36)) {
			if (argc != 3) {
				DebugPrintf("Please specify map number (-1: all maps)\n");
				return true;
			}
			number = atoi(argv[2]);
		}

		Common::List<ResourceId> resources = _engine->getResMan()->listResources(res, number);
		Common::sort(resources.begin(), resources.end());

		int cnt = 0;
		Common::List<ResourceId>::iterator itr;
		for (itr = resources.begin(); itr != resources.end(); ++itr) {
			if (number == -1) {
				DebugPrintf("%8i", itr->getNumber());
				if (++cnt % 10 == 0)
					DebugPrintf("\n");
			} else if (number == (int)itr->getNumber()) {
				const uint32 tuple = itr->getTuple();
				DebugPrintf("(%3i, %3i, %3i, %3i)   ", (tuple >> 24) & 0xff, (tuple >> 16) & 0xff,
							(tuple >> 8) & 0xff, tuple & 0xff);
				if (++cnt % 4 == 0)
					DebugPrintf("\n");
			}
		}
		DebugPrintf("\n");
	}

	return true;
}

bool Console::cmdSaveGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Saves the current game state to the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	int result = 0;
	for (uint i = 0; i < _engine->_gamestate->_fileHandles.size(); i++)
		if (_engine->_gamestate->_fileHandles[i].isOpen())
			result++;

	if (result)
		DebugPrintf("Note: Game state has %d open file handles.\n", result);

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out = saveFileMan->openForSaving(argv[1]);
	const char *version = "";
	if (!out) {
		DebugPrintf("Error opening savegame \"%s\" for writing\n", argv[1]);
		return true;
	}

	// TODO: enable custom descriptions? force filename into a specific format?
	if (!gamestate_save(_engine->_gamestate, out, "debugging", version)) {
		DebugPrintf("Saving the game state to '%s' failed\n", argv[1]);
	} else {
		out->finalize();
		if (out->err()) {
			warning("Writing the savegame failed");
		}
		delete out;
	}

	return true;
}

bool Console::cmdRestoreGame(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Restores a saved game from the hard disk\n");
		DebugPrintf("Usage: %s <filename>\n", argv[0]);
		return true;
	}

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in = saveFileMan->openForLoading(argv[1]);
	if (in) {
		// found a savegame file
		gamestate_restore(_engine->_gamestate, in);
		delete in;
	}

	if (_engine->_gamestate->r_acc == make_reg(0, 1)) {
		DebugPrintf("Restoring gamestate '%s' failed.\n", argv[1]);
		return true;
	}

	return Cmd_Exit(0, 0);
}

bool Console::cmdRestartGame(int argc, const char **argv) {
	_engine->_gamestate->abortScriptProcessing = kAbortRestartGame;

	return Cmd_Exit(0, 0);
}

// The scripts get IDs ranging from 100->199, because the scripts require us to assign unique ids THAT EVEN STAY BETWEEN
//  SAVES and the scripts also use "saves-count + 1" to create a new savedgame slot.
//  SCI1.1 actually recycles ids, in that case we will currently get "0".
// This behavior is required especially for LSL6. In this game, it's possible to quick save. The scripts will use
//  the last-used id for that feature. If we don't assign sticky ids, the feature will overwrite different saves all the
//  time. And sadly we can't just use the actual filename ids directly, because of the creation method for new slots.

extern void listSavegames(Common::Array<SavegameDesc> &saves);

bool Console::cmdListSaves(int argc, const char **argv) {
	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	for (uint i = 0; i < saves.size(); i++) {
		Common::String filename = g_sci->getSavegameName(saves[i].id);
		DebugPrintf("%s: '%s'\n", filename.c_str(), saves[i].name);
	}

	return true;
}

bool Console::cmdClassTable(int argc, const char **argv) {
	DebugPrintf("Available classes (parse a parameter to filter the table by a specific class):\n");

	for (uint i = 0; i < _engine->_gamestate->_segMan->classTableSize(); i++) {
		Class temp = _engine->_gamestate->_segMan->_classTable[i];
		if (temp.reg.getSegment()) {
			const char *className = _engine->_gamestate->_segMan->getObjectName(temp.reg);
			if (argc == 1 || (argc == 2 && !strcmp(className, argv[1]))) {
				DebugPrintf(" Class 0x%x (%s) at %04x:%04x (script %d)\n", i,
						className,
						PRINT_REG(temp.reg),
						temp.script);
			} else DebugPrintf(" Class 0x%x (not loaded; can't get name) (script %d)\n", i, temp.script);
		}
	}

	return true;
}

bool Console::cmdSentenceFragments(int argc, const char **argv) {
	DebugPrintf("Sentence fragments (used to build Parse trees)\n");

	for (uint i = 0; i < _engine->getVocabulary()->getParserBranchesSize(); i++) {
		int j = 0;

		const parse_tree_branch_t &branch = _engine->getVocabulary()->getParseTreeBranch(i);
		DebugPrintf("R%02d: [%x] ->", i, branch.id);
		while ((j < 10) && branch.data[j]) {
			int dat = branch.data[j++];

			switch (dat) {
			case VOCAB_TREE_NODE_COMPARE_TYPE:
				dat = branch.data[j++];
				DebugPrintf(" C(%x)", dat);
				break;

			case VOCAB_TREE_NODE_COMPARE_GROUP:
				dat = branch.data[j++];
				DebugPrintf(" WG(%x)", dat);
				break;

			case VOCAB_TREE_NODE_FORCE_STORAGE:
				dat = branch.data[j++];
				DebugPrintf(" FORCE(%x)", dat);
				break;

			default:
				if (dat > VOCAB_TREE_NODE_LAST_WORD_STORAGE) {
					int dat2 = branch.data[j++];
					DebugPrintf(" %x[%x]", dat, dat2);
				} else
					DebugPrintf(" ?%x?", dat);
			}
		}
		DebugPrintf("\n");
	}

	DebugPrintf("%d rules.\n", _engine->getVocabulary()->getParserBranchesSize());

	return true;
}

bool Console::cmdParse(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Parses a sequence of words with a GNF rule set and prints the resulting parse tree\n");
		DebugPrintf("Usage: %s <word1> <word2> ... <wordn>\n", argv[0]);
		return true;
	}

	char *error;
	char string[1000];

	// Construct the string
	strcpy(string, argv[1]);
	for (int i = 2; i < argc; i++) {
		strcat(string, " ");
		strcat(string, argv[i]);
	}

	DebugPrintf("Parsing '%s'\n", string);

	ResultWordListList words;
	bool res = _engine->getVocabulary()->tokenizeString(words, string, &error);
	if (res && !words.empty()) {
		int syntax_fail = 0;

		_engine->getVocabulary()->synonymizeTokens(words);

		DebugPrintf("Parsed to the following blocks:\n");

		for (ResultWordListList::const_iterator i = words.begin(); i != words.end(); ++i) {
			DebugPrintf("   ");
			for (ResultWordList::const_iterator j = i->begin(); j != i->end(); ++j) {
				DebugPrintf("%sType[%04x] Group[%04x]", j == i->begin() ? "" : " / ", j->_class, j->_group);
			}
			DebugPrintf("\n");
		}

		if (_engine->getVocabulary()->parseGNF(words, true))
			syntax_fail = 1; // Building a tree failed

		if (syntax_fail)
			DebugPrintf("Building a tree failed.\n");
		else
			_engine->getVocabulary()->dumpParseTree();

	} else {
		DebugPrintf("Unknown word: '%s'\n", error);
		free(error);
	}

	return true;
}

bool Console::cmdSaid(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Matches a string against a said spec\n");
		DebugPrintf("Usage: %s <string> > & <said spec>\n", argv[0]);
		DebugPrintf("<string> is a sequence of actual words.\n");
		DebugPrintf("<said spec> is a sequence of hex tokens.\n");
		return true;
	}

	char *error;
	char string[1000];
	byte spec[1000];

	int p;
	// Construct the string
	strcpy(string, argv[1]);
	for (p = 2; p < argc && strcmp(argv[p],"&") != 0; p++) {
		strcat(string, " ");
		strcat(string, argv[p]);
	}

	if (p >= argc-1) {
		DebugPrintf("Matches a string against a said spec\n");
		DebugPrintf("Usage: %s <string> > & <said spec>\n", argv[0]);
		DebugPrintf("<string> is a sequence of actual words.\n");
		DebugPrintf("<said spec> is a sequence of hex tokens.\n");
		return true;
	}

	// TODO: Maybe turn this into a proper said spec compiler
	unsigned int len = 0;
	for (p++; p < argc; p++) {
		if (strcmp(argv[p], ",") == 0) {
			spec[len++] = 0xf0;
		} else if (strcmp(argv[p], "&") == 0) {
			spec[len++] = 0xf1;
		} else if (strcmp(argv[p], "/") == 0) {
			spec[len++] = 0xf2;
		} else if (strcmp(argv[p], "(") == 0) {
			spec[len++] = 0xf3;
		} else if (strcmp(argv[p], ")") == 0) {
			spec[len++] = 0xf4;
		} else if (strcmp(argv[p], "[") == 0) {
			spec[len++] = 0xf5;
		} else if (strcmp(argv[p], "]") == 0) {
			spec[len++] = 0xf6;
		} else if (strcmp(argv[p], "#") == 0) {
			spec[len++] = 0xf7;
		} else if (strcmp(argv[p], "<") == 0) {
			spec[len++] = 0xf8;
		} else if (strcmp(argv[p], ">") == 0) {
			spec[len++] = 0xf9;
		} else if (strcmp(argv[p], "[<") == 0) {
			spec[len++] = 0xf5;
			spec[len++] = 0xf8;
		} else if (strcmp(argv[p], "[/") == 0) {
			spec[len++] = 0xf5;
			spec[len++] = 0xf2;
		} else if (strcmp(argv[p], "!*") == 0) {
			spec[len++] = 0x0f;
			spec[len++] = 0xfe;
		} else if (strcmp(argv[p], "[!*]") == 0) {
			spec[len++] = 0xf5;
			spec[len++] = 0x0f;
			spec[len++] = 0xfe;
			spec[len++] = 0xf6;
		} else {
			unsigned int s = strtol(argv[p], 0, 16);
			if (s >= 0xf0 && s <= 0xff) {
				spec[len++] = s;
			} else {
				spec[len++] = s >> 8;
				spec[len++] = s & 0xFF;
			}
		}
	}
	spec[len++] = 0xFF;

	debugN("Matching '%s' against:", string);
	_engine->getVocabulary()->debugDecipherSaidBlock(spec);
	debugN("\n");

	ResultWordListList words;
	bool res = _engine->getVocabulary()->tokenizeString(words, string, &error);
	if (res && !words.empty()) {
		int syntax_fail = 0;

		_engine->getVocabulary()->synonymizeTokens(words);

		DebugPrintf("Parsed to the following blocks:\n");

		for (ResultWordListList::const_iterator i = words.begin(); i != words.end(); ++i) {
			DebugPrintf("   ");
			for (ResultWordList::const_iterator j = i->begin(); j != i->end(); ++j) {
				DebugPrintf("%sType[%04x] Group[%04x]", j == i->begin() ? "" : " / ", j->_class, j->_group);
			}
			DebugPrintf("\n");
		}



		if (_engine->getVocabulary()->parseGNF(words, true))
			syntax_fail = 1; // Building a tree failed

		if (syntax_fail)
			DebugPrintf("Building a tree failed.\n");
		else {
			_engine->getVocabulary()->dumpParseTree();
			_engine->getVocabulary()->parserIsValid = true;

			int ret = said((byte *)spec, true);
			DebugPrintf("kSaid: %s\n", (ret == SAID_NO_MATCH ? "No match" : "Match"));
		}

	} else {
		DebugPrintf("Unknown word: '%s'\n", error);
		free(error);
	}

	return true;
}


bool Console::cmdParserNodes(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows the specified number of nodes from the parse node tree\n");
		DebugPrintf("Usage: %s <nr>\n", argv[0]);
		DebugPrintf("where <nr> is the number of nodes to show from the parse node tree\n");
		return true;
	}

	int end = MIN<int>(atoi(argv[1]), VOCAB_TREE_NODES);

	_engine->getVocabulary()->printParserNodes(end);

	return true;
}

bool Console::cmdSetPalette(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Sets a palette resource\n");
		DebugPrintf("Usage: %s <resourceId>\n", argv[0]);
		DebugPrintf("where <resourceId> is the number of the palette resource to set\n");
		return true;
	}

	uint16 resourceId = atoi(argv[1]);

	_engine->_gfxPalette->kernelSetFromResource(resourceId, true);
	return true;
}

bool Console::cmdDrawPic(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Draws a pic resource\n");
		DebugPrintf("Usage: %s <resourceId>\n", argv[0]);
		DebugPrintf("where <resourceId> is the number of the pic resource to draw\n");
		return true;
	}

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// If a graphical debugger overlay is used, hide it here, so that the
	// results can be drawn.
	g_system->hideOverlay();
#endif

	uint16 resourceId = atoi(argv[1]);
	_engine->_gfxPaint->kernelDrawPicture(resourceId, 100, false, false, false, 0);
	_engine->_gfxScreen->copyToScreen();
	_engine->sleep(2000);

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// Show the graphical debugger overlay
	g_system->showOverlay();
#endif

	return true;
}

bool Console::cmdDrawCel(int argc, const char **argv) {
	if (argc < 4) {
		DebugPrintf("Draws a cel from a view resource\n");
		DebugPrintf("Usage: %s <resourceId> <loopNr> <celNr> \n", argv[0]);
		DebugPrintf("where <resourceId> is the number of the view resource to draw\n");
		return true;
	}

	uint16 resourceId = atoi(argv[1]);
	uint16 loopNo = atoi(argv[2]);
	uint16 celNo = atoi(argv[3]);

	if (_engine->_gfxPaint16) {
		_engine->_gfxPaint16->kernelDrawCel(resourceId, loopNo, celNo, 50, 50, 0, 0, 128, 128, false, NULL_REG);
	} else {
		GfxView *view = _engine->_gfxCache->getView(resourceId);
		Common::Rect celRect(50, 50, 50 + view->getWidth(loopNo, celNo), 50 + view->getHeight(loopNo, celNo));
		view->draw(celRect, celRect, celRect, loopNo, celNo, 255, 0, false);
		_engine->_gfxScreen->copyRectToScreen(celRect);
	}
	return true;
}

bool Console::cmdUndither(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Enable/disable undithering.\n");
		DebugPrintf("Usage: %s <0/1>\n", argv[0]);
		return true;
	}

	bool flag = atoi(argv[1]) ? true : false;
	_engine->_gfxScreen->enableUndithering(flag);
	if (flag)
		DebugPrintf("undithering ENABLED\n");
	else
		DebugPrintf("undithering DISABLED\n");
	return true;
}

bool Console::cmdPicVisualize(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Enable/disable picture visualization (EGA only)\n");
		DebugPrintf("Usage: %s <0/1>\n", argv[0]);
		return true;
	}

	bool state = atoi(argv[1]) ? true : false;

	if (_engine->_resMan->getViewType() == kViewEga) {
		_engine->_gfxPaint16->debugSetEGAdrawingVisualize(state);
		if (state)
			DebugPrintf("picture visualization ENABLED\n");
		else
			DebugPrintf("picture visualization DISABLED\n");
	} else {
		DebugPrintf("picture visualization only available for EGA games\n");
	}
	return true;
}

bool Console::cmdPlayVideo(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Plays a SEQ, AVI, VMD, RBT or DUK video.\n");
		DebugPrintf("Usage: %s <video file name> <delay>\n", argv[0]);
		DebugPrintf("The video file name should include the extension\n");
		DebugPrintf("Delay is only used in SEQ videos and is measured in ticks (default: 10)\n");
		return true;
	}

	Common::String filename = argv[1];
	filename.toLowercase();

	if (filename.hasSuffix(".seq") || filename.hasSuffix(".avi") || filename.hasSuffix(".vmd") ||
		filename.hasSuffix(".rbt") || filename.hasSuffix(".duk")) {
		_videoFile = filename;
		_videoFrameDelay = (argc == 2) ? 10 : atoi(argv[2]);
		return Cmd_Exit(0, 0);
	} else {
		DebugPrintf("Unknown video file type\n");
		return true;
	}
}

bool Console::cmdAnimateList(int argc, const char **argv) {
	if (_engine->_gfxAnimate) {
		DebugPrintf("Animate list:\n");
		_engine->_gfxAnimate->printAnimateList(this);
	} else {
		DebugPrintf("This SCI version does not have an animate list\n");
	}
	return true;
}

bool Console::cmdWindowList(int argc, const char **argv) {
	if (_engine->_gfxPorts) {
		DebugPrintf("Window list:\n");
		_engine->_gfxPorts->printWindowList(this);
	} else {
		DebugPrintf("This SCI version does not have a list of ports\n");
	}
	return true;
}

bool Console::cmdPlaneList(int argc, const char **argv) {
#ifdef ENABLE_SCI32
	if (_engine->_gfxFrameout) {
		DebugPrintf("Plane list:\n");
		_engine->_gfxFrameout->printPlaneList(this);
	} else {
		DebugPrintf("This SCI version does not have a list of planes\n");
	}
#else
	DebugPrintf("SCI32 isn't included in this compiled executable\n");
#endif
	return true;
}

bool Console::cmdPlaneItemList(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows the list of items for a plane\n");
		DebugPrintf("Usage: %s <plane address>\n", argv[0]);
		return true;
	}

	reg_t planeObject = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &planeObject, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

#ifdef ENABLE_SCI32
	if (_engine->_gfxFrameout) {
		DebugPrintf("Plane item list:\n");
		_engine->_gfxFrameout->printPlaneItemList(this, planeObject);
	} else {
		DebugPrintf("This SCI version does not have a list of plane items\n");
	}
#else
	DebugPrintf("SCI32 isn't included in this compiled executable\n");
#endif
	return true;
}

bool Console::cmdSavedBits(int argc, const char **argv) {
	SegManager *segman = _engine->_gamestate->_segMan;
	SegmentId id = segman->findSegmentByType(SEG_TYPE_HUNK);
	HunkTable* hunks = (HunkTable *)segman->getSegmentObj(id);
	if (!hunks) {
		DebugPrintf("No hunk segment found.\n");
		return true;
	}

	Common::Array<reg_t> entries = hunks->listAllDeallocatable(id);

	for (uint i = 0; i < entries.size(); ++i) {
		uint16 offset = entries[i].getOffset();
		const Hunk& h = hunks->_table[offset];
		if (strcmp(h.type, "SaveBits()") == 0) {
			byte* memoryPtr = (byte *)h.mem;

			if (memoryPtr) {
				DebugPrintf("%04x:%04x:", PRINT_REG(entries[i]));

				Common::Rect rect;
				byte mask;
				assert(h.size >= sizeof(rect) + sizeof(mask));

				memcpy((void *)&rect, memoryPtr, sizeof(rect));
				memcpy((void *)&mask, memoryPtr + sizeof(rect), sizeof(mask));

				DebugPrintf(" %d,%d - %d,%d", rect.top, rect.left,
				                              rect.bottom, rect.right);
				if (mask & GFX_SCREEN_MASK_VISUAL)
					DebugPrintf(" visual");
				if (mask & GFX_SCREEN_MASK_PRIORITY)
					DebugPrintf(" priority");
				if (mask & GFX_SCREEN_MASK_CONTROL)
					DebugPrintf(" control");
				if (mask & GFX_SCREEN_MASK_DISPLAY)
					DebugPrintf(" display");
				DebugPrintf("\n");
			}
		}
	}


	return true;
}

bool Console::cmdShowSavedBits(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Display saved bits.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t memoryHandle = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &memoryHandle, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	if (memoryHandle.isNull()) {
		DebugPrintf("Invalid address.\n");
		return true;
	}

	SegManager *segman = _engine->_gamestate->_segMan;
	SegmentId id = segman->findSegmentByType(SEG_TYPE_HUNK);
	HunkTable* hunks = (HunkTable *)segman->getSegmentObj(id);
	if (!hunks) {
		DebugPrintf("No hunk segment found.\n");
		return true;
	}

	if (memoryHandle.getSegment() != id || !hunks->isValidOffset(memoryHandle.getOffset())) {
		DebugPrintf("Invalid address.\n");
		return true;
	}

	const Hunk& h = hunks->_table[memoryHandle.getOffset()];

	if (strcmp(h.type, "SaveBits()") != 0) {
		DebugPrintf("Invalid address.\n");
		return true;
	}

	byte *memoryPtr = segman->getHunkPointer(memoryHandle);

	if (!memoryPtr) {
		DebugPrintf("Invalid or freed bits.\n");
		return true;
	}

	// Now we _finally_ know these are valid saved bits

	Common::Rect rect;
	byte mask;
	assert(h.size >= sizeof(rect) + sizeof(mask));

	memcpy((void *)&rect, memoryPtr, sizeof(rect));
	memcpy((void *)&mask, memoryPtr + sizeof(rect), sizeof(mask));

	Common::Point tl(rect.left, rect.top);
	Common::Point tr(rect.right-1, rect.top);
	Common::Point bl(rect.left, rect.bottom-1);
	Common::Point br(rect.right-1, rect.bottom-1);

	DebugPrintf(" %d,%d - %d,%d", rect.top, rect.left,
	                              rect.bottom, rect.right);
	if (mask & GFX_SCREEN_MASK_VISUAL)
		DebugPrintf(" visual");
	if (mask & GFX_SCREEN_MASK_PRIORITY)
		DebugPrintf(" priority");
	if (mask & GFX_SCREEN_MASK_CONTROL)
		DebugPrintf(" control");
	if (mask & GFX_SCREEN_MASK_DISPLAY)
		DebugPrintf(" display");
	DebugPrintf("\n");

	if (!_engine->_gfxPaint16 || !_engine->_gfxScreen)
		return true;

	// We backup all planes, and then flash the saved bits
	// FIXME: This probably won't work well with hi-res games

	byte bakMask = GFX_SCREEN_MASK_VISUAL | GFX_SCREEN_MASK_PRIORITY | GFX_SCREEN_MASK_CONTROL;
	int bakSize = _engine->_gfxScreen->bitsGetDataSize(rect, bakMask);
	reg_t bakScreen = segman->allocateHunkEntry("show_saved_bits backup", bakSize);
	byte* bakMemory = segman->getHunkPointer(bakScreen);
	assert(bakMemory);
	_engine->_gfxScreen->bitsSave(rect, bakMask, bakMemory);

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// If a graphical debugger overlay is used, hide it here, so that the
	// results can be drawn.
	g_system->hideOverlay();
#endif

	const int paintCount = 3;
	for (int i = 0; i < paintCount; ++i) {
		_engine->_gfxScreen->bitsRestore(memoryPtr);
		_engine->_gfxScreen->drawLine(tl, tr, 0, 255, 255);
		_engine->_gfxScreen->drawLine(tr, br, 0, 255, 255);
		_engine->_gfxScreen->drawLine(br, bl, 0, 255, 255);
		_engine->_gfxScreen->drawLine(bl, tl, 0, 255, 255);
		_engine->_gfxScreen->copyRectToScreen(rect);
		g_system->updateScreen();
		g_sci->sleep(500);
		_engine->_gfxScreen->bitsRestore(bakMemory);
		_engine->_gfxScreen->copyRectToScreen(rect);
		g_system->updateScreen();
		if (i < paintCount - 1)
			g_sci->sleep(500);
	}

	_engine->_gfxPaint16->bitsFree(bakScreen);

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	// Show the graphical debugger overlay
	g_system->showOverlay();
#endif

	return true;
}


bool Console::cmdParseGrammar(int argc, const char **argv) {
	DebugPrintf("Parse grammar, in strict GNF:\n");

	_engine->getVocabulary()->buildGNF(true);

	return true;
}

bool Console::cmdPrintSegmentTable(int argc, const char **argv) {
	DebugPrintf("Segment table:\n");

	for (uint i = 0; i < _engine->_gamestate->_segMan->_heap.size(); i++) {
		SegmentObj *mobj = _engine->_gamestate->_segMan->_heap[i];
		if (mobj && mobj->getType()) {
			DebugPrintf(" [%04x] ", i);

			switch (mobj->getType()) {
			case SEG_TYPE_SCRIPT:
				DebugPrintf("S  script.%03d l:%d ", (*(Script *)mobj).getScriptNumber(), (*(Script *)mobj).getLockers());
				break;

			case SEG_TYPE_CLONES:
				DebugPrintf("C  clones (%d allocd)", (*(CloneTable *)mobj).entries_used);
				break;

			case SEG_TYPE_LOCALS:
				DebugPrintf("V  locals %03d", (*(LocalVariables *)mobj).script_id);
				break;

			case SEG_TYPE_STACK:
				DebugPrintf("D  data stack (%d)", (*(DataStack *)mobj)._capacity);
				break;

			case SEG_TYPE_LISTS:
				DebugPrintf("L  lists (%d)", (*(ListTable *)mobj).entries_used);
				break;

			case SEG_TYPE_NODES:
				DebugPrintf("N  nodes (%d)", (*(NodeTable *)mobj).entries_used);
				break;

			case SEG_TYPE_HUNK:
				DebugPrintf("H  hunk (%d)", (*(HunkTable *)mobj).entries_used);
				break;

			case SEG_TYPE_DYNMEM:
				DebugPrintf("M  dynmem: %d bytes", (*(DynMem *)mobj)._size);
				break;

#ifdef ENABLE_SCI32
			case SEG_TYPE_ARRAY:
				DebugPrintf("A  SCI32 arrays (%d)", (*(ArrayTable *)mobj).entries_used);
				break;

			case SEG_TYPE_STRING:
				DebugPrintf("T  SCI32 strings (%d)", (*(StringTable *)mobj).entries_used);
				break;
#endif

			default:
				DebugPrintf("I  Invalid (type = %x)", mobj->getType());
				break;
			}

			DebugPrintf("  \n");
		}
	}
	DebugPrintf("\n");

	return true;
}

bool Console::segmentInfo(int nr) {
	DebugPrintf("[%04x] ", nr);

	if ((nr < 0) || ((uint)nr >= _engine->_gamestate->_segMan->_heap.size()) || !_engine->_gamestate->_segMan->_heap[nr])
		return false;

	SegmentObj *mobj = _engine->_gamestate->_segMan->_heap[nr];

	switch (mobj->getType()) {

	case SEG_TYPE_SCRIPT: {
		Script *scr = (Script *)mobj;
		DebugPrintf("script.%03d locked by %d, bufsize=%d (%x)\n", scr->getScriptNumber(), scr->getLockers(), (uint)scr->getBufSize(), (uint)scr->getBufSize());
		if (scr->getExportTable())
			DebugPrintf("  Exports: %4d at %d\n", scr->getExportsNr(), (int)(((const byte *)scr->getExportTable()) - ((const byte *)scr->getBuf())));
		else
			DebugPrintf("  Exports: none\n");

		DebugPrintf("  Synonyms: %4d\n", scr->getSynonymsNr());

		if (scr->getLocalsCount() > 0)
			DebugPrintf("  Locals : %4d in segment 0x%x\n", scr->getLocalsCount(), scr->getLocalsSegment());
		else
			DebugPrintf("  Locals : none\n");

		ObjMap objects = scr->getObjectMap();
		DebugPrintf("  Objects: %4d\n", objects.size());

		ObjMap::iterator it;
		const ObjMap::iterator end = objects.end();
		for (it = objects.begin(); it != end; ++it) {
			DebugPrintf("    ");
			// Object header
			const Object *obj = _engine->_gamestate->_segMan->getObject(it->_value.getPos());
			if (obj)
				DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(it->_value.getPos()),
							_engine->_gamestate->_segMan->getObjectName(it->_value.getPos()),
							obj->getVarCount(), obj->getMethodCount());
		}
	}
	break;

	case SEG_TYPE_LOCALS: {
		LocalVariables *locals = (LocalVariables *)mobj;
		DebugPrintf("locals for script.%03d\n", locals->script_id);
		DebugPrintf("  %d (0x%x) locals\n", locals->_locals.size(), locals->_locals.size());
	}
	break;

	case SEG_TYPE_STACK: {
		DataStack *stack = (DataStack *)mobj;
		DebugPrintf("stack\n");
		DebugPrintf("  %d (0x%x) entries\n", stack->_capacity, stack->_capacity);
	}
	break;

	case SEG_TYPE_CLONES: {
		CloneTable *ct = (CloneTable *)mobj;

		DebugPrintf("clones\n");

		for (uint i = 0; i < ct->_table.size(); i++)
			if (ct->isValidEntry(i)) {
				reg_t objpos = make_reg(nr, i);
				DebugPrintf("  [%04x] %s; copy of ", i, _engine->_gamestate->_segMan->getObjectName(objpos));
				// Object header
				const Object *obj = _engine->_gamestate->_segMan->getObject(ct->_table[i].getPos());
				if (obj)
					DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(ct->_table[i].getPos()),
								_engine->_gamestate->_segMan->getObjectName(ct->_table[i].getPos()),
								obj->getVarCount(), obj->getMethodCount());
			}
	}
	break;

	case SEG_TYPE_LISTS: {
		ListTable *lt = (ListTable *)mobj;

		DebugPrintf("lists\n");
		for (uint i = 0; i < lt->_table.size(); i++)
			if (lt->isValidEntry(i)) {
				DebugPrintf("  [%04x]: ", i);
				printList(&(lt->_table[i]));
			}
	}
	break;

	case SEG_TYPE_NODES: {
		DebugPrintf("nodes (total %d)\n", (*(NodeTable *)mobj).entries_used);
		break;
	}

	case SEG_TYPE_HUNK: {
		HunkTable *ht = (HunkTable *)mobj;

		DebugPrintf("hunk  (total %d)\n", ht->entries_used);
		for (uint i = 0; i < ht->_table.size(); i++)
			if (ht->isValidEntry(i)) {
				DebugPrintf("    [%04x] %d bytes at %p, type=%s\n",
				          i, ht->_table[i].size, ht->_table[i].mem, ht->_table[i].type);
			}
	}
	break;

	case SEG_TYPE_DYNMEM: {
		DebugPrintf("dynmem (%s): %d bytes\n",
		          (*(DynMem *)mobj)._description.c_str(), (*(DynMem *)mobj)._size);

		Common::hexdump((*(DynMem *)mobj)._buf, (*(DynMem *)mobj)._size, 16, 0);
	}
	break;

#ifdef ENABLE_SCI32
	case SEG_TYPE_STRING:
		DebugPrintf("SCI32 strings\n");
		break;
	case SEG_TYPE_ARRAY:
		DebugPrintf("SCI32 arrays\n");
		break;
#endif

	default :
		DebugPrintf("Invalid type %d\n", mobj->getType());
		break;
	}

	DebugPrintf("\n");
	return true;
}

bool Console::cmdSegmentInfo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Provides information on the specified segment(s)\n");
		DebugPrintf("Usage: %s <segment number>\n", argv[0]);
		DebugPrintf("<segment number> can be a number, which shows the information of the segment with\n");
		DebugPrintf("the specified number, or \"all\" to show information on all active segments\n");
		return true;
	}

	if (!scumm_stricmp(argv[1], "all")) {
		for (uint i = 0; i < _engine->_gamestate->_segMan->_heap.size(); i++)
			segmentInfo(i);
	} else {
		int segmentNr;
		if (!parseInteger(argv[1], segmentNr))
			return true;
		if (!segmentInfo(segmentNr))
			DebugPrintf("Segment %04xh does not exist\n", segmentNr);
	}

	return true;
}


bool Console::cmdKillSegment(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Deletes the specified segment\n");
		DebugPrintf("Usage: %s <segment number>\n", argv[0]);
		return true;
	}
	int segmentNumber;
	if (!parseInteger(argv[1], segmentNumber))
		return true;
	_engine->_gamestate->_segMan->getScript(segmentNumber)->setLockers(0);

	return true;
}

bool Console::cmdShowMap(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Switches to one of the following screen maps\n");
		DebugPrintf("Usage: %s <screen map>\n", argv[0]);
		DebugPrintf("Screen maps:\n");
		DebugPrintf("- 0: visual map\n");
		DebugPrintf("- 1: priority map\n");
		DebugPrintf("- 2: control map\n");
		DebugPrintf("- 3: display screen\n");
		return true;
	}

	int map = atoi(argv[1]);

	switch (map) {
	case 0:
	case 1:
	case 2:
	case 3:
		_engine->_gfxScreen->debugShowMap(map);
		break;

	default:
		DebugPrintf("Map %d is not available.\n", map);
		return true;
	}
	return Cmd_Exit(0, 0);
}

bool Console::cmdSongLib(int argc, const char **argv) {
	DebugPrintf("Song library:\n");
	g_sci->_soundCmd->printPlayList(this);

	return true;
}

bool Console::cmdSongInfo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Shows information about a given song in the playlist\n");
		DebugPrintf("Usage: %s <song object>\n", argv[0]);
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	g_sci->_soundCmd->printSongInfo(addr, this);

	return true;
}

bool Console::cmdStartSound(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Adds the requested sound resource to the playlist, and starts playing it\n");
		DebugPrintf("Usage: %s <sound resource id>\n", argv[0]);
		return true;
	}

	int16 number = atoi(argv[1]);

	if (!_engine->getResMan()->testResource(ResourceId(kResourceTypeSound, number))) {
		DebugPrintf("Unable to load this sound resource, most probably it has an equivalent audio resource (SCI1.1)\n");
		return true;
	}

	g_sci->_soundCmd->startNewSound(number);
	return Cmd_Exit(0, 0);
}

bool Console::cmdToggleSound(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Plays or stops the specified sound in the playlist\n");
		DebugPrintf("Usage: %s <address> <state>\n", argv[0]);
		DebugPrintf("Where:\n");
		DebugPrintf("- <address> is the address of the sound to play or stop.\n");
		DebugPrintf("- <state> is the new state (play or stop).\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t id;

	if (parse_reg_t(_engine->_gamestate, argv[1], &id, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	Common::String newState = argv[2];
	newState.toLowercase();

	if (newState == "play")
		g_sci->_soundCmd->processPlaySound(id);
	else if (newState == "stop")
		g_sci->_soundCmd->processStopSound(id, false);
	else
		DebugPrintf("New state can either be 'play' or 'stop'");

	return true;
}

bool Console::cmdStopAllSounds(int argc, const char **argv) {
	g_sci->_soundCmd->stopAllSounds();

	DebugPrintf("All sounds have been stopped\n");
	return true;
}

bool Console::cmdIsSample(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Tests whether a given sound resource is a PCM sample, \n");
		DebugPrintf("and displays information on it if it is.\n");
		DebugPrintf("Usage: %s <sample id>\n", argv[0]);
		return true;
	}

	int16 number = atoi(argv[1]);

	if (!_engine->getResMan()->testResource(ResourceId(kResourceTypeSound, number))) {
		DebugPrintf("Unable to load this sound resource, most probably it has an equivalent audio resource (SCI1.1)\n");
		return true;
	}

	SoundResource *soundRes = new SoundResource(number, _engine->getResMan(), _engine->_features->detectDoSoundType());

	if (!soundRes) {
		DebugPrintf("Not a sound resource!\n");
		return true;
	}

	SoundResource::Track *track = soundRes->getDigitalTrack();
	if (!track || track->digitalChannelNr == -1) {
		DebugPrintf("Valid song, but not a sample.\n");
		delete soundRes;
		return true;
	}

	DebugPrintf("Sample size: %d, sample rate: %d, channels: %d, digital channel number: %d\n",
			track->digitalSampleSize, track->digitalSampleRate, track->channelCount, track->digitalChannelNr);

	return true;
}

bool Console::cmdGCInvoke(int argc, const char **argv) {
	DebugPrintf("Performing garbage collection...\n");
	run_gc(_engine->_gamestate);
	return true;
}

bool Console::cmdGCObjects(int argc, const char **argv) {
	AddrSet *use_map = findAllActiveReferences(_engine->_gamestate);

	DebugPrintf("Reachable object references (normalised):\n");
	for (AddrSet::iterator i = use_map->begin(); i != use_map->end(); ++i) {
		DebugPrintf(" - %04x:%04x\n", PRINT_REG(i->_key));
	}

	delete use_map;

	return true;
}

bool Console::cmdGCShowReachable(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints all addresses directly reachable from the memory object specified as parameter.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegmentObj(addr.getSegment());
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.getSegment());
		return 1;
	}

	DebugPrintf("Reachable from %04x:%04x:\n", PRINT_REG(addr));
	const Common::Array<reg_t> tmp = mobj->listAllOutgoingReferences(addr);
	for (Common::Array<reg_t>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
		if (it->getSegment())
			g_sci->getSciDebugger()->DebugPrintf("  %04x:%04x\n", PRINT_REG(*it));

	return true;
}

bool Console::cmdGCShowFreeable(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints all addresses freeable in the segment associated with the\n");
		DebugPrintf("given address (offset is ignored).\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegmentObj(addr.getSegment());
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.getSegment());
		return true;
	}

	DebugPrintf("Freeable in segment %04x:\n", addr.getSegment());
	const Common::Array<reg_t> tmp = mobj->listAllDeallocatable(addr.getSegment());
	for (Common::Array<reg_t>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
		if (it->getSegment())
			g_sci->getSciDebugger()->DebugPrintf("  %04x:%04x\n", PRINT_REG(*it));

	return true;
}

bool Console::cmdGCNormalize(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Prints the \"normal\" address of a given address,\n");
		DebugPrintf("i.e. the address we would free in order to free\n");
		DebugPrintf("the object associated with the original address.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegmentObj(addr.getSegment());
	if (!mobj) {
		DebugPrintf("Unknown segment : %x\n", addr.getSegment());
		return true;
	}

	addr = mobj->findCanonicAddress(_engine->_gamestate->_segMan, addr);
	DebugPrintf(" %04x:%04x\n", PRINT_REG(addr));

	return true;
}

bool Console::cmdVMVarlist(int argc, const char **argv) {
	EngineState *s = _engine->_gamestate;
	const char *varnames[] = {"global", "local", "temp", "param"};

	DebugPrintf("Addresses of variables in the VM:\n");

	for (int i = 0; i < 4; i++) {
		DebugPrintf("%s vars at %04x:%04x ", varnames[i], PRINT_REG(make_reg(s->variablesSegment[i], s->variables[i] - s->variablesBase[i])));
		if (s->variablesMax)
			DebugPrintf("  total %d", s->variablesMax[i]);
		DebugPrintf("\n");
	}

	return true;
}

bool Console::cmdVMVars(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Displays or changes variables in the VM\n");
		DebugPrintf("Usage: %s <type> <varnum> [<value>]\n", argv[0]);
		DebugPrintf("First parameter is either g(lobal), l(ocal), t(emp), p(aram) or a(cc).\n");
		DebugPrintf("Second parameter is the var number (not specified on acc)\n");
		DebugPrintf("Third parameter (if specified) is the value to set the variable to, in address form\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	EngineState *s = _engine->_gamestate;
	const char *varNames[] = {"global", "local", "temp", "param", "acc"};
	const char *varAbbrev = "gltpa";
	const char *varType_pre = strchr(varAbbrev, *argv[1]);
	int varType;
	int varIndex = 0;
	reg_t *curValue = NULL;
	const char *setValue = NULL;

	if (!varType_pre) {
		DebugPrintf("Invalid variable type '%c'\n", *argv[1]);
		return true;
	}

	varType = varType_pre - varAbbrev;

	switch (varType) {
	case 0:
	case 1:
	case 2:
	case 3: {
		// for global, local, temp and param, we need an index
		if (argc < 3) {
			DebugPrintf("Variable number must be specified for requested type\n");
			return true;
		}
		if (argc > 4) {
			DebugPrintf("Too many arguments\n");
			return true;
		}

		if (!parseInteger(argv[2], varIndex))
			return true;

		if (varIndex < 0) {
			DebugPrintf("Variable number may not be negative\n");
			return true;
		}

		if ((s->variablesMax) && (s->variablesMax[varType] <= varIndex)) {
			DebugPrintf("Maximum variable number for this type is %d (0x%x)\n", s->variablesMax[varType], s->variablesMax[varType]);
			return true;
		}
		curValue = &s->variables[varType][varIndex];
		if (argc == 4)
			setValue = argv[3];
		break;
	}

	case 4:
		// acc
		if (argc > 3) {
			DebugPrintf("Too many arguments\n");
			return true;
		}
		curValue = &s->r_acc;
		if (argc == 3)
			setValue = argv[2];
		break;

	default:
		break;
	}

	if (!setValue) {
		if (varType == 4)
			DebugPrintf("%s == %04x:%04x", varNames[varType], PRINT_REG(*curValue));
		else
			DebugPrintf("%s var %d == %04x:%04x", varNames[varType], varIndex, PRINT_REG(*curValue));
		printBasicVarInfo(*curValue);
		DebugPrintf("\n");
	} else {
		if (parse_reg_t(s, setValue, curValue, true)) {
			DebugPrintf("Invalid value/address passed.\n");
			DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
			DebugPrintf("Or pass a decimal or hexadecimal value directly (e.g. 12, 1Ah)\n");
			return true;
		}
	}
	return true;
}

bool Console::cmdStack(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Lists the specified number of stack elements.\n");
		DebugPrintf("Usage: %s <elements>\n", argv[0]);
		return true;
	}

	if (_engine->_gamestate->_executionStack.empty()) {
		DebugPrintf("No exec stack!");
		return true;
	}

	const ExecStack &xs = _engine->_gamestate->_executionStack.back();
	int nr = atoi(argv[1]);

	for (int i = nr; i > 0; i--) {
		if ((xs.sp - xs.fp - i) == 0)
			DebugPrintf("-- temp variables --\n");
		if (xs.sp - i >= _engine->_gamestate->stack_base)
			DebugPrintf("ST:%04x = %04x:%04x\n", (unsigned)(xs.sp - i - _engine->_gamestate->stack_base), PRINT_REG(xs.sp[-i]));
	}

	return true;
}

bool Console::cmdValueType(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Determines the type of a value.\n");
		DebugPrintf("The type can be one of the following:\n");
		DebugPrintf("Invalid, list, object, reference or arithmetic\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t val;

	if (parse_reg_t(_engine->_gamestate, argv[1], &val, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	int t = g_sci->getKernel()->findRegType(val);

	switch (t) {
	case SIG_TYPE_LIST:
		DebugPrintf("List");
		break;
	case SIG_TYPE_OBJECT:
		DebugPrintf("Object");
		break;
	case SIG_TYPE_REFERENCE:
		DebugPrintf("Reference");
		break;
	case SIG_TYPE_INTEGER:
		DebugPrintf("Integer");
	case SIG_TYPE_INTEGER | SIG_TYPE_NULL:
		DebugPrintf("Null");
		break;
	default:
		DebugPrintf("Erroneous unknown type 0x%02x (%d decimal)\n", t, t);
	}

	return true;
}

bool Console::cmdViewListNode(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Examines the list node at the given address.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	printNode(addr);
	return true;
}

bool Console::cmdViewReference(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Examines an arbitrary reference.\n");
		DebugPrintf("Usage: %s <start address> [<end address>]\n", argv[0]);
		DebugPrintf("Where <start address> is the starting address to examine\n");
		DebugPrintf("<end address>, if provided, is the address where examining ends at\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t reg = NULL_REG;
	reg_t reg_end = NULL_REG;

	if (parse_reg_t(_engine->_gamestate, argv[1], &reg, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	if (argc > 2) {
		if (parse_reg_t(_engine->_gamestate, argv[2], &reg_end, false)) {
			DebugPrintf("Invalid address passed.\n");
			DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	int type_mask = g_sci->getKernel()->findRegType(reg);
	int filter;
	int found = 0;

	DebugPrintf("%04x:%04x is of type 0x%x: ", PRINT_REG(reg), type_mask);

	if (reg.getSegment() == 0 && reg.getOffset() == 0) {
		DebugPrintf("Null.\n");
		return true;
	}

	if (reg_end.getSegment() != reg.getSegment() && reg_end != NULL_REG) {
		DebugPrintf("Ending segment different from starting segment. Assuming no bound on dump.\n");
		reg_end = NULL_REG;
	}

	for (filter = 1; filter < 0xf000; filter <<= 1) {
		int type = type_mask & filter;

		if (found && type) {
			DebugPrintf("--- Alternatively, it could be a ");
		}


		switch (type) {
		case 0:
			break;
		case SIG_TYPE_LIST: {
			List *list = _engine->_gamestate->_segMan->lookupList(reg);

			DebugPrintf("list\n");

			if (list)
				printList(list);
			else
				DebugPrintf("Invalid list.\n");
		}
			break;
		case SIG_TYPE_NODE:
			DebugPrintf("list node\n");
			printNode(reg);
			break;
		case SIG_TYPE_OBJECT:
			DebugPrintf("object\n");
			printObject(reg);
			break;
		case SIG_TYPE_REFERENCE: {
			switch (_engine->_gamestate->_segMan->getSegmentType(reg.getSegment())) {
#ifdef ENABLE_SCI32
				case SEG_TYPE_STRING: {
					DebugPrintf("SCI32 string\n");
					const SciString *str = _engine->_gamestate->_segMan->lookupString(reg);
					Common::hexdump((const byte *) str->getRawData(), str->getSize(), 16, 0);
					break;
				}
				case SEG_TYPE_ARRAY: {
					DebugPrintf("SCI32 array:\n");
					const SciArray<reg_t> *array = _engine->_gamestate->_segMan->lookupArray(reg);
					hexDumpReg(array->getRawData(), array->getSize(), 4, 0, true);
					break;
				}
#endif
				default: {
					const SegmentRef block = _engine->_gamestate->_segMan->dereference(reg);
					uint16 size = block.maxSize;

					DebugPrintf("raw data\n");

					if (reg_end.getSegment() != 0 && (size < reg_end.getOffset() - reg.getOffset())) {
						DebugPrintf("Block end out of bounds (size %d). Resetting.\n", size);
						reg_end = NULL_REG;
					}

					if (reg_end.getSegment() != 0 && (size >= reg_end.getOffset() - reg.getOffset()))
						size = reg_end.getOffset() - reg.getOffset();

					if (reg_end.getSegment() != 0)
						DebugPrintf("Block size less than or equal to %d\n", size);

					if (block.isRaw)
						Common::hexdump(block.raw, size, 16, 0);
					else
						hexDumpReg(block.reg, size / 2, 4, 0);
				}
			}
			break;
		}
		case SIG_TYPE_INTEGER:
			DebugPrintf("arithmetic value\n  %d (%04x)\n", (int16) reg.getOffset(), reg.getOffset());
			break;
		default:
			DebugPrintf("unknown type %d.\n", type);
		}

		if (type) {
			DebugPrintf("\n");
			found = 1;
		}
	}

	return true;
}

bool Console::cmdViewObject(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Examines the object at the given address.\n");
		DebugPrintf("Usage: %s <address>\n", argv[0]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	reg_t addr;

	if (parse_reg_t(_engine->_gamestate, argv[1], &addr, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	DebugPrintf("Information on the object at the given address:\n");
	printObject(addr);

	return true;
}

bool Console::cmdViewActiveObject(int argc, const char **argv) {
	DebugPrintf("Information on the currently active object or class:\n");
	printObject(_engine->_gamestate->xs->objp);

	return true;
}

bool Console::cmdViewAccumulatorObject(int argc, const char **argv) {
	DebugPrintf("Information on the currently active object or class at the address indexed by the accumulator:\n");
	printObject(_engine->_gamestate->r_acc);

	return true;
}

bool Console::cmdScriptSteps(int argc, const char **argv) {
	DebugPrintf("Number of executed SCI operations: %d\n", _engine->_gamestate->scriptStepCounter);
	return true;
}

bool Console::cmdBacktrace(int argc, const char **argv) {
	DebugPrintf("Call stack (current base: 0x%x):\n", _engine->_gamestate->executionStackBase);
	Common::List<ExecStack>::const_iterator iter;
	uint i = 0;

	for (iter = _engine->_gamestate->_executionStack.begin();
	     iter != _engine->_gamestate->_executionStack.end(); ++iter, ++i) {
		const ExecStack &call = *iter;
		const char *objname = _engine->_gamestate->_segMan->getObjectName(call.sendp);
		int paramc, totalparamc;

		switch (call.type) {
		case EXEC_STACK_TYPE_CALL: // Normal function
			if (call.type == EXEC_STACK_TYPE_CALL)
			DebugPrintf(" %x: script %d - ", i, (*(Script *)_engine->_gamestate->_segMan->_heap[call.addr.pc.getSegment()]).getScriptNumber());
			if (call.debugSelector != -1) {
				DebugPrintf("%s::%s(", objname, _engine->getKernel()->getSelectorName(call.debugSelector).c_str());
			} else if (call.debugExportId != -1) {
				DebugPrintf("export %d (", call.debugExportId);
			} else if (call.debugLocalCallOffset != -1) {
				DebugPrintf("call %x (", call.debugLocalCallOffset);
			}
			break;

		case EXEC_STACK_TYPE_KERNEL: // Kernel function
			DebugPrintf(" %x:[%x]  k%s(", i, call.debugOrigin, _engine->getKernel()->getKernelName(call.debugSelector).c_str());
			break;

		case EXEC_STACK_TYPE_VARSELECTOR:
			DebugPrintf(" %x:[%x] vs%s %s::%s (", i, call.debugOrigin, (call.argc) ? "write" : "read",
			          objname, _engine->getKernel()->getSelectorName(call.debugSelector).c_str());
			break;
		}

		totalparamc = call.argc;

		if (totalparamc > 16)
			totalparamc = 16;

		for (paramc = 1; paramc <= totalparamc; paramc++) {
			DebugPrintf("%04x:%04x", PRINT_REG(call.variables_argp[paramc]));

			if (paramc < call.argc)
				DebugPrintf(", ");
		}

		if (call.argc > 16)
			DebugPrintf("...");

		DebugPrintf(")\n     ");
		if (call.debugOrigin != -1)
			DebugPrintf("by %x ", call.debugOrigin);
		DebugPrintf("obj@%04x:%04x", PRINT_REG(call.objp));
		if (call.type == EXEC_STACK_TYPE_CALL) {
			DebugPrintf(" pc=%04x:%04x", PRINT_REG(call.addr.pc));
			if (call.sp == CALL_SP_CARRY)
				DebugPrintf(" sp,fp:carry");
			else {
				DebugPrintf(" sp=ST:%04x", (unsigned)(call.sp - _engine->_gamestate->stack_base));
				DebugPrintf(" fp=ST:%04x", (unsigned)(call.fp - _engine->_gamestate->stack_base));
			}
		} else
			DebugPrintf(" pc:none");

		DebugPrintf(" argp:ST:%04x", (unsigned)(call.variables_argp - _engine->_gamestate->stack_base));
		DebugPrintf("\n");
	}

	return true;
}

bool Console::cmdTrace(int argc, const char **argv) {
	if (argc == 2 && atoi(argv[1]) > 0)
		_debugState.runningStep = atoi(argv[1]) - 1;
	_debugState.debugging = true;

	return Cmd_Exit(0, 0);
}

bool Console::cmdStepOver(int argc, const char **argv) {
	_debugState.seeking = kDebugSeekStepOver;
	_debugState.seekLevel = _engine->_gamestate->_executionStack.size();
	_debugState.debugging = true;

	return Cmd_Exit(0, 0);
}

bool Console::cmdStepEvent(int argc, const char **argv) {
	_debugState.stopOnEvent = true;
	_debugState.debugging = true;

	return Cmd_Exit(0, 0);
}

bool Console::cmdStepRet(int argc, const char **argv) {
	_debugState.seeking = kDebugSeekLevelRet;
	_debugState.seekLevel = _engine->_gamestate->_executionStack.size() - 1;
	_debugState.debugging = true;

	return Cmd_Exit(0, 0);
}

bool Console::cmdStepGlobal(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Steps until the global variable with the specified index is modified.\n");
		DebugPrintf("Usage: %s <global variable index>\n", argv[0]);
		return true;
	}

	_debugState.seeking = kDebugSeekGlobal;
	_debugState.seekSpecial = atoi(argv[1]);
	_debugState.debugging = true;

	return Cmd_Exit(0, 0);
}

bool Console::cmdStepCallk(int argc, const char **argv) {
	int callk_index;
	char *endptr;

	if (argc == 2) {
		/* Try to convert the parameter to a number. If the conversion stops
		   before end of string, assume that the parameter is a function name
		   and scan the function table to find out the index. */
		callk_index = strtoul(argv[1], &endptr, 0);
		if (*endptr != '\0') {
			callk_index = -1;
			for (uint i = 0; i < _engine->getKernel()->getKernelNamesSize(); i++)
				if (argv[1] == _engine->getKernel()->getKernelName(i)) {
					callk_index = i;
					break;
				}

			if (callk_index == -1) {
				DebugPrintf("Unknown kernel function '%s'\n", argv[1]);
				return true;
			}
		}

		_debugState.seeking = kDebugSeekSpecialCallk;
		_debugState.seekSpecial = callk_index;
	} else {
		_debugState.seeking = kDebugSeekCallk;
	}
	_debugState.debugging = true;

	return Cmd_Exit(0, 0);
}

bool Console::cmdDisassemble(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("Disassembles a method by name.\n");
		DebugPrintf("Usage: %s <object> <method> <options>\n", argv[0]);
		DebugPrintf("Valid options are:\n");
		DebugPrintf(" bwt  : Print byte/word tag\n");
		DebugPrintf(" bc   : Print bytecode\n");
		return true;
	}

	reg_t objAddr = NULL_REG;
	bool printBytecode = false;
	bool printBWTag = false;

	if (parse_reg_t(_engine->_gamestate, argv[1], &objAddr, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	const Object *obj = _engine->_gamestate->_segMan->getObject(objAddr);
	int selectorId = _engine->getKernel()->findSelector(argv[2]);
	reg_t addr = NULL_REG;

	if (!obj) {
		DebugPrintf("Not an object.\n");
		return true;
	}

	if (selectorId < 0) {
		DebugPrintf("Not a valid selector name.\n");
		return true;
	}

	if (lookupSelector(_engine->_gamestate->_segMan, objAddr, selectorId, NULL, &addr) != kSelectorMethod) {
		DebugPrintf("Not a method.\n");
		return true;
	}

	for (int i = 3; i < argc; i++) {
		if (!scumm_stricmp(argv[i], "bwt"))
			printBWTag = true;
		else if (!scumm_stricmp(argv[i], "bc"))
			printBytecode = true;
	}

	reg_t farthestTarget = addr;
	do {
		reg_t prevAddr = addr;
		reg_t jumpTarget;
		if (isJumpOpcode(_engine->_gamestate, addr, jumpTarget)) {
			if (jumpTarget > farthestTarget)
				farthestTarget = jumpTarget;
		}
		// TODO: Use a true 32-bit reg_t for the position (addr)
		addr = disassemble(_engine->_gamestate, make_reg32(addr.getSegment(), addr.getOffset()), printBWTag, printBytecode);
		if (addr.isNull() && prevAddr < farthestTarget)
			addr = prevAddr + 1; // skip past the ret
	} while (addr.getOffset() > 0);

	return true;
}

bool Console::cmdDisassembleAddress(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Disassembles one or more commands.\n");
		DebugPrintf("Usage: %s [startaddr] <options>\n", argv[0]);
		DebugPrintf("Valid options are:\n");
		DebugPrintf(" bwt  : Print byte/word tag\n");
		DebugPrintf(" c<x> : Disassemble <x> bytes\n");
		DebugPrintf(" bc   : Print bytecode\n");
		return true;
	}

	reg_t vpc = NULL_REG;
	uint opCount = 1;
	bool printBWTag = false;
	bool printBytes = false;
	uint16 size;

	if (parse_reg_t(_engine->_gamestate, argv[1], &vpc, false)) {
		DebugPrintf("Invalid address passed.\n");
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	SegmentRef ref = _engine->_gamestate->_segMan->dereference(vpc);
	size = ref.maxSize + vpc.getOffset(); // total segment size

	for (int i = 2; i < argc; i++) {
		if (!scumm_stricmp(argv[i], "bwt"))
			printBWTag = true;
		else if (!scumm_stricmp(argv[i], "bc"))
			printBytes = true;
		else if (toupper(argv[i][0]) == 'C')
			opCount = atoi(argv[i] + 1);
		else {
			DebugPrintf("Invalid option '%s'\n", argv[i]);
			return true;
		}
	}

	do {
		// TODO: Use a true 32-bit reg_t for the position (vpc)
		vpc = disassemble(_engine->_gamestate, make_reg32(vpc.getSegment(), vpc.getOffset()), printBWTag, printBytes);
	} while ((vpc.getOffset() > 0) && (vpc.getOffset() + 6 < size) && (--opCount));

	return true;
}

void Console::printKernelCallsFound(int kernelFuncNum, bool showFoundScripts) {
	Common::List<ResourceId> resources = _engine->getResMan()->listResources(kResourceTypeScript);
	Common::sort(resources.begin(), resources.end());

	if (showFoundScripts)
		DebugPrintf("%d scripts found, dissassembling...\n", resources.size());

	int scriptSegment;
	Script *script;
	// Create a custom segment manager here, so that the game's segment
	// manager won't be affected by loading and unloading scripts here.
	SegManager *customSegMan = new SegManager(_engine->getResMan());

	Common::List<ResourceId>::iterator itr;
	for (itr = resources.begin(); itr != resources.end(); ++itr) {
		// Ignore specific leftover scripts, which require other non-existing scripts
		if ((_engine->getGameId() == GID_HOYLE3         && itr->getNumber() == 995) ||
		    (_engine->getGameId() == GID_KQ5            && itr->getNumber() == 980) ||
			(_engine->getGameId() == GID_KQ7            && itr->getNumber() == 111) ||
			(_engine->getGameId() == GID_MOTHERGOOSE256 && itr->getNumber() == 980) ||
		    (_engine->getGameId() == GID_SLATER         && itr->getNumber() == 947)) {
			continue;
		}

		// Load script
		scriptSegment = customSegMan->instantiateScript(itr->getNumber());
		script = customSegMan->getScript(scriptSegment);

		// Iterate through all the script's objects
		ObjMap objects = script->getObjectMap();
		ObjMap::iterator it;
		const ObjMap::iterator end = objects.end();
		for (it = objects.begin(); it != end; ++it) {
			const Object *obj = customSegMan->getObject(it->_value.getPos());
			const char *objName = customSegMan->getObjectName(it->_value.getPos());

			// Now dissassemble each method of the script object
			for (uint16 i = 0; i < obj->getMethodCount(); i++) {
				reg_t fptr = obj->getFunction(i);
				uint16 offset = fptr.getOffset();
				int16 opparams[4];
				byte extOpcode;
				byte opcode;
				uint16 maxJmpOffset = 0;

				while (true) {
					offset += readPMachineInstruction(script->getBuf(offset), extOpcode, opparams);
					opcode = extOpcode >> 1;

					if (opcode == op_callk) {
						uint16 kFuncNum = opparams[0];
						uint16 argc2 = opparams[1];

						if (kFuncNum == kernelFuncNum) {
							DebugPrintf("Called from script %d, object %s, method %s(%d) with %d bytes for arguments\n",
								itr->getNumber(), objName,
								_engine->getKernel()->getSelectorName(obj->getFuncSelector(i)).c_str(), i, argc2);
						}
					}

					// Monitor all jump opcodes (bt, bnt and jmp), so that if
					// there is a jump after a ret, we don't stop processing
					if (opcode == op_bt || opcode == op_bnt || opcode == op_jmp) {
						uint16 curJmpOffset = offset + (uint16)opparams[0];
						// QFG2 has invalid jumps outside the script buffer in script 260
						if (curJmpOffset > maxJmpOffset && curJmpOffset < script->getScriptSize())
							maxJmpOffset = curJmpOffset;
					}

					// Check for end of function/script
					if (offset >= script->getBufSize())
						break;
					if (opcode == op_ret && offset >= maxJmpOffset)
						break;
				}	// while (true)
			}	// for (uint16 i = 0; i < obj->getMethodCount(); i++)
		}	// for (it = script->_objects.begin(); it != end; ++it)

		customSegMan->uninstantiateScript(itr->getNumber());
	}

	delete customSegMan;
}

bool Console::cmdFindKernelFunctionCall(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Finds the scripts and methods that call a specific kernel function.\n");
		DebugPrintf("Usage: %s <kernel function>\n", argv[0]);
		DebugPrintf("Example: %s Display\n", argv[0]);
		DebugPrintf("Special usage:\n");
		DebugPrintf("%s Dummy - find all calls to actual dummy functions "
					"(mapped to kDummy, and dummy in the kernel table). "
					"There shouldn't be calls to these (apart from a known "
					"one in Shivers)\n", argv[0]);
		DebugPrintf("%s Unused - find all calls to unused functions (mapped to "
					"kDummy - i.e. mapped in SSCI but dummy in ScummVM, thus "
					"they'll error out when called). Only debug scripts should "
					"be calling these\n", argv[0]);
		DebugPrintf("%s Unmapped - find all calls to currently unmapped or "
					"unimplemented functions (mapped to kStub/kStubNull)\n", argv[0]);
		return true;
	}

	Kernel *kernel = _engine->getKernel();
	Common::String funcName(argv[1]);

	if (funcName != "Dummy" && funcName != "Unused" && funcName != "Unmapped") {
		// Find the number of the kernel function call
		int kernelFuncNum = kernel->findKernelFuncPos(argv[1]);

		if (kernelFuncNum < 0) {
			DebugPrintf("Invalid kernel function requested\n");
			return true;
		}

		printKernelCallsFound(kernelFuncNum, true);
	} else if (funcName == "Dummy") {
		// Find all actual dummy kernel functions (mapped to kDummy, and dummy
		// in the kernel table)
		for (uint i = 0; i < kernel->_kernelFuncs.size(); i++) {
			if (kernel->_kernelFuncs[i].function == &kDummy && kernel->getKernelName(i) == "Dummy") {
				DebugPrintf("Searching for kernel function %d (%s)...\n", i, kernel->getKernelName(i).c_str());
				printKernelCallsFound(i, false);
			}
		}
	} else if (funcName == "Unused") {
		// Find all actual dummy kernel functions (mapped to kDummy - i.e.
		// mapped in SSCI but dummy in ScummVM, thus they'll error out when
		// called)
		for (uint i = 0; i < kernel->_kernelFuncs.size(); i++) {
			if (kernel->_kernelFuncs[i].function == &kDummy && kernel->getKernelName(i) != "Dummy") {
				DebugPrintf("Searching for kernel function %d (%s)...\n", i, kernel->getKernelName(i).c_str());
				printKernelCallsFound(i, false);
			}
		}
	} else if (funcName == "Unmapped") {
		// Find all unmapped kernel functions (mapped to kStub/kStubNull)
		for (uint i = 0; i < kernel->_kernelFuncs.size(); i++) {
			if (kernel->_kernelFuncs[i].function == &kStub ||
				kernel->_kernelFuncs[i].function == &kStubNull) {
				DebugPrintf("Searching for kernel function %d (%s)...\n", i, kernel->getKernelName(i).c_str());
				printKernelCallsFound(i, false);
			}
		}
	}

	return true;
}

bool Console::cmdSend(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("Sends a message to an object.\n");
		DebugPrintf("Usage: %s <object> <selector name> <param1> <param2> ... <paramn>\n", argv[0]);
		DebugPrintf("Example: %s ?fooScript cue\n", argv[0]);
		return true;
	}

	reg_t object;

	if (parse_reg_t(_engine->_gamestate, argv[1], &object, false)) {
		DebugPrintf("Invalid address \"%s\" passed.\n", argv[1]);
		DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
		return true;
	}

	const char *selectorName = argv[2];
	int selectorId = _engine->getKernel()->findSelector(selectorName);

	if (selectorId < 0) {
		DebugPrintf("Unknown selector: \"%s\"\n", selectorName);
		return true;
	}

	const Object *o = _engine->_gamestate->_segMan->getObject(object);
	if (o == NULL) {
		DebugPrintf("Address \"%04x:%04x\" is not an object\n", PRINT_REG(object));
		return true;
	}

	SelectorType selector_type = lookupSelector(_engine->_gamestate->_segMan, object, selectorId, NULL, NULL);

	if (selector_type == kSelectorNone) {
		DebugPrintf("Object does not support selector: \"%s\"\n", selectorName);
		return true;
	}

	// everything after the selector name is passed as an argument to the send
	int send_argc = argc - 3;

	// Create the data block for send_selecor() at the top of the stack:
	// [selector_number][argument_counter][arguments...]
	StackPtr stackframe = _engine->_gamestate->_executionStack.back().sp;
	stackframe[0] = make_reg(0, selectorId);
	stackframe[1] = make_reg(0, send_argc);
	for (int i = 0; i < send_argc; i++) {
		if (parse_reg_t(_engine->_gamestate, argv[3+i], &stackframe[2+i], false)) {
			DebugPrintf("Invalid address \"%s\" passed.\n", argv[3+i]);
			DebugPrintf("Check the \"addresses\" command on how to use addresses\n");
			return true;
		}
	}

	reg_t old_acc = _engine->_gamestate->r_acc;

	// Now commit the actual function:
	ExecStack *old_xstack, *xstack;
	old_xstack = &_engine->_gamestate->_executionStack.back();
	xstack = send_selector(_engine->_gamestate, object, object,
	                       stackframe + 2 + send_argc,
	                       2 + send_argc, stackframe);

	bool restore_acc = old_xstack != xstack || argc == 3;

	if (old_xstack != xstack) {
		_engine->_gamestate->_executionStackPosChanged = true;
		DebugPrintf("Message scheduled for execution\n");

		// We call run_engine explictly so we can restore the value of r_acc
		// after execution.
		run_vm(_engine->_gamestate);

	}

	if (restore_acc) {
		// varselector read or message executed
		DebugPrintf("Message completed. Value returned: %04x:%04x\n", PRINT_REG(_engine->_gamestate->r_acc));
		_engine->_gamestate->r_acc = old_acc;
	}

	return true;
}

bool Console::cmdGo(int argc, const char **argv) {
	// CHECKME: is this necessary?
	_debugState.seeking = kDebugSeekNothing;

	return Cmd_Exit(argc, argv);
}

bool Console::cmdLogKernel(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("Logs calls to specified kernel function.\n");
		DebugPrintf("Usage: %s <kernel function/*> <on/off>\n", argv[0]);
		DebugPrintf("Example: %s StrCpy on\n", argv[0]);
		return true;
	}

	bool logging;
	if (strcmp(argv[2], "on") == 0)
		logging = true;
	else if (strcmp(argv[2], "off") == 0)
		logging = false;
	else {
		DebugPrintf("2nd parameter must be either on or off\n");
		return true;
	}

	if (g_sci->getKernel()->debugSetFunction(argv[1], logging, -1))
		DebugPrintf("Logging %s for k%s\n", logging ? "enabled" : "disabled", argv[1]);
	else
		DebugPrintf("Unknown kernel function %s\n", argv[1]);
	return true;
}

bool Console::cmdBreakpointList(int argc, const char **argv) {
	int i = 0;
	int bpdata;

	DebugPrintf("Breakpoint list:\n");

	Common::List<Breakpoint>::const_iterator bp = _debugState._breakpoints.begin();
	Common::List<Breakpoint>::const_iterator end = _debugState._breakpoints.end();
	for (; bp != end; ++bp) {
		DebugPrintf("  #%i: ", i);
		switch (bp->type) {
		case BREAK_SELECTOREXEC:
			DebugPrintf("Execute %s\n", bp->name.c_str());
			break;
		case BREAK_SELECTORREAD:
			DebugPrintf("Read %s\n", bp->name.c_str());
			break;
		case BREAK_SELECTORWRITE:
			DebugPrintf("Write %s\n", bp->name.c_str());
			break;
		case BREAK_EXPORT:
			bpdata = bp->address;
			DebugPrintf("Execute script %d, export %d\n", bpdata >> 16, bpdata & 0xFFFF);
			break;
		}

		i++;
	}

	if (!i)
		DebugPrintf("  No breakpoints defined.\n");

	return true;
}

bool Console::cmdBreakpointDelete(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Deletes a breakpoint with the specified index.\n");
		DebugPrintf("Usage: %s <breakpoint index>\n", argv[0]);
		DebugPrintf("<index> * will remove all breakpoints\n");
		return true;
	}

	if (strcmp(argv[1], "*") == 0) {
		_debugState._breakpoints.clear();
		_debugState._activeBreakpointTypes = 0;
		return true;
	}

	const int idx = atoi(argv[1]);

	// Find the breakpoint at index idx.
	Common::List<Breakpoint>::iterator bp = _debugState._breakpoints.begin();
	const Common::List<Breakpoint>::iterator end = _debugState._breakpoints.end();
	for (int i = 0; bp != end && i < idx; ++bp, ++i) {
		// do nothing
	}

	if (bp == end) {
		DebugPrintf("Invalid breakpoint index %i\n", idx);
		return true;
	}

	// Delete it
	_debugState._breakpoints.erase(bp);

	// Update EngineState::_activeBreakpointTypes.
	int type = 0;
	for (bp = _debugState._breakpoints.begin(); bp != end; ++bp) {
		type |= bp->type;
	}

	_debugState._activeBreakpointTypes = type;

	return true;
}

bool Console::cmdBreakpointMethod(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets a breakpoint on execution of a specified method/selector.\n");
		DebugPrintf("Usage: %s <name>\n", argv[0]);
		DebugPrintf("Example: %s ego::doit\n", argv[0]);
		DebugPrintf("May also be used to set a breakpoint that applies whenever an object\n");
		DebugPrintf("of a specific type is touched: %s foo::\n", argv[0]);
		return true;
	}

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */
	Breakpoint bp;
	bp.type = BREAK_SELECTOREXEC;
	bp.name = argv[1];

	_debugState._breakpoints.push_back(bp);
	_debugState._activeBreakpointTypes |= BREAK_SELECTOREXEC;
	return true;
}

bool Console::cmdBreakpointRead(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets a breakpoint on reading of a specified selector.\n");
		DebugPrintf("Usage: %s <name>\n", argv[0]);
		DebugPrintf("Example: %s ego::view\n", argv[0]);
		return true;
	}

	Breakpoint bp;
	bp.type = BREAK_SELECTORREAD;
	bp.name = argv[1];

	_debugState._breakpoints.push_back(bp);
	_debugState._activeBreakpointTypes |= BREAK_SELECTORREAD;
	return true;
}

bool Console::cmdBreakpointWrite(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Sets a breakpoint on writing of a specified selector.\n");
		DebugPrintf("Usage: %s <name>\n", argv[0]);
		DebugPrintf("Example: %s ego::view\n", argv[0]);
		return true;
	}

	Breakpoint bp;
	bp.type = BREAK_SELECTORWRITE;
	bp.name = argv[1];

	_debugState._breakpoints.push_back(bp);
	_debugState._activeBreakpointTypes |= BREAK_SELECTORWRITE;
	return true;
}

bool Console::cmdBreakpointKernel(int argc, const char **argv) {
	if (argc < 3) {
		DebugPrintf("Sets a breakpoint on execution of a kernel function.\n");
		DebugPrintf("Usage: %s <name> <on/off>\n", argv[0]);
		DebugPrintf("Example: %s DrawPic on\n", argv[0]);
		return true;
	}

	bool breakpoint;
	if (strcmp(argv[2], "on") == 0)
		breakpoint = true;
	else if (strcmp(argv[2], "off") == 0)
		breakpoint = false;
	else {
		DebugPrintf("2nd parameter must be either on or off\n");
		return true;
	}

	if (g_sci->getKernel()->debugSetFunction(argv[1], -1, breakpoint))
		DebugPrintf("Breakpoint %s for k%s\n", (breakpoint ? "enabled" : "disabled"), argv[1]);
	else
		DebugPrintf("Unknown kernel function %s\n", argv[1]);

	return true;
}

bool Console::cmdBreakpointFunction(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Sets a breakpoint on the execution of the specified exported function.\n");
		DebugPrintf("Usage: %s <script number> <export number\n", argv[0]);
		return true;
	}

	/* Note: We can set a breakpoint on a method that has not been loaded yet.
	   Thus, we can't check whether the command argument is a valid method name.
	   A breakpoint set on an invalid method name will just never trigger. */
	Breakpoint bp;
	bp.type = BREAK_EXPORT;
	// script number, export number
	bp.address = (atoi(argv[1]) << 16 | atoi(argv[2]));

	_debugState._breakpoints.push_back(bp);
	_debugState._activeBreakpointTypes |= BREAK_EXPORT;

	return true;
}

bool Console::cmdSfx01Header(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Dumps the header of a SCI01 song\n");
		DebugPrintf("Usage: %s <track>\n", argv[0]);
		return true;
	}

	Resource *song = _engine->getResMan()->findResource(ResourceId(kResourceTypeSound, atoi(argv[1])), 0);

	if (!song) {
		DebugPrintf("Doesn't exist\n");
		return true;
	}

	uint32 offset = 0;

	DebugPrintf("SCI01 song track mappings:\n");

	if (*song->data == 0xf0) // SCI1 priority spec
		offset = 8;

	if (song->size <= 0)
		return 1;

	while (song->data[offset] != 0xff) {
		byte device_id = song->data[offset];
		DebugPrintf("* Device %02x:\n", device_id);
		offset++;

		if (offset + 1 >= song->size)
			return 1;

		while (song->data[offset] != 0xff) {
			int track_offset;
			int end;
			byte header1, header2;

			if (offset + 7 >= song->size)
				return 1;

			offset += 2;

			track_offset = READ_LE_UINT16(song->data + offset);
			header1 = song->data[track_offset];
			header2 = song->data[track_offset+1];
			track_offset += 2;

			end = READ_LE_UINT16(song->data + offset + 2);
			DebugPrintf("  - %04x -- %04x", track_offset, track_offset + end);

			if (track_offset == 0xfe)
				DebugPrintf(" (PCM data)\n");
			else
				DebugPrintf(" (channel %d, special %d, %d playing notes, %d foo)\n",
				          header1 & 0xf, header1 >> 4, header2 & 0xf, header2 >> 4);
			offset += 4;
		}
		offset++;
	}

	return true;
}

static int _parse_ticks(byte *data, int *offset_p, int size) {
	int ticks = 0;
	int tempticks;
	int offset = 0;

	do {
		tempticks = data[offset++];
		ticks += (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX) ? SCI_MIDI_TIME_EXPANSION_LENGTH : tempticks;
	} while (tempticks == SCI_MIDI_TIME_EXPANSION_PREFIX && offset < size);

	if (offset_p)
		*offset_p = offset;

	return ticks;
}

// Specialised for SCI01 tracks (this affects the way cumulative cues are treated)
static void midi_hexdump(byte *data, int size, int notational_offset) {
	int offset = 0;
	int prev = 0;
	const int MIDI_cmdlen[16] = {0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0};

	if (*data == 0xf0) // SCI1 priority spec
		offset = 8;

	while (offset < size) {
		int old_offset = offset;
		int offset_mod;
		int time = _parse_ticks(data + offset, &offset_mod, size);
		int cmd;
		int pleft;
		int firstarg = 0;
		int i;
		int blanks = 0;

		offset += offset_mod;
		debugN("  [%04x] %d\t",
		        old_offset + notational_offset, time);

		cmd = data[offset];
		if (!(cmd & 0x80)) {
			cmd = prev;
			if (prev < 0x80) {
				debugN("Track broken at %x after"
				        " offset mod of %d\n",
				        offset + notational_offset, offset_mod);
				Common::hexdump(data, size, 16, notational_offset);
				return;
			}
			debugN("(rs %02x) ", cmd);
			blanks += 8;
		} else {
			++offset;
			debugN("%02x ", cmd);
			blanks += 3;
		}
		prev = cmd;

		pleft = MIDI_cmdlen[cmd >> 4];
		if (SCI_MIDI_CONTROLLER(cmd) && data[offset] == SCI_MIDI_CUMULATIVE_CUE)
			--pleft; // This is SCI(0)1 specific

		for (i = 0; i < pleft; i++) {
			if (i == 0)
				firstarg = data[offset];
			debugN("%02x ", data[offset++]);
			blanks += 3;
		}

		while (blanks < 16) {
			blanks += 4;
			debugN("    ");
		}

		while (blanks < 20) {
			++blanks;
			debugN(" ");
		}

		if (cmd == SCI_MIDI_EOT)
			debugN(";; EOT");
		else if (cmd == SCI_MIDI_SET_SIGNAL) {
			if (firstarg == SCI_MIDI_SET_SIGNAL_LOOP)
				debugN(";; LOOP point");
			else
				debugN(";; CUE (%d)", firstarg);
		} else if (SCI_MIDI_CONTROLLER(cmd)) {
			if (firstarg == SCI_MIDI_CUMULATIVE_CUE)
				debugN(";; CUE (cumulative)");
			else if (firstarg == SCI_MIDI_RESET_ON_SUSPEND)
				debugN(";; RESET-ON-SUSPEND flag");
		}
		debugN("\n");

		if (old_offset >= offset) {
			debugN("-- Not moving forward anymore,"
			        " aborting (%x/%x)\n", offset, old_offset);
			return;
		}
	}
}

bool Console::cmdSfx01Track(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Dumps a track of a SCI01 song\n");
		DebugPrintf("Usage: %s <track> <offset>\n", argv[0]);
		return true;
	}

	Resource *song = _engine->getResMan()->findResource(ResourceId(kResourceTypeSound, atoi(argv[1])), 0);

	int offset = atoi(argv[2]);

	if (!song) {
		DebugPrintf("Doesn't exist\n");
		return true;
	}

	midi_hexdump(song->data + offset, song->size, offset);

	return true;
}

bool Console::cmdQuit(int argc, const char **argv) {
	if (argc != 2) {
	}

	if (argc == 2 && !scumm_stricmp(argv[1], "now")) {
		// Quit ungracefully
		g_system->quit();
	} else if (argc == 1 || (argc == 2 && !scumm_stricmp(argv[1], "game"))) {

		// Quit gracefully
		_engine->_gamestate->abortScriptProcessing = kAbortQuitGame; // Terminate VM
		_debugState.seeking = kDebugSeekNothing;
		_debugState.runningStep = 0;

	} else {
		DebugPrintf("%s [game] - exit gracefully\n", argv[0]);
		DebugPrintf("%s now - exit ungracefully\n", argv[0]);
		return true;
	}

	return Cmd_Exit(0, 0);
}

bool Console::cmdAddresses(int argc, const char **argv) {
	DebugPrintf("Address parameters may be passed in one of three forms:\n");
	DebugPrintf(" - ssss:oooo -- where 'ssss' denotes a segment and 'oooo' an offset.\n");
	DebugPrintf("   Example: \"a:c5\" would address something in segment 0xa at offset 0xc5.\n");
	DebugPrintf(" - &scr:oooo -- where 'scr' is a script number and oooo an offset within that script; will\n");
	DebugPrintf("   fail if the script is not currently loaded\n");
	DebugPrintf(" - $REG -- where 'REG' is one of 'PC', 'ACC', 'PREV' or 'OBJ': References the address\n");
	DebugPrintf("   indicated by the register of this name.\n");
	DebugPrintf(" - $REG+n (or -n) -- Like $REG, but modifies the offset part by a specific amount (which\n");
	DebugPrintf("   is specified in hexadecimal).\n");
	DebugPrintf(" - ?obj -- Looks up an object with the specified name, uses its address. This will abort if\n");
	DebugPrintf("   the object name is ambiguous; in that case, a list of addresses and indices is provided.\n");
	DebugPrintf("   ?obj.idx may be used to disambiguate 'obj' by the index 'idx'.\n");

	return true;
}

// Returns 0 on success
static int parse_reg_t(EngineState *s, const char *str, reg_t *dest, bool mayBeValue) {
	// Pointer to the part of str which contains a numeric offset (if any)
	const char *offsetStr = NULL;

	// Flag that tells whether the value stored in offsetStr is an absolute offset,
	// or a relative offset against dest->offset.
	bool relativeOffset = false;

	// Non-NULL: Parse end of string for relative offsets
	char *endptr;

	if (*str == '$') { // Register: "$FOO" or "$FOO+NUM" or "$FOO-NUM
		relativeOffset = true;

		if (!scumm_strnicmp(str + 1, "PC", 2)) {
			// TODO: Handle 32-bit PC addresses
			reg32_t pc = s->_executionStack.back().addr.pc;
			*dest = make_reg(pc.getSegment(), (uint16)pc.getOffset());
			offsetStr = str + 3;
		} else if (!scumm_strnicmp(str + 1, "P", 1)) {
			// TODO: Handle 32-bit PC addresses
			reg32_t pc = s->_executionStack.back().addr.pc;
			*dest = make_reg(pc.getSegment(), (uint16)pc.getOffset());
			offsetStr = str + 2;
		} else if (!scumm_strnicmp(str + 1, "PREV", 4)) {
			*dest = s->r_prev;
			offsetStr = str + 5;
		} else if (!scumm_strnicmp(str + 1, "ACC", 3)) {
			*dest = s->r_acc;
			offsetStr = str + 4;
		} else if (!scumm_strnicmp(str + 1, "A", 1)) {
			*dest = s->r_acc;
			offsetStr = str + 2;
		} else if (!scumm_strnicmp(str + 1, "OBJ", 3)) {
			*dest = s->_executionStack.back().objp;
			offsetStr = str + 4;
		} else if (!scumm_strnicmp(str + 1, "O", 1)) {
			*dest = s->_executionStack.back().objp;
			offsetStr = str + 2;
		} else
			return 1; // No matching register

		if (!*offsetStr)
			offsetStr = NULL;
		else if (*offsetStr != '+' && *offsetStr != '-')
			return 1;
	} else if (*str == '&') { // Script relative: "&SCRIPT-ID:OFFSET"
		// Look up by script ID. The text from start till just before the colon
		// (resp. end of string, if there is no colon) contains the script ID.
		const char *colon = strchr(str, ':');
		if (!colon)
			return 1;

		// Extract the script id and parse it
		Common::String scriptStr(str, colon);
		int script_nr = strtol(scriptStr.c_str() + 1, &endptr, 10);
		if (*endptr)
			return 1;

		// Now lookup the script's segment
		dest->setSegment(s->_segMan->getScriptSegment(script_nr));
		if (!dest->getSegment()) {
			return 1;
		}

		// Finally, after the colon comes the offset
		offsetStr = colon + 1;

	} else {
		// Now we either got an object name, or segment:offset or plain value
		//  segment:offset is recognized by the ":"
		//  plain value may be "123" or "123h" or "fffh" or "0xfff"
		//  object name is assumed if nothing else matches or a "?" is used as prefix as override
		//  object name may contain "+", "-" and "." for relative calculations, those chars are used nowhere else

		// First we cycle through the string counting special chars
		const char *strLoop = str;
		int charsCount = strlen(str);
		int charsCountObject = 0;
		int charsCountSegmentOffset = 0;
		int charsCountLetter = 0;
		int charsCountNumber = 0;
		bool charsForceHex = false;
		bool charsForceObject = false;

		while (*strLoop) {
			switch (*strLoop) {
			case '+':
			case '-':
			case '.':
				charsCountObject++;
				break;
			case '?':
				if (strLoop == str) {
					charsForceObject = true;
					str++; // skip over prefix
				}
				break;
			case ':':
				charsCountSegmentOffset++;
				break;
			case 'h':
				if (*(strLoop + 1) == 0)
					charsForceHex = true;
				else
					charsCountObject++;
				break;
			case '0':
				if (*(strLoop + 1) == 'x') {
					str += 2; // skip "0x"
					strLoop++; // skip "x"
					charsForceHex = true;
				}
				charsCountNumber++;
				break;
			default:
				if ((*strLoop >= '0') && (*strLoop <= '9'))
					charsCountNumber++;
				if ((*strLoop >= 'a') && (*strLoop <= 'f'))
					charsCountLetter++;
				if ((*strLoop >= 'A') && (*strLoop <= 'F'))
					charsCountLetter++;
				if ((*strLoop >= 'i') && (*strLoop <= 'z'))
					charsCountObject++;
				if ((*strLoop >= 'I') && (*strLoop <= 'Z'))
					charsCountObject++;
			}
			strLoop++;
		}

		if ((charsCountObject) && (charsCountSegmentOffset))
			return 1; // input doesn't make sense

		if (!charsForceObject) {
			// input may be values/segment:offset

			if (charsCountSegmentOffset) {
				// ':' found, so must be segment:offset
				const char *colon = strchr(str, ':');

				offsetStr = colon + 1;

				Common::String segmentStr(str, colon);
				dest->setSegment(strtol(segmentStr.c_str(), &endptr, 16));
				if (*endptr)
					return 1;
			} else {
				int val = 0;
				dest->setSegment(0);

				if (charsCountNumber == charsCount) {
					// Only numbers in input, assume decimal value
					val = strtol(str, &endptr, 10);
					if (*endptr)
						return 1; // strtol failed?
					dest->setOffset(val);
					return 0;
				} else {
					// We also got letters, check if there were only hexadecimal letters and '0x' at the start or 'h' at the end
					if ((charsForceHex) && (!charsCountObject)) {
						val = strtol(str, &endptr, 16);
						if ((*endptr != 'h') && (*endptr != 0))
							return 1;
						dest->setOffset(val);
						return 0;
					} else {
						// Something else was in input, assume object name
						charsForceObject = true;
					}
				}
			}
		}

		if (charsForceObject) {
			// We assume now that input is object name
			// Object by name: "?OBJ" or "?OBJ.INDEX" or "?OBJ.INDEX+OFFSET" or "?OBJ.INDEX-OFFSET"
			// The (optional) index can be used to distinguish multiple object with the same name.
			int index = -1;

			// Look for an offset. It starts with + or -
			relativeOffset = true;
			offsetStr = strchr(str, '+');
			if (!offsetStr)	// No + found, look for -
				offsetStr = strchr(str, '-');

			// Strip away the offset and the leading '?'
			Common::String str_objname;
			if (offsetStr)
				str_objname = Common::String(str, offsetStr);
			else
				str_objname = str;

			// Scan for a period, after which (if present) we'll find an index
			const char *tmp = Common::find(str_objname.begin(), str_objname.end(), '.');
			if (tmp != str_objname.end()) {
				index = strtol(tmp + 1, &endptr, 16);
				if (*endptr)
					return -1;
				// Chop of the index
				str_objname = Common::String(str_objname.c_str(), tmp);
			}

			// Now all values are available; iterate over all objects.
			*dest = s->_segMan->findObjectByName(str_objname, index);
			if (dest->isNull())
				return 1;
		}
	}
	if (offsetStr) {
		int val = strtol(offsetStr, &endptr, 16);

		if (relativeOffset)
			dest->incOffset(val);
		else
			dest->setOffset(val);

		if (*endptr)
			return 1;
	}

	return 0;
}

bool Console::parseInteger(const char *argument, int &result) {
	char *endPtr = 0;
	int idxLen = strlen(argument);
	const char *lastChar = argument + idxLen - (idxLen == 0 ? 0 : 1);

	if ((strncmp(argument, "0x", 2) == 0) || (*lastChar == 'h')) {
		// hexadecimal number
		result = strtol(argument, &endPtr, 16);
		if ((*endPtr != 0) && (*endPtr != 'h')) {
			DebugPrintf("Invalid hexadecimal number '%s'\n", argument);
			return false;
		}
	} else {
		// decimal number
		result = strtol(argument, &endPtr, 10);
		if (*endPtr != 0) {
			DebugPrintf("Invalid decimal number '%s'\n", argument);
			return false;
		}
	}
	return true;
}

void Console::printBasicVarInfo(reg_t variable) {
	int regType = g_sci->getKernel()->findRegType(variable);
	int segType = regType;
	SegManager *segMan = g_sci->getEngineState()->_segMan;

	segType &= SIG_TYPE_INTEGER | SIG_TYPE_OBJECT | SIG_TYPE_REFERENCE | SIG_TYPE_NODE | SIG_TYPE_LIST | SIG_TYPE_UNINITIALIZED | SIG_TYPE_ERROR;

	switch (segType) {
	case SIG_TYPE_INTEGER: {
		uint16 content = variable.toUint16();
		if (content >= 10)
			DebugPrintf(" (%dd)", content);
		break;
	}
	case SIG_TYPE_OBJECT:
		DebugPrintf(" (object '%s')", segMan->getObjectName(variable));
		break;
	case SIG_TYPE_REFERENCE:
		DebugPrintf(" (reference)");
		break;
	case SIG_TYPE_NODE:
		DebugPrintf(" (node)");
		break;
	case SIG_TYPE_LIST:
		DebugPrintf(" (list)");
		break;
	case SIG_TYPE_UNINITIALIZED:
		DebugPrintf(" (uninitialized)");
		break;
	case SIG_TYPE_ERROR:
		DebugPrintf(" (error)");
		break;
	default:
		DebugPrintf(" (??\?)");
	}

	if (regType & SIG_IS_INVALID)
		DebugPrintf(" IS INVALID!");
}

void Console::printList(List *list) {
	reg_t pos = list->first;
	reg_t my_prev = NULL_REG;

	DebugPrintf("\t<\n");

	while (!pos.isNull()) {
		Node *node;
		NodeTable *nt = (NodeTable *)_engine->_gamestate->_segMan->getSegment(pos.getSegment(), SEG_TYPE_NODES);

		if (!nt || !nt->isValidEntry(pos.getOffset())) {
			DebugPrintf("   WARNING: %04x:%04x: Doesn't contain list node!\n",
			          PRINT_REG(pos));
			return;
		}

		node = &(nt->_table[pos.getOffset()]);

		DebugPrintf("\t%04x:%04x  : %04x:%04x -> %04x:%04x\n", PRINT_REG(pos), PRINT_REG(node->key), PRINT_REG(node->value));

		if (my_prev != node->pred)
			DebugPrintf("   WARNING: current node gives %04x:%04x as predecessor!\n",
			          PRINT_REG(node->pred));

		my_prev = pos;
		pos = node->succ;
	}

	if (my_prev != list->last)
		DebugPrintf("   WARNING: Last node was expected to be %04x:%04x, was %04x:%04x!\n",
		          PRINT_REG(list->last), PRINT_REG(my_prev));
	DebugPrintf("\t>\n");
}

int Console::printNode(reg_t addr) {
	SegmentObj *mobj = _engine->_gamestate->_segMan->getSegment(addr.getSegment(), SEG_TYPE_LISTS);

	if (mobj) {
		ListTable *lt = (ListTable *)mobj;
		List *list;

		if (!lt->isValidEntry(addr.getOffset())) {
			DebugPrintf("Address does not contain a list\n");
			return 1;
		}

		list = &(lt->_table[addr.getOffset()]);

		DebugPrintf("%04x:%04x : first x last = (%04x:%04x, %04x:%04x)\n", PRINT_REG(addr), PRINT_REG(list->first), PRINT_REG(list->last));
	} else {
		NodeTable *nt;
		Node *node;
		mobj = _engine->_gamestate->_segMan->getSegment(addr.getSegment(), SEG_TYPE_NODES);

		if (!mobj) {
			DebugPrintf("Segment #%04x is not a list or node segment\n", addr.getSegment());
			return 1;
		}

		nt = (NodeTable *)mobj;

		if (!nt->isValidEntry(addr.getOffset())) {
			DebugPrintf("Address does not contain a node\n");
			return 1;
		}
		node = &(nt->_table[addr.getOffset()]);

		DebugPrintf("%04x:%04x : prev x next = (%04x:%04x, %04x:%04x); maps %04x:%04x -> %04x:%04x\n",
		          PRINT_REG(addr), PRINT_REG(node->pred), PRINT_REG(node->succ), PRINT_REG(node->key), PRINT_REG(node->value));
	}

	return 0;
}

int Console::printObject(reg_t pos) {
	EngineState *s = _engine->_gamestate;	// for the several defines in this function
	const Object *obj = s->_segMan->getObject(pos);
	const Object *var_container = obj;
	uint i;

	if (!obj) {
		DebugPrintf("[%04x:%04x]: Not an object.", PRINT_REG(pos));
		return 1;
	}

	// Object header
	DebugPrintf("[%04x:%04x] %s : %3d vars, %3d methods\n", PRINT_REG(pos), s->_segMan->getObjectName(pos),
				obj->getVarCount(), obj->getMethodCount());

	if (!obj->isClass() && getSciVersion() != SCI_VERSION_3)
		var_container = s->_segMan->getObject(obj->getSuperClassSelector());
	DebugPrintf("  -- member variables:\n");
	for (i = 0; (uint)i < obj->getVarCount(); i++) {
		DebugPrintf("    ");
		if (var_container && i < var_container->getVarCount()) {
			uint16 varSelector = var_container->getVarSelector(i);
			DebugPrintf("[%03x] %s = ", varSelector, _engine->getKernel()->getSelectorName(varSelector).c_str());
		} else
			DebugPrintf("p#%x = ", i);

		reg_t val = obj->getVariable(i);
		DebugPrintf("%04x:%04x", PRINT_REG(val));

		if (!val.getSegment())
			DebugPrintf(" (%d)", val.getOffset());

		const Object *ref = s->_segMan->getObject(val);
		if (ref)
			DebugPrintf(" (%s)", s->_segMan->getObjectName(val));

		DebugPrintf("\n");
	}
	DebugPrintf("  -- methods:\n");
	for (i = 0; i < obj->getMethodCount(); i++) {
		reg_t fptr = obj->getFunction(i);
		DebugPrintf("    [%03x] %s = %04x:%04x\n", obj->getFuncSelector(i), _engine->getKernel()->getSelectorName(obj->getFuncSelector(i)).c_str(), PRINT_REG(fptr));
	}
	if (s->_segMan->_heap[pos.getSegment()]->getType() == SEG_TYPE_SCRIPT)
		DebugPrintf("\nOwner script: %d\n", s->_segMan->getScript(pos.getSegment())->getScriptNumber());

	return 0;
}

static void printChar(byte c) {
	if (c < 32 || c >= 127)
		c = '.';
	debugN("%c", c);
}

void Console::hexDumpReg(const reg_t *data, int len, int regsPerLine, int startOffset, bool isArray) {
	// reg_t version of Common::hexdump
	assert(1 <= regsPerLine && regsPerLine <= 8);
	int i;
	int offset = startOffset;
	while (len >= regsPerLine) {
		debugN("%06x: ", offset);
		for (i = 0; i < regsPerLine; i++) {
			debugN("%04x:%04x  ", PRINT_REG(data[i]));
		}
		debugN(" |");
		for (i = 0; i < regsPerLine; i++) {
			if (g_sci->isBE()) {
				printChar(data[i].toUint16() >> 8);
				printChar(data[i].toUint16() & 0xff);
			} else {
				printChar(data[i].toUint16() & 0xff);
				printChar(data[i].toUint16() >> 8);
			}
		}
		debugN("|\n");
		data += regsPerLine;
		len -= regsPerLine;
		offset += regsPerLine * (isArray ? 1 : 2);
	}

	if (len <= 0)
		return;

	debugN("%06x: ", offset);
	for (i = 0; i < regsPerLine; i++) {
		if (i < len)
			debugN("%04x:%04x  ", PRINT_REG(data[i]));
		else
			debugN("           ");
	}
	debugN(" |");
	for (i = 0; i < len; i++) {
		if (g_sci->isBE()) {
			printChar(data[i].toUint16() >> 8);
			printChar(data[i].toUint16() & 0xff);
		} else {
			printChar(data[i].toUint16() & 0xff);
			printChar(data[i].toUint16() >> 8);
		}
	}
	for (; i < regsPerLine; i++)
		debugN("  ");
	debugN("|\n");
}

} // End of namespace Sci
