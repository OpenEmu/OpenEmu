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



#include "common/config-manager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse.h"
#ifdef ENABLE_HE
#include "scumm/he/intern_he.h"
#endif
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v6.h"
#include "scumm/scumm_v8.h"
#include "scumm/verbs.h"
#include "scumm/he/sound_he.h"

namespace Scumm {



#pragma mark -
#pragma mark --- "High level" message code ---
#pragma mark -


void ScummEngine::printString(int m, const byte *msg) {
	switch (m) {
	case 0:
		actorTalk(msg);
		break;
	case 1:
		drawString(1, msg);
		break;
	case 2:
		debugMessage(msg);
		break;
	case 3:
		showMessageDialog(msg);
		break;
	}
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::printString(int m, const byte *msg) {
	if (m == 4) {
		const StringTab &st = _string[m];
		enqueueText(msg, st.xpos, st.ypos, st.color, st.charset, st.center);
	} else {
		ScummEngine::printString(m, msg);
	}
}
#endif

void ScummEngine::debugMessage(const byte *msg) {
	byte buffer[500];
	convertMessageToString(msg, buffer, sizeof(buffer));

	if ((buffer[0] != 0xFF) && _debugMode) {
		debug(0, "DEBUG: %s", buffer);
		return;
	}

	if (buffer[0] == 0xFF && buffer[1] == 10) {
		uint32 a, b;
		int channel = 0;

		a = buffer[2] | (buffer[3] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
		b = buffer[10] | (buffer[11] << 8) | (buffer[14] << 16) | (buffer[15] << 24);

		// Sam and Max uses a caching system, printing empty messages
		// and setting VAR_V6_SOUNDMODE beforehand. See patch 609791.
		if (_game.id == GID_SAMNMAX)
			channel = VAR(VAR_V6_SOUNDMODE);

		 if (channel != 2)
			_sound->talkSound(a, b, 1, channel);
	}
}

void ScummEngine::showMessageDialog(const byte *msg) {
	// Original COMI used different code at this point.
	// Seemed to use blastText for the messages
	byte buf[500];

	convertMessageToString(msg, buf, sizeof(buf));

	if (_string[3].color == 0)
		_string[3].color = 4;

	InfoDialog dialog(this, (char *)buf);
	VAR(VAR_KEYPRESS) = runDialog(dialog);
}


#pragma mark -
#pragma mark --- V6 blast text queue code ---
#pragma mark -


void ScummEngine_v6::enqueueText(const byte *text, int x, int y, byte color, byte charset, bool center) {
	BlastText &bt = _blastTextQueue[_blastTextQueuePos++];
	assert(_blastTextQueuePos <= ARRAYSIZE(_blastTextQueue));

	convertMessageToString(text, bt.text, sizeof(bt.text));
	bt.xpos = x;
	bt.ypos = y;
	bt.color = color;
	bt.charset = charset;
	bt.center = center;
}

void ScummEngine_v6::drawBlastTexts() {
	byte *buf;
	int c;
	int i;

	for (i = 0; i < _blastTextQueuePos; i++) {

		buf = _blastTextQueue[i].text;

		_charset->_top = _blastTextQueue[i].ypos + _screenTop;
		_charset->_right = _screenWidth - 1;
		_charset->_center = _blastTextQueue[i].center;
		_charset->setColor(_blastTextQueue[i].color);
		_charset->_disableOffsX = _charset->_firstChar = true;
		_charset->setCurID(_blastTextQueue[i].charset);

		do {
			_charset->_left = _blastTextQueue[i].xpos;

			// Center text if necessary
			if (_charset->_center) {
				_charset->_left -= _charset->getStringWidth(0, buf) / 2;
				if (_charset->_left < 0)
					_charset->_left = 0;
			}

			do {
				c = *buf++;

				// FIXME: This is a workaround for bugs #864030 and #1399843:
				// In COMI, some text contains ASCII character 11 = 0xB. It's
				// not quite clear what it is good for; so for now we just ignore
				// it, which seems to match the original engine (BTW, traditionally,
				// this is a 'vertical tab').
				if (c == 0x0B)
					continue;

				// Some localizations may override colors
				// See credits in Chinese COMI
				if (_game.id == GID_CMI &&	_language == Common::ZH_TWN &&
				      c == '^' && (buf == _blastTextQueue[i].text + 1)) {
					if (*buf == 'c') {
						int color = buf[3] - '0' + 10 *(buf[2] - '0');
						_charset->setColor(color);

						buf += 4;
						c = *buf++;
					}
				}

				if (c != 0 && c != 0xFF && c != '\n') {
					if (c & 0x80 && _useCJKMode) {
						if (_language == Common::JA_JPN && !checkSJISCode(c)) {
							c = 0x20; //not in S-JIS
						} else {
							c += *buf++ * 256;
						}
					}
					_charset->printChar(c, true);
				}
			} while (c && c != '\n');

			_charset->_top += _charset->getFontHeight();
		} while (c);

		_blastTextQueue[i].rect = _charset->_str;
	}
}

void ScummEngine_v6::removeBlastTexts() {
	int i;

	for (i = 0; i < _blastTextQueuePos; i++) {
		restoreBackground(_blastTextQueue[i].rect);
	}
	_blastTextQueuePos = 0;
}


#pragma mark -
#pragma mark --- V7 subtitle queue code ---
#pragma mark -


#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::processSubtitleQueue() {
	for (int i = 0; i < _subtitleQueuePos; ++i) {
		SubtitleText *st = &_subtitleQueue[i];
		if (!st->actorSpeechMsg && (!ConfMan.getBool("subtitles") || VAR(VAR_VOICE_MODE) == 0))
			// no subtitles and there's a speech variant of the message, don't display the text
			continue;
		enqueueText(st->text, st->xpos, st->ypos, st->color, st->charset, false);
	}
}

void ScummEngine_v7::addSubtitleToQueue(const byte *text, const Common::Point &pos, byte color, byte charset) {
	if (text[0] && strcmp((const char *)text, " ") != 0) {
		assert(_subtitleQueuePos < ARRAYSIZE(_subtitleQueue));
		SubtitleText *st = &_subtitleQueue[_subtitleQueuePos];
		int i = 0;
		while (1) {
			st->text[i] = text[i];
			if (!text[i])
				break;
			++i;
		}
		st->xpos = pos.x;
		st->ypos = pos.y;
		st->color = color;
		st->charset = charset;
		st->actorSpeechMsg = _haveActorSpeechMsg;
		++_subtitleQueuePos;
	}
}

void ScummEngine_v7::clearSubtitleQueue() {
	memset(_subtitleQueue, 0, sizeof(_subtitleQueue));
	_subtitleQueuePos = 0;
}
#endif



#pragma mark -
#pragma mark --- Core message/subtitle code ---
#pragma mark -


bool ScummEngine::handleNextCharsetCode(Actor *a, int *code) {
	uint32 talk_sound_a = 0;
	uint32 talk_sound_b = 0;
	int color, frme, c = 0, oldy;
	bool endLoop = false;
	byte *buffer = _charsetBuffer + _charsetBufPos;
	while (!endLoop) {
		c = *buffer++;
		if (!(c == 0xFF || (_game.version <= 6 && c == 0xFE))) {
			break;
		}
		c = *buffer++;

		if (_newLineCharacter != 0 && c == _newLineCharacter) {
			c = 13;
			break;
		}

		switch (c) {
		case 1:
			c = 13; // new line
			endLoop = true;
			break;
		case 2:
			_haveMsg = 0;
			_keepText = true;
			endLoop = true;
			break;
		case 3:
			_haveMsg = (_game.version >= 7) ? 1 : 0xFF;
			_keepText = false;
			endLoop = true;
			break;
		case 8:
			// Ignore this code here. Occurs e.g. in MI2 when you
			// talk to the carpenter on scabb island. It works like
			// code 1 (=newline) in verb texts, but is ignored in
			// spoken text (i.e. here). Used for very long verb
			// sentences.
			break;
		case 9:
			frme = buffer[0] | (buffer[1] << 8);
			buffer += 2;
			if (a)
				a->startAnimActor(frme);
			break;
		case 10:
			// Note the similarity to the code in debugMessage()
			talk_sound_a = buffer[0] | (buffer[1] << 8) | (buffer[4] << 16) | (buffer[5] << 24);
			talk_sound_b = buffer[8] | (buffer[9] << 8) | (buffer[12] << 16) | (buffer[13] << 24);
			buffer += 14;
			if (_game.heversion >= 60) {
				((SoundHE *)_sound)->startHETalkSound(talk_sound_a);
			} else {
				_sound->talkSound(talk_sound_a, talk_sound_b, 2);
			}
			_haveActorSpeechMsg = false;
			break;
		case 12:
			color = buffer[0] | (buffer[1] << 8);
			buffer += 2;
			if (color == 0xFF)
				_charset->setColor(_charsetColor);
			else
				_charset->setColor(color);
			break;
		case 13:
			debug(0, "handleNextCharsetCode: Unknown opcode 13 %d", READ_LE_UINT16(buffer));
			buffer += 2;
			break;
		case 14:
			oldy = _charset->getFontHeight();
			_charset->setCurID(*buffer++);
			buffer += 2;
			memcpy(_charsetColorMap, _charsetData[_charset->getCurID()], 4);
			_nextTop -= _charset->getFontHeight() - oldy;
			break;
		default:
			error("handleNextCharsetCode: invalid code %d", c);
		}
	}
	_charsetBufPos = buffer - _charsetBuffer;
	*code = c;
	return (c != 2 && c != 3);
}

#ifdef ENABLE_HE
bool ScummEngine_v72he::handleNextCharsetCode(Actor *a, int *code) {
	const int charsetCode = (_game.heversion >= 80) ? 127 : 64;
	uint32 talk_sound_a = 0;
	//uint32 talk_sound_b = 0;
	int i, c = 0;
	char value[32];
	bool endLoop = false;
	bool endText = false;
	byte *buffer = _charsetBuffer + _charsetBufPos;
	while (!endLoop) {
		c = *buffer++;
		if (c != charsetCode) {
			break;
		}
		c = *buffer++;
		switch (c) {
		case 84:
			i = 0;
			c = *buffer++;
			while (c != 44) {
				value[i] = c;
				c = *buffer++;
				i++;
			}
			value[i] = 0;
			talk_sound_a = atoi(value);
			i = 0;
			c = *buffer++;
			while (c != charsetCode) {
				value[i] = c;
				c = *buffer++;
				i++;
			}
			value[i] = 0;
			//talk_sound_b = atoi(value);
			((SoundHE *)_sound)->startHETalkSound(talk_sound_a);
			break;
		case 104:
			_haveMsg = 0;
			_keepText = true;
			endLoop = endText = true;
			break;
		case 110:
			c = 13; // new line
			endLoop = true;
			break;
		case 116:
			i = 0;
			memset(value, 0, sizeof(value));
			c = *buffer++;
			while (c != charsetCode) {
				value[i] = c;
				c = *buffer++;
				i++;
			}
			value[i] = 0;
			talk_sound_a = atoi(value);
			//talk_sound_b = 0;
			((SoundHE *)_sound)->startHETalkSound(talk_sound_a);
			break;
		case 119:
			_haveMsg = 0xFF;
			_keepText = false;
			endLoop = endText = true;
			break;
		default:
			error("handleNextCharsetCode: invalid code %d", c);
		}
	}
	_charsetBufPos = buffer - _charsetBuffer;
	*code = c;
	return (endText == 0);
}
#endif

bool ScummEngine::newLine() {
	_nextLeft = _string[0].xpos;
	if (_charset->_center) {
		_nextLeft -= _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos) / 2;
		if (_nextLeft < 0)
			_nextLeft = _game.version >= 6 ? _string[0].xpos : 0;
	}

	if (_game.version == 0) {
		return false;
	} else if (!(_game.platform == Common::kPlatformFMTowns) && _string[0].height) {
		_nextTop += _string[0].height;
	} else {
		bool useCJK = _useCJKMode;
		// SCUMM5 FM-Towns doesn't use the height of the ROM font here.
		if (_game.platform == Common::kPlatformFMTowns && _game.version == 5)
			_useCJKMode = false;
		_nextTop += _charset->getFontHeight();
		_useCJKMode = useCJK;
	}
	if (_game.version > 3) {
		// FIXME: is this really needed?
		_charset->_disableOffsX = true;
	}
	return true;
}

void ScummEngine::CHARSET_1() {
	Actor *a;
#ifdef ENABLE_SCUMM_7_8
	byte subtitleBuffer[200];
	byte *subtitleLine = subtitleBuffer;
	Common::Point subtitlePos;

	if (_game.version >= 7) {
		((ScummEngine_v7 *)this)->processSubtitleQueue();
	}
#endif

	if (_game.heversion >= 70 && _haveMsg == 3) {
		stopTalk();
		return;
	}

	if (!_haveMsg)
		return;

	if (_game.version >= 4 && _game.version <= 6) {
		// Do nothing while the camera is moving
		if ((camera._dest.x / 8) != (camera._cur.x / 8) || camera._cur.x != camera._last.x)
			return;
	}

	a = NULL;
	if (getTalkingActor() != 0xFF)
		a = derefActorSafe(getTalkingActor(), "CHARSET_1");

	if (a && _string[0].overhead) {
		int s;

		_string[0].xpos = a->getPos().x - _virtscr[kMainVirtScreen].xstart;
		_string[0].ypos = a->getPos().y - a->getElevation() - _screenTop;

		if (_game.version <= 5) {

			if (VAR(VAR_V5_TALK_STRING_Y) < 0) {
				s = (a->_scaley * (int)VAR(VAR_V5_TALK_STRING_Y)) / 0xFF;
				_string[0].ypos += (int)(((VAR(VAR_V5_TALK_STRING_Y) - s) / 2) + s);
			} else {
				_string[0].ypos = (int)VAR(VAR_V5_TALK_STRING_Y);
			}

		} else {
			s = a->_scalex * a->_talkPosX / 0xFF;
			_string[0].xpos += ((a->_talkPosX - s) / 2) + s;

			s = a->_scaley * a->_talkPosY / 0xFF;
			_string[0].ypos += ((a->_talkPosY - s) / 2) + s;

			if (_string[0].ypos > _screenHeight - 40)
				_string[0].ypos = _screenHeight - 40;
		}

		if (_string[0].ypos < 1)
			_string[0].ypos = 1;

		if (_string[0].xpos < 80)
			_string[0].xpos = 80;
		if (_string[0].xpos > _screenWidth - 80)
			_string[0].xpos = _screenWidth - 80;
	}

	_charset->_top = _string[0].ypos + _screenTop;
	_charset->_startLeft = _charset->_left = _string[0].xpos;
	_charset->_right = _string[0].right;
	_charset->_center = _string[0].center;
	_charset->setColor(_charsetColor);

	if (a && a->_charset)
		_charset->setCurID(a->_charset);
	else
		_charset->setCurID(_string[0].charset);

	if (_game.version >= 5)
		memcpy(_charsetColorMap, _charsetData[_charset->getCurID()], 4);

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_keepText && _game.platform == Common::kPlatformFMTowns)
		memcpy(&_charset->_str, &_curStringRect, sizeof(Common::Rect));
#endif

