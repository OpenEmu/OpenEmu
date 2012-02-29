/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aam�s                                    *
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

template<typename T>
class Array {
	T *a;
	std::size_t sz;
	
	Array(const Array &ar);
	
public:
	Array(const std::size_t size = 0) : a(size ? new T[size] : 0), sz(size) {}
	~Array() { delete []a; }
	void reset(const std::size_t size) { delete []a; a = size ? new T[size] : 0; sz = size; }
	std::size_t size() const { return sz; }
	operator T*() { return a; }
	operator const T*() const { return a; }
};

#endif
