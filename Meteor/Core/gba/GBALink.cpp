// This file was written by denopqrihg
// with major changes by tjm

#ifndef NO_LINK
// Joybus
bool gba_joybus_enabled = false;

// If disabled, gba core won't call any (non-joybus) link functions
bool gba_link_enabled = false;

#define LOCAL_LINK_NAME "VBA link memory"
#define IP_LINK_PORT 5738

#include <string.h>
#include <stdio.h>
#include "../common/Port.h"
#include "GBA.h"
#include "GBALink.h"
#include "GBASockClient.h"
#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) x
#endif
#define N_(x) x
#ifdef __WIN32__
#include <windows.h>
#else
#include <sys/mman.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#define ReleaseSemaphore(sem, nrel, orel) do { \
	for(int i = 0; i < nrel; i++) \
		sem_post(sem); \
} while(0)
#define WAIT_TIMEOUT -1
#ifdef HAVE_SEM_TIMEDWAIT
int WaitForSingleObject(sem_t *s, int t)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += t/1000;
	ts.tv_nsec += (t%1000) * 1000000;
	do {
		if(!sem_timedwait(s, &ts))
			return 0;
	} while(errno == EINTR);
	return WAIT_TIMEOUT;
}

// urg.. MacOSX has no sem_timedwait (POSIX) or semtimedop (SYSV)
// so we'll have to simulate it..
// MacOSX also has no clock_gettime, and since both are "real-time", assume
// anyone who doesn't have one also doesn't have the other

// 2 ways to do this:
//   - poll & sleep loop
//   - poll & wait for timer interrupt loop

// the first consumes more CPU and requires selection of a good sleep value

// the second may interfere with other timers running on system, and
// requires that a dummy signal handler be installed for SIGALRM
#else
#include <sys/time.h>
#ifndef TIMEDWAIT_ALRM
#define TIMEDWAIT_ALRM 1
#endif
#if TIMEDWAIT_ALRM
#include <signal.h>
static void alrmhand(int sig)
{
}
#endif
int WaitForSingleObject(sem_t *s, int t)
{
#if !TIMEDWAIT_ALRM
	struct timeval ts;
	gettimeofday(&ts, NULL);
	ts.tv_sec += t/1000;
	ts.tv_usec += (t%1000) * 1000;
#else
	struct sigaction sa, osa;
	sigaction(SIGALRM, NULL, &osa);
	sa = osa;
	sa.sa_flags &= ~SA_RESTART;
	sa.sa_handler = alrmhand;
	sigaction(SIGALRM, &sa, NULL);
	struct itimerval tv, otv;
	tv.it_value.tv_sec = t / 1000;
	tv.it_value.tv_usec = (t%1000) * 1000;
	// this should be 0/0, but in the wait loop, it's possible to
	// have the signal fire while not in sem_wait().  This will ensure
	// another signal within 1ms
	tv.it_interval.tv_sec = 0;
	tv.it_interval.tv_usec = 999;
	setitimer(ITIMER_REAL, &tv, &otv);
#endif
	while(1) {
#if !TIMEDWAIT_ALRM
		if(!sem_trywait(s))
			return 0;
		struct timeval ts2;
		gettimeofday(&ts2, NULL);
		if(ts2.tv_sec > ts.tv_sec || (ts2.tv_sec == ts.tv_sec &&
					      ts2.tv_usec > ts.tv_usec)) {
			return WAIT_TIMEOUT;
		}
		// is .1 ms short enough?  long enough?  who knows?
		struct timespec ts3;
		ts3.tv_sec = 0;
		ts3.tv_nsec = 100000;
		nanosleep(&ts3, NULL);
#else
		if(!sem_wait(s)) {
			setitimer(ITIMER_REAL, &otv, NULL);
			sigaction(SIGALRM, &osa, NULL);
			return 0;
		}
		getitimer(ITIMER_REAL, &tv);
		if(tv.it_value.tv_sec || tv.it_value.tv_usec > 999)
			continue;
		setitimer(ITIMER_REAL, &otv, NULL);
		sigaction(SIGALRM, &osa, NULL);
		break;
#endif
	}
	return WAIT_TIMEOUT;
}
#endif
#endif

#define UPDATE_REG(address, value) WRITE16LE(((u16 *)&ioMem[address]),value)

int linktime = 0;

GBASockClient* dol = NULL;
sf::IPAddress joybusHostAddr = sf::IPAddress::LocalHost;

// Hodgepodge
u8 tspeed = 3;
u8 transfer = 0;
LINKDATA *linkmem = NULL;
int linkid = 0, vbaid = 0;
#ifdef __WIN32__
HANDLE linksync[4];
#else
sem_t *linksync[4];
#endif
int savedlinktime = 0;
#ifdef __WIN32__
HANDLE mmf = NULL;
#else
int mmf = -1;
#endif
char linkevent[] =
#ifndef __WIN32__
	"/"
#endif
	"VBA link event  ";
static int i, j;
int linktimeout = 1000;
LANLINKDATA lanlink;
u16 linkdata[4];
lserver ls;
lclient lc;
bool oncewait = false, after = false;

// RFU crap (except for numtransfers note...should probably check that out)
bool rfu_enabled = false;
u8 rfu_cmd, rfu_qsend, rfu_qrecv;
int rfu_state, rfu_polarity, rfu_counter, rfu_masterq;
// numtransfers seems to be used interchangeably with linkmem->numtransfers
// in rfu code; probably a bug?
int rfu_transfer_end;
// in local comm, setting this keeps slaves from trying to communicate even
// when master isn't
u16 numtransfers = 0;
u32 rfu_masterdata[32];

// time to end of single GBA's transfer, in 16.78 MHz clock ticks
// first index is GBA #
int trtimedata[4][4] = {
      // 9600 38400 57600 115200
	{34080, 8520, 5680, 2840},
	{65536, 16384, 10923, 5461},
	{99609, 24903, 16602, 8301},
	{133692, 33423, 22282, 11141}
};

// time to end of transfer
// for 3 slaves, this is time to transfer machine 4
// for < 3 slaves, this is time to transfer last machine + time to detect lack
// of start bit from next slave
// first index is (# of slaves) - 1
int trtimeend[3][4] = {
      // 9600 38400 57600 115200
	{72527, 18132, 12088, 6044},
	{106608, 26652, 17768, 8884},
	{133692, 33423, 22282, 11141}
};

