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
#ifndef CATROM3X_H
#define CATROM3X_H

#include "filter.h"

struct FilterInfo;

class Catrom3x : public Filter {
	Gambatte::uint_least32_t *buffer;

public:
	Catrom3x();
	~Catrom3x();
	void init();
	void outit();
	const Gambatte::FilterInfo& info();
	void filter(Gambatte::uint_least32_t *dbuffer, unsigned pitch);
	Gambatte::uint_least32_t* inBuffer();
	unsigned inPitch();
};

#endif
