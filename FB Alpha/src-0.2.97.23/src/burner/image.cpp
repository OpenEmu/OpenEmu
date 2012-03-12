#include "burner.h"
#include "png.h"

#define PNG_SIG_CHECK_BYTES (8)

void img_free(IMAGE* img)
{
	free(img->rowptr);
	img->rowptr = NULL;
	if (img->flags & IMG_FREE) {
		if (img->bmpbits) {
			free(img->bmpbits);
			img->bmpbits = NULL;
		}
	}
}

INT32 img_alloc(IMAGE* img)
{
	img->flags		  = 0;

	img->rowbytes	  = ((DWORD)img->width * 24 + 31) / 32 * 4;
	img->imgbytes	  = img->rowbytes * img->height;
	img->rowptr		  = (BYTE**)malloc((size_t)img->height * sizeof(BYTE*));

	if (img->bmpbits == NULL) {
		img->flags   |= IMG_FREE;
		img->bmpbits  = (BYTE*)malloc((size_t)img->imgbytes);
	}

	if (img->rowptr == NULL || img->bmpbits == NULL) {
		img_free(img);
		return 1;
	}

	for (UINT32 y = 0; y < img->height; y++) {
		img->rowptr[img->height - y - 1] = img->bmpbits + y * img->rowbytes;
	}

	return 0;
}

// Interpolate using a Catmull-Rom spline
static inline double interpolate(const double f, const double* c)
{
	return 0.5 * ((				2.0 * c[1]					  ) +
				  (		-c[0] +					   c[2]		  ) *  f +
				  (2.0 * c[0] - 5.0 * c[1] + 4.0 * c[2] - c[3]) * (f * f) +
				  (		-c[0] + 3.0 * c[1] - 3.0 * c[2] + c[3]) * (f * f * f));
}

static inline double interpolatePixelH(const double f, const INT32 x, const UINT8* row, const INT32 width)
{
	double c[4];

	c[0] = x >= 1		   ? row[(x - 1) * 3] : row[0        ];
	c[1] =					 row[ x      * 3];
	c[2] = x < (width - 1) ? row[(x + 1) * 3] : row[width - 1];
	c[3] = x < (width - 2) ? row[(x + 2) * 3] : row[width - 1];

	return interpolate(f, c);
}

static void interpolateRowH(const IMAGE* img, const INT32 y, double* row, const INT32 width)
{
	for (INT32 x = 0, x2; x < width; x++) {

		double f = (double)x * img->width / width;
		x2 = (INT32)f;	f -= x2;

		row[x * 3 + 0] = interpolatePixelH(f, x2, img->rowptr[y] + 0, img->width);
		row[x * 3 + 1] = interpolatePixelH(f, x2, img->rowptr[y] + 1, img->width);
		row[x * 3 + 2] = interpolatePixelH(f, x2, img->rowptr[y] + 2, img->width);
	}
}

static inline void interpolateRowV(const double f, const INT32 y, double** row, const IMAGE* img)
{
	double c[5];

	for (UINT32 x = 0; x < img->width * 3; x++) {

		c[0] = row[0][x];
		c[1] = row[1][x];
		c[2] = row[2][x];
		c[3] = row[3][x];

		c[4] = interpolate(f, c);

		if (c[4] < 0.0) c[4] = 0.0; else if (c[4] > 255.0) c[4] = 255.0;

		img->rowptr[y][x] = (UINT8)c[4];
	}
}

