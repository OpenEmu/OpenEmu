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

#ifndef COMMON_STACK_H
#define COMMON_STACK_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Common {

/**
 * Extremly simple fixed size stack class.
 */
template<class T, uint MAX_SIZE = 10>
class FixedStack {
public:
	typedef uint size_type;

	FixedStack<T, MAX_SIZE>() : _size(0) {}

	bool empty() const {
		return _size <= 0;
	}

	void clear() {
		_size = 0;
	}

	void push(const T &x) {
		assert(_size < MAX_SIZE);
		_stack[_size++] = x;
	}

	const T &top() const {
		assert(_size > 0);
		return _stack[_size - 1];
	}

	T &top() {
		assert(_size > 0);
		return _stack[_size - 1];
	}

	T pop() {
		T tmp = top();
		--_size;
		return tmp;
	}

	size_type size() const {
		return _size;
	}

	T &operator[](size_type i) {
		assert(i < MAX_SIZE);
		return _stack[i];
	}

	const T &operator[](size_type i) const {
		assert(i < MAX_SIZE);
		return _stack[i];
	}

protected:
	T         _stack[MAX_SIZE];
	size_type _size;
};


/**
 * Variable size stack class, implemented using our Array class.
 */
template<class T>
class Stack {
private:
	Array<T> _stack;

public:
	typedef typename Array<T>::size_type size_type;

	Stack<T>() {}
	Stack<T>(const Array<T> &stackContent) : _stack(stackContent) {}

	bool empty() const {
		return _stack.empty();
	}

	void clear() {
		_stack.clear();
	}

	void push(const T &x) {
		_stack.push_back(x);
	}

	T &top() {
		return _stack.back();
	}

	const T &top() const {
		return _stack.back();
	}

	T pop() {
		T tmp = _stack.back();
		_stack.pop_back();
		return tmp;
	}

	size_type size() const {
		return _stack.size();
	}

	T &operator[](size_type i) {
		return _stack[i];
	}

	const T &operator[](size_type i) const {
		return _stack[i];
	}
};

} // End of namespace Common

#endif
