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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "util.h"

#include <list>
#include <map>
#include <vector>
#include <cstring>

class Hasher {
public:
	struct Hash {
		uint8 digest[16];

		bool operator==(const Hash &r) const {
			return !std::memcmp(digest, r.digest, sizeof(digest));
		}

		bool operator<(const Hash &r) const {
			return std::memcmp(digest, r.digest, sizeof(digest)) < 0;
		}
	};

	static Hash createHash(const byte *data, uint32 size);
};

struct SearchData {
	uint32 size;

	uint32 byteSum;
	Hasher::Hash hash;

	bool operator==(const SearchData &r) const {
		return (size == r.size && byteSum == r.byteSum && hash == r.hash);
	}

	bool operator<(const SearchData &r) const {
		return (size < r.size && byteSum < r.byteSum && hash < r.hash);
	}
};

class SearchCreator {
public:
	static SearchData create(const char *filename);
	static SearchData create(const byte *buffer, uint32 size);
};

class SumCreator {
public:
	typedef std::list<uint32> InputList;
	typedef std::vector<byte> DataInput;

	SumCreator(InputList list, const DataInput &input);

	bool nextByte();

	uint32 getOffset() const { return _curOffset; }

	bool hasSum(uint32 size) const;
	uint32 getSum(uint32 size) const;
private:
	uint32 _curOffset;
	DataInput _input;

	typedef std::map<uint32, uint32> SumMap;
	SumMap _sums;
};

class Search {
public:
	Search(const char *filename);
	Search(const byte *data, uint32 size);

	void addData(SearchData data);

	struct ResultData {
		SearchData data;
		uint32 offset;

		ResultData() : data(), offset() {}
		ResultData(SearchData d, uint32 o) : data(d), offset(o) {}
	};

	typedef std::list<ResultData> ResultList;
	bool search(ResultList &res);
private:
	SumCreator::DataInput _data;

	typedef std::list<SearchData> SearchList;
	SearchList _search;
};

#endif
