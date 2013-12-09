//
//  OEGameInfoHelper.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07/12/13.
//
//

#import <Foundation/Foundation.h>
#import "OEDBRom.h"
#import "OEDBSystem.h"
#import "OEDBGame.h"

@interface OEGameInfoHelper : NSObject
+ (id)sharedHelper;
- (NSDictionary*)gameInfoForROM:(OEDBRom*)rom error:(NSError *__autoreleasing*)error;
- (int)sizeOfROMHeaderForSystem:(OEDBSystem*)system;
@end
