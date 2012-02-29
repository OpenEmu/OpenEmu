// Lightweight interface for reading data from byte stream

// File_Extractor 1.0.0
#ifndef DATA_READER_H
#define DATA_READER_H

#include "blargg_common.h"

/* Some functions accept a long instead of int for convenience where caller has
a long due to some other interface, and would otherwise have to get a warning,
or cast it (and verify that it wasn't outside the range of an int).

To really support huge (>2GB) files, long isn't a solution, since there's no
guarantee it's more than 32 bits. We'd need to use long long (if available), or
something compiler-specific, and change all places file sizes or offsets are
used. */

// Supports reading and finding out how many bytes are remaining
class Data_Reader {
public:

	// Reads min(*n,remain()) bytes and sets *n to this number, thus trying to read more
	// tham remain() bytes doesn't result in error, just *n being set to remain().
	blargg_err_t read_avail( void* p, int* n );
	blargg_err_t read_avail( void* p, long* n );

	// Reads exactly n bytes, or returns error if they couldn't ALL be read.
	// Reading past end of file results in blargg_err_file_eof.
	blargg_err_t read( void* p, int n );

	// Number of bytes remaining until end of file
	int remain() const                              { return remain_; }

	// Reads and discards n bytes. Skipping past end of file results in blargg_err_file_eof.
	blargg_err_t skip( int n );
	
	virtual ~Data_Reader() { }

private:
	// noncopyable
	Data_Reader( const Data_Reader& );
	Data_Reader& operator = ( const Data_Reader& );

// Derived interface
protected:
	Data_Reader()                                   : remain_( 0 ) { }
	
	// Sets remain
	void set_remain( int n )                        { assert( n >= 0 ); remain_ = n; }
	
	// Do same as read(). Guaranteed that 0 < n <= remain(). Value of remain() is updated
	// AFTER this call succeeds, not before. set_remain() should NOT be called from this.
	virtual blargg_err_t read_v( void*, int n )     BLARGG_PURE( { (void)n; return blargg_ok; } )
	
	// Do same as skip(). Guaranteed that 0 < n <= remain(). Default just reads data
	// and discards it. Value of remain() is updated AFTER this call succeeds, not
	// before. set_remain() should NOT be called from this.
	virtual blargg_err_t skip_v( int n );

// Implementation
public:
	BLARGG_DISABLE_NOTHROW
	
private:
	int remain_;
};


// Supports seeking in addition to Data_Reader operations
class File_Reader : public Data_Reader {
public:

	// Size of file
	int size() const                    { return size_; }

	// Current position in file
	int tell() const                    { return size_ - remain(); }

	// Goes to new position
	blargg_err_t seek( int );

// Derived interface
protected:
	// Sets size and resets position
	void set_size( int n )              { size_ = n; Data_Reader::set_remain( n ); }
	void set_size( long n )             { set_size( STATIC_CAST(int, n) ); }
	
	// Sets reported position
	void set_tell( int i )              { assert( 0 <= i && i <= size_ ); Data_Reader::set_remain( size_ - i ); }
	
	// Do same as seek(). Guaranteed that 0 <= n <= size().  Value of tell() is updated
	// AFTER this call succeeds, not before. set_* functions should NOT be called from this.
	virtual blargg_err_t seek_v( int n ) BLARGG_PURE( { (void)n; return blargg_ok; } )
	
// Implementation
protected:
	File_Reader()                       : size_( 0 ) { }
	
	virtual blargg_err_t skip_v( int );

private:
	int size_;
	
	void set_remain(); // avoid accidental use of set_remain
};


// Reads from file on disk
class Std_File_Reader : public File_Reader {
public:

	// Opens file
	blargg_err_t open( const char path [] );
	
	// Closes file if one was open
	void close();

	// Switches to unbuffered mode. Useful if buffering is already being
	// done at a higher level.
	void make_unbuffered();

// Implementation
public:
	Std_File_Reader();
	virtual ~Std_File_Reader();
	
protected:
	virtual blargg_err_t read_v( void*, int );
	virtual blargg_err_t seek_v( int );

private:
	void* file_;
};


// Treats range of memory as a file
class Mem_File_Reader : public File_Reader {
public:

	Mem_File_Reader( const void* begin, long size );

// Implementation
protected:
	virtual blargg_err_t read_v( void*, int );
	virtual blargg_err_t seek_v( int );

private:
	const char* const begin;
};


// Allows only count bytes to be read from reader passed
class Subset_Reader : public Data_Reader {
public:

	Subset_Reader( Data_Reader*, int count );

// Implementation
protected:
	virtual blargg_err_t read_v( void*, int );

private:
	Data_Reader* const in;
};


// Joins already-read header and remaining data into original file.
// Meant for cases where you've already read header and don't want
// to seek and re-read data (for efficiency).
class Remaining_Reader : public Data_Reader {
public:

	Remaining_Reader( void const* header, int header_size, Data_Reader* );

// Implementation
protected:
	virtual blargg_err_t read_v( void*, int );

private:
	Data_Reader* const in;
	void const* header;
	int header_remain;
};


// Invokes callback function to read data
extern "C" { // necessary to be usable from C
	typedef const char* (*callback_reader_func_t)(
		void* user_data,    // Same value passed to constructor
		void* out,          // Buffer to place data into
		int count           // Number of bytes to read
	);
}
class Callback_Reader : public Data_Reader {
public:
	typedef callback_reader_func_t callback_t;
	Callback_Reader( callback_t, long size, void* user_data );
	
// Implementation
protected:
	virtual blargg_err_t read_v( void*, int );

private:
	callback_t const callback;
	void* const user_data;
};


// Invokes callback function to read data
extern "C" { // necessary to be usable from C
	typedef const char* (*callback_file_reader_func_t)(
		void* user_data,    // Same value passed to constructor
		void* out,          // Buffer to place data into
		int count,          // Number of bytes to read
		int pos             // Position in file to read from
	);
}
class Callback_File_Reader : public File_Reader {
public:
	typedef callback_file_reader_func_t callback_t;
	Callback_File_Reader( callback_t, long size, void* user_data );
	
// Implementation
protected:
	virtual blargg_err_t read_v( void*, int );
	virtual blargg_err_t seek_v( int );

private:
	callback_t const callback;
	void* const user_data;
};


#ifdef HAVE_ZLIB_H

// Reads file compressed with gzip (or uncompressed)
class Gzip_File_Reader : public File_Reader {
public:

	// Opens possibly gzipped file
	blargg_err_t open( const char path [] );
	
	// Closes file if one was open
	void close();

// Implementation
public:
	Gzip_File_Reader();
	~Gzip_File_Reader();
	
protected:
	virtual blargg_err_t read_v( void*, int );
	virtual blargg_err_t seek_v( int );
	
private:
	// void* so "zlib.h" doesn't have to be included here
	void* file_;
};
#endif

char* blargg_to_utf8( const wchar_t* );
wchar_t* blargg_to_wide( const char* );

#endif
