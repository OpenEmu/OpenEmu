//
//  OETextField.m
//  OETheme
//
//  Created by Christoph Leimbrock on 13.10.12.
//  Copyright (c) 2012 OpenEmu. All rights reserved.
//

#import "OELabel.h"
#import "OELabelCell.h"
@implementation OELabel

+ (Class)cellClass
{
    return [OELabelCell class];
}

- (void)setThemeKey:(NSString *)key
{
    NSString *backgroundKey = key;
    if(![key hasSuffix:@"_background"])
    {
        [self setThemeImageKey:key];
        backgroundKey = [key stringByAppendingString:@"_background"];
    }
    [self setBackgroundThemeImageKey:backgroundKey];
    [self setThemeTextAttributesKey:key];
}

- (void)setBackgroundThemeImageKey:(NSString *)key
{
    [self setBackgroundThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setThemeImageKey:(NSString *)key
{
    [self setThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setThemeTextAttributesKey:(NSString *)key
{
    [self setThemeTextAttributes:[[OETheme sharedTheme] themeTextAttributesForKey:key]];
}

- (void)setBackgroundThemeImage:(OEThemeImage *)backgroundThemeImage
{
    OELabelCell *cell = [self cell];
    if([cell isKindOfClass:[OELabelCell class]])
    {
        [cell setBackgroundThemeImage:backgroundThemeImage];
        [self setNeedsDisplay];
    }
}

- (OEThemeImage *)backgroundThemeImage
{
    OELabelCell *cell = [self cell];
    return ([cell isKindOfClass:[OELabelCell class]] ? [cell backgroundThemeImage] : nil);
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
    OELabelCell *cell = [self cell];
    if([cell isKindOfClass:[OELabelCell class]])
    {
        [cell setThemeImage:themeImage];
        [self setNeedsDisplay];
    }
}

- (OEThemeImage *)themeImage
{
    OELabelCell *cell = [self cell];
    return ([cell isKindOfClass:[OELabelCell class]] ? [cell themeImage] : nil);
}

- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
    OELabelCell *cell = [self cell];
    if([cell isKindOfClass:[OELabelCell class]])
    {
        [cell setThemeTextAttributes:themeTextAttributes];
        [self setNeedsDisplay];
    }
}

- (OEThemeTextAttributes *)themeTextAttributes
{
    OELabelCell *cell = [self cell];
    return ([cell isKindOfClass:[OELabelCell class]] ? [cell themeTextAttributes] : nil);
}
@synthesize backgroundThemeImage=_backgroundThemeImage, themeImage=_themeImage, themeTextAttributes=_themeTextAttributes,trackWindowActivity=_trackWindowActivity,trackMouseActivity=_trackMouseActivity;
@end
