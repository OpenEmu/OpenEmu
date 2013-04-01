#ifndef _MDFN_NETPLAY_H
#define _MDFN_NETPLAY_H

int InitNetplay(void);

void NetplayUpdate(const char **, void *PortData[], uint32 PortLen[], int NumPorts);
int NetplayStart(const char *PortDeviceCache[16], const uint32 PortDataLenCache[16]);
void NetplaySendState(void);
bool NetplaySendCommand(uint8, uint32);

extern int MDFNnetplay;

#define MDFNNPCMD_RESET 	MDFN_MSC_RESET
#define MDFNNPCMD_POWER 	MDFN_MSC_POWER

#define MDFNNPCMD_VSUNICOIN     MDFN_MSC_INSERT_COIN
#define MDFNNPCMD_VSUNIDIP0	MDFN_MSC_TOGGLE_DIP0
#define MDFNNPCMD_FDSINSERTx	MDFN_MSC_INSERT_DISK0
#define MDFNNPCMD_FDSINSERT	MDFN_MSC_INSERT_DISK
#define MDFNNPCMD_FDSEJECT	MDFN_MSC_EJECT_DISK
#define MDFNNPCMD_FDSSELECT	MDFN_MSC_SELECT_DISK

#define MDFNNPCMD_SETFPS        0x40 	// Client->server.  It should be ignored server-side if it's not from the first
                                        // active player for the game).

#define MDFNNPCMD_NOP           0x41	// Client->server.

//
#define MDFNNPCMD_CTRL_CHANGE     0x43  // Server->client.
#define MDFNNPCMD_CTRL_CHANGE_ACK 0x44	// Client->server.  Acknowledge controller change.  Sent using old local data length, everything after
					// this should be new data size.
//

#define MDFNNPCMD_CTRLR_SWAP_NOTIF	0x68	// Server->Client

#define MDFNNPCMD_CTRLR_TAKE		0x70	// Client->server.  Take the specified controllers(from other clients)
#define MDFNNPCMD_CTRLR_DROP		0x71	// Client->server.  Drop(relinquish) the specified controllers.
#define MDFNNPCMD_CTRLR_DUPE		0x72	// Client->server.  Take the specified controllers(but let other clients still keep their control).


#define MDFNNPCMD_CTRLR_SWAP		0x78	// Client->server.

#define MDFNNPCMD_REQUEST_LIST	0x7F	// client->server

#define MDFNNPCMD_LOADSTATE     0x80	// Client->server, and server->client
#define MDFNNPCMD_REQUEST_STATE 0x81	// Server->client

#define MDFNNPCMD_TEXT		0x90

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

#define MDFNNPCMD_LIST		0xC0 // Server->client

#define MDFNNPCMD_CTRLR_TAKE_NOTIF	0xF0	// Server->client
#define MDFNNPCMD_CTRLR_DROP_NOTIF	0xF1	// Server->client
#define MDFNNPCMD_CTRLR_DUPE_NOTIF	0xF2	// Server->client


#define MDFNNPCMD_QUIT          0xFF // Client->server

int MDFNNET_SendCommand(uint8, uint32);
#endif
