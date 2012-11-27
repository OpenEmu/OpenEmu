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

#ifndef AMIGAOS_FILESYSTEM_H
#define AMIGAOS_FILESYSTEM_H

#ifdef __USE_INLINE__
#undef __USE_INLINE__
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>

#ifndef USE_NEWLIB
#include <strings.h>
#endif

#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class AmigaOSFilesystemNode : public AbstractFSNode {
protected:
	BPTR _pFileLock;
	Common::String _sDisplayName;
	Common::String _sPath;
	bool _bIsDirectory;
	bool _bIsValid;
	uint32 _nProt;

	/**
	 * Creates a list with all the volumes present in the root node.
	 */
	virtual AbstractFSList listVolumes() const;

public:
	/**
	 * Creates an AmigaOSFilesystemNode with the root node as path.
	 */
	AmigaOSFilesystemNode();

	/**
	 * Creates an AmigaOSFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 */
	AmigaOSFilesystemNode(const Common::String &p);

	/**
	 * Creates an AmigaOSFilesystemNode given its lock and display name
	 *
	 * @param pLock BPTR to the lock.
	 * @param pDisplayName name to be used for display, in case not supplied the FilePart() of the filename will be used.
	 *
	 * @note This shouldn't even be public as it's only internally, at best it should have been protected if not private
	 */
	AmigaOSFilesystemNode(BPTR pLock, const char *pDisplayName = 0);

	/**
	 * Copy constructor.
	 *
	 * @note Needed because it duplicates the file lock
	 */
	AmigaOSFilesystemNode(const AmigaOSFilesystemNode &node);

	/**
	 * Destructor.
	 */
	virtual ~AmigaOSFilesystemNode();

	virtual bool exists() const;
	virtual Common::String getDisplayName() const { return _sDisplayName; }
	virtual Common::String getName() const { return _sDisplayName; }
	virtual Common::String getPath() const { return _sPath; }
	virtual bool isDirectory() const { return _bIsDirectory; }
	virtual bool isReadable() const;
	virtual bool isWritable() const;

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
};


#endif
