#ifndef __MDFN_LEPACKER_H
#define __MDFN_LEPACKER_H

#include "mednafen.h"

#include <vector>
#include <stdexcept>
#include <stdlib.h>

/* Little-endian byte packer(and unpacker). */

namespace MDFN
{

class LEPacker;
class LEPackable
{
 public:
 virtual void pack(LEPacker &lep) = 0;
};

class LEPacker : public std::vector<uint8>
{
 public:

 LEPacker() : read_mode(0), read_pos(0), randomize_read_mode(0)
 {

 }


 inline void set_read_mode(bool new_read_mode, bool new_randomize_read_mode = false)
 {
  read_mode = new_read_mode;
  randomize_read_mode = new_randomize_read_mode;
 }

 inline void reset_read_pos(void)
 {
  read_pos = 0;
 }

 void operator^(LEPackable &o)
 {
  o.pack(*this);
 }

 template<typename T> INLINE void operator^(T &val)
 {
  size_type csize = size();

  if(read_mode)
  {
   if((read_pos + sizeof(T)) > csize)
    throw(std::out_of_range("LEPacker::operator^"));

   uint8 *ptr = &(*this)[read_pos];
   val = 0;

   if(randomize_read_mode)
   {
    for(unsigned int n = 0; n < sizeof(T); n++)
     val |= ((T)((rand() >> 4) & 0xFF)) << (n << 3);
   }
   else
   {
    for(unsigned int n = 0; n < sizeof(T); n++)
     val |= ((T)ptr[n]) << (n << 3);
   }

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

 INLINE void operator^(bool &val)
 {
  uint8 tmp = val;

  (*this) ^ tmp;

  if(read_mode)
   val = tmp;
 }

 private:

 bool read_mode;
 uint64 read_pos;
 bool randomize_read_mode;
};

}

#endif
