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

#include "base/version.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/util.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/launcher.h"
#include "gui/massadd.h"
#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/options.h"
#include "gui/saveload.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/list.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/popup.h"
#include "gui/ThemeEval.h"

#include "graphics/cursorman.h"

using Common::ConfigManager;

namespace GUI {

enum {
	kStartCmd = 'STRT',
	kAboutCmd = 'ABOU',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kEditGameCmd = 'EDTG',
	kRemoveGameCmd = 'REMG',
	kLoadGameCmd = 'LOAD',
	kQuitCmd = 'QUIT',
	kSearchCmd = 'SRCH',
	kListSearchCmd = 'LSSR',
	kSearchClearCmd = 'SRCL',

	kCmdGlobalGraphicsOverride = 'OGFX',
	kCmdGlobalAudioOverride = 'OSFX',
	kCmdGlobalMIDIOverride = 'OMID',
	kCmdGlobalMT32Override = 'OM32',
	kCmdGlobalVolumeOverride = 'OVOL',

	kCmdChooseSoundFontCmd = 'chsf',

	kCmdExtraBrowser = 'PEXT',
	kCmdExtraPathClear = 'PEXC',
	kCmdGameBrowser = 'PGME',
	kCmdSaveBrowser = 'PSAV',
	kCmdSavePathClear = 'PSAC'
};

/*
 * TODO: Clean up this ugly design: we subclass EditTextWidget to perform
 * input validation. It would be much more elegant to use a decorator pattern,
 * or a validation callback, or something like that.
 */
class DomainEditTextWidget : public EditTextWidget {
public:
	DomainEditTextWidget(GuiObject *boss, const String &name, const String &text, const char *tooltip = 0)
		: EditTextWidget(boss, name, text, tooltip) {
	}

protected:
	bool tryInsertChar(byte c, int pos) {
		if (Common::isAlnum(c) || c == '-' || c == '_') {
			_editString.insertChar(c, pos);
			return true;
		}
		return false;
	}
};

/*
 * A dialog that allows the user to edit a config game entry.
 * TODO: add widgets for some/all of the following
 * - Maybe scaler/graphics mode. But there are two problems:
 *   1) Different backends can have different scalers with different names,
 *      so we first have to add a way to query those... no Ender, I don't
 *      think a bitmasked property() value is nice for this,  because we would
 *      have to add to the bitmask values whenever a backends adds a new scaler).
 *   2) At the time the launcher is running, the GFX backend is already setup.
 *      So when a game is run via the launcher, the custom scaler setting for it won't be
 *      used. So we'd also have to add an API to change the scaler during runtime
 *      (the SDL backend can already do that based on user input, but there is no API
 *      to achieve it)
 *   If the APIs for 1&2 are in place, we can think about adding this to the Edit&Option dialogs
 */

class EditGameDialog : public OptionsDialog {
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;
public:
	EditGameDialog(const String &domain, const String &desc);

	void open();
	void close();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	EditTextWidget *_descriptionWidget;
	DomainEditTextWidget *_domainWidget;

	StaticTextWidget *_gamePathWidget;
	StaticTextWidget *_extraPathWidget;
	StaticTextWidget *_savePathWidget;
	ButtonWidget *_extraPathClearButton;
	ButtonWidget *_savePathClearButton;

	StaticTextWidget *_langPopUpDesc;
	PopUpWidget *_langPopUp;
	StaticTextWidget *_platformPopUpDesc;
	PopUpWidget *_platformPopUp;

	CheckboxWidget *_globalGraphicsOverride;
	CheckboxWidget *_globalAudioOverride;
	CheckboxWidget *_globalMIDIOverride;
	CheckboxWidget *_globalMT32Override;
	CheckboxWidget *_globalVolumeOverride;

