/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "OEROMFileSystemTypeTransformer.h"


@implementation OEROMFileSystemTypeTransformer

+ (id)transformer
{
    return [[[self alloc] init] autorelease];
}

+ (BOOL)allowsReverseTransformation
{
    return YES;
}

- (NSString *)transformedValue:(NSNumber *)value
{
    // Will supports way more types than just NSNumber
    if(![value respondsToSelector:@selector(intValue)]) return @"Unknown";

#define OEHandleType(__type, __name) \
    case __type: \
        return (__name); \
        break;
    
    OEROMFileSystemType systemType = [value intValue];
    switch(systemType){
        OEHandleType(OEROMFileSystemTypeNES, @"NES");
        OEHandleType(OEROMFileSystemTypeSNES, @"SNES");
        OEHandleType(OEROMFileSystemTypeGenesis, @"Genesis");
        OEHandleType(OEROMFileSystemTypeGameBoy, @"GameBoy");
        OEHandleType(OEROMFileSystemTypeGameBoyAdvance, @"GameBoy Advance");
        OEHandleType(OEROMFileSystemTypeTurboGrafx16, @"TurboGrafx-16");
        OEHandleType(OEROMFileSystemTypeAtariLynx, @"Lynx");
        OEHandleType(OEROMFileSystemTypeNeoGeo, @"Neo Geo");
        case OEROMFileSystemTypeUnknown:
        default:
            return @"Unknown";
    }
#undef OEHandleType
    return @"Unknown";
}

- (NSNumber *)reverseTransformedValue:(NSString *)value
{
    if(![value isKindOfClass:[NSString class]]) return [NSNumber numberWithInt:OEROMFileSystemTypeUnknown];
    
#define OEHandleType(__type, __name) \
    if([value isEqualToString:(__name)]){ \
        return [NSNumber numberWithUnsignedInt:(__type)]; \
    }
    
    OEHandleType(OEROMFileSystemTypeNES, @"NES");
    OEHandleType(OEROMFileSystemTypeSNES, @"SNES");
    OEHandleType(OEROMFileSystemTypeGenesis, @"Genesis");
    OEHandleType(OEROMFileSystemTypeGameBoy, @"GameBoy");
    OEHandleType(OEROMFileSystemTypeGameBoyAdvance, @"GameBoy Advance");
    OEHandleType(OEROMFileSystemTypeTurboGrafx16, @"TurboGrafx-16");
    OEHandleType(OEROMFileSystemTypeAtariLynx, @"Lynx");
    OEHandleType(OEROMFileSystemTypeNeoGeo, @"Neo Geo");
#undef OEHandleType
    
    return [NSNumber numberWithInt:OEROMFileSystemTypeUnknown];
}

@end


@implementation OEROMFileSystemTypeReverseTransformer

- (NSString *)transformedValue:(NSNumber *)value
{
    NSLog(@"Reversy!");
    return [super reverseTransformedValue:value];
}

- (NSNumber *)reverseTransformedValue:(NSString *)value
{
    return [super transformedValue:value];
}

@end