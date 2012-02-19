/*
 File:        TaskWrapper.m
 
 Description:     This is the implementation of a generalized process handling class
 that that makes asynchronous interaction with an NSTask easier.
 Feel free to make use of this code in your own applications.
 TaskWrapper objects are one-shot (since NSTask is one-shot); if you need to
 run a task more than once, destroy/create new TaskWrapper objects.
 
 Author:        EP & MCF
 
 Copyright:     © Copyright 2002 Apple Computer, Inc. 
 
 Disclaimer:    IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
 copyrights in this original Apple software (the "Apple Software"), to use,
 reproduce, modify and redistribute the Apple Software, with or without
 modifications, in source and/or binary forms; provided that if you redistribute
 the Apple Software in its entirety and without modifications, you must retain
 this notice and the following text and disclaimers in all such redistributions of
 the Apple Software.  Neither the name, trademarks, service marks or logos of
 Apple Computer, Inc. may be used to endorse or promote products derived from the
 Apple Software without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or implied,
 are granted by Apple herein, including but not limited to any patent rights that
 may be infringed by your derivative works or by other works in which the Apple
 Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
 OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
 (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Version History: 1.1/1.2 released to fix a few bugs (not always removing the notification center,
 forgetting to release in some cases)
 1.3       fixes a code error (no incorrect behavior) where we were checking for
 if (task) in the -getData: notification when task would always be true.
 Now we just do the right thing in all cases without the superfluous if check.
 */



#import "OETaskWrapper.h"


@interface OETaskWrapper ()
// This method is called asynchronously when data is available from the task's file handle.
// We just pass the data along to the controller as an NSString.
- (void)OE_didReceiveData:(NSNotification *)aNotification;
@end

@implementation OETaskWrapper

@synthesize task;

// Do basic initialization
- (id)initWithController:(id<OETaskWrapperController>)cont arguments:(NSArray *)args userInfo:(id)someInfo
{
    if((self = [super init]))
    {
        controller = cont;
        arguments  = args;
        userInfo   = someInfo;
    }
    
    return self;
}

// tear things down
- (void)dealloc
{
    [self stopProcess];
}

#define USE_EXTERNAL_FILE 0

// Here's where we actually kick off the process via an NSTask.
- (void)startProcess
{
    // We first let the controller know that we are starting
    [controller processStarted: self];
    
    task = [[NSTask alloc] init];
    
    [task setLaunchPath:[arguments objectAtIndex:0]];
    [task setArguments:[arguments subarrayWithRange: NSMakeRange (1, ([arguments count] - 1))]];
    
    [task setStandardInput:[NSPipe pipe]];
    
#if USE_EXTERNAL_FILE
    NSString *debugPath = [@"~/Desktop/HelperOutput.txt" stringByExpandingTildeInPath];
    
    if(![[NSFileManager defaultManager] fileExistsAtPath:debugPath]) [@"" writeToFile:debugPath atomically:YES encoding:NSUTF8StringEncoding error:NULL];
    
    [task setStandardOutput:[NSFileHandle fileHandleForWritingAtPath:debugPath]];
#else
    // The output of stdin, stdout and stderr is sent to a pipe so that we can catch it later
    // and use it along to the controller
    [task setStandardOutput:[NSPipe pipe]];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(OE_didReceiveData:)
                                                 name:NSFileHandleReadCompletionNotification
                                               object:[[task standardOutput] fileHandleForReading]];
    
    [[[task standardOutput] fileHandleForReading] readInBackgroundAndNotify];
#endif
    [task setStandardError:[task standardOutput]];
    
    // launch the task asynchronously
    [task launch];
}

// If the task ends, there is no more data coming through the file handle even when the notification is
// sent, or the process object is released, then this method is called.
- (void)stopProcess
{
    // If we stopped already, ignore
    if (controller == nil) return;
    
#if !USE_EXTERNAL_FILE
    // It is important to clean up after ourselves so that we don't leave potentially deallocated
    // objects as observers in the notification center; this can lead to crashes.
    [[NSNotificationCenter defaultCenter] removeObserver: self
                                                    name: NSFileHandleReadCompletionNotification
                                                  object: [[task standardOutput] fileHandleForReading]];
#endif
    // Make sure the task will actually stop!
    [task terminate];
    
#if 0
#if !USE_EXTERNAL_FILE
    NSData *data;
    
    while((data = [[[task standardOutput] fileHandleForReading] availableData]) && [data length])
    {
        [controller appendOutput: [[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] autorelease]
                     fromProcess: self];
    }
#endif
#endif
    // we tell the controller that we finished, via the callback, and then blow away our connection
    // to the controller.  NSTasks are one-shot (not for reuse), so we might as well be too.
 //   [task waitUntilExit];
    

	if(![task isRunning])
    {
		int status = [task terminationStatus];
		NSLog(@"Task termination status %i", status);
    
		[controller processFinished: self withStatus: [task terminationStatus]];
    }
	controller = nil;
}

- (void)sendToProcess:(NSString *)aString
{
    NSFileHandle *outFile = [[task standardInput] fileHandleForWriting];
    
    [outFile writeData:[aString dataUsingEncoding: NSUTF8StringEncoding]];
}

- (BOOL)isRunning
{
    return [task isRunning];
}

- (id)userInfo
{
    return userInfo;
}

- (void)OE_didReceiveData:(NSNotification *)aNotification
{
    NSData *data = [[aNotification userInfo] objectForKey: NSFileHandleNotificationDataItem];
    
    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if([data length] > 0)
    {
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
        [controller appendOutput:[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding]
                     fromProcess:self];
    }
    else
    {
        // We're finished here
        [self stopProcess];
    }
    
    // we need to schedule the file handle go read more data in the background again.
    [[aNotification object] readInBackgroundAndNotify];  
}

@end
