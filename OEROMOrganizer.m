/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "OEROMOrganizer.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"
#import "OEROMFile.h"

@implementation OEROMOrganizer

/**
 Creates, retains, and returns the managed object model for the application 
 by merging all of the models found in the application bundle.
 */

- (NSManagedObjectModel *)managedObjectModel
{
    if (managedObjectModel != nil) {
        return managedObjectModel;
    }
    
    managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles:nil] retain];    
    return managedObjectModel;
}


/**
 Returns the persistent store coordinator for the application.  This 
 implementation will create and return a coordinator, having added the 
 store for the application to it.  (The folder for the store is created, 
 if necessary.)
 */

- (NSPersistentStoreCoordinator *)persistentStoreCoordinator
{
    
    if (persistentStoreCoordinator != nil) {
        return persistentStoreCoordinator;
    }
    
    NSFileManager *fileManager;
    NSString *applicationSupportFolder = nil;
    NSError *error;
    
    fileManager = [NSFileManager defaultManager];
    applicationSupportFolder = [(GameDocumentController *)[NSApp delegate] applicationSupportFolder];
    if ( ![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL] )
    {
        [fileManager createDirectoryAtPath:applicationSupportFolder attributes:nil];
    }
    
    persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
    if(![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType
                                                 configuration:nil
                                                           URL:[NSURL fileURLWithPath:[applicationSupportFolder stringByAppendingPathComponent:@"ROMs.xml"] isDirectory:NO]
                                                       options:nil
                                                         error:&error]){
        NSLog(@"Persistent store fail %@",error);
        [[NSApplication sharedApplication] presentError:error];
    }
    return persistentStoreCoordinator;
}


/**
 Returns the managed object context for the application (which is already
 bound to the persistent store coordinator for the application.) 
 */

- (NSManagedObjectContext *) managedObjectContext
{
	return [[NSApp delegate] managedObjectContext];
	
    if(managedObjectContext != nil)
        return managedObjectContext;
    
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    if (coordinator != nil)
    {
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
    }
    
    return managedObjectContext;
}

- (IBAction)addROM:(id)sender
{
    NSOpenPanel *panel = [[NSOpenPanel openPanel] retain];
    [panel setAllowedFileTypes:[OECorePlugin supportedTypeExtensions]];
    [panel setAllowsMultipleSelection:YES];
    
    [panel beginSheetForDirectory:nil file:nil modalForWindow:[self window]
                    modalDelegate:self didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                      contextInfo:self];
}

- (IBAction)removeROM:(id)sender
{
    [allROMSController remove:sender];
}

- (IBAction)playROMs:(id)sender
{
    GameDocumentController *controller = (GameDocumentController *)[NSApp delegate];
    NSError *error = nil;
    
    NSArray *romFiles = [allROMSController selectedObjects];
    for(OEROMFile *romFile in romFiles)
    {
        if(![controller openDocumentWithContentsOfURL:[romFile pathURL] display:YES error:&error] || error != nil)
            NSLog(@"Error! %@", error);
        else
            [romFile setLastPlayedDate:[NSDate date]];
    }
    
    [self save];
}

- (void)save
{
    NSError *error = nil;
    [[self managedObjectContext] save:&error];
    if(error) NSLog(@"Couldn't save! %@",error);
}

- (void)openPanelDidEnd:(NSOpenPanel *)openPanel returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if(returnCode == NSOKButton)
    {
        NSArray *filenames = [openPanel filenames];
        for(NSString *file in filenames)
        {
            [OEROMFile fileWithPath:file createIfNecessary:YES inManagedObjectContext:[self managedObjectContext]];
        }

        [self save];
    }
    
    [openPanel release];
}

- (id)init
{
    return [super initWithWindowNibName:@"OEROMOrganizer"];
}

- (id)initWithWindowNibName:(NSString *)aName
{
    return [self init];
}

- (void)dealloc
{
    [super dealloc];
}

@end
