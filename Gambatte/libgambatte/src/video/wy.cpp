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
#include "wy.h"

#include "we_master_checker.h"
#include "scx_reader.h"
#include "../event_queue.h"
#include "../savestate.h"

Wy::WyReader1::WyReader1(Wy &wy, const WeMasterChecker &weMasterChecker) :
	VideoEvent(3),
	wy(wy),
	weMasterChecker(weMasterChecker)
{}

void Wy::WyReader1::doEvent() {
	if (wy.src_ >= wy.lyCounter.ly() && /*wy >= lyCounter.ly()*/ !weMasterChecker.weMaster())
		wy.set(wy.src_);
	
	setTime(DISABLED_TIME);
}

Wy::WyReader2::WyReader2(Wy &wy) :
	VideoEvent(4),
	wy(wy)
{}

void Wy::WyReader2::doEvent() {
	if (wy.wy_ == wy.lyCounter.ly() + 1 - wy.lyCounter.isDoubleSpeed() && wy.src_ > wy.wy_)
		wy.set(wy.src_);
	
	setTime(DISABLED_TIME);
}

Wy::WyReader3::WyReader3(Wy &wy) :
	VideoEvent(5),
	wy(wy)
{}

void Wy::WyReader3::doEvent() {
	if (wy.src_ == wy.lyCounter.ly() && wy.wy_ > wy.lyCounter.ly())
		wy.set(wy.src_);
	
	setTime(DISABLED_TIME);
}

unsigned long Wy::WyReader3::schedule(const unsigned wxSrc, const ScxReader &scxReader, const LyCounter &lyCounter, const unsigned long cycleCounter) {
	const unsigned curLineCycle = 456 - ((lyCounter.time() - cycleCounter) >> lyCounter.isDoubleSpeed());
	const unsigned baseTime = 78 + lyCounter.isDoubleSpeed() * 6 + wxSrc;
	
	if (curLineCycle >= 82U + lyCounter.isDoubleSpeed() * 3) {
		if (baseTime + scxReader.scxAnd7() > curLineCycle)
			return lyCounter.time() + ((baseTime + scxReader.scxAnd7()) << lyCounter.isDoubleSpeed()) - lyCounter.lineTime();
		else
			return lyCounter.time() + ((baseTime + scxReader.getSource()) << lyCounter.isDoubleSpeed());
	} else
		return lyCounter.nextLineCycle(baseTime + scxReader.getSource(), cycleCounter);
}

Wy::WyReader4::WyReader4(Wy &wy) :
	VideoEvent(6),
	wy(wy)
{}

void Wy::WyReader4::doEvent() {
	wy.set(wy.src_);
	
	setTime(DISABLED_TIME);
}

Wy::Wy(const LyCounter &lyCounter, const WeMasterChecker &weMasterChecker, M3ExtraCycles &m3ExtraCycles) :
	lyCounter(lyCounter),
	m3ExtraCycles(m3ExtraCycles),
	reader1_(*this, weMasterChecker),
	reader2_(*this),
	reader3_(*this),
	reader4_(*this)
{
	setSource(0);
	wy_ = src_;
}

void Wy::saveState(SaveState &state) const {
	state.ppu.wy = wy_;
}

void Wy::loadState(const SaveState &state) {
	wy_ = state.ppu.wy;
}
