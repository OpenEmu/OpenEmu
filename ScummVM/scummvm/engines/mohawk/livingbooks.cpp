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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mohawk/livingbooks.h"
#include "mohawk/resource.h"
#include "mohawk/cursors.h"
#include "mohawk/video.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/memstream.h"

#include "graphics/palette.h"

#include "engines/util.h"

#include "gui/message.h"

namespace Mohawk {

// read a null-terminated string from a stream
Common::String MohawkEngine_LivingBooks::readString(Common::ReadStream *stream) {
	Common::String ret;
	while (!stream->eos()) {
		byte in = stream->readByte();
		if (!in)
			break;
		ret += in;
	}
	return ret;
}

// read a rect from a stream
Common::Rect MohawkEngine_LivingBooks::readRect(Common::ReadStreamEndian *stream) {
	Common::Rect rect;

	// the V1 mac games have their rects in QuickDraw order
	if (isPreMohawk() && getPlatform() == Common::kPlatformMacintosh) {
		rect.top = stream->readSint16();
		rect.left = stream->readSint16();
		rect.bottom = stream->readSint16();
		rect.right = stream->readSint16();
	} else {
		rect.left = stream->readSint16();
		rect.top = stream->readSint16();
		rect.right = stream->readSint16();
		rect.bottom = stream->readSint16();
	}

	return rect;
}

LBPage::LBPage(MohawkEngine_LivingBooks *vm) : _vm(vm) {
	_code = NULL;
	_mhk = NULL;

	_baseId = 0;
	_cascade = false;
}

void LBPage::open(Archive *mhk, uint16 baseId) {
	_mhk = mhk;
	_baseId = baseId;

	_vm->addArchive(_mhk);
	if (!_vm->hasResource(ID_BCOD, baseId)) {
		// assume that BCOD is mandatory for v4/v5
		if (_vm->getGameType() == GType_LIVINGBOOKSV4 || _vm->getGameType() == GType_LIVINGBOOKSV5)
			error("missing BCOD resource (id %d)", baseId);
		_code = new LBCode(_vm, 0);
	} else {
		_code = new LBCode(_vm, baseId);
	}

	loadBITL(baseId);
	for (uint i = 0; i < _items.size(); i++)
		_vm->addItem(_items[i]);

	for (uint32 i = 0; i < _items.size(); i++)
		_items[i]->init();

	for (uint32 i = 0; i < _items.size(); i++)
		_items[i]->startPhase(kLBPhaseLoad);
}

void LBPage::addClonedItem(LBItem *item) {
	_vm->addItem(item);
	_items.push_back(item);
}

void LBPage::itemDestroyed(LBItem *item) {
	for (uint i = 0; i < _items.size(); i++)
		if (item == _items[i]) {
			_items.remove_at(i);
			return;
		}
	error("itemDestroyed didn't find item");
}

LBPage::~LBPage() {
	delete _code;
	_vm->removeItems(_items);
	for (uint i = 0; i < _items.size(); i++)
		delete _items[i];
	_vm->removeArchive(_mhk);
	delete _mhk;
}

MohawkEngine_LivingBooks::MohawkEngine_LivingBooks(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	_needsUpdate = false;
	_needsRedraw = false;
	_screenWidth = _screenHeight = 0;

	_curLanguage = 1;
	_curSelectedPage = 1;

	_alreadyShowedIntro = false;

	_rnd = new Common::RandomSource("livingbooks");

	_page = NULL;

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	// Rugrats
	const Common::FSNode ProgPath = gameDataDir.getChild("program");
	if (ProgPath.exists())
		SearchMan.addDirectory(ProgPath.getPath(), ProgPath, 0, 2);
	const Common::FSNode RugPath = gameDataDir.getChild("Rugrats Adventure Game");
	if (RugPath.exists())
		SearchMan.addDirectory(RugPath.getPath(), RugPath, 0, 2);
	// CarmenTQ
	const Common::FSNode CTQPath = gameDataDir.getChild("95instal");
	if (CTQPath.exists())
		SearchMan.addDirectory(CTQPath.getPath(), CTQPath, 0, 4);
}

MohawkEngine_LivingBooks::~MohawkEngine_LivingBooks() {
	destroyPage();

	delete _console;
	delete _gfx;
	delete _rnd;
	_bookInfoFile.clear();
}

Common::Error MohawkEngine_LivingBooks::run() {
	MohawkEngine::run();

	_console = new LivingBooksConsole(this);
	// Load the book info from the detected file
	loadBookInfo(getBookInfoFileName());

	if (!_title.empty()) // Some games don't have the title stored
		debug("Starting Living Books Title \'%s\'", _title.c_str());
	if (!_copyright.empty())
		debug("Copyright: %s", _copyright.c_str());
	debug("This book has %d page(s) in %d language(s).", _numPages, _numLanguages);
	if (_poetryMode)
		debug("Running in poetry mode.");

	if (!_screenWidth || !_screenHeight)
		error("Could not find xRes/yRes variables");

	_gfx = new LBGraphics(this, _screenWidth, _screenHeight);

	if (getGameType() != GType_LIVINGBOOKSV1)
		_cursor = new LivingBooksCursorManager_v2();
	else if (getPlatform() == Common::kPlatformMacintosh)
		_cursor = new MacCursorManager(getAppName());
	else
		_cursor = new NECursorManager(getAppName());

	_cursor->setDefaultCursor();
	_cursor->showCursor();

	if (!tryLoadPageStart(kLBIntroMode, 1))
		error("Could not load intro page");

	Common::Event event;
	while (!shouldQuit()) {
		while (_eventMan->pollEvent(event)) {
			LBItem *found = NULL;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				_needsUpdate = true;
				break;

			case Common::EVENT_LBUTTONUP:
				if (_focus)
					_focus->handleMouseUp(event.mouse);
				break;

			case Common::EVENT_LBUTTONDOWN:
				for (Common::List<LBItem *>::const_iterator i = _orderedItems.begin(); i != _orderedItems.end(); ++i) {
					if ((*i)->contains(event.mouse)) {
						found = *i;
						break;
					}
				}

				if (found)
					found->handleMouseDown(event.mouse);
				break;

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_console->attach();
						_console->onFrame();
					}
					break;

				case Common::KEYCODE_SPACE:
					pauseGame();
					break;

				case Common::KEYCODE_ESCAPE:
					if (_curMode == kLBIntroMode)
						tryLoadPageStart(kLBControlMode, 1);
					break;

				case Common::KEYCODE_LEFT:
					prevPage();
					break;

				case Common::KEYCODE_RIGHT:
					nextPage();
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}

		updatePage();

		if (_video->updateMovies())
			_needsUpdate = true;

		if (_needsUpdate) {
			_system->updateScreen();
			_needsUpdate = false;
		}

		// Cut down on CPU usage
		_system->delayMillis(10);

		// handle pending notifications
		while (_notifyEvents.size()) {
			NotifyEvent notifyEvent = _notifyEvents.pop();
			handleNotify(notifyEvent);
		}
	}

	return Common::kNoError;
}

void MohawkEngine_LivingBooks::loadBookInfo(const Common::String &filename) {
	if (!_bookInfoFile.loadFromFile(filename))
		error("Could not open %s as a config file", filename.c_str());

	_title = getStringFromConfig("BookInfo", "title");
	_copyright = getStringFromConfig("BookInfo", "copyright");

	_numPages = getIntFromConfig("BookInfo", "nPages");
	_numLanguages = getIntFromConfig("BookInfo", "nLanguages");
	_screenWidth = getIntFromConfig("BookInfo", "xRes");
	_screenHeight = getIntFromConfig("BookInfo", "yRes");
	// nColors is here too, but it's always 256 anyway...

	// this is 1 in The New Kid on the Block, changes the hardcoded UI
	// v2 games changed the flag name to fPoetry
	if (getGameType() == GType_LIVINGBOOKSV1)
		_poetryMode = (getIntFromConfig("BookInfo", "poetry") == 1);
	else
		_poetryMode = (getIntFromConfig("BookInfo", "fPoetry") == 1);

	// The later Living Books games add some more options:
	//     - fNeedPalette                (always true?)
	//     - fUse254ColorPalette         (always true?)
	//     - nKBRequired                 (4096, RAM requirement?)
	//     - fDebugWindow                (always 0?)

	if (_bookInfoFile.hasSection("Globals")) {
		const Common::ConfigFile::SectionKeyList globals = _bookInfoFile.getKeys("Globals");
		for (Common::ConfigFile::SectionKeyList::const_iterator i = globals.begin(); i != globals.end(); i++) {
			Common::String command = Common::String::format("%s = %s", i->key.c_str(), i->value.c_str());
			LBCode tempCode(this, 0);
			uint offset = tempCode.parseCode(command);
			tempCode.runCode(NULL, offset);
		}
	}
}

Common::String MohawkEngine_LivingBooks::stringForMode(LBMode mode) {
	Common::String language = getStringFromConfig("Languages", Common::String::format("Language%d", _curLanguage));

	switch (mode) {
	case kLBIntroMode:
		return "Intro";
	case kLBControlMode:
		return "Control";
	case kLBCreditsMode:
		return "Credits";
	case kLBPreviewMode:
		return "Preview";
	case kLBReadMode:
		return language + ".Read";
	case kLBPlayMode:
		return language + ".Play";
	default:
		error("unknown game mode %d", (int)mode);
	}
}

void MohawkEngine_LivingBooks::destroyPage() {
	_sound->stopSound();
	_lastSoundOwner = 0;
	_lastSoundId = 0;
	_soundLockOwner = 0;

	_gfx->clearCache();
	_video->stopVideos();

	_eventQueue.clear();

	delete _page;
	assert(_items.empty());
	assert(_orderedItems.empty());
	_page = NULL;

	_notifyEvents.clear();

	_focus = NULL;
}

bool MohawkEngine_LivingBooks::loadPage(LBMode mode, uint page, uint subpage) {
	destroyPage();

	Common::String name = stringForMode(mode);

	Common::String base;
	if (subpage)
		base = Common::String::format("Page%d.%d", page, subpage);
	else
		base = Common::String::format("Page%d", page);

	Common::String filename, leftover;

	filename = getFileNameFromConfig(name, base, leftover);
	_readOnly = false;

	if (filename.empty()) {
		leftover.clear();
		filename = getFileNameFromConfig(name, base + ".r", leftover);
		_readOnly = true;
	}

	// TODO: fading between pages
#if 0
	bool fade = false;
	if (leftover.contains("fade")) {
		fade = true;
	}
#endif

	if (leftover.contains("read")) {
		_readOnly = true;
	}
	if (leftover.contains("load")) {
		// FIXME: don't ignore this
		warning("ignoring 'load' for filename '%s'", filename.c_str());
	}
	if (leftover.contains("cut")) {
		// FIXME: don't ignore this
		warning("ignoring 'cut' for filename '%s'", filename.c_str());
	}
	if (leftover.contains("killgag")) {
		// FIXME: don't ignore this
		warning("ignoring 'killgag' for filename '%s'", filename.c_str());
	}

	Archive *pageArchive = createArchive();
	if (!filename.empty() && pageArchive->openFile(filename)) {
		_page = new LBPage(this);
		_page->open(pageArchive, 1000);
	} else {
		delete pageArchive;
		debug(2, "Could not find page %d.%d for '%s'", page, subpage, name.c_str());
		return false;
	}

	if (getFeatures() & GF_LB_10) {
		if (_readOnly) {
			error("found .r entry in Living Books 1.0 game");
		} else {
			// some very early versions of the LB engine don't have
			// .r entries in their book info; instead, it is just hardcoded
			// like this (which would unfortunately break later games)
			_readOnly = (mode != kLBControlMode && mode != kLBPlayMode);
		}
	}

	debug(1, "Page Version: %d", _page->getResourceVersion());

	_curMode = mode;
	_curPage = page;
	_curSubPage = subpage;

	_cursor->showCursor();

	_gfx->setPalette(1000);

	_phase = 0;
	_introDone = false;

	_needsRedraw = true;

	return true;
}

