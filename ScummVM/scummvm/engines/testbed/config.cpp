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

#include "common/stream.h"
#include "common/config-manager.h"

#include "engines/engine.h"

#include "testbed/config.h"
#include "testbed/fs.h"

namespace Testbed {

TestbedOptionsDialog::TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan) : GUI::Dialog("Browser"), _testbedConfMan(tsConfMan) {

	new GUI::StaticTextWidget(this, "Browser.Headline", "Select Testsuites to Execute");
	new GUI::StaticTextWidget(this, "Browser.Path", "Use Doubleclick to select/deselect");

	// Construct a String Array
	Common::Array<Testsuite *>::const_iterator iter;
	Common::String description;
	uint selected = 0;

	for (iter = tsList.begin(); iter != tsList.end(); iter++) {
		_testSuiteArray.push_back(*iter);
		description = (*iter)->getDescription();
		if ((*iter)->isEnabled()) {
			_testSuiteDescArray.push_back(description + "(selected)");
			selected++;
			_colors.push_back(GUI::ThemeEngine::kFontColorNormal);
		} else {
			_testSuiteDescArray.push_back(description);
			_colors.push_back(GUI::ThemeEngine::kFontColorAlternate);
		}
	}

	_testListDisplay = new TestbedListWidget(this, "Browser.List", _testSuiteArray);
	_testListDisplay->setNumberingMode(GUI::kListNumberingOff);
	_testListDisplay->setList(_testSuiteDescArray, &_colors);

	// This list shouldn't be editable
	_testListDisplay->setEditable(false);

	if (selected > (tsList.size() - selected)) {
		_selectButton = new GUI::ButtonWidget(this, "Browser.Up", "Deselect All", 0, kTestbedDeselectAll, 0);
	} else {
		_selectButton = new GUI::ButtonWidget(this, "Browser.Up", "Select All", 0, kTestbedSelectAll, 0);
	}
	new GUI::ButtonWidget(this, "Browser.Cancel", "Run tests", 0, GUI::kCloseCmd);
	new GUI::ButtonWidget(this, "Browser.Choose", "Exit Testbed", 0, kTestbedQuitCmd);
}

TestbedOptionsDialog::~TestbedOptionsDialog() {}

void TestbedOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	Testsuite *ts;
	Common::WriteStream *ws;
	switch (cmd) {
	case GUI::kListItemDoubleClickedCmd:
		ts  = _testSuiteArray[_testListDisplay->getSelected()];
		if (ts) {
			// Toggle status
			if (ts->isEnabled()) {
				ts->enable(false);
			} else {
				ts->enable(true);
			}

			// Now render status
			if (ts->isEnabled()) {
				_testListDisplay->markAsSelected(_testListDisplay->getSelected());
			} else {
				_testListDisplay->markAsDeselected(_testListDisplay->getSelected());
			}
		}
		break;

	case kTestbedQuitCmd:
		Engine::quitGame();
		close();
		break;

	case kTestbedDeselectAll:
		_selectButton->setLabel("Select All");
		_selectButton->setCmd(kTestbedSelectAll);
		for (uint i = 0; i < _testSuiteArray.size(); i++) {
			_testListDisplay->markAsDeselected(i);
			ts  = _testSuiteArray[i];
			if (ts) {
				ts->enable(false);
			}
		}
		break;

	case kTestbedSelectAll:
		_selectButton->setLabel("Deselect All");
		_selectButton->setCmd(kTestbedDeselectAll);
		for (uint i = 0; i < _testSuiteArray.size(); i++) {
			_testListDisplay->markAsSelected(i);
			ts  = _testSuiteArray[i];
			if (ts) {
				ts->enable(true);
			}
		}
		break;
	case GUI::kCloseCmd:
		// This is final selected state, write it to config file.
		ws = _testbedConfMan->getConfigWriteStream();
		_testbedConfMan->writeTestbedConfigToStream(ws);
		delete ws;
		break;

	default:
		break;
	}

	GUI::Dialog::handleCommand(sender, cmd, data);
}

void TestbedInteractionDialog::addText(uint w, uint h, const Common::String text, Graphics::TextAlign textAlign, uint xOffset, uint yPadding) {
	if (!xOffset) {
		xOffset = _xOffset;
	}
	_yOffset += yPadding;
	new GUI::StaticTextWidget(this, xOffset, _yOffset, w, h, text, textAlign);
	_yOffset += h;
}

void TestbedInteractionDialog::addButton(uint w, uint h, const Common::String name, uint32 cmd, uint xOffset, uint yPadding) {
	if (!xOffset) {
		xOffset = _xOffset;
	}
	_yOffset += yPadding;
	_buttonArray.push_back(new GUI::ButtonWidget(this, xOffset, _yOffset, w, h, name, 0, cmd));
	_yOffset += h;
}

void TestbedInteractionDialog::addList(uint x, uint y, uint w, uint h, const Common::Array<Common::String> &strArray, GUI::ListWidget::ColorList *colors, uint yPadding) {
	_yOffset += yPadding;
	GUI::ListWidget *list = new GUI::ListWidget(this, x, y, w, h);
	list->setEditable(false);
	list->setNumberingMode(GUI::kListNumberingOff);
	list->setList(strArray, colors);
	_yOffset += h;
}

void TestbedInteractionDialog::addButtonXY(uint x, uint /* y */, uint w, uint h, const Common::String name, uint32 cmd) {
	_buttonArray.push_back(new GUI::ButtonWidget(this, x, _yOffset, w, h, name, 0, cmd));
}

void TestbedInteractionDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	GUI::Dialog::handleCommand(sender, cmd, data);
}

void TestbedConfigManager::initDefaultConfiguration() {
	// Default Configuration
	// Add Global configuration Parameters here.
	_configFileInterface.setKey("isSessionInteractive", "Global", "true");
}

void TestbedConfigManager::writeTestbedConfigToStream(Common::WriteStream *ws) {
	for (Common::Array<Testsuite *>::const_iterator i = _testsuiteList.begin(); i < _testsuiteList.end(); i++) {
		_configFileInterface.setKey("this", (*i)->getName(), boolToString((*i)->isEnabled()));
		const Common::Array<Test *> &testList = (*i)->getTestList();
		for (Common::Array<Test *>::const_iterator j = testList.begin(); j != testList.end(); j++) {
			_configFileInterface.setKey((*j)->featureName, (*i)->getName(), boolToString((*j)->enabled));
		}
	}
	_configFileInterface.saveToStream(*ws);
	ws->flush();
}

Common::SeekableReadStream *TestbedConfigManager::getConfigReadStream() const {
	// Look for config file using SearchMan
	Common::SeekableReadStream *rs = SearchMan.createReadStreamForMember(_configFileName);
	return rs;
}

Common::WriteStream *TestbedConfigManager::getConfigWriteStream() const {
	// Look for config file in game-path
	const Common::String &path = ConfMan.get("path");
	Common::WriteStream *ws;
	Common::FSNode gameRoot(path);
	Common::FSNode config = gameRoot.getChild(_configFileName);
	ws = config.createWriteStream();
	return ws;
}

void TestbedConfigManager::parseConfigFile() {
	Common::SeekableReadStream *rs = getConfigReadStream();

	if (!rs) {
		Testsuite::logPrintf("Info! No config file found, using default configuration.\n");
		initDefaultConfiguration();
		return;
	}
	_configFileInterface.loadFromStream(*rs);
	Common::ConfigFile::SectionList sections = _configFileInterface.getSections();
	Testsuite *currTS = 0;

	for (Common::ConfigFile::SectionList::const_iterator i = sections.begin(); i != sections.end(); i++) {
		if (i->name.equalsIgnoreCase("Global")) {
			// Global params may be directly queried, ignore them
		} else {
			// A testsuite, process it.
			currTS = getTestsuiteByName(i->name);
			Common::ConfigFile::SectionKeyList kList = i->getKeys();
			if (!currTS) {
				Testsuite::logPrintf("Warning! Error in config: Testsuite %s not found\n", i->name.c_str());
				continue;
			}

			for (Common::ConfigFile::SectionKeyList::const_iterator j = kList.begin(); j != kList.end(); j++) {
				if (j->key.equalsIgnoreCase("this")) {
					currTS->enable(stringToBool(j->value));
				} else {
					if (!currTS->enableTest(j->key, stringToBool(j->value))) {
						Testsuite::logPrintf("Warning! Error in config: Test %s not found\n", j->key.c_str());
					}
				}
			}
		}
	}
	delete rs;
}

int TestbedConfigManager::getNumSuitesEnabled() {
	int count = 0;
	for (uint i = 0; i < _testsuiteList.size(); i++) {
		if (_testsuiteList[i]->isEnabled()) {
			count++;
		}
	}
	return count;
}

Testsuite *TestbedConfigManager::getTestsuiteByName(const Common::String &name) {
	for (uint i = 0; i < _testsuiteList.size(); i++) {
		if (name.equalsIgnoreCase(_testsuiteList[i]->getName())) {
			return _testsuiteList[i];
		}
	}
	return 0;
}

void TestbedConfigManager::selectTestsuites() {

	parseConfigFile();

	if (_configFileInterface.hasKey("isSessionInteractive", "Global")) {
		Common::String in;
		_configFileInterface.getKey("isSessionInteractive", "Global", in);
		ConfParams.setSessionAsInteractive(stringToBool(in));
	}

	if (!ConfParams.isSessionInteractive()) {
		// Non interactive sessions don't need to go beyond
		return;
	}

	// XXX: disabling these as of now for fastly testing other tests
	// Testsuite::isSessionInteractive = false;
	Common::String prompt("Welcome to the ScummVM testbed!\n"
						"It is a framework to test the various ScummVM subsystems namely GFX, Sound, FS, events etc.\n"
						"If you see this, it means interactive tests would run on this system :)\n");

	if (!ConfParams.isGameDataFound()) {
		prompt += "\nSeems like Game data files are not configured properly.\n"
		"Create Game data files using script ./create-testbed-data.sh in dists/engine-data\n"
		"Next, Configure the game path in launcher / command-line.\n"
		"Currently a few testsuites namely FS/AudioCD/MIDI would be disabled\n";
	}

	Testsuite::logPrintf("Info! : Interactive tests are also being executed.\n");

	if (Testsuite::handleInteractiveInput(prompt, "Proceed?", "Customize", kOptionRight)) {
		if (Engine::shouldQuit()) {
			return;
		}
		// Select testsuites using checkboxes
		TestbedOptionsDialog tbd(_testsuiteList, this);
		tbd.runModal();
	}

	// Clear it to remove entries before next rerun
	_configFileInterface.clear();
}

}	// End of namespace Testbed
