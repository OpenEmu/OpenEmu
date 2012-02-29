// File_Extractor 1.0.0. http://www.slack.net/~ant/

#include "Zip_Extractor.h"

#include "blargg_endian.h"

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

/* To avoid copying filename string from catalog, I terminate it by modifying
catalog data. This potentially requires moving the first byte of the type
of the next entry elsewhere; I move it to the first byte of made_by. Kind
of hacky, but I'd rather not have to allocate memory for a copy of it. */

#include "blargg_source.h"

/* Reads this much from end of file when first opening. Only this much is
searched for the end catalog entry. If whole catalog is within this data,
nothing more needs to be read on open. */
int const end_read_size = 8 * 1024;

/* Reads are are made using file offset that's a multiple of this,
increasing performance. */
int const disk_block_size = 4 * 1024;

// Read buffer used for extracting file data
int const read_buf_size = 16 * 1024;

struct header_t
{
	char type [4];
	byte vers [2];
	byte flags [2];
	byte method [2];
	byte date [4];
	byte crc [4];
	byte raw_size [4];
	byte size [4];
	byte filename_len [2];
	byte extra_len [2];
	char filename [2]; // [filename_len]
	//char extra [extra_len];
};
int const header_size = 30;

struct entry_t
{
	char type [4];
	byte made_by [2];
	byte vers [2];
	byte flags [2];
	byte method [2];
	byte date [4];
	byte crc [4];
	byte raw_size [4];
	byte size [4];
	byte filename_len [2];
	byte extra_len [2];
	byte comment_len [2];
	byte disk [2];
	byte int_attrib [2];
	byte ext_attrib [4];
	byte file_offset [4];
	char filename [2]; // [filename_len]
	//char extra [extra_len];
	//char comment [comment_len];
};
int const entry_size = 46;

struct end_entry_t
{
	char type [4];
	byte disk [2];
	byte first_disk [2];
	byte disk_entry_count [2];
	byte entry_count [2];
	byte dir_size [4];
	byte dir_offset [4];
	byte comment_len [2];
	char comment [2]; // [comment_len]
};
int const end_entry_size = 22;

static blargg_err_t init_zip()
{
	get_crc_table(); // initialize zlib's CRC-32 tables
	return blargg_ok;
}

static File_Extractor* new_zip()
{
	return BLARGG_NEW Zip_Extractor;
}

fex_type_t_ const fex_zip_type [1] = {{
	".zip",
	&new_zip,
	"ZIP archive",
	&init_zip
}};

Zip_Extractor::Zip_Extractor() :
	File_Extractor( fex_zip_type )
{
	Zip_Extractor::clear_file_v();
	
	// If these fail, structures had extra padding inserted by compiler
	assert( offsetof (header_t,filename)   == header_size );
	assert( offsetof (entry_t,filename)    == entry_size );
	assert( offsetof (end_entry_t,comment) == end_entry_size );
}

Zip_Extractor::~Zip_Extractor()
{
	close();
}

blargg_err_t Zip_Extractor::open_path_v()
{
	RETURN_ERR( open_arc_file( true ) );
	return File_Extractor::open_path_v();
}

inline
void Zip_Extractor::reorder_entry_header( int offset )
{
	catalog [offset + 0] = 0;
	catalog [offset + 4] = 'P';
}