int gbtime = 1024;

int GetSIOMode(u16, u16);

void LinkClientThread(void *);
void LinkServerThread(void *);

int StartServer(void);

u16 StartRFU(u16);

const char *MakeInstanceFilename(const char *Input)
{
	if (vbaid == 0)
		return Input;

	static char *result=NULL;
	if (result!=NULL)
		free(result);

	result = (char *)malloc(strlen(Input)+3);
	char *p = strrchr((char *)Input, '.');
	sprintf(result, "%.*s-%d.%s", (int)(p-Input), Input, vbaid+1, p+1);
	return result;
}

void StartLink(u16 value)
{
	if (ioMem == NULL)
		return;

	if (rfu_enabled) {
		UPDATE_REG(COMM_SIOCNT, StartRFU(value));
		return;
	}

	switch (GetSIOMode(value, READ16LE(&ioMem[COMM_RCNT]))) {
	case MULTIPLAYER: {
		bool start = (value & 0x80) && !linkid && !transfer && gba_link_enabled;
		u16 si = value & 4;
		// clear start, seqno, si (RO on slave, start = pulse on master)
		value &= 0xff4b;
		// get current si.  This way, on slaves, it is low during xfer
		if(linkid) {
			if(!transfer)
				value |= 4;
			else
				value |= READ16LE(&ioMem[COMM_SIOCNT]) & 4;
		}
		if (start) {
			if (lanlink.active)
			{
				if (lanlink.connected)
				{
					linkdata[0] = READ16LE(&ioMem[COMM_SIODATA8]);
					savedlinktime = linktime;
					tspeed = value & 3;
					ls.Send();
					transfer = 1;
					linktime = 0;
					UPDATE_REG(COMM_SIOMULTI0, linkdata[0]);
					UPDATE_REG(COMM_SIOMULTI1, 0xffff);
					WRITE32LE(&ioMem[COMM_SIOMULTI2], 0xffffffff);
					if (lanlink.speed&&oncewait == false)
						ls.howmanytimes++;
					after = false;
					value &= ~0x40;
				} else
					value |= 0x40; // comm error
			}
			else if (linkmem->numgbas > 1)
			{
				// find first active attached GBA
				// doing this first reduces the potential
				// race window size for new connections
				int n = linkmem->numgbas + 1;
				int f = linkmem->linkflags;
				int m;
				do {
					n--;
					m = (1 << n) - 1;
				} while((f & m) != m);
				linkmem->trgbas = n;

				// before starting xfer, make pathetic attempt
				// at clearing out any previous stuck xfer
				// this will fail if a slave was stuck for
				// too long
				for(int i = 0; i < 4; i++)
					while(WaitForSingleObject(linksync[i], 0) != WAIT_TIMEOUT);

				// transmit first value
				linkmem->linkcmd = ('M' << 8) + (value & 3);
				linkmem->linkdata[0] = READ16LE(&ioMem[COMM_SIODATA8]);

				// start up slaves & sync clocks
				numtransfers = linkmem->numtransfers;
				if (numtransfers != 0)
					linkmem->lastlinktime = linktime;
				else
					linkmem->lastlinktime = 0;

				if ((++numtransfers) == 0)
					linkmem->numtransfers = 2;
				else
					linkmem->numtransfers = numtransfers;

				transfer = 1;
				linktime = 0;
				tspeed = value & 3;
				WRITE32LE(&ioMem[COMM_SIOMULTI0], 0xffffffff);
				WRITE32LE(&ioMem[COMM_SIOMULTI2], 0xffffffff);
				value &= ~0x40;
			}
		}
		value |= (transfer != 0) << 7;
		value |= (linkid && !transfer ? 0xc : 8); // set SD (high), SI (low on master)
		value |= linkid << 4; // set seq
		UPDATE_REG(COMM_SIOCNT, value);
		if (linkid)
			// SC low -> transfer in progress
			// not sure why SO is low
			UPDATE_REG(COMM_RCNT, transfer ? 6 : 7);
		else
			// SI is always low on master
			// SO, SC always low during transfer
			// not sure why SO low otherwise
			UPDATE_REG(COMM_RCNT, transfer ? 2 : 3);
		break;
	}
	case NORMAL8:
	case NORMAL32:
	case UART:
	default:
		UPDATE_REG(COMM_SIOCNT, value);
		break;
	}
}

void StartGPLink(u16 value)
{
	UPDATE_REG(COMM_RCNT, value);

	if (!value)
		return;

	switch (GetSIOMode(READ16LE(&ioMem[COMM_SIOCNT]), value)) {
	case MULTIPLAYER:
		value &= 0xc0f0;
		value |= 3;
		if (linkid)
			value |= 4;
		UPDATE_REG(COMM_SIOCNT, ((READ16LE(&ioMem[COMM_SIOCNT])&0xff8b)|(linkid ? 0xc : 8)|(linkid<<4)));
		break;

	case GP:
		if (rfu_enabled)
			rfu_state = RFU_INIT;
		break;
	}
}

void JoyBusConnect()
{
	delete dol;
	dol = NULL;

	dol = new GBASockClient(joybusHostAddr);
}

void JoyBusShutdown()
{
	delete dol;
	dol = NULL;
}

