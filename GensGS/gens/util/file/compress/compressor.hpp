/***************************************************************************
 * Gens: File Compression Base Class.                                      *
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

#ifndef GENS_COMPRESSOR_HPP
#define GENS_COMPRESSOR_HPP

#include <stdio.h>
#include <string>
#include <list>

using std::string;
using std::list;

// Compressed file struct
#include "compressedfile.hpp"

// Subcompressor
#include "subcompressor.hpp"

string fileNameOnly(const string& filename);

class Compressor
{
	public:
		Compressor(string filename, bool showErrMsg = false);
		~Compressor();
		
		bool isFileLoaded(void);
		int getNumFiles(void);
		list<CompressedFile>* getFileInfo(void);
		int getFile(const CompressedFile *fileInfo, unsigned char *buf, int size);
	
	private:
		SubCompressor *m_subCompressor;
		string m_zFilename;
		FILE *m_zf;
		bool m_fileLoaded;
};

#endif
