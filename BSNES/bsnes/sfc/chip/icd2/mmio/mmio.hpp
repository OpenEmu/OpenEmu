void render(const uint32 *source);

uint8 r6000_ly;   //SGB BIOS' cache of LY
uint8 r6000_row;  //SGB BIOS' cache of ROW
uint8 r6001;      //VRAM conversion
uint8 r6003;      //control port
uint8 r6004;      //joypad 1
uint8 r6005;      //joypad 2
uint8 r6006;      //joypad 3
uint8 r6007;      //joypad 4
uint8 r7000[16];  //JOYP packet data
unsigned r7800;   //VRAM offset
uint8 mlt_req;    //number of active joypads

struct LCD {
  uint32 buffer[4 * 160 * 8];  //four tile rows of linear video data
  uint16 output[320];          //one tile row of 2bpp video data
  unsigned row;                //active ICD2 rendering tile row
} lcd;
