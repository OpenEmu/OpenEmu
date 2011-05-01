/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Emulation for HuC6273, FXGA 3D chip */
/* Definitions:
	CMT = Command Macro Table
*/

enum
{
	OP_NOP = 0x00,
	OP_TRIANGLE_STRIP = 0x01,
	OP_TRIANGLE_LIST = 0x02,
	OP_POLY_LINE = 0x03,
	OP_LINE_LIST = 0x04,
	OP_RESERVED0 = 0x05,
	OP_PUT_IMAGE = 0x06,
	OP_READ_PIXEL = 0x07,
	OP_WRITE_TE_REGISTERS = 0x08,
	OP_WRITE_PE_REGISTERS = 0x09,
	OP_MISC = 0x0A,
	OP_RESERVED1 = 0x0B,
	OP_READ_TE_REGISTERS = 0x0C,
	OP_READ_PE_REGISTERS = 0x0D,
	OP_WRITE_LUT = 0x0E,
	OP_READ_LUT = 0x0F,
};

#include "pcfx.h"
#include "huc6273.h"

enum
{
 INT_RHIT = 15,	// Raster Hit
 INT_HBL = 14, // H blank
 INT_VBL = 13, // V blank
 INT_HSY = 12, // H sync
 INT_VSY = 11, // V sync
 INT_RBDONE = 10, // Read back done
 INT_TESYNC = 9, // TE Sync
 INT_SPDONE = 8, // Sprite done
 INT_CMDONE = 7, // Command macro done
 INT_PESYNC = 6, // PE sync
 INT_AEMP = 5, // Almost empty
 INT_CSE = 4, // Command sync error
 INT_AFL = 3, // Almost full
 INT_OVF = 2, // Overflow
 INT_FSY = 1, // Frame sync
};

static uint16 FIFO[0x20];
static uint8 InFIFO;
#define AFW (0x20 - InFIFO)

#define AEMPWD ((FIFOControl >> 4) & 0xFF)
#define AFLWD (FIFOControl & 0xF)
static uint16 FIFOControl; // 0x00004

static uint8 CMTBankSelect;
static uint16 CMTStartAddress;
static uint16 CMTByteCount;

static uint16 InterruptMask;
static uint16 InterruptStatus;

static uint16 ReadBack;

static uint16 HorizontalTiming, VerticalTiming;

static uint16 SCTAddressHi;
static uint16 SpriteControl;
static uint16 CDResult[2];
static uint16 SPWindowX[2]; // left and right
static uint16 SPWindowY[2]; // top and bottom
static uint16 MiscStatus;
static uint16 ErrorStatus; // Read only!
static uint16 DisplayControl;
static uint16 StatusControl;
static uint16 Config;

static uint16 RasterHit;


static uint16 Results[0x10];

static void CheckIRQ(void)
{
 uint16 MaskedResults = InterruptStatus & InterruptMask;



}


static void ProcessFIFO(void)
{
 uint8 length = FIFO[0] & 0xFF;

 if(length > 0x20) 
 {
  length = 0x20;
  puts("Length too long");
 }

 if(InFIFO >= length)
 {
  int opcode = FIFO[0] >> 12;
  int option = (FIFO[0] >> 8) & 0x0F;

  printf("Op: %02x, option: %02x\n", opcode, option);

  InFIFO -= length;
  for(int i = 0; i < InFIFO; i++)
   FIFO[i] = FIFO[length + i];
 }
}


static void StoreInFIFO(uint16 V)
{
 if(AFW > 0)
 {
  FIFO[InFIFO] = V;
  InFIFO++;

  ProcessFIFO();
 }
}

uint8 HuC6273_Read8(uint32 A)
{
 puts("73 Read8");
}

uint16 HuC6273_Read16(uint32 A)
{
 A &= 0xfffff;

 printf("HuC6273 Read: %04x\n", A);

 switch(A)
 {
  case 0x00000: 
  case 0x00002: return(AFW); // Command FIFO status
		
  case 0x00004: return(FIFOControl);
  case 0x00006: return(CMTBankSelect);
  case 0x00008: return(CMTStartAddress);
  case 0x0000A: return(CMTByteCount);
  case 0x0000C: return(InterruptMask);
  case 0x0000E: return(0);
  case 0x00010: return(InterruptStatus);
  case 0x00012: return(ReadBack);
  case 0x00014: return(HorizontalTiming);
  case 0x00016: return(VerticalTiming);
  case 0x00018: return(SCTAddressHi);
  case 0x0001A: return(SpriteControl);
  case 0x0001C: return(CDResult[0]);
  case 0x0001E: return(CDResult[1]);
  case 0x00020: return(SPWindowX[0]);
  case 0x00022: return(SPWindowY[0]);
  case 0x00024: return(SPWindowX[1]);
  case 0x00026: return(SPWindowY[1]);
  case 0x00028: return(MiscStatus);
  case 0x0002A: return(ErrorStatus);
  case 0x0002C: return(DisplayControl);
  case 0x0002E: return(Config);
 }
 if(A >= 0x00060 && A <= 0x0007E)
 {
  return(Results[(A >> 1) & 0xF]);
 }
 return(0);
}


void HuC6273_Write16(uint32 A, uint16 V)
{
 A &= 0xfffff;

 printf("HuC6273 Write: %04x:%04x\n", A, V);

 switch(A)
 {
  case 0x00000:
  case 0x00002: StoreInFIFO(V); break;

  case 0x00004: FIFOControl = V; break;
  case 0x00006: CMTBankSelect = V & 0x1F; break;
  case 0x00008: CMTStartAddress = V & 0xFFFE; break;
  case 0x0000A: CMTByteCount = V & 0xFFFE; break;
  case 0x0000C: InterruptMask = V;
		CheckIRQ();
		break;
  case 0x0000E: // Interrupt Clear
		CheckIRQ();
		break;
  case 0x00010: InterruptStatus = V; 
		CheckIRQ();
		break;
  case 0x00012: ReadBack = V; break;
  case 0x00014: HorizontalTiming = V; break;
  case 0x00016: VerticalTiming = V; break;
  case 0x00018: SCTAddressHi = V; break;
  case 0x0001A: SpriteControl = V; break;
  case 0x0001C: CDResult[0] = V; break;
  case 0x0001E: CDResult[1] = V; break;
  case 0x00020: SPWindowX[0] = V; break; // X Left
  case 0x00022: SPWindowY[0] = V; break; // Y Top
  case 0x00024: SPWindowX[1] = V; break; // X Right
  case 0x00026: SPWindowY[1] = V; break; // Y Bottom
  case 0x00028: MiscStatus = V; break;
  case 0x0002C: DisplayControl = V; break;
  case 0x0002E: StatusControl = V; break;

  case 0x0003C: RasterHit = V; break;
 }
}

void HuC6273_Write8(uint32 A, uint8 V)
{
 puts("73 Write8");
}

void HuC6273_Reset(void)
{
 InFIFO = 0;
 FIFOControl = 0x5 | (0x20 << 4);
}



bool HuC6273_Init(void)
{

 return(TRUE);
}