void MohawkEngine_LivingBooks::updatePage() {
	switch (_phase) {
	case kLBPhaseInit:
		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(kLBPhaseCreate);

		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		if (_curMode == kLBControlMode) {
			// hard-coded control page startup
			LBItem *item;

			uint16 page = _curPage;
			if (getFeatures() & GF_LB_10) {
				// Living Books 1.0 had the meanings of these pages reversed
				if (page == 2)
					page = 3;
				else if (page == 3)
					page = 2;
			}

			switch (page) {
			case 1:
				debug(2, "updatePage() for control page 1 (menu)");

				if (_poetryMode) {
					for (uint16 i = 0; i < _numPages; i++) {
						item = getItemById(1000 + i);
						if (item)
							item->setVisible(_curSelectedPage == i + 1);
						item = getItemById(1100 + i);
						if (item)
							item->setVisible(_curSelectedPage == i + 1);
					}
				}

				for (uint16 i = 0; i < _numLanguages; i++) {
					item = getItemById(100 + i);
					if (item)
						item->seek((i + 1 == _curLanguage) ? 0xFFFF : 1);
					item = getItemById(200 + i);
					if (item)
						item->setVisible(false);
				}

				item = getItemById(12);
				if (item)
					item->setVisible(false);

				if (_alreadyShowedIntro) {
					item = getItemById(10);
					if (item) {
						item->setVisible(false);
						item->seek(0xFFFF);
					}
				} else {
					_alreadyShowedIntro = true;
					item = getItemById(11);
					if (item)
						item->setVisible(false);
				}
				break;

			case 2:
				debug(2, "updatePage() for control page 2 (quit)");

				item = getItemById(12);
				if (item)
					item->setVisible(false);
				item = getItemById(13);
				if (item)
					item->setVisible(false);
				break;

			case 3:
				debug(2, "updatePage() for control page 3 (options)");

				for (uint i = 0; i < _numLanguages; i++) {
					item = getItemById(100 + i);
					if (item)
						item->setVisible(_curLanguage == i + 1);
				}
				for (uint i = 0; i < _numPages; i++) {
					item = getItemById(1000 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
					item = getItemById(1100 + i);
					if (item)
						item->setVisible(_curSelectedPage == i + 1);
				}
				item = getItemById(202);
				if (item)
					item->setVisible(false);
				break;
			}
		}
		_phase++;
		break;

	case kLBPhaseIntro:
		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		if (_curMode == kLBControlMode) {
			LBItem *item = getItemById(10);
			if (item)
				item->togglePlaying(false);
		}

		_phase++;
		break;

	case kLBPhaseMain:
		if (!_introDone)
			break;

		for (uint32 i = 0; i < _items.size(); i++)
			_items[i]->startPhase(_phase);

		_phase++;
		break;
	}

	while (_eventQueue.size()) {
		DelayedEvent delayedEvent = _eventQueue.pop();
		for (uint32 i = 0; i < _items.size(); i++) {
			if (_items[i] != delayedEvent.item)
				continue;

			switch (delayedEvent.type) {
			case kLBDelayedEventDestroy:
				_items.remove_at(i);
				i--;
				_orderedItems.remove(delayedEvent.item);
				delete delayedEvent.item;
				_page->itemDestroyed(delayedEvent.item);
				if (_focus == delayedEvent.item)
					_focus = NULL;
				break;
			case kLBDelayedEventSetNotVisible:
				_items[i]->setVisible(false);
				break;
			case kLBDelayedEventDone:
				_items[i]->done(true);
				break;
			}

			break;
		}
	}

	for (uint16 i = 0; i < _items.size(); i++)
		_items[i]->update();

	if (_needsRedraw) {
		for (Common::List<LBItem *>::const_iterator i = _orderedItems.reverse_begin(); i != _orderedItems.end(); --i)
			(*i)->draw();

		_needsRedraw = false;
		_needsUpdate = true;
	}
}

void MohawkEngine_LivingBooks::addArchive(Archive *archive) {
	_mhk.push_back(archive);
}

void MohawkEngine_LivingBooks::removeArchive(Archive *archive) {
	for (uint i = 0; i < _mhk.size(); i++) {
		if (archive != _mhk[i])
			continue;
		_mhk.remove_at(i);
		return;
	}

	error("removeArchive didn't find archive");
}

void MohawkEngine_LivingBooks::addItem(LBItem *item) {
	_items.push_back(item);
	_orderedItems.push_front(item);
	item->_iterator = _orderedItems.begin();
}

void MohawkEngine_LivingBooks::removeItems(const Common::Array<LBItem *> &items) {
	for (uint i = 0; i < items.size(); i++) {
		bool found = false;
		for (uint16 j = 0; j < _items.size(); j++) {
			if (items[i] != _items[j])
				continue;
			found = true;
			_items.remove_at(j);
			break;
		}
		assert(found);
		_orderedItems.erase(items[i]->_iterator);
	}
}

LBItem *MohawkEngine_LivingBooks::getItemById(uint16 id) {
	for (uint16 i = 0; i < _items.size(); i++)
		if (_items[i]->getId() == id)
			return _items[i];

	return NULL;
}

LBItem *MohawkEngine_LivingBooks::getItemByName(Common::String name) {
	for (uint16 i = 0; i < _items.size(); i++)
		if (_items[i]->getName() == name)
			return _items[i];

	return NULL;
}

void MohawkEngine_LivingBooks::setFocus(LBItem *focus) {
	_focus = focus;
}

void MohawkEngine_LivingBooks::setEnableForAll(bool enable, LBItem *except) {
	for (uint16 i = 0; i < _items.size(); i++)
		if (except != _items[i])
			_items[i]->setEnabled(enable);
}

void MohawkEngine_LivingBooks::notifyAll(uint16 data, uint16 from) {
	for (uint16 i = 0; i < _items.size(); i++)
		_items[i]->notify(data, from);
}

void MohawkEngine_LivingBooks::queueDelayedEvent(DelayedEvent event) {
	_eventQueue.push(event);
}

bool MohawkEngine_LivingBooks::playSound(LBItem *source, uint16 resourceId) {
	if (_lastSoundId && !_sound->isPlaying(_lastSoundId))
		_lastSoundId = 0;

	if (!source->isAmbient() || !_sound->isPlaying()) {
		if (!_soundLockOwner) {
			if (_lastSoundId && _lastSoundOwner != source->getId())
				if (source->getSoundPriority() >= _lastSoundPriority)
					return false;
		} else {
			if (_soundLockOwner != source->getId() && source->getSoundPriority() >= _maxSoundPriority)
				return false;
		}

		if (_lastSoundId)
			_sound->stopSound(_lastSoundId);

		_lastSoundOwner = source->getId();
		_lastSoundPriority = source->getSoundPriority();
	}

	_lastSoundId = resourceId;
	_sound->playSound(resourceId);

	return true;
}

void MohawkEngine_LivingBooks::lockSound(LBItem *owner, bool lock) {
	if (!lock) {
		_soundLockOwner = 0;
		return;
	}

	if (_soundLockOwner || (owner->isAmbient() && _sound->isPlaying()))
		return;

	if (_lastSoundId && !_sound->isPlaying(_lastSoundId))
		_lastSoundId = 0;

	_soundLockOwner = owner->getId();
	_maxSoundPriority = owner->getSoundPriority();
	if (_lastSoundId && _maxSoundPriority <= _lastSoundPriority) {
		_sound->stopSound(_lastSoundId);
		_lastSoundId = 0;
	}
}

// Only 1 VSRN resource per page
uint16 LBPage::getResourceVersion() {
	Common::SeekableReadStream *versionStream = _vm->getResource(ID_VRSN, _baseId);

	// FIXME: some V2 games have very strange version entries
	if (versionStream->size() != 2)
		debug(1, "Version Record size mismatch");

	uint16 version = versionStream->readUint16BE();

	delete versionStream;
	return version;
}

void LBPage::loadBITL(uint16 resourceId) {
	Common::SeekableSubReadStreamEndian *bitlStream = _vm->wrapStreamEndian(ID_BITL, resourceId);

	while (true) {
		Common::Rect rect = _vm->readRect(bitlStream);
		uint16 type = bitlStream->readUint16();

		LBItem *res;
		switch (type) {
		case kLBPictureItem:
			res = new LBPictureItem(_vm, this, rect);
			break;
		case kLBAnimationItem:
			res = new LBAnimationItem(_vm, this, rect);
			break;
		case kLBPaletteItem:
			res = new LBPaletteItem(_vm, this, rect);
			break;
		case kLBGroupItem:
			res = new LBGroupItem(_vm, this, rect);
			break;
		case kLBSoundItem:
			res = new LBSoundItem(_vm, this, rect);
			break;
		case kLBLiveTextItem:
			res = new LBLiveTextItem(_vm, this, rect);
			break;
		case kLBMovieItem:
			res = new LBMovieItem(_vm, this, rect);
			break;
		case kLBMiniGameItem:
			res = new LBMiniGameItem(_vm, this, rect);
			break;
		case kLBProxyItem:
			res = new LBProxyItem(_vm, this, rect);
			break;
		default:
			warning("Unknown item type %04x", type);
		case 3: // often used for buttons
			res = new LBItem(_vm, this, rect);
			break;
		}

		res->readFrom(bitlStream);
		_items.push_back(res);

		if (bitlStream->size() == bitlStream->pos())
			break;
	}

	delete bitlStream;
}

Common::SeekableSubReadStreamEndian *MohawkEngine_LivingBooks::wrapStreamEndian(uint32 tag, uint16 id) {
	Common::SeekableReadStream *dataStream = getResource(tag, id);
	return new Common::SeekableSubReadStreamEndian(dataStream, 0, dataStream->size(), isBigEndian(), DisposeAfterUse::YES);
}

Common::String MohawkEngine_LivingBooks::getStringFromConfig(const Common::String &section, const Common::String &key) {
	Common::String x, leftover;
	_bookInfoFile.getKey(key, section, x);
	Common::String tmp = removeQuotesFromString(x, leftover);
	if (!leftover.empty())
		warning("while parsing config key '%s' from section '%s', string '%s' was left after '%s'",
			key.c_str(), section.c_str(), leftover.c_str(), tmp.c_str());
	return tmp;
}

Common::String MohawkEngine_LivingBooks::getStringFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover) {
	Common::String x;
	_bookInfoFile.getKey(key, section, x);
	return removeQuotesFromString(x, leftover);
}

int MohawkEngine_LivingBooks::getIntFromConfig(const Common::String &section, const Common::String &key) {
	return atoi(getStringFromConfig(section, key).c_str());
}

Common::String MohawkEngine_LivingBooks::getFileNameFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover) {
	Common::String string = getStringFromConfig(section, key, leftover);
	Common::String x;

	uint32 i = 0;
	if (string.hasPrefix("//")) {
		// skip "//CD-ROM Title/" prefixes which we don't care about
		i = 3;
		while (i < string.size() && string[i - 1] != '/')
			i++;
	}
	x = string.c_str() + i;

	return (getPlatform() == Common::kPlatformMacintosh) ? convertMacFileName(x) : convertWinFileName(x);
}

Common::String MohawkEngine_LivingBooks::removeQuotesFromString(const Common::String &string, Common::String &leftover) {
	if (string.empty())
		return string;

	char quoteChar = string[0];
	if (quoteChar != '\"' && quoteChar != '\'')
		return string;

	Common::String tmp;
	bool inLeftover = false;
	for (uint32 i = 1; i < string.size(); i++) {
		if (inLeftover)
			leftover += string[i];
		else if (string[i] == quoteChar)
			inLeftover = true;
		else
			tmp += string[i];
	}

	return tmp;
}

Common::String MohawkEngine_LivingBooks::convertMacFileName(const Common::String &string) {
	Common::String filename;

	for (uint32 i = 0; i < string.size(); i++) {
		if (i == 0 && string[i] == ':') // First character should be ignored (another colon)
			continue;
		if (string[i] == ':')
			filename += '/';
		else
			filename += string[i];
	}

	return filename;
}

Common::String MohawkEngine_LivingBooks::convertWinFileName(const Common::String &string) {
	Common::String filename;

	for (uint32 i = 0; i < string.size(); i++) {
		if (i == 0 && (string[i] == '/' || string[i] == '\\')) // ignore slashes at start
			continue;
		if (string[i] == '\\')
			filename += '/';
		else
			filename += string[i];
	}

	return filename;
}

Archive *MohawkEngine_LivingBooks::createArchive() const {
	if (isPreMohawk())
		return new LivingBooksArchive_v1();

	return new MohawkArchive();
}

bool MohawkEngine_LivingBooks::isPreMohawk() const {
	return getGameType() == GType_LIVINGBOOKSV1
		|| (getGameType() == GType_LIVINGBOOKSV2 && getPlatform() == Common::kPlatformMacintosh);
}

void MohawkEngine_LivingBooks::addNotifyEvent(NotifyEvent event) {
	_notifyEvents.push(event);
}

bool MohawkEngine_LivingBooks::tryLoadPageStart(LBMode mode, uint page) {
	// try first subpage of the page
	if (loadPage(mode, page, 1))
		return true;

	// then just the plain page
	if (loadPage(mode, page, 0))
		return true;

	return false;
}

bool MohawkEngine_LivingBooks::tryDefaultPage() {
	if (_curMode == kLBCreditsMode || _curMode == kLBPreviewMode) {
		// go to options page
		if (getFeatures() & GF_LB_10) {
			if (tryLoadPageStart(kLBControlMode, 2))
				return true;
		} else {
			if (tryLoadPageStart(kLBControlMode, 3))
				return true;
		}
	}

	// go to menu page
	if (tryLoadPageStart(kLBControlMode, 1))
		return true;

	return false;
}

void MohawkEngine_LivingBooks::prevPage() {
	if (_curPage > 1 && (tryLoadPageStart(_curMode, _curPage - 1)))
		return;

	if (tryDefaultPage())
		return;

	error("Could not find page before %d.%d for mode %d", _curPage, _curSubPage, (int)_curMode);
}

void MohawkEngine_LivingBooks::nextPage() {
	// we try the next subpage first
	if (loadPage(_curMode, _curPage, _curSubPage + 1))
		return;

	if (tryLoadPageStart(_curMode, _curPage + 1))
		return;

	if (tryDefaultPage())
		return;

	error("Could not find page after %d.%d for mode %d", _curPage, _curSubPage, (int)_curMode);
}