	ExtraGuiOptions _engineOptions;
};

EditGameDialog::EditGameDialog(const String &domain, const String &desc)
	: OptionsDialog(domain, "GameOptions") {
	// Retrieve all game specific options.
	const EnginePlugin *plugin = 0;
	// To allow for game domains without a gameid.
	// TODO: Is it intentional that this is still supported?
	String gameId(ConfMan.get("gameid", domain));
	if (gameId.empty())
		gameId = domain;
	// Retrieve the plugin, since we need to access the engine's MetaEngine
	// implementation.
	EngineMan.findGame(gameId, &plugin);
	if (plugin) {
		_engineOptions = (*plugin)->getExtraGuiOptions(domain);
	} else {
		warning("Plugin for target \"%s\" not found! Game specific settings might be missing", domain.c_str());
	}

	// GAME: Path to game data (r/o), extra data (r/o), and save data (r/w)
	String gamePath(ConfMan.get("path", _domain));
	String extraPath(ConfMan.get("extrapath", _domain));
	String savePath(ConfMan.get("savepath", _domain));

	// GAME: Determine the description string
	String description(ConfMan.get("description", domain));
	if (description.empty() && !desc.empty()) {
		description = desc;
	}

	// GUI:  Add tab widget
	TabWidget *tab = new TabWidget(this, "GameOptions.TabWidget");

	//
	// 1) The game tab
	//
	tab->addTab(_("Game"));

	// GUI:  Label & edit widget for the game ID
	if (g_system->getOverlayWidth() > 320)
		new StaticTextWidget(tab, "GameOptions_Game.Id", _("ID:"), _("Short game identifier used for referring to savegames and running the game from the command line"));
	else
		new StaticTextWidget(tab, "GameOptions_Game.Id", _c("ID:", "lowres"), _("Short game identifier used for referring to savegames and running the game from the command line"));
	_domainWidget = new DomainEditTextWidget(tab, "GameOptions_Game.Domain", _domain, _("Short game identifier used for referring to savegames and running the game from the command line"));

	// GUI:  Label & edit widget for the description
	if (g_system->getOverlayWidth() > 320)
		new StaticTextWidget(tab, "GameOptions_Game.Name", _("Name:"), _("Full title of the game"));
	else
		new StaticTextWidget(tab, "GameOptions_Game.Name", _c("Name:", "lowres"), _("Full title of the game"));
	_descriptionWidget = new EditTextWidget(tab, "GameOptions_Game.Desc", description, _("Full title of the game"));

	// Language popup
	_langPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.LangPopupDesc", _("Language:"), _("Language of the game. This will not turn your Spanish game version into English"));
	_langPopUp = new PopUpWidget(tab, "GameOptions_Game.LangPopup", _("Language of the game. This will not turn your Spanish game version into English"));
	_langPopUp->appendEntry(_("<default>"), (uint32)Common::UNK_LANG);
	_langPopUp->appendEntry("", (uint32)Common::UNK_LANG);
	const Common::LanguageDescription *l = Common::g_languages;
	for (; l->code; ++l) {
		if (checkGameGUIOptionLanguage(l->id, _guioptionsString))
			_langPopUp->appendEntry(l->description, l->id);
	}

	// Platform popup
	if (g_system->getOverlayWidth() > 320)
		_platformPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.PlatformPopupDesc", _("Platform:"), _("Platform the game was originally designed for"));
	else
		_platformPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.PlatformPopupDesc", _c("Platform:", "lowres"), _("Platform the game was originally designed for"));
	_platformPopUp = new PopUpWidget(tab, "GameOptions_Game.PlatformPopup", _("Platform the game was originally designed for"));
	_platformPopUp->appendEntry(_("<default>"));
	_platformPopUp->appendEntry("");
	const Common::PlatformDescription *p = Common::g_platforms;
	for (; p->code; ++p) {
		_platformPopUp->appendEntry(p->description, p->id);
	}

	//
	// 2) The engine tab (shown only if there are custom engine options)
	//
	if (_engineOptions.size() > 0) {
		tab->addTab(_("Engine"));

		addEngineControls(tab, "GameOptions_Engine.", _engineOptions);
	}

	//
	// 3) The graphics tab
	//
	_graphicsTabId = tab->addTab(g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"));

	if (g_system->getOverlayWidth() > 320)
		_globalGraphicsOverride = new CheckboxWidget(tab, "GameOptions_Graphics.EnableTabCheckbox", _("Override global graphic settings"), 0, kCmdGlobalGraphicsOverride);
	else
		_globalGraphicsOverride = new CheckboxWidget(tab, "GameOptions_Graphics.EnableTabCheckbox", _c("Override global graphic settings", "lowres"), 0, kCmdGlobalGraphicsOverride);

	addGraphicControls(tab, "GameOptions_Graphics.");

	//
	// 4) The audio tab
	//
	tab->addTab(_("Audio"));

	if (g_system->getOverlayWidth() > 320)
		_globalAudioOverride = new CheckboxWidget(tab, "GameOptions_Audio.EnableTabCheckbox", _("Override global audio settings"), 0, kCmdGlobalAudioOverride);
	else
		_globalAudioOverride = new CheckboxWidget(tab, "GameOptions_Audio.EnableTabCheckbox", _c("Override global audio settings", "lowres"), 0, kCmdGlobalAudioOverride);

	addAudioControls(tab, "GameOptions_Audio.");
	addSubtitleControls(tab, "GameOptions_Audio.");

	//
	// 5) The volume tab
	//
	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Volume"));
	else
		tab->addTab(_c("Volume", "lowres"));

	if (g_system->getOverlayWidth() > 320)
		_globalVolumeOverride = new CheckboxWidget(tab, "GameOptions_Volume.EnableTabCheckbox", _("Override global volume settings"), 0, kCmdGlobalVolumeOverride);
	else
		_globalVolumeOverride = new CheckboxWidget(tab, "GameOptions_Volume.EnableTabCheckbox", _c("Override global volume settings", "lowres"), 0, kCmdGlobalVolumeOverride);

	addVolumeControls(tab, "GameOptions_Volume.");

	//
	// 6) The MIDI tab
	//
	if (!_guioptions.contains(GUIO_NOMIDI)) {
		tab->addTab(_("MIDI"));

		if (g_system->getOverlayWidth() > 320)
			_globalMIDIOverride = new CheckboxWidget(tab, "GameOptions_MIDI.EnableTabCheckbox", _("Override global MIDI settings"), 0, kCmdGlobalMIDIOverride);
		else
			_globalMIDIOverride = new CheckboxWidget(tab, "GameOptions_MIDI.EnableTabCheckbox", _c("Override global MIDI settings", "lowres"), 0, kCmdGlobalMIDIOverride);

		addMIDIControls(tab, "GameOptions_MIDI.");
	}

	//
	// 7) The MT-32 tab
	//
	if (!_guioptions.contains(GUIO_NOMIDI)) {
		tab->addTab(_("MT-32"));

		if (g_system->getOverlayWidth() > 320)
			_globalMT32Override = new CheckboxWidget(tab, "GameOptions_MT32.EnableTabCheckbox", _("Override global MT-32 settings"), 0, kCmdGlobalMT32Override);
		else
			_globalMT32Override = new CheckboxWidget(tab, "GameOptions_MT32.EnableTabCheckbox", _c("Override global MT-32 settings", "lowres"), 0, kCmdGlobalMT32Override);

		addMT32Controls(tab, "GameOptions_MT32.");
	}

	//
	// 8) The Paths tab
	//
	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Paths"));
	else
		tab->addTab(_c("Paths", "lowres"));

	// These buttons have to be extra wide, or the text will be truncated
	// in the small version of the GUI.

	// GUI:  Button + Label for the game path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GameOptions_Paths.Gamepath", _("Game Path:"), 0, kCmdGameBrowser);
	else
		new ButtonWidget(tab, "GameOptions_Paths.Gamepath", _c("Game Path:", "lowres"), 0, kCmdGameBrowser);
	_gamePathWidget = new StaticTextWidget(tab, "GameOptions_Paths.GamepathText", gamePath);

	// GUI:  Button + Label for the additional path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GameOptions_Paths.Extrapath", _("Extra Path:"), _("Specifies path to additional data used the game"), kCmdExtraBrowser);
	else
		new ButtonWidget(tab, "GameOptions_Paths.Extrapath", _c("Extra Path:", "lowres"), _("Specifies path to additional data used the game"), kCmdExtraBrowser);
	_extraPathWidget = new StaticTextWidget(tab, "GameOptions_Paths.ExtrapathText", extraPath, _("Specifies path to additional data used the game"));

	_extraPathClearButton = addClearButton(tab, "GameOptions_Paths.ExtraPathClearButton", kCmdExtraPathClear);

	// GUI:  Button + Label for the save path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GameOptions_Paths.Savepath", _("Save Path:"), _("Specifies where your savegames are put"), kCmdSaveBrowser);
	else
		new ButtonWidget(tab, "GameOptions_Paths.Savepath", _c("Save Path:", "lowres"), _("Specifies where your savegames are put"), kCmdSaveBrowser);
	_savePathWidget = new StaticTextWidget(tab, "GameOptions_Paths.SavepathText", savePath, _("Specifies where your savegames are put"));

	_savePathClearButton = addClearButton(tab, "GameOptions_Paths.SavePathClearButton", kCmdSavePathClear);

	// Activate the first tab
	tab->setActiveTab(0);
	_tabWidget = tab;

	// Add OK & Cancel buttons
	new ButtonWidget(this, "GameOptions.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "GameOptions.Ok", _("OK"), 0, kOKCmd);
}

void EditGameDialog::open() {
	OptionsDialog::open();

	String extraPath(ConfMan.get("extrapath", _domain));
	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPathWidget->setLabel(_c("None", "path"));
	}

	String savePath(ConfMan.get("savepath", _domain));
	if (savePath.empty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePathWidget->setLabel(_("Default"));
	}

	int sel, i;
	bool e;

	// En-/disable dialog items depending on whether overrides are active or not.

	e = ConfMan.hasKey("gfx_mode", _domain) ||
		ConfMan.hasKey("render_mode", _domain) ||
		ConfMan.hasKey("fullscreen", _domain) ||
		ConfMan.hasKey("aspect_ratio", _domain);
	_globalGraphicsOverride->setState(e);

	e = ConfMan.hasKey("music_driver", _domain) ||
		ConfMan.hasKey("output_rate", _domain) ||
		ConfMan.hasKey("opl_driver", _domain) ||
		ConfMan.hasKey("subtitles", _domain) ||
		ConfMan.hasKey("talkspeed", _domain);
	_globalAudioOverride->setState(e);

	e = ConfMan.hasKey("music_volume", _domain) ||
		ConfMan.hasKey("sfx_volume", _domain) ||
		ConfMan.hasKey("speech_volume", _domain);
	_globalVolumeOverride->setState(e);

	if (!_guioptions.contains(GUIO_NOMIDI)) {
		e = ConfMan.hasKey("soundfont", _domain) ||
			ConfMan.hasKey("multi_midi", _domain) ||
			ConfMan.hasKey("midi_gain", _domain);
		_globalMIDIOverride->setState(e);
	}

	if (!_guioptions.contains(GUIO_NOMIDI)) {
		e = ConfMan.hasKey("native_mt32", _domain) ||
			ConfMan.hasKey("enable_gs", _domain);
		_globalMT32Override->setState(e);
	}

	// TODO: game path

	const Common::Language lang = Common::parseLanguage(ConfMan.get("language", _domain));

	if (ConfMan.hasKey("language", _domain)) {
		_langPopUp->setSelectedTag(lang);
	} else {
		_langPopUp->setSelectedTag((uint32)Common::UNK_LANG);
	}

	if (_langPopUp->numEntries() <= 3) { // If only one language is avaliable
		_langPopUpDesc->setEnabled(false);
		_langPopUp->setEnabled(false);
	}

	// Set the state of engine-specific checkboxes
	for (uint j = 0; j < _engineOptions.size(); ++j) {
		// The default values for engine-specific checkboxes are not set when
		// ScummVM starts, as this would require us to load and poll all of the
		// engine plugins on startup. Thus, we set the state of each custom
		// option checkbox to what is specified by the engine plugin, and
		// update it only if a value has been set in the configuration of the
		// currently selected game.
		bool isChecked = _engineOptions[j].defaultState;
		if (ConfMan.hasKey(_engineOptions[j].configOption, _domain))
			isChecked = ConfMan.getBool(_engineOptions[j].configOption, _domain);
		_engineCheckboxes[j]->setState(isChecked);
	}

	const Common::PlatformDescription *p = Common::g_platforms;
	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", _domain));
	sel = 0;
	for (i = 0; p->code; ++p, ++i) {
		if (platform == p->id)
			sel = i + 2;
	}
	_platformPopUp->setSelected(sel);
}


void EditGameDialog::close() {
	if (getResult()) {
		ConfMan.set("description", _descriptionWidget->getEditString(), _domain);

		Common::Language lang = (Common::Language)_langPopUp->getSelectedTag();
		if (lang < 0)
			ConfMan.removeKey("language", _domain);
		else
			ConfMan.set("language", Common::getLanguageCode(lang), _domain);

		String gamePath(_gamePathWidget->getLabel());
		if (!gamePath.empty())
			ConfMan.set("path", gamePath, _domain);

		String extraPath(_extraPathWidget->getLabel());
		if (!extraPath.empty() && (extraPath != _c("None", "path")))
			ConfMan.set("extrapath", extraPath, _domain);
		else
			ConfMan.removeKey("extrapath", _domain);

		String savePath(_savePathWidget->getLabel());
		if (!savePath.empty() && (savePath != _("Default")))
			ConfMan.set("savepath", savePath, _domain);
		else
			ConfMan.removeKey("savepath", _domain);

		Common::Platform platform = (Common::Platform)_platformPopUp->getSelectedTag();
		if (platform < 0)
			ConfMan.removeKey("platform", _domain);
		else
			ConfMan.set("platform", Common::getPlatformCode(platform), _domain);

		// Set the state of engine-specific checkboxes
		for (uint i = 0; i < _engineOptions.size(); i++) {
			ConfMan.setBool(_engineOptions[i].configOption, _engineCheckboxes[i]->getState(), _domain);
		}
	}
	OptionsDialog::close();
}

void EditGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCmdGlobalGraphicsOverride:
		setGraphicSettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalAudioOverride:
		setAudioSettingsState(data != 0);
		setSubtitleSettingsState(data != 0);
		if (_globalVolumeOverride == NULL)
			setVolumeSettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalMIDIOverride:
		setMIDISettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalMT32Override:
		setMT32SettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalVolumeOverride:
		setVolumeSettingsState(data != 0);
		draw();
		break;
	case kCmdChooseSoundFontCmd: {
		BrowserDialog browser(_("Select SoundFont"), false);

		if (browser.runModal() > 0) {
			// User made this choice...
			Common::FSNode file(browser.getResult());
			_soundFont->setLabel(file.getPath());

			if (!file.getPath().empty() && (file.getPath() != _c("None", "path")))
				_soundFontClearButton->setEnabled(true);
			else
				_soundFontClearButton->setEnabled(false);

			draw();
		}
		break;
	}

	// Change path for the game
	case kCmdGameBrowser: {
		BrowserDialog browser(_("Select directory with game data"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());

			// TODO: Verify the game can be found in the new directory... Best
			// done with optional specific gameid to pluginmgr detectgames?
			// FSList files = dir.listDir(FSNode::kListFilesOnly);

			_gamePathWidget->setLabel(dir.getPath());
			draw();
		}
		draw();
		break;
	}

	// Change path for extra game data (eg, using sword cutscenes when playing via CD)
	case kCmdExtraBrowser: {
		BrowserDialog browser(_("Select additional game directory"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_extraPathWidget->setLabel(dir.getPath());
			draw();
		}
		draw();
		break;
	}
	// Change path for stored save game (perm and temp) data
	case kCmdSaveBrowser: {
		BrowserDialog browser(_("Select directory for saved games"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_savePathWidget->setLabel(dir.getPath());
			draw();
		}
		draw();
		break;
	}

	case kCmdExtraPathClear:
		_extraPathWidget->setLabel(_c("None", "path"));
		break;

	case kCmdSavePathClear:
		_savePathWidget->setLabel(_("Default"));
		break;

	case kOKCmd: {
		// Write back changes made to config object
		String newDomain(_domainWidget->getEditString());
		if (newDomain != _domain) {
			if (newDomain.empty()
				|| newDomain.hasPrefix("_")
				|| newDomain == ConfigManager::kApplicationDomain
				|| ConfMan.hasGameDomain(newDomain)) {
				MessageDialog alert(_("This game ID is already taken. Please choose another one."));
				alert.runModal();
				return;
			}
			ConfMan.renameGameDomain(_domain, newDomain);
			_domain = newDomain;
		}
		}
		// FALL THROUGH to default case
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}

#pragma mark -

LauncherDialog::LauncherDialog()
	: Dialog(0, 0, 320, 200) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_w = screenW;
	_h = screenH;

#ifndef DISABLE_FANCY_THEMES
	_logo = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GraphicsWidget(this, "Launcher.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageLogo));

		new StaticTextWidget(this, "Launcher.Version", gScummVMVersionDate);
	} else
		new StaticTextWidget(this, "Launcher.Version", gScummVMFullVersion);
#else
	// Show ScummVM version
	new StaticTextWidget(this, "Launcher.Version", gScummVMFullVersion);
#endif

	new ButtonWidget(this, "Launcher.QuitButton", _("~Q~uit"), _("Quit ScummVM"), kQuitCmd);
	new ButtonWidget(this, "Launcher.AboutButton", _("A~b~out..."), _("About ScummVM"), kAboutCmd);
	new ButtonWidget(this, "Launcher.OptionsButton", _("~O~ptions..."), _("Change global ScummVM options"), kOptionsCmd);
	_startButton =
		new ButtonWidget(this, "Launcher.StartButton", _("~S~tart"), _("Start selected game"), kStartCmd);

	_loadButton =
		new ButtonWidget(this, "Launcher.LoadGameButton", _("~L~oad..."), _("Load savegame for selected game"), kLoadGameCmd);

	// Above the lowest button rows: two more buttons (directly below the list box)
	if (g_system->getOverlayWidth() > 320) {
		_addButton =
			new ButtonWidget(this, "Launcher.AddGameButton", _("~A~dd Game..."), _("Hold Shift for Mass Add"), kAddGameCmd);
		_editButton =
			new ButtonWidget(this, "Launcher.EditGameButton", _("~E~dit Game..."), _("Change game options"), kEditGameCmd);
		_removeButton =
			new ButtonWidget(this, "Launcher.RemoveGameButton", _("~R~emove Game"), _("Remove game from the list. The game data files stay intact"), kRemoveGameCmd);
	} else {
		_addButton =
		new ButtonWidget(this, "Launcher.AddGameButton", _c("~A~dd Game...", "lowres"), _("Hold Shift for Mass Add"), kAddGameCmd);
		_editButton =
		new ButtonWidget(this, "Launcher.EditGameButton", _c("~E~dit Game...", "lowres"), _("Change game options"), kEditGameCmd);
		_removeButton =
		new ButtonWidget(this, "Launcher.RemoveGameButton", _c("~R~emove Game", "lowres"), _("Remove game from the list. The game data files stay intact"), kRemoveGameCmd);
	}

	// Search box
	_searchDesc = 0;
#ifndef DISABLE_FANCY_THEMES
	_searchPic = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		_searchPic = new GraphicsWidget(this, "Launcher.SearchPic", _("Search in game list"));
		_searchPic->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageSearch));
	} else
