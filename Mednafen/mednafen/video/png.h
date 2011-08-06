#ifndef __MDFN_PNG_H
#define __MDFN_PNG_H

#include "../video.h"
#include "../FileWrapper.h"

class PNGWrite
{
 public:

 PNGWrite(const char *path, const MDFN_Surface *src, const MDFN_Rect &rect, const MDFN_Rect *LineWidths);
 #if 0
 PNGWrite(FileWrapper &pngfile, const MDFN_Surface *src, const MDFN_Rect &rect, const MDFN_Rect *LineWidths);
 #endif

 ~PNGWrite();


 static void WriteChunk(FileWrapper &pngfile, uint32 size, const char *type, const uint8 *data);

 private:

 void WriteIt(FileWrapper &pngfile, const MDFN_Surface *src, const MDFN_Rect &rect, const MDFN_Rect *LineWidths);

 FileWrapper ownfile;
 std::vector<uint8> compmem;
 std::vector<uint8> tmp_buffer;
};

#endif

