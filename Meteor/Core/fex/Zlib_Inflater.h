// Simplifies use of zlib for inflating data

// File_Extractor 1.0.0
#ifndef ZLIB_INFLATER_H
#define ZLIB_INFLATER_H

#include "blargg_common.h"
#include "Data_Reader.h"
#include "zlib.h"

class Zlib_Inflater {
public:

	// Reads at most min(*count,bytes_until_eof()) bytes into *out and set *count
	// to that number, or returns error if that many can't be read.
	typedef blargg_err_t (*callback_t)( void* user_data, void* out, int* count );

	// Begins by setting callback and filling buffer. Default buffer is 16K and
	// filled to 4K, or specify buf_size and initial_read for custom buffer size
	// and how much to read initially.
	blargg_err_t begin( callback_t, void* user_data,
			int buf_size = 0, int initial_read = 0 );

	// Data read into buffer by begin()
	const unsigned char* data() const   { return zbuf.next_in; }
	int filled() const                  { return zbuf.avail_in; }

	// Begins inflation using specified mode. Using mode_auto selects between
	// mode_copy and mode_ungz by examining first two bytes of buffer. Use
	// buf_offset to specify where data begins in buffer, in case there is
	// header data that should be skipped.
	enum mode_t { mode_copy, mode_ungz, mode_raw_deflate, mode_auto };
	blargg_err_t set_mode( mode_t, int buf_offset = 0 );

	// True if set_mode() has been called with mode_ungz or mode_raw_deflate
	bool deflated() const               { return deflated_; }

	// Reads/inflates at most *count_io bytes into *out and sets *count_io to actual
	// number of bytes read (less than requested if end of data was reached).
	// Buffers source data internally, even in copy mode, so input file can be
	// unbuffered without sacrificing performance.
	blargg_err_t read( void* out, int* count_io );

	// Total number of bytes read since begin()
	int tell() const                    { return zbuf.total_out; }

	// Ends inflation and frees memory
	void end();

private:
	// noncopyable
	Zlib_Inflater( const Zlib_Inflater& );
	Zlib_Inflater& operator = ( const Zlib_Inflater& );

// Implementation
public:
	Zlib_Inflater();
	~Zlib_Inflater();

private:
	z_stream_s zbuf;
	blargg_vector<unsigned char> buf;
	bool deflated_;
	callback_t callback;
	void* user_data;

	blargg_err_t fill_buf( int count );
};

#endif
