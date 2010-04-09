
#ifndef _VDP_H_
#define _VDP_H_

/*
    vdp1

    mode 4 when m4 set and m1 reset

    vdp2

    mode 4 when m4 set and m2,m1 != 1,0


*/

/* Display timing (NTSC) */

#define MASTER_CLOCK        3579545
#define LINES_PER_FRAME     262
#define FRAMES_PER_SECOND   60
#define CYCLES_PER_LINE     ((MASTER_CLOCK / FRAMES_PER_SECOND) / LINES_PER_FRAME)

/* VDP context */
typedef struct
{
    uint8 vram[0x4000];
    uint8 cram[0x40]; 
    uint8 reg[0x10];
    uint8 status;     
    uint8 latch;      
    uint8 pending;    
    uint8 buffer;     
    uint8 code;       
    uint16 addr;
    int pn, ct, pg, sa, sg;
    int ntab;        
    int satb;
    int line;
    int left;
    uint8 height;
    uint8 extended;
    uint8 mode;
    uint8 vint_pending;
    uint8 hint_pending;
    uint16 cram_latch;
    uint8 bd;

    int lines_per_frame;
    int rshift, gshift, bshift;
} vdp_t;

/* Global data */
extern vdp_t vdp;

/* Function prototypes */
void vdp_init(void);
void vdp_shutdown(void);
void vdp_reset(void);
uint8 vdp_counter_r(int offset);
uint8 vdp_read(int offset);
void vdp_write(int offset, uint8 data);
void gg_vdp_write(int offset, uint8 data);
void md_vdp_write(int offset, uint8 data);
void tms_write(int offset, int data);


void SMS_VDPRunFrame(int skip_render);
void SMS_VDPSetPixelFormat(int, int, int);
int SMS_VDPStateAction(StateMem *sm, int load, int data_only);


#endif /* _VDP_H_ */

