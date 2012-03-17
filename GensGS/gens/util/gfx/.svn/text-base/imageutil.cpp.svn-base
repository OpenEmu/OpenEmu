#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imageutil.hpp"
#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/byteswap.h"

#include "macros/file_m.h"

#ifdef GENS_PNG
#include <png.h>
#endif /* GENS_PNG */

// Pixel masks
static const uint16_t MASK_RED_15	= 0x7C00;
static const uint16_t MASK_GREEN_15	= 0x03E0;
static const uint16_t MASK_BLUE_15	= 0x001F;

static const uint16_t MASK_RED_16	= 0xF800;
static const uint16_t MASK_GREEN_16	= 0x07E0;
static const uint16_t MASK_BLUE_16	= 0x001F;

static const uint32_t MASK_RED_32	= 0xFF0000;
static const uint32_t MASK_GREEN_32	= 0x00FF00;
static const uint32_t MASK_BLUE_32	= 0x0000FF;

// Pixel shifts
static const uint8_t SHIFT_RED_15	= 7;
static const uint8_t SHIFT_GREEN_15	= 2;
static const uint8_t SHIFT_BLUE_15	= 3;

static const uint8_t SHIFT_RED_16	= 8;
static const uint8_t SHIFT_GREEN_16	= 3;
static const uint8_t SHIFT_BLUE_16	= 3;

static const uint8_t SHIFT_RED_32	= 16;
static const uint8_t SHIFT_GREEN_32	= 8;
static const uint8_t SHIFT_BLUE_32	= 0;


/**
 * T_writeBMP_rows(): Write BMP rows.
 * @param screen Pointer to the screen buffer.
 * @param bmpOut Buffer to write BMP data to.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param maskR Red mask.
 * @param maskG Green mask.
 * @param maskB Blue mask.
 * @param shiftR Red shift. (Right)
 * @param shiftG Green shift. (Right)
 * @param shiftB Blue shift. (Left)
 */
template<typename pixel>
static inline void T_writeBMP_rows(const pixel *screen, uint8_t *bmpOut,
				   const int width, const int height, const int pitch,
				   const pixel maskR, const pixel maskG, const pixel maskB,
				   const uint8_t shiftR, const uint8_t shiftG, const uint8_t shiftB)
{
	// Bitmaps are stored upside-down.
	for (int y = height - 1; y >= 0; y--)
	{
		const pixel *curScreen = &screen[y * pitch];
		for (int x = 0; x < width; x++)
		{
			pixel MD_Color = *curScreen++;
			*bmpOut++ = (uint8_t)((MD_Color & maskB) << shiftB);
			*bmpOut++ = (uint8_t)((MD_Color & maskG) >> shiftG);
			*bmpOut++ = (uint8_t)((MD_Color & maskR) >> shiftR);
		}
	}
}

/**
 * writeBMP(): Write a BMP image.
 * @param fImg File handle to save the image to.
 * @param w Width of the image.
 * @param h Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param screen Pointer to screen buffer.
 * @param bpp Bits per pixel.
 * @return 1 on success; 0 on error.
 */
