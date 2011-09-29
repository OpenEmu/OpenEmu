//
//  OESetupAssistant.m
//  OpenEmu
//
//  Created by Anton Marini on 9/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistant.h"

#import "OECorePlugin.h"


@implementation OESetupAssistant

@synthesize searchResults;
@synthesize transition;
@synthesize replaceView;
@synthesize step1;
@synthesize step2;
@synthesize step3;
@synthesize step4;
@synthesize step5;
@synthesize resultTableView;
@synthesize resultProgress;
@synthesize resultFinishedLabel;
@synthesize resultController;
@synthesize dontSearchCommonTypes;
@synthesize libraryController;

- (id)init
{
    NSLog(@"Init Assistant");
    
    self = [self initWithWindowNibName:@"OESetupAssistant"];
    if (self) 
    {
    }
    
    return self;
}

- (void) awakeFromNib
{    
    self.searchResults = [[[NSMutableArray alloc] initWithCapacity:1] autorelease];
    [self.resultFinishedLabel setHidden:YES];
    
    self.transition = [CATransition animation];
    self.transition.type = kCATransitionFade;
    self.transition.subtype = kCATransitionFromRight;
    self.transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    self.transition.duration = 1.0;

	CGColorRef colorRef = CGColorCreateGenericGray(0.129, 1.0);
    [[[self.replaceView superview] layer] setBackgroundColor:colorRef];
	
    [[self.replaceView layer] setBackgroundColor:colorRef];
	CGColorRelease(colorRef);
    
    [self.replaceView setWantsLayer:YES];
    
    [self.replaceView setAnimations: [NSDictionary dictionaryWithObject:self.transition forKey:@"subviews"]];
    
    [[self.replaceView animator] addSubview:step1];
    
    [self.resultController setContent:self.searchResults];
    
    [[self window] center];
    [[self window] makeKeyAndOrderFront:nil];
}

- (IBAction) backToStep1:(id)sender
{
    [self goBackToView:self.step1];
}

- (IBAction) toStep2:(id)sender
{
    [self goForwardToView:self.step2];
}

- (IBAction) backToStep2:(id)sender
{
    [self goBackToView:self.step2];
}

- (IBAction) toStep3:(id)sender
{
    [self goForwardToView:self.step3];
}

- (IBAction) backToStep3:(id)sender
{
    [self goBackToView:self.step3];
}

- (IBAction) toStep4:(id)sender;
{
    [self goForwardToView:self.step4];
}

- (IBAction) backToStep4:(id)sender
{
    [self goBackToView:self.step4];
}

- (IBAction) toStep5:(id)sender;
{
    [self goForwardToView:self.step5];
}

