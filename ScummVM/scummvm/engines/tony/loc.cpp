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

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/memstream.h"
#include "common/scummsys.h"
#include "tony/mpal/mpalutils.h"
#include "tony/game.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

using namespace ::Tony::MPAL;

/****************************************************************************\
*       RMPalette Methods
\****************************************************************************/

void RMPalette::readFromStream(Common::ReadStream &ds) {
	ds.read(_data, 1024);
}

/****************************************************************************\
*       RMSlot Methods
\****************************************************************************/

void RMPattern::RMSlot::readFromStream(Common::ReadStream &ds, bool bLOX) {
	// Type
	byte type = ds.readByte();
	_type = (RMPattern::RMSlotType)type;

	// Data
	_data = ds.readSint32LE();

	// Position
	_pos.readFromStream(ds);

	// Generic flag
	_flag = ds.readByte();
}

/****************************************************************************\
*       RMPattern Methods
\****************************************************************************/

void RMPattern::readFromStream(Common::ReadStream &ds, bool bLOX) {
	// Pattern name
	if (!bLOX)
		_name = readString(ds);

	// Velocity
	_speed = ds.readSint32LE();

	// Position
	_pos.readFromStream(ds);

	// Flag for pattern looping
	_bLoop = ds.readSint32LE();

	// Number of slots
	_nSlots = ds.readSint32LE();

	// Create and read the slots
	_slots = new RMSlot[_nSlots];

	for (int i = 0; i < _nSlots && !ds.err(); i++) {
		if (bLOX)
			_slots[i].readFromStream(ds, true);
		else
			_slots[i].readFromStream(ds, false);
	}
}

void RMPattern::updateCoord() {
	_curPos = _pos + _slots[_nCurSlot].pos();
}

void RMPattern::stopSfx(RMSfx *sfx) {
	for (int i = 0; i < _nSlots; i++) {
		if (_slots[i]._type == SOUND) {
			if (!sfx[_slots[i]._data]._name.empty() && sfx[_slots[i]._data]._name[0] == '_')
				sfx[_slots[i]._data].stop();
			else if (GLOBALS._bSkipSfxNoLoop)
				sfx[_slots[i]._data].stop();
		}
	}
}

int RMPattern::init(RMSfx *sfx, bool bPlayP0, byte *bFlag) {
	// Read the current time
	_nStartTime = g_vm->getTime();
	_nCurSlot = 0;

	// Find the first frame of the pattern
	int i = 0;
	while (_slots[i]._type != SPRITE) {
		assert(i + 1 < _nSlots);
		i++;
	}

	_nCurSlot = i;
	_nCurSprite = _slots[i]._data;
	if (bFlag)
		*bFlag = _slots[i]._flag;

	// Calculate the current coordinates
	updateCoord();

	// Check for sound:
	//  If the slot is 0, play
	//  If speed == 0, must play unless it goes into loop '_', or if specified by the parameter
	//  If speed != 0, play only the loop
	for (i = 0; i < _nSlots; i++) {
		if (_slots[i]._type == SOUND) {
			if (i == 0) {
				if (!sfx[_slots[i]._data]._name.empty() && sfx[_slots[i]._data]._name[0] == '_') {
					sfx[_slots[i]._data].setVolume(_slots[i].pos()._x);
					sfx[_slots[i]._data].play(true);
				} else {
					sfx[_slots[i]._data].setVolume(_slots[i].pos()._x);
					sfx[_slots[i]._data].play();
				}
			} else if (_speed == 0) {
				if (bPlayP0) {
					sfx[_slots[i]._data].setVolume(_slots[i].pos()._x);
					sfx[_slots[i]._data].play();
				} else if (!sfx[_slots[i]._data]._name.empty() && sfx[_slots[i]._data]._name[0] == '_') {
					sfx[_slots[i]._data].setVolume(_slots[i].pos()._x);
					sfx[_slots[i]._data].play(true);
				}
			} else {
				if (_bLoop && !sfx[_slots[i]._data]._name.empty() && sfx[_slots[i]._data]._name[0] == '_') {
					sfx[_slots[i]._data].setVolume(_slots[i].pos()._x);
					sfx[_slots[i]._data].play(true);
				}
			}
		}
	}

	return _nCurSprite;
}

int RMPattern::update(uint32 hEndPattern, byte &bFlag, RMSfx *sfx) {
	int CurTime = g_vm->getTime();

	// If the speed is 0, then the pattern never advances
	if (_speed == 0) {
		CoroScheduler.pulseEvent(hEndPattern);
		bFlag = _slots[_nCurSlot]._flag;
		return _nCurSprite;
	}

	// Is it time to change the slots?
	while (_nStartTime + _speed <= (uint32)CurTime) {
		_nStartTime += _speed;
		if (_slots[_nCurSlot]._type == SPRITE)
			_nCurSlot++;
		if (_nCurSlot == _nSlots) {
			_nCurSlot = 0;
			bFlag = _slots[_nCurSlot]._flag;

			CoroScheduler.pulseEvent(hEndPattern);

			// @@@ If there is no loop pattern, and there's a warning that it's the final
			// frame, then remain on the last frame
			if (!_bLoop) {
				_nCurSlot = _nSlots - 1;
				bFlag = _slots[_nCurSlot]._flag;
				return _nCurSprite;
			}
		}

		for (;;) {
			switch (_slots[_nCurSlot]._type) {
			case SPRITE:
				// Read the next sprite
				_nCurSprite = _slots[_nCurSlot]._data;

				// Update the parent & child coordinates
				updateCoord();
				break;

			case SOUND:
				if (sfx != NULL) {
					sfx[_slots[_nCurSlot]._data].setVolume(_slots[_nCurSlot].pos()._x);

					if (sfx[_slots[_nCurSlot]._data]._name.empty() || sfx[_slots[_nCurSlot]._data]._name[0] != '_')
						sfx[_slots[_nCurSlot]._data].play(false);
					else
						sfx[_slots[_nCurSlot]._data].play(true);
				}
				break;

			case COMMAND:
				assert(0);
				break;

			default:
				assert(0);
				break;
			}

			if (_slots[_nCurSlot]._type == SPRITE)
				break;
			_nCurSlot++;
		}
	}

	// Return the current sprite
	bFlag = _slots[_nCurSlot]._flag;
	return _nCurSprite;
}

RMPattern::RMPattern() {
	_slots = NULL;
	_speed = 0;
	_bLoop  = 0;
	_nSlots = 0;
	_nCurSlot = 0;
	_nCurSprite = 0;
	_nStartTime = 0;
	_slots = NULL;
}

/**
 * Reads the position of the pattern
 */
RMPoint RMPattern::pos() {
	return _curPos;
}

RMPattern::~RMPattern() {
	if (_slots != NULL) {
		delete[] _slots;
		_slots = NULL;
	}
}

/****************************************************************************\
*       RMSprite Methods
\****************************************************************************/

void RMSprite::init(RMGfxSourceBuffer *buf) {
	_buf = buf;
}

void RMSprite::LOXGetSizeFromStream(Common::SeekableReadStream &ds, int *dimx, int *dimy) {
	uint32 pos = ds.pos();

	*dimx = ds.readSint32LE();
	*dimy = ds.readSint32LE();

	ds.seek(pos);
}

void RMSprite::getSizeFromStream(Common::SeekableReadStream &ds, int *dimx, int *dimy) {
	uint32 pos = ds.pos();

	_name = readString(ds);
	*dimx = ds.readSint32LE();
	*dimy = ds.readSint32LE();

	ds.seek(pos);
}

void RMSprite::readFromStream(Common::SeekableReadStream &ds, bool bLOX) {
	// Sprite name
	if (!bLOX)
		_name = readString(ds);

	// Dimensions
	int dimx = ds.readSint32LE();
	int dimy = ds.readSint32LE();

	// Bounding box
	_rcBox.readFromStream(ds);

	// Unused space
	if (!bLOX)
		ds.skip(32);

	// Create buffer and read
	_buf->init(ds, dimx, dimy);
}

