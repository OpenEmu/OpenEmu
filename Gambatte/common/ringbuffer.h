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
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "array.h"
#include <cstddef>
#include <algorithm>
#include <cstring>

template<typename T>
class RingBuffer {
	Array<T> buf;
	std::size_t sz;
	std::size_t rpos;
	std::size_t wpos;
	
public:
	RingBuffer(const std::size_t sz_in = 0) : sz(0), rpos(0), wpos(0) { reset(sz_in); }
	
	std::size_t avail() const {
		return (wpos < rpos ? 0 : sz) + rpos - wpos - 1;
	}
	
	void clear() {
		wpos = rpos = 0;
	}
	
	void fill(T value);
	
	void read(T *out, std::size_t num);
	
	void reset(std::size_t sz_in);
	
	std::size_t size() const {
		return sz - 1;
	}
	
	std::size_t used() const {
		return (wpos < rpos ? sz : 0) + wpos - rpos;
	}
	
	void write(const T *in, std::size_t num);
};

template<typename T>
void RingBuffer<T>::fill(const T value) {
	std::fill(buf + 0, buf + sz, value);
	rpos = 0;
	wpos = sz - 1;
}

template<typename T>
void RingBuffer<T>::read(T *out, std::size_t num) {
	if (rpos + num > sz) {
		const std::size_t n = sz - rpos;
		
		std::memcpy(out, buf + rpos, n * sizeof(T));
		
		rpos = 0;
		num -= n;
		out += n;
	}
	
	std::memcpy(out, buf + rpos, num * sizeof(T));
	
	if ((rpos += num) == sz)
		rpos = 0;
}

template<typename T>
void RingBuffer<T>::reset(const std::size_t sz_in) {
	sz = sz_in + 1;
	rpos = wpos = 0;
	buf.reset(sz_in ? sz : 0);
}

template<typename T>
void RingBuffer<T>::write(const T *in, std::size_t num) {
	if (wpos + num > sz) {
		const std::size_t n = sz - wpos;
		
		std::memcpy(buf + wpos, in, n * sizeof(T));
		
		wpos = 0;
		num -= n;
		in += n;
	}
	
	std::memcpy(buf + wpos, in, num * sizeof(T));
	
	if ((wpos += num) == sz)
		wpos = 0;
}

#endif
