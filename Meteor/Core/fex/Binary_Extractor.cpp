// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "Binary_Extractor.h"

/* Copyright (C) 2005-2009 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#include "blargg_source.h"

// TODO: could close file once data has been read into memory

static File_Extractor* new_binary()
{
	return BLARGG_NEW Binary_Extractor;
}

fex_type_t_ const fex_bin_type [1] = {{
	"",
	&new_binary,
	"file",
	NULL
}};

Binary_Extractor::Binary_Extractor() :
	File_Extractor( fex_bin_type )
{ }

Binary_Extractor::~Binary_Extractor()
{
	close();
}

blargg_err_t Binary_Extractor::open_path_v()
{
	set_name( arc_path() );
	return blargg_ok;
}

blargg_err_t Binary_Extractor::open_v()
{
	set_name( arc_path() );
	set_info( arc().remain(), 0, 0 );
	return blargg_ok;
}

void Binary_Extractor::close_v()
{ }

blargg_err_t Binary_Extractor::next_v()
{
	return blargg_ok;
}

blargg_err_t Binary_Extractor::rewind_v()
{
	return open_path_v();
}

blargg_err_t Binary_Extractor::stat_v()
{
	RETURN_ERR( open_arc_file() );
	RETURN_ERR( arc().seek( 0 ) );
	return open_v();
}

blargg_err_t Binary_Extractor::extract_v( void* p, int n )
{
	return arc().read( p, n );
}
