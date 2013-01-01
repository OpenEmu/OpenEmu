/*
  www.freedo.org
The first and only working 3DO multiplayer emulator.

The FreeDO licensed under modified GNU LGPL, with following notes:

*   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
*   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
*   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
    or any other material published by FreeDO is strictly forbidden without owners approval.

The above notes are taking precedence over GNU LGPL in conflicting situations.

Project authors:

Alexander Troosh
Maxim Grishin
Allen Wright
John Sammons
Felix Lazarev
*/

// Iso.cpp: implementation of the CIso class.
//
//////////////////////////////////////////////////////////////////////
#include "freedoconfig.h"
#include <memory.h>
#include "IsoXBUS.h"
#include "types.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define STATDELAY 100
#define REQSIZE	2048
enum MEI_CDROM_Error_Codes {
  MEI_CDROM_no_error = 0x00,
  MEI_CDROM_recv_retry = 0x01,
  MEI_CDROM_recv_ecc = 0x02,
  MEI_CDROM_not_ready = 0x03,
  MEI_CDROM_toc_error = 0x04,
  MEI_CDROM_unrecv_error = 0x05,
  MEI_CDROM_seek_error = 0x06,
  MEI_CDROM_track_error = 0x07,
  MEI_CDROM_ram_error = 0x08,
  MEI_CDROM_diag_error = 0x09,
  MEI_CDROM_focus_error = 0x0A,
  MEI_CDROM_clv_error = 0x0B,
  MEI_CDROM_data_error = 0x0C,
  MEI_CDROM_address_error = 0x0D,
  MEI_CDROM_cdb_error = 0x0E,
  MEI_CDROM_end_address = 0x0F,
  MEI_CDROM_mode_error = 0x10,
  MEI_CDROM_media_changed = 0x11,
  MEI_CDROM_hard_reset = 0x12,
  MEI_CDROM_rom_error = 0x13,
  MEI_CDROM_cmd_error = 0x14,
  MEI_CDROM_disc_out = 0x15,
  MEI_CDROM_hardware_error = 0x16,
  MEI_CDROM_illegal_request = 0x17
};


#define POLSTMASK	0x01
#define POLDTMASK	0x02
#define POLMAMASK	0x04
#define POLREMASK	0x08
#define POLST		0x10
#define POLDT		0x20
#define POLMA		0x40
#define POLRE		0x80

#define CDST_TRAY  0x80
#define CDST_DISC  0x40
#define CDST_SPIN  0x20
#define CDST_ERRO  0x10
#define CDST_2X    0x02
#define CDST_RDY   0x01
#define CDST_TRDISC 0xC0
#define CDST_OK    CDST_RDY|CDST_TRAY|CDST_DISC|CDST_SPIN

//medium specific
#define CD_CTL_PREEMPHASIS      0x01
#define CD_CTL_COPY_PERMITTED   0x02
#define CD_CTL_DATA_TRACK       0x04
#define CD_CTL_FOUR_CHANNEL     0x08
#define CD_CTL_QMASK            0xF0
#define CD_CTL_Q_NONE           0x00
#define CD_CTL_Q_POSITION       0x10
#define CD_CTL_Q_MEDIACATALOG   0x20
#define CD_CTL_Q_ISRC           0x30

#define MEI_DISC_DA_OR_CDROM    0x00
#define MEI_DISC_CDI            0x10
#define MEI_DISC_CDROM_XA       0x20

#define CDROM_M1_D              2048
#define CDROM_DA                2352
#define CDROM_DA_PLUS_ERR       2353
#define CDROM_DA_PLUS_SUBCODE   2448
#define CDROM_DA_PLUS_BOTH      2449

//medium specific
//drive specific
#define MEI_CDROM_SINGLE_SPEED  0x00
#define MEI_CDROM_DOUBLE_SPEED  0x80

#define MEI_CDROM_DEFAULT_RECOVERY         0x00
#define MEI_CDROM_CIRC_RETRIES_ONLY        0x01
#define MEI_CDROM_BEST_ATTEMPT_RECOVERY    0x20

#define Address_Blocks    0
#define Address_Abs_MSF   1
#define Address_Track_MSF 2

#pragma pack(push,1)
//drive specific
//disc data
struct TOCEntry{
unsigned char res0;
unsigned char CDCTL;
unsigned char TRKNUM;
unsigned char res1;
unsigned char mm;
unsigned char ss;
unsigned char ff;
unsigned char res2;
};
//disc data
struct DISCStc{
unsigned char curabsmsf[3]; //BIN form
unsigned char curtrack;
unsigned char nextmsf[3]; //BIN form
unsigned char tempmsf[3]; //BIN form
int  tempblk;
int  templba;
unsigned char currenterror;
unsigned char currentxbus;
unsigned int  currentoffset;
unsigned int  currentblocksize;
unsigned char currentspeed;
unsigned char  totalmsf[3];//BIN form
unsigned char  firsttrk;
unsigned char  lasttrk;
unsigned char  discid;
unsigned char  sesmsf[3]; //BIN form
TOCEntry DiscTOC[100];
};

