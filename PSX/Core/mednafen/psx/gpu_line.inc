struct line_fxp_coord
{
 int64 x, y;
 int32 r, g, b;
};

struct line_fxp_step
{
 int64 dx_dk, dy_dk;
 int32 dr_dk, dg_dk, db_dk;
};

enum { Line_XY_FractBits = 32 };
enum { Line_RGB_FractBits = 12 };

template<bool goraud>
static INLINE void LinePointToFXPCoord(const line_point &point, const line_fxp_step &step, line_fxp_coord &coord)
{
 coord.x = ((int64)point.x << Line_XY_FractBits) | (1LL << (Line_XY_FractBits - 1));
 coord.y = ((int64)point.y << Line_XY_FractBits) | (1LL << (Line_XY_FractBits - 1));

 if(goraud)
 {
  coord.r = (point.r << Line_RGB_FractBits) | (1 << (Line_RGB_FractBits - 1));
  coord.g = (point.g << Line_RGB_FractBits) | (1 << (Line_RGB_FractBits - 1));
  coord.b = (point.b << Line_RGB_FractBits) | (1 << (Line_RGB_FractBits - 1));
 }

 // Not sure if this is correct or just roughly corresponds to behavior of real system(need more testing):
 if(step.dx_dk < 0)
  coord.x--;

 if(step.dy_dk < 0)
  coord.y--;

 if(step.dr_dk < 0)
  coord.r--;

 if(step.dg_dk < 0)
  coord.g--;

 if(step.db_dk < 0)
  coord.b--;
}

template<typename T, unsigned bits>
static INLINE T LineDivide(T delta, int32 dk)
{
 delta <<= bits;

 if(delta < 0)
  delta -= dk - 1;
 if(delta > 0)
  delta += dk - 1;

 return(delta / dk);
}

template<bool goraud>
static INLINE void LinePointsToFXPStep(const line_point &point0, const line_point &point1, const int32 dk, line_fxp_step &step)
{
 if(!dk)
 {
  step.dx_dk = 0;
  step.dy_dk = 0;

  if(goraud)
  {
   step.dr_dk = 0;
   step.dg_dk = 0;
   step.db_dk = 0;
  }
  return;
 }

 step.dx_dk = LineDivide<int64, Line_XY_FractBits>(point1.x - point0.x, dk);
 step.dy_dk = LineDivide<int64, Line_XY_FractBits>(point1.y - point0.y, dk);

 if(goraud)
 {
  step.dr_dk = LineDivide<int32, Line_RGB_FractBits>(point1.r - point0.r, dk);
  step.dg_dk = LineDivide<int32, Line_RGB_FractBits>(point1.g - point0.g, dk);
  step.db_dk = LineDivide<int32, Line_RGB_FractBits>(point1.b - point0.b, dk);
 }
}

template<bool goraud>
static INLINE void AddLineStep(line_fxp_coord &point, const line_fxp_step &step, int32 count = 1)
{
 point.x += step.dx_dk * count;
 point.y += step.dy_dk * count;
 
 if(goraud)
 {
  point.r += step.dr_dk * count;
  point.g += step.dg_dk * count;
  point.b += step.db_dk * count;
 }
}

