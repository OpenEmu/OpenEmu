#include "tiles_generic.h"
#include "h6280_intf.h"

UINT16 *vce_data;			// allocate externally!
static UINT16 vce_address;
static UINT16 vce_control;

static INT32 vce_current_bitmap_line;

UINT8 *vdc_vidram[2];			// allocate externally!

static UINT8	vdc_register[2];
static UINT16	vdc_data[2][32];
static UINT8	vdc_latch[2];
static UINT16	vdc_yscroll[2];
static UINT16	vdc_width[2];
static UINT16	vdc_height[2];
static UINT8	vdc_inc[2];
static UINT8	vdc_dvssr_write[2];
static UINT8	vdc_status[2];
static UINT16	vdc_sprite_ram[2][0x100];

static INT32	vdc_vblank_triggered[2];
static UINT16	vdc_current_segment[2];
static UINT16	vdc_current_segment_line[2];
static INT32	vdc_raster_count[2];
static INT32	vdc_curline[2];
static INT32	vdc_satb_countdown[2];

UINT16 *vdc_tmp_draw;			// allocate externally!

static UINT16 vpc_priority;
static UINT16 vpc_window1;
static UINT16 vpc_window2;
static UINT8 vpc_vdc_select;
static UINT8 vpc_prio[4];
static UINT8 vpc_vdc0_enabled[4];
static UINT8 vpc_vdc1_enabled[4];
static UINT8 vpc_prio_map[0x200];

//--------------------------------------------------------------------------------------------------------------------------------

static void vpc_update_prio_map()
{
	for (INT32 i = 0; i < 512; i++)
	{
		vpc_prio_map[i] = 0;
		if (vpc_window1 < 0x40 || i > vpc_window1) vpc_prio_map[i] |= 1;
		if (vpc_window2 < 0x40 || i > vpc_window2) vpc_prio_map[i] |= 2;
	}
}

void vpc_write(UINT8 offset, UINT8 data)
{
	switch (offset & 0x07)
	{
		case 0x00:	/* Priority register #0 */
			vpc_priority = (vpc_priority & 0xff00) | data;
			vpc_prio[0] = ( data >> 2 ) & 3;
			vpc_vdc0_enabled[0] = data & 1;
			vpc_vdc1_enabled[0] = data & 2;
			vpc_prio[1] = ( data >> 6 ) & 3;
			vpc_vdc0_enabled[1] = data & 0x10;
			vpc_vdc1_enabled[1] = data & 0x20;
		break;

		case 0x01:	/* Priority register #1 */
			vpc_priority = (vpc_priority & 0x00ff) | (data << 8);
			vpc_prio[2] = ( data >> 2 ) & 3;
			vpc_vdc0_enabled[2] = data & 1;
			vpc_vdc1_enabled[2] = data & 2;
			vpc_prio[3] = ( data >> 6 ) & 3;
			vpc_vdc0_enabled[3] = data & 0x10;
			vpc_vdc1_enabled[3] = data & 0x20;
		break;

		case 0x02:	/* Window 1 LSB */
			vpc_window1 = (vpc_window1 & 0xff00) | data;
			vpc_update_prio_map();
		break;

		case 0x03:	/* Window 1 MSB */
			vpc_window1 = (vpc_window1 & 0x00ff) | ((data & 3) << 8);
			vpc_update_prio_map();
		break;

		case 0x04:	/* Window 2 LSB */
			vpc_window2 = (vpc_window2 & 0xff00) | data;
			vpc_update_prio_map();
		break;

		case 0x05:	/* Window 2 MSB */
			vpc_window2 = (vpc_window2 & 0x00ff) | ((data & 3) << 8);
			vpc_update_prio_map();
		break;

		case 0x06:	/* VDC I/O select */
			vpc_vdc_select = data & 1;
		break;
	}
}

UINT8 vpc_read(UINT8 offset)
{
	switch (offset & 0x07)
	{
		case 0x00:  /* Priority register #0 */
			return vpc_priority & 0xff;

		case 0x01:  /* Priority register #1 */
			return vpc_priority >> 8;

		case 0x02:  /* Window 1 LSB */
			return vpc_window1 & 0xff;

		case 0x03:  /* Window 1 MSB; high bits are 0 or 1? */
			return vpc_window1 >> 8;

		case 0x04:  /* Window 2 LSB */
			return vpc_window2 & 0xff;

		case 0x05:  /* Window 2 MSB; high bits are 0 or 1? */
			return vpc_window2 >> 8;
	}

	return 0;
}

void vpc_reset()
{
	memset (vpc_prio, 0, 4);
	memset (vpc_vdc0_enabled, 0, 4);
	memset (vpc_vdc1_enabled, 0, 4);
	memset (vpc_prio_map, 0, 0x200);

	vpc_write(0, 0x11);
	vpc_write(1, 0x11);
	vpc_window1 = 0;
	vpc_window2 = 0;
	vpc_vdc_select = 0;
	vpc_priority = 0;
}

//--------------------------------------------------------------------------------------------------------------------------------

