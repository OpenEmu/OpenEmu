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

#ifndef GOB_PREGOB_ONCEUPON_PARENTS_H
#define GOB_PREGOB_ONCEUPON_PARENTS_H

#include "gob/sound/sounddesc.h"

#include "gob/pregob/seqfile.h"

namespace Gob {

class Font;

class GCTFile;

namespace OnceUpon {

/** The home / parents animation sequence. */
class Parents : public SEQFile {
public:
	Parents(GobEngine *vm, const Common::String &seq, const Common::String &gct,
	        const Common::String &childName, uint8 house, const Font &font,
	        const byte *normalPalette, const byte *brightPalette, uint paletteSize);
	~Parents();

	void play();

protected:
	void handleFrameEvent();
	void handleInput(int16 key, int16 mouseX, int16 mouseY, MouseButtons mouseButtons);

private:
	static const uint kLoopCount = 7;

	static const uint16 kLoop[kLoopCount][3];

	enum Sound {
		kSoundCackle  = 0,
		kSoundThunder    ,
		kSoundCount
	};

	static const char *kSound[kSoundCount];


	uint8 _house;

	const Font *_font;

	uint _paletteSize;
	const byte *_normalPalette;
	const byte *_brightPalette;

	SoundDesc _sounds[kSoundCount];

	GCTFile *_gct;

	uint _loopID[kLoopCount];
	uint _currentLoop;


	void lightningEffect();

	void playSound(Sound sound);
	void setPalette(const byte *palette, uint size);

	void drawGCT(uint item, uint loop = 0xFFFF);
};

} // End of namespace OnceUpon

} // End of namespace Gob

#endif // GOB_PREGOB_ONCEUPON_PARENTS_H
