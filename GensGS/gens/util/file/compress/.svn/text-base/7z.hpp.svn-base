/***************************************************************************
 * Gens: 7z File Compression Class.                                        *
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

#ifndef GENS_7Z_HPP
#define GENS_7Z_HPP

#include <stdio.h>
#include <string>
#include <list>

// Subcompressor
#include "subcompressor.hpp"

class _7z : public SubCompressor
{
	public:
		_7z(const bool showErrMsg = false);
		~_7z();
		
		bool detectFormat(FILE *f);
		int getNumFiles(const std::string& zFilename);
		std::list<CompressedFile>* getFileInfo(const std::string& zFilename);
		int getFile(const std::string& zFilename,
			    const CompressedFile *fileInfo,
			    unsigned char *buf, const int size);
		
		bool checkExternalExec(void);
	
	protected:
		void errOpening7z(const int errorNumber);
};

#endif /* GENS_7Z_HPP */
