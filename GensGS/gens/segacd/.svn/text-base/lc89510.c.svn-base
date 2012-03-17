#include <stdio.h>
#include <string.h>
#include "gens_core/misc/misc.h"
#include "lc89510.h"
#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/sound/pcm.h"
#include "cd_sys.hpp"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "cd_aspi.h"
#endif

#define CDC_DMA_SPEED 256

int CDC_Decode_Reg_Read;


void CDD_Reset(void)
{
	int i;
	
	// Reseting CDD
	
	CDD.Fader = 0;
	CDD.Control = 0;
	CDD.Cur_Comm = 0;
	CDD.Status = 0;
	CDD.Minute = 0;
	CDD.Seconde = 0;
	CDD.Frame = 0;
	CDD.Ext = 0;
	
	for (i = 0; i < 10; i++)
	{
		CDD.Rcv_Status[i] = 0;
		CDD.Trans_Comm[i] = 0;
	}
	
	// Default checksum
	CDD.Rcv_Status[8] = 0xF;
	
	SCD.Cur_Track = 0;
	SCD.Cur_LBA = -150;
	SCD.Status_CDD = READY;
}


void CDC_Reset(void)
{
	//int i;
	
	// Reseting CDC
	
	memset(CDC.Buffer, 0, (16 * 1024 * 2) + 2352);
	CDC_Update_Header();
	
	CDC.COMIN = 0;
	CDC.IFSTAT = 0xFF;
	CDC.DAC.N = 0;
	CDC.DBC.N = 0;
	CDC.HEAD.N = 0x01000000;
	CDC.PT.N = 0;
	CDC.WA.N = 2352 * 2;
	CDC.STAT.N = 0x00000080;
	CDC.SBOUT = 0;
	CDC.IFCTRL = 0;
	CDC.CTRL.N = 0;
	
	CDC_Decode_Reg_Read = 0;
}


void LC89510_Reset(void)
{
	CDD_Reset();
	CDC_Reset();
	
	CDC.RS0 = 0;
	CDC.RS1 = 0;
	CDC.Host_Data = 0;
	CDC.DMA_Adr = 0;
	CDC.Stop_Watch = 0;
	
	SCD.Status_CDC = 0;
	CDD_Complete = 0;
}

void Update_CDC_TRansfert(void)
{
	unsigned int dep, length, add_dest;
	unsigned char *dest;
	
	if ((SCD.Status_CDC & 0x08) == 0)
		return;
	
	switch (CDC.RS0 & 0x0700)
	{
		case 0x0200:	// MAIN CPU
		case 0x0300:	// SUB CPU
			// Data ready in host port
			CDC.RS0 |= 0x4000;
			return;
			break;
		
		case 0x0400:		// PCM RAM
			dest = (unsigned char *) Ram_PCM;
			dep = ((CDC.DMA_Adr & 0x03FF) << 2) + PCM_Chip.Bank;
			add_dest = 2;
			break;
		
		case 0x0500:		// PRG RAM
			dest = (unsigned char *) Ram_Prg;
			dep = (CDC.DMA_Adr & 0xFFFF) << 3;
			add_dest = 2;
#ifdef DEBUG_CD
//                      fprintf(debug_SCD_file, "DMA transfert PRG RAM : adr = %.8X  ", dep);
#endif
			break;
		
		case 0x0700:		// WORD RAM
			if (Ram_Word_State >= 2)
			{
				dest = (unsigned char*)Ram_Word_1M;
				add_dest = 2;
				if (Ram_Word_State & 1)
					dep = ((CDC.DMA_Adr & 0x3FFF) << 3);
				else
					dep = ((CDC.DMA_Adr & 0x3FFF) << 3) + 0x20000;
			}
			else
			{
				dest = (unsigned char*)Ram_Word_2M;
				dep = ((CDC.DMA_Adr & 0x7FFF) << 3);
				add_dest = 2;
			}
			break;
		
		default:
			return;
	}
	
	if (CDC.DBC.N <= (CDC_DMA_SPEED * 2))
	{
		length = (CDC.DBC.N + 1) >> 1;
		SCD.Status_CDC &= ~0x08;	// Last transfert
		CDC.RS0 |= 0x8000;		// End data transfert
		CDC.RS0 &= ~0x4000;		// no more data ready
		CDC.IFSTAT |= 0x08;		// No more data transfert in progress
		
		// DTEIEN = Data Trasnfert End Interrupt Enable ?
		if (CDC.IFCTRL & 0x40)
		{
			CDC.IFSTAT &= ~0x40;
			
			if (Int_Mask_S68K & 0x20)
				sub68k_interrupt (5, -1);
#ifdef DEBUG_CD
			fprintf(debug_SCD_file, "CDC - DTE interrupt\n");
#endif
		}
	}
	else
		length = CDC_DMA_SPEED;

#ifdef DEBUG_CD
//      fprintf(debug_SCD_file, "DMA length = %.4X\n", length);
#endif
	
	
  	if ((CDC.RS0 & 0x0700) == 0x0400)
	{
		// PCM DMA
		int len = length;
		unsigned char *src = (unsigned char*)&CDC.Buffer[CDC.DAC.N];
		unsigned char *dst = (unsigned char*)dest + dep;
		
		while (len--)
		{
			*(unsigned short*)dst = *(unsigned short*)src;
			src += 2;
			dst += add_dest;
		}
		length <<= 1;
		CDC.DMA_Adr += length >> 2;
	}
	else
	{
		// OTHER DMA
		int len = length;
		unsigned char *src = (unsigned char*)&CDC.Buffer[CDC.DAC.N];
		unsigned char *dst = (unsigned char*)dest + dep;
		
		while (len--)
		{
			unsigned short outrol = *(unsigned short *) src;
			outrol = (outrol << 8) | (outrol >> 8);
			
			*(unsigned short*)dst = outrol;
			src += 2;
			dst += add_dest;
		}
		length <<= 1;
		CDC.DMA_Adr += length >> 3;
	}
	
	CDC.DAC.N = (CDC.DAC.N + length) & 0xFFFF;
	if (SCD.Status_CDC & 0x08)
		CDC.DBC.N -= length;
	else
		CDC.DBC.N = 0;
}


