#ifndef __MDFN_PLAYER_H
#define __MDFN_PLAYER_H

int Player_Init(int tsongs, UTF8 *album, UTF8 *artist, UTF8 *copyright, UTF8 **snames);
void Player_Draw(uint32 *XBuf, int CurrentSong, int16 *samples, int32 sampcount);

#endif
