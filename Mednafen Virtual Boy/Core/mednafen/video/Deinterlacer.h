#ifndef __MDFN_DEINTERLACER_H
#define __MDFN_DEINTERLACER_H

#include <vector>

class Deinterlacer
{
 public:

 Deinterlacer();
 ~Deinterlacer();

 void Process(MDFN_Surface *surface, const MDFN_Rect &DisplayRect, MDFN_Rect *LineWidths, const bool field);

 void ClearState(void);

 private:

 MDFN_Surface *FieldBuffer;
 std::vector<int32> LWBuffer;
 bool StateValid;
 int32 PrevHeight;
};

#endif
