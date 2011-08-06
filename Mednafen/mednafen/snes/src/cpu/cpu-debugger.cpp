#ifdef CPU_CPP

bool CPUDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  //internal
  if(id == n++) { name = "S-CPU MDR"; value = string::printf("0x%.2x", mdr()); return true; }

  //$2181-2183
  if(id == n++) { name = "$2181-$2183"; value = ""; return true; }
  if(id == n++) { name = "WRAM Address"; value = string::printf("0x%.6x", wram_address()); return true; }

  //$4016
  if(id == n++) { name = "$4016"; value = ""; return true; }
  if(id == n++) { name = "Joypad Strobe Latch"; value = joypad_strobe_latch(); return true; }

  //$4200
  if(id == n++) { name = "$4200"; value = ""; return true; }
  if(id == n++) { name = "NMI Enable"; value = nmi_enable(); return true; }
  if(id == n++) { name = "H-IRQ Enable"; value = hirq_enable(); return true; }
  if(id == n++) { name = "V-IRQ Enable"; value = virq_enable(); return true; }
  if(id == n++) { name = "Auto Joypad Poll"; value = auto_joypad_poll(); return true; }

  //$4201
  if(id == n++) { name = "$4201"; value = ""; return true; }
  if(id == n++) { name = "PIO"; value = string::printf("0x%.2x", pio_bits()); return true; }

  //$4202
  if(id == n++) { name = "$4202"; value = ""; return true; }
  if(id == n++) { name = "Multiplicand"; value = string::printf("0x%.2x", multiplicand()); return true; }

  //$4203
  if(id == n++) { name = "$4203"; value = ""; return true; }
  if(id == n++) { name = "Multiplier"; value = string::printf("0x%.2x", multiplier()); return true; }

  //$4204-$4205
  if(id == n++) { name = "$4204-$4205"; value = ""; return true; }
  if(id == n++) { name = "Dividend"; value = string::printf("0x%.4x", dividend()); return true; }

  //$4206
  if(id == n++) { name = "$4206"; value = ""; return true; }
  if(id == n++) { name = "Divisor"; value = string::printf("0x%.2x", divisor()); return true; }

  //$4207-$4208
  if(id == n++) { name = "$4207-$4208"; value = ""; return true; }
  if(id == n++) { name = "H-Time"; value = string::printf("0x%.4x", htime()); return true; }

  //$4209-$420a
  if(id == n++) { name = "$4209-$420a"; value = ""; return true; }
  if(id == n++) { name = "V-Time"; value = string::printf("0x%.4x", vtime()); return true; }

  //$420d
  if(id == n++) { name = "$420d"; value = ""; return true; }
  if(id == n++) { name = "FastROM Enable"; value = fastrom_enable(); return true; }

  return false;
}

#endif
