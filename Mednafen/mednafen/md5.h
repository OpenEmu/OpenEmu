#ifndef _MD5_H
#define _MD5_H

#include <string>

class md5_context
{
 public:
 md5_context(void);
 ~md5_context(void);

 static std::string asciistr(const uint8 digest[16], bool borked_order);
 std::string asciistr(void);
 void starts(void);
 void update(const uint8 *input, uint32 length);
 inline void update_u32_as_lsb(const uint32 input)
 {
  uint8 buf[4];

  buf[0] = input >> 0;
  buf[1] = input >> 8;
  buf[2] = input >> 16;
  buf[3] = input >> 24;

  update(buf, 4);
 }

 inline void update_string(const char *string)
 {
  update((const uint8 *)string, strlen(string));
 }
 void finish(uint8 digest[16]); 

 private:
 void process(const uint8 data[64]);
 uint32 total[2];
 uint32 state[4];
 uint8 buffer[64];
};

#endif /* md5.h */
