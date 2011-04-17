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

#import "OEROMOrganizer.h"
#import "OEGameDocumentController.h"
#import "OECorePlugin.h"
#import "OEROMFile.h"
#import "OEGameQuickLookDocument.h"

@implementation OEROMOrganizer

@synthesize searchPredicate, allROMSController, sourceList, searchField;

#pragma mark NSObject

- (id)init
{
    return [self initWithWindowNibName:@"OEROMOrganizer"];
}

- (void)dealloc
{
    [persistentStoreCoordinator release];
    [managedObjectModel release];
    [managedObjectContext release];
    
    [allROMSController release];
    [sourceList release];
    [searchPredicate release];
    [searchField release];
    [super dealloc];
}

/**
 Creates, retains, and returns the managed object model for the application 
 by merging all of the models found in the application bundle.
 */
- (NSManagedObjectModel *)managedObjectModel
{
    if(managedObjectModel == nil)
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
    if(persistentStoreCoordinator == nil)
    {
        NSFileManager *fileManager;
        NSString *applicationSupportFolder = nil;
        NSError *error;
        
        fileManager = [NSFileManager defaultManager];
        // FIXME: This should be provided by the owner of this object.
        applicationSupportFolder = [(OEGameDocumentController *) [NSApp delegate] applicationSupportFolder];
        
        [fileManager createDirectoryAtPath:applicationSupportFolder withIntermediateDirectories:YES attributes:nil error:nil];
        
        persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:[self managedObjectModel]];
        if(![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType
                                                     configuration:nil
                                                               URL:[NSURL fileURLWithPath:[applicationSupportFolder stringByAppendingPathComponent:@"ROMs.xml"]
                                                                              isDirectory:NO]
                                                           options:nil
                                                             error:&error])
        {
            NSLog(@"Persistent store fail %@", error);
            [[NSApplication sharedApplication] presentError:error];
        }
    }
    return persistentStoreCoordinator;
}


/**
 Returns the managed object context for the application (which is already
 bound to the persistent store coordinator for the application.) 
 */
- (NSManagedObjectContext *)managedObjectContext
{
    // FIXME: wtf ?
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
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setAllowedFileTypes:[OECorePlugin supportedTypeExtensions]];
    [panel setAllowsMultipleSelection:YES];
    
    [panel beginSheetModalForWindow:[self window] completionHandler:
     ^(NSInteger result)
     {
         if(result == NSOKButton)
         {
             for(NSString *file in [panel filenames])
                 [OEROMFile fileWithPath:file createIfNecessary:YES inManagedObjectContext:[self managedObjectContext]];
             
             [self save];
         }
     }];
}

- (IBAction)removeROM:(id)sender
{
    [allROMSController remove:sender];
}

- (IBAction)playROMs:(id)sender
{
    OEGameDocumentController *controller = (OEGameDocumentController *)[NSApp delegate];
    NSError *error = nil;
    
    NSArray *romFiles = [allROMSController selectedObjects];
    for(OEROMFile *romFile in romFiles)
    {
        if(![controller openDocumentWithContentsOfURL:[romFile pathURL] display:YES error:&error])
            NSLog(@"Error! %@", error);
        else
            [romFile setLastPlayedDate:[NSDate date]];
    }
    
    [self save];
}

- (IBAction)previewROM:(id)sender{
	for(OEROMFile *romFile in [allROMSController selectedObjects]){
//		NSError *error = NULL;
//		GameDocument *document = [[GameDocumentController sharedDocumentController] openDocumentWithContentsOfURL:[romFile pathURL] display:YES error:&error];
/*		OEGameQuickLookDocument *document = [[GameDocumentController sharedDocumentController] previewROMFile:romFile fromPoint:NSZeroPoint];

		if(!document && error){
			NSLog(@"Could not open Quick Look: %@",error);
		}
 */
	}
}

- (void)save
{
    NSError *error = nil;
    if(![[self managedObjectContext] save:&error]) NSLog(@"Couldn't save! %@", error);
}

