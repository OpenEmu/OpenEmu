//
//  OEBackgroundColorView.m
//  OETheme
//
//  Created by Christoph Leimbrock on 13.10.12.
//  Copyright (c) 2012 OpenEmu. All rights reserved.
//

#import "OEBackgroundColorView.h"

@interface OEBackgroundColorView ()
@property NSString *backgroundColorKey;
@end
@implementation OEBackgroundColorView
- (void)drawRect:(NSRect)dirtyRect
{
    [[self backgroundColor] setFill];
    NSRectFill(dirtyRect);
}

#pragma mark - OEControl Implementation -
- (BOOL)isTrackingMouseActivity
{
    return NO;
}
- (BOOL)isTrackingWindowActivity
{
    return NO;
}

- (void)setThemeKey:(NSString *)key
{
    NSString *backgroundKey = key;
    if(![key hasSuffix:@"_background"])
    {
        backgroundKey = [key stringByAppendingString:@"_background"];
    }
    [self setBackgroundColorKey:backgroundKey];
    [self setNeedsDisplay:YES];
}
@synthesize backgroundThemeImage, themeImage, themeTextAttributes;

- (void)setThemeImage:(OEThemeImage *)themeImage
{}
- (void)setThemeImageKey:(NSString *)key
{}
- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{}
- (void)setThemeTextAttributesKey:(NSString *)key
{}
- (void)setBackgroundThemeImage:(OEThemeImage *)backgroundThemeImage
{}
- (void)setBackgroundThemeImageKey:(NSString *)key{}

- (OEThemeState)OE_currentState
{
    return OEThemeStateDefault;
}

- (NSColor*)backgroundColor
{
    if(_backgroundColor)
        return _backgroundColor;
    else
    {
        OEThemeColor *color = [[OETheme sharedTheme] themeColorForKey:[self backgroundColorKey]];
        return [color colorForState:[self OE_currentState]];
    }
}
@end
