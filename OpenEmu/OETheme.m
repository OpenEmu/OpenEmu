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

#import "OETheme.h"
#import "OEThemeColor.h"
#import "OEThemeTextAttributes.h"
#import "OEThemeImage.h"
#import "OEThemeGradient.h"

#pragma mark - Theme.plist keys

static NSString * const OEThemeColorKey    = @"Colors";
static NSString * const OEThemeFontKey     = @"Fonts";
static NSString * const OEThemeImageKey    = @"Images";
static NSString * const OEThemeGradientKey = @"Gradients";

#pragma mark - Implementation

@interface OETheme ()

/// Dictionary of themed object types.
@property (copy) NSMutableDictionary <NSString *, NSDictionary <NSString *, id> *> *objectsByType;

- (NSDictionary *)OE_parseThemeSection:(NSDictionary <NSString *, id> *)section forThemeClass:(Class)class;
- (OEThemeObject *)OE_itemForType:(NSString *)type forKey:(NSString *)key;

@end

@implementation OETheme

- (instancetype)init {
    
    // Dealloc self if there is no Theme file to parse, the caller should raise a critical error here and halt the application's progress
    NSString *themeFile = [NSBundle.mainBundle pathForResource:@"Theme" ofType:@"plist"];
    if (!themeFile) {
        return nil;
    }

    if ((self = [super init])) {
        
        // Dealloc self if the Theme.plist failed to load, as in previous critical error, the application should halt at this point.
        NSDictionary *themeDictionary = [NSDictionary dictionaryWithContentsOfFile:themeFile];
        if (!themeDictionary) {
            return nil;
        }

        // Parse through all the types of UI elements: Colors, Fonts, Images, and Gradients.
        NSDictionary <NSString *, Class> *classesBySection = @{
            OEThemeColorKey    : [OEThemeColor class],
            OEThemeFontKey     : [OEThemeTextAttributes class],
            OEThemeImageKey    : [OEThemeImage class],
            OEThemeGradientKey : [OEThemeGradient class]
        };

        __block NSMutableDictionary <NSString *, id> *itemsByType = [NSMutableDictionary dictionary];
        [classesBySection enumerateKeysAndObjectsUsingBlock:^(NSString *key, Class themeClass, BOOL *stop) {
            NSDictionary <NSString *, id> *items = [self OE_parseThemeSection:themeDictionary[key] forThemeClass:themeClass];
            itemsByType[key] = items ?: @{};
        }];

        self.objectsByType = [itemsByType copy];
    }
    return self;
}

+ (OETheme *)sharedTheme {
    
    static OETheme *sharedTheme = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedTheme = [[OETheme alloc] init];
    });

    return sharedTheme;
}

- (NSDictionary <NSString *, id> *)OE_parseThemeSection:(NSDictionary *)sectionDictionary forThemeClass:(Class)class {
    
    // Each type of UI element represented in the Theme.plist should have an associated subclass of OEThemeObject. OEThemeObject is responsible for parsing the elements specified in that section of the Theme.plist
    __block NSMutableDictionary <NSString *, id> *results = [NSMutableDictionary dictionary];
    [sectionDictionary enumerateKeysAndObjectsUsingBlock:^(NSString *key, id definition, BOOL *stop) {
        // Each subclass of OEThemeObject should implement a customized version of +parseWithDefinition: to be able to parse the definitions
        id themeItem = [[class alloc] initWithDefinition:definition];
        if (themeItem) {
            results[key] = themeItem;
        }
     }];

    return [results copy];
}

- (id)OE_itemForType:(NSString *)type forKey:(NSString *)key {
    return self.objectsByType[type][key];
}

- (OEThemeColor *)themeColorForKey:(NSString *)key {
    return (OEThemeColor *)[self OE_itemForType:OEThemeColorKey forKey:key];
}

- (NSColor *)colorForKey:(NSString *)key forState:(OEThemeState)state {
    return [[self themeColorForKey:key] colorForState:state];
}

- (OEThemeTextAttributes *)themeTextAttributesForKey:(NSString *)key {
    return (OEThemeTextAttributes *)[self OE_itemForType:OEThemeFontKey forKey:key];
}

- (NSDictionary <NSString *, id> *)textAttributesForKey:(NSString *)key forState:(OEThemeState)state {
    return [[self themeTextAttributesForKey:key] textAttributesForState:state];
}

- (OEThemeImage *)themeImageForKey:(NSString *)key {
    return (OEThemeImage *)[self OE_itemForType:OEThemeImageKey forKey:key];
}

- (NSImage *)imageForKey:(NSString *)key forState:(OEThemeState)state {
    return [[self themeImageForKey:key] imageForState:state];
}

- (OEThemeGradient *)themeGradientForKey:(NSString *)key {
    return (OEThemeGradient *)[self OE_itemForType:OEThemeGradientKey forKey:key];
}

- (NSGradient *)gradientForKey:(NSString *)key forState:(OEThemeState)state {
    return [[self themeGradientForKey:key] gradientForState:state];
}

@end
