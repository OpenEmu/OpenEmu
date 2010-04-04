/*
 File:		TaskWrapper.m
 
 Description: 	This is the implementation of a generalized process handling class
 that that makes asynchronous interaction with an NSTask easier.
 Feel free to make use of this code in your own applications.
 TaskWrapper objects are one-shot (since NSTask is one-shot); if you need to
 run a task more than once, destroy/create new TaskWrapper objects.
 
 Author:		EP & MCF
 
 Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
 
 Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
 1.3	   fixes a code error (no incorrect behavior) where we were checking for
 if (task) in the -getData: notification when task would always be true.
 Now we just do the right thing in all cases without the superfluous if check.
 */



#import "TaskWrapper.h"


@interface TaskWrapper (PrivateMethods)
// This method is called asynchronously when data is available from the task's file handle.
// We just pass the data along to the controller as an NSString.
- (void) getData: (NSNotification *)aNotification;
@end

@implementation TaskWrapper

@synthesize task;

// Do basic initialization
- (id)initWithController: (id <TaskWrapperController>)cont arguments: (NSArray *)args userInfo: (id)someInfo
{
    if ([super init]) {
		controller	= cont;
		arguments	= [args retain];
		userInfo	= [someInfo retain];
    }
    return self;
}

// tear things down
- (void)dealloc
{
    [self stopProcess];
	
	[userInfo release];
    [arguments release];
    [task release];
	
    [super dealloc];
}

// Here's where we actually kick off the process via an NSTask.
- (void) startProcess
{
    // We first let the controller know that we are starting
    [controller processStarted: self];
	
    task = [[NSTask alloc] init];
	
    // The output of stdin, stdout and stderr is sent to a pipe so that we can catch it later
    // and use it along to the controller
    [task setStandardInput: [NSPipe pipe]];
    [task setStandardOutput: [NSPipe pipe]];
    [task setStandardError: [task standardOutput]];
	
    // The path to the binary is the first argument that was passed in
    [task setLaunchPath: [arguments objectAtIndex:0]];
    // The rest of the task arguments are just grabbed from the array
    [task setArguments: [arguments subarrayWithRange: NSMakeRange (1, ([arguments count] - 1))]];
	
    // Here we register as an observer of the NSFileHandleReadCompletionNotification, which lets
    // us know when there is data waiting for us to grab it in the task's file handle (the pipe
    // to which we connected stdout and stderr above).  -getData: will be called when there
    // is data waiting.  The reason we need to do this is because if the file handle gets
    // filled up, the task will block waiting to send data and we'll never get anywhere.
    // So we have to keep reading data from the file handle as we go.
    [[NSNotificationCenter defaultCenter] addObserver: self
											 selector: @selector(getData:)
												 name: NSFileHandleReadCompletionNotification
											   object: [[task standardOutput] fileHandleForReading]];
    // We tell the file handle to go ahead and read in the background asynchronously, and notify
    // us via the callback registered above when we signed up as an observer.  The file handle will
    // send a NSFileHandleReadCompletionNotification when it has data that is available.
    [[[task standardOutput] fileHandleForReading] readInBackgroundAndNotify];
	
    // launch the task asynchronously
    [task launch];    
}

// If the task ends, there is no more data coming through the file handle even when the notification is
// sent, or the process object is released, then this method is called.
- (void) stopProcess
{
    NSData *data;
    
	// If we stopped already, ignore
	if (!controller)
		return;
	
    // It is important to clean up after ourselves so that we don't leave potentially deallocated
    // objects as observers in the notification center; this can lead to crashes.
    [[NSNotificationCenter defaultCenter] removeObserver: self
													name: NSFileHandleReadCompletionNotification
												  object: [[task standardOutput] fileHandleForReading]];
    
    // Make sure the task will actually stop!
    [task terminate];
	
	
	while ((data = [[[task standardOutput] fileHandleForReading] availableData]) && [data length])
	{
		[controller appendOutput: [[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] autorelease]
					 fromProcess: self];
	}
	
	// we tell the controller that we finished, via the callback, and then blow away our connection
	// to the controller.  NSTasks are one-shot (not for reuse), so we might as well be too.
	[task waitUntilExit];
	
	int status = [task terminationStatus];
	NSLog(@"Task termiantion status %i", status);

	[controller processFinished: self withStatus: [task terminationStatus]];
	controller = nil;
}

- (void)sendToProcess: (NSString *)aString
{
	NSFileHandle	*outFile	= [[task standardInput] fileHandleForWriting];
	
	[outFile writeData: [aString dataUsingEncoding: NSUTF8StringEncoding]];
}

- (BOOL)isRunning
{
	return [task isRunning];
}

- (id)userInfo
{
	return userInfo;
}

@end

@implementation TaskWrapper (PrivateMethods)

- (void) getData: (NSNotification *)aNotification
{
    NSData	*data = [[aNotification userInfo] objectForKey: NSFileHandleNotificationDataItem];
	
    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if ([data length]) {
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
        [controller appendOutput: [[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] autorelease]
					 fromProcess: self];
    } else {
        // We're finished here
        [self stopProcess];
    }
    
    // we need to schedule the file handle go read more data in the background again.
    [[aNotification object] readInBackgroundAndNotify];  
}

@end