void MohawkEngine_LivingBooks::handleUIMenuClick(uint controlId) {
	LBItem *item;

	switch (controlId) {
	case 1:
		if (getFeatures() & GF_LB_10) {
			if (!tryLoadPageStart(kLBControlMode, 2))
				error("couldn't load options page");
		} else {
			if (!tryLoadPageStart(kLBControlMode, 3))
				error("couldn't load options page");
		}
		break;

	case 2:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById(199 + _curLanguage);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 3:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById(12);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 4:
		if (getFeatures() & GF_LB_10) {
			if (!tryLoadPageStart(kLBControlMode, 3))
				error("couldn't load quit page");
		} else {
			if (!tryLoadPageStart(kLBControlMode, 2))
				error("couldn't load quit page");
		}
		break;

	case 10:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false);
		}
		break;

	case 11:
		item = getItemById(11);
		if (item)
			item->togglePlaying(false, true);
		break;

	case 12:
		// start game, in play mode
		if (!tryLoadPageStart(kLBPlayMode, 1))
			error("couldn't start play mode");
		break;

	default:
		if (controlId >= 100 && controlId < 100 + (uint)_numLanguages) {
			uint newLanguage = controlId - 99;
			if (newLanguage == _curLanguage)
				break;
			item = getItemById(99 + _curLanguage);
			if (item)
				item->seek(1);
			_curLanguage = newLanguage;
		} else if (controlId >= 200 && controlId < 200 + (uint)_numLanguages) {
			// start game, in read mode
			if (!tryLoadPageStart(kLBReadMode, 1))
				error("couldn't start read mode");
		}
		break;
	}
}

void MohawkEngine_LivingBooks::handleUIPoetryMenuClick(uint controlId) {
	LBItem *item;

	// the menu UI in New Kid on the Block is a hybrid of the normal menu
	// and the normal options screen

	// TODO: this is mostly untested

	switch (controlId) {
	case 2:
	case 3:
		handleUIOptionsClick(controlId);
		break;

	case 4:
		handleUIMenuClick(controlId);
		break;

	case 6:
		handleUIMenuClick(2);
		break;

	case 7:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById(12);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 0xA:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false);
		}
		break;

	case 0xB:
		item = getItemById(11);
		if (item)
			item->togglePlaying(false, true);
		break;

	case 0xC:
		if (!tryLoadPageStart(kLBPlayMode, _curSelectedPage))
			error("failed to load page %d", _curSelectedPage);
		break;

	default:
		if (controlId < 100) {
			handleUIMenuClick(controlId);
		} else {
			if (!tryLoadPageStart(kLBReadMode, _curSelectedPage))
				error("failed to load page %d", _curSelectedPage);
		}
	}
}

void MohawkEngine_LivingBooks::handleUIQuitClick(uint controlId) {
	LBItem *item;

	switch (controlId) {
	case 1:
	case 2:
		// button clicked, run animation
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById((controlId == 1) ? 12 : 13);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false);
		}
		break;

	case 10:
	case 11:
		item = getItemById(11);
		if (item)
			item->togglePlaying(false, true);
		break;

	case 12:
		// 'yes', I want to quit
		quitGame();
		break;

	case 13:
		// 'no', go back to menu
		if (!tryLoadPageStart(kLBControlMode, 1))
			error("couldn't return to menu");
		break;
	}
}

