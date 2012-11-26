struct i_group
{
 int32 u, v;
 int32 r, g, b;
 int32 dummy0[3];
};

struct i_deltas
{
 int32 du_dx, dv_dx;
 int32 dr_dx, dg_dx, db_dx;
 int32 dummy0[3];

 int32 du_dy, dv_dy;
 int32 dr_dy, dg_dy, db_dy;
 int32 dummy1[3];
};

static INLINE int64 MakePolyXFP(int32 x)
{
 return ((int64)x << 32) + ((1LL << 32) - (1 << 11));
}

static INLINE int64 MakePolyXFPStep(int32 dx, int32 dy)
{
 int64 ret;
 int64 dx_ex = (int64)dx << 32;

 if(dx_ex < 0)
  dx_ex -= dy - 1;

 if(dx_ex > 0)
  dx_ex += dy - 1;

 ret = dx_ex / dy;

 return(ret);
}

static INLINE int32 GetPolyXFP_Int(int64 xfp)
{
 return(xfp >> 32);
}

//#define CALCIS(x,y) ( A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y) )
#define CALCIS(x,y) (((B.x - A.x) * (C.y - B.y)) - ((C.x - B.x) * (B.y - A.y)))
static INLINE bool CalcIDeltas(i_deltas &idl, const tri_vertex &A, const tri_vertex &B, const tri_vertex &C)
{
 int64 num = ((int64)COORD_MF_INT(1)) << 32;
 int64 denom = CALCIS(x, y);
 int64 one_div;

 if(!denom)
  return(false);

//num -= abs(denom) - 1;
// num += abs(denom) >> 1;

 one_div = num / denom;

 idl.dr_dx = ((one_div * CALCIS(r, y)) + 0x00000000) >> 32;
 idl.dr_dy = ((one_div * CALCIS(x, r)) + 0x00000000) >> 32;

 idl.dg_dx = ((one_div * CALCIS(g, y)) + 0x00000000) >> 32;
 idl.dg_dy = ((one_div * CALCIS(x, g)) + 0x00000000) >> 32;

 idl.db_dx = ((one_div * CALCIS(b, y)) + 0x00000000) >> 32;
 idl.db_dy = ((one_div * CALCIS(x, b)) + 0x00000000) >> 32;

 idl.du_dx = ((one_div * CALCIS(u, y)) + 0x00000000) >> 32;
 idl.du_dy = ((one_div * CALCIS(x, u)) + 0x00000000) >> 32;

 idl.dv_dx = ((one_div * CALCIS(v, y)) + 0x00000000) >> 32;
 idl.dv_dy = ((one_div * CALCIS(x, v)) + 0x00000000) >> 32;

// printf("  du_dx=%08x, du_dy=%08x\n", idl.du_dx, idl.du_dy);

 return(true);
}
#undef CALCIS

template<bool goraud, bool textured>
static INLINE void AddIDeltas_DX(i_group &ig, const i_deltas &idl)
{
 if(textured)
 {
  ig.u += idl.du_dx;
  ig.v += idl.dv_dx;
 }

 if(goraud)
 {
  ig.r += idl.dr_dx;
  ig.g += idl.dg_dx;
  ig.b += idl.db_dx;
 }
}

template<bool goraud, bool textured>
static INLINE void AddIDeltas_DY(i_group &ig, const i_deltas &idl, int32 count = 1)
{
 if(textured)
 {
  ig.u += idl.du_dy * count;
  ig.v += idl.dv_dy * count;
 }

 if(goraud)
 {
  ig.r += idl.dr_dy * count;
  ig.g += idl.dg_dy * count;
  ig.b += idl.db_dy * count;
 }
}

