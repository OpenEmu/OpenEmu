// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "fex.h"

#include "File_Extractor.h"
#include "blargg_endian.h"
#include <string.h>
#include <ctype.h>

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


//// Types

BLARGG_EXPORT const fex_type_t* fex_type_list( void )
{
	static fex_type_t const fex_type_list_ [] =
	{
		#ifdef FEX_TYPE_LIST
			FEX_TYPE_LIST
		#else
			// Modify blargg_config.h to change type list, NOT this file
			//fex_7z_type,
			fex_gz_type,
			#if FEX_ENABLE_RAR
				fex_rar_type,
			#endif
			fex_zip_type,
		#endif
		fex_bin_type,
		NULL
	};
	
	return fex_type_list_;
}

BLARGG_EXPORT fex_err_t fex_init( void )
{
	static bool inited;
	if ( !inited )
	{
		for ( fex_type_t const* t = fex_type_list(); *t != NULL; ++t )
		{
			if ( (*t)->init )
				RETURN_ERR( (*t)->init() );
		}
		inited = true;
	}
	return blargg_ok;
}

BLARGG_EXPORT const char* fex_identify_header( void const* header )
{
	unsigned four = get_be32( header );
	switch ( four )
	{
	case 0x52457E5E:
	case 0x52617221: return ".rar";

	//case 0x377ABCAF: return ".7z";

	case 0x504B0304:
	case 0x504B0506: return ".zip";

	case 0x53495421: return ".sit";
	case 0x41724301: return ".arc";
	case 0x4D534346: return ".cab";
	case 0x5A4F4F20: return ".zoo";
	}

	unsigned three = four >> 8;
	switch ( three )
	{
	case 0x425A68: return ".bz2";
	}

	unsigned two = four >> 16;
	switch ( two )
	{
	case 0x1F8B: return ".gz";
	case 0x60EA: return ".arj";
	}
	
	unsigned skip_first_two = four & 0xFFFF;
	if ( skip_first_two == 0x2D6C )
		return ".lha";
	
	return "";
}

static int fex_has_extension_( const char str [], const char suffix [], size_t str_len )
{
	size_t suffix_len = strlen( suffix );
	if ( str_len >= suffix_len )
	{
		str += str_len - suffix_len;
		while ( *str && tolower( (unsigned char) *str ) == *suffix )
		{
			str++;
			suffix++;
		}
	}
	return *suffix == 0;
}

BLARGG_EXPORT int fex_has_extension( const char str [], const char suffix [] )
{
	return fex_has_extension_( str, suffix, strlen( str ) );
}

static int is_archive_extension( const char str [] )
{
	static const char exts [] [6] = {
		//".7z",
		".arc",
		".arj",
		".bz2",
		".cab",
		".dmg",
		".gz",
		".lha",
		".lz",
		".lzh",
		".lzma",
		".lzo",
		".lzx",
		".pea",
		".rar",
		".sit",
		".sitx",
		".tgz",
		".tlz",
		".z",
		".zip",
		".zoo",
		""
	};
	
	size_t str_len = strlen( str );
	const char (*ext) [6] = exts;
	for ( ; **ext; ext++ )
	{
		if ( fex_has_extension_( str, *ext, str_len ) )
			return 1;
	}
	return 0;
}

BLARGG_EXPORT fex_type_t fex_identify_extension( const char str [] )
{
	size_t str_len = strlen( str );
	for ( fex_type_t const* types = fex_type_list(); *types; types++ )
	{
		if ( fex_has_extension_( str, (*types)->extension, str_len ) )
		{
			// Avoid treating known archive type as binary
			if ( *(*types)->extension || !is_archive_extension( str ) )
				return *types;
		}
	}
	return NULL;
}

BLARGG_EXPORT fex_err_t fex_identify_file( fex_type_t* type_out, const char path [] )
{
	*type_out = NULL;
	
	fex_type_t type = fex_identify_extension( path );
	
	// Unsupported extension?
	if ( !type )
		return blargg_ok; // reject
	
	// Unknown/no extension?
	if ( !*(type->extension) )
	{
		// Examine header
		FEX_FILE_READER in;
		RETURN_ERR( in.open( path ) );
		if ( in.remain() >= fex_identify_header_size )
		{
			char h [fex_identify_header_size];
			RETURN_ERR( in.read( h, sizeof h ) );
			
			type = fex_identify_extension( fex_identify_header( h ) );
		}
	}
	
	*type_out = type;
	return blargg_ok;
}

