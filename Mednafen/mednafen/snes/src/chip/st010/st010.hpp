class ST010 : public Memory {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  void serialize(serializer&);

private:
  uint8 ram[0x1000];
  static const int16 sin_table[256];
  static const int16 mode7_scale[176];
  static const uint8 arctan[32][32];

  //interfaces to sin table
  int16 sin(int16 theta);
  int16 cos(int16 theta);

  //interfaces to ram buffer
  uint8  readb (uint16 addr);
  uint16 readw (uint16 addr);
  uint32 readd (uint16 addr);
  void   writeb(uint16 addr, uint8  data);
  void   writew(uint16 addr, uint16 data);
  void   writed(uint16 addr, uint32 data);

  //opcodes
  void op_01();
  void op_02();
  void op_03();
  void op_04();
  void op_05();
  void op_06();
  void op_07();
  void op_08();

  void op_01(int16 x0, int16 y0, int16 &x1, int16 &y1, int16 &quadrant, int16 &theta);
};

extern ST010 st010;