void RMSprite::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	_buf->draw(coroParam, bigBuf, prim);
}

void RMSprite::setPalette(byte *buf) {
	((RMGfxSourceBufferPal *)_buf)->loadPalette(buf);
}

RMSprite::RMSprite() {
	_buf = NULL;
}

RMSprite::~RMSprite() {
	if (_buf) {
		delete _buf;
		_buf = NULL;
	}
}

/****************************************************************************\
*       RMSfx Methods
\****************************************************************************/

void RMSfx::readFromStream(Common::ReadStream &ds, bool bLOX) {
	// sfx name
	_name = readString(ds);

	int size = ds.readSint32LE();

	// Read the entire buffer into a MemoryReadStream
	byte *buffer = (byte *)malloc(size);
	ds.read(buffer, size);
	Common::SeekableReadStream *stream = new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);

	// Create the sound effect
	_fx = g_vm->createSFX(stream);
	_fx->setLoop(false);
}

RMSfx::RMSfx() {
	_fx = NULL;
	_bPlayingLoop = false;
}

RMSfx::~RMSfx() {
	if (_fx) {
		_fx->release();
		_fx = NULL;
	}
}

void RMSfx::play(bool bLoop) {
	if (_fx && !_bPlayingLoop) {
		_fx->setLoop(bLoop);
		_fx->play();

		if (bLoop)
			_bPlayingLoop = true;
	}
}

void RMSfx::setVolume(int vol) {
	if (_fx) {
		_fx->setVolume(vol);
	}
}

void RMSfx::pause(bool bPause) {
	if (_fx) {
		_fx->setPause(bPause);
	}
}

void RMSfx::stop() {
	if (_fx) {
		_fx->stop();
		_bPlayingLoop = false;
	}
}

/****************************************************************************\
*       RMItem Methods
\****************************************************************************/

int RMItem::getCurPattern() {
	return _nCurPattern;
}

RMGfxSourceBuffer *RMItem::newItemSpriteBuffer(int dimx, int dimy, bool bPreRLE) {
	if (_cm == CM_256) {
		RMGfxSourceBuffer8RLE *spr;

		if (_FX == 2) {    // AB
			spr = new RMGfxSourceBuffer8RLEWordAB;
		} else if (_FX == 1) { // OMBRA+AA
			if (dimx == -1 || dimx > 255)
				spr = new RMGfxSourceBuffer8RLEWordAA;
			else
				spr = new RMGfxSourceBuffer8RLEByteAA;

			spr->setAlphaBlendColor(_FXparm);
			if (bPreRLE)
				spr->setAlreadyCompressed();
		} else {
			if (dimx == -1 || dimx > 255)
				spr = new RMGfxSourceBuffer8RLEWord;
			else
				spr = new RMGfxSourceBuffer8RLEByte;

			if (bPreRLE)
				spr->setAlreadyCompressed();
		}

		return spr;
	} else
		return new RMGfxSourceBuffer16;
}

bool RMItem::isIn(const RMPoint &pt, int *size)  {
	RMRect rc;

	if (!_bIsActive)
		return false;

	// Search for the right bounding box to use - use the sprite's if it has one, otherwise use the generic one
	if (_nCurPattern != 0 && !_sprites[_nCurSprite]._rcBox.isEmpty())
		rc = _sprites[_nCurSprite]._rcBox + calculatePos();
	else if (!_rcBox.isEmpty())
		rc = _rcBox;
	// If no box, return immediately
	else
		return false;

	if (size != NULL)
		*size = rc.size();

	return rc.ptInRect(pt + _curScroll);
}

void RMItem::readFromStream(Common::SeekableReadStream &ds, bool bLOX) {
	// MPAL code
	_mpalCode = ds.readSint32LE();

	// Object name
	_name = readString(ds);

	// Z (signed)
	_z = ds.readSint32LE();

	// Parent position
	_pos.readFromStream(ds);

	// Hotspot
	_hot.readFromStream(ds);

	// Bounding box
	_rcBox.readFromStream(ds);

	// Number of sprites, sound effects, and patterns
	_nSprites = ds.readSint32LE();
	_nSfx = ds.readSint32LE();
	_nPatterns = ds.readSint32LE();

	// Color mode
	byte cm = ds.readByte();
	_cm = (RMColorMode)cm;

	// Flag for the presence of custom palette differences
	_bPal = ds.readByte();

	if (_cm == CM_256) {
		//  If there is a palette, read it in
		if (_bPal)
			_pal.readFromStream(ds);
	}

	// MPAL data
	if (!bLOX)
		ds.skip(20);

	_FX = ds.readByte();
	_FXparm = ds.readByte();

	if (!bLOX)
		ds.skip(106);

	// Create sub-classes
	if (_nSprites > 0)
		_sprites = new RMSprite[_nSprites];
	if (_nSfx > 0)
		_sfx = new RMSfx[_nSfx];
	_patterns = new RMPattern[_nPatterns + 1];

	int dimx, dimy;
	// Read in class data
	if (!ds.err()) {
		for (int i = 0; i < _nSprites && !ds.err(); i++) {
			// Download the sprites
			if (bLOX) {
				_sprites[i].LOXGetSizeFromStream(ds, &dimx, &dimy);
				_sprites[i].init(newItemSpriteBuffer(dimx, dimy, true));
				_sprites[i].readFromStream(ds, true);
			} else {
				_sprites[i].getSizeFromStream(ds, &dimx, &dimy);
				_sprites[i].init(newItemSpriteBuffer(dimx, dimy, false));
				_sprites[i].readFromStream(ds, false);
			}

			if (_cm == CM_256 && _bPal)
				_sprites[i].setPalette(_pal._data);
		}
	}

	if (!ds.err()) {
		for (int i = 0; i < _nSfx && !ds.err(); i++) {
			if (bLOX)
				_sfx[i].readFromStream(ds, true);
			else
				_sfx[i].readFromStream(ds, false);
		}
	}

	// Read the pattern from pattern 1
	if (!ds.err()) {
		for (int i = 1; i <= _nPatterns && !ds.err(); i++) {
			if (bLOX)
				_patterns[i].readFromStream(ds, true);
			else
				_patterns[i].readFromStream(ds, false);
		}
	}

	// Initialize the current pattern
	if (_bInitCurPattern)
		setPattern(mpalQueryItemPattern(_mpalCode));

	// Initialize the current activation state
	_bIsActive = mpalQueryItemIsActive(_mpalCode);
}

RMGfxPrimitive *RMItem::newItemPrimitive() {
	return new RMGfxPrimitive(this);
}

void RMItem::setScrollPosition(const RMPoint &scroll) {
	_curScroll = scroll;
}

bool RMItem::doFrame(RMGfxTargetBuffer *bigBuf, bool bAddToList) {
	int oldSprite = _nCurSprite;

	// Pattern 0 = Do not draw anything!
	if (_nCurPattern == 0)
		return false;

	// We do an update of the pattern, which also returns the current frame
	if (_nCurPattern != 0) {
		_nCurSprite = _patterns[_nCurPattern].update(_hEndPattern, _bCurFlag, _sfx);

		// WORKAROUND: Currently, m_nCurSprite = -1 is used to flag that an item should be removed.
		// However, this seems to be done inside a process waiting on an event pulsed inside the pattern
		// Update method. So the value of m_nCurSprite = -1 is being destroyed with the return value
		// replacing it. It may be that the current coroutine PulseEvent implementation is wrong somehow.
		// In any case, a special check here is done for items that have ended
		if (_nCurPattern == 0)
			_nCurSprite = -1;
	}

	// If the function returned -1, it means that the pattern has finished
	if (_nCurSprite == -1) {
		// We have pattern 0, so leave. The class will self de-register from the OT list
		_nCurPattern = 0;
		return false;
	}

	// If we are not in the OT list, add ourselves
	if (!_nInList && bAddToList)
		bigBuf->addPrim(newItemPrimitive());

	return oldSprite != _nCurSprite;
}

