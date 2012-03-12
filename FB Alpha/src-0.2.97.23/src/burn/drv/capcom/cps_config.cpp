#include "cps.h"

struct GfxRange {
	INT32 Type;
	INT32 Start;
	INT32 End;
	INT32 Bank;
};

static const struct GfxRange *GfxBankMapper = NULL;
static INT32 GfxBankSizes[4] = { 0, 0, 0, 0 };

static const struct GfxRange mapper_LWCHR_table[] = {
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SCROLL1, 0x00000, 0x1ffff, 0 },

	{ GFXTYPE_STARS,   0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL2, 0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 1 },
	{ 0              ,       0,       0, 0 }
};

static const struct GfxRange mapper_LW621_table[] = {
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SCROLL1, 0x00000, 0x1ffff, 0 },

	{ GFXTYPE_STARS,   0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL2, 0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 1 },
	{ 0              ,       0,       0, 0 }
};

static const struct GfxRange mapper_DM620_table[] = {
	{ GFXTYPE_SCROLL3, 0x8000, 0xbfff, 1 },

	{ GFXTYPE_SPRITES, 0x2000, 0x3fff, 2 },

	{ GFXTYPE_STARS | GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 0 },
	{ 0                                                                                    ,       0,       0, 0 }
};

static const struct GfxRange mapper_DM22A_table[] = {
	{ GFXTYPE_SPRITES, 0x00000, 0x01fff, 0 },
	{ GFXTYPE_SCROLL1, 0x02000, 0x03fff, 0 },

	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 1 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 2 },

	{ GFXTYPE_SPRITES, 0x02000, 0x03fff, 3 },
	{ 0              ,       0,       0, 0 }
};

static const struct GfxRange mapper_DAM63B_table[] = {
	{ GFXTYPE_SPRITES, 0x00000, 0x01fff, 0 },
	{ GFXTYPE_SCROLL1, 0x02000, 0x02fff, 0 },
	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 0 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SPRITES, 0x02000, 0x03fff, 1 },
	{ 0              ,       0,       0, 0 }
};

static const struct GfxRange mapper_ST24M1_table[] =
{
	{ GFXTYPE_STARS,   0x00000, 0x003ff, 0 },
	{ GFXTYPE_SPRITES, 0x00000, 0x04fff, 0 },
	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 0 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x07fff, 1 },
	{ GFXTYPE_SCROLL1, 0x07000, 0x07fff, 1 },
	{ 0              ,       0,       0, 0  }
};

static const struct GfxRange mapper_ST22B_table[] = {
	{ GFXTYPE_STARS,   0x00000, 0x1ffff, 0 },
	{ GFXTYPE_SPRITES, 0x00000, 0x03fff, 0 },

	{ GFXTYPE_SPRITES, 0x04000, 0x04fff, 1 },
	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 1 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x03fff, 2 },

	{ GFXTYPE_SCROLL3, 0x04000, 0x07fff, 3 },
	{ GFXTYPE_SCROLL1, 0x07000, 0x07fff, 3 },
	{ 0              ,       0,       0, 0 }
};

