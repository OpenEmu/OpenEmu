/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mednafen.h"
#include "FileWrapper.h"

#include <trio/trio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Some really bad preprocessor abuse follows to handle platforms that don't have fseeko and ftello...and of course
// for largefile support on Windows:

#ifndef HAVE_FSEEKO
 #define fseeko fseek
#endif

#ifndef HAVE_FTELLO
 #define ftello ftell
#endif

#if SIZEOF_OFF_T == 4

 #ifdef HAVE_FOPEN64
  #define fopen fopen64
 #endif

 #ifdef HAVE_FTELLO64
  #undef ftello
  #define ftello ftello64
 #endif

 #ifdef HAVE_FSEEKO64
  #undef fseeko
  #define fseeko fseeko64
 #endif

 #ifdef HAVE_FSTAT64
  #define fstat fstat64
  #define stat stat64
 #endif

#endif

// For special uses, IE in classes that take a path or a FileWrapper & in the constructor, and the FileWrapper non-pointer member
// is in the initialization list for the path constructor but not the constructor with FileWrapper&
#if 0
FileWrapper::FileWrapper()
{
 fp = NULL;
}
#endif

#if 0
FileWrapper::FileWrapper(FileWrapper &original) : OpenedMode(original.OpenedMode)
{
 int fd;
 int duped_fd;

 path_save = original.path_save;

 original.flush();

 fd = fileno(original.fp);
 if(-1 == (duped_fd = dup(fd)))
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error duping file descriptor: %s"), ene.StrError()));
 }

 if(!(fp = fdopen(duped_fd, (OpenedMode == MODE_READ) ? "rb" : "wb")))
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error during fdopen(): %s"), ene.StrError()));
 }
}
#endif

FileWrapper::FileWrapper(const char *path, const int mode, const char *purpose) : OpenedMode(mode)
{
 path_save = std::string(path);

 if(mode == MODE_READ)
  fp = fopen(path, "rb");
 else if(mode == MODE_WRITE)
  fp = fopen(path, "wb");
 else if(mode == MODE_WRITE_SAFE)	// SO ANNOYING
 {
  int open_flags = O_WRONLY | O_CREAT | O_EXCL;

  #ifdef O_BINARY
   open_flags |= O_BINARY;
  #elif defined(_O_BINARY)
   open_flags |= _O_BINARY;
  #endif

  #if defined(S_IRGRP) && defined(S_IROTH) 
  int tmpfd = open(path, open_flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  #else
  int tmpfd = open(path, open_flags, S_IRUSR | S_IWUSR);
  #endif
  if(tmpfd == -1)
  {
   ErrnoHolder ene(errno);

   if(purpose)
    throw(MDFN_Error(ene.Errno(), _("Error opening file \"%s\" for \"%s\": %s"), path_save.c_str(), purpose, ene.StrError()));
   else
    throw(MDFN_Error(ene.Errno(), _("Error opening file \"%s\": %s"), path_save.c_str(), ene.StrError()));
  }
  fp = fdopen(tmpfd, "wb");
 }

 if(!fp)
 {
  ErrnoHolder ene(errno);

  if(purpose)
   throw(MDFN_Error(ene.Errno(), _("Error opening file \"%s\" for \"%s\": %s"), path_save.c_str(), purpose, ene.StrError()));
  else
   throw(MDFN_Error(ene.Errno(), _("Error opening file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }
}

FileWrapper::~FileWrapper()
{
 try
 {
  close();
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
 }
}

void FileWrapper::close(void)
{
 if(fp)
 {
  FILE *tmp = fp;

  fp = NULL;

  if(fclose(tmp) == EOF)
  {
   ErrnoHolder ene(errno);

   throw(MDFN_Error(ene.Errno(), _("Error closing opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
  }
 }
}

uint64 FileWrapper::read(void *data, uint64 count, bool error_on_eof)
{
 uint64 read_count;

 clearerr(fp);

 read_count = fread(data, 1, count, fp);

 if(read_count != count)
 {
  ErrnoHolder ene(errno);

  if(ferror(fp))
   throw(MDFN_Error(ene.Errno(), _("Error reading from opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));

  if(error_on_eof)
   throw(MDFN_Error(0, _("Error reading from opened file \"%s\": %s"), path_save.c_str(), _("Unexpected EOF")));
 }

 return(read_count);
}

void FileWrapper::flush(void)
{
 if(fflush(fp) == EOF)
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error flushing to opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }
}

void FileWrapper::write(const void *data, uint64 count)
{
 if(fwrite(data, 1, count, fp) != count)
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error writing to opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }
}

int FileWrapper::scanf(const char *format, ...)
{
 va_list ap;
 int ret;

 clearerr(fp);

 va_start(ap, format);

 ret = trio_vfscanf(fp, format, ap);

 va_end(ap);

 if(ferror(fp))
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error reading from opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }

 //if(ret < 0 || ret == EOF)
 // throw(MDFN_Error(0, _("%s error on format string \"%s\""), "trio_vfscanf()", format));

 return(ret);
}

void FileWrapper::printf(const char *format, ...)
{
 va_list ap;

 clearerr(fp);

 va_start(ap, format);

 trio_vfprintf(fp, format, ap);

 va_end(ap);

 if(ferror(fp))
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error writing to opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }
}

void FileWrapper::put_char(int c)
{
 if(fputc(c, fp) == EOF)
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error writing to opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }
}

void FileWrapper::put_string(const char *str)
{
 write(str, strlen(str));
}

// We need to decide whether to prohibit NULL characters in output and input strings via std::string.
// Yes for correctness, no for potential security issues(though unlikely in context all things considered).
void FileWrapper::put_string(const std::string &str)
{
 write(str.data(), str.size());
}

char *FileWrapper::get_line(char *buf_s, int buf_size)
{
 char *ret;

 clearerr(fp);
 ret = ::fgets(buf_s, buf_size, fp);
 if(ferror(fp))
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error reading line in opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }
 return(ret);
}


void FileWrapper::seek(int64 offset, int whence)
{
 if(fseeko(fp, offset, whence) == -1)
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error seeking in opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }
}

int64 FileWrapper::size(void)
{
 struct stat buf;

 if(fstat(fileno(fp), &buf) == -1)
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error getting the size of opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }

 return(buf.st_size);

/* TODO for systems without fstat()?
  int64 orig_pos = tell();
  int64 ret;

  seek(0, SEEK_END);

  ret = tell();

  seek(orig_pos, SEEK_SET);
 */
}

int64 FileWrapper::tell(void)
{
 int64 offset;

 offset = ftello(fp);

 if(offset == -1)
 {
  ErrnoHolder ene(errno);

  throw(MDFN_Error(ene.Errno(), _("Error getting position in opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
 }

 return(offset);
}


