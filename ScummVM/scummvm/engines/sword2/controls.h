/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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

#ifndef	SWORD2_CONTROL_H
#define	SWORD2_CONTROL_H

#include "sword2/defs.h"
#include "sword2/saveload.h"

#define MAX_WIDGETS 25

namespace Sword2 {

class Sword2Engine;
class FontRendererGui;
class Widget;
class Switch;
class Slider;
class Button;
class ScrollButton;
class Slot;

enum {
	kSaveDialog,
	kRestoreDialog
};

/**
 * Base class for all dialogs.
 */

class Dialog {
private:
	int _numWidgets;
	Widget *_widgets[MAX_WIDGETS];
	bool _finish;
	int _result;

public:
	Sword2Engine *_vm;

	Dialog(Sword2Engine *vm);
	virtual ~Dialog();

	void registerWidget(Widget *widget);

	virtual void paint();
	virtual void setResult(int result);

	virtual int runModal();

	virtual void onAction(Widget *widget, int result = 0) {}
};

class OptionsDialog : public Dialog {
private:
	FontRendererGui *_fr;
	Widget *_panel;
	Switch *_objectLabelsSwitch;
	Switch *_subtitlesSwitch;
	Switch *_reverseStereoSwitch;
	Switch *_musicSwitch;
	Switch *_speechSwitch;
	Switch *_fxSwitch;
	Slider *_musicSlider;
	Slider *_speechSlider;
	Slider *_fxSlider;
	Slider *_gfxSlider;
	Widget *_gfxPreview;
	Button *_okButton;
	Button *_cancelButton;

	Audio::Mixer *_mixer;

public:
	OptionsDialog(Sword2Engine *vm);
	~OptionsDialog();

	virtual void paint();
	virtual void onAction(Widget *widget, int result = 0);
};

class SaveRestoreDialog : public Dialog {
private:
	int _mode, _selectedSlot;
	byte _editBuffer[SAVE_DESCRIPTION_LEN];
	int _editPos, _firstPos;
	int _cursorTick;

	FontRendererGui *_fr1;
	FontRendererGui *_fr2;
	Widget *_panel;
	Slot *_slotButton[8];
	ScrollButton *_zupButton;
	ScrollButton *_upButton;
	ScrollButton *_downButton;
	ScrollButton *_zdownButton;
	Button *_okButton;
	Button *_cancelButton;

public:
	SaveRestoreDialog(Sword2Engine *vm, int mode);
	~SaveRestoreDialog();

	void updateSlots();
	void drawEditBuffer(Slot *slot);

	virtual void onAction(Widget *widget, int result = 0);
	virtual void paint();
	virtual void setResult(int result);
	virtual int runModal();
};

/**
 * A "mini" dialog is usually a yes/no question, but also used for the
 * restart/restore dialog at the beginning of the game.
 */

class MiniDialog : public Dialog {
private:
	uint32 _headerTextId;
	uint32 _okTextId;
	uint32 _cancelTextId;
	FontRendererGui *_fr;
	Widget *_panel;
	Button *_okButton;
	Button *_cancelButton;

public:
	MiniDialog(Sword2Engine *vm, uint32 headerTextId, uint32 okTextId = TEXT_OK, uint32 cancelTextId = TEXT_CANCEL);
	virtual ~MiniDialog();
	virtual void paint();
	virtual void onAction(Widget *widget, int result = 0);
};

class StartDialog : public MiniDialog {
public:
	StartDialog(Sword2Engine *vm);
	virtual int runModal();
};

class RestartDialog : public MiniDialog {
public:
	RestartDialog(Sword2Engine *vm);
	virtual int runModal();
};

class QuitDialog : public MiniDialog {
public:
	QuitDialog(Sword2Engine *vm);
	virtual int runModal();
};

class SaveDialog : public SaveRestoreDialog {
public:
	SaveDialog(Sword2Engine *vm) : SaveRestoreDialog(vm, kSaveDialog) {}
};

class RestoreDialog : public SaveRestoreDialog {
public:
	RestoreDialog(Sword2Engine *vm) : SaveRestoreDialog(vm, kRestoreDialog) {}
};

} // End of namespace Sword2

#endif
