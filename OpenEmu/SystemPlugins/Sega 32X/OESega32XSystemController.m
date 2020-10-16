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

#import "OESega32XSystemController.h"

@implementation OESega32XSystemController

- (NSImage *)systemIcon
{
    NSString *imageName;

    if([[OELocalizationHelper sharedHelper] isRegionJAP])
    {
        imageName = @"32x_jp_library";
    }
    else if([[OELocalizationHelper sharedHelper] isRegionEU])
    {
        imageName = @"32x_eu_library";
    }
    else
    {
        imageName = @"32x_na_library";
    }

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

    NSString *dataString = [file readASCIIStringInRange:NSMakeRange(0x100, 8)];
    if ([dataString isEqualToString:@"SEGA 32X"])
        return OEFileSupportYes;

    return OEFileSupportNo;
}

@end