#endif
		_searchDesc = new StaticTextWidget(this, "Launcher.SearchDesc", _("Search:"));

	_searchWidget = new EditTextWidget(this, "Launcher.Search", _search, 0, kSearchCmd);
	_searchClearButton = addClearButton(this, "Launcher.SearchClearButton", kSearchClearCmd);

	// Add list with game titles
	_list = new ListWidget(this, "Launcher.GameList", 0, kListSearchCmd);
	_list->setEditable(false);
	_list->setNumberingMode(kListNumberingOff);

	// Populate the list
	updateListing();

	// Restore last selection
	String last(ConfMan.get("lastselectedgame", ConfigManager::kApplicationDomain));
	selectTarget(last);

	// En-/disable the buttons depending on the list selection
	updateButtons();

	// Create file browser dialog
	_browser = new BrowserDialog(_("Select directory with game data"), true);

	// Create Load dialog
	_loadDialog = new SaveLoadChooser(_("Load game:"), _("Load"), false);
}

void LauncherDialog::selectTarget(const String &target) {
	if (!target.empty()) {
		int itemToSelect = 0;
		StringArray::const_iterator iter;
		for (iter = _domains.begin(); iter != _domains.end(); ++iter, ++itemToSelect) {
			if (target == *iter) {
				_list->setSelected(itemToSelect);
				break;
			}
		}
	}
}

