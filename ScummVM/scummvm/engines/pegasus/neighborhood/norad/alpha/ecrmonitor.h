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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_ecrMONITOR_H
#define PEGASUS_NEIGHBORHOOD_NORAD_ALPHA_ecrMONITOR_H

#include "pegasus/interaction.h"
#include "pegasus/notification.h"
#include "pegasus/neighborhood/norad/alpha/panoramascroll.h"

namespace Pegasus {

class NoradAlphaECRMonitor : public GameInteraction, public NotificationReceiver {
public:
	NoradAlphaECRMonitor(Neighborhood *);
	virtual ~NoradAlphaECRMonitor() {}

	virtual void handleInput(const Input &, const Hotspot *);

protected:
	virtual void openInteraction();
	virtual void closeInteraction();

	virtual void receiveNotification(Notification *, const NotificationFlags);

	void ecrSection1Finished();
	void ecrPanFinished();
	void ecrSection2Finished();

	int findCurrentInterestingTime();
	void skipToNextInterestingTime();
	void skipToPreviousInterestingTime();

	Notification _ecrSlideShowNotification;
	Movie _ecrMovie;
	NotificationCallBack _ecrMovieCallBack;
	PanoramaScroll _ecrPan;
	NotificationCallBack _ecrPanCallBack;
};

} // End of namespace Pegasus

#endif
