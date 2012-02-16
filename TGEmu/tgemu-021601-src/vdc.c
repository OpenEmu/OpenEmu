
#include "shared.h"

#define LOG_DMA     0

uint32 y_offset=0;
uint32 byr=0;
uint8 vram[0x10000];
uint16 reg[0x20];
uint8 objram[0x200];
uint8 status;
uint8 latch;
uint8 addr_inc;
uint8 vram_data_latch = 0;
uint8 dvssr_trigger = 0;
int playfield_shift = 6;
uint32 playfield_col_mask = 0xFF;
uint32 playfield_row_mask = 0x1F;
int disp_width;
int disp_height;
uint32 disp_nt_width;
uint16 *vramw = (uint16 *)&vram[0];
uint16 *objramw = (uint16 *)&objram[0];
int old_width = 0;
int old_height = 0;
int playfield_shift_table[] = {6, 7, 8, 8};
int playfield_row_mask_table[] = {0x1F, 0x3F, 0x7F, 0x7F};


uint8 bg_name_dirty[0x800];
uint16 bg_name_list[0x800];
uint16 bg_list_index;
uint8 bg_pattern_cache[0x20000];

uint16 obj_name_dirty[0x200];
uint16 obj_name_list[0x200];
uint16 obj_list_index;
uint8 obj_pattern_cache[0x80000]; 

#define MARK_BG_DIRTY(addr)                                     \
{                                                               \
    int name = (addr >> 4) & 0x7FF;                             \
    if(bg_name_dirty[name] == 0)                                \
    {                                                           \
        bg_name_list[bg_list_index] = name;                     \
        bg_list_index += 1;                                     \
    }                                                           \
    bg_name_dirty[name] |= (1 << (addr & 0x07));                                 \
}

#define MARK_OBJ_DIRTY(addr)                                    \
{                                                               \
    int name = (addr >> 6) & 0x1FF;                             \
    if(obj_name_dirty[name] == 0)                               \
    {                                                           \
        obj_name_list[obj_list_index] = name;                   \
        obj_list_index += 1;                                    \
    }                                                           \
    obj_name_dirty[name] |= (1 << (addr & 0x0F));               \
}



/*--------------------------------------------------------------------------*/
/* Memory handlers                                                          */
/*--------------------------------------------------------------------------*/

int vdc_r(int offset)
{
    uint8 temp;
    uint8 msb = (offset & 1);

    switch(offset)
    {
        case 0x0000: /* Register latch / status flags */
            temp = status;
            status = 0;
            h6280_set_irq_line(0, CLEAR_LINE);
            return (temp);

        case 0x0002: /* Data port (LSB) */
        case 0x0003: /* Data port (MSB) */
            if(latch == 0x02)
            {
                temp = (vram[((reg[1] << 1) | (msb)) & 0xFFFF]);
                if(msb) reg[1] += addr_inc;
                return (temp);
            }
            break;
    }

    return (0xFF);
}

