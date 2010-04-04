/*
    vdp.c --
    Video Display Processor (VDP) emulation.
*/
#include "shared.h"
#include "hvc.h"

static const uint8 tms_crom[] =
{
    0x00, 0x00, 0x08, 0x0C,
    0x10, 0x30, 0x01, 0x3C,
    0x02, 0x03, 0x05, 0x0F,
    0x04, 0x33, 0x15, 0x3F
};

/* Mark a pattern as dirty */
#define MARK_BG_DIRTY(addr)                                \
{                                                          \
    int name = (addr >> 5) & 0x1FF;                        \
    if(bg_name_dirty[name] == 0)                           \
    {                                                      \
        bg_name_list[bg_list_index] = name;                \
        bg_list_index++;                                   \
    }                                                      \
    bg_name_dirty[name] |= (1 << ((addr >> 2) & 7));       \
}


/* VDP context */
vdp_t vdp;


/* Initialize VDP emulation */
void vdp_init(void)
{
 memset(&vdp, 0, sizeof(vdp_t));

 vdp.lines_per_frame = (sms.display == DISPLAY_NTSC) ? 262 : 313;

 vdp_reset();
}

void vdp_shutdown(void)
{
    /* Nothing to do */
}


/* Reset VDP emulation */
void vdp_reset(void)
{
 memset(vdp.vram, 0, sizeof(vdp.vram));
 memset(vdp.cram, 0, sizeof(vdp.cram));
 memset(vdp.reg, 0, sizeof(vdp.reg));
 vdp.status = 0;
 vdp.latch = 0;
 vdp.pending = 0;
 vdp.buffer = 0;
 vdp.code = 0;
 vdp.addr = 0;

 vdp.pn = 0;
 vdp.ct = 0;
 vdp.pg = 0;
 vdp.sa = 0;
 vdp.sg = 0;
 vdp.ntab = 0;
 vdp.satb = 0;
 vdp.line = 0;
 vdp.left = 0;
 vdp.height = 192;
 vdp.extended = 0;
 vdp.mode = 0;
 vdp.vint_pending = 0;
 vdp.hint_pending = 0;
 vdp.cram_latch = 0;
 vdp.bd = 0;
}


void viewport_check(void)
{
    int i;

    int m1 = (vdp.reg[1] >> 4) & 1;
    int m3 = (vdp.reg[1] >> 3) & 1;
    int m2 = (vdp.reg[0] >> 1) & 1;
    int m4 = (vdp.reg[0] >> 2) & 1;

    vdp.mode = (m4 << 3 | m3 << 2 | m2 << 1 | m1 << 0);

    // check if this is switching out of tms
    if(!IS_GG)
    {
        if(m4)
        {
            /* Restore SMS palette */
            for(i = 0; i < PALETTE_SIZE; i++)
            {
                palette_sync(i, 1);
            }
        }
        else
        {
            /* Load TMS9918 palette */
            for(i = 0; i < PALETTE_SIZE; i++)
            {
                int r, g, b;
    
                r = (tms_crom[i & 0x0F] >> 0) & 3;
                g = (tms_crom[i & 0x0F] >> 2) & 3;
                b = (tms_crom[i & 0x0F] >> 4) & 3;
        
                r = sms_cram_expand_table[r];
                g = sms_cram_expand_table[g];
                b = sms_cram_expand_table[b];
            
                bitmap.pal.color[i][0] = r;
                bitmap.pal.color[i][1] = g;
                bitmap.pal.color[i][2] = b;
            
                pixel[i] = MAKE_PIXEL(r, g, b);
            
                bitmap.pal.dirty[i] = bitmap.pal.update = 1;
            }
        }
    }

    /* Check for extended modes when M4 and M2 are set */
    if((vdp.reg[0] & 0x06) == 0x06)
    {
        /* Examine M1 and M3 to determine selected mode */
        switch(vdp.reg[1] & 0x18)
        {
            case 0x00: /* 192 */
            case 0x18: /* 192 */
                vdp.height = 192;
                vdp.extended = 0;
                vdp.ntab = (vdp.reg[2] << 10) & 0x3800;
                break;

            case 0x08: /* 240 */
                vdp.height = 240;
                vdp.extended = 2;
                vdp.ntab = ((vdp.reg[2] << 10) & 0x3000) | 0x0700;
                break;

            case 0x10: /* 224 */
                vdp.height = 224;
                vdp.extended = 1;
                vdp.ntab = ((vdp.reg[2] << 10) & 0x3000) | 0x0700;
                break;

        }
    }
    else
    {
        vdp.height = 192;
        vdp.extended = 0;
        vdp.ntab = (vdp.reg[2] << 10) & 0x3800;
    }

    vdp.pn = (vdp.reg[2] << 10) & 0x3C00;
    vdp.ct = (vdp.reg[3] <<  6) & 0x3FC0;
    vdp.pg = (vdp.reg[4] << 11) & 0x3800;
    vdp.sa = (vdp.reg[5] <<  7) & 0x3F80;
    vdp.sg = (vdp.reg[6] << 11) & 0x3800;

    render_bg  = (vdp.mode & 8) ? render_bg_sms  : render_bg_tms;
    render_obj = (vdp.mode & 8) ? render_obj_sms : render_obj_tms;
}


