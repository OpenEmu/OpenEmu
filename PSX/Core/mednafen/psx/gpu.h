// WARNING WARNING WARNING:  ONLY use CanRead() method of BlitterFIFO, and NOT CanWrite(), since the FIFO is larger than the actual PS1 GPU FIFO to accommodate
// our lack of fancy superscalarish command sequencer.

#ifndef __MDFN_PSX_GPU_H
#define __MDFN_PSX_GPU_H

#include "../cdrom/SimpleFIFO.h"

namespace MDFN_IEN_PSX
{

class PS_GPU;

struct CTEntry
{
 uint8 len;
 uint8 fifo_fb_len;
 bool ss_cmd;
 const char *name;
 void (PS_GPU::*func[8])(const uint32 *cb);
};

struct tri_vertex
{
 int32 x, y;
 int32 u, v;
 int32 r, g, b;
};

struct i_group;
struct i_deltas;

struct line_point
{
 int32 x, y;
 uint8 r, g, b;
};

class PS_GPU
{
 public:

 PS_GPU(bool pal_clock_and_tv);
 ~PS_GPU();

 void Power(void);

 void ResetTS(void);

 void StartFrame(EmulateSpecStruct *espec);

 pscpu_timestamp_t Update(const pscpu_timestamp_t timestamp);

 void Write(const pscpu_timestamp_t timestamp, uint32 A, uint32 V);

 INLINE bool CalcFIFOReadyBit(void)
 {
  if(InCmd & (INCMD_PLINE | INCMD_QUAD))
   return(false);

  if(BlitterFIFO.CanRead() == 0)
   return(true);

  if(InCmd & (INCMD_FBREAD | INCMD_FBWRITE))
   return(false);

  if(BlitterFIFO.CanRead() >= Commands[0][BlitterFIFO.ReadUnit(true) >> 24].fifo_fb_len)
   return(false);

  return(true);
 }

 INLINE bool DMACanWrite(void)
 {
  return CalcFIFOReadyBit();
 }

 INLINE void AbortDMA(void)
 {
  BlitterFIFO.Flush();
  InCmd = INCMD_NONE;
 }

 void WriteDMA(uint32 V);

 uint32 Read(const pscpu_timestamp_t timestamp, uint32 A);

 inline int32 GetScanlineNum(void)
 {
  return(scanline);
 } 

 INLINE uint16 PeekRAM(uint32 A)
 {
  return(GPURAM[(A >> 10) & 0x1FF][A & 0x3FF]);
 }

 INLINE void PokeRAM(uint32 A, uint16 V)
 {
  GPURAM[(A >> 10) & 0x1FF][A & 0x3FF] = V;
 }

 private:

 void ProcessFIFO(void);
 void WriteCB(uint32 data);
 void SoftReset(void);

 // Y, X
 uint16 GPURAM[512][1024];

 uint32 DMAControl;

 //
 // Drawing stuff
 //
 //int32 TexPageX;	// 0, 64, 128, 192, etc up to 960
 //int32 TexPageY;	// 0 or 256
 //uint32 abr;		// Semi-transparency mode(0~3)
 //bool dtd;		// Dithering enable

 int32 ClipX0;
 int32 ClipY0;
 int32 ClipX1;
 int32 ClipY1;

 int32 OffsX;
 int32 OffsY;

 bool dtd;
 bool dfe;

 uint32 MaskSetOR;
 uint32 MaskEvalAND;

 uint8 tww, twh, twx, twy;
 struct
 {
  uint8 TexWindowXLUT_Pre[16];
  uint8 TexWindowXLUT[256];
  uint8 TexWindowXLUT_Post[16];
 };

 struct
 {
  uint8 TexWindowYLUT_Pre[16];
  uint8 TexWindowYLUT[256];
  uint8 TexWindowYLUT_Post[16];
 };
 void RecalcTexWindowLUT(void);
 
 int32 TexPageX;
 int32 TexPageY;

 uint32 SpriteFlip;

 uint32 abr;
 uint32 TexMode;

 struct
 {
  uint8 RGB8SAT_Under[256];
  uint8 RGB8SAT[256];
  uint8 RGB8SAT_Over[256];
 };

 uint8 DitherLUT[4][4][512];	// Y, X, 8-bit source value(256 extra for saturation)

 bool LineSkipTest(unsigned y);

 template<int BlendMode, bool MaskEval_TA, bool textured>
 void PlotPixel(int32 x, int32 y, uint16 pix);

 template<uint32 TexMode_TA>
 uint16 GetTexel(uint32 clut_offset, int32 u, int32 v);

 uint16 ModTexel(uint16 texel, int32 r, int32 g, int32 b, const int32 dither_x, const int32 dither_y);

