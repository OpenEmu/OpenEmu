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

#ifndef GROOVIE_RESOURCE_H
#define GROOVIE_RESOURCE_H

namespace Common {
class MacResManager;
}

namespace Groovie {

struct ResInfo {
	uint16 gjd;
	uint32 offset;
	uint32 size;
	Common::String filename;
};

class ResMan {
public:
	virtual ~ResMan() {}

	Common::SeekableReadStream *open(uint32 fileRef);

	virtual uint32 getRef(Common::String name, Common::String scriptname = "") = 0;
	virtual bool getResInfo(uint32 fileRef, ResInfo &resInfo) = 0;

protected:
	Common::Array<Common::String> _gjds;

	uint16 _lastGjd;
};

class ResMan_t7g : public ResMan {
public:
	ResMan_t7g(Common::MacResManager *macResFork = 0);
	~ResMan_t7g() {}

	uint32 getRef(Common::String name, Common::String scriptname);
	bool getResInfo(uint32 fileRef, ResInfo &resInfo);

private:
	Common::MacResManager *_macResFork;
};

class ResMan_v2 : public ResMan {
public:
	ResMan_v2();
	~ResMan_v2() {}

	uint32 getRef(Common::String name, Common::String scriptname);
	bool getResInfo(uint32 fileRef, ResInfo &resInfo);
};

} // End of Groovie namespace

#endif // GROOVIE_RESOURCE_H
