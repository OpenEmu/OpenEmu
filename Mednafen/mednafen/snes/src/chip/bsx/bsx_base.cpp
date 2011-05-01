#ifdef BSX_CPP

BSXBase bsxbase;

void BSXBase::init() {
}

void BSXBase::enable() {
  for(uint16 i = 0x2188; i <= 0x219f; i++) memory::mmio.map(i, *this);
}

void BSXBase::power() {
  reset();
}

void BSXBase::reset() {
  memset(&regs, 0x00, sizeof regs);
}

uint8 BSXBase::mmio_read(unsigned addr) {
  addr &= 0xffff;

  switch(addr) {
    case 0x2188: return regs.r2188;
    case 0x2189: return regs.r2189;
    case 0x218a: return regs.r218a;
    case 0x218c: return regs.r218c;
    case 0x218e: return regs.r218e;
    case 0x218f: return regs.r218f;
    case 0x2190: return regs.r2190;

    case 0x2192: {
      unsigned counter = regs.r2192_counter++;
      if(regs.r2192_counter >= 18) regs.r2192_counter = 0;

      if(counter == 0) {
        time_t rawtime;
        time(&rawtime);
        tm *t = localtime(&rawtime);

        regs.r2192_hour   = t->tm_hour;
        regs.r2192_minute = t->tm_min;
        regs.r2192_second = t->tm_sec;
      }

      switch(counter) {
        case  0: return 0x00;  //???
        case  1: return 0x00;  //???
        case  2: return 0x00;  //???
        case  3: return 0x00;  //???
        case  4: return 0x00;  //???
        case  5: return 0x01;
        case  6: return 0x01;
        case  7: return 0x00;
        case  8: return 0x00;
        case  9: return 0x00;
        case 10: return regs.r2192_second;
        case 11: return regs.r2192_minute;
        case 12: return regs.r2192_hour;
        case 13: return 0x00;  //???
        case 14: return 0x00;  //???
        case 15: return 0x00;  //???
        case 16: return 0x00;  //???
        case 17: return 0x00;  //???
      }
    } break;

    case 0x2193: return regs.r2193 & ~0x0c;
    case 0x2194: return regs.r2194;
    case 0x2196: return regs.r2196;
    case 0x2197: return regs.r2197;
    case 0x2199: return regs.r2199;
  }

  return cpu.regs.mdr;
}

void BSXBase::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;

  switch(addr) {
    case 0x2188: {
      regs.r2188 = data;
    } break;

    case 0x2189: {
      regs.r2189 = data;
    } break;

    case 0x218a: {
      regs.r218a = data;
    } break;

    case 0x218b: {
      regs.r218b = data;
    } break;

    case 0x218c: {
      regs.r218c = data;
    } break;

    case 0x218e: {
      regs.r218e = data;
    } break;

    case 0x218f: {
      regs.r218e >>= 1;
      regs.r218e = regs.r218f - regs.r218e;
      regs.r218f >>= 1;
    } break;

    case 0x2191: {
      regs.r2191 = data;
      regs.r2192_counter = 0;
    } break;

    case 0x2192: {
      regs.r2190 = 0x80;
    } break;

    case 0x2193: {
      regs.r2193 = data;
    } break;

    case 0x2194: {
      regs.r2194 = data;
    } break;

    case 0x2197: {
      regs.r2197 = data;
    } break;

    case 0x2199: {
      regs.r2199 = data;
    } break;
  }
}

#endif