template<bool goraud, bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA>
INLINE void PS_GPU::DrawSpan(int y, uint32 clut_offset, const int32 x_start, const int32 x_bound, const int32 bv_x, i_group ig, const i_deltas &idl)
{
  int32 xs = x_start, xb = x_bound;

  if(LineSkipTest(y))
   return;

  if(xs < xb)	// (xs != xb)
  {
   if(xs < ClipX0)
    xs = ClipX0;

   if(xb > (ClipX1 + 1))
    xb = ClipX1 + 1;

   if(xs < xb)
   {
    DrawTimeAvail -= (xb - xs);

    if(goraud || textured)
    {
     DrawTimeAvail -= (xb - xs);
    }
    else if((BlendMode >= 0) || MaskEval_TA)
    {
     DrawTimeAvail -= (((xb + 1) & ~1) - (xs & ~1)) >> 1;
    }
   }

   if(textured)
   {
    ig.u += (xs - bv_x) * idl.du_dx;
    ig.v += (xs - bv_x) * idl.dv_dx;
   }

   if(goraud)
   {
    ig.r += (xs - bv_x) * idl.dr_dx;
    ig.g += (xs - bv_x) * idl.dg_dx;
    ig.b += (xs - bv_x) * idl.db_dx;
   }

   for(int32 x = xs; x < xb; x++)
   {
    uint32 r, g, b;

    if(goraud)
    {
     r = RGB8SAT[COORD_GET_INT(ig.r)];
     g = RGB8SAT[COORD_GET_INT(ig.g)];
     b = RGB8SAT[COORD_GET_INT(ig.b)];
    }
    else
    {
     r = COORD_GET_INT(ig.r);
     g = COORD_GET_INT(ig.g);
     b = COORD_GET_INT(ig.b);
    }

    if(textured)
    {
     uint16 fbw = GetTexel<TexMode_TA>(clut_offset, COORD_GET_INT(ig.u), COORD_GET_INT(ig.v));

     if(fbw)
     {
      if(TexMult)
      {
       if(dtd)
        fbw = ModTexel(fbw, r, g, b, x & 3, y & 3);
       else
        fbw = ModTexel(fbw, r, g, b, 3, 2); //x & 3, y & 3);
      }
      PlotPixel<BlendMode, MaskEval_TA, true>(x, y, fbw);
     }
    }
    else
    {
     uint16 pix = 0x8000;

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
    
     PlotPixel<BlendMode, MaskEval_TA, false>(x, y, pix);
    }

    AddIDeltas_DX<goraud, textured>(ig, idl);
    //AddStep<goraud, textured>(perp_coord, perp_step);
   }
  }
}

