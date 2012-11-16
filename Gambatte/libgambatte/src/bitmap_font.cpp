/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*
  The following font bitmaps (static const unsigned char *_bits[]), only used
  as data and included in this source file for convenience, are derived from
  the Bitstream Vera Sans font, which is distributed under the following
  copyright:

  Copyright (c) 2003 by Bitstream, Inc. All Rights Reserved. Bitstream Vera
  is a trademark of Bitstream, Inc.

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of the fonts accompanying this license ("Fonts") and associated
  documentation files (the "Font Software"), to reproduce and distribute the
  Font Software, including without limitation the rights to use, copy, merge,
  publish, distribute, and/or sell copies of the Font Software, and to permit
  persons to whom the Font Software is furnished to do so, subject to the
  following conditions:

  The above copyright and trademark notices and this permission notice shall
  be included in all copies of one or more of the Font Software typefaces.

  The Font Software may be modified, altered, or added to, and in particular
  the designs of glyphs or characters in the Fonts may be modified and
  additional glyphs or characters may be added to the Fonts, only if the fonts
  are renamed to names not containing either the words "Bitstream" or the word
  "Vera".

  This License becomes null and void to the extent applicable to Fonts or Font
  Software that has been modified and is distributed under the "Bitstream Vera"
  names.

  The Font Software may be sold as part of a larger software package but no
  copy of one or more of the Font Software typefaces may be sold by itself. 

  THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
  COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL BITSTREAM
  OR THE GNOME FOUNDATION BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
  OTHER DEALINGS IN THE FONT SOFTWARE.

  Except as contained in this notice, the names of Gnome, the Gnome
  Foundation, and Bitstream Inc., shall not be used in advertising or
  otherwise to promote the sale, use or other dealings in this Font Software
  without prior written authorization from the Gnome Foundation or
  Bitstream Inc., respectively. For further information, contact: fonts at
  gnome dot org.
*/

#include "bitmap_font.h"

static const unsigned char n0_bits[] = { 0x68,
   0x00, 0x1c, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c };

static const unsigned char n1_bits[] = { 0x68,
   0x00, 0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3e };

static const unsigned char n2_bits[] = { 0x68,
   0x00, 0x1c, 0x22, 0x20, 0x10, 0x08, 0x04, 0x3e };

static const unsigned char n3_bits[] = { 0x68,
   0x00, 0x1c, 0x22, 0x20, 0x1c, 0x20, 0x22, 0x1c };

static const unsigned char n4_bits[] = { 0x68,
   0x00, 0x18, 0x18, 0x14, 0x12, 0x3e, 0x10, 0x10 };

static const unsigned char n5_bits[] = { 0x68,
   0x00, 0x1e, 0x02, 0x1e, 0x20, 0x20, 0x20, 0x1e };

static const unsigned char n6_bits[] = { 0x68,
   0x00, 0x3c, 0x06, 0x02, 0x1e, 0x22, 0x22, 0x1c };

static const unsigned char n7_bits[] = { 0x68,
   0x00, 0x3e, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04 };

static const unsigned char n8_bits[] = { 0x68,
   0x00, 0x1c, 0x22, 0x22, 0x1c, 0x22, 0x22, 0x1c };

static const unsigned char n9_bits[] = { 0x68,
   0x00, 0x1c, 0x22, 0x22, 0x3c, 0x20, 0x30, 0x1e };

static const unsigned char A_bits[] = { 0x78,
   0x00, 0x08, 0x14, 0x14, 0x22, 0x3e, 0x22, 0x41 };

static const unsigned char a_bits[] = { 0x68,
   0x00, 0x00, 0x00, 0x1c, 0x20, 0x3c, 0x22, 0x3e };

static const unsigned char B_bits[] = { 0x78,
   0x00, 0x1e, 0x22, 0x22, 0x1e, 0x22, 0x22, 0x1e };

static const unsigned char b_bits[] = { 0x68,
   0x02, 0x02, 0x02, 0x1e, 0x22, 0x22, 0x22, 0x1e };

static const unsigned char C_bits[] = { 0x88,
   0x00, 0x38, 0x44, 0x02, 0x02, 0x02, 0x44, 0x38 };

static const unsigned char c_bits[] = { 0x58,
   0x00, 0x00, 0x00, 0x1c, 0x02, 0x02, 0x02, 0x1c };

