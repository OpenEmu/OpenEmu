//
//  OEPreferencePane.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol OEPreferencePane <NSObject>
- (NSImage*)icon;
- (NSString*)title;
- (NSString*)localizedTitle;

- (NSSize)viewSize;

@optional
- (NSColor*)toolbarSeparationColor;
@end
