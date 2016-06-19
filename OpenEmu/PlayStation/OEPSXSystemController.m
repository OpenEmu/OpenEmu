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
#import "OEPSXSystemResponder.h"
#import "OEPSXSystemResponderClient.h"

@implementation OEPSXSystemController

- (OECanHandleState)canHandleFile:(NSString *)path
{
    NSString *dataTrack;
    if([[[path pathExtension] lowercaseString] isEqualToString:@"ccd"])
    {
        OECloneCD *cueSheet = [[OECloneCD alloc] initWithURL:[NSURL fileURLWithPath:path]];
        dataTrack = [cueSheet dataTrackPath];
    }
    else if([[[path pathExtension] lowercaseString] isEqualToString:@"cue"])
    {
        OECUESheet *cueSheet = [[OECUESheet alloc] initWithPath:path];
        dataTrack = [cueSheet dataTrackPath];
    }
    else if([[[path pathExtension] lowercaseString] isEqualToString:@"m3u"])
        return OECanHandleYes;
        

    NSString *dataTrackPath = [[path stringByDeletingLastPathComponent] stringByAppendingPathComponent:dataTrack];
    NSLog(@"PSX data track path: %@", dataTrackPath);

    BOOL valid = [super canHandleFileExtension:[path pathExtension]];
    if(valid)
    {
        NSFileHandle *dataTrackFile;
        NSData *dataTrackBuffer;

        dataTrackFile = [NSFileHandle fileHandleForReadingAtPath: dataTrackPath];

        // TODO: Add frontend method to receive NSError from -canHandleFile: in system plugins; this doesn't belong here.
        // Check for ECM magic header. Fix for https://github.com/OpenEmu/OpenEmu/issues/2588
        uint8_t bytes[] = { 0x45, 0x43, 0x4D, 0x00 };
        [dataTrackFile seekToFileOffset: 0x0];
        dataTrackBuffer = [dataTrackFile readDataOfLength: 4];
        NSData *dataCompare = [[NSData alloc] initWithBytes:bytes length:sizeof(bytes)];
        BOOL isBinaryECM = [dataTrackBuffer isEqualToData:dataCompare];

        if(isBinaryECM)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                NSAlert *alert = [[NSAlert alloc] init];

                alert.messageText = NSLocalizedString(@"ECM compressed binary detected.", @"");
                alert.informativeText = NSLocalizedString(@"ECM compressed binaries cannot be imported. Please read the disc importing guide.", @"");
                alert.alertStyle = NSCriticalAlertStyle;
                [alert addButtonWithTitle:NSLocalizedString(@"View Guide in Browser", @"")];
                [alert addButtonWithTitle:NSLocalizedString(@"Dismiss", @"")];

                if([alert runModal] == NSAlertFirstButtonReturn)
                {
                    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-CD-based-games"]];
                }
            });

            return OECanHandleNo;
        }

        [dataTrackFile seekToFileOffset: 0x24E0];
        dataTrackBuffer = [dataTrackFile readDataOfLength: 16];
        
        NSString *dataTrackString = [[NSString alloc] initWithData:dataTrackBuffer encoding:NSUTF8StringEncoding];
        NSLog(@"'%@'", dataTrackString);
        NSArray *dataTrackList = @[ @"  Licensed  by  ", @"  Cracked   by  " ];

        valid = NO;
        for(NSString *d in dataTrackList)
        {
            if([dataTrackString isEqualToString:d])
            {
                valid = YES;
                break;
            }
        }

        [dataTrackFile closeFile];
    }
    return valid ? OECanHandleYes : OECanHandleNo;
}

