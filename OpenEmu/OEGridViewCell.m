/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OEGridViewCell.h"
#import "OEGridView.h"
#import "OEGridView+OEGridViewCell.h"

@interface OEGridViewCell ()

- (void)OE_reorderLayers;

@end

@implementation OEGridViewCell

@synthesize selected=_selected;
@synthesize editing=_editing;
@synthesize foregroundLayer=_foregroundLayer;

- (id)init
{
    if((self = [super init]))
    {
        [self setNeedsDisplayOnBoundsChange:YES];
        [self setLayoutManager:[OEGridViewLayoutManager layoutManager]];
        [self setInteractive:YES];
    }
    
    return self;
}

- (void)addSublayer:(CALayer *)layer
{
    [super addSublayer:layer];
    [self OE_reorderLayers];
}

- (void)insertSublayer:(CALayer *)layer atIndex:(unsigned int)idx
{
    [super insertSublayer:layer atIndex:idx];
    [self OE_reorderLayers];
}

- (void)layoutSublayers
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    [_foregroundLayer setFrame:[self bounds]];
    [CATransaction commit];
}

- (CALayer *)hitTest:(CGPoint)p
{
    return CGRectContainsPoint([self frame], p) ? self : nil;
}

- (void)prepareForReuse
{
    [self setTracking:NO];
    [self setEditing:NO];
    [self setSelected:NO];
    [self setHidden:NO];
    [self setOpacity:1.0];
    [self setShadowOpacity:0.0];
}

- (void)didBecomeFocused
{
}

- (void)willResignFocus
{
}

#pragma mark -
#pragma mark Properties

- (id)draggingImage
{
    const CGSize       imageSize     = [self bounds].size;
    NSBitmapImageRep  *dragImageRep  = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:imageSize.width pixelsHigh:imageSize.height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bytesPerRow:(NSInteger)ceil(imageSize.width) * 4 bitsPerPixel:32];
    NSGraphicsContext *bitmapContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:dragImageRep];
    CGContextRef       ctx           = (CGContextRef)[bitmapContext graphicsPort];

    if([self superlayer] == nil) CGContextConcatCTM(ctx, CGAffineTransformMake(1.0, 0.0, 0.0, -1.0, 0.0, imageSize.height));

    CGContextClearRect(ctx, CGRectMake(0.0, 0.0, imageSize.width, imageSize.height));
    CGContextSetAllowsAntialiasing(ctx, YES);
    [self renderInContext:ctx];
    CGContextFlush(ctx);

    NSImage *dragImage = [[NSImage alloc] initWithSize:imageSize];
    [dragImage addRepresentation:dragImageRep];
    [dragImage setFlipped:YES];

    return dragImage;
}

- (void)setSelected:(BOOL)selected
{
    [self setSelected:selected animated:NO];
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    _selected = selected;
}

- (void)setEditing:(BOOL)editing
{
    if(_editing != editing)
    {
        if(editing)  [[self gridView] OE_willBeginEditingCell:self];
        else         [[self gridView] OE_didEndEditingCell:self];
        _editing = editing;
    }
}

- (void)OE_reorderLayers
{
    [super insertSublayer:_foregroundLayer atIndex:[[self sublayers] count]];
}

- (void)setForegroundLayer:(CALayer *)foregroundLayer
{
    if(_foregroundLayer != foregroundLayer)
    {
        [_foregroundLayer removeFromSuperlayer];
        _foregroundLayer = foregroundLayer;

        [self OE_reorderLayers];
    }
}

- (OEGridView *)gridView
{
    OEGridView *superlayerDelegate = [[self superlayer] delegate];
    return [superlayerDelegate isKindOfClass:[OEGridView class]] ? superlayerDelegate : nil;
}

- (NSRect)hitRect
{
    return [self bounds];
}

- (void)OE_setIndex:(NSUInteger)index
{
    _index = index;
}

- (NSUInteger)OE_index
{
    return _index;
}

@end
