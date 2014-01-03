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

#import "OEUtilities.m"
#import <CommonCrypto/CommonDigest.h>

// output must be at least 2*len+1 bytes
void tohex(const unsigned char *input, size_t len, char *output)
{
    static char table[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for (int i = 0; i < len; ++i) {
        output[2*i] = table[input[i]>>4];
        output[2*i+1] = table[input[i]&0xF];
    }
    output[2*len+1] = '\0';
}

void OEPrintFirstResponderChain(void)
{
    NSWindow *keyWindow = [NSApp keyWindow];
    
    if(keyWindow == nil) return;
    
    NSLog(@"responders: %@", OENextRespondersFromResponder([keyWindow firstResponder]));
}

NSArray *OENextRespondersFromResponder(NSResponder *responder)
{
    if(responder == nil) return @[ ];
    
    NSMutableArray *responders = [NSMutableArray array];
    
    while(responder != nil)
    {
        [responders addObject:responder];
        responder = [responder nextResponder];
    }
    
    return responders;
}

NSString *temporaryDirectoryForDecompressionOfPath(NSString *aPath)
{
    NSFileManager *fm = [NSFileManager new];
    NSError *error;

    // First, check that known location in case we've already dealt with this one
    unsigned char hash[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1([aPath UTF8String], strlen([aPath UTF8String]), hash);

    char hexhash[2*CC_SHA1_DIGEST_LENGTH+1];
    tohex(hash, CC_SHA1_DIGEST_LENGTH, hexhash);
    // get the bundle identifier of ourself, or our parent app if we have none
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *folder = [bundle bundleIdentifier];
    if (!folder) {
        NSString *path = [bundle bundlePath];
        path = [[path stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
        bundle = [NSBundle bundleWithPath:path];
        folder = [bundle bundleIdentifier];
        if (!folder) {
            DLog(@"Couldn't get bundle identifier of OpenEmu");
            folder = @"OpenEmu";
        }
    }
    folder = [NSTemporaryDirectory() stringByAppendingPathComponent:folder];
    folder = [folder stringByAppendingPathComponent:[NSString stringWithUTF8String:hexhash]];
    if (![fm createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:&error]) {
        DLog(@"Couldn't create temp directory %@, %@", folder, error);
        return nil;
    }

    return folder;
}

// According to http://stackoverflow.com/questions/11072804/mac-os-x-10-8-replacement-for-gestalt-for-testing-os-version-at-runtime
// and http://cocoadev.com/wiki/DeterminingOSVersion
// this is the preferred way of getting the system version at runtime
bool GetSystemVersion(int *major, int *minor, int *bugfix)
{
	static int mMajor = 10;
	static int mMinor = 8;
	static int mBugfix = 0;
    
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		NSString* versionString = [[NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"] objectForKey:@"ProductVersion"];
		NSArray* versions = [versionString componentsSeparatedByString:@"."];
		check( versions.count >= 2 );
		if ( versions.count >= 1 ) {
			mMajor = [versions[0] integerValue];
		}
		if ( versions.count >= 2 ) {
			mMinor = [versions[1] integerValue];
		}
		if ( versions.count >= 3 ) {
			mBugfix = [versions[2] integerValue];
		}
	});
    
    if(major  != NULL) *major = mMajor;
    if(minor  != NULL) *minor = mMinor;
    if(bugfix != NULL) *bugfix = mBugfix;
    
    return YES;
}
