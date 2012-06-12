#ifndef __MDFN_PLAYER_H
#define __MDFN_PLAYER_H

int Player_Init(int tsongs, const std::string &album, const std::string &artist, const std::string &copyright,const std::vector<std::string> &snames = std::vector<std::string>());
int Player_Init(int tsongs, const std::string &album, const std::string &artist, const std::string &copyright, char **snames);

void Player_Draw(MDFN_Surface *surface, MDFN_Rect *dr, int CurrentSong, int16 *samples, int32 sampcount);

#endif