template<bool goraud, bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA>
void PS_GPU::DrawTriangle(tri_vertex *vertices, uint32 clut)
{
 i_deltas idl;

#if 0
 vertices[0].y = COORD_MF_INT(rand());
 vertices[1].y = COORD_MF_INT(rand());
 vertices[2].y = COORD_MF_INT(rand());

 vertices[0].x = COORD_MF_INT(rand());
 vertices[1].x = COORD_MF_INT(rand());
 vertices[2].x = COORD_MF_INT(rand());
#endif

 if(vertices[2].y < vertices[1].y)
 {
  tri_vertex tmp = vertices[1];
  vertices[1] = vertices[2];
  vertices[2] = tmp;
 }

 if(vertices[1].y < vertices[0].y)
 {
  tri_vertex tmp = vertices[0];
  vertices[0] = vertices[1];
  vertices[1] = tmp;
 }

 if(vertices[2].y < vertices[1].y)
 {
  tri_vertex tmp = vertices[1];
  vertices[1] = vertices[2];
  vertices[2] = tmp;
 }

 if(vertices[0].y == vertices[2].y)
  return;

 if((vertices[2].y - vertices[0].y) >= 512)
 {
  //PSX_WARNING("[GPU] Triangle height too large: %d", (vertices[2].y - vertices[0].y));
  return;
 }

 if(abs(vertices[2].x - vertices[0].x) >= 1024 ||
    abs(vertices[2].x - vertices[1].x) >= 1024 ||
    abs(vertices[1].x - vertices[0].x) >= 1024)
 {
  //PSX_WARNING("[GPU] Triangle width too large: %d %d %d", abs(vertices[2].x - vertices[0].x), abs(vertices[2].x - vertices[1].x), abs(vertices[1].x - vertices[0].x));
  return;
 }

 if(!CalcIDeltas(idl, vertices[0], vertices[1], vertices[2]))
  return;

 // [0] should be top vertex, [2] should be bottom vertex, [1] should be off to the side vertex.
 //
 //
 int32 y_start = vertices[0].y;
 int32 y_middle = vertices[1].y;
 int32 y_bound = vertices[2].y;

 int64 base_coord;
 int64 base_step;

 int64 bound_coord_ul;
 int64 bound_coord_us;

 int64 bound_coord_ll;
 int64 bound_coord_ls;

 bool right_facing;
 //bool bottom_up;
 i_group ig;

 ig.u = COORD_MF_INT(vertices[0].u) + (1 << (COORD_FBS - 1));
 ig.v = COORD_MF_INT(vertices[0].v) + (1 << (COORD_FBS - 1));
 ig.r = COORD_MF_INT(vertices[0].r);
 ig.g = COORD_MF_INT(vertices[0].g);
 ig.b = COORD_MF_INT(vertices[0].b);

 base_coord = MakePolyXFP(vertices[0].x); //COORD_MF_INT(vertices[0].x) + ((1 << COORD_FBS) - 1);
 base_step = MakePolyXFPStep((vertices[2].x - vertices[0].x), (vertices[2].y - vertices[0].y)); //ROUND_HELPER(COORD_MF_INT(vertices[2].x - vertices[0].x), (vertices[2].y - vertices[0].y));

 bound_coord_ul = MakePolyXFP(vertices[0].x); // + ((1 << COORD_FBS) - 1);
 bound_coord_ll = MakePolyXFP(vertices[1].x); // + ((1 << COORD_FBS) - 1);

 //
 //
 //


 if(vertices[1].y == vertices[0].y)
 {
  bound_coord_us = 0;
  right_facing = (bool)(vertices[1].x > vertices[0].x);
 }
 else
 {
  bound_coord_us = MakePolyXFPStep((vertices[1].x - vertices[0].x), (vertices[1].y - vertices[0].y));
  right_facing = (bool)(bound_coord_us > base_step);
 }

 if(vertices[2].y == vertices[1].y)
  bound_coord_ls = 0;
 else
  bound_coord_ls = MakePolyXFPStep((vertices[2].x - vertices[1].x), (vertices[2].y - vertices[1].y));

 if(y_start < ClipY0)
 {
  int32 count = ClipY0 - y_start;

  y_start = ClipY0;
  base_coord += base_step * count;
  bound_coord_ul += bound_coord_us * count;

  AddIDeltas_DY<goraud, textured>(ig, idl, count);

  if(y_middle < ClipY0)
  {
   int32 count_ls = ClipY0 - y_middle;

   y_middle = ClipY0;
   bound_coord_ll += bound_coord_ls * count_ls;
  }
 }

 if(y_bound > (ClipY1 + 1))
 {
  y_bound = ClipY1 + 1;

  if(y_middle > y_bound)
   y_middle = y_bound;
 }

 if(right_facing)
 {
  for(int32 y = y_start; y < y_middle; y++)
  {
   DrawSpan<goraud, textured, BlendMode, TexMult, TexMode_TA, MaskEval_TA>(y, clut, GetPolyXFP_Int(base_coord), GetPolyXFP_Int(bound_coord_ul), vertices[0].x, ig, idl);
   base_coord += base_step;
   bound_coord_ul += bound_coord_us;
   AddIDeltas_DY<goraud, textured>(ig, idl);
  }

  for(int32 y = y_middle; y < y_bound; y++)
  {
   DrawSpan<goraud, textured, BlendMode, TexMult, TexMode_TA, MaskEval_TA>(y, clut, GetPolyXFP_Int(base_coord), GetPolyXFP_Int(bound_coord_ll), vertices[0].x, ig, idl);
   base_coord += base_step;
   bound_coord_ll += bound_coord_ls;
   AddIDeltas_DY<goraud, textured>(ig, idl);
  }
 }
 else
 {
  for(int32 y = y_start; y < y_middle; y++)
  {
   DrawSpan<goraud, textured, BlendMode, TexMult, TexMode_TA, MaskEval_TA>(y, clut, GetPolyXFP_Int(bound_coord_ul), GetPolyXFP_Int(base_coord), vertices[0].x, ig, idl);
   base_coord += base_step;
   bound_coord_ul += bound_coord_us;
   AddIDeltas_DY<goraud, textured>(ig, idl);
  }

  for(int32 y = y_middle; y < y_bound; y++)
  {
   DrawSpan<goraud, textured, BlendMode, TexMult, TexMode_TA, MaskEval_TA>(y, clut, GetPolyXFP_Int(bound_coord_ll), GetPolyXFP_Int(base_coord), vertices[0].x, ig, idl);
   base_coord += base_step;
   bound_coord_ll += bound_coord_ls;
   AddIDeltas_DY<goraud, textured>(ig, idl);
  }
 }

#if 0
 printf("[GPU] Vertices: %d:%d(r=%d, g=%d, b=%d) -> %d:%d(r=%d, g=%d, b=%d) -> %d:%d(r=%d, g=%d, b=%d)\n\n\n", vertices[0].x, vertices[0].y,
	vertices[0].r, vertices[0].g, vertices[0].b,
	vertices[1].x, vertices[1].y,
	vertices[1].r, vertices[1].g, vertices[1].b,
	vertices[2].x, vertices[2].y,
	vertices[2].r, vertices[2].g, vertices[2].b);
#endif
}

