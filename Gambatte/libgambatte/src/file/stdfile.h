/***************************************************************************
Copyright (C) 2007 by Nach
http://nsrt.edgeemu.com

Copyright (C) 2007-2011 by Sindre Aamås
sinamas@users.sourceforge.net

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License version 2 for more details.

You should have received a copy of the GNU General Public License
version 2 along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
***************************************************************************/
#ifndef GAMBATTE_STD_FILE_H
#define GAMBATTE_STD_FILE_H

#include "file.h"
#include <fstream>

namespace gambatte {

class StdFile : public File {
	std::ifstream stream;
	std::size_t fsize;

public:
	explicit StdFile(const char *filename)
	: stream(filename, std::ios::in | std::ios::binary), fsize(0)
	{
		if (stream) {
			stream.seekg(0, std::ios::end);
			fsize = stream.tellg();
			stream.seekg(0, std::ios::beg);
		}
	}
	
	virtual void rewind() { stream.seekg(0, std::ios::beg); }
	virtual std::size_t size() const { return fsize; };
	virtual void read(char *buffer, std::size_t amount) { stream.read(buffer, amount); }
	virtual bool fail() const { return stream.fail(); }
};

}

#endif
