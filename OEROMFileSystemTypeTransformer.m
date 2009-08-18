//
//  OEROMFileSystemTypeTransformer.m
//  OpenEmu
//
//  Created by Steve Streza on 8/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OEROMFileSystemTypeTransformer.h"


@implementation OEROMFileSystemTypeTransformer

+(id)transformer{
	return [[[[self class] alloc] init] autorelease];
}

+(BOOL)allowsReverseTransformation{
	return YES;
}

- (NSString *)transformedValue:(NSNumber *)value{
	if(![value isKindOfClass:[NSNumber class]]) return @"Unknown";

#define OEHandleType(__type, __name) \
	case __type: \
		return (__name); \
		break;
	
	OEROMFileSystemType systemType = [value unsignedIntValue];
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

- (NSNumber *)reverseTransformedValue:(NSString *)value{
	if(![value isKindOfClass:[NSString class]]) return [NSNumber numberWithUnsignedInt:OEROMFileSystemTypeUnknown];
	
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
	
	return [NSNumber numberWithUnsignedInt:OEROMFileSystemTypeUnknown];
}

@end


@implementation OEROMFileSystemTypeReverseTransformer

- (NSString *)transformedValue:(NSNumber *)value{
	NSLog(@"Reversy!");
	return [super reverseTransformedValue:value];
}

- (NSNumber *)reverseTransformedValue:(NSString *)value{
	return [super transformedValue:value];
}

@end