// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "File_Extractor.h"

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

File_Extractor::fex_t( fex_type_t t ) :
	type_( t )
{
	own_file_ = NULL;
	
	close_();
}

// Open

blargg_err_t File_Extractor::set_path( const char* path )
{
	if ( !path )
		path = "";
	
	RETURN_ERR( path_.resize( strlen( path ) + 1 ) );
	memcpy( path_.begin(), path, path_.size() );
	return blargg_ok;
}

blargg_err_t File_Extractor::open( const char path [] )
{
	close();
	
	RETURN_ERR( set_path( path ) );
	
	blargg_err_t err = open_path_v();
	if ( err )
		close();
	else
		opened_ = true;
	
	return err;
}

blargg_err_t File_Extractor::open_path_v()
{
	RETURN_ERR( open_arc_file() );
	
	return open_v();
}

inline
static void make_unbuffered( Std_File_Reader* r )
{
	r->make_unbuffered();
}

inline
static void make_unbuffered( void* )
{ }

blargg_err_t File_Extractor::open_arc_file( bool unbuffered )
{
	if ( reader_ )
		return blargg_ok;
	
	FEX_FILE_READER* in = BLARGG_NEW FEX_FILE_READER;
	CHECK_ALLOC( in );
	
	blargg_err_t err = in->open( arc_path() );
	if ( err )
	{
		delete in;
	}
	else
	{
		reader_ = in;
		own_file();
		if ( unbuffered )
			make_unbuffered( in );
	}
	
	return err;
}

blargg_err_t File_Extractor::open( File_Reader* input, const char* path )
{
	close();
	
	RETURN_ERR( set_path( path ) );
	
	RETURN_ERR( input->seek( 0 ) );
	
	reader_ = input;
	blargg_err_t err = open_v();
	if ( err )
		close();
	else
		opened_ = true;
	
	return err;
}

// Close

void File_Extractor::close()
{
	close_v();
	close_();
}

void File_Extractor::close_()
{
	delete own_file_;
	own_file_ = NULL;
	
	tell_   = 0;
	reader_ = NULL;
	opened_ = false;
	
	path_.clear();
	clear_file();
}

File_Extractor::~fex_t()
{
	check( !opened() ); // fails if derived destructor didn't call close()
	
	delete own_file_;
}

// Scanning

void File_Extractor::clear_file()
{
	name_       = NULL;
	wname_      = NULL;
	done_       = true;
	stat_called = false;
	data_ptr_   = NULL;
	
	set_info( 0 );
	own_data_.clear();
	clear_file_v();
}

void File_Extractor::set_name( const char new_name [], const wchar_t* new_wname )
{
	name_  = new_name;
	wname_ = new_wname;
	done_  = false;
}

void File_Extractor::set_info( int new_size, unsigned date, unsigned crc )
{
	size_  = new_size;
	date_  = (date != 0xFFFFFFFF ? date : 0);
	crc32_ = crc;
	set_remain( new_size );
}

blargg_err_t File_Extractor::next_()
{
	tell_++;
	clear_file();
	
	blargg_err_t err = next_v();
	if ( err )
		clear_file();
	
	return err;
}

blargg_err_t File_Extractor::next()
{
	assert( !done() );
	return next_();
}

blargg_err_t File_Extractor::rewind()
{
	assert( opened() );

	tell_ = 0;
	clear_file();

	blargg_err_t err = rewind_v();
	if ( err )
		clear_file();
	
	return err;
}

blargg_err_t File_Extractor::stat()
{
	assert( !done() );
	
	if ( !stat_called )
	{
		RETURN_ERR( stat_v() );
		stat_called = true;
	}
	return blargg_ok;
}

// Tell/seek

int const pos_offset = 1;

fex_pos_t File_Extractor::tell_arc() const
{
	assert( opened() );
	
	fex_pos_t pos = tell_arc_v();
	assert( pos >= 0 );
	
	return pos + pos_offset;
}

blargg_err_t File_Extractor::seek_arc( fex_pos_t pos )
{
	assert( opened() );
	assert( pos != 0 );
	
	clear_file();
	
	blargg_err_t err = seek_arc_v( pos - pos_offset );
	if ( err )
		clear_file();
	
	return err;
}

fex_pos_t File_Extractor::tell_arc_v() const
{
	return tell_;
}

blargg_err_t File_Extractor::seek_arc_v( fex_pos_t pos )
{
	// >= because seeking to current file should always reset read pointer etc.
	if ( tell_ >= pos )
		RETURN_ERR( rewind() );
	
	while ( tell_ < pos )
	{
		RETURN_ERR( next_() );
		
		if ( done() )
		{
			assert( false );
			return blargg_err_caller;
		}
	}
	
	assert( tell_ == pos );
	
	return blargg_ok;
}

// Extraction

blargg_err_t File_Extractor::rewind_file()
{
	RETURN_ERR( stat() );
	
	if ( tell() > 0 )
	{
		if ( data_ptr_ )
		{
			set_remain( size() );
		}
		else
		{
			RETURN_ERR( seek_arc( tell_arc() ) );
			RETURN_ERR( stat() );
		}
	}
	
	return blargg_ok;
}

blargg_err_t File_Extractor::data( const void** data_out )
{
	assert( !done() );
	
	*data_out = NULL;
	if ( !data_ptr_ )
	{
		int old_tell = tell();
		
		RETURN_ERR( rewind_file() );
		
		void const* ptr;
		RETURN_ERR( data_v( &ptr ) );
		data_ptr_ = ptr;
		
		// Now that data is in memory, we can seek by simply setting remain
		set_remain( size() - old_tell );
	}
	
	*data_out = data_ptr_;
	return blargg_ok;
}

blargg_err_t File_Extractor::data_v( void const** out )
{
	RETURN_ERR( own_data_.resize( size() ) );
	*out = own_data_.begin();
	
	blargg_err_t err = extract_v( own_data_.begin(), own_data_.size() );
	if ( err )
		own_data_.clear();
	
	return err;
}

blargg_err_t File_Extractor::extract_v( void* out, int count )
{
	void const* p;
	RETURN_ERR( data( &p ) );
	memcpy( out, STATIC_CAST(char const*,p) + (size() - remain()), count );
	
	return blargg_ok;
}

blargg_err_t File_Extractor::read_v( void* out, int count )
{
	if ( data_ptr_ )
		return File_Extractor::extract_v( out, count );
	
	return extract_v( out, count );
}
