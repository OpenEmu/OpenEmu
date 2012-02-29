// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "Gzip_Reader.h"

#include "blargg_endian.h"

/* Copyright (C) 2006-2009 Shay Green. This module is free software; you
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

Gzip_Reader::Gzip_Reader()
{
	close();
}

Gzip_Reader::~Gzip_Reader()
{ }

static blargg_err_t gzip_reader_read( void* file, void* out, int* count )
{
	return STATIC_CAST(File_Reader*,file)->read_avail( out, count );
}

blargg_err_t Gzip_Reader::calc_size()
{
	size_  = in->size();
	crc32_ = 0;
	if ( inflater.deflated() )
	{
		byte trailer [8];
		int old_pos = in->tell();
		RETURN_ERR( in->seek( size_ - sizeof trailer ) );
		RETURN_ERR( in->read( trailer, sizeof trailer ) );
		RETURN_ERR( in->seek( old_pos ) );
		crc32_ = get_le32( trailer + 0 );
		
		unsigned n = get_le32( trailer + 4 );
		if ( n > INT_MAX )
			return BLARGG_ERR( BLARGG_ERR_FILE_FEATURE, "gzip larger than 2GB" );
		
		size_ = n;
	}
	return blargg_ok;
}

blargg_err_t Gzip_Reader::open( File_Reader* new_in )
{
	close();
	
	in = new_in;
	RETURN_ERR( in->seek( 0 ) );
	RETURN_ERR( inflater.begin( gzip_reader_read, new_in ) );
	RETURN_ERR( inflater.set_mode( inflater.mode_auto ) );
	RETURN_ERR( calc_size() );
	set_remain( size_ );
	
	return blargg_ok;
}

void Gzip_Reader::close()
{
	in = NULL;
	inflater.end();
}

blargg_err_t Gzip_Reader::read_v( void* out, int count )
{
	assert( in );
	int actual = count;
	RETURN_ERR( inflater.read( out, &actual ) );
	
	if ( actual != count )
		return blargg_err_file_corrupt;
	
	return blargg_ok;
}
