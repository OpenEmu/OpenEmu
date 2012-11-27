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

#include "gui/saveload-dialog.h"
#include "common/translation.h"
#include "common/config-manager.h"

#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widgets/edittext.h"

#include "graphics/scaler.h"

namespace GUI {

#ifndef DISABLE_SAVELOADCHOOSER_GRID
SaveLoadChooserType getRequestedSaveLoadDialog(const MetaEngine &metaEngine) {
	const Common::String &userConfig = ConfMan.get("gui_saveload_chooser", Common::ConfigManager::kApplicationDomain);

	// Check (and update if necessary) the theme config here. This catches
	// resolution changes, which happened after the GUI was closed. This
	// should assure that the correct GUI width/height are returned below and
	// prevent the logic from picking the grid dialog, even though it is not
	// possible to use it.
	g_gui.checkScreenChange();

	if (g_gui.getWidth() >= 640 && g_gui.getHeight() >= 400
	    && metaEngine.hasFeature(MetaEngine::kSavesSupportMetaInfo)
	    && metaEngine.hasFeature(MetaEngine::kSavesSupportThumbnail)
	    && userConfig.equalsIgnoreCase("grid")) {
		// In case we are 640x400 or higher, this dialog is not in save mode,
		// the user requested the grid dialog and the engines supports it we
		// try to set it up.
		return kSaveLoadDialogGrid;
	} else {
		// In all other cases we want to use the list dialog.
		return kSaveLoadDialogList;
	}
}

enum {
	kListSwitchCmd = 'LIST',
	kGridSwitchCmd = 'GRID'
};
#endif // !DISABLE_SAVELOADCHOOSER_GRID

SaveLoadChooserDialog::SaveLoadChooserDialog(const Common::String &dialogName, const bool saveMode)
	: Dialog(dialogName), _metaEngine(0), _delSupport(false), _metaInfoSupport(false),
	_thumbnailSupport(false), _saveDateSupport(false), _playTimeSupport(false), _saveMode(saveMode)
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	, _listButton(0), _gridButton(0)
#endif // !DISABLE_SAVELOADCHOOSER_GRID
	{
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	addChooserButtons();
#endif // !DISABLE_SAVELOADCHOOSER_GRID
}

SaveLoadChooserDialog::SaveLoadChooserDialog(int x, int y, int w, int h, const bool saveMode)
	: Dialog(x, y, w, h), _metaEngine(0), _delSupport(false), _metaInfoSupport(false),
	_thumbnailSupport(false), _saveDateSupport(false), _playTimeSupport(false), _saveMode(saveMode)
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	, _listButton(0), _gridButton(0)
#endif // !DISABLE_SAVELOADCHOOSER_GRID
	{
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	addChooserButtons();
#endif // !DISABLE_SAVELOADCHOOSER_GRID
}

void SaveLoadChooserDialog::open() {
	Dialog::open();

	// So that quitting ScummVM will not cause the dialog result to say a
	// savegame was selected.
	setResult(-1);
}

int SaveLoadChooserDialog::run(const Common::String &target, const MetaEngine *metaEngine) {
	_metaEngine = metaEngine;
	_target = target;
	_delSupport = _metaEngine->hasFeature(MetaEngine::kSupportsDeleteSave);
	_metaInfoSupport = _metaEngine->hasFeature(MetaEngine::kSavesSupportMetaInfo);
	_thumbnailSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportThumbnail);
	_saveDateSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportCreationDate);
	_playTimeSupport = _metaInfoSupport && _metaEngine->hasFeature(MetaEngine::kSavesSupportPlayTime);

	return runIntern();
}

void SaveLoadChooserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	switch (cmd) {
	case kListSwitchCmd:
		setResult(kSwitchSaveLoadDialog);
		// We save the requested dialog type here to avoid the setting to be
		// overwritten when our reflowLayout logic selects a different dialog
		// type.
		ConfMan.set("gui_saveload_chooser", "list", Common::ConfigManager::kApplicationDomain);
		close();
		break;

	case kGridSwitchCmd:
		setResult(kSwitchSaveLoadDialog);
		// See above.
		ConfMan.set("gui_saveload_chooser", "grid", Common::ConfigManager::kApplicationDomain);
		close();
		break;

	default:
		break;
	}
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	return Dialog::handleCommand(sender, cmd, data);
}

