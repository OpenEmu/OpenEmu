#ifndef __MDFN_LEPACKER_H
#define __MDFN_LEPACKER_H

#include "mednafen.h"

#include <vector>
#include <stdexcept>

/* Little-endian byte packer(and unpacker). */

namespace MDFN
{

class LEPacker : public std::vector<uint8>
{
 public:

 LEPacker() : read_mode(0), read_pos(0)
 {

 }


 inline void set_read_mode(bool new_read_mode)
 {
  read_mode = new_read_mode;
 }

 inline void reset_read_pos(void)
 {
  read_pos = 0;
 }

 template<typename T> void operator^(T &val)
 {
  size_type csize = size();

  if(read_mode)
  {
   if((read_pos + sizeof(T)) > csize)
    throw(std::out_of_range("LEPacker::operator^"));

   uint8 *ptr = &(*this)[read_pos];
   val = 0;

   for(unsigned int n = 0; n < sizeof(T); n++)
    val |= ((T)ptr[n]) << (n << 3);

   read_pos += sizeof(T);
  }
  else
  {
   resize(csize + sizeof(T));

   uint8 *ptr = &(*this)[csize];

   for(unsigned int n = 0; n < sizeof(T); n++)
    ptr[n] = val >> (n << 3);
  }
 }

 void operator^(bool &val)
 {
  size_type csize = size();

  if(read_mode)
  {
   if((read_pos + sizeof(bool)) > csize)
    throw(std::out_of_range("LEPacker::operator^ for bool"));

   uint8 *ptr = &(*this)[read_pos];
   val = (bool)*ptr;

   read_pos += sizeof(bool);
  }
  else
  {
   resize(csize + 1);

   (*this)[csize] = (unsigned int)val;
  }
 }

 private:

 bool read_mode;
 uint64 read_pos;
};

}

#endif