RMPoint RMItem::calculatePos() {
	return _pos + _patterns[_nCurPattern].pos();
}

void RMItem::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If CurSprite == -1, then the pattern is finished
	if (_nCurSprite == -1)
		return;

	// Set the flag
	prim->setFlag(_bCurFlag);

	// Offset direction for scrolling
	prim->getDst().offset(-_curScroll);

	// We must offset the cordinates of the item inside the primitive
	// It is estimated as nonno + (babbo + figlio)
	prim->getDst().offset(calculatePos());

	// No stretching, please
	prim->setStretch(false);

	// Now we turn to the generic surface drawing routines
	CORO_INVOKE_2(_sprites[_nCurSprite].draw, bigBuf, prim);

	CORO_END_CODE;
}

/**
 * Overloaded priority: it's based on Z ordering
 */
int RMItem::priority() {
	return _z;
}

/**
 * Pattern number
 */
int RMItem::numPattern() {
	return _nPatterns;
}

void RMItem::removeThis(CORO_PARAM, bool &result) {
	// Remove from the OT list if the current frame is -1 (pattern over)
	result = (_nCurSprite == -1);
}

void RMItem::setStatus(int nStatus) {
	_bIsActive = (nStatus > 0);
}

RMPoint RMItem::getHotspot() {
	return _hot;
}

int RMItem::mpalCode() {
	return _mpalCode;
}

void RMItem::setPattern(int nPattern, bool bPlayP0) {
	assert(nPattern >= 0 && nPattern <= _nPatterns);

	if (_sfx) {
		if (_nCurPattern > 0)
			_patterns[_nCurPattern].stopSfx(_sfx);
	}

	// Remember the current pattern
	_nCurPattern = nPattern;

	// Start the pattern to start the animation
	if (_nCurPattern != 0)
		_nCurSprite = _patterns[_nCurPattern].init(_sfx, bPlayP0, &_bCurFlag);
	else {
		_nCurSprite = -1;

		// Look for the sound effect for pattern 0
		if (bPlayP0) {
			for (int i = 0; i < _nSfx; i++) {
				if (_sfx[i]._name == "p0")
					_sfx[i].play();
			}
		}
	}
}

bool RMItem::getName(Common::String &name) {
	char buf[256];

	mpalQueryItemName(_mpalCode, buf);
	name = buf;
	if (buf[0] == '\0')
		return false;
	return true;
}

void RMItem::unload() {
	if (_patterns != NULL) {
		delete[] _patterns;
		_patterns = NULL;
	}

	if (_sprites != NULL) {
		delete[] _sprites;
		_sprites = NULL;
	}

	if (_sfx != NULL) {
		delete[] _sfx;
		_sfx = NULL;
	}
}

RMItem::RMItem() {
	_bCurFlag = 0;
	_patterns = NULL;
	_sprites = NULL;
	_sfx = NULL;
	_curScroll.set(0, 0);
	_bInitCurPattern = true;
	_nCurPattern = 0;
	_z = 0;
	_cm = CM_256;
	_FX = 0;
	_FXparm = 0;
	_mpalCode = 0;
	_nSprites = 0;
	_nSfx = 0;
	_nPatterns = 0;
	_bPal = 0;
	_nCurSprite = 0;

	_bIsActive = false;
	memset(_pal._data, 0, sizeof(_pal._data));

	_hEndPattern = CoroScheduler.createEvent(false, false);
}

RMItem::~RMItem() {
	unload();
	CoroScheduler.closeEvent(_hEndPattern);
}

void RMItem::waitForEndPattern(CORO_PARAM, uint32 hCustomSkip) {
	CORO_BEGIN_CONTEXT;
	uint32 h[2];
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_nCurPattern != 0) {
		if (hCustomSkip == CORO_INVALID_PID_VALUE)
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _hEndPattern, CORO_INFINITE);
		else {
			_ctx->h[0] = hCustomSkip;
			_ctx->h[1] = _hEndPattern;
			CORO_INVOKE_4(CoroScheduler.waitForMultipleObjects, 2, &_ctx->h[0], false, CORO_INFINITE);
		}
	}

	CORO_END_CODE;
}

void RMItem::changeHotspot(const RMPoint &pt) {
	_hot = pt;
}

void RMItem::setInitCurPattern(bool status) {
	_bInitCurPattern = status;
}

void RMItem::playSfx(int nSfx) {
	if (nSfx < _nSfx)
		_sfx[nSfx].play();
}

void RMItem::pauseSound(bool bPause) {
	for (int i = 0; i < _nSfx; i++)
		_sfx[i].pause(bPause);
}

/****************************************************************************\
*       RMWipe Methods
\****************************************************************************/

RMWipe::RMWipe() {
	_hUnregistered = CoroScheduler.createEvent(false, false);
	_hEndOfFade = CoroScheduler.createEvent(false, false);

	_bMustRegister = false;
	_bUnregister = false;
	_bEndFade = false;
	_bFading = false;
	_nFadeStep = 0;
}

RMWipe::~RMWipe() {
	CoroScheduler.closeEvent(_hUnregistered);
	CoroScheduler.closeEvent(_hEndOfFade);
}

int RMWipe::priority() {
	return 200;
}

void RMWipe::unregister() {
	RMGfxTask::unregister();
	assert(_nInList == 0);
	CoroScheduler.setEvent(_hUnregistered);
}

void RMWipe::removeThis(CORO_PARAM, bool &result) {
	result = _bUnregister;
}

void RMWipe::waitForFadeEnd(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _hEndOfFade, CORO_INFINITE);

	_bEndFade = true;
	_bFading = false;

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

	CORO_END_CODE;
}

void RMWipe::closeFade() {
	_wip0r.unload();
}

void RMWipe::initFade(int type) {
	// Activate the fade
	_bUnregister = false;
	_bEndFade = false;

	_nFadeStep = 0;

	_bMustRegister = true;

	RMRes res(RES_W_CIRCLE);
	Common::SeekableReadStream *ds = res.getReadStream();
	_wip0r.readFromStream(*ds);
	delete ds;

	_wip0r.setPattern(1);

	_bFading = true;
}

void RMWipe::doFrame(RMGfxTargetBuffer &bigBuf) {
	if (_bMustRegister) {
		bigBuf.addPrim(new RMGfxPrimitive(this));
		_bMustRegister = false;
	}

	if (_bFading) {
		_wip0r.doFrame(&bigBuf, false);

		_nFadeStep++;

		if (_nFadeStep == 10) {
			CoroScheduler.setEvent(_hEndOfFade);
		}
	}
}

void RMWipe::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bFading) {
		CORO_INVOKE_2(_wip0r.draw, bigBuf, prim);
	}

	if (_bEndFade)
		Common::fill((byte *)bigBuf, (byte *)bigBuf + bigBuf.getDimx() * bigBuf.getDimy() * 2, 0x0);

	CORO_END_CODE;
}

/****************************************************************************\
*       RMCharacter Methods
\****************************************************************************/

/****************************************************************************/
/* Find the shortest path between two nodes of the graph connecting the BOX */
/* Returns path along the vector path path[]                                */
/****************************************************************************/

