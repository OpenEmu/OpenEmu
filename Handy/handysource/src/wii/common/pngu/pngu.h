/********************************************************************************************

PNGU Version : 0.2a

Coder : frontier

More info : http://frontier-dev.net

Modified by Tantric, 2009

********************************************************************************************/
#ifndef __PNGU__
#define __PNGU__

// Return codes
#define PNGU_OK							0
#define PNGU_ODD_WIDTH					1
#define PNGU_ODD_STRIDE					2
#define PNGU_INVALID_WIDTH_OR_HEIGHT	3
#define PNGU_FILE_IS_NOT_PNG			4
#define PNGU_UNSUPPORTED_COLOR_TYPE		5
#define PNGU_NO_FILE_SELECTED			6
#define PNGU_CANT_OPEN_FILE				7
#define PNGU_CANT_READ_FILE				8
#define PNGU_LIB_ERROR					9

// Color types
#define PNGU_COLOR_TYPE_GRAY			1
#define PNGU_COLOR_TYPE_GRAY_ALPHA		2
#define PNGU_COLOR_TYPE_PALETTE			3
#define PNGU_COLOR_TYPE_RGB				4
#define PNGU_COLOR_TYPE_RGB_ALPHA		5
#define PNGU_COLOR_TYPE_UNKNOWN 		6


#ifdef __cplusplus
	extern "C" {
#endif

// Types
typedef unsigned char PNGU_u8;
typedef unsigned short PNGU_u16;
typedef unsigned int PNGU_u32;
typedef unsigned long long PNGU_u64;

typedef struct
{
	PNGU_u8 r;
	PNGU_u8 g;
	PNGU_u8 b;
} PNGUCOLOR;

typedef struct
{
	PNGU_u32 imgWidth; // In pixels
	PNGU_u32 imgHeight; // In pixels
	PNGU_u32 imgBitDepth; // In bitx
	PNGU_u32 imgColorType; // PNGU_COLOR_TYPE_*
	PNGU_u32 validBckgrnd; // Non zero if there is a background color
	PNGUCOLOR bckgrnd; // Backgroun color
	PNGU_u32 numTrans; // Number of transparent colors
	PNGUCOLOR *trans; // Transparent colors
} PNGUPROP;

// Image context, always initialize with SelectImageFrom* and free with ReleaseImageContext
struct _IMGCTX;
typedef struct _IMGCTX *IMGCTX; 


/****************************************************************************
*							 Pixel conversion								*
****************************************************************************/

// Macro to convert RGB8 values to RGB565
#define PNGU_RGB8_TO_RGB565(r,g,b) ( ((((PNGU_u16) r) & 0xF8U) << 8) | ((((PNGU_u16) g) & 0xFCU) << 3) | (((PNGU_u16) b) >> 3) )

// Macro to convert RGBA8 values to RGB5A3
#define PNGU_RGB8_TO_RGB5A3(r,g,b,a)	(PNGU_u16) (((a & 0xE0U) == 0xE0U) ? \
										(0x8000U | ((((PNGU_u16) r) & 0xF8U) << 7) | ((((PNGU_u16) g) & 0xF8U) << 2) | (((PNGU_u16) b) >> 3)) : \
										(((((PNGU_u16) a) & 0xE0U) << 7) | ((((PNGU_u16) r) & 0xF0U) << 4) | (((PNGU_u16) g) & 0xF0U) | ((((PNGU_u16) b) & 0xF0U) >> 4)))

// Function to convert two RGB8 values to YCbYCr
PNGU_u32 PNGU_RGB8_TO_YCbYCr (PNGU_u8 r1, PNGU_u8 g1, PNGU_u8 b1, PNGU_u8 r2, PNGU_u8 g2, PNGU_u8 b2);

// Function to convert an YCbYCr to two RGB8 values.
void PNGU_YCbYCr_TO_RGB8 (PNGU_u32 ycbycr, PNGU_u8 *r1, PNGU_u8 *g1, PNGU_u8 *b1, PNGU_u8 *r2, PNGU_u8 *g2, PNGU_u8 *b2);


/****************************************************************************
*							 Image context handling							*
****************************************************************************/

// Selects a PNG file, previosly loaded into a buffer, and creates an image context for subsequent procesing.
IMGCTX PNGU_SelectImageFromBuffer (const void *buffer);

// Selects a PNG file, from any devoptab device, and creates an image context for subsequent procesing.
IMGCTX PNGU_SelectImageFromDevice (const char *filename);

