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

#ifndef SCI_RESOURCE_H
#define SCI_RESOURCE_H

#include "common/str.h"
#include "common/list.h"
#include "common/hashmap.h"

#include "sci/graphics/helpers.h"		// for ViewType
#include "sci/decompressor.h"
#include "sci/sci.h"

namespace Common {
class File;
class FSList;
class FSNode;
class WriteStream;
class SeekableReadStream;
}

namespace Sci {

enum {
	/** The maximum allowed size for a compressed or decompressed resource */
	SCI_MAX_RESOURCE_SIZE = 0x0400000
};

/** Resource status types */
enum ResourceStatus {
	kResStatusNoMalloc = 0,
	kResStatusAllocated,
	kResStatusEnqueued, /**< In the LRU queue */
	kResStatusLocked /**< Allocated and in use */
};

/** Resource error codes. Should be in sync with s_errorDescriptions */
enum ResourceErrorCodes {
	SCI_ERROR_NONE = 0,
	SCI_ERROR_IO_ERROR = 1,
	SCI_ERROR_EMPTY_RESOURCE = 2,
	SCI_ERROR_RESMAP_INVALID_ENTRY = 3,	/**< Invalid resource.map entry */
	SCI_ERROR_RESMAP_NOT_FOUND = 4,
	SCI_ERROR_NO_RESOURCE_FILES_FOUND = 5,	/**< No resource at all was found */
	SCI_ERROR_UNKNOWN_COMPRESSION = 6,
	SCI_ERROR_DECOMPRESSION_ERROR = 7,	/**< sanity checks failed during decompression */
	SCI_ERROR_RESOURCE_TOO_BIG = 8	/**< Resource size exceeds SCI_MAX_RESOURCE_SIZE */
};

enum {
	MAX_OPENED_VOLUMES = 5 ///< Max number of simultaneously opened volumes
};

enum ResourceType {
	kResourceTypeView = 0,
	kResourceTypePic,
	kResourceTypeScript,
	kResourceTypeText,
	kResourceTypeSound,
	kResourceTypeMemory,
	kResourceTypeVocab,
	kResourceTypeFont,
	kResourceTypeCursor,
	kResourceTypePatch,
	kResourceTypeBitmap,
	kResourceTypePalette,
	kResourceTypeCdAudio,
	kResourceTypeAudio,
	kResourceTypeSync,
	kResourceTypeMessage,
	kResourceTypeMap,
	kResourceTypeHeap,
	kResourceTypeAudio36,
	kResourceTypeSync36,
	kResourceTypeTranslation, // Currently unsupported

	// SCI2.1+ Resources
	kResourceTypeRobot,
	kResourceTypeVMD,
	kResourceTypeChunk,
	kResourceTypeAnimation,

	// SCI3 Resources
	kResourceTypeEtc,
	kResourceTypeDuck,
	kResourceTypeClut,
	kResourceTypeTGA,
	kResourceTypeZZZ,

	// Mac-only resources
	kResourceTypeMacIconBarPictN, // IBIN resources (icon bar, not selected)
	kResourceTypeMacIconBarPictS, // IBIS resources (icon bar, selected)
	kResourceTypeMacPict,        // PICT resources (inventory)

	kResourceTypeInvalid
};

const char *getResourceTypeName(ResourceType restype);

enum ResVersion {
	kResVersionUnknown,
	kResVersionSci0Sci1Early,
	kResVersionSci1Middle,
	kResVersionKQ5FMT,
	kResVersionSci1Late,
	kResVersionSci11,
	kResVersionSci11Mac,
	kResVersionSci2,
	kResVersionSci3
};

class ResourceManager;
class ResourceSource;

class ResourceId {
	static inline ResourceType fixupType(ResourceType type) {
		if (type >= kResourceTypeInvalid)
			return kResourceTypeInvalid;
		return type;
	}

	ResourceType _type;
	uint16 _number;
	uint32 _tuple; // Only used for audio36 and sync36

public:
	ResourceId() : _type(kResourceTypeInvalid), _number(0), _tuple(0) { }

	ResourceId(ResourceType type_, uint16 number_, uint32 tuple_ = 0)
			: _type(fixupType(type_)), _number(number_), _tuple(tuple_) {
	}

	ResourceId(ResourceType type_, uint16 number_, byte noun, byte verb, byte cond, byte seq)
			: _type(fixupType(type_)), _number(number_) {
		_tuple = (noun << 24) | (verb << 16) | (cond << 8) | seq;
	}

