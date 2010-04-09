/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
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
#ifndef RESAMPLER_INFO_H
#define RESAMPLER_INFO_H

#include "resampler.h"

struct ResamplerInfo {
	const char *desc;
	Resampler* (*create)(long inRate, long outRate, std::size_t periodSz);
	
	static unsigned num() { return num_; }
	static const ResamplerInfo& get(unsigned n) { return resamplers[n]; }
	
private:
	static const ResamplerInfo resamplers[];
	static const unsigned num_;
};

#endif