void vdp_reg_w(uint8 r, uint8 d)
{
    /* Store register data */
    vdp.reg[r] = d;

    switch(r)
    {
        case 0x00: /* Mode Control No. 1 */
            if(vdp.hint_pending)
            {
                if(d & 0x10)
		 z80_set_interrupt(TRUE);
                else
		 z80_set_interrupt(FALSE);
            }
            viewport_check();
            break;

        case 0x01: /* Mode Control No. 2 */
            if(vdp.vint_pending)
            {
                if(d & 0x20)
		 z80_set_interrupt(TRUE);
                else
		 z80_set_interrupt(FALSE);
            }
            viewport_check();
            break;

        case 0x02: /* Name Table A Base Address */
            vdp.ntab = (vdp.reg[2] << 10) & 0x3800;
            vdp.pn = (vdp.reg[2] << 10) & 0x3C00;
            viewport_check();
            break;

        case 0x03:
            vdp.ct = (vdp.reg[3] <<  6) & 0x3FC0;
            break;

        case 0x04:
            vdp.pg = (vdp.reg[4] << 11) & 0x3800;
            break;

        case 0x05: /* Sprite Attribute Table Base Address */
            vdp.satb = (vdp.reg[5] << 7) & 0x3F00;
            vdp.sa = (vdp.reg[5] <<  7) & 0x3F80;
            break;

        case 0x06:
            vdp.sg = (vdp.reg[6] << 11) & 0x3800;
            break;

        case 0x07:
            vdp.bd = (vdp.reg[7] & 0x0F);
            break;
    }
}


void vdp_write(int offset, uint8 data)
{
    int index;

    switch(offset & 1)
    {
        case 0: /* Data port */

            vdp.pending = 0;

            switch(vdp.code)
            {
                case 0: /* VRAM write */
                case 1: /* VRAM write */
                case 2: /* VRAM write */
                    index = (vdp.addr & 0x3FFF);
                    if(data != vdp.vram[index])
                    {
                        vdp.vram[index] = data;
                        MARK_BG_DIRTY(vdp.addr);
                    }
                    break;
        
                case 3: /* CRAM write */
                    index = (vdp.addr & 0x1F);
                    if(data != vdp.cram[index])
                    {
                        vdp.cram[index] = data;
                        palette_sync(index, 0);
                    }
                    break;
            }
            vdp.addr = (vdp.addr + 1) & 0x3FFF;
            return;

        case 1: /* Control port */
            if(vdp.pending == 0)
            {
                vdp.addr = (vdp.addr & 0x3F00) | (data & 0xFF);
                vdp.latch = data;
                vdp.pending = 1;
            }
            else
            {
                vdp.pending = 0;
                vdp.code = (data >> 6) & 3;
                vdp.addr = (data << 8 | vdp.latch) & 0x3FFF;

                if(vdp.code == 0)
                {
                    vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
                    vdp.addr = (vdp.addr + 1) & 0x3FFF;
                }
        
                if(vdp.code == 2)
                {
                    int r = (data & 0x0F);
                    int d = vdp.latch;
                    vdp_reg_w(r, d);
                }
            }
            return;
    }
}

uint8 vdp_read(int offset)
{
    uint8 temp;

    switch(offset & 1)
    {
        case 0: /* CPU <-> VDP data buffer */
            vdp.pending = 0;
            temp = vdp.buffer;
            vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
            vdp.addr = (vdp.addr + 1) & 0x3FFF;
            return temp;

        case 1: /* Status flags */
            temp = vdp.status;
            vdp.pending = 0;
            vdp.status = 0;
            vdp.vint_pending = 0;
            vdp.hint_pending = 0;
	    z80_set_interrupt(FALSE);
            return temp;
    }

    /* Just to please the compiler */
    return -1;
}

