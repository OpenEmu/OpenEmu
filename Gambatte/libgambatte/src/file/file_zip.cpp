/***************************************************************************
Copyright (C) 2007 by Nach
http://nsrt.edgeemu.com

Copyright (C) 2007-2011 by Sindre Aamås
sinamas@users.sourceforge.net

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License version 2 for more details.

You should have received a copy of the GNU General Public License
version 2 along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
***************************************************************************/
#include "stdfile.h"
#include <cctype>
#include <cstring>

namespace zlib {
#include "unzip/unzip.h"
}

namespace {

class ZipFile : public gambatte::File {
  private:
  std::size_t fsize, count;
  void *zipfile;
  bool zip_sub_open;

  void zip(const char *filename);

  public:
  ZipFile(const char *filename);
  virtual ~ZipFile();
  virtual void rewind();
  bool is_open() const;
  virtual void close();
  virtual std::size_t size() const { return fsize; };
  virtual void read(char *buffer, std::size_t amount);
  std::size_t gcount() const { return count; }
  virtual bool fail() const { return !is_open(); }
};

using namespace std;
using namespace zlib;

static const unsigned int MAX_FILE_NAME = 512;

ZipFile::ZipFile(const char *filename) : fsize(0), count(0)
{
  zip(filename);
}

void ZipFile::zip(const char *filename)
{
  zipfile = unzOpen(filename);
  if (zipfile)
  {
    zip_sub_open = false;

    unz_file_info cFileInfo;
    char ourFile[MAX_FILE_NAME] = { '\n' };

    for (int cFile = unzGoToFirstFile((unzFile)zipfile); cFile == UNZ_OK; cFile = unzGoToNextFile((unzFile)zipfile))
    {
      //Temporary char array for file name
      char cFileName[MAX_FILE_NAME];

      //Gets info on current file, and places it in cFileInfo
      unzGetCurrentFileInfo((unzFile)zipfile, &cFileInfo, cFileName, MAX_FILE_NAME, 0, 0, 0, 0);

      //Check for largest file which should be the ROM
      if ((size_t)cFileInfo.uncompressed_size > fsize)
      {
        strcpy(ourFile, cFileName);
        fsize = (size_t)cFileInfo.uncompressed_size;
      }
    }

    if (ourFile[0] != '\n')
    {
      //Sets current file to the file we liked before
      unzLocateFile((unzFile)zipfile, ourFile, 1);

      if (unzOpenCurrentFile((unzFile)zipfile) == UNZ_OK)
      {
        zip_sub_open = true;
      }
    }

    if (!zip_sub_open)
    {
      unzClose((unzFile)zipfile);
      zipfile = 0;
    }
  }
}

ZipFile::~ZipFile()
{
  close();
}

void ZipFile::rewind()
{
  if (is_open())
  {
    unzCloseCurrentFile((unzFile)zipfile);
    unzOpenCurrentFile((unzFile)zipfile);
  }
}

bool ZipFile::is_open() const
{
  return(zipfile && zip_sub_open);
}

void ZipFile::close()
{
  if (is_open())
  {
    unzOpenCurrentFile((unzFile)zipfile);
    unzClose((unzFile)zipfile);
    zipfile = 0;
    zip_sub_open = false;
  }
}

void ZipFile::read(char *buffer, size_t amount)
{
  if (is_open())
  {
    count = (size_t)unzReadCurrentFile((unzFile)zipfile, buffer, amount);
  }
  else
  {
    count = 0;
  }
}

class GzFile : public gambatte::File {
	gzFile file_;
	std::size_t fsize_;
	
	void close();
	GzFile(const GzFile &);
	GzFile& operator=(const GzFile &);
public:
	explicit GzFile(const char *filename)
	: file_(gzopen(filename, "rb")), fsize_(0)
	{
		if (file_) {
			char buf[256];
			int ret;
			
			while ((ret = gzread(file_, buf, sizeof buf)) > 0)
				fsize_ += ret;
			
			if (ret < 0) {
				close();
				fsize_ = 0;
			}
		}
		
		rewind();
	}
	
	virtual ~GzFile() { close(); }
	
	virtual void rewind() {
		if (file_ && gzrewind(file_) < 0)
			close();
	}
	
	virtual std::size_t size() const { return fsize_; };
	
	virtual void read(char *buffer, std::size_t amount) {
		if (file_ && gzread(file_, buffer, amount) < 0)
			close();
	}
	
	virtual bool fail() const { return !file_; }
};

void GzFile::close() {
	if (file_) {
		gzclose(file_);
		file_ = 0;
	}
}

}

// Avoid checking magic header values, because there are no values that cannot occur in a GB ROM.
std::auto_ptr<gambatte::File> gambatte::newFileInstance(const std::string &filepath) {
	const std::size_t extpos = filepath.rfind(".");
	
	if (extpos != std::string::npos) {
		const std::string &ext = filepath.substr(extpos + 1);
		
		if (ext.length() == 3 && std::tolower(ext[0]) == 'z' && std::tolower(ext[1]) == 'i'&& std::tolower(ext[2]) == 'p')
			return std::auto_ptr<File>(new ZipFile(filepath.c_str()));
		
		if (!ext.empty() && std::tolower(ext[ext.length() - 1]) == 'z')
			return std::auto_ptr<File>(new GzFile(filepath.c_str()));
	}
	
	return std::auto_ptr<File>(new StdFile(filepath.c_str()));
}