void vdc_w(int offset, int data)
{
    uint8 msb = (offset & 1);

    switch(offset)
    {
        case 0x0000: /* Register latch / status flags */
            latch = (data & 0x1F);
            break;

        case 0x0002: /* Data port (LSB) */
        case 0x0003: /* Data port (MSB) */

            if(msb)
                reg[latch] = (reg[latch] & 0x00FF) | (data << 8);
            else
                reg[latch] = (reg[latch] & 0xFF00) | (data);

            switch(latch)
            {
                case 0x02: 
                    if(msb)
                    {
                        /* Form complete VRAM word */
                        uint16 vram_word = (data << 8 | vram_data_latch);

                        /* Check if data is new or not */
                        if(vram_word != vramw[(reg[0] & 0x7FFF)])
                        {
                            /* Write data to VRAM */
                            vramw[(reg[0] & 0x7FFF)] = vram_word;

                            /* Mark pattern dirty tables */
                            MARK_BG_DIRTY(reg[0]);
                            MARK_OBJ_DIRTY(reg[0]);
                        }

                        reg[0] += addr_inc;
                    }
                    else
                    {
                        vram_data_latch = data;
                    }
                    break;

                case 0x08:
                    y_offset = byr = (reg[0x08] & 0x1FF);
                    y_offset &= playfield_col_mask;
                    break;

                case 0x05:
                    if(msb) {
                        static uint8 add_tbl[] = {1, 32, 64, 128};
                        addr_inc = add_tbl[(data >> 3) & 3];
                    }
                    break;

                case 0x09:
                    if(!msb) {
                        playfield_shift = playfield_shift_table[(data >> 4) & 3];
                        playfield_row_mask = playfield_row_mask_table[(data >> 4) & 3];
                        playfield_col_mask = ((data >> 6) & 1) ? 0x01FF : 0x00FF;
                    }
                    break;

                case 0x0B:
                    disp_width = (1+(reg[0x0B] & 0x3F)) << 3;
                    disp_nt_width = (disp_width >> 3);

                    if(disp_width != old_width) {
                        bitmap.viewport.ow = bitmap.viewport.w;
                        bitmap.viewport.w = old_width = disp_width;
                        bitmap.viewport.changed = 1;
                    }

                    break;

                case 0x0D:
                    disp_height = 1+(reg[0x0D] & 0x01FF);

                    if(disp_height != old_height) {
                        bitmap.viewport.oh = bitmap.viewport.h;
                        bitmap.viewport.h = old_height = disp_height;
                        bitmap.viewport.changed = 1;
                    }
                    break;

                case 0x12:
                    if(msb) vdc_do_dma();
                    break;

                case 0x13:
                    if(msb) dvssr_trigger = 1;
                    break;
            }
    }
}

/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown routines                                           */
/*--------------------------------------------------------------------------*/

int vdc_init(void)
{
    return (0);
}

void vdc_reset(void)
{
    memset(vram, 0, 0x10000);
    memset(reg, 0, 0x20);
    status = latch = 0;
    addr_inc = 1;
    dvssr_trigger = 0;

    playfield_shift = 6;
    playfield_row_mask = 0x1f;
    playfield_col_mask = 0xff;

    memset(bg_name_dirty, 0, sizeof(bg_name_dirty));
    memset(bg_name_list, 0, sizeof(bg_name_list));
    memset(bg_pattern_cache, 0, sizeof(bg_pattern_cache));
    bg_list_index = 0;

    memset(obj_name_dirty, 0, sizeof(obj_name_dirty));
    memset(obj_name_list, 0, sizeof(obj_name_list));
    memset(obj_pattern_cache, 0, sizeof(obj_pattern_cache));
    obj_list_index = 0;
}


void vdc_shutdown(void)
{
}


void vdc_do_dma(void)
{
    int did = (reg[0x0F] >> 3) & 1;
    int sid = (reg[0x0F] >> 2) & 1;
    int dvc = (reg[0x0F] >> 1) & 1;
    int sour = (reg[0x10] & 0x7FFF);
    int desr = (reg[0x11] & 0x7FFF);
    int lenr = (reg[0x12] & 0x7FFF);

#if LOG_DMA
    //error("DMA S:%04X%c D:%04X%c L:%04X\n", sour, (sid) ? '-' : '+', desr, (did) ? '-' : '+', lenr);
#endif

    /* Do VRAM -> VRAM transfer and update pattern caches */
    do {
        uint16 temp = vramw[(sour & 0x7FFF)];

        if(temp != vramw[(desr & 0x7FFF)])
        {
            vramw[(desr & 0x7FFF)] = temp;
            MARK_BG_DIRTY(desr);
            MARK_OBJ_DIRTY(desr);
        }

        sour = (sid) ? (sour - 1) : (sour + 1);
        desr = (did) ? (desr - 1) : (desr + 1);
    } while (lenr--);

    /* Set VRAM -> VRAM transfer completed flag */
    status |= STATUS_DV;

    /* Cause IRQ1 if enabled */
    if(dvc)
    {
        h6280_set_irq_line(0, ASSERT_LINE);
    }
}


/*--------------------------------------------------------------------------*/