static const unsigned char D_bits[] = { 0x88,
   0x00, 0x3e, 0x62, 0x42, 0x42, 0x42, 0x62, 0x3e };

static const unsigned char d_bits[] = { 0x68,
   0x20, 0x20, 0x20, 0x3c, 0x22, 0x22, 0x22, 0x3c };

static const unsigned char E_bits[] = { 0x78,
   0x00, 0x3e, 0x02, 0x02, 0x3e, 0x02, 0x02, 0x3e };

static const unsigned char e_bits[] = { 0x68,
   0x00, 0x00, 0x00, 0x1c, 0x22, 0x3e, 0x02, 0x3c };

static const unsigned char F_bits[] = { 0x68,
   0x00, 0x1e, 0x02, 0x02, 0x1e, 0x02, 0x02, 0x02 };

static const unsigned char f_bits[] = { 0x48,
   0x0e, 0x02, 0x02, 0x07, 0x02, 0x02, 0x02, 0x02 };

static const unsigned char G_bits[] = { 0x88,
   0x00, 0x3c, 0x46, 0x02, 0x72, 0x42, 0x46, 0x3c };

static const unsigned char g_bits[] = { 0x6a,
   0x00, 0x00, 0x00, 0x3c, 0x22, 0x22, 0x22, 0x3c, 0x20, 0x1c };

static const unsigned char H_bits[] = { 0x88,
   0x00, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42 };

static const unsigned char h_bits[] = { 0x68,
   0x02, 0x02, 0x02, 0x1e, 0x22, 0x22, 0x22, 0x22 };

static const unsigned char I_bits[] = { 0x38,
   0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 };

static const unsigned char i_bits[] = { 0x28,
   0x02, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02 };

static const unsigned char J_bits[] = { 0x4a,
   0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x03 };

static const unsigned char j_bits[] = { 0x2a,
   0x02, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03 };

static const unsigned char K_bits[] = { 0x78,
   0x00, 0x22, 0x12, 0x0a, 0x06, 0x0a, 0x12, 0x22 };

static const unsigned char k_bits[] = { 0x58,
   0x02, 0x02, 0x02, 0x12, 0x0a, 0x06, 0x0a, 0x12 };

static const unsigned char L_bits[] = { 0x68,
   0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x3e };

static const unsigned char l_bits[] = { 0x28,
   0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 };

static const unsigned char M_bits[] = { 0x98,
   0x00, 0x00, 0x82, 0x00, 0xc6, 0x00, 0xc6, 0x00, 0xaa, 0x00, 0xaa, 0x00,
   0x92, 0x00, 0x82, 0x00 };

static const unsigned char m_bits[] = { 0xa8,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0x01, 0x22, 0x02, 0x22, 0x02,
   0x22, 0x02, 0x22, 0x02 };

static const unsigned char N_bits[] = { 0x88,
   0x00, 0x42, 0x46, 0x4a, 0x4a, 0x52, 0x62, 0x42 };

static const unsigned char n_bits[] = { 0x68,
   0x00, 0x00, 0x00, 0x1e, 0x22, 0x22, 0x22, 0x22 };

static const unsigned char O_bits[] = { 0x88,
   0x00, 0x3c, 0x66, 0x42, 0x42, 0x42, 0x66, 0x3c };

static const unsigned char o_bits[] = { 0x68,
   0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x1c };

static const unsigned char P_bits[] = { 0x78,
   0x00, 0x1e, 0x22, 0x22, 0x1e, 0x02, 0x02, 0x02 };

static const unsigned char p_bits[] = { 0x6a,
   0x00, 0x00, 0x00, 0x1e, 0x22, 0x22, 0x22, 0x1e, 0x02, 0x02 };

static const unsigned char Q_bits[] = { 0x89,
   0x00, 0x3c, 0x66, 0x42, 0x42, 0x42, 0x26, 0x1c, 0x20 };

static const unsigned char q_bits[] = { 0x6a,
   0x00, 0x00, 0x00, 0x3c, 0x22, 0x22, 0x22, 0x3c, 0x20, 0x20 };

static const unsigned char R_bits[] = { 0x78,
   0x00, 0x1e, 0x22, 0x22, 0x1e, 0x12, 0x22, 0x42 };

