#ifndef GBA_GBALINK_H
#define GBA_GBALINK_H

#pragma once

// register definitions; these are always present

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

#define COMM_SIODATA32_L	0x120
#define COMM_SIODATA32_H	0x122
#define COMM_SIOCNT			0x128
#define COMM_SIODATA8		0x12a
#define COMM_SIOMLT_SEND 0x12a
#define COMM_SIOMULTI0 0x120
#define COMM_SIOMULTI1 0x122
#define COMM_SIOMULTI2 0x124
#define COMM_SIOMULTI3 0x126
#define COMM_RCNT			0x134
#define COMM_JOYCNT			0x140
#define COMM_JOY_RECV_L		0x150
#define COMM_JOY_RECV_H		0x152
#define COMM_JOY_TRANS_L	0x154
#define COMM_JOY_TRANS_H	0x156
#define COMM_JOYSTAT		0x158

#define JOYSTAT_RECV		2
#define JOYSTAT_SEND		8

#define JOYCNT_RESET			1
#define JOYCNT_RECV_COMPLETE	2
#define JOYCNT_SEND_COMPLETE	4
#define JOYCNT_INT_ENABLE		0x40

enum
{
	JOY_CMD_RESET	= 0xff,
	JOY_CMD_STATUS	= 0x00,
	JOY_CMD_READ	= 0x14,
	JOY_CMD_WRITE	= 0x15		
};

// Link implementation; only present if enabled
#ifndef NO_LINK
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

class ServerInfoDisplay
{
public:
    virtual void ShowServerIP(sf::IPAddress addr) = 0;
    virtual void ShowConnect(int player) = 0;
    virtual void Ping() = 0;
    virtual void Connected() = 0;
};

typedef struct {
	u16 linkdata[5];
	u16 linkcmd;
	u16 numtransfers;
	int lastlinktime;
	u8 numgbas;
	u8 trgbas;
	u8 linkflags;
	int rfu_q[4];
	u8 rfu_request[4];
	int rfu_linktime[4];
	u32 rfu_bdata[4][7];
	u32 rfu_data[4][32];
} LINKDATA;

class lserver{
	int numbytes;
	sf::Selector<sf::SocketTCP> fdset;
	//timeval udptimeout;
	char inbuffer[256], outbuffer[256];
	s32 *intinbuffer;
	u16 *u16inbuffer;
	s32 *intoutbuffer;
	u16 *u16outbuffer;
	int counter;
	int done;
public:
	int howmanytimes;
	sf::SocketTCP tcpsocket[4];
	sf::IPAddress udpaddr[4];
	lserver(void);
	bool Init(ServerInfoDisplay *);
	void Send(void);
	void Recv(void);
};

class ClientInfoDisplay {
public:
    virtual void ConnectStart(sf::IPAddress addr) = 0;
    virtual void Ping() = 0;
    virtual void ShowConnect(int player, int togo) = 0;
    virtual void Connected() = 0;
};

class lclient{
	sf::Selector<sf::SocketTCP> fdset;
	char inbuffer[256], outbuffer[256];
	s32 *intinbuffer;
	u16 *u16inbuffer;
	s32 *intoutbuffer;
	u16 *u16outbuffer;
	int numbytes;
public:
	bool oncesend;
	sf::IPAddress serveraddr;
	unsigned short serverport;
	sf::SocketTCP noblock;
	int numtransfers;
	lclient(void);
	bool Init(sf::IPAddress, ClientInfoDisplay *);
	void Send(void);
	void Recv(void);
	void CheckConn(void);
};

typedef struct {
	sf::SocketTCP tcpsocket;
	//sf::SocketUDP udpsocket;
	int numslaves;
	sf::Thread *thread;
	int type;
	bool server;
	bool terminate;
	bool connected;
	bool speed;
	bool active;
} LANLINKDATA;

extern bool gba_joybus_enabled;
extern sf::IPAddress joybusHostAddr;
extern void JoyBusConnect();
extern void JoyBusShutdown();
extern void JoyBusUpdate(int ticks);

extern bool gba_link_enabled;

extern bool InitLink();
extern void CloseLink();
extern void StartLink(u16);
extern void StartGPLink(u16);
extern void LinkSSend(u16);
extern void LinkUpdate(int);
extern void LinkChildStop();
extern void LinkChildSend(u16);
extern void CloseLanLink();
extern void CleanLocalLink();
extern const char *MakeInstanceFilename(const char *Input);
extern LANLINKDATA lanlink;
extern int vbaid;
extern bool rfu_enabled;
extern int linktimeout;
extern lclient lc;
extern lserver ls;
extern int linkid;

#else

// stubs to keep #ifdef's out of mainline
#define StartLink(x)
#define StartGPLink(x)
#define LinkSSend(x)
#define LinkUpdate(x)
#define JoyBusUpdate(x)
#define InitLink() false
#define CloseLink()
#define gba_link_enabled false
#define gba_joybus_enabled false
#endif

#endif /* GBA_GBALINK_H */