short RMCharacter::findPath(short source, short destination) {
	static RMBox box[MAXBOXES];      // Matrix of adjacent boxes
	static short nodeCost[MAXBOXES]; // Cost per node
	static short valid[MAXBOXES];    // 0:Invalid 1:Valid 2:Saturated
	static short nextNode[MAXBOXES]; // Next node
	short minCost, error = 0;
	RMBoxLoc *cur;

	g_system->lockMutex(_csMove);

	if (source == -1 || destination == -1) {
		g_system->unlockMutex(_csMove);
		return 0;
	}

	// Get the boxes
	cur = _theBoxes->getBoxes(_curLocation);

	// Make a backup copy to work on
	for (int i = 0; i < cur->_numbBox; i++)
		memcpy(&box[i], &cur->_boxes[i], sizeof(RMBox));

	// Invalidate all nodes
	for (int i = 0; i < cur->_numbBox; i++)
		valid[i] = 0;

	// Prepare source and variables for the procedure
	nodeCost[source] = 0;
	valid[source] = 1;
	bool finish = false;

	// Find the shortest path
	while (!finish) {
		minCost = 32000; // Reset the minimum cost
		error = 1;       // Possible error

		// 1st cycle: explore possible new nodes
		for (int i = 0; i < cur->_numbBox; i++) {
			if (valid[i] == 1) {
				error = 0; // Failure de-bunked
				int j = 0;
				while (((box[i]._adj[j]) != 1) && (j < cur->_numbBox))
					j++;

				if (j >= cur->_numbBox)
					valid[i] = 2; // nodo saturated?
				else {
					nextNode[i] = j;
					if (nodeCost[i] + 1 < minCost)
						minCost = nodeCost[i] + 1;
				}
			}
		}

		if (error)
			finish = true; // All nodes saturated

		// 2nd cycle: adding new nodes that were found, saturate old nodes
		for (int i = 0; i < cur->_numbBox; i++) {
			if ((valid[i] == 1) && ((nodeCost[i] + 1) == minCost)) {
				box[i]._adj[nextNode[i]] = 2;
				nodeCost[nextNode[i]] = minCost;
				valid[nextNode[i]] = 1;
				for (int j = 0; j < cur->_numbBox; j++)
					if (box[j]._adj[nextNode[i]] == 1)
						box[j]._adj[nextNode[i]] = 0;

				if (nextNode[i] == destination)
					finish = true;
			}
		}
	}

	// Remove the path from the adjacent modified matrixes
	if (!error) {
		_pathLength = nodeCost[destination];
		short k = _pathLength;
		_path[k] = destination;

		while (_path[k] != source) {
			int i = 0;
			while (box[i]._adj[_path[k]] != 2)
				i++;
			k--;
			_path[k] = i;
		}

		_pathLength++;
	}

	g_system->unlockMutex(_csMove);

	return !error;
}

void RMCharacter::goTo(CORO_PARAM, RMPoint destcoord, bool bReversed) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_pos == destcoord) {
		if (_minPath == 0) {
			CORO_INVOKE_0(stop);
			CoroScheduler.pulseEvent(_hEndOfPath);
			return;
		}
	}

	_status = WALK;
	_lineStart = _pos;
	_lineEnd = destcoord;
	_dx = _lineStart._x - _lineEnd._x;
	_dy = _lineStart._y - _lineEnd._y;
	_fx = _dx;
	_fy = _dy;
	_dx = ABS(_dx);
	_dy = ABS(_dy);
	_walkSpeed = _curSpeed;
	_walkCount = 0;

	if (bReversed) {
		while (0) ;
	}

	int nPatt = getCurPattern();

	if (_dx > _dy) {
		_slope = _fy / _fx;
		if (_lineEnd._x < _lineStart._x)
			_walkSpeed = -_walkSpeed;
		_walkStatus = 1;

		// Change the pattern for the new direction
		_bNeedToStop = true;
		if ((_walkSpeed < 0 && !bReversed) || (_walkSpeed >= 0 && bReversed))  {
			if (nPatt != PAT_WALKLEFT)
				setPattern(PAT_WALKLEFT);
		} else {
			if (nPatt != PAT_WALKRIGHT)
				setPattern(PAT_WALKRIGHT);
		}
	} else {
		_slope = _fx / _fy;
		if (_lineEnd._y < _lineStart._y)
			_walkSpeed = -_walkSpeed;
		_walkStatus = 0;

		_bNeedToStop = true;
		if ((_walkSpeed < 0 && !bReversed) || (_walkSpeed >= 0 && bReversed)) {
			if (nPatt != PAT_WALKUP)
				setPattern(PAT_WALKUP);
		} else {
			if (nPatt != PAT_WALKDOWN)
				setPattern(PAT_WALKDOWN);
		}
	}

	_olddx = _dx;
	_olddy = _dy;

	CORO_END_CODE;
}

RMPoint RMCharacter::searching(char UP, char DOWN, char RIGHT, char LEFT, RMPoint point) {
	short steps;
	RMPoint newPt, foundPt;
	short minStep = 32000;

	if (UP) {
		newPt = point;
		steps = 0;
		while ((inWhichBox(newPt) == -1) && (newPt._y >= 0)) {
			newPt._y--;
			steps++;
		}
		if ((inWhichBox(newPt) != -1) && (steps < minStep) &&
		    findPath(inWhichBox(_pos), inWhichBox(newPt))) {
			minStep = steps;
			newPt._y--; // to avoid error?
			foundPt = newPt;
		}
	}

	if (DOWN) {
		newPt = point;
		steps = 0;
		while ((inWhichBox(newPt) == -1) && (newPt._y < 480)) {
			newPt._y++;
			steps++;
		}
		if ((inWhichBox(newPt) != -1) && (steps < minStep) &&
		    findPath(inWhichBox(_pos), inWhichBox(newPt))) {
			minStep = steps;
			newPt._y++; // to avoid error?
			foundPt = newPt;
		}
	}

	if (RIGHT) {
		newPt = point;
		steps = 0;
		while ((inWhichBox(newPt) == -1) && (newPt._x < 640)) {
			newPt._x++;
			steps++;
		}
		if ((inWhichBox(newPt) != -1) && (steps < minStep) &&
		    findPath(inWhichBox(_pos), inWhichBox(newPt))) {
			minStep = steps;
			newPt._x++; // to avoid error?
			foundPt = newPt;
		}
	}

	if (LEFT) {
		newPt = point;
		steps = 0;
		while ((inWhichBox(newPt) == -1) && (newPt._x >= 0)) {
			newPt._x--;
			steps++;
		}
		if ((inWhichBox(newPt) != -1) && (steps < minStep) &&
		    findPath(inWhichBox(_pos), inWhichBox(newPt))) {
			minStep = steps;
			newPt._x--; // to avoid error?
			foundPt = newPt;
		}
	}

	if (minStep == 32000)
		foundPt = point;

	return foundPt;
}

RMPoint RMCharacter::nearestPoint(const RMPoint &point) {
	return searching(1, 1, 1, 1, point);
}

short RMCharacter::scanLine(const RMPoint &point) {
	int Ldx, Ldy, Lcount;
	float Lfx, Lfy, Lslope;
	RMPoint Lstart, Lend, Lscan;
	signed char Lspeed, Lstatus;

	Lstart = _pos;
	Lend = point;
	Ldx = Lstart._x - Lend._x;
	Ldy = Lstart._y - Lend._y;
	Lfx = Ldx;
	Lfy = Ldy;
	Ldx = ABS(Ldx);
	Ldy = ABS(Ldy);
	Lspeed = 1;
	Lcount = 0;

	if (Ldx > Ldy) {
		Lslope = Lfy / Lfx;
		if (Lend._x < Lstart._x)
			Lspeed = -Lspeed;
		Lstatus = 1;
	} else {
		Lslope = Lfx / Lfy;
		if (Lend._y < Lstart._y)
			Lspeed = - Lspeed;
		Lstatus = 0;
	}

	Lscan = Lstart; // Start scanning
	while (inWhichBox(Lscan) != -1) {
		Lcount++;
		if (Lstatus) {
			Ldx = Lspeed * Lcount;
			Ldy = (int)(Lslope * Ldx);
		} else {
			Ldy = Lspeed * Lcount;
			Ldx = (int)(Lslope * Ldy);
		}

		Lscan._x = Lstart._x + Ldx;
		Lscan._y = Lstart._y + Ldy;

		if ((ABS(Lscan._x - Lend._x) <= 1) && (ABS(Lscan._y - Lend._y) <= 1))
			return 1;
	}

	return 0;
}