class cdrom_Device
{
private:
	unsigned char Poll;
	unsigned char XbusStatus;
	unsigned char StatusLen;
	int  DataLen;
	int  DataPtr;
	unsigned int olddataptr;
	unsigned char CmdPtr;
	unsigned char Status[256];
	unsigned char Data[REQSIZE];
	unsigned char Command[7];
	char STATCYC;
	int Requested;
	MEI_CDROM_Error_Codes MEIStatus;
	DISCStc DISC;
        unsigned int curr_sector;

public:

	void Init();

	unsigned int GetStatusFifo();
	void __fastcall SendCommand(unsigned char val);
	unsigned int GetPoll();
	bool TestFIQ();
	void SetPoll(unsigned int val);
	unsigned int GetDataFifo();
	void DoCommand();
	unsigned char BCD2BIN(unsigned char in);
	unsigned char BIN2BCD(unsigned char in);
	void MSF2BLK();
	void BLK2MSF();
	void LBA2MSF();
	void MSF2LBA();
	unsigned char * GetDataPtr();
	unsigned int GetDataLen();
	void ClearDataPoll(unsigned int len);
	bool InitCD();
	unsigned char  *  GetBytes(unsigned int len);
	unsigned int GedWord();
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma pack(pop)

extern unsigned int _3do_DiscSize();
extern void _3do_Read2048(void *buff);
extern void _3do_OnSector(unsigned int sector);


void cdrom_Device::Init()
{
	unsigned int filesize;

	filesize=150;
	DataPtr=0;

		XbusStatus=0;
		//XBPOLL=POLSTMASK|POLDTMASK|POLMAMASK|POLREMASK;
		Poll=0xf;
		XbusStatus|=CDST_TRAY; //Inject the disc
		XbusStatus|=CDST_RDY;
		XbusStatus|=CDST_DISC;
		XbusStatus|=CDST_SPIN;
		MEIStatus=MEI_CDROM_no_error;

		DISC.firsttrk=1;
		DISC.lasttrk=1;
		DISC.curabsmsf[0]=0;
		DISC.curabsmsf[1]=2;
		DISC.curabsmsf[2]=0;

		DISC.DiscTOC[1].CDCTL=CD_CTL_DATA_TRACK|CD_CTL_Q_NONE;//|CD_CTL_COPY_PERMITTED;
		DISC.DiscTOC[1].TRKNUM=1;
		DISC.DiscTOC[1].mm=0;
		DISC.DiscTOC[1].ss=2;
		DISC.DiscTOC[1].ff=0;

		DISC.firsttrk=1;
		DISC.lasttrk=1;
		DISC.discid=MEI_DISC_DA_OR_CDROM;

		DISC.templba=filesize;
		LBA2MSF();
		DISC.totalmsf[0]=DISC.tempmsf[0];
		DISC.totalmsf[1]=DISC.tempmsf[1];
		DISC.totalmsf[2]=DISC.tempmsf[2];

		DISC.templba=filesize-150;
		LBA2MSF();
		DISC.sesmsf[0]=DISC.tempmsf[0];
		DISC.sesmsf[1]=DISC.tempmsf[1];
		DISC.sesmsf[2]=DISC.tempmsf[2];


	STATCYC=STATDELAY;
}

unsigned int cdrom_Device::GetStatusFifo()
{
	unsigned int res;
	res=0;
	if(StatusLen>0)
	{
		res=Status[0];
		StatusLen--;
		if(StatusLen>0)
			memcpy(Status,Status+1,StatusLen);
		else
		{
				Poll&=~POLST;
		}
	}
	return res;

}

void __fastcall cdrom_Device::SendCommand(unsigned char val)
{
 	if (CmdPtr<7)
	{
			Command[CmdPtr]=(unsigned char)val;
			CmdPtr++;

	}
	if((CmdPtr>=7) || (Command[0]==0x8))
	{

			//Poll&=~0x80; ???
			DoCommand();
			CmdPtr=0;
	}
}

unsigned int cdrom_Device::GetPoll()
{
	return Poll;
}

bool cdrom_Device::TestFIQ()
{
	if(((Poll&POLST) && (Poll&POLSTMASK)) || ((Poll&POLDT) && (Poll&POLDTMASK)))
	{
		return true;
	}
	return false;
}

void cdrom_Device::SetPoll(unsigned int val)
{
	Poll&=0xF0;
	val&=0xf;
	Poll|=val;
}

unsigned int cdrom_Device::GetDataFifo()
{
	unsigned int res;
	res=0;
        //int i;

	if(DataLen>0)
	{
		res=(unsigned char)Data[DataPtr];
		DataLen--;
		DataPtr++;

		if(DataLen==0)
		{
			DataPtr=0;
			if(Requested)
			{
                                _3do_OnSector(curr_sector++);
                                _3do_Read2048(Data);
				Requested--;
				DataLen=REQSIZE;
			}
			else
			{
				Poll&=~POLDT;
				Requested=0;
				DataLen=0;
				DataPtr=0;
			}

		}
	}

	return res;
}

void cdrom_Device::DoCommand()
{
	int i;

	/*for(i=0;i<=0x100000;i++)
		Data[i]=0;

	DataLen=0;*/
	StatusLen=0;


	Poll&=~POLST;
	Poll&=~POLDT;
	XbusStatus&=~CDST_ERRO;
	XbusStatus&=~CDST_RDY;
	switch(Command[0])
	{
	case 0x1:
		//seek
		//not used in opera
		//01 00 ll-bb-aa 00 00.
		//01 02 mm-ss-ff 00 00.
		//status 4 bytes
		//xx xx xx XS  (xs=xbus status)
//		sprintf(str,"#CDROM 0x1 SEEK!!!\n");
//		CDebug::DPrint(str);
		break;
	case 0x2:
		//spin up
		//opera status request = 0
		//status 4 bytes
		//xx xx xx XS  (xs=xbus status)
		if((XbusStatus&CDST_TRAY) && (XbusStatus&CDST_DISC))
		{
			XbusStatus|=CDST_SPIN;
			XbusStatus|=CDST_RDY;
			MEIStatus=MEI_CDROM_no_error;
		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus&=~CDST_RDY;
			MEIStatus=MEI_CDROM_recv_ecc;

		}

		Poll|=POLST; //status is valid

		StatusLen=2;
		Status[0]=0x2;
		//Status[1]=0x0;
		//Status[2]=0x0;
		Status[1]=XbusStatus;


		break;
	case 0x3:
		// spin down
		//opera status request = 0 // not used in opera
		//status 4 bytes
		//xx xx xx XS  (xs=xbus status)
		if((XbusStatus&CDST_TRAY) && (XbusStatus&CDST_DISC))
		{
			XbusStatus&=~CDST_SPIN;
			XbusStatus|=CDST_RDY;
			MEIStatus=MEI_CDROM_no_error;

		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus|=CDST_RDY;
			MEIStatus=MEI_CDROM_recv_ecc;

		}

		Poll|=POLST; //status is valid

		StatusLen=2;
		Status[0]=0x3;
		//Status[1]=0x0;
		//Status[2]=0x0;
		Status[1]=XbusStatus;

		break;
	case 0x4:
		//diagnostics
		// not used in opera
         //04 00 ll-bb-aa 00 00.
         //04 02 mm-ss-ff 00 00.
		 //status 4 bytes
		 //xx S1 S2 XS
//		sprintf(str,"#CDROM 0x4 Diagnostic!!!\n");
//		CDebug::DPrint(str);

		break;
	case 0x6:
		// eject disc
		//opera status request = 0
		//status 4 bytes
		//xx xx xx XS
		// 1b command of scsi
		//emulation ---
		// Execute EJECT command;
		// Check Sense, update PollRegister (if medium present)
		XbusStatus&=~CDST_TRAY;
		XbusStatus&=~CDST_DISC;
		XbusStatus&=~CDST_SPIN;
		XbusStatus&=~CDST_2X;
		XbusStatus&=~CDST_ERRO;
		XbusStatus|=CDST_RDY;
		Poll|=POLST; //status is valid
		Poll&=~POLMA;
		MEIStatus=MEI_CDROM_no_error;

		StatusLen=2;
		Status[0]=0x6;
		//Status[1]=0x0;
		//Status[2]=0x0;
		Status[1]=XbusStatus;

	/*	ClearCDB();
		CDB[0]=0x1b;
		CDB[4]=0x2;
		CDBLen=12;
*/



		break;
	case 0x7:
		// inject disc
		//opera status request = 0
		//status 4 bytes
		//xx xx xx XS
		//1b command of scsi
//		sprintf(str,"#CDROM 0x7 INJECT!!!\n");
//		CDebug::DPrint(str);

		break;
	case 0x8:
		// abort !!!
		//opera status request = 31
		//status 4 bytes
		//xx xx xx XS
		//
		StatusLen=33;
		Status[0]=0x8;
		for(i=1;i<32;i++)
			Status[i]=0;
		Status[32]=XbusStatus;

		XbusStatus|=CDST_RDY;
		MEIStatus=MEI_CDROM_no_error;


		break;
	case 0x9:
		// mode set
		//09 MM nn 00 00 00 00 // 2048 or 2340 transfer size
		// to be checked -- wasn't called even once
		// 2nd byte is type selector
		// MM = mode nn= value
		//opera status request = 0
		//status 4 bytes
		//xx xx xx XS
		// to check!!!

	//	if((XbusStatus&CDST_TRAY) && (XbusStatus&CDST_DISC))
	//	{
			XbusStatus|=CDST_RDY;
			MEIStatus=MEI_CDROM_no_error;

			//CDMode[Command[1]]=Command[2];
	//	}
	//	else
	//	{
	//		XbusStatus|=CDST_ERRO;
	//		XbusStatus&=~CDST_RDY;
	//	}

		Poll|=POLST; //status is valid

		StatusLen=2;
		Status[0]=0x9;
		Status[1]=XbusStatus;



		break;
	case 0x0a:
		// reset
		//not used in opera
		//status 4 bytes
		//xx xx xx XS
//		sprintf(str,"#CDROM 0xa RESET!!!\n");
//		CDebug::DPrint(str);
		break;
	case 0x0b:
		// flush
		//opera status request = 31
		//status 4 bytes
		//xx xx xx XS
		//returns data
		//flush all internal buffer
		//1+31+1
		XbusStatus|=CDST_RDY;
		StatusLen=33;
		Status[0]=0xb;
		for(i=1;i<32;i++)
			Status[i]=0;
		Status[32]=XbusStatus;

		//XbusStatus|=CDST_RDY;
		MEIStatus=MEI_CDROM_no_error;



		break;
	case 0x10:
		//Read Data !!!
		//10 01 00 00 00 00 01 // read 0x0 blocks from MSF=1.0.0
		//10 xx-xx-xx nn-nn fl.
		//00 01 02 03 04 05 06
		//reads nn blocks from xx
		//fl=0 xx="lba"
		//fl=1 xx="msf"
		//block= 2048 bytes
		//opera status request = 0
		//status 4 bytes
		//xx xx xx XS
		//returns data
		// here we go


		//olddataptr=DataLen;
		if((XbusStatus&CDST_TRAY)&&(XbusStatus&CDST_DISC)&&(XbusStatus&CDST_SPIN))
		{
			XbusStatus|=CDST_RDY;
			//CDMode[Command[1]]=Command[2];
			StatusLen=2;
			Status[0]=0x10;
			//Status[1]=0x0;
			//Status[2]=0x0;
			Status[1]=XbusStatus;

			//if(Command[6]==Address_Abs_MSF)
			{
				DISC.curabsmsf[0]=(Command[1]);
				DISC.curabsmsf[1]=(Command[2]);
				DISC.curabsmsf[2]=(Command[3]);
				DISC.tempmsf[0]=DISC.curabsmsf[0];
				DISC.tempmsf[1]=DISC.curabsmsf[1];
				DISC.tempmsf[2]=DISC.curabsmsf[2];
				MSF2LBA();


				//if(fiso!=NULL)
				//	fseek(fiso,DISC.templba*2048+iso_off_from_begin,SEEK_SET);
				{
                                                curr_sector=DISC.templba;
						_3do_OnSector(DISC.templba);
				}
					//fseek(fiso,DISC.templba*2048,SEEK_SET);
					//fseek(fiso,DISC.templba*2336,SEEK_SET);
			}





			olddataptr=(Command[5]<<8)+Command[6];
			//olddataptr=olddataptr*2048; //!!!
			Requested=olddataptr;


				if(Requested)
				{
                                        _3do_OnSector(curr_sector++);
					_3do_Read2048(Data);
                                        DataLen=REQSIZE;
                                        Requested--;
				}
                                else DataLen=0;

			Poll|=POLDT;
			Poll|=POLST;
			MEIStatus=MEI_CDROM_no_error;


		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus&=~CDST_RDY;
			Poll|=POLST; //status is valid
			StatusLen=2;
			Status[0]=0x10;
			//Status[1]=0x0;
			//Status[2]=0x0;
			Status[1]=XbusStatus;
			MEIStatus=MEI_CDROM_recv_ecc;

		}


		break;
	case 0x80:
		// data path chech
		//opera status request = 2
		//MKE =2
		// status 4 bytes
		// 80 AA 55 XS
		XbusStatus|=CDST_RDY;
		StatusLen=4;
		Status[0]=0x80;
		Status[1]=0xaa;
		Status[2]=0x55;
		Status[3]=XbusStatus;
		Poll|=POLST;
		MEIStatus=MEI_CDROM_no_error;


		break;
	case 0x82:
		//read error (get last status???)
		//opera status request = 8 ---- tests status req=9?????
		//MKE = 8!!!
		//00
		//11
		//22   Current Status //MKE / Opera???
		//33
		//44
		//55
		//66
		//77
		//88   Current Status //TEST
		Status[0]=0x82;
		Status[1]=MEIStatus;
		Status[2]=MEIStatus;
		Status[3]=MEIStatus;
		Status[4]=MEIStatus;
		Status[5]=MEIStatus;
		Status[6]=MEIStatus;
		Status[7]=MEIStatus;
		Status[8]=MEIStatus;
		XbusStatus|=CDST_RDY;
		Status[9]=XbusStatus;
		//Status[9]=XbusStatus; // 1 == disc present
		StatusLen=10;
		Poll|=POLST;
		//Poll|=0x80; //MDACC



		break;
	case 0x83:
		//read id
		//opera status request = 10
		//status 12 bytes (3 words)
		//MEI text + XS
		//00 M E I 1 01 00 00 00 00 00 XS
		XbusStatus|=CDST_RDY;
		StatusLen=12;
		Status[0]=0x83;
		Status[1]=0x00;//manufacture id
		Status[2]=0x10;//10
		Status[3]=0x00;//MANUFACTURE NUM
		Status[4]=0x01;//01
		Status[5]=00;
		Status[6]=00;
		Status[7]=0;//REVISION NUMBER:
		Status[8]=0;
		Status[9]=0x00;//FLAG BYTES
		Status[10]=0x00;
		Status[11]=XbusStatus;//DEV.DRIVER SIZE
		//Status[11]=XbusStatus;
		//Status[12]=XbusStatus;
		Poll|=POLST;
		MEIStatus=MEI_CDROM_no_error;

		break;
	case 0x84:
		//mode sense
		//not used in opera
		//84 mm 00 00 00 00 00.
		//status 4 bytes
		//xx S1 S2 XS
		//xx xx nn XS
		//
		StatusLen=4;
		Status[0]=0x0;
		Status[1]=0x0;
		Status[2]=0x0;

		if((XbusStatus&CDST_TRAY) && (XbusStatus&CDST_DISC))
		{
			XbusStatus|=CDST_RDY;
			//CDMode[Command[1]]=Command[2];
			//Status[2]=CDMode[Command[1]];
		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus&=~CDST_RDY;
		}

		Poll|=POLST; //status is valid

		Status[3]=XbusStatus;



		break;
	case 0x85:
		//read capacity
		//status 8 bytes
		//opera status request = 6
		//cc cc cc cc cc cc cc XS
		//data?
		//00 85
		//11 mm  total
		//22 ss  total
		//33 ff  total
		//44 ??
		//55 ??
		//66 ??
		if((XbusStatus&CDST_TRAY)&&(XbusStatus&CDST_DISC)&&(XbusStatus&CDST_SPIN))
		{
			StatusLen=8;//CMD+status+DRVSTAT
			Status[0]=0x85;
			Status[1]=0;
			Status[2]=DISC.totalmsf[0]; //min
			Status[3]=DISC.totalmsf[1]; //sec
			Status[4]=DISC.totalmsf[2]; //fra
			Status[5]=0x00;
			Status[6]=0x00;
			XbusStatus|=CDST_RDY;
			Status[7]=XbusStatus;
			Poll|=POLST;
			MEIStatus=MEI_CDROM_no_error;


		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus&=~CDST_RDY;
			StatusLen=2;//CMD+status+DRVSTAT
			Status[0]=0x85;
			Status[1]=XbusStatus;
			Poll|=POLST;
			MEIStatus=MEI_CDROM_recv_ecc;

		}

		break;
	case 0x86:
		//read header
		// not used in opera
		// 86 00 ll-bb-aa 00 00.
		// 86 02 mm-ss-ff 00 00.
		// status 8 bytes
		// data?
//		sprintf(str,"#CDROM 0x86 READ HEADER!!!\n");
//		CDebug::DPrint(str);

		break;
	case 0x87:
		//read subq
		//opera status request = 10
		//87 fl 00 00 00 00 00
		//fl=0 "lba"
		//fl=1 "msf"
		//
		//11 00 (if !=00 then break)
		//22 Subq_ctl_adr=swapnibles(_11_)
		//33 Subq_trk = but2bcd(_22_)
		//44 Subq_pnt_idx=byt2bcd(_33_)
		//55 mm run tot
		//66 ss run tot
		//77 ff run tot
		//88 mm run trk
		//99 ss run trk
		//aa ff run trk

		if((XbusStatus&CDST_TRAY)&&(XbusStatus&CDST_DISC)&&(XbusStatus&CDST_SPIN))
		{
			StatusLen=12;//CMD+status+DRVSTAT
			Status[0]=0x87;
			Status[1]=0;//DISC.totalmsf[0]; //min
			Status[2]=0; //sec
			Status[3]=0; //fra
			Status[4]=0;
			Status[5]=0;
			XbusStatus|=CDST_RDY;
			Status[6]=0x0;
			Status[7]=0x0;
			Status[8]=0x0;
			Status[9]=0x0;
			Status[10]=0x0;
			Status[11]=XbusStatus;
			Poll|=POLST;
			MEIStatus=MEI_CDROM_no_error;


		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus&=~CDST_RDY;
			StatusLen=2;//CMD+status+DRVSTAT
			Status[0]=0x85;
			Status[1]=XbusStatus;
			Poll|=POLST;
			MEIStatus=MEI_CDROM_recv_ecc;

		}




		break;
	case 0x88:
		//read upc
		// not used in opera
		//88 00 ll-bb-aa 00 00
		//88 02 mm-ss-ff 00 00
		//status 20(16) bytes
		//data?
//		sprintf(str,"#CDROM 0x88 READ UPC!!!\n");
//		CDebug::DPrint(str);

		break;
	case 0x89:
		//read isrc
		// not used in opera
		//89 00 ll-bb-aa 00 00
		//89 02 mm-ss-ff 00 00
		//status 16(15) bytes
		//data?

//		sprintf(str,"#CDROM 0x89 READ ISRC!!!\n");
//		CDebug::DPrint(str);

		break;
	case 0x8a:
		//read disc code
		//ignore it yet...
		////opera status request = 10
		// 8a 00 00 00 00 00 00
		//status 10 bytes
		//????? which code???
		if((XbusStatus&CDST_TRAY)&&(XbusStatus&CDST_DISC)&&(XbusStatus&CDST_SPIN))
		{
			StatusLen=12;//CMD+status+DRVSTAT
			Status[0]=0x8a;
			Status[1]=0;//DISC.totalmsf[0]; //min
			Status[2]=0; //sec
			Status[3]=0; //fra
			Status[4]=0;
			Status[5]=0;
			XbusStatus|=CDST_RDY;
			Status[6]=0x0;
			Status[7]=0x0;
			Status[8]=0x0;
			Status[9]=0x0;
			Status[10]=0x0;
			Status[11]=XbusStatus;
			Poll|=POLST;
			MEIStatus=MEI_CDROM_no_error;


		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus&=~CDST_RDY;
			StatusLen=2;//CMD+status+DRVSTAT
			Status[0]=0x85;
			Status[1]=XbusStatus;
			Poll|=POLST;
			MEIStatus=MEI_CDROM_recv_ecc;

		}



		break;
	case 0x8b:
		//MKE !!!v the same
		//read disc information
		//opera status request = 6
		//8b 00 00 00 00 00 00
		//status 8(6) bytes
		//read the toc descritor
		//00 11 22 33 44 55 XS
		//00= 8b //command code
		//11= Disc ID /// XA_BYTE
		//22= 1st track#
		//33= last track#
		//44= minutes
		//55= seconds
		//66= frames


		StatusLen=8;//6+1 + 1 for what?
		Status[0]=0x8b;
		if(XbusStatus&(CDST_TRAY|CDST_DISC|CDST_SPIN))
		{
			Status[1]=DISC.discid;
			Status[2]=DISC.firsttrk;
			Status[3]=DISC.lasttrk;
			Status[4]=DISC.totalmsf[0]; //minutes
			Status[5]=DISC.totalmsf[1]; //seconds
			XbusStatus|=CDST_RDY;
			Status[6]=DISC.totalmsf[2]; //frames
			MEIStatus=MEI_CDROM_no_error;
			Status[7]=XbusStatus;
		}
		else
		{
			StatusLen=2;//6+1 + 1 for what?
			XbusStatus|=CDST_ERRO;
			MEIStatus=MEI_CDROM_recv_ecc;
			Status[1]=XbusStatus;
		}

		Poll|=POLST; //status is valid

		break;
	case 0x8c:
		//read toc
		//MKE !!!v the same
		//opera status request = 8
		//8c fl nn 00 00 00 00 // reads nn entry
		//status 12(8) bytes
		//00 11 22 33 44 55 66 77 XS
		//00=8c
		//11=reserved0; // NIX BYTE
		//22=addressAndControl; //TOCENT_CTL_ADR=swapnibbles(11) ??? UPCCTLADR=_10_ | x02 (_11_ &F0 = _10_)
		//33=trackNumber;  //TOC_ENT NUMBER
		//44=reserved3;    //TOC_ENT FORMAT
		//55=minutes;     //TOCENT ADRESS == 0x00445566
		//66=seconds;
		//77=frames;
		//88=reserved7;
		StatusLen=10;//CMD+status+DRVSTAT
		Status[0]=0x8c;
		if(XbusStatus&(CDST_TRAY|CDST_DISC|CDST_SPIN))
		{
			Status[1]=DISC.DiscTOC[Command[2]].res0;
			Status[2]=DISC.DiscTOC[Command[2]].CDCTL;
			Status[3]=DISC.DiscTOC[Command[2]].TRKNUM;
			Status[4]=DISC.DiscTOC[Command[2]].res1;
			Status[5]=DISC.DiscTOC[Command[2]].mm; //min
			XbusStatus|=CDST_RDY;
			Status[6]=DISC.DiscTOC[Command[2]].ss; //sec
			Status[7]=DISC.DiscTOC[Command[2]].ff; //frames
			Status[8]=DISC.DiscTOC[Command[2]].res2;
			MEIStatus=MEI_CDROM_no_error;
			Status[9]=XbusStatus;

		}
		else
		{
			StatusLen=2;
			XbusStatus|=CDST_ERRO;
			MEIStatus=MEI_CDROM_recv_ecc;
			Status[1]=XbusStatus;
		}

		Poll|=POLST;




		break;
	case 0x8d:
		//read session information
		//MKE !!!v the same
		//opera status request = 6
		//status 8(6)
		//00 11 22 33 44 55 XS ==
		//00=8d
		//11=valid;  // 0x80 = MULTISESS
		//22=minutes;
		//33=seconds;
		//44=frames;
		//55=rfu1; //ignore
		//66=rfu2  //ignore

		StatusLen=8;//CMD+status+DRVSTAT
		Status[0]=0x8d;
		if((XbusStatus&CDST_TRAY) && (XbusStatus&CDST_DISC))
		{
			Status[1]=0x00;
			Status[2]=0x0;//DISC.sesmsf[0];//min
			Status[3]=0x2;//DISC.sesmsf[1];//sec
			Status[4]=0x0;//DISC.sesmsf[2];//fra
			Status[5]=0x00;
			XbusStatus|=CDST_RDY;
			Status[6]=0x00;
			Status[7]=XbusStatus;
			MEIStatus=MEI_CDROM_no_error;

		}
		else
		{
			StatusLen=2;//CMD+status+DRVSTAT
			XbusStatus|=CDST_ERRO;
			Status[1]=XbusStatus;
			MEIStatus=MEI_CDROM_recv_ecc;

		}


		Poll|=POLST;




		break;
	case 0x8e:
		//read device driver
		break;
	case 0x93:
		//?????
		StatusLen=4;
		Status[0]=0x0;
		Status[1]=0x0;
		Status[2]=0x0;

		if((XbusStatus&CDST_TRAY) && (XbusStatus&CDST_DISC))
		{
			XbusStatus|=CDST_RDY;
			//CDMode[Command[1]]=Command[2];
//			Status[2]=CDMode[Command[1]];
		}
		else
		{
			XbusStatus|=CDST_ERRO;
			XbusStatus|=CDST_RDY;
		}

		Poll|=POLST; //status is valid

		Status[3]=XbusStatus;

		break;
	default:
		// error!!!
		//sprintf(str,"#CDROM %x!!!\n",Command[0]);
		//CDebug::DPrint(str);
		break;
	}

}

unsigned char cdrom_Device::BCD2BIN(unsigned char in)
{
	return ((in>>4)*10+(in&0x0F));
}

unsigned char cdrom_Device::BIN2BCD(unsigned char in)
{
	return((in/10)<<4)|(in%10);
}

void cdrom_Device::MSF2BLK()
{


	DISC.tempblk=(DISC.tempmsf[0] * 60 + DISC.tempmsf[1]) * 75 + DISC.tempmsf[2] - 150;
	if (DISC.tempblk<0)
		DISC.tempblk=0; //??

}

void cdrom_Device::BLK2MSF()
{
	unsigned int mm;
	DISC.tempmsf[0]=(DISC.tempblk+150) / (60*75);
	mm= (DISC.tempblk+150)%(60*75);
	DISC.tempmsf[1]=mm/75;
	DISC.tempmsf[2]=mm%75;
}


void cdrom_Device::LBA2MSF()
{
 		DISC.templba+=150;
		DISC.tempmsf[0]=DISC.templba/(60*75);
		DISC.templba%=(60*75);
		DISC.tempmsf[1]=DISC.templba/75;
		DISC.tempmsf[2]=DISC.templba%75;
}

void cdrom_Device::MSF2LBA()
{
	DISC.templba=(DISC.tempmsf[0] * 60 + DISC.tempmsf[1]) * 75 + DISC.tempmsf[2] - 150;
	if(DISC.templba<0)
		DISC.templba=0;

}

unsigned char * cdrom_Device::GetDataPtr()
{
	return Data;
}

unsigned int cdrom_Device::GetDataLen()
{
	return DataLen;
}

void cdrom_Device::ClearDataPoll(unsigned int len)
{
	if((int)len<=DataLen)
	{
		if(DataLen>0)
		{
			DataLen-=len;
			if(DataLen>0)
				memcpy(Data,Data+4,len);
			else
			{
				Poll&=~POLDT;
			}
		}
	}
	else
	{
		Poll&=~POLDT;

	}
}


bool cdrom_Device::InitCD()
{
	unsigned int filesize=0;



		//fseek(fiso,0,SEEK_END);
                curr_sector=0;
		_3do_OnSector(0);
		//filesize=800000000;//ftell(fiso);

		filesize=_3do_DiscSize()+150;


	//sprintf(str,"FILESIZE=0x%x\n",filesize);
	//CDebug::DPrint(str);

	XbusStatus=0;
	//XBPOLL=POLSTMASK|POLDTMASK|POLMAMASK|POLREMASK;
	Poll=0xf;
	XbusStatus|=CDST_TRAY; //Inject the disc
	XbusStatus|=CDST_RDY;
	XbusStatus|=CDST_DISC;
	XbusStatus|=CDST_SPIN;

	MEIStatus=MEI_CDROM_no_error;

	DISC.firsttrk=1;
	DISC.lasttrk=1;
	DISC.curabsmsf[0]=0;
	DISC.curabsmsf[1]=2;
	DISC.curabsmsf[2]=0;

	DISC.DiscTOC[1].CDCTL=CD_CTL_DATA_TRACK|CD_CTL_Q_NONE;//|CD_CTL_COPY_PERMITTED;
	DISC.DiscTOC[1].TRKNUM=1;
	DISC.DiscTOC[1].mm=0;
	DISC.DiscTOC[1].ss=2;
	DISC.DiscTOC[1].ff=0;

	DISC.firsttrk=1;
	DISC.lasttrk=1;
	DISC.discid=MEI_DISC_DA_OR_CDROM;

	DISC.templba=filesize;
	LBA2MSF();
	DISC.totalmsf[0]=DISC.tempmsf[0];
	DISC.totalmsf[1]=DISC.tempmsf[1];
	DISC.totalmsf[2]=DISC.tempmsf[2];

	//sprintf(str,"##ISO M=0x%x  S=0x%x  F=0x%x\n",DISC.totalmsf[0],DISC.totalmsf[1],DISC.totalmsf[2]);
	//CDebug::DPrint(str);


	DISC.templba=filesize-150;
	LBA2MSF();
	DISC.sesmsf[0]=DISC.tempmsf[0];
	DISC.sesmsf[1]=DISC.tempmsf[1];
	DISC.sesmsf[2]=DISC.tempmsf[2];
	return false;
}

unsigned char  *  cdrom_Device::GetBytes(unsigned int len)
{
	//unsigned char * retmem;

	//retmem=Data;
        (void)len;

	return Data;
}

unsigned int cdrom_Device::GedWord()
{
	unsigned int res;
	res=0;


	if(DataLen>0)
	{
		//res=(unsigned char)Data[0];
		//res
		res=(Data[0]<<24)+(Data[1]<<16)+(Data[2]<<8)+Data[3];
		if(DataLen<3)
		{
			DataLen--;
			if(DataLen>0)
				memcpy(Data,Data+1,DataLen);
			else
			{
				Poll&=~POLDT;
				DataLen=0;
			}
			DataLen--;
			if(DataLen>0)
				memcpy(Data,Data+1,DataLen);
			else
			{
				Poll&=~POLDT;
				DataLen=0;
			}
			DataLen--;
			if(DataLen>0)
				memcpy(Data,Data+1,DataLen);
			else
			{
				Poll&=~POLDT;
				DataLen=0;
			}
		}
		else
		{
			//DataLen-=4;
			{
				memcpy(Data,Data+4,DataLen-4);
				DataLen-=4;
			}

			if(DataLen<=0)
			{
				DataLen=0;
				Poll&=~POLDT;
			}
		}

	}

	return res;
}








//plugins----------------------------------------------------------------------------------
cdrom_Device isodrive;

void* _xbplug_MainDevice(int proc, void* data)
{
 uint32 tmp;
 //void* xfisonew;
 switch(proc)
 {
	case XBP_INIT:
		isodrive.Init();
		return (void*)true;
	case XBP_RESET:
                isodrive.Init();
                if(_3do_DiscSize())
		        isodrive.InitCD();
		break;
	case XBP_SET_COMMAND:
		isodrive.SendCommand((uintptr_t)data);
		break;
	case XBP_FIQ:
		return (void*)isodrive.TestFIQ();
	case XBP_GET_DATA:
		return (void*)isodrive.GetDataFifo();
	case XBP_GET_STATUS:
		return (void*)isodrive.GetStatusFifo();
	case XBP_SET_POLL:
		isodrive.SetPoll((uintptr_t)data);
		break;
	case XBP_GET_POLL:
		return (void*)isodrive.GetPoll();
	case XBP_DESTROY:
		break;
	case XBP_GET_SAVESIZE:
		tmp=sizeof(cdrom_Device);
		return (void*)tmp;
	case XBP_GET_SAVEDATA:
		memcpy(data,&isodrive,sizeof(cdrom_Device));
		break;
	case XBP_SET_SAVEDATA:
		memcpy(&isodrive,data,sizeof(cdrom_Device));
		return (void*)1;
 };

 return NULL;
}




