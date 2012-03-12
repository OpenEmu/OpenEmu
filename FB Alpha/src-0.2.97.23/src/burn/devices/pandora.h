extern INT32 pandora_flipscreen;

void pandora_set_clear(INT32 clear);
void pandora_update(UINT16 *dest);
void pandora_buffer_sprites();
void pandora_init(UINT8 *ram, UINT8 *gfx, INT32 color_offset, INT32 x, INT32 y);
void pandora_exit();