uint8 vdp_counter_r(int offset)
{
    int pixel;

    switch(offset & 1)
    {
        case 0: /* V Counter */
            return vc_table[sms.display][vdp.extended][vdp.line & 0x1FF];

        case 1: /* H Counter */
            //pixel = (((z80_get_elapsed_cycles() % CYCLES_PER_LINE) / 4) * 3) * 2;
	    // FIXME
            // FIXME
            pixel = (((sms.timestamp % CYCLES_PER_LINE) / 4) * 3) * 2;
            return hc_table[0][(pixel >> 1) & 0x01FF];
    }

    /* Just to please the compiler */
    return -1;
}


/*--------------------------------------------------------------------------*/
/* Game Gear VDP handlers                                                   */
/*--------------------------------------------------------------------------*/

void gg_vdp_write(int offset, uint8 data)
{
    int index;

    switch(offset & 1)
    {
        case 0: /* Data port */
            vdp.pending = 0;
            switch(vdp.code)
            {
                case 0: /* VRAM write */
                case 1: /* VRAM write */
                case 2: /* VRAM write */
                    index = (vdp.addr & 0x3FFF);
                    if(data != vdp.vram[index])
                    {
                        vdp.vram[index] = data;
                        MARK_BG_DIRTY(vdp.addr);
                    }
                    break;
        
                case 3: /* CRAM write */
                    if(vdp.addr & 1)
                    {                    
                        vdp.cram_latch = (vdp.cram_latch & 0x00FF) | ((data & 0xFF) << 8);
                        vdp.cram[(vdp.addr & 0x3E) | (0)] = (vdp.cram_latch >> 0) & 0xFF;
                        vdp.cram[(vdp.addr & 0x3E) | (1)] = (vdp.cram_latch >> 8) & 0xFF;
                        palette_sync((vdp.addr >> 1) & 0x1F, 0);
                    }
                    else
                    {
                        vdp.cram_latch = (vdp.cram_latch & 0xFF00) | ((data & 0xFF) << 0);
                    }
                    break;
            }
            vdp.addr = (vdp.addr + 1) & 0x3FFF;
            return;

        case 1: /* Control port */
            if(vdp.pending == 0)
            {
                vdp.addr = (vdp.addr & 0x3F00) | (data & 0xFF);
                vdp.latch = data;
                vdp.pending = 1;
            }
            else
            {
                vdp.pending = 0;
                vdp.code = (data >> 6) & 3;
                vdp.addr = (data << 8 | vdp.latch) & 0x3FFF;

                if(vdp.code == 0)
                {
                    vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
                    vdp.addr = (vdp.addr + 1) & 0x3FFF;
                }
        
                if(vdp.code == 2)
                {
                    int r = (data & 0x0F);
                    int d = vdp.latch;
                    vdp_reg_w(r, d);
                }
            }
            return;
    }
}

/*--------------------------------------------------------------------------*/
/* MegaDrive / Genesis VDP handlers                                         */
/*--------------------------------------------------------------------------*/

void md_vdp_write(int offset, uint8 data)
{
    int index;

    switch(offset & 1)
    {
        case 0: /* Data port */

            vdp.pending = 0;

            switch(vdp.code)
            {
                case 0: /* VRAM write */
                case 1: /* VRAM write */
                    index = (vdp.addr & 0x3FFF);
                    if(data != vdp.vram[index])
                    {
                        vdp.vram[index] = data;
                        MARK_BG_DIRTY(vdp.addr);
                    }
                    break;
        
                case 2: /* CRAM write */
                case 3: /* CRAM write */
                    index = (vdp.addr & 0x1F);
                    if(data != vdp.cram[index])
                    {
                        vdp.cram[index] = data;
                        palette_sync(index, 0);
                    }
                    break;
            }
            vdp.addr = (vdp.addr + 1) & 0x3FFF;
            return;

        case 1: /* Control port */
            if(vdp.pending == 0)
            {
                vdp.latch = data;
                vdp.pending = 1;
            }
            else
            {
                vdp.pending = 0;
                vdp.code = (data >> 6) & 3;
                vdp.addr = (data << 8 | vdp.latch) & 0x3FFF;

                if(vdp.code == 0)
                {
                    vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
                    vdp.addr = (vdp.addr + 1) & 0x3FFF;
                }
        
                if(vdp.code == 2)
                {
                    int r = (data & 0x0F);
                    int d = vdp.latch;
                    vdp_reg_w(r, d);
                }
            }
            return;
    }
}

/*--------------------------------------------------------------------------*/
/* TMS9918 VDP handlers                                                     */
/*--------------------------------------------------------------------------*/

