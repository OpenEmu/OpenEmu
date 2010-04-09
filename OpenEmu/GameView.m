//
//  MyOpenGLView.m
//  GambatteDecoy
//
//  Created by ben on 7/10/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameView.h"
#import <OpenGL/OpenGL.h>


@implementation GameView

- (void) resetVideo
{
	NSRect f = [self bounds];
	glViewport(0.0, 0.0, NSWidth(f),NSHeight(f));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,NSWidth(f),NSHeight(f), 0.0, 0.0, 1.0);
}

- (void) drawRect: (NSRect) bounds
{	
	//lock GL context, needed in multithreaded app
	CGLContextObj ctx = CGLGetCurrentContext();
	CGLLockContext(ctx);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT,1);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
	
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, currentWidth, currentHeight, pixelForm, pixelType, buffer);

	NSRect f;
	if(![self isInFullScreenMode])
	{
		f = [[ownerWindow contentView] bounds];
		
		f = [self bounds];
		glBegin(GL_QUADS);         
		{
			glTexCoord2f(0, 0);
			glVertex2f(0, 0);
			glTexCoord2f(0, currentHeight);
			glVertex2f(0, NSHeight(f));
			glTexCoord2f(currentWidth, currentHeight);
			glVertex2f(NSWidth(f), NSHeight(f));
			glTexCoord2f(currentWidth, 0);
			glVertex2f(NSWidth(f), 0);
		}
		glEnd();
	}
	else
	{
		f = [[ownerWindow contentView] bounds];
		NSLog(@"Fullscreen bounds: %d", f.size.width);
		float h_ratio = NSHeight(f)/(float)currentHeight;
		
		f=NSMakeRect((NSWidth(f)-(currentWidth*h_ratio))/2,0,currentWidth * h_ratio,currentHeight*h_ratio);
		glBegin(GL_QUADS);         
		{
			glTexCoord2f(0, 0);
			glVertex2f(NSMinX(f), NSMinY(f));
			glTexCoord2f(0, currentHeight);
			glVertex2f(NSMinX(f), NSMaxY(f));
//			glVertex2f(0, currentHeight);
			glTexCoord2f(currentWidth, currentHeight);
			glVertex2f(NSMaxX(f), NSMaxY(f));
//			glVertex2f(currentWidth, currentHeight);
			glTexCoord2f(currentWidth, 0);
			glVertex2f(NSMaxX(f), NSMinY(f));
//			glVertex2f(currentWidth, 0);
		}
		glEnd();
	}
		/*
	glBegin(GL_QUADS);         
	{
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(0, currentHeight);
		glVertex2f(0, NSHeight(bounds));
		glTexCoord2f(currentWidth, currentHeight);
		glVertex2f(NSWidth(bounds), NSHeight(bounds));
		glTexCoord2f(currentWidth, 0);
		glVertex2f(NSWidth(bounds), 0);
	}*/
	
	glFlush();
	//unlock GL context
	//[[NSOpenGLContext currentContext] flushBuffer];
	CGLUnlockContext(ctx);
}

- (void) setupGLWithBuffer: (unsigned char*) buf width: (int) width height: (int) height pixelType: (GLenum) type pixelFormat: (GLenum) pixform internalFormat: (GLenum) internpixform
{
	
	pixelType = type;
	pixelForm = pixform;
	internalForm = internpixform;
	
	//lock GL context, needed in multithreaded app
	CGLContextObj ctx = CGLGetCurrentContext();
	CGLLockContext(ctx);
	
	currentWidth = width;
	currentHeight = height;
	buffer = buf;
	
	glViewport(0.0, 0.0, NSWidth([self bounds]), NSHeight([self bounds]));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,NSWidth([self bounds]), NSHeight([self bounds]), 0.0, 0.0, 1.0);
	
	GLuint textures;
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glHint(GL_TRANSFORM_HINT_APPLE, GL_FASTEST);
	glEnable(GL_TEXTURE_RECTANGLE_EXT);
	glGenTextures(1, &textures);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 1);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, internalForm, currentWidth, currentHeight, 0, pixelForm, pixelType, buffer);
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	
	//unlock GL context
	CGLUnlockContext(ctx);
}

- (id)initWithFrame:(NSRect)frameRect
{	
	//lock GL context, needed in multithreaded app
	CGLContextObj ctx = CGLGetCurrentContext();
	CGLLockContext(ctx);
	
	NSOpenGLPixelFormatAttribute MyAttributes[] =
    {
        NSOpenGLPFAWindow,
        NSOpenGLPFASingleRenderer,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAScreenMask,
		(NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
        NSOpenGLPFADoubleBuffer,
        (NSOpenGLPixelFormatAttribute)0
    };
	
    NSOpenGLPixelFormat* pixelFormat =
	[[NSOpenGLPixelFormat alloc] initWithAttributes:MyAttributes];
	
	
	
	self = [super initWithFrame:frameRect pixelFormat:pixelFormat];
    if (self == nil)
    {
        return nil;
    }
	
	//unlock GL context
	CGLUnlockContext(ctx);
	
	return self;
}

- (void) awakeFromNib
{
	//center the window on the screen when the objects are loaded from the nib

	NSRect f = [self bounds];
	NSScreen* mainScreen = [NSScreen mainScreen];
	int width = ([mainScreen frame].size.width - NSWidth(f)) / 2;
	int height = ([mainScreen frame].size.height - NSHeight(f)) / 2;
	NSRect tempR = NSMakeRect(width, height, NSWidth(f), NSHeight(f) + 22);
	[[self window] setFrame:tempR display: NO];
	
}



@end