UINT8 vce_read(UINT8 offset)
{
	switch (offset & 7)
	{
		case 0x04:
			return (vce_data[vce_address] >> 0) & 0xff; //.l

		case 0x05:
			UINT8 ret = ((vce_data[vce_address] >> 8) & 0xff) | 0xfe; //.h
			vce_address = (vce_address + 1) & 0x01ff;
			return ret;
	}

	return 0xff;
}

void vce_write(UINT8 offset, UINT8 data)
{
	switch (offset & 7)
	{
		case 0x00:
			vce_control = data;
			break;

		case 0x02:
			vce_address = (vce_address & 0x100) | data;
			break;

		case 0x03:
			vce_address = (vce_address & 0x0ff) | ((data & 1) << 8);
			break;

		case 0x04:
			vce_data[vce_address] = (vce_data[vce_address] & 0x100) | data;
			break;

		case 0x05:
			vce_data[vce_address] = (vce_data[vce_address] & 0x0ff) | ((data & 1) << 8);
			vce_address = (vce_address + 1) & 0x01ff;
			break;
	}
}

void vce_palette_init(UINT32 *Palette)
{
	for (INT32 i = 0; i < 512; i++)
	{
		int r = ((i >> 3) & 7) << 5;
		int g = ((i >> 6) & 7) << 5;
		int b = ((i >> 0) & 7) << 5;

		int y = ((66 * r + 129 * g +  25 * b + 128) >> 8) +  16;

		Palette[0x000 + i] = BurnHighCol(r, g, b, 0);
		Palette[0x200 + i] = BurnHighCol(y, y, y, 0);
	}
}

void vce_reset()
{
	memset (vce_data, 0, 512 * sizeof(UINT16));

	vce_address = 0;
	vce_control = 0;

	vce_current_bitmap_line = 0;
}


//--------------------------------------------------------------------------------------------------------------------------------

#define STATE_VSW		0
#define STATE_VDS		1
#define STATE_VDW		2
#define STATE_VCR		3

#define VDC_WPF			684		/* width of a line in frame including blanking areas */
#define VDC_LPF		 262	 /* number of lines in a single frame */

#define VDC_BSY		 0x40	/* Set when the VDC accesses VRAM */
#define VDC_VD		  0x20	/* Set when in the vertical blanking period */
#define VDC_DV		  0x10	/* Set when a VRAM > VRAM DMA transfer is done */
#define VDC_DS		  0x08	/* Set when a VRAM > SATB DMA transfer is done */
#define VDC_RR		  0x04	/* Set when the current scanline equals the RCR register */
#define VDC_OR		  0x02	/* Set when there are more than 16 sprites on a line */
#define VDC_CR		  0x01	/* Set when sprite #0 overlaps with another sprite */

#define CR_BB		   0x80	/* Background blanking */
#define CR_SB		   0x40	/* Object blanking */
#define CR_VR		   0x08	/* Interrupt on vertical blank enable */
#define CR_RC		   0x04	/* Interrupt on line compare enable */
#define CR_OV		   0x02	/* Interrupt on sprite overflow enable */
#define CR_CC		   0x01	/* Interrupt on sprite #0 collision enable */

#define DCR_DSR		 0x10	/* VRAM > SATB auto-transfer enable */
#define DCR_DID		 0x08	/* Destination diretion */
#define DCR_SID		 0x04	/* Source direction */
#define DCR_DVC		 0x02	/* VRAM > VRAM EOT interrupt enable */
#define DCR_DSC		 0x01	/* VRAM > SATB EOT interrupt enable */

enum vdc_regs {MAWR = 0, MARR, VxR, reg3, reg4, CR, RCR, BXR, BYR, MWR, HSR, HDR, VPR, VDW, VCR, DCR, SOUR, DESR, LENR, DVSSR };


static void conv_obj(int which, int i, int l, int hf, int vf, char *buf)
{
	int b0, b1, b2, b3, i0, i1, i2, i3, x;
	int xi;
	int tmp;

	l &= 0x0F;
	if(vf) l = (15 - l);

	tmp = l + ( i << 5);

	b0  = vdc_vidram[which][(tmp + 0x00) * 2 + 0];
	b0 |= vdc_vidram[which][(tmp + 0x00) * 2 + 1]<<8;
	b1  = vdc_vidram[which][(tmp + 0x10) * 2 + 0];
	b1 |= vdc_vidram[which][(tmp + 0x10) * 2 + 1]<<8;
	b2  = vdc_vidram[which][(tmp + 0x20) * 2 + 0];
	b2 |= vdc_vidram[which][(tmp + 0x20) * 2 + 1]<<8;
	b3  = vdc_vidram[which][(tmp + 0x30) * 2 + 0];
	b3 |= vdc_vidram[which][(tmp + 0x30) * 2 + 1]<<8;

	for(x=0;x<16;x++)
	{
		if(hf) xi = x; else xi = (15 - x);
		i0 = (b0 >> xi) & 1;
		i1 = (b1 >> xi) & 1;
		i2 = (b2 >> xi) & 1;
		i3 = (b3 >> xi) & 1;
		buf[x] = (i3 << 3 | i2 << 2 | i1 << 1 | i0);
	}
}

