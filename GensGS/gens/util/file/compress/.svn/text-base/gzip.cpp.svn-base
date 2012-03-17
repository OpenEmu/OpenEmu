/***************************************************************************
 * Gens: GZip File Compression Class.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "gzip.hpp"

#include <zlib.h>

using std::string;
using std::list;


GZip::GZip(const bool showErrMsg)
{
	m_showErrMsg = showErrMsg;
}

GZip::~GZip()
{
}


/**
 * detectFormat(): Detect if a file is in GZip format.
 * @param f File pointer of the file to check.
 * @return True if this file is in GZip format.
 */
bool GZip::detectFormat(FILE *f)
{
	// Magic Number for GZip:
	// First two bytes: "\x1F\x8B"
	unsigned char buf[2];
	fseek(f, 0, SEEK_SET);
	fread(buf, 2, sizeof(unsigned char), f);
	return (buf[0] == 0x1F && buf[1] == 0x8B);
}


/**
 * getNumFiles(): Gets the number of files in the specified archive.
 * @param zFilename Filename of the archive.
 * @return Number of files, or 0 on error.
 */
int GZip::getNumFiles(const string& zFilename)
{
	// GZip files always contain a single file.
	return 1;
}


/**
 * getFileInfo(): Get information about all files in the specified archive.
 * @param zFilename Filename of the archive.
 * @return Pointer to list of CompressedFile structs, or NULL on error.
 */
list<CompressedFile>* GZip::getFileInfo(const string& zFilename)
{
	list<CompressedFile> *lst;
	CompressedFile file;
	
	gzFile gzfd;
	char buf[1024];
	
	// Copy the filename to the struct.
	file.filename = zFilename;
	
	gzfd = gzopen(zFilename.c_str(), "rb");
	if (!gzfd)
	{
		// Error obtaining a GZip file descriptor.
		return NULL;
	}
	
	// Read through the GZip file until we hit an EOF.
	file.filesize = 0;
	while (!gzeof(gzfd))
	{
		file.filesize += gzread(gzfd, buf, 1024);
	}
	
	// Close the GZip fd.
	gzclose(gzfd);
	
	// Create the list and add the file entry.
	lst = new list<CompressedFile>;
	lst->push_back(file);
	
	// Done.
	return lst;
}


/**
 * getFile(): Gets the specified file from the specified archive.
 * @param zFilename Filename of the archive.
 * @param fileInfo Information about the file to extract. (Unused in the GZip handler.)
 * @param buf Buffer to write the file to.
 * @param size Size of the buffer, in bytes.
 * @return Number of bytes read, or -1 on error.
 */
int GZip::getFile(const string& zFilename, const CompressedFile *fileInfo,
		  unsigned char *buf, const int size)
{
	gzFile gzfd;
	int retval;
	
	// All parameters (except fileInfo) must be specified.
	if (!buf || !size)
		return 0;
	
	gzfd = gzopen(zFilename.c_str(), "rb");
	if (!gzfd)
	{
		// Error obtaining a GZip file descriptor.
		return -1;
	}
	
	// Decompress the GZip file into memory.
	retval = gzread(gzfd, buf, size);
	gzclose(gzfd);
	return retval;
}
