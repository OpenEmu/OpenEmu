void iremga20_write(INT32 device, INT32 offset, INT32 data);
UINT8 iremga20_read(INT32 device, INT32 offset);

void iremga20_reset(INT32 device);
void iremga20_init(INT32 device, UINT8 *rom, INT32 rom_size, INT32 frequency);
void itemga20_set_route(INT32 device, double nVolume, INT32 nRouteDir);
void iremga20_update(INT32 device, INT16 *buffer, INT32 length);
void iremga20_exit();

INT32 iremga20_scan(INT32 device, INT32 nAction, INT32 *pnMin);