- (void)setSearchPredicate:(NSPredicate *)pred
{
    DLog(@"New search predicate! %@", pred);
    [searchPredicate autorelease];
    searchPredicate = [pred retain];
    
    [self updateFilterPredicate];
}

- (void)awakeFromNib
{
    DLog(@"Binding search predicate");
    [searchField bind:NSPredicateBinding
             toObject:self
          withKeyPath:@"searchPredicate"
              options:nil];

    [sourceList reloadData];
    [sourceList expandItem:@"Consoles"];
    [sourceList selectRowIndexes:[NSIndexSet indexSetWithIndex:1]
            byExtendingSelection:NO];
}

#pragma mark NSOutlineViewDataSource

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil)
        return 1;
    else if([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Consoles"])
        return [[OEPlugin allPlugins] count] + 1;
    
    return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    if(item == nil)
        return @"Consoles";
    else if([item isKindOfClass:[NSString class]] && [item isEqualToString:@"Consoles"])
        return index == 0 ? @"All Consoles" : [[OEPlugin allPlugins] objectAtIndex:index - 1];

    return nil;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return ![item isKindOfClass:[OEPlugin class]] && ![item isEqual:@"All Consoles"];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    NSUInteger columnIndex = [[outlineView tableColumns] indexOfObject:tableColumn];
    
    if([item isKindOfClass:[NSString class]])
    {
        NSString *text = (NSString *)item;
        if([text isEqualToString:@"Consoles"])
        {
            switch(columnIndex)
            {
                case 0:
                    return [NSImage imageNamed:@"about"];
                    break;
                case 1:
                    return @"Consoles";
                    break;
            }
        }
        else if([text isEqualToString:@"All Consoles"])
        {
            switch(columnIndex)
            {
                case 0:
                    return [NSImage imageNamed:@"about"];
                    break;
                case 1:
                    return @"All Consoles";
                    break;
            }
        }
    }
    else if([item isKindOfClass:[OECorePlugin class]])
    {
        OECorePlugin *plugin = (OECorePlugin *)item;
        switch(columnIndex
               ){
            case 0:
                return [plugin icon];
                break;
            case 1:
                return [plugin displayName];
                break;
        }        
    }
    return nil;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
    return ![item isEqual:@"Consoles"];
}

#pragma mark NSOutlineViewDelegate methods

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
    return ![item isKindOfClass:[NSString class]] || [item isEqual:@"All Consoles"];
}

#define TruePredicate() [NSPredicate predicateWithFormat:@"TRUEPREDICATE"]

- (NSPredicate *)predicateForSearchTerm:(NSString *)searchTerm
{
    if([searchTerm length] == 0) return TruePredicate();
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:
                              @"(path contains[c] %@) or "
                              @"(name contains[c] %@) or "
                              @"(lastPlayedDate.description contains[c] $value)", 
                              searchTerm, searchTerm, searchTerm];
    return predicate;
}

- (NSPredicate *)predicateForSelection
{
    NSPredicate *predicate = nil;
    id item = [sourceList itemAtRow:[sourceList selectedRow]];
    
    if([item isEqual:@"All Consoles"])
        predicate = TruePredicate();
    else if([item isKindOfClass:[OECorePlugin class]])
    {
        NSArray *extensions = [item supportedTypeExtensions];
        predicate = [NSPredicate predicateWithFormat:@"path.pathExtension IN %@",extensions];
    }
    
    return predicate;
}

- (NSPredicate *)predicateForCurrentState
{
    NSPredicate *selectionPredicate = [self predicateForSelection];
    return [NSCompoundPredicate andPredicateWithSubpredicates:[NSArray arrayWithObjects:
                                                               selectionPredicate,
                                                               [self searchPredicate],
                                                               nil]];
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    [self updateFilterPredicate];
}

- (void)updateFilterPredicate
{
    NSPredicate *predicate = [self predicateForCurrentState];
    [allROMSController setFilterPredicate:predicate];    
}

@end
