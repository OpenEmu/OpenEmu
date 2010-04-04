#pragma once

#include <winsock.h>

#define LINK_PARENTLOST 0x80
#define UNSUPPORTED -1
#define MULTIPLAYER 0
#define NORMAL8 1
#define NORMAL32 2
#define UART 3
#define JOYBUS 4
#define GP 5
#define RFU_INIT 0
#define RFU_COMM 1
#define RFU_SEND 2
#define RFU_RECV 3

typedef struct {
	WORD linkdata[4];
	WORD linkcmd[4];
	WORD numtransfers;
	int lastlinktime;
	unsigned char numgbas;
	unsigned char linkflags;
	int rfu_q[4];
	u8 rfu_request[4];
	int rfu_linktime[4];
	u32 rfu_bdata[4][7];
	u32 rfu_data[4][32];
} LINKDATA;

class lserver{
	int numbytes;
	fd_set fdset;
	timeval wsocktimeout;
	//timeval udptimeout;
	char inbuffer[256], outbuffer[256];
	int *intinbuffer;
	u16 *u16inbuffer;
	int *intoutbuffer;
	u16 *u16outbuffer;
	int counter;
	int done;
public:
	int howmanytimes;
	SOCKET tcpsocket[4];
	SOCKADDR_IN udpaddr[4];
	lserver(void);
	int Init(void*);
	void Send(void);
	void Recv(void);
};

class lclient{
	fd_set fdset;
	timeval wsocktimeout;
	char inbuffer[256], outbuffer[256];
	int *intinbuffer;
	u16 *u16inbuffer;
	int *intoutbuffer;
	u16 *u16outbuffer;
	int numbytes;
public:
	bool oncesend;
	SOCKADDR_IN serverinfo;
	SOCKET noblock;
	int numtransfers;
	lclient(void);
	int Init(LPHOSTENT, void*);
	void Send(void);
	void Recv(void);
	void CheckConn(void);
};

typedef struct {
	SOCKET tcpsocket;
	//SOCKET udpsocket;
	int numgbas;
	HANDLE thread;
	u8 type;
	u8 server;
	bool terminate;
	bool connected;
	bool speed;
	bool active;
} LANLINKDATA;

extern void LinkUpdate(void);
extern void LinkChildStop(void);
extern void LinkChildSend(u16);
extern int openLinkLog(void);
extern void closeLinkLog();
extern void CloseLanLink(void);
extern char *MakeInstanceFilename(const char *Input);

extern LANLINKDATA lanlink;
extern FILE *linklogfile;
extern int vbaid;
extern int linklog;
extern bool adapter;
extern bool linkenable;
extern int linktimeout;
extern lclient lc;
extern int linkid;
