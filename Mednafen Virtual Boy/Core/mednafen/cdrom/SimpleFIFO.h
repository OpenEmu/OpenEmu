#ifndef __MDFN_SIMPLEFIFO_H
#define __MDFN_SIMPLEFIFO_H

#include <vector>
#include <assert.h>

#include "../math_ops.h"

template<typename T>
class SimpleFIFO
{
 public:

 // Constructor
 SimpleFIFO(uint32 the_size) // Size should be a power of 2!
 {
  data.resize(round_up_pow2(the_size));
  size = the_size;
  read_pos = 0;
  write_pos = 0;
  in_count = 0;
 }

 // Destructor
 INLINE ~SimpleFIFO()
 {

 }

 INLINE uint32 CanRead(void)
 {
  return(in_count);
 }

 INLINE uint32 CanWrite(void)
 {
  return(size - in_count);
 }

 INLINE T ReadUnit(bool peek = false)
 {
  T ret;

  assert(in_count > 0);

  ret = data[read_pos];

  if(!peek)
  {
   read_pos = (read_pos + 1) & (data.size() - 1);
   in_count--;
  }

  return(ret);
 }

 INLINE uint8 ReadByte(bool peek = false)
 {
  assert(sizeof(T) == 1);

  return(ReadUnit(peek));
 }

 INLINE void Write(const T *happy_data, uint32 happy_count)
 {
  assert(CanWrite() >= happy_count);

  while(happy_count)
  {
   data[write_pos] = *happy_data;

   write_pos = (write_pos + 1) & (data.size() - 1);
   in_count++;
   happy_data++;
   happy_count--;
  }
 }

 INLINE void WriteUnit(const T& wr_data)
 {
  Write(&wr_data, 1);
 }

 INLINE void WriteByte(const T& wr_data)
 {
  assert(sizeof(T) == 1);
  Write(&wr_data, 1);
 }


 INLINE void Flush(void)
 {
  read_pos = 0;
  write_pos = 0;
  in_count = 0;
 }

 //private:
 std::vector<T> data;
 uint32 size;
 uint32 read_pos; // Read position
 uint32 write_pos; // Write position
 uint32 in_count; // Number of units in the FIFO
};


#endif
