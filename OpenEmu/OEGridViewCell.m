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

@interface OEGridViewCell (Private)

- (void)_reorderLayers;

@end

#pragma mark -
@implementation OEGridViewCell

#pragma mark - NSObject
- (id)init
{
    if(!(self = [super init]))
        return nil;

    [self setNeedsDisplayOnBoundsChange:YES];
    [self setLayoutManager:[OEGridViewLayoutManager layoutManager]];
    [self setInteractive:YES];

    return self;
}

#pragma mark - CALayer
- (void)addSublayer:(CALayer *)layer
{
    [super addSublayer:layer];
    [self _reorderLayers];
}

- (void)insertSublayer:(CALayer *)layer atIndex:(unsigned int)idx
{
    [super insertSublayer:layer atIndex:idx];
    [self _reorderLayers];
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
    if(CGRectContainsPoint([self frame], p))
        return self;

    return nil;
}

#pragma mark - OEGridViewCell
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

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    if(_selected == selected)
        return;

    _selected = selected;
}

#pragma mark - Properties
- (id)draggingImage
{
    const CGSize imageSize = [self bounds].size;

    NSBitmapImageRep *dragImageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                             pixelsWide:imageSize.width
                                                                             pixelsHigh:imageSize.height
                                                                          bitsPerSample:8
                                                                        samplesPerPixel:4
                                                                               hasAlpha:YES
                                                                               isPlanar:NO
                                                                         colorSpaceName:NSCalibratedRGBColorSpace
                                                                            bytesPerRow:(NSInteger)imageSize.width * 4
                                                                           bitsPerPixel:32];
    NSGraphicsContext *bitmapContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:dragImageRep];
    CGContextRef ctx = (CGContextRef)[bitmapContext graphicsPort];

    if(![self superlayer])
        CGContextConcatCTM(ctx, CGAffineTransformMake(1, 0, 0, -1, 0, imageSize.height));

    CGContextClearRect(ctx, CGRectMake(0.0, 0.0, imageSize.width, imageSize.height));
    CGContextSetAllowsAntialiasing(ctx, YES);
    [self renderInContext:ctx];
    CGContextFlush(ctx);

    NSImage *dragImage = [[NSImage alloc] initWithSize:NSSizeFromCGSize(imageSize)];
    [dragImage addRepresentation:dragImageRep];
    [dragImage setFlipped:YES];

    return dragImage;
}

- (void)setSelected:(BOOL)selected
{
    [self setSelected:selected animated:NO];
}

- (BOOL)isSelected
{
    return _selected;
}

- (void)setEditing:(BOOL)editing
{
    if(_editing == editing)
        return;

    if(editing)
        [[self gridView] _willBeginEditingCell:self];
    else
        [[self gridView] _didEndEditingCell:self];

    _editing = editing;
}

- (BOOL)isEditing
{
    return _editing;
}

- (void)setForegroundLayer:(CALayer *)foregroundLayer
{
    if(_foregroundLayer == foregroundLayer)
        return;

    [_foregroundLayer removeFromSuperlayer];
    _foregroundLayer = foregroundLayer;

    [self _reorderLayers];
}

- (CALayer *)foregroundLayer
{
    return _foregroundLayer;
}

- (OEGridView *)gridView
{
    id superlayerDelegate = [[self superlayer] delegate];
    if([superlayerDelegate isKindOfClass:[OEGridView class]])
        return (OEGridView *)superlayerDelegate;

    return nil;
}

- (NSRect)hitRect
{
    return NSRectFromCGRect([self frame]);
}

@end

#pragma mark -
@implementation OEGridViewCell (OEGridView)

- (void)_setIndex:(NSUInteger)index
{
    _index = index;
}

- (NSUInteger)_index
{
    return _index;
}

@end

#pragma mark -
@implementation OEGridViewCell (Private)

- (void)_reorderLayers
{
    [super insertSublayer:_foregroundLayer atIndex:(unsigned int)[[self sublayers] count]];
}

@end
