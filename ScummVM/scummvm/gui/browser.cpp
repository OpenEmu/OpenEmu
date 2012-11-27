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

#include "gui/browser.h"
#include "gui/widgets/list.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/algorithm.h"

#include "common/translation.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos',
	kGoUpCmd = 'GoUp'
};

/* We want to use this as a general directory selector at some point... possible uses
 * - to select the data dir for a game
 * - to select the place where save games are stored
 * - others???
 */

BrowserDialog::BrowserDialog(const char *title, bool dirBrowser)
	: Dialog("Browser") {

	_isDirBrowser = dirBrowser;
	_fileList = NULL;
	_currentPath = NULL;

	// Headline - TODO: should be customizable during creation time
	new StaticTextWidget(this, "Browser.Headline", title);

	// Current path - TODO: handle long paths ?
	_currentPath = new StaticTextWidget(this, "Browser.Path", "DUMMY");

	// Add file list
	_fileList = new ListWidget(this, "Browser.List");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	// Buttons
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(this, "Browser.Up", _("Go up"), _("Go to previous directory level"), kGoUpCmd);
	else
		new ButtonWidget(this, "Browser.Up", _c("Go up", "lowres"), _("Go to previous directory level"), kGoUpCmd);
	new ButtonWidget(this, "Browser.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "Browser.Choose", _("Choose"), 0, kChooseCmd);
}

void BrowserDialog::open() {
	// Call super implementation
	Dialog::open();

	if (ConfMan.hasKey("browser_lastpath"))
		_node = Common::FSNode(ConfMan.get("browser_lastpath"));
	if (!_node.isDirectory())
		_node = Common::FSNode(".");

	// Alway refresh file list
	updateListing();
}

void BrowserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd:
		if (_isDirBrowser) {
			// If nothing is selected in the list widget, choose the current dir.
			// Else, choose the dir that is selected.
			int selection = _fileList->getSelected();
			if (selection >= 0)
				_choice = _nodeContent[selection];
			else
				_choice = _node;
			setResult(1);
			close();
		} else {
			int selection = _fileList->getSelected();
			if (selection < 0)
				break;
			if (_nodeContent[selection].isDirectory()) {
				_node = _nodeContent[selection];
				updateListing();
			} else {
				_choice = _nodeContent[selection];
				setResult(1);
				close();
			}
		}
		break;
	case kGoUpCmd:
		_node = _node.getParent();
		updateListing();
		break;
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		if (_nodeContent[data].isDirectory()) {
			_node = _nodeContent[data];
			updateListing();
		} else if (!_isDirBrowser) {
			_choice = _nodeContent[data];
			setResult(1);
			close();
		}
		break;
	case kListSelectionChangedCmd:
		// We do not allow selecting directories in directory
		// browser mode, thus we will invalidate the selection
		// when the user selects an directory over here.
		if (data != (uint32)-1 && _isDirBrowser && !_nodeContent[data].isDirectory())
			_fileList->setSelected(-1);
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void BrowserDialog::updateListing() {
	// Update the path display
	_currentPath->setLabel(_node.getPath());

	// We memorize the last visited path.
	ConfMan.set("browser_lastpath", _node.getPath());

	// Read in the data from the file system
	if (!_node.getChildren(_nodeContent, Common::FSNode::kListAll))
		_nodeContent.clear();
	else
		Common::sort(_nodeContent.begin(), _nodeContent.end());

	// Populate the ListWidget
	ListWidget::StringArray list;
	ListWidget::ColorList colors;
	for (Common::FSList::iterator i = _nodeContent.begin(); i != _nodeContent.end(); ++i) {
		if (i->isDirectory())
			list.push_back(i->getDisplayName() + "/");
		else
			list.push_back(i->getDisplayName());

		if (_isDirBrowser) {
			if (i->isDirectory())
				colors.push_back(ThemeEngine::kFontColorNormal);
			else
				colors.push_back(ThemeEngine::kFontColorAlternate);
		}
	}

	if (_isDirBrowser)
		_fileList->setList(list, &colors);
	else
		_fileList->setList(list);
	_fileList->scrollTo(0);

	// Finally, redraw
	draw();
}

} // End of namespace GUI
