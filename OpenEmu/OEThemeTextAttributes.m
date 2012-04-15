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

#import "OEThemeTextAttributes.h"
#import "NSColor+OEAdditions.h"

#pragma mark -
#pragma mark Theme font attributes

static NSString * const OEThemeFontForegroundColorAttributeName = @"Color";
static NSString * const OEThemeFontBackgroundColorAttributeName = @"Background Color";

static NSString * const OEThemeFontFamilyAttributeName          = @"Family";
static NSString * const OEThemeFontSizeAttributeName            = @"Size";
static NSString * const OEThemeFontWeightAttributeName          = @"Weight";
static NSString * const OEThemeFontTraitsAttributeName          = @"Traits";

#pragma mark -
#pragma mark Theme font shadow

static NSString * const OEThemeFontShadowAttributeName          = @"Shadow";
static NSString * const OEThemeShadowOffsetAttributeName        = @"Offset";
static NSString * const OEThemeShadowBlurRadiusAttributeName    = @"Blur Radius";
static NSString * const OEThemeShadowColorAttributeName         = @"Color";

#pragma mark -
#pragma mark Theme font traits

static NSString * const OEThemeFontTraitBoldName                = @"Bold";
static NSString * const OEThemeFontTraitUnboldName              = @"Unbold";
static NSString * const OEThemeFontTraitItalicName              = @"Italic";
static NSString * const OEThemeFontTraitUnitalic                = @"Unitalic";

#pragma mark -
#pragma mark Implementation

static NSFontTraitMask _OENSFontTraitMaskFromString(NSString *string);
static id _OEObjectFromDictionary(NSDictionary *dictionary, NSString *attributeName, Class expectedClass, id (^parse)(id obj));

// Parses a comma separated NSString of theme font traits
NSFontTraitMask _OENSFontTraitMaskFromString(NSString *string)
{
    __block NSFontTraitMask mask = 0;

    [[string componentsSeparatedByString:@","] enumerateObjectsUsingBlock:
     ^ (NSString *obj, NSUInteger idx, BOOL *stop)
     {
         NSString *trait = [obj stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];

         if([trait caseInsensitiveCompare:OEThemeFontTraitBoldName])        mask |= NSBoldFontMask;
         else if([trait caseInsensitiveCompare:OEThemeFontTraitUnboldName]) mask |= NSUnboldFontMask;
         else if([trait caseInsensitiveCompare:OEThemeFontTraitItalicName]) mask |= NSItalicFontMask;
         else if([trait caseInsensitiveCompare:OEThemeFontTraitUnitalic])   mask |= NSUnitalicFontMask;
     }];

    return mask;
}

// This is a convenience function that allows us to retrieve an object from the specified dictionary
id _OEObjectFromDictionary(NSDictionary *dictionary, NSString *attributeName, Class expectedClass, id (^parse)(id obj))
{
    id obj = [dictionary objectForKey:attributeName];

    // If the object already conforms to the requested class, then return the object, otherwise parse the object
    return ([obj isKindOfClass:expectedClass] ? obj : parse(obj));
}

@implementation OEThemeTextAttributes

+ (id)parseWithDefinition:(NSDictionary *)definition
{
    // Parse the values from the new definition
    NSColor *foregroundColor = _OEObjectFromDictionary(definition, OEThemeFontForegroundColorAttributeName, [NSColor class],
                                                       ^ id (id color)
                                                       {
                                                           return ([color isKindOfClass:[NSString class]] ? (OENSColorFromString(color) ?: [NSColor blackColor]) : [NSColor blackColor]);
                                                       });

    NSColor *backgroundColor = _OEObjectFromDictionary(definition, OEThemeFontBackgroundColorAttributeName, [NSColor class],
                                                       ^ id (id color)
                                                       {
                                                           return ([color isKindOfClass:[NSString class]] ? (OENSColorFromString(color) ?: nil) : nil);
                                                       });

    NSShadow *shadow = _OEObjectFromDictionary(definition, OEThemeFontShadowAttributeName, [NSShadow class],
                                               ^ id (id shadow)
                                               {
                                                   if(![shadow isKindOfClass:[NSDictionary class]]) return nil;

                                                   NSSize  offset     = NSSizeFromString([shadow valueForKey:OEThemeShadowOffsetAttributeName]);
                                                   CGFloat blurRadius = [[shadow valueForKey:OEThemeShadowBlurRadiusAttributeName] floatValue];
                                                   id      color      = [shadow objectForKey:OEThemeShadowColorAttributeName];

                                                   if([color isKindOfClass:[NSString class]])      color = (OENSColorFromString(color) ?: [NSColor blackColor]);
                                                   else if(![color isKindOfClass:[NSColor class]]) color = [NSColor blackColor];

                                                   NSShadow *result = [[NSShadow alloc] init];
                                                   [result setShadowOffset:offset];
                                                   [result setShadowBlurRadius:blurRadius];
                                                   [result setShadowColor:color];

                                                   return result;
                                               });

    NSString   *familyAttribute = ([definition valueForKey:OEThemeFontFamilyAttributeName]   ?: [definition objectForKey:OEThemeObjectValueAttributeName]);
    CGFloat     size            = [([definition objectForKey:OEThemeFontSizeAttributeName]   ?: [NSNumber numberWithFloat:12.0]) floatValue];
    NSUInteger  weight          = [([definition objectForKey:OEThemeFontWeightAttributeName] ?: [NSNumber numberWithInt:5]) intValue];

    NSFontTraitMask  mask = [_OEObjectFromDictionary(definition, OEThemeFontTraitsAttributeName, [NSNumber class],
                                                     ^ id (id mask)
                                                     {
                                                         if(![mask isKindOfClass:[NSString class]]) return [NSNumber numberWithInt:0];
                                                         return [NSNumber numberWithUnsignedInteger:_OENSFontTraitMaskFromString(mask)];
                                                     }) unsignedIntegerValue];

    NSFont *font = [[NSFontManager sharedFontManager] fontWithFamily:familyAttribute traits:mask weight:weight size:size];

    NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
    if(font)            [attributes setValue:font            forKey:NSFontAttributeName];
    if(shadow)          [attributes setValue:shadow          forKey:NSShadowAttributeName];
    if(foregroundColor) [attributes setValue:foregroundColor forKey:NSForegroundColorAttributeName];
    if(backgroundColor) [attributes setValue:backgroundColor forKey:NSBackgroundColorAttributeName];

    return [attributes copy];
}

- (NSDictionary *)textAttributesForState:(OEThemeState)state
{
    return (NSDictionary *)[self objectForState:state];
}

@end
