// Compressed file archive interface

// File_Extractor 1.0.0
#ifndef FILE_EXTRACTOR_H
#define FILE_EXTRACTOR_H

#include "blargg_common.h"
#include "Data_Reader.h"
#include "fex.h"

struct fex_t : private Data_Reader {
public:
	virtual ~fex_t();
	
// Open/close

	// Opens archive from custom data source. Keeps pointer until close().
	blargg_err_t open( File_Reader* input, const char* path = NULL );
	
	// Takes ownership of File_Reader* passed to open(), so that close()
	// will delete it.
	void own_file()                     { own_file_ = reader_; }
	
	// See fex.h
	blargg_err_t open( const char path [] );
	fex_type_t type() const             { return type_; }
	void close();

// Scanning

	// See fex.h
	bool done() const                   { return done_; }
	blargg_err_t next();
	blargg_err_t rewind();
	fex_pos_t tell_arc() const;
	blargg_err_t seek_arc( fex_pos_t );

// Info

	// See fex.h
	const char* name() const            { return name_; }
	const wchar_t* wname() const        { return wname_; }
	blargg_err_t stat();
	int size() const                    { assert( stat_called ); return size_; }
	unsigned int dos_date() const       { return date_; }
	unsigned int crc32() const          { return crc32_; }
	
// Extraction

	// Data_Reader to current file's data, so standard Data_Reader interface can
	// be used, rather than having to treat archives specially. stat() must have
	// been called.
	Data_Reader& reader()               { assert( stat_called ); return *this; }

	// See fex.h
	blargg_err_t data( const void** data_out );
	int tell() const                    { return size_ - remain(); }
	
// Derived interface
protected:
	
	// Sets type of object
	fex_t( fex_type_t );
	
	// Path to archive file, or "" if none supplied
	const char* arc_path() const                    { return path_.begin(); }
	
	// Opens archive file if it's not already. If unbuffered is true, opens file
	// without any buffering.
	blargg_err_t open_arc_file( bool unbuffered = false );
	
	// Archive file
	File_Reader& arc() const                        { return *reader_; }
	
	// Sets current file name
	void set_name( const char name [], const wchar_t* wname = NULL );
	
	// Sets current file information
	void set_info( int size, unsigned date = 0, unsigned crc = 0 );
	
// User overrides

	// Overrides must do indicated task. Non-pure functions have reasonable default
	// implementation. Overrides should avoid calling public functions like
	// next() and rewind().
	
	// Open archive using file_path(). OK to delay actual file opening until later.
	// Default just calls open_arc_file(), then open_v().
	virtual blargg_err_t open_path_v();
	
	// Open archive using file() for source data. If unsupported, return error.
	virtual blargg_err_t open_v()                   BLARGG_PURE( ; )
	
	// Go to next file in archive and call set_name() and optionally set_info()
	virtual blargg_err_t next_v()                   BLARGG_PURE( ; )
	
	// Go back to first file in archive
	virtual blargg_err_t rewind_v()                 BLARGG_PURE( ; )
	
	// Close archive. Called even if open_path_v() or open_v() return unsuccessfully.
	virtual void         close_v()                  BLARGG_PURE( ; )
	
	// Clear any fields related to current file
	virtual void         clear_file_v()             { }
	
	// Call set_info() if not already called by next_v()
	virtual blargg_err_t stat_v()                   { return blargg_ok; }
	
	// Return value that allows later return to this file. Result must be >= 0.
	virtual fex_pos_t tell_arc_v() const;
	
	// Return to previously saved position
	virtual blargg_err_t seek_arc_v( fex_pos_t );
	
	// One or both of the following must be overridden
	
	// Provide pointer to data for current file in archive
	virtual blargg_err_t data_v( const void** out );
	
	// Extract next n bytes
	virtual blargg_err_t extract_v( void* out, int n );
	
// Implementation
public:
	BLARGG_DISABLE_NOTHROW

private:
	fex_type_t const type_;
	
	// Archive file
	blargg_vector<char> path_;
	File_Reader* reader_;
	File_Reader* own_file_;
	bool         opened_;
	
	// Position in archive
	fex_pos_t tell_;    // only used by default implementation of tell/seek
	bool      done_;
	
	// Info for current file in archive
	const char* name_;
	const wchar_t* wname_;
	unsigned    date_;
	unsigned    crc32_;
	int         size_;
	bool        stat_called;
	
	// Current file contents
	void const* data_ptr_; // NULL if not read into memory
	blargg_vector<char> own_data_;

	bool opened() const                         { return opened_; }
	void clear_file();
	void close_();
	blargg_err_t set_path( const char* path );
	blargg_err_t rewind_file();
	blargg_err_t next_();
	
	// Data_Reader overrides
	// TODO: override skip_v?
	virtual blargg_err_t read_v( void* out, int n );
};

struct fex_type_t_
{
	const char* extension;
	File_Extractor* (*new_fex)();
	const char* name;
	blargg_err_t (*init)(); // Called by fex_init(). Can be NULL.
};

extern const fex_type_t_
	fex_7z_type  [1],
	fex_gz_type  [1],
	fex_rar_type [1],
	fex_zip_type [1],
	fex_bin_type [1];

inline blargg_err_t File_Extractor::open_v()    { return blargg_ok; }
inline blargg_err_t File_Extractor::next_v()    { return blargg_ok; }
inline blargg_err_t File_Extractor::rewind_v()  { return blargg_ok; }
inline void         File_Extractor::close_v()   { }

// Default to Std_File_Reader for archive access
#ifndef FEX_FILE_READER
	#define FEX_FILE_READER Std_File_Reader
#elif defined (FEX_FILE_READER_INCLUDE)
	#include FEX_FILE_READER_INCLUDE
#endif

#endif
