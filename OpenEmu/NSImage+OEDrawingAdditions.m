//
//  NSImage+OEDrawingAdditions.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 13.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "NSImage+OEDrawingAdditions.h"


@implementation NSImage (OEDrawingAdditions)
- (void)drawInRect:(NSRect)rect fromRect:(NSRect)imageRect operation:(NSCompositingOperation)op fraction:(CGFloat)frac respectFlipped:(BOOL)flipped hints:(NSDictionary *)hints leftBorder:(float)leftBorder rightBorder:(float)rightBorder topBorder:(float)topBorder bottomBorder:(float)bottomBorder{
	
	if(NSEqualRects(imageRect, NSZeroRect)) imageRect=NSMakeRect(0, 0, [self size].width, [self size].height);
	
	// Draw Center
	NSRect sourceRect = NSMakeRect(imageRect.origin.x+leftBorder, imageRect.origin.y+topBorder, imageRect.size.width-rightBorder-leftBorder, imageRect.size.height-topBorder-bottomBorder);
	NSRect targetRect = rect;
	
	targetRect.origin.x += leftBorder;
	targetRect.size.width = rect.size.width-leftBorder-rightBorder;
	targetRect.origin.y += topBorder;
	targetRect.size.height = rect.size.height-topBorder-bottomBorder;
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	
	// Left Bottom
	sourceRect = imageRect;
	sourceRect.size.width = leftBorder;
	sourceRect.size.height = bottomBorder;
	
	targetRect = rect;
	targetRect.origin.y += targetRect.size.height-bottomBorder;
	targetRect.size.width = leftBorder;
	targetRect.size.height = bottomBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	// Left Top
	sourceRect = imageRect;
	sourceRect.origin.y += imageRect.size.height-topBorder;
	sourceRect.size.width = leftBorder;
	sourceRect.size.height = topBorder;
	
	targetRect = rect;
	targetRect.size.width = leftBorder;
	targetRect.size.height = topBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	// Right Top
	sourceRect = imageRect;
	sourceRect.origin.y += imageRect.size.height-topBorder;
	sourceRect.origin.x += imageRect.size.width-rightBorder;
	sourceRect.size.width = rightBorder;
	sourceRect.size.height = topBorder;
	
	targetRect = rect;
	targetRect.origin.x = rect.origin.x + rect.size.width - rightBorder;
	targetRect.size.width = rightBorder;
	targetRect.size.height = topBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	// Right Bottom
	sourceRect = imageRect;
	sourceRect.size.width = leftBorder;
	sourceRect.size.height = bottomBorder;
	sourceRect.origin.x += imageRect.size.width-rightBorder;
	
	targetRect = rect;
	targetRect.origin.x += rect.size.width - rightBorder;
	targetRect.origin.y += targetRect.size.height-bottomBorder;
	targetRect.size.width = leftBorder;
	targetRect.size.height = bottomBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	// Top Middle
	sourceRect = imageRect;
	sourceRect.size.width = imageRect.size.width-leftBorder-rightBorder;
	sourceRect.size.height = topBorder;
	sourceRect.origin.y += imageRect.size.height-topBorder;
	sourceRect.origin.x += leftBorder;
	
	targetRect = rect;
	targetRect.size.height = topBorder;
	targetRect.size.width = rect.size.width-leftBorder-rightBorder;
	targetRect.origin.x += leftBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	// Bottom Middle
	sourceRect = imageRect;
	sourceRect.size.width = imageRect.size.width-leftBorder-rightBorder;
	sourceRect.size.height = bottomBorder;
	sourceRect.origin.x += leftBorder;
	
	targetRect = rect;
	targetRect.size.height = bottomBorder;
	targetRect.size.width = rect.size.width-leftBorder-rightBorder;
	targetRect.origin.x += leftBorder;
	targetRect.origin.y += rect.size.height-bottomBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	// Left Middle
	sourceRect = imageRect;
	sourceRect.size.width = leftBorder;
	sourceRect.size.height = imageRect.size.height-topBorder-bottomBorder;
	sourceRect.origin.y += topBorder;
	
	targetRect = rect;
	targetRect.size.height = rect.size.height-topBorder-bottomBorder;
	targetRect.size.width = leftBorder;
	targetRect.origin.y += topBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
	
	// Right Middle
	sourceRect = imageRect;
	sourceRect.size.width = rightBorder;
	sourceRect.size.height = imageRect.size.height-topBorder-bottomBorder;
	sourceRect.origin.y += topBorder;
	sourceRect.origin.x += imageRect.size.width-rightBorder;
	
	targetRect = rect;
	targetRect.size.height = rect.size.height-topBorder-bottomBorder;
	targetRect.size.width = rightBorder;
	targetRect.origin.y += topBorder;
	targetRect.origin.x += rect.size.width-rightBorder;
	
	[self drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:nil];
}

- (NSImage*)subImageFromRect:(NSRect)rect{
	NSImage* newImage = [[NSImage alloc] initWithSize:rect.size];
	
	[newImage lockFocus];
	[self drawInRect:NSMakeRect(0, 0, newImage.size.width, newImage.size.height) fromRect:rect operation:NSCompositeCopy fraction:1.0];
	[newImage unlockFocus];
	
	return [newImage autorelease];
}

- (NSImage*)setName:(NSString*)name forSubimageInRect:(NSRect)aRect{
	NSImage* resultImage = [self subImageFromRect:aRect];
	[resultImage setName:name];
	
	return [resultImage retain];
}
@end