void SaveLoadChooserDialog::reflowLayout() {
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	addChooserButtons();

	const SaveLoadChooserType currentType = getType();
	const SaveLoadChooserType requestedType = getRequestedSaveLoadDialog(*_metaEngine);

	// Change the dialog type if there is any need for it.
	if (requestedType != currentType) {
		setResult(kSwitchSaveLoadDialog);
		close();
	}
#endif // !DISABLE_SAVELOADCHOOSER_GRID

	Dialog::reflowLayout();
}

#ifndef DISABLE_SAVELOADCHOOSER_GRID
void SaveLoadChooserDialog::addChooserButtons() {
	if (_listButton) {
		removeWidget(_listButton);
		delete _listButton;
	}

	if (_gridButton) {
		removeWidget(_gridButton);
		delete _gridButton;
	}

	_listButton = createSwitchButton("SaveLoadChooser.ListSwitch", "L", _("List view"), ThemeEngine::kImageList, kListSwitchCmd);
	_gridButton = createSwitchButton("SaveLoadChooser.GridSwitch", "G", _("Grid view"), ThemeEngine::kImageGrid, kGridSwitchCmd);
	if (!_metaInfoSupport || !_thumbnailSupport || !(g_gui.getWidth() >= 640 && g_gui.getHeight() >= 400)) {
		_gridButton->setEnabled(false);
		_listButton->setEnabled(false);
	}
}

ButtonWidget *SaveLoadChooserDialog::createSwitchButton(const Common::String &name, const char *desc, const char *tooltip, const char *image, uint32 cmd) {
	ButtonWidget *button;

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowChooserPics") == 1 && g_gui.theme()->supportsImages()) {
		button = new PicButtonWidget(this, name, tooltip, cmd);
		((PicButtonWidget *)button)->useThemeTransparency(true);
		((PicButtonWidget *)button)->setGfx(g_gui.theme()->getImageSurface(image));
	} else
#endif
		button = new ButtonWidget(this, name, desc, tooltip, cmd);

	return button;
}
#endif // !DISABLE_SAVELOADCHOOSER_GRID

// SaveLoadChooserSimple implementation

enum {
	kChooseCmd = 'CHOS',
	kDelCmd = 'DEL '
};

SaveLoadChooserSimple::SaveLoadChooserSimple(const String &title, const String &buttonLabel, bool saveMode)
	: SaveLoadChooserDialog("SaveLoadChooser", saveMode), _list(0), _chooseButton(0), _deleteButton(0), _gfxWidget(0)  {
	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	new StaticTextWidget(this, "SaveLoadChooser.Title", title);

	// Add choice list
	_list = new ListWidget(this, "SaveLoadChooser.List");
	_list->setNumberingMode(kListNumberingZero);
	_list->setEditable(saveMode);

	_gfxWidget = new GraphicsWidget(this, 0, 0, 10, 10);

	_date = new StaticTextWidget(this, 0, 0, 10, 10, _("No date saved"), Graphics::kTextAlignCenter);
	_time = new StaticTextWidget(this, 0, 0, 10, 10, _("No time saved"), Graphics::kTextAlignCenter);
	_playtime = new StaticTextWidget(this, 0, 0, 10, 10, _("No playtime saved"), Graphics::kTextAlignCenter);

	// Buttons
	new ButtonWidget(this, "SaveLoadChooser.Cancel", _("Cancel"), 0, kCloseCmd);
	_chooseButton = new ButtonWidget(this, "SaveLoadChooser.Choose", buttonLabel, 0, kChooseCmd);
	_chooseButton->setEnabled(false);

	_deleteButton = new ButtonWidget(this, "SaveLoadChooser.Delete", _("Delete"), 0, kDelCmd);
	_deleteButton->setEnabled(false);

	_delSupport = _metaInfoSupport = _thumbnailSupport = false;

	_container = new ContainerWidget(this, 0, 0, 10, 10);
//	_container->setHints(THEME_HINT_USE_SHADOW);
}

