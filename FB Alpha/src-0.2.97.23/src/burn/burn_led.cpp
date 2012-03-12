#include "burnint.h"
#include "burn_led.h"

#define MAX_LED		8

static INT32 led_status[MAX_LED];

static INT32 led_count;
static INT32 led_alpha_level;
static INT32 led_alpha_level2;
static INT32 led_color;
static INT32 led_size;
static INT32 led_position0;
static INT32 led_position;
static INT32 led_xpos;
static INT32 led_ypos;
static INT32 led_xadv;
static INT32 led_yadv;

static INT32 nScreenWidth, nScreenHeight;
static INT32 screen_flipped;
static INT32 flipscreen = -1;

static inline UINT32 alpha_blend32(UINT32 d)
{
	return (((((led_color & 0xff00ff) * led_alpha_level) + ((d & 0xff00ff) * led_alpha_level2)) & 0xff00ff00) |
		((((led_color & 0x00ff00) * led_alpha_level) + ((d & 0x00ff00) * led_alpha_level2)) & 0x00ff0000)) >> 8;
}

static void set_led_draw_position()
{
	led_position = led_position0;

	if (screen_flipped ^ flipscreen) {
		switch (led_position & 3) {
			case LED_POSITION_TOP_LEFT: led_position = LED_POSITION_BOTTOM_RIGHT; break;
			case LED_POSITION_TOP_RIGHT: led_position = LED_POSITION_BOTTOM_LEFT; break;
			case LED_POSITION_BOTTOM_LEFT: led_position = LED_POSITION_TOP_RIGHT; break;
			case LED_POSITION_BOTTOM_RIGHT: led_position = LED_POSITION_TOP_LEFT; break;
		}
	}

	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nScreenHeight, &nScreenWidth);

		led_xadv = 0;
		led_yadv = led_size + 1;
	
		switch (led_position & 3)
		{
			case LED_POSITION_TOP_LEFT:
				led_xpos = (nScreenWidth - 1) - led_size;
				led_ypos = 1;
			break;

			case LED_POSITION_BOTTOM_RIGHT:
				led_xpos = 1;
				led_ypos = (nScreenHeight - 1) - (led_yadv * led_count);
			break;

			case LED_POSITION_BOTTOM_LEFT:
				led_xpos = 1;
				led_ypos = 1;
			break;

			case LED_POSITION_TOP_RIGHT:
			default:
				led_xpos = (nScreenWidth  - 1) - led_size;
				led_ypos = (nScreenHeight - 1) - (led_yadv * led_count);
			break;
		}
	} else {
		BurnDrvGetVisibleSize(&nScreenWidth, &nScreenHeight);

		led_xadv = led_size + 1;
		led_yadv = 0;
	
		switch (led_position & 3)
		{
			case LED_POSITION_BOTTOM_LEFT:
				led_xpos = 1;
				led_ypos = (nScreenHeight - 1) - led_size;
//				led_ypos;
			break;

			case LED_POSITION_TOP_RIGHT:
				led_xpos = (nScreenWidth - 1) - (led_xadv * led_count);
				led_ypos = 1;
			break;

			case LED_POSITION_TOP_LEFT:
				led_xpos = 1;
				led_ypos = 1;
			break;

			case LED_POSITION_BOTTOM_RIGHT:
			default:
				led_xpos = (nScreenWidth  - 1) - (led_xadv * led_count);
				led_ypos = (nScreenHeight - 1) - led_size;
			break;
		}
	}
}

void BurnLEDSetFlipscreen(INT32 flip)
{
#if defined FBA_DEBUG
	if (!Debug_BurnLedInitted) bprintf(PRINT_ERROR, _T("BurnLEDSetFlipscreen called without init\n"));
#endif

	flip = flip ? 1 : 0;

	if (flipscreen != flip) {
		flipscreen = flip;
		set_led_draw_position();
	}
}

