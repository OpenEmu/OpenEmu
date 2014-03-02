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

#import "OEToolTipManager.h"
#import "OETheme.h"
#import <objc/objc-runtime.h>

NSString * const OEDefaultStyleToolTipThemeKey = @"default_tooltip";
NSString * const OEHUDStyleToolTipThemeKey     = @"hud_tooltip";

@interface NSToolTip : NSObject
{
@public
    NSView *view;
}
@end
 
@interface NSToolTipManager (Original)
// Names of methods after we relinked them
- (id)originalToolTipBackgroundColor;
- (id)originalToolTipTextColor;
- (id)originalToolTipAttributes;
- (void)originalDisplayToolTip:(NSToolTip*)tooltip;
- (id)toolTipAttributes;
@end

@implementation OEToolTipManager

+ (void)initialize
{
    if(self != [OEToolTipManager class]) return;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [self OE_replaceNSTooltipManagerSelector:@selector(toolTipBackgroundColor) offerOriginalAs:@selector(originalToolTipBackgroundColor)];
        [self OE_replaceNSTooltipManagerSelector:@selector(toolTipTextColor) offerOriginalAs:@selector(originalToolTipTextColor)];
        [self OE_replaceNSTooltipManagerSelector:@selector(displayToolTip:) offerOriginalAs:@selector(originalDisplayToolTip:)];
        [self OE_replaceNSTooltipManagerSelector:@selector(toolTipAttributes) offerOriginalAs:@selector(originalToolTipAttributes)];
    });
}

+ (void)OE_replaceNSTooltipManagerSelector:(SEL)currentSelector offerOriginalAs:(SEL)newSelector
{
    Class originalClass = [NSToolTipManager class];
    Method originalMethod = class_getInstanceMethod(originalClass, currentSelector);
    IMP originalImplementation = method_getImplementation(originalMethod);
    const char *typeEncoding = method_getTypeEncoding(originalMethod);
    
    class_addMethod(originalClass, newSelector, originalImplementation, typeEncoding);
    class_addMethod(originalClass, currentSelector, originalImplementation, typeEncoding);
    
    originalMethod = class_getInstanceMethod(originalClass, currentSelector);
    Method customMethod = class_getInstanceMethod(self, currentSelector);
    method_exchangeImplementations(originalMethod, customMethod);
}

+ (id)sharedToolTipManager
{
    static OEToolTipManager *sharedToolTipManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedToolTipManager = [[self alloc] init];
    });
    return sharedToolTipManager;
}

#pragma mark -

- (NSString *)OE_themeKeyForStyle:(OEToolTipStyle)style
{
    switch(style)
    {
        case OEToolTipStyleDefault : return OEDefaultStyleToolTipThemeKey;
        case OEToolTipStyleHUD     : return OEHUDStyleToolTipThemeKey;
    }
    return nil;
}

- (NSColor *)toolTipBackgroundColorForCurrentStyle
{
    NSString *key   = [[self OE_themeKeyForStyle:[self currentStyle]] stringByAppendingString:@"_background"];
    NSColor  *color = [[OETheme sharedTheme] colorForKey:key forState:OEThemeStateDefault];
    return color ?: [[NSToolTipManager sharedToolTipManager] originalToolTipBackgroundColor];
}

- (NSColor *)toolTipTextColorForCurrentStyle
{
    NSString     *key        = [self OE_themeKeyForStyle:[self currentStyle]];
    NSDictionary *attributes = [[OETheme sharedTheme] textAttributesForKey:key forState:OEThemeStateDefault];
    
    return [attributes objectForKey:NSForegroundColorAttributeName] ? : [[NSToolTipManager sharedToolTipManager] originalToolTipTextColor];
}

- (NSColor *)toolTipAttributesForCurrentStyle
{
    NSString     *key        = [self OE_themeKeyForStyle:[self currentStyle]];
    NSDictionary *attributes = [[OETheme sharedTheme] textAttributesForKey:key forState:OEThemeStateDefault];
    
    return attributes ? : [[NSToolTipManager sharedToolTipManager] originalToolTipAttributes];
}

#pragma mark -

- (void)displayToolTip:(id)tooltip
{
    NSView *view = ((NSToolTip *)tooltip)->view;
    OEToolTipStyle style = OEToolTipStyleDefault;
    if([view conformsToProtocol:@protocol(OEToolTip)])
        style = [(id <OEToolTip>)view toolTipStyle];

    [[OEToolTipManager sharedToolTipManager] setCurrentStyle:style];
    [super originalDisplayToolTip:tooltip];
}

- (id)toolTipBackgroundColor
{
    return [[OEToolTipManager sharedToolTipManager] toolTipBackgroundColorForCurrentStyle];
}

- (id)toolTipTextColor
{
    return [[OEToolTipManager sharedToolTipManager] toolTipTextColorForCurrentStyle];
}

- (id)toolTipAttributes
{
    return [[OEToolTipManager sharedToolTipManager] toolTipAttributesForCurrentStyle];
}
@end
