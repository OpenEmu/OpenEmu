//render.cpp
inline void render_line_mode0();
inline void render_line_mode1();
inline void render_line_mode2();
inline void render_line_mode3();
inline void render_line_mode4();
inline void render_line_mode5();
inline void render_line_mode6();
inline void render_line_mode7();

//cache.cpp
enum { COLORDEPTH_4 = 0, COLORDEPTH_16 = 1, COLORDEPTH_256 = 2 };
enum { TILE_2BIT = 0, TILE_4BIT = 1, TILE_8BIT = 2 };

struct pixel_t {
  //bgr555 color data for main/subscreen pixels: 0x0000 = transparent / use palette color # 0
  //needs to be bgr555 instead of palette index for direct color mode ($2130 bit 0) to work
  uint16 src_main, src_sub;
  //indicates source of palette # for main/subscreen (BG1-4, OAM, or back)
  uint8  bg_main,  bg_sub;
  //color_exemption -- true when bg == OAM && palette index >= 192, disables color add/sub effects
  uint8  ce_main,  ce_sub;
  //priority level of src_n. to set src_n,
  //the priority of the pixel must be >pri_n
  uint8  pri_main, pri_sub;
} pixel_cache[256];

uint8 *bg_tiledata[3];
uint8 *bg_tiledata_state[3];  //0 = valid, 1 = dirty

template<unsigned color_depth> void render_bg_tile(uint16 tile_num);
inline void flush_pixel_cache();
void alloc_tiledata_cache();
void flush_tiledata_cache();
void free_tiledata_cache();

//windows.cpp
struct window_t {
  uint8 main[256], sub[256];
} window[6];

void build_window_table(uint8 bg, bool mainscreen);
void build_window_tables(uint8 bg);

//bg.cpp
struct {
  uint16 tw,  th;  //tile width, height
  uint16 mx,  my;  //screen mask x, y
  uint16 scx, scy; //sc index offsets
} bg_info[4];
void update_bg_info();

template<unsigned bg> uint16 bg_get_tile(uint16 x, uint16 y);
template<unsigned mode, unsigned bg, unsigned color_depth> void render_line_bg(uint8 pri0_pos, uint8 pri1_pos);

//oam.cpp
struct sprite_item {
  uint8  width, height;
  uint16 x, y;
  uint8  character;
  bool   use_nameselect;
  bool   vflip, hflip;
  uint8  palette;
  uint8  priority;
  bool   size;
} sprite_list[128];
bool sprite_list_valid;
unsigned active_sprite;

uint8 oam_itemlist[32];
struct oam_tileitem {
  uint16 x, y, pri, pal, tile;
  bool   hflip;
} oam_tilelist[34];

enum { OAM_PRI_NONE = 4 };
uint8 oam_line_pal[256], oam_line_pri[256];

void update_sprite_list(unsigned addr, uint8 data);
void build_sprite_list();
bool is_sprite_on_scanline();
void load_oam_tiles();
void render_oam_tile(int tile_num);
void render_line_oam_rto();
void render_line_oam(uint8 pri0_pos, uint8 pri1_pos, uint8 pri2_pos, uint8 pri3_pos);

//mode7.cpp
template<unsigned bg> void render_line_mode7(uint8 pri0_pos, uint8 pri1_pos);

//addsub.cpp
inline uint16 addsub(uint32 x, uint32 y, bool halve);

//line.cpp
inline uint16 get_palette(uint8 index);
inline uint16 get_direct_color(uint8 p, uint8 t);
inline uint16 get_pixel_normal(uint32 x);
inline uint16 get_pixel_swap(uint32 x);
void   render_line_output();
void   render_line_clear();
