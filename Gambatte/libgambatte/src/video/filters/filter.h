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
#ifndef FILTER_H
#define FILTER_H

#include "int.h"

namespace Gambatte {
struct FilterInfo;
}

class Filter {
public:
	virtual ~Filter() {}
	virtual void init() {};
	virtual void outit() {};
	virtual const Gambatte::FilterInfo& info() = 0;
	virtual void filter(Gambatte::uint_least32_t *dbuffer, unsigned pitch) = 0;
	virtual Gambatte::uint_least32_t* inBuffer() = 0;
	virtual unsigned inPitch() = 0;
};

#endif
