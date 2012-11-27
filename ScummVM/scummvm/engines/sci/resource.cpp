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

// Resource library

#include "common/file.h"
#include "common/fs.h"
#include "common/macresman.h"
#include "common/textconsole.h"

#include "sci/resource.h"
#include "sci/resource_intern.h"
#include "sci/util.h"

namespace Sci {

enum {
	SCI0_RESMAP_ENTRIES_SIZE = 6,
	SCI1_RESMAP_ENTRIES_SIZE = 6,
	KQ5FMT_RESMAP_ENTRIES_SIZE = 7,
	SCI11_RESMAP_ENTRIES_SIZE = 5
};

/** resource type for SCI1 resource.map file */
struct resource_index_t {
	uint16 wOffset;
	uint16 wSize;
};

//////////////////////////////////////////////////////////////////////

static SciVersion s_sciVersion = SCI_VERSION_NONE;	// FIXME: Move this inside a suitable class, e.g. SciEngine

SciVersion getSciVersion() {
	assert(s_sciVersion != SCI_VERSION_NONE);
	return s_sciVersion;
}

const char *getSciVersionDesc(SciVersion version) {
	switch (version) {
	case SCI_VERSION_NONE:
		return "Invalid SCI version";
	case SCI_VERSION_0_EARLY:
		return "Early SCI0";
	case SCI_VERSION_0_LATE:
		return "Late SCI0";
	case SCI_VERSION_01:
		return "SCI01";
	case SCI_VERSION_1_EGA_ONLY:
		return "SCI1 EGA";
	case SCI_VERSION_1_EARLY:
		return "Early SCI1";
	case SCI_VERSION_1_MIDDLE:
		return "Middle SCI1";
	case SCI_VERSION_1_LATE:
		return "Late SCI1";
	case SCI_VERSION_1_1:
		return "SCI1.1";
	case SCI_VERSION_2:
		return "SCI2";
	case SCI_VERSION_2_1:
		return "SCI2.1";
	case SCI_VERSION_3:
		return "SCI3";
	default:
		return "Unknown";
	}
}

//////////////////////////////////////////////////////////////////////


#undef SCI_REQUIRE_RESOURCE_FILES

//#define SCI_VERBOSE_RESMAN 1

static const char *const s_errorDescriptions[] = {
	"No error",
	"I/O error",
	"Resource is empty (size 0)",
	"resource.map entry is invalid",
	"resource.map file not found",
	"No resource files found",
	"Unknown compression method",
	"Decompression failed: Sanity check failed",
	"Decompression failed: Resource too big"
};

static const char *const s_resourceTypeNames[] = {
	"view", "pic", "script", "text", "sound",
	"memory", "vocab", "font", "cursor",
	"patch", "bitmap", "palette", "cdaudio",
	"audio", "sync", "message", "map", "heap",
	"audio36", "sync36", "xlate", "robot", "vmd",
	"chunk", "animation", "etc", "duck", "clut",
	"tga", "zzz", "macibin", "macibis", "macpict"
};

// Resource type suffixes. Note that the
// suffic of SCI3 scripts has been changed from
// scr to csc
static const char *const s_resourceTypeSuffixes[] = {
	"v56", "p56", "scr", "tex", "snd",
	   "", "voc", "fon", "cur", "pat",
	"bit", "pal", "cda", "aud", "syn",
	"msg", "map", "hep",    "",    "",
	"trn", "rbt", "vmd", "chk",    "",
	"etc", "duk", "clu", "tga", "zzz",
	   "",    "",    ""
};

const char *getResourceTypeName(ResourceType restype) {
	if (restype != kResourceTypeInvalid)
		return s_resourceTypeNames[restype];
	else
		return "invalid";
}

static const ResourceType s_resTypeMapSci0[] = {
	kResourceTypeView, kResourceTypePic, kResourceTypeScript, kResourceTypeText,          // 0x00-0x03
	kResourceTypeSound, kResourceTypeMemory, kResourceTypeVocab, kResourceTypeFont,       // 0x04-0x07
	kResourceTypeCursor, kResourceTypePatch, kResourceTypeBitmap, kResourceTypePalette,   // 0x08-0x0B
	kResourceTypeCdAudio, kResourceTypeAudio, kResourceTypeSync, kResourceTypeMessage,    // 0x0C-0x0F
	kResourceTypeMap, kResourceTypeHeap, kResourceTypeAudio36, kResourceTypeSync36,       // 0x10-0x13
	kResourceTypeTranslation                                                              // 0x14
};

// TODO: 12 should be "Wave", but SCI seems to just store it in Audio resources
static const ResourceType s_resTypeMapSci21[] = {
	kResourceTypeView, kResourceTypePic, kResourceTypeScript, kResourceTypeAnimation,     // 0x00-0x03
	kResourceTypeSound, kResourceTypeEtc, kResourceTypeVocab, kResourceTypeFont,          // 0x04-0x07
	kResourceTypeCursor, kResourceTypePatch, kResourceTypeBitmap, kResourceTypePalette,   // 0x08-0x0B
	kResourceTypeInvalid, kResourceTypeAudio, kResourceTypeSync, kResourceTypeMessage,    // 0x0C-0x0F
	kResourceTypeMap, kResourceTypeHeap, kResourceTypeChunk, kResourceTypeAudio36,        // 0x10-0x13
	kResourceTypeSync36, kResourceTypeTranslation, kResourceTypeRobot, kResourceTypeVMD,  // 0x14-0x17
	kResourceTypeDuck, kResourceTypeClut, kResourceTypeTGA, kResourceTypeZZZ              // 0x18-0x1B
};

ResourceType ResourceManager::convertResType(byte type) {
	type &= 0x7f;

	if (_mapVersion < kResVersionSci2) {
		// SCI0 - SCI2
		if (type < ARRAYSIZE(s_resTypeMapSci0))
			return s_resTypeMapSci0[type];
	} else {
		// SCI2.1+
		if (type < ARRAYSIZE(s_resTypeMapSci21)) {
			// LSL6 hires doesn't have the chunk resource type, to match
			// the resource types of the lowres version, thus we use the
			// older resource types here.
			// PQ4 CD and QFG4 CD are SCI2.1, but use the resource types of the
			// corresponding SCI2 floppy disk versions.
			if (g_sci && (g_sci->getGameId() == GID_LSL6HIRES ||
				g_sci->getGameId() == GID_QFG4 || g_sci->getGameId() == GID_PQ4))
				return s_resTypeMapSci0[type];
			else
				return s_resTypeMapSci21[type];
		}
	}

	return kResourceTypeInvalid;
}

//-- Resource main functions --
Resource::Resource(ResourceManager *resMan, ResourceId id) : _resMan(resMan), _id(id) {
	data = NULL;
	size = 0;
	_fileOffset = 0;
	_status = kResStatusNoMalloc;
	_lockers = 0;
	_source = NULL;
	_header = NULL;
	_headerSize = 0;
}

Resource::~Resource() {
	delete[] data;
	delete[] _header;
	if (_source && _source->getSourceType() == kSourcePatch)
		delete _source;
}

void Resource::unalloc() {
	delete[] data;
	data = NULL;
	_status = kResStatusNoMalloc;
}

void Resource::writeToStream(Common::WriteStream *stream) const {
	stream->writeByte(getType() | 0x80); // 0x80 is required by old sierra sci, otherwise it wont accept the patch file
	stream->writeByte(_headerSize);
	if (_headerSize > 0)
		stream->write(_header, _headerSize);
	stream->write(data, size);
}

uint32 Resource::getAudioCompressionType() const {
	return _source->getAudioCompressionType();
}

uint32 AudioVolumeResourceSource::getAudioCompressionType() const {
	return _audioCompressionType;
}


ResourceSource::ResourceSource(ResSourceType type, const Common::String &name, int volNum, const Common::FSNode *resFile)
 : _sourceType(type), _name(name), _volumeNumber(volNum), _resourceFile(resFile) {
	_scanned = false;
}

ResourceSource::~ResourceSource() {
}

MacResourceForkResourceSource::MacResourceForkResourceSource(const Common::String &name, int volNum)
 : ResourceSource(kSourceMacResourceFork, name, volNum) {
	_macResMan = new Common::MacResManager();
	assert(_macResMan);
}

MacResourceForkResourceSource::~MacResourceForkResourceSource() {
	delete _macResMan;
}

//-- resMan helper functions --

// Resource source list management

ResourceSource *ResourceManager::addExternalMap(const Common::String &filename, int volume_nr) {
	ResourceSource *newsrc = new ExtMapResourceSource(filename, volume_nr);

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addExternalMap(const Common::FSNode *mapFile, int volume_nr) {
	ResourceSource *newsrc = new ExtMapResourceSource(mapFile->getName(), volume_nr, mapFile);

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addSource(ResourceSource *newsrc) {
	assert(newsrc);

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addPatchDir(const Common::String &dirname) {
	ResourceSource *newsrc = new DirectoryResourceSource(dirname);

	_sources.push_back(newsrc);
	return 0;
}

ResourceSource *ResourceManager::findVolume(ResourceSource *map, int volume_nr) {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		ResourceSource *src = (*it)->findVolume(map, volume_nr);
		if (src)
			return src;
	}

	return NULL;
}

// Resource manager constructors and operations

bool Resource::loadPatch(Common::SeekableReadStream *file) {
	Resource *res = this;

	// We assume that the resource type matches res->type
	//  We also assume that the current file position is right at the actual data (behind resourceid/headersize byte)

	res->data = new byte[res->size];

	if (res->_headerSize > 0)
		res->_header = new byte[res->_headerSize];

	if ((res->data == NULL) || ((res->_headerSize > 0) && (res->_header == NULL))) {
		error("Can't allocate %d bytes needed for loading %s", res->size + res->_headerSize, res->_id.toString().c_str());
	}

	unsigned int really_read;
	if (res->_headerSize > 0) {
		really_read = file->read(res->_header, res->_headerSize);
		if (really_read != res->_headerSize)
			error("Read %d bytes from %s but expected %d", really_read, res->_id.toString().c_str(), res->_headerSize);
	}

	really_read = file->read(res->data, res->size);
	if (really_read != res->size)
		error("Read %d bytes from %s but expected %d", really_read, res->_id.toString().c_str(), res->size);

	res->_status = kResStatusAllocated;
	return true;
}

bool Resource::loadFromPatchFile() {
	Common::File file;
	const Common::String &filename = _source->getLocationName();
	if (!file.open(filename)) {
		warning("Failed to open patch file %s", filename.c_str());
		unalloc();
		return false;
	}
	// Skip resourceid and header size byte
	file.seek(2, SEEK_SET);
	return loadPatch(&file);
}

Common::SeekableReadStream *ResourceManager::getVolumeFile(ResourceSource *source) {
	Common::List<Common::File *>::iterator it = _volumeFiles.begin();
	Common::File *file;

	if (source->_resourceFile)
		return source->_resourceFile->createReadStream();

	const char *filename = source->getLocationName().c_str();

	// check if file is already opened
	while (it != _volumeFiles.end()) {
		file = *it;
		if (scumm_stricmp(file->getName(), filename) == 0) {
			// move file to top
			if (it != _volumeFiles.begin()) {
				_volumeFiles.erase(it);
				_volumeFiles.push_front(file);
			}
			return file;
		}
		++it;
	}
	// adding a new file
	file = new Common::File;
	if (file->open(filename)) {
		if (_volumeFiles.size() == MAX_OPENED_VOLUMES) {
			it = --_volumeFiles.end();
			delete *it;
			_volumeFiles.erase(it);
		}
		_volumeFiles.push_front(file);
		return file;
	}
	// failed
	delete file;
	return NULL;
}

void ResourceManager::loadResource(Resource *res) {
	res->_source->loadResource(this, res);
}


void PatchResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	bool result = res->loadFromPatchFile();
	if (!result) {
		// TODO: We used to fallback to the "default" code here if loadFromPatchFile
		// failed, but I am not sure whether that is really appropriate.
		// In fact it looks like a bug to me, so I commented this out for now.
		//ResourceSource::loadResource(res);
	}
}

static Common::Array<uint32> resTypeToMacTags(ResourceType type);

static Common::String intToBase36(uint32 number, int minChar) {
	// Convert from an integer to a base36 string
	Common::String string;

	while (minChar--) {
		int character = number % 36;
		string = ((character < 10) ? (character + '0') : (character + 'A' - 10)) + string;
		number /= 36;
	}

	return string;
}

static Common::String constructPatchNameBase36(ResourceId resId) {
	// Convert from a resource ID to a base36 patch name
	Common::String output;

	output += (resId.getType() == kResourceTypeAudio36) ? '@' : '#'; // Identifier
	output += intToBase36(resId.getNumber(), 3);                     // Map
	output += intToBase36(resId.getTuple() >> 24, 2);                // Noun
	output += intToBase36((resId.getTuple() >> 16) & 0xff, 2);       // Verb
	output += '.';                                                   // Separator
	output += intToBase36((resId.getTuple() >> 8) & 0xff, 2);        // Cond
	output += intToBase36(resId.getTuple() & 0xff, 1);               // Seq

	assert(output.size() == 12); // We should always get 12 characters in the end
	return output;
}

void MacResourceForkResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	ResourceType type = res->getType();
	Common::SeekableReadStream *stream = 0;

	if (type == kResourceTypeAudio36 || type == kResourceTypeSync36) {
		// Handle audio36/sync36, convert back to audio/sync
		stream = _macResMan->getResource(constructPatchNameBase36(res->_id));
	} else {
		// Plain resource handling
		Common::Array<uint32> tagArray = resTypeToMacTags(type);

		for (uint32 i = 0; i < tagArray.size() && !stream; i++)
			stream = _macResMan->getResource(tagArray[i], res->getNumber());
	}

	if (stream)
		decompressResource(stream, res);
}

bool MacResourceForkResourceSource::isCompressableResource(ResourceType type) const {
	// Any types that were not originally an SCI format are not compressed, it seems.
	// (Audio/36 being Mac snd resources here)
	return type != kResourceTypeMacPict && type != kResourceTypeAudio &&
			type != kResourceTypeMacIconBarPictN && type != kResourceTypeMacIconBarPictS &&
			type != kResourceTypeAudio36 && type != kResourceTypeSync &&
			type != kResourceTypeSync36 && type != kResourceTypeCursor;
}

#define OUTPUT_LITERAL() \
	while (literalLength--) \
		*ptr++ = stream->readByte();

#define OUTPUT_COPY() \
	while (copyLength--) { \
		byte value = ptr[-offset]; \
		*ptr++ = value; \
	}

void MacResourceForkResourceSource::decompressResource(Common::SeekableReadStream *stream, Resource *resource) const {
	// KQ6 Mac is the only game not compressed. It's not worth writing a
	// heuristic just for that game. Also, skip over any resource that cannot
	// be compressed.
	bool canBeCompressed = !(g_sci && g_sci->getGameId() == GID_KQ6) && isCompressableResource(resource->_id.getType());
	uint32 uncompressedSize = 0;

	// GK2 Mac is crazy. In its Patches resource fork, picture 2315 is not
	// compressed and it is hardcoded in the executable to say that it's
	// not compressed. Why didn't they just add four zeroes to the end of
	// the resource? (Checked with PPC disasm)
	if (g_sci && g_sci->getGameId() == GID_GK2 && resource->_id.getType() == kResourceTypePic && resource->_id.getNumber() == 2315)
		canBeCompressed = false;

	// Get the uncompressed size from the end of the resource
	if (canBeCompressed && stream->size() > 4) {
		stream->seek(stream->size() - 4);
		uncompressedSize = stream->readUint32BE();
		stream->seek(0);
	}

	if (uncompressedSize == 0) {
		// Not compressed
		resource->size = stream->size();

		// Cut out the 'non-compressed marker' (four zeroes) at the end
		if (canBeCompressed)
			resource->size -= 4;

		resource->data = new byte[resource->size];
		stream->read(resource->data, resource->size);
	} else {
		// Decompress
		resource->size = uncompressedSize;
		resource->data = new byte[uncompressedSize];

		byte *ptr = resource->data;

		while (stream->pos() < stream->size()) {
			byte code = stream->readByte();

			int literalLength = 0, offset = 0, copyLength = 0;
			byte extraByte1 = 0, extraByte2 = 0;

			if (code == 0xFF) {
				// End of stream marker
				break;
			}

			switch (code & 0xC0) {
			case 0x80:
				// Copy chunk expanded
				extraByte1 = stream->readByte();
				extraByte2 = stream->readByte();

				literalLength = extraByte2 & 3;

				OUTPUT_LITERAL()

				offset = ((code & 0x3f) | ((extraByte1 & 0xe0) << 1) | ((extraByte2 & 0xfc) << 7)) + 1;
				copyLength = (extraByte1 & 0x1f) + 3;

				OUTPUT_COPY()
				break;
			case 0xC0:
				// Literal chunk
				if (code >= 0xD0) {
					// These codes cannot be used
					if (code == 0xD0 || code > 0xD3)
						error("Bad Mac compression code %02x", code);

					literalLength = code & 3;
				} else
					literalLength = (code & 0xf) * 4 + 4;

				OUTPUT_LITERAL()
				break;
			default:
				// Copy chunk
				extraByte1 = stream->readByte();

				literalLength = (extraByte1 >> 3) & 0x3;

				OUTPUT_LITERAL()

				offset = (code + ((extraByte1 & 0xE0) << 2)) + 1;
				copyLength = (extraByte1 & 0x7) + 3;

				OUTPUT_COPY()
				break;
			}
		}
	}

	resource->_status = kResStatusAllocated;
	delete stream;
}

#undef OUTPUT_LITERAL
#undef OUTPUT_COPY

Common::SeekableReadStream *ResourceSource::getVolumeFile(ResourceManager *resMan, Resource *res) {
	Common::SeekableReadStream *fileStream = resMan->getVolumeFile(this);

	if (!fileStream) {
		warning("Failed to open %s", getLocationName().c_str());
		if (res)
			res->unalloc();
	}

	return fileStream;
}

void ResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	Common::SeekableReadStream *fileStream = getVolumeFile(resMan, res);
	if (!fileStream)
		return;

