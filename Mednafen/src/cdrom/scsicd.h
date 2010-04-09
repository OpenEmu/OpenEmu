#ifndef __PCFX_SCSICD_H
#define __PCFX_SCSICD_H

#include <blip/Blip_Buffer.h>

typedef struct
{
 // Data bus(FIXME: we should have a variable for the target and the initiator, and OR them together to be truly accurate).
 uint8 DB;

 // Signals under our(the "target") control.
 bool BSY, MSG, CD, REQ, IO;

 // Signals under the control of the initiator(not us!)
 bool kingACK, kingRST, kingSEL, kingATN;
} scsicd_bus_t;

extern scsicd_bus_t cd_bus; // Don't access this structure directly by name outside of scsicd.c, but use the macros below.

#define BSY_signal ((const bool)cd_bus.BSY)
#define ACK_signal ((const bool)cd_bus.kingACK)
#define RST_signal ((const bool)cd_bus.kingRST)
#define MSG_signal ((const bool)cd_bus.MSG)
#define SEL_signal ((const bool)cd_bus.kingSEL)
#define REQ_signal ((const bool)cd_bus.REQ)
#define IO_signal ((const bool)cd_bus.IO)
#define CD_signal ((const bool)cd_bus.CD)
#define ATN_signal (cd_bus.kingATN)

#define DB_signal ((const uint8)cd_bus.DB)

#define SCSICD_GetDB() DB_signal
#define SCSICD_GetBSY() BSY_signal
#define SCSICD_GetIO() IO_signal
#define SCSICD_GetCD() CD_signal
#define SCSICD_GetMSG() MSG_signal
#define SCSICD_GetREQ() REQ_signal

// Should we phase out getting these initiator-driven signals like this(the initiator really should keep track of them itself)?
#define SCSICD_GetACK() ACK_signal
#define SCSICD_GetRST() RST_signal
#define SCSICD_GetSEL() SEL_signal
#define SCSICD_GetATN() ATN_signal

void SCSICD_Power(void);
void SCSICD_SetDB(uint8 data);

// These SCSICD_Set* functions are kind of misnomers, at least in comparison to the SCSICD_Get* functions...
// They will set/clear the bits corresponding to the KING's side of the bus.
void SCSICD_SetACK(bool set);
void SCSICD_SetSEL(bool set);
void SCSICD_SetRST(bool set);
void SCSICD_SetATN(bool set);

uint32 SCSICD_Run(uint32);
void SCSICD_ResetTS(void);

enum
{
 SCSICD_PCE = 1,
 SCSICD_PCFX
};

enum
{
 SCSICD_IRQ_DATA_TRANSFER_DONE = 1,
 SCSICD_IRQ_DATA_TRANSFER_READY,
 SCSICD_IRQ_MAGICAL_REQ,
};

void SCSICD_GetCDDAValues(int16 &left, int16 &right);

void SCSICD_SetLog(void (*logfunc)(const char *, const char *, ...));

void SCSICD_Init(int type, int CDDATimeDiv, Blip_Buffer *leftbuf, Blip_Buffer *rightbuf, uint32 TransferRate, uint32 SystemClock, void (*IRQFunc)(int), void (*SSCFunc)(uint8, int));
void SCSICD_SetCDDAVolume(double left, double right);
bool SCSICD_StealByte(uint8 &stolen);
int SCSICD_StateAction(StateMem *sm, int load, int data_only, const char *sname);

bool SCSICD_IsInserted(void);
bool SCSICD_EjectVirtual(void);
bool SCSICD_InsertVirtual(void);

#endif
