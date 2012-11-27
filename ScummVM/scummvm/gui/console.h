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

#ifndef CONSOLE_DIALOG_H
#define CONSOLE_DIALOG_H

#include "gui/dialog.h"

namespace GUI {

class ScrollBarWidget;

/*
 FIXME #1: The console dialog code has some fundamental problems.
 First of, note the conflict between the (constant) value kCharsPerLine, and the
 (variable) value _pageWidth. Look a bit at the code to get familiar with them,
 then return...
 Now, why don't we just drop kCharsPerLine? Because of the problem of resizing!
 When the user changes the scaler, the console will get resized. If the dialog
 becomes smaller because of this, we may have to rewrap text. If the resolution
 is then increased again, we'd end up with garbled content.

 One can now either ignore this problem (and modify our code accordingly to
 implement this simple rewrapping -- we currently don't do that at all!).

 Or, one can go and implement a more complete console, by replacing the
 _buffer by a real line buffer -- an array of char* pointers.
 This will allow one to implement resizing perfectly, but has the drawback
 of making things like scrolling, drawing etc. more complicated.

 Either way, the current situation is bad, and we should resolve it one way
 or the other (and if you can think of a third, feel free to suggest it).



 FIXME #2: Another problem is that apparently _pageWidth isn't computed quite
 correctly. The current line ends well before reaching the right side of the
 console dialog. That's irritating and should be fixed.


 FIXME #3: The scroll bar is not shown initially, but the area it would
 occupy is not used for anything else. As a result, the gap described above
 becomes even wider and thus even more irritating.
*/
class ConsoleDialog : public Dialog {
public:
	typedef bool (*InputCallbackProc)(ConsoleDialog *console, const char *input, void *refCon);
	typedef bool (*CompletionCallbackProc)(ConsoleDialog* console, const char *input, Common::String &completion, void *refCon);

protected:
	enum {
		kBufferSize	= 32768,
		kCharsPerLine = 128,
		kLineBufferSize = 256,

		kHistorySize = 20
	};

	const Graphics::Font *_font;

	char	_buffer[kBufferSize];
	int		_linesInBuffer;

	int		_pageWidth;
	int		_linesPerPage;

	int		_currentPos;
	int		_scrollLine;
	int		_firstLineInBuffer;

	int		_promptStartPos;
	int		_promptEndPos;

	bool	_caretVisible;
	uint32	_caretTime;

	enum SlideMode {
		kNoSlideMode,
		kUpSlideMode,
		kDownSlideMode
	};

	SlideMode	_slideMode;
	uint32	_slideTime;

	ScrollBarWidget *_scrollBar;

	// The _callbackProc is called whenver a data line is entered
	//
	InputCallbackProc _callbackProc;
	void *_callbackRefCon;

	// _completionCallbackProc is called when tab is pressed
	CompletionCallbackProc _completionCallbackProc;
	void *_completionCallbackRefCon;

	char _history[kHistorySize][kLineBufferSize];
	int _historySize;
	int _historyIndex;
	int _historyLine;

	float _widthPercent, _heightPercent;

	int	_leftPadding;
	int	_rightPadding;
	int	_topPadding;
	int	_bottomPadding;

	void slideUpAndClose();

public:
	ConsoleDialog(float widthPercent, float heightPercent);

	void open();
	void close();
	void drawDialog();

	void handleTickle();
	void reflowLayout();
	void handleMouseWheel(int x, int y, int direction);
	void handleKeyDown(Common::KeyState state);
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	int printFormat(int dummy, const char *format, ...) GCC_PRINTF(3, 4);
	int vprintFormat(int dummy, const char *format, va_list argptr);

	void printChar(int c);

	void setInputCallback(InputCallbackProc proc, void *refCon) {
		_callbackProc = proc;
		_callbackRefCon = refCon;
	}
	void setCompletionCallback(CompletionCallbackProc proc, void *refCon) {
		_completionCallbackProc = proc;
		_completionCallbackRefCon = refCon;
	}

	int getCharsPerLine() {
		return _pageWidth;
	}

protected:
	inline char &buffer(int idx) {
		return _buffer[idx % kBufferSize];
	}

	void init();

	int pos2line(int pos) { return (pos - (_scrollLine - _linesPerPage + 1) * kCharsPerLine) / kCharsPerLine; }

	void drawLine(int line, bool restoreBg = true);
	void drawCaret(bool erase);
	void printCharIntern(int c);
	void insertIntoPrompt(const char *str);
	void print(const char *str);
	void updateScrollBuffer();
	void scrollToCurrent();

	void defaultKeyDownHandler(Common::KeyState &state);

	// Line editing
	void specialKeys(int keycode);
	void nextLine();
	void killChar();
	void killLine();
	void killLastWord();

	// History
	void addToHistory(const char *str);
	void historyScroll(int direction);
};

} // End of namespace GUI

#endif
