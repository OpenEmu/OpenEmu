// Presents a single file as an "archive" of just that file.

// File_Extractor 1.0.0
#ifndef BINARY_EXTRACTOR_H
#define BINARY_EXTRACTOR_H

#include "File_Extractor.h"

class Binary_Extractor : public File_Extractor {
public:
	Binary_Extractor();
	virtual ~Binary_Extractor();

protected:
	virtual blargg_err_t open_path_v();
	virtual blargg_err_t open_v();
	virtual void         close_v();
	
	virtual blargg_err_t next_v();
	virtual blargg_err_t rewind_v();

	virtual blargg_err_t stat_v();
	virtual blargg_err_t extract_v( void*, int );
};

#endif
