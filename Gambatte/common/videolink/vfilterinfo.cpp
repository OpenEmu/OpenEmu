/***************************************************************************
 *   Copyright (C) 2009 by Sindre Aamås                                    *
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
#include "vfilterinfo.h"
#include "vfilters/catrom2x.h"
#include "vfilters/catrom3x.h"
#include "vfilters/kreed2xsai.h"
#include "vfilters/maxsthq2x.h"
#include "vfilters/maxsthq3x.h"

static VideoLink* createNone() { return 0; }

template<class T>
static VideoLink* createT() { return new T; }

#define VFINFO(handle, Type) { handle, Type::OUT_WIDTH, Type::OUT_HEIGHT, createT<Type> }

static const VfilterInfo vfinfos[] = {
	{ "None", VfilterInfo::IN_WIDTH, VfilterInfo::IN_HEIGHT, createNone },
	VFINFO("Bicubic Catmull-Rom spline 2x", Catrom2x),
	VFINFO("Bicubic Catmull-Rom spline 3x", Catrom3x),
	VFINFO("Kreed's 2xSaI", Kreed2xSaI),
	VFINFO("MaxSt's hq2x", MaxStHq2x),
	VFINFO("MaxSt's hq3x", MaxStHq3x)
};

std::size_t VfilterInfo::numVfilters() {
	return sizeof(vfinfos) / sizeof(vfinfos[0]);
}

const VfilterInfo& VfilterInfo::get(std::size_t n) {
	return vfinfos[n];
}
