
#ifndef _VDC_H_
#define _VDC_H_

/* Status register flags */
#define STATUS_BSY      (0x40)  /* Memory access busy */
#define STATUS_VD       (0x20)  /* Vertical blanking */
#define STATUS_DV       (0x10)  /* VRAM -> VRAM DMA */
#define STATUS_DS       (0x08)  /* VRAM -> SATB DMA */
#define STATUS_RR       (0x04)  /* Line interrupt */
#define STATUS_OR       (0x02)  /* Sprite overflow */
#define STATUS_CR       (0x01)  /* Sprite collision */

/* Global data */
extern uint32 y_offset;
extern uint32 byr;
extern uint8 vram[0x10000];
extern uint16 reg[0x20];
extern uint8 objram[0x200];
extern uint8 status;
extern uint8 latch;
extern uint8 addr_inc;
extern uint8 vram_data_latch;
extern uint8 dvssr_trigger;
extern int playfield_shift;
extern uint32 playfield_col_mask;
extern uint32 playfield_row_mask;
extern int disp_width;
extern int disp_height;
extern uint32 disp_nt_width;
extern uint16 *vramw;
extern uint16 *objramw;
extern int old_width;
extern int old_height;
extern int playfield_shift_table[];
extern int playfield_row_mask_table[];
extern uint8 bg_name_dirty[0x800];
extern uint16 bg_name_list[0x800];
extern uint16 bg_list_index;
extern uint8 bg_pattern_cache[0x20000];
extern uint16 obj_name_dirty[0x200];
extern uint16 obj_name_list[0x200];
extern uint16 obj_list_index;
extern uint8 obj_pattern_cache[0x80000]; 

/* Function prototypes */
int vdc_r(int offset);
void vdc_w(int offset, int data);
int vdc_init(void);
void vdc_reset(void);
void vdc_shutdown(void);
void vdc_do_dma(void);
void vdc_ctrl_w(int data);
int vdc_ctrl_r(void);
void vdc_data_w(int offset, int data);
int vdc_data_r(int offset);

#endif /* _VDC_H_ */
