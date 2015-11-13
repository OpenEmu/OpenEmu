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

#import "OEBackgroundColorView.h"
#import "OETheme.h"
#import "OEThemeColor.h"

@interface OEBackgroundColorView ()
@property NSString *backgroundColorKey;
@end

@implementation OEBackgroundColorView

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

    [[self backgroundColor] setFill];
    NSRectFill(dirtyRect);
}

- (BOOL)isOpaque
{
    return [[self backgroundColor] alphaComponent] == 1.0;
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
