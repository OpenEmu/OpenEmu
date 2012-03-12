#define EIGHTBIT 1
#define ROT 0

#define BPP 16
#define XSIZE 320
#define ZOOM 0
#define XFLIP 0
#define ZBUFFER 0
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render.h"
#undef ZBUFFER
#undef XFLIP
#define XFLIP 1
#define ZBUFFER 0
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render.h"
#undef ZBUFFER
#undef XFLIP
#undef ZOOM

#define XFLIP 0
#define ZOOM 1
#define ZBUFFER 0
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#undef ZOOM

#define ZOOM 2
#define ZBUFFER 0
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#undef ZOOM

#undef XFLIP
#undef XSIZE

#define XSIZE 384
#define ZOOM 0
#define XFLIP 0
#define ZBUFFER 0
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render.h"
#undef ZBUFFER
#undef XFLIP
#define XFLIP 1
#define ZBUFFER 0
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render.h"
#undef ZBUFFER
#undef XFLIP
#undef ZOOM

#define XFLIP 0
#define ZOOM 1
#define ZBUFFER 0
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#undef ZOOM

#define ZOOM 2
#define ZBUFFER 0
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 1
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 2
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#define ZBUFFER 3
#include "cave_sprite_render_zoom.h"
#undef ZBUFFER
#undef ZOOM

#undef XFLIP
#undef XSIZE

#undef BPP

#undef ROT

#undef EIGHTBIT

#include "cave_sprite_func_table.h"