int ImageUtil::writeBMP(FILE *fImg, const int w, const int h, const int pitch,
			const void *screen, const int bpp)
{
	if (!fImg || !screen || (w <= 0 || h <= 0 || pitch <= 0))
		return 0;
	
	unsigned char *bmpData = NULL;
	
	// Calculate the size of the bitmap image.
	int bmpSize = (w * h * 3);
	bmpData = static_cast<unsigned char*>(malloc(bmpSize + 54));
	if (!bmpData)
	{
		// Could not allocate enough memory.
		fprintf(stderr, "writeBMP): Could not allocate enough memory for the bitmap data.\n");
		return 0;
	}
	
	// Build the bitmap image.
	
	// Bitmap header.
	bmpData[0] = 'B';
	bmpData[1] = 'M';
	
	cpu_to_le32_ucptr(&bmpData[2], bmpSize); // Size of the bitmap.
	cpu_to_le16_ucptr(&bmpData[6], 0); // Reserved.
	cpu_to_le16_ucptr(&bmpData[8], 0); // Reserved.
	cpu_to_le32_ucptr(&bmpData[10], 54); // Bitmap is located 54 bytes from the start of the file.
	cpu_to_le32_ucptr(&bmpData[14], 40); // Size of the bitmap header, in bytes. (lol win32)
	cpu_to_le32_ucptr(&bmpData[18], w); // Width (pixels)
	cpu_to_le32_ucptr(&bmpData[22], h); // Height (pixels)
	cpu_to_le16_ucptr(&bmpData[26], 1); // Number of planes. (always 1)
	cpu_to_le16_ucptr(&bmpData[28], 24); // bpp (24-bit is the most common.)
	cpu_to_le32_ucptr(&bmpData[30], 0); // Compression. (0 == no compression)
	cpu_to_le32_ucptr(&bmpData[34], bmpSize); // Size of the bitmap data, in bytes.
	cpu_to_le32_ucptr(&bmpData[38], 0x0EC4); // Pixels per meter, X
	cpu_to_le32_ucptr(&bmpData[39], 0x0EC4); // Pixels per meter, Y
	cpu_to_le32_ucptr(&bmpData[46], 0); // Colors used (0 on non-paletted bitmaps)
	cpu_to_le32_ucptr(&bmpData[50], 0); // "Important" colors (0 on non-paletted bitmaps)
	
	// TODO: Verify endianness requirements.
	
	if (bpp == 15)
	{
		// 15-bit color. (Mode 555)
		T_writeBMP_rows(static_cast<const uint16_t*>(screen), &bmpData[54], w, h, pitch,
				MASK_RED_15, MASK_GREEN_15, MASK_BLUE_15,
				SHIFT_RED_15, SHIFT_GREEN_15, SHIFT_BLUE_15);
	}
	else if (bpp == 16)
	{
		// 16-bit color. (Mode 565)
		T_writeBMP_rows(static_cast<const uint16_t*>(screen), &bmpData[54], w, h, pitch,
				MASK_RED_16, MASK_GREEN_16, MASK_BLUE_16,
				SHIFT_RED_16, SHIFT_GREEN_16, SHIFT_BLUE_16);
	}
	else //if (bpp == 32)
	{
		// 32-bit color.
		// BMP uses 24-bit color, so a conversion is still necessary.
		T_writeBMP_rows(static_cast<const uint32_t*>(screen), &bmpData[54], w, h, pitch,
				MASK_RED_32, MASK_GREEN_32, MASK_BLUE_32,
				SHIFT_RED_32, SHIFT_GREEN_32, SHIFT_BLUE_32);
	}
	
	fwrite(bmpData, 1, bmpSize + 54, fImg);
	free(bmpData);
	
	return 1;
}


#ifdef GENS_PNG
/**
 * T_writePNG_rows_16(): Write 16-bit PNG rows.
 * @param screen Pointer to the screen buffer.
 * @param png_ptr PNG pointer.
 * @param info_ptr PNG info pointer.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param maskR Red mask.
 * @param maskG Green mask.
 * @param maskB Blue mask.
 * @param shiftR Red shift. (Right)
 * @param shiftG Green shift. (Right)
 * @param shiftB Blue shift. (Left)
 * @return 1 on success; 0 on error.
 */
