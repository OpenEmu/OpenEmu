//
//  OEBackgroundColorView.m
//  OETheme
//
//  Created by Christoph Leimbrock on 13.10.12.
//  Copyright (c) 2012 OpenEmu. All rights reserved.
//

#import "OEThemedBackgroundColorView.h"

@interface OEThemedBackgroundColorView ()
@property NSString *backgroundColorKey;
@end
@implementation OEThemedBackgroundColorView
- (void)drawRect:(NSRect)dirtyRect
{
    OEThemeColor *backgroundColor = [[OETheme sharedTheme] themeColorForKey:[self backgroundColorKey]];
    [[backgroundColor colorForState:OEThemeStateDefault] setFill];
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

@end
