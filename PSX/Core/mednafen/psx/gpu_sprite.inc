template<bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA, bool FlipX, bool FlipY>
void PS_GPU::DrawSprite(int32 x_arg, int32 y_arg, int32 w, int32 h, uint8 u_arg, uint8 v_arg, uint32 color, uint32 clut_offset)
{
 const int32 r = color & 0xFF;
 const int32 g = (color >> 8) & 0xFF;
 const int32 b = (color >> 16) & 0xFF;
 const uint16 fill_color = 0x8000 | ((r >> 3) << 0) | ((g >> 3) << 5) | ((b >> 3) << 10);

 int32 x_start, x_bound;
 int32 y_start, y_bound;
 uint8 u, v;
 int v_inc = 1, u_inc = 1;

 //printf("[GPU] Sprite: x=%d, y=%d, w=%d, h=%d\n", x_arg, y_arg, w, h);

 x_start = x_arg;
 x_bound = x_arg + w;

 y_start = y_arg;
 y_bound = y_arg + h;

 if(textured)
 {
  u = u_arg;
  v = v_arg;

  if(FlipX)
  {
   u_inc = -1;
   u |= 1;
  }
  // FIXME: Something weird happens when lower bit of u is set and we're not doing horizontal flip, but I'm not sure what it is exactly(needs testing)
  // It may only happen to the first pixel, so look for that case too during testing.
  //else
  // u = (u + 1) & ~1;

  if(FlipY)
  {
   v_inc = -1;
  }
 }

 if(x_start < ClipX0)
 {
  if(textured)
   u += (ClipX0 - x_start) * u_inc;

  x_start = ClipX0;
 }

 if(y_start < ClipY0)
 {
  if(textured)
   v += (ClipY0 - y_start) * v_inc;

  y_start = ClipY0;
 }

 if(x_bound > (ClipX1 + 1))
  x_bound = ClipX1 + 1;

 if(y_bound > (ClipY1 + 1))
  y_bound = ClipY1 + 1;

 if(y_bound > y_start && x_bound > x_start)
 {
  int32 suck_time = (x_bound - x_start) * (y_bound - y_start);

  if((BlendMode >= 0) || MaskEval_TA)
  {
   suck_time += ((((x_bound + 1) & ~1) - (x_start & ~1)) * (y_bound - y_start)) >> 1;
  }

  DrawTimeAvail -= suck_time;
 }


 //HeightMode && !dfe && ((y & 1) == ((DisplayFB_YStart + !field_atvs) & 1)) && !DisplayOff
 //printf("%d:%d, %d, %d ---- heightmode=%d displayfb_ystart=%d field_atvs=%d displayoff=%d\n", w, h, scanline, dfe, HeightMode, DisplayFB_YStart, field_atvs, DisplayOff);

 for(int32 y = y_start; y < y_bound; y++)
 {
  uint8 u_r;

  if(textured)
   u_r = u;

  if(!LineSkipTest(y))
  {
   for(int32 x = x_start; x < x_bound; x++)
   {
    if(textured)
    {
     uint16 fbw = GetTexel<TexMode_TA>(clut_offset, u_r, v);

     if(fbw)
     {
      if(TexMult)
      {
       fbw = ModTexel(fbw, r, g, b, 3, 2);
      }
      PlotPixel<BlendMode, MaskEval_TA, true>(x, y, fbw);
     }
    }
    else
     PlotPixel<BlendMode, MaskEval_TA, false>(x, y, fill_color);

    if(textured)
     u_r += u_inc;
   }
  }
  if(textured)
   v += v_inc;
 }
}

template<uint8 raw_size, bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA>
void PS_GPU::Command_DrawSprite(const uint32 *cb)
{
 int32 x, y;
 int32 w, h;
 uint8 u = 0, v = 0;
 uint32 color = 0;
 uint32 clut = 0;

 DrawTimeAvail -= 16;	// FIXME, correct time.

 color = *cb & 0x00FFFFFF;
 cb++;

 x = sign_x_to_s32(11, (*cb & 0xFFFF));
 y = sign_x_to_s32(11, (*cb >> 16));
 cb++;

 if(textured)
 {
  u = *cb & 0xFF;
  v = (*cb >> 8) & 0xFF;
  clut = ((*cb >> 16) & 0xFFFF) << 4;
  cb++;
 }

 switch(raw_size)
 {
  default:
  case 0:
	w = *cb & 0xFFFF;
	h = *cb >> 16;
	cb++;
	break;

  case 1:
	w = 1;
	h = 1;
	break;

  case 2:
	w = 8;
	h = 8;
	break;

  case 3:
	w = 16;
	h = 16;
	break;
 }

 //printf("SPRITE: %d %d %d -- %d %d\n", raw_size, x, y, w, h);

 x = sign_x_to_s32(11, x + OffsX);
 y = sign_x_to_s32(11, y + OffsY);

 switch(SpriteFlip & 0x3000)
 {
  case 0x0000:
	if(!TexMult || color == 0x808080)
  	 DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, false, false>(x, y, w, h, u, v, color, clut);
	else
	 DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, false, false>(x, y, w, h, u, v, color, clut);
	break;

  case 0x1000:
	if(!TexMult || color == 0x808080)
  	 DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, true, false>(x, y, w, h, u, v, color, clut);
	else
	 DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, true, false>(x, y, w, h, u, v, color, clut);
	break;

  case 0x2000:
	if(!TexMult || color == 0x808080)
  	 DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, false, true>(x, y, w, h, u, v, color, clut);
	else
	 DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, false, true>(x, y, w, h, u, v, color, clut);
	break;

  case 0x3000:
	if(!TexMult || color == 0x808080)
  	 DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, true, true>(x, y, w, h, u, v, color, clut);
	else
	 DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, true, true>(x, y, w, h, u, v, color, clut);
	break;
 }
}


