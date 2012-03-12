
// deco16 tilemap routines

extern UINT16 *deco16_pf_control[2];
extern UINT8 *deco16_pf_ram[4];
extern UINT8 *deco16_pf_rowscroll[4];

extern UINT16 deco16_priority;

void deco16_set_bank_callback(INT32 tmap, INT32 (*callback)(const INT32 bank));
void deco16_set_color_base(INT32 tmap, INT32 base);
void deco16_set_color_mask(INT32 tmap, INT32 mask);
void deco16_set_transparency_mask(INT32 tmap, INT32 mask);
void deco16_set_gfxbank(INT32 tmap, INT32 small, INT32 big);
void deco16_set_global_offsets(INT32 x, INT32 y);

void deco16_set_scroll_offs(INT32 tmap, INT32 size, INT32 offsetx, INT32 offsety);

INT32 deco16_get_tilemap_size(INT32 tmap);

extern UINT8 *deco16_prio_map;
void deco16_clear_prio_map();
void deco16_draw_prio_sprite(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 pri);
void deco16_draw_prio_sprite(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 pri, INT32 spri);
void deco16_draw_alphaprio_sprite(UINT32 *palette, UINT8 *gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 pri, INT32 spri, INT32 alpha);

void deco16_set_graphics(UINT8 *gfx0, INT32 len0, UINT8 *gfx1, INT32 len1, UINT8 *gfx2, INT32 len2);
void deco16_set_graphics(INT32 num, UINT8 *gfx, INT32 len, INT32 size /*tile size*/); // individual bank

void deco16Init(INT32 no_pf34, INT32 split, INT32 full_width);
void deco16Reset();
void deco16Exit();

void deco16Scan();

void deco16_pf12_update();
void deco16_pf34_update();

#define DECO16_LAYER_OPAQUE		0x010000
#define DECO16_LAYER_PRIORITY(x)	((x) & 0xff)
#define DECO16_LAYER_8BITSPERPIXEL	0x100000
#define DECO16_LAYER_5BITSPERPIXEL	0x200000
#define DECO16_LAYER_4BITSPERPIXEL	0x000000 	// just to clarify
#define DECO16_LAYER_TRANSMASK0		0x000100
#define DECO16_LAYER_TRANSMASK1		0x000000

void deco16_draw_layer(INT32 tmap, UINT16 *dest, INT32 flags);

void deco16_tile_decode(UINT8 *src, UINT8 *dst, INT32 len, INT32 type);
void deco16_sprite_decode(UINT8 *gfx, INT32 len);

void deco16_palette_recalculate(UINT32 *palette, UINT8 *pal);

#define deco16_write_control_word(num, addr, a, d)		\
	if ((addr & 0xfffffff0) == a) {				\
		deco16_pf_control[num][(addr & 0x0f)/2] = d;	\
		return;						\
	}


#define deco16_read_control_word(num, addr, a)			\
	if ((addr & 0xfffffff0) == a) {				\
		return deco16_pf_control[num][(addr & 0x0f)/2];	\
	}


#define deco16ic_71_read()	(0xffff)

// common sound hardware...

extern INT32 deco16_soundlatch;

void deco16SoundReset();
void deco16SoundInit(UINT8 *rom, UINT8 *ram, INT32 huc_clock, INT32 ym2203, void (ym2151_port)(UINT32,UINT32), double ym2151vol, INT32 msmclk0, double msmvol0, INT32 msmclk1, double msmvol1);
void deco16SoundExit();
void deco16SoundUpdate(INT16 *buf, INT32 len);
void deco16SoundScan(INT32 nAction, INT32 *pnMin);


// decrypt routines

void deco56_decrypt_gfx(UINT8 *rom, INT32 len);
void deco74_decrypt_gfx(UINT8 *rom, INT32 len);
void deco56_remap_gfx(UINT8 *rom, INT32 len);

void deco102_decrypt_cpu(UINT8 *data, UINT8 *ops, INT32 size, INT32 address_xor, INT32 data_select_xor, INT32 opcode_select_xor);

void deco156_decrypt(UINT8 *src, INT32 len);

// protection routines

extern UINT16 *deco16_prot_ram;
extern UINT16 *deco16_prot_inputs;
extern UINT16 *deco16_buffer_ram;
extern INT32 deco16_vblank;

void deco16_66_prot_w(INT32 offset, UINT16 data, INT32 mask); // mutant fighter
UINT16 deco16_66_prot_r(INT32 offset);

void deco16_60_prot_w(INT32 offset, UINT16 data, INT32 mask); // edward randy
UINT16 deco16_60_prot_r(INT32 offset);

UINT16 deco16_104_cninja_prot_r(INT32 offset); // caveman ninja

UINT16 deco16_146_funkyjet_prot_r(INT32 offset); // funky jet

void deco16_104_rohga_prot_w(INT32 offset, UINT16 data, INT32 mask); // rohga
UINT16 deco16_104_rohga_prot_r(INT32 offset);

UINT16 deco16_104_prot_r(INT32 offset);

void deco16_146_nitroball_prot_w(INT32 offset, UINT16 data, INT32 mask);
UINT16 deco16_146_nitroball_prot_r(INT32 offset);

void deco16_146_fghthist_prot_w(INT32 offset, UINT32 data, UINT32 mem_mask);
UINT32 deco16_146_fghthist_prot_r(INT32 offset);

