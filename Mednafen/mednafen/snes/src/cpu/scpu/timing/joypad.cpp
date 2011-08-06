#ifdef SCPU_CPP

void sCPU::run_auto_joypad_poll() {
  uint16 joy1 = 0, joy2 = 0, joy3 = 0, joy4 = 0;
  for(unsigned i = 0; i < 16; i++) {
    uint8 port0 = input.port_read(0);
    uint8 port1 = input.port_read(1);

    joy1 |= (port0 & 1) ? (0x8000 >> i) : 0;
    joy2 |= (port1 & 1) ? (0x8000 >> i) : 0;
    joy3 |= (port0 & 2) ? (0x8000 >> i) : 0;
    joy4 |= (port1 & 2) ? (0x8000 >> i) : 0;
  }

  status.joy1l = joy1;
  status.joy1h = joy1 >> 8;

  status.joy2l = joy2;
  status.joy2h = joy2 >> 8;

  status.joy3l = joy3;
  status.joy3h = joy3 >> 8;

  status.joy4l = joy4;
  status.joy4h = joy4 >> 8;
}

#endif