static const unsigned char r_bits[] = { 0x48,
   0x00, 0x00, 0x00, 0x0e, 0x02, 0x02, 0x02, 0x02 };

static const unsigned char S_bits[] = { 0x78,
   0x00, 0x1c, 0x22, 0x02, 0x1c, 0x20, 0x22, 0x1c };

static const unsigned char s_bits[] = { 0x58,
   0x00, 0x00, 0x00, 0x1e, 0x02, 0x1c, 0x10, 0x1e };

static const unsigned char T_bits[] = { 0x58,
   0x00, 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };

static const unsigned char t_bits[] = { 0x48,
   0x00, 0x02, 0x02, 0x0f, 0x02, 0x02, 0x02, 0x0e };

static const unsigned char U_bits[] = { 0x88,
   0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c };

static const unsigned char u_bits[] = { 0x68,
   0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x3c };

static const unsigned char V_bits[] = { 0x78,
   0x00, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08 };

static const unsigned char v_bits[] = { 0x68,
   0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x14, 0x08 };

static const unsigned char W_bits[] = { 0x98,
   0x00, 0x00, 0x11, 0x01, 0x11, 0x01, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00,
   0x44, 0x00, 0x44, 0x00 };

static const unsigned char w_bits[] = { 0x88,
   0x00, 0x00, 0x00, 0x92, 0xaa, 0xaa, 0x44, 0x44 };

static const unsigned char X_bits[] = { 0x68,
   0x00, 0x21, 0x12, 0x0c, 0x0c, 0x0c, 0x12, 0x21 };

static const unsigned char x_bits[] = { 0x68,
   0x00, 0x00, 0x00, 0x22, 0x14, 0x08, 0x14, 0x22 };

static const unsigned char Y_bits[] = { 0x78,
   0x00, 0x41, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08 };

static const unsigned char y_bits[] = { 0x6a,
   0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x06 };

static const unsigned char Z_bits[] = { 0x68,
   0x00, 0x3f, 0x10, 0x08, 0x0c, 0x04, 0x02, 0x3f };

static const unsigned char z_bits[] = { 0x58,
   0x00, 0x00, 0x00, 0x1e, 0x10, 0x08, 0x04, 0x1e };

static const unsigned char SPC_bits[] = { 0x38,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

namespace bitmapfont {
const unsigned char *const font[] = {
	0,
	n0_bits, n1_bits, n2_bits, n3_bits, n4_bits, n5_bits, n6_bits, n7_bits, n8_bits, n9_bits,
	A_bits, B_bits, C_bits, D_bits, E_bits, F_bits, G_bits, H_bits, I_bits, J_bits, K_bits, L_bits, M_bits,
	N_bits, O_bits, P_bits, Q_bits, R_bits, S_bits, T_bits, U_bits, V_bits, W_bits, X_bits, Y_bits, Z_bits,
	a_bits, b_bits, c_bits, d_bits, e_bits, f_bits, g_bits, h_bits, i_bits, j_bits, k_bits, l_bits, m_bits,
	n_bits, o_bits, p_bits, q_bits, r_bits, s_bits, t_bits, u_bits, v_bits, w_bits, x_bits, y_bits, z_bits,
	SPC_bits
};

unsigned getWidth(const char *chars) {
	unsigned w = 0;

	while (const int character = *chars++) {
		w += *font[character] >> 4;
	}
	
	return w;
}

namespace {
class Rgb32Fill {
	const unsigned long color;
	
public:
	explicit Rgb32Fill(unsigned long color) : color(color) {}
	
	void operator()(gambatte::uint_least32_t *dest, unsigned /*pitch*/) const {
		*dest = color;
	}
};
}

void print(gambatte::uint_least32_t *dest, const unsigned pitch, const unsigned long color, const char *chars) {
	print(dest, pitch, Rgb32Fill(color), chars);
}

static void reverse(char *first, char *last) {
	while (first < last) {
		const int tmp = *first;
		
		*first = *last;
		*last = tmp;
		
		++first;
		--last;
	}
}

void utoa(unsigned u, char *a) {
	char *aa = a;
	
	while (u > 9) {
		const unsigned div = u / 10;
		const unsigned rem = u % 10;
		
		u = div;
		*aa++ = rem + N0;
	}
	
	*aa = u + N0;
	
	reverse(a, aa);
}
}