template<typename pixel>
static inline int T_writePNG_rows_16(const pixel *screen, png_structp png_ptr, png_infop info_ptr,
				     const int width, const int height, const int pitch,
				     const pixel maskR, const pixel maskG, const pixel maskB,
				     const uint8_t shiftR, const uint8_t shiftG, const uint8_t shiftB)
{
	// Allocate the row buffer.
	uint8_t *rowBuffer;
	if ((rowBuffer = static_cast<uint8_t*>(malloc(width * 3))) == NULL)
	{
		// Could not allocate enough memory.
		fprintf(stderr, "%s: Could not allocate enough memory for the row buffer.\n", __func__);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 0;
	}
	
	// Write the rows.
	for (int y = 0; y < height; y++)
	{
		uint8_t *rowBufPtr = rowBuffer;
		for (int x = 0; x < width; x++)
		{
			pixel MD_Color = *screen++;
			*rowBufPtr++ = (uint8_t)((MD_Color & maskR) >> shiftR);
			*rowBufPtr++ = (uint8_t)((MD_Color & maskG) >> shiftG);
			*rowBufPtr++ = (uint8_t)((MD_Color & maskB) << shiftB);
		}
		
		// Write the row.
		png_write_row(png_ptr, rowBuffer);
		
		// Next row.
		screen += (pitch - width);
	}
	
	// Free the row buffer.
	free(rowBuffer);
	
	return 1;
}

/**
 * writePNG(): Write a PNG image.
 * @param fImg File handle to save the image to.
 * @param w Width of the image.
 * @param h Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param screen Pointer to screen buffer.
 * @param bpp Bits per pixel.
 * @param alpha Alpha channel specification. (32-bit color only.)
 * @return 1 on success; 0 on error.
 */
int ImageUtil::writePNG(FILE *fImg, const int w, const int h, const int pitch,
			const void *screen, const int bpp, const AlphaChannel alpha)
{
	if (!fImg || !screen || (w <= 0 || h <= 0 || pitch <= 0))
		return 0;
	
	png_structp png_ptr;
	png_infop info_ptr;
	
	// Initialize libpng.
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fprintf(stderr, "writePNG(): Error initializing the PNG pointer.\n");
		return 0;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fprintf(stderr, "writePNG(): Error initializing the PNG info pointer.\n");
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 0;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// TODO: Is setjmp() really necessary?
		fprintf(stderr, "writePNG(): Error initializing the PNG setjmp pointer.\n");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 0;
	}
	
	// Initialize libpng I/O.
	png_init_io(png_ptr, fImg);
	
	// Disable PNG filters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	
	// Set the compression level to 5. (Levels range from 1 through 9.)
	// TODO: Add a UI option to set compression level.
	png_set_compression_level(png_ptr, 5);
	
	// Set up the PNG header.
	if (!(bpp == 32 && alpha != ALPHACHANNEL_NONE))
	{
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB,
			     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			     PNG_FILTER_TYPE_DEFAULT);
	}
	else
	{
		// 32-bit color, with alpha channel.
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
			     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			     PNG_FILTER_TYPE_DEFAULT);
	}
	
	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);
	
#ifdef GENS_LIL_ENDIAN
	// PNG stores data in big-endian format.
	// On little-endian systems, byteswapping needs to be enabled.
	// TODO: Check if this really isn't needed on big-endian systems.
	png_set_swap(png_ptr);
#endif
	
	// Write the image.
	if (bpp == 15)
	{
		// 15-bit color. (Mode 555)
		if (!T_writePNG_rows_16(static_cast<const uint16_t*>(screen), png_ptr, info_ptr, w, h, pitch,
					MASK_RED_15, MASK_GREEN_15, MASK_BLUE_15,
					SHIFT_RED_15, SHIFT_GREEN_15, SHIFT_BLUE_15))
		{
			return 0;
		}
	}
	else if (bpp == 16)
	{
		// 16-bit color. (Mode 565)
		if (!T_writePNG_rows_16(static_cast<const uint16_t*>(screen), png_ptr, info_ptr, w, h, pitch,
					MASK_RED_16, MASK_GREEN_16, MASK_BLUE_16,
					SHIFT_RED_16, SHIFT_GREEN_16, SHIFT_BLUE_16))
		{
			return 0;
		}
	}
	else // if (bpp == 32)
	{
		// 32-bit color.
		// Depending on the alpha channel settings, libpng expects either
		// 24-bit data (no alpha) or 32-bit data (with alpha); however,
		// libpng offers an option to automatically convert 32-bit data
		// without alpha channel to 24-bit. (png_set_filler())
		
		// TODO: PNG_FILLER_AFTER, BGR mode - needed for little-endian.
		 // Figure out what's needed on big-endian.
		
		png_byte **row_pointers = static_cast<png_byte**>(malloc(sizeof(png_byte*) * h));
		uint32_t *screen32 = (uint32_t*)screen;
		
		for (int y = 0; y < h; y++)
		{
			row_pointers[y] = (uint8_t*)&screen32[y * pitch];
		}
		
		if (!alpha)
		{
			// No alpha channel. Set filler byte.
			png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
		}
		else if (alpha == ALPHACHANNEL_TRANSPARENCY)
		{
			// Alpha channel indicates transparency.
			// 0x00 == opaque; 0xFF == transparent.
			png_set_invert_alpha(png_ptr);
		}
		
		png_set_bgr(png_ptr);
		png_write_rows(png_ptr, row_pointers, h);
		
		free(row_pointers);
	}
	
	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	return 1;
}
#endif /* GENS_PNG */


