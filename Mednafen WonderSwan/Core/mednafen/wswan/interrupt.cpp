#include "wswan.h"
#include "interrupt.h"
#include "v30mz.h"
#include <trio/trio.h>

namespace MDFN_IEN_WSWAN
{


static uint8 IStatus;
static uint8 IEnable;
static uint8 IVectorBase;

static bool IOn_Cache;
static uint32 IOn_Which;
static uint32 IVector_Cache;

static void RecalcInterrupt(void)
{
 IOn_Cache = FALSE;
 IOn_Which = 0;
 IVector_Cache = 0;

 for(int i = 0; i < 8; i++)
 {
  if(IStatus & IEnable & (1 << i))
  {
   IOn_Cache = TRUE;
   IOn_Which = i;
   IVector_Cache = (IVectorBase + i) * 4;
   break;
  }
 }
}

void WSwan_InterruptDebugForce(unsigned int level)
{
 v30mz_int((IVectorBase + level) * 4, TRUE);
}

void WSwan_Interrupt(int which)
{
 if(IEnable & (1 << which))
  IStatus |= 1 << which;

 //printf("Interrupt: %d\n", which);
 RecalcInterrupt();
}

void WSwan_InterruptWrite(uint32 A, uint8 V)
{
 //printf("Write: %04x %02x\n", A, V);
 switch(A)
 {
  case 0xB0: IVectorBase = V; RecalcInterrupt(); break;
  case 0xB2: IEnable = V; IStatus &= IEnable; RecalcInterrupt(); break;
  case 0xB6: /*printf("IStatus: %02x\n", V);*/ IStatus &= ~V; RecalcInterrupt(); break;
 }
}

uint8 WSwan_InterruptRead(uint32 A)
{
 //printf("Read: %04x\n", A);
 switch(A)
 {
  case 0xB0: return(IVectorBase);
  case 0xB2: return(IEnable);
  case 0xB6: return(1 << IOn_Which); //return(IStatus);
 }
 return(0);
}

void WSwan_InterruptCheck(void)
{
 if(IOn_Cache)
 {
  v30mz_int(IVector_Cache, FALSE);
 }
}

void WSwan_InterruptReset(void)
{
 IEnable = 0x00;
 IStatus = 0x00;
 IVectorBase = 0x00;
 RecalcInterrupt();
}

#ifdef WANT_DEBUGGER
static const char *PrettyINames[8] = { "Serial Send", "Key Press", "RTC Alarm", "Serial Recv", "Line Hit", "VBlank Timer", "VBlank", "HBlank Timer" };

uint32 WSwan_InterruptGetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 ret = 0;

 switch(id)
 {
  case INT_GSREG_ISTATUS:
	ret = IStatus;
	break;

  case INT_GSREG_IENABLE:
	ret = IEnable;
	break;

  case INT_GSREG_IVECTORBASE:
	ret = IVectorBase;
	break;
 }

 if(special && (id == INT_GSREG_ISTATUS || id == INT_GSREG_IENABLE))
 {
  trio_snprintf(special, special_len, "%s: %d, %s: %d, %s: %d, %s: %d, %s: %d, %s: %d, %s: %d, %s: %d",
		PrettyINames[0], (ret >> 0) & 1,
	        PrettyINames[1], (ret >> 1) & 1,
	        PrettyINames[2], (ret >> 2) & 1,
	        PrettyINames[3], (ret >> 3) & 1,
	        PrettyINames[4], (ret >> 4) & 1,
	        PrettyINames[5], (ret >> 5) & 1,
	        PrettyINames[6], (ret >> 6) & 1,
	        PrettyINames[7], (ret >> 7) & 1);
 }

 return(ret);
}

void WSwan_InterruptSetRegister(const unsigned int id, uint32 value)
{
 switch(id)
 {
  case INT_GSREG_ISTATUS:
	IStatus = value;
	break;

  case INT_GSREG_IENABLE:
	IEnable = value;
	break;

  case INT_GSREG_IVECTORBASE:
	IVectorBase = value;
	break;
 }

 RecalcInterrupt();
}

#endif

int WSwan_InterruptStateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(IStatus),
  SFVAR(IEnable),
  SFVAR(IVectorBase),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "INTR"))
  return(0);

 if(load)
  RecalcInterrupt();

 return(1);
}

}
