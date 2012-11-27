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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "search.h"
#include "md5.h"

#include <algorithm>
#include <cassert>

Hasher::Hash Hasher::createHash(const byte *data, uint32 size) {
	md5_context ctx;
	md5_starts(&ctx);
	md5_update(&ctx, data, size);

	Hash hash;
	md5_finish(&ctx, hash.digest);
	return hash;
}

SearchData SearchCreator::create(const char *filename) {
	FILE *f = fopen(filename, "rb");
	assert(f);

	SearchData data;
	data.size = fileSize(f);

	byte *buffer = new byte[data.size];
	fread(buffer, 1, data.size, f);
	fclose(f);

	data = create(buffer, data.size);
	delete[] buffer;

	return data;
}

SearchData SearchCreator::create(const byte *buffer, uint32 size) {
	SearchData data;

	data.size = size;
	data.hash = Hasher::createHash(buffer, data.size);
	data.byteSum = 0;

	for (uint32 i = 0; i < data.size; ++i)
		data.byteSum += buffer[i];

	return data;
}

SumCreator::SumCreator(InputList list, const DataInput &input) : _curOffset(0), _input(input), _sums() {
	// Sort in ascending order
	list.sort(std::less<uint32>());

	uint32 byteSum = 0;
	uint32 oldSize = 0;

	for (InputList::const_iterator i = list.begin(); i != list.end(); ++i) {
		// Strip out entries, which exceed the buffer size
		if (*i > _input.size())
			continue;

		// Strip out duplicates
		if (_sums.find(*i) != _sums.end())
			continue;

		// Only add the bytes exceeding the old sum's size
		// to the sum. This saves a few accesses.
		for (uint32 j = oldSize; j < *i; ++j)
			byteSum += _input[j];

		_sums[*i] = byteSum;

		// Save this sum's size
		oldSize = *i;
	}
}

bool SumCreator::nextByte() {
	// Calculate the bytes available for summing. We need to add
	// 1 here, since we will only update the offset AFTER everything
	// is done.
	const uint32 sizeLeft = _input.size() - (_curOffset + 1);

	if (!sizeLeft) {
		_sums.clear();
		return false;
	}

	// Grab the old first byte.
	const byte firstByte = _input[_curOffset];

	typedef std::list<uint32> DeletionList;
	DeletionList toRemove;

	for (SumMap::iterator i = _sums.begin(); i != _sums.end(); ++i) {
		// If this entry needs to sum up a larger buffer than the buffer
		// size left, we will remove the entry and continue to the next
		// one.
		if (i->first > sizeLeft) {
			// Add the current entry to the removal list.
			toRemove.push_back(i->first);
			continue;
		}

		// Update the byte sum. First we remove the old first byte
		// from the sum, next we add the next available byte.
		i->second -= firstByte;
		i->second += _input[_curOffset + i->first];
	}

	// Remove all entries flagged for removal
	for (DeletionList::const_iterator i = toRemove.begin(); i != toRemove.end(); ++i)
		_sums.erase(*i);

	// Update out offset.
	++_curOffset;

	// We return whether there are still some sums left available.
	return !_sums.empty();
}

bool SumCreator::hasSum(uint32 size) const {
	return _sums.find(size) != _sums.end();
}

uint32 SumCreator::getSum(uint32 size) const {
	SumMap::const_iterator s = _sums.find(size);

	if (s == _sums.end())
		return 0;

	return s->second;
}

Search::Search(const char *filename) : _data(), _search() {
	FILE *src = fopen(filename, "rb");
	assert(src);

	uint32 size = fileSize(src);
	byte *data = new byte[size];
	fread(data, 1, size, src);
	fclose(src);

	_data.resize(size);
	std::copy(data, data + size, _data.begin());
	delete[] data;
}

Search::Search(const byte *data, uint32 size) : _data(), _search() {
	_data.resize(size);
	std::copy(data, data + size, _data.begin());
}

void Search::addData(SearchData data) {
	// Do not add any duplicates
	if (std::find(_search.begin(), _search.end(), data) != _search.end())
		return;

	_search.push_back(data);
}

bool Search::search(ResultList &res) {
	SumCreator::InputList list;
	for (SearchList::const_iterator i = _search.begin(); i != _search.end(); ++i)
		list.push_back(i->size);

	SumCreator sum(list, _data);
	list.clear();

	do {
		const uint32 offset = sum.getOffset();

		for (SearchList::iterator i = _search.begin(); i != _search.end(); ) {
			if (!sum.hasSum(i->size)) {
				i = _search.erase(i);
				continue;
			}

			const uint32 byteSum = sum.getSum(i->size);
			if (byteSum == i->byteSum) {
				if (Hasher::createHash(&_data[offset], i->size) == i->hash) {
					res.push_back(ResultData(*i, offset));
					i = _search.erase(i);
					continue;
				}
			}

			++i;
		}
	} while (sum.nextByte());

	return !res.empty();
}
