/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators
// Project started on December 29th, 2001
//
// Authors:
// Dave2001, original author, founded the project in 2001, left it in 2002
// Gugaman, joined the project in 2002, left it in 2002
// Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
// Hiroshi 'KoolSmoky' Morii, joined the project in 2007
//
//****************************************************************
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
//****************************************************************

static void mod_tex_inter_color_using_factor_CI (wxUint32 color, wxUint32 factor)
{
	float percent = factor / 255.0f;
	float percent_i = 1 - percent;
	wxUint8 cr, cg, cb;
	wxUint16 col;
	wxUint8 a, r, g, b;

	cr = (wxUint8)((color >> 24) & 0xFF);
	cg = (wxUint8)((color >> 16) & 0xFF);
	cb = (wxUint8)((color >> 8) & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		r = (wxUint8)(min(255, percent_i * r + percent * cr));
		g = (wxUint8)(min(255, percent_i * g + percent * cg));
		b = (wxUint8)(min(255, percent_i * b + percent * cb));
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void mod_tex_inter_col_using_col1_CI (wxUint32 color0, wxUint32 color1)
{
	wxUint8 cr, cg, cb;
	wxUint16 col;
	wxUint8 a, r, g, b;

	float percent_r = ((color1 >> 24) & 0xFF) / 255.0f;
	float percent_g = ((color1 >> 16) & 0xFF) / 255.0f;
	float percent_b = ((color1 >> 8)  & 0xFF) / 255.0f;
	float percent_r_i = 1.0f - percent_r;
	float percent_g_i = 1.0f - percent_g;
	float percent_b_i = 1.0f - percent_b;

	cr = (wxUint8)((color0 >> 24) & 0xFF);
	cg = (wxUint8)((color0 >> 16) & 0xFF);
	cb = (wxUint8)((color0 >> 8)  & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		r = (wxUint8)(min(255, percent_r_i * r + percent_r * cr));
		g = (wxUint8)(min(255, percent_g_i * g + percent_g * cg));
		b = (wxUint8)(min(255, percent_b_i * b + percent_b * cb));
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void mod_full_color_sub_tex_CI (wxUint32 color)
{
	wxUint8 cr, cg, cb, ca;
	wxUint16 col;
	wxUint8 a, r, g, b;

	cr = (wxUint8)((color >> 24) & 0xFF);
	cg = (wxUint8)((color >> 16) & 0xFF);
	cb = (wxUint8)((color >> 8) & 0xFF);
	ca = (wxUint8)(color & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		a = max(0, ca - a);
		r = max(0, cr - r);
		g = max(0, cg - g);
		b = max(0, cb - b);
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void mod_col_inter_col1_using_tex_CI (wxUint32 color0, wxUint32 color1)
{
	wxUint32 cr0, cg0, cb0, cr1, cg1, cb1;
	wxUint16 col;
	wxUint8 a, r, g, b;
	float percent_r, percent_g, percent_b;

	cr0 = (wxUint8)((color0 >> 24) & 0xFF);
	cg0 = (wxUint8)((color0 >> 16) & 0xFF);
	cb0 = (wxUint8)((color0 >> 8)  & 0xFF);
	cr1 = (wxUint8)((color1 >> 24) & 0xFF);
	cg1 = (wxUint8)((color1 >> 16) & 0xFF);
	cb1 = (wxUint8)((color1 >> 8)  & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		percent_r = ((col&0xF800) >> 11) / 31.0f;
		percent_g = ((col&0x07C0) >> 6) / 31.0f;
		percent_b = ((col&0x003E) >> 1) / 31.0f;
		r = (wxUint8)(min((1.0f-percent_r) * cr0 + percent_r * cr1, 255));
		g = (wxUint8)(min((1.0f-percent_g) * cg0 + percent_g * cg1, 255));
		b = (wxUint8)(min((1.0f-percent_b) * cb0 + percent_b * cb1, 255));
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}



static void mod_tex_sub_col_mul_fac_add_tex_CI (wxUint32 color, wxUint32 factor)
{
	float percent = factor / 255.0f;
	wxUint8 cr, cg, cb, a;
	wxUint16 col;
	float r, g, b;

	cr = (wxUint8)((color >> 24) & 0xFF);
	cg = (wxUint8)((color >> 16) & 0xFF);
	cb = (wxUint8)((color >> 8) & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		r = (r - cr) * percent + r;
		if (r > 255.0f) r = 255.0f;
		if (r < 0.0f) r = 0.0f;
		g = (g - cg) * percent + g;
		if (g > 255.0f) g = 255.0f;
		if (g < 0.0f) g = 0.0f;
		b = (b - cb) * percent + b;
		if (b > 255.0f) g = 255.0f;
		if (b < 0.0f) b = 0.0f;
        rdp.pal_8[i] = (wxUint16)(((wxUint16)((wxUint8)(r) >> 3) << 11) |
		          ((wxUint16)((wxUint8)(g) >> 3) << 6) |
		          ((wxUint16)((wxUint8)(b) >> 3) << 1) |
				  (wxUint16)(a) );
	}
}

static void mod_tex_scale_col_add_col_CI (wxUint32 color0, wxUint32 color1)
{
	wxUint8 cr, cg, cb;
	wxUint16 col;
	wxUint8 a, r, g, b;

	float percent_r = ((color0 >> 24) & 0xFF) / 255.0f;
	float percent_g = ((color0 >> 16) & 0xFF) / 255.0f;
	float percent_b = ((color0 >> 8)  & 0xFF) / 255.0f;
	cr = (wxUint8)((color1 >> 24) & 0xFF);
	cg = (wxUint8)((color1 >> 16) & 0xFF);
	cb = (wxUint8)((color1 >> 8)  & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		r = (wxUint8)(min(255, percent_r * r + cr));
		g = (wxUint8)(min(255, percent_g * g + cg));
		b = (wxUint8)(min(255, percent_b * b + cb));
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void mod_tex_add_col_CI (wxUint32 color)
{
	wxUint8 cr, cg, cb;
	wxUint16 col;
	wxUint8 a, r, g, b;

	cr = (wxUint8)((color >> 24) & 0xFF);
	cg = (wxUint8)((color >> 16) & 0xFF);
	cb = (wxUint8)((color >> 8) & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		r = min(cr + r, 255);
		g = min(cg + g, 255);
		b = min(cb + b, 255);
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void mod_tex_sub_col_CI (wxUint32 color)
{
	wxUint8 cr, cg, cb;
	wxUint16 col;
	wxUint8 a, r, g, b;

	cr = (wxUint8)((color >> 24) & 0xFF);
	cg = (wxUint8)((color >> 16) & 0xFF);
	cb = (wxUint8)((color >> 8) & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		r = max(r - cr, 0);
		g = max(g - cg, 0);
		b = max(b - cb, 0);
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void mod_tex_sub_col_mul_fac_CI (wxUint32 color, wxUint32 factor)
{
	float percent = factor / 255.0f;
	wxUint8 cr, cg, cb;
	wxUint16 col;
	wxUint8 a;
	float r, g, b;

	cr = (wxUint8)((color >> 24) & 0xFF);
	cg = (wxUint8)((color >> 16) & 0xFF);
	cb = (wxUint8)((color >> 8) & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);
		r = (float)((col&0xF800) >> 11) / 31.0f * 255.0f;
        g = (float)((col&0x07C0) >> 6) / 31.0f * 255.0f;
        b = (float)((col&0x003E) >> 1) / 31.0f * 255.0f;
		r = (r - cr) * percent;
		if (r > 255.0f) r = 255.0f;
		if (r < 0.0f) r = 0.0f;
		g = (g - cg) * percent;
		if (g > 255.0f) g = 255.0f;
		if (g < 0.0f) g = 0.0f;
		b = (b - cb) * percent;
		if (b > 255.0f) g = 255.0f;
		if (b < 0.0f) b = 0.0f;

        rdp.pal_8[i] = (wxUint16)(((wxUint16)((wxUint8)(r) >> 3) << 11) |
		          ((wxUint16)((wxUint8)(g) >> 3) << 6) |
		          ((wxUint16)((wxUint8)(b) >> 3) << 1) |
				  (wxUint16)(a) );
	}
}

static void mod_col_inter_tex_using_col1_CI (wxUint32 color0, wxUint32 color1)
{
	wxUint8 cr, cg, cb;
	wxUint16 col;
	wxUint8 a, r, g, b;

	float percent_r = ((color1 >> 24) & 0xFF) / 255.0f;
	float percent_g = ((color1 >> 16) & 0xFF) / 255.0f;
	float percent_b = ((color1 >> 8)  & 0xFF) / 255.0f;
	float percent_r_i = 1.0f - percent_r;
	float percent_g_i = 1.0f - percent_g;
	float percent_b_i = 1.0f - percent_b;

	cr = (wxUint8)((color0 >> 24) & 0xFF);
	cg = (wxUint8)((color0 >> 16) & 0xFF);
	cb = (wxUint8)((color0 >> 8)  & 0xFF);

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) / 31.0f * 255.0f);
        g = (wxUint8)((float)((col&0x07C0) >> 6) / 31.0f * 255.0f);
        b = (wxUint8)((float)((col&0x003E) >> 1) / 31.0f * 255.0f);
		r = (wxUint8)(min(255, percent_r * r + percent_r_i * cr));
		g = (wxUint8)(min(255, percent_g * g + percent_g_i * cg));
		b = (wxUint8)(min(255, percent_b * b + percent_b_i * cb));
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void mod_tex_inter_col_using_texa_CI (wxUint32 color)
{
	wxUint8 a, r, g, b;

	r = (wxUint8)((float)((color >> 24) & 0xFF) / 255.0f * 31.0f);
    g = (wxUint8)((float)((color >> 16) & 0xFF) / 255.0f * 31.0f);
    b = (wxUint8)((float)((color >> 8)  & 0xFF) / 255.0f * 31.0f);
    a = (color&0xFF) ? 1 : 0;
    wxUint16 col16 = (wxUint16)((r<<11)|(g<<6)|(b<<1)|a);

	for (int i=0; i<256; i++)
	{
	    if (rdp.pal_8[i]&1)
          rdp.pal_8[i] = col16;
	}
}

static void mod_tex_mul_col_CI (wxUint32 color)
{
	wxUint8 a, r, g, b;
	wxUint16 col;
	float cr, cg, cb;

	cr = (float)((color >> 24) & 0xFF) / 255.0f;
    cg = (float)((color >> 16) & 0xFF) / 255.0f;
    cb = (float)((color >> 8)  & 0xFF) / 255.0f;

	for (int i=0; i<256; i++)
	{
		col = rdp.pal_8[i];
		a = (wxUint8)(col&0x0001);;
		r = (wxUint8)((float)((col&0xF800) >> 11) * cr);
        g = (wxUint8)((float)((col&0x07C0) >> 6) * cg);
        b = (wxUint8)((float)((col&0x003E) >> 1) * cb);
        rdp.pal_8[i] = (wxUint16)(((wxUint16)(r >> 3) << 11) |
		          ((wxUint16)(g >> 3) << 6) |
		          ((wxUint16)(b >> 3) << 1) |
				  ((wxUint16)(a ) << 0));
	}
}

static void ModifyPalette(wxUint32 mod, wxUint32 modcolor, wxUint32 modcolor1, wxUint32 modfactor)
{
		switch (mod)
		{
		case TMOD_TEX_INTER_COLOR_USING_FACTOR:
			mod_tex_inter_color_using_factor_CI (modcolor, modfactor);
			break;
		case TMOD_TEX_INTER_COL_USING_COL1:
			mod_tex_inter_col_using_col1_CI (modcolor, modcolor1);
			break;
		case TMOD_FULL_COLOR_SUB_TEX:
			mod_full_color_sub_tex_CI (modcolor);
			break;
		case TMOD_COL_INTER_COL1_USING_TEX:
			mod_col_inter_col1_using_tex_CI (modcolor, modcolor1);
			break;
		case TMOD_TEX_SUB_COL_MUL_FAC_ADD_TEX:
			mod_tex_sub_col_mul_fac_add_tex_CI (modcolor, modfactor);
			break;
		case TMOD_TEX_SCALE_COL_ADD_COL:
			mod_tex_scale_col_add_col_CI (modcolor, modcolor1);
			break;
		case TMOD_TEX_ADD_COL:
			mod_tex_add_col_CI (modcolor);
			break;
		case TMOD_TEX_SUB_COL:
			mod_tex_sub_col_CI (modcolor);
			break;
		case TMOD_TEX_SUB_COL_MUL_FAC:
			mod_tex_sub_col_mul_fac_CI (modcolor, modfactor);
			break;
		case TMOD_COL_INTER_TEX_USING_COL1:
			mod_col_inter_tex_using_col1_CI (modcolor, modcolor1);
			break;
        case TMOD_TEX_INTER_COL_USING_TEXA:
            mod_tex_inter_col_using_texa_CI (modcolor);
            break;
        case TMOD_TEX_MUL_COL:
            mod_tex_mul_col_CI (modcolor);
            break;
		default:
			;
	   }
}