void JoyBusUpdate(int ticks)
{
	linktime += ticks;
	static int lastjoybusupdate = 0;

	// Kinda ugly hack to update joybus stuff intermittently
	if (linktime > lastjoybusupdate + 0x3000)
	{
		lastjoybusupdate = linktime;

		char data[5] = {0x10, 0, 0, 0, 0}; // init with invalid cmd
		std::vector<char> resp;

		if (!dol)
			JoyBusConnect();

		u8 cmd = dol->ReceiveCmd(data);
		switch (cmd) {
		case JOY_CMD_RESET:
			UPDATE_REG(COMM_JOYCNT, READ16LE(&ioMem[COMM_JOYCNT]) | JOYCNT_RESET);

		case JOY_CMD_STATUS:
			resp.push_back(0x00); // GBA device ID
			resp.push_back(0x04);
			break;
		
		case JOY_CMD_READ:
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_L]) & 0xff));
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_L]) >> 8));
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_H]) & 0xff));
			resp.push_back((u8)(READ16LE(&ioMem[COMM_JOY_TRANS_H]) >> 8));
			UPDATE_REG(COMM_JOYSTAT, READ16LE(&ioMem[COMM_JOYSTAT]) & ~JOYSTAT_SEND);
			UPDATE_REG(COMM_JOYCNT, READ16LE(&ioMem[COMM_JOYCNT]) | JOYCNT_SEND_COMPLETE);
			break;

		case JOY_CMD_WRITE:
			UPDATE_REG(COMM_JOY_RECV_L, (u16)((u16)data[2] << 8) | (u8)data[1]);
			UPDATE_REG(COMM_JOY_RECV_H, (u16)((u16)data[4] << 8) | (u8)data[3]);
			UPDATE_REG(COMM_JOYSTAT, READ16LE(&ioMem[COMM_JOYSTAT]) | JOYSTAT_RECV);
			UPDATE_REG(COMM_JOYCNT, READ16LE(&ioMem[COMM_JOYCNT]) | JOYCNT_RECV_COMPLETE);
			break;

		default:
			return; // ignore
		}

		resp.push_back((u8)READ16LE(&ioMem[COMM_JOYSTAT]));
		dol->Send(resp);

		// Generate SIO interrupt if we can
		if ( ((cmd == JOY_CMD_RESET) || (cmd == JOY_CMD_READ) || (cmd == JOY_CMD_WRITE))
			&& (READ16LE(&ioMem[COMM_JOYCNT]) & JOYCNT_INT_ENABLE) )
		{
			IF |= 0x80;
			UPDATE_REG(0x202, IF);
		}
	}
}

static void ReInitLink();

void LinkUpdate(int ticks)
{
	// this actually gets called every single instruction, so keep default
	// path as short as possible

	linktime += ticks;

	if (rfu_enabled)
	{
		rfu_transfer_end -= ticks;
		if (transfer && rfu_transfer_end <= 0) 
		{
			transfer = 0;
			if (READ16LE(&ioMem[COMM_SIOCNT]) & 0x4000)
			{
				IF |= 0x80;
				UPDATE_REG(0x202, IF);
			}
			UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) & 0xff7f);
		}
		return;
	}

	if (lanlink.active)
	{
		if (lanlink.connected)
		{
			if (after)
			{
				if (linkid && linktime > 6044) {
					lc.Recv();
					oncewait = true;
				}
				else
					return;
			}

			if (linkid && !transfer && lc.numtransfers > 0 && linktime >= savedlinktime)
			{
				linkdata[linkid] = READ16LE(&ioMem[COMM_SIODATA8]);

				if (!lc.oncesend)
					lc.Send();

				lc.oncesend = false;
				UPDATE_REG(COMM_SIODATA32_L, linkdata[0]);
				UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) | 0x80);
				transfer = 1;
				if (lc.numtransfers==1)
					linktime = 0;
				else
					linktime -= savedlinktime;
			}

			if (transfer && linktime >= trtimeend[lanlink.numslaves-1][tspeed])
			{
				if (READ16LE(&ioMem[COMM_SIOCNT]) & 0x4000)
				{
					IF |= 0x80;
					UPDATE_REG(0x202, IF);
				}

				UPDATE_REG(COMM_SIOCNT, (READ16LE(&ioMem[COMM_SIOCNT]) & 0xff0f) | (linkid << 4));
				transfer = 0;
				linktime -= trtimeend[lanlink.numslaves-1][tspeed];
				oncewait = false;

				if (!lanlink.speed)
				{
					if (linkid)
						lc.Recv();
					else
						ls.Recv(); // WTF is the point of this?

					UPDATE_REG(COMM_SIOMULTI1, linkdata[1]);
					UPDATE_REG(COMM_SIOMULTI2, linkdata[2]);
					UPDATE_REG(COMM_SIOMULTI3, linkdata[3]);
					oncewait = true;

				} else {

					after = true;
					if (lanlink.numslaves == 1)
					{
						UPDATE_REG(COMM_SIOMULTI1, linkdata[1]);
						UPDATE_REG(COMM_SIOMULTI2, linkdata[2]);
						UPDATE_REG(COMM_SIOMULTI3, linkdata[3]);
					}
				}
			}
		}
		return;
	}

	// slave startup depends on detecting change in numtransfers
	// and syncing clock with master (after first transfer)
	// this will fail if > ~2 minutes have passed since last transfer due
	// to integer overflow
	if(!transfer && numtransfers && linktime < 0) {
		linktime = 0;
		// there is a very, very, small chance that this will abort
		// a transfer that was just started
		linkmem->numtransfers = numtransfers = 0;
	}
	if (linkid && !transfer && linktime >= linkmem->lastlinktime &&
	    linkmem->numtransfers != numtransfers)
	{
		numtransfers = linkmem->numtransfers;
		if(!numtransfers)
			return;

		// if this or any previous machine was dropped, no transfer
		// can take place
		if(linkmem->trgbas <= linkid) {
			transfer = 0;
			numtransfers = 0;
			// if this is the one that was dropped, reconnect
			if(!(linkmem->linkflags & (1 << linkid)))
				ReInitLink();
			return;
		}

		// sync clock
		if (numtransfers == 1)
			linktime = 0;
		else
			linktime -= linkmem->lastlinktime;

		// there's really no point to this switch; 'M' is the only
		// possible command.
#if 0
		switch ((linkmem->linkcmd) >> 8)
		{
		case 'M':
#endif
			tspeed = linkmem->linkcmd & 3;
			transfer = 1;
			WRITE32LE(&ioMem[COMM_SIOMULTI0], 0xffffffff);
			WRITE32LE(&ioMem[COMM_SIOMULTI2], 0xffffffff);
			UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) & ~0x40 | 0x80);
#if 0
			break;
		}
