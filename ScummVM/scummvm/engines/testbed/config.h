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

#ifndef TESTBED_CONFIG_H
#define TESTBED_CONFIG_H


#include "common/array.h"
#include "common/config-file.h"
#include "common/str-array.h"
#include "common/tokenizer.h"

#include "gui/widgets/list.h"
#include "gui/options.h"
#include "gui/ThemeEngine.h"

#include "testbed/testsuite.h"

namespace Testbed {

enum {
	kTestbedQuitCmd = 'Quit',
	kTestbedSelectAll = 'sAll',
	kTestbedDeselectAll = 'dAll'
};



class TestbedConfigManager {
public:
	TestbedConfigManager(Common::Array<Testsuite *> &tList, const Common::String fName) : _testsuiteList(tList), _configFileName(fName) {}
	~TestbedConfigManager() {}
	void selectTestsuites();
	void setConfigFile(const Common::String fName) { _configFileName = fName; }
	Common::SeekableReadStream *getConfigReadStream() const;
	Common::WriteStream *getConfigWriteStream() const;
	void writeTestbedConfigToStream(Common::WriteStream *ws);
	Testsuite *getTestsuiteByName(const Common::String &name);
	bool stringToBool(const Common::String str) { return str.equalsIgnoreCase("true") ? true : false; }
	Common::String boolToString(bool val) { return val ? "true" : "false"; }
	void initDefaultConfiguration();
	int getNumSuitesEnabled();

private:
	Common::Array<Testsuite *> &_testsuiteList;
	Common::String	_configFileName;
	Common::ConfigFile	_configFileInterface;
	void parseConfigFile();
};

class TestbedListWidget : public GUI::ListWidget {
public:
	TestbedListWidget(GUI::Dialog *boss, const Common::String &name, Common::Array<Testsuite *> tsArray) : GUI::ListWidget(boss, name), _testSuiteArray(tsArray) {}

	void markAsSelected(int i) {
		if (!_list[i].contains("selected")) {
			_list[i] += " (selected)";
		}
		_listColors[i] = GUI::ThemeEngine::kFontColorNormal;
		draw();
	}

	void markAsDeselected(int i) {
		if (_list[i].contains("selected")) {
			_list[i] = _testSuiteArray[i]->getDescription();
		}
		_listColors[i] = GUI::ThemeEngine::kFontColorAlternate;
		draw();
	}

	void setColor(uint32 indx, GUI::ThemeEngine::FontColor color) {
		assert(indx < _listColors.size());
		_listColors[indx] = color;
		draw();
	}

private:
	Common::Array<Testsuite *>	_testSuiteArray;
};

class TestbedOptionsDialog : public GUI::Dialog {
public:
	TestbedOptionsDialog(Common::Array<Testsuite *> &tsList, TestbedConfigManager *tsConfMan);
	~TestbedOptionsDialog();
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

private:
	GUI::ListWidget::ColorList _colors;
	GUI::ButtonWidget	*_selectButton;
	Common::Array<Testsuite *> _testSuiteArray;
	Common::StringArray _testSuiteDescArray;
	TestbedListWidget *_testListDisplay;
	TestbedConfigManager *_testbedConfMan;
};

class TestbedInteractionDialog : public GUI::Dialog {
public:
	TestbedInteractionDialog(uint x, uint y, uint w, uint h) : GUI::Dialog(x, y, w, h) {}
	~TestbedInteractionDialog() {}
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	void addButton(uint w, uint h, const Common::String name, uint32 cmd, uint xOffset = 0, uint yPadding = 8);
	void addButtonXY(uint x, uint y, uint w, uint h, const Common::String name, uint32 cmd);
	void addText(uint w, uint h, const Common::String text, Graphics::TextAlign textAlign, uint xOffset, uint yPadding = 8);
	void addList(uint x, uint y, uint w, uint h, const Common::Array<Common::String> &strArray, GUI::ListWidget::ColorList *colors = 0, uint yPadding = 8);
protected:
	Common::Array<GUI::ButtonWidget *> _buttonArray;
	uint _xOffset;
	uint _yOffset;

};

} // End of namespace Testbed

#endif // TESTBED_CONFIG_H