unsigned short
Read_CDC_Host_SUB (void)
{
  unsigned short val;

  if (SCD.Status_CDC & 0x08)	// Transfert data
    {
      if ((CDC.RS0 & 0x0700) == 0x0300)	// SUB CPU
	{
	  CDC.DBC.N -= 2;

	  if (CDC.DBC.N <= 0)
	    {
	      CDC.DBC.N = 0;
	      SCD.Status_CDC &= ~0x08;	// Last transfert
	      CDC.RS0 |= 0x8000;	// End data transfert
	      CDC.RS0 &= ~0x4000;	// no more data ready
	      CDC.IFSTAT |= 0x08;	// No more data transfert in progress

	      if (CDC.IFCTRL & 0x40)	// DTEIEN = Data Trasnfert End Interrupt Enable ?
		{
		  CDC.IFSTAT &= ~0x40;

		  if (Int_Mask_S68K & 0x20)
		    sub68k_interrupt (5, -1);

#ifdef DEBUG_CD
		  fprintf (debug_SCD_file, "CDC - DTE interrupt\n");
#endif
		}
	    }
	  val = *(unsigned short *) &CDC.Buffer[CDC.DAC.N];
	  CDC.DAC.N += 2;
	  val = (val >> 8) | (val << 8);
#ifdef DEBUG_CD
//      fprintf(debug_SCD_file, "Host READ on SUB CPU side : %.4X  DBA = %.4X  DBC = %.4X\n", val, CDC.DBC.N, CDC.DAC.N);
#endif

	  return val;
	}
    }

  return 0;
}


unsigned short
Read_CDC_Host_MAIN (void)
{
  unsigned short val;

  if (SCD.Status_CDC & 0x08)	// Transfert data
    {
      if ((CDC.RS0 & 0x0700) == 0x0200)	// MAIN CPU
	{
	  CDC.DBC.N -= 2;

	  if (CDC.DBC.N <= 0)
	    {
	      SCD.Status_CDC &= ~0x08;	// Last transfert
	      CDC.RS0 |= 0x8000;	// End data transfert
	      CDC.RS0 &= ~0x4000;	// no more data ready
	      CDC.IFSTAT |= 0x08;	// No more data transfert in progress

	      if (CDC.IFCTRL & 0x40)	// DTEIEN = Data Trasnfert End Interrupt Enable ?
		{
		  CDC.IFSTAT &= ~0x40;

		  if (Int_Mask_S68K & 0x20)
		    sub68k_interrupt (5, -1);

#ifdef DEBUG_CD
		  fprintf (debug_SCD_file, "CDC - DTE interrupt\n");
#endif
		}
	    }
	  val = *(unsigned short *) &CDC.Buffer[CDC.DAC.N];
	  CDC.DAC.N += 2;
	  val = (val >> 8) | (val << 8);
#ifdef DEBUG_CD
//      fprintf(debug_SCD_file, "Host READ on SUB CPU side : %.4X  DBA = %.4X  DBC = %.4X\n", val, CDC.DBC.N, CDC.DAC.N);
#endif

	  return val;
	}
    }

  return 0;
}


