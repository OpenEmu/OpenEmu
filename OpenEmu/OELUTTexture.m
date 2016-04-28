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

#ifdef CG_SUPPORT

#import "OELUTTexture.h"

@implementation OELUTTexture

- (void)loadTexture;
{
    // Release the texture if it was already loaded
    if(_texture != nil)
        CGImageRelease(_texture);

    // Get the URL for the pathname passed to the function.
    NSURL *url = [NSURL fileURLWithPath:_path];

    // Create an image source from the URL.
    CGImageSourceRef source = CGImageSourceCreateWithURL((__bridge CFURLRef)url, NULL);

    // Make sure the image source exists before continuing
    if(source == nil){
        NSLog(@"Failed to find texture at %s", [_path UTF8String]);
        return;
    }

    // Create an image from the first item in the image source.
    _texture = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CFRelease(source);

    // Make sure the image exists before continuing
    if(_texture == nil)
    {
        NSLog(@"Failed to load texture from %s", [_path UTF8String]);
        return;
    }
}

- (void)dealloc
{
    if(_texture != nil)
        CGImageRelease(_texture);
}

@end

#endif