	fileStream->seek(res->_fileOffset, SEEK_SET);

	int error = res->decompress(resMan->getVolVersion(), fileStream);
	if (error) {
		warning("Error %d occurred while reading %s from resource file %s: %s",
				error, res->_id.toString().c_str(), res->getResourceLocation().c_str(),
				s_errorDescriptions[error]);
		res->unalloc();
	}

	if (_resourceFile)
		delete fileStream;
}

Resource *ResourceManager::testResource(ResourceId id) {
	return _resMap.getVal(id, NULL);
}

int ResourceManager::addAppropriateSources() {
	Common::ArchiveMemberList files;

	if (Common::File::exists("resource.map")) {
		// SCI0-SCI2 file naming scheme
		ResourceSource *map = addExternalMap("resource.map");

		SearchMan.listMatchingMembers(files, "resource.0??");

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			const Common::String name = (*x)->getName();
			const char *dot = strrchr(name.c_str(), '.');
			int number = atoi(dot + 1);

			addSource(new VolumeResourceSource(name, map, number));
		}
#ifdef ENABLE_SCI32
		// GK1CD hires content
		if (Common::File::exists("alt.map") && Common::File::exists("resource.alt"))
			addSource(new VolumeResourceSource("resource.alt", addExternalMap("alt.map", 10), 10));
#endif
	} else if (Common::MacResManager::exists("Data1")) {
		// Mac SCI1.1+ file naming scheme
		SearchMan.listMatchingMembers(files, "Data?*");

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			Common::String filename = (*x)->getName();
			addSource(new MacResourceForkResourceSource(filename, atoi(filename.c_str() + 4)));
		}

#ifdef ENABLE_SCI32
		// There can also be a "Patches" resource fork with patches
		if (Common::File::exists("Patches"))
			addSource(new MacResourceForkResourceSource("Patches", 100));
	} else {
		// SCI2.1-SCI3 file naming scheme
		Common::ArchiveMemberList mapFiles;
		SearchMan.listMatchingMembers(mapFiles, "resmap.0??");
		SearchMan.listMatchingMembers(files, "ressci.0??");

		// We need to have the same number of maps as resource archives
		if (mapFiles.empty() || files.empty() || mapFiles.size() != files.size())
			return 0;

		for (Common::ArchiveMemberList::const_iterator mapIterator = mapFiles.begin(); mapIterator != mapFiles.end(); ++mapIterator) {
			Common::String mapName = (*mapIterator)->getName();
			int mapNumber = atoi(strrchr(mapName.c_str(), '.') + 1);

			for (Common::ArchiveMemberList::const_iterator fileIterator = files.begin(); fileIterator != files.end(); ++fileIterator) {
				Common::String resName = (*fileIterator)->getName();
				int resNumber = atoi(strrchr(resName.c_str(), '.') + 1);

				if (mapNumber == resNumber) {
					addSource(new VolumeResourceSource(resName, addExternalMap(mapName, mapNumber), mapNumber));
					break;
				}
			}
		}

		// SCI2.1 resource patches
		if (Common::File::exists("resmap.pat") && Common::File::exists("ressci.pat")) {
			// We add this resource with a map which surely won't exist
			addSource(new VolumeResourceSource("ressci.pat", addExternalMap("resmap.pat", 100), 100));
		}
	}
