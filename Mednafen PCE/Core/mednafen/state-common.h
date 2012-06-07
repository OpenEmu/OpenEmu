#ifndef __MDFN_STATE_COMMON_H
#define __MDFN_STATE_COMMON_H

typedef struct
{
 int status[10];
 int current;
 int current_movie; // For movies only, status(recording/playback)

 // The most recently-saved-to slot
 int recently_saved;

 // R, G, B, R, G, B, R, G, B, ... for w * h * 3
 uint8 *gfx;
 uint32 w, h;
} StateStatusStruct;

#endif
