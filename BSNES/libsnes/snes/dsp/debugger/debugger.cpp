#ifdef DSP_CPP

bool DSPDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  item("Main Volume - Left", (unsigned)state.regs[0x0c]);
  item("Main Volume - Right", (unsigned)state.regs[0x1c]);
  item("Echo Volume - Left", (unsigned)state.regs[0x2c]);
  item("Echo Volume - Right", (unsigned)state.regs[0x3c]);
  item("Key On", string("0x", hex<2>(state.regs[0x4c])));
  item("Key Off", string("0x", hex<2>(state.regs[0x5c])));
  item("Flag - Reset", (bool)(state.regs[0x6c] & 0x80));
  item("Flag - Mute", (bool)(state.regs[0x6c] & 0x40));
  item("Flag - Echo Disable", (bool)(state.regs[0x6c] & 0x20));
  item("Flag - Noise Clock", (unsigned)state.regs[0x6c] & 0x1f);
  item("Source End Block", (unsigned)state.regs[0x7c]);
  item("Echo Feedback", (unsigned)state.regs[0x0d]);
  item("Pitch Modulation Enable", string("0x", hex<2>(state.regs[0x2d])));
  item("Noise Enable", string("0x", hex<2>(state.regs[0x3d])));
  item("Echo Enable", string("0x", hex<2>(state.regs[0x4d])));
  item("Source Directory", (unsigned)state.regs[0x5d]);
  item("Echo Start Address", (unsigned)state.regs[0x6d]);
  item("Echo Directory", (unsigned)state.regs[0x7d]);

  for(unsigned i = 0; i < 8; i++) {
    item(string("Coefficient ", i), string("0x", hex<2>(state.regs[(i << 4) + 0x0f])));
  }

  for(unsigned i = 0; i < 8; i++) {
    item(string("Voice ", i), "");
    item("Volume - Left", (unsigned)state.regs[(i << 4) + 0x00]);
    item("Volume - Right", (unsigned)state.regs[(i << 4) + 0x01]);
    item("Pitch Height", string("0x", hex<4>(state.regs[(i << 4) + 0x02] + (state.regs[(i << 4) + 0x03] << 8))));
    item("Source Number", (unsigned)state.regs[(i << 4) + 0x04]);
    item("ADSR1", (unsigned)state.regs[(i << 4) + 0x05]);
    item("ADSR2", (unsigned)state.regs[(i << 4) + 0x06]);
    item("GAIN", (unsigned)state.regs[(i << 4) + 0x07]);
    item("ENVX", (unsigned)state.regs[(i << 4) + 0x08]);
    item("OUTX", (unsigned)state.regs[(i << 4) + 0x09]);
  }

  #undef item
  return false;
}

#endif
