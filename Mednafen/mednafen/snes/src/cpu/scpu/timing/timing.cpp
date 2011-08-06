#ifdef SCPU_CPP

#include "event.cpp"
#include "irq.cpp"
#include "joypad.cpp"

unsigned sCPU::dma_counter() {
  return (status.dma_counter + hcounter()) & 7;
}

void sCPU::add_clocks(unsigned clocks) {
  event.tick(clocks);
  unsigned ticks = clocks >> 1;
  while(ticks--) {
    tick();
    if(hcounter() & 2) {
      input.tick();
      poll_interrupts();
    }
  }
  scheduler.addclocks_cpu(clocks);
}

//called by ppu.tick() when Hcounter=0
void sCPU::scanline() {
  status.dma_counter = (status.dma_counter + status.line_clocks) & 7;
  status.line_clocks = lineclocks();

  //forcefully sync S-CPU to other processors, in case chips are not communicating
  scheduler.sync_cpuppu();
  scheduler.sync_cpucop();
  scheduler.sync_cpusmp();
  system.scanline();

  if(vcounter() == 0) {
    //hdma init triggers once every frame
    event.enqueue(cpu_version == 1 ? 12 + 8 - dma_counter() : 12 + dma_counter(), EventHdmaInit);
  }

  //dram refresh occurs once every scanline
  if(cpu_version == 2) status.dram_refresh_position = 530 + 8 - dma_counter();
  event.enqueue(status.dram_refresh_position, EventDramRefresh);

  //hdma triggers once every visible scanline
  if(vcounter() <= (ppu.overscan() == false ? 224 : 239)) {
    event.enqueue(1104, EventHdmaRun);
  }

  if(status.auto_joypad_poll == true && vcounter() == (ppu.overscan() == false ? 227 : 242)) {
    input.poll();
    run_auto_joypad_poll();
  }
}

//used to test for H/DMA, which can trigger on the edge of every opcode cycle.
void sCPU::cycle_edge() {
  while(cycle_edge_state) {
    switch(bit::lowest(cycle_edge_state)) {
      case EventFlagHdmaInit: {
        hdma_init_reset();
        if(hdma_enabled_channels()) {
          status.hdma_pending = true;
          status.hdma_mode = 0;
        }
      } break;

      case EventFlagHdmaRun: {
        if(hdma_active_channels()) {
          status.hdma_pending = true;
          status.hdma_mode = 1;
        }
      } break;
    }

    cycle_edge_state = bit::clear_lowest(cycle_edge_state);
  }

  //H/DMA pending && DMA inactive?
  //.. Run one full CPU cycle
  //.. HDMA pending && HDMA enabled ? DMA sync + HDMA run
  //.. DMA pending && DMA enabled ? DMA sync + DMA run
  //.... HDMA during DMA && HDMA enabled ? DMA sync + HDMA run
  //.. Run one bus CPU cycle
  //.. CPU sync

  if(status.dma_active == true) {
    if(status.hdma_pending) {
      status.hdma_pending = false;
      if(hdma_enabled_channels()) {
        dma_add_clocks(8 - dma_counter());
        status.hdma_mode == 0 ? hdma_init() : hdma_run();
        if(!dma_enabled_channels()) {
          add_clocks(status.clock_count - (status.dma_clocks % status.clock_count));
          status.dma_active = false;
        }
      }
    }

    if(status.dma_pending) {
      status.dma_pending = false;
      if(dma_enabled_channels()) {
        dma_add_clocks(8 - dma_counter());
        dma_run();
        add_clocks(status.clock_count - (status.dma_clocks % status.clock_count));
        status.dma_active = false;
      }
    }
  }

  if(status.dma_active == false) {
    if(status.dma_pending || status.hdma_pending) {
      status.dma_clocks = 0;
      status.dma_active = true;
    }
  }
}

//used to test for NMI/IRQ, which can trigger on the edge of every opcode.
//test one cycle early to simulate two-stage pipeline of x816 CPU.
//
//status.irq_lock is used to simulate hardware delay before interrupts can
//trigger during certain events (immediately after DMA, writes to $4200, etc)
void sCPU::last_cycle() {
  if(!status.irq_lock) {
    status.nmi_pending |= nmi_test();
    status.irq_pending |= irq_test();

    status.interrupt_pending = (status.nmi_pending || status.irq_pending);
  }
}

void sCPU::timing_power() {
}

void sCPU::timing_reset() {
  event.reset();

  status.clock_count = 0;
  status.line_clocks = lineclocks();

  status.irq_lock = false;
  status.alu_lock = false;
  status.dram_refresh_position = (cpu_version == 1 ? 530 : 538);
  event.enqueue(status.dram_refresh_position, EventDramRefresh);

  status.nmi_valid      = false;
  status.nmi_line       = false;
  status.nmi_transition = false;
  status.nmi_pending    = false;
  status.nmi_hold       = false;

  status.irq_valid      = false;
  status.irq_line       = false;
  status.irq_transition = false;
  status.irq_pending    = false;
  status.irq_hold       = false;

  status.reset_pending     = true;
  status.interrupt_pending = true;
  status.interrupt_vector  = 0xfffc;  //reset vector address

  status.dma_active   = false;
  status.dma_counter  = 0;
  status.dma_clocks   = 0;
  status.dma_pending  = false;
  status.hdma_pending = false;
  status.hdma_mode    = 0;

  cycle_edge_state = 0;
}

#endif