- (NSString *)serialLookupForFile:(NSString *)path
{
    NSString *dataTrack;
    if([[[path pathExtension] lowercaseString] isEqualToString:@"ccd"])
    {
        OECloneCD *cueSheet = [[OECloneCD alloc] initWithURL:[NSURL fileURLWithPath:path]];
        dataTrack = [cueSheet dataTrackPath];
    }
    else if([[[path pathExtension] lowercaseString] isEqualToString:@"cue"])
    {
        OECUESheet *cueSheet = [[OECUESheet alloc] initWithPath:path];
        dataTrack = [cueSheet dataTrackPath];
    }

    NSString *dataTrackPath = [[path stringByDeletingLastPathComponent] stringByAppendingPathComponent:dataTrack];
    
    NSFileHandle *dataTrackFile;
    NSData *mode1DataBuffer, *mode2DataBuffer;
    
    // ISO 9660 CD001, check for MODE1 or MODE2
    dataTrackFile = [NSFileHandle fileHandleForReadingAtPath: dataTrackPath];
    [dataTrackFile seekToFileOffset: 0x8001]; // MODE1
    mode1DataBuffer = [dataTrackFile readDataOfLength: 5];
    [dataTrackFile seekToFileOffset: 0x9319]; // MODE2
    mode2DataBuffer = [dataTrackFile readDataOfLength: 5];
    
    NSUInteger discSectorSize, discSectorHeader, discOffset;
    
    NSString *mode1DataString = [[NSString alloc]initWithData:mode1DataBuffer encoding:NSUTF8StringEncoding];
    NSString *mode2DataString = [[NSString alloc]initWithData:mode2DataBuffer encoding:NSUTF8StringEncoding];
    
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
        [dataTrackFile closeFile];
        return nil;
    }
    
    // Root Directory Record offset
    NSUInteger rootDirectoryRecordLocationOffset = 158; // 0x9E
    NSData *rootDirectoryRecordBuffer;
    
    [dataTrackFile seekToFileOffset: discOffset + rootDirectoryRecordLocationOffset];
    rootDirectoryRecordBuffer = [dataTrackFile readDataOfLength: 8];
    
    int rootDirectoryRecordSector = *((int *)[rootDirectoryRecordBuffer bytes]);
    
    NSUInteger rootDirectoryRecordOffset = rootDirectoryRecordSector * discSectorSize;
    
    //NSLog(@"Root Directory Record Offset: 0x%08X", (uint32_t)rootDirectoryRecordOffset);
    
    [dataTrackFile seekToFileOffset: rootDirectoryRecordOffset];
    
    // Find SYSTEM.CNF
    BOOL foundTitleIDFile = NO;
    NSUInteger position = 0;
    NSData *systemcnfDataBuffer;
    
    while(position < discSectorSize)
    {
        systemcnfDataBuffer = [dataTrackFile readDataOfLength: 10];
        NSString *systemcnfString = [[NSString alloc]initWithData:systemcnfDataBuffer encoding:NSUTF8StringEncoding];
        
        if([systemcnfString isEqualToString:@"SYSTEM.CNF"])
        {
            // SYSTEM.CNF file record found
            //NSLog(@"SYSTEM.CNF file record found at pos: 0x%03X", (uint32_t)position);
            foundTitleIDFile = true;
            break;
        }
        
        position++;
        
        [dataTrackFile seekToFileOffset: rootDirectoryRecordOffset + position];
    }
    
    if(!foundTitleIDFile)
    {
        // bad dump, couldn't find SYSTEM.CNF entry on the specified sector
        [dataTrackFile closeFile];
        return nil;
    }
    else {
        // SYSTEM.CNF Extent Location (Data location)
        NSUInteger systemcnfDataOffset = (rootDirectoryRecordOffset + position) - 0x1F;
        
        NSData *systemcnfExtentLocationDataBuffer;
        [dataTrackFile seekToFileOffset: systemcnfDataOffset];
        systemcnfExtentLocationDataBuffer = [dataTrackFile readDataOfLength: 8];
        
        int sizeExtentOffset = *((int *)[systemcnfExtentLocationDataBuffer bytes]);
        
        NSUInteger systemcnfExtentOffset = sizeExtentOffset * discSectorSize;
        //NSLog(@"SYSTEM.CNF Extent (data) Offset: 0x%08X", (uint32_t)systemcnfExtentOffset);
        
        // Data length (size)
        NSUInteger systemcnfDataLengthOffset = (rootDirectoryRecordOffset + position) - 0x17;
        
        NSData *systemcnfDataLengthDataBuffer;
        [dataTrackFile seekToFileOffset: systemcnfDataLengthOffset];
        systemcnfDataLengthDataBuffer = [dataTrackFile readDataOfLength: 8];
        
        NSUInteger systemcnfDataLength = *((int *)[systemcnfDataLengthDataBuffer bytes]);
        
        //NSLog(@"SYSTEM.CNF Data Length: 0x%08X", (uint32_t)systemcnfDataLength);
        
        NSData *titleIDFileExtendDataBuffer;
        [dataTrackFile seekToFileOffset: systemcnfExtentOffset + discSectorHeader];
        titleIDFileExtendDataBuffer = [dataTrackFile readDataOfLength: systemcnfDataLength];
        
        NSString *output = [[NSString alloc]initWithData:titleIDFileExtendDataBuffer encoding:NSUTF8StringEncoding];
        
        //NSLog(@"output: %@", output);
        
        // RegEx pattern match the disc serial
        NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"BOOT\\s*=\\s*?cdrom:\\\\?(.+\\\\)?(.+);" options:NSRegularExpressionCaseInsensitive error:nil];
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
        
        [dataTrackFile closeFile];
        
        return formattedSerial;
    }
}

@end
