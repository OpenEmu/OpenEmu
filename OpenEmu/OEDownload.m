/*
 Copyright (c) 2014, OpenEmu Team
 
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

#import "OEDownload.h"

@interface OEDownload () <NSURLDownloadDelegate>
{
    dispatch_semaphore_t _waitSemaphore;
}

@property (nonatomic, readwrite) CGFloat progress;

@property (readwrite) NSURL   *destinationURL;
@property (readwrite) NSError *error;

@property NSURL *url;
@property NSURLDownload *download;
@property NSInteger totalDownloadLength, downloadedLength;
@end

@implementation OEDownload
@synthesize progress=_progress;

- (instancetype)initWithURL:(NSURL*)url
{
    self = [super init];
    if(self != nil)
    {
        [self setDestinationURL:nil];
        [self setError:nil];

        [self setUrl:url];

        _waitSemaphore = dispatch_semaphore_create(0);
        _totalDownloadLength = -1;
        _downloadedLength = 0;
    }
    return self;
}

- (void)dealloc
{
    dispatch_release(_waitSemaphore);
}

- (void)setProgress:(CGFloat)progress
{
    [self willChangeValueForKey:@"progress"];
    _progress = progress;
    [self didChangeValueForKey:@"progress"];

    if([self progressHandler] && ![self progressHandler](progress))
        [[self download] cancel];
}
- (CGFloat)progress
{
    return _progress;
}
#pragma mark -
- (void)startDownload
{
    [self setProgress:-1.0];

    NSURLRequest  *request  = [NSURLRequest requestWithURL:[self url] cachePolicy:NSURLCacheStorageAllowed timeoutInterval:60.0];
    NSURLDownload *download = [[NSURLDownload alloc] initWithRequest:request delegate:self];

    [self setDownload:download];
}

- (void)cancelDownload
{
    [[self download] cancel];

    [self setError:[NSError errorWithDomain:NSCocoaErrorDomain code:NSUserCancelledError userInfo:nil]];
    [self OE_endDownload];
}

- (void)waitUntilCompleted
{
    dispatch_semaphore_wait(_waitSemaphore, DISPATCH_TIME_FOREVER);
}
#pragma mark - NSURLDownloadDelegate
- (void)downloadDidBegin:(NSURLDownload *)download
{
    [self setError:nil];
    [self setDestinationURL:nil];
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
    [self setError:nil];
    [self OE_endDownload];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    DLog(@"%@", error);
    [self setError:error];

    [[NSFileManager defaultManager] removeItemAtURL:[self destinationURL] error:nil];
    [self setDestinationURL:nil];

    [self OE_endDownload];
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    NSString *temporaryDirectory = [NSTemporaryDirectory() stringByAppendingPathComponent:@"org.openemu.openemu/"];
    [[NSFileManager defaultManager] createDirectoryAtPath:temporaryDirectory withIntermediateDirectories:YES attributes:nil error:nil];

    [download setDestination:[temporaryDirectory stringByAppendingPathComponent:filename] allowOverwrite:YES];
}

- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)path
{
    [self setDestinationURL:[NSURL fileURLWithPath:path isDirectory:NO]];
}

- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response
{
    _totalDownloadLength = [response expectedContentLength];
    _downloadedLength = 0;

    if(_totalDownloadLength != -1)
        [self setProgress:0.0];
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length
{
    if(_totalDownloadLength != -1)
    {
        _downloadedLength += length;

        double progress = _downloadedLength / (double)_totalDownloadLength;

        // Try reducing number callbacks
        if(fabsf(progress-_progress) > 0.01)
            [self setProgress:progress];
    }
}

- (void)OE_endDownload
{
    dispatch_semaphore_signal(_waitSemaphore);
    if([self completionHandler]) [self completionHandler](self.destinationURL, self.error);
    // Make sure we only call completion handler once
    [self setCompletionHandler:nil];
}

@end