static const struct GfxRange mapper_TK22B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },

	{ GFXTYPE_SPRITES, 0x4000, 0x5fff, 1 },
	{ GFXTYPE_SCROLL1, 0x6000, 0x7fff, 1 },

	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 2 },

	{ GFXTYPE_SCROLL2, 0x4000, 0x7fff, 3 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_WL24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x4fff, 0 },
	{ GFXTYPE_SCROLL3, 0x5000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL1, 0x7000, 0x7fff, 0 },

	{ GFXTYPE_SCROLL2, 0x0000, 0x3fff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_S224B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x43ff, 0 },
	{ GFXTYPE_SCROLL1, 0x4400, 0x4bff, 0 },
	{ GFXTYPE_SCROLL3, 0x4c00, 0x5fff, 0 },
	{ GFXTYPE_SCROLL2, 0x6000, 0x7fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_YI24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x1fff, 0 },
	{ GFXTYPE_SCROLL3, 0x2000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x4000, 0x47ff, 0 },
	{ GFXTYPE_SCROLL2, 0x4800, 0x7fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_AR24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL1, 0x3000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x4000, 0x5fff, 0 },
	{ GFXTYPE_SCROLL3, 0x6000, 0x7fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_AR22B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL1, 0x3000, 0x3fff, 0 },

	{ GFXTYPE_SCROLL2, 0x4000, 0x5fff, 1 },
	{ GFXTYPE_SCROLL3, 0x6000, 0x7fff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_O224B_table[] = {
	{ GFXTYPE_SCROLL1, 0x0000, 0x0bff, 0 },
	{ GFXTYPE_SCROLL2, 0x0c00, 0x3bff, 0 },
	{ GFXTYPE_SCROLL3, 0x3c00, 0x4bff, 0 },
	{ GFXTYPE_SPRITES, 0x4c00, 0x7fff, 0 },

	{ GFXTYPE_SPRITES, 0x8000, 0xa7ff, 1 },
	{ GFXTYPE_SCROLL2, 0xa800, 0xb7ff, 1 },
	{ GFXTYPE_SCROLL3, 0xb800, 0xbfff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_MS24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x4000, 0x4fff, 0 },
	{ GFXTYPE_SCROLL2, 0x5000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_CK24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL1, 0x3000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x4000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_NM24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x4000, 0x47ff, 0 },
	{ GFXTYPE_SPRITES, 0x4800, 0x67ff, 0 },
	{ GFXTYPE_SCROLL2, 0x4800, 0x67ff, 0 },
	{ GFXTYPE_SCROLL3, 0x6800, 0x7fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_CA24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL3, 0x3000, 0x4fff, 0 },
	{ GFXTYPE_SCROLL1, 0x5000, 0x57ff, 0 },
	{ GFXTYPE_SPRITES, 0x5800, 0x7fff, 0 },
	{ GFXTYPE_SCROLL2, 0x5800, 0x7fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_CA22B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL3, 0x3000, 0x3fff, 0 },

	{ GFXTYPE_SCROLL3, 0x4000, 0x4fff, 1 },
	{ GFXTYPE_SCROLL1, 0x5000, 0x57ff, 1 },
	{ GFXTYPE_SPRITES, 0x5800, 0x7fff, 1 },
	{ GFXTYPE_SCROLL2, 0x5800, 0x7fff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_STF29_table[] = {
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },

	{ GFXTYPE_SPRITES, 0x08000, 0x0ffff, 1 },

	{ GFXTYPE_SPRITES, 0x10000, 0x11fff, 2 },
	{ GFXTYPE_SCROLL3, 0x02000, 0x03fff, 2 },
	{ GFXTYPE_SCROLL1, 0x04000, 0x04fff, 2 },
	{ GFXTYPE_SCROLL2, 0x05000, 0x07fff, 2 },
	{ 0              ,       0,       0, 0 }
};

static const struct GfxRange mapper_RT24B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x53ff, 0 },
	{ GFXTYPE_SCROLL1, 0x5400, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 0 },

	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 1 },
	{ GFXTYPE_SCROLL2, 0x2800, 0x7fff, 1 },
	{ GFXTYPE_SPRITES, 0x5400, 0x7fff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_RT22B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },

	{ GFXTYPE_SPRITES, 0x4000, 0x53ff, 1 },
	{ GFXTYPE_SCROLL1, 0x5400, 0x6fff, 1 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 1 },

	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 2 },
	{ GFXTYPE_SCROLL2, 0x2800, 0x3fff, 2 },

	{ GFXTYPE_SCROLL2, 0x4000, 0x7fff, 3 },
	{ GFXTYPE_SPRITES, 0x5400, 0x7fff, 3 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_KD29B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES, 0x8000, 0x8fff, 1 },
	{ GFXTYPE_SCROLL2, 0x9000, 0xbfff, 1 },
	{ GFXTYPE_SCROLL1, 0xc000, 0xd7ff, 1 },
	{ GFXTYPE_SCROLL3, 0xd800, 0xffff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_CC63B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x7fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES, 0x8000, 0xffff, 1 },
	{ GFXTYPE_SCROLL1, 0x8000, 0xffff, 1 },
	{ GFXTYPE_SCROLL2, 0x8000, 0xffff, 1 },
	{ GFXTYPE_SCROLL3, 0x8000, 0xffff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_KR63B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x7fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SCROLL1, 0x8000, 0x9fff, 1 },
	{ GFXTYPE_SPRITES, 0x8000, 0xcfff, 1 },
	{ GFXTYPE_SCROLL2, 0x8000, 0xcfff, 1 },
	{ GFXTYPE_SCROLL3, 0xd000, 0xffff, 1 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_S9263B_table[] = {
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },

	{ GFXTYPE_SPRITES, 0x08000, 0x0ffff, 1 },

	{ GFXTYPE_SPRITES, 0x10000, 0x11fff, 2 },
	{ GFXTYPE_SCROLL3, 0x02000, 0x03fff, 2 },
	{ GFXTYPE_SCROLL1, 0x04000, 0x04fff, 2 },
	{ GFXTYPE_SCROLL2, 0x05000, 0x07fff, 2 },
	{ 0              ,       0,       0, 0 }
};

