#ifndef MDFN_FILE_H
#define MDFN_FILE_H

#include <string>

#define MDFNFILE_EC_NOTFOUND	1
#define MDFNFILE_EC_OTHER	2

class MDFNFILE
{
	public:

	MDFNFILE();
	// WIP constructors:
	MDFNFILE(const char *path, const FileExtensionSpecStruct *known_ext, const char *purpose = NULL);

	~MDFNFILE();

	bool Open(const char *path, const FileExtensionSpecStruct *known_ext, const char *purpose = NULL, const bool suppress_notfound_pe = FALSE);
	INLINE bool Open(const std::string &path, const FileExtensionSpecStruct *known_ext, const char *purpose = NULL, const bool suppress_notfound_pe = FALSE)
	{
	 return(Open(path.c_str(), known_ext, purpose, suppress_notfound_pe));
	}

        bool ApplyIPS(FILE *);
	bool Close(void);

	const int64 &size;
	const uint8 * const &data;
	const char * const &ext;

	// Currently, only valid with Open()
	inline int GetErrorCode(int *get_errno = NULL)
	{
	 if(get_errno)
	  *get_errno = local_errno;

	 return(error_code);
	}

	inline int64 Size(void)
	{
	 return(f_size);
	}

	inline const uint8 *Data(void)
	{
	 return(f_data);
	}
	
	uint64 fread(void *ptr, size_t size, size_t nmemb);
	int fseek(int64 offset, int whence);

	inline uint64 ftell(void)
	{
	 return(location);
	}

	inline void rewind(void)
	{
	 location = 0;
	}

	int read32le(uint32 *Bufo);
	int read16le(uint16 *Bufo);

	inline int fgetc(void)
	{
	 if(location < f_size)
	  return f_data[location++];

	 return EOF;
	}

	inline int fisarchive(void)
	{
	 return(0);
	}

	char *fgets(char *s, int size);

	private:

        uint8 *f_data;
        int64 f_size;
        char *f_ext;

	int error_code;
	int local_errno;

        int64 location;

	#ifdef HAVE_MMAP
	bool is_mmap;
	#endif

	bool MakeMemWrapAndClose(void *tz, int type);
};

#if 0
MDFNFILE *MDFN_fopen(const char *path, const char *ipsfn, const char *mode, const FileExtensionSpecStruct *known_ext);
int MDFN_fclose(MDFNFILE*);
uint64 MDFN_fread(void *ptr, size_t size, size_t nmemb, MDFNFILE*);
uint64 MDFN_fwrite(void *ptr, size_t size, size_t nmemb, MDFNFILE*);
int MDFN_fseek(MDFNFILE*, int64 offset, int whence);
uint64 MDFN_ftell(MDFNFILE*);
void MDFN_rewind(MDFNFILE*);
int MDFN_read32le(uint32 *Bufo, MDFNFILE*);
int MDFN_read16le(uint16 *Bufo, MDFNFILE*);
int MDFN_fgetc(MDFNFILE*);
uint64 MDFN_fgetsize(MDFNFILE*);
int MDFN_fisarchive(MDFNFILE*);
char *MDFN_fgets(char *s, int size, MDFNFILE *);
#endif

class PtrLengthPair
{
 public:

 inline PtrLengthPair(const void *new_data, const uint64 new_length)
 {
  data = new_data;
  length = new_length;
 }

 ~PtrLengthPair() 
 { 

 } 

 INLINE const void *GetData(void) const
 {
  return(data);
 }

 INLINE uint64 GetLength(void) const
 {
  return(length);
 }

 private:
 const void *data;
 uint64 length;
};

#include <vector>

// These functions should be used for data like save states and non-volatile backup memory.
// Until(if, even) we add LoadFromFile functions, for reading the files these functions generate, just use gzopen(), gzread(), etc.
// "compress" is set to the zlib compression level.  0 disables compression entirely, and dumps the file without a gzip header or footer.
// (Note: There is a setting that will force compress to 0 in the internal DumpToFile logic, for hackers who don't want to ungzip save files.)

bool MDFN_DumpToFile(const char *filename, int compress, const void *data, const uint64 length);
bool MDFN_DumpToFile(const char *filename, int compress, const std::vector<PtrLengthPair> &pearpairs);

#endif
