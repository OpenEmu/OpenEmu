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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_FILLINGSTATION_H
#define PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_FILLINGSTATION_H

#include "pegasus/interaction.h"
#include "pegasus/movie.h"
#include "pegasus/notification.h"

namespace Pegasus {

class Item;

class NoradAlphaFillingStation : public GameInteraction, public NotificationReceiver {
public:
	NoradAlphaFillingStation(Neighborhood *);
	virtual ~NoradAlphaFillingStation() {}

	virtual void handleInput(const Input &, const Hotspot *);

	virtual void clickInHotspot(const Input &, const Hotspot *);
	virtual void activateHotspots();

	void newFillingItem(Item *);

protected:
	void receiveNotification(Notification *, const NotificationFlags);

	virtual void openInteraction();
	virtual void initInteraction();
	virtual void closeInteraction();

	void powerUpFinished();
	void splashFinished();
	void intakeWarningFinished();
	void intakeHighlightFinished();
	void dispenseHighlightFinished();
	void ArHighlightFinished();
	void CO2HighlightFinished();
	void HeHighlightFinished();
	void OHighlightFinished();
	void NHighlightFinished();

	void showIntakeInProgress(uint16);

	void clickInIntake();
	void clickInDispense();
	void clickInAr();
	void clickInCO2();
	void clickInHe();
	void clickInO();
	void clickInN();

	void dispenseGas();

	void setStaticState(TimeValue, int16);
	void setSegmentState(TimeValue, TimeValue, NotificationFlags, int16);

	Movie _rightSideMovie;
	Notification _rightSideNotification;
	NotificationCallBack _rightSideCallBack;
	int16 _state;
	ItemID _dispenseItemID;
};

} // End of namespace Pegasus

#endif