static void pce_refresh_sprites(int which, int line, UINT8 *drawn, UINT16 *line_buffer)
{
	int i;
	UINT8 sprites_drawn = 0;

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce_control & 0x80 ? 512 : 0;

	/* count up: Highest priority is Sprite 0 */
	for(i = 0; i < 64; i++)
	{
		static const int cgy_table[] = {16, 32, 64, 64};

		int obj_y = (vdc_sprite_ram[which][(i << 2) + 0] & 0x03FF) - 64;
		int obj_x = (vdc_sprite_ram[which][(i << 2) + 1] & 0x03FF) - 32;
		int obj_i = (vdc_sprite_ram[which][(i << 2) + 2] & 0x07FE);
		int obj_a = (vdc_sprite_ram[which][(i << 2) + 3]);
		int cgx   = (obj_a >> 8) & 1;   /* sprite width */
		int cgy   = (obj_a >> 12) & 3;  /* sprite height */
		int hf	= (obj_a >> 11) & 1;  /* horizontal flip */
		int vf	= (obj_a >> 15) & 1;  /* vertical flip */
		int palette = (obj_a & 0x000F);
		int priority = (obj_a >> 7) & 1;
		int obj_h = cgy_table[cgy];
		int obj_l = (line - obj_y);
		int cgypos;
		char buf[16];

		if ((obj_y == -64) || (obj_y > line)) continue;
		if ((obj_x == -32) || (obj_x >= vdc_width[which])) continue;

		/* no need to draw an object that's ABOVE where we are. */
		if((obj_y + obj_h) < line) continue;

		/* If CGX is set, bit 0 of sprite pattern index is forced to 0 */
		if ( cgx )
			obj_i &= ~2;

		/* If CGY is set to 1, bit 1 of the sprite pattern index is forced to 0. */
		if ( cgy & 1 )
			obj_i &= ~4;

		/* If CGY is set to 2 or 3, bit 1 and 2 of the sprite pattern index are forced to 0. */
		if ( cgy & 2 )
			obj_i &= ~12;

		if (obj_l < obj_h)
		{
			sprites_drawn++;
			if(sprites_drawn > 16)
			{
				if(vdc_data[which][0x05] & 0x02)
				{
					/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
					vdc_status[which] |= 0x02;
					h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
				}
				continue;  /* Should cause an interrupt */
			}

			cgypos = (obj_l >> 4);
			if(vf) cgypos = ((obj_h - 1) >> 4) - cgypos;

			if(cgx == 0)
			{
				int x;
				int pixel_x = ( ( obj_x * 512 ) / vdc_width[which] );

				conv_obj(which, obj_i + (cgypos << 2), obj_l, hf, vf, buf);

				for(x = 0; x < 16; x++)
				{
					if(((obj_x + x) < (vdc_width[which])) && ((obj_x + x) >= 0))
					{
						if ( buf[x] )
						{
							if( drawn[pixel_x] < 2 )
							{
								if( priority || drawn[pixel_x] == 0 )
								{
									line_buffer[pixel_x] = color_base + vce_data[0x100 + (palette << 4) + buf[x]];

									if ( vdc_width[which] != 512 )
									{
										int dp = 1;
										while ( pixel_x + dp < ( ( ( obj_x + x + 1 ) * 512 ) / vdc_width[which] ) )
										{
											drawn[pixel_x + dp] = i + 2;
											line_buffer[pixel_x + dp] = color_base + vce_data[0x100 + (palette << 4) + buf[x]];
											dp++;
										}
									}
								}
								drawn[pixel_x] = i + 2;
							}
							/* Check for sprite #0 collision */
							else if (drawn[pixel_x] == 2)
							{
								if(vdc_data[which][0x05] & 0x01)
									h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
								vdc_status[which] |= 0x01;
							}
						}
					}
					if ( vdc_width[which] != 512 )
					{
						pixel_x = ( ( obj_x + x + 1 ) * 512 ) / vdc_width[which];
					}
					else
					{
						pixel_x += 1;
					}
				}
			}
			else
			{
				int x;
				int pixel_x = ( ( obj_x * 512 ) / vdc_width[which] );

				conv_obj(which, obj_i + (cgypos << 2) + (hf ? 2 : 0), obj_l, hf, vf, buf);

				for(x = 0; x < 16; x++)
				{
					if(((obj_x + x) < (vdc_width[which])) && ((obj_x + x) >= 0))
					{
						if ( buf[x] )
						{
							if( drawn[pixel_x] < 2 )
							{
								if ( priority || drawn[pixel_x] == 0 )
								{
									line_buffer[pixel_x] = color_base + vce_data[0x100 + (palette << 4) + buf[x]];
									if ( vdc_width[which] != 512 )
									{
										int dp = 1;
										while ( pixel_x + dp < ( ( ( obj_x + x + 1 ) * 512 ) / vdc_width[which] ) )
										{
											drawn[pixel_x + dp] = i + 2;
											line_buffer[pixel_x + dp] = color_base + vce_data[0x100 + (palette << 4) + buf[x]];
											dp++;
										}
									}
								}
								drawn[pixel_x] = i + 2;
							}
							/* Check for sprite #0 collision */
							else if ( drawn[pixel_x] == 2 )
							{
								if(vdc_data[which][0x05] & 0x01)
									h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
								vdc_status[which] |= 0x01;
							}
						}
					}
					if ( vdc_width[which] != 512 )
					{
						pixel_x = ( ( obj_x + x + 1 ) * 512 ) / vdc_width[which];
					}
					else
					{
						pixel_x += 1;
					}
				}

				/* 32 pixel wide sprites are counted as 2 sprites and the right half
				   		is only drawn if there are 2 open slots.
						*/
				sprites_drawn++;
				if( sprites_drawn > 16 )
				{
					if(vdc_data[which][0x05] & 0x02)
					{
						/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
						vdc_status[which] |= 0x02;
						h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
					}
				}
				else
				{
					conv_obj(which, obj_i + (cgypos << 2) + (hf ? 0 : 2), obj_l, hf, vf, buf);
					for(x = 0; x < 16; x++)
					{
						if(((obj_x + 0x10 + x) < (vdc_width[which])) && ((obj_x + 0x10 + x) >= 0))
						{
							if ( buf[x] )
							{
								if( drawn[pixel_x] < 2 )
								{
									if( priority || drawn[pixel_x] == 0 )
									{
										line_buffer[pixel_x] = color_base + vce_data[0x100 + (palette << 4) + buf[x]];
										if ( vdc_width[which] != 512 )
										{
											int dp = 1;
											while ( pixel_x + dp < ( ( ( obj_x + x + 17 ) * 512 ) / vdc_width[which] ) )
											{
												drawn[pixel_x + dp] = i + 2;
												line_buffer[pixel_x + dp] = color_base + vce_data[0x100 + (palette << 4) + buf[x]];
												dp++;
											}
										}
									}
									drawn[pixel_x] = i + 2;
								}
								/* Check for sprite #0 collision */
								else if ( drawn[pixel_x] == 2 )
								{
									if(vdc_data[which][0x05] & 0x01)
										h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
									vdc_status[which] |= 0x01;
								}
							}
						}
						if ( vdc_width[which] != 512 )
						{
							pixel_x = ( ( obj_x + x + 17 ) * 512 ) / vdc_width[which];
						}
						else
						{
							pixel_x += 1;
						}
					}
				}
			}
		}
	}
}