static const struct GfxRange mapper_VA63B_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x07fff, 0 },
	{ 0              , 0,      0     , 0 }
};

static const struct GfxRange mapper_VA22B_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x03fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x04000, 0x07fff, 1 },
	{ 0              , 0,      0     , 0 }
};

static const struct GfxRange mapper_Q522B_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x0000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3,                   0x7000, 0x77ff, 0 },
	{ GFXTYPE_SCROLL1,                   0x7800, 0x7fff, 0 },
	{ 0                                ,      0,      0, 0 }
};

static const struct GfxRange mapper_TK263B_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x08000, 0x0ffff, 1 },
	{ 0                                ,      0,      0, 0 }
};

static const struct GfxRange mapper_CD63B_table[] = {
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SPRITES,                   0x1000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0xdfff, 1 },
	{ GFXTYPE_SCROLL3,                   0xe000, 0xffff, 1 },
	{ 0                                ,      0,      0, 0 }
};

static const struct GfxRange mapper_PS63B_table[] = {
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SPRITES,                   0x1000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0xdbff, 1 },
	{ GFXTYPE_SCROLL3,                   0xdc00, 0xffff, 1 },
	{ 0                                ,      0,      0, 0 }
};

static const struct GfxRange mapper_MB63B_table[] = {
	{ GFXTYPE_SCROLL1,                   0x00000, 0x00fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x01000, 0x07fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x08000, 0x0ffff, 1 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x10000, 0x167ff, 2 },
	{ GFXTYPE_SCROLL3,                   0x16800, 0x17fff, 2 },
	{ 0                                 ,      0,       0, 0 }
};

static const struct GfxRange mapper_QD22B_table[] = {
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 0 },
	{ 0              ,      0,      0, 0 }
};

static const struct GfxRange mapper_QD63B_table[] = {
	{ GFXTYPE_SCROLL1,                   0x0000, 0x07ff, 0 },
	{ GFXTYPE_SCROLL3,                   0x0800, 0x1fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x2000, 0x7fff, 0 },
	{ 0                                ,      0,      0, 0 }
};

static const struct GfxRange mapper_qtono2_table[] = {
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SCROLL3,                   0x1000, 0x3fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x4000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0xffff, 1 },
	{ 0                                ,      0,      0, 0 }
};

static const struct GfxRange mapper_RCM63B_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x07fff, 0 },
 	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x08000, 0x0ffff, 1 },
 	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x10000, 0x17fff, 2 },
 	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x18000, 0x1ffff, 3 },
	{ 0                                  ,      0,      0, 0 }
};

static const struct GfxRange mapper_PKB10B_table[] = {
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x1000, 0x5fff, 0 },
	{ GFXTYPE_SCROLL3,                   0x6000, 0x7fff, 0 },
	{ 0                                ,      0,      0, 0 }
};

static const struct GfxRange mapper_pang3_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0x9fff, 1 },
	{ GFXTYPE_SCROLL1,                   0xa000, 0xbfff, 1 },
	{ GFXTYPE_SCROLL3,                   0xc000, 0xffff, 1 },
	{ 0              ,                        0,      0, 0 }
};

