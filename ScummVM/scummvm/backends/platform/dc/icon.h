/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DC_ICON_H
#define DC_ICON_H

class Icon
{
 private:
  unsigned char bitmap[32*32/2];
  unsigned int palette[16];
  void *texture;

  int find_unused_pixel(const unsigned char *);
  bool load_image1(const void *data, int len, int offs);
  bool load_image2(const void *data, int len);

 public:
  bool load(const void *data, int len, int offs = 0);
  bool load(const char *filename);
  void create_texture();
  void setPalette(int pal);
  void draw(float x1, float y1, float x2, float y2, int pal,
	    unsigned argb = 0xffffffff);
  void create_vmicon(void *buffer);
};

#endif /* DC_ICON_H */