#endif
	}

	if (!transfer)
		return;

	if (transfer <= linkmem->trgbas && linktime >= trtimedata[transfer-1][tspeed])
	{
		// transfer #n -> wait for value n - 1
		if(transfer > 1 && linkid != transfer - 1) {
			if(WaitForSingleObject(linksync[transfer - 1], linktimeout) == WAIT_TIMEOUT) {
				// assume slave has dropped off if timed out
				if(!linkid) {
					linkmem->trgbas = transfer - 1;
					int f = linkmem->linkflags;
					f &= ~(1 << (transfer - 1));
					linkmem->linkflags = f;
					if(f < (1 << transfer) - 1)
						linkmem->numgbas = transfer - 1;
					char message[30];
					sprintf(message, _("Player %d disconnected."), transfer - 1);
					systemScreenMessage(message);
				}
				transfer = linkmem->trgbas + 1;
				// next cycle, transfer will finish up
				return;
			}
		}
		// now that value is available, store it
		UPDATE_REG((COMM_SIOMULTI0 - 2) + (transfer<<1), linkmem->linkdata[transfer-1]);

		// transfer machine's value at start of its transfer cycle
		if(linkid == transfer) {
			// skip if dropped
			if(linkmem->trgbas <= linkid) {
				transfer = 0;
				numtransfers = 0;
				// if this is the one that was dropped, reconnect
				if(!(linkmem->linkflags & (1 << linkid)))
					ReInitLink();
				return;
			}
			// SI becomes low
			UPDATE_REG(COMM_SIOCNT, READ16LE(&ioMem[COMM_SIOCNT]) & ~4);
			UPDATE_REG(COMM_RCNT, 10);
			linkmem->linkdata[linkid] = READ16LE(&ioMem[COMM_SIODATA8]);
			ReleaseSemaphore(linksync[linkid], linkmem->numgbas-1, NULL);
		}
		if(linkid == transfer - 1) {
			// SO becomes low to begin next trasnfer
			// may need to set DDR as well
			UPDATE_REG(COMM_RCNT, 0x22);
		}

		// next cycle
		transfer++;
	}

	if (transfer > linkmem->trgbas && linktime >= trtimeend[transfer-3][tspeed])
	{
		// wait for slaves to finish
		// this keeps unfinished slaves from screwing up last xfer
		// not strictly necessary; may just slow things down
		if(!linkid) {
			for(int i = 2; i < transfer; i++)
				if(WaitForSingleObject(linksync[0], linktimeout) == WAIT_TIMEOUT) {
					// impossible to determine which slave died
					// so leave them alone for now
					systemScreenMessage(_("Unknown slave timed out; resetting comm"));
					linkmem->numtransfers = numtransfers = 0;
					break;
				}
		} else if(linkmem->trgbas > linkid)
			// signal master that this slave is finished
			ReleaseSemaphore(linksync[0], 1, NULL);
		linktime -= trtimeend[transfer - 3][tspeed];
		transfer = 0;
		u16 value = READ16LE(&ioMem[COMM_SIOCNT]);
		if(!linkid)
			value |= 4; // SI becomes high on slaves after xfer
		UPDATE_REG(COMM_SIOCNT, (value & 0xff0f) | (linkid << 4));
		// SC/SI high after transfer
		UPDATE_REG(COMM_RCNT, linkid ? 15 : 11);
		if (value & 0x4000)
		{
			IF |= 0x80;
			UPDATE_REG(0x202, IF);
		}
	}

	return;
}

inline int GetSIOMode(u16 siocnt, u16 rcnt)
{
	if (!(rcnt & 0x8000))
	{
		switch (siocnt & 0x3000) {
		case 0x0000: return NORMAL8;
		case 0x1000: return NORMAL32;
		case 0x2000: return MULTIPLAYER;
		case 0x3000: return UART;
		}
	}

	if (rcnt & 0x4000)
		return JOYBUS;

	return GP;
}