#else
	} else
		return 0;
#endif

	addPatchDir(".");

	if (Common::File::exists("message.map"))
		addSource(new VolumeResourceSource("resource.msg", addExternalMap("message.map"), 0));

	if (Common::File::exists("altres.map"))
		addSource(new VolumeResourceSource("altres.000", addExternalMap("altres.map"), 0));

	return 1;
}

int ResourceManager::addAppropriateSources(const Common::FSList &fslist) {
	ResourceSource *map = 0;
	Common::Array<ResourceSource *> sci21Maps;

#ifdef ENABLE_SCI32
	ResourceSource *sci21PatchMap = 0;
	const Common::FSNode *sci21PatchRes = 0;
#endif

	// First, find resource.map
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.map"))
			map = addExternalMap(file);

		if (filename.contains("resmap.0")) {
			const char *dot = strrchr(file->getName().c_str(), '.');
			uint number = atoi(dot + 1);

			// We need to store each of these maps for use later on
			if (number >= sci21Maps.size())
				sci21Maps.resize(number + 1);

			sci21Maps[number] = addExternalMap(file, number);
		}

#ifdef ENABLE_SCI32
		// SCI2.1 resource patches
		if (filename.contains("resmap.pat"))
			sci21PatchMap = addExternalMap(file, 100);

		if (filename.contains("ressci.pat"))
			sci21PatchRes = file;
#endif
	}

	if (!map && sci21Maps.empty())
		return 0;

#ifdef ENABLE_SCI32
	if (sci21PatchMap && sci21PatchRes)
		addSource(new VolumeResourceSource(sci21PatchRes->getName(), sci21PatchMap, 100, sci21PatchRes));
#endif

	// Now find all the resource.0?? files
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.0")) {
			const char *dot = strrchr(filename.c_str(), '.');
			int number = atoi(dot + 1);

			addSource(new VolumeResourceSource(file->getName(), map, number, file));
		} else if (filename.contains("ressci.0")) {
			const char *dot = strrchr(filename.c_str(), '.');
			int number = atoi(dot + 1);

			// Match this volume to its own map
			addSource(new VolumeResourceSource(file->getName(), sci21Maps[number], number, file));
		}
	}

	// This function is only called by the advanced detector, and we don't really need
	// to add a patch directory or message.map here

	return 1;
}

void ResourceManager::addScriptChunkSources() {
#ifdef ENABLE_SCI32
	if (_mapVersion >= kResVersionSci2) {
		// If we have no scripts, but chunk 0 is present, open up the chunk
		// to try to get to any scripts in there. The Lighthouse SCI2.1 demo
		// does exactly this.

		Common::List<ResourceId> resources = listResources(kResourceTypeScript);

		if (resources.empty() && testResource(ResourceId(kResourceTypeChunk, 0)))
			addResourcesFromChunk(0);
	}
#endif
}

void ResourceManager::scanNewSources() {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		ResourceSource *source = *it;

		if (!source->_scanned) {
			source->_scanned = true;
			source->scanSource(this);
		}
	}
}

void DirectoryResourceSource::scanSource(ResourceManager *resMan) {
	resMan->readResourcePatches();

	// We can't use getSciVersion() at this point, thus using _volVersion
	if (resMan->_volVersion >= kResVersionSci11)	// SCI1.1+
		resMan->readResourcePatchesBase36();

	resMan->readWaveAudioPatches();
}

void ExtMapResourceSource::scanSource(ResourceManager *resMan) {
	if (resMan->_mapVersion < kResVersionSci1Late)
		resMan->readResourceMapSCI0(this);
	else
		resMan->readResourceMapSCI1(this);
}

void ExtAudioMapResourceSource::scanSource(ResourceManager *resMan) {
	resMan->readAudioMapSCI1(this);
}

void IntMapResourceSource::scanSource(ResourceManager *resMan) {
	resMan->readAudioMapSCI11(this);
}

#ifdef ENABLE_SCI32

// Chunk resources are resources that hold other resources. They are normally called
// when using the kLoadChunk SCI2.1 kernel function. However, for example, the Lighthouse
// SCI2.1 demo has a chunk but no scripts outside of the chunk.

// A chunk resource is pretty straightforward in terms of layout
// It begins with 11-byte entries in the header:
// =========
// b resType
// w nEntry
// dw offset
// dw length

ChunkResourceSource::ChunkResourceSource(const Common::String &name, uint16 number)
	: ResourceSource(kSourceChunk, name) {

	_number = 0;
}

void ChunkResourceSource::scanSource(ResourceManager *resMan) {
	Resource *chunk = resMan->findResource(ResourceId(kResourceTypeChunk, _number), false);

	if (!chunk)
		error("Trying to load non-existent chunk");

	byte *ptr = chunk->data;
	uint32 firstOffset = 0;

	for (;;) {
		ResourceType type = resMan->convertResType(*ptr);
		uint16 number = READ_LE_UINT16(ptr + 1);
		ResourceId id(type, number);

		ResourceEntry entry;
		entry.offset = READ_LE_UINT32(ptr + 3);
		entry.length = READ_LE_UINT32(ptr + 7);

		_resMap[id] = entry;
		ptr += 11;

		debugC(kDebugLevelResMan, 2, "Found %s in chunk %d", id.toString().c_str(), _number);

		resMan->updateResource(id, this, entry.length);

		// There's no end marker to the data table, but the first resource
		// begins directly after the entry table. So, when we hit the first
		// resource, we're at the end of the entry table.

		if (!firstOffset)
			firstOffset = entry.offset;

		if ((size_t)(ptr - chunk->data) >= firstOffset)
			break;
	}
}

void ChunkResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	Resource *chunk = resMan->findResource(ResourceId(kResourceTypeChunk, _number), false);

	if (!_resMap.contains(res->_id))
		error("Trying to load non-existent resource from chunk %d: %s %d", _number, getResourceTypeName(res->_id.getType()), res->_id.getNumber());

	ResourceEntry entry = _resMap[res->_id];
	res->data = new byte[entry.length];
	res->size = entry.length;
	res->_header = 0;
	res->_headerSize = 0;
	res->_status = kResStatusAllocated;

	// Copy the resource data over
	memcpy(res->data, chunk->data + entry.offset, entry.length);
}

void ResourceManager::addResourcesFromChunk(uint16 id) {
	addSource(new ChunkResourceSource(Common::String::format("Chunk %d", id), id));
	scanNewSources();
}

#endif

void ResourceManager::freeResourceSources() {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it)
		delete *it;

	_sources.clear();
}

ResourceManager::ResourceManager() {
}

void ResourceManager::init(bool initFromFallbackDetector) {
	_memoryLocked = 0;
	_memoryLRU = 0;
	_LRU.clear();
	_resMap.clear();
	_audioMapSCI1 = NULL;

	// FIXME: put this in an Init() function, so that we can error out if detection fails completely

	_mapVersion = detectMapVersion();
	_volVersion = detectVolVersion();

	// TODO/FIXME: Remove once SCI3 resource detection is finished
	if ((_mapVersion == kResVersionSci3 || _volVersion == kResVersionSci3) && (_mapVersion != _volVersion)) {
		warning("FIXME: Incomplete SCI3 detection: setting map and volume version to SCI3");
		_mapVersion = _volVersion = kResVersionSci3;
	}

	if ((_volVersion == kResVersionUnknown) && (_mapVersion != kResVersionUnknown)) {
		warning("Volume version not detected, but map version has been detected. Setting volume version to map version");
		_volVersion = _mapVersion;
	}

	if ((_mapVersion == kResVersionUnknown) && (_volVersion != kResVersionUnknown)) {
		warning("Map version not detected, but volume version has been detected. Setting map version to volume version");
		_mapVersion = _volVersion;
	}

	debugC(1, kDebugLevelResMan, "resMan: Detected resource map version %d: %s", _mapVersion, versionDescription(_mapVersion));
	debugC(1, kDebugLevelResMan, "resMan: Detected volume version %d: %s", _volVersion, versionDescription(_volVersion));

	if ((_mapVersion == kResVersionUnknown) && (_volVersion == kResVersionUnknown)) {
		warning("Volume and map version not detected, assuming that this is not a sci game");
		_viewType = kViewUnknown;
		return;
	}

	scanNewSources();

	if (!initFromFallbackDetector) {
		if (!addAudioSources()) {
			// FIXME: This error message is not always correct.
			// OTOH, it is nice to be able to detect missing files/sources
			// So we should definitely fix addAudioSources so this error
			// only pops up when necessary. Disabling for now.
			//error("Somehow I can't seem to find the sound files I need (RESOURCE.AUD/RESOURCE.SFX), aborting");
		}
		addScriptChunkSources();
		scanNewSources();
	}

	detectSciVersion();

	debugC(1, kDebugLevelResMan, "resMan: Detected %s", getSciVersionDesc(getSciVersion()));

	switch (_viewType) {
	case kViewEga:
		debugC(1, kDebugLevelResMan, "resMan: Detected EGA graphic resources");
		break;
	case kViewAmiga:
		debugC(1, kDebugLevelResMan, "resMan: Detected Amiga ECS graphic resources");
		break;
	case kViewAmiga64:
		debugC(1, kDebugLevelResMan, "resMan: Detected Amiga AGA graphic resources");
		break;
	case kViewVga:
		debugC(1, kDebugLevelResMan, "resMan: Detected VGA graphic resources");
		break;
	case kViewVga11:
		debugC(1, kDebugLevelResMan, "resMan: Detected SCI1.1 VGA graphic resources");
		break;
	default:
#ifdef ENABLE_SCI32
		error("resMan: Couldn't determine view type");
#else
		if (getSciVersion() >= SCI_VERSION_2) {
			// SCI support isn't built in, thus the view type won't be determined for
			// SCI2+ games. This will be handled further up, so throw no error here
		} else {
			error("resMan: Couldn't determine view type");
		}
#endif
	}
}

ResourceManager::~ResourceManager() {
	// freeing resources
	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		delete itr->_value;
		++itr;
	}
	freeResourceSources();

	Common::List<Common::File *>::iterator it = _volumeFiles.begin();
	while (it != _volumeFiles.end()) {
		delete *it;
		++it;
	}
}

void ResourceManager::removeFromLRU(Resource *res) {
	if (res->_status != kResStatusEnqueued) {
		warning("resMan: trying to remove resource that isn't enqueued");
		return;
	}
	_LRU.remove(res);
	_memoryLRU -= res->size;
	res->_status = kResStatusAllocated;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->_status != kResStatusAllocated) {
		warning("resMan: trying to enqueue resource with state %d", res->_status);
		return;
	}
	_LRU.push_front(res);
	_memoryLRU += res->size;
