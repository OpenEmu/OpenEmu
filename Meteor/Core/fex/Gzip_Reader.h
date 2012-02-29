// Transparently decompresses gzip files, as well as uncompressed

// File_Extractor 1.0.0
#ifndef GZIP_READER_H
#define GZIP_READER_H

#include "Data_Reader.h"
#include "Zlib_Inflater.h"

class Gzip_Reader : public Data_Reader {
public:
	// Keeps pointer to reader until close(). If 
	blargg_err_t open( File_Reader* );
	
	// True if file is open
	bool opened() const                 { return in != NULL; }
	
	// Frees memory
	void close();
	
	// True if file is compressed
	bool deflated() const               { return inflater.deflated(); }
	
	// CRC-32 of data, of 0 if unavailable
	unsigned int crc32() const          { return crc32_; }
	
	// Number of bytes read since opening
	int tell() const                    { return size_ - remain(); }

public:
	Gzip_Reader();
	virtual ~Gzip_Reader();
	
protected:
	virtual blargg_err_t read_v( void*, int );

private:
	File_Reader* in;
	unsigned crc32_;
	int size_;
	Zlib_Inflater inflater;

	blargg_err_t calc_size();
};

#endif
