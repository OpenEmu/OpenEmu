class Cx4 : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  void serialize(serializer&);

private:
  uint8  ram[0x0c00];
  uint8  reg[0x0100];
  uint32 r0, r1, r2,  r3,  r4,  r5,  r6,  r7,
         r8, r9, r10, r11, r12, r13, r14, r15;

  static const uint8  immediate_data[48];
  static const uint16 wave_data[40];
  static const uint32 sin_table[256];

  static const int16 SinTable[512];
  static const int16 CosTable[512];

  int16 C4WFXVal, C4WFYVal, C4WFZVal, C4WFX2Val, C4WFY2Val, C4WFDist, C4WFScale;
  int16 C41FXVal, C41FYVal, C41FAngleRes, C41FDist, C41FDistVal;

  void C4TransfWireFrame();
  void C4TransfWireFrame2();
  void C4CalcWireFrame();
  void C4DrawLine(int32 X1, int32 Y1, int16 Z1, int32 X2, int32 Y2, int16 Z2, uint8 Color);
  void C4DrawWireFrame();
  void C4DoScaleRotate(int row_padding);

public:
  uint32 ldr(uint8 r);
  void   str(uint8 r, uint32 data);
  void   mul(uint32 x, uint32 y, uint32 &rl, uint32 &rh);
  uint32 sin(uint32 rx);
  uint32 cos(uint32 rx);

  void   transfer_data();
  void   immediate_reg(uint32 num);

  void   op00_00();
  void   op00_03();
  void   op00_05();
  void   op00_07();
  void   op00_08();
  void   op00_0b();
  void   op00_0c();

  void   op00();
  void   op01();
  void   op05();
  void   op0d();
  void   op10();
  void   op13();
  void   op15();
  void   op1f();
  void   op22();
  void   op25();
  void   op2d();
  void   op40();
  void   op54();
  void   op5c();
  void   op5e();
  void   op60();
  void   op62();
  void   op64();
  void   op66();
  void   op68();
  void   op6a();
  void   op6c();
  void   op6e();
  void   op70();
  void   op72();
  void   op74();
  void   op76();
  void   op78();
  void   op7a();
  void   op7c();
  void   op89();

  uint8  readb(uint16 addr);
  uint16 readw(uint16 addr);
  uint32 readl(uint16 addr);

  void   writeb(uint16 addr, uint8 data);
  void   writew(uint16 addr, uint16 data);
  void   writel(uint16 addr, uint32 data);
};

extern Cx4 cx4;