 template<bool goraud, bool textured, int BlendMode, bool TexMult, uint32 TexMode, bool MaskEval_TA>
 void DrawSpan(int y, uint32 clut_offset, const int32 x_start, const int32 x_bound, const int32 bv_x, i_group ig, const i_deltas &idl);

 template<bool shaded, bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA>
 void DrawTriangle(tri_vertex *vertices, uint32 clut);

 template<int numvertices, bool shaded, bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA>
 void Command_DrawPolygon(const uint32 *cb);

 template<bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA, bool FlipX, bool FlipY>
 void DrawSprite(int32 x_arg, int32 y_arg, int32 w, int32 h, uint8 u_arg, uint8 v_arg, uint32 color, uint32 clut_offset);

 template<uint8 raw_size, bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA>
 void Command_DrawSprite(const uint32 *cb);

 template<bool goraud, int BlendMode, bool MaskEval_TA>
 void DrawLine(line_point *vertices);

 template<bool polyline, bool goraud, int BlendMode, bool MaskEval_TA>
 void Command_DrawLine(const uint32 *cb);

 void Command_ClearCache(const uint32 *cb);

 void Command_FBFill(const uint32 *cb);
 void Command_FBCopy(const uint32 *cb);
 void Command_FBWrite(const uint32 *cb);
 void Command_FBRead(const uint32 *cb);

 void Command_DrawMode(const uint32 *cb);
 void Command_TexWindow(const uint32 *cb);
 void Command_Clip0(const uint32 *cb);
 void Command_Clip1(const uint32 *cb);
 void Command_DrawingOffset(const uint32 *cb);
 void Command_MaskSetting(const uint32 *cb);

 static CTEntry Commands[4][256];

 SimpleFIFO<uint32> BlitterFIFO;

 uint32 DataReadBuffer;

 //
 //
 //
 // Powers of 2 for faster multiple equality testing(just for multi-testing; InCmd itself will only contain 0, or a power of 2).
 enum
 {
  INCMD_NONE = 0,
  INCMD_PLINE = (1 << 0),
  INCMD_QUAD = (1 << 1),
  INCMD_FBWRITE = (1 << 2),
  INCMD_FBREAD = (1 << 3)
 };
 uint8 InCmd;
 uint8 InCmd_CC;

 tri_vertex InQuad_F3Vertices[3];
 uint32 InQuad_clut;

 line_point InPLine_PrevPoint;

 uint32 FBRW_X;
 uint32 FBRW_Y;
 uint32 FBRW_W;
 uint32 FBRW_H;
 uint32 FBRW_CurY;
 uint32 FBRW_CurX;

 //
 // Display Parameters
 //
 uint32 DisplayMode;

 bool DisplayOff;
 uint32 DisplayFB_XStart;
 uint32 DisplayFB_YStart;

 uint32 HorizStart;
 uint32 HorizEnd;

 uint32 VertStart;
 uint32 VertEnd;

 //
 // Display work vars
 //
/*
 uint32 DisplayMode_Latch;

 bool DisplayOff_Latch;
 uint32 DisplayFB_XStart_Latch;
 uint32 DisplayFB_YStart_Latch;

 uint32 HorizStart_Latch;
 uint32 HorizEnd_Latch;

 uint32 VertStart_Latch;
 uint32 VertEnd_Latch;
*/
 uint32 DisplayFB_CurYOffset;
 uint32 DisplayFB_CurLineYReadout;

 uint32 DisplayHeightCounter;

 //
 //
 //
 uint32 LinesPerField;
 uint32 VisibleStartLine;
 bool FrameInterlaced;
 bool PALMode;
 bool HeightMode;
 uint32 scanline;
 bool field;
 bool field_atvs;
 bool PhaseChange;

 uint32 DotClockCounter;

 uint64 GPUClockCounter;
 uint32 GPUClockRatio;
 int32 LineClockCounter;
 int32 LinePhase;

 int32 DrawTimeAvail;

 pscpu_timestamp_t lastts;

 //
 //
 //
 EmulateSpecStruct *espec;
 MDFN_Surface *surface;
 MDFN_Rect *DisplayRect;
 MDFN_Rect *LineWidths;
 bool skip;
 bool HardwarePALType;

 uint32 OutputLUT[32768];
 void ReorderRGB_Var(uint32 out_Rshift, uint32 out_Gshift, uint32 out_Bshift, bool bpp24, const uint16 *src, uint32 *dest, const int32 dx_start, const int32 dx_end, int32 fb_x);

 template<uint32 out_Rshift, uint32 out_Gshift, uint32 out_Bshift>
 void ReorderRGB(bool bpp24, const uint16 *src, uint32 *dest, const int32 dx_start, const int32 dx_end, int32 fb_x) NO_INLINE;
};

}
#endif