void tms_write(int offset, int data)
{
    int index;

    switch(offset & 1)
    {
        case 0: /* Data port */

            vdp.pending = 0;

            switch(vdp.code)
            {
                case 0: /* VRAM write */
                case 1: /* VRAM write */
                case 2: /* VRAM write */
                case 3: /* VRAM write */
                    index = (vdp.addr & 0x3FFF);
                    if(data != vdp.vram[index])
                    {
                        vdp.vram[index] = data;
                        MARK_BG_DIRTY(vdp.addr);
                    }
                    break;
            }
            vdp.addr = (vdp.addr + 1) & 0x3FFF;
            return;

        case 1: /* Control port */
            if(vdp.pending == 0)
            {
                vdp.latch = data;
                vdp.pending = 1;
            }
            else
            {
                vdp.pending = 0;
                vdp.code = (data >> 6) & 3;
                vdp.addr = (data << 8 | vdp.latch) & 0x3FFF;

                if(vdp.code == 0)
                {
                    vdp.buffer = vdp.vram[vdp.addr & 0x3FFF];
                    vdp.addr = (vdp.addr + 1) & 0x3FFF;
                }
        
                if(vdp.code == 2)
                {
                    int r = (data & 0x07);
                    int d = vdp.latch;
                    vdp_reg_w(r, d);
                }
            }
            return;
    }
}

static INLINE void ExecuteCycles(int32 count)
{
 sms.cycle_counter += count;

 while(sms.cycle_counter > 0)
 {
  //printf("PC: %04x\n", z80_getpc());

  int32 temp = z80_do_opcode();
  sms.timestamp += temp;
  sms.cycle_counter -= temp;
 }
}


void SMS_VDPRunFrame(int skip_render)
{
    static int iline_table[] = {0xC0, 0xE0, 0xF0};
    int iline;

    text_counter = 0;

    /* End of frame, parse sprites for line 0 on line 261 (VCount=$FF) */
    if(vdp.mode <= 7)
        parse_line(0);

    for(vdp.line = 0; vdp.line < vdp.lines_per_frame;)
    {
        int moohack = 228;

        ExecuteCycles(228 - 32);
        moohack -= 228 - 32;

        iline = iline_table[vdp.extended];

        render_line(vdp.line, skip_render);

        if(vdp.line <= iline)
        {
            vdp.left -= 1;
            if(vdp.left == -1)
            {
                vdp.left = vdp.reg[0x0A];
                vdp.hint_pending = 1;

                ExecuteCycles(16);
                moohack -= 16;

                if(vdp.reg[0x00] & 0x10)
                {
                 z80_set_interrupt(TRUE);
                }
            }
        }
        else
        {
            vdp.left = vdp.reg[0x0A];
        }

        if(vdp.line == iline)
        {
            vdp.status |= 0x80;
            vdp.vint_pending = 1;

            ExecuteCycles(16);
            moohack -= 16;
            if(vdp.reg[0x01] & 0x20)
            {
             z80_set_interrupt(TRUE);
            }
        }

        if(moohack)
         ExecuteCycles(moohack);

        ++vdp.line;

        if(vdp.mode <= 7)
            parse_line(vdp.line);
    }
}

int SMS_VDPStateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAYN(vdp.vram, 0x4000, "vram"),
  SFARRAYN(vdp.cram, 0x40, "cram"),
  SFARRAYN(vdp.reg, 0x10, "reg"),

  SFVARN(vdp.status, "status"),
  SFVARN(vdp.latch, "latch"),
  SFVARN(vdp.pending, "pending"),
  SFVARN(vdp.buffer, "buffer"),
  SFVARN(vdp.code, "code"),
  SFVARN(vdp.addr, "addr"),

  SFVARN(vdp.pn, "pn"),
  SFVARN(vdp.ct, "ct"),
  SFVARN(vdp.pg, "pg"),
  SFVARN(vdp.sa, "sa"),
  SFVARN(vdp.sg, "sg"),
  SFVARN(vdp.ntab, "ntab"),
  SFVARN(vdp.satb, "satb"),
  SFVARN(vdp.line, "line"),
  SFVARN(vdp.left, "left"),
  SFVARN(vdp.height, "height"),
  SFVARN(vdp.extended, "extended"),
  SFVARN(vdp.mode, "mode"),
  SFVARN(vdp.vint_pending, "vint_pending"),
  SFVARN(vdp.hint_pending, "hint_pending"),
  SFVARN(vdp.cram_latch, "cram_latch"),
  SFVARN(vdp.bd, "bd"),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "VDP");

 if(load)
 {
   /* Force full pattern cache update */
   bg_list_index = 0x200;
   for(int i = 0; i < 0x200; i++)
   {
       bg_name_list[i] = i;
       bg_name_dirty[i] = -1;
   }

   /* Restore palette */
   for(int i = 0; i < PALETTE_SIZE; i++)
       palette_sync(i, 1);
 }

 return(ret);
}