void
CDC_Update_Header (void)
{
  if (CDC.CTRL.B.B1 & 0x01)	// Sub-Hearder wanted ?
    {
      CDC.HEAD.B.B0 = 0;
      CDC.HEAD.B.B1 = 0;
      CDC.HEAD.B.B2 = 0;
      CDC.HEAD.B.B3 = 0;
    }
  else
    {
      _msf MSF;

      LBA_to_MSF (SCD.Cur_LBA, &MSF);

      CDC.HEAD.B.B0 = INT_TO_BCDB (MSF.M);
      CDC.HEAD.B.B1 = INT_TO_BCDB (MSF.S);
      CDC.HEAD.B.B2 = INT_TO_BCDB (MSF.F);
      CDC.HEAD.B.B3 = 0x01;
    }
}


unsigned char
CDC_Read_Reg (void)
{
  unsigned char ret;

#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "CDC read reg %.2d = ", CDC.RS0 & 0xF);
#endif

  switch (CDC.RS0 & 0xF)
    {
    case 0x0:			// COMIN
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.COMIN);
#endif

      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x1;
      return CDC.COMIN;

    case 0x1:			// IFSTAT
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.IFSTAT);
#endif

      CDC_Decode_Reg_Read |= (1 << 1);	// Reg 1 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x2;
      return CDC.IFSTAT;

    case 0x2:			// DBCL
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.DBC.B.L);
#endif

      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x3;
      return CDC.DBC.B.L;

    case 0x3:			// DBCH
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.DBC.B.H);
#endif

      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x4;
      return CDC.DBC.B.H;

    case 0x4:			// HEAD0
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.HEAD.B.B0);
#endif

      CDC_Decode_Reg_Read |= (1 << 4);	// Reg 4 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x5;
      return CDC.HEAD.B.B0;

    case 0x5:			// HEAD1
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.HEAD.B.B1);
#endif

      CDC_Decode_Reg_Read |= (1 << 5);	// Reg 5 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x6;
      return CDC.HEAD.B.B1;

    case 0x6:			// HEAD2
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.HEAD.B.B2);
#endif

      CDC_Decode_Reg_Read |= (1 << 6);	// Reg 6 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x7;
      return CDC.HEAD.B.B2;

    case 0x7:			// HEAD3
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.HEAD.B.B3);
#endif

      CDC_Decode_Reg_Read |= (1 << 7);	// Reg 7 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x8;
      return CDC.HEAD.B.B3;

    case 0x8:			// PTL
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.PT.B.L);
#endif

      CDC_Decode_Reg_Read |= (1 << 8);	// Reg 8 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x9;
      return CDC.PT.B.L;

    case 0x9:			// PTH
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.PT.B.H);
#endif

      CDC_Decode_Reg_Read |= (1 << 9);	// Reg 9 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xA;
      return CDC.PT.B.H;

    case 0xA:			// WAL
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.WA.B.L);
#endif

      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xB;
      return CDC.WA.B.L;

    case 0xB:			// WAH
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.WA.B.H);
#endif

      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xC;
      return CDC.WA.B.H;

    case 0xC:			// STAT0
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.STAT.B.B0);
#endif

      CDC_Decode_Reg_Read |= (1 << 12);	// Reg 12 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xD;
      return CDC.STAT.B.B0;

    case 0xD:			// STAT1
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.STAT.B.B1);
#endif

      CDC_Decode_Reg_Read |= (1 << 13);	// Reg 13 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xE;
      return CDC.STAT.B.B1;

    case 0xE:			// STAT2
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.STAT.B.B2);
#endif

      CDC_Decode_Reg_Read |= (1 << 14);	// Reg 14 (decoding)
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xF;
      return CDC.STAT.B.B2;

    case 0xF:			// STAT3
#ifdef DEBUG_CD
      fprintf (debug_SCD_file, "%.2X\n", CDC.STAT.B.B3);