	if (_talkDelay)
		return;

	if ((_game.version <= 6 && _haveMsg == 1) ||
	    (_game.version == 7 && _haveMsg != 1)) {

		if (_game.heversion >= 60) {
			if (_sound->isSoundRunning(1) == 0)
				stopTalk();
		} else {
			if ((_sound->_sfxMode & 2) == 0)
				stopTalk();
		}
		return;
	}

	if (a && !_string[0].no_talk_anim) {
		a->runActorTalkScript(a->_talkStartFrame);
		_useTalkAnims = true;
	}

	_talkDelay = (VAR_DEFAULT_TALK_DELAY != 0xFF) ? VAR(VAR_DEFAULT_TALK_DELAY) : 60;

	if (!_keepText) {
		if (_game.version >= 7) {
#ifdef ENABLE_SCUMM_7_8
			((ScummEngine_v7 *)this)->clearSubtitleQueue();
			_nextLeft = _string[0].xpos;
			_nextTop = _string[0].ypos + _screenTop;
#endif
		} else {
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
			if (_game.platform == Common::kPlatformFMTowns)
				towns_restoreCharsetBg();
			else
#endif
				restoreCharsetBg();
		}
	}

	if (_game.version > 3) {
		int maxwidth = _charset->_right - _string[0].xpos - 1;
		if (_charset->_center) {
			if (maxwidth > _nextLeft)
				maxwidth = _nextLeft;
			maxwidth *= 2;
		}

		_charset->addLinebreaks(0, _charsetBuffer + _charsetBufPos, 0, maxwidth);
	}

