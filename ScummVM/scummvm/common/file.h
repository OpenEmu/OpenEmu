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

#ifndef COMMON_FILE_H
#define COMMON_FILE_H

#include "common/scummsys.h"
#include "common/fs.h"
#include "common/noncopyable.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {

class Archive;

/**
 * TODO: vital to document this core class properly!!! For both users and implementors
 */
class File : public SeekableReadStream, public NonCopyable {
protected:
	/** File handle to the actual file; 0 if no file is open. */
	SeekableReadStream *_handle;

	/** The name of this file, kept for debugging purposes. */
	String _name;

public:
	File();
	virtual ~File();

	/**
	 * Checks if a given file exists in any of the current default paths,
	 * as defined by SearchMan.
	 *
	 * @param	filename	the file to check for
	 * @return	true if the file exists, false otherwise
	 */
	static bool exists(const String &filename);

	/**
	 * Try to open the file with the given filename, by searching SearchMan.
	 * @note Must not be called if this file already is open (i.e. if isOpen returns true).
	 *
	 * @param	filename	the name of the file to open
	 * @return	true if file was opened successfully, false otherwise
	 */
	virtual bool open(const String &filename);

	/**
	 * Try to open the file with the given filename from within the given archive.
	 * @note Must not be called if this file already is open (i.e. if isOpen returns true).
	 *
	 * @param	filename	the name of the file to open
	 * @param	archive		the archive in which to search for the file
	 * @return	true if file was opened successfully, false otherwise
	 */
	virtual bool open(const String &filename, Archive &archive);

	/**
	 * Try to open the file corresponding to the give node. Will check whether the
	 * node actually refers to an existing file (and not a directory), and handle
	 * those cases gracefully.
	 * @note Must not be called if this file already is open (i.e. if isOpen returns true).
	 *
	 * @param	filename	the name of the file to open
	 * @param	archive		the archive in which to search for the file
	 * @return	true if file was opened successfully, false otherwise
	 */
	virtual bool open(const FSNode &node);

	/**
	 * Try to 'open' the given stream. That is, we just wrap around it, and if stream
	 * is a NULL pointer, we gracefully treat this as if opening failed.
	 * @note Must not be called if this file already is open (i.e. if isOpen returns true).
	 *
	 * @param	stream		a pointer to a SeekableReadStream, or 0
	 * @param	name		a string describing the 'file' corresponding to stream
	 * @return	true if stream was non-zero, false otherwise
	 */
	virtual bool open(SeekableReadStream *stream, const String &name);

	/**
	 * Close the file, if open.
	 */
	virtual void close();

	/**
	 * Checks if the object opened a file successfully.
	 *
	 * @return: true if any file is opened, false otherwise.
	 */
	bool isOpen() const;

	/**
	 * Returns the filename of the opened file for debugging purposes.
	 *
	 * @return: the filename
	 */
	const char *getName() const { return _name.c_str(); }

	bool err() const;	// implement abstract Stream method
	void clearErr();	// implement abstract Stream method
	bool eos() const;	// implement abstract SeekableReadStream method

	int32 pos() const;	// implement abstract SeekableReadStream method
	int32 size() const;	// implement abstract SeekableReadStream method
	bool seek(int32 offs, int whence = SEEK_SET);	// implement abstract SeekableReadStream method
	uint32 read(void *dataPtr, uint32 dataSize);	// implement abstract SeekableReadStream method
};


/**
 * TODO: document this class
 *
 * Some design ideas:
 *  - automatically drop all files into dumps/ dir? Might not be desired in all cases
 */
class DumpFile : public WriteStream, public NonCopyable {
protected:
	/** File handle to the actual file; 0 if no file is open. */
	WriteStream *_handle;

public:
	DumpFile();
	virtual ~DumpFile();

	virtual bool open(const String &filename);
	virtual bool open(const FSNode &node);

	virtual void close();

	/**
	 * Checks if the object opened a file successfully.
	 *
	 * @return: true if any file is opened, false otherwise.
	 */
	bool isOpen() const;

	bool err() const;
	void clearErr();

	virtual uint32 write(const void *dataPtr, uint32 dataSize);

	virtual bool flush();
};

} // End of namespace Common

#endif