int SaveLoadChooserSimple::runIntern() {
	if (_gfxWidget)
		_gfxWidget->setGfx(0);

	_resultString.clear();
	reflowLayout();
	updateSaveList();

	return Dialog::runModal();
}

const Common::String &SaveLoadChooserSimple::getResultString() const {
	int selItem = _list->getSelected();
	return (selItem >= 0) ? _list->getSelectedString() : _resultString;
}

void SaveLoadChooserSimple::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int selItem = _list->getSelected();

	switch (cmd) {
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		if (selItem >= 0 && _chooseButton->isEnabled()) {
			if (_list->isEditable() || !_list->getSelectedString().empty()) {
				_list->endEditMode();
				if (!_saveList.empty()) {
					setResult(_saveList[selItem].getSaveSlot());
					_resultString = _list->getSelectedString();
				}
				close();
			}
		}
		break;
	case kChooseCmd:
		_list->endEditMode();
		if (!_saveList.empty()) {
			setResult(_saveList[selItem].getSaveSlot());
			_resultString = _list->getSelectedString();
		}
		close();
		break;
	case kListSelectionChangedCmd:
		updateSelection(true);
		break;
	case kDelCmd:
		if (selItem >= 0 && _delSupport) {
			MessageDialog alert(_("Do you really want to delete this savegame?"),
								_("Delete"), _("Cancel"));
			if (alert.runModal() == kMessageOK) {
				_metaEngine->removeSaveState(_target.c_str(), _saveList[selItem].getSaveSlot());

				setResult(-1);
				_list->setSelected(-1);

				updateSaveList();
				updateSelection(true);
			}
		}
		break;
	case kCloseCmd:
		setResult(-1);
	default:
		SaveLoadChooserDialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooserSimple::reflowLayout() {
	if (g_gui.xmlEval()->getVar("Globals.SaveLoadChooser.ExtInfo.Visible") == 1 && _thumbnailSupport) {
		int16 x, y;
		uint16 w, h;

		if (!g_gui.xmlEval()->getWidgetData("SaveLoadChooser.Thumbnail", x, y, w, h))
			error("Error when loading position data for Save/Load Thumbnails");

		int thumbW = kThumbnailWidth;
		int thumbH = kThumbnailHeight2;
		int thumbX = x + (w >> 1) - (thumbW >> 1);
		int thumbY = y + kLineHeight;

		int textLines = 0;
		if (!_saveDateSupport)
			textLines++;
		if (!_playTimeSupport)
			textLines++;

		_container->resize(x, y, w, h - (kLineHeight * textLines));
		_gfxWidget->resize(thumbX, thumbY, thumbW, thumbH);

		int height = thumbY + thumbH + kLineHeight;

		if (_saveDateSupport) {
			_date->resize(thumbX, height, kThumbnailWidth, kLineHeight);
			height += kLineHeight;
			_time->resize(thumbX, height, kThumbnailWidth, kLineHeight);
			height += kLineHeight;
		}

		if (_playTimeSupport)
			_playtime->resize(thumbX, height, kThumbnailWidth, kLineHeight);

		_container->setVisible(true);
		_gfxWidget->setVisible(true);

		_date->setVisible(_saveDateSupport);
		_time->setVisible(_saveDateSupport);

		_playtime->setVisible(_playTimeSupport);

		updateSelection(false);
	} else {
		_container->setVisible(false);
		_gfxWidget->setVisible(false);
		_date->setVisible(false);
		_time->setVisible(false);
		_playtime->setVisible(false);
	}

	SaveLoadChooserDialog::reflowLayout();
}

void SaveLoadChooserSimple::updateSelection(bool redraw) {
	int selItem = _list->getSelected();

	bool isDeletable = _delSupport;
	bool isWriteProtected = false;
	bool startEditMode = _list->isEditable();

	// We used to support letting the themes specify the fill color with our
	// initial theme based GUI. But this support was dropped.
	_gfxWidget->setGfx(-1, -1, 0, 0, 0);
	_date->setLabel(_("No date saved"));
	_time->setLabel(_("No time saved"));
	_playtime->setLabel(_("No playtime saved"));

	if (selItem >= 0 && _metaInfoSupport) {
		SaveStateDescriptor desc = _metaEngine->querySaveMetaInfos(_target.c_str(), _saveList[selItem].getSaveSlot());

		isDeletable = desc.getDeletableFlag() && _delSupport;
		isWriteProtected = desc.getWriteProtectedFlag();

		// Don't allow the user to change the description of write protected games
		if (isWriteProtected)
			startEditMode = false;

		if (_thumbnailSupport) {
			const Graphics::Surface *thumb = desc.getThumbnail();
			if (thumb) {
				_gfxWidget->setGfx(thumb);
				_gfxWidget->useAlpha(256);
			}
		}

		if (_saveDateSupport) {
			const Common::String &saveDate = desc.getSaveDate();
			if (!saveDate.empty())
				_date->setLabel(_("Date: ") + saveDate);

			const Common::String &saveTime = desc.getSaveTime();
			if (!saveTime.empty())
				_time->setLabel(_("Time: ") + saveTime);
		}

		if (_playTimeSupport) {
			const Common::String &playTime = desc.getPlayTime();
			if (!playTime.empty())
				_playtime->setLabel(_("Playtime: ") + playTime);
		}
	}


	if (_list->isEditable()) {
		// Disable the save button if nothing is selected, or if the selected
		// game is write protected
		_chooseButton->setEnabled(selItem >= 0 && !isWriteProtected);

		if (startEditMode) {
			_list->startEditMode();

			if (_chooseButton->isEnabled() && _list->getSelectedString() == _("Untitled savestate") &&
					_list->getSelectionColor() == ThemeEngine::kFontColorAlternate) {
				_list->setEditString("");
				_list->setEditColor(ThemeEngine::kFontColorNormal);
			}
		}
	} else {
		// Disable the load button if nothing is selected, or if an empty
		// list item is selected.
		_chooseButton->setEnabled(selItem >= 0 && !_list->getSelectedString().empty());
	}

	// Delete will always be disabled if the engine doesn't support it.
	_deleteButton->setEnabled(isDeletable && (selItem >= 0) && (!_list->getSelectedString().empty()));

	if (redraw) {
		_gfxWidget->draw();
		_date->draw();
		_time->draw();
		_playtime->draw();
		_chooseButton->draw();
		_deleteButton->draw();

		draw();
	}
}

void SaveLoadChooserSimple::open() {
	SaveLoadChooserDialog::open();

	// Scroll the list to the last used entry.
	_list->scrollTo(ConfMan.getInt("gui_saveload_last_pos"));
}

void SaveLoadChooserSimple::close() {
	// Save the current scroll position/used entry.
	const int result = getResult();
	if (result >= 0) {
		ConfMan.setInt("gui_saveload_last_pos", result);
	} else {
		// Use the current scroll position here.
		// TODO: This means we canceled the dialog (or switch to the grid). Do
		// we want to save this position here? Does the user want that?
		// TODO: Do we want to save the current scroll position or the
		// currently selected item here? The scroll position is what the user
		// currently sees and seems to make more sense.
		ConfMan.setInt("gui_saveload_last_pos", _list->getCurrentScrollPos());
	}

	_metaEngine = 0;
	_target.clear();
	_saveList.clear();
	_list->setList(StringArray());

	SaveLoadChooserDialog::close();
}

void SaveLoadChooserSimple::updateSaveList() {
	_saveList = _metaEngine->listSaves(_target.c_str());

	int curSlot = 0;
	int saveSlot = 0;
	StringArray saveNames;
	ListWidget::ColorList colors;
	for (SaveStateList::const_iterator x = _saveList.begin(); x != _saveList.end(); ++x) {
		// Handle gaps in the list of save games
		saveSlot = x->getSaveSlot();
		if (curSlot < saveSlot) {
			while (curSlot < saveSlot) {
				SaveStateDescriptor dummySave(curSlot, "");
				_saveList.insert_at(curSlot, dummySave);
				saveNames.push_back(dummySave.getDescription());
				colors.push_back(ThemeEngine::kFontColorNormal);
				curSlot++;
			}

			// Sync the save list iterator
			for (x = _saveList.begin(); x != _saveList.end(); ++x) {
				if (x->getSaveSlot() == saveSlot)
					break;
			}
		}

		// Show "Untitled savestate" for empty/whitespace savegame descriptions
		Common::String description = x->getDescription();
		Common::String trimmedDescription = description;
		trimmedDescription.trim();
		if (trimmedDescription.empty()) {
			description = _("Untitled savestate");
			colors.push_back(ThemeEngine::kFontColorAlternate);
		} else {
			colors.push_back(ThemeEngine::kFontColorNormal);
		}

		saveNames.push_back(description);
		curSlot++;
	}

	// Fill the rest of the save slots with empty saves

	int maximumSaveSlots = _metaEngine->getMaximumSaveSlot();

#ifdef __DS__
	// Low memory on the DS means too many save slots are impractical, so limit
	// the maximum here.
	if (maximumSaveSlots > 99) {
		maximumSaveSlots = 99;
	}
#endif

	Common::String emptyDesc;
	for (int i = curSlot; i <= maximumSaveSlots; i++) {
		saveNames.push_back(emptyDesc);
		SaveStateDescriptor dummySave(i, "");
		_saveList.push_back(dummySave);
		colors.push_back(ThemeEngine::kFontColorNormal);
	}

	_list->setList(saveNames, &colors);
}

// SaveLoadChooserGrid implementation

#ifndef DISABLE_SAVELOADCHOOSER_GRID

enum {
	kNextCmd = 'NEXT',
	kPrevCmd = 'PREV',
	kNewSaveCmd = 'SAVE'
};

SaveLoadChooserGrid::SaveLoadChooserGrid(const Common::String &title, bool saveMode)
	: SaveLoadChooserDialog("SaveLoadChooser", saveMode), _lines(0), _columns(0), _entriesPerPage(0),
	_curPage(0), _newSaveContainer(0), _nextFreeSaveSlot(0), _buttons() {
	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	new StaticTextWidget(this, "SaveLoadChooser.Title", title);

	// Buttons
	new ButtonWidget(this, "SaveLoadChooser.Delete", _("Cancel"), 0, kCloseCmd);
	_nextButton = new ButtonWidget(this, "SaveLoadChooser.Choose", _("Next"), 0, kNextCmd);
	_nextButton->setEnabled(false);

	_prevButton = new ButtonWidget(this, "SaveLoadChooser.Cancel", _("Prev"), 0, kPrevCmd);
	_prevButton->setEnabled(false);

	// Page display
	_pageDisplay = new StaticTextWidget(this, "SaveLoadChooser.PageDisplay", Common::String());
	_pageDisplay->setAlign(Graphics::kTextAlignRight);
}

SaveLoadChooserGrid::~SaveLoadChooserGrid() {
	removeWidget(_pageDisplay);
	delete _pageDisplay;
}

const Common::String &SaveLoadChooserGrid::getResultString() const {
	return _resultString;
}

void SaveLoadChooserGrid::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd <= _entriesPerPage && cmd + _curPage * _entriesPerPage <= _saveList.size()) {
		const SaveStateDescriptor &desc = _saveList[cmd - 1 + _curPage * _entriesPerPage];

		if (_saveMode) {
			_resultString = desc.getDescription();
		}

		setResult(desc.getSaveSlot());
		close();
	}

	switch (cmd) {
	case kNextCmd:
		++_curPage;
		updateSaves();
		draw();
		break;

	case kPrevCmd:
		--_curPage;
		updateSaves();
		draw();
		break;

	case kNewSaveCmd:
		setResult(_nextFreeSaveSlot);
		close();
		break;

	case kCloseCmd:
		setResult(-1);
	default:
		SaveLoadChooserDialog::handleCommand(sender, cmd, data);
	}
}