	if (_charset->_center) {
		_nextLeft -= _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos) / 2;
		if (_nextLeft < 0)
			_nextLeft = _game.version >= 6 ? _string[0].xpos : 0;
	}

	_charset->_disableOffsX = _charset->_firstChar = !_keepText;

	int c = 0;
	while (handleNextCharsetCode(a, &c)) {
		if (c == 0) {
			// End of text reached, set _haveMsg accordingly
			_haveMsg = (_game.version >= 7) ? 2 : 1;
			_keepText = false;
			break;
		}

		if (c == 13) {
#ifdef ENABLE_SCUMM_7_8
			if (_game.version >= 7 && subtitleLine != subtitleBuffer) {
				((ScummEngine_v7 *)this)->addSubtitleToQueue(subtitleBuffer, subtitlePos, _charsetColor, _charset->getCurID());
				subtitleLine = subtitleBuffer;
			}
#endif
			if (!newLine())
				break;
			continue;
		}

		// Handle line overflow for V3. See also bug #1306269.
		if (_game.version == 3 && _nextLeft >= _screenWidth) {
			_nextLeft = _screenWidth;
		}
		// Handle line breaks for V1-V2
		if (_game.version <= 2 && _nextLeft >= _screenWidth) {
			if (!newLine())
				break;	// FIXME: Is this necessary? Only would be relevant for v0 games
		}

		_charset->_left = _nextLeft;
		_charset->_top = _nextTop;

		if (_game.version >= 7) {
#ifdef ENABLE_SCUMM_7_8
			if (subtitleLine == subtitleBuffer) {
				subtitlePos.x = _charset->_left;
				// BlastText position is relative to the top of the screen, adjust y-coordinate
				subtitlePos.y = _charset->_top - _screenTop;
			}
			*subtitleLine++ = c;
			*subtitleLine = '\0';
#endif
		} else {
			if (c & 0x80 && _useCJKMode) {
				if (checkSJISCode(c)) {
					byte *buffer = _charsetBuffer + _charsetBufPos;
					c += *buffer++ * 256; //LE
					_charsetBufPos = buffer - _charsetBuffer;
				}
			}
			if (_game.version <= 3) {
				_charset->printChar(c, false);
			} else {
				if (_game.features & GF_16BIT_COLOR) {
					// HE games which use sprites for subtitles
				} else if (_game.heversion >= 60 && !ConfMan.getBool("subtitles") && _sound->isSoundRunning(1)) {
					// Special case for HE games
				} else if (_game.id == GID_LOOM && !ConfMan.getBool("subtitles") && (_sound->pollCD())) {
					// Special case for Loom (CD), since it only uses CD audio.for sound
				} else if (!ConfMan.getBool("subtitles") && (!_haveActorSpeechMsg || _mixer->isSoundHandleActive(_sound->_talkChannelHandle))) {
					// Subtitles are turned off, and there is a voice version
					// of this message -> don't print it.
				} else {
					_charset->printChar(c, false);
				}
			}
			_nextLeft = _charset->_left;
			_nextTop = _charset->_top;
		}

		if (_game.version <= 2) {
			_talkDelay += _defaultTalkDelay;
			VAR(VAR_CHARCOUNT)++;
		} else {
			_talkDelay += (int)VAR(VAR_CHARINC);
		}
	}

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_game.platform == Common::kPlatformFMTowns && (c == 0 || c == 2 || c == 3))
		memcpy(&_curStringRect, &_charset->_str, sizeof(Common::Rect));