#if SCI_VERBOSE_RESMAN
	debug("Adding %s.%03d (%d bytes) to lru control: %d bytes total",
	      getResourceTypeName(res->type), res->number, res->size,
	      mgr->_memoryLRU);
#endif
	res->_status = kResStatusEnqueued;
}

void ResourceManager::printLRU() {
	int mem = 0;
	int entries = 0;
	Common::List<Resource *>::iterator it = _LRU.begin();
	Resource *res;

	while (it != _LRU.end()) {
		res = *it;
		debug("\t%s: %d bytes", res->_id.toString().c_str(), res->size);
		mem += res->size;
		++entries;
		++it;
	}

	debug("Total: %d entries, %d bytes (mgr says %d)", entries, mem, _memoryLRU);
}

void ResourceManager::freeOldResources() {
	while (MAX_MEMORY < _memoryLRU) {
		assert(!_LRU.empty());
		Resource *goner = *_LRU.reverse_begin();
		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_RESMAN
		debug("resMan-debug: LRU: Freeing %s.%03d (%d bytes)", getResourceTypeName(goner->type), goner->number, goner->size);
#endif
	}
}

Common::List<ResourceId> ResourceManager::listResources(ResourceType type, int mapNumber) {
	Common::List<ResourceId> resources;

	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		if ((itr->_value->getType() == type) && ((mapNumber == -1) || (itr->_value->getNumber() == mapNumber)))
			resources.push_back(itr->_value->_id);
		++itr;
	}

	return resources;
}

Resource *ResourceManager::findResource(ResourceId id, bool lock) {
	Resource *retval = testResource(id);

	if (!retval)
		return NULL;

	if (retval->_status == kResStatusNoMalloc)
		loadResource(retval);
	else if (retval->_status == kResStatusEnqueued)
		removeFromLRU(retval);
	// Unless an error occurred, the resource is now either
	// locked or allocated, but never queued or freed.

	freeOldResources();

	if (lock) {
		if (retval->_status == kResStatusAllocated) {
			retval->_status = kResStatusLocked;
			retval->_lockers = 0;
			_memoryLocked += retval->size;
		}
		retval->_lockers++;
	} else if (retval->_status != kResStatusLocked) { // Don't lock it
		if (retval->_status == kResStatusAllocated)
			addToLRU(retval);
	}

	if (retval->data)
		return retval;
	else {
		warning("resMan: Failed to read %s", retval->_id.toString().c_str());
		return NULL;
	}
}

void ResourceManager::unlockResource(Resource *res) {
	assert(res);

	if (res->_status != kResStatusLocked) {
		debugC(kDebugLevelResMan, 2, "[resMan] Attempt to unlock unlocked resource %s", res->_id.toString().c_str());
		return;
	}

	if (!--res->_lockers) { // No more lockers?
		res->_status = kResStatusAllocated;
		_memoryLocked -= res->size;
		addToLRU(res);
	}

	freeOldResources();
}

const char *ResourceManager::versionDescription(ResVersion version) const {
	switch (version) {
	case kResVersionUnknown:
		return "Unknown";
	case kResVersionSci0Sci1Early:
		return "SCI0 / Early SCI1";
	case kResVersionSci1Middle:
		return "Middle SCI1";
	case kResVersionKQ5FMT:
		return "KQ5 FM Towns";
	case kResVersionSci1Late:
		return "Late SCI1";
	case kResVersionSci11:
		return "SCI1.1";
	case kResVersionSci11Mac:
		return "Mac SCI1.1+";
	case kResVersionSci2:
		return "SCI2/2.1";
	case kResVersionSci3:
		return "SCI3";
	}

	return "Version not valid";
}

ResVersion ResourceManager::detectMapVersion() {
	Common::SeekableReadStream *fileStream = 0;
	byte buff[6];
	ResourceSource *rsrc= 0;

	// TODO: Add SCI3 support

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->getSourceType() == kSourceExtMap) {
			if (rsrc->_resourceFile) {
				fileStream = rsrc->_resourceFile->createReadStream();
			} else {
				Common::File *file = new Common::File();
				file->open(rsrc->getLocationName());
				if (file->isOpen())
					fileStream = file;
			}
			break;
		} else if (rsrc->getSourceType() == kSourceMacResourceFork) {
			delete fileStream;
			return kResVersionSci11Mac;
		}
	}

	if (!fileStream) {
		warning("Failed to open resource map file");
		return kResVersionUnknown;
	}

	// detection
	// SCI0 and SCI01 maps have last 6 bytes set to FF
	fileStream->seek(-4, SEEK_END);
	uint32 uEnd = fileStream->readUint32LE();
	if (uEnd == 0xFFFFFFFF) {
		// check if the last 7 bytes are all ff, indicating a KQ5 FM-Towns map
		fileStream->seek(-7, SEEK_END);
		fileStream->read(buff, 3);
		if (buff[0] == 0xff && buff[1] == 0xff && buff[2] == 0xff) {
			delete fileStream;
			return kResVersionKQ5FMT;
		}

		// check if 0 or 01 - try to read resources in SCI0 format and see if exists
		fileStream->seek(0, SEEK_SET);
		while (fileStream->read(buff, 6) == 6 && !(buff[0] == 0xFF && buff[1] == 0xFF && buff[2] == 0xFF)) {
			if (findVolume(rsrc, (buff[5] & 0xFC) >> 2) == NULL) {
				delete fileStream;
				return kResVersionSci1Middle;
			}
		}
		delete fileStream;
		return kResVersionSci0Sci1Early;
	}

	// SCI1 and SCI1.1 maps consist of a fixed 3-byte header, a directory list (3-bytes each) that has one entry
	// of id FFh and points to EOF. The actual entries have 6-bytes on SCI1 and 5-bytes on SCI1.1
	byte directoryType = 0;
	uint16 directoryOffset = 0;
	uint16 lastDirectoryOffset = 0;
	uint16 directorySize = 0;
	ResVersion mapDetected = kResVersionUnknown;
	fileStream->seek(0, SEEK_SET);

	while (!fileStream->eos()) {
		directoryType = fileStream->readByte();
		directoryOffset = fileStream->readUint16LE();

		// Only SCI32 has directory type < 0x80
		if (directoryType < 0x80 && (mapDetected == kResVersionUnknown || mapDetected == kResVersionSci2))
			mapDetected = kResVersionSci2;
		else if (directoryType < 0x80 || ((directoryType & 0x7f) > 0x20 && directoryType != 0xFF))
			break;

		// Offset is above file size? -> definitely not SCI1/SCI1.1
		if (directoryOffset > fileStream->size())
			break;

		if (lastDirectoryOffset && mapDetected == kResVersionUnknown) {
			directorySize = directoryOffset - lastDirectoryOffset;
			if ((directorySize % 5) && (directorySize % 6 == 0))
				mapDetected = kResVersionSci1Late;
			if ((directorySize % 5 == 0) && (directorySize % 6))
				mapDetected = kResVersionSci11;
		}

		if (directoryType == 0xFF) {
			// FFh entry needs to point to EOF
			if (directoryOffset != fileStream->size())
				break;

			delete fileStream;

			if (mapDetected)
				return mapDetected;
			return kResVersionSci1Late;
		}

		lastDirectoryOffset = directoryOffset;
	}

	delete fileStream;

	return kResVersionUnknown;
}

ResVersion ResourceManager::detectVolVersion() {
	Common::SeekableReadStream *fileStream = 0;
	ResourceSource *rsrc;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->getSourceType() == kSourceVolume) {
			if (rsrc->_resourceFile) {
				fileStream = rsrc->_resourceFile->createReadStream();
			} else {
				Common::File *file = new Common::File();
				file->open(rsrc->getLocationName());
				if (file->isOpen())
					fileStream = file;
			}
			break;
		} else if (rsrc->getSourceType() == kSourceMacResourceFork)
			return kResVersionSci11Mac;
	}

	if (!fileStream) {
		warning("Failed to open volume file - if you got resource.p01/resource.p02/etc. files, merge them together into resource.000");
		// resource.p01/resource.p02/etc. may be there when directly copying the files from the original floppies
		// the sierra installer would merge those together (perhaps we could do this as well?)
		// possible TODO
		// example for such game: Laura Bow 2
		return kResVersionUnknown;
	}

	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format:   {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	// Try to parse volume with SCI0 scheme to see if it make sense
	// Checking 1MB of data should be enough to determine the version
	uint16 wCompression;
	uint32 dwPacked, dwUnpacked;
	ResVersion curVersion = kResVersionSci0Sci1Early;
	bool failed = false;
	bool sci11Align = false;

	// Check for SCI0, SCI1, SCI1.1, SCI32 v2 (Gabriel Knight 1 CD) and SCI32 v3 (LSL7) formats
	while (!fileStream->eos() && fileStream->pos() < 0x100000) {
		if (curVersion > kResVersionSci0Sci1Early)
			fileStream->readByte();
		fileStream->skip(2);	// resId
		dwPacked = (curVersion < kResVersionSci2) ? fileStream->readUint16LE() : fileStream->readUint32LE();
		dwUnpacked = (curVersion < kResVersionSci2) ? fileStream->readUint16LE() : fileStream->readUint32LE();

		// The compression field is present, but bogus when
		// loading SCI3 volumes, the format is otherwise
		// identical to SCI2. We therefore get the compression
		// indicator here, but disregard it in the following
		// code.
		wCompression = fileStream->readUint16LE();

		if (fileStream->eos()) {
			delete fileStream;
			return curVersion;
		}

		int chk;

		if (curVersion == kResVersionSci0Sci1Early)
			chk = 4;
		else if (curVersion < kResVersionSci2)
			chk = 20;
		else
			chk = 32; // We don't need this, but include it for completeness

		int offs = curVersion < kResVersionSci11 ? 4 : 0;
		if ((curVersion < kResVersionSci2 && wCompression > chk)
				|| (curVersion == kResVersionSci2 && wCompression != 0 && wCompression != 32)
				|| (wCompression == 0 && dwPacked != dwUnpacked + offs)
		        || (dwUnpacked < dwPacked - offs)) {

			// Retry with a newer SCI version
			if (curVersion == kResVersionSci0Sci1Early) {
				curVersion = kResVersionSci1Late;
			} else if (curVersion == kResVersionSci1Late) {
				curVersion = kResVersionSci11;
			} else if (curVersion == kResVersionSci11 && !sci11Align) {
				// Later versions (e.g. QFG1VGA) have resources word-aligned
				sci11Align = true;
			} else if (curVersion == kResVersionSci11) {
				curVersion = kResVersionSci2;
			} else if (curVersion == kResVersionSci2) {
				curVersion = kResVersionSci3;
			} else {
				// All version checks failed, exit loop
				failed = true;
				break;
			}

			fileStream->seek(0);
			continue;
		}

		if (curVersion < kResVersionSci11)
			fileStream->seek(dwPacked - 4, SEEK_CUR);
		else if (curVersion == kResVersionSci11)
			fileStream->seek(sci11Align && ((9 + dwPacked) % 2) ? dwPacked + 1 : dwPacked, SEEK_CUR);
		else if (curVersion >= kResVersionSci2)
			fileStream->seek(dwPacked, SEEK_CUR);
	}

	delete fileStream;

	if (!failed)
		return curVersion;

	// Failed to detect volume version
	return kResVersionUnknown;
}

