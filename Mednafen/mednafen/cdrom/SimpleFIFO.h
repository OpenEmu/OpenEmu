#ifndef __MDFN_SIMPLEFIFO_H
#define __MDFN_SIMPLEFIFO_H

#include <vector>
#include <assert.h>

class SimpleFIFO
{
 public:

 // Constructor
 SimpleFIFO(uint32 the_size) // Size should be a power of 2!
 {
  ptr = new uint8[the_size];
  size = the_size;
 }

 // Copy constructor
 SimpleFIFO(const SimpleFIFO &cfifo)
 {
  size = cfifo.size;
  read_pos = cfifo.read_pos;
  write_pos = cfifo.write_pos;
  in_count = cfifo.in_count;  

  ptr = new uint8[cfifo.size];
  memcpy(ptr, cfifo.ptr, cfifo.size);
 }

 // Destructor
 ~SimpleFIFO()
 {
  delete[] ptr;
 }

 inline uint32 CanRead(void)
 {
  return(in_count);
 }

 uint32 CanWrite(void)
 {
  return(size - in_count);
 }

 inline uint8 ReadByte(bool peek = false)
 {
  uint8 ret;

  assert(in_count > 0);

  ret = ptr[read_pos];

  if(!peek)
  {
   read_pos = (read_pos + 1) & (size - 1);
   in_count--;
  }

  return(ret);
 }

 inline void Write(const uint8 *happy_data, uint32 happy_count)
 {
  assert(CanWrite() >= happy_count);

  while(happy_count)
  {
   ptr[write_pos] = *happy_data;

   write_pos = (write_pos + 1) & (size - 1);
   in_count++;
   happy_data++;
   happy_count--;
  }  
 }

 void Flush(void)
 {
  read_pos = 0;
  write_pos = 0;
  in_count = 0;
 }

 //private:
 uint8 *ptr;
 uint32 size;
 uint32 read_pos; // Read position
 uint32 write_pos; // Write position
 uint32 in_count; // Number of bytes in the FIFO
};


#endif