void MohawkEngine_LivingBooks::handleUIOptionsClick(uint controlId) {
	LBItem *item;

	switch (controlId) {
	case 1:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(202);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 2:
		// back
		item = getItemById(2);
		if (item)
			item->seek(1);
		if (_curSelectedPage == 1) {
			_curSelectedPage = _numPages;
		} else {
			_curSelectedPage--;
		}
		for (uint i = 0; i < _numPages; i++) {
			item = getItemById(1000 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
			item = getItemById(1100 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
		}
		break;

	case 3:
		// forward
		item = getItemById(3);
		if (item)
			item->seek(1);
		if (_curSelectedPage == _numPages) {
			_curSelectedPage = 1;
		} else {
			_curSelectedPage++;
		}
		for (uint i = 0; i < _numPages; i++) {
			item = getItemById(1000 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
			item = getItemById(1100 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
		}
		break;

	case 4:
		if (!tryLoadPageStart(kLBCreditsMode, 1))
			error("failed to start credits");
		break;

	case 5:
		if (!tryLoadPageStart(kLBPreviewMode, 1))
			error("failed to start preview");
		break;

	case 202:
		if (!tryLoadPageStart(kLBPlayMode, _curSelectedPage))
			error("failed to load page %d", _curSelectedPage);
		break;
	}
}

void MohawkEngine_LivingBooks::handleNotify(NotifyEvent &event) {
	// hard-coded behavior (GUI/navigation)

	switch (event.type) {
	case kLBNotifyGUIAction:
		debug(2, "kLBNotifyGUIAction: %d", event.param);

		if (_curMode != kLBControlMode)
			break;

		// The scripting passes us the control ID as param, so we work
		// out which control was clicked, then run the relevant code.

		uint16 page;
		page = _curPage;
		if (getFeatures() & GF_LB_10) {
			// Living Books 1.0 had the meanings of these pages reversed
			if (page == 2)
				page = 3;
			else if (page == 3)
				page = 2;
		}

		switch (page) {
		case 1:
			// main menu
			if (_poetryMode)
				handleUIPoetryMenuClick(event.param);
			else
				handleUIMenuClick(event.param);
			break;

		case 2:
			// quit screen
			handleUIQuitClick(event.param);
			break;

		case 3:
			// options screen
			handleUIOptionsClick(event.param);
			break;
		}
		break;

	case kLBNotifyGoToControls:
		debug(2, "kLBNotifyGoToControls: %d", event.param);

		if (!tryLoadPageStart(kLBControlMode, 1))
			error("couldn't load controls page");
		break;

	case kLBNotifyChangePage:
		switch (event.param) {
		case 0xfffe:
			debug(2, "kLBNotifyChangePage: next page");
			nextPage();
			return;

		case 0xffff:
			debug(2, "kLBNotifyChangePage: previous page");
			prevPage();
			break;

		default:
			debug(2, "kLBNotifyChangePage: trying %d", event.param);
			if (!tryLoadPageStart(_curMode, event.param)) {
				if (!tryDefaultPage()) {
					error("failed to load default page after change to page %d (mode %d) failed", event.param, _curMode);
				}
			}
			break;
		}
		break;

	case kLBNotifyGotoQuit:
		debug(2, "kLBNotifyGotoQuit: %d", event.param);

		if (!tryLoadPageStart(kLBControlMode, 2))
			error("couldn't load quit page");
		break;

	case kLBNotifyIntroDone:
		debug(2, "kLBNotifyIntroDone: %d", event.param);

		if (event.param != 1)
			break;

		_introDone = true;

		// TODO: if !_readOnly, go to next page (-2 case above)
		// if in older one (not in e.g. 1.4 w/tortoise),
		//   if mode is 6 (kLBPlayMode?), go to next page (-2 case) if curr page > nPages (i.e. the end)
		// else, nothing

		if (!_readOnly)
			break;

		nextPage();
		break;

	case kLBNotifyChangeMode:
		if (getGameType() == GType_LIVINGBOOKSV1) {
			debug(2, "kLBNotifyChangeMode: %d", event.param);
			quitGame();
			break;
		}

		debug(2, "kLBNotifyChangeMode: v2 type %d", event.param);
		switch (event.param) {
		case 1:
			debug(2, "kLBNotifyChangeMode:, mode %d, page %d.%d",
				event.newMode, event.newPage, event.newSubpage);
			// TODO: what is entry.newUnknown?
			if (!event.newMode)
				event.newMode = _curMode;
			if (!loadPage((LBMode)event.newMode, event.newPage, event.newSubpage)) {
				if (event.newPage != 0 || !loadPage((LBMode)event.newMode, _curPage, event.newSubpage))
					if (event.newSubpage != 0 || !loadPage((LBMode)event.newMode, event.newPage, 1))
						error("kLBNotifyChangeMode failed to move to mode %d, page %d.%d",
							event.newMode, event.newPage, event.newSubpage);
			}
			break;
		case 3:
			debug(2, "kLBNotifyChangeMode: new cursor '%s'", event.newCursor.c_str());
			_cursor->setCursor(event.newCursor);
			break;
		default:
			error("unknown v2 kLBNotifyChangeMode type %d", event.param);
		}
		break;

	case kLBNotifyCursorChange:
		debug(2, "kLBNotifyCursorChange: %d", event.param);

		// TODO: show/hide cursor according to parameter?
		break;

	case kLBNotifyPrintPage:
		debug(2, "kLBNotifyPrintPage: %d", event.param);

		warning("kLBNotifyPrintPage unimplemented");
		break;

	case kLBNotifyQuit:
		debug(2, "kLBNotifyQuit: %d", event.param);

		quitGame();
		break;

	default:
		error("Unknown notification %d (param 0x%04x)", event.type, event.param);
	}
}

LBAnimationNode::LBAnimationNode(MohawkEngine_LivingBooks *vm, LBAnimation *parent, uint16 scriptResourceId) : _vm(vm), _parent(parent) {
	loadScript(scriptResourceId);
}

LBAnimationNode::~LBAnimationNode() {
	for (uint32 i = 0; i < _scriptEntries.size(); i++)
		delete[] _scriptEntries[i].data;
}

void LBAnimationNode::loadScript(uint16 resourceId) {
	Common::SeekableSubReadStreamEndian *scriptStream = _vm->wrapStreamEndian(ID_SCRP, resourceId);

	reset();

	while (byte opcodeId = scriptStream->readByte()) {
		byte size = scriptStream->readByte();

		LBAnimScriptEntry entry;
		entry.opcode = opcodeId;
		entry.size = size;

		if (!size) {
			entry.data = NULL;
		} else {
			entry.data = new byte[entry.size];
			scriptStream->read(entry.data, entry.size);
		}

		_scriptEntries.push_back(entry);
	}

	byte size = scriptStream->readByte();
	if (size != 0 || scriptStream->pos() != scriptStream->size())
		error("Failed to read script correctly");

	delete scriptStream;
}

void LBAnimationNode::draw(const Common::Rect &_bounds) {
	if (!_currentCel)
		return;

	// this is also checked in SetCel, below
	if (_currentCel > _parent->getNumResources())
		error("Animation cel %d was too high, this shouldn't happen!", _currentCel);

	int16 xOffset = _xPos + _bounds.left;
	int16 yOffset = _yPos + _bounds.top;

	uint16 resourceId = _parent->getResource(_currentCel - 1);

	if (!_vm->isPreMohawk()) {
		Common::Point offset = _parent->getOffset(_currentCel - 1);
		xOffset -= offset.x;
		yOffset -= offset.y;
	}

	_vm->_gfx->copyOffsetAnimImageToScreen(resourceId, xOffset, yOffset);
}

void LBAnimationNode::reset() {
	// TODO: this causes stupid flickering
	//if (_currentCel)
	//	_vm->_needsRedraw = true;

	_currentCel = 0;
	_currentEntry = 0;
	_delay = 0;

	_xPos = 0;
	_yPos = 0;
}

NodeState LBAnimationNode::update(bool seeking) {
	if (_currentEntry == _scriptEntries.size())
		return kLBNodeDone;

	if (_delay > 0 && --_delay)
		return kLBNodeRunning;

	while (_currentEntry < _scriptEntries.size()) {
		LBAnimScriptEntry &entry = _scriptEntries[_currentEntry];
		_currentEntry++;
		debug(5, "Running script entry %d of %d", _currentEntry, _scriptEntries.size());

		switch (entry.opcode) {
		case kLBAnimOpPlaySound:
		case kLBAnimOpWaitForSound:
		case kLBAnimOpReleaseSound:
		case kLBAnimOpResetSound:
			{
			uint16 soundResourceId = READ_BE_UINT16(entry.data);

			if (!soundResourceId) {
				error("Unhandled named wave file, tell clone2727 where you found this");
				break;
			}

			Common::String cue;
			uint pos = 2;
			while (pos < entry.size) {
				char in = entry.data[pos];
				if (!in)
					break;
				pos++;
				cue += in;
			}
			if (pos == entry.size)
				error("Cue in sound kLBAnimOp wasn't null-terminated");

			switch (entry.opcode) {
			case kLBAnimOpPlaySound:
				if (seeking)
					break;
				debug(4, "a: PlaySound(%0d)", soundResourceId);
				_parent->playSound(soundResourceId);
				break;
			case kLBAnimOpWaitForSound:
				if (seeking)
					break;
				debug(4, "b: WaitForSound(%0d)", soundResourceId);
				if (!_parent->soundPlaying(soundResourceId, cue))
					break;
				_currentEntry--;
				return kLBNodeWaiting;
			case kLBAnimOpReleaseSound:
				debug(4, "c: ReleaseSound(%0d)", soundResourceId);
				// TODO
				_vm->_sound->stopSound(soundResourceId);
				break;
			case kLBAnimOpResetSound:
				debug(4, "d: ResetSound(%0d)", soundResourceId);
				// TODO
				_vm->_sound->stopSound(soundResourceId);
				break;
			}
			}
			break;

		case kLBAnimOpSetTempo:
		case kLBAnimOpSetTempoDiv:
			{
			assert(entry.size == 2);
			uint16 tempo = (int16)READ_BE_UINT16(entry.data);

			// TODO: LB 3 uses fixed-point here.
			if (entry.opcode == kLBAnimOpSetTempo) {
				debug(4, "3: SetTempo(%d)", tempo);
				// TODO: LB 3 uses (tempo * 1000) / 60, while
				// the original divides the system time by 16.
				_parent->setTempo(tempo * 16);
			} else {
				// LB 3.0+ only.
				debug(4, "E: SetTempoDiv(%d)", tempo);
				_parent->setTempo(1000 / tempo);
			}

			}
			break;

		case kLBAnimOpWait:
			assert(entry.size == 0);
			debug(5, "6: Wait()");
			return kLBNodeRunning;

		case kLBAnimOpMoveTo:
			{
			assert(entry.size == 4);
			int16 x = (int16)READ_BE_UINT16(entry.data);
			int16 y = (int16)READ_BE_UINT16(entry.data + 2);
			debug(4, "5: MoveTo(%d, %d)", x, y);

			_xPos = x;
			_yPos = y;
			_vm->_needsRedraw = true;
			}
			break;

		case kLBAnimOpDrawMode:
			{
			assert(entry.size == 2);
			uint16 mode = (int16)READ_BE_UINT16(entry.data);
			debug(4, "9: DrawMode(%d)", mode);

			// TODO
			}
			break;

		case kLBAnimOpSetCel:
			{
			assert(entry.size == 2);
			uint16 cel = (int16)READ_BE_UINT16(entry.data);
			debug(4, "7: SetCel(%d)", cel);

			_currentCel = cel;
			if (_currentCel > _parent->getNumResources())
				error("SetCel set current cel to %d, but we only have %d cels", _currentCel, _parent->getNumResources());
			_vm->_needsRedraw = true;
			}
			break;

		case kLBAnimOpNotify:
			{
			assert(entry.size == 2);
			uint16 data = (int16)READ_BE_UINT16(entry.data);

			if (seeking)
				break;

			debug(4, "2: Notify(%d)", data);
			_vm->notifyAll(data, _parent->getParentId());
			}
			break;

		case kLBAnimOpSleepUntil:
			{
			assert(entry.size == 4);
			uint32 frame = READ_BE_UINT32(entry.data);
			debug(4, "8: SleepUntil(%d)", frame);

			if (frame > _parent->getCurrentFrame()) {
				// *not* kLBNodeWaiting
				_currentEntry--;
				return kLBNodeRunning;
			}
			}
			break;

		case kLBAnimOpDelay:
			{
			assert(entry.size == 4);
			uint32 delay = READ_BE_UINT32(entry.data);
			debug(4, "f: Delay(%d)", delay);
			_delay = delay;
			return kLBNodeRunning;
			}
			break;

		default:
			error("Unknown opcode id %02x (size %d)", entry.opcode, entry.size);
			break;
		}
	}

	return kLBNodeRunning;
}

bool LBAnimationNode::transparentAt(int x, int y) {
	if (!_currentCel)
		return true;

	uint16 resourceId = _parent->getResource(_currentCel - 1);

	if (!_vm->isPreMohawk()) {
		Common::Point offset = _parent->getOffset(_currentCel - 1);
		x += offset.x;
		y += offset.y;
	}

	// TODO: only check pixels if necessary
	return _vm->_gfx->imageIsTransparentAt(resourceId, true, x - _xPos, y - _yPos);
}

LBAnimation::LBAnimation(MohawkEngine_LivingBooks *vm, LBAnimationItem *parent, uint16 resourceId) : _vm(vm), _parent(parent) {
	Common::SeekableSubReadStreamEndian *aniStream = _vm->wrapStreamEndian(ID_ANI, resourceId);

	if (aniStream->size() != 30)
		warning("ANI Record size mismatch");

	uint16 version = aniStream->readUint16();
	if (version != 1)
		warning("ANI version not 1");

	_bounds = _vm->readRect(aniStream);
	_clip = _vm->readRect(aniStream);
	// TODO: what is colorId for?
	uint32 colorId = aniStream->readUint32();
	uint32 sprResourceId = aniStream->readUint32();
	uint32 sprResourceOffset = aniStream->readUint32();

	debug(5, "ANI bounds: (%d, %d), (%d, %d)", _bounds.left, _bounds.top, _bounds.right, _bounds.bottom);
	debug(5, "ANI clip: (%d, %d), (%d, %d)", _clip.left, _clip.top, _clip.right, _clip.bottom);
	debug(5, "ANI color id: %d", colorId);
	debug(5, "ANI SPRResourceId: %d, offset %d", sprResourceId, sprResourceOffset);

	if (aniStream->pos() != aniStream->size())
		error("Still %d bytes at the end of anim stream", aniStream->size() - aniStream->pos());

	delete aniStream;

	if (sprResourceOffset)
		error("Cannot handle non-zero ANI offset yet");

	Common::SeekableSubReadStreamEndian *sprStream = _vm->wrapStreamEndian(ID_SPR, sprResourceId);

	uint16 numBackNodes = sprStream->readUint16();
	uint16 numFrontNodes = sprStream->readUint16();
	uint32 shapeResourceID = sprStream->readUint32();
	uint32 shapeResourceOffset = sprStream->readUint32();
	uint32 scriptResourceID = sprStream->readUint32();
	uint32 scriptResourceOffset = sprStream->readUint32();
	uint32 scriptResourceLength = sprStream->readUint32();
	debug(5, "SPR# stream: %d front, %d background", numFrontNodes, numBackNodes);
	debug(5, "Shape ID %d (offset 0x%04x), script ID %d (offset 0x%04x, length %d)", shapeResourceID, shapeResourceOffset,
		scriptResourceID, scriptResourceOffset, scriptResourceLength);

	Common::Array<uint16> scriptIDs;
	for (uint16 i = 0; i < numFrontNodes; i++) {
		uint32 unknown1 = sprStream->readUint32();
		uint32 unknown2 = sprStream->readUint32();
		uint32 unknown3 = sprStream->readUint32();
		uint16 scriptID = sprStream->readUint32();
		uint32 unknown4 = sprStream->readUint32();
		uint32 unknown5 = sprStream->readUint32();
		scriptIDs.push_back(scriptID);
		debug(6, "Front node %d: script ID %d", i, scriptID);
		if (unknown1 != 0 || unknown2 != 0 || unknown3 != 0 || unknown4 != 0 || unknown5 != 0)
			error("Anim node %d had non-zero unknowns %08x, %08x, %08x, %08x, %08x",
				i, unknown1, unknown2, unknown3, unknown4, unknown5);
	}

	if (numBackNodes)
		error("Ignoring %d back nodes", numBackNodes);

	if (sprStream->pos() != sprStream->size())
		error("Still %d bytes at the end of sprite stream", sprStream->size() - sprStream->pos());

	delete sprStream;

	loadShape(shapeResourceID);

	_nodes.push_back(new LBAnimationNode(_vm, this, scriptResourceID));
	for (uint16 i = 0; i < scriptIDs.size(); i++)
		_nodes.push_back(new LBAnimationNode(_vm, this, scriptIDs[i]));

	_currentFrame = 0;
	_currentSound = 0xffff;
	_running = false;
	_tempo = 1;
}

LBAnimation::~LBAnimation() {
	for (uint32 i = 0; i < _nodes.size(); i++)
		delete _nodes[i];
	if (_currentSound != 0xffff)
		_vm->_sound->stopSound(_currentSound);
}

void LBAnimation::loadShape(uint16 resourceId) {
	if (resourceId == 0)
		return;

	Common::SeekableSubReadStreamEndian *shapeStream = _vm->wrapStreamEndian(ID_SHP, resourceId);

	if (_vm->isPreMohawk()) {
		if (shapeStream->size() < 6)
			error("V1 SHP Record size too short (%d)", shapeStream->size());

		uint16 u0 = shapeStream->readUint16();
		if (u0 != 3)
			error("V1 SHP Record u0 is %04x, not 3", u0);

		uint16 u1 = shapeStream->readUint16();
		if (u1 != 0)
			error("V1 SHP Record u1 is %04x, not 0", u1);

		uint16 idCount = shapeStream->readUint16();
		debug(8, "V1 SHP: idCount: %d", idCount);

		if (shapeStream->size() != (idCount * 2) + 6)
			error("V1 SHP Record size mismatch (%d)", shapeStream->size());

		for (uint16 i = 0; i < idCount; i++) {
			_shapeResources.push_back(shapeStream->readUint16());
			debug(8, "V1 SHP: BMAP Resource Id %d: %d", i, _shapeResources[i]);
		}
	} else {
		uint16 idCount = shapeStream->readUint16();
		debug(8, "SHP: idCount: %d", idCount);

		if (shapeStream->size() != (idCount * 6) + 2)
			error("SHP Record size mismatch (%d)", shapeStream->size());

		for (uint16 i = 0; i < idCount; i++) {
			_shapeResources.push_back(shapeStream->readUint16());
			int16 x = shapeStream->readSint16();
			int16 y = shapeStream->readSint16();
			_shapeOffsets.push_back(Common::Point(x, y));
			debug(8, "SHP: tBMP Resource Id %d: %d, at (%d, %d)", i, _shapeResources[i], x, y);
		}
	}

	for (uint16 i = 0; i < _shapeResources.size(); i++)
		_vm->_gfx->preloadImage(_shapeResources[i]);

	delete shapeStream;
}

void LBAnimation::draw() {
	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->draw(_bounds);
}

bool LBAnimation::update() {
	if (!_running)
		return false;

	if (_vm->_system->getMillis() <= _lastTime + (uint32)_tempo)
		return false;

	// the second check is to try 'catching up' with lagged animations, might be crazy
	if (_lastTime == 0 || (_vm->_system->getMillis()) > _lastTime + (uint32)(_tempo * 2))
		_lastTime = _vm->_system->getMillis();
	else
		_lastTime += _tempo;

	if (_currentSound != 0xffff && !_vm->_sound->isPlaying(_currentSound)) {
		_currentSound = 0xffff;
	}

	NodeState state = kLBNodeDone;
	for (uint32 i = 0; i < _nodes.size(); i++) {
		NodeState s = _nodes[i]->update();
		if (s == kLBNodeWaiting) {
			state = kLBNodeWaiting;
			if (i != 0)
				warning("non-primary node was waiting");
			break;
		}
		if (s == kLBNodeRunning)
			state = kLBNodeRunning;
	}

	if (state == kLBNodeRunning) {
		_currentFrame++;
	} else if (state == kLBNodeDone) {
		if (_currentSound == 0xffff) {
			_running = false;
			return true;
		}
	}

	return false;
}

void LBAnimation::start() {
	_lastTime = 0;
	_running = true;
}

void LBAnimation::seek(uint16 pos) {
	_lastTime = 0;
	_currentFrame = 0;

	if (_currentSound != 0xffff) {
		_vm->_sound->stopSound(_currentSound);
		_currentSound = 0xffff;
	}

	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->reset();

	for (uint16 n = 0; n < pos; n++) {
		bool ranSomething = false;
		// nodes don't wait while seeking
		for (uint32 i = 0; i < _nodes.size(); i++)
			ranSomething |= (_nodes[i]->update(true) != kLBNodeDone);

		_currentFrame++;

		if (!ranSomething) {
			_running = false;
			break;
		}
	}
}

void LBAnimation::seekToTime(uint32 time) {
	_lastTime = 0;
	_currentFrame = 0;

	if (_currentSound != 0xffff) {
		_vm->_sound->stopSound(_currentSound);
		_currentSound = 0xffff;
	}

	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->reset();

	uint32 elapsed = 0;
	while (elapsed <= time) {
		bool ranSomething = false;
		// nodes don't wait while seeking
		for (uint32 i = 0; i < _nodes.size(); i++)
			ranSomething |= (_nodes[i]->update(true) != kLBNodeDone);

		elapsed += _tempo;
		_currentFrame++;

		if (!ranSomething) {
			_running = false;
			break;
		}
	}
}

void LBAnimation::stop() {
	_running = false;
	if (_currentSound != 0xffff) {
		_vm->_sound->stopSound(_currentSound);
		_currentSound = 0xffff;
	}
}

void LBAnimation::playSound(uint16 resourceId) {
	_currentSound = resourceId;
	_vm->_sound->playSound(_currentSound, Audio::Mixer::kMaxChannelVolume, false, &_cueList);
}

bool LBAnimation::soundPlaying(uint16 resourceId, const Common::String &cue) {
	if (_currentSound != resourceId)
		return false;
	if (!_vm->_sound->isPlaying(_currentSound))
		return false;

	if (cue.empty())
		return true;

	uint samples = _vm->_sound->getNumSamplesPlayed(_currentSound);
	for (uint i = 0; i < _cueList.pointCount; i++) {
		if (_cueList.points[i].sampleFrame > samples)
			break;
		if (_cueList.points[i].name == cue)
			return false;
	}

	return true;
}

bool LBAnimation::transparentAt(int x, int y) {
	for (uint32 i = 0; i < _nodes.size(); i++)
		if (!_nodes[i]->transparentAt(x - _bounds.left, y - _bounds.top))
			return false;

	return true;
}

void LBAnimation::setTempo(uint16 tempo) {
	_tempo = tempo;
}

uint16 LBAnimation::getParentId() {
	return _parent->getId();
}

LBScriptEntry::LBScriptEntry() {
	state = 0;
	data = NULL;
	argvParam = NULL;
	argvTarget = NULL;
}

LBScriptEntry::~LBScriptEntry() {
	delete[] argvParam;
	delete[] argvTarget;
	delete[] data;

	for (uint i = 0; i < subentries.size(); i++)
		delete subentries[i];
}

LBItem::LBItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : _vm(vm), _page(page), _rect(rect) {
	if (_vm->getGameType() == GType_LIVINGBOOKSV1 || _vm->getGameType() == GType_LIVINGBOOKSV2)
		_phase = kLBPhaseInit;
	else
		_phase = kLBPhaseLoad;

	_loopMode = 0;
	_delayMin = 0;
	_delayMax = 0;
	_timingMode = kLBAutoNone;
	_periodMin = 0;
	_periodMax = 0;
	_controlMode = kLBControlNone;
	_soundMode = 0;

	_loaded = false;
	_enabled = false;
	_visible = true;
	_playing = false;
	_globalEnabled = true;
	_globalVisible = true;
	_nextTime = 0;
	_startTime = 0;
	_loops = 0;

	_isAmbient = false;
	_doHitTest = true;
}

LBItem::~LBItem() {
	for (uint i = 0; i < _scriptEntries.size(); i++)
		delete _scriptEntries[i];
}

void LBItem::readFrom(Common::SeekableSubReadStreamEndian *stream) {
	_resourceId = stream->readUint16();
	_itemId = stream->readUint16();
	uint16 size = stream->readUint16();
	_desc = _vm->readString(stream);

	debug(2, "Item: size %d, resource %d, id %d", size, _resourceId, _itemId);
	debug(2, "Coords: %d, %d, %d, %d", _rect.left, _rect.top, _rect.right, _rect.bottom);
	debug(2, "String: '%s'", _desc.c_str());

	if (!_itemId)
		error("Item had invalid item id");

	int endPos = stream->pos() + size;
	if (endPos > stream->size())
		error("Item is larger (should end at %d) than stream (size %d)", endPos, stream->size());

	while (true) {
		if (stream->pos() == endPos)
			break;

		uint oldPos = stream->pos();

		uint16 dataType = stream->readUint16();
		uint16 dataSize = stream->readUint16();

		debug(4, "Data type %04x, size %d", dataType, dataSize);
		byte *buf = new byte[dataSize];
		stream->read(buf, dataSize);
		readData(dataType, dataSize, buf);
		delete[] buf;

		if ((uint)stream->pos() != oldPos + 4 + (uint)dataSize)
			error("Failed to read correct number of bytes (off by %d) for data type %04x (size %d)",
				(int)stream->pos() - (int)(oldPos + 4 + (uint)dataSize), dataType, dataSize);

		if (stream->pos() > endPos)
			error("Read off the end (at %d) of data (ends at %d)", stream->pos(), endPos);

		assert(!stream->eos());
	}
}

LBScriptEntry *LBItem::parseScriptEntry(uint16 type, uint16 &size, Common::MemoryReadStreamEndian *stream, bool isSubentry) {
	if (size < 6)
		error("Script entry of type 0x%04x was too small (%d)", type, size);

	uint16 expectedEndSize = 0;

	LBScriptEntry *entry = new LBScriptEntry;
	entry->type = type;
	if (isSubentry) {
		expectedEndSize = size - (stream->readUint16() + 2);
		entry->event = 0xffff;
	} else
		entry->event = stream->readUint16();
	entry->opcode = stream->readUint16();
	entry->param = stream->readUint16();
	debug(4, "Script entry: type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x",
			entry->type, entry->event, entry->opcode, entry->param);
	size -= 6;

	// TODO: read as bytes, if this is correct (but beware endianism)
	byte conditionTag = (entry->event & 0xff00) >> 8;
	entry->event = entry->event & 0xff;

	if (type == kLBMsgListScript && entry->opcode == kLBOpRunSubentries) {
		debug(4, "%d script subentries:", entry->param);
		entry->argc = 0;
		for (uint i = 0; i < entry->param; i++) {
			LBScriptEntry *subentry = parseScriptEntry(type, size, stream, true);
			entry->subentries.push_back(subentry);

			// subentries are aligned
			if (i + 1 < entry->param && size % 2 == 1) {
				stream->skip(1);
				size--;
			}
		}
	} else if (type == kLBMsgListScript) {
		if (size < 2)
			error("Script entry of type 0x%04x was too small (%d)", type, size);

		entry->argc = stream->readUint16();
		size -= 2;

		entry->targetingType = 0;

		uint16 targetingType = entry->argc;
		if (targetingType == kTargetTypeExpression || targetingType == kTargetTypeCode
			|| targetingType == kTargetTypeName) {
			entry->targetingType = targetingType;

			// FIXME
			if (targetingType == kTargetTypeCode)
				error("encountered kTargetTypeCode");

			if (size < 2)
				error("not enough bytes (%d) reading special targeting", size);
			uint16 count = stream->readUint16();
			size -= 2;

			debug(4, "%d targets with targeting type %04x", count, targetingType);

			uint oldAlign = size % 2;
			for (uint i = 0; i < count; i++) {
				Common::String target = _vm->readString(stream);
				debug(4, "target '%s'", target.c_str());
				entry->targets.push_back(target);
				if (target.size() + 1 > size)
					error("failed to read target (ran out of stream)");
				size -= target.size() + 1;
			}
			entry->argc = entry->targets.size();

			if ((uint)(size % 2) != oldAlign) {
				stream->skip(1);
				size--;
			}
		} else if (entry->argc) {
			entry->argvParam = new uint16[entry->argc];
			entry->argvTarget = new uint16[entry->argc];
			debug(4, "With %d targets:", entry->argc);

			if (size < (entry->argc * 4))
				error("Script entry of type 0x%04x was too small (%d)", type, size);

			for (uint i = 0; i < entry->argc; i++) {
				entry->argvParam[i] = stream->readUint16();
				entry->argvTarget[i] = stream->readUint16();
				debug(4, "Target %d, param 0x%04x", entry->argvTarget[i], entry->argvParam[i]);
			}

			size -= (entry->argc * 4);
		}
	}

	if (type == kLBMsgListScript && entry->opcode == kLBOpJumpUnlessExpression) {
		if (size < 6)
			error("not enough bytes (%d) in kLBOpJumpUnlessExpression, event 0x%04x", size, entry->event);
		entry->offset = stream->readUint32();
		entry->target = stream->readUint16();
		debug(4, "kLBOpJumpUnlessExpression: offset %08x, target %d", entry->offset, entry->target);
		size -= 6;
	}
	if (type == kLBMsgListScript && entry->opcode == kLBOpJumpToExpression) {
		if (size < 4)
			error("not enough bytes (%d) in kLBOpJumpToExpression, event 0x%04x", size, entry->event);
		entry->offset = stream->readUint32();
		debug(4, "kLBOpJumpToExpression: offset %08x", entry->offset);
		size -= 4;
	}

	if (type == kLBNotifyScript && entry->opcode == kLBNotifyChangeMode && _vm->getGameType() != GType_LIVINGBOOKSV1) {
		switch (entry->param) {
		case 1:
			if (size < 8)
				error("%d unknown bytes in notify entry kLBNotifyChangeMode", size);
			entry->newUnknown = stream->readUint16();
			entry->newMode = stream->readUint16();
			entry->newPage = stream->readUint16();
			entry->newSubpage = stream->readUint16();
			debug(4, "kLBNotifyChangeMode: unknown %04x, mode %d, page %d.%d",
				entry->newUnknown, entry->newMode, entry->newPage, entry->newSubpage);
			size -= 8;
			break;
		case 3:
			{
			Common::String newCursor = _vm->readString(stream);
			entry->newCursor = newCursor;
			if (size < newCursor.size() + 1)
				error("failed to read newCursor in notify entry");
			size -= newCursor.size() + 1;
			debug(4, "kLBNotifyChangeMode: new cursor '%s'", newCursor.c_str());
			}
			break;
		default:
			// the original engine also does something when param==2 (but not a notify)
			error("unknown v2 kLBNotifyChangeMode type %d", entry->param);
		}
	}
	if (entry->opcode == kLBOpSendExpression) {
		if (size < 4)
			error("not enough bytes (%d) in kLBOpSendExpression, event 0x%04x", size, entry->event);
		entry->offset = stream->readUint32();
		debug(4, "kLBOpSendExpression: offset %08x", entry->offset);
		size -= 4;
	}
	if (entry->opcode == kLBOpRunData) {
		if (size < 4)
			error("didn't get enough bytes (%d) to read data header in script entry", size);
		entry->dataType = stream->readUint16();
		entry->dataLen = stream->readUint16();
		size -= 4;

		if (size < entry->dataLen)
			error("didn't get enough bytes (%d) to read data in script entry", size);

		if (entry->dataType == kLBCommand) {
			Common::String command = _vm->readString(stream);
			uint commandSize = command.size() + 1;
			if (commandSize > entry->dataLen)
				error("failed to read command in script entry: dataLen %d, command '%s' (%d chars)",
					 entry->dataLen, command.c_str(), commandSize);
			entry->dataLen = commandSize;
			entry->data = new byte[commandSize];
			memcpy(entry->data, command.c_str(), commandSize);
			size -= commandSize;
		} else {
			if (conditionTag)
				error("kLBOpRunData had unexpected conditionTag");
			entry->data = new byte[entry->dataLen];
			stream->read(entry->data, entry->dataLen);
			size -= entry->dataLen;
		}
	}
	if (entry->event == kLBEventNotified) {
		if (size < 4)
			error("not enough bytes (%d) in kLBEventNotified, opcode 0x%04x", size, entry->opcode);
		entry->matchFrom = stream->readUint16();
		entry->matchNotify = stream->readUint16();
		debug(4, "kLBEventNotified: matches %04x (from %04x)",
			entry->matchNotify, entry->matchFrom);
		size -= 4;
	}

	if (isSubentry) {
		// TODO: subentries may be aligned, so this check is a bit too relaxed
		if (size != expectedEndSize && size != expectedEndSize + 1)
			error("expected %d bytes left at end of subentry, but had %d",
				expectedEndSize, size);
		return entry;
	}

	if (conditionTag == 1) {
		if (!size)
			error("failed to read condition (empty stream)");
		Common::String condition = _vm->readString(stream);
		if (condition.size() == 0) {
			size--;
			if (!size)
				error("failed to read condition (null byte, then ran out of stream)");
			condition = _vm->readString(stream);
		}
		if (condition.size() + 1 > size)
			error("failed to read condition (ran out of stream)");
		size -= (condition.size() + 1);

		entry->conditions.push_back(condition);
		debug(4, "script entry condition '%s'", condition.c_str());
	} else if (conditionTag == 2) {
		if (size < 4)
			error("expected more than %d bytes for conditionTag 2", size);
		// FIXME
		stream->skip(4);
		size -= 4;
	}

	if (size == 1) {
		// FIXME: this is alignment, but why?
		stream->skip(1);
		size--;
	} else if (size)
		error("failed to read script entry correctly (%d bytes left): type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x",
			size, entry->type, entry->event, entry->opcode, entry->param);

	return entry;
}

void LBItem::readData(uint16 type, uint16 size, byte *data) {
	Common::MemoryReadStreamEndian stream(data, size, _vm->isBigEndian());
	readData(type, size, &stream);
}

void LBItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBMsgListScript:
	case kLBNotifyScript:
		_scriptEntries.push_back(parseScriptEntry(type, size, stream));
		break;

	case kLBSetPlayInfo:
		{
		if (size != 20)
			error("kLBSetPlayInfo had wrong size (%d)", size);

		_loopMode = stream->readUint16();
		_delayMin = stream->readUint16();
		_delayMax = stream->readUint16();
		_timingMode = stream->readUint16();
		if (_timingMode > 7)
			error("encountered timing mode %04x", _timingMode);
		_periodMin = stream->readUint16();
		_periodMax = stream->readUint16();
		_relocPoint.x = stream->readSint16();
		_relocPoint.y = stream->readSint16();
		_controlMode = stream->readUint16();
		_soundMode = stream->readUint16();

		debug(2, "kLBSetPlayInfo: loop mode %d (%d to %d), timing mode %d (%d to %d), reloc (%d, %d), control mode %04x, sound mode %04x",
			_loopMode, _delayMin, _delayMax,
			_timingMode, _periodMin, _periodMax,
			_relocPoint.x, _relocPoint.y,
			_controlMode, _soundMode);
		}
		break;

	case kLBSetPlayPhase:
		if (size != 2)
			error("SetPlayPhase had wrong size (%d)", size);
		_phase = stream->readUint16();
		debug(2, "kLBSetPlayPhase: %d", _phase);
		break;

	case kLBSetKeyNotify:
		{
		// FIXME: variable-size notifies, targets
		if (size != 18)
			error("0x6f had wrong size (%d)", size);
		uint event = stream->readUint16();
		LBKey key;
		stream->read(&key, 4);
		uint opcode = stream->readUint16();
		uint param = stream->readUint16();
		uint u6 = stream->readUint16();
		uint u7 = stream->readUint16();
		uint u8 = stream->readUint16();
		uint u9 = stream->readUint16();
		warning("ignoring kLBSetKeyNotify: item %s, key code %02x (modifier mask %d, char %d, repeat %d), event %04x, opcode %04x, param %04x, unknowns %04x, %04x, %04x, %04x",
			_desc.c_str(), key.code, key.modifiers, key.char_, key.repeats, event, opcode, param, u6, u7, u8, u9);
		}
		break;

	case kLBCommand:
		{
			Common::String command = _vm->readString(stream);
			if (size != command.size() + 1)
				error("failed to read command string");

			runCommand(command);
		}
		break;

	case kLBSetNotVisible:
		assert(size == 0);
		_visible = false;
		break;

	case kLBGlobalDisable:
		assert(size == 0);
		_globalEnabled = false;
		break;

	case kLBGlobalSetNotVisible:
		assert(size == 0);
		_globalVisible = false;
		break;

	case kLBSetAmbient:
		assert(size == 0);
		_isAmbient = true;
		break;

	case kLBSetKeyEvent:
		{
		// FIXME: targets
		if (size != 10)
			error("kLBSetKeyEvent had wrong size (%d)", size);
		uint u3 = stream->readUint16();
		LBKey key;
		stream->read(&key, 4);
		uint target = stream->readUint16();
		uint16 event = stream->readUint16();
		// FIXME: this is scripting stuff: what to run when key is pressed
		warning("ignoring kLBSetKeyEvent: item %s, key code %02x (modifier mask %d, char %d, repeat %d) unknown %04x, target %d, event %04x",
			_desc.c_str(), key.code, key.modifiers, key.char_, key.repeats, u3, target, event);
		}
		break;

	case kLBSetHitTest:
		{
		assert(size == 2);
		uint val = stream->readUint16();
		_doHitTest = (bool)val;
		debug(2, "kLBSetHitTest (on %s): value %04x", _desc.c_str(), val);
		}
		break;

	case kLBSetRolloverData:
		{
		assert(size == 2);
		uint16 flag = stream->readUint16();
		warning("ignoring kLBSetRolloverData: item %s, flag %d", _desc.c_str(), flag);
		}
		break;

	case kLBSetParent:
		{
		assert(size == 2);
		uint16 parent = stream->readUint16();
		warning("ignoring kLBSetParent: item %s, parent id %d", _desc.c_str(), parent);
		}
		break;

	case kLBUnknown194:
		{
		assert(size == 4);
		uint offset = stream->readUint32();
		_page->_code->runCode(this, offset);
		}
		break;

	default:
		error("Unknown message %04x (size 0x%04x)", type, size);
		//for (uint i = 0; i < size; i++)
		//	debugN("%02x ", stream->readByte());
		//debugN("\n");
		break;
	}
}

void LBItem::destroySelf() {
	if (!this->_itemId)
		error("destroySelf() on an item which was already dead");

	_vm->queueDelayedEvent(DelayedEvent(this, kLBDelayedEventDestroy));

	_itemId = 0;
}

void LBItem::setEnabled(bool enabled) {
	if (enabled && !_loaded && !_playing) {
		if (_timingMode == kLBAutoUserIdle) {
			setNextTime(_periodMin, _periodMax);
			debug(2, "Enable time startup");
		}
	}

	_enabled = enabled;
}

void LBItem::setGlobalEnabled(bool enabled) {
	bool wasEnabled = _loaded && _enabled && _globalEnabled;
	_globalEnabled = enabled;
	if (wasEnabled != (_loaded && _enabled && _globalEnabled))
		setEnabled(enabled);
}

bool LBItem::contains(Common::Point point) {
	if (!_loaded)
		return false;

	if (_playing && _loopMode == 0xFFFF)
		stop();

	if (!_playing && _timingMode == kLBAutoUserIdle)
		setNextTime(_periodMin, _periodMax);

	return _visible && _globalVisible && _rect.contains(point);
}

void LBItem::update() {
	if (_phase != kLBPhaseNone && (!_loaded || !_enabled || !_globalEnabled))
		return;

	if (_nextTime == 0 || _nextTime > (uint32)(_vm->_system->getMillis() / 16))
		return;

	if (togglePlaying(_playing, true)) {
		_nextTime = 0;
	} else if (_loops == 0 && _timingMode == kLBAutoUserIdle) {
		debug(9, "Looping in update()");
		setNextTime(_periodMin, _periodMax);
	}
}

void LBItem::handleMouseDown(Common::Point pos) {
	if (!_loaded || !_enabled || !_globalEnabled)
		return;

	_vm->setFocus(this);
	runScript(kLBEventMouseDown);
	runScript(kLBEventMouseTrackIn);
}

void LBItem::handleMouseMove(Common::Point pos) {
	// TODO: handle drag
}

void LBItem::handleMouseUp(Common::Point pos) {
	_vm->setFocus(NULL);
	runScript(kLBEventMouseUp);
	runScript(kLBEventMouseUpIn);
}

bool LBItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		_vm->queueDelayedEvent(DelayedEvent(this, kLBDelayedEventDone));
		return true;
	}
	if (((_loaded && _enabled && _globalEnabled) || _phase == kLBPhaseNone) && !_playing) {
		_playing = togglePlaying(true, restart);
		if (_playing) {
			_nextTime = 0;
			_startTime = _vm->_system->getMillis() / 16;

			if (_loopMode == 0xFFFF || _loopMode == 0xFFFE)
				_loops = 0xFFFF;
			else
				_loops = _loopMode;

			if (_controlMode >= kLBControlHideMouse) {
				debug(2, "Hiding cursor");
				_vm->_cursor->hideCursor();
				_vm->lockSound(this, true);

				if (_controlMode >= kLBControlPauseItems) {
					debug(2, "Disabling all");
					_vm->setEnableForAll(false, this);
				}
			}

			runScript(kLBEventStarted);
			notify(0, _itemId);
		}
	}
	return _playing;
}

