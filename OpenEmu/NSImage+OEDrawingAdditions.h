//
//  NSImage+OEDrawingAdditions.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 13.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface NSImage (OEDrawingAdditions)
- (void)drawInRect:(NSRect)rect fromRect:(NSRect)fromRect operation:(NSCompositingOperation)op fraction:(CGFloat)requestedAlpha respectFlipped:(BOOL)respectContextIsFlipped hints:(NSDictionary *)hints leftBorder:(float)leftBorder rightBorder:(float)rightBorder topBorder:(float)topBorder bottomBorder:(float)bottomBorder;
- (NSImage*)subImageFromRect:(NSRect)rect;
- (NSImage*)setName:(NSString*)name forSubimageInRect:(NSRect)aRect;
@end