// Resize the image to the required size using area averaging
static INT32 img_process(IMAGE* img, UINT32 width, UINT32 height, INT32 preset, bool swapRB)
{
	static struct { double gamma; double sharpness; INT32 min; INT32 max; } presetdata[] = {
		{ 1.000, 0.000, 0x000000, 0xFFFFFF },			//  0 no effects
		{ 1.000, 1.000, 0x000000, 0xFFFFFF },			//  1 normal sharpening
		{ 1.000, 1.250, 0x000000, 0xFFFFFF },			//  2 preview 1
		{ 1.250, 1.750, 0x000000, 0xFFFFFF },			//  3 preview 2
		{ 1.000, 0.750, 0x000000, 0xFFFFFF },			//  4 preview 3
		{ 1.125, 1.500, 0x000000, 0xFFFFFF },			//  5 preview 4
		{ 1.000, 1.000, 0x000000, 0xFFFFFF },			//  6 marquee dim   1
		{ 1.250, 1.000, 0x202020, 0xFFFFFF },			//  7 marquee light 1
		{ 1.250, 1.000, 0x000000, 0xEFEFEF },			//  8 marquee dim   2
		{ 0.750, 1.750, 0x202020, 0xFFFFFF },			//  9 marquee light 2
	};

	IMAGE sized_img;
	
	double ratio = (double)(height * width) / (img->height * img->width);

	{
		double LUT[256];

		INT32 rdest = 0, gdest = 1, bdest = 2;
		if (swapRB) {
			rdest = 2, gdest = 1, bdest = 0;
		}

		// Compute a look-up table for gamma correction

		double min = (presetdata[preset].min & 255);
		double rng = ((presetdata[preset].max & 255) - (presetdata[preset].min & 255));

		for (INT32 i = 0; i < 256; i++) {

			LUT[i] = min + rng * pow((i / 255.0), presetdata[preset].gamma);
		}

		// Apply gamma
		for (UINT32 y = 0; y < img->height; y++) {
			for (UINT32 x = 0; x < img->width; x++) {
				UINT8 r = (UINT8)LUT[img->rowptr[y][x * 3 + 0]];
				UINT8 g = (UINT8)LUT[img->rowptr[y][x * 3 + 1]];
				UINT8 b = (UINT8)LUT[img->rowptr[y][x * 3 + 2]];

				img->rowptr[y][x * 3 + rdest] = r;
				img->rowptr[y][x * 3 + gdest] = g;
				img->rowptr[y][x * 3 + bdest] = b;
			}
		}
	}

	if (img->height == height && img->width == width) {

		// We don't need to resize the image

		return 0;
	}

	memset(&sized_img, 0, sizeof(IMAGE));
	sized_img.width = width;
	sized_img.height = height;
	img_alloc(&sized_img);

	if (ratio >= 1.0) {

		double* row[4];

		// Enlarge the image using bi-cubic filtering

		for (INT32 i = 0; i < 4; i++) {
			row[i] = (double*)malloc(width * 3 * sizeof(double));
		}

		interpolateRowH(img, 0, row[0], width);
		interpolateRowH(img, 0, row[1], width);
		interpolateRowH(img, 1, row[2], width);
		interpolateRowH(img, 2, row[3], width);

		for (UINT32 y = 0, ylast = 0, y2 = 0; y < height; ylast = y2, y++) {
			double f = (double)y * img->height / height;

			y2 = (UINT32)f; f -= y2;

			if (y2 > ylast) {
				double* r = row[0];
				row[0] = row[1];
				row[1] = row[2];
				row[2] = row[3];
				row[3] = r;

				interpolateRowH(img, (y2 + 2) < img->height ? y2 + 2 : img->height - 1, row[3], width);
			}

			interpolateRowV(f, y, row, &sized_img);
		}

		for (INT32 i = 0; i < 4; i++) {
			if (row[i]) {
				free(row[i]);
				row[i] = NULL;
			}
		}

		img_free(img);

		memcpy(img, &sized_img, sizeof(IMAGE));

		return 0;
	}

	// Shrink the image using area averaging and  apply gamma

	for (UINT32 y = 0; y < sized_img.height; y++) {
		for (UINT32 x = 0; x < sized_img.width; x++) {

			double r0, b0, g0, r1, g1, b1, xf, yf;
			UINT32 x1, y1;

			r0 = g0 = b0 = 0.0;

			y1 = img->height * y / sized_img.height;
			yf = (double)img->height * y / sized_img.height - y1;

			x1 = img->width * x / sized_img.width;
			xf = (double)img->width * x / sized_img.width - x1;
			r1 = (double)img->rowptr[y1][x1 * 3 + 0] * (1.0 - xf);
			g1 = (double)img->rowptr[y1][x1 * 3 + 1] * (1.0 - xf);
			b1 = (double)img->rowptr[y1][x1 * 3 + 2] * (1.0 - xf);

			for (x1 = x1 + 1; x1 < img->width * (x + 1) / sized_img.width; x1++) {
				r1 += (double)img->rowptr[y1][x1 * 3 + 0];
				g1 += (double)img->rowptr[y1][x1 * 3 + 1];
				b1 += (double)img->rowptr[y1][x1 * 3 + 2];
			}

			if (x1 < img->width) {
				xf = (double)img->width * (x + 1) / sized_img.width - x1;
				r1 += (double)img->rowptr[y1][x1 * 3 + 0] * xf;
				g1 += (double)img->rowptr[y1][x1 * 3 + 1] * xf;
				b1 += (double)img->rowptr[y1][x1 * 3 + 2] * xf;
			}

			r0 += r1 * (1.0 - yf);
			g0 += g1 * (1.0 - yf);
			b0 += b1 * (1.0 - yf);

			for (y1 = y1 + 1; y1 < img->height * (y + 1) / sized_img.height; y1++) {
				x1 = img->width * x / sized_img.width;
				xf = (double)img->width * x / sized_img.width - x1;
	
				r1 = (double)img->rowptr[y1][x1 * 3 + 0] * (1.0 - xf);
				g1 = (double)img->rowptr[y1][x1 * 3 + 1] * (1.0 - xf);
				b1 = (double)img->rowptr[y1][x1 * 3 + 2] * (1.0 - xf);
	
				for (x1 = x1 + 1; x1 < img->width * (x + 1) / sized_img.width; x1++) {
					r1 += (double)img->rowptr[y1][x1 * 3 + 0];
					g1 += (double)img->rowptr[y1][x1 * 3 + 1];
					b1 += (double)img->rowptr[y1][x1 * 3 + 2];
				}
	
				if (x1 < img->width) {
					xf = (double)img->width * (x + 1) / sized_img.width - x1;
					r1 += (double)img->rowptr[y1][x1 * 3 + 0] * xf;
					g1 += (double)img->rowptr[y1][x1 * 3 + 1] * xf;
					b1 += (double)img->rowptr[y1][x1 * 3 + 2] * xf;
				}

				r0 += r1;
				g0 += g1;
				b0 += b1;
			}

			if (y1 < img->height) {
				yf = (double)img->height * (y + 1) / sized_img.height - y1;
	
				x1 = img->width * x / sized_img.width;
				xf = (double)img->width * x / sized_img.width - x1;
				r1 = (double)img->rowptr[y1][x1 * 3 + 0] * (1.0 - xf);
				g1 = (double)img->rowptr[y1][x1 * 3 + 1] * (1.0 - xf);
				b1 = (double)img->rowptr[y1][x1 * 3 + 2] * (1.0 - xf);
	
				for (x1 = x1 + 1; x1 < img->width * (x + 1) / sized_img.width; x1++) {
					r1 += (double)img->rowptr[y1][x1 * 3 + 0];
					g1 += (double)img->rowptr[y1][x1 * 3 + 1];
					b1 += (double)img->rowptr[y1][x1 * 3 + 2];
				}
	
				if (x1 < img->width) {
					xf = (double)img->width * (x + 1) / sized_img.width - x1;
					r1 += (double)img->rowptr[y1][x1 * 3 + 0] * xf;
					g1 += (double)img->rowptr[y1][x1 * 3 + 1] * xf;
					b1 += (double)img->rowptr[y1][x1 * 3 + 2] * xf;
				}
	
				r0 += r1 * yf;
				g0 += g1 * yf;
				b0 += b1 * yf;
			}

			sized_img.rowptr[y][x * 3 + 0] = (UINT8)(r0 * ratio);// + 0.5;
			sized_img.rowptr[y][x * 3 + 1] = (UINT8)(g0 * ratio);// + 0.5;
			sized_img.rowptr[y][x * 3 + 2] = (UINT8)(b0 * ratio);// + 0.5;
		}
	}

	img_free(img);

//	if (!presetdata[preset].sharpness || ratio >= 1.0) {
		memcpy(img, &sized_img, sizeof(IMAGE));

		return 0;
//	}

	// Sharpen the image using an unsharp mask

/*	IMAGE sharp_img;

	memset(&sharp_img, 0, sizeof(IMAGE));
	sharp_img.width = width;
	sharp_img.height = height;
	img_alloc(&sharp_img);

	// Create a convolution matrix for a gaussian blur

	double matrix[9][9];

	// Control the radius of the blur based on sharpness and image reduction
	double b = presetdata[preset].sharpness / (8.0 * ratio);

//	dprintf(_T("    %3ix%3i -> %3ix%3i %0.4lf %0.4lf\n"), img->width, img->height, sized_img.width, sized_img.height, ratio, b);
	if (b > 1.5) { b = 1.5; }
	b = pow(b, 2.0);

	for (INT32 x = -4; x < 5; x++) {
		for (INT32 y = -4; y < 5; y++) {

			double c = sqrt(double(x * x + y * y));
			
			matrix[y + 4][x + 4] = 1.0 / exp(-c * -c / b);
		}
	}

	for (INT32 y = 0; y < sized_img.height; y++) {
		for (INT32 x = 0; x < sized_img.width; x++) {

			double r, g, b, m;

			r = g = b = m = 0.0;

			// Convolve the image
			for (INT32 y1 = -4; y1 < 5; y1++) {
				if (y + y1 > 0 && y + y1 < sized_img.height) {
					for (INT32 x1 = -4; x1 < 5; x1++) {
						if (x + x1 > 0 && x + x1 < sized_img.width) {
							r += matrix[y1 + 4][x1 + 4] * sized_img.rowptr[y + y1][(x + x1) * 3 + 0];
							g += matrix[y1 + 4][x1 + 4] * sized_img.rowptr[y + y1][(x + x1) * 3 + 1];
							b += matrix[y1 + 4][x1 + 4] * sized_img.rowptr[y + y1][(x + x1) * 3 + 2];
							m += abs(matrix[y1 + 4][x1 + 4]);
						}
					}
				}
			}

			// Normalise the image
			r /= m;
			g /= m;
			b /= m;

			// create the mask by subtracting the blurred image from the original
			r = presetdata[preset].sharpness * ((double)sized_img.rowptr[y][x * 3 + 0] - r);
			g = presetdata[preset].sharpness * ((double)sized_img.rowptr[y][x * 3 + 1] - g);
			b = presetdata[preset].sharpness * ((double)sized_img.rowptr[y][x * 3 + 2] - b);

			// Implement a treshold control, rolloff beneath the treshold based on image reduction

			double treshold = 32.0 / presetdata[preset].sharpness;

			if (abs(r) < treshold && abs(g) < treshold && abs(b) < treshold) {
				 r /= 1.0 + ((treshold - abs(r)) * ratio);
				 g /= 1.0 + ((treshold - abs(g)) * ratio);
				 b /= 1.0 + ((treshold - abs(b)) * ratio);
			}

			// Add the mask back to the original
			r = (double)sized_img.rowptr[y][x * 3 + 0] + r;
			g = (double)sized_img.rowptr[y][x * 3 + 1] + g;
			b = (double)sized_img.rowptr[y][x * 3 + 2] + b;

			// Clamp RGB values
			if (r < 0) { r = 0; } if (r > 255) { r = 255; }
			if (g < 0) { g = 0; } if (g > 255) { g = 255; }
			if (b < 0) { b = 0; } if (b > 255) { b = 255; }

			// Store image
			sharp_img.rowptr[y][x * 3 + 0] = r;
			sharp_img.rowptr[y][x * 3 + 1] = g;
			sharp_img.rowptr[y][x * 3 + 2] = b;
		}
	}*/

	img_free(&sized_img);

//	memcpy(img, &sharp_img, sizeof(IMAGE));

	return 0;
}

