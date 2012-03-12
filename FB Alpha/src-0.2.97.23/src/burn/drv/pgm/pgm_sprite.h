typedef INT32 (*sprite_draw_function)(UINT16 *dest, UINT8 *adata, INT32 pal);
typedef INT32 (*sprite_draw_nozoom_function)(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 pri);

static INT32 zoom_draw_00(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = adata[5] + pal;
	dest[6] = adata[6] + pal;
	dest[7] = adata[7] + pal;

	return 0x08;
}

static INT32 zoom_draw_01(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = adata[6] + pal;

	return 0x07;
}

static INT32 zoom_draw_02(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = adata[6] + pal;

	return 0x07;
}

static INT32 zoom_draw_03(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_04(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = adata[6] + pal;

	return 0x07;
}

static INT32 zoom_draw_05(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_06(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_07(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_08(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = adata[6] + pal;

	return 0x07;
}

static INT32 zoom_draw_09(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_0a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_0b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_0c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_0d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_0e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_0f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_10(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = adata[6] + pal;

	return 0x07;
}

static INT32 zoom_draw_11(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_12(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_13(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_14(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_15(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_16(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_17(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_18(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_19(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_1a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_1b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_1c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_1d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_1e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_1f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[0] + pal;
	dest[6] = adata[1] + pal;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_20(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[5] + pal;
	dest[7] = adata[6] + pal;

	return 0x07;
}

static INT32 zoom_draw_21(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_22(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_23(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_24(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_25(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_26(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_27(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_28(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_29(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_2a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_2b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_2c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_2d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_2e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_2f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_30(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_31(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_32(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_33(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_34(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_35(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_36(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_37(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_38(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_39(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_3a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_3b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_3c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_3d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_3e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_3f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[0] + pal;
	dest[7] = adata[1] + pal;

	return 0x02;
}

static INT32 zoom_draw_40(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = adata[5] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[6] + pal;

	return 0x07;
}

static INT32 zoom_draw_41(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_42(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_43(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_44(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_45(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_46(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_47(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_48(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_49(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_4a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_4b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_4c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_4d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_4e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_4f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_50(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_51(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_52(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_53(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_54(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_55(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_56(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_57(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_58(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_59(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_5a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_5b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_5c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_5d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_5e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_5f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[0] + pal;
	dest[6] = 0x8000;
	dest[7] = adata[1] + pal;

	return 0x02;
}

static INT32 zoom_draw_60(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[5] + pal;

	return 0x06;
}

static INT32 zoom_draw_61(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_62(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_63(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_64(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_65(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_66(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_67(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_68(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_69(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_6a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_6b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_6c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_6d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_6e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_6f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[1] + pal;

	return 0x02;
}

static INT32 zoom_draw_70(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[4] + pal;

	return 0x05;
}

static INT32 zoom_draw_71(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_72(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_73(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_74(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_75(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_76(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_77(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[1] + pal;

	return 0x02;
}

static INT32 zoom_draw_78(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[3] + pal;

	return 0x04;
}

static INT32 zoom_draw_79(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_7a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_7b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[1] + pal;

	return 0x02;
}

static INT32 zoom_draw_7c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[2] + pal;

	return 0x03;
}

static INT32 zoom_draw_7d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[1] + pal;

	return 0x02;
}

static INT32 zoom_draw_7e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[1] + pal;

	return 0x02;
}

static INT32 zoom_draw_7f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = adata[0] + pal;

	return 0x01;
}

static INT32 zoom_draw_80(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = adata[5] + pal;
	dest[6] = adata[6] + pal;
	dest[7] = 0x8000;

	return 0x07;
}

static INT32 zoom_draw_81(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = 0x8000;

	return 0x06;
}

static INT32 zoom_draw_82(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = 0x8000;

	return 0x06;
}

static INT32 zoom_draw_83(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_84(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = 0x8000;

	return 0x06;
}

static INT32 zoom_draw_85(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_86(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_87(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_88(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = 0x8000;

	return 0x06;
}

static INT32 zoom_draw_89(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_8a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_8b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_8c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_8d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_8e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_8f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_90(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[4] + pal;
	dest[6] = adata[5] + pal;
	dest[7] = 0x8000;

	return 0x06;
}

static INT32 zoom_draw_91(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_92(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_93(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_94(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_95(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_96(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_97(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_98(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_99(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_9a(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_9b(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_9c(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_9d(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_9e(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_9f(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[0] + pal;
	dest[6] = adata[1] + pal;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_a0(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[5] + pal;
	dest[7] = 0x8000;

	return 0x06;
}

static INT32 zoom_draw_a1(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_a2(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_a3(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_a4(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_a5(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_a6(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_a7(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_a8(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_a9(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_aa(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_ab(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_ac(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_ad(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_ae(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_af(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_b0(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[4] + pal;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_b1(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_b2(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_b3(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_b4(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_b5(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_b6(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_b7(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_b8(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[3] + pal;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_b9(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_ba(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_bb(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_bc(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[2] + pal;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_bd(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_be(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[1] + pal;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_bf(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = adata[0] + pal;
	dest[7] = 0x8000;

	return 0x01;
}

static INT32 zoom_draw_c0(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = adata[5] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x06;
}

static INT32 zoom_draw_c1(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_c2(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_c3(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_c4(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_c5(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_c6(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_c7(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_c8(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_c9(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_ca(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_cb(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_cc(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_cd(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_ce(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_cf(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_d0(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[4] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_d1(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_d2(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_d3(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_d4(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_d5(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_d6(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_d7(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_d8(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[3] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_d9(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_da(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_db(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_dc(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[2] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_dd(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_de(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[1] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_df(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = adata[0] + pal;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x01;
}

static INT32 zoom_draw_e0(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = adata[4] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x05;
}

static INT32 zoom_draw_e1(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_e2(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_e3(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_e4(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_e5(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_e6(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_e7(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_e8(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[3] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_e9(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_ea(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_eb(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_ec(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[2] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_ed(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_ee(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[1] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_ef(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = adata[0] + pal;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x01;
}

static INT32 zoom_draw_f0(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = adata[3] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x04;
}

static INT32 zoom_draw_f1(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_f2(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_f3(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_f4(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[2] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_f5(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_f6(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[1] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_f7(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = adata[0] + pal;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x01;
}

static INT32 zoom_draw_f8(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = adata[2] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x03;
}

static INT32 zoom_draw_f9(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_fa(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = adata[1] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_fb(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = adata[0] + pal;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x01;
}

static INT32 zoom_draw_fc(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = adata[1] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x02;
}

static INT32 zoom_draw_fd(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = 0x8000;
	dest[1] = adata[0] + pal;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x01;
}

static INT32 zoom_draw_fe(UINT16 *dest, UINT8 *adata, INT32 pal)
{
	dest[0] = adata[0] + pal;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x01;
}

static INT32 zoom_draw_ff(UINT16 *dest, UINT8 *, INT32 )
{
	dest[0] = 0x8000;
	dest[1] = 0x8000;
	dest[2] = 0x8000;
	dest[3] = 0x8000;
	dest[4] = 0x8000;
	dest[5] = 0x8000;
	dest[6] = 0x8000;
	dest[7] = 0x8000;

	return 0x00;
}

static INT32 nozoom_draw_00(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;
	dest[5] = adata[5] + pal;
	pdest[5] = prio;
	dest[6] = adata[6] + pal;
	pdest[6] = prio;
	dest[7] = adata[7] + pal;
	pdest[7] = prio;

	return 0x08;}


static INT32 nozoom_draw_01(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;
	dest[7] = adata[6] + pal;
	pdest[7] = prio;

	return 0x07;}


static INT32 nozoom_draw_02(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;
	dest[7] = adata[6] + pal;
	pdest[7] = prio;

	return 0x07;}


static INT32 nozoom_draw_03(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_04(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;
	dest[7] = adata[6] + pal;
	pdest[7] = prio;

	return 0x07;}


static INT32 nozoom_draw_05(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_06(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_07(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_08(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;
	dest[7] = adata[6] + pal;
	pdest[7] = prio;

	return 0x07;}


static INT32 nozoom_draw_09(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_0a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_0b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_0c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_0d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_0e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_0f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_10(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;
	dest[7] = adata[6] + pal;
	pdest[7] = prio;

	return 0x07;}


static INT32 nozoom_draw_11(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_12(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_13(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_14(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_15(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_16(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_17(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_18(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_19(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_1a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_1b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_1c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_1d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_1e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_1f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_20(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;
	dest[7] = adata[6] + pal;
	pdest[7] = prio;

	return 0x07;}


static INT32 nozoom_draw_21(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_22(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_23(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_24(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_25(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_26(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_27(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_28(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_29(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_2a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_2b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_2c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_2d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_2e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_2f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_30(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_31(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_32(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_33(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_34(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_35(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_36(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_37(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_38(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_39(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_3a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_3b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_3c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_3d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_3e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_3f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[7] = adata[1] + pal;
	pdest[7] = prio;

	return 0x02;}


static INT32 nozoom_draw_40(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;
	dest[5] = adata[5] + pal;
	pdest[5] = prio;
	dest[7] = adata[6] + pal;
	pdest[7] = prio;

	return 0x07;}


static INT32 nozoom_draw_41(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_42(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_43(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_44(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_45(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_46(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_47(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_48(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_49(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_4a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_4b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_4c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_4d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_4e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_4f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_50(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_51(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_52(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_53(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_54(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_55(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_56(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_57(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_58(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_59(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_5a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_5b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_5c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_5d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_5e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_5f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[7] = adata[1] + pal;
	pdest[7] = prio;

	return 0x02;}


static INT32 nozoom_draw_60(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;
	dest[7] = adata[5] + pal;
	pdest[7] = prio;

	return 0x06;}


static INT32 nozoom_draw_61(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_62(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_63(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_64(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_65(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_66(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_67(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_68(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_69(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_6a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_6b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_6c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_6d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_6e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_6f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[7] = adata[1] + pal;
	pdest[7] = prio;

	return 0x02;}


static INT32 nozoom_draw_70(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[7] = adata[4] + pal;
	pdest[7] = prio;

	return 0x05;}


static INT32 nozoom_draw_71(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_72(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_73(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_74(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_75(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_76(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_77(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[7] = adata[1] + pal;
	pdest[7] = prio;

	return 0x02;}


static INT32 nozoom_draw_78(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[7] = adata[3] + pal;
	pdest[7] = prio;

	return 0x04;}


static INT32 nozoom_draw_79(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_7a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_7b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[7] = adata[1] + pal;
	pdest[7] = prio;

	return 0x02;}


static INT32 nozoom_draw_7c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[7] = adata[2] + pal;
	pdest[7] = prio;

	return 0x03;}


static INT32 nozoom_draw_7d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[7] = adata[1] + pal;
	pdest[7] = prio;

	return 0x02;}


static INT32 nozoom_draw_7e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[7] = adata[1] + pal;
	pdest[7] = prio;

	return 0x02;}


static INT32 nozoom_draw_7f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;

	return 0x01;}


static INT32 nozoom_draw_80(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;
	dest[5] = adata[5] + pal;
	pdest[5] = prio;
	dest[6] = adata[6] + pal;
	pdest[6] = prio;

	return 0x07;}


static INT32 nozoom_draw_81(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;

	return 0x06;}


static INT32 nozoom_draw_82(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;

	return 0x06;}


static INT32 nozoom_draw_83(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_84(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;

	return 0x06;}


static INT32 nozoom_draw_85(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_86(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_87(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_88(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;

	return 0x06;}


static INT32 nozoom_draw_89(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_8a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_8b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_8c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_8d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_8e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_8f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_90(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;

	return 0x06;}


static INT32 nozoom_draw_91(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_92(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_93(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_94(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_95(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_96(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_97(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_98(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_99(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_9a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_9b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_9c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_9d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_9e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_9f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;

	return 0x02;}


static INT32 nozoom_draw_a0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;
	dest[6] = adata[5] + pal;
	pdest[6] = prio;

	return 0x06;}


static INT32 nozoom_draw_a1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_a2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_a3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_a4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_a5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_a6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_a7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_a8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_a9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_aa(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_ab(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_ac(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_ad(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_ae(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_af(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;

	return 0x02;}


static INT32 nozoom_draw_b0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[6] = adata[4] + pal;
	pdest[6] = prio;

	return 0x05;}


static INT32 nozoom_draw_b1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_b2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_b3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_b4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_b5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_b6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_b7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;

	return 0x02;}


static INT32 nozoom_draw_b8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[6] = adata[3] + pal;
	pdest[6] = prio;

	return 0x04;}


static INT32 nozoom_draw_b9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_ba(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_bb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;

	return 0x02;}


static INT32 nozoom_draw_bc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[6] = adata[2] + pal;
	pdest[6] = prio;

	return 0x03;}


static INT32 nozoom_draw_bd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;

	return 0x02;}


static INT32 nozoom_draw_be(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;

	return 0x02;}


static INT32 nozoom_draw_bf(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;

	return 0x01;}


static INT32 nozoom_draw_c0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;
	dest[5] = adata[5] + pal;
	pdest[5] = prio;

	return 0x06;}


static INT32 nozoom_draw_c1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;

	return 0x05;}


static INT32 nozoom_draw_c2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;

	return 0x05;}


static INT32 nozoom_draw_c3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_c4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;

	return 0x05;}


static INT32 nozoom_draw_c5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_c6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_c7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_c8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;

	return 0x05;}


static INT32 nozoom_draw_c9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_ca(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_cb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_cc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_cd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_ce(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_cf(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;

	return 0x02;}


static INT32 nozoom_draw_d0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[5] = adata[4] + pal;
	pdest[5] = prio;

	return 0x05;}


static INT32 nozoom_draw_d1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_d2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_d3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_d4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_d5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_d6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_d7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;

	return 0x02;}


static INT32 nozoom_draw_d8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[5] = adata[3] + pal;
	pdest[5] = prio;

	return 0x04;}


static INT32 nozoom_draw_d9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_da(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_db(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;

	return 0x02;}


static INT32 nozoom_draw_dc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;}


static INT32 nozoom_draw_dd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;

	return 0x02;}


static INT32 nozoom_draw_de(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;

	return 0x02;}


static INT32 nozoom_draw_df(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;

	return 0x01;}


static INT32 nozoom_draw_e0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[4] = adata[4] + pal;
	pdest[4] = prio;

	return 0x05;}


static INT32 nozoom_draw_e1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;

	return 0x04;}


static INT32 nozoom_draw_e2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;

	return 0x04;}


static INT32 nozoom_draw_e3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;}


static INT32 nozoom_draw_e4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;

	return 0x04;}


static INT32 nozoom_draw_e5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;}


static INT32 nozoom_draw_e6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;}


static INT32 nozoom_draw_e7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;

	return 0x02;}


static INT32 nozoom_draw_e8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;

	return 0x04;}


static INT32 nozoom_draw_e9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;}


static INT32 nozoom_draw_ea(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;}


static INT32 nozoom_draw_eb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;

	return 0x02;}


static INT32 nozoom_draw_ec(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;}


static INT32 nozoom_draw_ed(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;

	return 0x02;}


static INT32 nozoom_draw_ee(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;

	return 0x02;}


static INT32 nozoom_draw_ef(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;

	return 0x01;}


static INT32 nozoom_draw_f0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;

	return 0x04;}


static INT32 nozoom_draw_f1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;}


static INT32 nozoom_draw_f2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;}


static INT32 nozoom_draw_f3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;

	return 0x02;}


static INT32 nozoom_draw_f4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;}


static INT32 nozoom_draw_f5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;

	return 0x02;}


static INT32 nozoom_draw_f6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;

	return 0x02;}


static INT32 nozoom_draw_f7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;

	return 0x01;}


static INT32 nozoom_draw_f8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;}


static INT32 nozoom_draw_f9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;

	return 0x02;}


static INT32 nozoom_draw_fa(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;

	return 0x02;}


static INT32 nozoom_draw_fb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;

	return 0x01;}


static INT32 nozoom_draw_fc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;

	return 0x02;}


static INT32 nozoom_draw_fd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;

	return 0x01;}


static INT32 nozoom_draw_fe(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;

	return 0x01;}


static INT32 nozoom_draw_ff(UINT16 *, UINT8 *, UINT8 *, INT32 , INT32)
{

	return 0x00;}


static INT32 nozoom_draw_flipx_00(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;
	dest[2] = adata[5] + pal;
	pdest[2] = prio;
	dest[1] = adata[6] + pal;
	pdest[1] = prio;
	dest[0] = adata[7] + pal;
	pdest[0] = prio;

	return 0x08;
}

static INT32 nozoom_draw_flipx_01(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;
	dest[0] = adata[6] + pal;
	pdest[0] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_02(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;
	dest[0] = adata[6] + pal;
	pdest[0] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_03(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_04(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;
	dest[0] = adata[6] + pal;
	pdest[0] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_05(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_06(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_07(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_08(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;
	dest[0] = adata[6] + pal;
	pdest[0] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_09(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_0a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_0b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_0c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_0d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_0e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_0f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_10(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;
	dest[0] = adata[6] + pal;
	pdest[0] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_11(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_12(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_13(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_14(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_15(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_16(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_17(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_18(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_19(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_1a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_1b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_1c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_1d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_1e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_1f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_20(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;
	dest[0] = adata[6] + pal;
	pdest[0] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_21(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_22(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_23(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_24(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_25(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_26(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_27(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_28(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_29(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_2a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_2b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_2c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_2d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_2e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_2f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_30(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_31(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_32(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_33(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_34(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_35(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_36(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_37(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_38(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_39(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_3a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_3b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_3c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_3d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_3e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_3f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;
	dest[0] = adata[1] + pal;
	pdest[0] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_40(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;
	dest[2] = adata[5] + pal;
	pdest[2] = prio;
	dest[0] = adata[6] + pal;
	pdest[0] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_41(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_42(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_43(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_44(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_45(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_46(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_47(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_48(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_49(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_4a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_4b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_4c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_4d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_4e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_4f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_50(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_51(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_52(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_53(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_54(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_55(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_56(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_57(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_58(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_59(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_5a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_5b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_5c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_5d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_5e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_5f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[0] = adata[1] + pal;
	pdest[0] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_60(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;
	dest[0] = adata[5] + pal;
	pdest[0] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_61(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_62(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_63(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_64(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_65(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_66(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_67(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_68(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_69(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_6a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_6b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_6c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_6d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_6e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_6f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[0] = adata[1] + pal;
	pdest[0] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_70(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[0] = adata[4] + pal;
	pdest[0] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_71(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_72(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_73(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_74(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_75(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_76(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_77(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[0] = adata[1] + pal;
	pdest[0] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_78(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[0] = adata[3] + pal;
	pdest[0] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_79(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_7a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_7b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[0] = adata[1] + pal;
	pdest[0] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_7c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[0] = adata[2] + pal;
	pdest[0] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_7d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[0] = adata[1] + pal;
	pdest[0] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_7e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[0] = adata[1] + pal;
	pdest[0] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_7f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[0] = adata[0] + pal;
	pdest[0] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_80(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;
	dest[2] = adata[5] + pal;
	pdest[2] = prio;
	dest[1] = adata[6] + pal;
	pdest[1] = prio;

	return 0x07;
}

static INT32 nozoom_draw_flipx_81(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_82(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_83(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_84(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_85(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_86(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_87(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_88(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_89(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_8a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_8b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_8c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_8d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_8e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_8f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_90(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_91(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_92(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_93(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_94(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_95(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_96(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_97(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_98(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_99(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_9a(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_9b(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_9c(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_9d(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_9e(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_9f(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_a0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;
	dest[1] = adata[5] + pal;
	pdest[1] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_a1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_a2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_a3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_a4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_a5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_a6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_a7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_a8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_a9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_aa(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_ab(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_ac(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_ad(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_ae(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_af(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_b0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[1] = adata[4] + pal;
	pdest[1] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_b1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_b2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_b3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_b4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_b5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_b6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_b7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_b8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[1] = adata[3] + pal;
	pdest[1] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_b9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_ba(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_bb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_bc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[1] = adata[2] + pal;
	pdest[1] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_bd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_be(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[1] = adata[1] + pal;
	pdest[1] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_bf(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[1] = adata[0] + pal;
	pdest[1] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_c0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;
	dest[2] = adata[5] + pal;
	pdest[2] = prio;

	return 0x06;
}

static INT32 nozoom_draw_flipx_c1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_c2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_c3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_c4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_c5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_c6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_c7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_c8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_c9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_ca(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_cb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_cc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_cd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_ce(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_cf(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_d0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[2] = adata[4] + pal;
	pdest[2] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_d1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_d2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_d3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_d4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_d5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_d6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_d7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_d8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[2] = adata[3] + pal;
	pdest[2] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_d9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_da(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_db(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_dc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[2] = adata[2] + pal;
	pdest[2] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_dd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_de(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[2] = adata[1] + pal;
	pdest[2] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_df(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[2] = adata[0] + pal;
	pdest[2] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_e0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;
	dest[3] = adata[4] + pal;
	pdest[3] = prio;

	return 0x05;
}

static INT32 nozoom_draw_flipx_e1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_e2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_e3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_e4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_e5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_e6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_e7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_e8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[3] = adata[3] + pal;
	pdest[3] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_e9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_ea(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_eb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_ec(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[3] = adata[2] + pal;
	pdest[3] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_ed(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_ee(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[3] = adata[1] + pal;
	pdest[3] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_ef(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[3] = adata[0] + pal;
	pdest[3] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_f0(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;
	dest[4] = adata[3] + pal;
	pdest[4] = prio;

	return 0x04;
}

static INT32 nozoom_draw_flipx_f1(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_f2(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_f3(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_f4(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[4] = adata[2] + pal;
	pdest[4] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_f5(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_f6(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[4] = adata[1] + pal;
	pdest[4] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_f7(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[4] = adata[0] + pal;
	pdest[4] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_f8(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;
	dest[5] = adata[2] + pal;
	pdest[5] = prio;

	return 0x03;
}

static INT32 nozoom_draw_flipx_f9(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_fa(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[5] = adata[1] + pal;
	pdest[5] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_fb(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[5] = adata[0] + pal;
	pdest[5] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_fc(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;
	dest[6] = adata[1] + pal;
	pdest[6] = prio;

	return 0x02;
}

static INT32 nozoom_draw_flipx_fd(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[6] = adata[0] + pal;
	pdest[6] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_fe(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)
{
	dest[7] = adata[0] + pal;
	pdest[7] = prio;

	return 0x01;
}

static INT32 nozoom_draw_flipx_ff(UINT16 *, UINT8 *, UINT8 *, INT32 , INT32)
{

	return 0x00;
}

static sprite_draw_function zoom_draw_table[0x100] = {
	&zoom_draw_00, &zoom_draw_01, &zoom_draw_02, &zoom_draw_03, &zoom_draw_04, &zoom_draw_05, &zoom_draw_06, &zoom_draw_07, &zoom_draw_08, &zoom_draw_09, &zoom_draw_0a, &zoom_draw_0b, &zoom_draw_0c, &zoom_draw_0d, &zoom_draw_0e, &zoom_draw_0f,
	&zoom_draw_10, &zoom_draw_11, &zoom_draw_12, &zoom_draw_13, &zoom_draw_14, &zoom_draw_15, &zoom_draw_16, &zoom_draw_17, &zoom_draw_18, &zoom_draw_19, &zoom_draw_1a, &zoom_draw_1b, &zoom_draw_1c, &zoom_draw_1d, &zoom_draw_1e, &zoom_draw_1f,
	&zoom_draw_20, &zoom_draw_21, &zoom_draw_22, &zoom_draw_23, &zoom_draw_24, &zoom_draw_25, &zoom_draw_26, &zoom_draw_27, &zoom_draw_28, &zoom_draw_29, &zoom_draw_2a, &zoom_draw_2b, &zoom_draw_2c, &zoom_draw_2d, &zoom_draw_2e, &zoom_draw_2f,
	&zoom_draw_30, &zoom_draw_31, &zoom_draw_32, &zoom_draw_33, &zoom_draw_34, &zoom_draw_35, &zoom_draw_36, &zoom_draw_37, &zoom_draw_38, &zoom_draw_39, &zoom_draw_3a, &zoom_draw_3b, &zoom_draw_3c, &zoom_draw_3d, &zoom_draw_3e, &zoom_draw_3f,
	&zoom_draw_40, &zoom_draw_41, &zoom_draw_42, &zoom_draw_43, &zoom_draw_44, &zoom_draw_45, &zoom_draw_46, &zoom_draw_47, &zoom_draw_48, &zoom_draw_49, &zoom_draw_4a, &zoom_draw_4b, &zoom_draw_4c, &zoom_draw_4d, &zoom_draw_4e, &zoom_draw_4f,
	&zoom_draw_50, &zoom_draw_51, &zoom_draw_52, &zoom_draw_53, &zoom_draw_54, &zoom_draw_55, &zoom_draw_56, &zoom_draw_57, &zoom_draw_58, &zoom_draw_59, &zoom_draw_5a, &zoom_draw_5b, &zoom_draw_5c, &zoom_draw_5d, &zoom_draw_5e, &zoom_draw_5f,
	&zoom_draw_60, &zoom_draw_61, &zoom_draw_62, &zoom_draw_63, &zoom_draw_64, &zoom_draw_65, &zoom_draw_66, &zoom_draw_67, &zoom_draw_68, &zoom_draw_69, &zoom_draw_6a, &zoom_draw_6b, &zoom_draw_6c, &zoom_draw_6d, &zoom_draw_6e, &zoom_draw_6f,
	&zoom_draw_70, &zoom_draw_71, &zoom_draw_72, &zoom_draw_73, &zoom_draw_74, &zoom_draw_75, &zoom_draw_76, &zoom_draw_77, &zoom_draw_78, &zoom_draw_79, &zoom_draw_7a, &zoom_draw_7b, &zoom_draw_7c, &zoom_draw_7d, &zoom_draw_7e, &zoom_draw_7f,
	&zoom_draw_80, &zoom_draw_81, &zoom_draw_82, &zoom_draw_83, &zoom_draw_84, &zoom_draw_85, &zoom_draw_86, &zoom_draw_87, &zoom_draw_88, &zoom_draw_89, &zoom_draw_8a, &zoom_draw_8b, &zoom_draw_8c, &zoom_draw_8d, &zoom_draw_8e, &zoom_draw_8f,
	&zoom_draw_90, &zoom_draw_91, &zoom_draw_92, &zoom_draw_93, &zoom_draw_94, &zoom_draw_95, &zoom_draw_96, &zoom_draw_97, &zoom_draw_98, &zoom_draw_99, &zoom_draw_9a, &zoom_draw_9b, &zoom_draw_9c, &zoom_draw_9d, &zoom_draw_9e, &zoom_draw_9f,
	&zoom_draw_a0, &zoom_draw_a1, &zoom_draw_a2, &zoom_draw_a3, &zoom_draw_a4, &zoom_draw_a5, &zoom_draw_a6, &zoom_draw_a7, &zoom_draw_a8, &zoom_draw_a9, &zoom_draw_aa, &zoom_draw_ab, &zoom_draw_ac, &zoom_draw_ad, &zoom_draw_ae, &zoom_draw_af,
	&zoom_draw_b0, &zoom_draw_b1, &zoom_draw_b2, &zoom_draw_b3, &zoom_draw_b4, &zoom_draw_b5, &zoom_draw_b6, &zoom_draw_b7, &zoom_draw_b8, &zoom_draw_b9, &zoom_draw_ba, &zoom_draw_bb, &zoom_draw_bc, &zoom_draw_bd, &zoom_draw_be, &zoom_draw_bf,
	&zoom_draw_c0, &zoom_draw_c1, &zoom_draw_c2, &zoom_draw_c3, &zoom_draw_c4, &zoom_draw_c5, &zoom_draw_c6, &zoom_draw_c7, &zoom_draw_c8, &zoom_draw_c9, &zoom_draw_ca, &zoom_draw_cb, &zoom_draw_cc, &zoom_draw_cd, &zoom_draw_ce, &zoom_draw_cf,
	&zoom_draw_d0, &zoom_draw_d1, &zoom_draw_d2, &zoom_draw_d3, &zoom_draw_d4, &zoom_draw_d5, &zoom_draw_d6, &zoom_draw_d7, &zoom_draw_d8, &zoom_draw_d9, &zoom_draw_da, &zoom_draw_db, &zoom_draw_dc, &zoom_draw_dd, &zoom_draw_de, &zoom_draw_df,
	&zoom_draw_e0, &zoom_draw_e1, &zoom_draw_e2, &zoom_draw_e3, &zoom_draw_e4, &zoom_draw_e5, &zoom_draw_e6, &zoom_draw_e7, &zoom_draw_e8, &zoom_draw_e9, &zoom_draw_ea, &zoom_draw_eb, &zoom_draw_ec, &zoom_draw_ed, &zoom_draw_ee, &zoom_draw_ef,
	&zoom_draw_f0, &zoom_draw_f1, &zoom_draw_f2, &zoom_draw_f3, &zoom_draw_f4, &zoom_draw_f5, &zoom_draw_f6, &zoom_draw_f7, &zoom_draw_f8, &zoom_draw_f9, &zoom_draw_fa, &zoom_draw_fb, &zoom_draw_fc, &zoom_draw_fd, &zoom_draw_fe, &zoom_draw_ff
};

static sprite_draw_nozoom_function nozoom_draw_table[2][0x100] =
{
	{	
		&nozoom_draw_00, &nozoom_draw_01, &nozoom_draw_02, &nozoom_draw_03, &nozoom_draw_04, &nozoom_draw_05, &nozoom_draw_06, &nozoom_draw_07, &nozoom_draw_08, &nozoom_draw_09, &nozoom_draw_0a, &nozoom_draw_0b, &nozoom_draw_0c, &nozoom_draw_0d, &nozoom_draw_0e, &nozoom_draw_0f,
		&nozoom_draw_10, &nozoom_draw_11, &nozoom_draw_12, &nozoom_draw_13, &nozoom_draw_14, &nozoom_draw_15, &nozoom_draw_16, &nozoom_draw_17, &nozoom_draw_18, &nozoom_draw_19, &nozoom_draw_1a, &nozoom_draw_1b, &nozoom_draw_1c, &nozoom_draw_1d, &nozoom_draw_1e, &nozoom_draw_1f,
		&nozoom_draw_20, &nozoom_draw_21, &nozoom_draw_22, &nozoom_draw_23, &nozoom_draw_24, &nozoom_draw_25, &nozoom_draw_26, &nozoom_draw_27, &nozoom_draw_28, &nozoom_draw_29, &nozoom_draw_2a, &nozoom_draw_2b, &nozoom_draw_2c, &nozoom_draw_2d, &nozoom_draw_2e, &nozoom_draw_2f,
		&nozoom_draw_30, &nozoom_draw_31, &nozoom_draw_32, &nozoom_draw_33, &nozoom_draw_34, &nozoom_draw_35, &nozoom_draw_36, &nozoom_draw_37, &nozoom_draw_38, &nozoom_draw_39, &nozoom_draw_3a, &nozoom_draw_3b, &nozoom_draw_3c, &nozoom_draw_3d, &nozoom_draw_3e, &nozoom_draw_3f,
		&nozoom_draw_40, &nozoom_draw_41, &nozoom_draw_42, &nozoom_draw_43, &nozoom_draw_44, &nozoom_draw_45, &nozoom_draw_46, &nozoom_draw_47, &nozoom_draw_48, &nozoom_draw_49, &nozoom_draw_4a, &nozoom_draw_4b, &nozoom_draw_4c, &nozoom_draw_4d, &nozoom_draw_4e, &nozoom_draw_4f,
		&nozoom_draw_50, &nozoom_draw_51, &nozoom_draw_52, &nozoom_draw_53, &nozoom_draw_54, &nozoom_draw_55, &nozoom_draw_56, &nozoom_draw_57, &nozoom_draw_58, &nozoom_draw_59, &nozoom_draw_5a, &nozoom_draw_5b, &nozoom_draw_5c, &nozoom_draw_5d, &nozoom_draw_5e, &nozoom_draw_5f,
		&nozoom_draw_60, &nozoom_draw_61, &nozoom_draw_62, &nozoom_draw_63, &nozoom_draw_64, &nozoom_draw_65, &nozoom_draw_66, &nozoom_draw_67, &nozoom_draw_68, &nozoom_draw_69, &nozoom_draw_6a, &nozoom_draw_6b, &nozoom_draw_6c, &nozoom_draw_6d, &nozoom_draw_6e, &nozoom_draw_6f,
		&nozoom_draw_70, &nozoom_draw_71, &nozoom_draw_72, &nozoom_draw_73, &nozoom_draw_74, &nozoom_draw_75, &nozoom_draw_76, &nozoom_draw_77, &nozoom_draw_78, &nozoom_draw_79, &nozoom_draw_7a, &nozoom_draw_7b, &nozoom_draw_7c, &nozoom_draw_7d, &nozoom_draw_7e, &nozoom_draw_7f,
		&nozoom_draw_80, &nozoom_draw_81, &nozoom_draw_82, &nozoom_draw_83, &nozoom_draw_84, &nozoom_draw_85, &nozoom_draw_86, &nozoom_draw_87, &nozoom_draw_88, &nozoom_draw_89, &nozoom_draw_8a, &nozoom_draw_8b, &nozoom_draw_8c, &nozoom_draw_8d, &nozoom_draw_8e, &nozoom_draw_8f,
		&nozoom_draw_90, &nozoom_draw_91, &nozoom_draw_92, &nozoom_draw_93, &nozoom_draw_94, &nozoom_draw_95, &nozoom_draw_96, &nozoom_draw_97, &nozoom_draw_98, &nozoom_draw_99, &nozoom_draw_9a, &nozoom_draw_9b, &nozoom_draw_9c, &nozoom_draw_9d, &nozoom_draw_9e, &nozoom_draw_9f,
		&nozoom_draw_a0, &nozoom_draw_a1, &nozoom_draw_a2, &nozoom_draw_a3, &nozoom_draw_a4, &nozoom_draw_a5, &nozoom_draw_a6, &nozoom_draw_a7, &nozoom_draw_a8, &nozoom_draw_a9, &nozoom_draw_aa, &nozoom_draw_ab, &nozoom_draw_ac, &nozoom_draw_ad, &nozoom_draw_ae, &nozoom_draw_af,
		&nozoom_draw_b0, &nozoom_draw_b1, &nozoom_draw_b2, &nozoom_draw_b3, &nozoom_draw_b4, &nozoom_draw_b5, &nozoom_draw_b6, &nozoom_draw_b7, &nozoom_draw_b8, &nozoom_draw_b9, &nozoom_draw_ba, &nozoom_draw_bb, &nozoom_draw_bc, &nozoom_draw_bd, &nozoom_draw_be, &nozoom_draw_bf,
		&nozoom_draw_c0, &nozoom_draw_c1, &nozoom_draw_c2, &nozoom_draw_c3, &nozoom_draw_c4, &nozoom_draw_c5, &nozoom_draw_c6, &nozoom_draw_c7, &nozoom_draw_c8, &nozoom_draw_c9, &nozoom_draw_ca, &nozoom_draw_cb, &nozoom_draw_cc, &nozoom_draw_cd, &nozoom_draw_ce, &nozoom_draw_cf,
		&nozoom_draw_d0, &nozoom_draw_d1, &nozoom_draw_d2, &nozoom_draw_d3, &nozoom_draw_d4, &nozoom_draw_d5, &nozoom_draw_d6, &nozoom_draw_d7, &nozoom_draw_d8, &nozoom_draw_d9, &nozoom_draw_da, &nozoom_draw_db, &nozoom_draw_dc, &nozoom_draw_dd, &nozoom_draw_de, &nozoom_draw_df,
		&nozoom_draw_e0, &nozoom_draw_e1, &nozoom_draw_e2, &nozoom_draw_e3, &nozoom_draw_e4, &nozoom_draw_e5, &nozoom_draw_e6, &nozoom_draw_e7, &nozoom_draw_e8, &nozoom_draw_e9, &nozoom_draw_ea, &nozoom_draw_eb, &nozoom_draw_ec, &nozoom_draw_ed, &nozoom_draw_ee, &nozoom_draw_ef,
		&nozoom_draw_f0, &nozoom_draw_f1, &nozoom_draw_f2, &nozoom_draw_f3, &nozoom_draw_f4, &nozoom_draw_f5, &nozoom_draw_f6, &nozoom_draw_f7, &nozoom_draw_f8, &nozoom_draw_f9, &nozoom_draw_fa, &nozoom_draw_fb, &nozoom_draw_fc, &nozoom_draw_fd, &nozoom_draw_fe, &nozoom_draw_ff
	},
	{	
		&nozoom_draw_flipx_00, &nozoom_draw_flipx_01, &nozoom_draw_flipx_02, &nozoom_draw_flipx_03, &nozoom_draw_flipx_04, &nozoom_draw_flipx_05, &nozoom_draw_flipx_06, &nozoom_draw_flipx_07, &nozoom_draw_flipx_08, &nozoom_draw_flipx_09, &nozoom_draw_flipx_0a, &nozoom_draw_flipx_0b, &nozoom_draw_flipx_0c, &nozoom_draw_flipx_0d, &nozoom_draw_flipx_0e, &nozoom_draw_flipx_0f,
		&nozoom_draw_flipx_10, &nozoom_draw_flipx_11, &nozoom_draw_flipx_12, &nozoom_draw_flipx_13, &nozoom_draw_flipx_14, &nozoom_draw_flipx_15, &nozoom_draw_flipx_16, &nozoom_draw_flipx_17, &nozoom_draw_flipx_18, &nozoom_draw_flipx_19, &nozoom_draw_flipx_1a, &nozoom_draw_flipx_1b, &nozoom_draw_flipx_1c, &nozoom_draw_flipx_1d, &nozoom_draw_flipx_1e, &nozoom_draw_flipx_1f,
		&nozoom_draw_flipx_20, &nozoom_draw_flipx_21, &nozoom_draw_flipx_22, &nozoom_draw_flipx_23, &nozoom_draw_flipx_24, &nozoom_draw_flipx_25, &nozoom_draw_flipx_26, &nozoom_draw_flipx_27, &nozoom_draw_flipx_28, &nozoom_draw_flipx_29, &nozoom_draw_flipx_2a, &nozoom_draw_flipx_2b, &nozoom_draw_flipx_2c, &nozoom_draw_flipx_2d, &nozoom_draw_flipx_2e, &nozoom_draw_flipx_2f,
		&nozoom_draw_flipx_30, &nozoom_draw_flipx_31, &nozoom_draw_flipx_32, &nozoom_draw_flipx_33, &nozoom_draw_flipx_34, &nozoom_draw_flipx_35, &nozoom_draw_flipx_36, &nozoom_draw_flipx_37, &nozoom_draw_flipx_38, &nozoom_draw_flipx_39, &nozoom_draw_flipx_3a, &nozoom_draw_flipx_3b, &nozoom_draw_flipx_3c, &nozoom_draw_flipx_3d, &nozoom_draw_flipx_3e, &nozoom_draw_flipx_3f,
		&nozoom_draw_flipx_40, &nozoom_draw_flipx_41, &nozoom_draw_flipx_42, &nozoom_draw_flipx_43, &nozoom_draw_flipx_44, &nozoom_draw_flipx_45, &nozoom_draw_flipx_46, &nozoom_draw_flipx_47, &nozoom_draw_flipx_48, &nozoom_draw_flipx_49, &nozoom_draw_flipx_4a, &nozoom_draw_flipx_4b, &nozoom_draw_flipx_4c, &nozoom_draw_flipx_4d, &nozoom_draw_flipx_4e, &nozoom_draw_flipx_4f,
		&nozoom_draw_flipx_50, &nozoom_draw_flipx_51, &nozoom_draw_flipx_52, &nozoom_draw_flipx_53, &nozoom_draw_flipx_54, &nozoom_draw_flipx_55, &nozoom_draw_flipx_56, &nozoom_draw_flipx_57, &nozoom_draw_flipx_58, &nozoom_draw_flipx_59, &nozoom_draw_flipx_5a, &nozoom_draw_flipx_5b, &nozoom_draw_flipx_5c, &nozoom_draw_flipx_5d, &nozoom_draw_flipx_5e, &nozoom_draw_flipx_5f,
		&nozoom_draw_flipx_60, &nozoom_draw_flipx_61, &nozoom_draw_flipx_62, &nozoom_draw_flipx_63, &nozoom_draw_flipx_64, &nozoom_draw_flipx_65, &nozoom_draw_flipx_66, &nozoom_draw_flipx_67, &nozoom_draw_flipx_68, &nozoom_draw_flipx_69, &nozoom_draw_flipx_6a, &nozoom_draw_flipx_6b, &nozoom_draw_flipx_6c, &nozoom_draw_flipx_6d, &nozoom_draw_flipx_6e, &nozoom_draw_flipx_6f,
		&nozoom_draw_flipx_70, &nozoom_draw_flipx_71, &nozoom_draw_flipx_72, &nozoom_draw_flipx_73, &nozoom_draw_flipx_74, &nozoom_draw_flipx_75, &nozoom_draw_flipx_76, &nozoom_draw_flipx_77, &nozoom_draw_flipx_78, &nozoom_draw_flipx_79, &nozoom_draw_flipx_7a, &nozoom_draw_flipx_7b, &nozoom_draw_flipx_7c, &nozoom_draw_flipx_7d, &nozoom_draw_flipx_7e, &nozoom_draw_flipx_7f,
		&nozoom_draw_flipx_80, &nozoom_draw_flipx_81, &nozoom_draw_flipx_82, &nozoom_draw_flipx_83, &nozoom_draw_flipx_84, &nozoom_draw_flipx_85, &nozoom_draw_flipx_86, &nozoom_draw_flipx_87, &nozoom_draw_flipx_88, &nozoom_draw_flipx_89, &nozoom_draw_flipx_8a, &nozoom_draw_flipx_8b, &nozoom_draw_flipx_8c, &nozoom_draw_flipx_8d, &nozoom_draw_flipx_8e, &nozoom_draw_flipx_8f,
		&nozoom_draw_flipx_90, &nozoom_draw_flipx_91, &nozoom_draw_flipx_92, &nozoom_draw_flipx_93, &nozoom_draw_flipx_94, &nozoom_draw_flipx_95, &nozoom_draw_flipx_96, &nozoom_draw_flipx_97, &nozoom_draw_flipx_98, &nozoom_draw_flipx_99, &nozoom_draw_flipx_9a, &nozoom_draw_flipx_9b, &nozoom_draw_flipx_9c, &nozoom_draw_flipx_9d, &nozoom_draw_flipx_9e, &nozoom_draw_flipx_9f,
		&nozoom_draw_flipx_a0, &nozoom_draw_flipx_a1, &nozoom_draw_flipx_a2, &nozoom_draw_flipx_a3, &nozoom_draw_flipx_a4, &nozoom_draw_flipx_a5, &nozoom_draw_flipx_a6, &nozoom_draw_flipx_a7, &nozoom_draw_flipx_a8, &nozoom_draw_flipx_a9, &nozoom_draw_flipx_aa, &nozoom_draw_flipx_ab, &nozoom_draw_flipx_ac, &nozoom_draw_flipx_ad, &nozoom_draw_flipx_ae, &nozoom_draw_flipx_af,
		&nozoom_draw_flipx_b0, &nozoom_draw_flipx_b1, &nozoom_draw_flipx_b2, &nozoom_draw_flipx_b3, &nozoom_draw_flipx_b4, &nozoom_draw_flipx_b5, &nozoom_draw_flipx_b6, &nozoom_draw_flipx_b7, &nozoom_draw_flipx_b8, &nozoom_draw_flipx_b9, &nozoom_draw_flipx_ba, &nozoom_draw_flipx_bb, &nozoom_draw_flipx_bc, &nozoom_draw_flipx_bd, &nozoom_draw_flipx_be, &nozoom_draw_flipx_bf,
		&nozoom_draw_flipx_c0, &nozoom_draw_flipx_c1, &nozoom_draw_flipx_c2, &nozoom_draw_flipx_c3, &nozoom_draw_flipx_c4, &nozoom_draw_flipx_c5, &nozoom_draw_flipx_c6, &nozoom_draw_flipx_c7, &nozoom_draw_flipx_c8, &nozoom_draw_flipx_c9, &nozoom_draw_flipx_ca, &nozoom_draw_flipx_cb, &nozoom_draw_flipx_cc, &nozoom_draw_flipx_cd, &nozoom_draw_flipx_ce, &nozoom_draw_flipx_cf,
		&nozoom_draw_flipx_d0, &nozoom_draw_flipx_d1, &nozoom_draw_flipx_d2, &nozoom_draw_flipx_d3, &nozoom_draw_flipx_d4, &nozoom_draw_flipx_d5, &nozoom_draw_flipx_d6, &nozoom_draw_flipx_d7, &nozoom_draw_flipx_d8, &nozoom_draw_flipx_d9, &nozoom_draw_flipx_da, &nozoom_draw_flipx_db, &nozoom_draw_flipx_dc, &nozoom_draw_flipx_dd, &nozoom_draw_flipx_de, &nozoom_draw_flipx_df,
		&nozoom_draw_flipx_e0, &nozoom_draw_flipx_e1, &nozoom_draw_flipx_e2, &nozoom_draw_flipx_e3, &nozoom_draw_flipx_e4, &nozoom_draw_flipx_e5, &nozoom_draw_flipx_e6, &nozoom_draw_flipx_e7, &nozoom_draw_flipx_e8, &nozoom_draw_flipx_e9, &nozoom_draw_flipx_ea, &nozoom_draw_flipx_eb, &nozoom_draw_flipx_ec, &nozoom_draw_flipx_ed, &nozoom_draw_flipx_ee, &nozoom_draw_flipx_ef,
		&nozoom_draw_flipx_f0, &nozoom_draw_flipx_f1, &nozoom_draw_flipx_f2, &nozoom_draw_flipx_f3, &nozoom_draw_flipx_f4, &nozoom_draw_flipx_f5, &nozoom_draw_flipx_f6, &nozoom_draw_flipx_f7, &nozoom_draw_flipx_f8, &nozoom_draw_flipx_f9, &nozoom_draw_flipx_fa, &nozoom_draw_flipx_fb, &nozoom_draw_flipx_fc, &nozoom_draw_flipx_fd, &nozoom_draw_flipx_fe, &nozoom_draw_flipx_ff
	}
};

