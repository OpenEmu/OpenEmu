
void c6280_init(double clk, INT32 bAdd);
void c6280_exit();

void c6280_reset();

void c6280_update(INT16 *pBuf, INT32 samples);

void c6280_write(UINT8 offset, UINT8 data);
UINT8 c6280_read();

INT32 c6280_scan(INT32 nAction, INT32 *pnMin);