blargg_err_t Zip_Extractor::open_v()
{
	if ( arc().size() < end_entry_size )
		return blargg_err_file_type;

	// Read final end_read_size bytes of file
	int file_pos = max( 0, arc().size() - end_read_size );
	file_pos -= file_pos % disk_block_size;
	RETURN_ERR( catalog.resize( arc().size() - file_pos ) );
	RETURN_ERR( arc().seek( file_pos ) );
	RETURN_ERR( arc().read( catalog.begin(), catalog.size() ) );

	// Find end-of-catalog entry
	int end_pos = catalog.size() - end_entry_size;
	while ( end_pos >= 0 && memcmp( &catalog [end_pos], "PK\5\6", 4 ) )
		end_pos--;
	if ( end_pos < 0 )
		return blargg_err_file_type;
	end_entry_t const& end_entry = (end_entry_t&) catalog [end_pos];
	end_pos += file_pos;

	// some idiotic zip compressors add data to end of zip without setting comment len
//  check( arc().size() == end_pos + end_entry_size + get_le16( end_entry.comment_len ) );

	// Find file offset of beginning of catalog
	catalog_begin = get_le32( end_entry.dir_offset );
	int catalog_size = end_pos - catalog_begin;
	if ( catalog_size < 0 )
		return blargg_err_file_corrupt;
	catalog_size += end_entry_size;

	// See if catalog is entirely contained in bytes already read
	int begin_offset = catalog_begin - file_pos;
	if ( begin_offset >= 0 )
		memmove( catalog.begin(), &catalog [begin_offset], catalog_size );

	RETURN_ERR( catalog.resize( catalog_size ) );
	if ( begin_offset < 0 )
	{
		// Catalog begins before bytes read, so it needs to be read
		RETURN_ERR( arc().seek( catalog_begin ) );
		RETURN_ERR( arc().read( catalog.begin(), catalog.size() ) );
	}

	// First entry in catalog should be a file or end of archive
	if ( memcmp( catalog.begin(), "PK\1\2", 4 ) && memcmp( catalog.begin(), "PK\5\6", 4 ) )
		return blargg_err_file_type;
	
	reorder_entry_header( 0 );
	return rewind_v();
}

void Zip_Extractor::close_v()
{
	catalog.clear();
}

// Scanning

inline
static bool is_normal_file( entry_t const& e, unsigned len )
{
	int last_char = (len ? e.filename [len - 1] : '/');
	bool is_dir = (last_char == '/' || last_char == '\\');
	if ( is_dir && get_le32( e.size ) == 0 )
		return false;
	check( !is_dir );
	
	// Mac OS X puts meta-information in separate files with normal extensions,
	// so they must be filtered out or caller will mistake them for normal files.
	if ( e.made_by[1] == 3 )
	{
		const char* dir = strrchr( e.filename, '/' );
		if ( dir )
			dir++;
		else
			dir = e.filename;
		
		if ( *dir == '.' )
			return false;
		
		if ( !strcmp( dir, "Icon\x0D" ) )
			return false;
	}
	
	return true;
}

blargg_err_t Zip_Extractor::update_info( bool advance_first )
{
	while ( 1 )
	{
		entry_t& e = (entry_t&) catalog [catalog_pos];

		if ( memcmp( e.type, "\0K\1\2P", 5 ) && memcmp( e.type, "PK\1\2", 4 ) )
		{
			check( !memcmp( e.type, "\0K\5\6P", 5 ) );
			break;
		}

		unsigned len = get_le16( e.filename_len );
		int next_offset = catalog_pos + entry_size + len + get_le16( e.extra_len ) +
				get_le16( e.comment_len );
		if ( (unsigned) next_offset > catalog.size() - end_entry_size )
			return blargg_err_file_corrupt;
		
		if ( catalog [next_offset] == 'P' )
			reorder_entry_header( next_offset );

		if ( !advance_first )
		{
			e.filename [len] = 0; // terminate name
			
			if ( is_normal_file( e, len ) )
			{
				set_name( e.filename );
				set_info( get_le32( e.size ), get_le32( e.date ), get_le32( e.crc ) );
				break;
			}
		}

		catalog_pos = next_offset;
		advance_first = false;
	}
	
	return blargg_ok;
}

blargg_err_t Zip_Extractor::next_v()
{
	return update_info( true );
}

blargg_err_t Zip_Extractor::rewind_v()
{
	return seek_arc_v( 0 );
}