#endif

#ifdef ENABLE_SCUMM_7_8
	if (_game.version >= 7 && subtitleLine != subtitleBuffer) {
		((ScummEngine_v7 *)this)->addSubtitleToQueue(subtitleBuffer, subtitlePos, _charsetColor, _charset->getCurID());
	}
#endif
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::CHARSET_1() {
	if (_game.id == GID_FT) {
		ScummEngine::CHARSET_1();
		return;
	}

	byte subtitleBuffer[2048];
	byte *subtitleLine = subtitleBuffer;
	Common::Point subtitlePos;

	processSubtitleQueue();

	if (!_haveMsg)
		return;

	Actor *a = NULL;
	if (getTalkingActor() != 0xFF)
		a = derefActorSafe(getTalkingActor(), "CHARSET_1");

	StringTab saveStr = _string[0];
	if (a && _string[0].overhead) {
		int s;

		_string[0].xpos = a->getPos().x - _virtscr[kMainVirtScreen].xstart;
		s = a->_scalex * a->_talkPosX / 255;
		_string[0].xpos += (a->_talkPosX - s) / 2 + s;

		_string[0].ypos = a->getPos().y - a->getElevation() - _screenTop;
		s = a->_scaley * a->_talkPosY / 255;
		_string[0].ypos += (a->_talkPosY - s) / 2 + s;
	}

	_charset->setColor(_charsetColor);

	if (a && a->_charset)
		_charset->setCurID(a->_charset);
	else
		_charset->setCurID(_string[0].charset);

	if (_talkDelay)
		return;

	if (VAR(VAR_HAVE_MSG)) {
		if ((_sound->_sfxMode & 2) == 0) {
			stopTalk();
		}
		return;
	}

	if (a && !_string[0].no_talk_anim) {
		a->runActorTalkScript(a->_talkStartFrame);
	}

	if (!_keepText) {
		clearSubtitleQueue();
		_nextLeft = _string[0].xpos;
		_nextTop = _string[0].ypos + _screenTop;
	}

	_charset->_disableOffsX = _charset->_firstChar = !_keepText;

	_talkDelay = VAR(VAR_DEFAULT_TALK_DELAY);
	for (int i = _charsetBufPos; _charsetBuffer[i]; ++i) {
		_talkDelay += VAR(VAR_CHARINC);
	}

	if (_string[0].wrapping) {
		_charset->addLinebreaks(0, _charsetBuffer, _charsetBufPos, _screenWidth - 20);

		struct { int pos, w; } substring[10];
		int count = 0;
		int maxLineWidth = 0;
		int lastPos = 0;
		int code = 0;
		while (handleNextCharsetCode(a, &code)) {
			if (code == 13 || code == 0) {
				*subtitleLine++ = '\0';
				assert(count < 10);
				substring[count].w = _charset->getStringWidth(0, subtitleBuffer + lastPos);
				if (maxLineWidth < substring[count].w) {
					maxLineWidth = substring[count].w;
				}
				substring[count].pos = lastPos;
				++count;
				lastPos = subtitleLine - subtitleBuffer;
			} else {
				*subtitleLine++ = code;
				*subtitleLine = '\0';
			}
			if (code == 0) {
				break;
			}
		}

		int h = count * _charset->getFontHeight();
		h += _charset->getFontHeight() / 2;
		subtitlePos.y = _string[0].ypos;
		if (subtitlePos.y + h > _screenHeight - 10) {
			subtitlePos.y = _screenHeight - 10 - h;
		}
		if (subtitlePos.y < 10) {
			subtitlePos.y = 10;
		}

		for (int i = 0; i < count; ++i) {
			subtitlePos.x = _string[0].xpos;
			if (_string[0].center) {
				if (subtitlePos.x + maxLineWidth / 2 > _screenWidth - 10) {
					subtitlePos.x = _screenWidth - 10 - maxLineWidth / 2;
				}
				if (subtitlePos.x - maxLineWidth / 2 < 10) {
					subtitlePos.x = 10 + maxLineWidth / 2;
				}
				subtitlePos.x -= substring[i].w / 2;
			} else {
				if (subtitlePos.x + maxLineWidth > _screenWidth - 10) {
					subtitlePos.x = _screenWidth - 10 - maxLineWidth;
				}
				if (subtitlePos.x - maxLineWidth < 10) {
					subtitlePos.x = 10;
				}
			}
			if (subtitlePos.y < _screenHeight - 10) {
				addSubtitleToQueue(subtitleBuffer + substring[i].pos, subtitlePos, _charsetColor, _charset->getCurID());
			}
			subtitlePos.y += _charset->getFontHeight();
		}
	} else {
		int code = 0;
		subtitlePos.y = _string[0].ypos;
		if (subtitlePos.y < 10) {
			subtitlePos.y = 10;
		}
		while (handleNextCharsetCode(a, &code)) {
			if (code == 13 || code == 0) {
				subtitlePos.x = _string[0].xpos;
				if (_string[0].center) {
					subtitlePos.x -= _charset->getStringWidth(0, subtitleBuffer) / 2;
				}
				if (subtitlePos.x < 10) {
					subtitlePos.x = 10;
				}
				if (subtitlePos.y < _screenHeight - 10) {
					addSubtitleToQueue(subtitleBuffer, subtitlePos, _charsetColor, _charset->getCurID());
					subtitlePos.y += _charset->getFontHeight();
				}
				subtitleLine = subtitleBuffer;
			} else {
				*subtitleLine++ = code;
			}
			*subtitleLine = '\0';
			if (code == 0) {
				break;
			}
		}
	}
	_haveMsg = (_game.version == 8) ? 2 : 1;
	_keepText = false;
	_string[0] = saveStr;
}
#endif

