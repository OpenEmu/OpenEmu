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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GUI_DEBUGGER_H
#define GUI_DEBUGGER_H

#include "common/func.h"
#include "common/ptr.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/array.h"

namespace GUI {

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
class ConsoleDialog;
#endif

class Debugger {
public:
	Debugger();
	virtual ~Debugger();

	int DebugPrintf(const char *format, ...) GCC_PRINTF(2, 3);

	/**
	 * The onFrame() method should be invoked by the engine at regular
	 * intervals (usually once per main loop iteration) whenever the
	 * debugger is attached.
	 * This will open up the console and accept user input if certain
	 * preconditions are met, such as the frame countdown having
	 * reached zero.
	 *
	 * Subclasses can override this to e.g. check for breakpoints being
	 * triggered.
	 */
	virtual void onFrame();

	/**
	 * 'Attach' the debugger. This ensures that the next time onFrame()
	 * is invoked, the debugger will activate and accept user input.
	 */
	virtual void attach(const char *entry = 0);

	/**
	 * Return true if the debugger is currently active (i.e. executing
	 * a command or waiting for use input).
	 */
	bool isActive() const { return _isActive; }

protected:
	typedef Common::Functor2<int, const char **, bool> Debuglet;

	/**
	 * Convenience macro that makes it easier to register a method
	 * of a debugger subclass as a command.
	 * Usage example:
	 *   DCmd_Register("COMMAND", WRAP_METHOD(MyDebugger, MyCmd));
	 * would register the method MyDebugger::MyCmd(int, const char **)
	 * under the command name "COMMAND".
	 */
	#define WRAP_METHOD(cls, method) \
		new Common::Functor2Mem<int, const char **, bool, cls>(this, &cls::method)

	enum VarType {
		DVAR_BYTE,
		DVAR_INT,
		DVAR_BOOL,
		DVAR_INTARRAY,
		DVAR_STRING
	};

	struct DVar {
		Common::String name;
		void *variable;
		VarType type;
		int arraySize;
	};


	/**
	 * Register a variable with the debugger. This allows the user to read and modify
	 * this variable.
	 * @param varname	the identifier with which the user may access the variable
	 * @param variable	pointer to the actual storage of the variable
	 * @param type		the type of the variable (byte, int, bool, ...)
	 * @paral arraySize	for type DVAR_INTARRAY this specifies the size of the array
	 *
	 * @todo	replace this single method by type safe variants.
	 */
	void DVar_Register(const Common::String &varname, void *variable, VarType type, int arraySize);
	void DCmd_Register(const Common::String &cmdname, Debuglet *debuglet);


private:
	/**
	 * The frame countdown specifies a number of frames that must pass
	 * until the console will show up. This value is decremented by one
	 * each time onFrame() is called, until it reaches 0, at which point
	 * onFrame() will open the console and handle input into it.
	 *
	 * The user can modify this value using the debug_countdown command.
	 *
	 * Note: The console must be in *attached* state, otherwise, it
	 * won't show up (and the countdown won't count down either).
	 */
	uint _frameCountdown;

	Common::Array<DVar> _dvars;

	typedef Common::HashMap<Common::String, Common::SharedPtr<Debuglet>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> CommandsMap;
	CommandsMap _cmds;

	/**
	 * True if the debugger is currently active (i.e. executing
	 * a command or waiting for use input).
	 */
	bool _isActive;

	char *_errStr;

	/**
	 * Initially true, set to false when Debugger::enter is called
	 * the first time. We use this flag to show a greeting message
	 * to the user once, when he opens the debugger for the first
	 * time.
	 */
	bool _firstTime;

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
	GUI::ConsoleDialog *_debuggerDialog;
#endif

protected:
	/**
	 * Hook for subclasses which is called just before enter() is run.
	 * A typical usage example is pausing music and sound effects.
	 *
	 * The default implementation invokes Engine::pauseEngine(true).
	 */
	virtual void preEnter();

	/**
	 * Hook for subclasses which is called just after enter() was run.
	 * A typical usage example is resuming music and sound effects.
	 *
	 * The default implementation invokes Engine::pauseEngine(false).
	 */
	virtual void postEnter();

	/**
	 * Subclasses should invoke the detach() method in their Cmd_FOO methods
	 * if that command will resume execution of the program (as opposed to
	 * executing, say, a "single step through code" command).
	 *
	 * This currently only hides the virtual keyboard, if any.
	 */
	void detach();

private:
	void enter();

	bool parseCommand(const char *input);
	bool tabComplete(const char *input, Common::String &completion) const;

	/**
	 * Process the given command line.
	 * Returns true if and only if argv[0] is a known command and was
	 * handled, false otherwise.
	 */
	virtual bool handleCommand(int argc, const char **argv, bool &keepRunning);

protected:
	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_OpenLog(int argc, const char **argv);
	bool Cmd_DebugFlagsList(int argc, const char **argv);
	bool Cmd_DebugFlagEnable(int argc, const char **argv);
	bool Cmd_DebugFlagDisable(int argc, const char **argv);

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
private:
	static bool debuggerInputCallback(GUI::ConsoleDialog *console, const char *input, void *refCon);
	static bool debuggerCompletionCallback(GUI::ConsoleDialog *console, const char *input, Common::String &completion, void *refCon);
#elif defined(USE_READLINE)
public:
	char *readlineComplete(const char *input, int state);
#endif

};

}	// End of namespace GUI

#endif
