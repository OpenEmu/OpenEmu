



/*
 Copyright (c) 2009-2011, OpenEmu Team
 
 
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

#import <Cocoa/Cocoa.h>

extern NSString * const OEGameControlsBarCanDeleteSaveStatesKey;
extern NSString * const OEGameControlsBarFadeOutDelayKey;
extern NSString * const OEMaxSaveGameNameLengthKey;

extern NSString * const OEDefaultDatabasePathKey;
extern NSString * const OEDatabasePathKey;

extern NSString * const OEAutomaticallyGetInfoKey;

extern NSString * const OEGameVolumeKey;
extern NSString * const OEGameVideoFilterKey;

extern NSString * const OEBoxSizesKey;

extern NSString * const OESidebarMinWidth;
extern NSString * const OESidebarMaxWidth;
extern NSString * const OEMainViewMinWidth;

extern NSString * const OESidebarWidthKey;
extern NSString * const OESidebarVisibleKey;

extern NSString * const OELastGridSizeKey;
int main(int argc, char *argv[])
{	
    srand(time(NULL));
    
    @autoreleasepool {
        NSString *path = [[[[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"OpenEmu/Game Library"] path];
        
        NSDictionary *defaults = [[NSDictionary alloc] initWithObjectsAndKeys:
                                  // default thumbnail sizes for game box images
                                  @[@"{75,75}",
                                   @"{150,150}",
                                   @"{300,300}",
                                   @"{450,450}"],              OEBoxSizesKey,

                                  // Library Tab
                                  path,                             OEDefaultDatabasePathKey,
                                  path,                             OEDatabasePathKey,
                                  [NSNumber numberWithBool:YES],    OEAutomaticallyGetInfoKey,

                                  // others
                                  @"Linear",                        OEGameVideoFilterKey,
                                  [NSNumber numberWithFloat:0.5],   OEGameVolumeKey,                // Default Volume
                                  [NSNumber numberWithDouble:1.5],  OEGameControlsBarFadeOutDelayKey,       // Time until hud controls bar fades out
                                  [NSNumber numberWithBool:YES],    OEGameControlsBarCanDeleteSaveStatesKey,
                                  
                                  [NSNumber numberWithInt:40],      OEMaxSaveGameNameLengthKey,
                                  
                                  [NSNumber numberWithFloat:105.0], OESidebarMinWidth,
                                  [NSNumber numberWithFloat:450.0], OESidebarMaxWidth,
                                  [NSNumber numberWithFloat:495.0], OEMainViewMinWidth,
                                  
                                  [NSNumber numberWithFloat:186.0], OESidebarWidthKey,
                                  [NSNumber numberWithBool:YES],    OESidebarVisibleKey,
                                  
                                  [NSNumber numberWithFloat:1.0],   OELastGridSizeKey,
                                
                                  nil];
        
        NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
        [standardDefaults registerDefaults:defaults];
        defaults = nil;
    }
    
    
	return NSApplicationMain(argc, (const char **) argv);
}