LauncherDialog::~LauncherDialog() {
	delete _browser;
	delete _loadDialog;
}

void LauncherDialog::open() {
	// Clear the active domain, in case we return to the dialog from a
	// failure to launch a game. Otherwise, pressing ESC will attempt to
	// re-launch the same game again.
	ConfMan.setActiveDomain("");

	CursorMan.popAllCursors();
	Dialog::open();

	updateButtons();
}

void LauncherDialog::close() {
	// Save last selection
	const int sel = _list->getSelected();
	if (sel >= 0)
		ConfMan.set("lastselectedgame", _domains[sel], ConfigManager::kApplicationDomain);
	else
		ConfMan.removeKey("lastselectedgame", ConfigManager::kApplicationDomain);

	ConfMan.flushToDisk();
	Dialog::close();
}

void LauncherDialog::updateListing() {
	StringArray l;

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	ConfigManager::DomainMap::const_iterator iter;
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
#ifdef __DS__
		// DS port uses an extra section called 'ds'.  This prevents the section from being
		// detected as a game.
		if (iter->_key == "ds") {
			continue;
		}
#endif

		String gameid(iter->_value.getVal("gameid"));
		String description(iter->_value.getVal("description"));

		if (gameid.empty())
			gameid = iter->_key;
		if (description.empty()) {
			GameDescriptor g = EngineMan.findGame(gameid);
			if (g.contains("description"))
				description = g.description();
		}

		if (description.empty()) {
			description = Common::String::format("Unknown (target %s, gameid %s)", iter->_key.c_str(), gameid.c_str());
		}

		if (!gameid.empty() && !description.empty()) {
			// Insert the game into the launcher list
			int pos = 0, size = l.size();

			while (pos < size && (scumm_stricmp(description.c_str(), l[pos].c_str()) > 0))
				pos++;
			l.insert_at(pos, description);
			_domains.insert_at(pos, iter->_key);
		}
	}

	const int oldSel = _list->getSelected();
	_list->setList(l);
	if (oldSel < (int)l.size())
		_list->setSelected(oldSel);	// Restore the old selection
	else if (oldSel != -1)
		// Select the last entry if the list has been reduced
		_list->setSelected(_list->getList().size() - 1);
	updateButtons();

	// Update the filter settings, those are lost when "setList"
	// is called.
	_list->setFilter(_searchWidget->getEditString());
}

