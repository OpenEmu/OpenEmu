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

#import "NSColor+OEAdditions.h"

// Note: this file is compiled under MRR. It cannot be ARC because there is no supported way
// to return an autoreleased CF object under ARC.

#if __has_feature(objc_arc)
#error This file cannot be compiled with ARC
#endif

static NSColor *_OENSColorFromRGBA(NSArray *parameters);
static NSColor *_OENSColorFromHSLA(NSArray *parameters);
static NSColor *_OENSColorFromString(NSString *colorString);

@implementation NSColor (OEAdditions)

+ (NSColor *)colorWithCGColor:(CGColorRef)color
{
    const CGFloat *components = CGColorGetComponents(color);
    NSColorSpace  *colorSpace = [[NSColorSpace alloc] initWithCGColorSpace:CGColorGetColorSpace(color)];
    NSColor       *result     = [NSColor colorWithColorSpace:colorSpace components:components count:CGColorGetNumberOfComponents(color)];
    [colorSpace release];

    return result;
}

- (CGColorRef)CGColor
{
    if([self isEqualTo:[NSColor blackColor]]) return CGColorGetConstantColor(kCGColorBlack);
    if([self isEqualTo:[NSColor whiteColor]]) return CGColorGetConstantColor(kCGColorWhite);
    if([self isEqualTo:[NSColor clearColor]]) return CGColorGetConstantColor(kCGColorClear);

    NSColor *rgbColor = [self colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
    CGFloat components[4];
    [rgbColor getComponents:components];

    CGColorSpaceRef theColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGColorRef      theColor      = CGColorCreate(theColorSpace, components);
    CGColorSpaceRelease(theColorSpace);

    return (CGColorRef)[(id)theColor autorelease];
}

@end

NSColor *_OENSColorFromRGBA(NSArray *parameters)
{
    if([parameters count] < 3) return nil;

    CGFloat red   = MAX(MIN([[parameters objectAtIndex:0] intValue], 255), 0) / 255.0;
    CGFloat green = MAX(MIN([[parameters objectAtIndex:1] intValue], 255), 0) / 255.0;
    CGFloat blue  = MAX(MIN([[parameters objectAtIndex:2] intValue], 255), 0) / 255.0;
    CGFloat alpha = ([parameters count] > 3 ? MAX(MIN([[parameters objectAtIndex:3] floatValue], 1.0), 0.0) : 1.0);

    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

NSColor *_OENSColorFromHSLA(NSArray *parameters)
{
    if([parameters count] < 3) return nil;

    CGFloat hue        = MAX(MIN([[parameters objectAtIndex:0] intValue], 360), 0) / 360.0;
    CGFloat saturation = MAX(MIN([[parameters objectAtIndex:1] intValue], 100), 0) / 100.0;
    CGFloat brightness = MAX(MIN([[parameters objectAtIndex:2] intValue], 100), 0) / 100.0;
    CGFloat alpha      = ([parameters count] > 3 ? MAX(MIN([[parameters objectAtIndex:3] floatValue], 1.0), 0.0) : 1.0);

    return [NSColor colorWithCalibratedHue:hue saturation:saturation brightness:brightness alpha:alpha];
}

NSColor *_OENSColorFromString(NSString *colorString)
{
    if(colorString == nil) return nil;

    // Should be static?
    NSRegularExpression  *rgbRegex      = [NSRegularExpression regularExpressionWithPattern:@"\\s*(?:(?:#)|(?:0x))?([0-9a-f]+)\\s*" options:0 error:nil];
    NSRegularExpression  *functionRegex = [NSRegularExpression regularExpressionWithPattern:@"\\s*(.+)\\(((?:[^,]+,){2,3}[^,]+)\\)\\s*" options:0 error:nil];

    const NSRange         range  = NSMakeRange(0, [colorString length]);
    NSTextCheckingResult *match  = nil;
    NSColor              *result = nil;

    if((match = [[functionRegex matchesInString:colorString options:0 range:range] lastObject]))
    {
        NSString *function  = [[colorString substringWithRange:[match rangeAtIndex:1]] lowercaseString];
        NSArray *parameters = [[[colorString substringWithRange:[match rangeAtIndex:2]] lowercaseString] componentsSeparatedByString:@","];

        if([function isEqualToString:@"rgba"] || [function isEqualToString:@"rgb"])      result = _OENSColorFromRGBA(parameters);
        else if([function isEqualToString:@"hsla"] || [function isEqualToString:@"hsl"]) result = _OENSColorFromHSLA(parameters);
    }
    else if((match = [[rgbRegex matchesInString:colorString options:0 range:range] lastObject]))
    {
        const NSRange matchRange = [match rangeAtIndex:1];
        NSString *matchedString  = [[colorString substringWithRange:matchRange] lowercaseString];

        switch(matchRange.length)
        {
            case 3: // rgb format
            case 4: // rgba format
            {
                CGFloat red   = [matchedString characterAtIndex:0] / 255.0;
                CGFloat green = [matchedString characterAtIndex:1] / 255.0;
                CGFloat blue  = [matchedString characterAtIndex:2] / 255.0;
                CGFloat alpha = (matchRange.length == 4 ? [matchedString characterAtIndex:3] / 255.0 : 1.0);

                result = [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];

                break;
            }
            case 6: // rrggbb format
                matchedString = [matchedString stringByAppendingFormat:@"ff"];
                break;
            case 8: // rrggbbaa format
                break;
            default:
                if(matchRange.length > 8) matchedString = [matchedString substringToIndex:8];
                break;
        }

        if(result == nil)
        {
            long long unsigned int colorARGB = 0;

            NSScanner *hexScanner = [NSScanner scannerWithString:matchedString];
            [hexScanner scanHexLongLong:&colorARGB];

            const CGFloat components[] =
            {
                (CGFloat)((colorARGB & 0xFF000000) >> 24) / 255.0f, // r
                (CGFloat)((colorARGB & 0x00FF0000) >> 16) / 255.0f, // g
                (CGFloat)((colorARGB & 0x0000FF00) >>  8) / 255.0f, // b
                (CGFloat)((colorARGB & 0x000000FF) >>  0) / 255.0f  // a
            };

            result = [NSColor colorWithColorSpace:[NSColorSpace genericRGBColorSpace] components:components count:4];
        }
    }
    return result;
}

// Inspired by http://www.w3.org/TR/css3-color/ and https://github.com/kballard/uicolor-utilities/blob/master/UIColor-Expanded.m
NSColor *OENSColorFromString(NSString *colorString)
{
    static NSDictionary    *namedColors = nil;
    static dispatch_once_t  onceToken;
    dispatch_once(&onceToken, ^{
        // Mac OS X defined colors
        namedColors = [[NSMutableDictionary alloc] initWithObjectsAndKeys:
                       [NSColor clearColor],                        @"clear",
                       [NSColor alternateSelectedControlColor],     @"alternateselectedcontrol",
                       [NSColor alternateSelectedControlTextColor], @"alternateselectedcontroltext",
                       [NSColor controlBackgroundColor],            @"controlbackground",
                       [NSColor controlColor],                      @"control",
                       [NSColor controlHighlightColor],             @"controlhighlight",
                       [NSColor controlLightHighlightColor],        @"controllighthighlight",
                       [NSColor controlShadowColor],                @"controlshadow",
                       [NSColor controlDarkShadowColor],            @"controldarkshadow",
                       [NSColor controlTextColor],                  @"controltext",
                       [NSColor disabledControlTextColor],          @"disabledcontroltext",
                       [NSColor gridColor],                         @"grid",
                       [NSColor headerColor],                       @"header",
                       [NSColor headerTextColor],                   @"headertext",
                       [NSColor highlightColor],                    @"highlight",
                       [NSColor keyboardFocusIndicatorColor],       @"keyboardfocusindicator",
                       [NSColor knobColor],                         @"knob",
                       [NSColor scrollBarColor],                    @"scrollbar",
                       [NSColor secondarySelectedControlColor],     @"secondaryselectedcontrol",
                       [NSColor selectedControlColor],              @"selectedcontrol",
                       [NSColor selectedControlTextColor],          @"selectedcontroltext",
                       [NSColor selectedMenuItemColor],             @"selectedmenuitem",
                       [NSColor selectedMenuItemTextColor],         @"selectedmenuitemtext",
                       [NSColor selectedTextBackgroundColor],       @"selectedtextbackground",
                       [NSColor selectedTextColor],                 @"selectedtext",
                       [NSColor selectedKnobColor],                 @"selectedknob",
                       [NSColor shadowColor],                       @"shadow",
                       [NSColor textBackgroundColor],               @"textbackground",
                       [NSColor textColor],                         @"text",
                       [NSColor windowBackgroundColor],             @"windowbackground",
                       [NSColor windowFrameColor],                  @"windowframe",
                       [NSColor windowFrameTextColor],              @"windowframetext",
                       nil];

        // CSS3 named colors
        static const char *colorNameDB =
        "aliceblue=#f0f8ff;antiquewhite=#faebd7;aqua=#00ffff;aquamarine=#7fffd4;azure=#f0ffff;"
        "beige=#f5f5dc;bisque=#ffe4c4;black=#000000;blanchedalmond=#ffebcd;blue=#0000ff;"
        "blueviolet=#8a2be2;brown=#a52a2a;burlywood=#deb887;cadetblue=#5f9ea0;chartreuse=#7fff00;"
        "chocolate=#d2691e;coral=#ff7f50;cornflowerblue=#6495ed;cornsilk=#fff8dc;crimson=#dc143c;"
        "cyan=#00ffff;darkblue=#00008b;darkcyan=#008b8b;darkgoldenrod=#b8860b;darkgray=#a9a9a9;"
        "darkgreen=#006400;darkgrey=#a9a9a9;darkkhaki=#bdb76b;darkmagenta=#8b008b;"
        "darkolivegreen=#556b2f;darkorange=#ff8c00;darkorchid=#9932cc;darkred=#8b0000;"
        "darksalmon=#e9967a;darkseagreen=#8fbc8f;darkslateblue=#483d8b;darkslategray=#2f4f4f;"
        "darkslategrey=#2f4f4f;darkturquoise=#00ced1;darkviolet=#9400d3;deeppink=#ff1493;"
        "deepskyblue=#00bfff;dimgray=#696969;dimgrey=#696969;dodgerblue=#1e90ff;"
        "firebrick=#b22222;floralwhite=#fffaf0;forestgreen=#228b22;fuchsia=#ff00ff;"
        "gainsboro=#dcdcdc;ghostwhite=#f8f8ff;gold=#ffd700;goldenrod=#daa520;gray=#808080;"
        "green=#008000;greenyellow=#adff2f;grey=#808080;honeydew=#f0fff0;hotpink=#ff69b4;"
        "indianred=#cd5c5c;indigo=#4b0082;ivory=#fffff0;khaki=#f0e68c;lavender=#e6e6fa;"
        "lavenderblush=#fff0f5;lawngreen=#7cfc00;lemonchiffon=#fffacd;lightblue=#add8e6;"
        "lightcoral=#f08080;lightcyan=#e0ffff;lightgoldenrodyellow=#fafad2;lightgray=#d3d3d3;"
        "lightgreen=#90ee90;lightgrey=#d3d3d3;lightpink=#ffb6c1;lightsalmon=#ffa07a;"
        "lightseagreen=#20b2aa;lightskyblue=#87cefa;lightslategray=#778899;"
        "lightslategrey=#778899;lightsteelblue=#b0c4de;lightyellow=#ffffe0;lime=#00ff00;"
        "limegreen=#32cd32;linen=#faf0e6;magenta=#ff00ff;maroon=#800000;mediumaquamarine=#66cdaa;"
        "mediumblue=#0000cd;mediumorchid=#ba55d3;mediumpurple=#9370db;mediumseagreen=#3cb371;"
        "mediumslateblue=#7b68ee;mediumspringgreen=#00fa9a;mediumturquoise=#48d1cc;"
        "mediumvioletred=#c71585;midnightblue=#191970;mintcream=#f5fffa;mistyrose=#ffe4e1;"
        "moccasin=#ffe4b5;navajowhite=#ffdead;navy=#000080;oldlace=#fdf5e6;olive=#808000;"
        "olivedrab=#6b8e23;orange=#ffa500;orangered=#ff4500;orchid=#da70d6;palegoldenrod=#eee8aa;"
        "palegreen=#98fb98;paleturquoise=#afeeee;palevioletred=#db7093;papayawhip=#ffefd5;"
        "peachpuff=#ffdab9;peru=#cd853f;pink=#ffc0cb;plum=#dda0dd;powderblue=#b0e0e6;"
        "purple=#800080;red=#ff0000;rosybrown=#bc8f8f;royalblue=#4169e1;saddlebrown=#8b4513;"
        "salmon=#fa8072;sandybrown=#f4a460;seagreen=#2e8b57;seashell=#fff5ee;sienna=#a0522d;"
        "silver=#c0c0c0;skyblue=#87ceeb;slateblue=#6a5acd;slategray=#708090;slategrey=#708090;"
        "snow=#fffafa;springgreen=#00ff7f;steelblue=#4682b4;tan=#d2b48c;teal=#008080;"
        "thistle=#d8bfd8;tomato=#ff6347;turquoise=#40e0d0;violet=#ee82ee;wheat=#f5deb3;"
        "white=#ffffff;whitesmoke=#f5f5f5;yellow=#ffff00;yellowgreen=#9acd32;";

        const char *lineSeparator  = ";";
        const char *valueSeperator = "=";

        char *names = NULL;
        char *line, *name, *value, *brkt;

        @try
        {
            if((names = malloc(strlen(colorNameDB))) != NULL)
            {
                memcpy(names, colorNameDB, strlen(colorNameDB));

                NSString *colorName  = nil;
                NSColor  *colorValue = nil;

                for(line = strtok_r(names, lineSeparator, &brkt); line; line = strtok_r(NULL, lineSeparator, &brkt))
                {
                    name = strtok_r(line, valueSeperator, &value);
                    if(name != NULL && value != NULL)
                    {
                        colorName  = [[NSString stringWithCString:name encoding:NSUTF8StringEncoding] lowercaseString];
                        colorValue = _OENSColorFromString([NSString stringWithCString:value encoding:NSUTF8StringEncoding]);

                        if(colorName != nil && colorValue != nil) [namedColors setValue:colorValue forKey:colorName];
                    }
                }
            }
        }
        @finally
        {
            if(names != NULL) free(names);
        }
    });

    if(!colorString) return nil;

    NSColor *result = [namedColors valueForKey:[[colorString lowercaseString] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]];
    return (result ?: _OENSColorFromString(colorString));
}
