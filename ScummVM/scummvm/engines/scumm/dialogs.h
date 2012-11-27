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

#ifndef SCUMM_DIALOGS_H
#define SCUMM_DIALOGS_H

#include "common/str.h"
#include "gui/dialog.h"
#include "gui/widget.h"
#include "engines/dialogs.h"

#include "scumm/detection.h"

namespace GUI {
class ListWidget;
class CommandSender;
}


namespace Scumm {

class ScummEngine;

class ScummDialog : public GUI::Dialog {
public:
	ScummDialog(int x, int y, int w, int h);
	ScummDialog(Common::String name);

protected:
	typedef Common::String String;
};

#ifndef DISABLE_HELP
class ScummMenuDialog : public MainMenuDialog {
public:
	ScummMenuDialog(ScummEngine *scumm);
	~ScummMenuDialog();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

protected:
	GUI::Dialog		*_helpDialog;
};
#endif

/**
 * A dialog which displays an arbitrary message to the user and returns
 * ther users reply as its result value. More specifically, it returns
 * the ASCII code of the key used to close the dialog (0 if a mouse
 * click closed the dialog).
 */
class InfoDialog : public ScummDialog {
protected:
	ScummEngine		*_vm;
	String _message;
	GUI::StaticTextWidget *_text;

public:
	// arbitrary message
	InfoDialog(ScummEngine *scumm, const String& message);
	// from resources
	InfoDialog(ScummEngine *scumm, int res);

	void setInfoText(const String& message);

	virtual void handleMouseDown(int x, int y, int button, int clickCount) {
		setResult(0);
		close();
	}
	virtual void handleKeyDown(Common::KeyState state) {
		setResult(state.ascii);
		close();
	}

	virtual void reflowLayout();

protected:

	// Query a string from the resources
	const String queryResString(int stringno);
};

/**
 * The pause dialog, visible whenever the user activates pause mode. Goes
 * away uon any key or mouse button press.
 */
class PauseDialog : public InfoDialog {
public:
	PauseDialog(ScummEngine *scumm, int res);
	virtual void handleKeyDown(Common::KeyState state);
};

/**
 * A simple yes/no dialog, used to ask the user whether to really
 * quit/restart ScummVM.
 */
class ConfirmDialog : public InfoDialog {
public:
	ConfirmDialog(ScummEngine *scumm, int res);
	virtual void handleKeyDown(Common::KeyState state);

protected:
	char _yesKey, _noKey;
};

/**
 * A dialog used to display the music volume / text speed.
 * Automatically closes after a brief time passed.
 */
class ValueDisplayDialog : public GUI::Dialog {
public:
	ValueDisplayDialog(const Common::String& label, int minVal, int maxVal, int val, uint16 incKey, uint16 decKey);

	virtual void open();
	virtual void drawDialog();
	virtual void handleTickle();
	virtual void handleMouseDown(int x, int y, int button, int clickCount) {
		close();
	}
	virtual void handleKeyDown(Common::KeyState state);

	virtual void reflowLayout();

protected:
	enum {
		kDisplayDelay = 1500
	};
	Common::String _label;
	const int _min, _max;
	const uint16 _incKey, _decKey;
	int _percentBarWidth;
	int _value;
	uint32 _timer;
};

/**
 * A dialog used to display and cycle subtitle settings.
 * Automatically closes after a brief time has passed.
 */
class SubtitleSettingsDialog : public InfoDialog {
public:
	SubtitleSettingsDialog(ScummEngine *scumm, int value);

	virtual void open();
	virtual void handleTickle();
	virtual void handleMouseDown(int x, int y, int button, int clickCount) {
		close();
	}
	virtual void handleKeyDown(Common::KeyState state);
protected:
	int _value;
	uint32 _timer;

	void cycleValue();
};

//The Indy IQ dialog
class Indy3IQPointsDialog : public InfoDialog {
public:
	Indy3IQPointsDialog(ScummEngine *scumm, char* text);
	virtual void handleKeyDown(Common::KeyState state);
};

class DebugInputDialog : public InfoDialog {
public:
	DebugInputDialog(ScummEngine *scumm, char* text);
	virtual void handleKeyDown(Common::KeyState state);
	bool done;
	Common::String buffer;
	Common::String mainText;
};

/**
 * Difficulty selection dialog for Loom FM-Towns.
 */
class LoomTownsDifficultyDialog : public GUI::Dialog {
public:
	LoomTownsDifficultyDialog();

	int getSelectedDifficulty() const { return _difficulty; }
protected:
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

private:
	enum {
		kStandardCmd = 'STDD',
		kPracticeCmd = 'PRAD',
		kExpertCmd = 'EXPD'
	};

	int _difficulty;
};

} // End of namespace Scumm

#endif