void BurnLEDReset()
{
#if defined FBA_DEBUG
	if (!Debug_BurnLedInitted) bprintf(PRINT_ERROR, _T("BurnLEDReset called without init\n"));
#endif

	memset (led_status, 0, MAX_LED * sizeof(INT32));

	BurnLEDSetFlipscreen(0);
}

void BurnLEDInit(INT32 num, INT32 position, INT32 size, INT32 color, INT32 transparency)
{
	Debug_BurnLedInitted = 1;
	
	if (num >= MAX_LED) num = MAX_LED - 1;

	led_count = num;
	led_color = color;
	led_size = size;
	led_position0 = position;

	led_alpha_level = (255 * transparency) / 100;
	led_alpha_level2 = 256 - led_alpha_level;

	screen_flipped = (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) ? 1 : 0;

	BurnLEDReset();
}

void BurnLEDSetStatus(INT32 led, UINT32 status)
{
#if defined FBA_DEBUG
	if (!Debug_BurnLedInitted) bprintf(PRINT_ERROR, _T("BurnLEDSetStatus called without init\n"));
	if (led >= led_count) bprintf(PRINT_ERROR, _T("BurnLEDSetStatus called with invalid led %x\n"), led);
#endif

	if (led >= led_count) return;

	if (screen_flipped ^ flipscreen) {
		led = (led_count - 1) - led;
	}

	led_status[led] = status ? 1 : 0;
}

void BurnLEDExit()
{
#if defined FBA_DEBUG
	if (!Debug_BurnLedInitted) bprintf(PRINT_ERROR, _T("BurnLEDExit called without init\n"));
#endif

	BurnLEDReset();

	led_count = 0;

	led_alpha_level = 0;
	led_alpha_level2 = 0;
	led_color = 0;
	led_size = 0;
	led_position = 0;
	led_position0 = 0;

	led_xpos = 0;
	led_ypos = 0;

	screen_flipped = 0;
	nScreenWidth = 0;
	nScreenHeight = 0;

	flipscreen = -1;
	
	Debug_BurnLedInitted = 0;
}

void BurnLEDRender()
{
#if defined FBA_DEBUG
	if (!Debug_BurnLedInitted) bprintf(PRINT_ERROR, _T("BurnLEDRender called without init\n"));
#endif

	INT32 xpos = led_xpos;
	INT32 ypos = led_ypos;
	int color = BurnHighCol((led_color >> 16) & 0xff, (led_color >> 8) & 0xff, (led_color >> 0) & 0xff, 0);

	for (INT32 i = 0; i < led_count; i++)
	{
		if (xpos < 0 || xpos > (nScreenWidth - led_size)) break;

		if (led_status[i]) 
		{
			for (INT32 y = 0; y < led_size; y++)
			{
				UINT8 *ptr = pBurnDraw + (((ypos + y) * nScreenWidth) + xpos) * nBurnBpp;

				for (INT32 x = 0; x < led_size; x++)
				{
					if (nBurnBpp >= 4)
					{
						*((UINT32*)ptr) = alpha_blend32(*((UINT32*)ptr));
					}
					else if (nBurnBpp == 3)
					{
						UINT32 t = alpha_blend32((ptr[2] << 16) | (ptr[1] << 8) | ptr[0]);
	
						ptr[2] = t >> 16;
						ptr[1] = t >> 8;
						ptr[0] = t >> 0;
					}
					else if (nBurnBpp == 2) // alpha blend not supported for 16-bit
					{
						*((UINT16*)ptr) =  color;
					}

					ptr += nBurnBpp;
				}
			}
		}

		xpos += led_xadv;
		ypos += led_yadv;
	}
}

INT32 BurnLEDScan(INT32 nAction, INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!Debug_BurnLedInitted) bprintf(PRINT_ERROR, _T("BurnLEDScan called without init\n"));
#endif

	struct BurnArea ba;

	if (pnMin != NULL) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_DRIVER_DATA) {
		ba.Data		= &led_status;
		ba.nLen		= led_count * sizeof(INT32);
		ba.nAddress	= 0;
		ba.szName	= "Led status";
		BurnAcb(&ba);
	}

	return 0;
}
