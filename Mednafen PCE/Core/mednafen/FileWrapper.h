#ifndef __MDFN_FILEWRAPPER_H
#define __MDFN_FILEWRAPPER_H

#include <stdio.h>
#include <string>

// A stdio FILE wrapper(with some BSD and POSIXisms, and a little dash of Win32, thrown in for special behaviors)
class FileWrapper
{
 public:

 enum
 {
  MODE_READ = 0,
  MODE_WRITE,
  MODE_WRITE_SAFE	// Will throw an exception instead of overwriting an existing file.
 };

 FileWrapper(const char *path, const int mode, const char *purpose = NULL);
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

 char *get_line(char *s, int size);	// Same semantics as fgets(), for now

 void seek(int64 offset, int whence);

 int64 tell(void);

 int64 size(void);

 void flush(void);
 //void flushsync(void);	// TODO: see fflush and fsync

 void close(void);	// Flushes and closes the underlying OS/C lib file.  Calling any other method of this class after a call to
			// this method is illegal(except for the implicit call to the destructor).
			//
			// This is necessary since there can be errors when closing a file, and we can't safely throw an
			// exception from the destructor.
			//
			// Manually calling this method isn't strictly necessary, it'll be called from the destructor
			// automatically, but calling is strongly recommended when the file is opened for writing.
 private:

 FileWrapper & operator=(const FileWrapper &);    // Assignment operator
 FileWrapper(const FileWrapper &);		// Copy constructor
 //FileWrapper(FileWrapper &);                // Copy constructor

 FILE *fp;
 std::string path_save;
 const int OpenedMode;
};

#endif
