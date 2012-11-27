/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_AI_AIAREA_H
#define PEGASUS_AI_AIAREA_H

#include "pegasus/input.h"
#include "pegasus/movie.h"
#include "pegasus/timers.h"
#include "pegasus/ai/ai_rule.h"

namespace Common {
	class ReadStream;
	class WriteStream;
}

/*

	The AI area is the area at the bottom of the screen. There are three areas within
	the AI area:
	1)	the inventory/AI help area
	2)	the middle area
	3)	the biochip display area

	Area 1 is used for displaying the current inventory item. When the player changes the
	current item, either by selecting a new one in the inventory list or by picking
	up a new item, area 1 updates to show the new item.

	If the AI decides to give a message, the AI's head temporarily takes over area 1
	for the duration of the message, then goes away, returning the area to the current
	inventory item.

	Area 2 is used to display the current inventory item's state, the current biochip's
	state, and any extra information from the AI. The contention for this area is
	resolved as follows:
	--	If the AI needs to use the area while giving a message in area 1, it takes over
		area 2 for the duration of its message.
*** This is not true.
	--	If the player selects a new inventory item, the inventory item's state gets
		displayed immediately.
	--	If the player selects a new biochip, the biochip's state info gets displayed
		immediately.
	--	If any auto drawing is to occur, it seizes the area as soon as the drawing is
		to occur. For example, the mapping biochip does auto drawing every time the
		player takes a step. The only exception to this rule is if the AI is presenting
		a warning. When the AI seizes areas 1 and 2, it preempts all other uses.
	Some inventory items and biochips can cause arbitrary drawing to occur in this area
	at arbitrary times. The map biochip is one example which causes drawing when the
	player takes a step. Another example is the poison gas canister, which flashes in
	this area to indicate a dangerous compound.

	Area 3 is used to display the current biochip. When the player changes the current
	biochip, either by selecting a new one from the biochip list or by picking up a
	new one, area 3 updates to show the new item. In addition, some biochips can play
	animation in this area.

*/

namespace Pegasus {

class AIArea : public Surface, public Idler, public InputHandler {
public:
	AIArea(InputHandler *);
	virtual ~AIArea();

	void writeAIRules(Common::WriteStream *stream);
	void readAIRules(Common::ReadStream *stream);

	void initAIArea();

	void saveAIState();
	void restoreAIState();

	void handleInput(const Input &, const Hotspot *);
	void activateHotspots();
	void clickInHotspot(const Input &, const Hotspot *);

	void setAIVolume(const uint16);

	// There are only so many legal combinations of client/area.
	// Here is the list of supported pairs:
	//      kInventorySignature     kLeftAreaSignature
	//      kInventorySignature     kMiddleAreaSignature
	//      kBiochipSignature       kMiddleAreaSignature
	//      kBiochipSignature       kRightAreaSignature
	//      kAISignature            kLeftAreaSignature
	// Further, the kAISignature never sets a static frame time in the left area,
	// but only plays a sequence from the AI movie.
	void setAIAreaToTime(const LowerClientSignature, const LowerAreaSignature, const TimeValue);

	// The "Play" functions play the requested sequence synchronously.
	void playAIAreaSequence(const LowerClientSignature, const LowerAreaSignature, const TimeValue, const TimeValue);

	// For PlayAIMovie, it is assumed that the client is the AI itself.
	// This is used to play AI messages as well as Optical Memory video.
	// Returns true if the movie played all the way through, false if it was interrupted.
	bool playAIMovie(const LowerAreaSignature, const Common::String &movieName, bool keepLastFrame, const InputBits);

	// Loop the requested sequence indefinitely.
	void loopAIAreaSequence(const LowerClientSignature, const LowerAreaSignature, const TimeValue, const TimeValue);

	void addAIRule(AIRule *);

	// Remove and delete all rules.
	void removeAllRules();

	void lockAIOut();
	void unlockAI();
	void forceAIUnlocked();

	void checkMiddleArea();
	void checkRules();

	LowerClientSignature getMiddleAreaOwner();
	void toggleMiddleAreaOwner();

	TimeValue getBigInfoTime();
	void getSmallInfoSegment(TimeValue &, TimeValue &);

protected:
	void useIdleTime();

	void setLeftMovieTime(const TimeValue);
	void setMiddleMovieTime(const LowerClientSignature, const TimeValue);
	void setRightMovieTime(const TimeValue);

	Movie _leftAreaMovie;
	Movie _middleAreaMovie;
	Movie _rightAreaMovie;
	Movie _AIMovie;

	LowerClientSignature _leftAreaOwner;
	LowerClientSignature _middleAreaOwner;
	LowerClientSignature _rightAreaOwner;

	TimeValue _leftInventoryTime;
	TimeValue _middleInventoryTime;
	TimeValue _middleBiochipTime;
	TimeValue _rightBiochipTime;

	AIRuleList _AIRules;

	uint _lockCount;
};

extern AIArea *g_AIArea;

} // End of namespace Pegasus

#endif
