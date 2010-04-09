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

#ifndef INSERTION_SORT_H
#define INSERTION_SORT_H

#include <functional>

template<typename T, class Less>
void insertionSort(T *const start, T *const end, Less less) {
	if (start >= end)
		return;
	
	T *a = start;
	
	while (++a < end) {
		const T e = *a;
		
		T *b = a;
		
		while (b != start && less(e, *(b - 1))) {
			*b = *(b - 1);
			b = b - 1;
		}
		
		*b = e;
	}
}

template<typename T>
inline void insertionSort(T *const start, T *const end) {
	insertionSort(start, end, std::less<T>());
}

#endif /*INSERTION_SORT_H*/
