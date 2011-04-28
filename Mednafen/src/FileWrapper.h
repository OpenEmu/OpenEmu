#ifndef __MDFN_FILEWRAPPER_H
#define __MDFN_FILEWRAPPER_H

#include <stdio.h>
#include <string>

// A stdio FILE wrapper
class FileWrapper
{
 public:

 enum
 {
  MODE_READ = 0,
  MODE_WRITE,
  MODE_WRITE_SAFE	// Will throw an exception instead of overwriting an existing file.
 };

 FileWrapper(const char *path, const int mode);
 #if 0
 FileWrapper();
 #endif
 ~FileWrapper();

 uint64 read(void *data, uint64 count, bool error_on_eof = true);

 void write(const void *data, uint64 count);

 int scanf(const char *format, ...) MDFN_FORMATSTR(scanf, 2, 3);

 void printf(const char *format, ...) MDFN_FORMATSTR(printf, 2, 3);

 void put_char(int c);

 void put_string(const char *str);
 void put_string(const std::string &str);

 void seek(int64 offset, int whence);

 int64 tell(void);

 int64 size(void);

 void flush(void);
 //void flushsync(void);	// TODO: see fflush and fsync

 private:

 FileWrapper & operator=(const FileWrapper &);    // Assignment operator
 FileWrapper(const FileWrapper &);		// Copy constructor
 //FileWrapper(FileWrapper &);                // Copy constructor

 FILE *fp;
 std::string path_save;
 const int OpenedMode;
};

#endif
