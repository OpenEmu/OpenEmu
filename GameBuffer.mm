//
//  GameBuffer.mm
//  OpenEmu
//
//  Created by Josh Weinberg on 9/15/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameBuffer.h"
#import "GameCore.h"
//#include "scalebit.h"
#include "Filters.h"

@implementation GameBuffer

//@dynamic buffer;

@dynamic width;
@dynamic height;
@dynamic pixelForm;
@dynamic pixelType;
@dynamic internalForm;

@synthesize gameCore, filter;


unsigned short OEConvertPixel(unsigned int pixel)
{
	unsigned char R = (pixel & 0x00FF0000) >> 16;
	unsigned char G = (pixel & 0x0000FF00) >> 8;
	unsigned char B = (pixel & 0x000000FF);
	
	R = (R * 31) / 255;
	B = (B * 31) / 255;
	G = (G * 63) / 255;
	
	return (R << 11) | (G << 5) | B;
}

unsigned char *OEConvertBufferTo16BitPerPixels(const unsigned int *aBuffer, int aWidth, int aHeight)
{
    unsigned short *buf = new unsigned short[aWidth * aHeight];
	
	for(int h = 0; h < aHeight; h++)
		for(int w = 0; w < aWidth ; w++)
			buf[h * aWidth + w] = OEConvertPixel(aBuffer[h * aWidth + w]);
	
	//NSLog(@"Converted");
	return (unsigned char *)buf;
}

// No default version for this class
- (id)init
{
    [self release];
    return nil;
}

// Designated Initializer
- (id) initWithGameCore: (GameCore*) core
{
	self = [super init];
	if(self)
	{
		filterBuffer = NULL;
		gameCore = core;
		multiplier = 1;
	}
	return self;	
}

- (void)setFilter:(eFilter)aFilter
{
	filter = aFilter;
	
	if(filterBuffer != NULL)
	{
		delete[] filterBuffer;
		filterBuffer = NULL;
	}	
	
	// treat GLSL filters without using CPU side filters
	switch (filter) {
		case eFilter_None:
		case eFilter_Nearest:
		case eFilter_Scaler2xGLSL:
		case eFilter_Scaler4xGLSL:
		case eFilter_HQ2xGLSL:
		case eFilter_HQ4xGLSL:
			multiplier = 1;
			return;
		case eFilter_Scaler2x:
		case eFilter_HQ2x:
			multiplier = 2;
			break;
		case eFilter_Scaler3x:
		case eFilter_HQ3x:
			multiplier = 3;
			break;
	}
	
	filterBuffer = new unsigned char[[gameCore width] * [gameCore height] * 4 * multiplier * multiplier];
	
	InitLUTs();
}

- (GLenum)pixelForm
{
	return [gameCore pixelFormat];
}

- (GLenum)pixelType
{
//	return GL_UNSIGNED_INT_8_8_8_8;
	return [gameCore pixelType];
}

- (GLenum)internalForm
{
	return [gameCore internalPixelFormat];
}

- (void)updateBuffer
{
	// we handle the GLSL filters on the GPU, so treat them like  eFilter_None 
	switch (filter) {
		case eFilter_Scaler2x:
			scale(2, filterBuffer, [gameCore width] * 4 * multiplier * sizeof(unsigned char), [gameCore videoBuffer], [gameCore width] * 4 * sizeof(unsigned char), 4, [gameCore width], [gameCore height]);
			break;
		case eFilter_Scaler3x:
			scale(3, filterBuffer, [gameCore width] * 4 * multiplier * sizeof(unsigned char), [gameCore videoBuffer], [gameCore width] * 4 * sizeof(unsigned char), 4, [gameCore width], [gameCore height]);
			break;
		case eFilter_HQ2x:
        {
			unsigned char *tempBuf = OEConvertBufferTo16BitPerPixels((const unsigned int *)[gameCore videoBuffer], [gameCore width], [gameCore height]);
			hq2x_32(tempBuf, filterBuffer, [gameCore width], [gameCore height], [self width] * 4);
            
            delete[] tempBuf;
			break;
        }
		case eFilter_HQ3x:
		{
			unsigned char *tempBuf = OEConvertBufferTo16BitPerPixels((const unsigned int *)[gameCore videoBuffer], [gameCore width], [gameCore height]);
			hq3x_32(tempBuf, filterBuffer, [gameCore width], [gameCore height], [self width] * 4);			
            delete[] tempBuf;
			break;
		}	
	}
}


- (const unsigned char *)buffer
{
	// im so sorry this is so ugly :( - vade
	if(filter == eFilter_None || filter == eFilter_Nearest || filter == eFilter_Scaler2xGLSL || filter == eFilter_Scaler4xGLSL || filter == eFilter_HQ2xGLSL || filter == eFilter_HQ4xGLSL)
		return [gameCore videoBuffer];
	else 
		return filterBuffer;
}

- (int) width
{
	return [gameCore width] * multiplier;
}

- (int) height
{
	return [gameCore height] * multiplier;
}

- (void) dealloc
{
	if(filterBuffer != NULL)
	{
		delete[] filterBuffer;
		filterBuffer = NULL;
	}
	[super dealloc];
}
@end