void LBItem::done(bool onlyNotify) {
	if (onlyNotify) {
		if (_relocPoint.x || _relocPoint.y) {
			_rect.translate(_relocPoint.x, _relocPoint.y);
			// TODO: does drag box need adjusting?
		}

		if (_loops && --_loops) {
			debug(9, "Real looping (now 0x%04x left)", _loops);
			setNextTime(_delayMin, _delayMax, _startTime);
		} else
			done(false);

		return;
	}

	_playing = false;
	_loops = 0;
	_startTime = 0;

	if (_controlMode >= kLBControlHideMouse) {
		debug(2, "Showing cursor");
		_vm->_cursor->showCursor();
		_vm->lockSound(this, false);

		if (_controlMode >= kLBControlPauseItems) {
			debug(2, "Enabling all");
			_vm->setEnableForAll(true, this);
		}
	}

	if (_timingMode == kLBAutoUserIdle) {
		debug(9, "Looping in done() - %d to %d", _periodMin, _periodMax);
		setNextTime(_periodMin, _periodMax);
	}

	runScript(kLBEventDone);
	notify(0xFFFF, _itemId);
}

void LBItem::init() {
	runScript(kLBEventInit);
}

void LBItem::setVisible(bool visible) {
	if (visible == _visible)
		return;

	_visible = visible;
	_vm->_needsRedraw = true;
}

