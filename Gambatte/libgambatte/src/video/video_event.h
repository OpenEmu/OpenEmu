/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef VIDEO_EVENT_H
#define VIDEO_EVENT_H

class VideoEvent {
	unsigned long time_;
	const unsigned char priority_;
	
public:
	enum { DISABLED_TIME = 0xFFFFFFFFu };
	
	VideoEvent(const unsigned priority_in) :
		time_(DISABLED_TIME),
		priority_(priority_in)
	{}
	
	virtual ~VideoEvent() {}
	virtual void doEvent() = 0;
	
	unsigned priority() const {
		return priority_;
	}
	
	unsigned long time() const {
		return time_;
	}
	
	void setTime(const unsigned long time_in) {
		time_ = time_in;
	}
};

#endif