void ScummEngine::drawString(int a, const byte *msg) {
	byte buf[270];
	byte *space;
	int i, c;
	byte fontHeight = 0;
	uint color;
	int code = (_game.heversion >= 80) ? 127 : 64;

	// drawString is not used in SCUMM v7 and v8
	assert(_game.version < 7);

	convertMessageToString(msg, buf, sizeof(buf));

	_charset->_top = _string[a].ypos + _screenTop;
	_charset->_startLeft = _charset->_left = _string[a].xpos;
	_charset->_right = _string[a].right;
	_charset->_center = _string[a].center;
	_charset->setColor(_string[a].color);
	_charset->_disableOffsX = _charset->_firstChar = true;
	_charset->setCurID(_string[a].charset);

	// HACK: Correct positions of text in books in Indy3 Mac.
	// See also patch #1851568.
	if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformMacintosh && a == 1) {
		if (_currentRoom == 75) {
			// Grail Diary Page 1 (Library)
			if (_charset->_startLeft < 160)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 22;
			else if (_charset->_startLeft < 200)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 10;
		} else if (_currentRoom == 90) {
			// Grail Diary Page 2 (Catacombs - Engravings)
			if (_charset->_startLeft < 160)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 21;
			else if (_charset->_startLeft < 200)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 15;
		} else if (_currentRoom == 69) {
			// Grail Diary Page 3 (Catacombs - Music)
			if (_charset->_startLeft < 160)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 15;
			else if (_charset->_startLeft < 200)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 10;
		} else if (_currentRoom == 74) {
			// Biplane Manual
			_charset->_startLeft = _charset->_left = _string[a].xpos - 35;
		}
	}

	if (_game.version >= 5)
		memcpy(_charsetColorMap, _charsetData[_charset->getCurID()], 4);

	fontHeight = _charset->getFontHeight();

	if (_game.version >= 4) {
		// trim from the right
		byte *tmp = buf;
		space = NULL;
		while (*tmp) {
			if (*tmp == ' ') {
				if (!space)
					space = tmp;
			} else {
				space = NULL;
			}
			tmp++;
		}
		if (space)
			*space = '\0';
	}

	if (_charset->_center) {
		_charset->_left -= _charset->getStringWidth(a, buf) / 2;
	}

	if (!buf[0]) {
		if (_game.version >= 5) {
			buf[0] = ' ';
			buf[1] = 0;
		} else {
			_charset->_str.left = _charset->_left;
			_charset->_str.top = _charset->_top;
			_charset->_str.right = _charset->_left;
			_charset->_str.bottom = _charset->_top;
		}
	}

	for (i = 0; (c = buf[i++]) != 0;) {
		if (_game.heversion >= 72 && c == code) {
			c = buf[i++];
			switch (c) {
			case 110:
				if (_charset->_center) {
					_charset->_left = _charset->_startLeft - _charset->getStringWidth(a, buf + i);
				} else {
					_charset->_left = _charset->_startLeft;
				}
				_charset->_top += fontHeight;
				break;
			}
		} else if ((c == 0xFF || (_game.version <= 6 && c == 0xFE)) && (_game.heversion <= 71)) {
			c = buf[i++];
			switch (c) {
			case 9:
			case 10:
			case 13:
			case 14:
				i += 2;
				break;
			case 1:
			case 8:
				if (_charset->_center) {
					_charset->_left = _charset->_startLeft - _charset->getStringWidth(a, buf + i);
				} else {
					_charset->_left = _charset->_startLeft;
				}
				if (!(_game.platform == Common::kPlatformFMTowns) && _string[0].height) {
					_nextTop += _string[0].height;
				} else {
					_charset->_top += fontHeight;
				}
				break;
			case 12:
				color = buf[i] + (buf[i + 1] << 8);
				i += 2;
				if (color == 0xFF)
					_charset->setColor(_string[a].color);
				else
					_charset->setColor(color);
				break;
			}
		} else {
			if (a == 1 && _game.version >= 6) {
				// FIXME: The following code is a bit nasty. It is used for the
				// Highway surfing game in Sam&Max; there, _blitAlso is set to
				// true when writing the highscore numbers. It is also in DOTT
				// for parts the intro and for drawing newspaper headlines. It
				// is also used for scores in bowling mini game in fbear and
				// for names in load/save screen of all HE games. Maybe it is
				// also being used in other places.
				//
				// A better name for _blitAlso might be _imprintOnBackground

				if (_string[a].no_talk_anim == false) {
					//debug(0, "Would have set _charset->_blitAlso = true (wanted to print '%c' = %d)", c, c);
					_charset->_blitAlso = true;
				}
			}
			if (c & 0x80 && _useCJKMode) {
				if (checkSJISCode(c))
					c += buf[i++] * 256;
			}
			_charset->printChar(c, true);
			_charset->_blitAlso = false;
		}
	}

	if (a == 0) {
		_nextLeft = _charset->_left;
		_nextTop = _charset->_top;
	}

	_string[a].xpos = _charset->_str.right;

	if (_game.heversion >= 60) {
		_string[a]._default.xpos = _string[a].xpos;
		_string[a]._default.ypos = _string[a].ypos;
	}
}