// version-agnostic patch application
void ResourceManager::processPatch(ResourceSource *source, ResourceType resourceType, uint16 resourceNr, uint32 tuple) {
	Common::SeekableReadStream *fileStream = 0;
	Resource *newrsc = 0;
	ResourceId resId = ResourceId(resourceType, resourceNr, tuple);
	ResourceType checkForType = resourceType;

	// base36 encoded patches (i.e. audio36 and sync36) have the same type as their non-base36 encoded counterparts
	if (checkForType == kResourceTypeAudio36)
		checkForType = kResourceTypeAudio;
	else if (checkForType == kResourceTypeSync36)
		checkForType = kResourceTypeSync;

	if (source->_resourceFile) {
		fileStream = source->_resourceFile->createReadStream();
	} else {
		Common::File *file = new Common::File();
		if (!file->open(source->getLocationName())) {
			warning("ResourceManager::processPatch(): failed to open %s", source->getLocationName().c_str());
			return;
		}
		fileStream = file;
	}

	int fsize = fileStream->size();
	if (fsize < 3) {
		debug("Patching %s failed - file too small", source->getLocationName().c_str());
		return;
	}

	byte patchType = convertResType(fileStream->readByte());
	byte patchDataOffset = fileStream->readByte();

	delete fileStream;

	if (patchType != checkForType) {
		debug("Patching %s failed - resource type mismatch", source->getLocationName().c_str());
		return;
	}

	// Fixes SQ5/German, patch file special case logic taken from SCI View disassembly
	if (patchDataOffset & 0x80) {
		switch (patchDataOffset & 0x7F) {
			case 0:
				patchDataOffset = 24;
				break;
			case 1:
				patchDataOffset = 2;
				break;
			case 4:
				patchDataOffset = 8;
				break;
			default:
				error("Resource patch unsupported special case %X", patchDataOffset & 0x7F);
				return;
		}
	}

	if (patchDataOffset + 2 >= fsize) {
		debug("Patching %s failed - patch starting at offset %d can't be in file of size %d",
		      source->getLocationName().c_str(), patchDataOffset + 2, fsize);
		return;
	}

	// Overwrite everything, because we're patching
	newrsc = updateResource(resId, source, fsize - patchDataOffset - 2);
	newrsc->_headerSize = patchDataOffset;
	newrsc->_fileOffset = 0;


	debugC(1, kDebugLevelResMan, "Patching %s - OK", source->getLocationName().c_str());
}

static ResourceId convertPatchNameBase36(ResourceType type, const Common::String &filename) {
	// The base36 encoded resource contains the following:
	// uint16 resourceId, byte noun, byte verb, byte cond, byte seq

	// Skip patch type character
	uint16 resourceNr = strtol(Common::String(filename.c_str() + 1, 3).c_str(), 0, 36); // 3 characters
	uint16 noun = strtol(Common::String(filename.c_str() + 4, 2).c_str(), 0, 36);       // 2 characters
	uint16 verb = strtol(Common::String(filename.c_str() + 6, 2).c_str(), 0, 36);       // 2 characters
	// Skip '.'
	uint16 cond = strtol(Common::String(filename.c_str() + 9, 2).c_str(), 0, 36);       // 2 characters
	uint16 seq = strtol(Common::String(filename.c_str() + 11, 1).c_str(), 0, 36);       // 1 character

	return ResourceId(type, resourceNr, noun, verb, cond, seq);
}

void ResourceManager::readResourcePatchesBase36() {
	// The base36 encoded audio36 and sync36 resources use a different naming scheme, because they
	// cannot be described with a single resource number, but are a result of a
	// <number, noun, verb, cond, seq> tuple. Please don't be confused with the normal audio patches
	// (*.aud) and normal sync patches (*.syn). audio36 patches can be seen for example in the AUD
	// folder of GK1CD, and are like this file: @0CS0M00.0X1. GK1CD is the first game where these
	// have been observed. The actual audio36 and sync36 resources exist in SCI1.1 as well, but the
	// first game where external patch files for them have been found is GK1CD. The names of these
	// files are base36 encoded, and we handle their decoding here. audio36 files start with a '@',
	// whereas sync36 start with a '#'. Mac versions begin with 'A' (probably meaning AIFF). Torin
	// has several that begin with 'B'.

	Common::String name, inputName;
	Common::ArchiveMemberList files;
	ResourceSource *psrcPatch;

	for (int i = kResourceTypeAudio36; i <= kResourceTypeSync36; ++i) {
		files.clear();

		// audio36 resources start with a @, A, or B
		// sync36 resources start with a #
		if (i == kResourceTypeAudio36) {
			SearchMan.listMatchingMembers(files, "@???????.???");
			SearchMan.listMatchingMembers(files, "A???????.???");
			SearchMan.listMatchingMembers(files, "B???????.???");
		} else
			SearchMan.listMatchingMembers(files, "#???????.???");

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			name = (*x)->getName();

			ResourceId resource36 = convertPatchNameBase36((ResourceType)i, name);

			/*
			if (i == kResourceTypeAudio36)
				debug("audio36 patch: %s => %s. tuple:%d, %s\n", name.c_str(), inputName.c_str(), resource36.tuple, resource36.toString().c_str());
			else
				debug("sync36 patch: %s => %s. tuple:%d, %s\n", name.c_str(), inputName.c_str(), resource36.tuple, resource36.toString().c_str());
			*/

			// Make sure that the audio patch is a valid resource
			if (i == kResourceTypeAudio36) {
				Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(name);
				uint32 tag = stream->readUint32BE();

				if (tag == MKTAG('R','I','F','F') || tag == MKTAG('F','O','R','M')) {
					delete stream;
					processWavePatch(resource36, name);
					continue;
				}

				// Check for SOL as well
				tag = (tag << 16) | stream->readUint16BE();

				if (tag != MKTAG('S','O','L',0)) {
					delete stream;
					continue;
				}

				delete stream;
			}

			psrcPatch = new PatchResourceSource(name);
			processPatch(psrcPatch, (ResourceType)i, resource36.getNumber(), resource36.getTuple());
		}
	}
}

void ResourceManager::readResourcePatches() {
	// Note: since some SCI1 games(KQ5 floppy, SQ4) might use SCI0 naming scheme for patch files
	// this function tries to read patch file with any supported naming scheme,
	// regardless of s_sciVersion value

	Common::String mask, name;
	Common::ArchiveMemberList files;
	uint16 resourceNr = 0;
	const char *szResType;
	ResourceSource *psrcPatch;

	for (int i = kResourceTypeView; i < kResourceTypeInvalid; ++i) {
		// Ignore the types that can't be patched (and Robot/VMD is handled externally for now)
		if (!s_resourceTypeSuffixes[i] || (i >= kResourceTypeRobot && i != kResourceTypeChunk))
			continue;

		files.clear();
		szResType = getResourceTypeName((ResourceType)i);
		// SCI0 naming - type.nnn
		mask = szResType;
		mask += ".???";
		SearchMan.listMatchingMembers(files, mask);
		// SCI1 and later naming - nnn.typ
		mask = "*.";
		mask += s_resourceTypeSuffixes[i];
		SearchMan.listMatchingMembers(files, mask);

		if (i == kResourceTypeView) {
			SearchMan.listMatchingMembers(files, "*.v16");	// EGA SCI1 view patches
			SearchMan.listMatchingMembers(files, "*.v32");	// Amiga SCI1 view patches
			SearchMan.listMatchingMembers(files, "*.v64");	// Amiga AGA SCI1 (i.e. Longbow) view patches
		} else if (i == kResourceTypePic) {
			SearchMan.listMatchingMembers(files, "*.p16");	// EGA SCI1 picture patches
			SearchMan.listMatchingMembers(files, "*.p32");	// Amiga SCI1 picture patches
			SearchMan.listMatchingMembers(files, "*.p64");	// Amiga AGA SCI1 (i.e. Longbow) picture patches
		} else if (i == kResourceTypeScript) {
			if (files.size() == 0)
				// SCI3 (we can't use getSciVersion() at this point)
				SearchMan.listMatchingMembers(files, "*.csc");
		}

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			bool bAdd = false;
			name = (*x)->getName();

			// SCI1 scheme
			if (Common::isDigit(name[0])) {
				char *end = 0;
				resourceNr = strtol(name.c_str(), &end, 10);
				bAdd = (*end == '.'); // Ensure the next character is the period
			} else {
				// SCI0 scheme
				int resname_len = strlen(szResType);
				if (scumm_strnicmp(name.c_str(), szResType, resname_len) == 0
					&& !Common::isAlpha(name[resname_len + 1])) {
					resourceNr = atoi(name.c_str() + resname_len + 1);
					bAdd = true;
				}
			}

			if (bAdd) {
				psrcPatch = new PatchResourceSource(name);
				processPatch(psrcPatch, (ResourceType)i, resourceNr);
			}
		}
	}
}

