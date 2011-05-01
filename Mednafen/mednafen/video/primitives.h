#ifndef __MDFN_VIDEO_PRIMITIVES_H
#define __MDFN_VIDEO_PRIMITIVES_H

void MDFN_DrawRectangle(uint32 *XBuf, int pitch, int xpos, int ypos, uint32 color, uint32 width, uint32 height);
void MDFN_DrawRectangleAlpha(uint32 *XBuf, int pitch, int xpos, int ypos, uint32 color, uint32 alpha_color, uint32 width, uint32 height);
void MDFN_DrawRectangleFill(uint32 *XBuf, int pitch, int xpos, int ypos, uint32 color, uint32 fillcolor, uint8 width, uint8 height);

#endif
