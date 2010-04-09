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
 void finish(uint8 digest[16]); 

 private:
 void process(const uint8 data[64]);
 uint32 total[2];
 uint32 state[4];
 uint8 buffer[64];
};

#endif /* md5.h */
