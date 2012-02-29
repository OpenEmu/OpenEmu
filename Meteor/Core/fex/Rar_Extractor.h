// RAR archive extractor

// File_Extractor 1.0.0
#ifndef RAR_EXTRACTOR_H
#define RAR_EXTRACTOR_H

#include "File_Extractor.h"
#include "unrar/unrar.h"

class Rar_Extractor : public File_Extractor {
public:
	Rar_Extractor();
	virtual ~Rar_Extractor();

	struct read_callback_t
	{
		const char* err;
		int pos;
		File_Reader* in;
	};
	
protected:
	virtual blargg_err_t open_v();
	virtual void         close_v();
	
	virtual blargg_err_t next_v();
	virtual blargg_err_t rewind_v();
	virtual fex_pos_t    tell_arc_v() const;
	virtual blargg_err_t seek_arc_v( fex_pos_t );

	virtual blargg_err_t data_v( void const** );
	virtual blargg_err_t extract_v( void*, int );
	
private:
	unrar_t* unrar;
	read_callback_t reader;
	
	blargg_err_t convert_err( unrar_err_t );
	blargg_err_t skip_unextractables();
	blargg_err_t next_raw();
};

#endif
