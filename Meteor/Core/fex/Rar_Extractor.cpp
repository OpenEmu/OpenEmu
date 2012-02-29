// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "blargg_common.h"

#if FEX_ENABLE_RAR

#include "Rar_Extractor.h"

/* Copyright (C) 2009 Shay Green. This module is free software; you
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

static blargg_err_t init_rar()
{
	unrar_init();
	return blargg_ok;
}

static File_Extractor* new_rar()
{
	return BLARGG_NEW Rar_Extractor;
}

fex_type_t_ const fex_rar_type [1] = {{
	".rar",
	&new_rar,
	"RAR archive",
	&init_rar
}};

blargg_err_t Rar_Extractor::convert_err( unrar_err_t err )
{
	blargg_err_t reader_err = reader.err;
	reader.err = blargg_ok;
	if ( reader_err )
		check( err == unrar_next_err );
	
	switch ( err )
	{
	case unrar_ok:              return blargg_ok;
	case unrar_err_memory:      return blargg_err_memory;
	case unrar_err_open:        return blargg_err_file_read;
	case unrar_err_not_arc:     return blargg_err_file_type;
	case unrar_err_corrupt:     return blargg_err_file_corrupt;
	case unrar_err_io:          return blargg_err_file_io;
	case unrar_err_arc_eof:     return blargg_err_internal;
	case unrar_err_encrypted:   return BLARGG_ERR( BLARGG_ERR_FILE_FEATURE, "RAR encryption not supported" );
	case unrar_err_segmented:   return BLARGG_ERR( BLARGG_ERR_FILE_FEATURE, "RAR segmentation not supported" );
	case unrar_err_huge:        return BLARGG_ERR( BLARGG_ERR_FILE_FEATURE, "Huge RAR files not supported" );
	case unrar_err_old_algo:    return BLARGG_ERR( BLARGG_ERR_FILE_FEATURE, "Old RAR compression not supported" );
	case unrar_err_new_algo:    return BLARGG_ERR( BLARGG_ERR_FILE_FEATURE, "RAR uses unknown newer compression" );
	case unrar_next_err:        break;
	default:
		check( false ); // unhandled RAR error
	}
	
	if ( reader_err )
		return reader_err;
	
	check( false );
	return BLARGG_ERR( BLARGG_ERR_INTERNAL, "RAR archive" );
}

static inline unrar_err_t handle_err( Rar_Extractor::read_callback_t* h, blargg_err_t err )
{
	if ( !err )
		return unrar_ok;
	
	h->err = err;
	return unrar_next_err;
}

extern "C"
{
	static unrar_err_t my_unrar_read( void* data, void* out, int* count, unrar_pos_t pos )
	{
		// TODO: 64-bit file support
		
		Rar_Extractor::read_callback_t* h = STATIC_CAST(Rar_Extractor::read_callback_t*,data);
		if ( h->pos != pos )
		{
			blargg_err_t err = h->in->seek( pos );
			if ( err )
				return handle_err( h, err );
			
			h->pos = pos;
		}
		
		blargg_err_t err = h->in->read_avail( out, count );
		if ( err )
			return handle_err( h, err );
		
		h->pos += *count;
		
		return unrar_ok;
	}
}

Rar_Extractor::Rar_Extractor() :
	File_Extractor( fex_rar_type )
{
	unrar = NULL;
}

Rar_Extractor::~Rar_Extractor()
{
	close();
}

blargg_err_t Rar_Extractor::open_v()
{
	reader.pos = 0;
	reader.in  = &arc();
	reader.err = blargg_ok;
	
	RETURN_ERR( arc().seek( 0 ) );
	RETURN_ERR( convert_err( unrar_open_custom( &unrar, &my_unrar_read, &reader ) ) );
	return skip_unextractables();
}

void Rar_Extractor::close_v()
{
	unrar_close( unrar );
	
	unrar     = NULL;
	reader.in = NULL;
}

blargg_err_t Rar_Extractor::skip_unextractables()
{
	while ( !unrar_done( unrar ) && unrar_try_extract( unrar ) )
		RETURN_ERR( next_raw() );
	
	if ( !unrar_done( unrar ) )
	{
		unrar_info_t const* info = unrar_info( unrar );
		
		set_name( info->name, (info->name_w && *info->name_w) ? info->name_w : NULL );
		set_info( info->size, info->dos_date, (info->is_crc32 ? info->crc : 0) );
	}
	
	return blargg_ok;
}

blargg_err_t Rar_Extractor::next_raw()
{
	return convert_err( unrar_next( unrar ) );
}

blargg_err_t Rar_Extractor::next_v()
{
	RETURN_ERR( next_raw() );
	return skip_unextractables();
}

blargg_err_t Rar_Extractor::rewind_v()
{
	RETURN_ERR( convert_err( unrar_rewind( unrar ) ) );
	return skip_unextractables();
}

fex_pos_t Rar_Extractor::tell_arc_v() const
{
	return unrar_tell( unrar );
}

blargg_err_t Rar_Extractor::seek_arc_v( fex_pos_t pos )
{
	RETURN_ERR( convert_err( unrar_seek( unrar, pos ) ) );
	return skip_unextractables();
}

blargg_err_t Rar_Extractor::data_v( void const** out )
{
	return convert_err( unrar_extract_mem( unrar, out ) );
}

blargg_err_t Rar_Extractor::extract_v( void* out, int count )
{
	// We can read entire file directly into user buffer
	if ( count == size() )
		return convert_err( unrar_extract( unrar, out, count ) );
	
	// This will call data_v() and copy from that buffer for us
	return File_Extractor::extract_v( out, count );
}

#endif
