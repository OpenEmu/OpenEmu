#define ROT 0
#define BPP 16
#define FLIP 0
#define ZOOM 0
#define ZBUFFER 0

#define TRANS 0

#define ROWSCROLL 0
#define DOCLIP 0
#include "psikyo_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "psikyo_render.h"
#undef DOCLIP
#undef ROWSCROLL
#define ROWSCROLL 1
#define DOCLIP 0
#include "psikyo_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "psikyo_render.h"
#undef DOCLIP
#undef ROWSCROLL

#undef TRANS
#define TRANS 15

#define ROWSCROLL 0
#define DOCLIP 0
#include "psikyo_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "psikyo_render.h"
#undef DOCLIP
#undef ROWSCROLL
#define ROWSCROLL 1
#define DOCLIP 0
#include "psikyo_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "psikyo_render.h"
#undef DOCLIP
#undef ROWSCROLL

#undef TRANS
#define TRANS -1

#define ROWSCROLL 0
#define DOCLIP 0
#include "psikyo_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "psikyo_render.h"
#undef DOCLIP
#undef ROWSCROLL
#define ROWSCROLL 1
#define DOCLIP 0
#include "psikyo_render.h"
#undef DOCLIP
#define DOCLIP 1
#include "psikyo_render.h"
#undef DOCLIP
#undef ROWSCROLL

#undef TRANS
#undef ZBUFFER
#undef ZOOM
#undef FLIP
#undef BPP
#undef ROT

#include "psikyo_tile_func_table.h"