static const struct GfxRange mapper_sfzch_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 0 },
	{ 0                                                                    ,       0,       0, 0 }
};

static const struct GfxRange mapper_cps2_table[] =
{
	{ GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3 | GFXTYPE_SPRITES, 0x00000, 0x1ffff, 1 },
	{ 0                                                                    ,       0,       0, 0 }
};

static const struct GfxRange mapper_frog_table[] = {
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00001, 0x1ffff, 0 },
	{ 0                                                                    ,       0,       0, 0 }
};

void SetGfxMapper(INT32 MapperId)
{
	switch (MapperId) {
		case mapper_LWCHR: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_LWCHR_table;
			return;
		}
		
		case mapper_LW621: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_LW621_table;
			return;
		}
		
		case mapper_DM620: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x2000;
			GfxBankSizes[2] = 0x2000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_DM620_table;
			return;
		}
		
		case mapper_DM22A: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x2000;
			GfxBankSizes[3] = 0x2000;
			GfxBankMapper = mapper_DM22A_table;
			return;
		}
		
		case mapper_DAM63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_DAM63B_table;
			return;
		}
		
		case mapper_ST24M1: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_ST24M1_table;
			return;
		}
		
		case mapper_ST22B: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x4000;
			GfxBankSizes[3] = 0x4000;
			GfxBankMapper = mapper_ST22B_table;
			return;
		}
		
		case mapper_TK22B: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x4000;
			GfxBankSizes[3] = 0x4000;
			GfxBankMapper = mapper_TK22B_table;
			return;
		}
		
		case mapper_WL24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_WL24B_table;
			return;
		}
		
		case mapper_S224B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_S224B_table;
			return;
		}
		
		case mapper_YI24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_YI24B_table;
			return;
		}
		
		case mapper_AR24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_AR24B_table;
			return;
		}
		
		case mapper_AR22B: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_AR22B_table;
			return;
		}
				
		case mapper_O224B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_O224B_table;
			return;
		}
		
		case mapper_MS24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_MS24B_table;
			return;
		}
		
		case mapper_CK24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_CK24B_table;
			return;
		}
		
		case mapper_NM24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_NM24B_table;
			return;
		}
		
		case mapper_CA24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_CA24B_table;
			return;
		}
		
		case mapper_CA22B: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_CA22B_table;
			return;
		}
		
		case mapper_STF29: {
			GfxBankSizes[0] = 0x08000;
			GfxBankSizes[1] = 0x08000;
			GfxBankSizes[2] = 0x08000;
			GfxBankSizes[3] = 0x00000;
			GfxBankMapper = mapper_STF29_table;
			return;
		}
		
		case mapper_RT24B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_RT24B_table;
			return;
		}
				
		case mapper_RT22B: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x4000;
			GfxBankSizes[3] = 0x4000;
			GfxBankMapper = mapper_RT22B_table;
			return;
		}
		
		case mapper_KD29B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_KD29B_table;
			return;
		}
		
		case mapper_CC63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_CC63B_table;
			return;
		}
		
		case mapper_KR63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_KR63B_table;
			return;
		}
		
		case mapper_S9263B: {
			GfxBankSizes[0] = 0x08000;
			GfxBankSizes[1] = 0x08000;
			GfxBankSizes[2] = 0x08000;
			GfxBankSizes[3] = 0x00000;
			GfxBankMapper = mapper_S9263B_table;
			return;
		}
		
		case mapper_VA63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_VA63B_table;
			return;
		}
		
		case mapper_VA22B: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x4000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_VA22B_table;
			return;
		}
		
		case mapper_Q522B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_Q522B_table;
			return;
		}
		
		case mapper_TK263B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_TK263B_table;
			return;
		}
		
		case mapper_CD63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_CD63B_table;
			return;
		}
		
		case mapper_PS63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_PS63B_table;
			return;
		}
		
		case mapper_MB63B: {
			GfxBankSizes[0] = 0x08000;
			GfxBankSizes[1] = 0x08000;
			GfxBankSizes[2] = 0x08000;
			GfxBankSizes[3] = 0x00000;
			GfxBankMapper = mapper_MB63B_table;
			return;
		}
		
		case mapper_QD22B: {
			GfxBankSizes[0] = 0x4000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_QD22B_table;
			return;
		}
		
		case mapper_QD63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_QD63B_table;
			return;
		}
		
		case mapper_qtono2: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_qtono2_table;
			return;
		}
		
		case mapper_RCM63B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x8000;
			GfxBankSizes[3] = 0x8000;
			GfxBankMapper = mapper_RCM63B_table;
			return;
		}
		
		case mapper_PKB10B: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x0000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_PKB10B_table;
			return;
		}
		
		case mapper_pang3: {
			GfxBankSizes[0] = 0x8000;
			GfxBankSizes[1] = 0x8000;
			GfxBankSizes[2] = 0x0000;
			GfxBankSizes[3] = 0x0000;
			GfxBankMapper = mapper_pang3_table;
			return;
		}
		
		case mapper_sfzch: {
			GfxBankSizes[0] = 0x20000;
			GfxBankSizes[1] = 0x00000;
			GfxBankSizes[2] = 0x00000;
			GfxBankSizes[3] = 0x00000;
			GfxBankMapper = mapper_sfzch_table;
			return;
		}
		
		case mapper_cps2: {
			GfxBankSizes[0] = 0x20000;
			GfxBankSizes[1] = 0x20000;
			GfxBankSizes[2] = 0x00000;
			GfxBankSizes[3] = 0x00000;
			GfxBankMapper = mapper_cps2_table;
			return;
		}
		
		case mapper_frog: {
			GfxBankSizes[0] = 0x20000;
			GfxBankSizes[1] = 0x00000;
			GfxBankSizes[2] = 0x00000;
			GfxBankSizes[3] = 0x00000;
			GfxBankMapper = mapper_frog_table;
			return;
		}
	}
}