int ResourceManager::readResourceMapSCI0(ResourceSource *map) {
	Common::SeekableReadStream *fileStream = 0;
	ResourceType type = kResourceTypeInvalid;	// to silence a false positive in MSVC
	uint16 number, id;
	uint32 offset;

	if (map->_resourceFile) {
		fileStream = map->_resourceFile->createReadStream();
		if (!fileStream)
			return SCI_ERROR_RESMAP_NOT_FOUND;
	} else {
		Common::File *file = new Common::File();
		if (!file->open(map->getLocationName()))
			return SCI_ERROR_RESMAP_NOT_FOUND;
		fileStream = file;
	}

	fileStream->seek(0, SEEK_SET);

	byte bMask = (_mapVersion >= kResVersionSci1Middle) ? 0xF0 : 0xFC;
	byte bShift = (_mapVersion >= kResVersionSci1Middle) ? 28 : 26;

	do {
		// King's Quest 5 FM-Towns uses a 7 byte version of the SCI1 Middle map,
		// splitting the type from the id.
		if (_mapVersion == kResVersionKQ5FMT)
			type = convertResType(fileStream->readByte());

		id = fileStream->readUint16LE();
		offset = fileStream->readUint32LE();

		if (fileStream->eos() || fileStream->err()) {
			delete fileStream;
			warning("Error while reading %s", map->getLocationName().c_str());
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}

		if (offset == 0xFFFFFFFF)
			break;

		if (_mapVersion == kResVersionKQ5FMT) {
			number = id;
		} else {
			type = convertResType(id >> 11);
			number = id & 0x7FF;
		}

		ResourceId resId = ResourceId(type, number);
		// adding a new resource
		if (_resMap.contains(resId) == false) {
			ResourceSource *source = findVolume(map, offset >> bShift);
			if (!source) {
				warning("Could not get volume for resource %d, VolumeID %d", id, offset >> bShift);
				if (_mapVersion != _volVersion) {
					warning("Retrying with the detected volume version instead");
					warning("Map version was: %d, retrying with: %d", _mapVersion, _volVersion);
					_mapVersion = _volVersion;
					bMask = (_mapVersion == kResVersionSci1Middle) ? 0xF0 : 0xFC;
					bShift = (_mapVersion == kResVersionSci1Middle) ? 28 : 26;
					source = findVolume(map, offset >> bShift);
				}
			}

			addResource(resId, source, offset & (((~bMask) << 24) | 0xFFFFFF));
		}
	} while (!fileStream->eos());

	delete fileStream;
	return 0;
}

int ResourceManager::readResourceMapSCI1(ResourceSource *map) {
	Common::SeekableReadStream *fileStream = 0;

	if (map->_resourceFile) {
		fileStream = map->_resourceFile->createReadStream();
		if (!fileStream)
			return SCI_ERROR_RESMAP_NOT_FOUND;
	} else {
		Common::File *file = new Common::File();
		if (!file->open(map->getLocationName()))
			return SCI_ERROR_RESMAP_NOT_FOUND;
		fileStream = file;
	}

	resource_index_t resMap[32];
	memset(resMap, 0, sizeof(resource_index_t) * 32);
	byte type = 0, prevtype = 0;
	byte nEntrySize = _mapVersion == kResVersionSci11 ? SCI11_RESMAP_ENTRIES_SIZE : SCI1_RESMAP_ENTRIES_SIZE;
	ResourceId resId;

	// Read resource type and offsets to resource offsets block from .MAP file
	// The last entry has type=0xFF (0x1F) and offset equals to map file length
	do {
		type = fileStream->readByte() & 0x1F;
		resMap[type].wOffset = fileStream->readUint16LE();
		resMap[prevtype].wSize = (resMap[type].wOffset
		                          - resMap[prevtype].wOffset) / nEntrySize;
		prevtype = type;
	} while (type != 0x1F); // the last entry is FF

	// reading each type's offsets
	uint32 fileOffset = 0;
	for (type = 0; type < 32; type++) {
		if (resMap[type].wOffset == 0) // this resource does not exist in map
			continue;
		fileStream->seek(resMap[type].wOffset);
		for (int i = 0; i < resMap[type].wSize; i++) {
			uint16 number = fileStream->readUint16LE();
			int volume_nr = 0;
			if (_mapVersion == kResVersionSci11) {
				// offset stored in 3 bytes
				fileOffset = fileStream->readUint16LE();
				fileOffset |= fileStream->readByte() << 16;
				fileOffset <<= 1;
			} else {
				// offset/volume stored in 4 bytes
				fileOffset = fileStream->readUint32LE();
				if (_mapVersion < kResVersionSci11) {
					volume_nr = fileOffset >> 28; // most significant 4 bits
					fileOffset &= 0x0FFFFFFF;     // least significant 28 bits
				} else {
					// in SCI32 it's a plain offset
				}
			}
			if (fileStream->eos() || fileStream->err()) {
				delete fileStream;
				warning("Error while reading %s", map->getLocationName().c_str());
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}
			resId = ResourceId(convertResType(type), number);
			// NOTE: We add the map's volume number here to the specified volume number
			// for SCI2.1 and SCI3 maps that are not resmap.000. The resmap.* files' numbers
			// need to be used in concurrence with the volume specified in the map to get
			// the actual resource file.
			int mapVolumeNr = volume_nr + map->_volumeNumber;
			ResourceSource *source = findVolume(map, mapVolumeNr);

			assert(source);

			Resource *resource = _resMap.getVal(resId, NULL);
			if (!resource) {
				addResource(resId, source, fileOffset);
			} else {
				// If the resource is already present in a volume, change it to
				// the new content (but only in a volume, so as not to overwrite
				// external patches - refer to bug #3366295).
				// This is needed at least for the German version of Pharkas.
				// That version contains several duplicate resources INSIDE the
				// resource data files like fonts, views, scripts, etc. Thus,
				// if we use the first entries in the resource file, half of the
				// game will be English and umlauts will also be missing :P
				if (resource->_source->getSourceType() == kSourceVolume) {
					resource->_source = source;
					resource->_fileOffset = fileOffset;
					resource->size = 0;
				}
			}
		}
	}

	delete fileStream;
	return 0;
}

struct MacResTag {
	uint32 tag;
	ResourceType type;
};

static const MacResTag macResTagMap[] = {
	{ MKTAG('V','5','6',' '), kResourceTypeView },
	{ MKTAG('P','5','6',' '), kResourceTypePic },
	{ MKTAG('S','C','R',' '), kResourceTypeScript },
	{ MKTAG('T','E','X',' '), kResourceTypeText },
	{ MKTAG('S','N','D',' '), kResourceTypeSound },
	{ MKTAG('V','O','C',' '), kResourceTypeVocab },
	{ MKTAG('F','O','N',' '), kResourceTypeFont },
	{ MKTAG('C','U','R','S'), kResourceTypeCursor },
	{ MKTAG('c','r','s','r'), kResourceTypeCursor },
	{ MKTAG('P','a','t',' '), kResourceTypePatch },
	{ MKTAG('P','A','L',' '), kResourceTypePalette },
	{ MKTAG('s','n','d',' '), kResourceTypeAudio },
	{ MKTAG('M','S','G',' '), kResourceTypeMessage },
	{ MKTAG('H','E','P',' '), kResourceTypeHeap },
	{ MKTAG('I','B','I','N'), kResourceTypeMacIconBarPictN },
	{ MKTAG('I','B','I','S'), kResourceTypeMacIconBarPictS },
	{ MKTAG('P','I','C','T'), kResourceTypeMacPict },
	{ MKTAG('S','Y','N',' '), kResourceTypeSync },
	{ MKTAG('S','Y','N','C'), kResourceTypeSync }
};

static Common::Array<uint32> resTypeToMacTags(ResourceType type) {
	Common::Array<uint32> tags;

	for (uint32 i = 0; i < ARRAYSIZE(macResTagMap); i++)
		if (macResTagMap[i].type == type)
			tags.push_back(macResTagMap[i].tag);

	return tags;
}

void MacResourceForkResourceSource::scanSource(ResourceManager *resMan) {
	if (!_macResMan->open(getLocationName().c_str()))
		error("%s is not a valid Mac resource fork", getLocationName().c_str());

	Common::MacResTagArray tagArray = _macResMan->getResTagArray();

	for (uint32 i = 0; i < tagArray.size(); i++) {
		ResourceType type = kResourceTypeInvalid;

		// Map the Mac tags to our ResourceType
		for (uint32 j = 0; j < ARRAYSIZE(macResTagMap); j++)
			if (tagArray[i] == macResTagMap[j].tag) {
				type = macResTagMap[j].type;
				break;
			}

		if (type == kResourceTypeInvalid)
			continue;

		Common::MacResIDArray idArray = _macResMan->getResIDArray(tagArray[i]);

		for (uint32 j = 0; j < idArray.size(); j++) {
			ResourceId resId;

			// Check to see if we've got a base36 encoded resource name
			if (type == kResourceTypeAudio) {
				Common::String resourceName = _macResMan->getResName(tagArray[i], idArray[j]);

				// If we have a file name on an audio resource, we've got an audio36
				// resource. Parse the file name to get the id.
				if (!resourceName.empty() && resourceName[0] == '@')
					resId = convertPatchNameBase36(kResourceTypeAudio36, resourceName);
				else
					resId = ResourceId(type, idArray[j]);
			} else if (type == kResourceTypeSync) {
				Common::String resourceName = _macResMan->getResName(tagArray[i], idArray[j]);

				// Same as with audio36 above
				if (!resourceName.empty() && resourceName[0] == '#')
					resId = convertPatchNameBase36(kResourceTypeSync36, resourceName);
				else
					resId = ResourceId(type, idArray[j]);
			} else {
				// Otherwise, we're just going with the id that was given
				resId = ResourceId(type, idArray[j]);
			}

			// Overwrite Resource instance. Resource forks may contain patches.
			// The size will be filled in later by decompressResource()
			resMan->updateResource(resId, this, 0);
		}
	}
}

void ResourceManager::addResource(ResourceId resId, ResourceSource *src, uint32 offset, uint32 size) {
	// Adding new resource only if it does not exist
	if (_resMap.contains(resId) == false) {
		Resource *res = new Resource(this, resId);
		_resMap.setVal(resId, res);
		res->_source = src;
		res->_fileOffset = offset;
		res->size = size;
	}
}

Resource *ResourceManager::updateResource(ResourceId resId, ResourceSource *src, uint32 size) {
	// Update a patched resource, whether it exists or not
	Resource *res = 0;

	if (_resMap.contains(resId)) {
		res = _resMap.getVal(resId);
	} else {
		res = new Resource(this, resId);
		_resMap.setVal(resId, res);
	}

	res->_status = kResStatusNoMalloc;
	res->_source = src;
	res->_headerSize = 0;
	res->size = size;

	return res;
}

