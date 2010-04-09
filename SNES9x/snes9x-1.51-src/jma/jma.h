/*
Copyright (C) 2005-2006 NSRT Team ( http://nsrt.edgeemu.com )

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef JMA_H
#define JMA_H

#include <string>
#include <fstream>
#include <vector>
#include <time.h>

namespace JMA
{
  enum jma_errors { JMA_NO_CREATE, JMA_NO_MEM_ALLOC, JMA_NO_OPEN, JMA_BAD_FILE,
                    JMA_UNSUPPORTED_VERSION, JMA_COMPRESS_FAILED, JMA_DECOMPRESS_FAILED,
                    JMA_FILE_NOT_FOUND };

  struct jma_file_info_base
  {
    std::string name;
    std::string comment;
    size_t size;
    unsigned int crc32;
  };

  struct jma_public_file_info : jma_file_info_base
  {
    time_t datetime;
  };

  struct jma_file_info : jma_file_info_base
  {
    unsigned short date;
    unsigned short time;
    const unsigned char *buffer;
  };

  template<class jma_file_type>
  inline size_t get_total_size(std::vector<jma_file_type>& files)
  {
    size_t size = 0;
    for (typename std::vector<jma_file_type>::iterator i = files.begin(); i != files.end(); i++)
    {
      size += i->size; //We do have a problem if this wraps around
    }

    return(size);
  }

  class jma_open
  {
    public:
    jma_open(const char *) throw(jma_errors);
    ~jma_open();

    std::vector<jma_public_file_info> get_files_info();
    std::vector<unsigned char *> get_all_files(unsigned char *) throw(jma_errors);
    void extract_file(std::string& name, unsigned char *) throw(jma_errors);
    bool is_solid();

    private:
    std::ifstream stream;
    std::vector<jma_file_info> files;
    size_t chunk_size;
    unsigned char *decompressed_buffer;
    unsigned char *compressed_buffer;

    void chunk_seek(unsigned int) throw(jma_errors);
    void retrieve_file_block() throw(jma_errors);
  };

  const char *jma_error_text(jma_errors);
}
#endif