void LauncherDialog::addGame() {
	int modifiers = g_system->getEventManager()->getModifierState();

#ifndef DISABLE_MASS_ADD
	const bool massAdd = (modifiers & Common::KBD_SHIFT) != 0;

	if (massAdd) {
		MessageDialog alert(_("Do you really want to run the mass game detector? "
							  "This could potentially add a huge number of games."), _("Yes"), _("No"));
		if (alert.runModal() == GUI::kMessageOK && _browser->runModal() > 0) {
			MassAddDialog massAddDlg(_browser->getResult());

			massAddDlg.runModal();

			// Update the ListWidget and force a redraw

			// If new target(s) were added, update the ListWidget and move
			// the selection to to first newly detected game.
			Common::String newTarget = massAddDlg.getFirstAddedTarget();
			if (!newTarget.empty()) {
				updateListing();
				selectTarget(newTarget);
			}

			draw();
		}

		// We need to update the buttons here, so "Mass add" will revert to "Add game"
		// without any additional event.
		updateButtons();
		return;
	}
#endif

	// Allow user to add a new game to the list.
	// 1) show a dir selection dialog which lets the user pick the directory
	//    the game data resides in.
	// 2) try to auto detect which game is in the directory, if we cannot
	//    determine it uniquely present a list of candidates to the user
	//    to pick from
	// 3) Display the 'Edit' dialog for that item, letting the user specify
	//    an alternate description (to distinguish multiple versions of the
	//    game, e.g. 'Monkey German' and 'Monkey English') and set default
	//    options for that game
	// 4) If no game is found in the specified directory, return to the
	//    dialog.

	bool looping;
	do {
		looping = false;

		if (_browser->runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(_browser->getResult());
			Common::FSList files;
			if (!dir.getChildren(files, Common::FSNode::kListAll)) {
				MessageDialog alert(_("ScummVM couldn't open the specified directory!"));
				alert.runModal();
				return;
			}

			// ...so let's determine a list of candidates, games that
			// could be contained in the specified directory.
			GameList candidates(EngineMan.detectGames(files));

			int idx;
			if (candidates.empty()) {
				// No game was found in the specified directory
				MessageDialog alert(_("ScummVM could not find any game in the specified directory!"));
				alert.runModal();
				idx = -1;

				looping = true;
			} else if (candidates.size() == 1) {
				// Exact match
				idx = 0;
			} else {
				// Display the candidates to the user and let her/him pick one
				StringArray list;
				for (idx = 0; idx < (int)candidates.size(); idx++)
					list.push_back(candidates[idx].description());

				ChooserDialog dialog(_("Pick the game:"));
				dialog.setList(list);
				idx = dialog.runModal();
			}
			if (0 <= idx && idx < (int)candidates.size()) {
				GameDescriptor result = candidates[idx];

				// TODO: Change the detectors to set "path" !
				result["path"] = dir.getPath();

				Common::String domain = addGameToConf(result);

				// Display edit dialog for the new entry
				EditGameDialog editDialog(domain, result.description());
				if (editDialog.runModal() > 0) {
					// User pressed OK, so make changes permanent

					// Write config to disk
					ConfMan.flushToDisk();

					// Update the ListWidget, select the new item, and force a redraw
					updateListing();
					selectTarget(editDialog.getDomain());
					draw();
				} else {
					// User aborted, remove the the new domain again
					ConfMan.removeGameDomain(domain);
				}

			}
		}
	} while (looping);
}