BLARGG_EXPORT fex_err_t fex_open_type( fex_t** fe_out, const char path [], fex_type_t type )
{
	*fe_out = NULL;
	
	if ( !type )
		return blargg_err_file_type;

	fex_t* fe = type->new_fex();
	CHECK_ALLOC( fe );

	fex_err_t err = fe->open( path );
	if ( err )
	{
		delete fe;
		return err;
	}
	
	*fe_out = fe;
	return blargg_ok;
}

BLARGG_EXPORT fex_err_t fex_open( fex_t** fe_out, const char path [] )
{
	*fe_out = NULL;
	
	fex_type_t type;
	RETURN_ERR( fex_identify_file( &type, path ) );
	
	return fex_open_type( fe_out, path, type );
}


//// Wide paths

char* fex_wide_to_path( const wchar_t* wide )
{
	return blargg_to_utf8( wide );
}

void fex_free_path( char* path )
{
	free( path );
}


//// Errors

#define ENTRY( name ) { blargg_err_##name, fex_err_##name }
static blargg_err_to_code_t const fex_codes [] =
{
	ENTRY( generic ),
	ENTRY( memory ),
	ENTRY( caller ),
	ENTRY( internal ),
	ENTRY( limitation ),
	
	ENTRY( file_missing ),
	ENTRY( file_read ),
	ENTRY( file_io ),
	ENTRY( file_eof ),
	
	ENTRY( file_type ),
	ENTRY( file_feature ),
	ENTRY( file_corrupt ),
	
	{ 0, -1 }
};
#undef ENTRY

static int err_code( fex_err_t err )
{
	return blargg_err_to_code( err, fex_codes );
}

BLARGG_EXPORT int fex_err_code( fex_err_t err )
{
	int code = err_code( err );
	return (code >= 0 ? code : fex_err_generic);
}

BLARGG_EXPORT fex_err_t fex_code_to_err( int code )
{
	return blargg_code_to_err( code, fex_codes );
}

BLARGG_EXPORT const char* fex_err_details( fex_err_t err )
{
	// If we don't have error code assigned, return entire string
	return (err_code( err ) >= 0 ? blargg_err_details( err ) : blargg_err_str( err ));
}


//// Wrappers

BLARGG_EXPORT fex_err_t fex_read( fex_t* fe, void* out, int count )
{
	RETURN_ERR( fe->stat() );
	return fe->reader().read( out, count );
}

BLARGG_EXPORT void        fex_close           ( fex_t* fe )                         { delete fe; }
BLARGG_EXPORT fex_type_t  fex_type            ( const fex_t* fe )                   { return fe->type(); }
BLARGG_EXPORT int         fex_done            ( const fex_t* fe )                   { return fe->done(); }
BLARGG_EXPORT const char* fex_name            ( const fex_t* fe )                   { return fe->name(); }
BLARGG_EXPORT const wchar_t* fex_wname        ( const fex_t* fe )                   { return fe->wname(); }
BLARGG_EXPORT int         fex_size            ( const fex_t* fe )                   { return fe->size(); }
BLARGG_EXPORT unsigned    fex_dos_date        ( const fex_t* fe )                   { return fe->dos_date(); }
BLARGG_EXPORT unsigned    fex_crc32           ( const fex_t* fe )                   { return fe->crc32(); }
BLARGG_EXPORT fex_err_t   fex_stat            ( fex_t* fe )                         { return fe->stat(); }
BLARGG_EXPORT fex_err_t   fex_next            ( fex_t* fe )                         { return fe->next(); }
BLARGG_EXPORT fex_err_t   fex_rewind          ( fex_t* fe )                         { return fe->rewind(); }
BLARGG_EXPORT int         fex_tell            ( const fex_t* fe )                   { return fe->tell(); }
BLARGG_EXPORT fex_pos_t   fex_tell_arc        ( const fex_t* fe )                   { return fe->tell_arc(); }
BLARGG_EXPORT fex_err_t   fex_seek_arc        ( fex_t* fe, fex_pos_t pos )          { return fe->seek_arc( pos ); }
BLARGG_EXPORT const char* fex_type_extension  ( fex_type_t t )                      { return t->extension; }
BLARGG_EXPORT const char* fex_type_name       ( fex_type_t t )                      { return t->name; }
BLARGG_EXPORT fex_err_t   fex_data            ( fex_t* fe, const void** data_out )  { return fe->data( data_out ); }
BLARGG_EXPORT const char* fex_err_str         ( fex_err_t err )                     { return blargg_err_str( err ); }