// The GBA wireless RFU (see adapter3.txt)
// Just try to avert your eyes for now ^^ (note, it currently can be called, tho)
u16 StartRFU(u16 value)
{
	switch (GetSIOMode(value, READ16LE(&ioMem[COMM_RCNT]))) {
	case NORMAL8:
		rfu_polarity = 0;
		return value;
		break;

	case NORMAL32:
		if (value & 8)
			value &= 0xfffb;	// A kind of acknowledge procedure
		else
			value |= 4;

		if (value & 0x80)
		{
			if ((value&3) == 1)
				rfu_transfer_end = 2048;
			else
				rfu_transfer_end = 256;

			u16 a = READ16LE(&ioMem[COMM_SIODATA32_H]);

			switch (rfu_state) {
			case RFU_INIT:
				if (READ32LE(&ioMem[COMM_SIODATA32_L]) == 0xb0bb8001)
					rfu_state = RFU_COMM;	// end of startup

				UPDATE_REG(COMM_SIODATA32_H, READ16LE(&ioMem[COMM_SIODATA32_L]));
				UPDATE_REG(COMM_SIODATA32_L, a);
				break;

			case RFU_COMM:
				if (a == 0x9966)
				{
					rfu_cmd = ioMem[COMM_SIODATA32_L];
					if ((rfu_qsend=ioMem[0x121]) != 0) {
						rfu_state = RFU_SEND;
						rfu_counter = 0;
					}
					if (rfu_cmd == 0x25 || rfu_cmd == 0x24) {
						linkmem->rfu_q[vbaid] = rfu_qsend;
					}
					UPDATE_REG(COMM_SIODATA32_L, 0);
					UPDATE_REG(COMM_SIODATA32_H, 0x8000);
				}
				else if (a == 0x8000)
				{
					switch (rfu_cmd) {
					case 0x1a:	// check if someone joined
						if (linkmem->rfu_request[vbaid] != 0) {
							rfu_state = RFU_RECV;
							rfu_qrecv = 1;
						}
						linkid = -1;
						rfu_cmd |= 0x80;
						break;

					case 0x1e:	// receive broadcast data
					case 0x1d:	// no visible difference
						rfu_polarity = 0;
						rfu_state = RFU_RECV;
						rfu_qrecv = 7;
						rfu_counter = 0;
						rfu_cmd |= 0x80;
						break;

					case 0x30:
						linkmem->rfu_request[vbaid] = 0;
						linkmem->rfu_q[vbaid] = 0;
						linkid = 0;
						numtransfers = 0;
						rfu_cmd |= 0x80;
						if (linkmem->numgbas == 2)
							ReleaseSemaphore(linksync[1-vbaid], 1, NULL);
						break;

					case 0x11:	// ? always receives 0xff - I suspect it's something for 3+ players
					case 0x13:	// unknown
					case 0x20:	// this has something to do with 0x1f
					case 0x21:	// this too
						rfu_cmd |= 0x80;
						rfu_polarity = 0;
						rfu_state = 3;
						rfu_qrecv = 1;
						break;

					case 0x26:
						if(linkid>0){
							rfu_qrecv = rfu_masterq;
						}
						if((rfu_qrecv=linkmem->rfu_q[1-vbaid])!=0){
							rfu_state = RFU_RECV;
							rfu_counter = 0;
						}
						rfu_cmd |= 0x80;
						break;

					case 0x24:	// send data
						if((numtransfers++)==0) linktime = 1;
						linkmem->rfu_linktime[vbaid] = linktime;
						if(linkmem->numgbas==2){
							ReleaseSemaphore(linksync[1-vbaid], 1, NULL);
							WaitForSingleObject(linksync[vbaid], linktimeout);
						}
						rfu_cmd |= 0x80;
						linktime = 0;
						linkid = -1;
						break;

					case 0x25:	// send & wait for data
					case 0x1f:	// pick a server
					case 0x10:	// init
					case 0x16:	// send broadcast data
					case 0x17:	// setup or something ?
					case 0x27:	// wait for data ?
					case 0x3d:	// init
					default:
						rfu_cmd |= 0x80;
						break;

					case 0xa5:	//	2nd part of send&wait function 0x25
					case 0xa7:	//	2nd part of wait function 0x27
						if (linkid == -1) {
							linkid++;
							linkmem->rfu_linktime[vbaid] = 0;
						}
						if (linkid&&linkmem->rfu_request[1-vbaid] == 0) {
							linkmem->rfu_q[1-vbaid] = 0;
							rfu_transfer_end = 256;
							rfu_polarity = 1;
							rfu_cmd = 0x29;
							linktime = 0;
							break;
						}
						if ((numtransfers++) == 0)
							linktime = 0;
						linkmem->rfu_linktime[vbaid] = linktime;
						if (linkmem->numgbas == 2) {
							if (!linkid || (linkid && numtransfers))
								ReleaseSemaphore(linksync[1-vbaid], 1, NULL);
							WaitForSingleObject(linksync[vbaid], linktimeout);
						}
						if ( linkid > 0) {
							memcpy(rfu_masterdata, linkmem->rfu_data[1-vbaid], 128);
							rfu_masterq = linkmem->rfu_q[1-vbaid];
						}
						rfu_transfer_end = linkmem->rfu_linktime[1-vbaid] - linktime + 256;
						
						if (rfu_transfer_end < 256)
							rfu_transfer_end = 256;

						linktime = -rfu_transfer_end;
						rfu_polarity = 1;
						rfu_cmd = 0x28;
						break;
					}
					UPDATE_REG(COMM_SIODATA32_H, 0x9966);
					UPDATE_REG(COMM_SIODATA32_L, (rfu_qrecv<<8) | rfu_cmd);

				} else {

					UPDATE_REG(COMM_SIODATA32_L, 0);
					UPDATE_REG(COMM_SIODATA32_H, 0x8000);
				}
				break;

			case RFU_SEND:
				if(--rfu_qsend == 0)
					rfu_state = RFU_COMM;

				switch (rfu_cmd) {
				case 0x16:
					linkmem->rfu_bdata[vbaid][rfu_counter++] = READ32LE(&ioMem[COMM_SIODATA32_L]);
					break;

				case 0x17:
					linkid = 1;
					break;

				case 0x1f:
					linkmem->rfu_request[1-vbaid] = 1;
					break;

				case 0x24:
				case 0x25:
					linkmem->rfu_data[vbaid][rfu_counter++] = READ32LE(&ioMem[COMM_SIODATA32_L]);
					break;
				}
				UPDATE_REG(COMM_SIODATA32_L, 0);
				UPDATE_REG(COMM_SIODATA32_H, 0x8000);
				break;

			case RFU_RECV:
				if (--rfu_qrecv == 0)
					rfu_state = RFU_COMM;

				switch (rfu_cmd) {
				case 0x9d:
				case 0x9e:
					if (rfu_counter == 0) {
						UPDATE_REG(COMM_SIODATA32_L, 0x61f1);
						UPDATE_REG(COMM_SIODATA32_H, 0);
						rfu_counter++;
						break;
					}
					UPDATE_REG(COMM_SIODATA32_L, linkmem->rfu_bdata[1-vbaid][rfu_counter-1]&0xffff);
					UPDATE_REG(COMM_SIODATA32_H, linkmem->rfu_bdata[1-vbaid][rfu_counter-1]>>16);
					rfu_counter++;
					break;

			case 0xa6:
				if (linkid>0) {
					UPDATE_REG(COMM_SIODATA32_L, rfu_masterdata[rfu_counter]&0xffff);
					UPDATE_REG(COMM_SIODATA32_H, rfu_masterdata[rfu_counter++]>>16);
				} else {
					UPDATE_REG(COMM_SIODATA32_L, linkmem->rfu_data[1-vbaid][rfu_counter]&0xffff);
					UPDATE_REG(COMM_SIODATA32_H, linkmem->rfu_data[1-vbaid][rfu_counter++]>>16);
				}
				break;

			case 0x93:	// it seems like the game doesn't care about this value
				UPDATE_REG(COMM_SIODATA32_L, 0x1234);	// put anything in here
				UPDATE_REG(COMM_SIODATA32_H, 0x0200);	// also here, but it should be 0200
				break;

			case 0xa0:
			case 0xa1:
				UPDATE_REG(COMM_SIODATA32_L, 0x641b);
				UPDATE_REG(COMM_SIODATA32_H, 0x0000);
				break;

			case 0x9a:
				UPDATE_REG(COMM_SIODATA32_L, 0x61f9);
				UPDATE_REG(COMM_SIODATA32_H, 0);
				break;

			case 0x91:
				UPDATE_REG(COMM_SIODATA32_L, 0x00ff);
				UPDATE_REG(COMM_SIODATA32_H, 0x0000);
				break;

			default:
				UPDATE_REG(COMM_SIODATA32_L, 0x0173);
				UPDATE_REG(COMM_SIODATA32_H, 0x0000);
				break;
			}
			break;
		}
		transfer = 1;
	}

	if (rfu_polarity)
		value ^= 4;	// sometimes it's the other way around

	default:
		return value;
	}
}

//////////////////////////////////////////////////////////////////////////
// Probably from here down needs to be replaced with SFML goodness :)
// tjm: what SFML goodness?  SFML for network, yes, but not for IPC

