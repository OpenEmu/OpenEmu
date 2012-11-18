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
#ifndef ARRAY_H
#define ARRAY_H

#include <cstddef>
#include "uncopyable.h"

template<typename T>
class Array : Uncopyable {
	T *a;
	std::size_t sz;
	
public:
	explicit Array(const std::size_t size = 0) : a(size ? new T[size] : 0), sz(size) {}
	~Array() { delete []a; }
	void reset(const std::size_t size = 0) { delete []a; a = size ? new T[size] : 0; sz = size; }
	std::size_t size() const { return sz; }
	T * get() const { return a; }
	operator T*() const { return a; }
};

#endif
