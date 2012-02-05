//
//  NSImage+OEDrawingAdditions.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 13.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifndef NoInterpol
#define NoInterpol [NSDictionary dictionaryWithObject:[NSNumber numberWithInteger:NSImageInterpolationNone] forKey:NSImageHintInterpolation]
#endif
@interface NSImage (NSImage_OEDrawingAdditions)
- (void)drawInRect:(NSRect)targetRect fromRect:(NSRect)sourceRect operation:(NSCompositingOperation)op fraction:(CGFloat)requestedAlpha respectFlipped:(BOOL)respectContextIsFlipped hints:(NSDictionary *)hints leftBorder:(float)leftBorder rightBorder:(float)rightBorder topBorder:(float)topBorder bottomBorder:(float)bottomBorder;
- (NSImage*)subImageFromRect:(NSRect)rect;
- (void)setName:(NSString*)name forSubimageInRect:(NSRect)aRect;
@end