INT32 GfxRomBankMapper(INT32 Type, INT32 Code)
{
	const struct GfxRange *Range = GfxBankMapper;
	INT32 Shift = 0;

	switch (Type) {
		case GFXTYPE_SPRITES: Shift = 1; break;
		case GFXTYPE_SCROLL1: Shift = 0; break;
		case GFXTYPE_SCROLL2: Shift = 1; break;
		case GFXTYPE_SCROLL3: Shift = 3; break;
	}

	Code <<= Shift;

	while (Range->Type) {
		if (Code >= Range->Start && Code <= Range->End)	{
			if (Range->Type & Type)	{
				INT32 Base = 0;
				INT32 i;

				for (i = 0; i < Range->Bank; ++i)
					Base += GfxBankSizes[i];

				return (Base + (Code & (GfxBankSizes[Range->Bank] - 1))) >> Shift;
			}
		}

		++Range;
	}

//	bprintf(PRINT_NORMAL, _T("tile %02x/%04x out of range\n"), Type,Code>>Shift);

	return -1;
}

void SetCpsBId(INT32 CpsBId, INT32 bStars)
{
	switch (CpsBId) {
		case CPS_B_01: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x66;
			MaskAddr[0] = 0x68;
			MaskAddr[1] = 0x6a;
			MaskAddr[2] = 0x6c;
			MaskAddr[3] = 0x6e;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x30;
				CpsLayEn[5] = 0x30;
			}
			return;
		}
		
		case CPS_B_02: {
			CpsBID[0]   = 0x60;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x02;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x6c;
			MaskAddr[0] = 0x6a;
			MaskAddr[1] = 0x68;
			MaskAddr[2] = 0x66;
			MaskAddr[3] = 0x64;
			
			nCpsPalCtrlReg = 0x62;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_03: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x70;
			MaskAddr[0] = 0x6e;
			MaskAddr[1] = 0x6c;
			MaskAddr[2] = 0x6a;
			MaskAddr[3] = 0x68;
			
			nCpsPalCtrlReg = 0x66;
  			
			CpsLayEn[1] = 0x20;
			CpsLayEn[2] = 0x10;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_04: {
			CpsBID[0]   = 0x60;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x04;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x6e;
			MaskAddr[0] = 0x66;
			MaskAddr[1] = 0x70;
			MaskAddr[2] = 0x68;
			MaskAddr[3] = 0x72;
			
			nCpsPalCtrlReg = 0x6a;
  			
			CpsLayEn[1] = 0x02;
			//CpsLayEn[2] = 0x0c;
			CpsLayEn[2] = 0x04;
			//CpsLayEn[3] = 0x0c;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_05: {
			CpsBID[0]   = 0x60;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x05;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x68;
			MaskAddr[0] = 0x6a;
			MaskAddr[1] = 0x6c;
			MaskAddr[2] = 0x6e;
			MaskAddr[3] = 0x70;
			
			nCpsPalCtrlReg = 0x72;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x08;
			CpsLayEn[3] = 0x20;
			if (bStars) {
				CpsLayEn[4] = 0x14;
				CpsLayEn[5] = 0x14;
			}
			return;
		}
		
		case CPS_B_11: {
			CpsBID[0]   = 0x72;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x01;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x66;
			MaskAddr[0] = 0x68;
			MaskAddr[1] = 0x6a;
			MaskAddr[2] = 0x6c;
			MaskAddr[3] = 0x6e;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x08;
			CpsLayEn[2] = 0x10;
			CpsLayEn[3] = 0x20;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_12: {
			CpsBID[0]   = 0x60;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x02;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x6c;
			MaskAddr[0] = 0x6a;
			MaskAddr[1] = 0x68;
			MaskAddr[2] = 0x66;
			MaskAddr[3] = 0x64;
			
			nCpsPalCtrlReg = 0x62;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_13: {
			CpsBID[0]   = 0x6e;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x03;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x62;
			MaskAddr[0] = 0x64;
			MaskAddr[1] = 0x66;
			MaskAddr[2] = 0x68;
			MaskAddr[3] = 0x6a;
			
			nCpsPalCtrlReg = 0x6c;
  			
			CpsLayEn[1] = 0x20;
			CpsLayEn[2] = 0x02;
			CpsLayEn[3] = 0x04;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_14: {
			CpsBID[0]   = 0x5e;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x04;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x52;
			MaskAddr[0] = 0x54;
			MaskAddr[1] = 0x56;
			MaskAddr[2] = 0x58;
			MaskAddr[3] = 0x5a;
			
			nCpsPalCtrlReg = 0x5c;
  			
			CpsLayEn[1] = 0x08;
			CpsLayEn[2] = 0x20;
			CpsLayEn[3] = 0x10;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_15: {
			CpsBID[0]   = 0x4e;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x05;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x42;
			MaskAddr[0] = 0x44;
			MaskAddr[1] = 0x46;
			MaskAddr[2] = 0x48;
			MaskAddr[3] = 0x4a;
			
			nCpsPalCtrlReg = 0x4c;
  			
			CpsLayEn[1] = 0x04;
			CpsLayEn[2] = 0x02;
			CpsLayEn[3] = 0x20;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_16: {
			CpsBID[0]   = 0x40;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x06;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x4c;
			MaskAddr[0] = 0x4a;
			MaskAddr[1] = 0x48;
			MaskAddr[2] = 0x46;
			MaskAddr[3] = 0x44;
			
			nCpsPalCtrlReg = 0x42;
  			
			CpsLayEn[1] = 0x10;
			CpsLayEn[2] = 0x0a;
			CpsLayEn[3] = 0x0a;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_17: {
			CpsBID[0]   = 0x48;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x07;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x54;
			MaskAddr[0] = 0x52;
			MaskAddr[1] = 0x50;
			MaskAddr[2] = 0x4e;
			MaskAddr[3] = 0x4c;
			
			nCpsPalCtrlReg = 0x4a;
  			
			CpsLayEn[1] = 0x08;
			CpsLayEn[2] = 0x10;
			CpsLayEn[3] = 0x02;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_18: {
			CpsBID[0]   = 0xd0;
			CpsBID[1]   = 0x04;
			CpsBID[2]   = 0x08;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0xdc;
			MaskAddr[0] = 0xda;
			MaskAddr[1] = 0xd8;
			MaskAddr[2] = 0xd6;
			MaskAddr[3] = 0xd4;
			
			nCpsPalCtrlReg = 0xd2;
  			
			CpsLayEn[1] = 0x10;
			CpsLayEn[2] = 0x08;
			CpsLayEn[3] = 0x02;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_BT1: {
			CpsBID[0]   = 0x72;
			CpsBID[1]   = 0x08;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x4e;
			CpsMProt[1] = 0x4c;
			CpsMProt[2] = 0x4a;
			CpsMProt[3] = 0x48;
  			
			nCpsLcReg   = 0x68;
			MaskAddr[0] = 0x66;
			MaskAddr[1] = 0x64;
			MaskAddr[2] = 0x62;
			MaskAddr[3] = 0x60;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x20;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x12;
				CpsLayEn[5] = 0x12;
			}
			return;
		}
		
		case CPS_B_21_BT2: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x5e;
			CpsMProt[1] = 0x5c;
			CpsMProt[2] = 0x5a;
			CpsMProt[3] = 0x58;
  			
			nCpsLcReg   = 0x60;
			MaskAddr[0] = 0x6e;
			MaskAddr[1] = 0x6c;
			MaskAddr[2] = 0x6a;
			MaskAddr[3] = 0x68;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x30;
			CpsLayEn[2] = 0x08;
			CpsLayEn[3] = 0x30;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_BT3: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x46;
			CpsMProt[1] = 0x44;
			CpsMProt[2] = 0x42;
			CpsMProt[3] = 0x40;
  			
			nCpsLcReg   = 0x60;
			MaskAddr[0] = 0x6e;
			MaskAddr[1] = 0x6c;
			MaskAddr[2] = 0x6a;
			MaskAddr[3] = 0x68;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x20;
			CpsLayEn[2] = 0x12;
			CpsLayEn[3] = 0x12;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_BT4: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x46;
			CpsMProt[1] = 0x44;
			CpsMProt[2] = 0x42;
			CpsMProt[3] = 0x40;
  			
			nCpsLcReg   = 0x68;
			MaskAddr[0] = 0x66;
			MaskAddr[1] = 0x64;
			MaskAddr[2] = 0x62;
			MaskAddr[3] = 0x60;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x20;
			CpsLayEn[2] = 0x10;
			CpsLayEn[3] = 0x02;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_BT5: {
			CpsBID[0]   = 0x32;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x4e;
			CpsMProt[1] = 0x4c;
			CpsMProt[2] = 0x4a;
			CpsMProt[3] = 0x48;
  			
			nCpsLcReg   = 0x60;
			MaskAddr[0] = 0x6e;
			MaskAddr[1] = 0x6c;
			MaskAddr[2] = 0x6a;
			MaskAddr[3] = 0x68;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x20;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x02;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_BT6: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x60;
			MaskAddr[0] = 0x6e;
			MaskAddr[1] = 0x6c;
			MaskAddr[2] = 0x6a;
			MaskAddr[3] = 0x68;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x20;
			CpsLayEn[2] = 0x14;
			CpsLayEn[3] = 0x14;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_BT7: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x6c;
			MaskAddr[0] = 0x00;
			MaskAddr[1] = 0x00;
			MaskAddr[2] = 0x00;
			MaskAddr[3] = 0x00;
			
			nCpsPalCtrlReg = 0x52;
  			
			CpsLayEn[1] = 0x14;
			CpsLayEn[2] = 0x02;
			CpsLayEn[3] = 0x14;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_DEF: {
			CpsBID[0]   = 0x32;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x40;
			CpsMProt[1] = 0x42;
			CpsMProt[2] = 0x44;
			CpsMProt[3] = 0x46;
  			
			nCpsLcReg   = 0x66;
			MaskAddr[0] = 0x68;
			MaskAddr[1] = 0x6a;
			MaskAddr[2] = 0x6c;
			MaskAddr[3] = 0x6e;
			
			nCpsPalCtrlReg = 0x70;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x30;
				CpsLayEn[5] = 0x30;
			}
			return;
		}
		
		case CPS_B_21_QS1: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x62;
			MaskAddr[0] = 0x64;
			MaskAddr[1] = 0x66;
			MaskAddr[2] = 0x68;
			MaskAddr[3] = 0x6a;
			
			nCpsPalCtrlReg = 0x6c;
  			
			CpsLayEn[1] = 0x10;
			CpsLayEn[2] = 0x08;
			CpsLayEn[3] = 0x04;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_QS2: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x4a;
			MaskAddr[0] = 0x4c;
			MaskAddr[1] = 0x4e;
			MaskAddr[2] = 0x40;
			MaskAddr[3] = 0x42;
			
			nCpsPalCtrlReg = 0x44;
  			
			CpsLayEn[1] = 0x16;
			CpsLayEn[2] = 0x16;
			CpsLayEn[3] = 0x16;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_QS3: {
			CpsBID[0]   = 0x4e;
			CpsBID[1]   = 0x0c;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x52;
			MaskAddr[0] = 0x54;
			MaskAddr[1] = 0x56;
			MaskAddr[2] = 0x48;
			MaskAddr[3] = 0x4a;
			
			nCpsPalCtrlReg = 0x4c;
  			
			CpsLayEn[1] = 0x04;
			CpsLayEn[2] = 0x02;
			CpsLayEn[3] = 0x20;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_QS4: {
			CpsBID[0]   = 0x6e;
			CpsBID[1]   = 0x0c;
			CpsBID[2]   = 0x01;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x56;
			MaskAddr[0] = 0x40;
			MaskAddr[1] = 0x42;
			MaskAddr[2] = 0x68;
			MaskAddr[3] = 0x6a;
			
			nCpsPalCtrlReg = 0x6c;
  			
			CpsLayEn[1] = 0x04;
			CpsLayEn[2] = 0x08;
			CpsLayEn[3] = 0x10;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case CPS_B_21_QS5: {
			CpsBID[0]   = 0x5e;
			CpsBID[1]   = 0x0c;
			CpsBID[2]   = 0x02;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x6a;
			MaskAddr[0] = 0x6c;
			MaskAddr[1] = 0x6e;
			MaskAddr[2] = 0x70;
			MaskAddr[3] = 0x72;
			
			nCpsPalCtrlReg = 0x5c;
  			
			CpsLayEn[1] = 0x04;
			CpsLayEn[2] = 0x08;
			CpsLayEn[3] = 0x10;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case HACK_B_1: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x54;
			MaskAddr[0] = 0x52;
			MaskAddr[1] = 0x50;
			MaskAddr[2] = 0x4e;
			MaskAddr[3] = 0x4c;
			
			nCpsPalCtrlReg = 0x4a;
  			
			CpsLayEn[1] = 0xff;
			CpsLayEn[2] = 0xff;
			CpsLayEn[3] = 0xff;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case HACK_B_2: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0xc4;
			MaskAddr[0] = 0x52;
			MaskAddr[1] = 0x50;
			MaskAddr[2] = 0x4e;
			MaskAddr[3] = 0x4c;
			
			nCpsPalCtrlReg = 0x4a;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x02;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
		
		case HACK_B_3: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x60;
			MaskAddr[0] = 0x68;
			MaskAddr[1] = 0x6a;
			MaskAddr[2] = 0x6c;
			MaskAddr[3] = 0x6e;
			
			nCpsPalCtrlReg = 0x6a;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x30;
				CpsLayEn[5] = 0x30;
			}
			return;
		}
		
		case HACK_B_4: {
			CpsBID[0]   = 0x00;
			CpsBID[1]   = 0x00;
			CpsBID[2]   = 0x00;
  			
			CpsMProt[0] = 0x00;
			CpsMProt[1] = 0x00;
			CpsMProt[2] = 0x00;
			CpsMProt[3] = 0x00;
  			
			nCpsLcReg   = 0x70;
			MaskAddr[0] = 0x6e;
			MaskAddr[1] = 0x6c;
			MaskAddr[2] = 0x6a;
			MaskAddr[3] = 0x68;
			
			nCpsPalCtrlReg = 0x66;
  			
			CpsLayEn[1] = 0x02;
			CpsLayEn[2] = 0x04;
			CpsLayEn[3] = 0x08;
			if (bStars) {
				CpsLayEn[4] = 0x00;
				CpsLayEn[5] = 0x00;
			}
			return;
		}
	}
}
