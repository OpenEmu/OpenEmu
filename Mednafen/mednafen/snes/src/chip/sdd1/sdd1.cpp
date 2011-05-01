#include <../base.hpp>

#define SDD1_CPP
namespace SNES {

SDD1 sdd1;

#include "serialization.cpp"
#include "sdd1emu.cpp"

void SDD1::init() {}

void SDD1::enable() {
  //hook S-CPU DMA MMIO registers to gather information for struct dma[];
  //buffer address and transfer size information for use in SDD1::read()
  for(unsigned i = 0x4300; i <= 0x437f; i++) {
    cpu_mmio[i & 0x7f] = memory::mmio.mmio[i - 0x2000];
    memory::mmio.map(i, *this);
  }

  //hook S-DD1 MMIO registers
  for(unsigned i = 0x4800; i <= 0x4807; i++) {
    memory::mmio.map(i, *this);
  }
}

void SDD1::power() {
  reset();
}

void SDD1::reset() {
  sdd1_enable = 0x00;
  xfer_enable = 0x00;

  mmc[0] = 0 << 20;
  mmc[1] = 1 << 20;
  mmc[2] = 2 << 20;
  mmc[3] = 3 << 20;

  for(unsigned i = 0; i < 8; i++) {
    dma[i].addr = 0;
    dma[i].size = 0;
  }

  buffer.ready = false;

  bus.map(Bus::MapDirect, 0xc0, 0xff, 0x0000, 0xffff, *this);
}

uint8 SDD1::mmio_read(unsigned addr) {
  addr &= 0xffff;

  if((addr & 0x4380) == 0x4300) {
    return cpu_mmio[addr & 0x7f]->mmio_read(addr);
  }

  switch(addr) {
    case 0x4804: return mmc[0] >> 20;
    case 0x4805: return mmc[1] >> 20;
    case 0x4806: return mmc[2] >> 20;
    case 0x4807: return mmc[3] >> 20;
  }

  return cpu.regs.mdr;
}

void SDD1::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;

  if((addr & 0x4380) == 0x4300) {
    unsigned channel = (addr >> 4) & 7;
    switch(addr & 15) {
      case 2: dma[channel].addr = (dma[channel].addr & 0xffff00) + (data <<  0); break;
      case 3: dma[channel].addr = (dma[channel].addr & 0xff00ff) + (data <<  8); break;
      case 4: dma[channel].addr = (dma[channel].addr & 0x00ffff) + (data << 16); break;

      case 5: dma[channel].size = (dma[channel].size &   0xff00) + (data <<  0); break;
      case 6: dma[channel].size = (dma[channel].size &   0x00ff) + (data <<  8); break;
    }
    return cpu_mmio[addr & 0x7f]->mmio_write(addr, data);
  }

  switch(addr) {
    case 0x4800: sdd1_enable = data; break;
    case 0x4801: xfer_enable = data; break;

    case 0x4804: mmc[0] = data << 20; break;
    case 0x4805: mmc[1] = data << 20; break;
    case 0x4806: mmc[2] = data << 20; break;
    case 0x4807: mmc[3] = data << 20; break;
  }
}

//SDD1::read() is mapped to $[c0-ff]:[0000-ffff]
//the design is meant to be as close to the hardware design as possible, thus this code
//avoids adding S-DD1 hooks inside S-CPU::DMA emulation.
//
//the real S-DD1 cannot see $420b (DMA enable) writes, as they are not placed on the bus.
//however, $43x0-$43xf writes (DMAx channel settings) most likely do appear on the bus.
//the S-DD1 also requires fixed addresses for transfers, which wouldn't be necessary if
//it could see $420b writes (eg it would know when the transfer should begin.)
//
//the hardware needs a way to distinguish program code after $4801 writes from DMA
//decompression that follows soon after.
//
//the only plausible design for hardware would be for the S-DD1 to spy on DMAx settings,
//and begin spooling decompression on writes to $4801 that activate a channel. after that,
//it feeds decompressed data only when the ROM read address matches the DMA channel address.
//
//the actual S-DD1 transfer can occur on any channel, but it is most likely limited to
//one transfer per $420b write (for spooling purposes). however, this is not known for certain.
uint8 SDD1::read(unsigned addr) {
  if(sdd1_enable & xfer_enable) {
    //at least one channel has S-DD1 decompression enabled ...
    for(unsigned i = 0; i < 8; i++) {
      if(sdd1_enable & xfer_enable & (1 << i)) {
        //S-DD1 always uses fixed transfer mode, so address will not change during transfer
        if(addr == dma[i].addr) {
          if(!buffer.ready) {
            //first byte read for channel performs full decompression.
            //this really should stream byte-by-byte, but it's not necessary since the size is known
            buffer.offset = 0;
            buffer.size = dma[i].size ? dma[i].size : 65536;

            //sdd1emu calls this function; it needs to access uncompressed data;
            //so temporarily disable decompression mode for decompress() call.
            uint8 temp = sdd1_enable;
            sdd1_enable = false;
            sdd1emu.decompress(addr, buffer.size, buffer.data);
            sdd1_enable = temp;

            buffer.ready = true;
          }

          //fetch a decompressed byte; once buffer is depleted, disable channel and invalidate buffer
          uint8 data = buffer.data[(uint16)buffer.offset++];
          if(buffer.offset >= buffer.size) {
            buffer.ready = false;
            xfer_enable &= ~(1 << i);
          }

          return data;
        }  //address matched
      }  //channel enabled
    }  //channel loop
  }  //S-DD1 decompressor enabled

  //S-DD1 decompression mode inactive; return ROM data
  return memory::cartrom.read(mmc[(addr >> 20) & 3] + (addr & 0x0fffff));
}

void SDD1::write(unsigned addr, uint8 data) {
}

SDD1::SDD1() {
}

SDD1::~SDD1() {
}

};
