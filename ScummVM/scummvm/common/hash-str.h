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

#ifndef COMMON_HASH_STR_H
#define COMMON_HASH_STR_H

#include "common/hashmap.h"
#include "common/str.h"

namespace Common {

uint hashit(const char *str);
uint hashit_lower(const char *str);	// Generate a hash based on the lowercase version of the string
inline uint hashit(const String &str) { return hashit(str.c_str()); }
inline uint hashit_lower(const String &str) { return hashit_lower(str.c_str()); }


// FIXME: The following functors obviously are not consistently named

struct CaseSensitiveString_EqualTo {
	bool operator()(const String& x, const String& y) const { return x.equals(y); }
};

struct CaseSensitiveString_Hash {
	uint operator()(const String& x) const { return hashit(x.c_str()); }
};


struct IgnoreCase_EqualTo {
	bool operator()(const String& x, const String& y) const { return x.equalsIgnoreCase(y); }
};

struct IgnoreCase_Hash {
	uint operator()(const String& x) const { return hashit_lower(x.c_str()); }
};



// Specalization of the Hash functor for String objects.
// We do case sensitve hashing here, because that is what
// the default EqualTo is compatible with. If one wants to use
// case insensitve hashing, then only because one wants to use
// IgnoreCase_EqualTo, and then one has to specify a custom
// hash anyway.
template<>
struct Hash<String> {
	uint operator()(const String& s) const {
		return hashit(s.c_str());
	}
};

template<>
struct Hash<const char *> {
	uint operator()(const char *s) const {
		return hashit(s);
	}
};

// String map -- by default case insensitive
typedef HashMap<String, String, IgnoreCase_Hash, IgnoreCase_EqualTo> StringMap;



}	// End of namespace Common


#endif
