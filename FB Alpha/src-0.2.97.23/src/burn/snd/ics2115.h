extern UINT8 *ICSSNDROM;
extern UINT32 nICSSNDROMLen;

extern UINT8 ics2115read(UINT8 offset);
extern void ics2115write(UINT8 offset, UINT8 data);

extern INT32 ics2115_init();
extern void ics2115_exit();
extern void ics2115_reset();

extern UINT16 ics2115_soundlatch_r(INT32 i);
extern void ics2115_soundlatch_w(INT32 i, UINT16 d);

extern void ics2115_frame();
extern void ics2115_update(INT32 length);
extern void ics2115_scan(INT32 nAction,INT32 *pnMin);