- (void) goBackToView:(NSView*)view
{
    self.transition.type = kCATransitionPush;
    self.transition.subtype = kCATransitionFromLeft;
    
    NSView* subview  = [[replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

- (void) goForwardToView:(NSView*)view
{
    self.transition.type = kCATransitionPush;
    self.transition.subtype = kCATransitionFromRight;
    
    NSView* subview  = [[self.replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

- (void) dissolveToView:(NSView*)view
{
    self.transition.type = kCATransitionFade;
    
    NSView* subview  = [[replaceView subviews] objectAtIndex:0];
    
    [[self.replaceView animator] replaceSubview:subview with:view];
}

#pragma mark -
#pragma Import Rom Discovery

- (IBAction)discoverRoms:(id)sender
{

    NSMutableArray* supportedFileExtensions = [[OECorePlugin supportedTypeExtensions] mutableCopy];
    
    if([dontSearchCommonTypes state] == NSOnState)
    {
        NSArray* commonTypes = [NSArray arrayWithObjects:@"bin", @"zip", @"elf", nil];
        
        [supportedFileExtensions removeObjectsInArray:commonTypes];
    }
    
    NSLog(@"Supported search Extensions are: %@", supportedFileExtensions);

    NSString* searchString = @"";
    for(NSString* extension in supportedFileExtensions)
    {
        searchString = [searchString stringByAppendingFormat:@"(kMDItemDisplayName == *.%@)", extension, nil];
        searchString = [searchString stringByAppendingString:@" || "];
    }
    
    searchString = [searchString substringWithRange:NSMakeRange(0, [searchString length] - 4)];
    
    NSLog(@"SearchString: %@", searchString);
    
    MDQueryRef searchQuery = MDQueryCreate(kCFAllocatorDefault, (CFStringRef)searchString, NULL, NULL /* sortArray */);
            
    if(searchQuery)
    {
        NSLog(@"Valid search query ref");
        
        [self.searchResults removeAllObjects];
        [self.resultProgress startAnimation:nil];
        [self.resultProgress setHidden:NO];
        [self.resultFinishedLabel setHidden:YES];    

        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(finalizeSearchResults:)
                                                     name:(NSString*)kMDQueryDidFinishNotification
                                                   object:(id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryProgressNotification
                                                   object:(id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryDidUpdateNotification
                                                   object:(id)searchQuery];
    
        MDQuerySetSearchScope(searchQuery, (CFArrayRef) [NSArray arrayWithObject:(NSString*) kMDQueryScopeComputer], 0);
        
//        MDQueryBatchingParams batchParam = MDQueryGetBatchingParameters(searchQuery);
//        
//        batchParam.first_max_num = 1;
//        batchParam.progress_max_num = 1;
//        batchParam.update_max_num = 1;
//        
//        MDQuerySetBatchingParameters(searchQuery, batchParam);
        
        if(MDQueryExecute(searchQuery, kMDQueryWantsUpdates))
            NSLog(@"Searching for importable roms");
        
        [self.resultProgress startAnimation:nil];
        
        [self dissolveToView:self.step5];
    }
}

- (void) updateSearchResults:(NSNotification*)notification
{    
    MDQueryRef searchQuery = (MDQueryRef)[notification object]; 

    MDItemRef resultItem = NULL;
    NSString* resultPath = nil;
    
    // assume the latest result is the last index?
    CFIndex index = 0;
    
    for(index = 0; index < MDQueryGetResultCount(searchQuery); index++)
    {
        resultItem = (MDItemRef)MDQueryGetResultAtIndex(searchQuery, index);
        resultPath = (NSString*)MDItemCopyAttribute(resultItem, kMDItemPath);
    
        // Check to see if the path contains any in appropriate dirctories, like /tmp, /Library /System
        //NSString* pathRoot = [[resultPath pathComponents] objectAtIndex:1]; // (0 = "/")
        
        NSArray* dontTouchThisDunDunDunDunHammerTime = [NSArray arrayWithObjects:
                                                        @"System",
                                                        @"Library",
                                                        @"Developer",
                                                        @"Volumes",
                                                        @"Applications",
                                                        @"bin",
                                                        @"cores",
                                                        @"dev",
                                                        @"etc",
                                                        @"home",
                                                        @"net",
                                                        @"sbin",
                                                        @"private",
                                                        @"tmp",
                                                        @"usr",
                                                        @"var",
                                                        nil];
        // Nothing in common
        if(![[resultPath pathComponents] firstObjectCommonWithArray:dontTouchThisDunDunDunDunHammerTime])
        {                
            if(![[self.resultController content] containsObject:resultPath])
            {
                NSMutableDictionary* resultDict = [NSMutableDictionary dictionary];

                [resultDict setValue:resultPath forKey:@"Path"];
                [resultDict setValue:[[resultPath lastPathComponent] stringByDeletingPathExtension] forKey:@"Name"];
                
                [self.resultController addObject:resultDict];
            
                //NSLog(@"found %@", resultPath);
                
            }
        }

        [resultPath release];
        resultPath = nil;
    
    } 
}

- (void) finalizeSearchResults:(NSNotification*)notification
{
    MDQueryRef searchQuery = (MDQueryRef)[notification object]; 

    NSLog(@"Finished searching, found: %lu items", MDQueryGetResultCount(searchQuery));
    
    [self.resultFinishedLabel setStringValue:[NSString stringWithFormat:@"Found %i Games", [[self.resultController content] count], nil]]; 
    
    [self.resultProgress stopAnimation:nil];
    [self.resultProgress setHidden:YES];
    [self.resultFinishedLabel setHidden:NO];    
}

- (IBAction) import:(id)sender;
{
    [[self window] close];
    
    NSMutableArray* URLS = [NSMutableArray array];
    
    for(NSDictionary* searchResult in self.searchResults)
    {
        [URLS addObject:[NSURL fileURLWithPath:[searchResult objectForKey:@"Path"]]];
    }
        
    [libraryController startImportSheet:URLS];
}


@end