#endif

      ret = CDC.STAT.B.B3;
      CDC.IFSTAT |= 0x20;	// decoding interrupt flag cleared
      if ((CDC.CTRL.B.B0 & 0x80) && (CDC.IFCTRL & 0x20))
	{
	  if ((CDC_Decode_Reg_Read & 0x73F2) == 0x73F2)
	    CDC.STAT.B.B3 = 0x80;
	}
      return ret;
    }

  return 0;
}


void
CDC_Write_Reg (unsigned char Data)
{

#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "CDC write reg%d = %.2X\n", CDC.RS0 & 0xF, Data);
#endif

  switch (CDC.RS0 & 0xF)
    {
    case 0x0:			// SBOUT
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x1;
      CDC.SBOUT = Data;

      break;

    case 0x1:			// IFCTRL
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x2;
      CDC.IFCTRL = Data;

      if ((CDC.IFCTRL & 0x02) == 0)	// Stop data transfert
	{
	  CDC.DBC.N = 0;
	  SCD.Status_CDC &= ~0x08;
	  CDC.IFSTAT |= 0x08;	// No more data transfert in progress

/*
if (CDD.Trans_Comm[1] != 0)
{
	SCD.Status_CDD = READY;
	SCD.Status_CDC &= 0;
}*/

	}
      break;

    case 0x2:			// DBCL
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x3;
      CDC.DBC.B.L = Data;

      break;

    case 0x3:			// DBCH
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x4;
      CDC.DBC.B.H = Data;

      break;

    case 0x4:			// DACL
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x5;
      CDC.DAC.B.L = Data;

      break;

    case 0x5:			// DACH
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x6;
      CDC.DAC.B.H = Data;

      break;

    case 0x6:			// DTTRG
      if (CDC.IFCTRL & 0x02)	// Data transfert enable ?
	{
	  CDC.IFSTAT &= ~0x08;	// Data transfert in progress
	  SCD.Status_CDC |= 0x08;	// Data transfert in progress
	  CDC.RS0 &= 0x7FFF;	// A data transfert start

#ifdef DEBUG_CD
	  fprintf (debug_SCD_file,
		   "\n************** Starting Data Transfert ***********\n");
	  fprintf (debug_SCD_file,
		   "RS0 = %.4X  DAC = %.4X  DBC = %.4X  DMA adr = %.4X\n\n",
		   CDC.RS0, CDC.DAC.N, CDC.DBC.N, CDC.DMA_Adr);
#endif
	}
      break;

    case 0x7:			// DTACK
      CDC.IFSTAT |= 0x40;	// end data transfert interrupt flag cleared
      break;

    case 0x8:			// WAL
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x9;
      CDC.WA.B.L = Data;

      break;

    case 0x9:			// WAH
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xA;
      CDC.WA.B.H = Data;

      break;

    case 0xA:			// CTRL0
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xB;
      CDC.CTRL.B.B0 = Data;

      break;

    case 0xB:			// CTRL1
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xC;
      CDC.CTRL.B.B1 = Data;

      break;

    case 0xC:			// PTL
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xD;
      CDC.PT.B.L = Data;

      break;

    case 0xD:			// PTH
      CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xE;
      CDC.PT.B.H = Data;

      break;

    case 0xE:			// CTRL2
      CDC.CTRL.B.B2 = Data;
      break;

    case 0xF:			// RESET
      CDC_Reset ();
      break;
    }
}


void
CDD_Processing (void)
{

#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "CDD exporting status\n");
  fprintf (debug_SCD_file,
	   "Status=%.4X, Minute=%.4X, Seconde=%.4X, Frame=%.4X, Ext=%.4X\n",
	   CDD.Status, CDD.Minute, CDD.Seconde, CDD.Frame, CDD.Ext);
#endif

  CDD_Export_Status ();

  sub68k_interrupt (4, -1);
}


