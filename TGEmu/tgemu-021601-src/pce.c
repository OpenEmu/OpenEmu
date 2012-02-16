
#include "shared.h"

/* System memory */
uint8 ram[0x8000];      /* Work RAM */
uint8 cdram[0x10000];   /* CD unit RAM (64k) */
uint8 bram[0x2000];     /* Backup RAM (8K) */
uint8 rom[0x100000];    /* HuCard ROM (1MB) */
uint8 save_bram;        /* 1= BRAM registers were accessed */
#ifdef FAST_MEM
uint8 dummy[0x2000];    /* Dummy block for unknown access */
uint8 *read_ptr[8];     /* Memory read pointers */
uint8 *write_ptr[8];    /* Memory write pointers */
#endif

/* I/O port data */
uint8 joy_sel = 0;
uint8 joy_clr = 0;
uint8 joy_cnt = 0;


/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown functions                                          */
/*--------------------------------------------------------------------------*/

int pce_init(void)
{
#ifdef FAST_MEM
    bank_reset();
#endif
    h6280_reset(0);
    h6280_set_irq_callback(&pce_irq_callback);
    return (1);
}

void pce_reset(void)
{
    joy_sel = joy_clr = joy_cnt = 0;
    memset(ram, 0, 0x8000);
    memset(cdram, 0, 0x10000);
#ifdef FAST_MEM
    memset(dummy, 0, 0x2000);
    bank_reset();
#endif
    //load_file("pce.brm", bram, 0x2000);
    h6280_reset(0);
    h6280_set_irq_callback(&pce_irq_callback);
}

void pce_shutdown(void)
{
    //if(save_bram) save_file("pce.brm", bram, 0x2000);
#ifdef DEBUG
    error("PC:%04X\n", h6280_get_pc());
#endif    
}

/*--------------------------------------------------------------------------*/
/* Memory handlers                                                          */
/*--------------------------------------------------------------------------*/

#ifndef FAST_MEM

void cpu_writeport16(int port, int data)
{
    vdc_w(port, data);
}

void cpu_writemem21(int address, int data)
{
    uint8 page = (address >> 13) & 0xFF;

    /* RAM (F8) */
    if(page == 0xF8 || page == 0xF9 || page == 0xFA || page == 0xFB) {
        ram[(address & 0x7FFF)] = data;
        return;
    }

    /* I/O (FF) */
    if(page == 0xFF) {
        io_page_w(address & 0x1FFF, data);
        return;
    }

    /* CD RAM */
    if((page >= 0x80) && (page <= 0x87)) {
        cdram[(address & 0xFFFF)] = data;
        return;
    }

    /* Backup RAM (F7) */
    if(page == 0xF7) {
        bram[(address & 0x1FFF)] = data;
        return;
    }

#ifdef DEBUG
    error("write %02X to %02X:%04X (%08X)\n", data, page, address & 0x1fff, h6280_get_reg(H6280_PC));
#endif
}

int cpu_readmem21(int address)
{
    uint8 page;

    if(address <= 0x0FFFFF) return (rom[(address)]);

    page = (address >> 13) & 0xFF;

    /* ROM (00-7F) */
    if(page <= 0x7F) return (rom[(address)]);

    /* RAM (F8) */
    if(page == 0xF8 || page == 0xF9 || page == 0xFA || page == 0xFB) return (ram[(address & 0x7FFF)]);

    /* I/O (FF) */
    if(page == 0xFF) return (io_page_r(address & 0x1FFF));

    /* CD RAM */
    if((page >= 0x80) && (page <= 0x87)) return (cdram[(address & 0xFFFF)]);

    /* Backup RAM (F7) */
    if(page == 0xF7) return (bram[(address & 0x1FFF)]);

#ifdef DEBUG
    error("read %02X:%04X (%08X)\n", page, address & 0x1fff, h6280_get_reg(H6280_PC));
#endif
    return (0xFF);
}

#endif

int pce_irq_callback(int irqline)
{
    return (0);
}

/*--------------------------------------------------------------------------*/
/* Hardware page handlers                                                   */
/*--------------------------------------------------------------------------*/