/**
 * Calculates intersections between the straight line and the closest BBOX
 */
RMPoint RMCharacter::invScanLine(const RMPoint &point) {
	RMPoint lStart = point; // Exchange!
	RMPoint lEnd = _pos; // :-)
	int lDx = lStart._x - lEnd._x;
	int lDy = lStart._y - lEnd._y;
	float lFx = lDx;
	float lFy = lDy;
	lDx = ABS(lDx);
	lDy = ABS(lDy);
	signed char lSpeed = 1;
	int lCount = 0;

	signed char lStatus;
	float lSlope;

	if (lDx > lDy) {
		lSlope = lFy / lFx;
		if (lEnd._x < lStart._x)
			lSpeed = -lSpeed;
		lStatus = 1;
	} else {
		lSlope = lFx / lFy;
		if (lEnd._y < lStart._y)
			lSpeed = -lSpeed;
		lStatus = 0;
	}

	RMPoint lScan = lStart;
	signed char lBox = -1;

	for (;;) {
		if (inWhichBox(lScan) != -1) {
			if (inWhichBox(lScan) != lBox) {
				if (inWhichBox(_pos) == inWhichBox(lScan) || findPath(inWhichBox(_pos), inWhichBox(lScan)))
					return lScan;
				else
					lBox = inWhichBox(lScan);
			}
		}

		lCount++;
		if (lStatus) {
			lDx = lSpeed * lCount;
			lDy = (int)(lSlope * lDx);
		} else {
			lDy = lSpeed * lCount;
			lDx = (int)(lSlope * lDy);
		}
		lScan._x = lStart._x + lDx;
		lScan._y = lStart._y + lDy;

		// WORKAROUND: Handles cases where the points never fall inside a bounding box
		if (lScan._x < -100 || lScan._y < -100 || lScan._x >= 1000 || lScan._y >= 1000)
			return point;
	}
}

/**
 * Returns the HotSpot coordinate closest to the player
 */

RMPoint RMCharacter::nearestHotSpot(int sourcebox, int destbox) {
	RMPoint hotspot;
	int x, y;
	int minDist = 10000000;
	RMBoxLoc *cur = _theBoxes->getBoxes(_curLocation);

	for (short cc = 0; cc < cur->_boxes[sourcebox]._numHotspot; cc++)
		if ((cur->_boxes[sourcebox]._hotspot[cc]._destination) == destbox) {
			x = ABS(cur->_boxes[sourcebox]._hotspot[cc]._hotx - _pos._x);
			y = ABS(cur->_boxes[sourcebox]._hotspot[cc]._hoty - _pos._y);

			if ((x * x + y * y) < minDist) {
				minDist = x * x + y * y;
				hotspot._x = cur->_boxes[sourcebox]._hotspot[cc]._hotx;
				hotspot._y = cur->_boxes[sourcebox]._hotspot[cc]._hoty;
			}
		}

	return hotspot;
}

void RMCharacter::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bDrawNow) {
		prim->getDst() += _fixedScroll;

		CORO_INVOKE_2(RMItem::draw, bigBuf, prim);
	}

	CORO_END_CODE;
}

void RMCharacter::newBoxEntered(int nBox) {
	RMBoxLoc *cur;

	// Recall on ExitBox
	mpalQueryDoAction(3, _curLocation, _curBox);

	cur = _theBoxes->getBoxes(_curLocation);
	bool bOldReverse = cur->_boxes[_curBox]._bReversed;
	_curBox = nBox;

	// If Z is changed, we must remove it from the OT
	if (cur->_boxes[_curBox]._destZ != _z) {
		_bRemoveFromOT = true;
		_z = cur->_boxes[_curBox]._destZ;
	}

	// Movement management is reversed, only if we are not in the shortest path. If we are in the shortest
	// path, directly do the DoFrame
	if (_bMovingWithoutMinpath) {
		if ((cur->_boxes[_curBox]._bReversed && !bOldReverse) || (!cur->_boxes[_curBox]._bReversed && bOldReverse)) {
			switch (getCurPattern()) {
			case PAT_WALKUP:
				setPattern(PAT_WALKDOWN);
				break;
			case PAT_WALKDOWN:
				setPattern(PAT_WALKUP);
				break;
			case PAT_WALKRIGHT:
				setPattern(PAT_WALKLEFT);
				break;
			case PAT_WALKLEFT:
				setPattern(PAT_WALKRIGHT);
				break;
			}
		}
	}

	// Recall On EnterBox
	mpalQueryDoAction(2, _curLocation, _curBox);
}

void RMCharacter::doFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int loc) {
	CORO_BEGIN_CONTEXT;
	bool bEndNow;
	RMBoxLoc *cur;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bEndNow = false;
	_bEndOfPath = false;
	_bDrawNow = (_curLocation == loc);

	g_system->lockMutex(_csMove);

	// If we're walking..
	if (_status != STAND) {
		// If we are going horizontally
		if (_walkStatus == 1) {
			_dx = _walkSpeed * _walkCount;
			_dy = (int)(_slope * _dx);
			_pos._x = _lineStart._x + _dx;
			_pos._y = _lineStart._y + _dy;

			// Right
			if (((_walkSpeed > 0) && (_pos._x > _lineEnd._x)) || ((_walkSpeed < 0) && (_pos._x < _lineEnd._x))) {
				_pos = _lineEnd;
				_status = STAND;
				_ctx->bEndNow = true;
			}
		}

		// If we are going vertical
		if (_walkStatus == 0) {
			_dy = _walkSpeed * _walkCount;
			_dx = (int)(_slope * _dy);
			_pos._x = _lineStart._x + _dx;
			_pos._y = _lineStart._y + _dy;

			// Down
			if (((_walkSpeed > 0) && (_pos._y > _lineEnd._y)) || ((_walkSpeed < 0) && (_pos._y < _lineEnd._y))) {
				_pos = _lineEnd;
				_status = STAND;
				_ctx->bEndNow = true;
			}
		}

		// Check if the character came out of the BOX in error, in which case he returns immediately
		if (inWhichBox(_pos) == -1) {
			_pos._x = _lineStart._x + _olddx;
			_pos._y = _lineStart._y + _olddy;
		}

		// If we have just moved to a temporary location, and is over the shortest path, we stop permanently
		if (_ctx->bEndNow && _minPath == 0) {
			if (!_bEndOfPath)
				CORO_INVOKE_0(stop);
			_bEndOfPath = true;
			CoroScheduler.pulseEvent(_hEndOfPath);
		}

		_walkCount++;

		// Update the character Z. @@@ Should remove only if the Z was changed

		// Check if the box was changed
		if (!_theBoxes->isInBox(_curLocation, _curBox, _pos))
			newBoxEntered(inWhichBox(_pos));

		// Update the old coordinates
		_olddx = _dx;
		_olddy = _dy;
	}

	// If we stop
	if (_status == STAND) {
		// Check if there is still the shortest path to calculate
		if (_minPath == 1) {
			_ctx->cur = _theBoxes->getBoxes(_curLocation);

			// If we still have to go through a box
			if (_pathCount < _pathLength) {
				// Check if the box we're going into is active
				if (_ctx->cur->_boxes[_path[_pathCount - 1]]._bActive) {
					// Move in a straight line towards the nearest hotspot, taking into account the reversing
					// NEWBOX = path[pathcount-1]
					CORO_INVOKE_2(goTo, nearestHotSpot(_path[_pathCount - 1], _path[_pathCount]), _ctx->cur->_boxes[_path[_pathCount - 1]]._bReversed);
					_pathCount++;
				} else {
					// If the box is off, we can only block all
					// @@@ Whilst this should not happen, because have improved
					// the search for the minimum path
					_minPath = 0;
					if (!_bEndOfPath)
						CORO_INVOKE_0(stop);
					_bEndOfPath = true;
					CoroScheduler.pulseEvent(_hEndOfPath);
				}
			} else {
				// If we have already entered the last box, we just have to move in a straight line towards the
				// point of arrival
				// NEWBOX = InWhichBox(pathend)
				_minPath = 0;
				CORO_INVOKE_2(goTo, _pathEnd, _ctx->cur->_boxes[inWhichBox(_pathEnd)]._bReversed);
			}
		}
	}

	g_system->unlockMutex(_csMove);

	// Invoke the DoFrame of the item
	RMItem::doFrame(bigBuf);

	CORO_END_CODE;
}

