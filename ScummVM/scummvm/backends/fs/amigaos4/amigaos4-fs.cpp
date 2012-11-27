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

#if defined(__amigaos4__)

#include "backends/fs/amigaos4/amigaos4-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/debug.h"
#include "common/util.h"

#define ENTER() /* debug(6, "Enter") */
#define LEAVE() /* debug(6, "Leave") */

/**
 * Returns the last component of a given path.
 *
 * @param str Common::String containing the path.
 * @return Pointer to the first char of the last component inside str.
 */
const char *lastPathComponent(const Common::String &str) {
	int offset = str.size();

	if (offset <= 0) {
		debug(6, "Bad offset");
		return 0;
	}

	const char *p = str.c_str();

	while (offset > 0 && (p[offset-1] == '/' || p[offset-1] == ':'))
		offset--;

	while (offset > 0 && (p[offset-1] != '/' && p[offset-1] != ':'))
		offset--;

	return p + offset;
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode() {
	ENTER();
	_sDisplayName = "Available Disks";
	_bIsValid = true;
	_bIsDirectory = true;
	_sPath = "";
	_pFileLock = 0;
	_nProt = 0; // protection is ignored for the root volume
	LEAVE();
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode(const Common::String &p) {
	ENTER();

	int offset = p.size();

	//assert(offset > 0);

	if (offset <= 0) {
		debug(6, "Bad offset");
		return;
	}

	_sPath = p;
	_sDisplayName = ::lastPathComponent(_sPath);
	_pFileLock = 0;
	_bIsDirectory = false;

	// Check whether the node exists and if it is a directory
	struct ExamineData * pExd = IDOS->ExamineObjectTags(EX_StringNameInput,_sPath.c_str(),TAG_END);
	if (pExd) {
		_nProt = pExd->Protection;
		if (EXD_IS_DIRECTORY(pExd)) {
			_bIsDirectory = true;
			_pFileLock = IDOS->Lock((CONST_STRPTR)_sPath.c_str(), SHARED_LOCK);
			_bIsValid = (_pFileLock != 0);

			// Add a trailing slash if it is needed
			const char c = _sPath.lastChar();
			if (c != '/' && c != ':')
				_sPath += '/';
		} else {
			//_bIsDirectory = false;
			_bIsValid = true;
		}

		IDOS->FreeDosObject(DOS_EXAMINEDATA, pExd);
	}

	LEAVE();
}

AmigaOSFilesystemNode::AmigaOSFilesystemNode(BPTR pLock, const char *pDisplayName) {
	ENTER();
	int bufSize = MAXPATHLEN;
	_pFileLock = 0;

	while (true) {
		char *n = new char[bufSize];
		if (IDOS->NameFromLock(pLock, (STRPTR)n, bufSize) != DOSFALSE) {
			_sPath = n;
			_sDisplayName = pDisplayName ? pDisplayName : IDOS->FilePart((STRPTR)n);
			delete[] n;
			break;
		}

		if (IDOS->IoErr() != ERROR_LINE_TOO_LONG) {
			_bIsValid = false;
			debug(6, "IoErr() != ERROR_LINE_TOO_LONG");
			LEAVE();
			delete[] n;
			return;
		}

		bufSize *= 2;
		delete[] n;
	}

	_bIsValid =	false;
	_bIsDirectory = false;

	struct ExamineData * pExd = IDOS->ExamineObjectTags(EX_FileLockInput,pLock,TAG_END);
	if (pExd) {
		_nProt = pExd->Protection;
		if (EXD_IS_DIRECTORY(pExd)) {
			_bIsDirectory = true;
			_pFileLock = IDOS->DupLock(pLock);
			_bIsValid = _pFileLock != 0;

			const char c = _sPath.lastChar();
			if (c != '/' && c != ':')
				_sPath += '/';
		} else {
			//_bIsDirectory = false;
			_bIsValid = true;
		}

        IDOS->FreeDosObject(DOS_EXAMINEDATA, pExd);
	} else {
		debug(6, "ExamineObject() returned NULL");
    }

	LEAVE();
}

// We need the custom copy constructor because of DupLock()
AmigaOSFilesystemNode::AmigaOSFilesystemNode(const AmigaOSFilesystemNode& node)
: AbstractFSNode() {
	ENTER();
	_sDisplayName = node._sDisplayName;
	_bIsValid = node._bIsValid;
	_bIsDirectory = node._bIsDirectory;
	_sPath = node._sPath;
	_pFileLock = IDOS->DupLock(node._pFileLock);
	_nProt = node._nProt;
	LEAVE();
}

AmigaOSFilesystemNode::~AmigaOSFilesystemNode() {
	ENTER();
	if (_pFileLock)
		IDOS->UnLock(_pFileLock);
	LEAVE();
}

bool AmigaOSFilesystemNode::exists() const {
	ENTER();
	if (_sPath.empty())
		return false;

	bool nodeExists = false;

	// previously we were trying to examine the node in order
	// to determine if the node exists or not.
	// I don't see the point : once you have been granted a
	// lock on it then it means it exists...
	//
	// =============================  Old code
	// BPTR pLock = IDOS->Lock((STRPTR)_sPath.c_str(), SHARED_LOCK);
	// if (pLock)
	// {
	// 	if (IDOS->Examine(pLock, fib) != DOSFALSE)
	// 		nodeExists = true;
	// 	IDOS->UnLock(pLock);
	// }
	//
	// IDOS->FreeDosObject(DOS_FIB, fib);
	//
	// =============================  New code
	BPTR pLock = IDOS->Lock(_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
		nodeExists = true;
		IDOS->UnLock(pLock);
	}

	LEAVE();
	return nodeExists;
}

AbstractFSNode *AmigaOSFilesystemNode::getChild(const Common::String &n) const {
	ENTER();
	if (!_bIsDirectory) {
		debug(6, "Not a directory");
		return 0;
	}

	Common::String newPath(_sPath);

	if (_sPath.lastChar() != '/')
		newPath += '/';

	newPath += n;

	LEAVE();
	return new AmigaOSFilesystemNode(newPath);
}

bool AmigaOSFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	ENTER();
	bool ret = false;

	//TODO: honor the hidden flag
	// There is nothing like a hidden flag under AmigaOS...

	if (!_bIsValid) {
		debug(6, "Invalid node");
		LEAVE();
		return false; // Empty list
	}

	if (!_bIsDirectory) {
		debug(6, "Not a directory");
		LEAVE();
		return false; // Empty list
	}

	if (_pFileLock == 0) {
		debug(6, "Root node");
		LEAVE();
		myList = listVolumes();
		return true;
	}

	APTR context = IDOS->ObtainDirContextTags(  EX_FileLockInput,	_pFileLock,
												EX_DoCurrentDir,	TRUE,  /* for softlinks */
												EX_DataFields,		(EXF_NAME|EXF_LINK|EXF_TYPE),
												TAG_END);
	if (context) {
		struct ExamineData * pExd = NULL; // NB: no need to free value after usage, all is dealt by the DirContext release

		AmigaOSFilesystemNode *entry ;
		while ( (pExd = IDOS->ExamineDir(context)) ) {
			if (     (EXD_IS_FILE(pExd) && ( Common::FSNode::kListFilesOnly == mode ))
				||  (EXD_IS_DIRECTORY(pExd) && ( Common::FSNode::kListDirectoriesOnly == mode ))
				||  Common::FSNode::kListAll == mode
				)
			{
				BPTR pLock = IDOS->Lock( pExd->Name, SHARED_LOCK );
				if (pLock) {
					entry = new AmigaOSFilesystemNode( pLock, pExd->Name );
					if (entry) {
						myList.push_back(entry);
					}

					IDOS->UnLock(pLock);
				}
			}
		}

		if (ERROR_NO_MORE_ENTRIES != IDOS->IoErr() ) {
			debug(6, "An error occurred during ExamineDir");
			ret = false;
		} else {
			ret = true;
		}


		IDOS->ReleaseDirContext(context);
	} else {
		debug(6, "Unable to ObtainDirContext");
		ret = false;
	}

	LEAVE();

	return ret;
}

AbstractFSNode *AmigaOSFilesystemNode::getParent() const {
	ENTER();

	if (!_bIsDirectory) {
		debug(6, "Not a directory");
		LEAVE();
		return 0;
	}

	if (_pFileLock == 0) {
		debug(6, "Root node");
		LEAVE();
		return new AmigaOSFilesystemNode(*this);
	}

	AmigaOSFilesystemNode *node;

	BPTR parentDir = IDOS->ParentDir( _pFileLock );
	if (parentDir) {
		node = new AmigaOSFilesystemNode(parentDir);
		IDOS->UnLock(parentDir);
	} else
		node = new AmigaOSFilesystemNode();

	LEAVE();

	return node;
}

bool AmigaOSFilesystemNode::isReadable() const {
	// Regular RWED protection flags are low-active or inverted, thus the negation.
	// moreover pseudo root filesystem (null _pFileLock) is readable whatever the
	// protection says
	bool readable = !(_nProt & EXDF_READ) || _pFileLock == 0;

	return readable;
}

bool AmigaOSFilesystemNode::isWritable() const {
	// Regular RWED protection flags are low-active or inverted, thus the negation.
	// moreover pseudo root filesystem (null _pFileLock) is never writable whatever
	// the protection says (because of the pseudo nature)
	bool writable = !(_nProt & EXDF_WRITE) && _pFileLock !=0;

	return writable;
}

AbstractFSList AmigaOSFilesystemNode::listVolumes() const {
	ENTER();

	AbstractFSList myList;

	const uint32 kLockFlags = LDF_READ | LDF_VOLUMES;
	char buffer[MAXPATHLEN];

	struct DosList *dosList = IDOS->LockDosList(kLockFlags);
	if (!dosList) {
		debug(6, "Cannot lock the DOS list");
		LEAVE();
		return myList;
	}

	dosList = IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	while (dosList) {
		if (dosList->dol_Type == DLT_VOLUME &&
			dosList->dol_Name &&
			dosList->dol_Task) {

			// Copy name to buffer
			IDOS->CopyStringBSTRToC(dosList->dol_Name, buffer, MAXPATHLEN);

			// Volume name + '\0'
			char *volName = new char [strlen(buffer) + 1];

			strcpy(volName, buffer);

			strcat(buffer, ":");

			BPTR volumeLock = IDOS->Lock((STRPTR)buffer, SHARED_LOCK);
			if (volumeLock) {

				char *devName = new char [MAXPATHLEN];

				// Find device name
				IDOS->DevNameFromLock(volumeLock, devName, MAXPATHLEN, DN_DEVICEONLY);

				sprintf(buffer, "%s (%s)", volName, devName);

				delete[] devName;

				AmigaOSFilesystemNode *entry = new AmigaOSFilesystemNode(volumeLock, buffer);
				if (entry) {
					myList.push_back(entry);
				}

				IDOS->UnLock(volumeLock);
			}

			delete[] volName;
		}
		dosList	= IDOS->NextDosEntry(dosList, LDF_VOLUMES);
	}

	IDOS->UnLockDosList(kLockFlags);

	LEAVE();

	return myList;
}

Common::SeekableReadStream *AmigaOSFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), false);
}

Common::WriteStream *AmigaOSFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath(), true);
}

#endif //defined(__amigaos4__)
