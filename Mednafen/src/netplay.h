#ifndef _MDFN_NETPLAY_H
#define _MDFN_NETPLAY_H

int InitNetplay(void);

void NetplayUpdate(const char **, void *PortData[], uint32 PortLen[], int NumPorts);

int NetplayStart(const char *PortDeviceCache[16], const uint32 PortDataLenCache[16], uint32 local_players, uint32 netmerge, const std::string &nickname, const std::string &game_key, const std::string &connect_password);

extern int MDFNnetplay;

#define MDFNNPCMD_RESET 	0x01
#define MDFNNPCMD_POWER 	0x02

#define MDFNNPCMD_VSUNICOIN     0x07
#define MDFNNPCMD_VSUNIDIP0	0x08
#define MDFNNPCMD_FDSINSERTx	0x10
#define MDFNNPCMD_FDSINSERT	0x18
#define MDFNNPCMD_FDSEJECT	0x19
#define MDFNNPCMD_FDSSELECT	0x1A

#define MDFNNPCMD_SETFPS        0x40 /* Sent from client to server ONLY(it should be ignored server-side if it's not from the first
                                        active player for the game). */

#define MDFNNPCMD_LOADSTATE     0x80

#define MDFNNPCMD_SAVESTATE     0x81 /* Sent from server to client. */

#define MDFNNPCMD_TEXT		0x90

#define MDFNNPCMD_SETSETTING	0x91 /* TODO:  WIP */

#define MDFNNPCMD_SETNICK       0x98 /* Sent from client to server only. */

#define MDFNNPCMD_PLAYERJOINED	0xA0	// Data:  <byte: bitmask, which inputs this player controls>
					//	  <bytestream: nickname>
#define MDFNNPCMD_PLAYERLEFT	0xA1	// Data: (see above)
#define MDFNNPCMD_YOUJOINED     0xB0
#define MDFNNPCMD_YOULEFT       0xB1

#define MDFNNPCMD_NICKCHANGED	0xB8

int MDFNNET_SendCommand(uint8, uint32);
int MDFNNET_SendState(void);
#endif
