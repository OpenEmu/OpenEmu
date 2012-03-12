
// video
void vdc_reset();
void vdc_write(int which, UINT8 offset, UINT8 data);
UINT8 vdc_read(int which, UINT8 offset);
extern UINT8 *vdc_vidram[2];
extern UINT16 *vdc_tmp_draw;

void pce_interrupt(); // update scanline...
void sgx_interrupt();

void vdc_get_dimensions(INT32 which, INT32 *x, INT32 *y); // get resolution

void sgx_vdc_write(UINT8 offset, UINT8 data);
UINT8 sgx_vdc_read(UINT8 offset);


// priority
void vpc_reset();
void vpc_write(UINT8 offset, UINT8 data);
UINT8 vpc_read(UINT8 offset);

// palette
void vce_reset();
void vce_palette_init(UINT32 *Palette);
void vce_write(UINT8 offset, UINT8 data);
UINT8 vce_read(UINT8 offset);
extern UINT16 *vce_data;

INT32 vdc_scan(INT32 nAction, INT32 *pnMin);
