//
//  OEProgressIndicator.m
//  OETheme
//
//  Created by Christoph Leimbrock on 13.10.12.
//  Copyright (c) 2012 OpenEmu. All rights reserved.
//

#import "OEProgressIndicator.h"
#import <Quartz/Quartz.h>
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
    __block void (^animationBlock)(void) = ^{
        if([self animating])
        {
            [self setCandyOffset:[self candyOffset]+1.0];
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 1/30.0 * NSEC_PER_SEC);
            dispatch_after(popTime, dispatch_get_main_queue(), animationBlock);
        }
        else
            animationBlock = nil;
    };
    dispatch_async(dispatch_get_main_queue(), animationBlock);
}

- (void)stopAnimation:(id)sender
{
    [self setAnimating:NO];
}

#pragma mark - parts
- (NSRect)trackRect
{
    NSRect result = [self bounds];
    result.size.height = 13.0;
    
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
    result.size.height = 12.0;
    
    double percentValue = (self.doubleValue-self.minValue)/(self.maxValue-self.minValue);
    float width = MAX([self bounds].size.width*percentValue, 5.0);
    result.size.width = roundf(width);
    
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
}

- (void)setBackgroundThemeImageKey:(NSString *)key{}
- (void)setThemeImageKey:(NSString *)key {}
- (void)setThemeTextAttributesKey:(NSString *)key {}
@end
