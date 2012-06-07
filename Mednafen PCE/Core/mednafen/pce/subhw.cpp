#include "pce.h"
#include "c68k/c68k.h"

static inline uint8 READ8_MSB(const uint8 *base, const unsigned int addr)
{
 return(base[addr + 0]);
}

static inline uint16 READ16_MSB(const uint8 *base, const unsigned int addr)
{
 return((base[addr + 0] << 8) | (base[addr + 1] << 0));
}

static inline void WRITE8_MSB(uint8 *base, const unsigned int addr, const uint8 value)
{
 base[addr + 0] = value;
}

static inline void WRITE16_MSB(uint8 *base, const unsigned int addr, const uint16 value)
{
 base[addr + 0] = value >> 8;
 base[addr + 1] = value;
}

namespace MDFN_IEN_PCE
{

static const uint8 VROM[8] = { 0x47, 0x6E, 0x31, 0x14, 0xB3, 0xEB, 0xEC, 0x2B };
static uint8 VROM_Index;
static bool HuVisible;


static uint8 ExHuRAM[8192];
static uint8 *BigRAM = NULL;
static uint8 *FBRAM[2] = { NULL, NULL };
static uint16 PalRAM[0x100];

static uint32 BigRAMWOTemp, BigRAMWO;

static uint8 CommPort[4];

static bool HuBigMaster;
static bool M68KResetPending;
//static bool M68KIntPending;
static bool OverlayEnabled;

static bool CurDisplayFB;
static bool FBSwapPending;
static uint8 VSyncCounter;
static uint32 DisplayScanline;

static uint32 AutoClearAddr;
static int32 AutoClearCount;

static c68k_struc M68K;
static uint8 M68K_ReadMemory8(uint32 A);
static uint16 M68K_ReadMemory16(uint32 A);
static void M68K_WriteMemory8(uint32 A, uint8 V);
static void M68K_WriteMemory16(uint32 A, uint16 V);

static int32 lastts;
static int32 M68K_cycle_counter;

static void AllocSubHWMem(void)
{
 if(!BigRAM)
  BigRAM = (uint8 *)MDFN_calloc(1 << 20, 1, "BIG RAM RAM");

 if(!FBRAM[0])
  FBRAM[0] = (uint8 *)MDFN_calloc(65536, 1, "FB RAM");

 if(!FBRAM[1])
  FBRAM[1] = (uint8 *)MDFN_calloc(65536, 1, "FB RAM");

 memset(FBRAM[0], 0xFF, 65536);
 memset(FBRAM[1], 0xFF, 65536);
}

static void FreeSubHWMem(void)
{
 if(BigRAM)
 {
  MDFN_free(BigRAM);
  BigRAM = NULL;
 }

 if(FBRAM[0])
 {
  MDFN_free(FBRAM[0]);
  FBRAM[0] = NULL;
 }

 if(FBRAM[1])
 {
  MDFN_free(FBRAM[1]);
  FBRAM[1] = NULL;
 }
}


#if 0
#include <desa68/desa68.h>

static uint16_t dis_callb(uint32_t A, void *private_data)
{
 //printf("%04x\n", M68K_ReadMemory16(A & 0xFFFFFF));
 return(M68K_ReadMemory16(A & 0xFFFFFF));
}
#endif

static void UpdateM68K(const int32 timestamp)
{
 int32 hucycles = timestamp - lastts;
 lastts = timestamp;

 if(HuBigMaster)
  return;

 if(M68KResetPending)
 {
  M68KResetPending = false;
  C68k_Reset(&M68K);
 }

 M68K_cycle_counter += hucycles;

 while(M68K_cycle_counter > 0)
 {
#if 0
  char TextBuf[256];
  DESA68parm_t d;
  strcpy(TextBuf, "Invalid");
  memset(&d, 0, sizeof(DESA68parm_t));

  d.mem_callb = dis_callb;
  d.memmsk = 0xFFFFFF;
  d.pc = M68K.PC;
  d.str = TextBuf;
  d.strmax = 255;        // FIXME, MDFN API change

  desa68(&d);
  printf("%08x: %s\n", M68K.PC, TextBuf);
#endif
  //M68K_cycle_counter -= C68k_Exec(&M68K) * 2;
  M68K.timestamp = 0;
  C68k_Exec(&M68K);
  M68K_cycle_counter -= M68K.timestamp * 2;
 }
}

void SubHW_EndFrame(const int32 timestamp)
{
 UpdateM68K(timestamp);
 lastts = 0;
}


static int M68K_Int_Ack_Callback(int level)
{
 return C68K_INT_ACK_AUTOVECTOR;
}

void SubHW_Init(void)
{
 C68k_Init(&M68K, M68K_Int_Ack_Callback);

 C68k_Set_ReadB(&M68K, M68K_ReadMemory8);
 C68k_Set_ReadW(&M68K, M68K_ReadMemory16);

 C68k_Set_WriteB(&M68K, M68K_WriteMemory8);
 C68k_Set_WriteW(&M68K, M68K_WriteMemory16);
}

void SubHW_Kill(void)
{
 FreeSubHWMem();

 if(BigRAM)
 {
  MDFN_free(BigRAM);
  BigRAM = NULL;
 }

 if(FBRAM[0])
 {
  MDFN_free(FBRAM[0]);
  FBRAM[0] = NULL;
 }

 if(FBRAM[1])
 {
  MDFN_free(FBRAM[1]);
  FBRAM[1] = NULL;
 }
}

static INLINE uint16 M68K_ReadIO(uint32 A)
{
 if(A & 0x100)
 {
  uint16 ret = (FBSwapPending << 15) | ((AutoClearCount > 0) ? 0x4000 : 0) | VSyncCounter;

  //printf("Moo: %04x\n", ret);

  return(ret);
 }
 else
 {
  return(CommPort[(A >> 1) & 0x3]);
 }
}

static INLINE void M68K_WriteIO(uint32 A, uint16 V)
{
 if(A & 0x100)
 {
  if(V & 0x100)
   C68k_Set_IRQ(&M68K, 0);
  OverlayEnabled = (bool)(V & 0x2);
  FBSwapPending |= (bool)(V & 0x1);
 }
 else
 {
  CommPort[(A >> 1) & 0x3] = V;
 }
}


static uint8 M68K_ReadMemory8(uint32 A)
{
 switch((A >> 20) & 0xF)
 {
  default: return(0xFF);

  case 0x0:
	return READ8_MSB(BigRAM, A & 0x7FFFF);

  case 0x4:
	return M68K_ReadIO(A);
 }
}

static uint16 M68K_ReadMemory16(uint32 A)
{
 //printf("Read16: %08x\n", A);
 switch((A >> 20) & 0xF)
 {
  default: return(0xFFFF);

  case 0x0 ... 0x3:
        return READ16_MSB(BigRAM, A & 0x7FFFF);

  case 0x4:
	return M68K_ReadIO(A);
 }
}

static void M68K_WriteMemory8(uint32 A, uint8 V)
{
 //printf("Write8 %08x %02x\n", A, V);
 switch((A >> 20) & 0xF)
 {
  default: break;

  case 0x0:
        WRITE8_MSB(BigRAM, A & 0x7FFFF, V);
        break;

  case 0x1:
        if(AutoClearCount > 0)
        {
         puts("Write during autoclear");
         return;
        }
        {
         uint16 fba = A & 0xFFFF;

         if(A & 0x10000)
          fba = (fba << 8) | (fba >> 8);

         if(V)
          FBRAM[CurDisplayFB ^ 1][fba] = V;
        }
	break;

  case 0x4:
	M68K_WriteIO(A, V);
	break;
 }
}
static void M68K_WriteMemory16(uint32 A, uint16 V)
{
 //printf("Write16 %08x %02x\n", A, V);
 switch((A >> 20) & 0xF)
 {
  default: break;

  case 0x0:
        WRITE16_MSB(BigRAM, A & 0x7FFFF, V);
	break;

  case 0x1:
        if(AutoClearCount > 0)
        {
         puts("Write during autoclear");
         return;
        }

	{
	 uint16 fba = A & 0xFFFF;

	 if(A & 0x10000)
	  fba = (fba << 8) | (fba >> 8);

	 if(V & 0xFF00)
          FBRAM[CurDisplayFB ^ 1][(fba + 0) & 0xFFFF] = V >> 8;
	 if(V & 0x00FF)
          FBRAM[CurDisplayFB ^ 1][(fba + 1) & 0xFFFF] = V >> 0;
	}
        break;

  case 0x2:
        PalRAM[(A >> 1) & 0xFF] = V;
	break;

  case 0x4:
	M68K_WriteIO(A, V);
	break;
 }
}

void SubHW_WriteIOPage(uint32 A, uint8 V)
{
 if(!HuVisible)
 {
  if(VROM[VROM_Index] == V)
   VROM_Index++;
  else
   VROM_Index = 0;

  if(VROM_Index == 8)
  {
   printf("Overlay hardware enabled.");
   MDFN_DispMessage("Overlay hardware enabled.");
   HuVisible = true;

   AllocSubHWMem();
  }
  return;
 }

 switch(A & 0x1FFF)
 {
  case 0x1C00 ... 0x1FEF:
	ExHuRAM[A & 0x1FFF] = V;
	break;

  case 0x1FF0 ... 0x1FF3:
	UpdateM68K(HuCPU->Timestamp());
	CommPort[A & 0x3] = V;
	break;

  case 0x1FF4:
	BigRAMWOTemp &= 0xFF00;
	BigRAMWOTemp |= V;
	break;

  case 0x1FF5:
	BigRAMWOTemp &= 0x00FF;
	BigRAMWOTemp |= V << 8;
	BigRAMWO = BigRAMWOTemp << 4;
	break;

  case 0x1FF6:
	if(HuBigMaster)
	{
	 //printf("BIG: %08x %02x\n", BigRAMWO, V);
	 BigRAM[BigRAMWO] = V;
	 BigRAMWO = (BigRAMWO + 1) & ((1 << 20) - 1);
	}
	break;

  case 0x1FF7:
	UpdateM68K(HuCPU->Timestamp());
	HuBigMaster = V & 0x01;
	M68KResetPending |= (bool)(V & 0x02);
	C68k_Set_IRQ(&M68K, (bool)(V & 0x04));
	//M68KIntPending |= (bool)(V & 0x04);
	OverlayEnabled = (bool)(V & 0x80);
	break;
 }
}

uint8 SubHW_ReadIOPage(uint32 A)
{
 if(!HuVisible)
  return(0xFF);

 switch(A & 0x1FFF)
 {
  case 0x1C00 ... 0x1FEF:
        return ExHuRAM[A & 0x1FFF];
        break;

  case 0x1FF0 ... 0x1FF3:
	UpdateM68K(HuCPU->Timestamp());
        return CommPort[A & 0x3];
        break;
 }

 return(0xFF);
}

DECLFW(SubHW_WriteFEPage)
{
 if(!HuVisible)
  return;

 ExHuRAM[A & 0x1FFF] = V;
}

DECLFR(SubHW_ReadFEPage)
{
 if(!HuVisible)
  return(0xFF);

 return(ExHuRAM[A & 0x1FFF]);
}

void SubHW_CVSync(void)
{
 if(!HuVisible)
  return;

 UpdateM68K(HuCPU->Timestamp());

 if(FBSwapPending)
 {
  FBSwapPending = false;
  CurDisplayFB = !CurDisplayFB;

  AutoClearAddr = 0;
  AutoClearCount = 65536;
 }
 VSyncCounter++;
 DisplayScanline = 0;
}

bool SubHW_DisplayLine(uint16 *out_buffer)
{
 if(!HuVisible)
  return(false);

 UpdateM68K(HuCPU->Timestamp());

 if(OverlayEnabled)
  for(int x = 0; x < 256; x++)
   out_buffer[x] = PalRAM[FBRAM[CurDisplayFB][(((DisplayScanline - 17) & 0xFF) << 8) | x]];

 DisplayScanline++;

 for(int i = 0; i < 683; i++)
 {
  if(AutoClearCount <= 0)
   break;

  FBRAM[CurDisplayFB ^ 1][AutoClearAddr++] = 0;
  AutoClearCount--;
 }

 return(OverlayEnabled);
}

void SubHW_Power(void)
{
 memset(ExHuRAM, 0xFF, sizeof(ExHuRAM));

 HuVisible = false;
 VROM_Index = 0;
 HuBigMaster = true;
 M68KResetPending = true;
 //M68KIntPending = false;
 C68k_Set_IRQ(&M68K, 0);

 OverlayEnabled = false;

 CurDisplayFB = 0;
 FBSwapPending = false;
 VSyncCounter = 0;

 AutoClearAddr = 0;
 AutoClearCount = 0;

 CommPort[0] = 0xFF;
 CommPort[1] = 0xFF;
 CommPort[2] = 0xFF;
 CommPort[3] = 0xFF;
}

int SubHW_StateAction(StateMem *sm, int load, int data_only)
{
 int ret;

 SFORMAT VisibleStateRegs[] = 
 {
  SFVAR(HuVisible),
  SFEND
 };

 // Since "SubHWV" section is optional, we need to default to not being visible if the section isn't there.
 if(load)
  HuVisible = false;

 ret = MDFNSS_StateAction(sm, load, data_only, VisibleStateRegs, "SubHWV", true);

 if(HuVisible)
 {
  unsigned int c68k_state_len = C68k_Get_State_Max_Len();
  uint8 c68k_state[c68k_state_len];

  C68k_Save_State(&M68K, c68k_state);

  SFORMAT StateRegs[] =
  {
   SFVAR(VROM_Index),
   SFARRAY(ExHuRAM, 8192),
   SFARRAY(BigRAM, 1 << 20),
   SFARRAY(FBRAM[0], 65536),
   SFARRAY(FBRAM[1], 65536),
   SFARRAY16(PalRAM, 0x100),
   SFVAR(BigRAMWOTemp),
   SFVAR(BigRAMWO),
   SFARRAY(CommPort, 4),
   SFVAR(HuBigMaster),
   SFVAR(M68KResetPending),
   SFVAR(OverlayEnabled),
   SFVAR(CurDisplayFB),
   SFVAR(FBSwapPending),
   SFVAR(VSyncCounter),
   SFVAR(DisplayScanline),
   SFVAR(AutoClearAddr),
   SFVAR(AutoClearCount),

   SFARRAY(c68k_state, c68k_state_len),
   SFEND
  };
  AllocSubHWMem();

  ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, "SubHW");
  if(load)
  {
   C68k_Load_State(&M68K, c68k_state);
  }
 }
 else
 {
  FreeSubHWMem();
 }


 return(ret);
}


}
