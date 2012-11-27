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

#ifndef GOB_HOTSPOTS_H
#define GOB_HOTSPOTS_H

#include "common/stack.h"

#include "gob/util.h"

namespace Gob {

class Font;
class Script;

class Hotspots {
public:
	static const int kHotspotCount = 250;

	enum Type {
		kTypeNone              =  0,
		kTypeMove              =  1,
		kTypeClick             =  2,
		kTypeInput1NoLeave     =  3,
		kTypeInput1Leave       =  4,
		kTypeInput2NoLeave     =  5,
		kTypeInput2Leave       =  6,
		kTypeInput3NoLeave     =  7,
		kTypeInput3Leave       =  8,
		kTypeInputFloatNoLeave =  9,
		kTypeInputFloatLeave   = 10,
		kTypeEnable2           = 11,
		kTypeEnable1           = 12,
		kTypeClickEnter        = 21
	};

	enum State {
		kStateFilledDisabled = 0xC,
		kStateFilled         = 0x8,
		kStateDisabled       = 0x4,
		kStateType2          = 0x2,
		kStateType1          = 0x1
	};

	Hotspots(GobEngine *vm);
	~Hotspots();

	/** Remove all hotspots. */
	void clear();

	/** Add a hotspot, returning the new index. */
	uint16 add(uint16 id,
			uint16 left,  uint16 top, uint16 right, uint16 bottom,
			uint16 flags, uint16 key,
			uint16 funcEnter, uint16 funcLeave, uint16 funcPos);

	/** Remove a specific hotspot. */
	void remove(uint16 id);
	/** Remove all hotspots in this state. */
	void removeState(uint8 state);

	/** Push the current hotspots onto the stack.
	 *
	 *  @param all   0: Don't push global ones; 1: Push all; 2: Push only the disabled ones
	 *  @param force Force a push although _shouldPush is false
	 */
	void push(uint8 all, bool force = false);
	/** Pop hotspots from the stack. */
	void pop();

	/** Check the current hotspot. */
	uint16 check(uint8 handleMouse, int16 delay, uint16 &id, uint16 &index);
	/** Check the current hotspot. */
	uint16 check(uint8 handleMouse, int16 delay);

	/** Evaluate hotspot changes. */
	void evaluate();

	/** Return the cursor found in the hotspot to the coordinates. */
	int16 findCursor(uint16 x, uint16 y) const;

	/** implementation of oPlaytoons_F_1B code*/
	void oPlaytoons_F_1B();

private:
	struct Hotspot {
		uint16  id;
		uint16  left;
		uint16  top;
		uint16  right;
		uint16  bottom;
		uint16  flags;
		uint16  key;
		uint16  funcEnter;
		uint16  funcLeave;
		uint16  funcPos;
		Script *script;

		Hotspot();
		Hotspot(uint16 i,
				uint16 l, uint16 t, uint16 r, uint16 b, uint16 f, uint16 k,
				uint16 enter, uint16 leave, uint16 pos);

		void clear();

		Type         getType  () const;
		MouseButtons getButton() const;
		uint16       getWindow() const;
		uint8        getCursor() const;
		uint8        getState () const;

		/** Is this hotspot the block end marker? */
		bool isEnd() const;

		bool isInput      () const;
		bool isActiveInput() const;
		bool isInputLeave () const;

		bool isFilled       () const;
		bool isFilledEnabled() const;
		bool isFilledNew    () const;
		bool isDisabled     () const;

		/** Are the specified coordinates in the hotspot? */
		bool isIn(uint16 x, uint16 y) const;
		/** Does the specified button trigger the hotspot? */
		bool buttonMatch(MouseButtons button) const;

		static uint8 getState(uint16 id);

		void disable();
		void enable ();
	};

	struct StackEntry {
		bool     shouldPush;
		Hotspot *hotspots;
		uint32   size;
		uint32   key;
		uint32   id;
		uint32   index;
		uint16   x;
		uint16   y;
	};

	struct InputDesc {
		uint16 fontIndex;
		uint16 backColor;
		uint16 frontColor;
		uint16 length;
		const char *str;
	};

	GobEngine *_vm;

	Hotspot *_hotspots;
	Common::Stack<StackEntry> _stack;

	bool _shouldPush;

	uint16 _currentKey;
	uint16 _currentIndex;
	uint16 _currentId;
	uint16 _currentX;
	uint16 _currentY;