void LBItem::setGlobalVisible(bool visible) {
	bool wasEnabled = _visible && _globalVisible;
	_globalVisible = visible;
	if (wasEnabled != (_visible && _globalVisible))
		_vm->_needsRedraw = true;
}

void LBItem::startPhase(uint phase) {
	if (_phase == phase) {
		if (_phase != kLBPhaseNone) {
			setEnabled(true);
		}

		load();
	}

	switch (phase) {
	case kLBPhaseLoad:
		runScript(kLBEventListLoad);
		break;
	case kLBPhaseCreate:
		runScript(kLBEventPhaseCreate);
		if (_timingMode == kLBAutoCreate) {
			debug(2, "Phase create: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	case kLBPhaseInit:
		runScript(kLBEventPhaseInit);
		if (_timingMode == kLBAutoInit) {
			debug(2, "Phase init: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	case kLBPhaseIntro:
		runScript(kLBEventPhaseIntro);
		if (_timingMode == kLBAutoIntro || _timingMode == kLBAutoUserIdle) {
			debug(2, "Phase intro: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	case kLBPhaseMain:
		runScript(kLBEventPhaseMain);
		if (_timingMode == kLBAutoUserIdle || _timingMode == kLBAutoMain) {
			debug(2, "Phase main: time startup");
			setNextTime(_periodMin, _periodMax);
		}
		break;
	}
}

void LBItem::stop() {
	if (!_playing)
		return;

	_loops = 0;
	seek(0xFFFF);
	done(true);
}

void LBItem::notify(uint16 data, uint16 from) {
	if (_timingMode == kLBAutoSync) {
		// TODO: is this correct?
		if (_periodMin == data && _periodMax == from) {
			debug(2, "Handling notify 0x%04x (from %d)", data, from);
			setNextTime(0, 0);
		}
	}

	runScript(kLBEventNotified, data, from);
}

void LBItem::load() {
	if (_loaded)
		return;

	_loaded = true;

	// FIXME: events etc
	if (_timingMode == kLBAutoLoad) {
		debug(2, "Load: time startup");
		setNextTime(_periodMin, _periodMax);
	}
}

void LBItem::unload() {
	if (!_loaded)
		return;

	_loaded = false;

	// FIXME: stuff
}

void LBItem::moveBy(const Common::Point &pos) {
	_rect.translate(pos.x, pos.y);
}

void LBItem::moveTo(const Common::Point &pos) {
	_rect.moveTo(pos);
}

LBItem *LBItem::clone(uint16 newId, const Common::String &newName) {
	LBItem *item = createClone();

	item->_itemId = newId;
	item->_desc = newName;

	item->_resourceId = _resourceId;
	// FIXME: the rest

	_page->addClonedItem(item);
	// FIXME: zorder?
	return item;
}

LBItem *LBItem::createClone() {
	return new LBItem(_vm, _page, _rect);
}

void LBItem::runScript(uint event, uint16 data, uint16 from) {
	for (uint i = 0; i < _scriptEntries.size(); i++) {
		LBScriptEntry *entry = _scriptEntries[i];

		if (entry->event != event)
			continue;

		if (event == kLBEventNotified) {
			if ((entry->matchFrom && entry->matchFrom != from) || entry->matchNotify != data)
				continue;
		}

		bool conditionsMatch = true;
		for (uint n = 0; n < entry->conditions.size(); n++) {
			if (!checkCondition(entry->conditions[n])) {
				conditionsMatch = false;
				break;
			}
		}
		if (!conditionsMatch)
			continue;

		if (entry->type == kLBNotifyScript) {
			debug(2, "Notify: event 0x%04x, opcode 0x%04x, param 0x%04x",
				entry->event, entry->opcode, entry->param);

			if (entry->opcode == kLBNotifyGUIAction)
				_vm->addNotifyEvent(NotifyEvent(entry->opcode, _itemId));
			else if (entry->opcode == kLBNotifyChangeMode && _vm->getGameType() != GType_LIVINGBOOKSV1) {
				NotifyEvent notifyEvent(entry->opcode, entry->param);
				notifyEvent.newUnknown = entry->newUnknown;
				notifyEvent.newMode = entry->newMode;
				notifyEvent.newPage = entry->newPage;
				notifyEvent.newSubpage = entry->newSubpage;
				notifyEvent.newCursor = entry->newCursor;
				_vm->addNotifyEvent(notifyEvent);
			} else
				_vm->addNotifyEvent(NotifyEvent(entry->opcode, entry->param));
		} else
			runScriptEntry(entry);
	}
}

int LBItem::runScriptEntry(LBScriptEntry *entry) {
	if (entry->state == 0xffff)
		return 0;

	uint start = 0;
	uint count = entry->argc;
	// zero targets = apply to self
	if (!count)
		count = 1;

	if (entry->opcode != kLBOpRunSubentries) switch (entry->param) {
	case 0xfffe:
		// Run once (disable self after run).
		entry->state = 0xffff;
		break;
	case 0xffff:
		break;
	case 0:
	case 1:
	case 2:
		start = entry->state;
		entry->state++;
		if (entry->state >= count) {
			switch (entry->param) {
			case 0:
				// Disable..
				entry->state = 0xffff;
				return 0;
			case 1:
				// Stay at the end.
				entry->state = count - 1;
				break;
			case 2:
				// Loop.
				entry->state = 0;
				break;
			}
		}
		count = 1;
		break;
	case 3:
		// Pick random target.
		start = _vm->_rnd->getRandomNumberRng(0, count);
		count = 1;
		break;
	default:
		warning("Weird param for script entry (type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x)",
			entry->type, entry->event, entry->opcode, entry->param);
	}

	for (uint n = start; n < count; n++) {
		LBItem *target;

		debug(2, "Script run: type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x",
			entry->type, entry->event, entry->opcode, entry->param);

		if (entry->argc) {
			switch (entry->targetingType) {
			case kTargetTypeExpression:
				{
				// FIXME: this should be EVALUATED
				LBValue &tgt = _vm->_variables[entry->targets[n]];
				switch (tgt.type) {
				case kLBValueItemPtr:
					target = tgt.item;
					break;
				case kLBValueString:
					// FIXME: handle 'self', at least
					// TODO: correct otherwise? or only self?
					target = _vm->getItemByName(tgt.string);
					break;
				case kLBValueInteger:
					target = _vm->getItemById(tgt.integer);
					break;
				default:
					// FIXME: handle list
					warning("Target '%s' (by expression) resulted in unknown type, skipping", entry->targets[n].c_str());
					continue;
				}
				}
				if (!target) {
					debug(2, "Target '%s' (by expression) doesn't exist, skipping", entry->targets[n].c_str());
					continue;
				}
				debug(2, "Target: '%s' (expression '%s')", target->_desc.c_str(), entry->targets[n].c_str());
				break;
			case kTargetTypeCode:
				// FIXME
				error("encountered kTargetTypeCode");
				break;
			case kTargetTypeName:
				// FIXME: handle 'self'
				target = _vm->getItemByName(entry->targets[n]);
				if (!target) {
					debug(2, "Target '%s' (by name) doesn't exist, skipping", entry->targets[n].c_str());
					continue;
				}
				debug(2, "Target: '%s' (by name)", target->_desc.c_str());
				break;
			default:
				uint16 targetId = entry->argvTarget[n];
				// TODO: is this type, perhaps?
				uint16 param = entry->argvParam[n];
				target = _vm->getItemById(targetId);
				if (!target) {
					debug(2, "Target %04x (%04x) doesn't exist, skipping", targetId, param);
					continue;
				}
				debug(2, "Target: %04x (%04x) '%s'", targetId, param, target->_desc.c_str());
			}
		} else {
			target = this;
			debug(2, "Self-target on '%s'", _desc.c_str());
		}

		// an opcode in the form 0x1xx means to run the script for event 0xx
		if ((entry->opcode & 0xff00) == 0x0100) {
			// FIXME: pass on param
			target->runScript(entry->opcode & 0xff);
			break;
		}

		switch (entry->opcode) {
		case kLBOpNone:
			warning("ignoring kLBOpNone (event 0x%04x, param 0x%04x, target '%s')",
					entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpXShow:
			// TODO: should be setVisible(true) - not a delayed event -
			// when we're doing the param 1/2/3 stuff above?
			// and in modern LB this is perhaps just a direct target->setVisible(true)..
			if (_vm->getGameType() != GType_LIVINGBOOKSV1)
				warning("kLBOpXShow on '%s' is probably broken", target->_desc.c_str());
			_vm->queueDelayedEvent(DelayedEvent(this, kLBDelayedEventSetNotVisible));
			break;

		case kLBOpTogglePlay:
			target->togglePlaying(false, true);
			break;

		case kLBOpSetNotVisible:
			target->setVisible(false);
			break;

		case kLBOpSetVisible:
			target->setVisible(true);
			break;

		case kLBOpDestroy:
			target->destroySelf();
			break;

		case kLBOpRewind:
			target->seek(1);
			break;

		case kLBOpStop:
			target->stop();
			break;

		case kLBOpDisable:
			target->setEnabled(false);
			break;

		case kLBOpEnable:
			target->setEnabled(true);
			break;

		case kLBOpGlobalSetNotVisible:
			target->setGlobalVisible(false);
			break;

		case kLBOpGlobalSetVisible:
			target->setGlobalVisible(true);
			break;

		case kLBOpGlobalDisable:
			target->setGlobalEnabled(false);
			break;

		case kLBOpGlobalEnable:
			target->setGlobalEnabled(true);
			break;

		case kLBOpSeekToEnd:
			target->seek(0xFFFF);
			break;

		case kLBOpMute:
		case kLBOpUnmute:
			// FIXME
			warning("ignoring kLBOpMute/Unmute (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpLoad:
			target->load();
			break;

		case kLBOpPreload:
			// FIXME
			warning("ignoring kLBOpPreload (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpUnload:
			target->unload();
			break;

		case kLBOpSeekToPrev:
		case kLBOpSeekToNext:
			// FIXME
			warning("ignoring kLBOpSeekToPrev/Next (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpDragBegin:
		case kLBOpDragEnd:
			// FIXME
			warning("ignoring kLBOpDragBegin/End (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpScriptDisable:
		case kLBOpScriptEnable:
			// FIXME
			warning("ignoring kLBOpScriptDisable/Enable (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpUnknown1C:
			// FIXME
			warning("ignoring kLBOpUnknown1C (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpSendExpression:
			_page->_code->runCode(this, entry->offset);
			break;

		case kLBOpRunSubentries:
			for (uint i = 0; i < entry->subentries.size(); i++) {
				LBScriptEntry *subentry = entry->subentries[i];

				int e = runScriptEntry(subentry);

				switch (subentry->opcode) {
				case kLBOpJumpUnlessExpression:
					debug(2, "JumpUnless got %d (to %d, on %d, of %d)", e, subentry->target, i, entry->subentries.size());
					if (!e)
						i = subentry->target - 1;
					break;
				case kLBOpBreakExpression:
					debug(2, "BreakExpression");
					i = entry->subentries.size();
					break;
				case kLBOpJumpToExpression:
					debug(2, "JumpToExpression got %d (on %d, of %d)", e, i, entry->subentries.size());
					i = e - 1;
					break;
				}
			}
			break;

		case kLBOpRunData:
			readData(entry->dataType, entry->dataLen, entry->data);
			break;

		case kLBOpJumpUnlessExpression:
		case kLBOpBreakExpression:
		case kLBOpJumpToExpression:
			{
			LBValue r = _page->_code->runCode(this, entry->offset);
			// FIXME
			return r.integer;
			}

		default:
			error("Unknown script opcode (type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x, target '%s')",
				entry->type, entry->event, entry->opcode, entry->param, target->_desc.c_str());
		}
	}

	return 0;
}

void LBItem::setNextTime(uint16 min, uint16 max) {
	setNextTime(min, max, _vm->_system->getMillis() / 16);
}

void LBItem::setNextTime(uint16 min, uint16 max, uint32 start) {
	_nextTime = start + _vm->_rnd->getRandomNumberRng((uint)min, (uint)max);
	debug(9, "nextTime is now %d frames away", _nextTime - (uint)(_vm->_system->getMillis() / 16));
}

void LBItem::runCommand(const Common::String &command) {
	LBCode tempCode(_vm, 0);

	debug(2, "running command '%s'", command.c_str());

	uint offset = tempCode.parseCode(command);
	tempCode.runCode(this, offset);
}

bool LBItem::checkCondition(const Common::String &condition) {
	LBCode tempCode(_vm, 0);

	debug(3, "checking condition '%s'", condition.c_str());

	uint offset = tempCode.parseCode(condition);
	LBValue result = tempCode.runCode(this, offset);

	return result.toInt();
}

LBSoundItem::LBSoundItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBSoundItem");
	_running = false;
}

LBSoundItem::~LBSoundItem() {
	if (_running)
		_vm->_sound->stopSound(_resourceId);
}

void LBSoundItem::update() {
	if (_running && !_vm->_sound->isPlaying(_resourceId)) {
		_running = false;
		done(true);
	}

	LBItem::update();
}

bool LBSoundItem::togglePlaying(bool playing, bool restart) {
	if (!playing)
		return LBItem::togglePlaying(playing, restart);

	if (_running) {
		_running = false;
		_vm->_sound->stopSound(_resourceId);
	}

	if (!_loaded || !_enabled || !_globalEnabled)
		return false;

	_running = true;
	debug(4, "sound %d play for item %d (%s)", _resourceId, _itemId, _desc.c_str());
	_vm->playSound(this, _resourceId);
	return true;
}

void LBSoundItem::stop() {
	if (_running) {
		_running = false;
		_vm->_sound->stopSound(_resourceId);
	}

	LBItem::stop();
}

LBItem *LBSoundItem::createClone() {
	return new LBSoundItem(_vm, _page, _rect);
}

LBGroupItem::LBGroupItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBGroupItem");
	_starting = false;
}

void LBGroupItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBGroupData:
		{
		_groupEntries.clear();
		uint16 count = stream->readUint16();
		debug(3, "Group data: %d entries", count);

		if (size != 2 + count * 4)
			error("kLBGroupData was wrong size (%d, for %d entries)", size, count);

		for (uint i = 0; i < count; i++) {
			GroupEntry entry;
			// TODO: is type important for any game? at the moment, we ignore it
			entry.entryType = stream->readUint16();
			entry.entryId = stream->readUint16();
			_groupEntries.push_back(entry);
			debug(3, "group entry: id %d, type %d", entry.entryId, entry.entryType);
		}
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

void LBGroupItem::destroySelf() {
	LBItem::destroySelf();

	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->destroySelf();
	}
}

void LBGroupItem::setEnabled(bool enabled) {
	if (_starting) {
		_starting = false;
		LBItem::setEnabled(enabled);
	} else {
		for (uint i = 0; i < _groupEntries.size(); i++) {
			LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
			if (item)
				item->setEnabled(enabled);
		}
	}
}

void LBGroupItem::setGlobalEnabled(bool enabled) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->setGlobalEnabled(enabled);
	}
}

bool LBGroupItem::contains(Common::Point point) {
	return false;
}

bool LBGroupItem::togglePlaying(bool playing, bool restart) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->togglePlaying(playing, restart);
	}

	return false;
}

void LBGroupItem::seek(uint16 pos) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->seek(pos);
	}
}

void LBGroupItem::setVisible(bool visible) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->setVisible(visible);
	}
}

void LBGroupItem::setGlobalVisible(bool visible) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->setGlobalVisible(visible);
	}
}

void LBGroupItem::startPhase(uint phase) {
	_starting = true;
	LBItem::startPhase(phase);
	_starting = false;
}

void LBGroupItem::stop() {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->stop();
	}
}

