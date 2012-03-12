
#include "tiles_generic.h"
#include "math.h"

#define TABLE_SIZE	0x10000 // excessive?

struct vector_line {
	INT32 x;
	INT32 y;
	INT32 color;
	UINT8 intensity;
};

static struct vector_line *vector_table;
struct vector_line *vector_ptr; // pointer
static INT32 vector_cnt;

void vector_add_point(INT32 x, INT32 y, INT32 color, INT32 intensity)
{
	vector_ptr->x = x >> 16;
	vector_ptr->y = y >> 16;
	vector_ptr->color = color;
	vector_ptr->intensity = intensity;

	vector_cnt++;
	if (vector_cnt > (TABLE_SIZE - 2)) return;
	vector_ptr++;
	vector_ptr->color = -1; // mark it as the last one to save some cycles later...
}

static void lineSimple(INT32 x0, INT32 y0, INT32 x1, INT32 y1, INT32 color, INT32 intensity)
{
 	INT32 dx = x1 - x0;
	INT32 dy = y1 - y0;

	color = color * 256 + intensity;

	if (!dx && dy) // vertical line
	{
		INT32 sy = (y1 < y0) ? y1 : y0;
		INT32 y2 = (y1 < y0) ? (y0 - y1) : (y1 - y0);

		if (x1 >= 0 && x1 < nScreenWidth)
		{
			UINT16 *dst = pTransDraw + x1;

			for (INT32 y = 0; y < y2; y++, sy++) {
				if (sy >= 0 && sy < nScreenHeight) {
					dst[sy * nScreenWidth] = color;
				}
			}
		}
	}
	else if (!dy && dx) // horizontal line
	{
		INT32 sx = (x1 < x0) ? x1 : x0;
		INT32 x2 = (x1 < x0) ? (x0 - x1) : (x1 - x0);

		if (y1 >= 0 && y1 < nScreenHeight)
		{
			UINT16 *dst = pTransDraw + y1 * nScreenWidth;

			for (INT32 x = 0; x < x2; x++, sx++) {
				if (sx >= 0 && sx < nScreenWidth) {
					dst[sx] = color;
				}
			}
		}
	}
	else if (dx && dy) // can we optimize further?
	{
		INT32 md = (dy << 16) / dx;
		INT32 zd = (y0 << 16) - (md * x0) + 0x8000; // + 0x8000 for rounding!

		dx = (x1 > x0) ? 1 : -1;

		while (x0 != x1) {
			x0 += dx;
			y0 =  ((md * x0) + zd) >> 16;

			if (x0 >= 0 && x0 < nScreenWidth && y0 >= 0 && y0 < nScreenHeight) {
				pTransDraw[y0 * nScreenWidth + x0] = color;
			}
		}
	}
	else // point
	{
		if (x0 >= 0 && x0 < nScreenWidth && y0 >= 0 && y0 < nScreenHeight) {
			pTransDraw[y0 * nScreenWidth + x0] = color;
		}
	}
}

void draw_vector(UINT32 *palette)
{
	struct vector_line *ptr = &vector_table[0];

	INT32 prev_x = 0, prev_y = 0;

	BurnTransferClear();

	for (INT32 i = 0; i < vector_cnt && i < TABLE_SIZE; i++, ptr++)
	{
		if (ptr->color == -1) break;

		INT32 curr_y = ptr->y;
		INT32 curr_x = ptr->x;

		if (ptr->intensity != 0) { // intensity 0 means turn off the beam...
			lineSimple(curr_x, curr_y, prev_x, prev_y, ptr->color, ptr->intensity);
		}

		prev_x = curr_x;
		prev_y = curr_y;
	}

	BurnTransferCopy(palette);
}

void vector_reset()
{
	vector_cnt = 0;
	vector_ptr = &vector_table[0];
	vector_ptr->color = -1;
}

void vector_init()
{
	GenericTilesInit();

	vector_table = (struct vector_line*)malloc(TABLE_SIZE * sizeof(vector_line));

	memset (vector_table, 0, TABLE_SIZE * sizeof(vector_line));

	vector_reset();
}

void vector_exit()
{
	GenericTilesExit();

	free (vector_table);
	vector_table = NULL;
	vector_ptr = NULL;
}

INT32 vector_scan(INT32 nAction)
{
	struct BurnArea ba;

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = (UINT8*)vector_table;
		ba.nLen	  = TABLE_SIZE * sizeof(vector_line);
		ba.szName = "Vector Table";
		BurnAcb(&ba);

		SCAN_VAR(vector_cnt);
	}

	if (nAction & ACB_WRITE) {
		vector_ptr = &vector_table[vector_cnt];
	}

	return 0;
}
