unsigned cache_access_speed;
unsigned memory_access_speed;
bool r15_modified;

void step(unsigned clocks);

void rombuffer_sync();
void rombuffer_update();
uint8 rombuffer_read();

void rambuffer_sync();
uint8 rambuffer_read(uint16 addr);
void rambuffer_write(uint16 addr, uint8 data);

void r14_modify(uint16);
void r15_modify(uint16);

void update_speed();
void timing_reset();
