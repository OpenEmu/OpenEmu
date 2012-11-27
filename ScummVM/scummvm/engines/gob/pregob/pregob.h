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

#ifndef GOB_PREGOB_PREGOB_H
#define GOB_PREGOB_PREGOB_H

#include "common/str.h"
#include "common/array.h"

#include "gob/util.h"
#include "gob/aniobject.h"

#include "gob/sound/sounddesc.h"

#include "gob/pregob/txtfile.h"

namespace Gob {

class GobEngine;
class Surface;

class GCTFile;

class PreGob {
public:
	PreGob(GobEngine *vm);
	virtual ~PreGob();

	virtual void run() = 0;

	struct AnimProperties {
		uint16 animation;
		uint16 frame;

		ANIObject::Mode mode;

		bool visible;
		bool paused;

		bool hasPosition;
		int16 x;
		int16 y;
	};

protected:
	typedef Common::Array<ANIObject *> ANIList;

	static const char  kLanguageSuffixShort[5];
	static const char *kLanguageSuffixLong [5];


	GobEngine *_vm;


	// -- Graphics --

	/** Initialize the game screen. */
	void initScreen();

	void fadeOut(); ///< Fade to black.
	void fadeIn();  ///< Fade to the current palette.

	void clearScreen();

	/** Change the palette.
	 *
	 *  @param palette The palette to change to.
	 *  @param size Size of the palette in colors.
	 */
	void setPalette(const byte *palette, uint16 size); ///< Change the palette

	/** Add a new cursor that can be manipulated to the stack. */
	void addCursor();
	/** Remove the top-most cursor from the stack. */
	void removeCursor();

	/** Set the current cursor. */
	void setCursor(Surface &sprite, int16 hotspotX, int16 hotspotY);
	/** Set the current cursor. */
	void setCursor(Surface &sprite, int16 left, int16 top, int16 right, int16 bottom,
	               int16 hotspotX, int16 hotspotY);

	/** Show the cursor. */
	void showCursor();
	/** Hide the cursor. */
	void hideCursor();

	/** Is the cursor currently visible? */
	bool isCursorVisible() const;

	/** Remove an animation from the screen. */
	void clearAnim(ANIObject &anim);
	/** Draw an animation to the screen, advancing it. */
	void drawAnim(ANIObject &anim);
	/** Clear and draw an animation to the screen, advancing it. */
	void redrawAnim(ANIObject &anim);

	/** Remove animations from the screen. */
	void clearAnim(const ANIList &anims);
	/** Draw animations to the screen, advancing them. */
	void drawAnim(const ANIList &anims);
	/** Clear and draw animations to the screen, advancing them. */
	void redrawAnim(const ANIList &anims);

	void loadAnims(ANIList &anims, ANIFile &ani, uint count, const AnimProperties *props) const;
	void freeAnims(ANIList &anims) const;

	void setAnim(ANIObject &anim, const AnimProperties &props) const;

	/** Wait for the frame to end, handling screen updates and optionally update input. */
	void endFrame(bool doInput);


	// -- Sound --

	/** Load all sounds that can be played interactively in the game. */
	void loadSounds(const char * const *sounds, uint soundCount);
	/** Free all loaded sound. */
	void freeSounds();

	/** Play a loaded sound. */
	void playSound(uint sound, int16 frequency = 0, int16 repCount = 0);
	/** Stop all sound playback. */
	void stopSound();

	/** Play a sound until it ends or is interrupted by a keypress. */
	void playSoundFile(const Common::String &file, int16 frequency = 0, int16 repCount = 0, bool interruptible = true);

	/** Beep the PC speaker. */
	void beep(int16 frequency, int32 length);


	// -- Input --

	/** Check mouse and keyboard input. */
	int16 checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons);
	/** Wait for mouse or keyboard input. */
	int16 waitInput (int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons);
	/** Wait for mouse or keyboard input, but don't care about what was done with the mouse. */
	int16 waitInput();
	/** Did we have mouse or keyboard input? */
	bool  hasInput();


	// -- TXT helpers --

	/** Get the name of a localized file. */
	Common::String getLocFile(const Common::String &file) const;
	/** Open a TXT file. */
	TXTFile *loadTXT(const Common::String &txtFile, TXTFile::Format format) const;

	/** Called by loadTXT() to fix strings within the TXT file. */
	virtual void fixTXTStrings(TXTFile &txt) const;


	// -- GCT helpers --

	GCTFile *loadGCT(const Common::String &gctFile) const;


private:
	/** Did we fade out? */
	bool _fadedOut;

	/** All loaded sounds. */
	Common::Array<SoundDesc> _sounds;


	/** Load a sound file. */
	bool loadSound(SoundDesc &sound, const Common::String &file) const;
};

} // End of namespace Gob

#endif // GOB_PREGOB_PREGOB_H
