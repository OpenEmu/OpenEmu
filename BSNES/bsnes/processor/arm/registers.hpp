struct GPR {
  uint32 data;
  function<void ()> modify;

  inline operator uint32() const { return data; }
  inline GPR& operator=(uint32 n) { data = n; if(modify) modify(); return *this; }
  inline GPR& operator=(const GPR& source) { return operator=(source.data); }

  inline GPR& operator &=(uint32 n) { return operator=(data  & n); }
  inline GPR& operator |=(uint32 n) { return operator=(data  | n); }
  inline GPR& operator ^=(uint32 n) { return operator=(data  ^ n); }
  inline GPR& operator +=(uint32 n) { return operator=(data  + n); }
  inline GPR& operator -=(uint32 n) { return operator=(data  - n); }
  inline GPR& operator *=(uint32 n) { return operator=(data  * n); }
  inline GPR& operator /=(uint32 n) { return operator=(data  / n); }
  inline GPR& operator %=(uint32 n) { return operator=(data  % n); }
  inline GPR& operator<<=(uint32 n) { return operator=(data << n); }
  inline GPR& operator>>=(uint32 n) { return operator=(data >> n); }
};

struct PSR {
  bool n;      //negative
  bool z;      //zero
  bool c;      //carry
  bool v;      //overflow
  bool i;      //irq
  bool f;      //fiq
  bool t;      //thumb
  unsigned m;  //mode

  inline operator uint32() const {
    return (n << 31) + (z << 30) + (c << 29) + (v << 28)
         + (i <<  7) + (f <<  6) + (t <<  5) + (m <<  0);
  }

  inline PSR& operator=(uint32 d) {
    n = d & (1 << 31);
    z = d & (1 << 30);
    c = d & (1 << 29);
    v = d & (1 << 28);
    i = d & (1 <<  7);
    f = d & (1 <<  6);
    t = d & (1 <<  5);
    m = d & 31;
    return *this;
  }

  void serialize(serializer&);
};

struct Pipeline {
  bool reload;

  struct Instruction {
    uint32 address;
    uint32 instruction;
  };

  Instruction execute;
  Instruction decode;
  Instruction fetch;
};

struct Processor {
  enum class Mode : unsigned {
    USR = 0x10,  //user
    FIQ = 0x11,  //fast interrupt request
    IRQ = 0x12,  //interrupt request
    SVC = 0x13,  //supervisor (software interrupt)
    ABT = 0x17,  //abort
    UND = 0x1b,  //undefined
    SYS = 0x1f,  //system
  };

  GPR r0, r1, r2, r3, r4, r5, r6, r7;

  struct USR {
    GPR r8, r9, r10, r11, r12, sp, lr;
  } usr;

  struct FIQ {
    GPR r8, r9, r10, r11, r12, sp, lr;
    PSR spsr;
  } fiq;

  struct IRQ {
    GPR sp, lr;
    PSR spsr;
  } irq;

  struct SVC {
    GPR sp, lr;
    PSR spsr;
  } svc;

  struct ABT {
    GPR sp, lr;
    PSR spsr;
  } abt;

  struct UND {
    GPR sp, lr;
    PSR spsr;
  } und;

  GPR pc;
  PSR cpsr;
  bool carryout;
  bool sequential;
  bool irqline;

  GPR *r[16];
  PSR *spsr;

  void power();
  void setMode(Mode);
};

Processor processor;
Pipeline pipeline;
bool crash;

void pipeline_step();

alwaysinline GPR& r(unsigned n) { return *processor.r[n]; }
alwaysinline PSR& cpsr() { return processor.cpsr; }
alwaysinline PSR& spsr() { return *processor.spsr; }
alwaysinline bool& carryout() { return processor.carryout; }
alwaysinline bool& sequential() { return processor.sequential; }
alwaysinline uint32 instruction() { return pipeline.execute.instruction; }
alwaysinline Processor::Mode mode() { return (Processor::Mode)processor.cpsr.m; }
alwaysinline bool privilegedmode() const { return (Processor::Mode)processor.cpsr.m != Processor::Mode::USR; }
alwaysinline bool exceptionmode() const { return privilegedmode() && (Processor::Mode)processor.cpsr.m != Processor::Mode::SYS; }