void SaveLoadChooserGrid::handleMouseWheel(int x, int y, int direction) {
	if (direction > 0) {
		if (_nextButton->isEnabled()) {
			++_curPage;
			updateSaves();
			draw();
		}
	} else {
		if (_prevButton->isEnabled()) {
			--_curPage;
			updateSaves();
			draw();
		}
	}
}

void SaveLoadChooserGrid::open() {
	SaveLoadChooserDialog::open();

	_saveList = _metaEngine->listSaves(_target.c_str());
	_resultString.clear();

	// Load information to restore the last page the user had open.
	assert(_entriesPerPage != 0);
	const uint lastPos = ConfMan.getInt("gui_saveload_last_pos");
	const uint listSize = _saveList.size();
	uint bestMatch = 0;
	uint diff = 0xFFFFFFFF;

	// We look for the nearest available slot, since a slot might be missing
	// due to the user deleting it via the list based chooser, by deleting
	// it by hand, etc.
	for (uint i = 0; i < listSize; ++i) {
		uint curDiff = ABS(_saveList[i].getSaveSlot() - (int)lastPos);
		if (curDiff < diff) {
			diff = curDiff;
			bestMatch = i;
		}
	}

	_curPage = bestMatch / _entriesPerPage;

	// Determine the next free save slot for save mode
	if (_saveMode) {
		int lastSlot = -1;
		_nextFreeSaveSlot = -1;
		for (SaveStateList::const_iterator x = _saveList.begin(); x != _saveList.end(); ++x) {
			const int curSlot = x->getSaveSlot();

			// In case there was a gap found use the slot.
			if (lastSlot + 1 < curSlot) {
				_nextFreeSaveSlot = lastSlot + 1;
				break;
			}

			lastSlot = curSlot;
		}

		// Use the next available slot otherwise.
		if (_nextFreeSaveSlot == -1 && lastSlot + 1 < _metaEngine->getMaximumSaveSlot()) {
			_nextFreeSaveSlot = lastSlot + 1;
		}
	}

	updateSaves();
}

