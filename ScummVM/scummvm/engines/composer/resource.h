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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/hashmap.h"
#include "common/file.h"
#include "common/str.h"

#ifndef COMPOSER_RESOURCE_H
#define COMPOSER_RESOURCE_H

namespace Composer {

struct Animation;

#define ID_LBRC MKTAG('L','B','R','C') // Main FourCC

#define ID_ACEL MKTAG('A','C','E','L') // Keyboard Accelerator (v1)
#define ID_AMBI MKTAG('A','M','B','I') // Ambient (v1 sprite button)
#define ID_ANIM MKTAG('A','N','I','M') // Animation
#define ID_BMAP MKTAG('B','M','A','P') // Bitmap
#define ID_BUTN MKTAG('B','U','T','N') // Button
#define ID_CTBL MKTAG('C','T','B','L') // Color Table
#define ID_EVNT MKTAG('E','V','N','T') // Event
#define ID_PIPE MKTAG('P','I','P','E') // Pipe
#define ID_RAND MKTAG('R','A','N','D') // Random Object
#define ID_SCRP MKTAG('S','C','R','P') // Script
#define ID_VARI MKTAG('V','A','R','I') // Variables
#define ID_WAVE MKTAG('W','A','V','E') // Wave

#define ID_FRME MKTAG('F','R','M','E') // Frame

class Archive {
public:
	Archive();
	virtual ~Archive();

	bool openFile(const Common::String &fileName);
	virtual bool openStream(Common::SeekableReadStream *stream) = 0;
	void close();

	bool isOpen() const { return _stream != 0; }

	bool hasResource(uint32 tag, uint16 id) const;
	bool hasResource(uint32 tag, const Common::String &resName) const;
	Common::SeekableReadStream *getResource(uint32 tag, uint16 id);
	uint32 getResourceFlags(uint32 tag, uint16 id) const;
	uint32 getOffset(uint32 tag, uint16 id) const;
	uint16 findResourceID(uint32 tag, const Common::String &resName) const;
	Common::String getName(uint32 tag, uint16 id) const;

	Common::Array<uint32> getResourceTypeList() const;
	Common::Array<uint16> getResourceIDList(uint32 type) const;

protected:
	Common::SeekableReadStream *_stream;

	struct Resource {
		uint32 offset;
		uint32 size;
		Common::String name;
		uint32 flags;
	};

	typedef Common::HashMap<uint16, Resource> ResourceMap;
	typedef Common::HashMap<uint32, ResourceMap> TypeMap;
	TypeMap _types;
};

class ComposerArchive : public Archive {
public:
	ComposerArchive() : Archive() {}
	~ComposerArchive() {}

	bool openStream(Common::SeekableReadStream *stream);
};

struct PipeResourceEntry {
	uint32 size;
	uint32 offset;
};

struct PipeResource {
	Common::Array<PipeResourceEntry> entries;
};

class Pipe {
public:
	Pipe(Common::SeekableReadStream *stream);
	virtual ~Pipe();
	virtual void nextFrame();

	Animation *_anim;

	bool hasResource(uint32 tag, uint16 id) const;
	Common::SeekableReadStream *getResource(uint32 tag, uint16 id, bool buffering);

	virtual const Common::Array<uint16> *getScripts() { return NULL; }

protected:
	Common::SeekableReadStream *_stream;

	typedef Common::HashMap<uint16, PipeResource> ResourceMap;
	typedef Common::HashMap<uint32, ResourceMap> TypeMap;
	TypeMap _types;

	uint32 _offset;
};

class OldPipe : public Pipe {
public:
	OldPipe(Common::SeekableReadStream *stream);
	void nextFrame();

	const Common::Array<uint16> *getScripts() { return &_scripts; }

protected:
	uint32 _currFrame, _numFrames;
	Common::Array<uint16> _scripts;
};

} // End of namespace Composer

#endif
