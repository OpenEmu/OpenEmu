/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OEGCSystemController.h"

@implementation OEGCSystemController

// Read header to detect GameCube ISO, GCM & CISO.
- (OEFileSupport)canHandleFile:(__kindof OEFile *)file
{
    // Handle gcm file and return early
    if([file.fileExtension isEqualToString:@"gcm"])
        return OEFileSupportYes;

    NSRange dataRange = NSMakeRange(0x1C, 4);

    // Handle ciso file and set the offset for the Magicword in compressed iso.
    if ([file.fileExtension isEqualToString:@"ciso"])
        dataRange.location = 0x801C;

    // Gamecube Magicword 0xC2339F3D
    NSData *dataBuffer = [file readDataInRange:dataRange];
    NSData *comparisonData = [[NSData alloc] initWithBytes:(const uint8_t[]){ 0xC2, 0x33, 0x9F, 0x3D } length:4];

    if ([dataBuffer isEqualToData:comparisonData])
        return OEFileSupportYes;

    return OEFileSupportNo;
}

- (NSString *)serialLookupForFile:(__kindof OEFile *)file
{
    NSRange dataRange = NSMakeRange(0x0, 6);

    // Check if it's a CISO and adjust the Game ID offset location
    NSMutableString *magic = [NSMutableString stringWithString:[file readASCIIStringInRange:NSMakeRange(0x0, 4)]];
    if ([magic isEqualToString:@"CISO"]) {
        dataRange.location = 0x8000;
    }

    // Read the game ID
    NSMutableString *gameID = [NSMutableString stringWithString:[file readASCIIStringInRange:dataRange]];

    // Read the disc number and version number bytes from the header.
    int headerDiscByte    = *(int*)([file readDataInRange:NSMakeRange(dataRange.location + 0x6, 1)].bytes);
    int headerVersionByte = *(int*)([file readDataInRange:NSMakeRange(dataRange.location + 0x7, 1)].bytes);

    // Append disc and version to the game ID if found.
    if (headerDiscByte > 0) {
        headerDiscByte++;
        [gameID appendString:[NSString stringWithFormat:@"-DISC%d", headerDiscByte]];
    }

    if (headerVersionByte > 0) {
        [gameID appendString:[NSString stringWithFormat:@"-REV%d", headerVersionByte]];
    }

    NSLog(@"Game ID: %@", gameID);

    return gameID;
}

@end
