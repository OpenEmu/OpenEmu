



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

int main(int argc, char *argv[])
{	
    srand(time(NULL));
    
    @autoreleasepool {
        NSString *path = [[[[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"OpenEmu/Game Library"] path];
        
        NSDictionary *defaults = [[NSDictionary alloc] initWithObjectsAndKeys:
                                  // default thumbnail sizes for game box images
                                  [NSArray arrayWithObjects:@"{75,75}",
                                   @"{150,150}",
                                   @"{300,300}",
                                   @"{450,450}", nil],              UDBoxSizesKey,

                                  // Library Tab
                                  path,                             UDDefaultDatabasePathKey,
                                  path,                             UDDatabasePathKey,
                                  [NSNumber numberWithBool:YES],    UDAutmaticallyGetInfoKey,

                                  // others
                                  @"Save States",                   UDSaveStateFolderNameKey,	// Default name of Save States folder
                                  @"Linear",                        UDVideoFilterKey,
                                  [NSNumber numberWithBool:YES],    UDUseMD5HashingKey      ,	// Default Hashing algorithm
                                  [NSNumber numberWithFloat:0.5],   UDVolumeKey,                // Default Volume
                                  [NSNumber numberWithDouble:1.5],  UDHUDFadeOutDelayKey,       // Time until hud controls bar fades out
                                  [NSNumber numberWithBool:YES],    UDHUDCanDeleteStateKey,
                                  
                                  [NSNumber numberWithInt:40],      UDMaxSaveGameNameLengthKey,
                                  
                                  [NSNumber numberWithFloat:105.0], UDSidebarMinWidth,
                                  [NSNumber numberWithFloat:450.0], UDSidebarMaxWidth,
                                  [NSNumber numberWithFloat:495.0], UDMainViewMinWidth,
                                  
                                  [NSNumber numberWithFloat:186.0], UDSidebarWidthKey,
                                  [NSNumber numberWithBool:YES],    UDSidebarVisibleKey,
                                  
                                  [NSNumber numberWithFloat:1.0],   UDLastGridSizeKey,
                                  
                                  [NSNumber numberWithBool:YES],    UDControlsButtonHighlightRollsOver,
                                  nil];
        
        NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
        [standardDefaults registerDefaults:defaults];
        defaults = nil;
    }
    
    
	return NSApplicationMain(argc, (const char **) argv);
}

