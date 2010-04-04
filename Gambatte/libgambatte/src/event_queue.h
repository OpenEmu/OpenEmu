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
#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <cstddef>

template<typename T, class Comparer>
class event_queue {
	Comparer comparer;
	T *const a;
	const std::size_t capacity_;
	std::size_t size_;


	std::size_t indexOf(T e);
	void internalDec(std::size_t i, T e);
	template<bool child2BoundsCheck> void internalInc(std::size_t i, T e);

public:
	event_queue(std::size_t capacity, const Comparer &comparer);
	~event_queue();

	std::size_t capacity() const {
		return capacity_;
	}

	void clear() {
		size_ = 0;
	}

	void dec(const T oldE, const T newE) {
		internalDec(indexOf(oldE), newE);
	}
	
	bool empty() const {
		return size_ == 0;
	}
	
	void inc(const T oldE, const T newE) {
		internalInc<true>(indexOf(oldE), newE);
	}
	
	void modify_root(const T newRoot) {
		internalInc<true>(0, newRoot);
	}
	
	void pop() {
		internalInc<false>(0, a[--size_]);
	}
	
	void push(const T e) {
		internalDec(size_++, e);
	}
	
	void remove(T e);
	
	std::size_t size() const {
		return size_;
	}
	
	T top() const {
		return a[0];
	}
};

template<typename T, class Comparer>
event_queue<T,Comparer>::event_queue(const std::size_t capacity, const Comparer &comparer_in) :
	comparer(comparer_in),
	a(new T[capacity]),
	capacity_(capacity),
	size_(0)
{}

template<typename T, class Comparer>
event_queue<T,Comparer>::~event_queue() {
	delete[] a;
}

template<typename T, class Comparer>
std::size_t event_queue<T,Comparer>::indexOf(const T e) {
	std::size_t i = 0;
	
	while (a[i] != e)
		++i;
	
	return i;
}

template<typename T, class Comparer>
void event_queue<T,Comparer>::internalDec(std::size_t i, const T e) {
	a[i] = e;
	
	while (i != 0) {
		const std::size_t parentI = (i - 1) >> 1;
		
		if (!comparer.less(e, a[parentI]))
			break;
		
		a[i] = a[parentI];
		a[parentI] = e;
		i = parentI;
	}
}

template<typename T, class Comparer>
template<const bool child2BoundsCheck>
void event_queue<T,Comparer>::internalInc(std::size_t i, const T e) {
	a[i] = e;
	
	for (;;) {
		std::size_t childI = i * 2 + 1;
		
		if (childI >= size_)
			break;
		
		if ((!child2BoundsCheck || childI + 1 < size_) && comparer.less(a[childI + 1], a[childI]))
			++childI;
		
		if (!comparer.less(a[childI], e))
			break;
		
		a[i] = a[childI];
		a[childI] = e;
		i = childI;
	}
}

template<typename T, class Comparer>
void event_queue<T,Comparer>::remove(const T e) {
	std::size_t i = indexOf(e);
	
	while (i != 0) {
		const std::size_t parentI = (i - 1) >> 1;
		
		a[i] = a[parentI];
		a[parentI] = e;
		i = parentI;
	}
	
	pop();
}

#endif
