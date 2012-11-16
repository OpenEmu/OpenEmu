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
#ifndef VIDEO_LYC_IRQ_H
#define VIDEO_LYC_IRQ_H

namespace gambatte {

struct SaveState;
class LyCounter;

class LycIrq {
	unsigned long time_;
 	unsigned char lycRegSrc_;
 	unsigned char statRegSrc_;
	unsigned char lycReg_;
	unsigned char statReg_;
	bool cgb_;
	
	void regChange(unsigned statReg, unsigned lycReg, const LyCounter &lyCounter, unsigned long cc);
	
public:
	LycIrq();
	void doEvent(unsigned char *ifreg, const LyCounter &lyCounter);
	unsigned lycReg() const { return lycRegSrc_; }
	void loadState(const SaveState &state);
	void saveState(SaveState &state) const;
	unsigned long time() const { return time_; }
	void setCgb(const bool cgb) { cgb_ = cgb; }
	void lcdReset();
	void reschedule(const LyCounter & lyCounter, unsigned long cc);
	
	void statRegChange(unsigned statReg, const LyCounter &lyCounter, unsigned long cc) {
		regChange(statReg, lycRegSrc_, lyCounter, cc);
	}
	
	void lycRegChange(unsigned lycReg, const LyCounter &lyCounter, unsigned long cc) {
		regChange(statRegSrc_, lycReg, lyCounter, cc);
	}
};

}

#endif
