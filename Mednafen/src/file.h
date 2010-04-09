#ifndef MDFN_FILE_H
#define MDFN_FILE_H
typedef struct {
	uint8 *data;
	int64 size;
	int64 location;
	char *ext;
	#ifdef HAVE_MMAP
	bool is_mmap;
	#endif
} MDFNFILE;

MDFNFILE *MDFN_fopen(const char *path, const char *ipsfn, const char *mode, const char *ext);
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

 inline const void *GetData(void) const
 {
  return(data);
 }

 inline uint64 GetLength(void) const
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