fex_pos_t Zip_Extractor::tell_arc_v() const
{
	return catalog_pos;
}

blargg_err_t Zip_Extractor::seek_arc_v( fex_pos_t pos )
{
	assert( 0 <= pos && (size_t) pos <= catalog.size() - end_entry_size );
	
	catalog_pos = pos;
	return update_info( false );
}

// Reading

void Zip_Extractor::clear_file_v()
{
	buf.end();
}

blargg_err_t Zip_Extractor::inflater_read( void* data, void* out, int* count )
{
	Zip_Extractor& self = *STATIC_CAST(Zip_Extractor*,data);
	
	if ( *count > self.raw_remain )
		*count = self.raw_remain;
	
	self.raw_remain -= *count;
	
	return self.arc().read( out, *count );
}

blargg_err_t Zip_Extractor::fill_buf( int offset, int buf_size, int initial_read )
{
	raw_remain = arc().size() - offset;
	RETURN_ERR( arc().seek( offset ) );
	return buf.begin( inflater_read, this, buf_size, initial_read );
}

blargg_err_t Zip_Extractor::first_read( int count )
{
	entry_t const& e = (entry_t&) catalog [catalog_pos];
	
	// Determine compression
	{
		int method = get_le16( e.method );
		if ( (method && method != Z_DEFLATED) || get_le16( e.vers ) > 20 )
			return BLARGG_ERR( BLARGG_ERR_FILE_FEATURE, "compression method" );
		file_deflated = (method != 0);
	}
	
	int raw_size = get_le32( e.raw_size );

	int file_offset = get_le32( e.file_offset );
	int align = file_offset % disk_block_size;
	{
		// read header
		int buf_size = 3 * disk_block_size - 1 + raw_size; // space for all raw data
		buf_size -= buf_size % disk_block_size;
		int initial_read = buf_size;
		if ( !file_deflated || count < size() )
		{
			buf_size     = read_buf_size;
			initial_read = disk_block_size * 2;
		}
		// TODO: avoid re-reading if buffer already has data we want?
		RETURN_ERR( fill_buf( file_offset - align, buf_size, initial_read ) );
	}
	header_t const& h = (header_t&) buf.data() [align];
	if ( buf.filled() < align + header_size || memcmp( h.type, "PK\3\4", 4 ) )
		return blargg_err_file_corrupt;

	// CRCs of header and file data
	correct_crc = get_le32( h.crc );
	if ( !correct_crc )
		correct_crc = get_le32( e.crc );
	check( correct_crc == get_le32( e.crc ) ); // catalog CRC should match
	crc = ::crc32( 0, NULL, 0 );

	// Data offset
	int data_offset = file_offset + header_size +
			get_le16( h.filename_len ) + get_le16( h.extra_len );
	if ( data_offset + raw_size > catalog_begin )
		return blargg_err_file_corrupt;

	// Refill buffer if there's lots of extra data after header
	int buf_offset = data_offset - file_offset + align;
	if ( buf_offset > buf.filled() )
	{
		// TODO: this will almost never occur, making it a good place for bugs
		buf_offset = data_offset % disk_block_size;
		RETURN_ERR( fill_buf( data_offset - buf_offset, read_buf_size, disk_block_size ) );
	}

	raw_remain = raw_size - (buf.filled() - buf_offset);
	return buf.set_mode( (file_deflated ? buf.mode_raw_deflate : buf.mode_copy), buf_offset );
}

blargg_err_t Zip_Extractor::extract_v( void* out, int count )
{
	if ( tell() == 0 )
		RETURN_ERR( first_read( count ) );
	
	int actual = count;
	RETURN_ERR( buf.read( out, &actual ) );
	if ( actual < count )
		return blargg_err_file_corrupt;
	
	crc = ::crc32( crc, (byte const*) out, count );
	if ( count == reader().remain() && crc != correct_crc )
		return blargg_err_file_corrupt;
	
	return blargg_ok;
}
