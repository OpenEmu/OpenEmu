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

#include "agi/agi.h"

#include "common/textconsole.h"

namespace Agi {

//
// Local implementation to avoid problems with strndup() used by
// gcc 3.2 Cygwin (see #635984)
//
static char *myStrndup(const char *src, int n) {
	char *tmp = strncpy((char *)malloc(n + 1), src, n);
	tmp[n] = 0;
	return tmp;
}

int AgiEngine::loadWords_v1(Common::File &f) {
	char str[64];
	int k;

	debug(0, "Loading dictionary");

	// Loop through alphabet, as words in the dictionary file are sorted by
	// first character
	f.seek(f.pos() + 26 * 2, SEEK_SET);
	do {
		// Read next word
		for (k = 0; k < (int)sizeof(str) - 1; k++) {
			str[k] = f.readByte();
			if (str[k] == 0 || (uint8)str[k] == 0xFF)
				break;
		}

		// And store it in our internal dictionary
		if (k > 0) {
			AgiWord *w = new AgiWord;
			w->word = myStrndup(str, k + 1);
			w->id = f.readUint16LE();
			_game.words[str[0] - 'a'].push_back(w);
			debug(3, "'%s' (%d)", w->word, w->id);
		}
	} while((uint8)str[0] != 0xFF);

	return errOK;
}

int AgiEngine::loadWords(const char *fname) {
	Common::File fp;

	if (!fp.open(fname)) {
		warning("loadWords: can't open %s", fname);
		return errOK; // err_BadFileOpen
	}
	debug(0, "Loading dictionary: %s", fname);

	// Loop through alphabet, as words in the dictionary file are sorted by
	// first character
	for (int i = 0; i < 26; i++) {
		fp.seek(i * 2, SEEK_SET);
		int offset = fp.readUint16BE();
		if (offset == 0)
			continue;
		fp.seek(offset, SEEK_SET);
		int k = fp.readByte();
		while (!fp.eos() && !fp.err()) {
			// Read next word
			char c, str[64];
			do {
				c = fp.readByte();
				str[k++] = (c ^ 0x7F) & 0x7F;
			} while (!(c & 0x80) && k < (int)sizeof(str) - 1);
			str[k] = 0;

			// And store it in our internal dictionary
			AgiWord *w = new AgiWord;
			w->word = myStrndup(str, k);
			w->id = fp.readUint16BE();
			_game.words[i].push_back(w);

			// Are there more words with an already known prefix?
			if (!(k = fp.readByte()))
				break;
		}
	}

	return errOK;
}

void AgiEngine::unloadWords() {
	for (int i = 0; i < 26; i++)
		_game.words[i].clear();
}

/**
 * Find a word in the dictionary
 * Uses an algorithm hopefully like the one Sierra used. Returns the ID
 * of the word and the length in flen. Returns -1 if not found.
 */
int AgiEngine::findWord(const char *word, int *flen) {
	int c;
	int result = -1;

	debugC(2, kDebugLevelScripts, "find_word(%s)", word);

	if (word[0] >= 'a' && word[0] <= 'z')
		c = word[0] - 'a';
	else
		return -1;

	*flen = 0;
	Common::Array<AgiWord *> &a = _game.words[c];

	for (int i = 0; i < (int)a.size(); i++) {
		int wlen = strlen(a[i]->word);
		// Keep looking till we find the word itself, or the whole phrase.
		// Try to find the best match (i.e. the longest matching phrase).
		if (!strncmp(a[i]->word, word, wlen) && (word[wlen] == 0 || word[wlen] == 0x20) && wlen >= *flen) {
			*flen = wlen;
			result = a[i]->id;
		}
	}

	return result;
}

void AgiEngine::dictionaryWords(char *msg) {
	char *p = NULL;
	char *q = NULL;
	int wid, wlen;

	debugC(2, kDebugLevelScripts, "msg = \"%s\"", msg);

	cleanInput();

	for (p = msg; p && *p && getvar(vWordNotFound) == 0;) {
		if (*p == 0x20)
			p++;

		if (*p == 0)
			break;

		wid = findWord(p, &wlen);
		debugC(2, kDebugLevelScripts, "find_word(p) == %d", wid);

		switch (wid) {
		case -1:
			debugC(2, kDebugLevelScripts, "unknown word");
			_game.egoWords[_game.numEgoWords].word = strdup(p);

			q = _game.egoWords[_game.numEgoWords].word;

			_game.egoWords[_game.numEgoWords].id = 19999;
			setvar(vWordNotFound, 1 + _game.numEgoWords);

			_game.numEgoWords++;

			p += strlen(p);
			break;
		case 0:
			// ignore this word
			debugC(2, kDebugLevelScripts, "ignore word");
			p += wlen;
			q = NULL;
			break;
		default:
			// an OK word
			debugC(3, kDebugLevelScripts, "ok word (%d)", wid);
			_game.egoWords[_game.numEgoWords].id = wid;
			_game.egoWords[_game.numEgoWords].word = myStrndup(p, wlen);
			_game.numEgoWords++;
			p += wlen;
			break;
		}

		if (p != NULL && *p) {
			debugC(2, kDebugLevelScripts, "p = %s", p);
			*p = 0;
			p++;
		}

		if (q != NULL) {
			for (; (*q != 0 && *q != 0x20); q++)
				;
			if (*q) {
				*q = 0;
				q++;
			}
		}
	}

	debugC(4, kDebugLevelScripts, "num_ego_words = %d", _game.numEgoWords);
	if (_game.numEgoWords > 0) {
		setflag(fEnteredCli, true);
		setflag(fSaidAcceptedInput, false);
	}
}

} // End of namespace Agi