bool RMCharacter::endOfPath() {
	return _bEndOfPath;
}

void RMCharacter::stop(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_bMoving = false;

	// You never know..
	_status = STAND;
	_minPath = 0;

	if (!_bNeedToStop)
		return;

	_bNeedToStop = false;

	switch (getCurPattern()) {
	case PAT_WALKUP:
		setPattern(PAT_STANDUP);
		break;

	case PAT_WALKDOWN:
		setPattern(PAT_STANDDOWN);
		break;

	case PAT_WALKLEFT:
		setPattern(PAT_STANDLEFT);
		break;

	case PAT_WALKRIGHT:
		setPattern(PAT_STANDRIGHT);
		break;

	default:
		setPattern(PAT_STANDDOWN);
		break;
	}

	CORO_END_CODE;
}

/**
 * Check if the character is moving
 */
bool RMCharacter::isMoving() {
	return _bMoving;
}

inline int RMCharacter::inWhichBox(const RMPoint &pt) {
	return _theBoxes->whichBox(_curLocation, pt);
}

void RMCharacter::move(CORO_PARAM, RMPoint pt, bool *result) {
	CORO_BEGIN_CONTEXT;
	RMPoint dest;
	int numbox;
	RMBoxLoc *cur;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_bMoving = true;

	// 0, 0 does not do anything, just stops the character
	if (pt._x == 0 && pt._y == 0) {
		_minPath = 0;
		_status = STAND;
		CORO_INVOKE_0(stop);
		if (result)
			*result = true;
		return;
	}

	// If clicked outside the box
	_ctx->numbox = inWhichBox(pt);
	if (_ctx->numbox == -1) {
		// Find neareste point inside the box
		_ctx->dest = nearestPoint(pt);

		// ???!??
		if (_ctx->dest == pt)
			_ctx->dest = invScanLine(pt);

		pt = _ctx->dest;
		_ctx->numbox = inWhichBox(pt);
	}

	_ctx->cur = _theBoxes->getBoxes(_curLocation);

	_minPath = 0;
	_status = STAND;
	_bMovingWithoutMinpath = true;
	if (scanLine(pt))
		CORO_INVOKE_2(goTo, pt, _ctx->cur->_boxes[_ctx->numbox]._bReversed);
	else if (findPath(inWhichBox(_pos), inWhichBox(pt))) {
		_bMovingWithoutMinpath = false;
		_minPath = 1;
		_pathCount = 1;
		_pathEnd = pt;
	} else {
		// @@@ This case is whether a hotspot is inside a box, but there is
		// a path to get there. We use the InvScanLine to search around a point
		_ctx->dest = invScanLine(pt);
		pt = _ctx->dest;

		if (scanLine(pt))
			CORO_INVOKE_2(goTo, pt, _ctx->cur->_boxes[_ctx->numbox]._bReversed);
		else if (findPath(inWhichBox(_pos), inWhichBox(pt))) {
			_bMovingWithoutMinpath = false;
			_minPath = 1;
			_pathCount = 1;
			_pathEnd = pt;
			if (result)
				*result = true;
		} else {
			if (result)
				*result = false;
		}

		return;
	}

	if (result)
		*result = true;

	CORO_END_CODE;
}

void RMCharacter::setPosition(const RMPoint &pt, int newloc) {
	RMBoxLoc *box;

	_minPath = 0;
	_status = STAND;
	_pos = pt;

	if (newloc != -1)
		_curLocation = newloc;

	// Update the character's Z value
	box = _theBoxes->getBoxes(_curLocation);
	_curBox = inWhichBox(_pos);
	assert(_curBox != -1);
	_z = box->_boxes[_curBox]._destZ;
	_bRemoveFromOT = true;
}

void RMCharacter::waitForEndMovement(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bMoving)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _hEndOfPath, CORO_INFINITE);

	CORO_END_CODE;
}

void RMCharacter::setFixedScroll(const RMPoint &fix) {
	_fixedScroll = fix;
}

void RMCharacter::setSpeed(int speed) {
	_curSpeed = speed;
}

void RMCharacter::removeThis(CORO_PARAM, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bRemoveFromOT)
		result = true;
	else
		CORO_INVOKE_1(RMItem::removeThis, result);

	CORO_END_CODE;
}

RMCharacter::RMCharacter() {
	_csMove = g_system->createMutex();
	_hEndOfPath = CoroScheduler.createEvent(false, false);
	_minPath = 0;
	_curSpeed = 3;
	_bRemoveFromOT = false;
	_bMoving = false;
	_curLocation = 0;
	_curBox = 0;
	_dx = _dy = 0;
	_olddx = _olddy = 0;
	_fx = _fy = _slope = 0;
	_walkSpeed = _walkStatus = 0;
	_nextBox = 0;
	_pathLength = _pathCount = 0;
	_status = STAND;
	_theBoxes = NULL;
	_walkCount = 0;
	_bEndOfPath = false;
	_bMovingWithoutMinpath = false;
	_bDrawNow = false;
	_bNeedToStop = false;

	memset(_path, 0, sizeof(_path));

	_pos.set(0, 0);
}

RMCharacter::~RMCharacter() {
	g_system->deleteMutex(_csMove);
	CoroScheduler.closeEvent(_hEndOfPath);
}

void RMCharacter::linkToBoxes(RMGameBoxes *boxes) {
	_theBoxes = boxes;
}

/****************************************************************************\
*       RMBox Methods
\****************************************************************************/

void RMBox::readFromStream(Common::ReadStream &ds) {
	// Bbox
	_left = ds.readSint32LE();
	_top = ds.readSint32LE();
	_right = ds.readSint32LE();
	_bottom = ds.readSint32LE();

	// Adjacency
	for (int i = 0; i < MAXBOXES; i++) {
		_adj[i] = ds.readSint32LE();
	}

	// Misc
	_numHotspot = ds.readSint32LE();
	_destZ = ds.readByte();
	byte b = ds.readByte();
	_bActive = b;
	b = ds.readByte();
	_bReversed = b;

	// Reversed expansion space
	for (int i = 0; i < 30; i++)
		ds.readByte();

	uint16 w;
	// Hotspots
	for (int i = 0; i < _numHotspot; i++) {
		w = ds.readUint16LE();
		_hotspot[i]._hotx = w;
		w = ds.readUint16LE();
		_hotspot[i]._hoty = w;
		w = ds.readUint16LE();
		_hotspot[i]._destination = w;
	}
}

/****************************************************************************\
*       RMBoxLoc Methods
\****************************************************************************/

RMBoxLoc::RMBoxLoc() {
	_boxes = NULL;
	_numbBox = 0;
}

RMBoxLoc::~RMBoxLoc() {
	delete[] _boxes;
}