void LBGroupItem::load() {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->load();
	}
}

void LBGroupItem::unload() {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->unload();
	}
}

void LBGroupItem::moveBy(const Common::Point &pos) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->moveBy(pos);
	}
}

void LBGroupItem::moveTo(const Common::Point &pos) {
	for (uint i = 0; i < _groupEntries.size(); i++) {
		LBItem *item = _vm->getItemById(_groupEntries[i].entryId);
		if (item)
			item->moveTo(pos);
	}
}

LBItem *LBGroupItem::createClone() {
	// TODO: needed?
	error("LBGroupItem::createClone unimplemented");
	return new LBGroupItem(_vm, _page, _rect);
}

LBPaletteItem::LBPaletteItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBPaletteItem");

	_fadeInStart = 0;
	_palette = NULL;
}

LBPaletteItem::~LBPaletteItem() {
	delete[] _palette;
}

void LBPaletteItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBPaletteXData:
		{
		assert(size >= 8);
		_fadeInPeriod = stream->readUint16();
		_fadeInStep = stream->readUint16();
		_drawStart = stream->readUint16();
		_drawCount = stream->readUint16();
		if (_drawStart + _drawCount > 256)
			error("encountered palette trying to set more than 256 colors");
		assert(size == 8 + _drawCount * 4);

		// TODO: _drawCount is really more like _drawEnd, so once we're sure that
		// there's really no use for the palette entries before _drawCount, we
		// might want to just discard them here, at load time.
		_palette = new byte[_drawCount * 3];
		for (uint i = 0; i < _drawCount; i++) {
			_palette[i*3 + 0] = stream->readByte();
			_palette[i*3 + 1] = stream->readByte();
			_palette[i*3 + 2] = stream->readByte();
			stream->readByte();
		}
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

bool LBPaletteItem::togglePlaying(bool playing, bool restart) {
	// TODO: this likely isn't the right place

	if (playing) {
		_fadeInStart = _vm->_system->getMillis();
		_fadeInCurrent = 0;

		return true;
	}

	return LBItem::togglePlaying(playing, restart);
}

void LBPaletteItem::update() {
	if (_fadeInStart) {
		if (!_palette)
			error("LBPaletteItem had no palette on startup");

		uint32 elapsedTime = _vm->_system->getMillis() - _fadeInStart;
		uint32 divTime = elapsedTime / _fadeInStep;

		if (divTime > _fadeInPeriod)
			divTime = _fadeInPeriod;

		if (_fadeInCurrent != divTime) {
			_fadeInCurrent = divTime;

			// TODO: actual fading-in
			if (_visible && _globalVisible) {
				_vm->_system->getPaletteManager()->setPalette(_palette + _drawStart * 3, _drawStart, _drawCount - _drawStart);
				_vm->_needsRedraw = true;
			}
		}

		if (elapsedTime >= (uint32)_fadeInPeriod * (uint32)_fadeInStep) {
			// TODO: correct?
			_fadeInStart = 0;
		}
	}

	LBItem::update();
}

LBItem *LBPaletteItem::createClone() {
	error("can't clone LBPaletteItem");
}

LBLiveTextItem::LBLiveTextItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	_currentPhrase = 0xFFFF;
	_currentWord = 0xFFFF;
	debug(3, "new LBLiveTextItem");
}

void LBLiveTextItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBLiveTextData:
		{
		stream->read(_backgroundColor, 4); // unused?
		stream->read(_foregroundColor, 4);
		stream->read(_highlightColor, 4);
		_paletteIndex = stream->readUint16();
		uint16 phraseCount = stream->readUint16();
		uint16 wordCount = stream->readUint16();

		debug(3, "LiveText has %d words in %d phrases, palette index 0x%04x", wordCount, phraseCount, _paletteIndex);
		debug(3, "LiveText colors: background %02x%02x%02x%02x, foreground %02x%02x%02x%02x, highlight %02x%02x%02x%02x",
			_backgroundColor[0], _backgroundColor[1], _backgroundColor[2], _backgroundColor[3],
			_foregroundColor[0], _foregroundColor[1], _foregroundColor[2], _foregroundColor[3],
			_highlightColor[0], _highlightColor[1], _highlightColor[2], _highlightColor[3]);

		if (size != 18 + 14 * wordCount + 18 * phraseCount)
			error("Bad Live Text data size (got %d, wanted %d words and %d phrases)", size, wordCount, phraseCount);

		_words.clear();
		for (uint i = 0; i < wordCount; i++) {
			LiveTextWord word;
			word.bounds = _vm->readRect(stream);
			word.soundId = stream->readUint16();
			word.itemType = stream->readUint16();
			word.itemId = stream->readUint16();
			debug(4, "Word: (%d, %d) to (%d, %d), sound %d, item %d (type %d)",
				word.bounds.left, word.bounds.top, word.bounds.right, word.bounds.bottom, word.soundId, word.itemId, word.itemType);
			_words.push_back(word);
		}

		_phrases.clear();
		for (uint i = 0; i < phraseCount; i++) {
			LiveTextPhrase phrase;
			phrase.wordStart = stream->readUint16();
			phrase.wordCount = stream->readUint16();
			phrase.highlightStart = stream->readUint16();
			phrase.startId = stream->readUint16();
			phrase.highlightEnd = stream->readUint16();
			phrase.endId = stream->readUint16();

			// The original stored the values in uint32's so we need to swap here
			if (_vm->isBigEndian()) {
				SWAP(phrase.highlightStart, phrase.startId);
				SWAP(phrase.highlightEnd, phrase.endId);
			}

			uint32 unknown1 = stream->readUint16();
			uint16 unknown2 = stream->readUint32();

			if (unknown1 != 0 || unknown2 != 0)
				error("Unexpected unknowns %08x/%04x in LiveText word", unknown1, unknown2);

			debug(4, "Phrase: start %d, count %d, start at %d (from %d), end at %d (from %d)",
				phrase.wordStart, phrase.wordCount, phrase.highlightStart, phrase.startId, phrase.highlightEnd, phrase.endId);

			_phrases.push_back(phrase);
		}
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

bool LBLiveTextItem::contains(Common::Point point) {
	if (!LBItem::contains(point))
		return false;

	point.x -= _rect.left;
	point.y -= _rect.top;

	for (uint i = 0; i < _words.size(); i++) {
		if (_words[i].bounds.contains(point))
			return true;
	}

	return false;
}

void LBLiveTextItem::paletteUpdate(uint16 word, bool on) {
	_vm->_needsRedraw = true;

	// Sometimes the last phrase goes out-of-bounds, the original engine
	// only checks the words which are valid in the palette updating code.
	if (word >= _words.size())
		return;

	if (_resourceId) {
		// with a resource, we draw a bitmap in draw() rather than changing the palette
		return;
	}

	if (on) {
		_vm->_system->getPaletteManager()->setPalette(_highlightColor, _paletteIndex + word, 1);
	} else {
		_vm->_system->getPaletteManager()->setPalette(_foregroundColor, _paletteIndex + word, 1);
	}
}

void LBLiveTextItem::update() {
	if (_currentWord != 0xFFFF) {
		uint16 soundId = _words[_currentWord].soundId;
		if (soundId && !_vm->_sound->isPlaying(soundId)) {
			paletteUpdate(_currentWord, false);

			// TODO: check this in RE
			LBItem *item = _vm->getItemById(_words[_currentWord].itemId);
			if (item)
				item->togglePlaying(false, true);

			_currentWord = 0xFFFF;
		}
	}

	LBItem::update();
}

void LBLiveTextItem::draw() {
	// this is only necessary when we are drawing using a bitmap
	if (!_resourceId)
		return;

	if (_currentWord != 0xFFFF) {
		uint yPos = 0;
		if (_currentWord > 0) {
			for (uint i = 0; i < _currentWord; i++) {
				yPos += (_words[i].bounds.bottom - _words[i].bounds.top);
			}
		}
		drawWord(_currentWord, yPos);
		return;
	}

	if (_currentPhrase == 0xFFFF)
		return;

	uint wordStart = _phrases[_currentPhrase].wordStart;
	uint wordCount = _phrases[_currentPhrase].wordCount;
	if (wordStart + wordCount > _words.size())
		error("phrase %d was invalid (%d words, from %d, out of only %d total)",
			_currentPhrase, wordCount, wordStart, _words.size());

	uint yPos = 0;
	for (uint i = 0; i < wordStart + wordCount; i++) {
		if (i >= wordStart)
			drawWord(i, yPos);
		yPos += (_words[i].bounds.bottom - _words[i].bounds.top);
	}
}

void LBLiveTextItem::drawWord(uint word, uint yPos) {
	Common::Rect srcRect(0, yPos, _words[word].bounds.right - _words[word].bounds.left,
		yPos + _words[word].bounds.bottom - _words[word].bounds.top);
	Common::Rect dstRect = _words[word].bounds;
	dstRect.translate(_rect.left, _rect.top);
	_vm->_gfx->copyAnimImageSectionToScreen(_resourceId, srcRect, dstRect);
}

void LBLiveTextItem::handleMouseDown(Common::Point pos) {
	if (!_loaded || !_enabled || !_globalEnabled || _playing)
		return LBItem::handleMouseDown(pos);

	pos.x -= _rect.left;
	pos.y -= _rect.top;

	for (uint i = 0; i < _words.size(); i++) {
		if (_words[i].bounds.contains(pos)) {
			if (_currentWord != 0xFFFF) {
				paletteUpdate(_currentWord, false);
				_currentWord = 0xFFFF;
			}
			uint16 soundId = _words[i].soundId;
			if (!soundId) {
				// TODO: can we be smarter here, using timing?
				warning("ignoring click due to no soundId");
				return;
			}
			_currentWord = i;
			_vm->playSound(this, soundId);
			paletteUpdate(_currentWord, true);
			return;
		}
	}

	return LBItem::handleMouseDown(pos);
}

bool LBLiveTextItem::togglePlaying(bool playing, bool restart) {
	if (!playing)
		return LBItem::togglePlaying(playing, restart);
	if (!_loaded || !_enabled || !_globalEnabled)
		return _playing;

	// TODO: handle this properly
	_vm->_sound->stopSound();

	_currentWord = 0xFFFF;
	_currentPhrase = 0xFFFF;

	return true;
}

void LBLiveTextItem::stop() {
	// TODO: stop sound, refresh palette

	LBItem::stop();
}

void LBLiveTextItem::notify(uint16 data, uint16 from) {
	if (!_loaded || !_enabled || !_globalEnabled || !_playing)
		return LBItem::notify(data, from);

	if (_currentWord != 0xFFFF) {
		// TODO: handle this properly
		_vm->_sound->stopSound();
		paletteUpdate(_currentWord, false);
		_currentWord = 0xFFFF;
	}

	for (uint i = 0; i < _phrases.size(); i++) {
		if (_phrases[i].highlightStart == data && _phrases[i].startId == from) {
			debug(2, "Enabling phrase %d", i);
			for (uint j = 0; j < _phrases[i].wordCount; j++) {
				paletteUpdate(_phrases[i].wordStart + j, true);
			}
			_currentPhrase = i;
			// TODO: not sure this is the correct logic
			if (i == _phrases.size() - 1) {
				_currentPhrase = 0xFFFF;
				done(true);
			}
		} else if (_phrases[i].highlightEnd == data && _phrases[i].endId == from) {
			debug(2, "Disabling phrase %d", i);
			for (uint j = 0; j < _phrases[i].wordCount; j++) {
				paletteUpdate(_phrases[i].wordStart + j, false);
			}
			_currentPhrase = 0xFFFF;
		}
	}

	LBItem::notify(data, from);
}

LBItem *LBLiveTextItem::createClone() {
	error("can't clone LBLiveTextItem");
}

LBPictureItem::LBPictureItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBPictureItem");
}

void LBPictureItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBSetDrawMode:
		{
		assert(size == 2);
		// TODO: this probably sets whether points are always contained (0x10)
		// or whether the bitmap contents are checked (00, or anything else?)
		uint16 val = stream->readUint16();
		debug(2, "LBPictureItem: kLBSetDrawMode: %04x", val);
		}
		break;

	default:
		LBItem::readData(type, size, stream);
	}
}

bool LBPictureItem::contains(Common::Point point) {
	if (!LBItem::contains(point))
		return false;

	if (!_doHitTest)
		return true;

	// TODO: only check pixels if necessary
	return !_vm->_gfx->imageIsTransparentAt(_resourceId, false, point.x - _rect.left, point.y - _rect.top);
}

void LBPictureItem::init() {
	_vm->_gfx->preloadImage(_resourceId);

	LBItem::init();
}

void LBPictureItem::draw() {
	if (!_loaded || !_visible || !_globalVisible)
		return;

	_vm->_gfx->copyAnimImageToScreen(_resourceId, _rect.left, _rect.top);
}

LBItem *LBPictureItem::createClone() {
	return new LBPictureItem(_vm, _page, _rect);
}

LBAnimationItem::LBAnimationItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	_anim = NULL;
	_running = false;
	debug(3, "new LBAnimationItem");
}

LBAnimationItem::~LBAnimationItem() {
	delete _anim;
}

void LBAnimationItem::setEnabled(bool enabled) {
	if (_running) {
		if (enabled && _globalEnabled && !_loaded)
			_anim->start();
		else if (_loaded && !enabled && _enabled && _globalEnabled)
			_anim->stop();
	}

	return LBItem::setEnabled(enabled);
}

bool LBAnimationItem::contains(Common::Point point) {
	if (!LBItem::contains(point))
		return false;

	if (!_doHitTest)
		return true;

	return !_anim->transparentAt(point.x, point.y);
}

void LBAnimationItem::update() {
	if (_loaded && _enabled && _globalEnabled && _running) {
		bool wasDone = _anim->update();
		if (wasDone) {
			_running = false;
			done(true);
		}
	}

	LBItem::update();
}

bool LBAnimationItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		if (_loaded && _enabled && _globalEnabled) {
			if (restart)
				seek(1);
			_running = true;
			_anim->start();
		}

		return _running;
	}

	return LBItem::togglePlaying(playing, restart);
}

void LBAnimationItem::done(bool onlyNotify) {
	if (!onlyNotify) {
		_anim->stop();
	}

	LBItem::done(onlyNotify);
}

void LBAnimationItem::init() {
	_anim = new LBAnimation(_vm, this, _resourceId);

	LBItem::init();
}

void LBAnimationItem::stop() {
	if (_running) {
		_anim->stop();
		seek(0xFFFF);
	}

	_running = false;

	LBItem::stop();
}

void LBAnimationItem::seek(uint16 pos) {
	_anim->seek(pos);
}

void LBAnimationItem::seekToTime(uint32 time) {
	_anim->seekToTime(time);
}

void LBAnimationItem::startPhase(uint phase) {
	if (phase == _phase)
		seek(1);

	LBItem::startPhase(phase);
}

void LBAnimationItem::draw() {
	if (!_visible || !_globalVisible)
		return;

	_anim->draw();
}

LBItem *LBAnimationItem::createClone() {
	LBAnimationItem *item = new LBAnimationItem(_vm, _page, _rect);
	item->_anim = new LBAnimation(_vm, item, _resourceId);
	return item;
}

LBMovieItem::LBMovieItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBMovieItem");
}

LBMovieItem::~LBMovieItem() {
}

void LBMovieItem::update() {
	if (_playing) {
		VideoHandle videoHandle = _vm->_video->findVideoHandle(_resourceId);
		if (_vm->_video->endOfVideo(videoHandle))
			done(true);
	}

	LBItem::update();
}

bool LBMovieItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		if ((_loaded && _enabled && _globalEnabled) || _phase == kLBPhaseNone) {
			_vm->_video->playMovie(_resourceId, _rect.left, _rect.top);

			return true;
		}
	}

	return LBItem::togglePlaying(playing, restart);
}

LBItem *LBMovieItem::createClone() {
	return new LBMovieItem(_vm, _page, _rect);
}

LBMiniGameItem::LBMiniGameItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBMiniGameItem");
}

LBMiniGameItem::~LBMiniGameItem() {
}

bool LBMiniGameItem::togglePlaying(bool playing, bool restart) {
	// HACK: Since we don't support any of these hardcoded mini games yet,
	// just skip to the most logical page. For optional minigames, this
	// will return the player to the previous page. For mandatory minigames,
	// this will send the player to the next page.
	// TODO: Document mini games from Arthur's Reading Race

	uint16 destPage;

	// Figure out what minigame we have and bring us back to a page where
	// the player can continue
	if (_desc == "Kitch")     // Green Eggs and Ham: Kitchen minigame
		destPage = 4;
	else if (_desc == "Eggs") // Green Eggs and Ham: Eggs minigame
		destPage = 5;
	else if (_desc == "Fall") // Green Eggs and Ham: Fall minigame
		destPage = 13;
	else
		error("Unknown minigame '%s'", _desc.c_str());

	GUI::MessageDialog dialog(Common::String::format("The '%s' minigame is not supported yet.", _desc.c_str()));
	dialog.runModal();

	_vm->addNotifyEvent(NotifyEvent(kLBNotifyChangePage, destPage));

	return false;
}

LBItem *LBMiniGameItem::createClone() {
	error("can't clone LBMiniGameItem");
}

LBProxyItem::LBProxyItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	debug(3, "new LBProxyItem");

	_page = NULL;
}

LBProxyItem::~LBProxyItem() {
	delete _page;
}

void LBProxyItem::load() {
	if (_loaded)
		return;

	Common::String leftover;
	Common::String filename = _vm->getFileNameFromConfig("Proxies", _desc.c_str(), leftover);
	if (!leftover.empty())
		error("LBProxyItem tried loading proxy '%s' but got leftover '%s'", _desc.c_str(), leftover.c_str());
	uint16 baseId = 0;
	for (uint i = 0; i < filename.size(); i++) {
		if (filename[i] == ';') {
			baseId = atoi(filename.c_str() + i + 1);
			filename = Common::String(filename.c_str(), i);
		}
	}

	debug(1, "LBProxyItem loading archive '%s' with id %d", filename.c_str(), baseId);
	Archive *pageArchive = _vm->createArchive();
	if (!pageArchive->openFile(filename))
		error("failed to open archive '%s' (for proxy '%s')", filename.c_str(), _desc.c_str());
	_page = new LBPage(_vm);
	_page->open(pageArchive, baseId);

	LBItem::load();
}

void LBProxyItem::unload() {
	delete _page;
	_page = NULL;

	LBItem::unload();
}

LBItem *LBProxyItem::createClone() {
	return new LBProxyItem(_vm, _page, _rect);
}

} // End of namespace Mohawk
