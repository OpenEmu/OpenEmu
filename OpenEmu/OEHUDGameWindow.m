//
//  OEHUDGameWindow.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 05.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDGameWindow.h"
#import "OENewGameDocument.h"
@implementation OEHUDGameWindow
@synthesize gameDocument;
- (id)initWithContentRect:(NSRect)contentRect andGameDocument:(OENewGameDocument*)gameDoc {
    
    self = [super initWithContentRect:contentRect styleMask: NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask  backing:NSBackingStoreBuffered defer:NO];
    
    //self = [super initWithContentRect:contentRect];
    if (self) {
		[self setReleasedWhenClosed:YES];
		self.gameDocument = gameDoc;
		self.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
    }
        
    return self;
}

- (void)dealloc{
	if(gameDocument){
		[gameDocument terminateEmulation];
	}
	[gameDocument release], gameDocument = nil;
	
	[super dealloc];
}

- (void)setGameDocument:(OENewGameDocument *)gameDoc{
	if(gameDoc && [gameDoc url]){
		NSURL* url = [gameDoc url];
		self.title = [[url lastPathComponent] stringByDeletingPathExtension];
	} else if(gameDoc && [gameDoc rom]){
		id rom = [gameDoc rom];
		NSString* path = [rom valueForKey:@"path"];
		self.title = [[path lastPathComponent] stringByDeletingPathExtension];
	}
	
	if([[[self contentView] subviews] count]){
		[[[[self contentView] subviews] lastObject] removeFromSuperview]; 
	}
	
	if(gameDocument){
		[gameDocument terminateEmulation];
	}
	
	[gameDoc retain];
	[gameDocument release];
	gameDocument = gameDoc;
	
	if(gameDocument){
		NSView* gameView = (NSView*)[gameDocument gameView];
		NSRect frame = [[self contentView] frame];
		frame.origin = NSMakePoint(0, 0);
		[gameView setFrame:frame];
		[[self contentView] addSubview:gameView];
	}
}

@end
