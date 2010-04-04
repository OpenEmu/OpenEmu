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
#ifndef WINDOW_H
#define WINDOW_H

#include "we.h"
#include "we_master_checker.h"
#include "wy.h"
#include "wx_reader.h"

struct Window {
	We we;
	WeMasterChecker weMasterChecker;
	Wy wyReg;
	WxReader wxReader;
	
	Window(event_queue<VideoEvent*,VideoEventComparer> &m3EventQueue,
           const LyCounter &lyCounter,
           M3ExtraCycles &m3ExtraCycles) :
		we(m3ExtraCycles),
		weMasterChecker(m3EventQueue, wyReg, lyCounter, m3ExtraCycles),
		wyReg(lyCounter, weMasterChecker, m3ExtraCycles),
		wxReader(m3EventQueue, we.enableChecker(), we.disableChecker(), m3ExtraCycles)
	{}
	
	bool enabled(const unsigned ly) const {
		return we.value() && wxReader.wx() < 0xA7 && ly >= wyReg.value() && (weMasterChecker.weMaster() || ly == wyReg.value());
	}
};

#endif /*WINDOW_H*/
