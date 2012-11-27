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

#ifndef GOB_DEMOPLAYER_H
#define GOB_DEMOPLAYER_H

#include "common/str.h"
#include "common/stream.h"
#include "common/hashmap.h"

namespace Gob {

class GobEngine;

class DemoPlayer {
public:
	DemoPlayer(GobEngine *vm);
	virtual ~DemoPlayer();

	bool play(const char *fileName);
	bool play(uint32 index);

protected:
	GobEngine *_vm;
	bool _doubleMode;
	bool _autoDouble;
	bool _rebase0;

	virtual bool playStream(Common::SeekableReadStream &stream) = 0;

	bool lineStartsWith(const Common::String &line, const char *start);

	void init();

	void evaluateVideoMode(const char *mode);
	void clearScreen();
	void playVideo(const char *fileName);
	void playADL(const char *params);

	void playVideoNormal(int slot);
	void playVideoDoubled(int slot);
	void playADL(const Common::String &fileName, bool waitEsc = true, int32 repeat = -1);

private:
	enum ScriptSource {
		kScriptSourceFile,
		kScriptSourceDirect
	};

	struct Script {
		ScriptSource source;
		const char *script;
	};

	static Script _scripts[];
};

} // End of namespace Gob

#endif // GOB_DEMOPLAYER_H
