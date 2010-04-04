/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
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
#ifndef GAMBATTE_H
#define GAMBATTE_H

class CPU;

#include "videoblitter.h"
#include "inputstate.h"
#include "inputstategetter.h"
#include "filterinfo.h"
#include "int.h"
#include <vector>

namespace Gambatte {
class GB {
	CPU *const z80;
	int stateNo;

	void loadState(const char *filepath, bool osdMessage);

public:
	GB();
	~GB();
	bool load(const char* romfile, bool forceDmg = false);
	
	/** Emulates until at least 'samples' stereo sound samples are produced in the supplied buffer.
	  * There are 35112 stereo sound samples in a video frame.
	  * May run for uptil 2064 stereo samples too long.
	  * A stereo sample consists of two native endian 2s complement 16-bit PCM samples,
	  * with the left sample preceding the right one. Usually casting soundBuf to/from
	  * short* is OK and recommended. The reason for not using a short* in the interface
	  * is to avoid implementation defined behaviour without compromising performance.
	  *
	  * @param soundBuf buffer with space >= samples + 2064
	  * @param samples number of stereo samples to produce
	  * @return actual number of samples produced
	  */
	unsigned runFor(Gambatte::uint_least32_t *soundBuf, unsigned samples);
	
	void reset();
	void setVideoBlitter(VideoBlitter *vb);
	void videoBufferChange();
	unsigned videoWidth() const;
	unsigned videoHeight() const;
	void setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned rgb32);
	
	void setVideoFilter(unsigned n);
	std::vector<const FilterInfo*> filterInfo() const;
	void setInputStateGetter(InputStateGetter *getInput);
	
	void set_savedir(const char *sdir);
	bool isCgb() const;
	void saveState();
	void loadState();
	void saveState(const char *filepath);
	void loadState(const char *filepath);
	void selectState(int n);
	int currentState() const { return stateNo; }
};
}

#endif
