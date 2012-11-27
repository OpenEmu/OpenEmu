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
 *
 */

#ifndef _WII_FILESYSTEM_FACTORY_H_
#define _WII_FILESYSTEM_FACTORY_H_

#include "common/str.h"
#include "common/singleton.h"
#include "backends/fs/fs-factory.h"

#include <gctypes.h>

/**
 * Creates WiiFilesystemNode objects.
 *
 * Parts of this class are documented in the base interface class, FilesystemFactory.
 */
class WiiFilesystemFactory : public FilesystemFactory, public Common::Singleton<WiiFilesystemFactory> {
public:
	typedef Common::String String;

	enum FileSystemType {
		kDVD,
		kSMB
	};

	virtual AbstractFSNode *makeRootFileNode() const;
	virtual AbstractFSNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFSNode *makeFileNodePath(const Common::String &path) const;

	void asyncInit();
	void asyncDeinit();

#ifdef USE_WII_SMB
	void asyncInitNetwork();
	void setSMBLoginData(const String &server, const String &share,
							const String &username, const String &password);
#endif

	bool isMounted(FileSystemType type);
	bool failedToMount(FileSystemType type);

	void mount(FileSystemType type);
	void umount(FileSystemType type);

	void mountByPath(const String &path);
	void umountUnused(const String &path);

protected:
	WiiFilesystemFactory();

private:
	friend class Common::Singleton<SingletonBaseType>;

	bool _dvdMounted;
	bool _smbMounted;
	bool _dvdError;
	bool _smbError;

#ifdef USE_WII_SMB
	String _smbServer;
	String _smbShare;
	String _smbUsername;
	String _smbPassword;
#endif
};

#endif /*Wii_FILESYSTEM_FACTORY_H*/