void SaveLoadChooserGrid::reflowLayout() {
	// HACK: The page display is not available in low resolution layout. We
	// remove and readd the widget here to avoid our GUI from erroring out.
	removeWidget(_pageDisplay);
	if (g_gui.xmlEval()->getVar("Globals.ShowChooserPageDisplay") == 1) {
		_pageDisplay->init();
	}

	SaveLoadChooserDialog::reflowLayout();
	destroyButtons();

	// HACK: The whole code below really works around the fact, that we have
	// no easy way to dynamically layout widgets.
	const uint16 availableWidth = getWidth() - 20;
	uint16 availableHeight;

	int16 x, y;
	uint16 w;
	g_gui.xmlEval()->getWidgetData("SaveLoadChooser.List", x, y, w, availableHeight);

	const int16 buttonWidth = kThumbnailWidth + 6;
	const int16 buttonHeight = kThumbnailHeight2 + 6;

	const int16 containerFrameWidthAdd = 10;
	const int16 containerFrameHeightAdd = 0;
	const int16 containerWidth = buttonWidth + containerFrameWidthAdd;
	const int16 containerHeight = buttonHeight + kLineHeight + containerFrameHeightAdd;

	const int16 defaultSpacingHorizontal = 4;
	const int16 defaultSpacingVertical = 8;
	const int16 slotAreaWidth = containerWidth + defaultSpacingHorizontal;
	const int16 slotAreaHeight = containerHeight + defaultSpacingVertical;

	const uint oldEntriesPerPage = _entriesPerPage;
	_columns = MAX<uint>(1, availableWidth / slotAreaWidth);
	_lines = MAX<uint>(1, availableHeight / slotAreaHeight);
	_entriesPerPage = _columns * _lines;

	// In save mode the first button is always "New Save", thus we need to
	// adjust the entries per page here.
	if (_saveMode) {
		--_entriesPerPage;
	}

	// Recalculate the page number
	if (!_saveList.empty() && oldEntriesPerPage != 0) {
		if (_entriesPerPage != 0) {
			_curPage = (_curPage * oldEntriesPerPage) / _entriesPerPage;
		} else {
			_curPage = 0;
		}
	}

	const uint addX = _columns > 1 ? (availableWidth % slotAreaWidth) / (_columns - 1) : 0;
	//const uint addY = _lines > 1 ? (availableHeight % slotAreaHeight) / (_lines - 1) : 0;

	_buttons.reserve(_lines * _columns);
	y += defaultSpacingVertical / 2;
	for (uint curLine = 0; curLine < _lines; ++curLine, y += slotAreaHeight/* + addY*/) {
		for (uint curColumn = 0, curX = x + defaultSpacingHorizontal / 2; curColumn < _columns; ++curColumn, curX += slotAreaWidth + addX) {
			int dstY = containerFrameHeightAdd / 2;
			int dstX = containerFrameWidthAdd / 2;

			// In the save mode we will always create a new save button as the first button.
			if (_saveMode && curLine == 0 && curColumn == 0) {
				_newSaveContainer = new ContainerWidget(this, curX, y, containerWidth, containerHeight);
				ButtonWidget *newSave = new ButtonWidget(_newSaveContainer, dstX, dstY, buttonWidth, buttonHeight, _("New Save"), _("Create a new save game"), kNewSaveCmd);
				// In case no more slots are free, we will disable the new save button
				if (_nextFreeSaveSlot == -1) {
					newSave->setEnabled(false);
				}
				continue;
			}

			ContainerWidget *container = new ContainerWidget(this, curX, y, containerWidth, containerHeight);
			container->setVisible(false);

			// Command 0 cannot be used, since it won't be send. Thus we will adjust
			// command number here, if required. This is only the case for load mode
			// since for save mode, the first button used is index 1 anyway.
			uint buttonCmd = curLine * _columns + curColumn;
			if (!_saveMode) {
				buttonCmd += 1;
			}

			PicButtonWidget *button = new PicButtonWidget(container, dstX, dstY, buttonWidth, buttonHeight, 0, buttonCmd);
			dstY += buttonHeight;

			StaticTextWidget *description = new StaticTextWidget(container, dstX, dstY, buttonWidth, kLineHeight, Common::String(), Graphics::kTextAlignLeft);

			_buttons.push_back(SlotButton(container, button, description));
		}
	}

	if (!_target.empty())
		updateSaves();
}

