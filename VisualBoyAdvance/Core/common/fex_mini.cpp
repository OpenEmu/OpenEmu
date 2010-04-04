// Minimal implementation of fex.h. Supports gzipped files if you have zlib
// available and HAVE_ZLIB_H is defined.

// File_Extractor 0.4.3. http://www.slack.net/~ant/

#include "fex.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

/* Copyright (C) 2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#ifndef HAVE_ZLIB_H
	#define FILE_GZ( norm, gz ) norm
	#define FILE_READ( ptr, size, file ) fread( ptr, 1, size, file )
#else
	#define FILE_GZ( norm, gz ) gz
	#define FILE_READ( ptr, size, file ) gzread( file, ptr, size )

#include "zlib.h"

static const char* get_gzip_size( const char* path, long* eof )
{
	FILE* file = fopen( path, "rb" );
	if ( !file )
		return "Couldn't open file";

	unsigned char buf [4];
	if ( fread( buf, 2, 1, file ) > 0 && buf [0] == 0x1F && buf [1] == 0x8B )
	{
		fseek( file, -4, SEEK_END );
		if ( !fread( buf, 4, 1, file ) )
		{
			fclose( file );
			return "Couldn't get file size";
		}
		*eof = buf [3] * 0x1000000 + buf [2] * 0x10000 + buf [1] * 0x100 + buf [0];
	}
	else
	{
		fseek( file, 0, SEEK_END );
		*eof = ftell( file );
	}
	const char* err = (ferror( file ) || feof( file )) ? "Couldn't get file size" : 0;
	fclose( file );
	return err;
}
#endif

const char fex_wrong_file_type [] = "Archive format not supported";

struct File_Extractor
{
	FILE_GZ(FILE*,gzFile) file;
	int done;
	long size;
	void* data; // file data read into memory, ot 0 if not read
	void* user_data;
	fex_user_cleanup_t user_cleanup;

	char* name() const { return (char*) (this + 1); }
};

// Always identify as single file extractor
fex_type_t_ const fex_bin_type [1] = {{ ""  , 0 }};
const char* fex_identify_header   ( void const* )               { return ""; }
fex_type_t  fex_identify_extension( const char* )               { return fex_bin_type; }
fex_type_t  fex_identify_file     ( const char*, fex_err_t* e ) { if ( e ) *e = 0; return fex_bin_type; }

static fex_err_t fex_open_( const char* path, File_Extractor** fe_out )
{
	*fe_out = 0;

	// name
	const char* name = strrchr( path, '\\' ); // DOS
	if ( !name )
		name = strrchr( path, '/' ); // UNIX
	if ( !name )
		name = strrchr( path, ':' ); // Mac
	if ( !name )
		name = path;

	// allocate space for struct and name
	long name_size = strlen( name ) + 1;
	File_Extractor* fe = (File_Extractor*) malloc( sizeof (File_Extractor) + name_size );
	if ( !fe ) return "Out of memory";

	fe->done         = 0;
	fe->data         = 0;
	fe->user_data    = 0;
	fe->user_cleanup = 0;
	memcpy( fe->name(), name, name_size );

	#ifdef HAVE_ZLIB_H
		// get gzip size BEFORE opening file
		const char* err = get_gzip_size( path, &fe->size );
		if ( err )
		{
			free( fe );
			return err;
		}
	#endif

	// open file
	fe->file = FILE_GZ(fopen,gzopen)( path, "rb" );
	if ( !fe->file )
	{
		free( fe );
		return "Couldn't open file";
	}

	// get normal size
	#ifndef HAVE_ZLIB_H
		fseek( fe->file, 0, SEEK_END );
		fe->size = ftell( fe->file );
		rewind( fe->file );
	#endif

	*fe_out = fe;
	return 0;
}

File_Extractor* fex_open( const char* path, fex_err_t* err_out )
{
	File_Extractor* fe;
	fex_err_t err = fex_open_( path, &fe );
	if ( err_out )
		*err_out = err;
	return fe;
}

File_Extractor* fex_open_type( fex_type_t, const char* path, fex_err_t* err_out )
{
	return fex_open( path, err_out );
}

void*     fex_user_data         ( File_Extractor const* fe )                    { return fe->user_data; }
void      fex_set_user_data     ( File_Extractor* fe, void* new_user_data )     { fe->user_data = new_user_data; }
void      fex_set_user_cleanup  ( File_Extractor* fe, fex_user_cleanup_t func ) { fe->user_cleanup = func; }

fex_type_t fex_type             ( File_Extractor const* )                       { return fex_bin_type; }
int       fex_done              ( File_Extractor const* fe )                    { return fe->done; }
const char* fex_name            ( File_Extractor* fe )                           { return fe->name(); }
unsigned long fex_dos_date      ( File_Extractor const* )                       { return 0; }
long      fex_size              ( File_Extractor const* fe )                    { return fe->size; }
long      fex_remain            ( File_Extractor const* fe )                    { return fe->size - FILE_GZ(ftell,gztell)( fe->file ); }
void      fex_scan_only         ( File_Extractor* )                             { }
fex_err_t fex_read_once         ( File_Extractor* fe, void* out, long count )   { return fex_read( fe, out, count ); }
long      fex_read_avail        ( File_Extractor* fe, void* out, long count )   { return FILE_READ( out, count, fe->file ); }

fex_err_t fex_read( File_Extractor* fe, void* out, long count )
{
	if ( count == (long) FILE_READ( out, count, fe->file ) )
		return 0;

	if ( FILE_GZ(feof,gzeof)( fe->file ) )
		return "Unexpected end of file";

	return "Couldn't read from file";
}

fex_err_t fex_next( File_Extractor* fe )
{
	fe->done = 1;
	return 0;
}

fex_err_t fex_rewind( File_Extractor* fe )
{
	fe->done = 0;
	FILE_GZ(rewind,gzrewind)( fe->file );
	return 0;
}

static fex_err_t fex_data_( File_Extractor* fe )
{
	if ( !fe->data )
	{
		fe->data = malloc( fe->size );
		if ( !fe->data ) return "Out of memory";

		fex_err_t err = fex_read( fe, fe->data, fe->size );
		if ( err )
		{
			free( fe->data );
			return err;
		}
	}
	return 0;
}

const unsigned char* fex_data( File_Extractor* fe, fex_err_t* err_out )
{
	fex_err_t err = fex_data_( fe );
	if ( err_out )
		*err_out = err;
	return (const unsigned char*) fe->data;
}

void fex_close( File_Extractor* fe )
{
	if ( fe )
	{
		free( fe->data );
		FILE_GZ(fclose,gzclose)( fe->file );

		if ( fe->user_cleanup )
			fe->user_cleanup( fe->user_data );

		free( fe );
	}
}
