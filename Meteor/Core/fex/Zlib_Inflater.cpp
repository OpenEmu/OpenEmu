// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "Zlib_Inflater.h"

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

int const block_size = 4096;

static const char* get_zlib_err( int code )
{
	assert( code != Z_OK );
	switch ( code )
	{
	case Z_MEM_ERROR:   return blargg_err_memory;
	case Z_DATA_ERROR:  return blargg_err_file_corrupt;
	// TODO: handle more error codes
	}

	const char* str = zError( code );
	if ( !str )
		str = BLARGG_ERR( BLARGG_ERR_GENERIC, "problem unzipping data" );
	
	return str;
}

void Zlib_Inflater::end()
{
	if ( deflated_ )
	{
		deflated_ = false;
		if ( inflateEnd( &zbuf ) )
			check( false );
	}
	buf.clear();

	static z_stream const empty = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	memcpy( &zbuf, &empty, sizeof zbuf );
}

Zlib_Inflater::Zlib_Inflater()
{
	deflated_ = false;
	end(); // initialize things
}

Zlib_Inflater::~Zlib_Inflater()
{
	end();
}

blargg_err_t Zlib_Inflater::fill_buf( int count )
{
	byte* out = buf.end() - count;
	RETURN_ERR( callback( user_data, out, &count ) );
	zbuf.avail_in = count;
	zbuf.next_in  = out;
	return blargg_ok;
}

blargg_err_t Zlib_Inflater::begin( callback_t new_callback, void* new_user_data,
		int new_buf_size, int initial_read )
{
	callback  = new_callback;
	user_data = new_user_data;

	end();

	// TODO: decide whether using different size on alloc failure is a good idea
	//RETURN_ERR( buf.resize( new_buf_size ? new_buf_size : 4 * block_size ) );
	if ( new_buf_size && buf.resize( new_buf_size ) )
	{
		ACK_FAILURE();
		new_buf_size = 0;
	}
	
	if ( !new_buf_size )
	{
		RETURN_ERR( buf.resize( 4 * block_size ) );
		initial_read = 0;
	}
	
	// Fill buffer with some data, less than normal buffer size since caller might
	// just be examining beginning of file.
	return fill_buf( initial_read ? initial_read : block_size );
}

blargg_err_t Zlib_Inflater::set_mode( mode_t mode, int data_offset )
{
	zbuf.next_in  += data_offset;
	zbuf.avail_in -= data_offset;

	if ( mode == mode_auto )
	{
		// examine buffer for gzip header
		mode = mode_copy;
		unsigned const min_gzip_size = 2 + 8 + 8;
		if ( zbuf.avail_in >= min_gzip_size &&
				zbuf.next_in [0] == 0x1F && zbuf.next_in [1] == 0x8B )
			mode = mode_ungz;
	}

	if ( mode != mode_copy )
	{
		int wb = MAX_WBITS + 16; // have zlib handle gzip header
		if ( mode == mode_raw_deflate )
			wb = -MAX_WBITS;

		int zerr = inflateInit2( &zbuf, wb );
		if ( zerr )
		{
			zbuf.next_in = NULL;
			return get_zlib_err( zerr );
		}

		deflated_ = true;
	}
	return blargg_ok;
}

/*
// Reads/inflates entire stream. All input must be in buffer, and count must be total
// of all output.
blargg_err_t read_all( void* out, int count );


// zlib automatically applies this optimization (uses inflateFast)
// TODO: remove
blargg_err_t Zlib_Inflater::read_all( void* out, int count )
{
	if ( deflated_ )
	{
		zbuf.next_out  = (Bytef*) out;
		zbuf.avail_out = count;
		
		int err = inflate( &zbuf, Z_FINISH );
		
		if ( zbuf.avail_out || err != Z_STREAM_END )
			return blargg_err_file_corrupt;
	}
	else
	{
		if ( zbuf.avail_in < count )
			return blargg_err_file_corrupt;
		
		memcpy( out, zbuf.next_in, count );
		
		zbuf.next_in  += count;
		zbuf.avail_in -= count;
	}
	
	return blargg_ok;
}
*/

blargg_err_t Zlib_Inflater::read( void* out, int* count_io )
{
	int remain = *count_io;
	if ( remain && zbuf.next_in )
	{
		if ( deflated_ )
		{
			zbuf.next_out  = (Bytef*) out;
			zbuf.avail_out = remain;
			
			while ( 1 )
			{
				uInt old_avail_in = zbuf.avail_in;
				int err = inflate( &zbuf, Z_NO_FLUSH );
				if ( err == Z_STREAM_END )
				{
					remain = zbuf.avail_out;
					end();
					break; // no more data to inflate
				}

				if ( err && (err != Z_BUF_ERROR || old_avail_in) )
					return get_zlib_err( err );

				if ( !zbuf.avail_out )
				{
					remain = 0;
					break; // requested number of bytes inflated
				}

				if ( zbuf.avail_in )
				{
					// inflate() should never leave input if there's still space for output
					check( false );
					return blargg_err_file_corrupt;
				}

				RETURN_ERR( fill_buf( buf.size() ) );
				if ( !zbuf.avail_in )
					return blargg_err_file_corrupt; // stream didn't end but there's no more data
			}
		}
		else
		{
			while ( 1 )
			{
				// copy buffered data
				if ( zbuf.avail_in )
				{
					long count = zbuf.avail_in;
					if ( count > remain )
						count = remain;
					memcpy( out, zbuf.next_in, count );
					zbuf.total_out += count;
					out = (char*) out + count;
					remain        -= count;
					zbuf.next_in  += count;
					zbuf.avail_in -= count;
				}

				if ( !zbuf.avail_in && zbuf.next_in < buf.end() )
				{
					end();
					break;
				}

				// read large request directly
				if ( remain + zbuf.total_out % block_size >= buf.size() )
				{
					int count = remain;
					RETURN_ERR( callback( user_data, out, &count ) );
					zbuf.total_out += count;
					out = (char*) out + count;
					remain -= count;

					if ( remain )
					{
						end();
						break;
					}
				}

				if ( !remain )
					break;

				RETURN_ERR( fill_buf( buf.size() - zbuf.total_out % block_size ) );
			}
		}
	}
	*count_io -= remain;
	return blargg_ok;
}