void RMBoxLoc::readFromStream(Common::ReadStream &ds) {
	char buf[2];

	// ID and version
	buf[0] = ds.readByte();
	buf[1] = ds.readByte();
	byte ver = ds.readByte();
	assert(buf[0] == 'B' && buf[1] == 'X');
	assert(ver == 3);

	// Number of boxes
	_numbBox = ds.readSint32LE();

	// Allocate memory for the boxes
	_boxes = new RMBox[_numbBox];

	// Read in boxes
	for (int i = 0; i < _numbBox; i++)
		_boxes[i].readFromStream(ds);
}

void RMBoxLoc::recalcAllAdj() {
	for (int i = 0; i < _numbBox; i++) {
		Common::fill(_boxes[i]._adj, _boxes[i]._adj + MAXBOXES, 0);

		for (int j = 0; j < _boxes[i]._numHotspot; j++) {
			if (_boxes[_boxes[i]._hotspot[j]._destination]._bActive)
				_boxes[i]._adj[_boxes[i]._hotspot[j]._destination] = 1;
		}
	}
}

/****************************************************************************\
*       RMGameBoxes methods
\****************************************************************************/

RMGameBoxes::RMGameBoxes() {
	_nLocBoxes = 0;
	Common::fill(_allBoxes, _allBoxes + GAME_BOXES_SIZE, (RMBoxLoc *)NULL);
}

RMGameBoxes::~RMGameBoxes() {
	for (int i = 1; i <= _nLocBoxes; ++i)
		delete _allBoxes[i];
}

void RMGameBoxes::init() {
	// Load boxes from disk
	_nLocBoxes = 130;
	for (int i = 1; i <= _nLocBoxes; i++) {
		RMRes res(10000 + i);

		Common::SeekableReadStream *ds = res.getReadStream();

		_allBoxes[i] = new RMBoxLoc();
		_allBoxes[i]->readFromStream(*ds);

		_allBoxes[i]->recalcAllAdj();

		delete ds;
	}
}

void RMGameBoxes::close() {
}

RMBoxLoc *RMGameBoxes::getBoxes(int nLoc) {
	return _allBoxes[nLoc];
}

int RMGameBoxes::getLocBoxesCount() const {
	return _nLocBoxes;
}

bool RMGameBoxes::isInBox(int nLoc, int nBox, const RMPoint &pt) {
	RMBoxLoc *cur = getBoxes(nLoc);

	if ((pt._x >= cur->_boxes[nBox]._left) && (pt._x <= cur->_boxes[nBox]._right) &&
	    (pt._y >= cur->_boxes[nBox]._top)  && (pt._y <= cur->_boxes[nBox]._bottom))
		return true;
	else
		return false;
}

int RMGameBoxes::whichBox(int nLoc, const RMPoint &punto) {
	RMBoxLoc *cur = getBoxes(nLoc);

	if (!cur)
		return -1;

	for (int i = 0; i < cur->_numbBox; i++) {
		if (cur->_boxes[i]._bActive) {
			if ((punto._x >= cur->_boxes[i]._left) && (punto._x <= cur->_boxes[i]._right) &&
			    (punto._y >= cur->_boxes[i]._top)  && (punto._y <= cur->_boxes[i]._bottom))
				return i;
		}
	}

	return -1;
}

void RMGameBoxes::changeBoxStatus(int nLoc, int nBox, int status) {
	_allBoxes[nLoc]->_boxes[nBox]._bActive = status;
	_allBoxes[nLoc]->recalcAllAdj();
}

int RMGameBoxes::getSaveStateSize() {
	int size = 4;

	for (int i = 1; i <= _nLocBoxes; i++) {
		size += 4;
		size += _allBoxes[i]->_numbBox;
	}

	return size;
}

void RMGameBoxes::saveState(byte *state) {
	// Save the number of locations with boxes
	WRITE_LE_UINT32(state, _nLocBoxes);
	state += 4;

	// For each location, write out the number of boxes and their status
	for (int i = 1; i <= _nLocBoxes; i++) {
		WRITE_LE_UINT32(state, _allBoxes[i]->_numbBox);
		state += 4;

		for (int j = 0; j < _allBoxes[i]->_numbBox; j++)
			*state++ = _allBoxes[i]->_boxes[j]._bActive;
	}
}

void RMGameBoxes::loadState(byte *state) {
	// Load number of items
	int nloc = READ_LE_UINT32(state);
	state += 4;

	assert(nloc <= _nLocBoxes);

	int nbox;
	// For each location, read the number of boxes and their status
	for (int i = 1; i <= nloc; i++) {
		nbox = READ_LE_UINT32(state);
		state += 4;

		for (int j = 0; j < nbox ; j++) {
			if (j < _allBoxes[i]->_numbBox)
				_allBoxes[i]->_boxes[j]._bActive = *state;

			state++;
		}

		_allBoxes[i]->recalcAllAdj();
	}
}

/****************************************************************************\
*       RMLocation Methods
\****************************************************************************/

/**
 * Standard constructor
 */
RMLocation::RMLocation() {
	_nItems = 0;
	_items = NULL;
	_buf = NULL;
	TEMPNumLoc = 0;
	_cmode = CM_256;
}

RMPoint RMLocation::TEMPGetTonyStart() {
	return TEMPTonyStart;
}

int RMLocation::TEMPGetNumLoc() {
	return TEMPNumLoc;
}

/**
 * Load a location (.LOC) from a given data stream
 *
 * @param ds                        Data stream
 * @returns     True if succeeded OK, false in case of error.
 */
bool RMLocation::load(Common::SeekableReadStream &ds) {
	char id[3];

	// Reset dirty rectangling
	_prevScroll.set(-1, -1);
	_prevFixedScroll.set(-1, -1);

	// Check the ID
	ds.read(id, 3);

	// Check if we are in a LOX
	if (id[0] == 'L' && id[1] == 'O' && id[2] == 'X')
		return loadLOX(ds);

	// Otherwise, check that it is a normal LOC
	if (id[0] != 'L' || id[1] != 'O' || id[2] != 'C')
		return false;

	// Version
	byte ver = ds.readByte();
	assert(ver == 6);

	// Location name
	_name = readString(ds);

	// Skip the MPAL bailouts (64 bytes)
	TEMPNumLoc = ds.readSint32LE();
	TEMPTonyStart._x = ds.readSint32LE();
	TEMPTonyStart._y = ds.readSint32LE();
	ds.skip(64 - 4 * 3);

	// Skip flag associated with the background (?)
	ds.skip(1);

	// Location dimensions
	int dimx = ds.readSint32LE();
	int dimy = ds.readSint32LE();
	_curScroll.set(0, 0);

	// Read the color mode
	byte cm = ds.readByte();
	_cmode = (RMColorMode)cm;

	// Initialize the source buffer and read the location
	switch (_cmode)     {
	case CM_256:
		_buf = new RMGfxSourceBuffer8;
		break;

	case CM_65K:
		_buf = new RMGfxSourceBuffer16;
		break;

	default:
		assert(0);
		break;
	};

	// Initialize the surface, loading the palette if necessary
	_buf->init(ds, dimx, dimy, true);

	// Check the size of the location
	//assert(dimy!=512);

	// Number of objects
	_nItems = ds.readSint32LE();

	// Create and read in the objects
	if (_nItems > 0)
		_items = new RMItem[_nItems];


	g_vm->freezeTime();
	for (int i = 0; i < _nItems && !ds.err(); i++)
		_items[i].readFromStream(ds);
	g_vm->unfreezeTime();

	return ds.err();
}

