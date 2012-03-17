#ifndef GENS_IMAGEUTIL_HPP
#define GENS_IMAGEUTIL_HPP

// TODO: Eliminate this include.
// Move GENS_PATH_MAX somewhere else.
#include "emulator/g_main.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class ImageUtil
{
	public:
		enum ImageFormat
		{
			IMAGEFORMAT_BMP = 0,
			#ifdef GENS_PNG
				IMAGEFORMAT_PNG = 1,
			#endif /* GENS_PNG */
		};
		
		enum AlphaChannel
		{
			ALPHACHANNEL_NONE,
			ALPHACHANNEL_OPACITY,
			ALPHACHANNEL_TRANSPARENCY,
		};
		
		#ifdef GENS_PNG
			static const ImageFormat DefaultImageFormat = IMAGEFORMAT_PNG;
		#else /* !GENS_PNG */
			static const ImageFormat DefaultImageFormat = IMAGEFORMAT_BMP;
		#endif /* GENS_PNG */
		
		static int write(const string& filename, const ImageFormat format,
				 const int w, const int h, const int pitch,
				 const void *screen, const int bpp,
				 const AlphaChannel alpha = ALPHACHANNEL_NONE);
		
		static int screenShot(void);
	
	protected:
		ImageUtil() { }
		~ImageUtil() { }
		
		static int writeBMP(FILE *fImg, const int w, const int h, const int pitch,
				    const void *screen, const int bpp);
		
		static int writePNG(FILE *fImg, const int w, const int h, const int pitch,
				    const void *screen, const int bpp,
				    const AlphaChannel alpha = ALPHACHANNEL_NONE);
};

#endif /* __cplusplus */

#endif /* GENS_IMAGEUTIL_HPP */
