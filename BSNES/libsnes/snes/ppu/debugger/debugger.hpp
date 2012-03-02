class PPUDebugger : public PPU, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);
};
