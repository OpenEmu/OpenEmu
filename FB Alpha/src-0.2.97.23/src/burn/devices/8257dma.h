
typedef UINT8 (*ior_in_functs)(UINT16 address);
typedef void (*ior_out_functs)(UINT16 address, UINT8 data);

void i8257Reset();
UINT8 i8257Read(UINT8 offset);
void i8257Write(UINT8 offset, UINT8 data);
void i8257_drq_write(int channel, int state);
void i8257_do_transfer(int state);		// call after last drq_write
void i8257Init();
void i8257Config(UINT8 (*cpuread)(UINT16), void (*cpuwrite)(UINT16,UINT8), INT32 (*idle)(INT32), ior_in_functs *read_f, ior_out_functs *write_f);
void i8257Scan();