static void draw_overscan_line(int line)
{
	int i;

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce_control & 0x80 ? 512 : 0;

	/* our line buffer */
	UINT16 *line_buffer = vdc_tmp_draw + line * 684; //&vce.bmp->pix16(line);

	for ( i = 0; i < 684; i++ )
		line_buffer[i] = color_base + vce_data[0x100];
}

static void draw_sgx_overscan_line(int line)
{
	int i;

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce_control & 0x80 ? 512 : 0;

	/* our line buffer */
	UINT16 *line_buffer = vdc_tmp_draw + line * 684; //&vce.bmp->pix16(line);

	for ( i = 0; i < VDC_WPF; i++ )
		line_buffer[i] = color_base + vce_data[0];
}

static void draw_black_line(int line)
{
	UINT16 *line_buffer = vdc_tmp_draw + line * 684; //&vce.bmp->pix16(line);

	for(INT32 i=0; i< 684; i++)
		line_buffer[i] = 0x400; // black
}

static void vdc_advance_line(int which)
{
	int ret = 0;

	vdc_curline[which] += 1;
	vdc_current_segment_line[which] += 1;
	vdc_raster_count[which] += 1;

	if ( vdc_satb_countdown[which] )
	{
		vdc_satb_countdown[which] -= 1;
		if ( vdc_satb_countdown[which] == 0 )
		{
			if ( vdc_data[which][DCR] & DCR_DSC )
			{
				vdc_status[which] |= VDC_DS;	/* set satb done flag */
				ret = 1;
			}
		}
	}

	if ( vce_current_bitmap_line == 0 )
	{
		vdc_current_segment[which] = STATE_VSW;
		vdc_current_segment_line[which] = 0;
		vdc_vblank_triggered[which] = 0;
		vdc_curline[which] = 0;
	}

	if ( STATE_VSW == vdc_current_segment[which] && vdc_current_segment_line[which] >= ( (vdc_data[which][VPR]&0xff) & 0x1F ) )
	{
		vdc_current_segment[which] = STATE_VDS;
		vdc_current_segment_line[which] = 0;
	}

	if ( STATE_VDS == vdc_current_segment[which] && vdc_current_segment_line[which] >= (vdc_data[which][VPR] >> 8) )
	{
		vdc_current_segment[which] = STATE_VDW;
		vdc_current_segment_line[which] = 0;
		vdc_raster_count[which] = 0x40;
	}

	if ( STATE_VDW == vdc_current_segment[which] && vdc_current_segment_line[which] > ( vdc_data[which][VDW] & 0x01FF ) )
	{
		vdc_current_segment[which] = STATE_VCR;
		vdc_current_segment_line[which] = 0;

		/* Generate VBlank interrupt, sprite DMA */
		vdc_vblank_triggered[which] = 1;
		if ( vdc_data[which][CR] & CR_VR )
		{
			vdc_status[which] |= VDC_VD;
			ret = 1;
		}

		/* do VRAM > SATB DMA if the enable bit is set or the DVSSR reg. was written to */
		if( ( vdc_data[which][DCR] & DCR_DSR ) || vdc_dvssr_write[which] )
		{
			int i;

			vdc_dvssr_write[which] = 0;

			for( i = 0; i < 256; i++ )
			{
				vdc_sprite_ram[which][i] = ( vdc_vidram[which][ ( vdc_data[which][DVSSR] << 1 ) + i * 2 + 1 ] << 8 ) | vdc_vidram[which][ ( vdc_data[which][DVSSR] << 1 ) + i * 2 ];
			}

			/* generate interrupt if needed */
			if ( vdc_data[which][DCR] & DCR_DSC )
			{
				vdc_satb_countdown[which] = 4;
			}
		}
	}

	if ( STATE_VCR == vdc_current_segment[which] )
	{
		if ( vdc_current_segment_line[which] >= 3 && vdc_current_segment_line[which] >= (vdc_data[which][VCR]&0xff) )
		{
			vdc_current_segment[which] = STATE_VSW;
			vdc_current_segment_line[which] = 0;
			vdc_curline[which] = 0;
		}
	}

	/* generate interrupt on line compare if necessary */
	if ( vdc_raster_count[which] == vdc_data[which][RCR] && vdc_data[which][CR] & CR_RC )
	{
		vdc_status[which] |= VDC_RR;
		ret = 1;
	}

	/* handle frame events */
	if(vdc_curline[which] == 261 && ! vdc_vblank_triggered[which] )
	{

		vdc_vblank_triggered[which] = 1;
		if(vdc_data[which][CR] & CR_VR)
		{	/* generate IRQ1 if enabled */
			vdc_status[which] |= VDC_VD;	/* set vblank flag */
			ret = 1;
		}

		/* do VRAM > SATB DMA if the enable bit is set or the DVSSR reg. was written to */
		if ( ( vdc_data[which][DCR] & DCR_DSR ) || vdc_dvssr_write[which] )
		{
			int i;

			vdc_dvssr_write[which] = 0;

			for( i = 0; i < 256; i++ )
			{
				vdc_sprite_ram[which][i] = ( vdc_vidram[which][ ( vdc_data[which][DVSSR] << 1 ) + i * 2 + 1 ] << 8 ) | vdc_vidram[which][ ( vdc_data[which][DVSSR] << 1 ) + i * 2 ];
			}

			/* generate interrupt if needed */
			if(vdc_data[which][DCR] & DCR_DSC)
			{
				vdc_satb_countdown[which] = 4;
			}
		}
	}

	if (ret)
		h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
}

