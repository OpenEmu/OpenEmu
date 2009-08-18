//
//  OECoreDownloader.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 8/2/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OECoreDownloader.h"
#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>
#import "OEDownload.h"
#import "GameDocumentController.h"
#import "OECorePlugin.h"

@implementation OECoreDownloader

@synthesize downloads;

- (id) init
{
	self = [super initWithWindowNibName:@"CoreDownloader"];
	if(self != nil)
	{
		
		NSString* string = [[[NSBundle mainBundle] infoDictionary] valueForKey:@"OECoreListURL"];
		NSError *error;

		
		NSString* combinedList = [NSString stringWithContentsOfURL:[NSURL URLWithString:string] encoding:NSUTF8StringEncoding error:&error];
		
		NSArray * list = [combinedList componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
		
		NSMutableArray* tempURLList = [NSMutableArray array];
		
		NSArray* allPlugins = [OECorePlugin allPlugins];
		for( NSString* appcastString in list )
		{
			BOOL pluginExists = NO;
			
			NSURL* appcastURL = [NSURL URLWithString:appcastString];
			
			for( OECorePlugin* plugin in allPlugins )
			{
				@try {
					SUUpdater* updater =  [SUUpdater updaterForBundle: [plugin bundle] ];
					if ( [[updater feedURL] isEqual: appcastURL] )
					{
						pluginExists = YES;
						break;
					}
				}
				@catch (NSException * e) {
					
				}

			
			}
			
			if( !pluginExists )			
				[tempURLList addObject:appcastURL];
		}
		
		urlList = [NSArray arrayWithArray:tempURLList];
		
		appcasts = [[NSMutableArray alloc] init];
		
		docController = [GameDocumentController sharedDocumentController];
		downloadToPathMap = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (void)windowDidLoad
{
	[self loadAppcasts];
}

- (void) dealloc
{
	[appcasts release];
	[urlList release];
	[super dealloc];
}

- (void) loadAppcasts
{
	
	for( NSURL* appcastURL in urlList )
	{
		SUAppcast* appcast = [[SUAppcast alloc] init];
		
		[appcast setDelegate:self];
		[appcast fetchAppcastFromURL:appcastURL];
		
		[appcasts addObject:appcast];
		[appcast release];
	}
	
}

- (void)appcastDidFinishLoading:(SUAppcast *)appcast
{
	OEDownload* downlad = [[OEDownload alloc] initWithAppcast:appcast];
	
	[downloadArrayController addObject:downlad];
	[downlad release];


	NSLog(@"Finished loading");
	
}

- (void)appcast:(SUAppcast *)appcast failedToLoadWithError:(NSError *)error
{
	[appcasts removeObject:appcast];
	//NSLog(@"Failed to load appcast %@", appcast );
}

- (IBAction)downloadSelectedCores:(id)sender
{
	for(OEDownload *download in downloads)
	{
		if( [download enabled] )
		{
			NSURLRequest *request = [NSURLRequest requestWithURL:[[download appcastItem] fileURL]];
			NSURLDownload *fileDownload = [[[NSURLDownload alloc] initWithRequest:request delegate:self] autorelease];
			
			if(fileDownload != nil)
				NSLog(@"Couldn't download!??");
		}
	}
}

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)filename
{
    NSString *destinationFilename;
	
    destinationFilename=[NSString stringWithCString:tmpnam(nil) 
										   encoding:[NSString defaultCStringEncoding]];
	
	[downloadToPathMap setValue:download forKey:destinationFilename];
    [download setDestination:destinationFilename allowOverwrite:NO];
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error
{
    // release the connection
    [download release];
	
    // inform the user
    NSLog(@"Download failed! Error - %@ %@",
          [error localizedDescription],
          [[error userInfo] objectForKey:NSErrorFailingURLStringKey]);
}

- (void) download: (NSURLDownload*)download didCreateDestination: (NSString*)path
{
	 //  NSLog(@"%@",@"created dest");
}

- (void)downloadDidFinish:(NSURLDownload *)download
{
	NSString *path = nil;	
	for(NSString *key in [downloadToPathMap keyEnumerator])
		if([downloadToPathMap objectForKey:key] == download)
			path = key;
	
	XADArchive* archive = [XADArchive archiveForFile:path];
	
	NSString* appsupportFolder = [docController applicationSupportFolder];
	appsupportFolder = [appsupportFolder stringByAppendingPathComponent:@"Cores"];
	[archive extractTo:appsupportFolder];
	
    // release the connection
    [download release];
	
    // do something with the data
   // NSLog(@"downloadDidFinish to path %@",path);
	
	[[NSFileManager defaultManager] removeFileAtPath:path handler:nil];
}

@end