	/** Add a hotspot, returning the new index. */
	uint16 add(const Hotspot &hotspot);

	/** Recalculate all hotspot parameters
	 *
	 *  @param force Force recalculation of all hotspots, including global ones.
	 */
	void recalculate(bool force);

	/** Is this a valid hotspot? */
	bool isValid(uint16 key, uint16 id, uint16 index) const;

	/** Call a hotspot subroutine. */
	void call(uint16 offset);
	/** Handling hotspot enter events. */
	void enter(uint16 index);
	/** Handling hotspot leave events. */
	void leave(uint16 index);

	/** Check whether a specific part of the window forces a certain cursor. */
	int16 windowCursor(int16 &dx, int16 &dy) const;

	/** Which hotspot is the mouse cursor currently at? */
	uint16 checkMouse(Type type, uint16 &id, uint16 &index) const;

	/** Did the current hotspot change in the meantime? */
	bool checkHotspotChanged();

	/** Update events from a specific input. */
	uint16 updateInput(uint16 xPos, uint16 yPos, uint16 width, uint16 height,
			uint16 backColor, uint16 frontColor, char *str, uint16 fontIndex,
			Type type, int16 &duration, uint16 &id, uint16 &index);

	/** Handle all inputs we currently manage. */
	uint16 handleInputs(int16 time, uint16 inputCount, uint16 &curInput,
			InputDesc *inputs, uint16 &id, uint16 &index);

	/** Evaluate adding new hotspots script commands. */
	void evaluateNew(uint16 i, uint16 *ids, InputDesc *inputs,
			uint16 &inputId, bool &hasInput, uint16 &inputCount);
	/** Find the hotspot requested by script commands. */
	bool evaluateFind(uint16 key, int16 timeVal, const uint16 *ids,
			uint16 leaveWindowIndex, uint16 hotspotIndex1, uint16 hotspotIndex2,
			uint16 endIndex, int16 &duration, uint16 &id, uint16 &index, bool &finished);

	// Finding specific hotspots
	/** Find the hotspot index that corresponds to the input index. */
	uint16 inputToHotspot(uint16 input) const;
	/** Find the input index that corresponds to the hotspot index. */
	uint16 hotspotToInput(uint16 hotspot) const;
	/** Find the input that was clicked on. */
	uint16 findClickedInput(uint16 index) const;
	/** Find the first input hotspot with a leave function. */
	bool findFirstInputLeave(uint16 &id, uint16 &inputId, uint16 &index) const;
	/** Find the hotspot with the matching key, case sensitively. */
	bool findKey(uint16 key, uint16 &id, uint16 &index) const;
	/** Find the hotspot with the matching key, case insensitively. */
	bool findKeyCaseInsensitive(uint16 key, uint16 &id, uint16 &index) const;
	/** Find the nth plain (without Type1 or Type2 state) hotspot. */
	bool findNthPlain(uint16 n, uint16 startIndex, uint16 &id, uint16 &index) const;

	/** Leave the nth plain (without Type1 or Type2 state) hotspot. */
	bool leaveNthPlain(uint16 n, uint16 startIndex, int16 timeVal, const uint16 *ids,
			uint16 &id, uint16 &index, int16 &duration);

	// Hotspot ID variable access
	void setCurrentHotspot(const uint16 *ids, uint16 id) const;
	uint32 getCurrentHotspot() const;

	// String input functions
	void cleanFloatString(const Hotspot &spot) const;
	void checkStringMatch(const Hotspot &spot, const InputDesc &input,
			uint16 inputPos) const;
	void matchInputStrings(const InputDesc *inputs) const;

	uint16 convertSpecialKey(uint16 key) const;

	/** Calculate the graphical cursor position. */
	void getTextCursorPos(const Font &font, const char *str,
			uint32 pos, uint16 x, uint16 y, uint16 width, uint16 height,
			uint16 &cursorX, uint16 &cursorY, uint16 &cursorWidth, uint16 &cursorHeight) const;

	/** Fill that rectangle with the color. */
	void fillRect(uint16 x, uint16 y, uint16 width, uint16 height, uint16 color) const;
	/** Print the given text. */
	void printText(uint16 x, uint16 y, const char *str, uint16 fontIndex, uint16 color) const;

	/** Go through all inputs we manage and redraw their texts. */
	void updateAllTexts(const InputDesc *inputs) const;
};

} // End of namespace Gob

#endif // GOB_HOTSPOTS_H