static void pce_refresh_line(int which, int /*line*/, int external_input, UINT8 *drawn, UINT16 *line_buffer)
{
	static const int width_table[4] = {5, 6, 7, 7};

	int scroll_y = ( vdc_yscroll[which] & 0x01FF);
	int scroll_x = (vdc_data[which][BXR] & 0x03FF);
	int nt_index;

	/* is virtual map 32 or 64 characters tall ? (256 or 512 pixels) */
	int v_line = (scroll_y) & (vdc_data[which][MWR] & 0x0040 ? 0x1FF : 0x0FF);

	/* row within character */
	int v_row = (v_line & 7);

	/* row of characters in BAT */
	int nt_row = (v_line >> 3);

	/* virtual X size (# bits to shift) */
	int v_width =		width_table[(vdc_data[which][MWR] >> 4) & 3];

	/* pointer to the name table (Background Attribute Table) in VRAM */
	UINT8 *bat = &(vdc_vidram[which][nt_row << (v_width+1)]);

	/* Are we in greyscale mode or in color mode? */
	int color_base = vce_control & 0x80 ? 512 : 0;

	int b0, b1, b2, b3;
	int i0, i1, i2, i3;
	int cell_pattern_index;
	int cell_palette;
	int x, c, i;

	/* character blanking bit */
	if(!(vdc_data[which][CR] & CR_BB))
	{
		return;
	}
	else
	{
		int	pixel = 0;
		int phys_x = - ( scroll_x & 0x07 );

		for(i=0;i<(vdc_width[which] >> 3) + 1;i++)
		{
			nt_index = (i + (scroll_x >> 3)) & ((2 << (v_width-1))-1);
			nt_index *= 2;

			/* get name table data: */

			/* palette # = index from 0-15 */
			cell_palette = ( bat[nt_index + 1] >> 4 ) & 0x0F;

			/* This is the 'character number', from 0-0x0FFF		 */
			/* then it is shifted left 4 bits to form a VRAM address */
			/* and one more bit to convert VRAM word offset to a	 */
			/* byte-offset within the VRAM space					 */
			cell_pattern_index = ( ( ( bat[nt_index + 1] << 8 ) | bat[nt_index] ) & 0x0FFF) << 5;

			int vram_offs = (cell_pattern_index + (v_row << 1)) & 0xffff;

			b0 = vdc_vidram[which][vram_offs + 0x00];
			b1 = vdc_vidram[which][vram_offs + 0x01];
			b2 = vdc_vidram[which][vram_offs + 0x10];
			b3 = vdc_vidram[which][vram_offs + 0x11];

			for(x=0;x<8;x++)
			{
				i0 = (b0 >> (7-x)) & 1;
				i1 = (b1 >> (7-x)) & 1;
				i2 = (b2 >> (7-x)) & 1;
				i3 = (b3 >> (7-x)) & 1;
				c = (cell_palette << 4 | i3 << 3 | i2 << 2 | i1 << 1 | i0);

				/* colour #0 always comes from palette #0 */
				if ( ! ( c & 0x0F ) )
					c &= 0x0F;

				if ( phys_x >= 0 && phys_x < vdc_width[which] )
				{
					drawn[ pixel ] = c ? 1 : 0;
					if ( c || ! external_input )
						line_buffer[ pixel ] = color_base + vce_data[c];
					pixel++;
					if ( vdc_width[which] != 512 )
					{
						while ( pixel < ( ( ( phys_x + 1 ) * 512 ) / vdc_width[which] ) )
						{
							drawn[ pixel ] = c ? 1 : 0;
							if ( c || ! external_input )
								line_buffer[ pixel ] = color_base + vce_data[c];
							pixel++;
						}
					}
				}
				phys_x += 1;
			}
		}
	}
}

