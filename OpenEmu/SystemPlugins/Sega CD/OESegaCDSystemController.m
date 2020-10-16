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

#import "OESegaCDSystemController.h"

@implementation OESegaCDSystemController

- (OEFileSupport)canHandleFile:(__kindof OEFile *)file
{
    if (![file isKindOfClass:[OECUESheet class]])
        return OEFileSupportNo;

    OECUESheet *cueSheet = file;
    NSURL *dataTrackURL = cueSheet.dataTrackFileURL;

    NSLog(@"SCD data track: %@", dataTrackURL);

    NSString *dataTrackString = [cueSheet readASCIIStringInRange:NSMakeRange(0, 16)];
    NSString *otherDataTrackString = [cueSheet readASCIIStringInRange:NSMakeRange(0x10, 16)];
    NSLog(@"'%@'", dataTrackString);
    NSLog(@"'%@'", otherDataTrackString);
    NSArray *dataTrackList = @[ @"SEGADISCSYSTEM  ", @"SEGABOOTDISC    ", @"SEGADISC        ", @"SEGADATADISC    " ];

    for(NSString *d in dataTrackList)
    {
        if([dataTrackString isEqualToString:d] || [otherDataTrackString isEqualToString:d])
            return OEFileSupportYes;
    }

    return OEFileSupportNo;
}

- (NSString *)headerLookupForFile:(__kindof OEFile *)file
{
    if (![file isKindOfClass:[OECUESheet class]])
        return nil;

    // Read both offsets because of various dumps
    NSString *dataTrackString = [file readASCIIStringInRange:NSMakeRange(0x100, 16)];
    NSString *otherDataTrackString = [file readASCIIStringInRange:NSMakeRange(0x110, 16)];

    unsigned long long offsetFound = 0;
    NSArray *dataTrackList = @[ @"SEGA GENESIS    ", @"SEGA MEGA DRIVE " ];
    
    // Find which offset contains the 256-byte header
    for(NSString *d in dataTrackList)
    {
        if([dataTrackString isEqualToString:d])
        {
            offsetFound = 0x0100;
            break;
        }
        else if ([otherDataTrackString isEqualToString:d])
        {
            offsetFound = 0x0110;
            break;
        }
    }

    // Read the full header at the offset found
    NSData *headerDataTrackBuffer = [file readDataInRange:NSMakeRange(offsetFound, 256)];

    // Format the hexadecimal representation and return
    NSString *hex = [headerDataTrackBuffer oe_hexStringRepresentation];
    
    return hex;
}

@end