	Common::String toString() const {
		char buf[32];

		snprintf(buf, 32, "%s.%d", getResourceTypeName(_type), _number);
		Common::String retStr = buf;

		if (_tuple != 0) {
			snprintf(buf, 32, "(%d, %d, %d, %d)", _tuple >> 24, (_tuple >> 16) & 0xff, (_tuple >> 8) & 0xff, _tuple & 0xff);
			retStr += buf;
		}

		return retStr;
	}

	inline ResourceType getType() const { return _type; }
	inline uint16 getNumber() const { return _number; }
	inline uint32 getTuple() const { return _tuple; }

	inline uint hash() const {
		return ((uint)((_type << 16) | _number)) ^ _tuple;
	}

	bool operator==(const ResourceId &other) const {
		return (_type == other._type) && (_number == other._number) && (_tuple == other._tuple);
	}

	bool operator<(const ResourceId &other) const {
		return (_type < other._type) || ((_type == other._type) && (_number < other._number))
			    || ((_type == other._type) && (_number == other._number) && (_tuple < other._tuple));
	}
};

struct ResourceIdHash : public Common::UnaryFunction<ResourceId, uint> {
	uint operator()(ResourceId val) const { return val.hash(); }
};

/** Class for storing resources in memory */
class Resource {
	friend class ResourceManager;

	// FIXME: These 'friend' declarations are meant to be a temporary hack to
	// ease transition to the ResourceSource class system.
	friend class ResourceSource;
	friend class PatchResourceSource;
	friend class WaveResourceSource;
	friend class AudioVolumeResourceSource;
	friend class MacResourceForkResourceSource;
#ifdef ENABLE_SCI32
	friend class ChunkResourceSource;
#endif

// NOTE : Currently most member variables lack the underscore prefix and have
// public visibility to let the rest of the engine compile without changes.
public:
	byte *data;
	uint32 size;
	byte *_header;
	uint32 _headerSize;

public:
	Resource(ResourceManager *resMan, ResourceId id);
	~Resource();
	void unalloc();

	inline ResourceType getType() const { return _id.getType(); }
	inline uint16 getNumber() const { return _id.getNumber(); }
	bool isLocked() const { return _status == kResStatusLocked; }
	/**
	 * Write the resource to the specified stream.
	 * This method is used only by the "dump" debugger command.
	 */
	void writeToStream(Common::WriteStream *stream) const;

	const Common::String &getResourceLocation() const;

	// FIXME: This audio specific method is a hack. After all, why should a
	// Resource have audio specific methods? But for now we keep this, as it
	// eases transition.
	uint32 getAudioCompressionType() const;

protected:
	ResourceId _id;	// TODO: _id could almost be made const, only readResourceInfo() modifies it...
	int32 _fileOffset; /**< Offset in file */
	ResourceStatus _status;
	uint16 _lockers; /**< Number of places where this resource was locked */
	ResourceSource *_source;
	ResourceManager *_resMan;

	bool loadPatch(Common::SeekableReadStream *file);
	bool loadFromPatchFile();
	bool loadFromWaveFile(Common::SeekableReadStream *file);
	bool loadFromAudioVolumeSCI1(Common::SeekableReadStream *file);
	bool loadFromAudioVolumeSCI11(Common::SeekableReadStream *file);
	int decompress(ResVersion volVersion, Common::SeekableReadStream *file);
	int readResourceInfo(ResVersion volVersion, Common::SeekableReadStream *file, uint32 &szPacked, ResourceCompression &compression);
};

typedef Common::HashMap<ResourceId, Resource *, ResourceIdHash> ResourceMap;

class ResourceManager {
	// FIXME: These 'friend' declarations are meant to be a temporary hack to
	// ease transition to the ResourceSource class system.
	friend class ResourceSource;
	friend class DirectoryResourceSource;
	friend class PatchResourceSource;
	friend class ExtMapResourceSource;
	friend class IntMapResourceSource;
	friend class AudioVolumeResourceSource;
	friend class ExtAudioMapResourceSource;
	friend class WaveResourceSource;
	friend class MacResourceForkResourceSource;
#ifdef ENABLE_SCI32
	friend class ChunkResourceSource;
#endif

public:
	/**
	 * Creates a new SCI resource manager.
	 */
	ResourceManager();
	~ResourceManager();


	/**
	 * Initializes the resource manager.
	 */
	void init(bool initFromFallbackDetector = false);

	int addAppropriateSources();
	int addAppropriateSources(const Common::FSList &fslist);	// TODO: Switch from FSList to Common::Archive?

