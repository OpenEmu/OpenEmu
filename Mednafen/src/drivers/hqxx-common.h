void hq4x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int srcBpL, int BpL);
void hq3x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int srcBpL, int BpL);
void hq2x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int srcBpL, int BpL);

#ifdef HQXX_INTERNAL

static const int   Ymask = 0x00FF0000;
static const int   Umask = 0x0000FF00;
static const int   Vmask = 0x000000FF;
static const  int   trY   = 0x00300000;
static const  int   trU   = 0x00000700;
static const  int   trV   = 0x00000006;

static inline unsigned int hqxx_RGB_to_YUV(unsigned int value)
{
 int r,g,b;
 int Y, u, v;

#if 0
 r = (value >> 0) & 0xF8;
 g = (value >> 8) & 0xFC;
 b = (value >> 16) & 0xF8;

 Y = (r + g + b) >> 2;
 u = 128 + ((r - b) >> 2);
 v = 128 + ((-r + 2*g - b) >> 3);
#endif

 r = (value >> (16 + 2)) & 0x3E;
 g = (value >> ( 8 + 2)) & 0x3F;
 b = (value >> ( 0 + 2)) & 0x3E;

 Y = (r + g + b);
 u = 128 + (r - b);
 v = 128 + ((-r + 2*g - b) >> 1);

 return((Y<<16) + (u<<8) + v);
}
#endif