bool InitLink()
{
	linkid = 0;

#ifdef __WIN32__
	if((mmf=CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LINKDATA), LOCAL_LINK_NAME))==NULL){
		systemMessage(0, N_("Error creating file mapping"));
		return false;
	}

	if(GetLastError() == ERROR_ALREADY_EXISTS)
		vbaid = 1;
	else
		vbaid = 0;


	if((linkmem=(LINKDATA *)MapViewOfFile(mmf, FILE_MAP_WRITE, 0, 0, sizeof(LINKDATA)))==NULL){
		CloseHandle(mmf);
		systemMessage(0, N_("Error mapping file"));
		return false;
	}
#else
	if((mmf = shm_open("/" LOCAL_LINK_NAME, O_RDWR|O_CREAT|O_EXCL, 0777)) < 0) {
		vbaid = 1;
		mmf = shm_open("/" LOCAL_LINK_NAME, O_RDWR, 0);
	} else
		vbaid = 0;
	if(mmf < 0 || ftruncate(mmf, sizeof(LINKDATA)) < 0 ||
	   !(linkmem = (LINKDATA *)mmap(NULL, sizeof(LINKDATA),
					PROT_READ|PROT_WRITE, MAP_SHARED,
					mmf, 0))) {
		systemMessage(0, N_("Error creating file mapping"));
		if(mmf) {
			if(!vbaid)
				shm_unlink("/" LOCAL_LINK_NAME);
			close(mmf);
		}
	}
#endif

	// get lowest-numbered available machine slot
	bool firstone = !vbaid;
	if(firstone) {
		linkmem->linkflags = 1;
		linkmem->numgbas = 1;
		linkmem->numtransfers=0;
		for(i=0;i<4;i++)
			linkmem->linkdata[i] = 0xffff;
	} else {
		// FIXME: this should be done while linkmem is locked
		// (no xfer in progress, no other vba trying to connect)
		int n = linkmem->numgbas;
		int f = linkmem->linkflags;
		for(int i = 0; i <= n; i++)
			if(!(f & (1 << i))) {
				vbaid = i;
				break;
			}
		if(vbaid == 4){
#ifdef __WIN32__
			UnmapViewOfFile(linkmem);
			CloseHandle(mmf);
#else
			munmap(linkmem, sizeof(LINKDATA));
			if(!vbaid)
				shm_unlink("/" LOCAL_LINK_NAME);
			close(mmf);
#endif
			systemMessage(0, N_("5 or more GBAs not supported."));
			return false;
		}
		if(vbaid == n)
			linkmem->numgbas = n + 1;
		linkmem->linkflags = f | (1 << vbaid);
	}
	linkid = vbaid;

	for(i=0;i<4;i++){
		linkevent[sizeof(linkevent)-2]=(char)i+'1';
#ifdef __WIN32__
		linksync[i] = firstone ?
			CreateSemaphore(NULL, 0, 4, linkevent) :
			OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, linkevent);
		if(linksync[i] == NULL) {
			UnmapViewOfFile(linkmem);
			CloseHandle(mmf);
			for(j=0;j<i;j++){
				CloseHandle(linksync[j]);
			}
			systemMessage(0, N_("Error opening event"));
			return false;
		}
#else
		if((linksync[i] = sem_open(linkevent,
					   firstone ? O_CREAT|O_EXCL : 0,
					   0777, 0)) == SEM_FAILED) {
			if(firstone)
				shm_unlink("/" LOCAL_LINK_NAME);
			munmap(linkmem, sizeof(LINKDATA));
			close(mmf);
			for(j=0;j<i;j++){
				sem_close(linksync[i]);
				if(firstone) {
					linkevent[sizeof(linkevent)-2]=(char)i+'1';
					sem_unlink(linkevent);
				}
			}
			systemMessage(0, N_("Error opening event"));
			return false;
		}
#endif
	}
	for(i=0;i<4;i++)
		linkdata[i] = 0xffff;
	return true;
}

static void ReInitLink()
{
	int f = linkmem->linkflags;
	int n = linkmem->numgbas;
	if(f & (1 << linkid)) {
		systemMessage(0, N_("Lost link; reinitialize to reconnect"));
		return;
	}
	linkmem->linkflags |= 1 << linkid;
	if(n < linkid + 1)
		linkmem->numgbas = linkid + 1;
	numtransfers = linkmem->numtransfers;
	systemScreenMessage(_("Lost link; reconnected"));
}

void CloseLink(void){
	if(lanlink.connected){
		if(linkid){
			char outbuffer[4];
			outbuffer[0] = 4;
			outbuffer[1] = -32;
			if(lanlink.type==0) lanlink.tcpsocket.Send(outbuffer, 4);
		} else {
			char outbuffer[12];
			int i;
			outbuffer[0] = 12;
			outbuffer[1] = -32;
			for(i=1;i<=lanlink.numslaves;i++){
				if(lanlink.type==0){
					ls.tcpsocket[i].Send(outbuffer, 12);
				}
				ls.tcpsocket[i].Close();
			}
		}
	}
	int f = linkmem->linkflags;
	f &= ~(1 << linkid);
	if(f & 0xf) {
		linkmem->linkflags = f;
		int n = linkmem->numgbas;
		for(int i = 0; i < n; i--)
			if(f <= (1 << (i + 1)) - 1) {
				linkmem->numgbas = i + 1;
				break;
			}
	}

	for(i=0;i<4;i++){
		if(linksync[i]!=NULL){
#ifdef __WIN32__
			ReleaseSemaphore(linksync[i], 1, NULL);
			CloseHandle(linksync[i]);
#else
			sem_close(linksync[i]);
			if(!(f & 0xf)) {
				linkevent[sizeof(linkevent)-2]=(char)i+'1';
				sem_unlink(linkevent);
			}
#endif
		}
	}
#ifdef __WIN32__
	CloseHandle(mmf);
	UnmapViewOfFile(linkmem);

	// FIXME: move to caller
	// (but there are no callers, so why bother?)
	//regSetDwordValue("LAN", lanlink.active);
#else
	if(!(f & 0xf))
		shm_unlink("/" LOCAL_LINK_NAME);
	munmap(linkmem, sizeof(LINKDATA));
	close(mmf);
#endif
	return;
}

