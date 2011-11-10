//
//  OEGridBlankSlateView.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>
@class OESystemPlugin;
@interface OEGridBlankSlateView : NSView
- (id)initWithCollectionName:(NSString*)collectionName;
- (id)initWithSystemPlugin:(OESystemPlugin*)plugin;

- (void)centerInSuperview;
@end
