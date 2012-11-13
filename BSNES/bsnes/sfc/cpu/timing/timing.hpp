//timing.cpp
unsigned dma_counter();

void add_clocks(unsigned clocks);
void scanline();

alwaysinline void alu_edge();
alwaysinline void dma_edge();
alwaysinline void last_cycle();

void timing_power();
void timing_reset();

//irq.cpp
alwaysinline void poll_interrupts();
void nmitimen_update(uint8 data);
bool rdnmi();
bool timeup();

alwaysinline bool nmi_test();
alwaysinline bool irq_test();

//joypad.cpp
void step_auto_joypad_poll();