// call this to clean up crashed program's shared state
// or to use TCP on same machine (for testing)
// this may be necessary under MSW as well, but I wouldn't know how
void CleanLocalLink()
{
#ifndef __WIN32__
	shm_unlink("/" LOCAL_LINK_NAME);
	for(int i = 0; i < 4; i++) {
		linkevent[sizeof(linkevent) - 2] = '1' + i;
		sem_unlink(linkevent);
	}
#endif
}

// Server
lserver::lserver(void){
	intinbuffer = (s32*)inbuffer;
	u16inbuffer = (u16*)inbuffer;
	intoutbuffer = (s32*)outbuffer;
	u16outbuffer = (u16*)outbuffer;
	oncewait = false;
}

bool lserver::Init(ServerInfoDisplay *sid){
	// too bad Listen() doesn't take an address as well
	// then again, old code used INADDR_ANY anyway
	if(!lanlink.tcpsocket.Listen(IP_LINK_PORT))
		// Note: old code closed socket & retried once on bind failure
		return false; // FIXME: error code?

	if(lanlink.thread!=NULL){
		lanlink.terminate = true;
		WaitForSingleObject(linksync[vbaid], 500);
		lanlink.thread = NULL;
	}
	lanlink.terminate = false;
	linkid = 0;

	// should probably use GetPublicAddress()
	sid->ShowServerIP(sf::IPAddress::GetLocalAddress());

	lanlink.thread = new sf::Thread(LinkServerThread, sid);
	lanlink.thread->Launch();

	return true;

}

void LinkServerThread(void *_sid){
	ServerInfoDisplay *sid = (ServerInfoDisplay *)_sid;
	sf::Selector<sf::SocketTCP> fdset;
	char inbuffer[256], outbuffer[256];
	s32 *intinbuffer = (s32*)inbuffer;
	u16 *u16inbuffer = (u16*)inbuffer;
	s32 *intoutbuffer = (s32*)outbuffer;
	u16 *u16outbuffer = (u16*)outbuffer;

	i = 0;

	while(i<lanlink.numslaves){
		fdset.Clear();
		fdset.Add(lanlink.tcpsocket);
		if(lanlink.terminate){
			ReleaseSemaphore(linksync[vbaid], 1, NULL);
			return;
		}
		if(fdset.Wait(0.1)==1){
			sf::Socket::Status st =
				lanlink.tcpsocket.Accept(ls.tcpsocket[i+1]);
			if(st == sf::Socket::Error) {
				for(int j=1;j<i;j++) ls.tcpsocket[j].Close();
				systemMessage(0, N_("Network error."));
				lanlink.terminate = true;
			} else {
				i++;
				WRITE16LE(&u16outbuffer[0], i);
				WRITE16LE(&u16outbuffer[1], lanlink.numslaves);
				ls.tcpsocket[i].Send(outbuffer, 4);
				sid->ShowConnect(i);
			}
		}
		sid->Ping();
	}

	lanlink.connected = true;

	sid->Connected();

	for(i=1;i<=lanlink.numslaves;i++){
		outbuffer[0] = 4;
		ls.tcpsocket[i].Send(outbuffer, 4);
	}

	return;
}

void lserver::Send(void){
	if(lanlink.type==0){	// TCP
		if(savedlinktime==-1){
			outbuffer[0] = 4;
			outbuffer[1] = -32;	//0xe0
			for(i=1;i<=lanlink.numslaves;i++){
				tcpsocket[i].Send(outbuffer, 4);
				size_t nr;
				tcpsocket[i].Receive(inbuffer, 4, nr);
			}
		}
		outbuffer[1] = tspeed;
		WRITE16LE(&u16outbuffer[1], linkdata[0]);
		WRITE32LE(&intoutbuffer[1], savedlinktime);
		if(lanlink.numslaves==1){
			if(lanlink.type==0){
				outbuffer[0] = 8;
				tcpsocket[1].Send(outbuffer, 8);
			}
		}
		else if(lanlink.numslaves==2){
			WRITE16LE(&u16outbuffer[4], linkdata[2]);
			if(lanlink.type==0){
				outbuffer[0] = 10;
				tcpsocket[1].Send(outbuffer, 10);
				WRITE16LE(&u16outbuffer[4], linkdata[1]);
				tcpsocket[2].Send(outbuffer, 10);
			}
		} else {
			if(lanlink.type==0){
				outbuffer[0] = 12;
				WRITE16LE(&u16outbuffer[4], linkdata[2]);
				WRITE16LE(&u16outbuffer[5], linkdata[3]);
				tcpsocket[1].Send(outbuffer, 12);
				WRITE16LE(&u16outbuffer[4], linkdata[1]);
				tcpsocket[2].Send(outbuffer, 12);
				WRITE16LE(&u16outbuffer[5], linkdata[2]);
				tcpsocket[3].Send(outbuffer, 12);
			}
		}
	}
	return;
}

void lserver::Recv(void){
	int numbytes;
	if(lanlink.type==0){	// TCP
		fdset.Clear();
		for(i=0;i<lanlink.numslaves;i++) fdset.Add(tcpsocket[i+1]);
		// was linktimeout/1000 (i.e., drop ms part), but that's wrong
		if(fdset.Wait((float)linktimeout / 1000.0)==0){
			return;
		}
		howmanytimes++;
		for(i=0;i<lanlink.numslaves;i++){
			numbytes = 0;
			inbuffer[0] = 1;
			while(numbytes<howmanytimes*inbuffer[0]) {
				size_t nr;
				tcpsocket[i+1].Receive(inbuffer+numbytes, howmanytimes*inbuffer[0]-numbytes, nr);
				numbytes += nr;
			}
			if(howmanytimes>1) memmove(inbuffer, inbuffer+inbuffer[0]*(howmanytimes-1), inbuffer[0]);
			if(inbuffer[1]==-32){
				char message[30];
				lanlink.connected = false;
				sprintf(message, _("Player %d disconnected."), i+2);
				systemScreenMessage(message);
				outbuffer[0] = 4;
				outbuffer[1] = -32;
				for(i=1;i<lanlink.numslaves;i++){
					tcpsocket[i].Send(outbuffer, 12);
					size_t nr;
					tcpsocket[i].Receive(inbuffer, 256, nr);
					tcpsocket[i].Close();
				}
				return;
			}
			linkdata[i+1] = READ16LE(&u16inbuffer[1]);
		}
		howmanytimes = 0;
	}
	after = false;
	return;
}


