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

#include "gui/themebrowser.h"
#include "gui/widgets/list.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"

#include "common/translation.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos'
};

// TODO: this is a rip off of GUI::Browser right now
// it will get some more output like theme name,
// theme style, theme preview(?) in the future
// but for now this simple browser works,
// also it will get its own theme config values
// and not use 'browser_' anymore
ThemeBrowser::ThemeBrowser() : Dialog("Browser") {
	_fileList = 0;

	new StaticTextWidget(this, "Browser.Headline", _("Select a Theme"));

	// Add file list
	_fileList = new ListWidget(this, "Browser.List");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	// Buttons
	new ButtonWidget(this, "Browser.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "Browser.Choose", _("Choose"), 0, kChooseCmd);
}

void ThemeBrowser::open() {
	// Alway refresh file list
	updateListing();

	// Call super implementation
	Dialog::open();
}

void ThemeBrowser::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd:
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd: {
		int selection = _fileList->getSelected();
		if (selection < 0)
			break;

		// TODO:
		// Currently ThemeEngine::listUseableThemes uses a
		// list. Thus we can not use operator[] here but
		// need to iterate through the list. We might want
		// to think of changing it, but it should not be
		// of high importance anyway.
		ThemeDescList::const_iterator sel = _themes.begin();
		for (int i = 0; i < selection; ++i)
			++sel;

		_select = sel->id;
		setResult(1);
		close();
		break;
	}
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void ThemeBrowser::updateListing() {
	_themes.clear();

	ThemeEngine::listUsableThemes(_themes);

	const Common::String currentThemeId = g_gui.theme()->getThemeId();
	int currentThemeIndex = 0, index = 0;

	ListWidget::StringArray list;
	for (ThemeDescList::const_iterator i = _themes.begin(); i != _themes.end(); ++i, ++index) {
		list.push_back(i->name);

		if (i->id == currentThemeId)
			currentThemeIndex = index;
	}

	_fileList->setList(list);
	_fileList->scrollTo(0);
	_fileList->setSelected(currentThemeIndex);

	// Finally, redraw
	draw();
}

} // End of namespace GUI
