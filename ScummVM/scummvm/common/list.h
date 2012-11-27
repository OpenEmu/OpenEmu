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

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include "common/list_intern.h"

namespace Common {

/**
 * Simple double linked list, modeled after the list template of the standard
 * C++ library.
 */
template<typename t_T>
class List {
protected:
	typedef ListInternal::NodeBase		NodeBase;
	typedef ListInternal::Node<t_T>		Node;

	NodeBase _anchor;

public:
	typedef ListInternal::Iterator<t_T>		iterator;
	typedef ListInternal::ConstIterator<t_T>	const_iterator;

	typedef t_T value_type;
	typedef uint size_type;

public:
	List() {
		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;
	}
	List(const List<t_T> &list) {
		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;

		insert(begin(), list.begin(), list.end());
	}

	~List() {
		clear();
	}

	/**
	 * Inserts element before pos.
	 */
	void insert(iterator pos, const t_T &element) {
		insert(pos._node, element);
	}

	/**
	 * Inserts the elements from first to last before pos.
	 */
	template<typename iterator2>
	void insert(iterator pos, iterator2 first, iterator2 last) {
		for (; first != last; ++first)
			insert(pos, *first);
	}

	/**
	 * Deletes the element at location pos and returns an iterator pointing
	 * to the element after the one which was deleted.
	 */
	iterator erase(iterator pos) {
		assert(pos != end());
		return iterator(erase(pos._node)._next);
	}

	/**
	 * Deletes the element at location pos and returns an iterator pointing
	 * to the element before the one which was deleted.
	 */
	iterator reverse_erase(iterator pos) {
		assert(pos != end());
		return iterator(erase(pos._node)._prev);
	}

	/**
	 * Deletes the elements between first and last (including first but not
	 * last) and returns an iterator pointing to the element after the one
	 * which was deleted (i.e., last).
	 */
	iterator erase(iterator first, iterator last) {
		NodeBase *f = first._node;
		NodeBase *l = last._node;
		while (f != l)
			f = erase(f)._next;
		return last;
	}

	/**
	 * Removes all elements that are equal to val from the list.
	 */
	void remove(const t_T &val) {
		NodeBase *i = _anchor._next;
		while (i != &_anchor)
			if (val == static_cast<Node *>(i)->_data)
				i = erase(i)._next;
			else
				i = i->_next;
	}

	/** Inserts element at the start of the list. */
	void push_front(const t_T &element) {
		insert(_anchor._next, element);
	}

	/** Appends element to the end of the list. */
	void push_back(const t_T &element) {
		insert(&_anchor, element);
	}

	/** Removes the first element of the list. */
	void pop_front() {
		assert(!empty());
		erase(_anchor._next);
	}

	/** Removes the last element of the list. */
	void pop_back() {
		assert(!empty());
		erase(_anchor._prev);
	}

	/** Returns a reference to the first element of the list. */
	t_T &front() {
		return static_cast<Node *>(_anchor._next)->_data;
	}

	/** Returns a reference to the first element of the list. */
	const t_T &front() const {
		return static_cast<Node *>(_anchor._next)->_data;
	}

	/** Returns a reference to the last element of the list. */
	t_T &back() {
		return static_cast<Node *>(_anchor._prev)->_data;
	}

	/** Returns a reference to the last element of the list. */
	const t_T &back() const {
		return static_cast<Node *>(_anchor._prev)->_data;
	}

	List<t_T> &operator=(const List<t_T> &list) {
		if (this != &list) {
			iterator i;
			const iterator e = end();
			const_iterator i2;
			const_iterator e2 = list.end();

			for (i = begin(), i2 = list.begin();  (i != e) && (i2 != e2) ; ++i, ++i2) {
				static_cast<Node *>(i._node)->_data = static_cast<const Node *>(i2._node)->_data;
			}

			if (i == e)
				insert(i, i2, e2);
			else
				erase(i, e);
		}

		return *this;
	}

	size_type size() const {
		size_type n = 0;
		for (const NodeBase *cur = _anchor._next; cur != &_anchor; cur = cur->_next)
			++n;
		return n;
	}

	void clear() {
		NodeBase *pos = _anchor._next;
		while (pos != &_anchor) {
			Node *node = static_cast<Node *>(pos);
			pos = pos->_next;
			delete node;
		}

		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;
	}

	bool empty() const {
		return (&_anchor == _anchor._next);
	}


	iterator		begin() {
		return iterator(_anchor._next);
	}

	iterator		reverse_begin() {
		return iterator(_anchor._prev);
	}

	iterator		end() {
		return iterator(&_anchor);
	}

	const_iterator	begin() const {
		return const_iterator(_anchor._next);
	}

	const_iterator	reverse_begin() const {
		return const_iterator(_anchor._prev);
	}

	const_iterator	end() const {
		return const_iterator(const_cast<NodeBase *>(&_anchor));
	}

protected:
	NodeBase erase(NodeBase *pos) {
		NodeBase n = *pos;
		Node *node = static_cast<Node *>(pos);
		n._prev->_next = n._next;
		n._next->_prev = n._prev;
		delete node;
		return n;
	}

	/**
	 * Inserts element before pos.
	 */
	void insert(NodeBase *pos, const t_T &element) {
		ListInternal::NodeBase *newNode = new Node(element);
		assert(newNode);

		newNode->_next = pos;
		newNode->_prev = pos->_prev;
		newNode->_prev->_next = newNode;
		newNode->_next->_prev = newNode;
	}
};

} // End of namespace Common

#endif