int ScummEngine::convertMessageToString(const byte *msg, byte *dst, int dstSize) {
	uint num = 0;
	uint32 val;
	byte chr;
	byte lastChr = 0;
	const byte *src;
	byte *end;
	byte transBuf[384];

	assert(dst);
	end = dst + dstSize;

	if (msg == NULL) {
		debug(0, "Bad message in convertMessageToString, ignoring");
		return 0;
	}

	if (_game.version >= 7) {
		translateText(msg, transBuf);
		src = transBuf;
	} else {
		src = msg;
	}

	num = 0;

	while (1) {
		chr = src[num++];
		if (chr == 0)
			break;

		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
			// Code for TM character
			if (chr == 0x0F && src[num] == 0x20) {
				*dst++ = 0x5D;
				*dst++ = 0x5E;
				continue;
			// Code for (C) character
			} else if (chr == 0x1C && src[num] == 0x20) {
				*dst++ = 0x3E;
				*dst++ = 0x2A;
				continue;
			// Code for " character
			} else if (chr == 0x19) {
				*dst++ = 0x2F;
				continue;
			}
		}

		if (chr == 0xFF) {
			chr = src[num++];

			// WORKAROUND for bug #985948, a script bug in Indy3. Apparently,
			// a german 'sz' was encoded incorrectly as 0xFF2E. We replace
			// this by the correct encoding here. See also ScummEngine::resStrLen().
			if (_game.id == GID_INDY3 && chr == 0x2E) {
				*dst++ = 0xE1;
				continue;
			}

			// WORKAROUND for bug #1514457: Yet another script bug in Indy3.
			// Once more a german 'sz' was encoded incorrectly, but this time
			// they simply encoded it as 0xFF instead of 0xE1. Happens twice
			// in script 71.
			if (_game.id == GID_INDY3 && chr == 0x20 && vm.slot[_currentScript].number == 71) {
				num--;
				*dst++ = 0xE1;
				continue;
			}

			if (chr == 1 || chr == 2 || chr == 3 || chr == 8) {
				// Simply copy these special codes
				*dst++ = 0xFF;
				*dst++ = chr;
			} else {
				val = (_game.version == 8) ? READ_LE_UINT32(src + num) : READ_LE_UINT16(src + num);
				switch (chr) {
				case 4:
					dst += convertIntMessage(dst, end - dst, val);
					break;
				case 5:
					dst += convertVerbMessage(dst, end - dst, val);
					break;
				case 6:
					dst += convertNameMessage(dst, end - dst, val);
					break;
				case 7:
					dst += convertStringMessage(dst, end - dst, val);
					break;
				case 9:
				case 10:
				case 12:
				case 13:
				case 14:
					// Simply copy these special codes
					*dst++ = 0xFF;
					*dst++ = chr;
					*dst++ = src[num+0];
					*dst++ = src[num+1];
					if (_game.version == 8) {
						*dst++ = src[num+2];
						*dst++ = src[num+3];
					}
					break;
				default:
					error("convertMessageToString(): string escape sequence %d unknown", chr);
				}
				num += (_game.version == 8) ? 4 : 2;
			}
		} else {
			if ((chr != '@') || (_game.id == GID_CMI && _language == Common::ZH_TWN) ||
				(_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && _language == Common::JA_JPN) ||
				(_game.platform == Common::kPlatformFMTowns && _language == Common::JA_JPN && checkSJISCode(lastChr))) {
				*dst++ = chr;
			}
			lastChr = chr;
		}

		// Check for a buffer overflow
		if (dst >= end)
			error("convertMessageToString: buffer overflow");
	}

	// WORKAROUND: Russian The Dig pads messages with 03. No idea why
	// it does not work as is with our rendering code, thus fixing it
	// with a workaround.
	if (_game.id == GID_DIG) {
		while (*(dst - 1) == 0x03)
			dst--;
	}
	*dst = 0;

	return dstSize - (end - dst);
}

#ifdef ENABLE_HE
int ScummEngine_v72he::convertMessageToString(const byte *msg, byte *dst, int dstSize) {
	uint num = 0;
	byte chr;
	const byte *src;
	byte *end;

	assert(dst);
	end = dst + dstSize;

	if (msg == NULL) {
		debug(0, "Bad message in convertMessageToString, ignoring");
		return 0;
	}

	src = msg;
	num = 0;

	while (1) {
		chr = src[num++];
		if (_game.heversion >= 80 && src[num - 1] == '(' && (src[num] == 'p' || src[num] == 'P')) {
			// Filter out the following prefixes in subtitles
			// (pickup4)
			// (PU1)
			// (PU2)
			while (src[num++] != ')')
				;
			continue;
		}
		if ((_game.features & GF_HE_LOCALIZED) && chr == '[') {
			while (src[num++] != ']')
				;
			continue;
		}

		if (chr == 0)
			break;

		*dst++ = chr;

		// Check for a buffer overflow
		if (dst >= end)
			error("convertMessageToString: buffer overflow");
	}
	*dst = 0;

	return dstSize - (end - dst);
}
#endif

int ScummEngine::convertIntMessage(byte *dst, int dstSize, int var) {
	int num;

	num = readVar(var);
	return snprintf((char *)dst, dstSize, "%d", num);
}

int ScummEngine::convertVerbMessage(byte *dst, int dstSize, int var) {
	int num, k;

	num = readVar(var);
	if (num) {
		for (k = 1; k < _numVerbs; k++) {
			// Fix ZAK FM-TOWNS bug #1013617 by emulating exact (inconsistant?) behavior of the original code
			if (num == _verbs[k].verbid && !_verbs[k].type && (!_verbs[k].saveid || (_game.version == 3 && _game.platform == Common::kPlatformFMTowns))) {
				const byte *ptr = getResourceAddress(rtVerb, k);
				return convertMessageToString(ptr, dst, dstSize);
			}
		}
	}
	return 0;
}

