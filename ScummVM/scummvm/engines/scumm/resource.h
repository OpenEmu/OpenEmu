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

#ifndef SCUMM_RESOURCE_H
#define SCUMM_RESOURCE_H

#include "common/array.h"
#include "scumm/scumm.h"	// for ResType

namespace Scumm {

enum {
	OF_OWNER_MASK = 0x0F,
	OF_STATE_MASK = 0xF0,

	OF_STATE_SHL = 4
};

class ResourceIterator {
	uint32 _size;
	uint32 _pos;
	const byte *_ptr;
	bool _smallHeader;
public:
	ResourceIterator(const byte *searchin, bool smallHeader);
	const byte *findNext(uint32 tag);
};

enum {
	RES_INVALID_OFFSET = 0xFFFFFFFF
};

class ScummEngine;

/**
 * The mode of a resource type indicates whether the resource can be restored
 * from the game data files or not.
 * This affects for example whether the resource is stored in savestates.
 *
 * Note that we treat sound resources somewhat differently: On the one hand,
 * these behave mostly like a kStaticResTypeMode res type. However, when we
 * create a savestate, we do save *some* information about them: Namely, which
 * sound resources are loaded in memory at the time the save is made. And when
 * loading, we invoke ensureResourceLoaded() for each sound resource that was
 * marked in this way.
 */
enum ResTypeMode {
	kDynamicResTypeMode = 0,	///< Resource is generated during runtime and may change
	kStaticResTypeMode = 1,		///< Resource comes from data files, does not change
	kSoundResTypeMode = 2		///< Resource comes from data files, but may change
};

/**
 * The 'resource manager' class. Currently doesn't really deserve to be called
 * a 'class', at least until somebody gets around to OOfying this more.
 */
class ResourceManager {
	//friend class ScummDebugger;
	//friend class ScummEngine;
protected:
	ScummEngine *_vm;

public:
	class Resource {
	public:
		/**
		 * Pointer to the data contained in this resource
		 */
		byte *_address;

		/**
		 * Size of this resource, i.e. of the data contained in it.
		 */
		uint32 _size;

	protected:
		/**
		 * The uppermost bit indicates whether the resources is locked.
		 * The lower 7 bits contain a counter. This counter measures roughly
		 * how old the resource is; it starts out with a count of 1 and can go
		 * as high as 127. When memory falls low resp. when the engine decides
		 * that it should throw out some unused stuff, then it begins by
		 * removing the resources with the highest counter (excluding locked
		 * resources and resources that are known to be in use).
		 */
		byte _flags;

		/**
		 * The status of the resource. Currently only one bit is used, which
		 * indicates whether the resource is modified.
		 */
		byte _status;

	public:
		/**
		 * The id of the room (resp. the disk) the resource is contained in.
		 */
		byte _roomno;

		/**
		 * The offset (in bytes) where the data for this resources can be found
		 * in the game data file(s), relative to the start of the room the
		 * resource is contained in.
		 *
		 * A value of RES_INVALID_OFFSET indicates a resources that is not contained
		 * in the game data files.
		 */
		uint32 _roomoffs;

	public:
		Resource();
		~Resource();

		void nuke();

		inline void setResourceCounter(byte counter);
		inline byte getResourceCounter() const;

		void lock();
		void unlock();
		bool isLocked() const;

		// HE specific
		void setModified();
		bool isModified() const;
		void setOffHeap();
		void setOnHeap();
		bool isOffHeap() const;
	};

	/**
	 * This struct represents a resource type and all resource of that type.
	 */
	class ResTypeData : public Common::Array<Resource> {
	friend class ResourceManager;
	public:
		/**
		 * The mode of this res type.
		 */
		ResTypeMode _mode;

		/**
		 * The 4-byte tag or chunk type associated to this resource type, if any.
		 * Only applies to resources that are loaded from the game data files.
		 * This value is only used for debugging purposes.
		 */
		uint32 _tag;

	public:
		ResTypeData();
		~ResTypeData();
	};
	ResTypeData _types[rtLast + 1];

protected:
	uint32 _allocatedSize;
	uint32 _maxHeapThreshold, _minHeapThreshold;
	byte _expireCounter;

public:
	ResourceManager(ScummEngine *vm);
	~ResourceManager();

	void setHeapThreshold(int min, int max);

	void allocResTypeData(ResType type, uint32 tag, int num, ResTypeMode mode);
	void freeResources();

	byte *createResource(ResType type, ResId idx, uint32 size);
	void nukeResource(ResType type, ResId idx);

//	inline Resource &getRes(ResType type, ResId idx) { return _types[type][idx]; }
//	inline const Resource &getRes(ResType type, ResId idx) const { return _types[type][idx]; }

	bool isResourceLoaded(ResType type, ResId idx) const;

	void lock(ResType type, ResId idx);
	void unlock(ResType type, ResId idx);
	bool isLocked(ResType type, ResId idx) const;

	// HE Specific
	void setModified(ResType type, ResId idx);
	bool isModified(ResType type, ResId idx) const;
	void setOffHeap(ResType type, ResId idx);
	void setOnHeap(ResType type, ResId idx);

	/**
	 * This method increments the _expireCounter, and if it overflows (which happens
	 * after at most 256 calls), it calls increaseResourceCounter.
	 * It is invoked in the engine's main loop ScummEngine::scummLoop().
	 */
	void increaseExpireCounter();

	/**
	 * Update the specified resource's counter.
	 */
	void setResourceCounter(ResType type, ResId idx, byte counter);

	/**
	 * Increment the counter of all unlocked loaded resources.
	 * The maximal count is 255.
	 * This is called by increaseExpireCounter and expireResources,
	 * but also by ScummEngine::startScene.
	 */
	void increaseResourceCounters();

	void resourceStats();

//protected:
	bool validateResource(const char *str, ResType type, ResId idx) const;
protected:
	void expireResources(uint32 size);
};

} // End of namespace Scumm

#endif
