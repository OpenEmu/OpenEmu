/*
    genio.c
    I/O controller chip emulation
*/

#include "shared.h"
#include "input/gamepad.h"
#include "input/mouse.h"

namespace MDFN_IEN_MD
{

static bool is_pal;
static bool is_overseas;
static bool is_overseas_reported;
static bool is_pal_reported;

// 3 internal ports
enum port_names {PORT_A = 0, PORT_B, PORT_C, PORT_MAX};
enum device_names {DEVICE_NONE = 0, DEVICE_MS2B, DEVICE_MD3B, DEVICE_MD6B, DEVICE_MM};

static void SetDevice(int i, int type);

static MD_Input_Device *port[PORT_MAX];

static InputDeviceInfoStruct InputDeviceInfo[] =
{
 // None
 {
  "none",
  "none",
  NULL,
  0,
  NULL
 },

 {
  "gamepad2",
  "2-Button Gamepad",
  NULL,
  sizeof(Gamepad2IDII) / sizeof(InputDeviceInputInfoStruct),
  Gamepad2IDII,
 },

 {
  "gamepad",
  "3-Button Gamepad",
  NULL,
  sizeof(GamepadIDII) / sizeof(InputDeviceInputInfoStruct),
  GamepadIDII,
 },

 {
  "gamepad6",
  "6-Button Gamepad",
  NULL,
  sizeof(Gamepad6IDII) / sizeof(InputDeviceInputInfoStruct),
  Gamepad6IDII,
 },

 {
  "megamouse",
  "Sega Mega Mouse",
  NULL,
  sizeof(MegaMouseIDII) / sizeof(InputDeviceInputInfoStruct),
  MegaMouseIDII,
 },

};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "port1", "Port 1", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port2", "Port 2", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" }
};

InputInfoStruct MDInputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


void MDIO_Init(bool overseas, bool PAL, bool overseas_reported, bool PAL_reported)
{
 is_overseas = overseas;
 is_pal = PAL;
 is_overseas_reported = overseas_reported;
 is_pal_reported = PAL_reported;

 for(int i = 0; i < PORT_MAX; i++)
  SetDevice(i, DEVICE_NONE);
}

uint8 PortData[3];
uint8 PortCtrl[3];
uint8 PortTxData[3];
uint8 PortSCtrl[3];

void gen_io_reset(void)
{
 for(int i = 0; i < 3; i++)
 {
  PortData[i] = 0x7F;
  PortCtrl[i] = 0x00;
  PortTxData[i] = 0xFF;
  PortSCtrl[i] = 0x00;

  port[i]->Write(0x00);
 }
 PortTxData[2] = 0xFB;
}

/*--------------------------------------------------------------------------*/
/* I/O chip functions                                                       */
/*--------------------------------------------------------------------------*/

void gen_io_w(int offset, int value)
{
    //printf("I/O Write: %04x:%04x, %d @ %08x\n", offset, value, md_timestamp, C68k_Get_PC(&Main68K));

    switch(offset)
    {
        case 0x01: /* Port A Data */
	case 0x02: /* Port B Data */
	case 0x03: /* Port C Data */
	    {
	     int wp = offset - 0x01;
             PortData[wp] = value;
             port[wp]->Write(value & PortCtrl[wp] & 0x7F);
	    }
            break;

        case 0x04: /* Port A Ctrl */
        case 0x05: /* Port B Ctrl */
        case 0x06: /* Port C Ctrl */
	    {
	     int wp = offset - 0x04;

	     PortCtrl[wp] = value;
             port[wp]->Write(value & PortCtrl[wp] & 0x7F);
	    }
            break;

        case 0x07: /* Port A TxData */
        case 0x0A: /* Port B TxData */
        case 0x0D: /* Port C TxData */
	    PortTxData[(offset - 0x07) / 3] = value;
            break;

        case 0x09: /* Port A S-Ctrl */
        case 0x0C: /* Port B S-Ctrl */
        case 0x0F: /* Port C S-Ctrl */
	    PortSCtrl[(offset - 0x09) / 3] = value & 0xF8;
            break;
    }
}