void SaveLoadChooserGrid::close() {
	// Save the current page.
	const int result = getResult();
	if (result >= 0 && result != _nextFreeSaveSlot) {
		// If the user selected a slot we use that one. We ignore new slots
		// here, since otherwise the dialog would reset to page 0 when the
		// user cancels the savename dialog.
		ConfMan.setInt("gui_saveload_last_pos", result);
	} else {
		// Otherwise save the first entry on the current page.
		// This is less precise than the solution above, since the number of
		// entries shown differs between save and load version of the dialog,
		// thus it might wrap to a different page than expected.
		// Similar things happen on resolution changes.
		// TODO: Should we ignore this here? Is the user likely to be
		// interested in having this page restored when he canceled?
		ConfMan.setInt("gui_saveload_last_pos", !_saveList.empty() ? _saveList[_curPage * _entriesPerPage].getSaveSlot() : 0);
	}

	SaveLoadChooserDialog::close();
	hideButtons();
}

int SaveLoadChooserGrid::runIntern() {
	int slot;
	do {
		const SaveLoadChooserType currentType = getType();
		const SaveLoadChooserType requestedType = getRequestedSaveLoadDialog(*_metaEngine);

		// Catch resolution changes when the save name dialog was open.
		if (currentType != requestedType) {
			setResult(kSwitchSaveLoadDialog);
			return kSwitchSaveLoadDialog;
		}

		slot = runModal();
	} while (_saveMode && slot >= 0 && !selectDescription());

	// Special case for new save games. We need to handle this here, since
	// we cannot handle it in close() without problems.
	if (slot == _nextFreeSaveSlot) {
		ConfMan.setInt("gui_saveload_last_pos", slot);
	}

	return slot;
}