template<int numvertices, bool goraud, bool textured, int BlendMode, bool TexMult, uint32 TexMode_TA, bool MaskEval_TA>
void PS_GPU::Command_DrawPolygon(const uint32 *cb)
{
 const unsigned cb0 = cb[0];
 tri_vertex vertices[3];
 uint32 clut = 0;
 unsigned sv = 0;
 //uint32 tpage = 0;

 // Base timing is approximate, and could be improved.
 if(numvertices == 4 && InCmd == INCMD_QUAD)
  DrawTimeAvail -= (28 + 18);
 else
  DrawTimeAvail -= (64 + 18);

 if(goraud && textured)
  DrawTimeAvail -= 150 * 3;
 else if(goraud)
  DrawTimeAvail -= 96 * 3;
 else if(textured)
  DrawTimeAvail -= 60 * 3;

 if(numvertices == 4)
 {
  if(InCmd == INCMD_QUAD)
  {
   memcpy(&vertices[0], &InQuad_F3Vertices[1], 2 * sizeof(tri_vertex));
   clut = InQuad_clut;
   sv = 2;
  }
 }
 //else
 // memset(vertices, 0, sizeof(vertices));

 for(unsigned v = sv; v < 3; v++)
 {
  if(v == 0 || goraud)
  {
   uint32 raw_color = (*cb & 0xFFFFFF);

   vertices[v].r = raw_color & 0xFF;
   vertices[v].g = (raw_color >> 8) & 0xFF;
   vertices[v].b = (raw_color >> 16) & 0xFF;

   cb++;
  }
  else
  {
   vertices[v].r = vertices[0].r;
   vertices[v].g = vertices[0].g;
   vertices[v].b = vertices[0].b;
  }

  vertices[v].x = sign_x_to_s32(11, ((int16)(*cb & 0xFFFF))) + OffsX;
  vertices[v].y = sign_x_to_s32(11, ((int16)(*cb >> 16))) + OffsY;
  cb++;

  if(textured)
  {
   vertices[v].u = (*cb & 0xFF);
   vertices[v].v = (*cb >> 8) & 0xFF;

   if(v == 0)
   {
    clut = ((*cb >> 16) & 0xFFFF) << 4;
   }

   cb++;
  }
 }

 if(numvertices == 4)
 {
  if(InCmd == INCMD_QUAD)
  {
   InCmd = INCMD_NONE;
  }
  else
  {
   InCmd = INCMD_QUAD;
   InCmd_CC = cb0 >> 24;
   memcpy(&InQuad_F3Vertices[0], &vertices[0], sizeof(tri_vertex) * 3);
   InQuad_clut = clut;
  }
 }

 DrawTriangle<goraud, textured, BlendMode, TexMult, TexMode_TA, MaskEval_TA>(vertices, clut);
}

