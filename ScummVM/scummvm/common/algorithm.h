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

#ifndef COMMON_ALGORITHM_H
#define COMMON_ALGORITHM_H

#include "common/scummsys.h"
#include "common/func.h"
#include "common/util.h"

namespace Common {

/**
 * Copies data from the range [first, last) to [dst, dst + (last - first)).
 * It requires the range [dst, dst + (last - first)) to be valid.
 * It also requires dst not to be in the range [first, last).
 */
template<class In, class Out>
Out copy(In first, In last, Out dst) {
	while (first != last)
		*dst++ = *first++;
	return dst;
}

/**
 * Copies data from the range [first, last) to [dst - (last - first), dst).
 * It requires the range [dst - (last - first), dst) to be valid.
 * It also requires dst not to be in the range [first, last).
 *
 * Unlike copy copy_backward copies the data from the end to the beginning.
 */
template<class In, class Out>
Out copy_backward(In first, In last, Out dst) {
	while (first != last)
		*--dst = *--last;
	return dst;
}

/**
 * Copies data from the range [first, last) to [dst, dst + (last - first)).
 * It requires the range [dst, dst + (last - first)) to be valid.
 * It also requires dst not to be in the range [first, last).
 *
 * Unlike copy or copy_backward it does not copy all data. It only copies
 * a data element when operator() of the op parameter returns true for the
 * passed data element.
 */
template<class In, class Out, class Op>
Out copy_if(In first, In last, Out dst, Op op) {
	while (first != last) {
		if (op(*first))
			*dst++ = *first;
		++first;
	}
	return dst;
}

// Our 'specialized' 'fill' template for char, signed char and unsigned char arrays.
// Since C++ doesn't support partial specialized template functions (currently) we
// are going this way...
// With this we assure the usage of memset for those, which should be
// faster than a simple loop like for the generic 'fill'.
template<class Value>
signed char *fill(signed char *first, signed char *last, Value val) {
	memset(first, (val & 0xFF), last - first);
	return last;
}

template<class Value>
unsigned char *fill(unsigned char *first, unsigned char *last, Value val) {
	memset(first, (val & 0xFF), last - first);
	return last;
}

template<class Value>
char *fill(char *first, char *last, Value val) {
	memset(first, (val & 0xFF), last - first);
	return last;
}

/**
 * Sets all elements in the range [first, last) to val.
 */
template<class In, class Value>
In fill(In first, In last, const Value &val) {
	while (first != last)
		*first++ = val;
	return first;
}

/**
 * Finds the first data value in the range [first, last) matching v.
 * For data comperance it uses operator == of the data elements.
 */
template<class In, class T>
In find(In first, In last, const T &v) {
	while (first != last) {
		if (*first == v)
			return first;
		++first;
	}
	return last;
}

/**
 * Finds the first data value in the range [first, last) for which
 * the specified predicate p returns true.
 */
template<class In, class Pred>
In find_if(In first, In last, Pred p) {
	while (first != last) {
		if (p(*first))
			return first;
		++first;
	}
	return last;
}

/**
 * Applies the function f on all elements of the range [first, last).
 * The processing order is from beginning to end.
 */
template<class In, class Op>
Op for_each(In first, In last, Op f) {
	while (first != last) f(*first++);
	return f;
}

template<typename T>
unsigned int distance(T *first, T *last) {
	return last - first;
}

template<typename T>
unsigned int distance(T first, T last) {
	unsigned int n = 0;
	while (first != last) {
		++n;
		++first;
	}
	return n;
}

template<typename T>
T *sortChoosePivot(T *first, T *last) {
	return first + distance(first, last) / 2;
}

template<typename T>
T sortChoosePivot(T first, T last) {
	unsigned int n = distance(first, last);
	n /= 2;
	while (n--)
		++first;
	return first;
}

template<typename T, class StrictWeakOrdering>
T sortPartition(T first, T last, T pivot, StrictWeakOrdering &comp) {
	--last;
	SWAP(*pivot, *last);

	T sorted;
	for (sorted = first; first != last; ++first) {
		if (!comp(*last, *first)) {
			if (first != sorted)
				SWAP(*first, *sorted);
			++sorted;
		}
	}

	SWAP(*last, *sorted);
	return sorted;
}

/**
 * Simple sort function, modeled after std::sort.
 * It compares data with the given comparator object comp.
 *
 * Like std::sort this is not guaranteed to be stable.
 *
 * Two small quotes from wikipedia about stability:
 *
 * Stable sorting algorithms maintain the relative order of records with
 * equal keys.
 *
 * Unstable sorting algorithms may change the relative order of records with
 * equal keys, but stable sorting algorithms never do so.
 *
 * For more information on that topic check out:
 * http://en.wikipedia.org/wiki/Sorting_algorithm#Stability
 *
 * NOTE: Actually as the time of writing our implementation is unstable.
 */
template<typename T, class StrictWeakOrdering>
void sort(T first, T last, StrictWeakOrdering comp) {
	if (first == last)
		return;

	T pivot = sortChoosePivot(first, last);
	pivot = sortPartition(first, last, pivot, comp);
	sort<T, StrictWeakOrdering>(first, pivot, comp);
	sort<T, StrictWeakOrdering>(++pivot, last, comp);
}

/**
 * Simple sort function, modeled after std::sort.
 */
template<typename T>
void sort(T *first, T *last) {
	sort(first, last, Less<T>());
}

template<class T>
void sort(T first, T last) {
	sort(first, last, Less<typename T::ValueType>());
}

// MSVC is complaining about the minus operator being applied to an unsigned type
// We disable this warning for the affected section of code
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4146)
#endif

/**
 * Euclid's algorithm to compute the greatest common divisor.
 */
template<class T>
T gcd(T a, T b) {
	// Note: We check for <= instead of < to avoid spurious compiler
	// warnings if T is an unsigned type, i.e. warnings like "comparison
	// of unsigned expression < 0 is always false".
	if (a <= 0)
		a = -a;
	if (b <= 0)
		b = -b;

	while (a > 0) {
		T tmp = a;
		a = b % a;
		b = tmp;
	}

	return b;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

} // End of namespace Common
#endif
