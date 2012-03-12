#include <stdio.h>

int main()
{
	printf ("typedef INT32 (*sprite_draw_function)(UINT16 *dest, UINT8 *adata, INT32 pal);\n");
	printf ("typedef INT32 (*sprite_draw_nozoom_function)(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 pri);\n\n");

	int i,j;
	for (i = 0; i < 0x100; i++)
	{
		if (i == 0xff)
			printf ("static INT32 zoom_draw_%2.2x(UINT16 *dest, UINT8 *, INT32 )\n", i);
		else
			printf ("static INT32 zoom_draw_%2.2x(UINT16 *dest, UINT8 *adata, INT32 pal)\n", i);

		printf ("{\n");

		int cntr = 0;

		for (j = 0; j < 8; j++)
		{
			if (i & (1 << j))
			{
				printf ("\tdest[%d] = 0x8000;\n", j);
			}
			else
			{
				printf ("\tdest[%d] = adata[%d] + pal;\n", j, cntr);
				cntr++;
			}
		}

		printf ("\n\treturn 0x%2.2x;\n}\n\n", cntr);
	}

	for (i = 0; i < 0x100; i++)
	{
		if (i == 0xff)
			printf ("static INT32 nozoom_draw_%2.2x(UINT16 *, UINT8 *, UINT8 *, INT32 , INT32)\n", i);
		else
			printf ("static INT32 nozoom_draw_%2.2x(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)\n", i);

		printf ("{\n");

		int cntr = 0;

		for (j = 0; j < 8; j++)
		{
			if (~i & (1 << j))
			{
				printf ("\tdest[%d] = adata[%d] + pal;\n", j, cntr);
				printf ("\tpdest[%d] = prio;\n", j);
				cntr++;
			}
		}

		printf ("\n\treturn 0x%2.2x;}\n\n\n", cntr);
	}

	for (i = 0; i < 0x100; i++)
	{
		if (i == 0xff)
			printf ("static INT32 nozoom_draw_flipx_%2.2x(UINT16 *, UINT8 *, UINT8 *, INT32 , INT32)\n", i);
		else
			printf ("static INT32 nozoom_draw_flipx_%2.2x(UINT16 *dest, UINT8 *pdest, UINT8 *adata, INT32 pal, INT32 prio)\n", i);

		printf ("{\n");

		int cntr = 0;

		for (j = 0; j < 8; j++)
		{
			if (~i & (1 << j))
			{
				printf ("\tdest[%d] = adata[%d] + pal;\n", 7-j, cntr);
				printf ("\tpdest[%d] = prio;\n", 7-j);
				cntr++;
			}
		}

		printf ("\n\treturn 0x%2.2x;\n}\n\n", cntr);
	}

	printf ("static sprite_draw_function zoom_draw_table[0x100] = {\n");
	for (i = 0; i < 0x100; i++)
	{
		if ((i & 0x0f) == 0) printf ("\t");
		printf ("&zoom_draw_%2.2x", i);
		if (i != 0xff) printf (",");
		if ((i & 0x0f)==0x0f) {
			printf ("\n");
		} else {
			printf (" ");
		}
	}
	printf ("};\n\n");

	printf ("static sprite_draw_nozoom_function nozoom_draw_table[2][0x100] =\n{\n");
	printf ("\t{\t\n");
	for (i = 0; i < 0x100; i++)
	{
		if ((i & 0x0f) == 0) printf ("\t\t");
		printf ("&nozoom_draw_%2.2x", i);
		if (i != 0xff) printf (",");
		if ((i & 0x0f)==0x0f) {
			printf ("\n");
		} else {
			printf (" ");
		}
	}
	printf ("\t},\n");

	printf ("\t{\t\n");
	for (i = 0; i < 0x100; i++)
	{
		if ((i & 0x0f) == 0) printf ("\t\t");
		printf ("&nozoom_draw_flipx_%2.2x", i);
		if (i != 0xff) printf (",");
		if ((i & 0x0f)==0x0f) {
			printf ("\n");
		} else {
			printf (" ");
		}
	}
	printf ("\t}\n};\n\n");
	
	return 0;
}
