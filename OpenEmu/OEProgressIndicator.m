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

#import "OEProgressIndicator.h"
#import "OETheme.h"
#import "OEThemeImage.h"
@import Quartz;

@interface OEProgressIndicator ()
{
    OEThemeImage *trackImage;
    OEThemeImage *barImage;
    OEThemeImage *candyImage;
}
@property BOOL animating;
@property (nonatomic)  float candyOffset;
@end
@implementation OEProgressIndicator
@synthesize backgroundThemeImage, themeTextAttributes, themeImage;

@synthesize trackWindowActivity = _trackWindowActivity;
@synthesize trackMouseActivity  = _trackMouseActivity;
@synthesize hovering = _hovering;
@synthesize themed   = _themed;
@synthesize stateMask = _stateMask;

- (id)init
{
    self = [super init];
    if (self) {
        [self setAnimating:NO];
    }
    return self;
}

- (BOOL)allowsVibrancy
{
    return false;
}

- (void)setCandyOffset:(float)candyOffset
{
    if(candyOffset == _candyOffset) return;
    
    [self willChangeValueForKey:@"candyOffset"];
    _candyOffset = (int)candyOffset%10;
    [self didChangeValueForKey:@"candyOffset"];
    
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect trackRect = [self trackRect];
    
    OEThemeState currentState = [self currentState];
    [[trackImage imageForState:currentState] drawInRect:trackRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    [NSGraphicsContext saveGraphicsState];

    [[NSGraphicsContext currentContext] setColorRenderingIntent:NSColorRenderingIntentDefault];
    
    NSRect maxCandyRect = [self candyRect];
    NSRectClip(maxCandyRect);
    NSRect candyRect = NSMakeRect([self candyOffset]-10, 2, 10, 10);
    while(NSMinX(candyRect) < NSMaxX(dirtyRect))
    {
        [[candyImage imageForState:currentState] drawInRect:candyRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
        candyRect.origin.x += NSWidth(candyRect);
    }
    [NSGraphicsContext restoreGraphicsState];
    if([self doubleValue] != [self minValue])
    {
        NSRect progressRect = [self progressRect];
        [[barImage imageForState:currentState] drawInRect:progressRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
}

- (void)startAnimation:(id)sender
{
    if([self animating]) return;
    
    [self setAnimating:YES];
    __unsafe_unretained __block dispatch_block_t recAnimationBlock;
    __block void (^animationBlock)(void) = [^{
        if([self animating])
        {
            [self setCandyOffset:[self candyOffset]+1.0];
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 1/30.0 * NSEC_PER_SEC);
            dispatch_after(popTime, dispatch_get_main_queue(), recAnimationBlock);
        }
    } copy];
    recAnimationBlock = animationBlock;
    dispatch_async(dispatch_get_main_queue(), animationBlock);
}

- (void)stopAnimation:(id)sender
{
    [self setAnimating:NO];
}

#pragma mark - parts
- (NSRect)trackRect
{
    NSRect bounds = [self bounds];
    NSRect result = bounds;

    OEThemeState currentState = [self currentState];
    result.size.height = [[trackImage imageForState:currentState] size].height;

    return result;
}

- (NSRect)candyRect
{
    NSRect result = [self bounds];
    result.size.height = 10.0;
    result.origin.y += 2.0;
    result.size.width -= 1.0;
    result = NSInsetRect(result, 1, 0);
    
    return result;
}

- (NSRect)progressRect
{
    NSRect result = [self bounds];

    OEThemeState currentState = [self currentState];
    NSSize barSize = [[barImage imageForState:currentState] size];
    result.size.height = barSize.height;

    double percentValue = (self.doubleValue-self.minValue)/(self.maxValue-self.minValue);
    float width = MAX([self bounds].size.width*percentValue, 5.0);
    result.size.width = roundf(width);
    if(result.size.width < barSize.width)
        result.size.width = result.size.width*2<barSize.width ? 0 : barSize.width;

    return result;
}

#pragma mark - Controling State
- (OEThemeState)currentState
{
    return [self isIndeterminate] ? OEThemeInputStateToggleOff : OEThemeInputStateToggleOn;
}

#pragma mark - OEControl Implementation -
- (void)setThemeKey:(NSString *)key
{
    trackImage = [[OETheme sharedTheme] themeImageForKey:[key stringByAppendingString:@"_track_background"]];
    barImage = [[OETheme sharedTheme] themeImageForKey:[key stringByAppendingString:@"_bar"]];
    candyImage = [[OETheme sharedTheme] themeImageForKey:[key stringByAppendingString:@"_candy"]];
    [self setNeedsDisplay:YES];
}

- (void)setBackgroundThemeImageKey:(NSString *)key{}
- (void)setThemeImageKey:(NSString *)key {}
- (void)setThemeTextAttributesKey:(NSString *)key {}
@end
