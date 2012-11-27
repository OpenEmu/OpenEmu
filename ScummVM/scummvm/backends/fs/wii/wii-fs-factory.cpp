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

#if defined(__WII__)

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd

#include <unistd.h>

#include "backends/fs/wii/wii-fs-factory.h"
#include "backends/fs/wii/wii-fs.h"

#ifdef USE_WII_DI
#include <di/di.h>
#include <iso9660.h>
#endif

#ifdef USE_WII_SMB
#include <network.h>
#include <smb.h>
#endif

namespace Common {
DECLARE_SINGLETON(WiiFilesystemFactory);
}

WiiFilesystemFactory::WiiFilesystemFactory() :
	_dvdMounted(false),
	_smbMounted(false),
	_dvdError(false),
	_smbError(false) {
}

AbstractFSNode *WiiFilesystemFactory::makeRootFileNode() const {
	return new WiiFilesystemNode();
}

AbstractFSNode *WiiFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];

	if (getcwd(buf, MAXPATHLEN))
		return new WiiFilesystemNode(buf);
	else
		return new WiiFilesystemNode();
}

AbstractFSNode *WiiFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	return new WiiFilesystemNode(path);
}

void WiiFilesystemFactory::asyncInit() {
#ifdef USE_WII_SMB
	asyncInitNetwork();
#endif
}

void WiiFilesystemFactory::asyncDeinit() {
#ifdef USE_WII_DI
	umount(kDVD);
#ifndef GAMECUBE
	DI_Close();
#endif
#endif
#ifdef USE_WII_SMB
	umount(kSMB);
	net_deinit();
#endif
}

#ifdef USE_WII_SMB
void WiiFilesystemFactory::asyncInitNetwork() {
	net_init_async(NULL, NULL);
}

void WiiFilesystemFactory::setSMBLoginData(const String &server,
											const String &share,
											const String &username,
											const String &password) {
	_smbServer = server;
	_smbShare = share;
	_smbUsername = username;
	_smbPassword = password;
}
#endif

bool WiiFilesystemFactory::isMounted(FileSystemType type) {
	switch (type) {
	case kDVD:
		return _dvdMounted;
	case kSMB:
		return _smbMounted;
	}

	return false;
}

bool WiiFilesystemFactory::failedToMount(FileSystemType type) {
	switch (type) {
	case kDVD:
		return _dvdError;
	case kSMB:
		return _smbError;
	}

	return false;
}

void WiiFilesystemFactory::mount(FileSystemType type) {
	switch (type) {
	case kDVD:
#ifdef USE_WII_DI
		if (_dvdMounted)
			break;

		printf("mount dvd\n");
		if (ISO9660_Mount()) {
			_dvdMounted = true;
			_dvdError = false;
			printf("ISO9660 mounted\n");
		} else {
			_dvdError = true;
			printf("ISO9660 mount failed\n");
		}
#endif
		break;

	case kSMB:
#ifdef USE_WII_SMB
		if (_smbMounted)
			break;

		printf("mount smb\n");

		if (net_get_status()) {
			printf("network not inited\n");
			break;
		}

		if (smbInit(_smbUsername.c_str(), _smbPassword.c_str(),
					_smbShare.c_str(), _smbServer.c_str())) {
			_smbMounted = true;
			_smbError = false;
			printf("smb mounted\n");
		} else {
			_smbError = true;
			printf("error mounting smb\n");
		}
#endif
		break;
	}
}

void WiiFilesystemFactory::umount(FileSystemType type) {
	switch (type) {
	case kDVD:
#ifdef USE_WII_DI
		if (!_dvdMounted)
			break;

		printf("umount dvd\n");

		ISO9660_Unmount();

		_dvdMounted = false;
		_dvdError = false;
#endif
		break;

	case kSMB:
#ifdef USE_WII_SMB
		if (!_smbMounted)
			break;

		printf("umount smb\n");

		smbClose("smb:");

		_smbMounted = false;
		_smbError = false;
#endif
		break;
	}
}

void WiiFilesystemFactory::mountByPath(const String &path) {
	if (path.hasPrefix("dvd:/"))
		mount(kDVD);
	else if (path.hasPrefix("smb:/"))
		mount(kSMB);
}

void WiiFilesystemFactory::umountUnused(const String &path) {
	if (!path.hasPrefix("dvd:/"))
		umount(kDVD);

	if (!path.hasPrefix("smb:/"))
		umount(kSMB);
}

#endif