void io_page_w(int address, int data)
{
    switch(address & 0x1C00)
    {
        case 0x0000: /* VDC */
            if(address <= 0x0003) { vdc_w(address, data); return; }
            break;

        case 0x0400: /* VCE */
            if(address <= 0x0405) { vce_w(address, data); return; }
            break;

        case 0x0800: /* PSG */
            if(address <= 0x0809) { psg_w(address, data); return; };
            break;

        case 0x0C00: /* Timer */
            if(address == 0x0C00 || address == 0x0C01) { H6280_timer_w(address & 1, data); return; };
            break;

        case 0x1000: /* I/O */
            if(address == 0x1000) { input_w(data); return; }
            break;

        case 0x1400: /* IRQ control */
            if(address == 0x1402 || address == 0x1403) { H6280_irq_status_w(address & 1, data); return; };
            break;

        case 0x1800: /* CD-ROM */
#ifdef DEBUG
            error("cdrom %04X = %02X\n", address, data);
#endif
            if(address == 0x1807) save_bram = 1;
            break;

        case 0x1C00: /* Expansion */
            break;
    }

#ifdef DEBUG
    error("write %02X to IO %04X (%08X)\n", data, address, h6280_get_reg(H6280_PC));
#endif
}


int io_page_r(int address)
{
    switch(address & 0x1C00)
    {
        case 0x0000: /* VDC */
            if(address <= 0x0003) return (vdc_r(address));
            break;

        case 0x0400: /* VCE */
            if(address <= 0x0405) return (vce_r(address));
            break;

        case 0x0800: /* PSG */
            break;

        case 0x0C00: /* Timer */
            if(address == 0x0C00 || address == 0x0C01) return (H6280_timer_r(address & 1));
            break;

        case 0x1000: /* I/O */
            if(address == 0x1000) return (input_r());
            break;

        case 0x1400: /* IRQ control */
            if(address == 0x1402 || address == 0x1403) return (H6280_irq_status_r(address & 1));
            break;

        case 0x1800: /* CD-ROM */
#ifdef DEBUG
            error("cdrom %04X\n", address);
#endif
            break;

        case 0x1C00: /* Expansion */
            break;
    }

#ifdef DEBUG
    error("read IO %04X (%08X)\n", address, h6280_get_reg(H6280_PC));
#endif
    return (0x00);
}

/*--------------------------------------------------------------------------*/
/* Input routines                                                           */
/*--------------------------------------------------------------------------*/

void input_w(uint8 data)
{
    joy_sel = (data & 1);
    joy_clr = (data >> 1) & 1;
}

uint8 input_r(void)
{
    uint8 temp = 0xFF;

    if(input.pad[joy_cnt] & INPUT_LEFT)   temp &= ~0x80;
    if(input.pad[joy_cnt] & INPUT_DOWN)   temp &= ~0x40;
    if(input.pad[joy_cnt] & INPUT_RIGHT)  temp &= ~0x20;
    if(input.pad[joy_cnt] & INPUT_UP)     temp &= ~0x10;
    if(input.pad[joy_cnt] & INPUT_RUN)    temp &= ~0x08;
    if(input.pad[joy_cnt] & INPUT_SELECT) temp &= ~0x04;
    if(input.pad[joy_cnt] & INPUT_B2)     temp &= ~0x02;
    if(input.pad[joy_cnt] & INPUT_B1)     temp &= ~0x01;

    if(joy_sel & 1) temp >>= 4;
    temp &= 0x0F;

    /* Set D6 for TurboGrafx-16, clear for PC-Engine */
    if(input.system & SYSTEM_TGX) temp |= 0x40;

    return (temp);
}

#ifdef FAST_MEM

void bank_reset(void)
{
    int i;
    for(i = 0; i < 8; i += 1)
    {
        read_ptr[i] = &rom[0x0000];
        write_ptr[i] = &dummy[0x0000];
    }
}

void bank_set(int bank, int value)
{
    /* ROM */
    if(value <= 0x7F) {
        read_ptr[bank] = &rom[(value << 13)];
        write_ptr[bank] = &dummy[0x0000];
        return;
    }

    /* CD RAM */
    if((value >= 0x80) && (value <= 0x87)) {
        read_ptr[bank] = write_ptr[bank] = &cdram[(value & 0x07) << 13];
        return;
    }

    /* RAM */
    if((value >= 0xF8) && (value <= 0xFB)) {
        read_ptr[bank] = write_ptr[bank] = &ram[(value & 0x03) << 13];
        return;
    }

    /* Backup RAM */
    if(value == 0xF7) {
        read_ptr[bank] = write_ptr[bank] = &bram[0x0000];
        return;
    }

    /* I/O page */
    if(value == 0xFF) {
        read_ptr[bank] = write_ptr[bank] = NULL;
        return;
    }

    /* Unknown page */
#ifdef DEBUG
    error("Map unknown page %02X to MMR #%d\n", value, bank);
#endif
    read_ptr[bank] = write_ptr[bank] = &dummy[0x0000];
}

#endif