int ScummEngine::convertNameMessage(byte *dst, int dstSize, int var) {
	int num;

	num = readVar(var);
	if (num) {
		const byte *ptr = getObjOrActorName(num);
		if (ptr) {
			return convertMessageToString(ptr, dst, dstSize);
		}
	}
	return 0;
}

int ScummEngine::convertStringMessage(byte *dst, int dstSize, int var) {
	const byte *ptr;

	if (_game.version <= 2) {
		byte chr;
		int i = 0;
		while ((chr = (byte)_scummVars[var++])) {
			if (chr != '@') {
				*dst++ = chr;
				i++;
			}
		}

		return i;
	}

	if (_game.version == 3 || (_game.version >= 6 && _game.heversion < 72))
		var = readVar(var);

	if (var) {
		ptr = getStringAddress(var);
		if (ptr) {
			return convertMessageToString(ptr, dst, dstSize);
		}
	}
	return 0;
}


#pragma mark -
#pragma mark --- Charset initialisation ---
#pragma mark -


#ifdef ENABLE_HE
void ScummEngine_v80he::initCharset(int charsetno) {
	ScummEngine::initCharset(charsetno);
	VAR(VAR_CURRENT_CHARSET) = charsetno;
}
#endif

void ScummEngine::initCharset(int charsetno) {
	if (_game.id == GID_FT) {
		if (!_res->isResourceLoaded(rtCharset, charsetno))
			loadCharset(charsetno);
	} else {
		if (!getResourceAddress(rtCharset, charsetno))
			loadCharset(charsetno);
	}

	_string[0]._default.charset = charsetno;
	_string[1]._default.charset = charsetno;

	memcpy(_charsetColorMap, _charsetData[charsetno], sizeof(_charsetColorMap));
}


#pragma mark -
#pragma mark --- Translation/localization code ---
#pragma mark -


#ifdef ENABLE_SCUMM_7_8
static int indexCompare(const void *p1, const void *p2) {
	const ScummEngine_v7::LangIndexNode *i1 = (const ScummEngine_v7::LangIndexNode *) p1;
	const ScummEngine_v7::LangIndexNode *i2 = (const ScummEngine_v7::LangIndexNode *) p2;

	return strcmp(i1->tag, i2->tag);
}

// Create an index of the language file.
void ScummEngine_v7::loadLanguageBundle() {
	ScummFile file;
	int32 size;

	// if game is manually set to English, don't try to load localized text
	if ((_language == Common::EN_ANY) || (_language == Common::EN_USA) || (_language == Common::EN_GRB)) {
		warning("Language file is forced to be ignored");

		_existLanguageFile = false;
		return;
	}

	if (_game.id == GID_DIG) {
		openFile(file, "language.bnd");
	} else if (_game.id == GID_CMI) {
		openFile(file, "language.tab");
	} else {
		return;
	}
	if (file.isOpen() == false) {
		_existLanguageFile = false;
		return;
	}

	_existLanguageFile = true;

	size = file.size();
	_languageBuffer = (char *)calloc(1, size+1);
	file.read(_languageBuffer, size);
	file.close();

	int32 i;
	char *ptr = _languageBuffer;

	// Count the number of lines in the language file.
	for (_languageIndexSize = 0; ; _languageIndexSize++) {
		ptr = strpbrk(ptr, "\n\r");
		if (ptr == NULL)
			break;
		while (*ptr == '\n' || *ptr == '\r')
			ptr++;
	}

	// Fill the language file index. This is just an array of
	// tags and offsets. I did consider using a balanced tree
	// instead, but the extra overhead in the node structure would
	// easily have doubled the memory consumption of the index.
	// And anyway, using qsort + bsearch gives us the exact same
	// O(log(n)) access time anyway ;-).

	_languageIndex = (LangIndexNode *)calloc(_languageIndexSize, sizeof(LangIndexNode));

	ptr = _languageBuffer;

	if (_game.id == GID_DIG) {
		int lineCount = _languageIndexSize;
		const char *baseTag = "";
		byte enc = 0;	// Initially assume the language file is not encoded

		// We'll determine the real index size as we go.
		_languageIndexSize = 0;
		for (i = 0; i < lineCount; i++) {
			if (*ptr == '!') {
				// Don't know what a line with '!' means, just ignore it
			} else if (*ptr == 'h') {
				// File contains Korean text (Hangul). just ignore it
			} else if (*ptr == 'j') {
				// File contains Japanese text. just ignore it
			} else if (*ptr == 'c') {
				// File contains Chinese text. just ignore it
			} else if (*ptr == 'e') {
				// File is encoded!
				enc = 0x13;
			} else if (*ptr == '@') {
				// A new 'base tag'
				baseTag = ptr + 1;
			} else if (*ptr == '#') {
				// Number of subtags following a given basetag. We don't need that
				// information so we just skip it
			} else if (Common::isDigit(*ptr)) {
				int idx = 0;
				// A number (up to three digits)...
				while (Common::isDigit(*ptr)) {
					idx = idx * 10 + (*ptr - '0');
					ptr++;
				}

				// ...followed by a slash...
				assert(*ptr == '/');
				ptr++;

				// ...and then the translated message, possibly encoded
				_languageIndex[_languageIndexSize].offset = ptr - _languageBuffer;

				// Decode string if necessary.
				if (enc) {
					while (*ptr != '\n' && *ptr != '\r')
						*ptr++ ^= enc;
				}

				// The tag is the basetag, followed by a dot and then the index
				sprintf(_languageIndex[_languageIndexSize].tag, "%s.%03d", baseTag, idx);

				// That was another index entry
				_languageIndexSize++;
			} else {
				error("Unknown language.bnd entry found: '%s'", ptr);
			}

			// Skip over newlines (and turn them into null bytes)
			ptr = strpbrk(ptr, "\n\r");
			if (ptr == NULL)
				break;
			while (*ptr == '\n' || *ptr == '\r')
				*ptr++ = 0;
		}
	} else {
		for (i = 0; i < _languageIndexSize; i++) {
			// First 8 chars in the line give the string ID / 'tag'
			int j;
			for (j = 0; j < 8 && !Common::isSpace(*ptr); j++, ptr++)
				_languageIndex[i].tag[j] = toupper(*ptr);
			_languageIndex[i].tag[j] = 0;

			// After that follows a single space which we skip
			assert(Common::isSpace(*ptr));
			ptr++;

			// Then comes the translated string: we record an offset to that.
			_languageIndex[i].offset = ptr - _languageBuffer;

			// Skip over newlines (and turn them into null bytes)
			ptr = strpbrk(ptr, "\n\r");
			if (ptr == NULL)
				break;
			while (*ptr == '\n' || *ptr == '\r')
				*ptr++ = 0;

			// Convert '\n' code to a newline. See also bug #902415.
			char *src, *dst;
			src = dst = _languageBuffer + _languageIndex[i].offset;
			while (*src) {
				if (src[0] == '\\' && src[1] == 'n') {
					*dst++ = '\n';
					src += 2;
				} else {
					*dst++ = *src++;
				}
			}
			*dst = 0;
		}
	}

	// Sort the index nodes. We'll later use bsearch on it, which is just as efficient
	// as using a binary tree, speed wise.
	qsort(_languageIndex, _languageIndexSize, sizeof(LangIndexNode), indexCompare);
}

