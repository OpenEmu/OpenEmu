/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COMMON_QUEUE_H
#define COMMON_QUEUE_H

#include "common/scummsys.h"
#include "common/list.h"

namespace Common {

/**
 * Variable size Queue class, implemented using our List class.
 */
template<class T>
class Queue {
//public:
//	typedef T value_type;

public:
	Queue<T>() : _impl() {}
	Queue<T>(const Queue<T> &queue) : _impl(queue._impl) {}

	bool empty() const {
		return _impl.empty();
	}

	void clear() {
		_impl.clear();
	}

	void push(const T &x) {
		_impl.push_back(x);
	}

	T &front() {
		return _impl.front();
	}

	const T &front() const {
		return _impl.front();
	}

	T &back() {
		return _impl.back();
	}

	const T &back() const {
		return _impl.back();
	}

	T pop() {
		T tmp = front();
		_impl.pop_front();
		return tmp;
	}

	int size() const {
		return _impl.size();
	}

private:
	List<T>	_impl;
};

} // End of namespace Common

#endif
