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

#import "OEPSXSystemController.h"

@implementation OEPSXSystemController

- (OEFileSupport)canHandleFile:(__kindof OEFile *)file
{
    if (![file isKindOfClass:[OEDiscDescriptor class]])
        return OEFileSupportNo;

    // TODO: Add frontend method to receive NSError from -canHandleFile: in system plugins; this doesn't belong here.
    // Check for ECM magic header. Fix for https://github.com/OpenEmu/OpenEmu/issues/2588
    const uint8_t bytes[] = { 0x45, 0x43, 0x4D, 0x00 };
    NSData *dataCompare = [[NSData alloc] initWithBytes:bytes length:sizeof(bytes)];
    NSData *dataTrackBuffer = [file readDataInRange:NSMakeRange(0, 4)];
    BOOL isBinaryECM = [dataTrackBuffer isEqualToData:dataCompare];

    if(isBinaryECM)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            NSAlert *alert = [[NSAlert alloc] init];

            alert.messageText = NSLocalizedString(@"ECM compressed binary detected.", @"");
            alert.informativeText = NSLocalizedString(@"ECM compressed binaries cannot be imported. Please read the disc importing guide.", @"");
            alert.alertStyle = NSAlertStyleCritical;
            [alert addButtonWithTitle:NSLocalizedString(@"View Guide in Browser", @"")];
            [alert addButtonWithTitle:NSLocalizedString(@"Dismiss", @"")];

            if([alert runModal] == NSAlertFirstButtonReturn)
            {
                [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-CD-based-games"]];
            }
        });

        return OEFileSupportNo;
    }

    NSString *dataTrackString = [file readASCIIStringInRange:NSMakeRange(0x24E0, 16)];
    NSLog(@"'%@'", dataTrackString);

    return [dataTrackString isEqualToString:@"  Licensed  by  "] ? OEFileSupportYes : OEFileSupportNo;
}

- (NSString *)serialLookupForFile:(__kindof OEFile *)file
{
    if (![file isKindOfClass:[OEDiscDescriptor class]])
        return nil;

    NSString *mode1DataString = [file readASCIIStringInRange:NSMakeRange(0x8001, 5)];
    NSString *mode2DataString = [file readASCIIStringInRange:NSMakeRange(0x9319, 5)];

    NSUInteger discSectorSize, discSectorHeader, discOffset;

    // Find which offset contains CD001
    if([mode1DataString isEqualToString:@"CD001"])
    {
        // ISO9660/MODE1/2048
        discSectorSize	= 2048; // 0x800
        discSectorHeader = 0;
        discOffset	= ((discSectorSize * 16) + discSectorHeader);
        
    }
    else if([mode2DataString isEqualToString:@"CD001"])
    {
        // ISO9660/MODE2/FORM1/2352
        discSectorSize = 2352;  // 0x930
        discSectorHeader = 24; // 0x18
        discOffset = ((discSectorSize * 16) + discSectorHeader);
    }
    else
    {
        // bad dump, not ISO 9660
        return nil;
    }
    
    // Root Directory Record offset
    NSUInteger rootDirectoryRecordLocationOffset = 0x9E;
    NSData *rootDirectoryRecordBuffer = [file readDataInRange:NSMakeRange(discOffset + rootDirectoryRecordLocationOffset, 8)];

    int rootDirectoryRecordSector = *((int *)[rootDirectoryRecordBuffer bytes]);
    
    NSUInteger rootDirectoryRecordOffset = rootDirectoryRecordSector * discSectorSize;
    
    //NSLog(@"Root Directory Record Offset: 0x%08X", (uint32_t)rootDirectoryRecordOffset);

    // Find SYSTEM.CNF
    BOOL foundTitleIDFile = NO;
    NSUInteger position = 0;

    while(position < discSectorSize)
    {
        NSString *systemcnfString = [file readASCIIStringInRange:NSMakeRange(rootDirectoryRecordOffset + position, 10)];
        if([systemcnfString isEqualToString:@"SYSTEM.CNF"])
        {
            // SYSTEM.CNF file record found
            //NSLog(@"SYSTEM.CNF file record found at pos: 0x%03X", (uint32_t)position);
            foundTitleIDFile = true;
            break;
        }
        
        position++;
    }
    
    if(!foundTitleIDFile)
    {
        // bad dump, couldn't find SYSTEM.CNF entry on the specified sector
        return nil;
    }

    // SYSTEM.CNF Extent Location (Data location)
    NSUInteger systemcnfDataOffset = (rootDirectoryRecordOffset + position) - 0x1F;
    NSData *systemcnfExtentLocationDataBuffer = [file readDataInRange:NSMakeRange(systemcnfDataOffset, 8)];

    int sizeExtentOffset = *((int *)[systemcnfExtentLocationDataBuffer bytes]);

    NSUInteger systemcnfExtentOffset = sizeExtentOffset * discSectorSize;
    //NSLog(@"SYSTEM.CNF Extent (data) Offset: 0x%08X", (uint32_t)systemcnfExtentOffset);

    // Data length (size)
    NSUInteger systemcnfDataLengthOffset = (rootDirectoryRecordOffset + position) - 0x17;
    NSData *systemcnfDataLengthDataBuffer = [file readDataInRange:NSMakeRange(systemcnfDataLengthOffset, 8)];

    NSUInteger systemcnfDataLength = *((int *)[systemcnfDataLengthDataBuffer bytes]);

    //NSLog(@"SYSTEM.CNF Data Length: 0x%08X", (uint32_t)systemcnfDataLength);
    NSString *output = [file readASCIIStringInRange:NSMakeRange(systemcnfExtentOffset + discSectorHeader, systemcnfDataLength)];

    //NSLog(@"output: %@", output);

    // RegEx pattern match the disc serial (Note: regex backslashes are escaped)
    // Handles all cases I've encountered so far:
    //  BOOT=cdrom:\SCES_015.64;1           (no whitespace)
    //  BOOT=cdrom:\SLUS_004.49             (no semicolon)
    //  BOOT=cdrom:\SLUS-000.05;1           (hyphen instead of underscore)
    //  BOOT = cdrom:\SLES_025.37;1         (whitespace)
    //  BOOT = cdrom:SLUS_000.67;1          (no backslash)
    //  BOOT = cdrom:\slus_005.94;1         (lowercase)
    //  BOOT = cdrom:\TEKKEN3\SLUS_004.02;1 (extra path)
    //  BOOT	= cdrom:\SLUS_010.41;1      (horizontal tab)
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"BOOT\\s*=\\s*?cdrom:\\\\?(.+\\\\)?(.+?(?=;|\\s))" options:NSRegularExpressionCaseInsensitive error:nil];
    NSTextCheckingResult *match = [regex firstMatchInString:output options:0 range:NSMakeRange(0, [output length])];

    if(match == nil)
    {
        NSLog(@"RegEx pattern could not match serial. Full string:\n%@", output);
        return @"NO MATCH";
    }

    NSString *serial = [output substringWithRange:[match rangeAtIndex:2]];

    // Format serial and return
    serial = [[serial componentsSeparatedByCharactersInSet:[[NSCharacterSet alphanumericCharacterSet] invertedSet]] componentsJoinedByString:@""];

    NSMutableString *formattedSerial = [NSMutableString stringWithString:[serial uppercaseString]];
    [formattedSerial insertString:@"-" atIndex:4];

    NSLog(@"Serial: %@", formattedSerial);

    return formattedSerial;
}

@end
