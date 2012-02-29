// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "blargg_common.h"

/* Copyright (C) 2008-2009 Shay Green. This module is free software; you
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

void blargg_vector_::init()
{
	begin_ = NULL;
	size_  = 0;
}

void blargg_vector_::clear()
{
	void* p = begin_;
	begin_  = NULL;
	size_   = 0;
	free( p );
}

blargg_err_t blargg_vector_::resize_( size_t n, size_t elem_size )
{
	if ( n != size_ )
	{
		if ( n == 0 )
		{
			// Simpler to handle explicitly. Realloc will handle a size of 0,
			// but then we have to avoid raising an error for a NULL return.
			clear();
		}
		else
		{
			void* p = realloc( begin_, n * elem_size );
			CHECK_ALLOC( p );
			begin_ = p;
			size_  = n;
		}
	}
	return blargg_ok;
}
