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
#import "OEGCSystemResponder.h"
#import "OEGCSystemResponderClient.h"

@implementation OEGCSystemController

// Read header to detect GameCube ISO, GCM & CISO
- (OECanHandleState)canHandleFile:(NSString *)path
{
    BOOL handleFileExtension = [super canHandleFileExtension:[path pathExtension]];
    OECanHandleState canHandleFile = OECanHandleNo;

    if(handleFileExtension)
    {
        // Handle gcm file and return early
        if([[[path pathExtension] lowercaseString] isEqualToString:@"gcm"])
            return OECanHandleYes;

        NSFileHandle *dataFile;
        NSData *dataBuffer;

        dataFile = [NSFileHandle fileHandleForReadingAtPath:path];

        // Handle ciso file and set the offset for the Magicword in compressed iso
        if([[[path pathExtension] lowercaseString] isEqualToString:@"ciso"])
            [dataFile seekToFileOffset: 0x801C];
        else
            [dataFile seekToFileOffset: 0x1C];

        dataBuffer = [dataFile readDataOfLength:4]; // Gamecube Magicword 0xC2339F3D
        NSString *dataString = [[NSString alloc] initWithData:dataBuffer encoding:NSMacOSRomanStringEncoding];
        NSLog(@"'%@'", dataString);

        if([dataString isEqualToString:@"¬3ü="])
            canHandleFile = OECanHandleYes;

        [dataFile closeFile];
    }
    return canHandleFile;
}
@end
