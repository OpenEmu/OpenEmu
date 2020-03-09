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

#import "OEGenesisSystemController.h"
#import "OEGenesisSystemResponder.h"
#import "OEGenesisSystemResponderClient.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

@implementation OEGenesisSystemController

- (NSString *)systemName
{
    return ([[OELocalizationHelper sharedHelper] isRegionNA]
            ? @"Sega Genesis"
            : @"Sega Mega Drive");
}

- (NSImage *)systemIcon
{
    NSString *imageName = ([[OELocalizationHelper sharedHelper] isRegionNA]
                           ? @"genesis_library"
                           : @"megadrive_library");

    NSImage *image = [NSImage imageNamed:imageName];
    if(image == nil)
    {
        NSBundle *bundle = [NSBundle bundleForClass:[self class]];
        image = [bundle imageForResource:imageName];
        [image setName:imageName];
    }
    return image;
}

- (OEFileSupport)canHandleFile:(__kindof OEFile *)file
{
    if(![file.fileExtension isEqualToString:@"bin"])
        return OEFileSupportUncertain;

    NSString *dataString = [file readASCIIStringInRange:NSMakeRange(0x100, 16)];
    if ([dataString hasPrefix:@"SEGA GENESIS"] || [dataString hasPrefix:@"SEGA MEGA DRIVE"])
        return OEFileSupportYes;

    return OEFileSupportNo;
}

@end