void
CDD_Import_Command (void)
{

#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "CDD importing command\n");
  fprintf (debug_SCD_file,
	   "Commande=%.4X, Minute=%.4X, Seconde=%.4X, Frame=%.4X  Checksum=%.4X\n",
	   (CDD.Trans_Comm[0] & 0xFF) + ((CDD.Trans_Comm[1] & 0xFF) << 8),
	   (CDD.Trans_Comm[2] & 0xFF) + ((CDD.Trans_Comm[3] & 0xFF) << 8),
	   (CDD.Trans_Comm[4] & 0xFF) + ((CDD.Trans_Comm[5] & 0xFF) << 8),
	   (CDD.Trans_Comm[6] & 0xFF) + ((CDD.Trans_Comm[7] & 0xFF) << 8),
	   (CDD.Trans_Comm[8] & 0xFF) + ((CDD.Trans_Comm[9] & 0xFF) << 8));
#endif

  switch (CDD.Trans_Comm[1])
    {
    case 0x0:			// STATUS (?)
      Get_Status_CDD_c0 ();
      break;

    case 0x1:			// STOP ALL (?)
      Stop_CDD_c1 ();
      break;

    case 0x2:			// GET TOC INFORMATIONS
      switch (CDD.Trans_Comm[2])
	{
	case 0x0:		// get current position (MSF format)
	  CDD.Status = (CDD.Status & 0xFF00);
	  Get_Pos_CDD_c20 ();
	  break;

	case 0x1:		// get elapsed time of current track played/scanned (relative MSF format)
	  CDD.Status = (CDD.Status & 0xFF00) | 1;
	  Get_Track_Pos_CDD_c21 ();
	  break;

	case 0x2:		// get current track in RS2-RS3
	  CDD.Status = (CDD.Status & 0xFF00) | 2;
	  Get_Current_Track_CDD_c22 ();
	  break;

	case 0x3:		// get total length (MSF format)
	  CDD.Status = (CDD.Status & 0xFF00) | 3;
	  Get_Total_Length_CDD_c23 ();
	  break;

	case 0x4:		// first & last track number
	  CDD.Status = (CDD.Status & 0xFF00) | 4;
	  Get_First_Last_Track_CDD_c24 ();
	  break;

	case 0x5:		// get track addresse (MSF format)
	  CDD.Status = (CDD.Status & 0xFF00) | 5;
	  Get_Track_Adr_CDD_c25 ();
	  break;

	default:		// invalid, then we return status
	  CDD.Status = (CDD.Status & 0xFF00) | 0xF;
	  Get_Status_CDD_c0 ();
	  break;
	}
      break;

    case 0x3:			// READ
      Play_CDD_c3 ();
      break;

    case 0x4:			// SEEK
      Seek_CDD_c4 ();
      break;

    case 0x6:			// PAUSE/STOP
      Pause_CDD_c6 ();
      break;

    case 0x7:			// RESUME
      Resume_CDD_c7 ();
      break;

    case 0x8:			// FAST FOWARD
      Fast_Foward_CDD_c8 ();
      break;

    case 0x9:			// FAST REWIND
      Fast_Rewind_CDD_c9 ();
      break;

    case 0xA:			// RECOVER INITIAL STATE (?)
      CDD_cA ();
      break;

    case 0xC:			// CLOSE TRAY
      Close_Tray_CDD_cC ();
      break;

    case 0xD:			// OPEN TRAY
      Open_Tray_CDD_cD ();
      break;

    default:
      // UNKNOW
      CDD_Def ();
      break;
    }
}


unsigned char
SCD_Read_Byte (unsigned int Adr)
{

#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "SCD read (B), address = %.8X\n", Adr);
#endif

  return 0;
}


unsigned short
SCD_Read_Word (unsigned int Adr)
{

#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "SCD read (W), address = %.8X\n", Adr);
#endif

  return 0;
}




void
debug_truc (unsigned int val)
{
#ifdef DEBUG_CD
  fprintf (debug_SCD_file, "ebx = %.8X\n", val);
#endif
}


// Symbol aliases for cross-OS asm compatibility.
unsigned short _Read_CDC_Host_SUB(void)
	__attribute__ ((weak, alias ("Read_CDC_Host_SUB")));
unsigned short _Read_CDC_Host_MAIN(void)
	__attribute__ ((weak, alias ("Read_CDC_Host_MAIN")));
unsigned char _CDC_Read_Reg(void)
	__attribute__ ((weak, alias ("CDC_Read_Reg")));
void _CDC_Write_Reg(unsigned char Data)
	__attribute__ ((weak, alias ("CDC_Write_Reg")));
void _CDD_Processing(void)
	__attribute__ ((weak, alias ("CDD_Processing")));
void _CDD_Import_Command(void)
	__attribute__ ((weak, alias ("CDD_Import_Command")));
unsigned char _SCD_Read_Byte(unsigned int Adr)
	__attribute__ ((weak, alias ("SCD_Read_Byte")));
unsigned char _SCD_Read_Word(unsigned int Adr)
	__attribute__ ((weak, alias ("SCD_Read_Word")));