template<bool goraud, int BlendMode, bool MaskEval_TA>
void PS_GPU::DrawLine(line_point *points)
{
 int32 i_dx;
 int32 i_dy;
 int32 k;
 line_fxp_coord cur_point;
 line_fxp_step step;

 i_dx = abs(points[1].x - points[0].x);
 i_dy = abs(points[1].y - points[0].y);
 k = (i_dx > i_dy) ? i_dx : i_dy;

 if(i_dx >= 1024)
 {
  //PSX_WARNING("[GPU] Line too long: i_dx=%d", i_dx);
  return;
 }

 if(i_dy >= 512)
 {
  //PSX_WARNING("[GPU] Line too long: i_dy=%d", i_dy);
  return;
 }

 // May not be correct(do tests for the case of k == i_dy on real thing.
 if(points[0].x > points[1].x)
 {
  line_point tmp = points[1];

  points[1] = points[0];
  points[0] = tmp;  
 }

 DrawTimeAvail -= k * ((BlendMode >= 0) ? 2 : 1);

 //
 //
 //

 LinePointsToFXPStep<goraud>(points[0], points[1], k, step);
 LinePointToFXPCoord<goraud>(points[0], step, cur_point);
 
 //
 //
 //
 for(int32 i = 0; i <= k; i++)	// <= is not a typo.
 {
  // Sign extension is not necessary here for x and y, due to the maximum values that ClipX1 and ClipY1 can contain.
  const int32 x = (cur_point.x >> Line_XY_FractBits) & 2047;
  const int32 y = (cur_point.y >> Line_XY_FractBits) & 2047;
  uint16 pix = 0x8000;

  if(!LineSkipTest(y))
  {
   uint8 r, g, b;

   if(goraud)
   {
    r = cur_point.r >> Line_RGB_FractBits;
    g = cur_point.g >> Line_RGB_FractBits;
    b = cur_point.b >> Line_RGB_FractBits;
   }
   else
   {
    r = points[0].r;
    g = points[0].g;
    b = points[0].b;
   }

   if(goraud && dtd)
   {
    pix |= DitherLUT[y & 3][x & 3][r] << 0;
    pix |= DitherLUT[y & 3][x & 3][g] << 5;
    pix |= DitherLUT[y & 3][x & 3][b] << 10;
   }
   else
   {
    pix |= (r >> 3) << 0;
    pix |= (g >> 3) << 5;
    pix |= (b >> 3) << 10;
   }

   // FIXME: There has to be a faster way than checking for being inside the drawing area for each pixel.
   if(x >= ClipX0 && x <= ClipX1 && y >= ClipY0 && y <= ClipY1)
    PlotPixel<BlendMode, MaskEval_TA, false>(x, y, pix);
  }

  AddLineStep<goraud>(cur_point, step);
 }
}

template<bool polyline, bool goraud, int BlendMode, bool MaskEval_TA>
void PS_GPU::Command_DrawLine(const uint32 *cb)
{
 const uint8 cc = cb[0] >> 24; // For pline handling later.
 line_point points[2];

 DrawTimeAvail -= 16;	// FIXME, correct time.

 if(polyline && InCmd == INCMD_PLINE)
 {
  //printf("PLINE N\n");
  points[0] = InPLine_PrevPoint;
 }
 else
 {
  points[0].r = (*cb >> 0) & 0xFF;
  points[0].g = (*cb >> 8) & 0xFF;
  points[0].b = (*cb >> 16) & 0xFF;
  cb++;

  points[0].x = sign_x_to_s32(11, ((*cb >> 0) & 0xFFFF)) + OffsX;
  points[0].y = sign_x_to_s32(11, ((*cb >> 16) & 0xFFFF)) + OffsY;
  cb++;
 }

 if(goraud)
 {
  points[1].r = (*cb >> 0) & 0xFF;
  points[1].g = (*cb >> 8) & 0xFF;
  points[1].b = (*cb >> 16) & 0xFF;
  cb++;
 }
 else
 {
  points[1].r = points[0].r;
  points[1].g = points[0].g;
  points[1].b = points[0].b;
 }

 points[1].x = sign_x_to_s32(11, ((*cb >> 0) & 0xFFFF)) + OffsX;
 points[1].y = sign_x_to_s32(11, ((*cb >> 16) & 0xFFFF)) + OffsY;
 cb++;

 if(polyline)
 {
  InPLine_PrevPoint = points[1];

  if(InCmd != INCMD_PLINE)
  {
   InCmd = INCMD_PLINE;
   InCmd_CC = cc;
  }
 }

 DrawLine<goraud, BlendMode, MaskEval_TA>(points);
}