Common::String addGameToConf(const GameDescriptor &result) {
	// The auto detector or the user made a choice.
	// Pick a domain name which does not yet exist (after all, we
	// are *adding* a game to the config, not replacing).
	Common::String domain = result.preferredtarget();

	assert(!domain.empty());
	if (ConfMan.hasGameDomain(domain)) {
		int suffixN = 1;
		Common::String gameid(domain);

		while (ConfMan.hasGameDomain(domain)) {
			domain = gameid + Common::String::format("-%d", suffixN);
			suffixN++;
		}
	}

	// Add the name domain
	ConfMan.addGameDomain(domain);

	// Copy all non-empty key/value pairs into the new domain
	for (GameDescriptor::const_iterator iter = result.begin(); iter != result.end(); ++iter) {
		if (!iter->_value.empty() && iter->_key != "preferredtarget")
			ConfMan.set(iter->_key, iter->_value, domain);
	}

	// TODO: Setting the description field here has the drawback
	// that the user does never notice when we upgrade our descriptions.
	// It might be nice ot leave this field empty, and only set it to
	// a value when the user edits the description string.
	// However, at this point, that's impractical. Once we have a method
	// to query all backends for the proper & full description of a given
	// game target, we can change this (currently, you can only query
	// for the generic gameid description; it's not possible to obtain
	// a description which contains extended information like language, etc.).

	return domain;
}

