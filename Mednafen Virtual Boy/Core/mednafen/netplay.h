#ifndef _MDFN_NETPLAY_H
#define _MDFN_NETPLAY_H

int InitNetplay(void);

void NetplayUpdate(const char **, void *PortData[], uint32 PortLen[], int NumPorts);

int NetplayStart(const char *PortDeviceCache[16], const uint32 PortDataLenCache[16], uint32 local_players, uint32 netmerge, const std::string &nickname, const std::string &game_key, const std::string &connect_password);

extern int MDFNnetplay;

#define MDFNNPCMD_RESET 	MDFN_MSC_RESET
#define MDFNNPCMD_POWER 	MDFN_MSC_POWER

#define MDFNNPCMD_VSUNICOIN     MDFN_MSC_INSERT_COIN
#define MDFNNPCMD_VSUNIDIP0	MDFN_MSC_TOGGLE_DIP0
#define MDFNNPCMD_FDSINSERTx	MDFN_MSC_INSERT_DISK0
#define MDFNNPCMD_FDSINSERT	MDFN_MSC_INSERT_DISK
#define MDFNNPCMD_FDSEJECT	MDFN_MSC_EJECT_DISK
#define MDFNNPCMD_FDSSELECT	MDFN_MSC_SELECT_DISK

#define MDFNNPCMD_SETFPS        0x40 /* Sent from client to server ONLY(it should be ignored server-side if it's not from the first
                                        active player for the game). */

#define MDFNNPCMD_LOADSTATE     0x80

#define MDFNNPCMD_SAVESTATE     0x81 /* Sent from server to client. */

#define MDFNNPCMD_TEXT		0x90

#define MDFNNPCMD_SETSETTING	0x91 /* TODO:  WIP */

#define MDFNNPCMD_SERVERTEXT	0x93 // Server text message(informational), server->client
#define MDFNNPCMD_ECHO		0x94 // Echos the string(no larger than 256 bytes) back to the client(used for pinging).

#define MDFNNPCMD_INTEGRITY	0x95 // Send from a client to a server, then from the server to all clients.
#define MDFNNPCMD_INTEGRITY_RES	0x96 // Integrity result, sent from the clients to the server.  The result should be no larger
				     // than 256 bytes.

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
