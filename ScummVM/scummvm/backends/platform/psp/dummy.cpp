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

 //#include "common/scummsys.h"
 #include <time.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <png.h>
 #include <sys/socket.h>

//void userWriteFn(png_structp png_ptr, png_bytep data, png_size_t length) {
//}

//void userFlushFn(png_structp png_ptr) {
//}

 // Dummy functions are pulled in so that we don't need to build the plugins with certain libs

 int dummyFunc() {

	// For Broken Sword 2.5
 	volatile int i;
	i = clock();
	rename("dummyA", "dummyB");

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_set_write_fn(png_ptr, NULL, NULL, NULL);
	png_infop info_ptr;
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// For lua's usage of libc: very heavy usage so it pulls in sockets?
	setsockopt(0, 0, 0, NULL, 0);
	getsockopt(0, 0, 0, NULL, NULL);

	return i;
}