	/**
	 * Looks up a resource's data.
	 * @param id	The resource type to look for
	 * @param lock	non-zero iff the resource should be locked
	 * @return The resource, or NULL if it doesn't exist
	 * @note Locked resources are guaranteed not to have their contents freed until
	 *       they are unlocked explicitly (by unlockResource).
	 */
	Resource *findResource(ResourceId id, bool lock);

	/**
	 * Unlocks a previously locked resource.
	 * @param res	The resource to free
	 */
	void unlockResource(Resource *res);

	/**
	 * Tests whether a resource exists.
	 *
	 * This function may often be much faster than finding the resource
	 * and should be preferred for simple tests.
	 * The resource object returned is, indeed, the resource in question, but
	 * it should be used with care, as it may be unallocated.
	 * Use scir_find_resource() if you want to use the data contained in the resource.
	 *
	 * @param id	Id of the resource to check
	 * @return		non-NULL if the resource exists, NULL otherwise
	 */
	Resource *testResource(ResourceId id);

	/**
	 * Returns a list of all resources of the specified type.
	 * @param type		The resource type to look for
	 * @param mapNumber	For audio36 and sync36, limit search to this map
	 * @return			The resource list
	 */
	Common::List<ResourceId> listResources(ResourceType type, int mapNumber = -1);

	void setAudioLanguage(int language);
	int getAudioLanguage() const;
	void changeAudioDirectory(Common::String path);
	bool isGMTrackIncluded();
	bool isSci11Mac() const { return _volVersion == kResVersionSci11Mac; }
	ViewType getViewType() const { return _viewType; }
	const char *getMapVersionDesc() const { return versionDescription(_mapVersion); }
	const char *getVolVersionDesc() const { return versionDescription(_volVersion); }
	ResVersion getVolVersion() const { return _volVersion; }

	/**
	 * Adds the appropriate GM patch from the Sierra MIDI utility as 4.pat, without
	 * requiring the user to rename the file to 4.pat. Thus, the original Sierra
	 * archive can be extracted in the extras directory, and the GM patches can be
	 * applied per game, if applicable.
	 */
	void addNewGMPatch(SciGameId gameId);

#ifdef ENABLE_SCI32
	/**
	 * Parses all resources from a SCI2.1 chunk resource and adds them to the
	 * resource manager.
	 */
	void addResourcesFromChunk(uint16 id);
#endif

	bool detectHires();
	// Detects, if standard font of current game includes extended characters (>0x80)
	bool detectFontExtended();
	// Detects, if SCI1.1 game uses palette merging
	bool detectPaletteMergingSci11();
	// Detects, if SCI0EARLY game also has SCI0EARLY sound resources
	bool detectEarlySound();

	/**
	 * Finds the internal Sierra ID of the current game from script 0.
	 */
	Common::String findSierraGameId();

	/**
	 * Finds the location of the game object from script 0.
	 * @param addSci11ScriptOffset	Adjust the return value for SCI1.1 and newer
	 *        games. Needs to be false when the heap is accessed directly inside
	 *        findSierraGameId().
	 */
	reg_t findGameObject(bool addSci11ScriptOffset = true);

	/**
	 * Converts a map resource type to our type
	 * @param sciType The type from the map/patch
	 * @return The ResourceType
	 */
	ResourceType convertResType(byte type);

protected:
	// Maximum number of bytes to allow being allocated for resources
	// Note: maxMemory will not be interpreted as a hard limit, only as a restriction
	// for resources which are not explicitly locked. However, a warning will be
	// issued whenever this limit is exceeded.
	enum {
		MAX_MEMORY = 256 * 1024	// 256KB
	};

	ViewType _viewType; // Used to determine if the game has EGA or VGA graphics
	Common::List<ResourceSource *> _sources;
	int _memoryLocked;	///< Amount of resource bytes in locked memory
	int _memoryLRU;		///< Amount of resource bytes under LRU control
	Common::List<Resource *> _LRU; ///< Last Resource Used list
	ResourceMap _resMap;
	Common::List<Common::File *> _volumeFiles; ///< list of opened volume files
	ResourceSource *_audioMapSCI1; ///< Currently loaded audio map for SCI1
	ResVersion _volVersion; ///< resource.0xx version
	ResVersion _mapVersion; ///< resource.map version