void LauncherDialog::removeGame(int item) {
	MessageDialog alert(_("Do you really want to remove this game configuration?"), _("Yes"), _("No"));

	if (alert.runModal() == GUI::kMessageOK) {
		// Remove the currently selected game from the list
		assert(item >= 0);
		ConfMan.removeGameDomain(_domains[item]);

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget and force a redraw
		updateListing();
		draw();
	}
}

void LauncherDialog::editGame(int item) {
	// Set game specific options. Most of these should be "optional", i.e. by
	// default set nothing and use the global ScummVM settings. E.g. the user
	// can set here an optional alternate music volume, or for specific games
	// a different music driver etc.
	// This is useful because e.g. MonkeyVGA needs AdLib music to have decent
	// music support etc.
	assert(item >= 0);
	String gameId(ConfMan.get("gameid", _domains[item]));
	if (gameId.empty())
		gameId = _domains[item];
	EditGameDialog editDialog(_domains[item], EngineMan.findGame(gameId).description());
	if (editDialog.runModal() > 0) {
		// User pressed OK, so make changes permanent

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget, reselect the edited game and force a redraw
		updateListing();
		selectTarget(editDialog.getDomain());
		draw();
	}
}

void LauncherDialog::loadGame(int item) {
	String gameId = ConfMan.get("gameid", _domains[item]);
	if (gameId.empty())
		gameId = _domains[item];

	const EnginePlugin *plugin = 0;

	EngineMan.findGame(gameId, &plugin);

	String target = _domains[item];
	target.toLowercase();

	if (plugin) {
		if ((*plugin)->hasFeature(MetaEngine::kSupportsListSaves) &&
			(*plugin)->hasFeature(MetaEngine::kSupportsLoadingDuringStartup)) {
			int slot = _loadDialog->runModalWithPluginAndTarget(plugin, target);
			if (slot >= 0) {
				ConfMan.setActiveDomain(_domains[item]);
				ConfMan.setInt("save_slot", slot, Common::ConfigManager::kTransientDomain);
				close();
			}
		} else {
			MessageDialog dialog
				(_("This game does not support loading games from the launcher."), _("OK"));
			dialog.runModal();
		}
	} else {
		MessageDialog dialog(_("ScummVM could not find any engine capable of running the selected game!"), _("OK"));
		dialog.runModal();
	}
}