bool RMLocation::loadLOX(Common::SeekableReadStream &ds) {
	// Version
	byte ver = ds.readByte();
	assert(ver == 1);

	// Location name
	_name = readString(ds);

	// Location number
	TEMPNumLoc = ds.readSint32LE();
	TEMPTonyStart._x = ds.readSint32LE();
	TEMPTonyStart._y = ds.readSint32LE();

	// Dimensions
	int dimx = ds.readSint32LE();
	int dimy = ds.readSint32LE();
	_curScroll.set(0, 0);

	// It's always 65K (16-bit) mode
	_cmode = CM_65K;
	_buf = new RMGfxSourceBuffer16;

	// Initialize the surface, loading in the palette if necessary
	_buf->init(ds, dimx, dimy, true);

	// Number of items
	_nItems = ds.readSint32LE();

	// Create and read objects
	if (_nItems > 0)
		_items = new RMItem[_nItems];

	for (int i = 0; i < _nItems && !ds.err(); i++)
		_items[i].readFromStream(ds, true);

	return ds.err();
}

/**
 * Draw method overloaded from RMGfxSourceBUffer8
 */
void RMLocation::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
		bool priorTracking;
		bool hasChanges;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Set the position of the source scrolling
	if (_buf->getDimy() > RM_SY || _buf->getDimx() > RM_SX) {
		prim->setSrc(RMRect(_curScroll, _curScroll + RMPoint(640, 480)));
	}

	prim->setDst(_fixedScroll);

	// Check whether dirty rects are being tracked, and if there are changes, leave tracking
	// turned on so a dirty rect will be added for the entire background
	_ctx->priorTracking = bigBuf.getTrackDirtyRects();
	_ctx->hasChanges = (_prevScroll != _curScroll) || (_prevFixedScroll != _fixedScroll);
	bigBuf.setTrackDirtyRects(_ctx->priorTracking && _ctx->hasChanges);

	// Invoke the drawing method fo the image class, which will draw the location background
	CORO_INVOKE_2(_buf->draw, bigBuf, prim);

	if (_ctx->hasChanges) {
		_prevScroll = _curScroll;
		_prevFixedScroll = _fixedScroll;
	}
	bigBuf.setTrackDirtyRects(_ctx->priorTracking);

	CORO_END_CODE;
}

/**
 * Prepare a frame, adding the location to the OT list, and all the items that have changed animation frame.
 */
void RMLocation::doFrame(RMGfxTargetBuffer *bigBuf) {
	// If the location is not in the OT list, add it in
	if (!_nInList)
		bigBuf->addPrim(new RMGfxPrimitive(this));

	// Process all the location items
	for (int i = 0; i < _nItems; i++)
		_items[i].doFrame(bigBuf);
}

RMItem *RMLocation::getItemFromCode(uint32 dwCode) {
	for (int i = 0; i < _nItems; i++) {
		if (_items[i].mpalCode() == (int)dwCode)
			return &_items[i];
	}

	return NULL;
}

RMItem *RMLocation::whichItemIsIn(const RMPoint &pt) {
	int found = -1;
	int foundSize = 0;
	int size;

	for (int i = 0; i < _nItems; i++) {
		size = 0;
		if (_items[i].isIn(pt, &size)) {
			if (found == -1 || size < foundSize) {
				foundSize = size;
				found = i;
			}
		}
	}

	if (found == -1)
		return NULL;
	else
		return &_items[found];
}

RMLocation::~RMLocation() {
	unload();
}

void RMLocation::unload() {
	// Clear memory
	if (_items) {
		delete[] _items;
		_items = NULL;
	}

	// Destroy the buffer
	if (_buf) {
		delete _buf;
		_buf = NULL;
	}
}

void RMLocation::updateScrolling(const RMPoint &ptShowThis) {
	RMPoint oldScroll = _curScroll;

	if (_curScroll._x + 250 > ptShowThis._x) {
		_curScroll._x = ptShowThis._x - 250;
	} else if (_curScroll._x + RM_SX - 250 < ptShowThis._x) {
		_curScroll._x = ptShowThis._x + 250 - RM_SX;
	} else if (ABS(_curScroll._x + RM_SX / 2 - ptShowThis._x) > 32 && _buf->getDimx() > RM_SX) {
		if (_curScroll._x + RM_SX / 2 < ptShowThis._x)
			_curScroll._x++;
		else
			_curScroll._x--;
	}

	if (_curScroll._y + 180 > ptShowThis._y) {
		_curScroll._y = ptShowThis._y - 180;
	} else if (_curScroll._y + RM_SY - 180 < ptShowThis._y) {
		_curScroll._y = ptShowThis._y + 180 - RM_SY;
	} else if (ABS(_curScroll._y + RM_SY / 2 - ptShowThis._y) > 16 && _buf->getDimy() > RM_SY) {
		if (_curScroll._y + RM_SY / 2 < ptShowThis._y)
			_curScroll._y++;
		else
			_curScroll._y--;
	}

	if (_curScroll._x < 0)
		_curScroll._x = 0;
	if (_curScroll._y < 0)
		_curScroll._y = 0;
	if (_curScroll._x + RM_SX > _buf->getDimx())
		_curScroll._x = _buf->getDimx() - RM_SX;
	if (_curScroll._y + RM_SY > _buf->getDimy())
		_curScroll._y = _buf->getDimy() - RM_SY;

	if (oldScroll != _curScroll) {
		for (int i = 0; i < _nItems; i++)
			_items[i].setScrollPosition(_curScroll);
	}
}

void RMLocation::setFixedScroll(const RMPoint &scroll) {
	_fixedScroll = scroll;

	for (int i = 0; i < _nItems; i++)
		_items[i].setScrollPosition(_curScroll - _fixedScroll);
}

void RMLocation::setScrollPosition(const RMPoint &scroll) {
	RMPoint pt = scroll;
	if (pt._x < 0)
		pt._x = 0;
	if (pt._y < 0)
		pt._y = 0;
	if (pt._x + RM_SX > _buf->getDimx())
		pt._x = _buf->getDimx() - RM_SX;
	if (pt._y + RM_SY > _buf->getDimy())
		pt._y = _buf->getDimy() - RM_SY;

	_curScroll = pt;

	for (int i = 0; i < _nItems; i++)
		_items[i].setScrollPosition(_curScroll);
}


void RMLocation::pauseSound(bool bPause) {
	for (int i = 0; i < _nItems; i++)
		_items[i].pauseSound(bPause);
}

/**
 * Read the current scroll position
 */
RMPoint RMLocation::scrollPosition() {
	return _curScroll;
}

/****************************************************************************\
*       RMMessage Methods
\****************************************************************************/

RMMessage::RMMessage(uint32 dwId) {
	load(dwId);
}

RMMessage::RMMessage() {
	_lpMessage = NULL;
	_nPeriods = 0;
	for (int i = 0; i < 256; i++)
		_lpPeriods[i] = 0;
}

RMMessage::~RMMessage() {
	if (_lpMessage)
		globalDestroy(_lpMessage);
}

void RMMessage::load(uint32 dwId) {
	_lpMessage = mpalQueryMessage(dwId);
	assert(_lpMessage != NULL);

	if (_lpMessage)
		parseMessage();
}

void RMMessage::parseMessage() {
	char *p;

	assert(_lpMessage != NULL);

	_nPeriods = 1;
	p = _lpPeriods[0] = _lpMessage;

	for (;;) {
		// Find the end of the current period
		while (*p != '\0')
			p++;

		// If there is another '0' at the end of the string, the end has been found
		p++;
		if (*p == '\0')
			break;

		// Otherwise there is another line, and remember it's start
		_lpPeriods[_nPeriods++] = p;
	}
}

bool RMMessage::isValid() {
	return _lpMessage != NULL;
}

int RMMessage::numPeriods() {
	return _nPeriods;
}

char *RMMessage::period(int num) {
	return _lpPeriods[num];
}

char *RMMessage::operator[](int num) {
	return _lpPeriods[num];
}

} // End of namespace Tony
