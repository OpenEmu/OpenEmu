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

#import "OEThemeImage.h"

#import "OpenEmu-Swift.h"

#pragma mark - Theme image attributes

static NSString * const OEThemeImageResourceAttributeName = @"Resource";
static NSString * const OEThemeImagePartsAttributeName    = @"Parts";
static NSString * const OEThemeImageVerticalAttributeName = @"Vertical";

#pragma mark - Implementation

@implementation OEThemeImage

+ (id)parseWithDefinition:(NSDictionary *)definition {
    
    NSString *resource = definition[OEThemeImageResourceAttributeName];
    if (resource == nil) {
        return nil;
    }

    id parts = ([definition objectForKey:OEThemeImagePartsAttributeName] ?: definition[OEThemeObjectValueAttributeName]);
    BOOL vertical = ((NSNumber *)definition[OEThemeImageVerticalAttributeName]).boolValue;

    if ([parts isKindOfClass:[NSString class]]) {
        parts = @[ parts ];
    } else if (![parts isKindOfClass:[NSArray class]]) {
        parts = nil;
    }

    return [[NSImage imageNamed:resource] imageFromParts:parts vertical:vertical];
}

- (NSImage *)imageForState:(OEThemeState)state {
    return (NSImage *)[self objectForState:state];
}

@end
