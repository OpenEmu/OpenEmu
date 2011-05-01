#ifdef SA1_CPP

//====================
//direct data transfer
//====================

void SA1::dma_normal() {
  while(mmio.dtc--) {
    uint8 data = regs.mdr;
    uint32 dsa = mmio.dsa++;
    uint32 dda = mmio.dda++;

    //source and destination cannot be the same
    if(mmio.sd == DMA::SourceBWRAM && mmio.dd == DMA::DestBWRAM) continue;
    if(mmio.sd == DMA::SourceIRAM  && mmio.dd == DMA::DestIRAM ) continue;

    switch(mmio.sd) {
      case DMA::SourceROM: {
        if((dsa & 0x408000) == 0x008000 || (dsa & 0xc00000) == 0xc00000) {
          data = sa1bus.read(dsa);
        }
      } break;

      case DMA::SourceBWRAM: {
        if((dsa & 0x40e000) == 0x006000 || (dsa & 0xf00000) == 0x400000) {
          data = sa1bus.read(dsa);
        }
      } break;

      case DMA::SourceIRAM: {
        data = memory::iram.read(dsa & 0x07ff);
      } break;
    }

    switch(mmio.dd) {
      case DMA::DestBWRAM: {
        if((dda & 0x40e000) == 0x006000 || (dda & 0xf00000) == 0x400000) {
          sa1bus.write(dda, data);
        }
      } break;

      case DMA::DestIRAM: {
        memory::iram.write(dda & 0x07ff, data);
      } break;
    }
  }

  mmio.dma_irqfl = true;
  if(mmio.dma_irqen) mmio.dma_irqcl = 0;
}

//((byte & 6) << 3) + (byte & 1) explanation:
//transforms a byte index (0-7) into a planar index:
//result[] = {  0,  1, 16, 17, 32, 33, 48, 49 };
//works for 2bpp, 4bpp and 8bpp modes

//===========================
//type-1 character conversion
//===========================

void SA1::dma_cc1() {
  memory::cc1bwram.dma = true;
  mmio.chdma_irqfl = true;
  if(mmio.chdma_irqen) {
    mmio.chdma_irqcl = 0;
    cpu.regs.irq = 1;
  }
}

uint8 SA1::dma_cc1_read(unsigned addr) {
  //16 bytes/char (2bpp); 32 bytes/char (4bpp); 64 bytes/char (8bpp)
  unsigned charmask = (1 << (6 - mmio.dmacb)) - 1;

  if((addr & charmask) == 0) {
    //buffer next character to I-RAM
    unsigned bpp = 2 << (2 - mmio.dmacb);
    unsigned bpl = (8 << mmio.dmasize) >> mmio.dmacb;
    unsigned bwmask = memory::cartram.size() - 1;
    unsigned tile = ((addr - mmio.dsa) & bwmask) >> (6 - mmio.dmacb);
    unsigned ty = (tile >> mmio.dmasize);
    unsigned tx = tile & ((1 << mmio.dmasize) - 1);
    unsigned bwaddr = mmio.dsa + ty * 8 * bpl + tx * bpp;

    for(unsigned y = 0; y < 8; y++) {
      uint64 data = 0;
      for(unsigned byte = 0; byte < bpp; byte++) {
        data |= (uint64)memory::cartram.read((bwaddr + byte) & bwmask) << (byte << 3);
      }
      bwaddr += bpl;

      uint8 out[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      for(unsigned x = 0; x < 8; x++) {
        out[0] |= (data & 1) << (7 - x); data >>= 1;
        out[1] |= (data & 1) << (7 - x); data >>= 1;
        if(mmio.dmacb == 2) continue;
        out[2] |= (data & 1) << (7 - x); data >>= 1;
        out[3] |= (data & 1) << (7 - x); data >>= 1;
        if(mmio.dmacb == 1) continue;
        out[4] |= (data & 1) << (7 - x); data >>= 1;
        out[5] |= (data & 1) << (7 - x); data >>= 1;
        out[6] |= (data & 1) << (7 - x); data >>= 1;
        out[7] |= (data & 1) << (7 - x); data >>= 1;
      }

      for(unsigned byte = 0; byte < bpp; byte++) {
        unsigned p = mmio.dda + (y << 1) + ((byte & 6) << 3) + (byte & 1);
        memory::iram.write(p & 0x07ff, out[byte]);
      }
    }
  }

  return memory::iram.read((mmio.dda + (addr & charmask)) & 0x07ff);
}

//===========================
//type-2 character conversion
//===========================

void SA1::dma_cc2() {
  //select register file index (0-7 or 8-15)
  const uint8 *brf = &mmio.brf[(dma.line & 1) << 3];
  unsigned bpp = 2 << (2 - mmio.dmacb);
  unsigned addr = mmio.dda & 0x07ff;
  addr &= ~((1 << (7 - mmio.dmacb)) - 1);
  addr += (dma.line & 8) * bpp;
  addr += (dma.line & 7) * 2;

  for(unsigned byte = 0; byte < bpp; byte++) {
    uint8 output = 0;
    for(unsigned bit = 0; bit < 8; bit++) {
      output |= ((brf[bit] >> byte) & 1) << (7 - bit);
    }
    memory::iram.write(addr + ((byte & 6) << 3) + (byte & 1), output);
  }

  dma.line = (dma.line + 1) & 15;
}

#endif
