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
#ifndef MINKEEPER_H
#define MINKEEPER_H

#include <algorithm>

namespace MinKeeperUtil {
template<int n> struct CeiledLog2 { enum { RESULT = 1 + CeiledLog2<(n + 1) / 2>::RESULT }; };
template<> struct CeiledLog2<1> { enum { RESULT = 0 }; };

template<int v, int n> struct RoundedDiv2n { enum { RESULT = RoundedDiv2n<(v + 1) / 2, n - 1>::RESULT }; };
template<int v> struct RoundedDiv2n<v,1> { enum { RESULT = v }; };

template<template<int> class T, int n> struct Sum { enum { RESULT = T<n-1>::RESULT + Sum<T, n-1>::RESULT }; };
template<template<int> class T> struct Sum<T,0> { enum { RESULT = 0 }; };
}

// Keeps track of minimum value identified by id as values change.
// Higher ids prioritized (as min value) if values are equal. Can easily be reversed by swapping < for <=.
// Higher ids can be faster to change when the number of ids isn't a power of 2.
// Thus the ones that change more frequently should have higher ids if priority allows it.
template<int ids>
class MinKeeper {
	enum { LEVELS = MinKeeperUtil::CeiledLog2<ids>::RESULT };
	template<int l> struct Num { enum { RESULT = MinKeeperUtil::RoundedDiv2n<ids, LEVELS + 1 - l>::RESULT }; };
	template<int l> struct Sum { enum { RESULT = MinKeeperUtil::Sum<Num, l>::RESULT }; };
	
	template<int id, int level>
	struct UpdateValue {
		enum { P = Sum<level-1>::RESULT + id };
		enum { C0 = Sum<level>::RESULT + id * 2 };
		
		static void updateValue(MinKeeper<ids> *const s) {
			// GCC 4.3 generates better code with the ternary operator on i386.
			s->a[P] = (id * 2 + 1 == Num<level>::RESULT || s->values[s->a[C0]] < s->values[s->a[C0 + 1]]) ? s->a[C0] : s->a[C0 + 1];
	
			UpdateValue<id / 2, level - 1>::updateValue(s);
		}
	};
	
	template<int id>
	struct UpdateValue<id,0> {
		static void updateValue(MinKeeper<ids> *const s) {
			s->minValue_ = s->values[s->a[0]];
		}
	};
	
	template<int id, int dummy> struct FillLut {
		static void fillLut(MinKeeper<ids> *const s) {
			s->updateValueLut[id] = updateValue<id>;
			FillLut<id-1,dummy>::fillLut(s);
		}
	};
	
	template<int dummy> struct FillLut<-1,dummy> {
		static void fillLut(MinKeeper<ids> *) {}
	};
	
	
	unsigned long values[ids];
	unsigned long minValue_;
	void (*updateValueLut[Num<LEVELS-1>::RESULT])(MinKeeper<ids>*);
	int a[Sum<LEVELS>::RESULT];
	
	template<int id> static void updateValue(MinKeeper<ids> *s);
	
public:
	MinKeeper(unsigned long initValue = 0xFFFFFFFF);
	
	int min() const { return a[0]; }
	unsigned long minValue() const { return minValue_; }
	
	template<int id>
	void setValue(const unsigned long cnt) {
		values[id] = cnt;
		updateValue<id / 2>(this);
	}
	
	void setValue(const int id, const unsigned long cnt) {
		values[id] = cnt;
		updateValueLut[id >> 1](this);
	}
	
	unsigned long value(const int id) const { return values[id]; }
};

template<int ids>
MinKeeper<ids>::MinKeeper(const unsigned long initValue) {
	std::fill(values, values + ids, initValue);
	
	for (int i = 0; i < Num<LEVELS-1>::RESULT; ++i) {
		a[Sum<LEVELS-1>::RESULT + i] = (i * 2 + 1 == ids || values[i * 2] < values[i * 2 + 1]) ? i * 2 : i * 2 + 1;
	}
	
	int n   = Num<LEVELS-1>::RESULT;
	int off = Sum<LEVELS-1>::RESULT;
	
	while (off) {
		const int pn = (n + 1) >> 1;
		const int poff = off - pn;
		
		for (int i = 0; i < pn; ++i) {
			a[poff + i] = (i * 2 + 1 == n ||
					values[a[off + i * 2]] < values[a[off + i * 2 + 1]]) ?
					a[off + i * 2] : a[off + i * 2 + 1];
		}
		
		off = poff;
		n   = pn;
	}
	
	minValue_ = values[a[0]];

	FillLut<Num<LEVELS-1>::RESULT-1,0>::fillLut(this);
}

template<int ids>
template<int id>
void MinKeeper<ids>::updateValue(MinKeeper<ids> *const s) {
	s->a[Sum<LEVELS-1>::RESULT + id] = (id * 2 + 1 == ids || s->values[id * 2] < s->values[id * 2 + 1]) ? id * 2 : id * 2 + 1;

	UpdateValue<id / 2, LEVELS-1>::updateValue(s);
}

#endif
