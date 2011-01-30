/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

@class OEDownload;

@protocol OEDownloadDelegate

- (void)OEDownloadDidStart:(OEDownload *)download;
- (void)OEDownloadDidFinish:(OEDownload *)download;
- (void)OEIconDownloadDidFinish:(OEDownload *)download;

@end

@class SUAppcast, SUAppcastItem;

@interface OEDownload : NSViewController
{
    id<OEDownloadDelegate>  delegate;
    
    NSString               *downloadTitle;
    NSString               *downloadDescription;
    NSImage                *downloadIcon;
    
    SUAppcast              *appcast;
    SUAppcastItem          *appcastItem;
    
    NSProgressIndicator    *progressBar;
    NSButton               *startDownloadButton;
    NSString               *downloadPath;
    NSString               *fullPluginPath;
    unsigned long long      expectedLength;
    unsigned long long      downloadedSize;
    
    NSMutableData          *iconData;
    NSURLConnection        *iconConnection;
    
    BOOL                    downloading;
    BOOL                    enabled;
}

@property(copy)     NSString               *downloadTitle;
@property(copy)     NSString               *downloadDescription;
@property(retain)   NSImage                *downloadIcon;

@property(retain)   SUAppcast              *appcast;
@property(retain)   SUAppcastItem          *appcastItem;

@property(readonly) NSProgressIndicator    *progressBar;
@property(readonly) NSButton               *startDownloadButton;
@property(readonly) NSString               *fullPluginPath;

@property(assign)   id<OEDownloadDelegate>  delegate;

@property(assign  , getter=isEnabled)     BOOL enabled;
@property(readonly, getter=isDownloading) BOOL downloading;

- (void)startDownload:(id)sender;
- (void)downloadIconFromURL:(NSURL *)iconURL;

@end