bool PNGIsImage(FILE* fp)
{
	if (fp) {
		UINT8 pngsig[PNG_SIG_CHECK_BYTES];

		fseek(fp, 0, SEEK_SET);
		fread(pngsig, 1, PNG_SIG_CHECK_BYTES, fp);
		fseek(fp, 0, SEEK_SET);

		if (png_sig_cmp(pngsig, 0, PNG_SIG_CHECK_BYTES) == 0) {
			return true;
		}
	}

	return false;
}

INT32 PNGLoad(IMAGE* img, FILE* fp, INT32 nPreset)
{
	IMAGE temp_img;
	png_uint_32 width, height;
	INT32 bit_depth, color_type;
	
	if (fp) {
		// check signature
		UINT8 pngsig[PNG_SIG_CHECK_BYTES];
		fread(pngsig, 1, PNG_SIG_CHECK_BYTES, fp);
		if (png_sig_cmp(pngsig, 0, PNG_SIG_CHECK_BYTES)) {
			return 1;
		}
		
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return 1;
		}
	
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			return 1;
		}
		
		memset(&temp_img, 0, sizeof(IMAGE));
		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, PNG_SIG_CHECK_BYTES);
		png_read_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
	
		if (setjmp(png_jmpbuf(png_ptr))) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return 1;
		}
		
		// Instruct libpng to convert the image to 24-bit RGB format
		if (color_type == PNG_COLOR_TYPE_PALETTE) {
			png_set_palette_to_rgb(png_ptr);
		}
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
			png_set_gray_to_rgb(png_ptr);
		}
		if (bit_depth == 16) {
			png_set_strip_16(png_ptr);
		}
		if (color_type & PNG_COLOR_MASK_ALPHA) {
			png_set_strip_alpha(png_ptr);
		}
	
		temp_img.width  = width;
		temp_img.height = height;
		
		// Initialize our img structure
		if (img_alloc(&temp_img)) {
			//longjmp(png_ptr->jmpbuf, 1);
			png_jmpbuf(png_ptr);
		}
		
		// If bad things happen in libpng we need to do img_free(&temp_img) as well
		if (setjmp(png_jmpbuf(png_ptr))) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			img_free(&temp_img);
			return 1;
		}
	
		// Read the .PNG image
		png_set_bgr(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
		png_read_image(png_ptr, temp_img.rowptr);
		png_read_end(png_ptr, (png_infop)NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	} else {

#ifdef WIN32
		// Find resource
		HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(BMP_SPLASH), RT_BITMAP);
		HGLOBAL hglobal = LoadResource(NULL, (HRSRC)hrsrc);
		BYTE* pResourceData = (BYTE*)LockResource(hglobal);

		BITMAPINFOHEADER* pbmih = (BITMAPINFOHEADER*)LockResource(hglobal);
		
		// Allocate a new image
		memset(&temp_img, 0, sizeof(IMAGE));
		temp_img.width   = pbmih->biWidth;
		temp_img.height  = pbmih->biHeight;
		temp_img.bmpbits = pResourceData + pbmih->biSize;
		img_alloc(&temp_img);
		
#else
		return 1;
#endif

	}
	
	if (img_process(&temp_img, img->width ? img->width : temp_img.width, img->height ? img->height : temp_img.height, nPreset, false)) {
		img_free(&temp_img);
		return 1;
	}
	
	memcpy(img, &temp_img, sizeof(IMAGE));

	return 0;
}
