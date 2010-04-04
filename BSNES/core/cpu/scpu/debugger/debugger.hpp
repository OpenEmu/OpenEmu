class sCPUDebug : public sCPU {
public:
  void op_step();
  uint8 op_read(uint32 addr);
  void op_write(uint32 addr, uint8 data);
};
