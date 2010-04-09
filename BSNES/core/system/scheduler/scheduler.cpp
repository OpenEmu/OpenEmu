#ifdef SYSTEM_CPP

Scheduler scheduler;

void threadentry_cpu() { cpu.enter(); }
void threadentry_cop() { system.coprocessor_enter(); }
void threadentry_smp() { smp.enter(); }
void threadentry_ppu() { ppu.enter(); }
void threadentry_dsp() { dsp.enter(); }

void Scheduler::enter() {
  co_switch(thread_active);
}

void Scheduler::exit() {
  co_switch(thread_snes);
}

void Scheduler::init() {
  clock.cpu_freq = system.region() == System::NTSC
                 ? config.cpu.ntsc_clock_rate
                 : config.cpu.pal_clock_rate;
  clock.smp_freq = system.region() == System::NTSC
                 ? config.smp.ntsc_clock_rate
                 : config.smp.pal_clock_rate;

  clock.cpucop = 0;
  clock.cpuppu = 0;
  clock.cpusmp = 0;
  clock.smpdsp = 0;

  if(thread_cpu) co_delete(thread_cpu);
  if(thread_cop) co_delete(thread_cop);
  if(thread_smp) co_delete(thread_smp);
  if(thread_ppu) co_delete(thread_ppu);
  if(thread_dsp) co_delete(thread_dsp);

  thread_snes = co_active();
  thread_cpu  = co_create(65536 * sizeof(void*), threadentry_cpu);
  thread_cop  = co_create(65536 * sizeof(void*), threadentry_cop);
  thread_smp  = co_create(65536 * sizeof(void*), threadentry_smp);
  thread_ppu  = co_create(65536 * sizeof(void*), threadentry_ppu);
  thread_dsp  = co_create(65536 * sizeof(void*), threadentry_dsp);

  //start execution with S-CPU after reset
  thread_active = thread_cpu;
}

Scheduler::Scheduler() {
  thread_snes   = 0;
  thread_cpu    = 0;
  thread_cop    = 0;
  thread_smp    = 0;
  thread_ppu    = 0;
  thread_dsp    = 0;
  thread_active = 0;
}

#endif
