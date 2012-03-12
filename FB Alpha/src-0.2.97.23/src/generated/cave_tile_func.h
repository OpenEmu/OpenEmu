#define XFLIP 0
#define YFLIP 0
#define ROT 0

#define BPP 16
#define XSIZE 320
#define EIGHTBIT 1
#define ROWSCROLL 0
#define ROWSELECT 0
#define DOCLIP 0
#include "cave_tile_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "cave_tile_render.h"
#undef DOCLIP
#undef ROWSELECT
#undef ROWSCROLL
#define ROWSCROLL 1
#define ROWSELECT 0
#define DOCLIP 0
#include "cave_tile_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "cave_tile_render.h"
#undef DOCLIP
#undef ROWSELECT
#undef ROWSCROLL
#define ROWSCROLL 0
#define ROWSELECT 1
#define DOCLIP 0
#include "cave_tile_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "cave_tile_render.h"
#undef DOCLIP
#undef ROWSELECT
#undef ROWSCROLL
#undef EIGHTBIT

#undef XSIZE

#define XSIZE 384
#define EIGHTBIT 1
#define ROWSCROLL 0
#define ROWSELECT 0
#define DOCLIP 0
#include "cave_tile_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "cave_tile_render.h"
#undef DOCLIP
#undef ROWSELECT
#undef ROWSCROLL
#define ROWSCROLL 1
#define ROWSELECT 0
#define DOCLIP 0
#include "cave_tile_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "cave_tile_render.h"
#undef DOCLIP
#undef ROWSELECT
#undef ROWSCROLL
#define ROWSCROLL 0
#define ROWSELECT 1
#define DOCLIP 0
#include "cave_tile_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "cave_tile_render.h"
#undef DOCLIP
#undef ROWSELECT
#undef ROWSCROLL
#undef EIGHTBIT

#undef XSIZE

#undef BPP

#undef ROT

#undef YFLIP
#undef XFLIP

#include "cave_tile_func_table.h"
