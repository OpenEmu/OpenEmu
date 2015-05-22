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

#import "OEPCECDSystemController.h"
#import "OEPCECDSystemResponder.h"
#import "OEPCECDSystemResponderClient.h"

@implementation OEPCECDSystemController

- (NSString *)systemName
{
    return ([[OELocalizationHelper sharedHelper] isRegionJAP]
            ? @"PC Engine CD"
            : @"TurboGrafx-CD");
}

- (NSImage *)systemIcon
{
    NSString *imageName = ([[OELocalizationHelper sharedHelper] isRegionJAP]
                           ? @"pcenginecd_library"
                           : @"tgcd_library");

    NSImage *image = [NSImage imageNamed:imageName];
    if(image == nil)
    {
        NSBundle *bundle = [NSBundle bundleForClass:[self class]];
        NSString *path = [bundle pathForImageResource:imageName];
        image = [[NSImage alloc] initWithContentsOfFile:path];
        [image setName:imageName];
    }
    return image;
}

- (OECanHandleState)canHandleFile:(NSString *)path
{
    NSArray *dataTracks;
    if([[[path pathExtension] lowercaseString] isEqualToString:@"ccd"])
    {
        OECloneCD *ccd = [[OECloneCD alloc] initWithURL:[NSURL fileURLWithPath:path]];
        dataTracks = [ccd referencedFiles];
    }
    else if([[[path pathExtension] lowercaseString] isEqualToString:@"cue"])
    {
        OECUESheet *cueSheet = [[OECUESheet alloc] initWithPath:path];
        dataTracks = [cueSheet referencedFiles];
    }

    BOOL handleFileExtension = [super canHandleFileExtension:[path pathExtension]];
    OECanHandleState canHandleFile = OECanHandleNo;

    if(handleFileExtension)
    {
        for(id dataTrack in dataTracks)
        {
            NSString *dataTrackPath = [[path stringByDeletingLastPathComponent] stringByAppendingPathComponent:dataTrack];

            NSError *error = nil;
            NSData *dataTrackBuffer = [NSData dataWithContentsOfFile:dataTrackPath options:NSDataReadingMappedIfSafe | NSDataReadingUncached error:&error];

            NSString *dataTrackString = @"PC Engine CD-ROM SYSTEM";
            NSData *dataSearch = [dataTrackString dataUsingEncoding:NSUTF8StringEncoding];
            // this still slows import down but we need to scan the disc as there's no common offset
            NSRange indexOfData = [dataTrackBuffer rangeOfData: dataSearch options:0 range:NSMakeRange(0, [dataTrackBuffer length])];

            if(indexOfData.length > 0)
            {
                NSLog(@"PCE-CD data track path: %@", dataTrackPath);
                NSLog (@"'%@' at offset = 0x%lX", dataTrackString, indexOfData.location);
                canHandleFile = OECanHandleYes;
                break;
            }
        }
    }
    return canHandleFile;
}

@end
