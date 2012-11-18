enum : unsigned { OBJ = 0, BG0 = 1, BG1 = 2, BG2 = 3, BG3 = 4, SFX = 5 };
enum : unsigned { In0 = 0, In1 = 1, Obj = 2, Out = 3 };

struct Registers {
  struct Control {
    uint3 bgmode;
    uint1 cgbmode;
    uint1 frame;
    uint1 hblank;
    uint1 objmapping;
    uint1 forceblank;
    uint1 enable[5];
    uint1 enablewindow[3];

    operator uint16() const;
    uint16 operator=(uint16 source);
    Control& operator=(const Control&) = delete;
  } control;

  uint1 greenswap;

  struct Status {
    uint1 vblank;
    uint1 hblank;
    uint1 vcoincidence;
    uint1 irqvblank;
    uint1 irqhblank;
    uint1 irqvcoincidence;
    uint8 vcompare;

    operator uint16() const;
    uint16 operator=(uint16 source);
    Status& operator=(const Status&) = delete;
  } status;

  uint16 vcounter;

  struct BackgroundControl {
    uint2 priority;
    uint2 characterbaseblock;
    uint1 mosaic;
    uint1 colormode;
    uint5 screenbaseblock;
    uint1 affinewrap;  //BG2,3 only
    uint2 screensize;

    operator uint16() const;
    uint16 operator=(uint16 source);
    BackgroundControl& operator=(const BackgroundControl&) = delete;
  };

  struct Background {
    BackgroundControl control;
    uint9 hoffset;
    uint9 voffset;

    //BG2,3 only
    int16 pa, pb, pc, pd;
    int28 x, y;

    //internal
    int28 lx, ly;
    unsigned vmosaic;
    unsigned hmosaic;
    unsigned id;
  } bg[4];

  struct WindowFlags {
    uint1 enable[6];

    operator uint8() const;
    uint8 operator=(uint8 source);
    WindowFlags& operator=(const WindowFlags&) = delete;
  };

  struct Window {
    uint8 x1, x2;
    uint8 y1, y2;
  } window[2];

  WindowFlags windowflags[4];

  struct Mosaic {
    uint4 bghsize;
    uint4 bgvsize;
    uint4 objhsize;
    uint4 objvsize;
  } mosaic;

  struct BlendControl {
    uint1 above[6];
    uint1 below[6];
    uint2 mode;

    operator uint16() const;
    uint16 operator=(uint16 source);
    BlendControl& operator=(const BlendControl&) = delete;
  };

  struct Blend {
    BlendControl control;
    uint5 eva;
    uint5 evb;
    uint5 evy;
  } blend;
} regs;
