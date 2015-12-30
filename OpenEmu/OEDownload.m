/*
 Copyright (c) 2015, OpenEmu Team
 
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

@interface OEDownload () <NSURLSessionDownloadDelegate>

@property dispatch_semaphore_t waitSemaphore;

@property (nonatomic, readwrite) CGFloat progress;

@property (readwrite) NSURL *destinationURL;
@property (readwrite) NSError *error;

@property NSURL *url;
@property NSURLSession *downloadSession;

@end

@implementation OEDownload
@synthesize progress=_progress;

- (instancetype)initWithURL:(NSURL *)url {
    
    self = [super init];
    if (self != nil) {
        _url = url;
        _waitSemaphore = dispatch_semaphore_create(0);
    }
    
    return self;
}

- (void)setProgress:(CGFloat)progress {
    
    [self willChangeValueForKey:@"progress"];
    _progress = progress;
    [self didChangeValueForKey:@"progress"];

    if (self.progressHandler && !self.progressHandler(progress)) {
        [self.downloadSession invalidateAndCancel];
        self.downloadSession = nil;
    }
}

- (CGFloat)progress {
    return _progress;
}

#pragma mark - Starting And Cancelling Downloads

- (void)startDownload {
    
    self.progress = 0.0;

    NSURLRequest *request  = [NSURLRequest requestWithURL:self.url cachePolicy:NSURLRequestReturnCacheDataElseLoad timeoutInterval:60.0];
    
    NSAssert(self.downloadSession == nil, @"There shouldn't be a previous download session.");
    
    self.downloadSession = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration] delegate:self delegateQueue:[NSOperationQueue mainQueue]];
    self.downloadSession.sessionDescription = self.url.path;
    
    NSURLSessionDownloadTask *downloadTask = [self.downloadSession downloadTaskWithRequest:request];
    
    DLog(@"Starting download (%@)", self.downloadSession.sessionDescription);
    
    [downloadTask resume];
    
    self.error = nil;
    self.destinationURL = nil;
}

- (void)cancelDownload {
    
    DLog("Cancelling download (%@)", self.downloadSession.sessionDescription);
    
    [self.downloadSession invalidateAndCancel];
}

- (void)waitUntilCompleted {
    dispatch_semaphore_wait(self.waitSemaphore, DISPATCH_TIME_FOREVER);
}

#pragma mark - NSURLDownloadDelegate

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    
    DLog(@"Download (%@) did complete: %@", session.sessionDescription, error ? error.localizedDescription : @"no errors");
    
    self.error = error;
    
    [self OE_callCompletionHandler];
    
    self.destinationURL = nil;
    
    [self.downloadSession finishTasksAndInvalidate];
    self.downloadSession = nil;
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didWriteData:(int64_t)bytesWritten totalBytesWritten:(int64_t)totalBytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite {
    
    double progress = (double)totalBytesWritten / (double)totalBytesExpectedToWrite;
    
    // Try reducing the number of callbacks.
    if (fabs(progress - self.progress) > 0.01) {
        self.progress = progress;
    }
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didFinishDownloadingToURL:(NSURL *)location {
    
    DLog(@"Download (%@) did finish downloading temporary data.", self.downloadSession.sessionDescription);
    
    NSString *temporaryFolderPath = [[NSTemporaryDirectory() stringByAppendingPathComponent:@"org.openemu.openemu/"] stringByAppendingPathComponent:[[NSUUID UUID] UUIDString]];
    NSString *destinationPath = [temporaryFolderPath stringByAppendingPathComponent:self.url.lastPathComponent];
    NSURL *destinationURL = [NSURL fileURLWithPath:destinationPath];
    
    [[NSFileManager defaultManager] createDirectoryAtPath:temporaryFolderPath withIntermediateDirectories:YES attributes:nil error:nil];
    
    [[NSFileManager defaultManager] copyItemAtURL:location toURL:destinationURL error:nil];
    
    self.destinationURL = destinationURL;
}

#pragma mark - Private

- (void)OE_callCompletionHandler {
    
    dispatch_semaphore_signal(self.waitSemaphore);
    
    if (self.completionHandler) {
        self.completionHandler(self.destinationURL, self.error);
    }
    
    // Make sure we only call completion handler once
    self.completionHandler = nil;
}

@end