// Client
lclient::lclient(void){
	intinbuffer = (s32*)inbuffer;
	u16inbuffer = (u16*)inbuffer;
	intoutbuffer = (s32*)outbuffer;
	u16outbuffer = (u16*)outbuffer;
	numtransfers = 0;
	oncesend = false;
	return;
}

bool lclient::Init(sf::IPAddress addr, ClientInfoDisplay *cid){
	serveraddr = addr;
	serverport = IP_LINK_PORT;
	lanlink.tcpsocket.SetBlocking(false);

	if(lanlink.thread!=NULL){
		lanlink.terminate = true;
		WaitForSingleObject(linksync[vbaid], 500);
		lanlink.thread = NULL;
	}

	cid->ConnectStart(addr);
	lanlink.terminate = false;
	lanlink.thread = new sf::Thread(LinkClientThread, cid);
	lanlink.thread->Launch();
	return 0;
}

void LinkClientThread(void *_cid){
	ClientInfoDisplay *cid = (ClientInfoDisplay *)_cid;
	sf::Selector<sf::SocketTCP> fdset;
	int numbytes;
	char inbuffer[16];
	u16 *u16inbuffer = (u16*)inbuffer;
	unsigned long block = 0;

	while(lanlink.tcpsocket.Connect(lc.serverport, lc.serveraddr) != sf::Socket::Done) {
		// stupid SFML has no way of giving what sort of error occurred
		// so we'll just have to do a retry loop, I guess.
		cid->Ping();
		if(lanlink.terminate) return;
		// old code had broken sleep on socket, which isn't
		// even connected yet
		// corrected sleep on socket worked, but this is more sane
		// and probably less portable... works with mingw32 at least
		usleep(100000);
	}

	numbytes = 0;
	size_t got;
	while(numbytes<4) {
		lanlink.tcpsocket.Receive(inbuffer+numbytes, 4 - numbytes, got);
		numbytes += got;
		fdset.Clear();
		fdset.Add(lanlink.tcpsocket);
		fdset.Wait(0.1);
		cid->Ping();
		if(lanlink.terminate) {
			lanlink.tcpsocket.Close();
			return;
		}
	}
	linkid = (int)READ16LE(&u16inbuffer[0]);
	lanlink.numslaves = (int)READ16LE(&u16inbuffer[1]);

	cid->ShowConnect(linkid + 1, lanlink.numslaves - linkid);

	numbytes = 0;
	inbuffer[0] = 1;
	while(numbytes<inbuffer[0]) {
		lanlink.tcpsocket.Receive(inbuffer+numbytes, inbuffer[0] - got, got);
		numbytes += got;
		fdset.Clear();
		fdset.Add(lanlink.tcpsocket);
		fdset.Wait(0.1);
		cid->Ping();
		if(lanlink.terminate) {
			lanlink.tcpsocket.Close();
			return;
		}
	}

	lanlink.connected = true;

	cid->Connected();

	return;
}

void lclient::CheckConn(void){
	size_t nr;
	lanlink.tcpsocket.Receive(inbuffer, 1, nr);
	numbytes = nr;
	if(numbytes>0){
		while(numbytes<inbuffer[0]) {
			lanlink.tcpsocket.Receive(inbuffer+numbytes, inbuffer[0] - numbytes, nr);
			numbytes += nr;
		}
		if(inbuffer[1]==-32){
			outbuffer[0] = 4;
			lanlink.tcpsocket.Send(outbuffer, 4);
			lanlink.connected = false;
			systemScreenMessage(_("Server disconnected."));
			return;
		}
		numtransfers = 1;
		savedlinktime = 0;
		linkdata[0] = READ16LE(&u16inbuffer[1]);
		tspeed = inbuffer[1] & 3;
		for(i=1, numbytes=4;i<=lanlink.numslaves;i++)
			if(i!=linkid) {
				linkdata[i] = READ16LE(&u16inbuffer[numbytes]);
				numbytes++;
			}
		after = false;
		oncewait = true;
		oncesend = true;
	}
	return;
}

void lclient::Recv(void){
	fdset.Clear();
	// old code used socket # instead of mask again
	fdset.Add(lanlink.tcpsocket);
	// old code stripped off ms again
	if(fdset.Wait((float)linktimeout / 1000.0)==0){
		numtransfers = 0;
		return;
	}
	numbytes = 0;
	inbuffer[0] = 1;
	size_t nr;
	while(numbytes<inbuffer[0]) {
		lanlink.tcpsocket.Receive(inbuffer+numbytes, inbuffer[0] - numbytes, nr);
		numbytes += nr;
	}
	if(inbuffer[1]==-32){
		outbuffer[0] = 4;
		lanlink.tcpsocket.Send(outbuffer, 4);
		lanlink.connected = false;
		systemScreenMessage(_("Server disconnected."));
		return;
	}
	tspeed = inbuffer[1] & 3;
	linkdata[0] = READ16LE(&u16inbuffer[1]);
	savedlinktime = (s32)READ32LE(&intinbuffer[1]);
	for(i=1, numbytes=4;i<lanlink.numslaves+1;i++)
		if(i!=linkid) {
			linkdata[i] = READ16LE(&u16inbuffer[numbytes]);
			numbytes++;
		}
	numtransfers++;
	if(numtransfers==0) numtransfers = 2;
	after = false;
}

void lclient::Send(){
	outbuffer[0] = 4;
	outbuffer[1] = linkid<<2;
	WRITE16LE(&u16outbuffer[1], linkdata[linkid]);
	lanlink.tcpsocket.Send(outbuffer, 4);
	return;
}


// Uncalled
void LinkSStop(void){
	if(!oncewait){
		if(linkid){
			if(lanlink.numslaves==1) return;
			lc.Recv();
		}
		else ls.Recv();

		oncewait = true;
		UPDATE_REG(COMM_SIOMULTI1, linkdata[1]);
		UPDATE_REG(COMM_SIOMULTI2, linkdata[2]);
		UPDATE_REG(COMM_SIOMULTI3, linkdata[3]);
	}
	return;
}

// ??? Called when COMM_SIODATA8 written
void LinkSSend(u16 value){
	if(linkid&&!lc.oncesend){
		linkdata[linkid] = value;
		lc.Send();
		lc.oncesend = true;
	}
}
#endif