void ScummEngine_v7::playSpeech(const byte *ptr) {
	if (_game.id == GID_DIG && (ConfMan.getBool("speech_mute") || VAR(VAR_VOICE_MODE) == 2))
		return;

	if ((_game.id == GID_DIG || _game.id == GID_CMI) && ptr[0]) {
		char pointer[20];
		strcpy(pointer, (const char *)ptr);

		// Play speech
		if (!(_game.features & GF_DEMO) && (_game.id == GID_CMI)) // CMI demo does not have .IMX for voice
			strcat(pointer, ".IMX");

		_sound->stopTalkSound();
		_imuseDigital->stopSound(kTalkSoundID);
		_imuseDigital->startVoice(kTalkSoundID, pointer);
		_sound->talkSound(0, 0, 2);
	}
}

void ScummEngine_v7::translateText(const byte *text, byte *trans_buff) {
	LangIndexNode target;
	LangIndexNode *found = NULL;
	int i;

	trans_buff[0] = 0;
	_lastStringTag[0] = 0;

	if (_game.version >= 7 && text[0] == '/') {
		// Extract the string tag from the text: /..../
		for (i = 0; (i < 12) && (text[i + 1] != '/'); i++)
			_lastStringTag[i] = toupper(text[i + 1]);
		_lastStringTag[i] = 0;
	}

	// WORKAROUND for bug #1172655.
	if (_game.id == GID_DIG) {
		// Based on the second release of The Dig
		// Only applies to the subtitles and not speech
		if (!strcmp((const char *)text, "faint light"))
			text = (const byte *)"/NEW.007/faint light";
		else if (!strcmp((const char *)text, "glowing crystal"))
			text = (const byte *)"/NEW.008/glowing crystal";
		else if (!strcmp((const char *)text, "glowing crystals"))
			text = (const byte *)"/NEW.009/glowing crystals";
		else if (!strcmp((const char *)text, "pit"))
			text = (const byte *)"/NEW.010/pit";
		else if (!strcmp((const char *)text, "You wish."))
			text = (const byte *)"/NEW.011/You wish.";
		else if (!strcmp((const char *)text, "In your dreams."))
			text = (const byte *)"/NEW.012/In your dreams";
		else if (!strcmp((const char *)text, "left"))
			text = (const byte *)"/CATHPLAT.068/left";
		else if (!strcmp((const char *)text, "right"))
			text = (const byte *)"/CATHPLAT.070/right";
		else if (!strcmp((const char *)text, "top"))
			text = (const byte *)"/CATHPLAT.067/top";
		else if (!strcmp((const char *)text, "exit"))
			text = (const byte *)"/SKY.008/exit";
		else if (!strcmp((const char *)text, "unattached lens"))
			text = (const byte *)"/NEW.013/unattached lens";
		else if (!strcmp((const char *)text, "lens slot"))
			text = (const byte *)"/NEW.014/lens slot";
		else if (!strcmp((const char *)text, "Jonathon Jackson"))
			text = (const byte *)"Aram Gutowski";
		else if (!strcmp((const char *)text, "Brink"))
			text = (const byte *)"/CREVICE.049/Brink";
		else if (!strcmp((const char *)text, "Robbins"))
			text = (const byte *)"/NEST.061/Robbins";
	}


	if (_game.version >= 7 && text[0] == '/') {
		// Extract the string tag from the text: /..../
		for (i = 0; (i < 12) && (text[i + 1] != '/'); i++)
			target.tag[i] = toupper(text[i + 1]);
		target.tag[i] = 0;
		text += i + 2;

		// If a language file was loaded, try to find a translated version
		// by doing a lookup on the string tag.
		if (_existLanguageFile) {
			// HACK: These are used for the object line in COMI when
			// using one object on another. I don't know if the
			// text in the language file is a placeholder or if
			// we're supposed to use it, but at least in the
			// English version things will work so much better if
			// we can't find translations for these.

			if (*text && strcmp(target.tag, "PU_M001") != 0 && strcmp(target.tag, "PU_M002") != 0)
				found = (LangIndexNode *)bsearch(&target, _languageIndex, _languageIndexSize, sizeof(LangIndexNode), indexCompare);
		}
	}

	if (found != NULL) {
		strcpy((char *)trans_buff, _languageBuffer + found->offset);

		if ((_game.id == GID_DIG) && !(_game.features & GF_DEMO)) {
			// Replace any '%___' by the corresponding special codes in the source text
			const byte *src = text;
			char *dst = (char *)trans_buff;

			while ((dst = strstr(dst, "%___"))) {
				// Search for a special code in the message.
				while (*src && *src != 0xFF) {
					src++;
				}

				// Replace the %___ by the special code. Luckily, we can do
				// that in-place.
				if (*src == 0xFF) {
					memcpy(dst, src, 4);
					src += 4;
					dst += 4;
				} else
					break;
			}
		}
	} else {
		// Default: just copy the string
		memcpy(trans_buff, text, resStrLen(text) + 1);
	}
}

#endif

void ScummEngine::translateText(const byte *text, byte *trans_buff) {
	// Default: just copy the string
	memcpy(trans_buff, text, resStrLen(text) + 1);
}

} // End of namespace Scumm
