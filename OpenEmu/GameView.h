//
//  GameView.h
//  OpenEmu
//
//  Created by Josh Weinberg on 9/1/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface GameView : NSOpenGLView {
	unsigned char* buffer; 
	unsigned currentWidth;
	unsigned currentHeight;
	IBOutlet NSWindow* ownerWindow;
	
	GLenum pixelType;
	GLenum pixelForm;
	GLenum internalForm;
}

- (void) resetVideo;
- (void) drawRect: (NSRect) bounds;
- (void) setupGLWithBuffer: (unsigned char*) buf width: (int) width height: (int) height pixelType: (GLenum) type pixelFormat: (GLenum) pixform internalFormat: (GLenum) internpixform;
- (id) initWithFrame:(NSRect)frameRect;
- (void) awakeFromNib;

@end