// Frees resources associated with an image context. Always call this function when you no longer need the IMGCTX.
void PNGU_ReleaseImageContext (IMGCTX ctx);


/****************************************************************************
*							 Miscelaneous									*
****************************************************************************/

// Retrieves info from selected PNG file, including image dimensions, color format, background and transparency colors.
int PNGU_GetImageProperties (IMGCTX ctx, PNGUPROP *fileproperties);


/****************************************************************************
*							 Image conversion								*
****************************************************************************/

// Expands selected image into an YCbYCr buffer. You need to specify context, image dimensions, 
// destination address and stride in pixels (stride = buffer width - image width).
int PNGU_DecodeToYCbYCr (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride);

// Macro for decoding an image inside a buffer at given coordinates.
#define PNGU_DECODE_TO_COORDS_YCbYCr(ctx,coordX,coordY,imgWidth,imgHeight,bufferWidth,bufferHeight,buffer)	\
																											\
		PNGU_DecodeToYCbYCr (ctx, imgWidth, imgHeight, ((void *) buffer) + (coordY) * (bufferWidth) * 2 +	\
							(coordX) * 2, (bufferWidth) - (imgWidth))

// Expands selected image into a linear RGB565 buffer. You need to specify context, image dimensions, 
// destination address and stride in pixels (stride = buffer width - image width).
int PNGU_DecodeToRGB565 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride);

// Macro for decoding an image inside a buffer at given coordinates.
#define PNGU_DECODE_TO_COORDS_RGB565(ctx,coordX,coordY,imgWidth,imgHeight,bufferWidth,bufferHeight,buffer)	\
																											\
		PNGU_DecodeToRGB565 (ctx, imgWidth, imgHeight, ((void *) buffer) + (coordY) * (bufferWidth) * 2 +	\
							(coordX) * 2, (bufferWidth) - (imgWidth))

// Expands selected image into a linear RGBA8 buffer. You need to specify context, image dimensions, 
// destination address, stride in pixels and default alpha value, which is used if the source image 
// doesn't have an alpha channel.
int PNGU_DecodeToRGBA8 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride, PNGU_u8 default_alpha);

// Macro for decoding an image inside a buffer at given coordinates.
#define PNGU_DECODE_TO_COORDS_RGBA8(ctx,coordX,coordY,imgWidth,imgHeight,default_alpha,bufferWidth,bufferHeight,buffer)	\
																											\
		PNGU_DecodeToRGBA8 (ctx, imgWidth, imgHeight, ((void *) buffer) + (coordY) * (bufferWidth) * 2 +	\
							(coordX) * 2, (bufferWidth) - (imgWidth), default_alpha)

// Expands selected image into a 4x4 tiled RGB565 buffer. You need to specify context, image dimensions
// and destination address.
int PNGU_DecodeTo4x4RGB565 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer);

// Expands selected image into a 4x4 tiled RGB5A3 buffer. You need to specify context, image dimensions,
// destination address and default alpha value, which is used if the source image doesn't have an alpha channel.
int PNGU_DecodeTo4x4RGB5A3 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u8 default_alpha);

// Expands selected image into a 4x4 tiled RGBA8 buffer. You need to specify context, image dimensions,
// destination address and default alpha value, which is used if the source image doesn't have an alpha channel.
int PNGU_DecodeTo4x4RGBA8 (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u8 default_alpha);

// Encodes an YCbYCr image in PNG format and stores it in the selected device or memory buffer. You need to 
// specify context, image dimensions, destination address and stride in pixels (stride = buffer width - image width).
int PNGU_EncodeFromYCbYCr (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride);

int PNGU_EncodeFromRGB (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride);
int PNGU_EncodeFromGXTexture (IMGCTX ctx, PNGU_u32 width, PNGU_u32 height, void *buffer, PNGU_u32 stride);

// Macro for encoding an image stored into an YCbYCr buffer at given coordinates.
#define PNGU_ENCODE_TO_COORDS_YCbYCr(ctx,coordX,coordY,imgWidth,imgHeight,bufferWidth,bufferHeight,buffer)	\
																											\
		PNGU_EncodeFromYCbYCr (ctx, imgWidth, imgHeight, ((void *) buffer) + (coordY) * (bufferWidth) * 2 +	\
							(coordX) * 2, (bufferWidth) - (imgWidth))

#ifdef __cplusplus
	}
#endif

#endif