bool SaveLoadChooserGrid::selectDescription() {
	_savenameDialog.setDescription(_resultString);
	_savenameDialog.setTargetSlot(getResult());
	if (_savenameDialog.runModal() == 0) {
		_resultString = _savenameDialog.getDescription();
		return true;
	} else {
		return false;
	}
}

void SaveLoadChooserGrid::destroyButtons() {
	if (_newSaveContainer) {
		removeWidget(_newSaveContainer);
		delete _newSaveContainer;
		_newSaveContainer = 0;
	}

	for (ButtonArray::iterator i = _buttons.begin(), end = _buttons.end(); i != end; ++i) {
		removeWidget(i->container);
		delete i->container;
	}

	_buttons.clear();
}

void SaveLoadChooserGrid::hideButtons() {
	for (ButtonArray::iterator i = _buttons.begin(), end = _buttons.end(); i != end; ++i) {
		i->button->setGfx(0);
		i->setVisible(false);
	}
}

void SaveLoadChooserGrid::updateSaves() {
	hideButtons();

	for (uint i = _curPage * _entriesPerPage, curNum = 0; i < _saveList.size() && curNum < _entriesPerPage; ++i, ++curNum) {
		const uint saveSlot = _saveList[i].getSaveSlot();

		SaveStateDescriptor desc = _metaEngine->querySaveMetaInfos(_target.c_str(), saveSlot);
		SlotButton &curButton = _buttons[curNum];
		curButton.setVisible(true);
		const Graphics::Surface *thumbnail = desc.getThumbnail();
		if (thumbnail) {
			curButton.button->setGfx(desc.getThumbnail());
		} else {
			curButton.button->setGfx(kThumbnailWidth, kThumbnailHeight2, 0, 0, 0);
		}
		curButton.description->setLabel(Common::String::format("%d. %s", saveSlot, desc.getDescription().c_str()));

		Common::String tooltip(_("Name: "));
		tooltip += desc.getDescription();

		if (_saveDateSupport) {
			const Common::String &saveDate = desc.getSaveDate();
			if (!saveDate.empty()) {
				tooltip += "\n";
				tooltip +=  _("Date: ") + saveDate;
			}

			const Common::String &saveTime = desc.getSaveTime();
			if (!saveTime.empty()) {
				tooltip += "\n";
				tooltip += _("Time: ") + saveTime;
			}
		}

		if (_playTimeSupport) {
			const Common::String &playTime = desc.getPlayTime();
			if (!playTime.empty()) {
				tooltip += "\n";
				tooltip += _("Playtime: ") + playTime;
			}
		}

		curButton.button->setTooltip(tooltip);

		// In save mode we disable the button, when it's write protected.
		// TODO: Maybe we should not display it at all then?
		if (_saveMode && desc.getWriteProtectedFlag()) {
			curButton.button->setEnabled(false);
		} else {
			curButton.button->setEnabled(true);
		}
	}

	const uint numPages = (_entriesPerPage != 0 && !_saveList.empty()) ? ((_saveList.size() + _entriesPerPage - 1) / _entriesPerPage) : 1;
	_pageDisplay->setLabel(Common::String::format("%u/%u", _curPage + 1, numPages));

	if (_curPage > 0)
		_prevButton->setEnabled(true);
	else
		_prevButton->setEnabled(false);

	if ((_curPage + 1) * _entriesPerPage < _saveList.size())
		_nextButton->setEnabled(true);
	else
		_nextButton->setEnabled(false);
}

SavenameDialog::SavenameDialog()
	: Dialog("SavenameDialog") {
	_title = new StaticTextWidget(this, "SavenameDialog.DescriptionText", Common::String());

	new ButtonWidget(this, "SavenameDialog.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "SavenameDialog.Ok", _("OK"), 0, kOKCmd);

	_description = new EditTextWidget(this, "SavenameDialog.Description", Common::String(), 0, 0, kOKCmd);
}

void SavenameDialog::setDescription(const Common::String &desc) {
	_description->setEditString(desc);
}

const Common::String &SavenameDialog::getDescription() {
	return _description->getEditString();
}

void SavenameDialog::open() {
	Dialog::open();
	setResult(-1);

	_title->setLabel(Common::String::format(_("Enter a description for slot %d:"), _targetSlot));
}

void SavenameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kOKCmd:
		setResult(0);
		close();
		break;

	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

#endif // !DISABLE_SAVELOADCHOOSER_GRID

} // End of namespace GUI