int ImageUtil::write(const string& filename, const ImageFormat format,
		     const int w, const int h, const int pitch,
		     const void *screen, const int bpp, const AlphaChannel alpha)
{
	// Write an image file.
	FILE *fImg = fopen(filename.c_str(), "wb");
	if (!fImg)
	{
		fprintf(stderr, "Image::write(): Error opening %s.\n", filename.c_str());
		return 0;
	}
	
	int rval;
#ifdef GENS_PNG
	if (format == IMAGEFORMAT_PNG)
	{
		rval = writePNG(fImg, w, h, pitch, screen, bpp, alpha);
	}
	else
#endif /* GENS_PNG */
	{
		rval = writeBMP(fImg, w, h, pitch, screen, bpp);
	}
	
	fclose(fImg);
	return rval;
}


/**
 * screenShot(): Convenience function to take a screenshot of the game.
 * @return 1 on success; 0 on error.
 */
int ImageUtil::screenShot(void)
{
	// If no game is running, don't do anything.
	if (!Game)
		return 0;
	
	// Variables used:
	// VDP_Num_Vis_Lines: Number of lines visible on the screen. (bitmap height)
	// MD_Screen: MD screen buffer.
	// VDP_Reg.Set4: If 0x01 is set, 320 pixels width; otherwise, 256 pixels width.
	// TODO: Use macros in video/v_inline.h
	const int w = (VDP_Reg.Set4 & 0x01 ? 320 : 256);
	const int h = VDP_Num_Vis_Lines;
	
	// Build the filename.
	int num = -1;
	char filename[GENS_PATH_MAX];
#ifdef GENS_PNG
	const char* ext = "png";
#else /* !GENS_PNG */
	const char* ext = "bmp";
#endif /* GENS_PNG */
	
	do
	{
		num++;
		sprintf(filename, "%s%s_%03d.%s", PathNames.Screenshot_Dir, ROM_Name, num, ext);
	} while (fileExists(filename));
	
	// Attempt to open the file.
	FILE *img = fopen(filename, "wb");
	if (!img)
	{
		// Error opening the file.
		fprintf(stderr, "ImageUtil::screenShot(): Error opening %s\n", filename);
		return 0;
	}
	
	// Save the image.
	void *screen;
	if (bppMD == 15 || bppMD == 16)
		screen = (void*)(&MD_Screen[8]);
	else //if (bppMD == 32)
		screen = (void*)(&MD_Screen32[8]);
	
#ifdef GENS_PNG
	int rval = writePNG(img, w, h, 336, screen, bppMD);
#else /* !GENS_PNG */
	int rval = writeBMP(img, w, h, 336, screen, bppMD);
#endif /* GENS_PNG */
	
	// Close the file.
	fclose(img);
	
	if (rval == 1)
		MESSAGE_NUM_L("Screen shot %d saved", "Screen shot %d saved", num, 1500);
	
	return rval;
}
