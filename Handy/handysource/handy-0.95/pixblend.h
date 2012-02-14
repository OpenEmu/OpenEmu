//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//
// Pixel Blender Object to allow Eagle like graphics
//

#ifndef _PIXBLEND_H
#define _PIXBLEND_H

class CPixelBlender16BPP
{
	public:
		CPixelBlender16BPP(int width,int height,int zoom,ULONG *data)
		{
			mWidth=width;
			mHeight=height;
			mZoom=zoom;
			mPixelBuffer=(UWORD*)data;
			mLinebase0=0;
			mLinebase1=0;
			mLinebase2=0;
		}

		~CPixelBlender16BPP(void)
		{
		}

		inline void SetLinebase(int posy)
		{
			if(posy>0 && posy<((mZoom*mHeight)-1))
			{
				mLinebase0=mPixelBuffer+(((posy-1)/mZoom)*mWidth);
				mLinebase1=mPixelBuffer+(((posy  )/mZoom)*mWidth);
				mLinebase2=mPixelBuffer+(((posy+1)/mZoom)*mWidth);
			}
			else
			{
				mLinebase0=mPixelBuffer+(((posy)/mZoom)*mWidth);
				mLinebase1=mLinebase0;
				mLinebase2=mLinebase1;
			}
		}

		inline ULONG GetEagle(int posx)
		{
			UWORD pixel1,pixel2,pixel3,pixel4;
			UWORD pixel5,pixel6,pixel7,pixel8,pixel9;
			int	x0,x1,x2;
			//
			// X0 X1 X2
			//
			//  2  3  4    mLinebase0
			//  9  1  5    mLinebase1
			//  8  7  6    mLinebase2
			//
			if(posx>0 && posx<((mZoom*mWidth)-1))
			{
				x0=(posx-1)/mZoom;
				x1=posx/mZoom;
				x2=(posx+1)/mZoom;

				// Upper left quad
				pixel1=*(mLinebase1+x1);
				pixel2=*(mLinebase0+x0);
				pixel3=*(mLinebase0+x1);
				pixel9=*(mLinebase1+x0);
				if(pixel2==pixel3 && pixel3==pixel9 && pixel2!=pixel1) return pixel2;

				// Upper right
				pixel4=*(mLinebase0+x2);
				pixel5=*(mLinebase1+x2);
				if(pixel3==pixel4 && pixel4==pixel5 && pixel3!=pixel1) return pixel3;

				// Lower left
				pixel8=*(mLinebase2+x0);
				pixel7=*(mLinebase2+x1);
				if(pixel7==pixel8 && pixel8==pixel9 && pixel7!=pixel1) return pixel8;

				// Lower right
				pixel6=*(mLinebase2+x2);
				if(pixel5==pixel6 && pixel6==pixel7 && pixel5!=pixel1) return pixel5;

				return pixel1;
			}
			else
			{
				pixel1=*(mLinebase1+(posx/mZoom));
				return pixel1;
			}
		}

	private:
		int	mWidth;
		int mHeight;
		int mZoom;
		UWORD *mPixelBuffer;
		UWORD *mLinebase0;
		UWORD *mLinebase1;
		UWORD *mLinebase2;
};



class CPixelBlender24BPP
{
	public:
		CPixelBlender24BPP(int width,int height,int zoom,ULONG *data)
		{
			mWidth=width;
			mHeight=height;
			mZoom=zoom;
			mPixelBuffer=(UBYTE*)data;
			mLinebase0=0;
			mLinebase1=0;
			mLinebase2=0;
		}

		~CPixelBlender24BPP(void)
		{
		}

		inline void SetLinebase(int posy)
		{
			if(posy>0 && posy<((mZoom*mHeight)-1))
			{
				mLinebase0=mPixelBuffer+((posy-1)/mZoom)*mWidth*3;
				mLinebase1=mPixelBuffer+((posy  )/mZoom)*mWidth*3;
				mLinebase2=mPixelBuffer+((posy+1)/mZoom)*mWidth*3;
			}
			else
			{
				mLinebase0=mPixelBuffer+(posy/mZoom)*mWidth*3;
				mLinebase1=mLinebase0;
				mLinebase2=mLinebase1;
			}
		}