int gen_io_r(int offset)
{
    uint8 temp;
    uint8 has_scd = MD_IsCD ? 0x00 : 0x20;
    uint8 gen_ver = 0x00; /* Version 0 hardware */

    //printf("I/O Read: %04x, %d @ %08x\n", offset, md_timestamp, C68k_Get_PC(&Main68K));
    switch(offset)
    {
        case 0x00: /* Version */
	    temp = 0x00;
	    if(is_overseas_reported)
	     temp |= 0x80;
	    if(is_pal_reported)
	     temp |= 0x40;
            return (temp | has_scd | gen_ver);
            break;

        case 0x01: /* Port A Data */
        case 0x02: /* Port B Data */
        case 0x03: /* Port C Data */
	    {
	     int wp = offset - 0x01;

             return(PortData[wp] & (0x80 | PortCtrl[wp])) | (port[wp]->Read() & ~PortCtrl[wp]);
	    }

        case 0x04: /* Port A Ctrl */
        case 0x05: /* Port B Ctrl */
        case 0x06: /* Port C Ctrl */
            return(PortCtrl[offset - 0x04]);

        case 0x07: /* Port A TxData */
        case 0x0A: /* Port B TxData */
        case 0x0D: /* Port C TxData */
            return(PortTxData[(offset - 0x07) / 3]);

        case 0x09: /* Port A S-Ctrl */
        case 0x0C: /* Port B S-Ctrl */
        case 0x0F: /* Port C S-Ctrl */
            return(PortSCtrl[(offset - 0x09) / 3]);
    }

    printf("Unmapped I/O Read: %04x\n", offset);
    return(0x00);
}

/*--------------------------------------------------------------------------*/
/* Null device                                                              */
/*--------------------------------------------------------------------------*/
MD_Input_Device::MD_Input_Device()
{

}

MD_Input_Device::~MD_Input_Device()
{


}

uint8 MD_Input_Device::Read(void)
{
 return(0x7F);
}

void MD_Input_Device::Write(uint8)
{

}

void MD_Input_Device::Update(const void *data)
{

}

int MD_Input_Device::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 return(1);
}


static void SetDevice(int i, int type)
{
    switch(type)
    {
        case DEVICE_NONE:
	    port[i] = new MD_Input_Device();
            break;

        case DEVICE_MS2B:
	    port[i] = MDInput_MakeMS2B();
            break;

        case DEVICE_MD3B:
	    port[i] = MDInput_MakeMD3B();
            break;

        case DEVICE_MD6B:
	    port[i] = MDInput_MakeMD6B();
            break;

        case DEVICE_MM:
	    port[i] = MDInput_MakeMegaMouse();
            break;
    }
}

static void *data_ptr[8];

void MDINPUT_Frame(void)
{
 for(int i = 0; i < 2; i++)
 {
  port[i]->Update(data_ptr[i]);
 }
}

void MDINPUT_SetInput(int aport, const char *type, void *ptr)
{
 int itype = 0;

 if(!strcasecmp(type, "none"))
 {
  itype = DEVICE_NONE;
 }
 if(!strcasecmp(type, "gamepad"))
 {
  itype = DEVICE_MD3B;
 }
 else if(!strcasecmp(type, "gamepad6"))
 {
  itype = DEVICE_MD6B;
 }
 else if(!strcasecmp(type, "gamepad2"))
 {
  itype = DEVICE_MS2B;
 }
 else if(!strcasecmp(type, "megamouse"))
 {
  itype = DEVICE_MM;
 }

 data_ptr[aport] = ptr;
 SetDevice(aport, itype);
}

int MDINPUT_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(PortData, 3),
  SFARRAY(PortCtrl, 3),
  SFARRAY(PortTxData, 3),
  SFARRAY(PortSCtrl, 3),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "IO");

 ret &= port[0]->StateAction(sm, load, data_only, "PRTA");
 ret &= port[1]->StateAction(sm, load, data_only, "PRTB");
 ret &= port[2]->StateAction(sm, load, data_only, "PRTC");

 if(load)
 {

 }
 return(ret);
}


}