	/**
	 * Add a path to the resource manager's list of sources.
	 * @return a pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addPatchDir(const Common::String &path);

	ResourceSource *findVolume(ResourceSource *map, int volume_nr);

	/**
	 * Adds a source to the resource manager's list of sources.
	 * @param source	The new source to add
	 * @return		A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addSource(ResourceSource *source);

	/**
	 * Add an external (i.e., separate file) map resource to the resource
	 * manager's list of sources.
	 * @param filename	 The name of the volume to add
	 * @param volume_nr  The volume number the map starts at, 0 for <SCI2.1
	 * @return		A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addExternalMap(const Common::String &filename, int volume_nr = 0);

	ResourceSource *addExternalMap(const Common::FSNode *mapFile, int volume_nr = 0);

	/**
	 * Scans newly registered resource sources for resources, earliest addition first.
	 * @param detected_version Pointer to the detected version number,
	 *					 used during startup. May be NULL.
	 * @return One of SCI_ERROR_*.
	 */
	void scanNewSources();

	bool addAudioSources();
	void addScriptChunkSources();
	void freeResourceSources();

	/**
	 * Returns a string describing a ResVersion.
	 * @param version	The resource version
	 * @return		The description of version
	 */
	const char *versionDescription(ResVersion version) const;

	Common::SeekableReadStream *getVolumeFile(ResourceSource *source);
	void loadResource(Resource *res);
	void freeOldResources();
	void addResource(ResourceId resId, ResourceSource *src, uint32 offset, uint32 size = 0);
	Resource *updateResource(ResourceId resId, ResourceSource *src, uint32 size);
	void removeAudioResource(ResourceId resId);

	/**--- Resource map decoding functions ---*/
	ResVersion detectMapVersion();
	ResVersion detectVolVersion();

	/**
	 * Reads the SCI0 resource.map file from a local directory.
	 * @param map The map
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readResourceMapSCI0(ResourceSource *map);

	/**
	 * Reads the SCI1 resource.map file from a local directory.
	 * @param map The map
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readResourceMapSCI1(ResourceSource *map);

	/**
	 * Reads SCI1.1 audio map resources.
	 * @param map The map
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readAudioMapSCI11(ResourceSource *map);

	/**
	 * Reads SCI1 audio map files.
	 * @param map The map
	 * @param unload Unload the map instead of loading it
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readAudioMapSCI1(ResourceSource *map, bool unload = false);

	/**--- Patch management functions ---*/

	/**
	 * Reads patch files from a local directory.
	 */
	void readResourcePatches();
	void readResourcePatchesBase36();
	void processPatch(ResourceSource *source, ResourceType resourceType, uint16 resourceNr, uint32 tuple = 0);

	/**
	 * Process wave files as patches for Audio resources.
	 */
	void readWaveAudioPatches();
	void processWavePatch(ResourceId resourceId, Common::String name);

 	/**
	 * Applies to all versions before 0.000.395 (i.e. KQ4 old, XMAS 1988 and LSL2).
	 * Old SCI versions used two word header for script blocks (first word equal
	 * to 0x82, meaning of the second one unknown). New SCI versions used one
	 * word header.
	 * Also, old SCI versions assign 120 degrees to left & right, and 60 to up
	 * and down. Later versions use an even 90 degree distribution.
	 */
	bool hasOldScriptHeader();

	void printLRU();
	void addToLRU(Resource *res);
	void removeFromLRU(Resource *res);

	ResourceCompression getViewCompression();
	ViewType detectViewType();
	bool hasSci0Voc999();
	bool hasSci1Voc900();
	void detectSciVersion();
};

class SoundResource {
public:
	struct Channel {
		byte number;
		byte poly;
		uint16 prio;
		uint16 size;
		byte *data;
		uint16 curPos;
		long time;
		byte prev;
	};

	struct Track {
		byte type;
		byte channelCount;
		Channel *channels;
		int16 digitalChannelNr;
		uint16 digitalSampleRate;
		uint16 digitalSampleSize;
		uint16 digitalSampleStart;
		uint16 digitalSampleEnd;
	};
public:
	SoundResource(uint32 resNumber, ResourceManager *resMan, SciVersion soundVersion);
	~SoundResource();
#if 0
	Track *getTrackByNumber(uint16 number);
#endif
	Track *getTrackByType(byte type);
	Track *getDigitalTrack();
	int getChannelFilterMask(int hardwareMask, bool wantsRhythm);
	byte getInitialVoiceCount(byte channel);

private:
	SciVersion _soundVersion;
	int _trackCount;
	Track *_tracks;
	Resource *_innerResource;
	ResourceManager *_resMan;
};

} // End of namespace Sci

#endif // SCI_RESOURCE_H
