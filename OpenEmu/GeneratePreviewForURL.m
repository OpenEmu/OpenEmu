#include <Foundation/Foundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#import <CoreData/CoreData.h>
#import "OESaveState.h"

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	NSManagedObjectModel* managedObjectModel = managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles:[NSArray arrayWithObject:[NSBundle bundleWithIdentifier:@"com.openemu.savestategenerator"]]] retain];
	NSPersistentStoreCoordinator* persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: managedObjectModel];
	
	NSError *error = nil;
	if (![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:(NSURL*)url options:nil error:&error]){		
		NSLog(@"Couldn't create store");
		[managedObjectModel release];
		[persistentStoreCoordinator release];
		[pool release];
        return noErr;
	}    
	
	
	NSManagedObjectContext* managedObjectContext = [[NSManagedObjectContext alloc] init];
	[managedObjectContext setPersistentStoreCoordinator: persistentStoreCoordinator];
	
	
	NSFetchRequest *request = [[[NSFetchRequest alloc] init] autorelease];
	NSEntityDescription *entity =
    [NSEntityDescription entityForName:@"SaveState"
				inManagedObjectContext:managedObjectContext];
	[request setEntity:entity];
	
	error = nil;
	NSArray *array = [managedObjectContext executeFetchRequest:request error:&error];

	OESaveState* state = [array objectAtIndex:0];
	
	NSImage* image = [state screenshot]; //[[NSImage alloc] initWithData:[state.screenShot valueForKey:@"screenShot"]];
	
	
	NSSize canvasSize = [image size];
	
    // Preview will be drawn in a vectorized context
    CGContextRef cgContext = QLPreviewRequestCreateContext(preview, *(CGSize *)&canvasSize, true, NULL);
    if(cgContext) {
		DLog(@"Got CGContext");
        NSGraphicsContext* context = [NSGraphicsContext graphicsContextWithGraphicsPort:(void *)cgContext flipped:YES];
        if(context) {
			DLog(@"Got NSContext");
			NSGraphicsContext *gc = [NSGraphicsContext graphicsContextWithGraphicsPort:cgContext flipped:NO]; 
			[NSGraphicsContext saveGraphicsState]; 
			[NSGraphicsContext setCurrentContext:gc]; 
			[image drawAtPoint:NSZeroPoint fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0f];
			[NSGraphicsContext restoreGraphicsState];
        }
        QLPreviewRequestFlushContext(preview, cgContext);
        CFRelease(cgContext);
    }
    [pool release];
    return noErr;
}

void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview)
{
    // implement only if supported
}
