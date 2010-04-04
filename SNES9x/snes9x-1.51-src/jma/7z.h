/*
Copyright (C) 2005-2006 NSRT Team ( http://nsrt.edgeemu.com )
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License version 2.1 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __7Z_H
#define __7Z_H

#include "iiostrm.h"

bool decompress_lzma_7z(ISequentialInStream& in, unsigned in_size, ISequentialOutStream& out, unsigned out_size) throw ();
bool decompress_lzma_7z(const unsigned char* in_data, unsigned in_size, unsigned char* out_data, unsigned out_size) throw ();

#endif