		inline ULONG GetEagle(int posx)
		{
			ULONG pixel1,pixel2,pixel3,pixel4;
			ULONG pixel5,pixel6,pixel7,pixel8,pixel9;
			int	x0,x1,x2;
			//
			// X0 X1 X2
			//
			//  2  3  4    mLinebase0
			//  9  1  5    mLinebase1
			//  8  7  6    mLinebase2
			//
			if(posx>0 && posx<((mZoom*mWidth)-1))
			{
				x0=((posx-1)/mZoom)*3;
				x1=(posx/mZoom)*3;
				x2=((posx+1)/mZoom)*3;

				// Upper left quad
				pixel1=(*(ULONG*)(mLinebase1+x1))&0x00ffffff;;
				pixel2=(*(ULONG*)(mLinebase0+x0))&0x00ffffff;
				pixel3=(*(ULONG*)(mLinebase0+x1))&0x00ffffff;
				pixel9=(*(ULONG*)(mLinebase1+x0))&0x00ffffff;
				if(pixel2==pixel3 && pixel3==pixel9 && pixel2!=pixel1) return pixel2;

				// Upper right
				pixel4=(*(ULONG*)(mLinebase0+x2))&0x00ffffff;
				pixel5=(*(ULONG*)(mLinebase1+x2))&0x00ffffff;
				if(pixel3==pixel4 && pixel4==pixel5 && pixel3!=pixel1) return pixel3;

				// Lower left
				pixel8=(*(ULONG*)(mLinebase2+x0))&0x00ffffff;
				pixel7=(*(ULONG*)(mLinebase2+x1))&0x00ffffff;
				if(pixel7==pixel8 && pixel8==pixel9 && pixel7!=pixel1) return pixel8;

				// Lower right
				pixel6=(*(ULONG*)(mLinebase2+x2))&0x00ffffff;
				if(pixel5==pixel6 && pixel6==pixel7 && pixel5!=pixel1) return pixel5;

				return pixel1;
			}
			else
			{
				pixel1=*(ULONG*)(mLinebase1+((posx/mZoom)*3));
				return pixel1&0x00ffffff;
			}
		}

	private:
		int	mWidth;
		int mHeight;
		int mZoom;
		UBYTE *mPixelBuffer;
		UBYTE *mLinebase0;
		UBYTE *mLinebase1;
		UBYTE *mLinebase2;
};


class CPixelBlender32BPP
{
	public:
		CPixelBlender32BPP(int width,int height,int zoom,ULONG *data)
		{
			mWidth=width;
			mHeight=height;
			mZoom=zoom;
			mPixelBuffer=data;
			mLinebase0=0;
			mLinebase1=0;
			mLinebase2=0;
		}

		~CPixelBlender32BPP(void)
		{
		}

		inline void SetLinebase(int posy)
		{
			if(posy>0 && posy<((mZoom*mHeight)-1))
			{
				mLinebase0=mPixelBuffer+(((posy-1)/mZoom)*mWidth);
				mLinebase1=mPixelBuffer+(((posy  )/mZoom)*mWidth);
				mLinebase2=mPixelBuffer+(((posy+1)/mZoom)*mWidth);
			}
			else
			{
				mLinebase0=mPixelBuffer+(((posy)/mZoom)*mWidth);
				mLinebase1=mLinebase0;
				mLinebase2=mLinebase1;
			}
		}

		inline ULONG GetEagle(int posx)
		{
			ULONG pixel1,pixel2,pixel3,pixel4;
			ULONG pixel5,pixel6,pixel7,pixel8,pixel9;
			int	x0,x1,x2;
			//
			// X0 X1 X2
			//
			//  2  3  4    mLinebase0
			//  9  1  5    mLinebase1
			//  8  7  6    mLinebase2
			//
			if(posx>0 && posx<((mZoom*mWidth)-1))
			{
				x0=(posx-1)/mZoom;
				x1=posx/mZoom;
				x2=(posx+1)/mZoom;

				// Upper left quad
				pixel1=*(mLinebase1+x1);
				pixel2=*(mLinebase0+x0);
				pixel3=*(mLinebase0+x1);
				pixel9=*(mLinebase1+x0);
				if(pixel2==pixel3 && pixel3==pixel9 && pixel2!=pixel1) return pixel2;

				// Upper right
				pixel4=*(mLinebase0+x2);
				pixel5=*(mLinebase1+x2);
				if(pixel3==pixel4 && pixel4==pixel5 && pixel3!=pixel1) return pixel3;

				// Lower left
				pixel8=*(mLinebase2+x0);
				pixel7=*(mLinebase2+x1);
				if(pixel7==pixel8 && pixel8==pixel9 && pixel7!=pixel1) return pixel8;

				// Lower right
				pixel6=*(mLinebase2+x2);
				if(pixel5==pixel6 && pixel6==pixel7 && pixel5!=pixel1) return pixel5;

				return pixel1;
			}
			else
			{
				pixel1=*(mLinebase1+(posx/mZoom));
				return pixel1;
			}
		}

	private:
		int	mWidth;
		int mHeight;
		int mZoom;
		ULONG *mPixelBuffer;
		ULONG *mLinebase0;
		ULONG *mLinebase1;
		ULONG *mLinebase2;
};


#endif


