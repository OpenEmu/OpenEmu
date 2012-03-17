/***************************************************************************
 * Gens: File Compression Sub Class.                                       *
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

#ifndef GENS_SUBCOMPRESSOR_HPP
#define GENS_SUBCOMPRESSOR_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string>
#include <list>

// Compressed file struct
#include "compressedfile.hpp"

using std::string;
using std::list;

class SubCompressor
{
	public:
		SubCompressor(const bool showErrMsg = false);
		virtual ~SubCompressor();
		
		virtual bool detectFormat(FILE *f) = 0;
		virtual int getNumFiles(const string& zFilename) = 0;
		virtual list<CompressedFile>* getFileInfo(const string& zFilename) = 0;
		virtual int getFile(const string& zFilename, const CompressedFile *fileInfo,
				    unsigned char *buf, const int size) = 0;
		
		virtual bool checkExternalExec(void) { return true; }
	
	protected:
		bool m_showErrMsg;
};

// Various subcompressors.

// Subcompressors that require zlib.
#ifdef GENS_ZLIB
#include "gzip.hpp"
#include "Zip.hpp"
#endif /* GENS_ZLIB */

#include "7z.hpp"
#include "rar.hpp"

#endif /* GENS_SUBCOMPRESSOR_HPP */