int Resource::readResourceInfo(ResVersion volVersion, Common::SeekableReadStream *file,
                                      uint32 &szPacked, ResourceCompression &compression) {
	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format :  {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	uint16 w, number;
	uint32 wCompression, szUnpacked;
	ResourceType type;

	if (file->size() == 0)
		return SCI_ERROR_EMPTY_RESOURCE;

	switch (volVersion) {
	case kResVersionSci0Sci1Early:
	case kResVersionSci1Middle:
		w = file->readUint16LE();
		type = _resMan->convertResType(w >> 11);
		number = w & 0x7FF;
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case kResVersionSci1Late:
		type = _resMan->convertResType(file->readByte());
		number = file->readUint16LE();
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case kResVersionSci11:
		type = _resMan->convertResType(file->readByte());
		number = file->readUint16LE();
		szPacked = file->readUint16LE();
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
#ifdef ENABLE_SCI32
	case kResVersionSci2:
	case kResVersionSci3:
		type = _resMan->convertResType(file->readByte());
		number = file->readUint16LE();
		szPacked = file->readUint32LE();
		szUnpacked = file->readUint32LE();

		// The same comment applies here as in
		// detectVolVersion regarding SCI3. We ignore the
		// compression field for SCI3 games, but must presume
		// it exists in the file.
		wCompression = file->readUint16LE();

		if (volVersion == kResVersionSci3)
			wCompression = szPacked != szUnpacked ? 32 : 0;

		break;
#endif
	default:
		return SCI_ERROR_RESMAP_INVALID_ENTRY;
	}

	// check if there were errors while reading
	if ((file->eos() || file->err()))
		return SCI_ERROR_IO_ERROR;

	_id = ResourceId(type, number);
	size = szUnpacked;

	// checking compression method
	switch (wCompression) {
	case 0:
		compression = kCompNone;
		break;
	case 1:
		compression = (getSciVersion() <= SCI_VERSION_01) ? kCompLZW : kCompHuffman;
		break;
	case 2:
		compression = (getSciVersion() <= SCI_VERSION_01) ? kCompHuffman : kCompLZW1;
		break;
	case 3:
		compression = kCompLZW1View;
		break;
	case 4:
		compression = kCompLZW1Pic;
		break;
	case 18:
	case 19:
	case 20:
		compression = kCompDCL;
		break;
#ifdef ENABLE_SCI32
	case 32:
		compression = kCompSTACpack;
		break;
#endif
	default:
		compression = kCompUnknown;
	}

	return (compression == kCompUnknown) ? SCI_ERROR_UNKNOWN_COMPRESSION : SCI_ERROR_NONE;
}

int Resource::decompress(ResVersion volVersion, Common::SeekableReadStream *file) {
	int errorNum;
	uint32 szPacked = 0;
	ResourceCompression compression = kCompUnknown;

	// fill resource info
	errorNum = readResourceInfo(volVersion, file, szPacked, compression);
	if (errorNum)
		return errorNum;

	// getting a decompressor
	Decompressor *dec = NULL;
	switch (compression) {
	case kCompNone:
		dec = new Decompressor;
		break;
	case kCompHuffman:
		dec = new DecompressorHuffman;
		break;
	case kCompLZW:
	case kCompLZW1:
	case kCompLZW1View:
	case kCompLZW1Pic:
		dec = new DecompressorLZW(compression);
		break;
	case kCompDCL:
		dec = new DecompressorDCL;
		break;
#ifdef ENABLE_SCI32
	case kCompSTACpack:
		dec = new DecompressorLZS;
		break;
#endif
	default:
		error("Resource %s: Compression method %d not supported", _id.toString().c_str(), compression);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	data = new byte[size];
	_status = kResStatusAllocated;
	errorNum = data ? dec->unpack(file, data, szPacked, size) : SCI_ERROR_RESOURCE_TOO_BIG;
	if (errorNum)
		unalloc();

	delete dec;
	return errorNum;
}

ResourceCompression ResourceManager::getViewCompression() {
	int viewsTested = 0;

	// Test 10 views to see if any are compressed
	for (int i = 0; i < 1000; i++) {
		Common::SeekableReadStream *fileStream = 0;
		Resource *res = testResource(ResourceId(kResourceTypeView, i));

		if (!res)
			continue;

		if (res->_source->getSourceType() != kSourceVolume)
			continue;

		fileStream = getVolumeFile(res->_source);

		if (!fileStream)
			continue;
		fileStream->seek(res->_fileOffset, SEEK_SET);

		uint32 szPacked;
		ResourceCompression compression;

		if (res->readResourceInfo(_volVersion, fileStream, szPacked, compression)) {
			if (res->_source->_resourceFile)
				delete fileStream;
			continue;
		}

		if (res->_source->_resourceFile)
			delete fileStream;

		if (compression != kCompNone)
			return compression;

		if (++viewsTested == 10)
			break;
	}

	return kCompNone;
}

ViewType ResourceManager::detectViewType() {
	for (int i = 0; i < 1000; i++) {
		Resource *res = findResource(ResourceId(kResourceTypeView, i), 0);

		if (res) {
			// Skip views coming from patch files
			if (res->_source->getSourceType() == kSourcePatch)
				continue;

			switch (res->data[1]) {
			case 128:
				// If the 2nd byte is 128, it's a VGA game.
				// However, Longbow Amiga (AGA, 64 colors), also sets this byte
				// to 128, but it's a mixed VGA/Amiga format. Detect this from
				// the platform here.
				if (g_sci && g_sci->getPlatform() == Common::kPlatformAmiga)
					return kViewAmiga64;

				return kViewVga;
			case 0:
				// EGA or Amiga, try to read as Amiga view

				if (res->size < 10)
					return kViewUnknown;

				// Read offset of first loop
				uint16 offset = READ_LE_UINT16(res->data + 8);

				if (offset + 6U >= res->size)
					return kViewUnknown;

				// Read offset of first cel
				offset = READ_LE_UINT16(res->data + offset + 4);

				if (offset + 4U >= res->size)
					return kViewUnknown;

				// Check palette offset, amiga views have no palette
				if (READ_LE_UINT16(res->data + 6) != 0)
					return kViewEga;

				uint16 width = READ_LE_UINT16(res->data + offset);
				offset += 2;
				uint16 height = READ_LE_UINT16(res->data + offset);
				offset += 6;

				// To improve the heuristic, we skip very small views
				if (height < 10)
					continue;

				// Check that the RLE data stays within bounds
				int y;
				for (y = 0; y < height; y++) {
					int x = 0;

					while ((x < width) && (offset < res->size)) {
						byte op = res->data[offset++];
						x += (op & 0x07) ? op & 0x07 : op >> 3;
					}

					// Make sure we got exactly the right number of pixels for this row
					if (x != width)
						return kViewEga;
				}

				return kViewAmiga;
			}
		}
	}

	// this may happen if there are serious system issues (or trying to add a broken game)
	warning("resMan: Couldn't find any views");
	return kViewUnknown;
}

void ResourceManager::detectSciVersion() {
	// We use the view compression to set a preliminary s_sciVersion for the sake of getResourceInfo
	// Pretend we have a SCI0 game
	s_sciVersion = SCI_VERSION_0_EARLY;
	bool oldDecompressors = true;

	ResourceCompression viewCompression;
#ifdef ENABLE_SCI32
	viewCompression = getViewCompression();
#else
	if (_volVersion >= kResVersionSci2) {
		// SCI32 support isn't built in, thus view detection will fail
		viewCompression = kCompUnknown;
	} else {
		viewCompression = getViewCompression();
	}
#endif

	if (viewCompression != kCompLZW) {
		// If it's a different compression type from kCompLZW, the game is probably
		// SCI_VERSION_1_EGA_ONLY or later. If the views are uncompressed, it is
		// likely not an early disk game.
		s_sciVersion = SCI_VERSION_1_EGA_ONLY;
		oldDecompressors = false;
	}

	// Set view type
	if (viewCompression == kCompDCL
		|| _volVersion == kResVersionSci11 // pq4demo
		|| _volVersion == kResVersionSci11Mac
#ifdef ENABLE_SCI32
		|| viewCompression == kCompSTACpack
		|| _volVersion == kResVersionSci2 // kq7
#endif
		) {
		// SCI1.1 VGA views
		_viewType = kViewVga11;
	} else {
#ifdef ENABLE_SCI32
		// Otherwise we detect it from a view
		_viewType = detectViewType();
#else
		if (_volVersion == kResVersionSci2 && viewCompression == kCompUnknown) {
			// A SCI32 game, but SCI32 support is disabled. Force the view type
			// to kViewVga11, as we can't read from the game's resource files
			_viewType = kViewVga11;
		} else {
			_viewType = detectViewType();
		}
#endif
	}

	if (_volVersion == kResVersionSci11Mac) {
		Resource *res = testResource(ResourceId(kResourceTypeScript, 64920));
		// Distinguish between SCI1.1 and SCI32 games here. SCI32 games will
		// always include script 64920 (the Array class). Note that there are
		// no Mac SCI2 games. Yes, that means that GK1 Mac is SCI2.1 and not SCI2.

		// TODO: Decide between SCI2.1 and SCI3
		if (res)
			s_sciVersion = SCI_VERSION_2_1;
		else
			s_sciVersion = SCI_VERSION_1_1;
		return;
	}

	// Handle SCI32 versions here
	if (_volVersion >= kResVersionSci2) {
		Common::List<ResourceId> heaps = listResources(kResourceTypeHeap);
		bool hasHeapResources = !heaps.empty();

		// SCI2.1/3 and SCI1 Late resource maps are the same, except that
		// SCI1 Late resource maps have the resource types or'd with
		// 0x80. We differentiate between SCI2 and SCI2.1/3 based on that.
		if (_mapVersion == kResVersionSci1Late) {
			s_sciVersion = SCI_VERSION_2;
			return;
		} else if (hasHeapResources) {
			s_sciVersion = SCI_VERSION_2_1;
			return;
		} else {
			s_sciVersion = SCI_VERSION_3;
			return;
		}
	}

	// Check for transitive SCI1/SCI1.1 games, like PQ1 here
	// If the game has any heap file (here we check for heap file 0), then
	// it definitely uses a SCI1.1 kernel
	if (testResource(ResourceId(kResourceTypeHeap, 0))) {
		s_sciVersion = SCI_VERSION_1_1;
		return;
	}

	switch (_mapVersion) {
	case kResVersionSci0Sci1Early:
		if (_viewType == kViewVga) {
			// VGA
			s_sciVersion = SCI_VERSION_1_EARLY;
			return;
		}

		// EGA
		if (hasOldScriptHeader()) {
			s_sciVersion = SCI_VERSION_0_EARLY;
			return;
		}

		if (hasSci0Voc999()) {
			s_sciVersion = SCI_VERSION_0_LATE;
			return;
		}

		if (oldDecompressors) {
			// It's either SCI_VERSION_0_LATE or SCI_VERSION_01

			// We first check for SCI1 vocab.999
			if (testResource(ResourceId(kResourceTypeVocab, 999))) {
				s_sciVersion = SCI_VERSION_01;
				return;
			}

			// If vocab.999 is missing, we try vocab.900
			if (testResource(ResourceId(kResourceTypeVocab, 900))) {
				if (hasSci1Voc900()) {
					s_sciVersion = SCI_VERSION_01;
					return;
				} else {
					s_sciVersion = SCI_VERSION_0_LATE;
					return;
				}
			}

			error("Failed to accurately determine SCI version");
			// No parser, we assume SCI_VERSION_01.
			s_sciVersion = SCI_VERSION_01;
			return;
		}

		// New decompressors. It's either SCI_VERSION_1_EGA_ONLY or SCI_VERSION_1_EARLY.
		if (hasSci1Voc900()) {
			s_sciVersion = SCI_VERSION_1_EGA_ONLY;
			return;
		}

		// SCI_VERSION_1_EARLY EGA versions lack the parser vocab
		s_sciVersion = SCI_VERSION_1_EARLY;
		return;
	case kResVersionSci1Middle:
	case kResVersionKQ5FMT:
		s_sciVersion = SCI_VERSION_1_MIDDLE;
		// Amiga SCI1 middle games are actually SCI1 late
		if (_viewType == kViewAmiga || _viewType == kViewAmiga64)
			s_sciVersion = SCI_VERSION_1_LATE;
		// Same goes for Mac SCI1 middle games
		if (g_sci && g_sci->getPlatform() == Common::kPlatformMacintosh)
			s_sciVersion = SCI_VERSION_1_LATE;
		return;
	case kResVersionSci1Late:
		if (_volVersion == kResVersionSci11) {
			s_sciVersion = SCI_VERSION_1_1;
			return;
		}
		// FIXME: this is really difficult, lsl1 spanish has map/vol sci1late
		//  and the only current detection difference is movecounttype which
		//  is increment here, but ignore for all the regular sci1late games
		//  the problem is, we dont have access to that detection till later
		//  so maybe (part of?) that detection should get moved in here
		if (g_sci && (g_sci->getGameId() == GID_LSL1) && (g_sci->getLanguage() == Common::ES_ESP)) {
			s_sciVersion = SCI_VERSION_1_MIDDLE;
			return;
		}
		s_sciVersion = SCI_VERSION_1_LATE;
		return;
	case kResVersionSci11:
		s_sciVersion = SCI_VERSION_1_1;
		return;
	default:
		s_sciVersion = SCI_VERSION_NONE;
		error("detectSciVersion(): Unable to detect the game's SCI version");
	}
}

bool ResourceManager::detectHires() {
	// SCI 1.1 and prior is never hires
	if (getSciVersion() <= SCI_VERSION_1_1)
		return false;

#ifdef ENABLE_SCI32
	for (int i = 0; i < 32768; i++) {
		Resource *res = findResource(ResourceId(kResourceTypePic, i), 0);

		if (res) {
			if (READ_SCI11ENDIAN_UINT16(res->data) == 0x0e) {
				// SCI32 picture
				uint16 width = READ_SCI11ENDIAN_UINT16(res->data + 10);
				uint16 height = READ_SCI11ENDIAN_UINT16(res->data + 12);
				// Surely lowres (e.g. QFG4CD)
				if ((width == 320) && ((height == 190) || (height == 200)))
					return false;
				// Surely hires
				if ((width >= 600) || (height >= 400))
					return true;
			}
		}
	}

	// We haven't been able to find hires content

	return false;
#else
	error("no sci32 support");
#endif
}

bool ResourceManager::detectFontExtended() {

	Resource *res = findResource(ResourceId(kResourceTypeFont, 0), 0);
	if (res) {
		if (res->size >= 4) {
			uint16 numChars = READ_LE_UINT16(res->data + 2);
			if (numChars > 0x80)
				return true;
		}
	}
	return false;
}

// detects, if SCI1.1 game uses palette merging or copying - this is supposed to only get used on SCI1.1 games
bool ResourceManager::detectPaletteMergingSci11() {
	// Load palette 999 (default palette)
	Resource *res = findResource(ResourceId(kResourceTypePalette, 999), false);

	if ((res) && (res->size > 30)) {
		byte *data = res->data;
		// Old palette format used in palette resource? -> it's merging
		if ((data[0] == 0 && data[1] == 1) || (data[0] == 0 && data[1] == 0 && READ_LE_UINT16(data + 29) == 0))
			return true;
		return false;
	}
	return false;
}

// is called on SCI0EARLY games to make sure that sound resources are in fact also SCI0EARLY
bool ResourceManager::detectEarlySound() {
	Resource *res = findResource(ResourceId(kResourceTypeSound, 1), 0);
	if (res) {
		if (res->size >= 0x22) {
			if (READ_LE_UINT16(res->data + 0x1f) == 0) // channel 15 voice count + play mask is 0 in SCI0LATE
				if (res->data[0x21] == 0) // last byte right before actual data is 0 as well
					return false;
		}
	}
	return true;
}

// Functions below are based on PD code by Brian Provinciano (SCI Studio)
bool ResourceManager::hasOldScriptHeader() {
	Resource *res = findResource(ResourceId(kResourceTypeScript, 0), 0);

	if (!res) {
		error("resMan: Failed to find script.000");
		return false;
	}

	uint offset = 2;
	const int objTypes = 17;

	while (offset < res->size) {
		uint16 objType = READ_LE_UINT16(res->data + offset);

		if (!objType) {
			offset += 2;
			// We should be at the end of the resource now
			return offset == res->size;
		}

		if (objType >= objTypes) {
			// Invalid objType
			return false;
		}

		int skip = READ_LE_UINT16(res->data + offset + 2);

		if (skip < 2) {
			// Invalid size
			return false;
		}

		offset += skip;
	}

	return false;
}

bool ResourceManager::hasSci0Voc999() {
	Resource *res = findResource(ResourceId(kResourceTypeVocab, 999), 0);

	if (!res) {
		// No vocab present, possibly a demo version
		return false;
	}

	if (res->size < 2)
		return false;

	uint16 count = READ_LE_UINT16(res->data);

	// Make sure there's enough room for the pointers
	if (res->size < (uint)count * 2)
		return false;

	// Iterate over all pointers
	for (uint i = 0; i < count; i++) {
		// Offset to string
		uint16 offset = READ_LE_UINT16(res->data + 2 + count * 2);

		// Look for end of string
		do {
			if (offset >= res->size) {
				// Out of bounds
				return false;
			}
		} while (res->data[offset++]);
	}

	return true;
}

bool ResourceManager::hasSci1Voc900() {
	Resource *res = findResource(ResourceId(kResourceTypeVocab, 900), 0);

	if (!res )
		return false;

	if (res->size < 0x1fe)
		return false;

	uint16 offset = 0x1fe;

	while (offset < res->size) {
		offset++;
		do {
			if (offset >= res->size) {
				// Out of bounds;
				return false;
			}
		} while (res->data[offset++]);
		offset += 3;
	}

	return offset == res->size;
}

// Same function as Script::findBlockSCI0(). Slight code
// duplication here, but this has been done to keep the resource
// manager independent from the rest of the engine
static byte *findSci0ExportsBlock(byte *buffer) {
	byte *buf = buffer;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (oldScriptHeader)
		buf += 2;

	do {
		int seekerType = READ_LE_UINT16(buf);

		if (seekerType == 0)
			break;
		if (seekerType == 7)	// exports
			return buf;

		int seekerSize = READ_LE_UINT16(buf + 2);
		assert(seekerSize > 0);
		buf += seekerSize;
	} while (1);

	return NULL;
}

// This code duplicates Script::relocateOffsetSci3, but we can't use
// that here since we can't instantiate scripts at this point.
static int relocateOffsetSci3(const byte *buf, uint32 offset) {
	int relocStart = READ_LE_UINT32(buf + 8);
	int relocCount = READ_LE_UINT16(buf + 18);
	const byte *seeker = buf + relocStart;

	for (int i = 0; i < relocCount; ++i) {
		if (READ_SCI11ENDIAN_UINT32(seeker) == offset) {
			// TODO: Find out what UINT16 at (seeker + 8) means
			return READ_SCI11ENDIAN_UINT16(buf + offset) + READ_SCI11ENDIAN_UINT32(seeker + 4);
		}
		seeker += 10;
	}

	return -1;
}

reg_t ResourceManager::findGameObject(bool addSci11ScriptOffset) {
	Resource *script = findResource(ResourceId(kResourceTypeScript, 0), false);

	if (!script)
		return NULL_REG;

	byte *offsetPtr = 0;

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		byte *buf = (getSciVersion() == SCI_VERSION_0_EARLY) ? script->data + 2 : script->data;

		// Check if the first block is the exports block (in most cases, it is)
		bool exportsIsFirst = (READ_LE_UINT16(buf + 4) == 7);
		if (exportsIsFirst) {
			offsetPtr = buf + 4 + 2;
		} else {
			offsetPtr = findSci0ExportsBlock(script->data);
			if (!offsetPtr)
				error("Unable to find exports block from script 0");
			offsetPtr += 4 + 2;
		}

		int16 offset = !isSci11Mac() ? READ_LE_UINT16(offsetPtr) : READ_BE_UINT16(offsetPtr);
		return make_reg(1, offset);
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		offsetPtr = script->data + 4 + 2 + 2;

		// In SCI1.1 - SCI2.1, the heap is appended at the end of the script,
		// so adjust the offset accordingly if requested
		int16 offset = !isSci11Mac() ? READ_LE_UINT16(offsetPtr) : READ_BE_UINT16(offsetPtr);
		if (addSci11ScriptOffset) {
			offset += script->size;

			// Ensure that the start of the heap is word-aligned - same as in Script::init()
			if (script->size & 2)
				offset++;
		}

		return make_reg(1, offset);
	} else {
		return make_reg(1, relocateOffsetSci3(script->data, 22));
	}
}

Common::String ResourceManager::findSierraGameId() {
	// In SCI0-SCI1, the heap is embedded in the script. In SCI1.1 - SCI2.1,
	// it's in a separate heap resource
	Resource *heap = 0;
	int nameSelector = 3;

	if (getSciVersion() < SCI_VERSION_1_1) {
		heap = findResource(ResourceId(kResourceTypeScript, 0), false);
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		heap = findResource(ResourceId(kResourceTypeHeap, 0), false);
		nameSelector += 5;
	} else if (getSciVersion() == SCI_VERSION_3) {
		warning("TODO: findSierraGameId(): SCI3 equivalent");
	}

	if (!heap)
		return "";

	int16 gameObjectOffset = findGameObject(false).getOffset();

	if (!gameObjectOffset)
		return "";

	// Seek to the name selector of the first export
	byte *seeker = heap->data + READ_UINT16(heap->data + gameObjectOffset + nameSelector * 2);
	Common::String sierraId;
	sierraId += (const char *)seeker;

	return sierraId;
}

const Common::String &Resource::getResourceLocation() const {
	return _source->getLocationName();
}

} // End of namespace Sci