void pce_interrupt()
{
	int which = 0; // only 1 on pce

	if (vce_current_bitmap_line >= 14 && vce_current_bitmap_line < 256)
	{
		draw_overscan_line(vce_current_bitmap_line);

		if (vdc_current_segment[which] == 0x02)
		{
			UINT8 drawn[684];
			UINT16 *line_buffer = vdc_tmp_draw + (vce_current_bitmap_line * 684) + 86;

			memset (drawn, 0, 684);

			vdc_yscroll[which] = (vdc_current_segment_line[which] == 0) ? vdc_data[which][BYR] : (vdc_yscroll[which] + 1);

			pce_refresh_line(0, vdc_current_segment_line[which], 0, drawn, line_buffer);

			if (vdc_data[which][CR] & CR_SB)
			{
				pce_refresh_sprites(0, vdc_current_segment_line[which], drawn, line_buffer);
			}
		}
	}
	else
	{
		draw_black_line(vce_current_bitmap_line);
	}

	vce_current_bitmap_line = (vce_current_bitmap_line + 1) % VDC_LPF;
	vdc_advance_line(0);
}

void sgx_interrupt()
{
	if (vce_current_bitmap_line >= 14 && vce_current_bitmap_line < 256)
	{
		draw_sgx_overscan_line(vce_current_bitmap_line);

		if ( vdc_current_segment[0] == STATE_VDW )
		{
			UINT8 drawn[2][512];
			UINT16 *line_buffer;
			UINT16 temp_buffer[2][512];
			int i;

			memset( drawn, 0, sizeof(drawn) );

			vdc_yscroll[0] = ( vdc_current_segment_line[0] == 0 ) ? vdc_data[0][BYR] : ( vdc_yscroll[0] + 1 );
			vdc_yscroll[1] = ( vdc_current_segment_line[1] == 0 ) ? vdc_data[1][BYR] : ( vdc_yscroll[1] + 1 );

			pce_refresh_line( 0, vdc_current_segment_line[0], 0, drawn[0], temp_buffer[0]);

			if(vdc_data[0][CR] & CR_SB)
			{
				pce_refresh_sprites(0, vdc_current_segment_line[0], drawn[0], temp_buffer[0]);
			}

			pce_refresh_line( 1, vdc_current_segment_line[1], 1, drawn[1], temp_buffer[1]);

			if ( vdc_data[1][CR] & CR_SB )
			{
				pce_refresh_sprites(1, vdc_current_segment_line[1], drawn[1], temp_buffer[1]);
			}

			line_buffer = vdc_tmp_draw + (vce_current_bitmap_line * 684) + 86;

			for( i = 0; i < 512; i++ )
			{
				int cur_prio = vpc_prio_map[i];

				if ( vpc_vdc0_enabled[cur_prio] )
				{
					if ( vpc_vdc1_enabled[cur_prio] )
					{
						switch( vpc_prio[cur_prio] )
						{
						case 0:	/* BG1 SP1 BG0 SP0 */
							if ( drawn[0][i] )
							{
								line_buffer[i] = temp_buffer[0][i];
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						case 1:	/* BG1 BG0 SP1 SP0 */
							if ( drawn[0][i] )
							{
								if ( drawn[0][i] > 1 )
								{
									line_buffer[i] = temp_buffer[0][i];
								}
								else
								{
									if ( drawn[1][i] > 1 )
									{
										line_buffer[i] = temp_buffer[1][i];
									}
									else
									{
										line_buffer[i] = temp_buffer[0][i];
									}
								}
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						case 2:
							if ( drawn[0][i] )
							{
								if ( drawn[0][i] > 1 )
								{
									if ( drawn[1][i] == 1 )
									{
										line_buffer[i] = temp_buffer[1][i];
									}
									else
									{
										line_buffer[i] = temp_buffer[0][i];
									}
								}
								else
								{
									line_buffer[i] = temp_buffer[0][i];
								}
							}
							else if ( drawn[1][i] )
							{
								line_buffer[i] = temp_buffer[1][i];
							}
							break;
						}
					}
					else
					{
						if ( drawn[0][i] )
						{
							line_buffer[i] = temp_buffer[0][i];
						}
					}
				}
				else
				{
					if ( vpc_vdc1_enabled[cur_prio] )
					{
						if ( drawn[1][i] )
						{
							line_buffer[i] = temp_buffer[1][i];
						}
					}
				}
			}
		}
	}
	else
	{
		draw_black_line(vce_current_bitmap_line);
	}

	/* bump current scanline */
	vce_current_bitmap_line = ( vce_current_bitmap_line + 1 ) % VDC_LPF;
	vdc_advance_line( 0 );
	vdc_advance_line( 1 );
}

static void vdc_do_dma(int which)
{
	int src = vdc_data[which][0x10];
	int dst = vdc_data[which][0x11];
	int len = vdc_data[which][0x12];

	int did = (vdc_data[which][0x0f] >> 3) & 1;
	int sid = (vdc_data[which][0x0f] >> 2) & 1;
	int dvc = (vdc_data[which][0x0f] >> 1) & 1;

	do {
		UINT8 l, h;

		l = vdc_vidram[which][((src * 2) + 0) & 0xffff];
		h = vdc_vidram[which][((src * 2) + 1) & 0xffff];

		if ((dst & 0x8000) == 0) {
			vdc_vidram[which][(dst * 2) + 0] = l;
			vdc_vidram[which][(dst * 2) + 1] = h;
		}

		if(sid) src = (src - 1) & 0xffff;
		else	src = (src + 1) & 0xffff;

		if(did) dst = (dst - 1) & 0xffff;
		else	dst = (dst + 1) & 0xffff;

		len = (len - 1) & 0xffff;

	} while (len != 0xffff);

	vdc_status[which] |= 0x10;
	vdc_data[which][0x10] = src;
	vdc_data[which][0x11] = dst;
	vdc_data[which][0x12] = len;

	if (dvc)
	{
		h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
	}
}

void vdc_write(int which, UINT8 offset, UINT8 data)
{
	switch (offset & 3)
	{
		case 0x00:
			vdc_register[which] = data & 0x1f;
		break;

		case 0x02:
		{
			vdc_data[which][vdc_register[which]] = (vdc_data[which][vdc_register[which]] & 0xff00) | data;

			switch (vdc_register[which])
			{
				case VxR:{
					vdc_latch[which] = data;
				}
				break;

				case BYR: {
					vdc_yscroll[which]=vdc_data[which][BYR];
				}
				break;

				case HDR: {
					vdc_width[which] = ((data & 0x003F) + 1) << 3;
				}
				break;

				case VDW: {
					vdc_height[which] &= 0xFF00;
					vdc_height[which] |= (data & 0xFF);
					vdc_height[which] &= 0x01FF;
				}
				break;

				case LENR:
				case SOUR:
				case DESR:
				break;
			}
		}
		break;

		case 0x03:
		{
			vdc_data[which][vdc_register[which]] = (vdc_data[which][vdc_register[which]] & 0x00ff) | (data << 8);

			switch (vdc_register[which])
			{
				case VxR:
				{
					INT32 voff = vdc_data[which][MAWR] * 2;
					if ((voff & 0x10000) == 0) {
						vdc_vidram[which][voff + 0] = vdc_latch[which];
						vdc_vidram[which][voff + 1] = data;
					}
					vdc_data[which][MAWR] += vdc_inc[which];
				}
				break;

				case CR:
				{
					static const unsigned char inctab[] = {1, 32, 64, 128};
					vdc_inc[which] = inctab[(data >> 3) & 3];
				}
				break;

				case VDW:
				{
					vdc_height[which] &= 0x00FF;
					vdc_height[which] |= (data << 8);
					vdc_height[which] &= 0x01FF;
				}
				break;

				case DVSSR:
					vdc_dvssr_write[which] = 1;
					break;

				case BYR:
					vdc_yscroll[which]=vdc_data[which][BYR];
					break;

				case LENR:
					vdc_do_dma( which );
					break;

				case SOUR:
				case DESR:
				break;
			}
		}
		break;
	}
}

UINT8 vdc_read(int which, UINT8 offset)
{
	switch(offset & 3)
	{
		case 0x00: {
			UINT8 ret = vdc_status[which];
			vdc_status[which] &= ~0x3f;
			h6280SetIRQLine(0, H6280_IRQSTATUS_NONE);
			return ret;
		}

		case 0x02: {
			INT32 voff = (vdc_data[which][1] * 2 + 0) & 0xffff;
			return vdc_vidram[which][voff];
		}

		case 0x03: {
			INT32 voff = (vdc_data[which][1] * 2 + 1) & 0xffff;
			if (vdc_register[which] == 0x02) vdc_data[which][1] += vdc_inc[which];
			return vdc_vidram[which][voff];
		}
	}

	return 0;
}

void sgx_vdc_write(UINT8 offset, UINT8 data)
{
	if (vpc_vdc_select)
	{
		vdc_write( 1, offset, data );
	}
	else
	{
		vdc_write( 0, offset, data );
	}
}

UINT8 sgx_vdc_read(UINT8 offset)
{
	return (vpc_vdc_select) ? vdc_read( 1, offset ) : vdc_read( 0, offset );
}

void vdc_reset()
{
	memset (vdc_register,			0, 2);
	memset (vdc_data,			0, 2 * 32 * sizeof(UINT16));
	memset (vdc_latch,			0, 2);
	memset (vdc_yscroll,			0, 2 * sizeof(UINT16));
	memset (vdc_width,			0, 2 * sizeof(UINT16));
	memset (vdc_height,			0, 2 * sizeof(UINT16));
	memset (vdc_inc,			0, 2);
	memset (vdc_dvssr_write,		0, 2);
	memset (vdc_status,			0, 2);
	memset (vdc_sprite_ram,			0, 2 * 0x100 * sizeof(UINT16));
	memset (vdc_vblank_triggered,		0, 2 * sizeof(INT32));
	memset (vdc_current_segment,		0, 2 * sizeof(UINT16));
	memset (vdc_current_segment_line,	0, 2 * sizeof(UINT16));
	memset (vdc_raster_count,		0, 2 * sizeof(INT32));
	memset (vdc_curline,			0, 2 * sizeof(INT32));
	memset (vdc_satb_countdown,		0, 2 * sizeof(INT32));

	vdc_inc[0] = 1;
	vdc_inc[1] = 1;
}

void vdc_get_dimensions(int which, INT32 *x, INT32 *y)
{
	*x = vdc_width[which] * 2;
	*y = vdc_height[which];
}

INT32 vdc_scan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin =  0x029702;
	}

	if (nAction & ACB_DRIVER_DATA) {
		for (INT32 i = 0; i < 2; i++) {
			SCAN_VAR(vdc_register[i]);
			SCAN_VAR(vdc_data[i]);
			SCAN_VAR(vdc_latch[i]);
			SCAN_VAR(vdc_yscroll[i]);
			SCAN_VAR(vdc_width[i]);
			SCAN_VAR(vdc_height[i]);
			SCAN_VAR(vdc_inc[i]);
			SCAN_VAR(vdc_dvssr_write[i]);
			SCAN_VAR(vdc_status[i]);
			SCAN_VAR(vdc_sprite_ram[i]);
			SCAN_VAR(vdc_vblank_triggered[i]);
			SCAN_VAR(vdc_current_segment[i]);
			SCAN_VAR(vdc_current_segment_line[i]);
			SCAN_VAR(vdc_raster_count[i]);
			SCAN_VAR(vdc_curline[i]);
			SCAN_VAR(vdc_satb_countdown[i]);
		}

		ba.Data		= vdc_sprite_ram;
		ba.nLen		= 2 * 0x100 * sizeof(UINT16);
		ba.nAddress	= 0;
		ba.szName	= "VDC Sprite RAM";
		BurnAcb(&ba);

		ba.Data		= vdc_data;
		ba.nLen		= 2 * 0x20 * sizeof(UINT16);
		ba.nAddress	= 0;
		ba.szName	= "VDC DATA";
		BurnAcb(&ba);

		SCAN_VAR(vce_address);
		SCAN_VAR(vce_control);
		SCAN_VAR(vce_current_bitmap_line);

		SCAN_VAR(vpc_window1);
		SCAN_VAR(vpc_window2);
		SCAN_VAR(vpc_vdc_select);
		SCAN_VAR(vpc_priority);

		for (INT32 i = 0; i < 4; i++) {
			SCAN_VAR(vpc_prio[i]);
			SCAN_VAR(vpc_vdc0_enabled[i]);
			SCAN_VAR(vpc_vdc1_enabled[i]);
		}

		ba.Data		= vpc_prio_map;
		ba.nLen		= 0x0000200;
		ba.nAddress	= 0;
		ba.szName	= "VPC Priority Map";
		BurnAcb(&ba);
	}

	return 0;
}