void LauncherDialog::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_TAB) {
		// Toggle between the game list and the quick search field.
		if (getFocusWidget() == _searchWidget) {
			setFocusWidget(_list);
		} else if (getFocusWidget() == _list) {
			setFocusWidget(_searchWidget);
		}
	}
	Dialog::handleKeyDown(state);
	updateButtons();
}

void LauncherDialog::handleKeyUp(Common::KeyState state) {
	Dialog::handleKeyUp(state);
	updateButtons();
}

void LauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item = _list->getSelected();

	switch (cmd) {
	case kAddGameCmd:
		addGame();
		break;
	case kRemoveGameCmd:
		removeGame(item);
		break;
	case kEditGameCmd:
		editGame(item);
		break;
	case kLoadGameCmd:
		loadGame(item);
		break;
	case kOptionsCmd: {
		GlobalOptionsDialog options;
		options.runModal();
		}
		break;
	case kAboutCmd: {
		AboutDialog about;
		about.runModal();
		}
		break;
	case kStartCmd:
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		// Start the selected game.
		assert(item >= 0);
		ConfMan.setActiveDomain(_domains[item]);
		close();
		break;
	case kListItemRemovalRequestCmd:
		removeGame(item);
		break;
	case kListSelectionChangedCmd:
		updateButtons();
		break;
	case kQuitCmd:
		ConfMan.setActiveDomain("");
		setResult(-1);
		close();
		break;
	case kSearchCmd:
		// Update the active search filter.
		_list->setFilter(_searchWidget->getEditString());
		break;
	case kSearchClearCmd:
		// Reset the active search filter, thus showing all games again
		_searchWidget->setEditString("");
		_list->setFilter("");
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void LauncherDialog::updateButtons() {
	bool enable = (_list->getSelected() >= 0);
	if (enable != _startButton->isEnabled()) {
		_startButton->setEnabled(enable);
		_startButton->draw();
	}
	if (enable != _editButton->isEnabled()) {
		_editButton->setEnabled(enable);
		_editButton->draw();
	}
	if (enable != _removeButton->isEnabled()) {
		_removeButton->setEnabled(enable);
		_removeButton->draw();
	}

	int item = _list->getSelected();
	bool en = enable;

	if (item >= 0)
		en = !(Common::checkGameGUIOption(GUIO_NOLAUNCHLOAD, ConfMan.get("guioptions", _domains[item])));

	if (en != _loadButton->isEnabled()) {
		_loadButton->setEnabled(en);
		_loadButton->draw();
	}

	// Update the label of the "Add" button depending on whether shift is pressed or not
	int modifiers = g_system->getEventManager()->getModifierState();
	const bool massAdd = (modifiers & Common::KBD_SHIFT) != 0;
	const bool lowRes = g_system->getOverlayWidth() <= 320;

	const char *newAddButtonLabel = massAdd
		? (lowRes ? _c("Mass Add...", "lowres") : _("Mass Add..."))
		: (lowRes ? _c("~A~dd Game...", "lowres") : _("~A~dd Game..."));

	if (_addButton->getLabel() != newAddButtonLabel)
		_addButton->setLabel(newAddButtonLabel);
}

void LauncherDialog::reflowLayout() {
#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		StaticTextWidget *ver = (StaticTextWidget *)findWidget("Launcher.Version");
		if (ver) {
			ver->setAlign((Graphics::TextAlign)g_gui.xmlEval()->getVar("Launcher.Version.Align", Graphics::kTextAlignCenter));
			ver->setLabel(gScummVMVersionDate);
		}

		if (!_logo)
			_logo = new GraphicsWidget(this, "Launcher.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageLogo));
	} else {
		StaticTextWidget *ver = (StaticTextWidget *)findWidget("Launcher.Version");
		if (ver) {
			ver->setAlign((Graphics::TextAlign)g_gui.xmlEval()->getVar("Launcher.Version.Align", Graphics::kTextAlignCenter));
			ver->setLabel(gScummVMFullVersion);
		}

		if (_logo) {
			removeWidget(_logo);
			_logo->setNext(0);
			delete _logo;
			_logo = 0;
		}
	}

	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		if (!_searchPic)
			_searchPic = new GraphicsWidget(this, "Launcher.SearchPic");
		_searchPic->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageSearch));

		if (_searchDesc) {
			removeWidget(_searchDesc);
			_searchDesc->setNext(0);
			delete _searchDesc;
			_searchDesc = 0;
		}
	} else {
		if (!_searchDesc)
			_searchDesc = new StaticTextWidget(this, "Launcher.SearchDesc", _("Search:"));

		if (_searchPic) {
			removeWidget(_searchPic);
			_searchPic->setNext(0);
			delete _searchPic;
			_searchPic = 0;
		}
	}

	removeWidget(_searchClearButton);
	_searchClearButton->setNext(0);
	delete _searchClearButton;
	_searchClearButton = addClearButton(this, "Launcher.SearchClearButton", kSearchClearCmd);
#endif

	_w = g_system->getOverlayWidth();
	_h = g_system->getOverlayHeight();

	Dialog::reflowLayout();
}

} // End of namespace GUI
