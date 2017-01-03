/*
 Copyright (c) 2016, OpenEmu Team

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

#import "OEThemeGradient.h"
#import "NSColor+OEAdditions.h"

#pragma mark - Theme gradient attributes

static NSString * const OEThemeGradientLocationsAttributeName = @"Locations";
static NSString * const OEThemeGradientColorsAttributeName    = @"Colors";
static NSString * const OEThemeGradientAngleAttributeName     = @"Angle";

#pragma mark - Implementation

@interface OEGradient : NSGradient

/// Saves the angle specified by the theme definition.
@property (nonatomic) CGFloat angle;

@end

@implementation OEThemeGradient

+ (id)parseWithDefinition:(NSDictionary *)definition {
    
    NSArray *rawLocations = definition[OEThemeGradientLocationsAttributeName];
    NSArray *rawColorStrings = definition[OEThemeGradientColorsAttributeName];
    id angle = definition[OEThemeGradientAngleAttributeName];

    // Make sure that the gradient definition is well-formed (we should report errors)
    if ([rawLocations isKindOfClass:[NSString class]] || [rawLocations isKindOfClass:[NSNumber class]]) {
        rawLocations = @[ rawLocations ];
    } else if (![rawLocations isKindOfClass:[NSArray class]]) {                                             return nil;
    }

    if ([rawColorStrings isKindOfClass:[NSString class]] || [rawColorStrings isKindOfClass:[NSNumber class]]) {
        rawColorStrings = @[ rawColorStrings ];
    } else if (![rawColorStrings isKindOfClass:[NSArray class]]) {                                                return nil;
    }

    if (![angle isKindOfClass:[NSString class]] && ![angle isKindOfClass:[NSNumber class]]) {
        angle = nil;
    }

    // Make sure that there are color stops and colors
    if (rawLocations.count == 0 || rawColorStrings.count == 0 || rawLocations.count != rawColorStrings.count) {
        return nil;
    }

    // Translate color strings into NSColor
    id result = nil;
    NSMutableArray *colors = [NSMutableArray arrayWithCapacity:rawColorStrings.count];

    for (id obj in rawColorStrings) {
        [colors addObject:([NSColor colorFromString:obj] ?: NSColor.blackColor)];
    }

    // Translate NSNumber objects to CGFloats
    CGFloat *locations = NULL;
    @try {
        if ((locations = calloc(colors.count, sizeof(CGFloat))) != NULL) {
            [rawLocations enumerateObjectsUsingBlock:^(NSNumber *location, NSUInteger idx, BOOL *stop) {
                locations[idx] = location.floatValue;
            }];

            result = [[OEGradient alloc] initWithColors:colors atLocations:locations colorSpace:NSColorSpace.genericRGBColorSpace];
            ((OEGradient *)result).angle = [angle floatValue];
        }
    } @finally {
        if (locations != NULL) {
            free(locations);
        }
    }

    return result;
}

- (NSGradient *)gradientForState:(OEThemeState)state {
    return (NSGradient *)[self objectForState:state];
}

@end

@implementation NSGradient (OEThemeGradientAdditions)

- (void)drawInRect:(NSRect)rect {
    [self drawInRect:rect angle:0.0];
}

- (void)drawInBezierPath:(NSBezierPath *)path {
    [self drawInBezierPath:path angle:0.0];
}

@end

@implementation OEGradient

- (void)drawInRect:(NSRect)rect {
    [self drawInRect:rect angle:_angle];
}

- (void)drawInBezierPath:(NSBezierPath *)path {
    [self drawInBezierPath:path angle:_angle];
}

@end
