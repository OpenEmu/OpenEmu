class Dis6280
{
 public:
 Dis6280(void);
 ~Dis6280();

 virtual uint8 Read(uint16 A);
 virtual uint8 GetX(void);
 virtual uint8 GetY(void);

 void Disassemble(uint16 &a, uint16 SpecialA, char *);

 private:
};
