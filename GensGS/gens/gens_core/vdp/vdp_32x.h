#ifndef GENS_VDP_32X_H
#define GENS_VDP_32X_H

#ifdef __cplusplus
extern "C" {
#endif

struct VDP_32X_t
{
	unsigned int Mode;
	unsigned int State;
	unsigned int AF_Data;
	unsigned int AF_St;
	unsigned int AF_Len;
	unsigned int AF_Line;
};

extern unsigned short _32X_Palette_16B[0x10000];
extern unsigned int _32X_Palette_32B[0x10000];
extern unsigned char _32X_VDP_Ram[0x100 * 1024];
extern unsigned short _32X_VDP_CRam[0x100];
extern unsigned short _32X_VDP_CRam_Ajusted[0x100];
extern unsigned int _32X_VDP_CRam_Ajusted32[0x100];

extern struct VDP_32X_t _32X_VDP;

void _32X_VDP_Reset(void);
void _32X_VDP_Draw(int FB_Num);

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDP_32X_H */
