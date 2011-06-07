//
//  CoreView.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 31.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class CoreItem;
@interface CoreView : NSView {
@private
    NSArray* cores;
	
	CoreItem* coreToBeExpanded;
	
	IBOutlet NSView* sampleCoreSettings;
}

@end

@interface CoreItem : NSObject {
@private
	BOOL collapsed;
	NSString* title;
	NSString* version;
	
	
	NSRect btnRect;
	NSSize detailViewSize;
	NSView* detailView;
}
@property (readwrite) BOOL collapsed;
@property (copy, readwrite) NSString* title;
@property (copy, readwrite) NSString* version;
@property (readwrite) NSSize detailViewSize;
@property (retain, readwrite) NSView* detailView;

@property (readwrite) NSRect btnRect;
@end