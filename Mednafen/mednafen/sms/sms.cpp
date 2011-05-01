/*
    sms.c --
    Sega Master System console emulation.
*/
#include "shared.h"

namespace MDFN_IEN_SMS
{

/* SMS context */
sms_t sms;

void sms_writebyte(uint16 A, uint8 V)
{
 if(A >= 0xC000)
  sms.wram[A & 0x1FFF] = V;

 SMS_CartWrite(A, V);
}

uint8 sms_readbyte(uint16 A)
{
 uint8 ret;

 if(A >= 0xC000)
  ret = sms.wram[A & 0x1FFF];
 else
  ret = SMS_CartRead(A);

 return(ret);
}

void sms_init(void)
{
    z80_init();

    /* Default: open bus */
    data_bus_pullup     = 0x00;
    data_bus_pulldown   = 0x00;

    z80_writebyte = sms_writebyte;
    z80_readbyte = sms_readbyte;

    /* Force SMS (J) console type if FM sound enabled */
    if(sms.use_fm)
    {
        sms.console = CONSOLE_SMSJ;
        sms.territory = TERRITORY_DOMESTIC;
        sms.display = DISPLAY_NTSC;
    }

    /* Initialize selected console emulation */
    switch(sms.console)
    {
        case CONSOLE_SMS:
            z80_writeport = sms_port_w;
            z80_readport = sms_port_r;
            break;

        case CONSOLE_SMSJ:
            z80_writeport = smsj_port_w;
            z80_readport = smsj_port_r;
            break;
  
        case CONSOLE_SMS2:
            z80_writeport = sms_port_w;
            z80_readport = sms_port_r;
            data_bus_pullup = 0xFF;
            break;

        case CONSOLE_GG:
            z80_writeport = gg_port_w;
            z80_readport = gg_port_r;
            data_bus_pullup = 0xFF;
            break;

        case CONSOLE_GGMS:
            z80_writeport = ggms_port_w;
            z80_readport = ggms_port_r;
            data_bus_pullup = 0xFF;
            break;

        case CONSOLE_GEN:
        case CONSOLE_MD:
            z80_writeport = md_port_w;
            z80_readport = md_port_r;
            break;

        case CONSOLE_GENPBC:
        case CONSOLE_MDPBC:
            z80_writeport = md_port_w;
            z80_readport = md_port_r;
            data_bus_pullup = 0xFF;
            break;
    }
}

void sms_shutdown(void)
{
    /* Nothing to do */
}

void sms_reset(void)
{
    z80_reset();

    /* Clear SMS context */
    memset(sms.wram,    0, sizeof(sms.wram));

    sms.paused      = 0x00;
    sms.save        = 0x00;
    sms.fm_detect   = 0x00;
    sms.memctrl     = 0xAB;
    sms.ioctrl      = 0xFF;
}

}
