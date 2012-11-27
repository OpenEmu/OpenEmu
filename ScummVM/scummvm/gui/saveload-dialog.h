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

#ifndef GUI_SAVELOAD_DIALOG_H
#define GUI_SAVELOAD_DIALOG_H

#include "gui/dialog.h"
#include "gui/widgets/list.h"

#include "engines/metaengine.h"

namespace GUI {

#define kSwitchSaveLoadDialog -2

// TODO: We might want to disable the grid based save/load chooser for more
// platforms, than those which define DISABLE_FANCY_THEMES. But those are
// probably not able to handle the grid chooser anyway, so disabling it
// for them is a good start.
#ifdef DISABLE_FANCY_THEMES
#define DISABLE_SAVELOADCHOOSER_GRID
#endif // DISABLE_FANCY_THEMES

#ifndef DISABLE_SAVELOADCHOOSER_GRID
enum SaveLoadChooserType {
	kSaveLoadDialogList = 0,
	kSaveLoadDialogGrid = 1
};

SaveLoadChooserType getRequestedSaveLoadDialog(const MetaEngine &metaEngine);
#endif // !DISABLE_SAVELOADCHOOSER_GRID

class SaveLoadChooserDialog : protected Dialog {
public:
	SaveLoadChooserDialog(const Common::String &dialogName, const bool saveMode);
	SaveLoadChooserDialog(int x, int y, int w, int h, const bool saveMode);

	virtual void open();

	virtual void reflowLayout();

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

#ifndef DISABLE_SAVELOADCHOOSER_GRID
	virtual SaveLoadChooserType getType() const = 0;
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	int run(const Common::String &target, const MetaEngine *metaEngine);
	virtual const Common::String &getResultString() const = 0;

protected:
	virtual int runIntern() = 0;

	const bool				_saveMode;
	const MetaEngine		*_metaEngine;
	bool					_delSupport;
	bool					_metaInfoSupport;
	bool					_thumbnailSupport;
	bool					_saveDateSupport;
	bool					_playTimeSupport;
	Common::String			_target;

#ifndef DISABLE_SAVELOADCHOOSER_GRID
	ButtonWidget *_listButton;
	ButtonWidget *_gridButton;

	void addChooserButtons();
	ButtonWidget *createSwitchButton(const Common::String &name, const char *desc, const char *tooltip, const char *image, uint32 cmd = 0);
#endif // !DISABLE_SAVELOADCHOOSER_GRID
};

class SaveLoadChooserSimple : public SaveLoadChooserDialog {
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;
public:
	SaveLoadChooserSimple(const String &title, const String &buttonLabel, bool saveMode);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	virtual const Common::String &getResultString() const;

	virtual void reflowLayout();

#ifndef DISABLE_SAVELOADCHOOSER_GRID
	virtual SaveLoadChooserType getType() const { return kSaveLoadDialogList; }
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	virtual void open();
	virtual void close();
private:
	virtual int runIntern();

	ListWidget		*_list;
	ButtonWidget	*_chooseButton;
	ButtonWidget	*_deleteButton;
	GraphicsWidget	*_gfxWidget;
	ContainerWidget	*_container;
	StaticTextWidget	*_date;
	StaticTextWidget	*_time;
	StaticTextWidget	*_playtime;

	SaveStateList			_saveList;
	String					_resultString;

	void updateSaveList();
	void updateSelection(bool redraw);
};

#ifndef DISABLE_SAVELOADCHOOSER_GRID

class EditTextWidget;

class SavenameDialog : public Dialog {
public:
	SavenameDialog();

	void setDescription(const Common::String &desc);
	const Common::String &getDescription();

	void setTargetSlot(int slot) { _targetSlot = slot; }

	virtual void open();
protected:
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
private:
	int _targetSlot;
	StaticTextWidget *_title;
	EditTextWidget *_description;
};

class SaveLoadChooserGrid : public SaveLoadChooserDialog {
public:
	SaveLoadChooserGrid(const Common::String &title, bool saveMode);
	~SaveLoadChooserGrid();

	virtual const Common::String &getResultString() const;

	virtual void open();

	virtual void reflowLayout();

	virtual SaveLoadChooserType getType() const { return kSaveLoadDialogGrid; }

	virtual void close();
protected:
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleMouseWheel(int x, int y, int direction);
private:
	virtual int runIntern();

	uint _columns, _lines;
	uint _entriesPerPage;
	uint _curPage;
	SaveStateList _saveList;

	ButtonWidget *_nextButton;
	ButtonWidget *_prevButton;

	StaticTextWidget *_pageDisplay;

	ContainerWidget *_newSaveContainer;
	int _nextFreeSaveSlot;
	Common::String _resultString;

	SavenameDialog _savenameDialog;
	bool selectDescription();

	struct SlotButton {
		SlotButton() : container(0), button(0), description(0) {}
		SlotButton(ContainerWidget *c, PicButtonWidget *b, StaticTextWidget *d) : container(c), button(b), description(d) {}

		ContainerWidget  *container;
		PicButtonWidget  *button;
		StaticTextWidget *description;

		void setVisible(bool state) {
			container->setVisible(state);
		}
	};
	typedef Common::Array<SlotButton> ButtonArray;
	ButtonArray _buttons;
	void destroyButtons();
	void hideButtons();
	void updateSaves();
};

#endif // !DISABLE_SAVELOADCHOOSER_GRID

} // End of namespace GUI

#endif
