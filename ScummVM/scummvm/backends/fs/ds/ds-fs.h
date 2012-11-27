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

#ifndef _DS_FS_H
#define _DS_FS_H

#include "common/fs.h"
#include "common/stream.h"
#include "backends/fs/abstract-fs.h"

#include "zipreader.h"
#include "fat/gba_nds_fat.h"

namespace DS {

/**
 * Implementation of the ScummVM file system API.
 * This class is used when a Flash cart is in use.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class DSFileSystemNode : public AbstractFSNode {
protected:
	static ZipFile *_zipFile;

	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;

public:
	/**
	 * Creates a DSFilesystemNode with the root node as path.
	 */
	DSFileSystemNode();

	/**
	 * Creates a DSFilesystemNode for a given path.
	 *
	 * @param path String with the path the new node should point to.
	 */
	DSFileSystemNode(const Common::String &path);

	/**
	 * Creates a DSFilesystemNode for a given path.
	 *
	 * @param path String with the path the new node should point to.
	 * @param path true if path is a directory, false otherwise.
	 */
	DSFileSystemNode(const Common::String& path, bool isDir);

	/**
	 * Copy constructor.
	 */
	DSFileSystemNode(const DSFileSystemNode *node);

	virtual bool exists() const { return true; }		//FIXME: this is just a stub
	virtual Common::String getDisplayName() const {  return _displayName; }
	virtual Common::String getName() const {  return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return true; }	//FIXME: this is just a stub
	virtual bool isWritable() const { return true; }	//FIXME: this is just a stub

	/**
	 * Returns a copy of this node.
	 */
	virtual AbstractFSNode *clone() const { return new DSFileSystemNode(this); }
	virtual AbstractFSNode *getChild(const Common::String& name) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();

	/**
	 * Returns the zip file this node points to.
	 * TODO: check this documentation.
	 */
	static ZipFile *getZip() { return _zipFile; }
};

 /**
 * Implementation of the ScummVM file system API.
 * This class is used when the GBAMP (GBA Movie Player) is used with a CompactFlash card.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class GBAMPFileSystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;

public:
	/**
	 * Creates a GBAMPFilesystemNode with the root node as path.
	 */
	GBAMPFileSystemNode();

	/**
	 * Creates a GBAMPFilesystemNode for a given path.
	 *
	 * @param path String with the path the new node should point to.
	 */
	GBAMPFileSystemNode(const Common::String &path);

	/**
	 * Creates a DSFilesystemNode for a given path.
	 *
	 * @param path String with the path the new node should point to.
	 * @param path true if path is a directory, false otherwise.
	 */
	GBAMPFileSystemNode(const Common::String &path, bool isDirectory);

	/**
	 * Copy constructor.
	 */
	GBAMPFileSystemNode(const GBAMPFileSystemNode *node);

	virtual bool exists() const { return _isValid || _isDirectory; }
	virtual Common::String getDisplayName() const {  return _displayName; }
	virtual Common::String getName() const {  return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const { return true; }	//FIXME: this is just a stub
	virtual bool isWritable() const { return true; }	//FIXME: this is just a stub

	/**
	 * Returns a copy of this node.
	 */
	virtual AbstractFSNode *clone() const { return new GBAMPFileSystemNode(this); }
	virtual AbstractFSNode *getChild(const Common::String& name) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
};

struct fileHandle {
	int pos;
	bool used;
	char *data;
	int size;
};


class DSFileStream : public Common::SeekableReadStream, public Common::WriteStream, public Common::NonCopyable {
protected:

	/** File handle to the actual file. */
	void 	*_handle;

public:
	/**
	 * Given a path, invokes fopen on that path and wrap the result in a
	 * StdioStream instance.
	 */
	static DSFileStream *makeFromPath(const Common::String &path, bool writeMode);

	DSFileStream(void *handle);
	virtual ~DSFileStream();

	bool err() const;
	void clearErr();
	bool eos() const;

	virtual uint32 write(const void *dataPtr, uint32 dataSize);
	virtual bool flush();

	virtual int32 pos() const;
	virtual int32 size() const;
	bool seek(int32 offs, int whence = SEEK_SET);
	uint32 read(void *dataPtr, uint32 dataSize);
};


// FIXME/TODO: Get rid of the following hacks. Top priority: Get rid of
// the 'FILE' (re)definition. Simply calling it STD_FILE or so wold already
// suffice (need to adjust affected code, of course).
// Once the OSystem::logMessage() patch is in SVN, we should also be
// able to get rid of stderr, stdout, stdin.
// Moreover, the std_FOO() functions could then be moved to a private
// header, or even completely merged into DSFileStream, and all other
// DS code switch to use that.
#undef stderr
#undef stdout
#undef stdin

#define stdout ((DS::fileHandle *) -1)
#define stderr ((DS::fileHandle *) -2)
#define stdin ((DS::fileHandle *) -3)

#define FILE DS::fileHandle

// Please do not remove any of these prototypes that appear not to be required.
FILE*	std_fopen(const char *name, const char *mode);
void	std_fclose(FILE *handle);
size_t	std_fread(void *ptr, size_t size, size_t numItems, FILE *handle);
size_t	std_fwrite(const void *ptr, size_t size, size_t numItems, FILE *handle);
bool	std_feof(FILE *handle);
long int std_ftell(FILE *handle);
int		std_fseek(FILE *handle, long int offset, int whence);
void	std_clearerr(FILE *handle);
int		std_fflush(FILE *handle);
int		std_ferror(FILE *handle);

}	// End of namespace DS

#endif //_DS_FS_H
