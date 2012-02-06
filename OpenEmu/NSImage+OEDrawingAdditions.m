/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "NSImage+OEDrawingAdditions.h"

@implementation NSImage (NSImage_OEDrawingAdditions)
static NSMutableArray* interfaceImages;

- (void)drawInRect:(NSRect)targetRect fromRect:(NSRect)sourceRect operation:(NSCompositingOperation)op fraction:(CGFloat)frac respectFlipped:(BOOL)flipped hints:(NSDictionary *)hints leftBorder:(float)leftBorder rightBorder:(float)rightBorder topBorder:(float)topBorder bottomBorder:(float)bottomBorder{
    
    if(NSEqualRects(sourceRect, NSZeroRect)) sourceRect=NSMakeRect(0, 0, [self size].width, [self size].height);
    
    NSRect workingSourceRect;
    NSRect workingTargetRect;
    
    BOOL sourceFlipped = [self isFlipped];
    BOOL targetFlipped = [[NSGraphicsContext currentContext] isFlipped];
    
    NSDictionary *drawingHints = hints;
    if(!drawingHints)
        drawingHints = NoInterpol;
    
    // Bottom Left
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x, sourceRect.origin.y+sourceRect.size.height-bottomBorder, leftBorder, bottomBorder);
    } else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x, sourceRect.origin.y, leftBorder, bottomBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x, targetRect.origin.y+targetRect.size.height-bottomBorder, leftBorder, bottomBorder);
    }
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x, targetRect.origin.y, leftBorder, bottomBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    // Bottom Center
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+leftBorder, sourceRect.origin.y+sourceRect.size.height-bottomBorder, sourceRect.size.width-leftBorder-rightBorder, bottomBorder);
    } 
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+leftBorder, sourceRect.origin.y, sourceRect.size.width-leftBorder-rightBorder, bottomBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+leftBorder, targetRect.origin.y+targetRect.size.height-bottomBorder, targetRect.size.width-leftBorder-rightBorder, bottomBorder);
    } 
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+leftBorder, targetRect.origin.y, targetRect.size.width-leftBorder-rightBorder, bottomBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    // Bottom Right
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+sourceRect.size.width-rightBorder, sourceRect.origin.y+sourceRect.size.height-bottomBorder, rightBorder, bottomBorder);
    } 
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+sourceRect.size.width-rightBorder, sourceRect.origin.y, rightBorder, bottomBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+targetRect.size.width-rightBorder, targetRect.origin.y+targetRect.size.height-bottomBorder, rightBorder, bottomBorder);
    }
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+targetRect.size.width-rightBorder, targetRect.origin.y, rightBorder, bottomBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    
    // Center Left
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x, sourceRect.origin.y+topBorder, leftBorder, sourceRect.size.height-topBorder-bottomBorder);
    } 
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x, sourceRect.origin.y+bottomBorder, leftBorder, sourceRect.size.height-topBorder-bottomBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x, targetRect.origin.y+topBorder, leftBorder, targetRect.size.height-topBorder-bottomBorder);
    } 
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x, targetRect.origin.y+bottomBorder, leftBorder, targetRect.size.height-topBorder-bottomBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    
    // Center Center
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+leftBorder, sourceRect.origin.y+topBorder, sourceRect.size.width-rightBorder-leftBorder, sourceRect.size.height-topBorder-bottomBorder);
    }
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+leftBorder, sourceRect.origin.y+bottomBorder, sourceRect.size.width-rightBorder-leftBorder, sourceRect.size.height-topBorder-bottomBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+leftBorder, targetRect.origin.y+topBorder, targetRect.size.width-leftBorder-rightBorder, targetRect.size.height-topBorder-bottomBorder);
    } 
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+leftBorder, targetRect.origin.y+bottomBorder, targetRect.size.width-leftBorder-rightBorder, targetRect.size.height-topBorder-bottomBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    
    // Center Right
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+sourceRect.size.width-rightBorder, sourceRect.origin.y+topBorder, rightBorder, sourceRect.size.height-topBorder-bottomBorder);
    } 
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+sourceRect.size.width-rightBorder, sourceRect.origin.y+bottomBorder, rightBorder, sourceRect.size.height-topBorder-bottomBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+targetRect.size.width-rightBorder, targetRect.origin.y+topBorder, rightBorder, targetRect.size.height-topBorder-bottomBorder);
    } 
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+targetRect.size.width-rightBorder, targetRect.origin.y+bottomBorder, rightBorder, targetRect.size.height-topBorder-bottomBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    
    // Top Left
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x, sourceRect.origin.y, leftBorder, topBorder);
    }
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x, sourceRect.origin.y+sourceRect.size.height-topBorder, leftBorder, topBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x, targetRect.origin.y, leftBorder, topBorder);
    } 
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x, targetRect.origin.y+targetRect.size.height-topBorder, leftBorder, topBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    // Top Center
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+leftBorder, sourceRect.origin.y, sourceRect.size.width-rightBorder-leftBorder, topBorder);
    } 
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+leftBorder, sourceRect.origin.y+sourceRect.size.height-topBorder, sourceRect.size.width-rightBorder-leftBorder, topBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+leftBorder, targetRect.origin.y, targetRect.size.width-leftBorder-rightBorder, topBorder);
    }
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+leftBorder, targetRect.origin.y+targetRect.size.height-topBorder, targetRect.size.width-leftBorder-rightBorder, topBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
    
    // Top Right
    if(sourceFlipped)
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+sourceRect.size.width-rightBorder, sourceRect.origin.y, rightBorder, topBorder);
    } 
    else 
    {
        workingSourceRect = NSMakeRect(sourceRect.origin.x+sourceRect.size.width-rightBorder, sourceRect.origin.y+sourceRect.size.height-topBorder, rightBorder, topBorder);
    }
    
    if(targetFlipped)
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+targetRect.size.width-rightBorder, targetRect.origin.y, rightBorder, topBorder);
    }
    else 
    {
        workingTargetRect = NSMakeRect(targetRect.origin.x+targetRect.size.width-rightBorder, targetRect.origin.y+targetRect.size.height-topBorder, rightBorder, topBorder);
    }
    [self drawInRect:workingTargetRect fromRect:workingSourceRect operation:op fraction:frac respectFlipped:flipped hints:drawingHints];
}

- (NSImage*)subImageFromRect:(NSRect)rect
{
    NSImage *newImage = [[NSImage alloc] initWithSize:rect.size];
    
    [newImage lockFocus];
    [self drawInRect:NSMakeRect(0, 0, newImage.size.width, newImage.size.height) fromRect:rect operation:NSCompositeCopy fraction:1.0];
    [newImage unlockFocus];
    
    return [newImage autorelease];
}

- (void)setName:(NSString*)name forSubimageInRect:(NSRect)aRect
{
    if(!interfaceImages)
        interfaceImages = [[NSMutableArray alloc] init];
    
    NSImage *resultImage = [self subImageFromRect:aRect];
    [resultImage setName:name];
    
    [interfaceImages addObject:resultImage];
}
@end
