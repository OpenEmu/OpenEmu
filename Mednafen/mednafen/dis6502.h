class Dis6502
{
 public:
 Dis6502(bool);
 virtual ~Dis6502();

 virtual uint8 Read(uint16 A);
 virtual uint8 GetX(void);
 virtual uint8 GetY(void);

 void Disassemble(uint16 &a, uint16 SpecialA, char *);

 private:
 bool IsCMOS;
};
