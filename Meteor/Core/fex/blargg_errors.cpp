// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "blargg_errors.h"

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

blargg_err_def_t blargg_err_generic      = BLARGG_ERR_GENERIC;
blargg_err_def_t blargg_err_memory       = BLARGG_ERR_MEMORY;
blargg_err_def_t blargg_err_caller       = BLARGG_ERR_CALLER;
blargg_err_def_t blargg_err_internal     = BLARGG_ERR_INTERNAL;
blargg_err_def_t blargg_err_limitation   = BLARGG_ERR_LIMITATION;

blargg_err_def_t blargg_err_file_missing = BLARGG_ERR_FILE_MISSING;
blargg_err_def_t blargg_err_file_read    = BLARGG_ERR_FILE_READ;
blargg_err_def_t blargg_err_file_write   = BLARGG_ERR_FILE_WRITE;
blargg_err_def_t blargg_err_file_io      = BLARGG_ERR_FILE_IO;
blargg_err_def_t blargg_err_file_full    = BLARGG_ERR_FILE_FULL;
blargg_err_def_t blargg_err_file_eof     = BLARGG_ERR_FILE_EOF;

blargg_err_def_t blargg_err_file_type    = BLARGG_ERR_FILE_TYPE;
blargg_err_def_t blargg_err_file_feature = BLARGG_ERR_FILE_FEATURE;
blargg_err_def_t blargg_err_file_corrupt = BLARGG_ERR_FILE_CORRUPT;

const char* blargg_err_str( blargg_err_t err )
{
	if ( !err )
		return "";
	
	if ( *err == BLARGG_ERR_TYPE("")[0] )
		return err + 1;
	
	return err;
}

bool blargg_is_err_type( blargg_err_t err, const char type [] )
{
	if ( err )
	{
		// True if first strlen(type) characters of err match type
		char const* p = err;
		while ( *type && *type == *p )
		{
			type++;
			p++;
		}
		
		if ( !*type )
			return true;
	}
	
	return false;
}

const char* blargg_err_details( blargg_err_t err )
{
	const char* p = err;
	if ( !p )
	{
		p = "";
	}
	else if ( *p == BLARGG_ERR_TYPE("")[0] )
	{
		while ( *p && *p != ';' )
			p++;
		
		// Skip ; and space after it
		if ( *p )
		{
			p++;
		
			check( *p == ' ' );
			if ( *p )
				p++;
		}
	}
	return p;
}

int blargg_err_to_code( blargg_err_t err, blargg_err_to_code_t const codes [] )
{
	if ( !err )
		return 0;
	
	while ( codes->str && !blargg_is_err_type( err, codes->str ) )
		codes++;
	
	return codes->code;
}

blargg_err_t blargg_code_to_err( int code, blargg_err_to_code_t const codes [] )
{
	if ( !code )
		return blargg_ok;
	
	while ( codes->str && codes->code != code )
		codes++;
	
	if ( !codes->str )
		return blargg_err_generic;
	
	return codes->str;
}
